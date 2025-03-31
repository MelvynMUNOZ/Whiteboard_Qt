#ifndef WHITEBOARDSERVER_H
#define WHITEBOARDSERVER_H

#include "client.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QColor>

#define TCP_PORT (12345)
#define UDP_PORT (TCP_PORT + 1)

// TCP frame (Big Endian) :
// 0    | 1  -  4 | 5  -  n
// type | id      | payload

#define TCP_FRAME_MIN_LEN (sizeof(quint8) + sizeof(quint32))

// UDP frame (Big Endian) :
// 0    | 1  -  4 | 5  -  n
// type | id      | payload

#define UDP_FRAME_MIN_LEN (sizeof(quint8) + sizeof(quint32))

class WhiteboardServer : public QObject
{
    Q_OBJECT

public:
    enum MessageType {
        ERROR = -1,
        NONE,
        REGISTER_CLIENT,
        ACK_REGISTER_CLIENT,
        REGISTER_UDP_PORT,
        ACK_REGISTER_UDP_PORT,
        REQUEST_ALL_CLIENTS_INFOS,
        SEND_CLIENTS_INFOS,
        DATA_CANVAS_CLIENT,
        DATA_CANVAS_SYNC,
        CLIENT_DISCONNECTED,
    };

public:
    explicit WhiteboardServer(QObject *parent = nullptr);
    ~WhiteboardServer();

    void start();

protected:
    void processTcpFrame(Client *client, const QByteArray &data);
    void processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data);

    void sendAckRegisterClient(Client *client);
    void sendAckRegisterUdpPort(Client *client);

    void broadcastClientsInfos();
    void broadcastDataCanvasSync();
    void broadcastClientDisconnected(Client *client);


private:
    static int nextClientId;

    QTcpServer *m_tcp_server;
    QUdpSocket *m_udp_socket;
    QHash<int, Client*> m_clients;
    QList<QColor> m_used_colors;

private:
    QString getHostIpAddress();
    Client *getClientByTcpSocket(QTcpSocket *socket);

    QColor generateUniqueColor();
    void addColorToUsedList(const QColor &color);
    bool isColorUsed(const QColor &color) const;
    void removeUsedColor(const QColor &color);

public slots:
    void onTcpNewConnection();
    void onTcpClientDisconnected();
    void onTcpReadyRead();
    void onUdpReadyRead();
};

#endif // WHITEBOARDSERVER_H
