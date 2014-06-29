
#ifndef __JUSYNTAXHIGHLIGHTER_H__
#define __JUSYNTAXHIGHLIGHTER_H__

#include <QSyntaxHighlighter>

class QTextDocument;

class JUSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

 public:
     JUSyntaxHighlighter(QTextDocument *parent = 0);

 protected:
     void highlightBlock(const QString &text);

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;

     QRegExp commentStartExpression;
     QRegExp commentEndExpression;

     QTextCharFormat keywordFormat;
     QTextCharFormat typeFormat;
     QTextCharFormat numberFormat;
     QTextCharFormat labelFormat;
     QTextCharFormat singleLineCommentFormat;
};

#endif