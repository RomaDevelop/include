//==================================================================================================
/*
 * ### Добавление в проект:
 * Разместить в папке проекта cleanerthread.cpp и cleanerthread.h
 * Указать их в .pro в секциях SOURCES и HEADERS
 *
 * ### Применение в проекте:
 * std::shared_ptr <CleanerThread> cleanerThread1;								// в объекте-владельце CleanerThread (или глобально)
 * cleanerThread1 = make_shared<CleanerThread>(ui->textBrowser, maxRowsCount);	// в конструкторе объекта-владельца CleanerThread (или в начале программы)
*/
//==================================================================================================
#ifndef TextEditCleaner_HPP
#define TextEditCleaner_HPP
//--------------------------------------------------------------------------------------------------
#include <memory>
#include <thread>

#include <QDebug>
#include <QObject>
#include <QTextEdit>
//--------------------------------------------------------------------------------------------------
class CleanerThread : public QObject
{
public:

	///\brief connect SignalNeedClean -> SlotClean
	inline explicit CleanerThread(QObject *parent = nullptr);
	///\brief CleanerThread() and StartCleanObject(...)
	inline explicit CleanerThread(QTextEdit  *objToClean_, unsigned int linesThreshold, QObject *parent = nullptr);
	///\brief Вызывает StopClean()
	inline ~CleanerThread();

	CleanerThread(const CleanerThread & src) = delete;
	CleanerThread(CleanerThread && src) = delete;
	CleanerThread& operator= (const CleanerThread & src) = delete;
	CleanerThread& operator= (CleanerThread && src) = delete;

	///\brief Запуск потока очистки
	/// если был забущен ранее - перед запуском вызывает StopClean()
	///\param[in] objToClean_ - объект, который будет очищаться
	///\param[in] linesThreshold_ - порог строк, свыше которого верхние удаляются
	inline void StartClean(QTextEdit  *objToClean_, unsigned int linesThreshold_);
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
	void SignalNeedClean(int count);

private slots:
	///\brief Слот очищающий objToClean. connect с SignalNeedClean
	inline void SlotClean(int count);

private:

	Q_OBJECT

	///\brief Поток, который будет осуществлять очистку
	std::thread *threadCleaner = nullptr;
	///\brief Объект, который будет очищаться
	QTextEdit  *objToClean = nullptr;
	///\brief Порог строк, свыше которого верхние удаляются
	unsigned int linesThreshold {1000};
	///\brief Флаг остановки очистки
	bool stopClean = false;
	///\brief Флаг приостановки очистки
	bool doClean = true;
};

CleanerThread::CleanerThread(QObject * parent):
	QObject(parent)
{
	connect(this, &CleanerThread::SignalNeedClean, this, &CleanerThread::SlotClean);
}

CleanerThread::CleanerThread(QTextEdit * objToClean_, unsigned int linesThreshold, QObject * parent):
	CleanerThread(parent)
{
	StartClean(objToClean_, linesThreshold);
}

CleanerThread::~CleanerThread()
{
	StopClean();
}

void CleanerThread::StartClean(QTextEdit * objToClean_, unsigned int linesThreshold_)
{
	objToClean = objToClean_;
	stopClean = false;
	linesThreshold = linesThreshold_;

	StopClean();

	threadCleaner = new std::thread([this]
	{
		unsigned int count;
		while(!stopClean)
		{
			count = objToClean->document()->lineCount();
			// document()->lineCount() подходит для этой задачи,
			// он не анализирует весь тект, а возвращает имеющееся внутри объекта значение
			// альтернативой может быть document()->blockCount()
			if(doClean && count > linesThreshold)
			{
				emit SignalNeedClean(count - linesThreshold);
			}

			if(!stopClean) std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
}

void CleanerThread::StopClean() // если поток существует, то он останавливается и удаляется
{
	if(threadCleaner)
	{
		stopClean = true;
		threadCleaner->join();
		delete threadCleaner;
		threadCleaner = nullptr;
	}
}

void CleanerThread::SuspendClean()
{
	doClean = false;
}

void CleanerThread::ContinueClean()
{
	doClean = true;
}

void CleanerThread::SetLinesThreshold(unsigned int linesThreshold_)
{
	linesThreshold = linesThreshold_;
}

unsigned int CleanerThread::GetLinesThreshold()
{
	return linesThreshold;
}

void CleanerThread::SlotClean(int count)
{
	QTextCursor cursor = objToClean->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, count);
	cursor.removeSelectedText();
}
//--------------------------------------------------------------------------------------------------
#endif
