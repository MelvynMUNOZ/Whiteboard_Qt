#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QMainWindow>
#include <QString>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtEndian>

#include "whiteboard.h"

class connection : public QWidget
{
    Q_OBJECT
public:
    explicit connection(QWidget *parent = nullptr);

    void createConnectionLayout();

private:

    QVBoxLayout *vBoxGeneral;
    QLabel *labelTitle;
    QLabel *labelPseudo;
    QLineEdit *lineEditPseudo;
    QLabel *labelIP;

    QHBoxLayout *hBoxConnection;
    QLineEdit *lineEditIP;
    QPushButton *pushButtonConnection;

    QMessageBox *messageConnectionKO;
    QMessageBox *messageConnectionWAIT;

    bool isConnected = false;
    //uint8_t try_to_connect = 0;
    //static constexpr uint8_t timeout = 5;

    void on_pushButtonConnection_clicked();
    void messageBox_IP();
    void connectionCanva();

    //Partie connexion au serveur
    bool connectionToServer();

    //TCP
    void processTcpFrame(const QByteArray &data);
    void registerClientMessage();
    void requestAllClientInfoMessage();
    int getClientInfos(QByteArray payload);
    void registerUDPPortMessage();

public slots:
    void onTCPReadyRead();

signals:
    void connectionSuccessful();
    void getClientInfosSignal(int id_new_client);
};

#endif // CONNECTION_H
