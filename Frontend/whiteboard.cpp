#include "whiteboard.h"
#include <QObject>

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

    QObject::connect(pushButtonPen, &QPushButton::clicked, this, &whiteboard::on_pushButtonPen_clicked);
    QObject::connect(pushButtonRubber, &QPushButton::clicked, this, &whiteboard::on_pushButtonRubber_clicked);
}

void whiteboard::createWhiteboardLayout(){
    vBoxGeneral = new QVBoxLayout(this);
    labelTitle = new QLabel("Canva", this);
    labelTitle->setObjectName("labelTitle");
    labelTitle->setAlignment(Qt::AlignCenter);

    hBoxChoices = new QHBoxLayout(); //pas de parent pour ne pas avoir de conflit
    pushButtonPen = new QPushButton("Pen", this);
    pushButtonRubber = new QPushButton("Rubber", this);

    hBoxWhiteboard = new QHBoxLayout(); //pas de parent pour ne pas avoir de conflit
    listPseudo = new QListWidget(this);
    listPseudo->setFixedWidth(150);

    pen = new QPen();
    pen->setCapStyle(Qt::RoundCap);

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

    if(writer){
        pen->setBrush(QColor(0,0,255));
        pen->setWidth(3);
    }else{
        pen->setBrush(QColor(255,255,255));
        pen->setWidth(10);
    }
    painterWhiteboard->setPen(*pen);
    painterWhiteboard->drawLine(pointBegin, pointEnd);
    pointBegin = pointEnd;
    update();
}

void whiteboard::mouseReleaseEvent(QMouseEvent *event){
    enable = false;
}

void whiteboard::on_pushButtonPen_clicked(){
    writer = true;
}

void whiteboard::on_pushButtonRubber_clicked(){
    writer = false;
}
