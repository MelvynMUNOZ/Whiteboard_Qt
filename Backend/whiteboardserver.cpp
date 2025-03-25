#include "whiteboardserver.h"

#include <QNetworkInterface>

int WhiteboardServer::nextClientId = 1;

WhiteboardServer::WhiteboardServer(QObject *parent)
    : QObject(parent),
    m_tcp_server(new QTcpServer(this)),
    m_udp_socket(new QUdpSocket(this))
{}

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

WhiteboardServer::~WhiteboardServer()
{
    qDeleteAll(m_clients);
    m_clients.clear();

    m_tcp_server->close();
    m_tcp_server->deleteLater();

    m_udp_socket->close();
    m_udp_socket->deleteLater();
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

int WhiteboardServer::getClientIdBySocket(QTcpSocket *socket)
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        if (it.value()->getSocket() == socket)
        {
            return it.key();
        }
    }
    return -1;
}

void WhiteboardServer::processTcpFrame(Client *client, const QByteArray &data)
{
    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    QString payload = QString::fromUtf8(data.sliced(1));

    qDebug() << ">> TCP from" << client->getSocket()->peerAddress().toString() << "port" << client->getSocket()->peerPort() << "| Type:" << type << "| Payload Size:" << payload.size() << "| Payload:" << payload;

    switch (type)
    {
    case SEND_CLIENT_NAME:
        client->setName(payload);
        break;

    case REQ_CLIENTS_INFOS:
        break;

    case DATA_CANVAS_CLIENT:
        break;

    default:
        //qWarning() << "Unknown TCP message type received: " << type;
        break;
    }
}

void WhiteboardServer::processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data)
{
    auto type = static_cast<WhiteboardServer::MessageType>(data[0]);
    QString payload = QString::fromUtf8(data.sliced(1));

    qDebug() << ">> UDP from" << sender.toString() << "port" << sender_port << "| Type:" << type << "| Payload Size:" << payload.size() << "| Payload:" << payload;

    switch (type)
    {
    case DATA_CANVAS_CLIENT:
        break;

    default:
        //qWarning() << "Unknown UDP message type received: " << type;
        break;
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
    Client *client = new Client(client_id, client_socket);
    m_clients[client_id] = client;

    connect(client_socket, &QTcpSocket::readyRead, this, &WhiteboardServer::onTcpReadyRead);
    connect(client_socket, &QTcpSocket::disconnected, this, &WhiteboardServer::onTcpClientDisconnected);


    qInfo() << "Client (id" << client_id << ") connected from" << client_socket->peerAddress().toString() << "port" << client_socket->peerPort();
}

void WhiteboardServer::onTcpClientDisconnected()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());
    if (!client_socket)
    {
        return;
    }

    int client_id = getClientIdBySocket(client_socket);
    if (client_id != -1)
    {
        qInfo() << "Client (id" << client_id << ") disconnected";
        delete m_clients.take(client_id);
    }
}

void WhiteboardServer::onTcpReadyRead()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());
    if (!client_socket)
    {
        return;
    }

    int client_id = getClientIdBySocket(client_socket);
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
