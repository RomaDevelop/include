#ifndef SyntaxHighlighters
#define SyntaxHighlighters
//------------------------------------------------------------------------------------------------------------------------------------------
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QDebug>

#include "MyQShortings.h"
//------------------------------------------------------------------------------------------------------------------------------------------
class ForSyntaxHighlighters
{
public:
    inline static QTextCharFormat GetFormatUnderCursor(QTextEdit* textEdit);
};
//------------------------------------------------------------------------------------------------------------------------------------------
class XmlHighlighter : public QSyntaxHighlighter {

public:
    inline XmlHighlighter(QTextDocument* parent,
		   QTextCharFormat *tagFormat=nullptr,
		   QTextCharFormat *attributeFormat=nullptr,
		   QTextCharFormat *valueFormat=nullptr);

    inline void highlightBlock(const QString &text) override;

private:
    QTextCharFormat formatTag;
    QTextCharFormat formatAttribute;
    QTextCharFormat formatValue;

    struct HighlightingRule {
	QRegularExpression pattern;
	QTextCharFormat format;
    };
    //QVector<HighlightingRule> highlightingRules;
};
//------------------------------------------------------------------------------------------------------------------------------------------
QTextCharFormat ForSyntaxHighlighters::GetFormatUnderCursor(QTextEdit * textEdit)
{
    QTextCursor cursor = textEdit->textCursor();
    QTextBlock block = cursor.block();
    int pos = cursor.position() - block.position();
    QTextLayout *layout = block.layout();
    if (layout) {
	auto formats = layout->formats();
	for (const QTextLayout::FormatRange &range : formats) {
	    if (pos >= range.start && pos < (range.start + range.length)) {
		return range.format;
	    }
	}
    }
    return QTextCharFormat();
}
//------------------------------------------------------------------------------------------------------------------------------------------
XmlHighlighter::XmlHighlighter(QTextDocument * parent, QTextCharFormat * tagFormat, QTextCharFormat * attributeFormat, QTextCharFormat * valueFormat) :
    QSyntaxHighlighter(parent)
{
    if(tagFormat) this->formatTag = *tagFormat;
    else this->formatTag.setForeground(Qt::blue);
    if(attributeFormat) this->formatAttribute = *attributeFormat;
    else this->formatAttribute.setForeground(Qt::blue);
    if(valueFormat) this->formatValue = *valueFormat;
    else this->formatValue.setForeground(Qt::blue);

    //highlightingRules.append({ QRegularExpression("<[^>]*>"), this->formatTag });
    //highlightingRules.append({ QRegularExpression("\\b\\w+(?=\\=)"), this->formatAttribute });
    //highlightingRules.append({ QRegularExpression("\"[^\"]*\"(?=[^<]*>)"), this->formatValue });
}

void XmlHighlighter::highlightBlock(const QString & text)
{
//    for (const HighlightingRule &rule : highlightingRules)
//    {
//	QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
//	while (matchIterator.hasNext())
//	{
//	    QRegularExpressionMatch match = matchIterator.next();
//	    setFormat(match.capturedStart(), match.capturedLength(), rule.format);
//	}
//    }

    if(text.isEmpty()) return;

    setFormat(0, text.length(), formatTag);

    int firstLetter = 0;
    while(firstLetter < text.size() && text[firstLetter] == ' ') firstLetter++;
    if(firstLetter >= text.length())
    {
	for(int i=0; i<text.length(); i++)
	{
	    if(text[i] != ' ')
		qDebug() << "XmlHighlighter error define textStarts" << firstLetter << text.length() << text; return;
	}
    }

    bool tagStartedNow = false;
    if(text[firstLetter] == '<') tagStartedNow = true;

    int attrNameStart = -1;
    std::vector<int> quatsAt;
    int firstAssing = -1;
    for(int i=firstLetter; i<text.length(); i++)
    {
	if(text[i] == '"') quatsAt.push_back(i);
	if(text[i] == '=' && firstAssing == -1) firstAssing = i;

	if(text[i] == ' ') { attrNameStart = i+1; continue; }
	if(!tagStartedNow && i==firstLetter) { attrNameStart = i; continue; }
	if(attrNameStart != -1 && text[i] == '=')
	{
	    setFormat(attrNameStart, i-attrNameStart, formatAttribute);
	    attrNameStart = -1;
	    continue;
	}
    }

    if(quatsAt.empty() && !tagStartedNow) setFormat(0, text.length(), formatValue);

    if(quatsAt.size() == 1)
    {
	if(quatsAt[0] == text.size()-1)
	    setFormat(0, text.length(), formatValue);
	if(text.endsWith("\"/>"))
	    setFormat(0, quatsAt[0]+1, formatValue);

	if(quatsAt[0]-1 == firstAssing)
	{
	    setFormat(0, firstAssing, formatAttribute);
	    setFormat(firstAssing+1, text.length(), formatValue);
	}
    }

    if(quatsAt.size() % 2 == 0)
	for(uint i=0; i<quatsAt.size(); i+=2)
	{
	    setFormat(quatsAt[i], quatsAt[i+1] - quatsAt[i] + 1, formatValue);
	}
}


//------------------------------------------------------------------------------------------------------------------------------------------
#endif

