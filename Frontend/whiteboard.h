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

class whiteboard : public QWidget
{
    Q_OBJECT
public:
    explicit whiteboard(QWidget *parent = nullptr);

    void createWhiteboardLayout();

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

signals:
};

#endif // WHITEBOARD_H
