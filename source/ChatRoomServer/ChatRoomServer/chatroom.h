#ifndef CHATROOM_H
#define CHATROOM_H

#include <QObject>
#include <QTcpSocket>
#include <QMap>

class ChatRoom
{
public:
    ChatRoom(QString, QString, QTcpSocket *);
    unsigned int count();
    QList<QString> allUsers();
    QTcpSocket* findUser(QString);
    void addUser(QString, QTcpSocket *);
    void removeUser(QString);

private:
    QString chatRoomName;
    QMap<QString, QTcpSocket *> users;
};

#endif // CHATROOM_H
