#include "whiteboard.h"
#include "ihmfrontend.h"
#include <QObject>

whiteboard::whiteboard(QWidget *parent)
    : QWidget{parent}
    ,imageWhiteboard(new QImage(QGuiApplication::primaryScreen()->geometry().size(),
                                   QImage::Format_ARGB32_Premultiplied))
{
    setWindowFlags(Qt::WindowMinimizeButtonHint);
    createWhiteboardLayout();

    painterWhiteboard = new QPainter(imageWhiteboard);
    enable = false;

    QObject::connect(pushButtonPen, &QPushButton::clicked, this, &whiteboard::on_pushButtonPen_clicked);
    QObject::connect(pushButtonRubber, &QPushButton::clicked, this, &whiteboard::on_pushButtonRubber_clicked);
}

void whiteboard::createWhiteboardLayout(){
    vBoxGeneral = new QVBoxLayout(this);
    //Labels
    labelTitle = new QLabel("Canva", this);
    labelTitle->setObjectName("labelTitle");
    labelTitle->setAlignment(Qt::AlignCenter);

    //PushButton
    hBoxChoices = new QHBoxLayout(); //pas de parent pour ne pas avoir de conflit
    pushButtonPen = new QPushButton("Pen", this);
    pushButtonRubber = new QPushButton("Rubber", this);

    //ListPseudo
    hBoxWhiteboard = new QHBoxLayout(); //pas de parent pour ne pas avoir de conflit
    listPseudo = new QListWidget(this);
    listPseudo->setFixedWidth(150);

    vBoxGeneral->addWidget(labelTitle);

    hBoxChoices->addWidget(pushButtonPen);
    hBoxChoices->addWidget(pushButtonRubber);
    vBoxGeneral->addLayout(hBoxChoices);

    QWidget *whiteboardWidget = new QWidget(this);
    whiteboardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    hBoxWhiteboard->addWidget(whiteboardWidget);
    hBoxWhiteboard->addWidget(listPseudo);

    vBoxGeneral->addLayout(hBoxWhiteboard);

    setLayout(vBoxGeneral);
}

void whiteboard::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.drawImage(0,0, *imageWhiteboard);
}

void whiteboard::mousePressEvent(QMouseEvent *event){
    enable = true;
    pointBegin = event->pos();
}

void whiteboard::mouseMoveEvent(QMouseEvent *event){
    if(!enable){
        event->accept();
        return;
    }
    pointEnd = event->pos();
    painterWhiteboard->setPen(*globalDataClient.my_client_pen);
    painterWhiteboard->drawLine(pointBegin, pointEnd);
    dataCanvasClients(pointBegin, pointEnd);
    pointBegin = pointEnd;
    update();
}

void whiteboard::mouseReleaseEvent(QMouseEvent *event){
    enable = false;
}

void whiteboard::on_pushButtonPen_clicked(){
    writer = true;
    globalDataClient.my_client_pen->setBrush(globalDataClient.my_client->getColor());
    globalDataClient.my_client_pen->setWidth(3);
}

void whiteboard::on_pushButtonRubber_clicked(){
    writer = false;
    //Met la couleur du crayon en blanc pour donner l'impression de gommer les traits colores
    globalDataClient.my_client_pen->setBrush(QColor(255,255,255));
    globalDataClient.my_client_pen->setWidth(10);
}

void whiteboard::updateListClientInfos(int id_client){
    //ajoute dans la listWidget les noms des clients
    Client *clientPseudo = globalDataClient.client_infos->find(id_client).value();
    QString name = clientPseudo->getName();
    QColor color = clientPseudo->getColor();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setForeground(color);
    listPseudo->addItem(item);
}

/******************* UDP **********************/

void whiteboard::onUdpReadyRead(){
    while (globalDataClient.udp_socket->hasPendingDatagrams())
    {
        QByteArray buffer;
        buffer.resize(globalDataClient.udp_socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        if (globalDataClient.udp_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort) > 0)
        {
            if (buffer.size() >= (qsizetype)UDP_FRAME_MIN_LEN)
            {
                processUdpFrame(sender, senderPort, buffer);
            }
        }
    }
}

void whiteboard::processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data)
{
    auto type = static_cast<MessageType>(data[0]);
    auto payload = data.sliced(1);

    if (!payload.isEmpty() && payload.endsWith('\n')) {
        payload.chop(1);  // Retire le dernier caractère
    }

    qDebug() << ">>> UDP from" << sender.toString() << "port" << sender_port
             << "| Type:" << type
             << "| Payload:" << payload;

    switch (type)
    {
    case DATA_CANVAS_SYNC:
        qDebug() << "DATA_CANVAS_SYNC received";
        dataCanvasSync(payload);
        break;

    default:
        break;
    }
}

void whiteboard::dataCanvasClients(QPoint pointBegin, QPoint pointEnd){
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(MessageType::DATA_CANVAS_CLIENT); // Type du message
    stream << static_cast<int>(pointBegin.x());
    stream << static_cast<int>(pointBegin.y());
    stream << static_cast<int>(pointEnd.x());
    stream << static_cast<int>(pointEnd.y());
    stream << static_cast<quint32>(globalDataClient.my_client_pen->width());
    message.append(globalDataClient.my_client_pen->color().name(QColor::HexRgb).toUtf8());
    message.append('\n');

    globalDataClient.udp_socket->writeDatagram(message, message.size(), globalDataClient.tcp_socket->peerAddress(), UDP_PORT);
    globalDataClient.udp_socket->flush();
}

void whiteboard::dataCanvasSync(const QByteArray &payload){
    int XBegin = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(0, 4).data()));
    int YBegin = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(4, 8).data()));
    int XEnd = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(8, 12).data()));
    int YEnd = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(12, 16).data()));
    int penWidth = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(16,20).data()));
    int colorStartIndex = payload.indexOf('#');
    if (colorStartIndex == -1 || colorStartIndex + 7 > payload.size()) {
        qWarning() << "Invalid color format in payload.";
    }
    QString colorHex = QString::fromUtf8(payload.mid(colorStartIndex, 7));

    QPen *pen_client = new QPen();
    pen_client->setCapStyle(Qt::RoundCap);
    pen_client->setBrush(QColor(colorHex));
    pen_client->setWidth(penWidth);

    //retrace les lignes des autres clients avec le meme crayon (taille, couleur)
    painterWhiteboard->setPen(*pen_client);
    painterWhiteboard->drawLine(XBegin, YBegin, XEnd, YEnd);
}

