#include "whiteboard.h"

whiteboard::whiteboard(QWidget *parent)
    : QWidget{parent}
    ,imageWhiteboard(new QImage(QGuiApplication::primaryScreen()->geometry().size(),
                                   QImage::Format_ARGB32_Premultiplied))
{
    //setMinimumSize(800, 600);
    setWindowFlags(Qt::WindowMinimizeButtonHint);
    createWhiteboardLayout();

    painterWhiteboard = new QPainter(imageWhiteboard);
    enable = false;
    writer = true;

}

void whiteboard::createWhiteboardLayout(){
    vBoxGeneral = new QVBoxLayout(this);
    labelTitle = new QLabel(this);

    hBoxChoices = new QHBoxLayout(); //pas de parent pour ne pas avoir de conflit
    pushButtonPen = new QPushButton("Pen", this);
    pushButtonRubber = new QPushButton("Rubber", this);

    hBoxWhiteboard = new QHBoxLayout(); //pas de parent pour ne pas avoir de conflit
    listPseudo = new QListWidget(this);
    listPseudo->setFixedWidth(150);

    vBoxGeneral->addWidget(labelTitle);

    hBoxChoices->addWidget(pushButtonPen);
    hBoxChoices->addWidget(pushButtonRubber);
    vBoxGeneral->addLayout(hBoxChoices);

    QWidget *whiteboardWidget = new QWidget(this);
    whiteboardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    hBoxWhiteboard->addWidget(whiteboardWidget);
    hBoxWhiteboard->addWidget(listPseudo);

    vBoxGeneral->addLayout(hBoxWhiteboard);

    setLayout(vBoxGeneral);
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
