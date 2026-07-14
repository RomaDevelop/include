#ifndef MyQProgressWindow_H
#define MyQProgressWindow_H

#include <QWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QByteArray>
#include <QEvent>
#include <QMoveEvent>
#include <QResizeEvent>

//--------------------------------------------------------------------------------------------------------------------------

class MyQProgressWindow : public QWidget {
public:
    inline static QByteArray geo = QByteArray();

    MyQProgressWindow(QWidget* parent, int minimum = 0, int maximum = 1000) 
        : QWidget() 
    {
        QObject::connect(parent, &QWidget::destroyed, this, &MyQProgressWindow::deleteLater);
        
        this->setWindowFlags(Qt::WindowType::Tool | Qt::WindowType::WindowStaysOnTopHint);
        this->setAttribute(Qt::WidgetAttribute::WA_ShowWithoutActivating);
        this->setWindowTitle("Active downloads progress");

        auto* layout = new QVBoxLayout(this);
        progress_bar = new QProgressBar();
        progress_bar->setRange(minimum, maximum);
        progress_bar->setValue(minimum);
        layout->addWidget(progress_bar);
    }

    void set_value(int value) {
        if (!this->isVisible()) {
            if (!MyQProgressWindow::geo.isEmpty()) {
                this->restoreGeometry(MyQProgressWindow::geo);
            }
            this->show();
        }
        this->progress_bar->setValue(value);
        if (this->progress_bar->value() >= this->progress_bar->maximum()) {
            this->adjustSize();
        }
    }

    void add_points(int count) {
        this->set_value(this->progress_bar->value() + count);
    }

protected:
    void moveEvent(QMoveEvent* event) override {
        MyQProgressWindow::geo = this->saveGeometry();
        QWidget::moveEvent(event);
    }

    void resizeEvent(QResizeEvent* event) override {
        MyQProgressWindow::geo = this->saveGeometry();
        QWidget::resizeEvent(event);
    }

private:
    QProgressBar* progress_bar;
};

//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
#endif
