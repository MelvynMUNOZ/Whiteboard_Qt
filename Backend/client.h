#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>

class Client
{
public:
    Client(int id, QTcpSocket *socket, QColor color);
    ~Client();

    int getId() const;
    QTcpSocket *getTcpSocket() const;
    quint16 getUdpPort() const;
    QString getName() const;
    QColor getColor() const;

    void setUdpPort(const quint16 port);
    void setName(const QString &name);
    void setColor(const QColor &color);

private:
    int m_id;
    QTcpSocket *m_tcp_socket;
    quint16 m_udp_port;
    QString m_name;
    QColor m_color;
};

#endif // CLIENT_H
