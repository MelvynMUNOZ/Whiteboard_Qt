#ifndef WHITEBOARD_H
#define WHITEBOARD_H

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QImage>
#include <QDesktopServices>

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
    QLabel *labelTitle;
    QListWidget *listPseudo;

    QPainter *painterWhiteboard;
    QImage *imageWhiteboard;
    QPoint pointBegin;
    QPoint pointEnd;
    bool enable;

signals:
};

#endif // WHITEBOARD_H
