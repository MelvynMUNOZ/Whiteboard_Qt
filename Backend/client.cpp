#include "client.h"

Client::Client(int id, QTcpSocket *socket, QColor color)
    : m_id(id), m_tcp_socket(socket), m_color(color)
{
}

Client::~Client()
{
    if (m_tcp_socket)
    {
        m_tcp_socket->close();
        m_tcp_socket->deleteLater();
    }
}

int Client::getId() const
{
    return m_id;
}

QTcpSocket *Client::getTcpSocket() const
{
    return m_tcp_socket;
}

quint16 Client::getUdpPort() const
{
    return m_udp_port;
}

QString Client::getName() const
{
    return m_name;
}

QColor Client::getColor() const
{
    return m_color;
}

void Client::setUdpPort(const quint16 port)
{
    m_udp_port = port;
}

void Client::setName(const QString &name)
{
    m_name = name;
}

void Client::setColor(const QColor &color)
{
    m_color = color;
}
