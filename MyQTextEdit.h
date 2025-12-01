#ifndef MyQTextEdit_H
#define MyQTextEdit_H

#include <QDebug>
#include <QTextEdit>
#include <QTextBlock>
#include <QMimeData>

#include "MyQShortings.h"

//---------------------------------------------------------------------------------------------------------------------

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

	inline static void SelectText(QTextEdit *textEdit, int row, int indexInRow, int length);
	inline static void SelectText(QTextEdit *textEdit, int index, int length);

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

	inline void SelectText(int row, int indexInRow, int length) { SelectText(this, row, indexInRow, length); }
	inline void SelectText(int index, int length) { SelectText(this, index, length); }

protected:
	/// переопределение вставки текста из буффера обмена для richTextPaste
	inline virtual void insertFromMimeData(const QMimeData *source) override;

	inline virtual void keyPressEvent(QKeyEvent *event) override;
	inline virtual bool IndentingOnEnterMechanic(QKeyEvent *event);
	inline virtual bool IndentingMultirowMechanic(QKeyEvent *event);
};

//---------------------------------------------------------------------------------------------------------------------

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

void MyQTextEdit::SelectText(QTextEdit *textEdit, int row, int indexInRow, int length)
{
	auto cursor = textEdit->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, row);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, indexInRow);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, length);
	textEdit->setTextCursor(cursor);
}

void MyQTextEdit::SelectText(QTextEdit *textEdit, int index, int length)
{
	auto cursor = textEdit->textCursor();
	cursor.setPosition(index);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, length);
	textEdit->setTextCursor(cursor);
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

void MyQTextEdit::keyPressEvent(QKeyEvent *event)
{
	if(IndentingOnEnterMechanic(event)) return;
	if(IndentingMultirowMechanic(event)) return;

	// Для всех остальных клавиш используем стандартную обработку
	QTextEdit::keyPressEvent(event);
}

bool MyQTextEdit::IndentingOnEnterMechanic(QKeyEvent *event)
{
	if (event->key() != Qt::Key_Return and event->key() != Qt::Key_Enter) return false;

	// Если нажата клавиша Enter или Return запускается механика

	QTextCursor cursor = textCursor();

	// использую LineUnderCursor, потому что BlockUnderCursor захватывает разделитель параграфа
	cursor.select(QTextCursor::LineUnderCursor);
	QString currentLine = cursor.selectedText();

	QChar leaderChar;
	uint leaderCharCount = 0;
	for (QChar c : currentLine)
	{
		if(leaderChar.isNull() and (c == ' ' or c == '\t')) leaderChar = c;

		if (c == leaderChar)
		{
			leaderCharCount++;
		}
		else
		{
			break;
		}
	}

	cursor.beginEditBlock();

	// Вызываем стандартную обработку нажатия Enter (создание новой строки)
	QTextEdit::keyPressEvent(event);

	// Вставляем полученный отступ в начало новой строки
	cursor = textCursor(); // Обновляем курсор после добавления новой строки
	for(uint i=0; i<leaderCharCount; i++)
		cursor.insertText(leaderChar);

	cursor.endEditBlock();

	return true;
}

bool MyQTextEdit::IndentingMultirowMechanic(QKeyEvent *event)
{
	if (event->key() != Qt::Key_Tab) return false;

	QTextCursor cursor = textCursor();
	if (!cursor.hasSelection()) return false;

	bool selectionFromRowBegin = false;

	cursor.beginEditBlock();

	int start_pos = cursor.selectionStart();
	int end_pos = cursor.selectionEnd();

	cursor.setPosition(start_pos);
	int start_block_number = cursor.blockNumber();
	cursor.setPosition(end_pos);
	int end_block_number = cursor.blockNumber();

	for (int block_n = start_block_number; block_n <= end_block_number; ++block_n) {
		QTextBlock block = document()->findBlockByNumber(block_n);

		if(block_n == start_block_number) // если это первый выбранный блок
			selectionFromRowBegin = start_pos == block.position();

		QTextCursor block_cursor(block);
		block_cursor.movePosition(QTextCursor::StartOfBlock);
		block_cursor.insertText("\t");
	}

	cursor.endEditBlock();

	const int oneTabLen = 1;

	// Восстанавливаем выделение
	// начало выделения
	if(selectionFromRowBegin) cursor.setPosition(start_pos);
	else cursor.setPosition(start_pos+oneTabLen); // с учетом добавленных отступов

	// конец выделения с учетом добавленных отступов
	int added_length_total = (end_block_number - start_block_number + 1) * oneTabLen;
	cursor.setPosition(end_pos + added_length_total, QTextCursor::KeepAnchor);
	setTextCursor(cursor);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------------------------------------------------
