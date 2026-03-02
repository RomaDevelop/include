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
	///\brief Scrolls horisontal if vertical scroll is not visible or with shift pressed
	bool smartHorisontalScrolling = true;

protected:
	inline virtual void wheelEvent(QWheelEvent *event) override;
};

//---------------------------------------------------------------------------------------------------------------------

void MyQScrollArea::wheelEvent(QWheelEvent * event)
{
	if(smartHorisontalScrolling and horizontalScrollBar()->isVisible())
	{
		if(not verticalScrollBar()->isVisible() or event->modifiers() & Qt::ShiftModifier)
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
