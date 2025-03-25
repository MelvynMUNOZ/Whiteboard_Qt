#include "whiteboard.h"

whiteboard::whiteboard(QWidget *parent)
    : QWidget{parent}
    ,imageWhiteboard(new QImage(QGuiApplication::primaryScreen()->geometry().size(),
                                   QImage::Format_ARGB32_Premultiplied))
{
    createWhiteboardLayout();
    painterWhiteboard = new QPainter(imageWhiteboard);
    enable = false;
}

void whiteboard::createWhiteboardLayout(){

}

void whiteboard::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.drawImage(0,0, *imageWhiteboard);
}

void whiteboard::mousePressEvent(QMouseEvent *event){
    enable = true;
    pointBegin = event->pos();
}

void whiteboard::mouseMoveEvent(QMouseEvent *event){
    if(!enable){
        event->accept();
        return;
    }
    pointEnd = event->pos();
    painterWhiteboard->setPen(QPen(QColor(0,0,255)));
    painterWhiteboard->drawLine(pointBegin, pointEnd);
    pointBegin = pointEnd;
    update();
}

void whiteboard::mouseReleaseEvent(QMouseEvent *event){
    enable = false;
}
