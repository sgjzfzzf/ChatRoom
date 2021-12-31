#ifndef CREATEROOMDIALOG_H
#define CREATEROOMDIALOG_H

#include <QDialog>
#include <QObject>
#include <QTcpSocket>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include "multiusersdialog.h"

class CreateRoomDialog : public QDialog
{
    Q_OBJECT
public:
    CreateRoomDialog(QString, QTcpSocket *);

signals:
    void createMultiUsersDialog(QString);

public slots:
    void tryToCreateRoom();

private:
    QString userName;
    QTcpSocket *clientSocket;
    QLabel *roomNameLabel;
    QLineEdit *roomNameEdit;
    QPushButton *createRoomBtn;
    QGridLayout *mainLayout;
};

#endif // CREATEROOMDIALOG_H
