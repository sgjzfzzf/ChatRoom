#include "chatroomclient.h"
#include "ui_chatroomclient.h"

ChatRoomClient::ChatRoomClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatRoomClient)
{
    ui->setupUi(this);
    clientSocket = new QTcpSocket;
    menuBar = new QMenuBar;
    connectMenu = new QMenu(QString::fromLocal8Bit("连接"));
    fileMenu = new QMenu(QString::fromLocal8Bit("文件"));
    connectAction = new QAction(QString::fromLocal8Bit("连接到服务器"));
    disconnectAction = new QAction(QString::fromLocal8Bit("从服务器断开"));
    createRoomAction = new QAction(QString::fromLocal8Bit("创建聊天室"));
    clearFilesAction = new QAction(QString::fromLocal8Bit("清空本地文件"));
    liveUserLabel = new QLabel(QString::fromLocal8Bit("在线用户列表"));
    liveRoomLabel = new QLabel(QString::fromLocal8Bit("在线聊天室列表"));
    liveUserList = new QListWidget;
    liveRoomList = new QListWidget;
    mainLayout = new QVBoxLayout(this);

    menuBar->addMenu(connectMenu);
    menuBar->addMenu(fileMenu);
    connectMenu->addAction(connectAction);
    connectMenu->addAction(disconnectAction);
    connectMenu->addAction(createRoomAction);
    fileMenu->addAction(clearFilesAction);
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    createRoomAction->setEnabled(false);
    clearFilesAction->setEnabled(true);

    mainLayout->addWidget(menuBar);
    mainLayout->addWidget(liveUserLabel);
    mainLayout->addWidget(liveUserList);
    mainLayout->addWidget(liveRoomLabel);
    mainLayout->addWidget(liveRoomList);

    connect(connectAction, SIGNAL(triggered()), this, SLOT(connectActionTriggered()));
    connect(disconnectAction, SIGNAL(triggered()), this, SLOT(disconnectToServer()));
    connect(createRoomAction, SIGNAL(triggered()), this, SLOT(createRoom()));
    connect(clearFilesAction, SIGNAL(triggered()), this, SLOT(clearLocalFiles()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

ChatRoomClient::~ChatRoomClient()
{
    delete ui;
}

void ChatRoomClient::connectActionTriggered()
{
    ConnectDialog *dialog = new ConnectDialog;
    connect(dialog, SIGNAL(returnDialog(QString,int,QString)), this, SLOT(connectToServer(QString,int,QString)));
    dialog->show();
}

void ChatRoomClient::connectToServer(QString address, int port, QString newUserName)
{
    // The method try to connect to server.
    if ((userName = newUserName) == "")
    {
        QMessageBox::information(this, QString::fromLocal8Bit("连接到服务器"), QString::fromLocal8Bit("请输入用户名"));
        return;
    }
    clientSocket->connectToHost(address, port);
    if (clientSocket->state() == QAbstractSocket::UnconnectedState)
    {
        QMessageBox::information(this, QString::fromLocal8Bit("连接服务器"), QString::fromLocal8Bit("连接失败，请检查您的输入"));
        return;
    }
    clientSocket->write(userName.toLocal8Bit());
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    createRoomAction->setEnabled(true);
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnectToServer()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

void ChatRoomClient::disconnectToServer()
{
    // Deal with the situations when disconnection happens, such as set buttons enabled or unenables.
    clientSocket->disconnectFromHost();
    if (clientSocket->state() == QAbstractSocket::ConnectedState)
    {
        QMessageBox::information(this, QString::fromLocal8Bit("连接服务器"), QString::fromLocal8Bit("断开失败，请重试"));
        return;
    }
    else
    {
        liveUserList->clear();
        liveRoomList->clear();
        singleUserDialogs.clear();
        multiUsersDialogs.clear();
        connectAction->setEnabled(true);
        disconnectAction->setEnabled(false);
        createRoomAction->setEnabled(false);
    }
}

void ChatRoomClient::updateClient()
{
    QByteArray contentByteArray = clientSocket->readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(contentByteArray, &error);
    if (document.isNull() || (error.error != QJsonParseError::NoError))
    {
        return;
    }
    QJsonObject json = document.object();
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    QString sender, receiver, contentType;
    if (json.contains("sender"))
    {
        sender = json.value("sender").toString();
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        return;
    }
    if (json.contains("receiver"))
    {
        receiver = json.value("receiver").toString();
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        return;
    }
    if (json.contains("contentType"))
    {
        contentType = json.value("contentType").toString();
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        return;
    }
    if (sender == "" && receiver == userName && contentType == "updateInfo")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        if (json.contains("content"))
        {
            liveUserList->clear();
            liveRoomList->clear();
            QJsonObject liveObjects = json.value("content").toObject();
            if (liveObjects.contains("liveUsers"))
            {
                QJsonArray liveUsers = liveObjects.value("liveUsers").toArray();
                for (QJsonArray::iterator it = liveUsers.begin(); it != liveUsers.end(); ++it)
                {
                    QListWidgetItem *item = new QListWidgetItem;
                    liveUserList->addItem(item);
                    QPushButton *btn = new QPushButton;
                    btn->setText(it->toString());
                    connect(btn, SIGNAL(clicked()), this, SLOT(openSingleUserDialog()));
                    liveUserList->setItemWidget(item, btn);
                }
            }
            if (liveObjects.contains("liveRooms"))
            {
                QJsonObject liveRooms = liveObjects.value("liveRooms").toObject();
                for (QJsonObject::iterator it = liveRooms.begin(); it != liveRooms.end(); ++it)
                {
                    QString roomName = it.key();
                    QJsonArray allLiveUsersInRoom = it.value().toArray();
                    QList<QString> allLiveUsers;
                    for (QJsonArray::iterator it = allLiveUsersInRoom.begin(); it != allLiveUsersInRoom.end(); ++it)
                    {
                        allLiveUsers.append(it->toString());
                    }
                    QListWidgetItem *item = new QListWidgetItem;
                    liveRoomList->addItem(item);
                    QPushButton *btn = new QPushButton;
                    btn->setText(it.key());
                    connect(btn, SIGNAL(clicked()), this, SLOT(createMultiUsersDialogBtnClicked()));
                    liveRoomList->setItemWidget(item, btn);
                    emit updateRoomInfo(roomName, allLiveUsers);
                }
            }
        }
    }
    else if (receiver == userName && contentType == "text")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        if (json.contains("content"))
        {
            emit receiveRelatedText(sender, receiver, "", json.value("content").toString());
        }
        else
        {
            return;
        }
    }
    else if (multiUsersDialogs.contains(receiver) && contentType == "roomText")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        if (json.contains("content"))
        {
            emit receiveRelatedText(sender, "", receiver, json.value("content").toString());
        }
        else
        {
            return;
        }
    }
    else if (receiver == userName && contentType == "file")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        if (json.contains("content"))
        {
            emit receiveRelatedFile(sender, receiver, "", json.value("content").toObject().value("fileName").toString());
        }
        else
        {
            return;
        }
    }
    else if (contentType == "replyFile" || contentType == "replyRoomFile")
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
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
                QDir dir(QDir::currentPath());
                if (!dir.exists("doc"))
                {
                    dir.mkdir("doc");
                }
                dir.cd("doc");
                QString absoluteFileName = QString("%1/%2").arg(dir.absolutePath(), fileName);
                QFile file(absoluteFileName);
                if (!file.open(QFile::WriteOnly))
                {
                    QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("存储文件失败"));
                    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
                    return;
                }
                file.write(fileContentArray);
                emit receiveRelatedReplyFile(sender, receiver, "", fileName);
            }
            else
            {
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
                return;
            }
        }
        else
        {
            connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
            return;
        }
    }
    else if (contentType == "img")
    {
        if (json.contains("content"))
        {
            QJsonObject imgInfo = json.value("content").toObject();
            if (imgInfo.contains("imgName") && imgInfo.contains("imgSize"))
            {
                QString imgName = imgInfo.value("imgName").toString();
                int imgSize = imgInfo.value("imgSize").toInt();
                QByteArray fileContentArray, subFileContentArray;
                while (imgSize > 0)
                {
                    if (clientSocket->waitForReadyRead())
                    {
                        subFileContentArray = clientSocket->readAll();
                        imgSize -= subFileContentArray.size();
                        fileContentArray.append(subFileContentArray);
                    }
                }
                connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
                QDir dir(QDir::currentPath());
                if (!dir.exists("img"))
                {
                    dir.mkdir("img");
                }
                dir.cd("img");
                QString absoluteFileName = QString("%1/%2").arg(dir.absolutePath(), imgName);
                QFile file(absoluteFileName);
                if (!file.open(QFile::WriteOnly))
                {
                    QMessageBox::information(this, QString::fromLocal8Bit("图片传输"), QString::fromLocal8Bit("存储图片失败"));
                    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
                    return;
                }
                file.write(fileContentArray);
                emit receiveRelatedImg(sender, receiver, "", imgName);
            }
        }
        else
        {
            return;
        }
    }
    else if (contentType == "roomFile")
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        if (json.contains("content"))
        {
            emit receiveRelatedFile(sender, "", receiver, json.value("content").toObject().value("fileName").toString());
        }
        else
        {
            return;
        }
    }
    else
    {
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        return;
    }
}

void ChatRoomClient::openSingleUserDialog()
{
    QPushButton *btn = (QPushButton *)sender();
    QString otherName = btn->text();
    SingleUserDialog *dialog = new SingleUserDialog(userName, otherName, clientSocket);
    singleUserDialogs.insert(otherName, dialog);
    connect(this, SIGNAL(receiveRelatedText(QString,QString,QString,QString)), dialog, SLOT(updateClientText(QString,QString,QString,QString)));
    connect(this, SIGNAL(receiveRelatedFile(QString,QString,QString,QString)), dialog, SLOT(updateClientFile(QString,QString,QString,QString)));
    connect(this, SIGNAL(receiveRelatedReplyFile(QString,QString,QString,QString)), dialog, SLOT(updateClientLocalFile(QString,QString,QString,QString)));
    connect(this, SIGNAL(receiveRelatedImg(QString,QString,QString,QString)), dialog, SLOT(updateClientLocalImg(QString,QString,QString,QString)));
    connect(dialog, SIGNAL(closeSingleUserDialog(QString)), this, SLOT(closeSingleUserDialog(QString)));
    dialog->show();
}

void ChatRoomClient::closeSingleUserDialog(QString otherName)
{
    SingleUserDialog *dialog = singleUserDialogs.find(otherName).value();
    singleUserDialogs.remove(otherName);
    disconnect(this, SIGNAL(receiveRelatedText(QString,QString,QString,QString)), dialog, SLOT(updateClientText(QString,QString,QString,QString)));
    disconnect(this, SIGNAL(receiveRelatedFile(QString,QString,QString,QString)), dialog, SLOT(updateClientFile(QString,QString,QString,QString)));
    disconnect(this, SIGNAL(receiveRelatedReplyFile(QString,QString,QString,QString)), dialog, SLOT(updateClientLocalFile(QString,QString,QString,QString)));
    disconnect(this, SIGNAL(receiveRelatedImg(QString,QString,QString,QString)), dialog, SLOT(updateClientLocalImg(QString,QString,QString,QString)));
    disconnect(dialog, SIGNAL(closeSingleUserDialog(QString)), this, SLOT(closeSingleUserDialog(QString)));
}

void ChatRoomClient::createRoom()
{
    CreateRoomDialog *dialog = new CreateRoomDialog(userName, clientSocket);
    connect(dialog, SIGNAL(createMultiUsersDialog(QString)), this, SLOT(openMultiUsersDialog(QString)));
    dialog->show();
}

void ChatRoomClient::createMultiUsersDialogBtnClicked()
{
    QPushButton *btn = (QPushButton *)sender();
    openMultiUsersDialog(btn->text());
}

void ChatRoomClient::openMultiUsersDialog(QString roomName)
{
    MultiUsersDialog *dialog = new MultiUsersDialog(userName, roomName, clientSocket);
    multiUsersDialogs.insert(roomName, dialog);
    connect(this, SIGNAL(receiveRelatedText(QString,QString,QString,QString)), dialog, SLOT(updateClientText(QString,QString,QString,QString)));
    connect(this, SIGNAL(receiveRelatedFile(QString,QString,QString,QString)), dialog, SLOT(updateClientFile(QString,QString,QString,QString)));
    connect(this, SIGNAL(receiveRelatedReplyFile(QString,QString,QString,QString)), dialog, SLOT(updateClientLocalFile(QString,QString,QString,QString)));
    connect(this, SIGNAL(updateRoomInfo(QString,QList<QString>)), dialog, SLOT(updateRoomInfo(QString,QList<QString>)));
    connect(dialog, SIGNAL(closeMultiUsersDialog(QString)), this, SLOT(closeMultiUsersDialog(QString)));
    dialog->show();
}

void ChatRoomClient::closeMultiUsersDialog(QString roomName)
{
    MultiUsersDialog *dialog = multiUsersDialogs.find(roomName).value();
    multiUsersDialogs.remove(roomName);
    disconnect(this, SIGNAL(receiveRelatedText(QString,QString,QString,QString)), dialog, SLOT(updateClientText(QString,QString,QString,QString)));
    disconnect(this, SIGNAL(receiveRelatedFile(QString,QString,QString,QString)), dialog, SLOT(updateClientFile(QString,QString,QString,QString)));
    disconnect(this, SIGNAL(receiveRelatedReplyFile(QString,QString,QString,QString)), dialog, SLOT(updateClientLocalFile(QString,QString,QString,QString)));
    disconnect(this, SIGNAL(updateRoomInfo(QString, QList<QString>)), dialog, SLOT(updateRoomInfo(QString, QList<QString>)));
    disconnect(dialog, SIGNAL(closeMultiUsersDialog(QString)), this, SLOT(closeMultiUsersDialog(QString)));
}

void ChatRoomClient::clearLocalFiles()
{
    QDir dir(QDir::currentPath()), imgDir(QDir::currentPath());
    if (dir.exists("doc"))
    {
        dir.cd("doc");
        dir.removeRecursively();
    }
    if (imgDir.exists("img"))
    {
        imgDir.cd("img");
        imgDir.removeRecursively();
    }
}
