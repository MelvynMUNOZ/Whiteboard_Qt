/**
 * @file whiteboardserver.cpp
 * @brief Implementation of the WhiteboardServer class.
 * @author Raphael CAUSSE
 */

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

void WhiteboardServer::start(quint16 tcp_port, quint16 udp_port)
{
    qInfo() << "Host Server IP address:" << getHostIpAddress();

    /***** TCP *****/

    if (m_tcp_server->listen(QHostAddress::Any, tcp_port))
    {
        qInfo() << "Started TCP server on port" << tcp_port;
    }
    else
    {
        qCritical() << "Failed to start TCP server on port" << tcp_port;
    }

    connect(m_tcp_server, &QTcpServer::newConnection, this, &WhiteboardServer::onTcpNewConnection);

    /***** UDP *****/

    if (m_udp_socket->bind(QHostAddress::Any, udp_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        qInfo() << "Started UDP socket on port" << udp_port;
    }
    else
    {
        qCritical() << "Failed to start UDP socket on port" << udp_port;
    }

    connect(m_udp_socket, &QUdpSocket::readyRead, this, &WhiteboardServer::onUdpReadyRead);
}

void WhiteboardServer::processTcpFrame(Client *client, const QByteArray &data)
{
    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);

    quint32 idRaw;
    memcpy(&idRaw, data.mid(1, 4).data(), 4);
    int id = qFromBigEndian(idRaw);

    QByteArray payload = data.sliced(5).chopped(1);

    qInfo() << ">>> TCP from" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "| Type:" << type
             << "| Id:" << id
             << "| Payload:" << payload;

    // qInfo() << ">>> TCP from " << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
    //         << "|" << data.toHex(' ');

    // S'assurer qu'on communique avec le client avec le meme id
    if (id == client->getId())
    {
        switch (type)
        {
        case REGISTER_CLIENT:
            client->setName(QString::fromUtf8(payload));
            // Envoi ACK
            sendAckRegisterClient(client);
            // Envoi a tous les autres clients qu'un nouveau client s'est connecte et enregistre
            broadcastClientConnected(client);
            break;

        case REGISTER_UDP_PORT:
            client->setUdpPort(qFromBigEndian(*reinterpret_cast<const quint16*>(payload.constData())));
            // Envoi ACK
            sendAckRegisterUdpPort(client);
            break;

        case REQUEST_ALL_CLIENT_INFOS:
            sendAllClientsInfos(client);
            break;

        default:
            break;
        }
    }
}

void WhiteboardServer::processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data)
{
    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    QByteArray payload = data.sliced(1).chopped(1);

    qInfo() << ">>> UDP from" << sender.toString() << "port" << sender_port
             << "| Type:" << type
             << "| Payload:" << payload;

    // qInfo() << ">>> UDP from " << sender.toString() << "port" << sender_port
    //         << "|" << data.toHex(' ');

    switch (type)
    {
    case DATA_CANVAS_CLIENT:
        broadcastDataCanvasSync(payload);
        break;

    default:
        break;
    }
}

void WhiteboardServer::sendAckConnect(Client *client)
{
    if (!client)
    {
        return;
    }

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::ACK_CONNECT);  // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client (4 bytes)
    message.append(client->getColor().name(QColor::HexRgb).toUtf8());  // La couleur du client en hex
    message.append('\n');

    client->getTcpSocket()->write(message);
    client->getTcpSocket()->flush();

    qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "| ACK_CONNECT"
             << "| Id:" << client->getId()
             << "| Color:" << client->getColor().name(QColor::HexRgb);

    // qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
    //         << "|" << message.toHex(' ');
}

void WhiteboardServer::sendAckRegisterClient(Client *client)
{
    if (!client)
    {
        return;
    }

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::ACK_REGISTER_CLIENT);  // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client (4 bytes)
    message.append(client->getName().toUtf8());  // La couleur du client en hex
    message.append('\n');

    client->getTcpSocket()->write(message);
    client->getTcpSocket()->flush();

    qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "| ACK_REGISTER_CLIENT"
             << "| Id:" << client->getId()
             << "| Name:" << client->getName();

    // qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
    //         << "|" << message.toHex(' ');
}

void WhiteboardServer::sendAckRegisterUdpPort(Client *client)
{
    if (!client)
    {
        return;
    }

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::ACK_REGISTER_UDP_PORT);  // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client (4 bytes)
    stream << static_cast<quint16>(client->getUdpPort());  // Port UDP du client (2 bytes)
    message.append('\n');

    client->getTcpSocket()->write(message);
    client->getTcpSocket()->flush();

    qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "| ACK_REGISTER_UDP_PORT"
             << "| Id:" << client->getId()
             << "| UDP Port:" << client->getUdpPort();

    // qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getUdpPort()
    //         << "|" << message.toHex(' ');
}

void WhiteboardServer::sendAllClientsInfos(Client *client)
{
    if (!client)
    {
        return;
    }

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Envoi toutes les infos de tous les clients enregistres
    for (const auto &other_client : std::as_const(m_clients))
    {
        if (other_client != client)
        {
            stream << static_cast<quint8>(WhiteboardServer::CLIENT_INFOS); // Type du message
            stream << static_cast<quint32>(client->getId()); // ID du client (4 bytes)
            stream << static_cast<quint32>(other_client->getId()); // ID du client a envoyer (4 bytes)
            message.append(other_client->getColor().name(QColor::HexRgb).toUtf8());  // La couleur du client a envoyer
            message.append(other_client->getName().toUtf8());  // Nom du client a envoyer
            message.append('\n');

            client->getTcpSocket()->write(message);
            client->getTcpSocket()->flush();

            qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
                     << "| CLIENT_INFOS"
                     << "| Id:" << client->getId()
                     << "| OId:" << other_client->getId()
                     << "| Color:" << other_client->getColor().name(QColor::HexRgb)
                     << "| Name:" << other_client->getName();

            // qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
            //         << "|" << message.toHex(' ');
        }

        message.clear();
    }
}

void WhiteboardServer::broadcastDataCanvasSync(const QByteArray &data)
{
    /***** Decodage du message *****/

    quint32 x_beg_raw, y_beg_raw, x_end_raw, y_end_raw, width_raw;

    // Lire les 4 octets pour chaque variable et les convertir en int (big endian)
    memcpy(&x_beg_raw, data.mid(0, 4).data(), 4);
    int x_beg = qFromBigEndian(x_beg_raw);

    memcpy(&y_beg_raw, data.mid(4, 4).data(), 4);
    int y_beg = qFromBigEndian(y_beg_raw);

    memcpy(&x_end_raw, data.mid(8, 4).data(), 4);
    int x_end = qFromBigEndian(x_end_raw);

    memcpy(&y_end_raw, data.mid(12, 4).data(), 4);
    int y_end = qFromBigEndian(y_end_raw);

    memcpy(&width_raw, data.mid(16, 4).data(), 4);
    int width = qFromBigEndian(width_raw);

    // Lire le reste pour obtenir la couleur (en hex)
    QString color_hex = QString::fromUtf8(data.mid(20, 7)); // 7 caracteres
    QColor color(color_hex);

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::DATA_CANVAS_SYNC);  // Type du message
    message.append(data);
    message.append('\n');

    // Envoi des données de canvas du client recus a tous les autres clients
    for (const auto &client : std::as_const(m_clients))
    {
        m_udp_socket->writeDatagram(message, client->getTcpSocket()->peerAddress(), client->getUdpPort());

        qInfo() << "<<< UDP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
                << "| DATA_CANVAS_SYNC"
                << "| Xbeg:" << x_beg
                << "| Ybeg:" << y_beg
                << "| Xend:" << x_end
                << "| Yend:" << y_end
                << "| Width:" << width
                << "| Color:" << color.name(QColor::HexRgb);

        // qInfo() << "<<< UDP to" << other_client->getTcpSocket()->peerAddress().toString() << "port" << other_client->getTcpSocket()->peerPort()
        //         << "|" << message.toHex(' ');
    }
}

void WhiteboardServer::broadcastClientConnected(Client *client)
{
    if (!client)
    {
        return;
    }

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Envoi a tous les autres client qu'un client s'est connecte, sauf celui concerne
    for (const auto &other_client : std::as_const(m_clients))
    {
        stream << static_cast<quint8>(WhiteboardServer::CLIENT_INFOS); // Type du message
        stream << static_cast<quint32>(other_client->getId()); // ID du client (4 bytes)
        stream << static_cast<quint32>(client->getId()); // ID du client qui s'est connecte (4 bytes)
        message.append(client->getColor().name(QColor::HexRgb).toUtf8());  // La couleur du client qui s'est connecte
        message.append(client->getName().toUtf8());  // Nom du client qui s'est connecte
        message.append('\n');

        if (other_client != client && other_client->getTcpSocket())
        {
            other_client->getTcpSocket()->write(message);
            other_client->getTcpSocket()->flush();

            qInfo() << "<<< TCP to" << other_client->getTcpSocket()->peerAddress().toString() << "port" << other_client->getTcpSocket()->peerPort()
                    << "| CLIENT_INFOS"
                    << "| Id:" << other_client->getId()
                    << "| OId:" << client->getId()
                    << "| Color:" << client->getColor().name(QColor::HexRgb)
                    << "| Name:" << client->getName();

            // qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
            //         << "|" << message.toHex(' ');
        }

        message.clear();
    }
}

void WhiteboardServer::broadcastClientDisconnected(Client *client)
{
    if (!client)
    {
        return;
    }

    /***** Encodage et envoi du message *****/

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << static_cast<quint8>(WhiteboardServer::CLIENT_DISCONNECTED); // Type du message
    stream << static_cast<quint32>(client->getId());  // ID du client qui se déconnecte (4 bytes)
    message.append(client->getName().toUtf8());  // Nom du client
    message.append('\n');

    // Envoi a tous les clients sauf celui qui se deconnecte
    for (const auto &other_client : std::as_const(m_clients))
    {
        if (other_client != client && other_client->getTcpSocket())
        {
            other_client->getTcpSocket()->write(message);
            other_client->getTcpSocket()->flush();

            qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
                     << "| CLIENT_DISCONNECTED"
                     << "| Id:" << client->getId()
                     << "| Name:" << client->getName();

            // qInfo() << "<<< TCP to" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
            //         << "|" << message.toHex(' ');
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
    int value = 200 + QRandomGenerator::global()->bounded(55); // Luminosite haute (200-255)

    return QColor::fromHsv(hue, saturation, value);
}

void WhiteboardServer::addColorToUsedList(const QColor &color)
{
    if (!isColorUsed(color)) {
        m_used_colors.append(color);
    } else {
        qInfo() << "Color" << color.name(QColor::HexRgb) << "already used!";
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
        qInfo() << "Color" << color.name(QColor::HexRgb) <<  "not found in used colors!";
    }
}

void WhiteboardServer::onTcpNewConnection()
{
    QTcpSocket *client_socket = m_tcp_server->nextPendingConnection();
    if (!client_socket)
    {
        return;
    }

    connect(client_socket, &QTcpSocket::readyRead, this, &WhiteboardServer::onTcpReadyRead);
    connect(client_socket, &QTcpSocket::disconnected, this, &WhiteboardServer::onTcpClientDisconnected);

    /***** Creation nouveau client *****/

    int client_id = nextClientId++;
    // Attibution d'une couleur unique a chaque client
    QColor client_color = Qt::black;
    do
    {
        client_color = generateUniqueColor();
    } while (isColorUsed(client_color));
    addColorToUsedList(client_color);

    Client *client = new Client(client_id, client_socket, client_color);
    m_clients[client_id] = client;

    qInfo() << "Client connected from" << client_socket->peerAddress().toString() << "port" << client_socket->peerPort()
            << "| Id:" << client_id
            << "| Color:" << client_color.name(QColor::HexRgb);

    // Envoi ACK
    sendAckConnect(client);
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
        // Nettoyer les ressources et prevenir tous les autres clients de la deconnexion
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

    // Lecture des trames TCP
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
    // Lecture des trames UDP
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
