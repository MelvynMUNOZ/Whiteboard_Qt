#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , Connexion(new connexion(this))
{
    ui->setupUi(this);
    setCentralWidget(Connexion);
}

MainWindow::~MainWindow()
{
    delete ui;
}


