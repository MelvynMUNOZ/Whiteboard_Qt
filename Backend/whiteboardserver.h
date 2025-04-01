/**
 * @file whiteboardserver.h
 * @brief Definition of the WhiteboardServer class.
 * @author Raphael CAUSSE
 */

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

#define TCP_PORT (12345) ///< Default TCP port for the server.
#define UDP_PORT (TCP_PORT + 1) ///< Default UDP port for the server.

#define TCP_FRAME_MIN_LEN (sizeof(quint8) + sizeof(quint32)) ///< Minimum length of a TCP frame.
#define UDP_FRAME_MIN_LEN (sizeof(quint8) + sizeof(quint32)) ///< Minimum length of a UDP frame.

/**
 * @class WhiteboardServer
 * @brief Manages client connections and data synchronization for a collaborative whiteboard.
 */
class WhiteboardServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Enum representing message types exchanged between the server and clients.
     */
    enum MessageType {
        NONE,                     ///< No message.
        ACK_CONNECT,              ///< Acknowledgment of client connection.
        REGISTER_CLIENT,          ///< Client registration request.
        ACK_REGISTER_CLIENT,      ///< Acknowledgment of client registration.
        REGISTER_UDP_PORT,        ///< Request to register UDP port.
        ACK_REGISTER_UDP_PORT,    ///< Acknowledgment of UDP port registration.
        REQUEST_ALL_CLIENT_INFOS, ///< Request for all connected clients' information.
        CLIENT_INFOS,             ///< Information about a client.
        DATA_CANVAS_CLIENT,       ///< Canvas data sent by a client.
        DATA_CANVAS_SYNC,         ///< Synchronize canvas data with all clients.
        CLIENT_DISCONNECTED       ///< Notification of client disconnection.
    };

public:
    /**
     * @brief Constructor.
     * @param parent Parent QObject.
     */
    explicit WhiteboardServer(QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~WhiteboardServer();

    /**
     * @brief Starts the server.
     * @param tcp_port The port on which the TCP server will listen for incoming connections.
     *        Defaults to `TCP_PORT` if not specified.
     * @param udp_port The port on which the UDP socket will listen for incoming datagrams.
     *        Defaults to `UDP_PORT` if not specified.
     */
    void start(quint16 tcp_port = TCP_PORT, quint16 udp_port = UDP_PORT);

protected:
    /**
     * @brief Processes an incoming TCP frame from a client.
     * @param client Pointer to the client that sent the data.
     * @param data The received TCP data frame.
     */
    void processTcpFrame(Client *client, const QByteArray &data);

    /**
     * @brief Processes an incoming UDP frame.
     * @param sender The IP address of the sender.
     * @param sender_port The port from which the data was sent.
     * @param data The received UDP data frame.
     */
    void processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data);

    /**
     * @brief Sends an acknowledgment for a successful client connection.
     * @param client Pointer to the connected client.
     */
    void sendAckConnect(Client *client);

    /**
     * @brief Sends an acknowledgment confirming the client's registration.
     * @param client Pointer to the registered client.
     */
    void sendAckRegisterClient(Client *client);

    /**
     * @brief Sends an acknowledgment confirming the UDP port registration.
     * @param client Pointer to the client that registered the UDP port.
     */
    void sendAckRegisterUdpPort(Client *client);

    /**
     * @brief Sends information about all connected clients to a specific client.
     * @param client Pointer to the requesting client.
     */
    void sendAllClientsInfos(Client *client);

    /**
     * @brief Broadcasts canvas synchronization data to all clients.
     * @param client Pointer to the client sending the data.
     * @param data The canvas data to be broadcasted.
     */
    void broadcastDataCanvasSync(Client *client, const QByteArray &data);

    /**
     * @brief Broadcasts a client disconnection event to all remaining clients.
     * @param client Pointer to the client that disconnected.
     */
    void broadcastClientDisconnected(Client *client);

private:
    static int nextClientId; ///< Static variable to generate unique client IDs.

    QTcpServer *m_tcp_server; ///< TCP server instance.
    QUdpSocket *m_udp_socket; ///< UDP socket instance.
    QHash<int, Client*> m_clients; ///< List of connected clients indexed by ID.
    QList<QColor> m_used_colors; ///< List of colors already assigned to clients.

    /**
     * @brief Retrieves the host's IP address.
     * @return A QString representing the host's IP address.
     */
    QString getHostIpAddress();

    /**
     * @brief Retrieves a client based on their associated TCP socket.
     * @param socket Pointer to the client's TCP socket.
     * @return A pointer to the corresponding Client object, or nullptr if not found.
     */
    Client *getClientByTcpSocket(QTcpSocket *socket);

    /**
     * @brief Generates a unique color for a new client.
     * @return A unique QColor for the client.
     */
    QColor generateUniqueColor();

    /**
     * @brief Marks a color as used by adding it to the list of assigned colors.
     * @param color The color to be added to the used list.
     */
    void addColorToUsedList(const QColor &color);

    /**
     * @brief Checks whether a given color is already assigned to a client.
     * @param color The color to check.
     * @return True if the color is already in use, false otherwise.
     */
    bool isColorUsed(const QColor &color) const;

    /**
     * @brief Removes a color from the list of used colors.
     * @param color The color to be removed from the used list.
     */
    void removeUsedColor(const QColor &color);

public slots:
    /**
     * @brief Handles a new incoming TCP connection.
     * This slot is triggered when a new client connects to the TCP server.
     */
    void onTcpNewConnection();

    /**
     * @brief Handles a TCP client disconnection.
     * This slot is triggered when a client disconnects from the TCP server.
     */
    void onTcpClientDisconnected();

    /**
     * @brief Processes incoming TCP data.
     * This slot reads and processes data received via TCP from a connected client.
     */
    void onTcpReadyRead();

    /**
     * @brief Processes incoming UDP data.
     * This slot reads and processes data received via UDP from a remote sender.
     */
    void onUdpReadyRead();
};

#endif // WHITEBOARDSERVER_H
