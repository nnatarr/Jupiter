
#ifndef __JUPARSERVHDL_H__
#define __JUPARSERVHDL_H__

#include "../JUParser.h"

class JUParserVHDL : public JUParser
{
public:
    JUParserVHDL();
    virtual ~JUParserVHDL();

    virtual QList<JUEntity *> parse(const QString& filePath);
    virtual bool isErrorSet();
    virtual QString errorMsg();

private:
    enum ParserState { ParserStateNone, ParserStateError, ParserStateComment, ParserStateCommentMultiLine, ParserStateEntity, ParserStateArchitecture };
    ParserState m_state;

    QChar m_currentChar;
    int m_caretPos;
    struct FilePos {
        int line;
        int column;
    } m_currentPos;
    QString m_fileContent;

    QString m_errorMsg;

    QList<JUEntity *> m_entities;

    bool canReadNext();
    bool isEOF();
    bool readNext();

    bool isCharAcceptableInIdentifier(const QChar& c);

    void readEntity();
    void readEntityHeader(JUEntity *entity);
    void readArchitecture();
    void readArchitectureDeclarativePart(JUEntity *e);
    void readArchitectureStatementPart(JUEntity *e);
    void readInstantiationStatement(JUEntity *e);
    void readProcessStatement(JUEntity *e);
    void readPort(JUEntity *entity);
    void readSignal(JUEntity *e);
    void readComponent(JUEntity *e);
    void readInterfaceList(JUEntity *entity);
    QString readIdentifier();

    bool readKeyword(QString keyWord);

    JUEntity* entityByName(QString name);
    bool hasEntityWithName(QString name);
    bool entityUseComponent(JUEntity *e, QString name);

    enum ParserError { ParserError_ExpectedKeyword, ParserError_ExpectedSymbol, ParserError_UnknownSequence, ParserError_UnexpectedEOF, 
                       ParserError_WrongSymbol, ParserError_IdentifierDuplicate, ParserError_UnknownEntity, ParserError_UndescribedEntityComponent,
                       ParserError_IncorrectLUTDescription, ParserError_UnknownOutputPort };
    void setErrorAtPosition(ParserError error, FilePos pos, const QString& keyword = "");
    void setError(ParserError error, const QString& keyword = "");

    void showErrorMsg(QString msg);
};

#endif