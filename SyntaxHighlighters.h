#ifndef SyntaxHighlighters
#define SyntaxHighlighters
//---------------------------------------------------------------------------
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
//---------------------------------------------------------------------------
class ForSyntaxHighlighters
{
public:
    inline static QTextCharFormat GetFormatUnderCursor(QTextEdit* textEdit);
};
//---------------------------------------------------------------------------
class XmlHighlighter : public QSyntaxHighlighter {

public:
    XmlHighlighter(QTextDocument* parent,
		   QTextCharFormat *tagFormat=nullptr,
		   QTextCharFormat *attributeFormat=nullptr,
		   QTextCharFormat *valueFormat=nullptr);

    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat tagFormat;
    QTextCharFormat attributeFormat;
    QTextCharFormat valueFormat;

    struct HighlightingRule {
	QRegularExpression pattern;
	QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
};
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
XmlHighlighter::XmlHighlighter(QTextDocument * parent, QTextCharFormat * tagFormat, QTextCharFormat * attributeFormat, QTextCharFormat * valueFormat) :
    QSyntaxHighlighter(parent)
{
    if(tagFormat) this->tagFormat = *tagFormat;
    else this->tagFormat.setForeground(Qt::blue);
    if(attributeFormat) this->attributeFormat = *attributeFormat;
    else this->attributeFormat.setForeground(Qt::blue);
    if(valueFormat) this->valueFormat = *valueFormat;
    else this->valueFormat.setForeground(Qt::blue);

    highlightingRules.append({ QRegularExpression("<[^>]*>"), this->tagFormat });
    highlightingRules.append({ QRegularExpression("\\b\\w+(?=\\=)"), this->attributeFormat });
    highlightingRules.append({ QRegularExpression("\"[^\"]*\"(?=[^<]*>)"), this->valueFormat });
}

void XmlHighlighter::highlightBlock(const QString & text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
	QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
	while (matchIterator.hasNext()) {
	    QRegularExpressionMatch match = matchIterator.next();
	    setFormat(match.capturedStart(), match.capturedLength(), rule.format);
	}
    }
}




//---------------------------------------------------------------------------
#endif

