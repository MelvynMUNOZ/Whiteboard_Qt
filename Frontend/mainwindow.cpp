#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , Connection(new connection(this))
{
    ui->setupUi(this);
    //setCentralWidget(Connection);
}

MainWindow::~MainWindow()
{
    delete ui;
}


