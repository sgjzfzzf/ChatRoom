#include "chatroom.h"

ChatRoom::ChatRoom(QString chatRoomName, QString userName, QTcpSocket *userSocket): chatRoomName(chatRoomName)
{
    users.insert(userName, userSocket);
}

unsigned int ChatRoom::count()
{
    return users.count();
}

QList<QString> ChatRoom::allUsers()
{
    QList<QString> allUsersList;
    for (QMap<QString, QTcpSocket *>::iterator it = users.begin(); it != users.end(); ++it)
    {
        allUsersList.append(it.key());
    }
    return allUsersList;
}

QTcpSocket* ChatRoom::findUser(QString userName)
{
    for (QMap<QString, QTcpSocket *>::iterator it = users.begin(); it != users.end(); ++it)
    {
        if (it.key() == userName)
        {
            return it.value();
        }
    }
    return nullptr;
}

void ChatRoom::addUser(QString userName, QTcpSocket *userSocket)
{
    users.insert(userName, userSocket);
}

void ChatRoom::removeUser(QString userName)
{
    users.remove(userName);
}
