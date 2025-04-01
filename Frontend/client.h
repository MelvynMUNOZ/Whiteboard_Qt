#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>
#include <QHash>

class Client
{
public:
    Client(int id, QTcpSocket *socket, QColor color);
    ~Client();

    int getId() const;
    void setId(const int id);
    QTcpSocket *getTcpSocket() const;
    QString getName() const;
    QColor getColor() const;

    void setName(const QString &name);
    void setColor(const QColor &color);

private:
    int m_id;
    QTcpSocket *m_tcp_socket;
    QString m_name;
    QColor m_color;
};

extern QHash<int, Client*> MapOfClients;

#endif // CLIENT_H

