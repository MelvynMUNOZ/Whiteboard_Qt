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

    //Creation messageBox pour l'erreur en cas de format d'adresse IP incorrecte
    messageConnectionKO = new QMessageBox();
    messageConnectionKO->setText("The IP adress have to be : X.X.X.X");
    messageConnectionKO->setIcon(QMessageBox::Critical);
    messageConnectionKO->setWindowTitle("Error");

    //Creation messageBox pour l'attente pendant la connexion au serveur
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

    //Regex pour la verification du format de l'adresse IP
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
        connection::connectionToServer();
    }
}

void connection::messageBox_IP(){
    messageConnectionKO->exec();
}

void connection::onConnectedOK(){
    //Ferme le messageBox d'attente de connexion
    messageConnectionWAIT->hide();
    qInfo() << "Connexion réussie au serveur" << lineEditIP->text() << "sur le port" << TCP_PORT;
    connect(globalDataClient.tcp_socket, &QTcpSocket::readyRead, this, &connection::onTCPReadyRead);

    globalDataClient.udp_socket = new QUdpSocket();
    if (globalDataClient.udp_socket->bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        qInfo() << "Connexion socket UDP sur le port : " << globalDataClient.udp_socket->localPort();
        connect(globalDataClient.udp_socket, &QUdpSocket::readyRead, globalDataClient.whiteboardWidget, &whiteboard::onUdpReadyRead);
    }
    else
    {
        qCritical() << "Echec de connexion socket UDP sur le port : " << globalDataClient.udp_socket->localPort();
    }

    globalDataClient.client_infos = new QHash<int, Client*>();
    emit connectionSuccessful();
}

void connection::onConnectedKO(){
    qWarning() << "Échec de la connexion au serveur :" << globalDataClient.tcp_socket->errorString();
    //Ferme le messageBox d'attente de connexion
    messageConnectionWAIT->hide();
}

void connection::connectionToServer(){
    messageConnectionWAIT->show();
    //Creer la socket pour se connecter au serveur
    globalDataClient.tcp_socket = new QTcpSocket();

    connect(globalDataClient.tcp_socket, &QTcpSocket::connected, this, &connection::onConnectedOK);
    connect(globalDataClient.tcp_socket, &QTcpSocket::errorOccurred, this, &connection::onConnectedKO);

    //Si au bout de 5 secondes la connexion n'est pas etabli alors on leve un warning
    QTimer::singleShot(5000, this, &connection::onConnectedKO);

    globalDataClient.tcp_socket->connectToHost(lineEditIP->text(), TCP_PORT);
}

void connection::onTCPReadyRead(){
    //Recupere la socket du serveur
    QTcpSocket *server_socket = qobject_cast<QTcpSocket *>(sender());
    if (!server_socket)
    {
        qDebug() << server_socket;
        return;
    }

    //Lecture de la trame ligne par ligne
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

    //Action differente en fonction de chaque type de message
    switch (type)
    {
    //Recupere la couleur associe au client
    case ACK_CONNECT:
        globalDataClient.my_client = new Client(id, globalDataClient.tcp_socket, QColor(QString::fromUtf8(payload)));
        qDebug() << "id : " << globalDataClient.my_client->getId()
                 << "| color : " << globalDataClient.my_client->getColor().name();
        //Met la couleur associee au client pour l'ecriture
        globalDataClient.my_client_pen = new QPen();
        globalDataClient.my_client_pen->setCapStyle(Qt::RoundCap);
        globalDataClient.my_client_pen->setBrush(globalDataClient.my_client->getColor());
        globalDataClient.my_client_pen->setWidth(3);

        registerClientMessage();
        break;
    //Envoi du nom du client au serveur
    case ACK_REGISTER_CLIENT:
        qDebug() << "ACK_REGISTER_CLIENT received";
        registerUDPPortMessage();
        break;
    //Demande au serveur de lui envoyer toutes les informations des clients connectes au serveur
    case ACK_REGISTER_UDP_PORT:
        qDebug() << "ACK_REGISTER_UDP_PORT received";
        requestAllClientInfoMessage();
        break;
    //Recupere toutes les informations de tous les clients connectes au serveur
    case CLIENT_INFOS:
    {
        qDebug() << "CLIENT_INFOS received";
        int id_new_client = getClientInfos(payload);
        emit getClientInfosSignal(id_new_client);
        break;
    }

    default:
        break;
    }
}

void connection::registerClientMessage(){
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(MessageType::REGISTER_CLIENT); // Type du message
    stream << static_cast<quint32>(globalDataClient.my_client->getId());  // ID du client
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
    stream << static_cast<quint32>(globalDataClient.my_client->getId()); // ID du client
    message.append('\n');

    globalDataClient.tcp_socket->write(message);
    globalDataClient.tcp_socket->flush();
}

int connection::getClientInfos(QByteArray payload){
    qDebug() << "payload  : " << payload;
    int id_client = qFromBigEndian(*reinterpret_cast<const int*>(payload.mid(0, 4).data())); // ID du client reçu
    int colorStartIndex = payload.indexOf('#');
    if (colorStartIndex == -1 || colorStartIndex + 7 > payload.size()) {
        qWarning() << "Invalid color format in payload.";
        return -1;
    }
    QString colorHex = QString::fromUtf8(payload.mid(colorStartIndex, 7)); //Couleur du client reçu
    QString name = QString::fromUtf8(payload.mid(colorStartIndex+7)); // Nom du client reçu
    Client *new_client = new Client(id_client, NULL, QColor(colorHex));
    new_client->setName(name);
    globalDataClient.client_infos->insert(id_client, new_client);
    return id_client;
}

void connection::registerUDPPortMessage(){
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(MessageType::REGISTER_UDP_PORT); // Type du message
    stream << static_cast<quint32>(globalDataClient.my_client->getId()); //ID du client
    stream << static_cast<quint16>(globalDataClient.udp_socket->localPort());  // Numero du port UDP
    message.append('\n');

    globalDataClient.tcp_socket->write(message);
    globalDataClient.tcp_socket->flush();
}
