#ifndef ADDITIONALTRAY_H
#define ADDITIONALTRAY_H

#include <memory>
#include <set>

#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QIcon>
#include <QTimer>
#include <QGuiApplication>
#include <QScreen>
#include <QMenu>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTextEdit>

#include "MyQShortings.h"
#include "PlatformDependent.h"
#include "ClickableQWidget.h"

class AdditionalTrayIcon : public ClickableQWidget
{
	Q_OBJECT

public:
	explicit AdditionalTrayIcon(const QIcon &icon);
	AdditionalTrayIcon(const AdditionalTrayIcon &) = delete;
	AdditionalTrayIcon(AdditionalTrayIcon &&) = delete;
	AdditionalTrayIcon& operator= (const AdditionalTrayIcon &) = delete;
	AdditionalTrayIcon& operator= (AdditionalTrayIcon &&) = delete;
	virtual ~AdditionalTrayIcon();

	///\brief Sets the specified menu to be the context menu for the AdditionalTrayIcon.
	///
	/// AdditionalTrayIcon does not take ownership of the menu.
	inline void setContextMenu(QMenu *menu) { this->menu = menu; }

	void CreateMousePosShower(bool moveWithMouse, QWidget *parent);
	void CreateLogsWidget(bool show);
	std::unique_ptr<QWidget> widgetLogs;
	QTextEdit *textEditLogs;
	void Log(const QString &str);
	void Error(const QString &str);
	void Warning(const QString &str);

	inline static std::function<void(QPoint pos)> fnClientGetCommandSetPos;
	inline static void CallFnClientGetCommandSetPos(QPoint pos);

private:
	QMenu *menu = nullptr;
	void SlotShowContextMenu(const QPoint& pos) {
		if(menu) menu->exec(mapToGlobal(pos));
		QTimer::singleShot(0,[this](){ PlatformDependent::SetTopMost(this,true); });
	}

	inline static std::set<AdditionalTrayIcon*> existingIcons;
};



#endif // ADDITIONALTRAY_H
