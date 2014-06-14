
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"

#include "Parser/JUSchemeTree.h"

#define __JUMODULE__ "MainWindow"

JUMainWindow::JUMainWindow(QWidget *parent) : QMainWindow(parent)
{
    JUMLog("ctor {%p}.");
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}.");
}