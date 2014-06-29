
#include "JUConfigViewer.h"

#include <QBoxLayout>
#include <QLabel>

JUConfigViewer::JUConfigViewer(QString vhdl, QPixmap pixmap, QWidget *parent) : QDialog(parent)
{
    m_vhdl = vhdl;
    m_pixmap = pixmap;

    setModal(true);
    setFixedSize(700, 500);
    setWindowTitle("Configuration Viewer");

    m_tabWidget = new QTabWidget(this);

    QWidget *text = new QWidget(m_tabWidget);

    m_vhdlWidget = new QTextEdit(text);
    m_vhdlWidget->setReadOnly(true);

    m_highlighter = new JUSyntaxHighlighter(m_vhdlWidget->document());
    m_vhdlWidget->setPlainText(m_vhdl);

    QHBoxLayout *textContentLayout = new QHBoxLayout;
    textContentLayout->setContentsMargins(0, 0, 0, 0);
    textContentLayout->addWidget(m_vhdlWidget);
    text->setLayout(textContentLayout);
    
    QWidget *img = new QWidget(m_tabWidget);

    QLabel *lbl = new QLabel(img);
    lbl->setPixmap(pixmap);

    QHBoxLayout *imgContentLayout = new QHBoxLayout;
    imgContentLayout->setContentsMargins(0, 0, 0, 0);
    imgContentLayout->addWidget(lbl);
    img->setLayout(imgContentLayout);

    m_tabWidget->addTab(text, "VHDL");
    m_tabWidget->addTab(img, "Image");

    m_okBtn = new QPushButton("OK", this);
    connect(m_okBtn, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch();
    btnLayout->addWidget(m_okBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabWidget, 1);
    mainLayout->addLayout(btnLayout, 0);
    setLayout(mainLayout);
}

JUConfigViewer::~JUConfigViewer()
{
}