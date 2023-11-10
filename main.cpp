#include "QTQQ_server.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTQQ_Server w;
    w.show();
    return a.exec();
}
