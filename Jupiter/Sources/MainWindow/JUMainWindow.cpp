
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"
#include "Parser/ParserVHDL/JUParserVHDL.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStringBuilder>
#include <QLabel>

#include "JUConfigViewer.h"

#include <QList>

#define __JUMODULE__ "MainWindow"

JUMainWindow::JUMainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *cw = new QWidget(this);

    setMinimumSize(700, 200);
    setWindowTitle("Jupiter");

    QVBoxLayout *vhdlLayout = new QVBoxLayout;
    m_vhdl = new QTextEdit(this);
    m_vhdl->setReadOnly(true);
    m_loadFileBtn = new QPushButton("Load...", this);
    m_analizeBtn = new QPushButton("Analize", this);
    m_processBtn = new QPushButton("Generate", this);
    m_analizeBtn->setEnabled(false);
    m_processBtn->setEnabled(false);

    m_errorSpinBox = new QSpinBox(this);
    m_errorSpinBox->setMinimum(1);
    m_errorSpinBox->setMaximum(10);
    m_errorSpinBox->setValue(1);
    m_errorSpinBox->setEnabled(false);

    m_reservedSpinBox = new QSpinBox(this);
    m_reservedSpinBox->setMinimum(0);
    m_reservedSpinBox->setValue(0);
    m_reservedSpinBox->setMaximum(20);
    m_reservedSpinBox->setEnabled(false);

    connect(m_loadFileBtn, SIGNAL(clicked()), this, SLOT(loadFileSlot()));
    connect(m_analizeBtn, SIGNAL(clicked()), this, SLOT(analizeSlot()));
    connect(m_processBtn, SIGNAL(clicked()), this, SLOT(processSlot()));

    connect(m_vhdl, SIGNAL(textChanged()), this, SLOT(vhdlTextChanged()));

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addWidget(m_loadFileBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_analizeBtn);
    btnLayout->addWidget(new QLabel("Max errors count:", this));
    btnLayout->addWidget(m_errorSpinBox);
    btnLayout->addWidget(new QLabel("Reserved elements count:", this));
    btnLayout->addWidget(m_reservedSpinBox);
    btnLayout->addWidget(m_processBtn);

    vhdlLayout->setContentsMargins(0, 0, 0, 0);
    vhdlLayout->addWidget(m_vhdl);
    vhdlLayout->addLayout(btnLayout);

    QHBoxLayout *descLayout = new QHBoxLayout;
    m_desc = new QTextEdit(this);
    m_desc->setReadOnly(true);

    descLayout->addLayout(vhdlLayout, 2);
    descLayout->addWidget(m_desc, 1);

    m_highlighter = new JUSyntaxHighlighter(m_vhdl->document());

    m_errorsWidget = new QListWidget(this);
    connect(m_errorsWidget, &QListWidget::itemActivated, this, &JUMainWindow::itemActivatedSlot);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(descLayout, 2);
    mainLayout->addWidget(m_errorsWidget, 1);

    cw->setLayout(mainLayout);
    setCentralWidget(cw);

    m_reconfigurator = NULL;
}

JUMainWindow::~JUMainWindow()
{
    JUMLog("dtor {%p}.");

    for (int i = 0; i < m_entities.count(); ++i) {
        delete m_entities[i];
    }

    m_entities.clear();

    if (m_reconfigurator != NULL) {
        delete m_reconfigurator;
    }
}

// ========================================

void JUMainWindow::loadFileSlot()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open scheme VHDL description", QString(), "VHDL file (*.vhdl)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QTextStream inputStream(&file);
        m_vhdl->setText(inputStream.readAll());

        m_filepath = filename;

        m_desc->clear();
        m_errorsWidget->clear();
        m_errors.clear();

        m_processBtn->setEnabled(false);
        m_errorSpinBox->setEnabled(false);
        m_reservedSpinBox->setEnabled(false);
    }
}

void JUMainWindow::vhdlTextChanged()
{
    m_analizeBtn->setEnabled(!m_vhdl->toPlainText().isEmpty());
}

void JUMainWindow::analizeSlot()
{
    m_loadFileBtn->setEnabled(false);
    m_analizeBtn->setEnabled(false);
    m_processBtn->setEnabled(false);
    m_errorSpinBox->setEnabled(false);
    m_reservedSpinBox->setEnabled(false);

    if (m_entities.count() > 0) {
        for (int i = 0; i < m_entities.count(); ++i) {
            delete m_entities[i];
        }
        m_entities.clear();
    }

    JUParserVHDL *parser = new JUParserVHDL();
    m_entities = parser->parse(m_filepath);
    delete parser;

    if (m_reconfigurator) {
        delete m_reconfigurator;
        m_reconfigurator = NULL;
    }
    m_reconfigurator = new JUReconfigurator(&m_entities, m_errorSpinBox->value(), m_reservedSpinBox->value());
    fillDesc();

    m_loadFileBtn->setEnabled(true);
    m_analizeBtn->setEnabled(true);
    m_processBtn->setEnabled(m_reconfigurator->isValid());
    m_errorSpinBox->setEnabled(m_reconfigurator->isValid());
    m_reservedSpinBox->setEnabled(m_reconfigurator->isValid());
}

void JUMainWindow::processSlot()
{
    if (m_reconfigurator == NULL || !m_reconfigurator->isValid()) {
        return;
    }

    m_errorsWidget->clear();
    m_reconfigurator->setMaxErrorsCount(m_errorSpinBox->value());
    m_reconfigurator->setReservedElementsCount(m_reservedSpinBox->value());
    m_errors = m_reconfigurator->generateErrors();
    for (int i = 0; i < m_errors.count(); ++i) {
        QList<JUSchemeError *> errorCase = m_errors[i];
        QString desc = "";
        for (int j = 0; j < errorCase.count(); ++j) {
            if (j > 0) {
                desc = desc % ", ";
            }
            desc = desc % errorCase[j]->description();
        }

        m_errorsWidget->addItem(desc);
    }
}

void JUMainWindow::itemActivatedSlot(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    int index = m_errorsWidget->row(item);
    QList<JUSchemeError *> error = m_errors[index];
    JUReconfigurator::ConfigurationResult result = m_reconfigurator->reconfigureForErrors(error);
    if (result.isValid) {
        JUConfigViewer *cv = new JUConfigViewer(result.vhdl, result.pixmap, this);
        cv->show();
    }
}

// ========================================

void JUMainWindow::fillDesc()
{
    if (m_reconfigurator == NULL || !m_reconfigurator->isValid()) {
        return;
    }

    JUEntity *mainEntity = m_reconfigurator->mainEntity();
    m_desc->setPlainText(mainEntity->description());
}