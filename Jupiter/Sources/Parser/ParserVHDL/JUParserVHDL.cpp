
#include "JUParserVHDL.h"
#include "Shared/Logger/JULogger.h"
#include "Shared/JUAssert.h"

#include <QFile>
#include <QTextStream>
#include <QStringBuilder>

#define CHECK_ERROR { if (m_state == JUParserVHDL::ParserStateError) { return; } }

#define CHECK_UEOF { if (isEOF()) { setError(ParserError_UnexpectedEOF); return; } }

#define SKIP_SPACES {\
                        while (!isEOF() && m_currentChar.isSpace()) { \
                            readNext(); \
                        } \
                    }

#define SKIP_SPACES_WITH_CHECK { \
                                   SKIP_SPACES;\
                                   CHECK_UEOF;\
                               }

#define __JUMODULE__ "ParserVHDL"

JUParserVHDL::JUParserVHDL() : JUParser()
{
    JUMLog("ctor {%p}.", this);

    m_state = JUParserVHDL::ParserStateNone;
}

JUParserVHDL::~JUParserVHDL()
{
    JUMLog("dtor {%p}.", this);

    //for (int i = 0; i < m_entities.count(); ++i) {
    //    if (m_entities[i] != m_ubsEntity && m_entities[i] != m_inverter) { 
    //        delete m_entities[i];
    //    }
    //}
    //m_entities.clear();

    //delete m_ubsEntity;
    //delete m_inverter;
}

// ========================================

QList<JUEntity *> JUParserVHDL::parse(const QString& filePath)
{
    JUMLog("parse.");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        JUMLog("can't load file %s.", Q(filePath));
        return QList<JUEntity *>();
    }

    QTextStream inputStream(&file);
    m_fileContent = inputStream.readAll();
    
    if (m_fileContent.length() == 0) {
        return QList<JUEntity *>();
    }

    m_caretPos = 0;
    m_currentPos.line = 0;
    m_currentPos.column = 0;
    while (canReadNext() && m_state != JUParserVHDL::ParserStateError) {
        QChar next = 0;
        readNext();
        if (m_currentChar == QChar(' ') || m_currentChar == QChar('\t')) {
            continue;
        }
        if (m_currentChar == QChar('-')) {
            if (m_state == JUParserVHDL::ParserStateNone) {
                m_state = JUParserVHDL::ParserStateComment;
            } else if (m_state == JUParserVHDL::ParserStateComment || m_state == JUParserVHDL::ParserStateCommentMultiLine) {
                //skip
            } else {
                setError(ParserError_ExpectedKeyword, "entity");
            }
        } else if (m_currentChar == QChar('/')) {
            if (m_state == JUParserVHDL::ParserStateNone) {
                readNext();
                if (m_currentChar == QChar('*')) {
                    m_state = JUParserVHDL::ParserStateCommentMultiLine;
                } else {
                    setError(JUParserVHDL::ParserError_UnknownSequence);
                }
            } else if (m_state == JUParserVHDL::ParserStateComment || m_state == JUParserVHDL::ParserStateCommentMultiLine) {
                //skip
            } else {
                setError(ParserError_ExpectedKeyword, "entity");
            }
        } else if (m_currentChar == QChar('*')) {
            if (m_state == JUParserVHDL::ParserStateCommentMultiLine) {
                readNext();
                if (m_currentChar == QChar('/')) {
                    m_state = JUParserVHDL::ParserStateNone;
                }
            } else if (m_state == JUParserVHDL::ParserStateComment) {
                //skip
            } else {
                setError(JUParserVHDL::ParserError_UnknownSequence);
            }
        } else if (m_currentChar == QChar('\n')) {
            if (m_state == JUParserVHDL::ParserStateComment) {
                m_state = JUParserVHDL::ParserStateNone;
            }
        } else {
            if (m_state == JUParserVHDL::ParserStateComment || m_state == JUParserVHDL::ParserStateCommentMultiLine) {
                //skip
            } else if (m_state == JUParserVHDL::ParserStateNone) {
                if (m_currentChar == QChar('e')) {
                    readEntity();
                } else if (m_currentChar == QChar('a')) {
                    readArchitecture();
                }
            } else {
                JUAssert(false, "how is that even possible?");
            }
        }
    }
    
    if (m_state != JUParserVHDL::ParserStateNone && m_state != JUParserVHDL::ParserStateError) {
        setError(JUParserVHDL::ParserError_UnexpectedEOF);
        JUMLog("parser state is %d.", m_state);
    }

    if (m_state == JUParserVHDL::ParserStateError) {
        m_fileContent.clear();
        return QList<JUEntity *>();
    }

    /*for (int i = 0; i < m_entities.count(); ++i) {
        JUMLog("%s\n", Q(m_entities[i]->description()));
    }*/

    JUMLog("VHDL parsed successful.");
    return m_entities;
}

// ========================================

void JUParserVHDL::readEntity()
{
    if (!readKeyword("entity")) {
        return;
    }

    m_state = JUParserVHDL::ParserStateEntity;

    QString entityName = readIdentifier();
    CHECK_ERROR;
    JUMLog("entity name is \"%s\".", Q(entityName));

    JUEntity *entity = new JUEntity(entityName);

    if (!readKeyword("is")) {
        return;
    }

    readEntityHeader(entity);
    m_entities.append(entity);
    CHECK_ERROR;

    if (!readKeyword("end")) {
        return;
    }

    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != QChar(';')) {
        FilePos pos = m_currentPos;
        QString lit = readIdentifier();
        CHECK_ERROR;
        if (lit != QString("entity") && lit != entityName) {
            setErrorAtPosition(ParserError_UnknownSequence, pos);
            return;
        }

        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != QChar(';')) {
            FilePos pos2 = m_currentPos;
            QString lit2 = readIdentifier();
            CHECK_ERROR;
            if (lit2 != QString("entity") && lit2 != entityName) {
                setErrorAtPosition(ParserError_UnknownSequence, pos2);
                return;
            } else {
                if (!(lit == QString("entity") && lit2 == entityName)) {
                    setErrorAtPosition(ParserError_UnknownSequence, pos);
                    return;
                }
            }
        } else {
            readNext();
        }
    } else {
        readNext();
    }

    m_state = JUParserVHDL::ParserStateNone;
}

void JUParserVHDL::readEntityHeader(JUEntity *entity)
{
    CHECK_ERROR;
    while (!isEOF() && m_currentChar.isSpace()) {
        readNext();
    }
    if (isEOF()) {
        setError(ParserError_UnexpectedEOF);
        return;
    }
    if (m_currentChar == QChar('p')) {
        readPort(entity);
    }
}

void JUParserVHDL::readPort(JUEntity *entity)
{
    CHECK_ERROR;

    SKIP_SPACES_WITH_CHECK;

    if (!readKeyword("port")) {
        return;
    }

    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != QChar('(')) {
        setError(ParserError_ExpectedSymbol, "(");
        return;
    }

    readNext();
    readInterfaceList(entity);

    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != QChar(')')) {
        setError(ParserError_ExpectedSymbol, ")");
        return;
    }

    readNext();
    if (m_currentChar != QChar(';')) {
        setError(ParserError_ExpectedSymbol, ";");
        return;
    }

    readNext();
}

void JUParserVHDL::readInterfaceList(JUEntity *entity)
{
    SKIP_SPACES_WITH_CHECK;

    while (!isEOF() && m_currentChar != QChar(')')) {
        QList<QString> ports;
        QString mode;
        QString type;
        while (!isEOF() && m_currentChar != QChar(':') && m_currentChar != QChar(')')) {
            QString portName = readIdentifier();
            CHECK_ERROR;
            //JUMLog("read port -> %s", Q(portName));
            if (!ports.contains(portName)) {
                ports.append(portName);
            } else {
                FilePos pos = m_currentPos;
                pos.column -= portName.length();
                setErrorAtPosition(ParserError_IdentifierDuplicate, pos);
                return;
            }
            SKIP_SPACES_WITH_CHECK;
            if (m_currentChar != QChar(',') && m_currentChar != QChar(':')) {
                setError(ParserError_ExpectedSymbol, ":");
                return;
            }
            SKIP_SPACES_WITH_CHECK;
            if (m_currentChar == QChar(',')) {
                readNext();
            }
        }
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != QChar(':')) {
            setError(ParserError_ExpectedSymbol, ":");
            return;
        }
        readNext();
        CHECK_UEOF;
        mode = readIdentifier();
        //JUMLog("read mode -> %s", Q(mode));
        CHECK_ERROR;
        SKIP_SPACES_WITH_CHECK;
        while (!isEOF() && m_currentChar != QChar(';')/* && m_currentChar != QChar(')')*/) {
            if (m_currentChar == ')') {
                if (!(type.toLower().contains("std_ulogic_vector") || type.toLower().contains("std_logic_vector"))) {
                    break;
                }
            }
            type = type % m_currentChar;
            readNext();
            SKIP_SPACES_WITH_CHECK;
        }
        //JUMLog("read type -> %s", Q(type));
        if (m_currentChar == QChar(';') || m_currentChar == QChar(')')) {
            for (int i = 0; i < ports.count(); ++i) {
                if (entity) {
                    entity->addPort(ports[i], mode, type);
                }
            }
        } else {
            setError(ParserError_ExpectedSymbol, ")");
            return;
        }
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar == QChar(';')) {
            readNext();
        }
        SKIP_SPACES_WITH_CHECK;
    }
}

void JUParserVHDL::readArchitecture()
{
    if (!readKeyword("architecture")) {
        return;
    }

    m_state = JUParserVHDL::ParserStateArchitecture;

    QString architectureName = readIdentifier();
    CHECK_ERROR;

    if (!readKeyword("of")) {
        return;
    }

    FilePos pos = m_currentPos;
    QString entityName = readIdentifier();
    CHECK_ERROR;
    if (!hasEntityWithName(entityName)) {
        setErrorAtPosition(ParserError_UnknownEntity, pos);
        return;
    }

    if (!readKeyword("is")) {
        return;
    }

    JUEntity *e = entityByName(entityName);
    readArchitectureDeclarativePart(e);
    CHECK_ERROR;

    JUMLog("architecture name for entity \"%s\" is \"%s.\"", Q(entityName), Q(architectureName));

    if (!readKeyword("begin")) {
        return;
    }

    readArchitectureStatementPart(e);
    CHECK_ERROR;

    if (!readKeyword("end")) {
        return;
    }

    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != QChar(';')) {
        pos = m_currentPos;
        QString lit = readIdentifier();
        CHECK_ERROR;
        if (lit != QString("architecture") && lit != architectureName) {
            setErrorAtPosition(ParserError_UnknownSequence, pos);
            return;
        }

        SKIP_SPACES;
        if (m_currentChar != QChar(';')) {
            FilePos pos2 = m_currentPos;
            QString lit2 = readIdentifier();
            CHECK_ERROR;
            if (lit2 != QString("architecture") && lit2 != architectureName) {
                setErrorAtPosition(ParserError_UnknownSequence, pos2);
                return;
            } else {
                if (!(lit == QString("architecture") && lit2 == architectureName)) {
                    setErrorAtPosition(ParserError_UnknownSequence, pos);
                    return;
                }
            }
        } else {
            readNext();
        }
    } else {
        readNext();
    }

    e->validate();
    m_state = JUParserVHDL::ParserStateNone;
}

void JUParserVHDL::readArchitectureDeclarativePart(JUEntity *e)
{
    SKIP_SPACES_WITH_CHECK;
    while (!isEOF() && m_currentChar != QChar('b') && m_state != ParserStateError) {
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar == QChar('c')) {
            readComponent(e);
        } else if (m_currentChar == QChar('s')) {
            readSignal(e);
        } else if (m_currentChar == QChar('b')) {
            //assume that this is 'begin' keyword
        } else {
            setError(ParserError_UnknownSequence);
        }
    }
}

void JUParserVHDL::readComponent(JUEntity *e)
{
    if (!readKeyword("component")) {
        return;
    }

    FilePos pos = m_currentPos;
    QString componentName = readIdentifier();
    CHECK_ERROR;
    if (!hasEntityWithName(componentName)/* && componentName != QString("2band2_or2")*/) {
        setErrorAtPosition(ParserError_UnknownEntity, pos);
        return;
    }

    if (m_currentChar == QChar('i')) {
        if (!readKeyword("is")) {
            return;
        }
    }

    readPort(NULL);

    if (!readKeyword("end")) {
        return;
    }

    if (!readKeyword("component")) {
        return;
    }

    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != QChar(';')) {
        pos = m_currentPos;
        QString name = readIdentifier();
        CHECK_ERROR;
        if (name != componentName) {
            setErrorAtPosition(ParserError_UnknownSequence, pos);
            return;
        }

        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != QChar(';')) {
            setError(ParserError_ExpectedSymbol, ";");
            return;
        } else {
            readNext();
        }
    } else {
        readNext();
    }

    //JUMLog("storing component %s.", Q(componentName));
    JUEntity *en = entityByName(componentName);
    JUAssert(en != NULL, "entity not found");
    e->addComponent(en);
}

void JUParserVHDL::readSignal(JUEntity *e)
{
    SKIP_SPACES_WITH_CHECK;
    if (!readKeyword("signal")) {
        return;
    }

    QList<QString> ports;
    QString type;
    while (!isEOF() && m_currentChar != QChar(':') && m_currentChar != QChar(';')) {
        QString portName = readIdentifier();
        CHECK_ERROR;
        //JUMLog("read port -> %s", Q(portName));
        if (!ports.contains(portName)) {
            ports.append(portName);
        } else {
            FilePos pos = m_currentPos;
            pos.column -= portName.length();
            setErrorAtPosition(ParserError_IdentifierDuplicate, pos);
            return;
        }
        if (m_currentChar != QChar(',') && m_currentChar != QChar(':')) {
            setError(ParserError_ExpectedSymbol, ":");
            return;
        }
        if (m_currentChar == QChar(',')) {
            readNext();
        }
    }
    CHECK_UEOF;
    if (m_currentChar != QChar(':')) {
        setError(ParserError_ExpectedSymbol, ":");
        return;
    }
    readNext();
    SKIP_SPACES_WITH_CHECK;
    while (!isEOF() && m_currentChar != QChar(';')) {
        type = type % m_currentChar;
        readNext();
    }
    //JUMLog("read type -> %s", Q(type));
    if (m_currentChar == QChar(';')) {
        //success, save ports
        for (int i = 0; i < ports.count(); ++i) {
            e->addDeclaredSignal(JUEntity::createPort(ports[i], "", type));
        }
        readNext();
    } else {
        setError(ParserError_ExpectedSymbol, ";");
        return;
    }
}

void JUParserVHDL::readArchitectureStatementPart(JUEntity *e)
{
    SKIP_SPACES;
    bool process = true;
    while (!isEOF()) {
        SKIP_SPACES;
        if (m_currentChar == QChar('e')) {
            int tmpCaretPos = m_caretPos;
            FilePos tmpFilePos = m_currentPos;
            QString lit = readIdentifier();
            CHECK_ERROR;
            if (lit == QString("end")) {
                m_currentPos = tmpFilePos;
                m_caretPos = tmpCaretPos;
                m_currentChar = m_fileContent[m_caretPos - 1];
                return;
            }
        } else if (m_currentChar == QChar('p')) {
            int tmpCaretPos = m_caretPos;
            FilePos tmpFilePos = m_currentPos;
            QString lit = readIdentifier();
            CHECK_ERROR;
            if (lit == QString("process")) {
                m_currentPos = tmpFilePos;
                m_caretPos = tmpCaretPos;
                m_currentChar = m_fileContent[m_caretPos - 1];
                readProcessStatement(e);
                CHECK_ERROR;
            } else {
                m_currentPos = tmpFilePos;
                m_caretPos = tmpCaretPos;
                m_currentChar = m_fileContent[m_caretPos - 1];
                readInstantiationStatement(e);
                CHECK_ERROR;
            }
        } else {
            readInstantiationStatement(e);
            CHECK_ERROR;
        }
    }
}

void JUParserVHDL::readProcessStatement(JUEntity *e)
{
    SKIP_SPACES;
    if (!readKeyword("process")) {
        return;
    }

    SKIP_SPACES;
    bool waitCloseBracket = false;
    if (m_currentChar == '(') {
        waitCloseBracket = true;
    }
    readNext();

    QString signal_name = readIdentifier();
    CHECK_ERROR;

    SKIP_SPACES;
    if (waitCloseBracket && m_currentChar != ')') {
        setError(ParserError_ExpectedSymbol, ")");
        return;
    }
    readNext();

    if (!readKeyword("is")) {
        return;
    }

    if (!readKeyword("begin")) {
        return;
    }

    if (!readKeyword("case")) {
        return;
    }

    FilePos tmpPos = m_currentPos;
    QString signal_case = readIdentifier();
    CHECK_ERROR;
    if (signal_case != signal_name) {
        setErrorAtPosition(ParserError_ExpectedKeyword, tmpPos, signal_name);
        return;
    }

    if (!readKeyword("of")) {
        return;
    }

    SKIP_SPACES_WITH_CHECK;
    QMap<QString, QString> lut;
    QString lit = readIdentifier();
    FilePos pos = m_currentPos;
    QChar cc = m_currentChar;
    int caretPos = m_caretPos;
    while (m_state != JUParserVHDL::ParserStateError && lit == "when") {
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != '"') {
            setError(ParserError_ExpectedSymbol, "\"");
            return;
        }
        readNext();

        QString input = readIdentifier();
        CHECK_ERROR;
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != '"') {
            setError(ParserError_ExpectedSymbol, "\"");
            return;
        }
        readNext();

        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != '=') {
            setError(ParserError_ExpectedSymbol, "=>");
            return;
        }
        readNext();
        CHECK_UEOF;
        if (m_currentChar != '>') {
            setError(ParserError_ExpectedSymbol, "=>");
            return;
        }
        readNext();
        SKIP_SPACES_WITH_CHECK;

        pos = m_currentPos;
        cc = m_currentChar;
        caretPos = m_caretPos;
        QString output_name = readIdentifier();
        CHECK_ERROR;

        if (!e->hasOutputPort(output_name)) {
            m_currentPos = pos;
            m_currentChar = cc;
            m_caretPos = caretPos;
            setError(ParserError_UnknownOutputPort);
            return;
        }

        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != '<') {
            setError(ParserError_ExpectedSymbol, "<=");
            return;
        }
        readNext();
        CHECK_UEOF;
        if (m_currentChar != '=') {
            setError(ParserError_ExpectedSymbol, "<=");
            return;
        }
        readNext();
        SKIP_SPACES_WITH_CHECK;

        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != '"') {
            setError(ParserError_ExpectedSymbol, "\"");
            return;
        }
        readNext();

        QString output = readIdentifier();
        CHECK_ERROR;
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != '"') {
            setError(ParserError_ExpectedSymbol, "\"");
            return;
        }
        readNext();

        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != ';') {
            setError(ParserError_ExpectedSymbol, ";");
            return;
        } 
        
        readNext();

        lut[input] = output;

        pos = m_currentPos;
        cc = m_currentChar;
        caretPos = m_caretPos;
        lit = readIdentifier();
    }

    CHECK_ERROR;
    if (lit != "end") {
        m_currentPos = pos;
        m_currentChar = cc;
        m_caretPos = caretPos;
        setError(ParserError_ExpectedKeyword, "end");
        return;
    }
    
    if (!readKeyword("case")) {
        return;
    }

    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != ';') {
        setError(ParserError_ExpectedSymbol, ";");
        return;
    }
    readNext();

    if (!readKeyword("end")) {
        return;
    }

    if (!readKeyword("process")) {
        return;
    }

    if (m_currentChar == ';') {
        e->setType(JUEntity::EntityTypeLUT);
        if (!e->setLUT(lut)) {
            setError(ParserError_IncorrectLUTDescription);
            return;
        }
        readNext();
    } else {
        setError(ParserError_ExpectedSymbol, ";");
    }
}

void JUParserVHDL::readInstantiationStatement(JUEntity *e)
{
    QString componentName = "";
    SKIP_SPACES;
    FilePos pos = m_currentPos;
    QString lit = readIdentifier();
    CHECK_ERROR;
    if (m_currentChar == QChar(':')) {
        readNext();
        SKIP_SPACES;
        pos = m_currentPos;
        componentName = readIdentifier();
        CHECK_ERROR;
    } else {
        componentName = lit;
        lit = "";
    }

    if (!entityUseComponent(e, componentName)) {
        setErrorAtPosition(ParserError_UndescribedEntityComponent, pos);
        return;
    }

    if (!readKeyword("port")) {
        return;
    }

    if (!readKeyword("map")) {
        return;
    }

    SKIP_SPACES;
    if (m_currentChar != QChar('(')) {
        setError(ParserError_ExpectedSymbol, "(");
        return;
    }
    readNext();

    QList<QString> signal;
    while (!isEOF() && m_currentChar != QChar(')')) {
        QString signalName = readIdentifier();
        CHECK_ERROR;
        //JUMLog("read signal -> %s", Q(signalName));
        signal.append(signalName);
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar != QChar(',') && m_currentChar != QChar(')')) {
            setError(ParserError_ExpectedSymbol, ")");
            return;
        }
        if (m_currentChar == QChar(',')) {
            readNext();
        }
    }
    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar != QChar(')')) {
        setError(ParserError_ExpectedSymbol, ")");
        return;
    }
    readNext();
    SKIP_SPACES_WITH_CHECK;
    if (m_currentChar == QChar(';')) {
        readNext();
    } else {
        setError(ParserError_ExpectedSymbol, ";");
        return;
    }

    e->addMappedSignals(componentName, signal, lit);
    e->setType(JUEntity::EntityTypeUBS);
}

QString JUParserVHDL::readIdentifier()
{
    QString identifier;
    //JUMLog("start: ident is %s", Q(identifier));
    while (canReadNext() && m_currentChar.isSpace()) {
        readNext();
    }
    while (canReadNext() && isCharAcceptableInIdentifier(m_currentChar)) {
        identifier = identifier % m_currentChar;
        //JUMLog("loop: ident is %s", Q(identifier));
        readNext();
    }

    //JUMLog("read identifier -> %s", Q(identifier));
    return identifier.toLower();

}

bool JUParserVHDL::readKeyword(QString keyword)
{
    SKIP_SPACES;
    FilePos pos = m_currentPos;
    QString lit = readIdentifier();
    bool success = m_state != ParserStateError && lit == keyword;
    if (!success) {
        setErrorAtPosition(ParserError_ExpectedKeyword, pos, keyword);
    }
    return success;
}

// ========================================

bool JUParserVHDL::canReadNext()
{
    return m_caretPos < m_fileContent.length();
}

bool JUParserVHDL::isEOF()
{
    return m_caretPos >= m_fileContent.length();
}

bool JUParserVHDL::readNext()
{
    if (canReadNext()) {
        m_currentChar = m_fileContent[m_caretPos++].toLower();
        m_currentPos.column++;
        if (m_currentChar == '\n') {
            m_currentPos.line++;
            m_currentPos.column = -1;
        }
        //JUMLog("read char -> %c (at line %d, col %d).", m_currentChar, m_currentPos.line, m_currentPos.column);
        return true;
    }
    return false;
}

void JUParserVHDL::setError(JUParserVHDL::ParserError error, const QString& keyword)
{
    setErrorAtPosition(error, m_currentPos, keyword);
}

void JUParserVHDL::setErrorAtPosition(JUParserVHDL::ParserError error, FilePos pos, const QString& keyword)
{
    m_state = JUParserVHDL::ParserStateError;

    QString description = "unknown error";
    switch (error) {
    case JUParserVHDL::ParserError_ExpectedSymbol:
        description = QString("waiting for '%1'").arg(keyword);
        break;
    case JUParserVHDL::ParserError_ExpectedKeyword:
        description = QString("waiting for '%1'").arg(keyword);
        break;
    case JUParserVHDL::ParserError_UnknownSequence:
        description = "unknown sequence";
        break;
    case JUParserVHDL::ParserError_UnexpectedEOF:
        description = "unexpected end of file";
        break;
    case JUParserVHDL::ParserError_WrongSymbol:
        description = "wrong symbol";
        break;
    case JUParserVHDL::ParserError_UnknownEntity:
        description = "unknown entity name";
        break;
    case JUParserVHDL::ParserError_UndescribedEntityComponent:
        description = "undescribed entity component";
        break;
    case JUParserVHDL::ParserError_IncorrectLUTDescription:
        description = "incorrect look-up table description";
        break;
    case JUParserVHDL::ParserError_UnknownOutputPort:
        description = "unknown output port";
        break;
    default:
        description = "unknown error";
        JUAssert(false, "not impl.");
    }

    QString errorStr = QString("parse error: line %d, column %d => %1.").arg(description);
    JUMLog(Q(errorStr), pos.line + 1, pos.column + 1);
}

bool JUParserVHDL::isCharAcceptableInIdentifier(const QChar& c)
{
    return !c.isSpace() && (c.isLetterOrNumber() || c == QChar('_'));
}

JUEntity* JUParserVHDL::entityByName(QString name)
{
    JUEntity *e = NULL;
    for (int i = 0; i < m_entities.count(); ++i) {
        if (m_entities[i]->name() == name) {
            e = m_entities[i];
            break;
        }
    }
    if (name == QString("inverter")) {
        e = new JUEntity("inverter");
        m_entities.append(e);
    }
    if (name == QString("2band2_or2")) {
        e = new JUEntity("2band2_or2");
        m_entities.append(e);
    }
    return e;
}

bool JUParserVHDL::hasEntityWithName(QString name)
{
    for (int i = 0; i < m_entities.count(); ++i) {
        if (m_entities[i]->name() == name) {
            return true;
        }
    }
    if (name == QString("inverter") || name == QString("2band2_or2")) {
        m_entities.append(new JUEntity(name));
        return true;
    }
    return false;
}

bool JUParserVHDL::entityUseComponent(JUEntity *e, QString name)
{
    QList<JUEntity *> components = e->components();
    for (int i = 0; i < components.count(); ++i) {
        if (components[i]->name() == name) {
            return true;
        }
    }
    if (name == QString("inverter") || name == QString("2band2_or2")) {
        JUEntity *inverter = new JUEntity(name);
        m_entities.append(inverter);
        e->addComponent(inverter);
        return true;

    }
    return false;
}