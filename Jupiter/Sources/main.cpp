#include <QApplication>
#include "MainWindow/JUMainWindow.h"

#include <QTime>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    srand((uint)QTime::currentTime().msec());

    JUMainWindow *w = new JUMainWindow;
    w->show();
    int result = a.exec();
    delete w;

    return result;
}
