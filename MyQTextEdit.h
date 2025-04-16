#ifndef MyQTextEdit_H
#define MyQTextEdit_H

#include <QDebug>
#include <QTextEdit>
#include <QMimeData>

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

public:
	explicit MyQTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {}
	virtual ~MyQTextEdit() = default;
	bool richTextPaste = true; // если флаг установлен - текст будет вставляться с сохранением его форматирования

protected:
	inline void insertFromMimeData(const QMimeData *source) override; // переопределение вставки текста из буффера обмена для richTextPaste
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

void MyQTextEdit::insertFromMimeData(const QMimeData * source)
{
	if(richTextPaste) QTextEdit::insertFromMimeData(source);
	else
	{
		QString text = source->text(); // Получаем текст из буфера обмена без форматирования
		textCursor().insertText(text); // Вставляем текст с текущим форматированием QTextEdit
	}
}

#endif
