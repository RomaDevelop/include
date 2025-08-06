#ifndef MyQTimer_H
#define MyQTimer_H

#include <functional>
#include <queue>

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



#endif // MYQTABLEWIDGET_H
