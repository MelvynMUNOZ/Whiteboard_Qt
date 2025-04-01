#include "mainwindow.h"
#include "connection.h"
#include "whiteboard.h"

#include <QApplication>
#include <QStackedWidget>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile  styleFile(":/resources.qss");
    styleFile.open(QFile::ReadOnly);
    QString  style(styleFile.readAll());
    a.setStyleSheet(style);

    QStackedWidget stackedWidget;
    connection *connectionWidget = new connection(&stackedWidget);
    whiteboard *whiteboardWidget = new whiteboard(&stackedWidget);
    stackedWidget.addWidget(connectionWidget);
    stackedWidget.addWidget(whiteboardWidget);

    stackedWidget.setCurrentWidget(connectionWidget);
    stackedWidget.setFixedSize(800, 600);
    stackedWidget.show();

    QObject::connect(connectionWidget, &connection::connectionSuccessful, [&]() {
        stackedWidget.setCurrentWidget(whiteboardWidget);
    });

    return a.exec();
}
