#ifndef MyQSearch_H
#define MyQSearch_H

#include <functional>

#include <QApplication>
#include <QDebug>
#include <QStyle>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QToolTip>
#include <QTimer>
#include <QHBoxLayout>

#include "MyQShortings.h"
#include "MyQDifferent.h"
#include "MyQDialogs.h"

//==========================================================================================================================
/*
 * Если объект MyQSearch создается на стеке, он должен существовать, пока существуют виджеты объекта.
 * Если он будет уничтожен, при взаимодействии с виджетами будет работа с мёртвым this.
 * Поэтому объект стоит делать членом класса окна в котором он расположен, либо создавать в куче.
 *
 * Если объект MyQSearch создается в куче, можно использовать механику deleteThisOnLineEditDestroyed,
 * тогда созданный MyQSearch будет уничтожен при уничтожении LineEdit-а.
 * Либо уничтожить его самостоятельно в нужный момент.
 *
 * Альтернатива на будущее: перенести все члены класса MyQSearch в property LineEdit-а, нигде не использовать this.
 *
*/
//==========================================================================================================================

struct MyQSearch
{
public:

	inline static MyQSearch MakeOnStack() { return MyQSearch(true); }
	inline static MyQSearch* MakeInHeap(bool deleteThisOnLineEditDestroyed);

	inline void DeleteThisOnLineEditDestroyed(bool enable);

    std::function<void()> cbSearchFinished;
    QString searchResult;

    inline std::array<QWidget*, 3> AllButtons();
	inline std::array<QWidget*, 5> AllWidgets();
    inline QLineEdit* LineEdit() { return lineEditTextToFind; }
	inline QCheckBox* CheckBoxCaseSense() { return checkBoxCaseSense; }

    inline void Place(QTextEdit *textEdit, QHBoxLayout *hlo);
    inline void SetTextEdit(QTextEdit *textEdit);

	inline ~MyQSearch();

private:

	inline MyQSearch(bool onStack);
	
    inline void SlotBtnFind();
    inline void SlotBtnNext();
    inline void SlotBtnPrev();
	
    QLineEdit *lineEditTextToFind;
    QPushButton *btnSearch;
    QPushButton *btnBack;
    QPushButton *btnForward;
	QCheckBox *checkBoxCaseSense = nullptr;

    Qt::CaseSensitivity caseSenseForFind = Qt::CaseInsensitive;
    ///\brief Вектор в котором хранятся результаты поиска (индексы найденного)
    std::vector<int> foundIndexes;
    ///\brief Длина искомого текста
    ulong promptLength {0};

    QTextEdit *textEdit = nullptr;

	bool existOnStack = false;
	inline static const char *deleteMyQSearchOnLineEditDestroyed = "deleteMyQSearchOnLineEditDestroyed";
	inline static const char *MyQSearch_has_been_deleted = "MyQSearch_has_been_deleted";
	inline static const char *widgets_destroy_called = "widgets_destroy_called";
};

MyQSearch::MyQSearch(bool onStack)
{
	existOnStack = onStack;

	lineEditTextToFind = new QLineEdit();
    btnSearch = new QPushButton();
    btnSearch->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_FileDialogContentsView));
    btnBack = new QPushButton(); // 🡸 🡺   🠴 🠶   🡄 🡆 🠬 🠮 🠨 🠪 🠤 🠦 🠠 🠢
    btnBack->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ArrowBack));
    btnForward = new QPushButton();
    btnForward->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ArrowForward));
	checkBoxCaseSense = new QCheckBox("Учитывать регистр");
	QObject::connect(lineEditTextToFind, &QLineEdit::returnPressed, [this]() { SlotBtnFind(); });
    QObject::connect(btnSearch, &QPushButton::clicked, btnSearch, [this]() { SlotBtnFind(); });
    QObject::connect(btnBack, &QPushButton::clicked, btnBack, [this]() { SlotBtnPrev(); });
    QObject::connect(btnForward, &QPushButton::clicked, btnForward, [this]() { SlotBtnNext(); });
	QObject::connect(checkBoxCaseSense, &QCheckBox::stateChanged, [this](int state)
    {
        if(state == Qt::Unchecked) caseSenseForFind = Qt::CaseInsensitive;
        else if(state == Qt::Checked) caseSenseForFind = Qt::CaseSensitive;
		qCritical() << "wrong checkstate for logs filter";
    });

	QObject::connect(lineEditTextToFind, &QObject::destroyed, lineEditTextToFind, [le = lineEditTextToFind, thisPtr = this]()
	{
		le->setProperty(widgets_destroy_called, true);
		if(le->property(deleteMyQSearchOnLineEditDestroyed).toBool())
		{
			if(not le->property(MyQSearch_has_been_deleted).toBool())
				delete thisPtr;
			else qCritical() << "MyQSearch: mechanic deleteThisOnLineEditDestroyed should delete MyQSearch object, "
								"but it has already been destroyed";
		}
	});
}

MyQSearch *MyQSearch::MakeInHeap(bool deleteThisOnLineEditDestroyed) {
	auto obj = new MyQSearch(false);
	obj->DeleteThisOnLineEditDestroyed(deleteThisOnLineEditDestroyed);
	return obj;
}

MyQSearch::~MyQSearch()
{
	if(not lineEditTextToFind->property(widgets_destroy_called).toBool())
	{
		for(auto w:AllWidgets())
		{
			w->setToolTip("MyQSearch object has been destroyed");
			w->setDisabled(true);
		}
	}

	lineEditTextToFind->setProperty(MyQSearch_has_been_deleted, true);
}

void MyQSearch::DeleteThisOnLineEditDestroyed(bool enable)
{
	if(not existOnStack) lineEditTextToFind->setProperty(deleteMyQSearchOnLineEditDestroyed, enable);
	else qCritical() << "MyQSearch::DeleteThisOnLineEditDestroyed called for object on stack";
}

void MyQSearch::Place(QTextEdit * textEdit, QHBoxLayout * hlo)
{
	this->textEdit = textEdit;

	hlo->addWidget(lineEditTextToFind);
	hlo->addWidget(btnBack);
	hlo->addWidget(btnSearch);
    hlo->addWidget(btnForward);
	hlo->addWidget(checkBoxCaseSense);
}

void MyQSearch::SetTextEdit(QTextEdit * textEdit)
{
    this->textEdit = textEdit;
    searchResult.clear();
    foundIndexes.clear();
    promptLength = 0;
}

std::array<QWidget *, 3> MyQSearch::AllButtons()
{
	std::array<QWidget*, 3> buttons {
		btnSearch, btnBack, btnForward
	};
	return buttons;
}

std::array<QWidget *, 5> MyQSearch::AllWidgets()
{
	std::array<QWidget*, 5> widgets {
		lineEditTextToFind, btnSearch, btnBack, btnForward, checkBoxCaseSense
	};
	return widgets;
}

void MyQSearch::SlotBtnFind()
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

	if(searchResult.isEmpty()) searchResult = "Найдено совпадений: " + QSn(foundIndexes.size());

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

	MyQDialogs::ToastMessage("Поиск", searchResult, textEdit, 3000);
}

void MyQSearch::SlotBtnNext()
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

void MyQSearch::SlotBtnPrev()
{
    if (foundIndexes.empty()) return;

    auto curs = textEdit->textCursor();
    int curPos = textEdit->textCursor().selectionStart();
    bool find = false;
    for(int i=static_cast<int>(foundIndexes.size())-1; i>=0; i--)
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

//--------------------------------------------------------------------------------------------------------------------------
#endif
