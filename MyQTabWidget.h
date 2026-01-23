//----------------------------------------------------------------------------------------------------------------
#ifndef MyQTabWidget_H
#define MyQTabWidget_H
//--------------------------------------------------------------------------------------------------------------------------
#include <functional>

#include <QDebug>
#include <QMouseEvent>
#include <QTabBar>
#include <QTabWidget>
//--------------------------------------------------------------------------------------------------------------------------

class DetachFilter;

class MyQTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	explicit MyQTabWidget(QWidget *parent = nullptr) : QTabWidget(parent) { setAcceptDrops(true); }
	virtual ~MyQTabWidget() = default;

	///\brief Enable mechanics tabs detaching when dragging them vertically
	/// disables detaching if detachDistance <= 0
	/// can be called repeatedly to reset detachDistance
	inline virtual void EnableDetaching(int detachDistance = 50);

	inline bool IsOverTabs(const QPoint &globalPos);
	inline void UpdatePlaceholder(const QPoint &globalPos, const QString &text);
	inline void ReplacePlaceholder(QString caption, QWidget *window);

private:
	DetachFilter *detachFilter {};

	QWidget *tabPlaceholder = {};
};

//--------------------------------------------------------------------------------------------------------------------------

class DetachableWindow : public QWidget {
	Q_OBJECT
public:
	inline explicit DetachableWindow(MyQTabWidget *tabWidget, std::function<QString()> GetTabCaption, QWidget *parent = nullptr);
	~DetachableWindow() = default;

protected:
	inline bool event(QEvent *event) override;
	inline void mouseReleaseEvent(QMouseEvent *event) override;

private:
	MyQTabWidget *m_targetTabWidget = nullptr;
	std::function<QString()> GetTabCaption;
	bool m_isDragging = false;
};

//--------------------------------------------------------------------------------------------------------------------------

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

bool MyQTabWidget::IsOverTabs(const QPoint &globalPos)
{
	QPoint localPos = tabBar()->mapFromGlobal(globalPos);
	return tabBar()->rect().contains(localPos);
}

void MyQTabWidget::UpdatePlaceholder(const QPoint &globalPos, const QString &text) {
	// Если мышь НЕ над таб-баром — удаляем заглушку и выходим
	if(!IsOverTabs(globalPos)) {
		if (tabPlaceholder) {
			delete tabPlaceholder;
			tabPlaceholder = nullptr;
		}
		return;
	}

	QPoint localPos = tabBar()->mapFromGlobal(globalPos);

	// Если мышь над таб-баром, а заглушки еще нет — создаем
	if (!tabPlaceholder) {
		tabPlaceholder = new QWidget();
		int index = tabBar()->tabAt(localPos);
		if (index == -1) index = count();
		insertTab(index, tabPlaceholder, text);
		setCurrentWidget(tabPlaceholder);
	} else {
		// Если заглушка уже есть, просто двигаем её под курсор
		int currentIndex = indexOf(tabPlaceholder);
		int targetIndex = tabBar()->tabAt(localPos);
		if (targetIndex != -1 && targetIndex != currentIndex) {
			tabBar()->moveTab(currentIndex, targetIndex);
		}
	}
}

void MyQTabWidget::ReplacePlaceholder(QString caption, QWidget *window)
{
	int index = 0;
	if(tabPlaceholder) index = indexOf(tabPlaceholder);
	else qDebug() << "ReplacePlaceholder called for invalid placeholder";
	if(index < 0) { qDebug() << "ReplacePlaceholder, but m_placeholder has invalid index"; index = 0; }
	insertTab(index, window, caption);
	setCurrentIndex(index);
	delete tabPlaceholder;
	tabPlaceholder = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------------------------------

DetachableWindow::DetachableWindow(MyQTabWidget *tabWidget, std::function<QString ()> GetTabCaption, QWidget *parent) :
	QWidget(parent) ,
	m_targetTabWidget { tabWidget },
	GetTabCaption { GetTabCaption }
{}

bool DetachableWindow::event(QEvent *event) {
	// Обработка нажатия кнопки на заголовке
	if (event->type() == QEvent::NonClientAreaMouseButtonPress) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			// Тут нужна ещё проверка, что клик именно в заголовок, а не в иной NonClientAreaMouseButtonPress
			m_isDragging = true;
		}
	}

	if (event->type() == QEvent::Move) {
		// обрабатываем именно Move, потому что когда мы держим заголовок,
		// NonClientAreaMouseButtonMove не вызывается (возможно потому что мышь не двигается относительно окна)
		if (m_targetTabWidget and m_isDragging)
		{
			m_targetTabWidget->UpdatePlaceholder(QCursor::pos(), GetTabCaption());

			if(m_targetTabWidget->IsOverTabs(QCursor::pos())) setWindowOpacity(0.3); // Окно станет прозрачным
			else setWindowOpacity(1);
		}
	}

	// Обработка отпускания кнопки на заголовке
	if (event->type() == QEvent::NonClientAreaMouseButtonRelease) {
		if(m_isDragging)
		{
			if(m_targetTabWidget->IsOverTabs(QCursor::pos()))
			{
				setWindowOpacity(1);
				m_targetTabWidget->ReplacePlaceholder(GetTabCaption(), this);
			}
		}
		m_isDragging = false;
	}

	return QWidget::event(event);
}

void DetachableWindow::mouseReleaseEvent(QMouseEvent *event) {
	m_isDragging = false;
	QWidget::mouseReleaseEvent(event);
}

//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------
