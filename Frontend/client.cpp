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

QString Client::getName() const
{
    return m_name;
}

QColor Client::getColor() const
{
    return m_color;
}

void Client::setName(const QString &name)
{
    m_name = name;
}

void Client::setColor(const QColor &color)
{
    m_color = color;
}

void Client::setId(const int id)
{
    m_id = id;
}
