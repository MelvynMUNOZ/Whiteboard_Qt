#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , Connexion(new connexion(this))
    , Whiteboard(new whiteboard(this))
{
    ui->setupUi(this);
    setCentralWidget(Connexion);
    //setCentralWidget(Whiteboard);
}

MainWindow::~MainWindow()
{
    delete ui;
}


