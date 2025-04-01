/**
 * @file client.h
 * @brief Definition of the Client class.
 * @author Raphael CAUSSE
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>

/**
 * @class Client
 * @brief Represents a connected client in the whiteboard server.
 */
class Client
{
public:
    /**
     * @brief Constructor.
     * @param id Unique identifier for the client.
     * @param socket Pointer to the QTcpSocket associated with the client.
     * @param color Assigned color for the client.
     */
    Client(int id, QTcpSocket *socket, QColor color);

    /**
     * @brief Destructor.
     */
    ~Client();

    /**
     * @brief Gets the client ID.
     * @return The unique client ID.
     */
    int getId() const;

    /**
     * @brief Gets the client's TCP socket.
     * @return Pointer to the QTcpSocket.
     */
    QTcpSocket *getTcpSocket() const;

    /**
     * @brief Gets the client's UDP port.
     * @return The UDP port number.
     */
    quint16 getUdpPort() const;

    /**
     * @brief Gets the client's name.
     * @return The client's name as a QString.
     */
    QString getName() const;

    /**
     * @brief Gets the client's assigned color.
     * @return The client's color as a QColor.
     */
    QColor getColor() const;

    /**
     * @brief Sets the client's UDP port.
     * @param port The new UDP port.
     */
    void setUdpPort(const quint16 port);

    /**
     * @brief Sets the client's name.
     * @param name The new name for the client.
     */
    void setName(const QString &name);

    /**
     * @brief Sets the client's color.
     * @param color The new color for the client.
     */
    void setColor(const QColor &color);

private:
    int m_id;                 ///< Unique identifier for the client.
    QTcpSocket *m_tcp_socket; ///< TCP socket associated with the client.
    quint16 m_udp_port;       ///< UDP port used by the client.
    QString m_name;           ///< Name of the client.
    QColor m_color;           ///< Assigned color of the client.
};

#endif // CLIENT_H
