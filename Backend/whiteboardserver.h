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
        NONE,
        SEND_CLIENT_NAME,
        REQ_CLIENTS_INFOS,
        UPDATE_CLIENTS_INFOS,
        DATA_CANVAS_CLIENT,
        DATA_CANVAS_SYNC,
    };

public:
    explicit WhiteboardServer(QObject *parent = nullptr);
    ~WhiteboardServer();

    void start();

private:
    QTcpServer *m_tcp_server;
    QUdpSocket *m_udp_socket;

    QMap<int, Client*> m_clients;

private:
    QString getHostIpAddress();
    void processTcpReadData(QTcpSocket *socket, const QByteArray &data);
    void processUdpReadData(QTcpSocket *socket, const QByteArray &data);

signals:

public slots:
    void onTcpNewConnection();
    void onTcpClientDisconnected();
    void onTcpReadyRead();
    void onUdpReadyRead();
};

#endif // WHITEBOARDSERVER_H
