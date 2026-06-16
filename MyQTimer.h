#ifndef MyQTimer_H
#define MyQTimer_H

#include <set>
#include <queue>
#include <memory>
#include <functional>

#include <QDebug>
#include <QTimer>

//--------------------------------------------------------------------------------------------------------------------------

///\brief MyQTimerPool is intended for use in one thread.
struct MyQTimerPool
{
	MyQTimerPool() { }
	~MyQTimerPool() { ClearTimers(); }

	inline static void SingleShotSt(QObject *parent, int msec, std::function<void()> task);

	using Tasks = std::vector<std::pair<int, std::function<void()>>>;
	inline static void SingleShotsSt(QObject *parent, Tasks tasks);

	inline void SingleShot(QObject *parent, int msec, std::function<void()> function);
	inline void SingleShots(QObject *parent, Tasks tasks);
	inline void ClearTimers();

private:
	std::set<QTimer*> allTimers;
	std::set<QTimer*> freeTimers;

	static MyQTimerPool& staticPool() { static MyQTimerPool pool; return pool; }
};

struct MyQTimerPoolSt
{
	inline void SingleShot(QObject *parent, int msec, std::function<void()> function) { MyQTimerPool::SingleShotSt(parent, msec, std::move(function)); }
	using Tasks = MyQTimerPool::Tasks;
	inline void SingleShots(QObject *parent, Tasks tasks){ MyQTimerPool::SingleShotsSt(parent, std::move(tasks)); }
};

//--------------------------------------------------------------------------------------------------------------------------

class MyQTimer : public QTimer {
    Q_OBJECT
public:

	///\brief Асинхронный цикл выполняемый в потоке GUI
	/// эквивалент for (int i = i_start; i < i_less_than; i++)
	/// но выполняется через интервалы interval
	/// to_do: сейчас таймеру просто ставится интервал, можно сделать версию с паузами между итерациями
	inline static void For(int i_start, int i_less_than, int interval, QObject *parent,
				  std::function<void(int &i)> iteration,
				  std::function<void()> finishAction);


	int timerTicks = 0;
	int counter = 0;
	int tag = 0;
	int finish = false;
	QString stringValue;
	std::map<QString,int> intValues;
	explicit MyQTimer(QObject *parent = nullptr) : QTimer(parent) { }
	void Finish(bool deleteTimer)
	{
		stop();
		if(deleteTimer) deleteLater();
	}
};

void MyQTimer::For(int i_start, int i_less_than, int interval, QObject *parent,
				   std::function<void (int &)> iteration, std::function<void ()> finishAction)
{
	auto iShPtr = std::make_shared<int>(i_start);
	QTimer *timer = new QTimer(parent);
	connect(timer, &QTimer::timeout, parent, [iShPtr, i_less_than, iteration, finishAction, timer](){
		auto &i = *iShPtr.get();
		iteration(i);
		i++;
		if(i >= i_less_than)
		{
			finishAction();
			timer->stop();
			timer->deleteLater();
		}
	});
	timer->start(interval);
}

void MyQTimerPool::SingleShotSt(QObject *parent, int msec, std::function<void ()> task)
{
	staticPool().SingleShot(parent, msec, std::move(task));
}

void MyQTimerPool::SingleShotsSt(QObject *parent, MyQTimerPool::Tasks tasks)
{
	staticPool().SingleShots(parent, std::move(tasks));
}

void MyQTimerPool::SingleShot(QObject *parent, int msec, std::function<void ()> function)
{
	QTimer *timer;
	if(freeTimers.empty())
	{
		timer = new QTimer(parent);
		timer->setSingleShot(true);
		allTimers.insert(timer);
		QObject::connect(timer, &QObject::destroyed, [this, timer](){ freeTimers.erase(timer); allTimers.erase(timer); });
	}
	else
	{
		timer = *freeTimers.begin();
		freeTimers.erase(freeTimers.begin());
		timer->setParent(parent);
	}
	QObject::connect(timer, &QTimer::timeout, [this, timer, function = std::move(function)](){
		function();
		timer->setParent(nullptr);
		QObject::disconnect(timer, &QTimer::timeout, nullptr, nullptr);
		freeTimers.insert(timer);
	});
	timer->start(msec);
}

void MyQTimerPool::SingleShots(QObject *parent, MyQTimerPool::Tasks tasks)
{
	for(auto &[msec, function]:tasks)
	{
		SingleShot(parent, msec, std::move(function));
	}
}

void MyQTimerPool::ClearTimers()
{
	std::vector<QTimer*> timers { allTimers.begin(), allTimers.end() };
	// should work with copy of list, because original list changes when timer destroys

	for(auto &timer:timers)
	{
		timer->deleteLater();
	}
}

//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------
