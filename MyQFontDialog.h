#ifndef MYQFONTDIALOG_H
#define MYQFONTDIALOG_H
//--------------------------------------------------------------------------------------------------------------------------
#include <climits>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontDatabase>
#include <QFontInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include "MyQDialogs.h"
#include "MyQShortings.h"
//--------------------------------------------------------------------------------------------------------------------------
class MyQFontDialog
{
public:
	struct Result
	{
		QFont font;
		bool accepted = false;
	};

	inline static Result GetFontExt(const QFont &startFont = QFont(), QWidget *parent = nullptr,
									QString caption = QStringLiteral("Font"), uint w = 900, uint h = 520);
	inline static QFont GetFont(const QFont &startFont = QFont(), QWidget *parent = nullptr,
								QString caption = QStringLiteral("Font"), bool *accepted = nullptr,
								uint w = 900, uint h = 520);

private:
	inline static QStringList SizeValues(const QString &family);
	inline static int FindFirstRowStartsWith(QListWidget *listWidget, const QString &text);
	inline static int FindNearestSizeRow(QListWidget *listWidget, const QString &text);
	inline static void SelectRow(QListWidget *listWidget, int row);
	inline static void SyncEditWithList(QLineEdit *lineEdit, QListWidget *listWidget);
	inline static void FillList(QListWidget *listWidget, const QStringList &values);
};
//--------------------------------------------------------------------------------------------------------------------------
QStringList MyQFontDialog::SizeValues(const QString &family)
{
	QFontDatabase fontDatabase;
	QList<int> sizes = fontDatabase.smoothSizes(family, QString());
	if(sizes.isEmpty()) sizes = fontDatabase.pointSizes(family, QString());
	if(sizes.isEmpty()) sizes = QFontDatabase::standardSizes();

	QStringList result;
	result.reserve(sizes.size());
	for(int size : sizes) result += QSn(size);
	return result;
}

int MyQFontDialog::FindFirstRowStartsWith(QListWidget *listWidget, const QString &text)
{
	if(!listWidget) return -1;
	if(text.isEmpty()) return (listWidget->count() ? 0 : -1);

	for(int row = 0; row < listWidget->count(); row++)
	{
		if(auto item = listWidget->item(row); item and item->text().startsWith(text, Qt::CaseInsensitive))
			return row;
	}

	for(int row = 0; row < listWidget->count(); row++)
	{
		if(auto item = listWidget->item(row); item and item->text().contains(text, Qt::CaseInsensitive))
			return row;
	}

	return -1;
}

int MyQFontDialog::FindNearestSizeRow(QListWidget *listWidget, const QString &text)
{
	if(!listWidget or !listWidget->count()) return -1;
	if(text.isEmpty()) return 0;

	bool ok = false;
	const int value = text.toInt(&ok);
	if(!ok) return FindFirstRowStartsWith(listWidget, text);

	int bestRow = 0;
	int bestDistance = INT_MAX;
	for(int row = 0; row < listWidget->count(); row++)
	{
		bool currentOk = false;
		const int currentValue = listWidget->item(row)->text().toInt(&currentOk);
		if(!currentOk) continue;

		const int distance = qAbs(currentValue - value);
		if(distance < bestDistance or (distance == bestDistance and currentValue >= value))
		{
			bestDistance = distance;
			bestRow = row;
		}
	}
	return bestRow;
}

void MyQFontDialog::SelectRow(QListWidget *listWidget, int row)
{
	if(!listWidget) return;
	if(row < 0 or row >= listWidget->count()) return;

	listWidget->setCurrentRow(row);
	if(auto item = listWidget->item(row))
		listWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
}

void MyQFontDialog::SyncEditWithList(QLineEdit *lineEdit, QListWidget *listWidget)
{
	if(!lineEdit or !listWidget) return;
	if(auto item = listWidget->currentItem())
	{
		QSignalBlocker blocker(lineEdit);
		lineEdit->setText(item->text());
		lineEdit->setCursorPosition(0);
	}
}

void MyQFontDialog::FillList(QListWidget *listWidget, const QStringList &values)
{
	if(!listWidget) return;
	listWidget->clear();
	listWidget->addItems(values);
}

MyQFontDialog::Result MyQFontDialog::GetFontExt(const QFont &startFont, QWidget *parent, QString caption, uint w, uint h)
{
	Result result;
	result.font = startFont;

	QDialog dialog(parent);
	dialog.setWindowTitle(caption);
	dialog.resize(w, h);

	QVBoxLayout *vloMain = new QVBoxLayout(&dialog);
	QHBoxLayout *hloTop = new QHBoxLayout;
	vloMain->addLayout(hloTop);

	QVBoxLayout *vloFamily = new QVBoxLayout;
	QLineEdit *editFamily = new QLineEdit;
	QListWidget *listFamily = new QListWidget;
	editFamily->setClearButtonEnabled(true);
	vloFamily->addWidget(new QLabel(QStringLiteral("Family")));
	vloFamily->addWidget(editFamily);
	vloFamily->addWidget(listFamily);
	hloTop->addLayout(vloFamily, 1);

	QVBoxLayout *vloFlags = new QVBoxLayout;
	QCheckBox *chStrikeOut = new QCheckBox(QStringLiteral("Strike out"));
	QCheckBox *chUnderline = new QCheckBox(QStringLiteral("Underline"));
	QCheckBox *chItalic = new QCheckBox(QStringLiteral("Italic"));
	QCheckBox *chBold = new QCheckBox(QStringLiteral("Bold"));
	vloFlags->addWidget(new QLabel(QStringLiteral("Style")));
	vloFlags->addWidget(chStrikeOut);
	vloFlags->addWidget(chUnderline);
	vloFlags->addWidget(chItalic);
	vloFlags->addWidget(chBold);
	vloFlags->addStretch();
	hloTop->addLayout(vloFlags);

	QVBoxLayout *vloSize = new QVBoxLayout;
	QLineEdit *editSize = new QLineEdit;
	QListWidget *listSize = new QListWidget;
	editSize->setClearButtonEnabled(true);
	vloSize->addWidget(new QLabel(QStringLiteral("Size")));
	vloSize->addWidget(editSize);
	vloSize->addWidget(listSize);
	hloTop->addLayout(vloSize);

	QDialogButtonBox *buttonBox = new QDialogButtonBox;
	QPushButton *btnAccept = buttonBox->addButton(MyQDialogs::Accept, QDialogButtonBox::AcceptRole);
	QPushButton *btnCansel = buttonBox->addButton(MyQDialogs::Cansel, QDialogButtonBox::RejectRole);
	vloMain->addWidget(buttonBox);

	const QStringList families = QFontDatabase().families();
	FillList(listFamily, families);
	listFamily->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	listSize->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	auto rebuildSizes = [listSize, editSize](const QString &family, int preferredSize)
	{
		const QStringList values = SizeValues(family);
		FillList(listSize, values);

		int row = -1;
		if(preferredSize > 0) row = FindNearestSizeRow(listSize, QSn(preferredSize));
		if(row == -1 and !editSize->text().isEmpty()) row = FindNearestSizeRow(listSize, editSize->text());
		if(row == -1 and listSize->count()) row = 0;
		SelectRow(listSize, row);
		SyncEditWithList(editSize, listSize);
	};

	auto familyChanged = [listFamily, editFamily, listSize, editSize, &rebuildSizes]()
	{
		SyncEditWithList(editFamily, listFamily);

		int preferredSize = -1;
		if(auto item = listSize->currentItem())
		{
			bool ok = false;
			preferredSize = item->text().toInt(&ok);
			if(!ok) preferredSize = -1;
		}
		if(preferredSize <= 0)
		{
			bool ok = false;
			preferredSize = editSize->text().toInt(&ok);
			if(!ok) preferredSize = -1;
		}

		if(auto item = listFamily->currentItem()) rebuildSizes(item->text(), preferredSize);
	};

	QObject::connect(listFamily, &QListWidget::currentRowChanged, &dialog, familyChanged);
	QObject::connect(listSize, &QListWidget::currentRowChanged, &dialog, [editSize, listSize]() {
		SyncEditWithList(editSize, listSize);
	});

	QObject::connect(editFamily, &QLineEdit::textChanged, &dialog, [listFamily](const QString &text) {
		SelectRow(listFamily, FindFirstRowStartsWith(listFamily, text));
	});
	QObject::connect(editSize, &QLineEdit::textChanged, &dialog, [listSize](const QString &text) {
		SelectRow(listSize, FindNearestSizeRow(listSize, text));
	});

	QObject::connect(listFamily, &QListWidget::itemDoubleClicked, &dialog, [btnAccept]() { btnAccept->click(); });
	QObject::connect(listSize, &QListWidget::itemDoubleClicked, &dialog, [btnAccept]() { btnAccept->click(); });

	QObject::connect(btnAccept, &QPushButton::clicked, &dialog, [&]() {
		auto familyItem = listFamily->currentItem();
		auto sizeItem = listSize->currentItem();

		if(!familyItem) { QMbError(QStringLiteral("Choose font family")); return; }
		if(!sizeItem) { QMbError(QStringLiteral("Choose font size")); return; }

		bool ok = false;
		const int sizeValue = sizeItem->text().toInt(&ok);
		if(!ok or sizeValue <= 0) { QMbError(QStringLiteral("Wrong font size")); return; }

		result.font.setFamily(familyItem->text());
		result.font.setPointSize(sizeValue);
		result.font.setStrikeOut(chStrikeOut->isChecked());
		result.font.setUnderline(chUnderline->isChecked());
		result.font.setItalic(chItalic->isChecked());
		result.font.setBold(chBold->isChecked());
		result.accepted = true;
		dialog.accept();
	});
	QObject::connect(btnCansel, &QPushButton::clicked, &dialog, [&dialog]() { dialog.reject(); });

	const QString startFamily = startFont.family();
	int familyRow = FindFirstRowStartsWith(listFamily, startFamily);
	if(familyRow == -1 and !families.isEmpty()) familyRow = 0;
	SelectRow(listFamily, familyRow);
	if(editFamily->text().isEmpty() and !startFamily.isEmpty()) editFamily->setText(startFamily);

	const int startPointSize = (startFont.pointSize() > 0 ? startFont.pointSize() : QFontInfo(startFont).pointSize());
	rebuildSizes((listFamily->currentItem() ? listFamily->currentItem()->text() : QString()), startPointSize);

	chStrikeOut->setChecked(startFont.strikeOut());
	chUnderline->setChecked(startFont.underline());
	chItalic->setChecked(startFont.italic());
	chBold->setChecked(startFont.bold());

	dialog.exec();
	return result;
}

QFont MyQFontDialog::GetFont(const QFont &startFont, QWidget *parent, QString caption, bool *accepted, uint w, uint h)
{
	const Result result = GetFontExt(startFont, parent, std::move(caption), w, h);
	if(accepted) *accepted = result.accepted;
	return result.font;
}
//--------------------------------------------------------------------------------------------------------------------------
#endif
