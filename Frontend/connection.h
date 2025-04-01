#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QMainWindow>
#include <QString>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtEndian>

/**
 * @class connection
 * @brief Manages the connection interface and interactions with the server.
 */
class connection : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a connection interface.
     * @param parent Parent widget.
     */
    explicit connection(QWidget *parent = nullptr);

    /**
     * @brief Creates layouts for the connection interface.
     */
    void createConnectionLayout();

private:
    QVBoxLayout *vBoxGeneral; ///< General vertical layout.
    QLabel *labelTitle; ///< Title label.
    QLabel *labelPseudo; ///< Label for username input.
    QLineEdit *lineEditPseudo; ///< Input field for username.
    QLabel *labelIP; ///< Label for IP address input.

    QHBoxLayout *hBoxConnection; ///< Horizontal layout for connection elements.
    QLineEdit *lineEditIP; ///< Input field for IP address.
    QPushButton *pushButtonConnection; ///< Button to initiate connection.

    QMessageBox *messageConnectionKO; ///< Message box for incorrect IP format.
    QMessageBox *messageConnectionWAIT; ///< Message box for waiting during connection.

    /**
     * @brief Handles the connection button click event.
     */
    void on_pushButtonConnection_clicked();

    /**
     * @brief Displays a message box for incorrect IP format.
     */
    void messageBox_IP();

    /**
     * @brief Sets up the connection interface layout.
     */
    void connectionCanva();

    /**
     * @brief Initiates the connection to the server.
     */
    void connectionToServer();

    /**
     * @brief Processes incoming TCP data frames.
     * @param data The received TCP data.
     */
    void processTcpFrame(const QByteArray &data);

    /**
     * @brief Sends a message to register the client on the server.
     */
    void registerClientMessage();

    /**
     * @brief Sends a request to get all connected clients' information.
     */
    void requestAllClientInfoMessage();

    /**
     * @brief Extracts client information from a received payload.
     * @param payload The data payload containing client info.
     * @return The ID of the new client.
     */
    int getClientInfos(QByteArray payload);

    /**
     * @brief Sends a message to register the UDP port for communication.
     */
    void registerUDPPortMessage();

public slots:
    /**
     * @brief Handles incoming TCP data.
     */
    void onTCPReadyRead();

    /**
     * @brief Handles a successful connection event.
     */
    void onConnectedOK();

    /**
     * @brief Handles a failed connection event.
     */
    void onConnectedKO();

signals:
    /**
     * @brief Signal emitted when the connection is successfully established.
     */
    void connectionSuccessful();

    /**
     * @brief Signal emitted when new client information is received.
     * @param id_new_client ID of the new client.
     */
    void getClientInfosSignal(int id_new_client);
};

#endif // CONNECTION_H
