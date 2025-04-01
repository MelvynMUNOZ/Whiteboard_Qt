#include "connection.h"
#include <QEventLoop>
#include <QTimer>

connection::connection(QWidget *parent)
    : QWidget{parent}
{
    createConnectionLayout();
}

void connection::createConnectionLayout(){
    vBoxGeneral = new QVBoxLayout(this);
    labelTitle = new QLabel("Connection to Canva", this);
    labelTitle->setObjectName("labelTitle");
    labelPseudo = new QLabel("Pseudo : ", this);
    lineEditPseudo = new QLineEdit(this);

    labelIP = new QLabel("Adresse IP : ", this);
    lineEditIP = new QLineEdit(this);
    pushButtonConnection = new QPushButton("Connection", this);
    pushButtonConnection->setObjectName("pushButtonConnection");

    //set all aligments
    vBoxGeneral->setAlignment(Qt::AlignCenter);
    labelTitle->setAlignment(Qt::AlignCenter);

    //set all spaces
    vBoxGeneral->setSpacing(10);

    //set all width
    lineEditPseudo->setFixedWidth(250);
    lineEditIP->setFixedWidth(250);
    pushButtonConnection->setFixedWidth(250);

    //add in the general layout
    vBoxGeneral->addStretch();
    vBoxGeneral->addWidget(labelTitle, 0, Qt::AlignTop);
    vBoxGeneral->addSpacing(50);
    vBoxGeneral->addWidget(labelPseudo, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(lineEditPseudo, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(labelIP, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(lineEditIP, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(pushButtonConnection, 0, Qt::AlignTop);
    vBoxGeneral->addStretch();

    setLayout(vBoxGeneral);

    QObject::connect(pushButtonConnection, &QPushButton::clicked, this, &connection::on_pushButtonConnection_clicked);

    messageConnectionKO = new QMessageBox();
    messageConnectionKO->setText("The IP adress have to be : X.X.X.X");
    messageConnectionKO->setIcon(QMessageBox::Critical);
    messageConnectionKO->setWindowTitle("Error");

    messageConnectionWAIT = new QMessageBox();
    messageConnectionWAIT->setText("Connection...");
    messageConnectionWAIT->setIcon(QMessageBox::Information);
    messageConnectionWAIT->setWindowTitle("Connection to canva");
    messageConnectionWAIT->setStandardButtons(QMessageBox::NoButton);
}

void connection::on_pushButtonConnection_clicked(){
    QString pseudo = lineEditPseudo->text();
    QString ip = lineEditIP->text();

    bool checkPseudo = pseudo.isEmpty();
    bool checkIP = ip.isEmpty();

    QRegularExpression regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionMatch matchIP = regex.match(ip);
    bool correctIP = matchIP.hasMatch();

    if(checkPseudo){
        lineEditPseudo->setStyleSheet("border: 1 solid red; border-radius: 2px; padding: 2px;");
    }else{
        lineEditPseudo->setStyleSheet("border: 1 solid green; border-radius: 2px; padding: 2px;");
    }
    if(checkIP || !correctIP){
        lineEditIP->setStyleSheet("border: 1 solid red; border-radius: 2px; padding: 2px;");
        connection::messageBox_IP();
    }else{
        lineEditIP->setStyleSheet("border: 1 solid green; border-radius: 2px; padding: 2px;");
    }

    if(!checkPseudo && !checkIP && correctIP){
        connection::connectionCanva();
    }
}

void connection::messageBox_IP(){
    messageConnectionKO->exec();
}

void connection::connectionCanva(){
    try_to_connect = 0;
    messageConnectionWAIT->show();
    isConnected = false;

    QEventLoop loop;
    QTimer timer;
    //wait the connection to the canva
    connect(&timer, &QTimer::timeout, [&](){
        qDebug() << "Tentative de connexion : " << try_to_connect;
        try_to_connect++;
        if(isConnected || try_to_connect > timeout){
            qDebug() << "Fin des tentatives de connexion : " << try_to_connect;
            loop.quit();
            try_to_connect = 0;
            emit connectionSuccessful();
        }
    });
    timer.start(500);
    loop.exec();
    //Close the message box when the connection is correct
    messageConnectionWAIT->hide();
}


