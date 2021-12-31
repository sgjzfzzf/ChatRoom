#include "singleuserdialog.h"

SingleUserDialog::SingleUserDialog(QString userName, QString otherName, QTcpSocket *clientSocket): userName(userName), otherName(otherName), clientSocket(clientSocket)
{
    setAttribute(Qt::WA_DeleteOnClose);

    menuBar = new QMenuBar;
    fileMenu = new QMenu(QString::fromLocal8Bit("文件"));
    sendFileAction = new QAction(QString::fromLocal8Bit("发送本地文件"));
    sendImgAction = new QAction(QString::fromLocal8Bit("发送本地表情"));
    contentList = new QListWidget;
    textEdit = new QTextEdit;
    sendBtn = new QPushButton(QString::fromLocal8Bit("发送"));
    mainLayout = new QVBoxLayout(this);

    menuBar->addMenu(fileMenu);
    fileMenu->addAction(sendFileAction);
    fileMenu->addAction(sendImgAction);
    contentList->setIconSize(QSize(200, 200));

    mainLayout->addWidget(menuBar);
    mainLayout->addWidget(contentList);
    mainLayout->addWidget(textEdit);
    mainLayout->addWidget(sendBtn);

    connect(sendBtn, SIGNAL(clicked()), this, SLOT(sendText()));
    connect(sendFileAction, SIGNAL(triggered()), this, SLOT(sendFile()));
    connect(sendImgAction, SIGNAL(triggered()), this, SLOT(sendImg()));
}

SingleUserDialog::~SingleUserDialog()
{
    emit closeSingleUserDialog(otherName);
}

void SingleUserDialog::sendText()
{
    QString text = textEdit->toPlainText().trimmed();
    QJsonObject json;
    json.insert("sender", userName);
    json.insert("receiver", otherName);
    json.insert("contentType", "text");
    json.insert("content", text);
    QJsonDocument document(json);
    QByteArray jsonData = document.toJson();
    clientSocket->write(jsonData);
    contentList->addItem(QString::fromLocal8Bit("%1: %2").arg(userName, text));
    textEdit->clear();
}

void SingleUserDialog::updateClientText(QString sender, QString receiver, QString roomName, QString text)
{
    if (sender == otherName && receiver == userName && roomName == "")
    {
        contentList->addItem(QString::fromLocal8Bit("%1: %2").arg(sender, text));
    }
    else
    {
        return;
    }
}

void SingleUserDialog::sendFile()
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
    json.insert("receiver", otherName);
    json.insert("contentType", "file");
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
    contentList->addItem(QString::fromLocal8Bit("%1向%2发送了文件%3").arg(userName, otherName, fileName));
}

void SingleUserDialog::updateClientFile(QString sender, QString receiver, QString roomName, QString fileName)
{
    if (receiver != "" && roomName == "")
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

void SingleUserDialog::requireFile()
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
    json.insert("sender", otherName);
    json.insert("receiver", userName);
    json.insert("contentType", "requireFile");
    json.insert("content", fileName);
    QJsonDocument document(json);
    clientSocket->write(document.toJson());
}

void SingleUserDialog::updateClientLocalFile(QString sender, QString receiver, QString roomName, QString fileName)
{
    if (receiver != "" && roomName == "")
    {
        QPushButton *btn = fileBtns.find(fileName).value();
        btn->setEnabled(false);
        contentList->addItem(QString::fromLocal8Bit("已成功接收%1").arg(fileName));
    }
}

void SingleUserDialog::sendImg()
{
    QDir dir;
    dir.currentPath();
    if (!dir.exists("img"))
    {
        dir.mkdir("img");
    }
    dir.cd("img");
    QString imgPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("本地表情发送"), dir.currentPath() + "/img", "*.png;;*.jpg;;*.jpeg;;*.gif");
    if (imgPath == "")
    {
        return;
    }
    QFile img(imgPath);
    img.open(QIODevice::ReadOnly);
    if (!img.isOpen())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("本地表情"), QString::fromLocal8Bit("获取本地资源失败"));
        return;
    }
    int imgSize = img.size();
    QByteArray content = img.readAll();
    img.close();
    QJsonObject json, imgInfo;
    json.insert("sender", userName);
    json.insert("receiver", otherName);
    json.insert("contentType", "img");
    imgInfo.insert("imgName", imgPath.split('/').last());
    imgInfo.insert("imgSize", imgSize);
    json.insert("content", imgInfo);
    QJsonDocument document;
    document.setObject(json);
    clientSocket->write(document.toJson());
    if (clientSocket->waitForBytesWritten());
    else
    {
        QMessageBox::information(this, QString::fromLocal8Bit("表情传输"), QString::fromLocal8Bit("发送错误"));
        return;
    }
    clientSocket->write(content);
}

void SingleUserDialog::updateClientLocalImg(QString sender, QString receiver, QString roomName, QString imgName)
{
    if (receiver == userName && roomName == "")
    {
        QDir dir(QDir::currentPath());
        if (!dir.exists("img"))
        {
            dir.mkdir("img");
        }
        imgName = QString("%1/%2").arg("img", imgName);
        QFile imgFile(imgName);
        if (!imgFile.open(QIODevice::ReadOnly))
        {
            return;
        }
        QByteArray contentByteArray = imgFile.readAll();
        if (imgName.split('.').last() == "gif")
        {
            QLabel *gifLabel = new QLabel;
            QMovie *gif = new QMovie(imgName);
            gif->setScaledSize(QSize(200, 200));
            if (!gif->isValid())
            {
                delete gifLabel;
                return;
            }
            gifLabel->setMovie(gif);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(200, 200));
            contentList->addItem(QString("%1 :").arg(userName));
            contentList->addItem(item);
            contentList->setItemWidget(item, gifLabel);
            gif->start();
        }
        else
        {
            QPixmap pixmap;
            pixmap.loadFromData(contentByteArray);
            QIcon img(pixmap);
            QListWidgetItem *item = new QListWidgetItem(img, "");
            contentList->addItem(QString("%1 :").arg(userName));
            contentList->addItem(item);
        }
    }
    else
    {
        return;
    }
}
