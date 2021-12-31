#ifndef CHATROOMSERVER_H
#define CHATROOMSERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QMessageBox>
#include <QThread>
#include "chatroom.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatRoomServer; }
QT_END_NAMESPACE

class ChatRoomServer : public QDialog
{
    Q_OBJECT

public:
    ChatRoomServer(QWidget *parent = nullptr);
    ~ChatRoomServer();
    void updateLiveObjects();

public slots:
    void handleNewConnection();
    void handleNewData();
    void handleDisconnection();

private:
    Ui::ChatRoomServer *ui;
    const QString FLAG_RECEIVE = "Receive json.";
    unsigned int port;
    QTcpServer *serverSocket;
    QMap<QString, QTcpSocket *> users;
    QMap<QString, ChatRoom *> rooms;
    QAction *connectAction;
    QAction *disconnectAction;
    QLabel *liveUserLabel;
    QListWidget *liveUserList;
    QLabel *liveRoomLabel;
    QListWidget *liveRoomList;
    QVBoxLayout *mainLayout;
    void updateLocalLivaObjects();
};
#endif // CHATROOMSERVER_H
