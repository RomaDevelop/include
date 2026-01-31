#ifndef MyQTimer_H
#define MyQTimer_H

#include <queue>
#include <memory>
#include <functional>

#include <QTimer>

class MyQTimerPool
{
public:
	using Tasks = std::vector<std::pair<int, std::function<void()>>>;

	static void SingleShotSt(int msec, std::function<void()> task) { staticPool().SingleShot(msec, std::move(task)); }
	static void SingleShotsSt(Tasks tasks) { staticPool().SingleShots(std::move(tasks)); }

	void SingleShot(int msec, std::function<void()> function)
	{
		QTimer *timer;
		if(freeTimers.empty())
		{
			timer = new QTimer;
			timer->setSingleShot(true);
			count++;
		}
		else
		{
			timer = freeTimers.front();
			freeTimers.pop();
		}
		QObject::connect(timer, &QTimer::timeout, [this, timer, function = std::move(function)](){
			function();
			QObject::disconnect(timer, &QTimer::timeout, nullptr, nullptr);
			freeTimers.push(timer);
		});
		timer->start(msec);
	}
	void SingleShots(Tasks tasks)
	{
		for(auto &[msec, function]:tasks)
		{
			SingleShot(msec, std::move(function));
		}
	}
	void ClearFreeTimers()
	{
		while(!freeTimers.empty())
		{
			freeTimers.front()->deleteLater();
			freeTimers.pop();
		}
	}

	~MyQTimerPool()
	{
		ClearFreeTimers();
	}

	int count = 0;

private:
	std::queue<QTimer*> freeTimers;
	static MyQTimerPool& staticPool() { static MyQTimerPool pool; return pool; }
};

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


#endif
