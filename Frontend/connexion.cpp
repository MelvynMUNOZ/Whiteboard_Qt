#include "connexion.h"

connexion::connexion(QWidget *parent)
    : QWidget{parent}
{
    createConnexionLayout();
}

void connexion::createConnexionLayout(){
    vBoxGeneral = new QVBoxLayout(this);
    labelTitle = new QLabel("Connexion to Canva", this);
    labelPseudo = new QLabel("Pseudo : ", this);
    lineEditPseudo = new QLineEdit(this);

    labelIP = new QLabel("Adresse IP : ", this);
    lineEditIP = new QLineEdit(this);
    pushButtonConnexion = new QPushButton("Connexion", this);

    //set all aligments
    vBoxGeneral->setAlignment(Qt::AlignCenter);
    labelTitle->setAlignment(Qt::AlignCenter);

    //set all spaces
    vBoxGeneral->setSpacing(10);

    //set all width
    lineEditPseudo->setFixedWidth(250);
    lineEditIP->setFixedWidth(250);
    pushButtonConnexion->setFixedWidth(250);

    //add in the general layout
    vBoxGeneral->addWidget(labelTitle, 0, Qt::AlignTop);
    vBoxGeneral->addStretch();
    vBoxGeneral->addWidget(labelPseudo, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(lineEditPseudo, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(labelIP, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(lineEditIP, 0, Qt::AlignTop);
    vBoxGeneral->addWidget(pushButtonConnexion, 0, Qt::AlignTop);
    vBoxGeneral->addStretch();

    setLayout(vBoxGeneral);

    QObject::connect(pushButtonConnexion, &QPushButton::clicked, this, &connexion::on_pushButtonConnexion_clicked);
}

void connexion::on_pushButtonConnexion_clicked(){
    QString pseudo = lineEditPseudo->text();
    QString ip = lineEditIP->text();

    bool checkPseudo = pseudo.isEmpty();
    bool checkIP = ip.isEmpty();

    QRegularExpression regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionMatch matchIP = regex.match(ip);
    bool correctIP = matchIP.hasMatch();

    if(checkPseudo){
        lineEditPseudo->setStyleSheet("border: 1 solid red");
    }else{
        lineEditPseudo->setStyleSheet("border: 1 solid green");
    }
    if(checkIP || !correctIP){
        lineEditIP->setStyleSheet("border: 1 solid red");
    }else{
        lineEditIP->setStyleSheet("border: 1 solid green");
    }

    if(!checkPseudo && !checkIP && correctIP){
        qDebug("Tentative de connexion");
    }
}


