
#ifndef __JUMAINWINDOW_H__
#define __JUMAINWINDOW_H__

#include "Parser/JUEntity.h"

#include "JUSyntaxHighlighter.h"
#include "Reconfigurator/JUReconfigurator.h"

#include <QMainWindow>
#include <QWidget>
#include <QList>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSpinBox>

class JUMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    JUMainWindow(QWidget *parent = NULL);
    ~JUMainWindow();

private:
    QTextEdit *m_vhdl;
    QTextEdit *m_desc;
    QPushButton *m_analizeBtn;
    QPushButton *m_processBtn;
    QPushButton *m_loadFileBtn;
    QPushButton *m_doAllBtn;
    QListWidget *m_errorsWidget;
    QSpinBox *m_errorSpinBox;
    QSpinBox *m_reservedSpinBox;
    JUSyntaxHighlighter *m_highlighter;
    QString m_filepath;

    JUReconfigurator *m_reconfigurator;
    QList<JUEntity *> m_entities;
    QList<QList<JUSchemeError *>> m_errors;

    void fillDesc();
    void showErrorMsg(QString title, QString msg);

protected slots:
    void loadFileSlot();
    void analizeSlot();
    void processSlot();
    void doAll();
    void vhdlTextChanged();
    void itemActivatedSlot(QListWidgetItem *item);
};

#endif