#ifndef ClickableQWidget_H
#define ClickableQWidget_H

#include <QMouseEvent>
#include <QWidget>

class ClickableQWidget : public QWidget {
	Q_OBJECT

public:
	explicit ClickableQWidget(QWidget *parent = nullptr) :
	    QWidget(parent) {}
	explicit ClickableQWidget(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags()) :
	    QWidget(parent, f) {}
	virtual ~ClickableQWidget() = default;

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent *event) override {
		if (event->button() == Qt::LeftButton) emit clicked();

		QWidget::mousePressEvent(event);
	}
};

#endif
