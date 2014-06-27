
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
    m_entities = parser->parse(appDir.path() + "/2band2_or2_2.vhdl");
    delete parser;

    if (m_entities.count() == 0) {
        JUMLog("no entities found.");
        return;
    }

    QList<JUEntity *> usedEntities;
    for (int i = 0; i < m_entities.count(); ++i) {
        JUEntity *e = m_entities[i];
        if (e->name() == "inverter" || e->name() == "2band2_or2") {
            continue;
        }
        QList<JUEntity *> components = e->components();
        for (int j = 0; j < components.count(); ++j) {
            if (!usedEntities.contains(components[j])) {
                usedEntities.append(components[j]);
            }
        }
    }

    JUEntity *mainEntity = NULL;
    for (int i = 0; i < m_entities.count(); ++i) {
        JUEntity *entity = m_entities[i];
        if (!usedEntities.contains(entity) && !(entity->name() == "inverter" || entity->name() == "2band2_or2")) {
            mainEntity = entity;
            break;
        }
    }

    if (!mainEntity) {
        JUMLog("cannot find main entity.");
        return;
    }

    bool isValid = mainEntity->validate();
    if (isValid) {
        JUMLog("main entity description:\n%s", Q(mainEntity->description()));
    } else {
        JUMLog("main entity can not be validated.");
    }
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}.");

    for (int i = 0; i < m_entities.count(); ++i) {
        delete m_entities[i];
    }

    m_entities.clear();
}