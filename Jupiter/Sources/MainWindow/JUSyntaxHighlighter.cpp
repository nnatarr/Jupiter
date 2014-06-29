
#include "JUSyntaxHighlighter.h"

JUSyntaxHighlighter::JUSyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
 {
     HighlightingRule rule;

     keywordFormat.setForeground(Qt::darkBlue);
     keywordFormat.setFontWeight(QFont::Bold);
     QStringList keywordPatterns;
     keywordPatterns << "\\bentity\\b" << "\\bport\\b" << "\\bmap\\b"
                     << "\\bis\\b" << "\\bof\\b" << "\\bend\\b"
                     << "\\bcase\\b" << "\\bprocess\\b" << "\\barchitecture\\b"
                     << "\\bdownto\\b" << "\\bwhen\\b" << "\\bin\\b"
                     << "\\bout\\b" << "\\bcomponent\\b" << "\\bbegin\\b"
                     << "\\bsignal\\b" << "=>" << "<=";
     foreach (const QString &pattern, keywordPatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = keywordFormat;
         highlightingRules.append(rule);
     }

     QStringList typePatterns;
     typePatterns << "\\bbit\\b" << "\\bBIT\\b" << "\\bstd_ulogic_vector\\b" << "\\bstd_logic_vector\\b";
     typeFormat.setFontWeight(QFont::Bold);
     typeFormat.setForeground(Qt::darkMagenta);
     foreach (const QString &pattern, typePatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = typeFormat;
         highlightingRules.append(rule);
     }

     singleLineCommentFormat.setForeground(Qt::gray);
     rule.pattern = QRegExp("--[^\n]*");
     rule.format = singleLineCommentFormat;
     highlightingRules.append(rule);

     numberFormat.setForeground(QColor(100, 200, 80));
     rule.pattern = QRegExp("\\b\\d+\\b");
     rule.format = numberFormat;
     highlightingRules.append(rule);

     labelFormat.setForeground(QColor(200, 100, 80));
     rule.pattern = QRegExp("^\\s*[\\w\\d_]+\\s*:(?!\\s*in|\\s*out)");
     rule.format = labelFormat;
     highlightingRules.append(rule);
 }

 void JUSyntaxHighlighter::highlightBlock(const QString &text)
 {
     foreach (const HighlightingRule &rule, highlightingRules) {
         QRegExp expression(rule.pattern);
         int index = expression.indexIn(text);
         while (index >= 0) {
             int length = expression.matchedLength();
             setFormat(index, length, rule.format);
             index = expression.indexIn(text, index + length);
         }
     }
     setCurrentBlockState(0);
 }