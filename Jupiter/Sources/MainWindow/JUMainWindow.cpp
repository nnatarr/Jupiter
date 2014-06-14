
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"

#define __JUModule__ "MainWindow"

JUMainWindow::JUMainWindow(QWidget *parent) : QMainWindow(parent)
{
    JUMLog("ctor {%p}");
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}");
}