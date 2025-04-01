#ifndef WHITEBOARD_H
#define WHITEBOARD_H

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QImage>
#include <QDesktopServices>
#include <QPushButton>
#include <QHostAddress>

/**
 * @class whiteboard
 * @brief Class representing an interactive whiteboard.
 *
 * This class allows drawing on a whiteboard, handling mouse inputs,
 * and synchronizing drawings between clients via UDP.
 */
class whiteboard : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for the whiteboard class.
     * @param parent Pointer to the parent widget (can be nullptr).
     */
    explicit whiteboard(QWidget *parent = nullptr);

    /**
     * @brief Creates and initializes the whiteboard UI.
     */
    void createWhiteboardLayout();

    /**
     * @brief Updates the list of connected clients' information.
     * @param id_client Identifier of the client to be added.
     */
    void updateListClientInfos(int id_client);

protected:
    /**
     * @brief Event handler for rendering the whiteboard.
     * @param event Paint event.
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Event handler for mouse press actions.
     * @param event Mouse event.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief Event handler for mouse movement actions.
     * @param event Mouse event.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief Event handler for mouse release actions.
     * @param event Mouse event.
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QVBoxLayout *vBoxGeneral; /**< General vertical layout. */
    QLabel *labelTitle; /**< Whiteboard title label. */

    QHBoxLayout *hBoxChoices; /**< Horizontal layout for tool selection. */
    QPushButton *pushButtonPen; /**< Button to select the pen tool. */
    QPushButton *pushButtonRubber; /**< Button to select the eraser tool. */

    QHBoxLayout *hBoxWhiteboard; /**< Horizontal layout for the whiteboard. */
    QListWidget *listPseudo; /**< List of connected clients' pseudonyms. */

    QPainter *painterWhiteboard; /**< Drawing object for the whiteboard. */
    QImage *imageWhiteboard; /**< Image used to store drawings. */
    QPoint pointBegin; /**< Starting point of the drawing stroke. */
    QPoint pointEnd; /**< Ending point of the drawing stroke. */
    bool enable; /**< Indicates whether drawing is enabled. */

    QPen *pen; /**< Drawing tool used. */
    bool writer = true; /**< Indicates whether writing mode is enabled (pen/eraser). */

    /**
     * @brief Activates the pen mode.
     */
    void on_pushButtonPen_clicked();

    /**
     * @brief Activates the eraser mode.
     */
    void on_pushButtonRubber_clicked();

    /**
     * @brief Processes received UDP frames.
     * @param sender Sender's address.
     * @param sender_port Sender's port.
     * @param data Received data.
     */
    void processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data);

    /**
     * @brief Sends drawing data to other clients.
     * @param pointBegin Starting point of the line.
     * @param pointEnd Ending point of the line.
     */
    void dataCanvasClients(QPoint pointBegin, QPoint pointEnd);

    /**
     * @brief Synchronizes drawings between clients.
     * @param payload Synchronized data.
     */
    void dataCanvasSync(const QByteArray &payload);

public slots:
    /**
     * @brief Handles incoming UDP data.
     */
    void onUdpReadyRead();

signals:
};

#endif // WHITEBOARD_H
