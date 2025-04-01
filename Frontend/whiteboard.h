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

class whiteboard : public QWidget
{
    Q_OBJECT
public:
    explicit whiteboard(QWidget *parent = nullptr);

    void createWhiteboardLayout();
    void updateListClientInfos(int id_client);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QVBoxLayout *vBoxGeneral;
    QLabel *labelTitle;

    QHBoxLayout *hBoxChoices;
    QPushButton *pushButtonPen;
    QPushButton *pushButtonRubber;

    QHBoxLayout *hBoxWhiteboard;
    QListWidget *listPseudo;

    QPainter *painterWhiteboard;
    QImage *imageWhiteboard;
    QPoint pointBegin;
    QPoint pointEnd;
    bool enable;

    QPen *pen;
    bool writer = true; //pour faire la différence entre le crayon et la gomme

    void on_pushButtonPen_clicked();
    void on_pushButtonRubber_clicked();

    //UPD
    void processUdpFrame(const QHostAddress sender, const quint16 sender_port, const QByteArray &data);
    void dataCanvasClients(QPoint pointBegin, QPoint pointEnd);
    void dataCanvasSync(const QByteArray &payload);

public slots:
    void onUdpReadyRead();

signals:
};

#endif // WHITEBOARD_H
