
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
                                   CHECK_ERROR;\
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
}

// ========================================

JUSchemeTree* JUParserVHDL::parse(const QString& filePath)
{
    JUMLog("parse.");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        JUMLog("can't load file %s.", Q(filePath));
        return NULL;
    }

    QTextStream inputStream(&file);
    m_fileContent = inputStream.readAll();
    
    if (m_fileContent.length() == 0) {
        return NULL;
    }

    JUSchemeTree *tree = new JUSchemeTree;

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
                    if (m_entities.count() > 0) {
                        JUMLog("added:\n%s", Q(m_entities.last().description()));
                    }
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
        delete tree;
        m_fileContent.clear();
        return NULL;
    }

    JUMLog("VHDL parsed successful.");
    return tree;
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
    JUMLog("entity name is %s.", Q(entityName));

    JUEntity entity(entityName);

    if (!readKeyword("is")) {
        return;
    }

    readEntityHeader(entity);
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

    m_entities.append(entity);

    m_state = JUParserVHDL::ParserStateNone;
}

void JUParserVHDL::readEntityHeader(JUEntity &entity)
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

void JUParserVHDL::readPort(JUEntity &entity)
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

void JUParserVHDL::readInterfaceList(JUEntity &entity)
{
    SKIP_SPACES_WITH_CHECK;

    while (!isEOF() && m_currentChar != QChar(')')) {
        QList<QString> ports;
        QString mode;
        QString type;
        while (!isEOF() && m_currentChar != QChar(':') && m_currentChar != QChar(')')) {
            QString portName = readIdentifier();
            CHECK_ERROR;
            JUMLog("read port -> %s", Q(portName));
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
        CHECK_UEOF;
        mode = readIdentifier();
        JUMLog("read mode -> %s", Q(mode));
        CHECK_ERROR;
        SKIP_SPACES_WITH_CHECK;
        while (!isEOF() && m_currentChar != QChar(';') && m_currentChar != QChar(')')) {
            type = type % m_currentChar;
            readNext();
        }
        JUMLog("read type -> %s", Q(type));
        if (m_currentChar == QChar(';') || m_currentChar == QChar(')')) {
            for (int i = 0; i < ports.count(); ++i) {
                entity.addPort(ports[i], mode, type);
            }
        } else {
            setError(ParserError_ExpectedSymbol, ")");
            return;
        }
        if (m_currentChar == QChar(';')) {
            readNext();
        }
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
    JUMLog("architecture name is %s.", Q(architectureName));

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

    readArchitectureDeclarativePart();
    CHECK_ERROR;

    if (!readKeyword("begin")) {
        return;
    }

    readArchitectureStatementPart();
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

        SKIP_SPACES_WITH_CHECK;
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

    m_state = JUParserVHDL::ParserStateNone;
}

void JUParserVHDL::readArchitectureDeclarativePart()
{
    SKIP_SPACES_WITH_CHECK;
    while (!isEOF() && m_currentChar != QChar('b') && m_state != ParserStateError) {
        SKIP_SPACES_WITH_CHECK;
        if (m_currentChar == QChar('c')) {
            readComponent();
        } else if (m_currentChar == QChar('s')) {
            readSignal();
        } else if (m_currentChar == QChar('b')) {
            //assume that this is 'begin' keyword
        } else {
            setError(ParserError_UnknownSequence);
        }
    }
}

void JUParserVHDL::readComponent()
{
    if (!readKeyword("component")) {
        return;
    }

    FilePos pos = m_currentPos;
    QString componentName = readIdentifier();
    CHECK_ERROR;
    if (!hasEntityWithName(componentName) && componentName != QString("2band2_or2")) {
        setErrorAtPosition(ParserError_UnknownEntity, pos);
        return;
    }

    if (m_currentChar == QChar('i')) {
        if (!readKeyword("is")) {
            return;
        }
    }

    JUEntity e("tmpName");
    readPort(e);

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
}

void JUParserVHDL::readSignal()
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
        JUMLog("read port -> %s", Q(portName));
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
    JUMLog("read type -> %s", Q(type));
    if (m_currentChar == QChar(';')) {
        //success, save ports
    } else {
        setError(ParserError_ExpectedSymbol, ";");
        return;
    }
    if (m_currentChar == QChar(';')) {
        readNext();
    }
}

void JUParserVHDL::readArchitectureStatementPart()
{
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
    if (!isEOF()) {
        //JUMLog("read identifier -> %s", Q(identifier));
        return identifier.toLower();
    }

    setError(ParserError_UnexpectedEOF);
    return "";
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
        m_currentChar = m_fileContent[m_caretPos++];
        m_currentPos.column++;
        if (m_currentChar == '\n') {
            m_currentPos.line++;
            m_currentPos.column = -1;
        }
        JUMLog("read char -> %c (at line %d, col %d).", m_currentChar, m_currentPos.line, m_currentPos.column);
        return true;
    }
    return false;
}

void JUParserVHDL::setError(JUParserVHDL::ParserError error, const QString& keyword)
{
    setErrorAtPosition(error, m_currentPos);
}

void JUParserVHDL::setErrorAtPosition(JUParserVHDL::ParserError error, FilePos pos, const QString& keyword)
{
    m_state = JUParserVHDL::ParserStateError;

    QString description = "unknown error";
    switch (error) {
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
        description = "wrong symbol.";
        break;
    case JUParserVHDL::ParserError_UnknownEntity:
        description = "unknown entity name.";
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

bool JUParserVHDL::hasEntityWithName(QString name)
{
    for (int i = 0; i < m_entities.count(); ++i) {
        if (m_entities[i].name() == name) {
            return true;
        }
    }
    return false;
}
