
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"

#include "Parser/ParserVHDL/JUParserVHDL.h"
#include <QApplication>
#include <QDir>

#define __JUMODULE__ "MainWindow"

JUMainWindow::JUMainWindow(QWidget *parent) : QMainWindow(parent)
{
    JUMLog("ctor {%p}.");

    JUParserVHDL *parser = new JUParserVHDL();

    QDir appDir(qApp->applicationDirPath());
    appDir.cdUp();
    appDir.cdUp();
    appDir.cd("TestData");
    parser->parse(appDir.path() + "/2band2_or2.vhdl");
    delete parser;
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}.");
}