#include "whiteboardserver.h"

#include <QNetworkInterface>
#include <QtEndian>
#include <QRandomGenerator>

int WhiteboardServer::nextClientId = 1;

WhiteboardServer::WhiteboardServer(QObject *parent)
    : QObject(parent), m_tcp_server(new QTcpServer(this)), m_udp_socket(new QUdpSocket(this))
{
}

WhiteboardServer::~WhiteboardServer()
{
    qDeleteAll(m_clients);
    m_clients.clear();

    m_tcp_server->close();
    m_tcp_server->deleteLater();

    m_udp_socket->close();
    m_udp_socket->deleteLater();
}

void WhiteboardServer::start()
{
    qInfo() << "Host IP address:" << getHostIpAddress();

    /***** TCP *****/

    if (m_tcp_server->listen(QHostAddress::Any, TCP_PORT))
    {
        qInfo() << "Started TCP server on port" << TCP_PORT;
    }
    else
    {
        qCritical() << "Failed to start TCP server on port" << TCP_PORT;
    }

    connect(m_tcp_server, &QTcpServer::newConnection, this, &WhiteboardServer::onTcpNewConnection);

    /***** UDP *****/

    if (m_udp_socket->bind(QHostAddress::Any, UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        qInfo() << "Started UDP socket on port" << UDP_PORT;
    }
    else
    {
        qCritical() << "Failed to start UDP socket on port" << UDP_PORT;
    }

    connect(m_udp_socket, &QUdpSocket::readyRead, this, &WhiteboardServer::onUdpReadyRead);
}

void WhiteboardServer::processTcpFrame(Client *client, const QByteArray &data)
{
    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    int id = qFromBigEndian(*reinterpret_cast<const int*>(data.mid(1, 4).data()));
    QByteArray payload = data.sliced(5);

    qDebug() << ">>> TCP from" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "| Type:" << type
             << "| Id:" << id
             << "| Payload:" << payload;

    // qDebug() << ">> TCP from " << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
    //          << "|" << data.toHex(' ');

    switch (type)
    {
    case REGISTER_CLIENT:
        client->setName(QString::fromUtf8(payload));
        sendAckRegisterClient(client);
        break;

    case REGISTER_UDP_PORT:
        client->setUdpPort(qFromBigEndian(payload.toUShort()));
        sendAckRegisterUdpPort(client);
        break;

    case REQUEST_ALL_CLIENTS_INFOS:
        // TODO: Envoi d'une réponse TCP SEND_CLIENTS_INFOS par client enregistré, avec comme données : id client, color (hex), nom
        // broadcastClientsInfos();
        break;

    default:
        break;
    }
}

void WhiteboardServer::processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data)
{
    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    int id = qFromBigEndian(*reinterpret_cast<const int*>(data.mid(1, 4).data()));
    auto payload = data.sliced(5);

    qDebug() << ">>> UDP from" << sender.toString() << "port" << sender_port
             << "| Type:" << type
             << "| Id:" << id
             << "| Payload:" << payload;

    // qDebug() << ">> UDP from " << sender.toString() << "port" << sender_port
    //          << "|" << data.toHex(' ');

    switch (type)
    {
    case DATA_CANVAS_CLIENT:
        // TODO: Reception et enregistrement donnees Canvas, puis broadcast des nouvelles données a tous les clients
        break;

    default:
        break;
    }
}

void WhiteboardServer::sendAckRegisterClient(Client *client)
{
    if (!client)
    {
        return;
    }

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::ACK_REGISTER_UDP_PORT);  // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client (4 bytes)
    message.append(client->getColor().name(QColor::HexRgb).toUtf8());  // La couleur du client en hex

    client->getTcpSocket()->write(message);
    client->getTcpSocket()->flush();

    // qDebug() << "<<< TCP to" << client->getTcpSocket()->peerAddress() << "port" << client->getTcpSocket()->peerPort()
    //          << "| ACK_REGISTER_CLIENT"
    //          << "| Id:" << client->getId()
    //          << "| Color:" << colorHex;

    qDebug() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "|" << message.toHex(' ');
}

void WhiteboardServer::sendAckRegisterUdpPort(Client *client)
{
    if (!client)
    {
        return;
    }

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::ACK_REGISTER_UDP_PORT);  // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client (4 bytes)
    stream << static_cast<quint16>(client->getUdpPort());  // Port UDP du client (2 bytes)

    client->getTcpSocket()->write(message);
    client->getTcpSocket()->flush();

    // qDebug() << "<<< UDP to" << client->getTcpSocket()->peerAddress() << "port" << client->getTcpSocket()->peerPort()
    //          << "| ACK_REGISTER_UDP_PORT"
    //          << "| Id:" << client->getId()
    //          << "| UDP Port:" << client->getUdpPort();

    qDebug() << "<<< UDP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getUdpPort()
             << "|" << message.toHex(' ');
}

void WhiteboardServer::broadcastClientDisconnected(Client *client)
{
    if (!client)
    {
        return;
    }

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::CLIENT_DISCONNECTED); // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client qui se déconnecte (4 bytes Big Endian)
    message.append(client->getName().toUtf8());  // Nom du client

    // Envoi à tous les clients sauf celui qui se deconnecte
    for (const auto &otherClient : std::as_const(m_clients))
    {
        if (otherClient != client && otherClient->getTcpSocket())
        {
            otherClient->getTcpSocket()->write(message);
            otherClient->getTcpSocket()->flush();

            // qDebug() << "<<< UDP to" << client->getTcpSocket()->peerAddress() << "port" << client->getTcpSocket()->peerPort()
            //          << "| CLIENT_DISCONNECTED"
            //          << "| Id:" << client->getId()
            //          << "| Name:" << client->getName();

            qDebug() << "<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
                     << "|" << message.toHex(' ');
        }
    }
}

QString WhiteboardServer::getHostIpAddress()
{
    QList<QHostAddress> list_ip = QNetworkInterface::allAddresses();
    for (const auto &addr : std::as_const(list_ip))
    {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost)
        {
            return addr.toString();
        }
    }
    return QString();
}

Client *WhiteboardServer::getClientByTcpSocket(QTcpSocket *socket)
{
    for (const auto &client : std::as_const(m_clients))
    {
        if (client->getTcpSocket() == socket)
        {
            return client;
        }
    }
    return nullptr;
}

QColor WhiteboardServer::generateUniqueColor()
{
    int hue = QRandomGenerator::global()->bounded(0, 360);  // Teinte (0-359°)
    int saturation = 200 + QRandomGenerator::global()->bounded(55); // Saturation haute (200-255)
    int value = 200 + QRandomGenerator::global()->bounded(55); // Luminosité haute (200-255)

    return QColor::fromHsv(hue, saturation, value);
}

void WhiteboardServer::addColorToUsedList(const QColor &color)
{
    if (!isColorUsed(color)) {
        m_used_colors.append(color);
    } else {
        qDebug() << "Color already used!";
    }
}

bool WhiteboardServer::isColorUsed(const QColor &color) const
{
    return m_used_colors.contains(color);
}

void WhiteboardServer::removeUsedColor(const QColor &color)
{
    if (isColorUsed(color)) {
        m_used_colors.removeAll(color);
    } else {
        qDebug() << "Color not found in used colors!";
    }
}

void WhiteboardServer::onTcpNewConnection()
{
    QTcpSocket *client_socket = m_tcp_server->nextPendingConnection();
    if (!client_socket)
    {
        return;
    }

    int client_id = nextClientId++;
    QColor client_color = Qt::black;
    do
    {
        client_color = generateUniqueColor();
    } while (isColorUsed(client_color));
    addColorToUsedList(client_color);

    Client *client = new Client(client_id, client_socket, client_color);
    m_clients[client_id] = client;

    qInfo() << "Client connected from" << client_socket->peerAddress().toString() << "port" << client_socket->peerPort()
            << "| Id:" << client_id;

    connect(client_socket, &QTcpSocket::readyRead, this, &WhiteboardServer::onTcpReadyRead);
    connect(client_socket, &QTcpSocket::disconnected, this, &WhiteboardServer::onTcpClientDisconnected);
}

void WhiteboardServer::onTcpClientDisconnected()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());
    if (!client_socket)
    {
        return;
    }

    Client *client = getClientByTcpSocket(client_socket);
    if (client)
    {
        qInfo() << "Client disconnected | Id:" << client->getId();

        broadcastClientDisconnected(client);

        removeUsedColor(m_clients[client->getId()]->getColor());
        delete m_clients.take(client->getId());
    }
}

void WhiteboardServer::onTcpReadyRead()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());
    if (!client_socket)
    {
        return;
    }

    Client *client = getClientByTcpSocket(client_socket);
    if (!client)
    {
        return;
    }

    while (client_socket->canReadLine())
    {
        QByteArray data = client_socket->readAll();
        if (!data.isEmpty() && data.length() >= (qsizetype)TCP_FRAME_MIN_LEN)
        {
            processTcpFrame(client, data);
        }
    }
}

void WhiteboardServer::onUdpReadyRead()
{
    while (m_udp_socket->hasPendingDatagrams())
    {
        QByteArray buffer;
        buffer.resize(m_udp_socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        if (m_udp_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort) > 0)
        {
            if (buffer.size() >= (qsizetype)UDP_FRAME_MIN_LEN)
            {
                processUdpFrame(sender, senderPort, buffer);
            }
        }
    }
}
