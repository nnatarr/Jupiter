
#include "JUMainWindow.h"
#include "Shared/Logger/JULogger.h"
#include "Parser/ParserVHDL/JUParserVHDL.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStringBuilder>
#include <QLabel>
#include <QMessageBox>

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
    m_analizeBtn = new QPushButton("Analyse", this);
    m_processBtn = new QPushButton("Generate", this);
    m_analizeBtn->setEnabled(false);
    m_processBtn->setEnabled(false);

    m_doAllBtn = new QPushButton("Process all errors", this);
    m_doAllBtn->setEnabled(false);

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
    connect(m_doAllBtn, SIGNAL(clicked()), this, SLOT(doAll()));

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

    QHBoxLayout *l = new QHBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    l->addStretch();
    l->addWidget(m_doAllBtn);

    mainLayout->addWidget(m_errorsWidget, 1);
    mainLayout->addLayout(l);

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
        m_doAllBtn->setEnabled(false);
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
    m_doAllBtn->setEnabled(false);

    if (m_entities.count() > 0) {
        for (int i = 0; i < m_entities.count(); ++i) {
            delete m_entities[i];
        }
        m_entities.clear();
    }

    JUParserVHDL *parser = new JUParserVHDL();
    m_entities = parser->parse(m_filepath);

    QString errorMsg = "";
    if (parser->isErrorSet()) {
        errorMsg = parser->errorMsg();
    }

    delete parser;

    if (errorMsg.length() > 0) {
        showErrorMsg("Parsing failed", errorMsg);
    }

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
    m_doAllBtn->setEnabled(m_reconfigurator->isValid());
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
    m_reconfigurator->setMaxErrorsCount(m_errorSpinBox->value());
    m_reconfigurator->setReservedElementsCount(m_reservedSpinBox->value());
    JUReconfigurator::ConfigurationResult result = m_reconfigurator->reconfigureForErrors(error);
    if (result.isValid) {
        JUConfigViewer *cv = new JUConfigViewer(result.vhdl, result.pixmap, this);
        cv->show();
    } else {
        showErrorMsg("Reconfiguration failed", m_reconfigurator->errorMsg());
    }
}

void JUMainWindow::doAll()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select directory for schemes storing");
    if (!dir.isEmpty()) {
        QFileInfo fi(dir);

        if (!fi.isWritable()) {
            showErrorMsg("Errors processing failed", QString("We don't have access to %1").arg(dir));
            return;
        }

        int succeeded = 0;
        for (int i = 0; i < m_errors.count(); ++i) {
            JUReconfigurator::ConfigurationResult res = m_reconfigurator->reconfigureForErrors(m_errors[i]);
            if (res.isValid) {
                succeeded++;
                QString dirName = dir % "/error_" % QString::number(i + 1);
                if (QDir().mkdir(dirName)) {
                    QFile descFile(dirName % "/description.txt");
                    descFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream s(&descFile);
                    s << m_errorsWidget->item(i)->text();
                    descFile.close();
                    QFile vhdlFile(dirName % "/scheme.vhdl");
                    vhdlFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream t(&vhdlFile);
                    t << res.vhdl;
                    vhdlFile.close();
                    res.pixmap.save(dirName % "/scheme.png");

                } else {
                    showErrorMsg("Scheme saving failed", QString("Can not create folder for error %1.").arg(i));
                }
            }
            m_desc->setPlainText(QString("Completed %1/%2.\nSucceeded %3").arg(i + 1).arg(m_errors.count()).arg(succeeded));
        }
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

void JUMainWindow::showErrorMsg(QString title, QString msg)
{
    QMessageBox *mbox = new QMessageBox(this);
    mbox->setIcon(QMessageBox::Critical);
    mbox->setWindowTitle(title);
    mbox->setText(msg);
    mbox->show();
}