
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"

#include "Parser/ParserVHDL/JUParserVHDL.h"
#include "Reconfigurator/JUReconfigurator.h"

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
    m_entities = parser->parse(appDir.path() + "/2band2_or2_2.vhdl");
    delete parser;

    JUReconfigurator *reconfigurator = new JUReconfigurator(&m_entities, 5, 0);
    delete reconfigurator;
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}.");

    for (int i = 0; i < m_entities.count(); ++i) {
        delete m_entities[i];
    }

    m_entities.clear();
}