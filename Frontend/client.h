#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>
#include <QHash>

/**
 * @class Client
 * @brief Represents a client connected via a TCP socket.
 *
 * This class stores information about a client, including its ID, TCP socket, name, and color.
 */
class Client
{
public:
    /**
     * @brief Constructs a Client object.
     * @param id The unique identifier of the client.
     * @param socket The TCP socket associated with the client.
     * @param color The color assigned to the client.
     */
    Client(int id, QTcpSocket *socket, QColor color);

    /**
     * @brief Destroys the Client object.
     *
     * Closes and deletes the TCP socket if it exists.
     */
    ~Client();

    /**
     * @brief Gets the client's ID.
     * @return The unique identifier of the client.
     */
    int getId() const;

    /**
     * @brief Sets the client's ID.
     * @param id The new identifier for the client.
     */
    void setId(const int id);

    /**
     * @brief Gets the TCP socket associated with the client.
     * @return A pointer to the QTcpSocket object.
     */
    QTcpSocket *getTcpSocket() const;

    /**
     * @brief Gets the client's name.
     * @return The name of the client.
     */
    QString getName() const;

    /**
     * @brief Gets the client's assigned color.
     * @return The QColor object representing the client's color.
     */
    QColor getColor() const;

    /**
     * @brief Sets the client's name.
     * @param name The new name to assign to the client.
     */
    void setName(const QString &name);

    /**
     * @brief Sets the client's color.
     * @param color The new color to assign to the client.
     */
    void setColor(const QColor &color);

private:
    int m_id;                ///< Unique identifier of the client.
    QTcpSocket *m_tcp_socket; ///< Pointer to the client's TCP socket.
    QString m_name;           ///< Name of the client.
    QColor m_color;           ///< Color assigned to the client.
};

#endif // CLIENT_H
