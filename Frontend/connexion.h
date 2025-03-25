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

class connexion : public QWidget
{
    Q_OBJECT
public:
    explicit connexion(QWidget *parent = nullptr);

    void createConnexionLayout();

private:
    QVBoxLayout *vBoxGeneral;
    QLabel *labelTitle;
    QLabel *labelPseudo;
    QLineEdit *lineEditPseudo;
    QLabel *labelIP;

    QHBoxLayout *hBoxConnexion;
    QLineEdit *lineEditIP;
    QPushButton *pushButtonConnexion;


signals:
};

#endif // CONNEXION_H
