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

#include "whiteboard.h"

#define TCP_PORT 12345
#define UDP_PORT 12346

class connection : public QWidget
{
    Q_OBJECT
public:
    explicit connection(QWidget *parent = nullptr);

    void createConnectionLayout();

    enum MessageType{
        NONE,
        SEND_CLIENT_NAME,
        REQ_CLIENTS_INFOS,
        UPDATE_CLIENTS_INFOS,
        DATA_CANVAS_CLIENTS,
        DATA_CANVAS_SYNC
    };

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
    uint8_t try_to_connect = 0;
    static constexpr uint8_t timeout = 5;

    void on_pushButtonConnection_clicked();
    void messageBox_IP();
    void connectionCanva();


signals:
    void connectionSuccessful();
};

#endif // CONNECTION_H
