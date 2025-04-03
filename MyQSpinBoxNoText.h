#ifndef MyQSpinBoxNoText_H
#define MyQSpinBoxNoText_H

#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QStyleOptionSpinBox>
#include <QPainter>
#include <QApplication>
#include <QLabel>
#include <QSpinBox>
#include <QProxyStyle>

#include "MyQShortings.h"

class MyQSpinBoxNoText; // inherits QWidget
class QSpinBoxNoText;	// inherits QSpinBox

class MyQSpinBoxNoText : public QWidget
{
	Q_OBJECT
public:
	inline static void ShowDifferent();

	explicit MyQSpinBoxNoText(QWidget *parent = nullptr) :
		QWidget(parent),
		btnUp {new QToolButton(this)},
		btnDown {new QToolButton(this)}

	{
		auto size = sizeHint();
		resize(size);
		resizeButtons();

		//btnUp->setArrowType(Qt::UpArrow); // к сожалению так выводится криво
		//btnDown->setArrowType(Qt::DownArrow); // к сожалению так выводится криво
	}

	~MyQSpinBoxNoText() = default;

protected:
	void resizeEvent(QResizeEvent *) override {
		resizeButtons();
	}
	void paintEvent(QPaintEvent *) override {
		if(!imageShowed)
		{
			auto sizePixmap = QSize(size().width()/2, size().height()/2);

			pixmapUp = QApplication::style()->standardIcon(QStyle::SP_TitleBarShadeButton).pixmap(sizePixmap);
			pixmapDown = QApplication::style()->standardIcon(QStyle::SP_TitleBarUnshadeButton).pixmap(sizePixmap);

			labelPixmapUp = new QLabel(this);
			labelPixmapUp->setPixmap(pixmapUp);
			labelPixmapUp->resize(sizePixmap);
			labelPixmapUp->setAttribute(Qt::WA_TransparentForMouseEvents); // Клики проходят сквозь него!
			labelPixmapUp->raise(); // Помещаем поверх кнопок

			labelPixmapDown = new QLabel(this);
			labelPixmapDown->setPixmap(pixmapDown);
			labelPixmapDown->resize(sizePixmap);
			labelPixmapDown->setAttribute(Qt::WA_TransparentForMouseEvents); // Клики проходят сквозь него!
			labelPixmapDown->raise(); // Помещаем поверх кнопок

			labelPixmapUp->move((this->width()/2) - (labelPixmapUp->width()/2), 0);
			labelPixmapDown->move((this->width()/2) - (labelPixmapDown->width()/2), this->height()/2);

			qdbg << this->width() << labelPixmapUp->width() << labelPixmapUp->x();

			labelPixmapUp->setVisible(true);
			labelPixmapDown->setVisible(true);

			imageShowed = true;
		}
	}
	QSize sizeHint() const override {
		return QSize(btnUp->sizeHint().width(), btnUp->sizeHint().height());
	}

private:
	QToolButton *btnUp;
	QToolButton *btnDown;
	void resizeButtons()
	{
		int halfHeight = size().height()/2;
		btnUp->setGeometry(0,0,size().width(),halfHeight+1);
		btnDown->setGeometry(0,halfHeight-1,size().width(),halfHeight+1);
	}

	QPixmap pixmapUp;
	QPixmap pixmapDown;
	QLabel *labelPixmapUp;
	QLabel *labelPixmapDown;
	bool imageShowed = false;
};

class QSpinBoxNoText : public QSpinBox {
public:
	QSpinBoxNoText(QWidget* parent = nullptr) : QSpinBox(parent) {
		// Устанавливаем стиль, который скроет текстовое поле
		this->setStyle(new SpinBoxStyle());
	}

	QSize sizeHint() const override {
		return QSize(16, QSpinBox::sizeHint().height()); // Ширина 16px, высота стандартная
	}

public:
	class SpinBoxStyle : public QProxyStyle {
	public:
		void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
							  QPainter* painter, const QWidget* widget) const override {
			if (control == CC_SpinBox) {
				//QStyleOptionComplex opt = *option;
				QStyleOptionSpinBox opt = *qstyleoption_cast<const QStyleOptionSpinBox*>(option);
				//opt.subControls &= ~SC_SpinBoxEditField; // с обводкой
				opt.subControls = SC_SpinBoxUp | SC_SpinBoxDown; // без обводки
				QProxyStyle::drawComplexControl(control, &opt, painter, widget);
				return;
			}
			QProxyStyle::drawComplexControl(control, option, painter, widget);
		}

		QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option,
							SubControl subControl, const QWidget* widget) const override {
			if (control == CC_SpinBox) {
				const QStyleOptionSpinBox* spinBoxOpt = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
				if (!spinBoxOpt)
					return QProxyStyle::subControlRect(control, option, subControl, widget);

				// Получаем общий прямоугольник спинбокса
				QRect rect = spinBoxOpt->rect;

				if (subControl == SC_SpinBoxEditField) {
					return QRect(); // Скрываем текстовое поле
				}
				else if (subControl == SC_SpinBoxUp || subControl == SC_SpinBoxDown) {
					// Делим пространство пополам между кнопками
					int buttonHeight = rect.height() / 2;
					if (subControl == SC_SpinBoxUp) {
						return QRect(rect.left(), rect.top(), rect.width(), buttonHeight);
					} else {
						return QRect(rect.left(), rect.top() + buttonHeight, rect.width(), rect.height() - buttonHeight);
					}
				}
			}
			return QProxyStyle::subControlRect(control, option, subControl, widget);
		}
	};
};

//------------------------------------------------------------------------------------------------------------------------------

void MyQSpinBoxNoText::ShowDifferent()
{
	QWidget *w = new QWidget;
	auto parent = w;

	QSpinBoxNoText* spinBox2 = new QSpinBoxNoText(parent);
	spinBox2->move(10,10);

	QSpinBoxNoText* spinBox3 = new QSpinBoxNoText(parent);
	spinBox3->resize(32,32);
	spinBox3->move(50,10);

	QSpinBoxNoText* spinBox4 = new QSpinBoxNoText(parent);
	spinBox4->resize(50,50);
	spinBox4->move(100,10);

	auto mspBox = new MyQSpinBoxNoText(parent);
	mspBox->move(10,150);

	auto mspBox2 = new MyQSpinBoxNoText(parent);
	mspBox2->resize(32,32);
	mspBox2->move(50,150);

	auto mspBox3 = new MyQSpinBoxNoText(parent);
	mspBox3->resize(50,50);
	mspBox3->move(100,150);

	w->resize(500,500);
	w->show();
}

//------------------------------------------------------------------------------------------------------------------------------
#endif
