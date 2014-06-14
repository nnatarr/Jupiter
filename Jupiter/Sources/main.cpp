#include <QApplication>
#include "MainWindow/JUMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    JUMainWindow *w = new JUMainWindow;
    w->show();
    int result = a.exec();
    delete w;

    return result;
}
