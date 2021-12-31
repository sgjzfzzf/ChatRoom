#include "createroomdialog.h"

CreateRoomDialog::CreateRoomDialog(QString userName, QTcpSocket *clientSocket): userName(userName), clientSocket(clientSocket)
{
    setAttribute(Qt::WA_DeleteOnClose);

    roomNameLabel = new QLabel(QString::fromLocal8Bit("房间名："));
    roomNameEdit = new QLineEdit;
    createRoomBtn = new QPushButton(QString::fromLocal8Bit("创建"));
    mainLayout = new QGridLayout(this);

    mainLayout->addWidget(roomNameLabel, 0, 0);
    mainLayout->addWidget(roomNameEdit, 0, 1);
    mainLayout->addWidget(createRoomBtn, 2, 1);

    connect(createRoomBtn, SIGNAL(clicked()), this, SLOT(tryToCreateRoom()));
}

void CreateRoomDialog::tryToCreateRoom()
{
    QString roomName = roomNameEdit->text().trimmed();
    QJsonObject json, roomInfo;
    json.insert("sender", userName);
    json.insert("receiver", "");
    json.insert("contentType", "roomCreate");
    roomInfo.insert("roomName", roomName);
    json.insert("content", roomInfo);
    QJsonDocument document(json);
    clientSocket->write(document.toJson());
    emit createMultiUsersDialog(roomName);
    close();
}
