#include "client.h"

Client::Client(QTcpSocket *socket)
    : m_socket(socket)
{
    // TODO set unique color
}

Client::~Client()
{
    m_socket->close();
    m_socket->deleteLater();
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
