#include "chatroomserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatRoomServer w;
    w.show();
    return a.exec();
}
