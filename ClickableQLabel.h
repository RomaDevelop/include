#ifndef ClickableQLabel_H
#define ClickableQLabel_H

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

class ClickableQLabel : public QLabel {
	Q_OBJECT

public:
	explicit ClickableQLabel(QWidget *parent = nullptr) :
		QLabel(parent) {}
	explicit ClickableQLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags()) :
		QLabel(text, parent, f) {}
	virtual ~ClickableQLabel() = default;

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent *event) override {
		if (event->button() == Qt::LeftButton) emit clicked();

		QLabel::mousePressEvent(event);
	}
};

#endif
