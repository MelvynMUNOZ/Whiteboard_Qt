#ifndef IHMFRONTEND_H
#define IHMFRONTEND_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include "client.h"
#include "connection.h"
#include "whiteboard.h"

#define TCP_PORT (12345)
#define UDP_PORT (TCP_PORT + 1)

#define TCP_FRAME_MIN_LEN (sizeof(quint8) + sizeof(quint32))
#define UDP_FRAME_MIN_LEN (sizeof(quint8) + sizeof(quint32))

///<All type of messages sended and received
enum MessageType {
    NONE,
    ACK_CONNECT,
    REGISTER_CLIENT,
    ACK_REGISTER_CLIENT,
    REGISTER_UDP_PORT,
    ACK_REGISTER_UDP_PORT,
    REQUEST_ALL_CLIENT_INFOS,
    CLIENT_INFOS,
    DATA_CANVAS_CLIENT,
    DATA_CANVAS_SYNC,
    CLIENT_DISCONNECTED,
};

///<All general data for the IHM
struct data_client{
    connection *connectionWidget;
    whiteboard *whiteboardWidget;
    QTcpSocket *tcp_socket;
    QUdpSocket *udp_socket;
    Client *my_client;
    QPen *my_client_pen;
    QHash<int, Client*> *client_infos;
};

extern data_client globalDataClient;

#endif // IHMFRONTEND_H
