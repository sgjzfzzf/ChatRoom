#include "chatroomserver.h"
#include "ui_chatroomserver.h"

ChatRoomServer::ChatRoomServer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatRoomServer)
{
    ui->setupUi(this);
    serverSocket = new QTcpServer;
    liveUserLabel = new QLabel(QString::fromLocal8Bit("在线用户列表"));
    liveRoomLabel = new QLabel(QString::fromLocal8Bit("在线聊天室列表"));
    liveUserList = new QListWidget;
    liveRoomList = new QListWidget;
    mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(liveUserLabel);
    mainLayout->addWidget(liveUserList);
    mainLayout->addWidget(liveRoomLabel);
    mainLayout->addWidget(liveRoomList);

    port = 8000;
    serverSocket->listen(QHostAddress::Any, 8000);

    connect(serverSocket, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}

ChatRoomServer::~ChatRoomServer()
{
    QDir dir(QDir::currentPath());
    if (dir.exists("doc"))
    {
        dir.cd("doc");
        dir.removeRecursively();
    }
    delete ui;
}

void ChatRoomServer::updateLiveObjects()
{
    QJsonObject json;
    json.insert("sender", "");
    json.insert("contentType", "updateInfo");
    QJsonArray liveUsers;
    for (QMap<QString, QTcpSocket *>::iterator it = users.begin(); it != users.end(); ++it)
    {
        liveUsers.append(it.key());
    }
    QJsonObject liveRooms;
    for (QMap<QString, ChatRoom *>::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        QJsonArray liveUsersInRooms;
        QList<QString> liveUsers = it.value()->allUsers();
        for (QList<QString>::iterator it = liveUsers.begin(); it != liveUsers.end(); ++it)
        {
            liveUsersInRooms.append(*it);
        }
        liveRooms.insert(it.key(), liveUsersInRooms);
    }
    QJsonObject liveObjects;
    liveObjects.insert("liveUsers", liveUsers);
    liveObjects.insert("liveRooms", liveRooms);
    json.insert("content", liveObjects);
    for (QMap<QString, QTcpSocket *>::iterator it = users.begin(); it != users.end(); ++it)
    {
        QTcpSocket *socket = it.value();
        json.insert("receiver", it.key());
        QJsonDocument document(json);
        socket->write(document.toJson());
    }
}

void ChatRoomServer::handleNewConnection()
{
    QTcpSocket *clientSocket = serverSocket->nextPendingConnection();
    QString userName;
    if (clientSocket->waitForReadyRead())
    {
        userName = QString::fromLocal8Bit(clientSocket->readAll());
    }
    users.insert(userName, clientSocket);
    liveUserList->addItem(userName);
    updateLiveObjects();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
}

void ChatRoomServer::handleNewData()
{
    // This method is used to handle new data sent into this server.
    QTcpSocket *clientSocket = (QTcpSocket *)sender();
    QByteArray contentByteArray = clientSocket->readAll();
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(contentByteArray, &error);
    // If the json data is illegal, it'll stop this process.
    if (document.isNull() || (error.error != QJsonParseError::NoError))
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        return;
    }
    QJsonObject json = document.object();
    QString sender, receiver, contentType;
    if (json.contains("sender"))
    {
        sender = json.value("sender").toString();
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        return;
    }
    if (json.contains("receiver"))
    {
        receiver = json.value("receiver").toString();
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        return;
    }
    if (json.contains("contentType"))
    {
        contentType = json.value("contentType").toString();
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        return;
    }
    if (sender != "" && receiver != "" && contentType == "text")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        QTcpSocket *clientSocket = users.find(receiver).value();
        clientSocket->write(contentByteArray);
    }
    else if (sender != "" && receiver != "" && (contentType == "requireFile" || contentType == "requireRoomFile"))
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        QTcpSocket *clientSocket = users.find(receiver).value();
        QString fileName = json.value("content").toString();
        QDir dir(QDir::currentPath());
        if (!dir.exists("doc"))
        {
            dir.mkdir("doc");
        }
        dir.cd("doc");
        fileName = QString("%1/%2").arg(dir.absolutePath(), fileName);
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }
        QByteArray fileData = file.readAll();
        QJsonObject json;
        json.insert("sender", sender);
        json.insert("receiver", receiver);
        if (contentType == "requireFile")
        {
            json.insert("contentType", "replyFile");
        }
        else if (contentType == "requireRoomFile")
        {
            json.insert("contentType", "replyRoomFile");
        }
        QJsonObject fileInfo;
        fileInfo.insert("fileName", fileName.split('/').last());
        fileInfo.insert("fileSize", fileData.size());
        json.insert("content", fileInfo);
        QJsonDocument document(json);
        clientSocket->write(document.toJson());
        if (clientSocket->waitForBytesWritten());
        else
        {
            return;
        }
        clientSocket->write(fileData);
    }
    else if (sender != "" && receiver != "" && contentType == "img")
    {
        if (json.contains("content"))
        {
            QJsonObject imgInfo = json.value("content").toObject();
            if (imgInfo.contains("imgName") && imgInfo.contains("imgSize"))
            {
                int imgSize = imgInfo.value("imgSize").toInt();
                QByteArray imgContentByteArray, subImgContentByteArray;
                do {
                    if (clientSocket->waitForReadyRead())
                    {
                        subImgContentByteArray = clientSocket->readAll();
                        imgContentByteArray.append(subImgContentByteArray);
                        imgSize -= subImgContentByteArray.size();
                    }
                    else
                    {
                        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
                        return;
                    }
                } while (imgSize > 0);
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
                QTcpSocket *clientSocket = users.find(receiver).value();
                clientSocket->write(document.toJson());
                if (clientSocket->waitForBytesWritten());
                else
                {
                    return;
                }
                QThread::msleep(100);
                clientSocket->write(imgContentByteArray);
            }
            else
            {
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
                return;
            }
        }
        else
        {
            connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
            return;
        }
    }
    else if (sender != "" && receiver != "" && contentType == "roomText")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        ChatRoom *room = rooms.find(receiver).value();
        QList<QString> allLiveUsersInRoom = room->allUsers();
        for (QList<QString>::iterator it = allLiveUsersInRoom.begin(); it != allLiveUsersInRoom.end(); ++it)
        {
            users.find(*it).value()->write(contentByteArray);
        }
    }
    else if (sender != "" && receiver == "" && contentType == "roomCreate")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        if (json.contains("content"))
        {
            QJsonObject roomInfo = json.value("content").toObject();
            if (roomInfo.contains("roomName"))
            {
                QString roomName = roomInfo.value("roomName").toString();
                ChatRoom *room = new ChatRoom(roomName, sender, clientSocket);
                rooms.insert(roomName, room);
                liveRoomList->clear();
                updateLocalLivaObjects();
                updateLiveObjects();
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    else if (sender != "" && receiver == "" && contentType == "roomExit")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        if (json.contains("content"))
        {
            QString roomName = json.value("content").toString();
            ChatRoom *room = rooms.find(roomName).value();
            room->removeUser(sender);
            if (room->count() == 0)
            {
                rooms.remove(roomName);
                updateLocalLivaObjects();
            }
            updateLiveObjects();
        }
    }
    else if (sender != "" && receiver != "" && (contentType == "file" || contentType == "roomFile" ))
    {
        if (json.contains("content"))
        {
            QJsonObject fileInfo = json.value("content").toObject();
            if (fileInfo.contains("fileName") && fileInfo.contains("fileSize"))
            {
                QString fileName = fileInfo.value("fileName").toString();
                int fileSize = fileInfo.value("fileSize").toInt();
                QByteArray fileContentArray, subFileContentArray;
                while (fileSize > 0)
                {
                    if (clientSocket->waitForReadyRead())
                    {
                        subFileContentArray = clientSocket->readAll();
                        fileSize -= subFileContentArray.size();
                        fileContentArray.append(subFileContentArray);
                    }
                }
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
                QDir dir(QDir::currentPath());
                if (!dir.exists("doc"))
                {
                    dir.mkdir("doc");
                }
                QFile file(QString("doc/%1").arg(fileName));
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(fileContentArray);
                    if (contentType == "file")
                    {
                        if (users.contains(receiver))
                        {
                            users.find(receiver).value()->write(contentByteArray);
                        }
                        else
                        {
                            return;
                        }
                    }
                    else if (contentType == "roomFile")
                    {
                        if (rooms.contains(receiver))
                        {
                            QList<QString> allLiveUsersInRoom = rooms.find(receiver).value()->allUsers();
                            for (QList<QString>::iterator it = allLiveUsersInRoom.begin(); it != allLiveUsersInRoom.end(); ++it)
                            {
                                users.find(*it).value()->write(contentByteArray);
                            }
                        }
                        else
                        {
                            return;
                        }
                    }
                }
                else
                {
                    QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("文件接收错误"));
                }
            }
            else
            {
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
                return;
            }
        }
        else
        {
            connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
            return;
        }
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        return;
    }
}

void ChatRoomServer::handleDisconnection()
{
    // Handle the disconnetion from client, remove the "UserSocket" from the map and disconnet functions.
    // The server will also broadcast this information to all clients.
    QString userName;
    QTcpSocket *clientSocket = (QTcpSocket *)sender();
    for (QMap<QString, QTcpSocket *>::iterator it = users.begin(); it != users.end(); ++it)
    {
        if (it.value() == clientSocket)
        {
            userName = it.key();
            break;
        }
    }
    users.remove(userName);
    updateLocalLivaObjects();
    for (QMap<QString, ChatRoom *>::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        ChatRoom *room = it.value();
        if (room->findUser(userName) != nullptr)
        {
            room->removeUser(userName);
        }
    }
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
    updateLiveObjects();
}

void ChatRoomServer::updateLocalLivaObjects()
{
    liveUserList->clear();
    liveRoomList->clear();
    for (QMap<QString, QTcpSocket *>::iterator it = users.begin(); it != users.end(); ++it)
    {
        liveUserList->addItem(it.key());
    }
    for (QMap<QString, ChatRoom *>::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        liveRoomList->addItem(it.key());
    }
}
