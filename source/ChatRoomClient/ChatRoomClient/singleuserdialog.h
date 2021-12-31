#ifndef SINGLEUSERDIALOG_H
#define SINGLEUSERDIALOG_H

#include <QDialog>
#include <QObject>
#include <QTcpSocket>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>
#include <QMovie>

class SingleUserDialog : public QDialog
{
    Q_OBJECT
public:
    SingleUserDialog(QString, QString, QTcpSocket*);
    ~SingleUserDialog();

public slots:
    void sendText();
    void updateClientText(QString, QString, QString, QString);
    void sendFile();
    void updateClientFile(QString, QString, QString, QString);
    void requireFile();
    void updateClientLocalFile(QString, QString, QString, QString);
    void sendImg();
    void updateClientLocalImg(QString, QString, QString, QString);

signals:
    void closeSingleUserDialog(QString);

private:
    QString userName;
    QString otherName;
    QTcpSocket *clientSocket;
    QMenuBar *menuBar;
    QMenu *fileMenu;
    QAction *sendFileAction;
    QAction *sendImgAction;
    QListWidget *contentList;
    QMap<QString, QPushButton *> fileBtns;
    QTextEdit *textEdit;
    QPushButton *sendBtn;
    QVBoxLayout *mainLayout;
};

#endif // SINGLEUSERDIALOG_H
