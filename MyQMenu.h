#ifndef MyQMenu_h
#define MyQMenu_h

#include <QDebug>
#include <QMenu>

//--------------------------------------------------------------------------------------------------------------------------

struct MyQMenu
{
	inline static QAction* InsertActionAfter(QMenu* menu, QAction* targetAction, QAction* newAction);
};

//--------------------------------------------------------------------------------------------------------------------------

QAction *MyQMenu::InsertActionAfter(QMenu *menu, QAction *targetAction, QAction *newAction) {
	if (!menu || !targetAction || !newAction) return {};

	const QList<QAction*> actions = menu->actions();
	int index = actions.indexOf(targetAction);

	if (index != -1) {
		index++; // Ищем элемент, перед которым нужно вставить (индекс + 1)
		if(index < actions.size()) menu->insertAction(actions.at(index), newAction);
		else menu->addAction(newAction);
	}
	else {
		qDebug() << "MyQMenu::InsertActionAfter not found targetAction in menu";
	}

	return newAction;
}



//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------
