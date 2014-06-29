
#ifndef __JUCONFIGVIEWER_H__
#define __JUCONFIGVIEWER_H__

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include "JUSyntaxHighlighter.h"

#include <QString>
#include <QPixmap>

class JUConfigViewer : public QDialog
{
    Q_OBJECT

public:
    JUConfigViewer(QString vhdl, QPixmap pixmap, QWidget *parent = NULL);
    ~JUConfigViewer();

private:
    QString m_vhdl;
    QPixmap m_pixmap;

    QTextEdit *m_vhdlWidget;
    JUSyntaxHighlighter *m_highlighter;
    QPushButton *m_okBtn;
};

#endif