#ifndef MyQScrollArea_H
#define MyQScrollArea_H

#include <QApplication>
#include <QDebug>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>

//---------------------------------------------------------------------------------------------------------------------

class MyQScrollArea : public QScrollArea
{
	Q_OBJECT

public:
	bool horisontalScrollingWithShift = true;

protected:
	inline virtual void wheelEvent(QWheelEvent *event) override;
};

//---------------------------------------------------------------------------------------------------------------------

void MyQScrollArea::wheelEvent(QWheelEvent * event)
{
	if(horisontalScrollingWithShift)
	{
		if (event->modifiers() & Qt::ShiftModifier)
		{
			QApplication::sendEvent(horizontalScrollBar(), event);
			return;
		}
	}

	QScrollArea::wheelEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------------------------------------------------
