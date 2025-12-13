//----------------------------------------------------------------------------------------------------------------
#ifndef MyQTabWidget_H
#define MyQTabWidget_H
//----------------------------------------------------------------------------------------------------------------
#include <functional>

#include <QMouseEvent>
#include <QTabBar>
#include <QTabWidget>
//----------------------------------------------------------------------------------------------------------------

class DetachFilter;

class MyQTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	explicit MyQTabWidget(QWidget *parent = nullptr) : QTabWidget(parent) {}
	virtual ~MyQTabWidget() = default;

	///\brief Enable mechanics tabs detaching when dragging them vertically
	/// disables detaching if detachDistance <= 0
	/// can be called repeatedly to reset detachDistance
	inline virtual void EnableDetaching(int detachDistance = 50);

private:
	DetachFilter *detachFilter {};
};

//----------------------------------------------------------------------------------------------------------------

class DetachFilter : public QObject
{
	Q_OBJECT

public:
	explicit DetachFilter(std::function<void(QPoint pos)> DetachEvent, QWidget *parent = nullptr):
		QObject(parent),
		DetachEvent{ std::move(DetachEvent) }
	{}

	int detachDistance = 50;

protected:
	///\brief Analyzes mouse events. If necessary, shows a widget preview, calls DetachEvent.
	inline bool eventFilter(QObject *obj, QEvent *event) override;

private:
	int startY;
	bool shouldCallDetach = false;
	std::function<void(QPoint pos)> DetachEvent;
	QWidget *widgetDetachedPreview {};
};

//----------------------------------------------------------------------------------------------------------------

void MyQTabWidget::EnableDetaching(int detachDistance)
{
	if(detachDistance > 0)
	{
		if(not detachFilter)
		{
			auto DetachEvent = [this](QPoint pos){
				QWidget* widgetToDetach = currentWidget();
				widgetToDetach->setParent(nullptr);
				QString tabTitle = tabText(currentIndex());

				pos = tabBar()->mapToGlobal(pos);
				pos.setY(pos.y()-10);
				pos.setX(pos.x()-(width()/2));
				widgetToDetach->move(pos);
				widgetToDetach->show();
			};

			detachFilter = new DetachFilter(std::move(DetachEvent), this);

			QTabBar* bar = tabBar();
			bar->installEventFilter(detachFilter);
		}

		detachFilter->detachDistance = detachDistance;
	}
	else
	{
		delete detachFilter;
		detachFilter = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------

bool DetachFilter::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress)
	{
		startY = static_cast<QMouseEvent*>(event)->y();
		shouldCallDetach = false;
	}
	if(event->type() == QEvent::MouseButtonRelease)
	{
		if(shouldCallDetach and DetachEvent) DetachEvent(static_cast<QMouseEvent*>(event)->pos());
		delete widgetDetachedPreview;
		widgetDetachedPreview = nullptr;
	}
	if(event->type() == QEvent::MouseMove)
	{
		auto mouseEvent = static_cast<QMouseEvent*>(event);
		int distance = abs(abs(startY) - abs(mouseEvent->y()));
		if(distance >= detachDistance)
		{
			shouldCallDetach = true;
			if(not widgetDetachedPreview)
			{
				widgetDetachedPreview = new QWidget;
				widgetDetachedPreview->setWindowFlags(Qt::FramelessWindowHint);
				widgetDetachedPreview->setStyleSheet("background-color: gray;");
				widgetDetachedPreview->resize(200,30);
				widgetDetachedPreview->show();
			}

			auto pos = static_cast<QWidget*>(obj)->mapToGlobal(mouseEvent->pos());
			pos.setX(pos.x()-100);
			pos.setY(pos.y()-15);
			widgetDetachedPreview->move(pos);
		}
		else
		{
			shouldCallDetach = false;
			delete widgetDetachedPreview;
			widgetDetachedPreview = nullptr;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------------------------------------
