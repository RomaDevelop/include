#ifndef MyQTextEdit_H
#define MyQTextEdit_H

#include <QDebug>
#include <QTextEdit>
#include <QTextBlock>
#include <QMimeData>

#include "MyQShortings.h"

//------------------------------------------------------------------------------------------------------------------------------------------

class MyQTextEdit : public QTextEdit
{
	Q_OBJECT
public:
	inline static void AppendInLastRow(QTextEdit *textEdit, const QString& text);
	inline static void ColorizeLastRow(QTextEdit *textEdit, const QBrush &brush);
	inline static void ColorizeLastCount(QTextEdit *textEdit, const QBrush &brush, uint count);
	inline void Colorize(QTextEdit *textEdit, int from, int to, const QColor &color);

	inline static QTextCharFormat LetterFormat(QTextEdit *textEdit, int letterIndex);
	enum direction { up, down };
	inline static bool MoveCurrentRow(QTextEdit *edit, direction direction);
	inline static void RemoveCurrentRow(QTextEdit *edit);

public:
	enum richTextPasteValue { richTextPasteDisabled = 0, richTextPasteEnabled = 1 };

	inline explicit MyQTextEdit(QWidget *parent = nullptr) : QTextEdit(parent)
	{ }
	inline explicit MyQTextEdit(MyQTextEdit::richTextPasteValue aRichTextPaste, QWidget *parent = nullptr) :
		QTextEdit(parent),
		richTextPaste{aRichTextPaste == richTextPasteEnabled}
	{ }
	virtual ~MyQTextEdit() = default;

	bool richTextPaste = true; // если флаг установлен - текст будет вставляться с сохранением его форматирования

	inline QTextCharFormat LetterFormat(int letterIndex) { return LetterFormat(this, letterIndex); }

private:

protected:
	inline virtual void insertFromMimeData(const QMimeData *source) override; // переопределение вставки текста из буффера обмена для richTextPaste
};

//------------------------------------------------------------------------------------------------------------------------------------------

void MyQTextEdit::AppendInLastRow(QTextEdit * textEdit, const QString & text)
{
    auto cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    cursor.insertText(text);
}

void MyQTextEdit::ColorizeLastRow(QTextEdit * textEdit, const QBrush & brush)
{
    QTextCharFormat format;
    format.setForeground(brush);

    auto curs = textEdit->textCursor();
    curs.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    curs.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    curs.setCharFormat(format);
    textEdit->repaint();
}

void MyQTextEdit::ColorizeLastCount(QTextEdit * textEdit, const QBrush & brush, uint count)
{
    QTextCharFormat format;
    format.setForeground(brush);

    auto curs = textEdit->textCursor();
    curs.setPosition(textEdit->document()->characterCount() - 1 - count, QTextCursor::MoveAnchor);
    curs.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    curs.setCharFormat(format);
    textEdit->repaint();
}

void MyQTextEdit::Colorize(QTextEdit * textEdit, int from, int to, const QColor & color)
{
    QTextCharFormat format;
    format.setForeground(color);

    auto cursor = textEdit->textCursor();
    cursor.setPosition(from,QTextCursor::MoveAnchor);
    cursor.setPosition(to,QTextCursor::KeepAnchor);
    cursor.setCharFormat(format);
}

QTextCharFormat MyQTextEdit::LetterFormat(QTextEdit *textEdit, int letterIndex)
{
	auto cursor = textEdit->textCursor();
	cursor.setPosition(letterIndex, cursor.MoveAnchor);
	cursor.setPosition(letterIndex+1, cursor.KeepAnchor);
	return cursor.charFormat();
}

bool MyQTextEdit::MoveCurrentRow(QTextEdit *edit, direction direction)
{
	QTextCursor cursor = edit->textCursor();
	int curBlockNum = cursor.blockNumber();
	if (direction == up && curBlockNum == 0) return false;
	if (direction == down && curBlockNum == edit->document()->blockCount()-1) return false;

	QTextBlock curBlock = cursor.block();
	QTextBlock otherBlock = direction == up ? curBlock.previous() : curBlock.next();
	QString otherBlockText = otherBlock.text();

	cursor.beginEditBlock();

	cursor = QTextCursor(otherBlock);
	cursor.select(QTextCursor::LineUnderCursor);
	cursor.insertText(curBlock.text());

	cursor = QTextCursor(curBlock);
	cursor.select(QTextCursor::LineUnderCursor);
	cursor.insertText(otherBlockText);

	cursor = edit->textCursor();
	cursor.movePosition(direction == up ? QTextCursor::Up : QTextCursor::Down, QTextCursor::MoveAnchor);
	edit->setTextCursor(cursor);

	cursor.endEditBlock();

	return true;
}

void MyQTextEdit::RemoveCurrentRow(QTextEdit *edit)
{
	auto cursor = edit->textCursor();
	cursor.beginEditBlock();
	if (cursor.blockNumber() == edit->document()->blockCount()-1)
	{
		auto cursor = edit->textCursor();
		cursor.movePosition(cursor.Up);
		edit->setTextCursor(cursor);
	}
	cursor.select(QTextCursor::LineUnderCursor);
	cursor.removeSelectedText();
	cursor.movePosition(cursor.Left);
	cursor.deleteChar();
	cursor.endEditBlock();
}

void MyQTextEdit::insertFromMimeData(const QMimeData * source)
{
	if(richTextPaste) QTextEdit::insertFromMimeData(source);
	else
	{
		QString text = source->text(); // Получаем текст из буфера обмена без форматирования
		textCursor().insertText(text); // Вставляем текст с текущим форматированием QTextEdit
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
