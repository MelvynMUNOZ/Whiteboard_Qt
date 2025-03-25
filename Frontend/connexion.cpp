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
    hBoxConnexion = new QHBoxLayout(this);
    lineEditIP = new QLineEdit(this);
    pushButtonConnexion = new QPushButton("Connexion", this);

    //add in the general layout
    vBoxGeneral->addWidget(labelTitle);
    vBoxGeneral->addWidget(labelPseudo);
    vBoxGeneral->addWidget(lineEditPseudo);
    vBoxGeneral->addWidget(labelIP);

    hBoxConnexion->addWidget(lineEditIP);
    hBoxConnexion->addWidget(pushButtonConnexion);

    vBoxGeneral->addLayout(hBoxConnexion);
}


