#include <QCoreApplication>

#include "whiteboardserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    WhiteboardServer server;
    server.start();

    return a.exec();
}
