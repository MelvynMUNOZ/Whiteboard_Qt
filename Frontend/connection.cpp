#include "connection.h"
#include "ihmfrontend.h"
#include "client.h"
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

connection::connection(QWidget *parent)
    : QWidget{parent}
{
    createConnectionLayout();
}

void connection::createConnectionLayout(){
    vBoxGeneral = new QVBoxLayout(this);
    labelTitle = new QLabel("Connection to Canva", this);
    labelTitle->setObjectName("labelTitle");
    labelPseudo = new QLabel("Username : ", this);
    lineEditPseudo = new QLineEdit(this);

    labelIP = new QLabel("Adresse IP : ", this);
    lineEditIP = new QLineEdit(this);
    pushButtonConnection = new QPushButton("Connection", this);
    pushButtonConnection->setObjectName("pushButtonConnection");

    //Alignements
    vBoxGeneral->setAlignment(Qt::AlignCenter);
    labelTitle->setAlignment(Qt::AlignCenter);

    //Espaces
    vBoxGeneral->setSpacing(10);

    //Tailles
    lineEditPseudo->setFixedWidth(250);
    lineEditIP->setFixedWidth(250);
    pushButtonConnection->setFixedWidth(250);

    //Ajout au layout
    vBoxGeneral->addStretch();
    vBoxGeneral->addWidget(labelTitle, 0, Qt::AlignTop);
    vBoxGeneral->addSpacing(50);
    vBoxGeneral->addWidget(labelPseudo, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(lineEditPseudo, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(labelIP, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(lineEditIP, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(pushButtonConnection, 0, Qt::AlignTop);
    vBoxGeneral->addStretch();

    setLayout(vBoxGeneral);

    QObject::connect(pushButtonConnection, &QPushButton::clicked, this, &connection::on_pushButtonConnection_clicked);

    messageConnectionKO = new QMessageBox();
    messageConnectionKO->setText("The IP adress have to be : X.X.X.X");
    messageConnectionKO->setIcon(QMessageBox::Critical);
    messageConnectionKO->setWindowTitle("Error");

    messageConnectionWAIT = new QMessageBox();
    messageConnectionWAIT->setText("Connection...");
    messageConnectionWAIT->setIcon(QMessageBox::Information);
    messageConnectionWAIT->setWindowTitle("Connection to canva");
    messageConnectionWAIT->setStandardButtons(QMessageBox::NoButton);
}

void connection::on_pushButtonConnection_clicked(){
    QString pseudo = lineEditPseudo->text();
    QString ip = lineEditIP->text();

    bool checkPseudo = pseudo.isEmpty();
    bool checkIP = ip.isEmpty();

    QRegularExpression regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionMatch matchIP = regex.match(ip);
    bool correctIP = matchIP.hasMatch();

    if(checkPseudo){
        lineEditPseudo->setStyleSheet("border: 1 solid red; border-radius: 2px; padding: 2px;");
    }else{
        lineEditPseudo->setStyleSheet("border: 1 solid green; border-radius: 2px; padding: 2px;");
    }
    if(checkIP || !correctIP){
        lineEditIP->setStyleSheet("border: 1 solid red; border-radius: 2px; padding: 2px;");
        connection::messageBox_IP();
    }else{
        lineEditIP->setStyleSheet("border: 1 solid green; border-radius: 2px; padding: 2px;");
    }

    if(!checkPseudo && !checkIP && correctIP){
        connection::connectionCanva();
    }
}

void connection::messageBox_IP(){
    messageConnectionKO->exec();
}

void connection::connectionCanva(){
    //try_to_connect = 0;
    messageConnectionWAIT->show();
    isConnected = false;

    QEventLoop loop;
    QTimer timer;
    //Attent la connexion au canva
    // connect(&timer, &QTimer::timeout, [&](){
    //     qDebug() << "Try to connect : " << try_to_connect;
    //     try_to_connect++;
    //     if(isConnected || try_to_connect > timeout){
    //         qDebug() << "End : try to connect : " << try_to_connect;
    //         loop.quit();
    //         try_to_connect = 0;
    //         emit connectionSuccessful();
    //     }
    // });
    // timer.start(500);
    // loop.exec();
    if(connectionToServer()){
        emit connectionSuccessful();
    }
    //Ferme le messageBox d'attente de connexion
    messageConnectionWAIT->hide();
}

bool connection::connectionToServer(){
    //Creer la socket pour se connecter au serveur
    globalDataClient.tcp_socket = new QTcpSocket();
    globalDataClient.tcp_socket->connectToHost(lineEditIP->text(), TCP_PORT);;

    if (!globalDataClient.tcp_socket->waitForConnected(5000)) {
        qDebug() << "Échec de la connexion au serveur :" << globalDataClient.tcp_socket->errorString();
        return false;
    }

    qDebug() << "Connexion réussie au serveur" << lineEditIP->text() << "sur le port" << TCP_PORT;
    connect(globalDataClient.tcp_socket, &QTcpSocket::readyRead, this, &connection::onTCPReadyRead);

    globalDataClient.udp_socket = new QUdpSocket();
    if (globalDataClient.udp_socket->bind(QHostAddress::Any, UDP_PORT_CLIENT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        qInfo() << "Started UDP socket on port" << UDP_PORT_CLIENT;
        qInfo() << "Started UDP socket on port" << static_cast<quint16>(globalDataClient.udp_socket->localPort());
        connect(globalDataClient.udp_socket, &QUdpSocket::readyRead, globalDataClient.whiteboardWidget, &whiteboard::onUdpReadyRead);
    }
    else
    {
        qCritical() << "Failed to start UDP socket on port" << UDP_PORT_CLIENT;
    }

    globalDataClient.client_infos = new QHash<int, Client*>();

    return true;
}

void connection::onTCPReadyRead(){
    QTcpSocket *server_socket = qobject_cast<QTcpSocket *>(sender());
    if (!server_socket)
    {
        qDebug() << server_socket;
        return;
    }

    while (server_socket->canReadLine())
    {
        QByteArray data = server_socket->readLine();
        if (!data.isEmpty() && data.length() >= (qsizetype)TCP_FRAME_MIN_LEN)
        {
            processTcpFrame(data);
        }
    }
}

void connection::processTcpFrame(const QByteArray &data){
    auto type = static_cast<MessageType>(data[0]);
    int id = qFromBigEndian(*reinterpret_cast<const int*>(data.mid(1, 4).data()));
    QByteArray payload = data.mid(5);

    if (!payload.isEmpty() && payload.endsWith('\n')) {
        payload.chop(1);  // Retire le dernier caractère
    }

    qDebug() << ">>> TCP from" << globalDataClient.tcp_socket->peerAddress().toString() << "port" << TCP_PORT
             << "| Type:" << type
             << "| Id:" << id
             << "| Payload:" << payload;

    // qDebug() << ">> TCP from " << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
    //          << "|" << data.toHex(' ');

    switch (type)
    {
    case ACK_CONNECT:
        globalDataClient.my_client = new Client(id, globalDataClient.tcp_socket, QColor(QString::fromUtf8(payload)));
        qDebug() << "id : " << globalDataClient.my_client->getId()
                 << " ; color : " << globalDataClient.my_client->getColor().name();
        //Met la couleur associee au client
        globalDataClient.my_client_pen = new QPen();
        globalDataClient.my_client_pen->setCapStyle(Qt::RoundCap);
        globalDataClient.my_client_pen->setBrush(globalDataClient.my_client->getColor());
        globalDataClient.my_client_pen->setWidth(3);
        registerClientMessage();
        break;

    case ACK_REGISTER_CLIENT:
        // globalDataClient.my_client = new Client(id, globalDataClient.tcp_socket, QColor(QString::fromUtf8(payload)));
        // qDebug() << "id : " << globalDataClient.my_client->getId()
        //          << " ; color : " << globalDataClient.my_client->getColor().name();
        // registerClientMessage();
        qDebug() << "ACK_REGISTER_CLIENT received";
        registerUDPPortMessage();
        break;

    case ACK_REGISTER_UDP_PORT:
        qDebug() << "ACK_REGISTER_UDP_PORT received";
        requestAllClientInfoMessage();
        break;

    case CLIENT_INFOS:
    {
        qDebug() << "CLIENT_INFOS received";
        int id_new_client = getClientInfos(payload);
        emit getClientInfosSignal(id_new_client);
        break;
    }

    // case REGISTER_UDP_PORT:
    //     client->setUdpPort(qFromBigEndian(payload.toUShort()));
    //     sendAckRegisterUdpPort(client);
    //     break;

    // case REQUEST_ALL_CLIENTS_INFOS:
    //     // TODO: Envoi d'une réponse TCP SEND_CLIENTS_INFOS par client enregistré, avec comme données : id client, color (hex), nom
    //     // broadcastClientsInfos();
    //     break;

    default:
        break;
    }
}

void connection::registerClientMessage(){
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(MessageType::REGISTER_CLIENT); // Type du message
    stream << static_cast<quint32>(globalDataClient.my_client->getId());  // ID du client qui se déconnecte (4 bytes Big Endian)
    message.append(lineEditPseudo->text().toUtf8());  // Nom du client
    message.append('\n');

    globalDataClient.tcp_socket->write(message);
    globalDataClient.tcp_socket->flush();
}

void connection::requestAllClientInfoMessage(){
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(MessageType::REQUEST_ALL_CLIENT_INFOS); // Type du message
    stream << static_cast<quint32>(globalDataClient.my_client->getId());
    message.append('\n');

    globalDataClient.tcp_socket->write(message);
    globalDataClient.tcp_socket->flush();
}

int connection::getClientInfos(QByteArray payload){
    qDebug() << "payload  : " << payload;
    int id_client = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(0, 3).data()));
    int colorStartIndex = payload.indexOf('#');
    if (colorStartIndex == -1 || colorStartIndex + 7 > payload.size()) {
        qWarning() << "Invalid color format in payload.";
        return -1;
    }
    QString colorHex = QString::fromUtf8(payload.mid(colorStartIndex, 7));
    QString name = QString::fromUtf8(payload.mid(colorStartIndex+7));
    qDebug() << "name = " << name;
    Client *new_client = new Client(id_client, NULL, QColor(colorHex));
    new_client->setName(name);
    globalDataClient.client_infos->insert(id_client, new_client);
    //afficher QHash
    // for (auto it = globalDataClient.client_infos->begin(); it != globalDataClient.client_infos->end(); ++it) {
    //     int clientId = it.key();
    //     Client* client = it.value();
    //     if (client) {
    //         qDebug() << "ID:" << clientId
    //                  << "Nom:" << client->getName();
    //     } else {
    //         qDebug() << "ID:" << clientId << "-> Client NULL";
    //     }
    // }
    return id_client;
}

void connection::registerUDPPortMessage(){
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(MessageType::REGISTER_UDP_PORT); // Type du message
    stream << static_cast<quint32>(globalDataClient.my_client->getId());
    stream << static_cast<quint16>(UDP_PORT_CLIENT);  // Numero du port UDP
    message.append('\n');

    globalDataClient.tcp_socket->write(message);
    globalDataClient.tcp_socket->flush();
}
