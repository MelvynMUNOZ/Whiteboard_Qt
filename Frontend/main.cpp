#include "mainwindow.h"
#include "connection.h"
#include "whiteboard.h"
#include "ihmfrontend.h"

#include <QApplication>
#include <QStackedWidget>
#include <QFile>

data_client globalDataClient;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile  styleFile(":/resources.qss");
    styleFile.open(QFile::ReadOnly);
    QString  style(styleFile.readAll());
    a.setStyleSheet(style);

    QStackedWidget stackedWidget;
    globalDataClient.connectionWidget = new connection(&stackedWidget);
    globalDataClient.whiteboardWidget = new whiteboard(&stackedWidget);
    stackedWidget.addWidget(globalDataClient.connectionWidget);
    stackedWidget.addWidget(globalDataClient.whiteboardWidget);

    stackedWidget.setCurrentWidget(globalDataClient.connectionWidget);
    stackedWidget.setFixedSize(800, 600);
    stackedWidget.show();

    QObject::connect(globalDataClient.connectionWidget, &connection::connectionSuccessful, [&]() {
        stackedWidget.setCurrentWidget(globalDataClient.whiteboardWidget);
    });

    //Mise en place de l'attente de signal correspondant a la reponse du message REQUEST_ALL_CLIENT_INFOS
    QObject::connect(globalDataClient.connectionWidget, &connection::getClientInfosSignal, [&](int id_client) {
        globalDataClient.whiteboardWidget->updateListClientInfos(id_client);
    });

    return a.exec();
}
