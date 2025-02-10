#ifndef MyQTimer_H
#define MyQTimer_H

#include <QTimer>

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
