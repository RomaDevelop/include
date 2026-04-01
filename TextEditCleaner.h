//==========================================================================================================================
/*
 * ### Добавление в проект:
 * Добавить этот файл в .pro
 *
 * ### Применение в проекте:
 * textEdit = new QTextEdit;
 * new TextEditCleaner(textEdit, 1000, textEdit);
 * тут textEdit передается и как объект для очистки, и как родитель,
 * и textEditCleaner уничтожится перед уничтожением textEdit
*/
//==========================================================================================================================
#ifndef TextEditCleaner_HPP
#define TextEditCleaner_HPP
//--------------------------------------------------------------------------------------------------------------------------
#include <memory>
#include <thread>

#include <QDebug>
#include <QObject>
#include <QTextEdit>
//--------------------------------------------------------------------------------------------------------------------------
class TextEditCleaner : public QObject
{
	Q_OBJECT

public:

	///\brief connect SignalNeedClean -> SlotClean
	inline explicit TextEditCleaner(QObject *parent = nullptr);
	///\brief CleanerThread() and StartCleanObject(...)
	inline explicit TextEditCleaner(QTextEdit  *objToClean_, int linesThreshold, QObject *parent = nullptr);
	///\brief Вызывает StopClean()
	inline ~TextEditCleaner();

	TextEditCleaner(const TextEditCleaner & src) = delete;
	TextEditCleaner(TextEditCleaner && src) = delete;
	TextEditCleaner& operator= (const TextEditCleaner & src) = delete;
	TextEditCleaner& operator= (TextEditCleaner && src) = delete;

	///\brief Запуск потока очистки
	/// если был забущен ранее - перед запуском вызывает StopClean()
	///\param[in] objToClean_ - объект, который будет очищаться
	///\param[in] linesThreshold_ - порог строк, свыше которого верхние удаляются
	inline void StartClean(QTextEdit  *objToClean_, int linesThreshold_);
	///\brief Если поток существует, то он останавливает и удаляет его
	inline void StopClean();

	///\brief Приостановка очистки
	inline void SuspendClean();
	///\brief Продолжение очистки
	inline void ContinueClean();

	///\brief Устанавливает linesThreshold
	/// остановка не требуется
	inline void SetLinesThreshold(unsigned int linesThreshold_);
	///\brief Возвращает linesThreshold
	inline unsigned int GetLinesThreshold();

private: signals:
	///\brief Сигнал, который посылает поток если превышен linesThreshold. connect с SlotClean
	void SignalNeedClean();

private slots:
	///\brief Слот очищающий objToClean. connect с SignalNeedClean
	inline void SlotClean();

private:

	///\brief Поток, который будет осуществлять очистку
	std::thread *threadCleaner = nullptr;
	///\brief Объект, который будет очищаться
	QTextEdit  *objToClean = nullptr;
	///\brief Порог строк, свыше которого верхние удаляются
	int linesThreshold {1000};
	///\brief Флаг остановки очистки
	bool stopClean = false;
	///\brief Флаг приостановки очистки
	bool doClean = true;

	inline int LinesCount();
};

TextEditCleaner::TextEditCleaner(QObject * parent):
	QObject(parent)
{
	connect(this, &TextEditCleaner::SignalNeedClean, this, &TextEditCleaner::SlotClean);
}

TextEditCleaner::TextEditCleaner(QTextEdit * objToClean_, int linesThreshold, QObject * parent):
	TextEditCleaner(parent)
{
	StartClean(objToClean_, linesThreshold);
}

TextEditCleaner::~TextEditCleaner()
{
	StopClean();
}

void TextEditCleaner::StartClean(QTextEdit * objToClean_, int linesThreshold_)
{
	objToClean = objToClean_;
	stopClean = false;
	linesThreshold = linesThreshold_;

	StopClean();

	threadCleaner = new std::thread([this]
	{
		while(!stopClean)
		{
			int count = LinesCount();

			if(doClean && count > linesThreshold)
			{
				emit SignalNeedClean();
			}

			if(!stopClean) std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
}

void TextEditCleaner::StopClean() // если поток существует, то он останавливается и удаляется
{
	if(threadCleaner)
	{
		stopClean = true;
		threadCleaner->join();
		delete threadCleaner;
		threadCleaner = nullptr;
	}
}

void TextEditCleaner::SuspendClean()
{
	doClean = false;
}

void TextEditCleaner::ContinueClean()
{
	doClean = true;
}

void TextEditCleaner::SetLinesThreshold(unsigned int linesThreshold_)
{
	linesThreshold = linesThreshold_;
}

unsigned int TextEditCleaner::GetLinesThreshold()
{
	return linesThreshold;
}

void TextEditCleaner::SlotClean()
{
	int count = LinesCount();
	int countToRemove = count - linesThreshold;
	if(countToRemove <= 0) return;
	/// Проверка нужна, на случай если заполнение текста идет долго, занимая поток.
	/// В этом случае сигнал будет выброшен несколько раз, слот поставлен в очередь,
	/// а когда заполнение текста закончится слот будет вызван несколько раз и удалит всё.

	QTextCursor cursor = objToClean->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, countToRemove);
	cursor.removeSelectedText();
}

int TextEditCleaner::LinesCount()
{
	return objToClean->document()->lineCount();
	// document()->lineCount() подходит для этой задачи,
	// он не анализирует весь текст, а возвращает имеющееся внутри объекта значение
	// альтернативой может быть document()->blockCount()
}

//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------
