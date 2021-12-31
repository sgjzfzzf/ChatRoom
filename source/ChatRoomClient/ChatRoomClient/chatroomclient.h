#ifndef CHATROOMCLIENT_H
#define CHATROOMCLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include "connectdialog.h"
#include "singleuserdialog.h"
#include "createroomdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatRoomClient; }
QT_END_NAMESPACE

class ChatRoomClient : public QDialog
{
    Q_OBJECT

public:
    ChatRoomClient(QWidget *parent = nullptr);
    ~ChatRoomClient();

signals:
    void receiveRelatedText(QString, QString, QString, QString); // The third paramter is the name of room. Null string means that this message is sent to a single user.
    void receiveRelatedFile(QString, QString, QString, QString);
    void receiveRelatedReplyFile(QString, QString, QString, QString);
    void receiveRelatedImg(QString, QString, QString, QString);
    void updateRoomInfo(QString, QList<QString>);

public slots:
    void connectActionTriggered();
    void connectToServer(QString, int, QString);
    void disconnectToServer();
    void updateClient();
    void openSingleUserDialog();
    void closeSingleUserDialog(QString);
    void createRoom();
    void createMultiUsersDialogBtnClicked();
    void openMultiUsersDialog(QString);
    void closeMultiUsersDialog(QString);
    void clearLocalFiles();

private:
    Ui::ChatRoomClient *ui;
    const QString FLAG_RECEIVE = "Receive json.";
    QString userName;
    QTcpSocket *clientSocket;
    QMap<QString, SingleUserDialog *> singleUserDialogs;
    QMap<QString, MultiUsersDialog *> multiUsersDialogs;
    QMenuBar *menuBar;
    QMenu *connectMenu;
    QMenu *fileMenu;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *createRoomAction;
    QAction *clearFilesAction;
    QLabel *liveUserLabel;
    QListWidget *liveUserList;
    QLabel *liveRoomLabel;
    QListWidget *liveRoomList;
    QVBoxLayout *mainLayout;
};
#endif // CHATROOMCLIENT_H
