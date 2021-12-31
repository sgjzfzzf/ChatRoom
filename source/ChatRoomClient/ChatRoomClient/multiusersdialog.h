#ifndef MULTIUSERSDIALOG_H
#define MULTIUSERSDIALOG_H

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

class MultiUsersDialog : public QDialog
{
    Q_OBJECT
public:
    MultiUsersDialog(QString, QString, QTcpSocket *);
    ~MultiUsersDialog();

public slots:
    void sendText();
    void updateClientText(QString, QString, QString, QString);
    void updateRoomInfo(QString, QList<QString>);
    void sendFile();
    void updateClientFile(QString, QString, QString, QString);
    void requireFile();
    void updateClientLocalFile(QString, QString, QString, QString);

signals:
    void closeMultiUsersDialog(QString);

private:
    QString userName;
    QString roomName;
    QTcpSocket *clientSocket;
    QMenuBar *menuBar;
    QMenu *fileMenu;
    QAction *sendFileAction;
    QAction *sendImgAction;
    QListWidget *liveUsersList;
    QListWidget *contentList;
    QMap<QString, QPushButton *> fileBtns;
    QTextEdit *textEdit;
    QPushButton *sendBtn;
    QVBoxLayout *mainLayout;
};

#endif // MULTIUSERSDIALOG_H
