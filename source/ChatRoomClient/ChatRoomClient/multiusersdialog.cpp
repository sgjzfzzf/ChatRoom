#include "multiusersdialog.h"

MultiUsersDialog::MultiUsersDialog(QString userName, QString roomName, QTcpSocket *clientSocket): userName(userName), roomName(roomName), clientSocket(clientSocket)
{
    setAttribute(Qt::WA_DeleteOnClose);

    menuBar = new QMenuBar;
    fileMenu = new QMenu(QString::fromLocal8Bit("文件"));
    sendFileAction = new QAction(QString::fromLocal8Bit("发送本地文件"));
    sendImgAction = new QAction(QString::fromLocal8Bit("发送本地表情"));
    liveUsersList = new QListWidget;
    contentList = new QListWidget;
    textEdit = new QTextEdit;
    sendBtn = new QPushButton(QString::fromLocal8Bit("发送"));
    mainLayout = new QVBoxLayout(this);

    menuBar->addMenu(fileMenu);
    fileMenu->addAction(sendFileAction);
    fileMenu->addAction(sendImgAction);

    mainLayout->addWidget(menuBar);
    mainLayout->addWidget(liveUsersList);
    mainLayout->addWidget(contentList);
    mainLayout->addWidget(textEdit);
    mainLayout->addWidget(sendBtn);

    connect(sendBtn, SIGNAL(clicked()), this, SLOT(sendText()));
    connect(sendFileAction, SIGNAL(triggered()), this, SLOT(sendFile()));
}

MultiUsersDialog::~MultiUsersDialog()
{
    QJsonObject json;
    json.insert("sender", userName);
    json.insert("receiver", "");
    json.insert("contentType", "roomExit");
    json.insert("content", roomName);
    QJsonDocument document(json);
    clientSocket->write(document.toJson());
    emit closeMultiUsersDialog(roomName);
}

void MultiUsersDialog::sendText()
{
    QString text = textEdit->toPlainText().trimmed();
    QJsonObject json;
    json.insert("sender", userName);
    json.insert("receiver", roomName);
    json.insert("contentType", "roomText");
    json.insert("content", text);
    QJsonDocument document(json);
    QByteArray jsonData = document.toJson();
    clientSocket->write(jsonData);
    textEdit->clear();
}

void MultiUsersDialog::updateClientText(QString sender, QString receiver, QString roomName, QString text)
{
    if (receiver == "" && this->roomName == roomName)
    {
        contentList->addItem(QString::fromLocal8Bit("%1: %2").arg(sender, text));
    }
    else
    {
        return;
    }
}

void MultiUsersDialog::updateRoomInfo(QString newRoomName, QList<QString> allLiveUsersInRoom)
{
    if (roomName == newRoomName)
    {
        liveUsersList->clear();
        for (QList<QString>::iterator it = allLiveUsersInRoom.begin(); it != allLiveUsersInRoom.end(); ++it)
        {
            liveUsersList->addItem(*it);
        }
    }
}

void MultiUsersDialog::sendFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择发送文件"), ".", "");
    if (filePath == "")
    {
        return;
    }
    QString fileName = filePath.split("/").last();
    int fileSize;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("打开文件失败"));
        return;
    }
    fileSize = file.size();
    QByteArray content = file.readAll();
    file.close();
    QJsonObject json, fileInfo;
    json.insert("sender", userName);
    json.insert("receiver", roomName);
    json.insert("contentType", "roomFile");
    fileInfo.insert("fileName", fileName);
    fileInfo.insert("fileSize", fileSize);
    json.insert("content", fileInfo);
    QJsonDocument document(json);
    clientSocket->write(document.toJson());
    if (clientSocket->waitForBytesWritten());
    else
    {
        QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("发送文件失败"));
        return;
    }
    clientSocket->write(content);
}

void MultiUsersDialog::updateClientFile(QString sender, QString receiver, QString roomName, QString fileName)
{
    if (receiver == "" && roomName != "")
    {
        QPushButton *btn = new QPushButton(QString::fromLocal8Bit("接收"));
        QListWidgetItem *item = new QListWidgetItem;
        contentList->addItem(QString::fromLocal8Bit("%1发送了文件%2").arg(sender, fileName));
        contentList->addItem(item);
        contentList->setItemWidget(item, btn);
        fileBtns.insert(fileName, btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(requireFile()));
    }
}

void MultiUsersDialog::requireFile()
{
    QPushButton *btn = (QPushButton *)sender();
    QString fileName = "";
    for (QMap<QString, QPushButton *>::iterator it = fileBtns.begin(); it != fileBtns.end(); ++it)
    {
        if (it.value() == btn)
        {
            fileName = it.key();
        }
    }
    if (fileName == "")
    {
        QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("文件错误"));
        return;
    }
    QJsonObject json;
    json.insert("sender", roomName);
    json.insert("receiver", userName);
    json.insert("contentType", "requireRoomFile");
    json.insert("content", fileName);
    QJsonDocument document(json);
    clientSocket->write(document.toJson());
}

void MultiUsersDialog::updateClientLocalFile(QString sender, QString receiver, QString roomName, QString fileName)
{
    if (roomName == "")
    {
        QPushButton *btn = fileBtns.find(fileName).value();
        btn->setEnabled(false);
        contentList->addItem(QString::fromLocal8Bit("已成功接收%1").arg(fileName));
    }
}
