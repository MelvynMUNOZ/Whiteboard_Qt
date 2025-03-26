#ifndef WHITEBOARDSERVER_H
#define WHITEBOARDSERVER_H

#include "client.h"

#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#define TCP_PORT (12345)
#define UDP_PORT (TCP_PORT + 1)

class WhiteboardServer : public QObject
{
    Q_OBJECT

public:
    enum MessageType {
        ERROR = -1,
        NONE,
        REGISTER_CLIENT,
        ACK_REGISTER_CLIENT,
        REQUEST_ALL_CLIENTS_INFOS,
        SEND_ALL_CLIENTS_INFOS,
        DATA_CANVAS_CLIENT,
        DATA_CANVAS_SYNC,
    };

public:
    explicit WhiteboardServer(QObject *parent = nullptr);
    ~WhiteboardServer();

    void start();

private:
    static int nextClientId;

    QTcpServer *m_tcp_server;
    QUdpSocket *m_udp_socket;
    QHash<int, Client*> m_clients;

private:
    QString getHostIpAddress();
    int getClientIdBySocket(QTcpSocket *socket);
    void processTcpFrame(Client *client, const QByteArray &data);
    void processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data);

signals:

public slots:
    void onTcpNewConnection();
    void onTcpClientDisconnected();
    void onTcpReadyRead();
    void onUdpReadyRead();
};

#endif // WHITEBOARDSERVER_H
