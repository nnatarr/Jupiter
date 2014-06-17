
#ifndef __JUPARSERVHDL_H__
#define __JUPARSERVHDL_H__

#include "../JUParser.h"
#include "JUEntity.h"

class JUParserVHDL : public JUParser
{
public:
    JUParserVHDL();
    virtual ~JUParserVHDL();

    virtual JUSchemeTree* parse(const QString& filePath);

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

    QList<JUEntity> m_entities;

    bool canReadNext();
    bool isEOF();
    bool readNext();

    bool isCharAcceptableInIdentifier(const QChar& c);

    void readEntity();
    void readEntityHeader(JUEntity &entity);
    void readArchitecture();
    void readArchitectureDeclarativePart();
    void readArchitectureStatementPart();
    void readPort(JUEntity &entity);
    void readSignal();
    void readComponent();
    void readInterfaceList(JUEntity &entity);
    QString readIdentifier();

    bool readKeyword(QString keyWord);

    bool hasEntityWithName(QString name);

    enum ParserError { ParserError_ExpectedKeyword, ParserError_ExpectedSymbol, ParserError_UnknownSequence, ParserError_UnexpectedEOF, ParserError_WrongSymbol, ParserError_IdentifierDuplicate, ParserError_UnknownEntity };
    void setErrorAtPosition(ParserError error, FilePos pos, const QString& keyword = "");
    void setError(ParserError error, const QString& keyword = "");
};

#endif