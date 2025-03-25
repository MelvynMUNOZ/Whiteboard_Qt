#include "whiteboardserver.h"

#include <QNetworkInterface>

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
    // TODO: delete clients

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

void WhiteboardServer::processTcpReadData(QTcpSocket *socket, const QByteArray &data)
{
    qDebug() << "(socket" << socket->socketDescriptor() << ") TCP Received:" << data[0];

    // int type = (int)data[0];
    // switch (type)
    // {
    //     case SEND_CLIENT_NAME:
    //     {
    //         QString client_name = QString::fromUtf8(data.sliced(1));
    //         qInfo() << "Client name: " << client_name;
    //         break;
    //     }

    // case REQ_CLIENTS_INFOS:
    //     break;

    // case DATA_CANVAS_CLIENT:
    //     break;

    // default:
    //     break;
    // }
}

void WhiteboardServer::processUdpReadData(QTcpSocket *socket, const QByteArray &data)
{
    qDebug() << "(socket" << socket->socketDescriptor() << ") UCP Received:" << data;
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

    /* Socket descriptor is unique so used as ID */
    int client_id = client_socket->socketDescriptor();
    m_clients[client_id] = new Client(client_socket);

    qInfo() << "Client connected, from" << client_socket->localAddress().toString() << "(socket" << client_id <<")";
}

void WhiteboardServer::onTcpClientDisconnected()
{
    qInfo() << "Client disconnected !";
}

void WhiteboardServer::onTcpReadyRead()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());
    if (!client_socket)
    {
        return;
    }

    while (client_socket->canReadLine())
    {
        QByteArray data = client_socket->readAll();
        if (!data.isEmpty())
        {
            processTcpReadData(client_socket, data);
        }
    }
}

void WhiteboardServer::onUdpReadyRead()
{
    // TODO
}
