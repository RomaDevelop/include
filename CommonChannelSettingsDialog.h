//--------------------------------------------------------------------------------------------------------------------------------
#ifndef CommonChannelSettingsDialog_H
#define CommonChannelSettingsDialog_H
//--------------------------------------------------------------------------------------------------------------------------------
#include <map>

#include <QString>
#include <QMessageBox>
#include <QComboBox>
#include <QRadioButton>

#include "MyQShortings.h"
//--------------------------------------------------------------------------------------------------------------------------------

class CommonChannelSettingsDialog
{
public:
	virtual ~CommonChannelSettingsDialog() = default;

protected:
	// map name and widget
	std::map<QString,QWidget*> widgetsWithStates;
	// map name and value
	inline std::map<QString,QString> CurrentSettingsStates();
	inline void SetFromSettingsStates(const std::map<QString,QString> &states);
};

//--------------------------------------------------------------------------------------------------------------------------------

std::map<QString, QString> CommonChannelSettingsDialog::CurrentSettingsStates()
{
	std::map<QString,QString> mapRes;
	for(auto &[name, widget]:widgetsWithStates)
	{
		if(auto widgetResolved = dynamic_cast<QComboBox*>(widget))
			mapRes[name] = widgetResolved->currentText();
		else if(auto widgetResolved = dynamic_cast<QRadioButton*>(widget))
			mapRes[name] = QSn(widgetResolved->isChecked());
		else QMbError(QString("COMSettingsDialog::CurrentSettingsStates unrealesed class ") + widget->metaObject()->className());
	}
	return mapRes;
}

void CommonChannelSettingsDialog::SetFromSettingsStates(const std::map<QString, QString> &states)
{
	for(auto &[name,text] : states)
	{
		if(auto it = widgetsWithStates.find(name); it != widgetsWithStates.end())
		{
			QWidget *widget = it->second;
			if(auto widgetResolved = dynamic_cast<QComboBox*>(widget))
				widgetResolved->setCurrentText(text);
			else if(auto widgetResolved = dynamic_cast<QRadioButton*>(widget))
				widgetResolved->setChecked(text.toShort());
			else QMbError(QString("COMSettingsDialog::SetFromSettingsStates unrealesed class ") + widget->metaObject()->className());
		}
		else QMbError("COMSettingsDialog::SetFromSettingsStates not found name " + name);
	}
}

//---------------------------------------------------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------------------------------------------------------
