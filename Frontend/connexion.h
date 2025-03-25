#ifndef CONNEXION_H
#define CONNEXION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QMainWindow>
#include <QString>

#define TCP_PORT 12345
#define UDP_PORT 12346

class connexion : public QWidget
{
    Q_OBJECT
public:
    explicit connexion(QWidget *parent = nullptr);

    void createConnexionLayout();

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

    QHBoxLayout *hBoxConnexion;
    QLineEdit *lineEditIP;
    QPushButton *pushButtonConnexion;

    void on_pushButtonConnexion_clicked();


signals:
};

#endif // CONNEXION_H
