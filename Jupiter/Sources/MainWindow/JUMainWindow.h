
#ifndef __JUMAINWINDOW_H__
#define __JUMAINWINDOW_H__

#include <QMainWindow>
#include <QWidget>
#include <QList>

#include "Parser/JUEntity.h"

class JUMainWindow : public QMainWindow
{
public:
    JUMainWindow(QWidget *parent = NULL);
    ~JUMainWindow();

private:
    QList<JUEntity *> m_entities;
};

#endif