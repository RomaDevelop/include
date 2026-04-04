#ifndef MyQSearch_H
#define MyQSearch_H

#include <functional>

#include <QApplication>
#include <QDebug>
#include <QStyle>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>
#include <QHBoxLayout>

#include "MyQShortings.h"
#include "MyQDifferent.h"

//--------------------------------------------------------------------------------------------------------------------------

struct MyQSearch
{
	QLineEdit *lineEditTextToFind;
	QPushButton *btnSearch;
	QPushButton *btnBack;
	QPushButton *btnForward;
	QCheckBox *chBoxSearchCaseSense = nullptr;
	Qt::CaseSensitivity caseSenseForFind = Qt::CaseSensitive;
	///\brief Вектор в котором хранятся результаты поиска (индексы найденного)
	std::vector<int> foundIndexes;
	///\brief Длина искомого текста
	ulong promptLength {0};
	
	QTextEdit *textEdit = nullptr;

	QString searchResult;

	std::function<void()> cbSearchFinished;
	
	MyQSearch()
	{
		lineEditTextToFind = new QLineEdit;
		btnSearch = new QPushButton();
		btnSearch->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_FileDialogContentsView));
		btnBack = new QPushButton(); // 🡸 🡺   🠴 🠶   🡄 🡆 🠬 🠮 🠨 🠪 🠤 🠦 🠠 🠢
		btnBack->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ArrowBack));
		btnForward = new QPushButton();
		btnForward->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ArrowForward));
		chBoxSearchCaseSense = new QCheckBox("Учитывать регистр");
		QObject::connect(btnSearch, &QPushButton::clicked, btnSearch, [this]() { SlotBtnFind(); });
		QObject::connect(btnBack, &QPushButton::clicked, btnBack, [this]() { SlotBtnPrev(); });
		QObject::connect(btnForward, &QPushButton::clicked, btnForward, [this]() { SlotBtnNext(); });
		QObject::connect(chBoxSearchCaseSense, &QCheckBox::stateChanged, [this](int state)
		{
			if(state == Qt::Unchecked) caseSenseForFind = Qt::CaseInsensitive;
			else if(state == Qt::Checked) caseSenseForFind = Qt::CaseSensitive;
			qdbg << "wrong checkstate for logs filter";
		});
	}

	std::array<QWidget*, 3> AllButtons()
	{
		std::array<QWidget*, 3> buttons {
			btnSearch, btnBack, btnForward
		};
		return buttons;
	}

	void Place(QTextEdit *textEdit, QHBoxLayout *hlo)
	{
		this->textEdit = textEdit;

		hlo->addWidget(lineEditTextToFind);
		hlo->addWidget(btnBack);
		hlo->addWidget(btnSearch);
		hlo->addWidget(btnForward);
		hlo->addWidget(chBoxSearchCaseSense);
	}
	
	void SlotBtnFind()
	{
		QString toFind = lineEditTextToFind->text();
		if(toFind.isEmpty()) return;
		
		foundIndexes.clear();
		searchResult.clear();
		auto curs = textEdit->textCursor();
		auto cursBack = curs;

		// сброс имевшихся ранее выделений цветом
		textEdit->selectAll();
		textEdit->setTextBackgroundColor(Qt::white);
		curs.clearSelection();
		textEdit->setTextCursor(curs);

		promptLength = toFind.length();
		QString text = textEdit->toPlainText();
		long find = 0;
		while(true)
		{
			find = text.indexOf(toFind, find, caseSenseForFind);
			if(find == -1) break;
			
			foundIndexes.push_back(find);
			curs.setPosition(find, QTextCursor::MoveAnchor);
			curs.setPosition(find+promptLength, QTextCursor::KeepAnchor);
			textEdit->setTextCursor(curs);
			textEdit->setTextBackgroundColor(Qt::yellow);
			find += promptLength;

			if (foundIndexes.size() > 300)
			{
				searchResult = "Найдено более 300 совпадений. Поиск остановлен.";
				break;
			}
		}

		if(not searchResult.isEmpty()) searchResult = "Найдено совпадений: " + QSn(foundIndexes.size());
		
		if(foundIndexes.size())
		{
			curs.setPosition(foundIndexes[0]+promptLength, QTextCursor::MoveAnchor);
			curs.setPosition(foundIndexes[0], QTextCursor::KeepAnchor);
			textEdit->setTextCursor(curs);
		}
		else
		{
			textEdit->setTextCursor(cursBack);
		}
		
		// SlotBtnForthClicked(); нужно ли?
		textEdit->setFocus();

		if(cbSearchFinished) cbSearchFinished();
	}

	void SlotBtnNext()
	{
		if (foundIndexes.empty()) return;

		auto curs = textEdit->textCursor();
		int curPos = textEdit->textCursor().selectionStart();
		bool find = false;
		for(auto f:foundIndexes)
			if(f > curPos)
			{
				curPos = f;
				find = true;
				break;
			}
		if(find)
		{
			curs.setPosition(curPos, QTextCursor::MoveAnchor);
			curs.setPosition(curPos+promptLength, QTextCursor::KeepAnchor);
			textEdit->setTextCursor(curs);
		}
		textEdit->setFocus();
	}

	void SlotBtnPrev()
	{
		if (foundIndexes.empty()) return;

		auto curs = textEdit->textCursor();
		int curPos = textEdit->textCursor().selectionStart();
		bool find = false;
		for(int i=foundIndexes.size()-1; i>=0; i--)
			if(foundIndexes[i] < curPos)
			{
				curPos = foundIndexes[i];
				find = true;
				break;
			}
		if(find)
		{
			curs.setPosition(curPos+promptLength, QTextCursor::MoveAnchor);
			curs.setPosition(curPos, QTextCursor::KeepAnchor);
			textEdit->setTextCursor(curs);
		}
		textEdit->setFocus();
	}
	
};

//--------------------------------------------------------------------------------------------------------------------------
#endif
