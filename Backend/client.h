#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>

class Client
{
public:
    Client(int id, QTcpSocket *socket);
    ~Client();

    int getId() const;
    QTcpSocket *getSocket() const;
    QString getName() const;
    QColor getColor() const;

    void setName(const QString &name);
    void setColor(const QColor &color);

private:
    int m_id;
    QTcpSocket *m_socket;
    QString m_name;
    QColor m_color;
    // TODO: Add Canvas info form user ???
};

#endif // CLIENT_H
