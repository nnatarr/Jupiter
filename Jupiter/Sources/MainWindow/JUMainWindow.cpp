
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"

#include "Parser/ParserVHDL/JUParserVHDL.h"

#define __JUMODULE__ "MainWindow"

JUMainWindow::JUMainWindow(QWidget *parent) : QMainWindow(parent)
{
    JUMLog("ctor {%p}.");

    JUParserVHDL *parser = new JUParserVHDL();
    parser->parse("e:\\Dropbox\\!study\\dimploma\\Jupiter\\Jupiter\\Sources\\2band2_o2.vhdl");
    delete parser;
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}.");
}