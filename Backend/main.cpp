/**
 * @file main.cpp
 * @brief Entry point of application.
 * @author Raphael CAUSSE
 */

#include <QCoreApplication>

#include "whiteboardserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    WhiteboardServer server;
    server.start();

    return app.exec();
}
