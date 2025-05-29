#ifndef INPUTBLOCKER_H
#define INPUTBLOCKER_H

#include <QObject>
#include <QEvent>

class InputBlocker : public QObject {
public:
	explicit InputBlocker(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~InputBlocker() {}

protected:
	bool eventFilter(QObject *obj, QEvent *event) override {
		switch (event->type()) {
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseMove:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			case QEvent::Wheel:
			case QEvent::TouchBegin:
			case QEvent::TouchUpdate:
			case QEvent::TouchEnd:
				return true; // Заблокировать событие
			default:
				return QObject::eventFilter(obj, event);
		}
	}
};

#endif // INPUTBLOCKER_H
