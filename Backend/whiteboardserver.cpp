#include "whiteboardserver.h"

#include <QNetworkInterface>
#include <QtEndian>
#include <QRandomGenerator>

int WhiteboardServer::nextClientId = 1;

WhiteboardServer::WhiteboardServer(QObject *parent)
    : QObject(parent), m_tcp_server(new QTcpServer(this)), m_udp_socket(new QUdpSocket(this))
{}

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

    connect(m_tcp_server, &QTcpServer::newConnection, this, &WhiteboardServer::onTcpNewConnection);

    if (m_tcp_server->listen(QHostAddress::Any, TCP_PORT))
    {
        qInfo() << "Started TCP server on port" << TCP_PORT;
    }
    else
    {
        qCritical() << "Failed to start TCP server on port" << TCP_PORT;
    }

    /***** UDP *****/

    connect(m_udp_socket, &QUdpSocket::readyRead, this, &WhiteboardServer::onUdpReadyRead);

    if (m_udp_socket->bind(QHostAddress::Any, UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        qInfo() << "Started UDP socket on port" << UDP_PORT;
    }
    else
    {
        qCritical() << "Failed to start UDP socket on port" << UDP_PORT;
    }
}

void WhiteboardServer::processTcpFrame(Client *client, const QByteArray &data)
{
    // TCP frame (Big Endian) :
    // 0    | 1  -  4 | 5  -  n
    // type | id      | payload

    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    int id = qFromBigEndian(*reinterpret_cast<const int*>(data.mid(1, 4).data()));
    auto payload = QString::fromUtf8(data.sliced(5));

    qDebug() << ">> TCP from" << client->getTcpSocket()->peerAddress().toString() << "port" << client->getTcpSocket()->peerPort()
             << "| Type:" << type
             << "| Id:" << id
             << "| Payload:" << payload;

    switch (type)
    {
    case REGISTER_CLIENT:
        client->setName(payload);
        sendAckRegisterClient(client);
        break;

    case REGISTER_UDP_PORT:
        // TODO: enregistre le port UDP utilisé par le client (pour les prochains envois) dans la classe client associé à l'id reçu: Besoin de verifier l'adresse IP qui doit matcher avec celle de la socket Tcp
        // Envoi d'une reponse TCP ACK_REGISTER_UDP_PORT avec comme données : id, port udp enregistré (quint16 en Big Endian)
        break;

    case REQUEST_ALL_CLIENTS_INFOS:
        // TODO: Envoi d'une réponse TCP SEND_CLIENTS_INFOS par client enregistré, avec comme données : id client, color (format #XXXXXX), nom
        break;

    default:
        // qWarning() << "Unknown TCP message type received: " << type;
        break;
    }
}

void WhiteboardServer::processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data)
{
    // UDP frame (Big Endian) :
    // 0    | 1  -  4 | 5  -  n
    // type | id      | payload

    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    int id = qFromBigEndian(*reinterpret_cast<const int*>(data.mid(1, 4).data()));
    auto payload = QString::fromUtf8(data.sliced(5));

    qDebug() << ">> UDP from" << sender.toString() << "port" << sender_port
             << "| Type:" << type
             << "| Id:" << id
             << "| Payload:" << payload;

    switch (type)
    {
    case DATA_CANVAS_CLIENT:
        break;

    default:
        // qWarning() << "Unknown UDP message type received: " << type;
        break;
    }
}

void WhiteboardServer::sendAckRegisterClient(Client *client)
{
    QString colorHex = client->getColor().name(QColor::HexRgb);

    QByteArray message;
    message.append(static_cast<char>(ACK_REGISTER_CLIENT));  // Type du message
    message.append(qToBigEndian(client->getId()));  // ID du client en Big Endian (4 bytes)
    message.append(colorHex.toUtf8());  // Payload contenant la couleur en hex

    client->getTcpSocket()->write(message);

    qDebug() << "<<< TCP to" << client->getTcpSocket()->peerAddress() << "port" << client->getTcpSocket()->peerPort()
             << "| ACK_REGISTER_CLIENT"
             << "| Id:" << client->getId()
             << "| Color:" << colorHex;
}

void WhiteboardServer::sendAckRegisterUdpPort(Client *client)
{
    QByteArray message;
    message.append(static_cast<char>(WhiteboardServer::ACK_REGISTER_UDP_PORT));  // Type du message
    message.append(qToBigEndian(client->getId()));  // ID du client en Big Endian (4 bytes)
    message.append(qToBigEndian(client->getUdpPort()));  // Port UDP du client en Big Endian (2 bytes)

    client->getTcpSocket()->write(message);

    qDebug() << "<<< UDP to" << client->getTcpSocket()->peerAddress() << "port" << client->getTcpSocket()->peerPort()
             << "| ACK_REGISTER_CLIENT"
             << "| Id:" << client->getId()
             << "| UDP Port:" << client->getUdpPort();
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

int WhiteboardServer::getClientIdByTcpSocket(QTcpSocket *socket)
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        if (it.value()->getTcpSocket() == socket)
        {
            return it.key();
        }
    }
    return -1;
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

    /***** Create new client *****/

    int client_id = nextClientId++;
    QColor client_color = Qt::black; // default color
    do {
        client_color = generateUniqueColor();
    } while (isColorUsed(client_color));
    addColorToUsedList(client_color);

    Client *client = new Client(client_id, client_socket, client_color);
    m_clients[client_id] = client;

    qInfo() << "Client connected from" << client_socket->peerAddress().toString() << "port" << client_socket->peerPort()
            << "| Id:" << client_id
            << "| Color:" << client_color.name();

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

    int client_id = getClientIdByTcpSocket(client_socket);
    if (client_id != -1)
    {
        removeUsedColor(m_clients[client_id]->getColor());
        delete m_clients.take(client_id);

        qInfo() << "Client disconnected | Id:" << client_id;
    }
}

void WhiteboardServer::onTcpReadyRead()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());
    if (!client_socket)
    {
        return;
    }

    int client_id = getClientIdByTcpSocket(client_socket);
    if (client_id == -1)
    {
        return;
    }

    while (client_socket->canReadLine())
    {
        QByteArray data = client_socket->readAll();
        if (!data.isEmpty())
        {
            Client *client = m_clients[client_id];
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
            processUdpFrame(sender, senderPort, buffer);
        }
    }
}
