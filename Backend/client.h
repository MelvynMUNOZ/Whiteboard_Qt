#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>

class Client
{
public:
    Client(QTcpSocket *socket);
    ~Client();

    QTcpSocket *getSocket() const;
    QString getName() const;
    QColor getColor() const;

    void setName(const QString &name);
    void setColor(const QColor &color);

private:
    QTcpSocket *m_socket;
    QString m_name;
    QColor m_color;
    // TODO: Add Canvas info form user ???
};

#endif // CLIENT_H
