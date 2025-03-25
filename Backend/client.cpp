#include "client.h"

Client::Client(int id, QTcpSocket *socket)
    : m_id(id), m_socket(socket)
{
    // TODO: Set Unique color
}

Client::~Client()
{
    if (m_socket)
    {
        m_socket->close();
        m_socket->deleteLater();
    }
}

int Client::getId() const
{
    return m_id;
}

QTcpSocket *Client::getSocket() const
{
    return m_socket;
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
