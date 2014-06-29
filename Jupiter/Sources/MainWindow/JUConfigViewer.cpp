
#include "JUConfigViewer.h"

#include <QBoxLayout>

JUConfigViewer::JUConfigViewer(QString vhdl, QPixmap pixmap, QWidget *parent) : QDialog(parent)
{
    m_vhdl = vhdl;
    m_pixmap = pixmap;

    setModal(true);
    setFixedSize(700, 500);
    setWindowTitle("Configuration Viewer");

    m_vhdlWidget = new QTextEdit(this);
    m_vhdlWidget->setReadOnly(true);
    
    m_highlighter = new JUSyntaxHighlighter(m_vhdlWidget->document());
    m_vhdlWidget->setPlainText(m_vhdl);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(m_vhdlWidget);

    m_okBtn = new QPushButton("OK", this);
    connect(m_okBtn, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch();
    btnLayout->addWidget(m_okBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(contentLayout, 1);
    mainLayout->addLayout(btnLayout, 0);
    setLayout(mainLayout);
}

JUConfigViewer::~JUConfigViewer()
{
}