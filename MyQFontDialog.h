#ifndef MYQFONTDIALOG_H
#define MYQFONTDIALOG_H
//--------------------------------------------------------------------------------------------------------------------------

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontDatabase>
#include <QFontInfo>
#include <QGridLayout>
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

    inline static Result GetFont(const QFont &startFont = QFont(), QString caption = "Select font", QWidget *parent = nullptr);

private:
	inline static QStringList SizeValues(const QString &family);
	inline static int FindFirstRowStartsWith(QListWidget *listWidget, const QString &text);
	inline static int FindNearestSizeRow(QListWidget *listWidget, const QString &text);
	inline static void SelectRow(QListWidget *listWidget, int row);
	inline static void SyncEditWithList(QLineEdit *lineEdit, QListWidget *listWidget);
	inline static std::vector<std::pair<QString, QString>> SampleTexts();
};

//--------------------------------------------------------------------------------------------------------------------------

MyQFontDialog::Result MyQFontDialog::GetFont(const QFont &startFont, QString caption, QWidget *parent)
{
    Result result;
    result.font = startFont;

    QDialog dialog(parent);
    dialog.setWindowTitle(caption);

    QVBoxLayout *vloMain = new QVBoxLayout(&dialog);
    QGridLayout *gridMain = new QGridLayout;
    vloMain->addLayout(gridMain);

    QVBoxLayout *vloFamily = new QVBoxLayout;
    QLineEdit *editFamily = new QLineEdit;
    QListWidget *listFamily = new QListWidget;
    editFamily->setClearButtonEnabled(true);
    vloFamily->addWidget(new QLabel(QStringLiteral("Family")));
    vloFamily->addWidget(editFamily);
    vloFamily->addWidget(listFamily);
    gridMain->addLayout(vloFamily, 0, 0);

    QVBoxLayout *vloSize = new QVBoxLayout;
    QLineEdit *editSize = new QLineEdit;
    QListWidget *listSize = new QListWidget;
    editSize->setClearButtonEnabled(true);
    vloSize->addWidget(new QLabel(QStringLiteral("Size")));
    vloSize->addWidget(editSize);
    vloSize->addWidget(listSize);
    gridMain->addLayout(vloSize, 0, 1);

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
    gridMain->addLayout(vloFlags, 0, 2);

    QVBoxLayout *vloSampleInput = new QVBoxLayout;
    QLineEdit *editSampleText = new QLineEdit(QStringLiteral("AaBbYyZz"));
    QComboBox *comboSampleText = new QComboBox;
    editSampleText->setClearButtonEnabled(true);
    vloSampleInput->addWidget(new QLabel(QStringLiteral("Sample text")));
    vloSampleInput->addWidget(editSampleText);
    vloSampleInput->addWidget(comboSampleText);
    gridMain->addLayout(vloSampleInput, 1, 0);

    QVBoxLayout *vloPreview = new QVBoxLayout;
    QLabel *labelPreview = new QLabel;
    labelPreview->setAlignment(Qt::AlignCenter);
    labelPreview->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelPreview->setFrameShape(QFrame::StyledPanel);
    labelPreview->setFrameShadow(QFrame::Sunken);
    labelPreview->setAutoFillBackground(true);
    labelPreview->setMargin(6);

    QPalette previewPalette = labelPreview->palette();
    previewPalette.setColor(QPalette::Window, previewPalette.color(QPalette::Base));
    labelPreview->setPalette(previewPalette);
    vloPreview->addWidget(new QLabel(QStringLiteral("Preview")));
    vloPreview->addWidget(labelPreview, 1);
    gridMain->addLayout(vloPreview, 1, 1, 1, 2);

    gridMain->setColumnStretch(0, 2);
    gridMain->setColumnStretch(1, 1);
    gridMain->setColumnStretch(2, 0);
    gridMain->setRowStretch(0,1);
    gridMain->setRowStretch(1,0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    QPushButton *btnAccept = buttonBox->addButton(MyQDialogs::Accept, QDialogButtonBox::AcceptRole);
    QPushButton *btnCansel = buttonBox->addButton(MyQDialogs::Cansel, QDialogButtonBox::RejectRole);
    vloMain->addWidget(buttonBox);

    const QStringList families = QFontDatabase().families();
    listFamily->addItems(families);
    listFamily->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listSize->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    for(const auto &sample : SampleTexts()) comboSampleText->addItem(sample.first, sample.second);

    auto RebuildSizes = [listSize, editSize](const QString &family, int preferredSize)
    {
        const QStringList values = SizeValues(family);
        listSize->clear();
        listSize->addItems(values);

        int row = -1;
        if(preferredSize > 0) row = FindNearestSizeRow(listSize, QSn(preferredSize));
        if(row == -1 and !editSize->text().isEmpty()) row = FindNearestSizeRow(listSize, editSize->text());
        if(row == -1 and listSize->count()) row = 0;
        SelectRow(listSize, row);
        SyncEditWithList(editSize, listSize);
    };

    auto BuildFontFromDialog = [&]() {
        QFont font = startFont;

        if(auto familyItem = listFamily->currentItem(); familyItem)
            font.setFamily(familyItem->text());

        bool sizeOk = false;
        const int sizeValue = editSize->text().toInt(&sizeOk);
        if(sizeOk and sizeValue > 0) font.setPointSize(sizeValue);

        font.setStrikeOut(chStrikeOut->isChecked());
        font.setUnderline(chUnderline->isChecked());
        font.setItalic(chItalic->isChecked());
        font.setBold(chBold->isChecked());
        return font;
    };

    auto UpdatePreview = [&]() {
        labelPreview->setFont(BuildFontFromDialog());
        labelPreview->setText(editSampleText->text());
    };

    auto FamilyChanged = [listFamily, listSize, editSize, &RebuildSizes]()
    {
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

        if(auto item = listFamily->currentItem()) RebuildSizes(item->text(), preferredSize);
    };

    QObject::connect(listFamily, &QListWidget::currentRowChanged, &dialog, FamilyChanged);
    QObject::connect(listSize, &QListWidget::currentRowChanged, &dialog, [editSize, listSize]() {
        if(!listSize->hasFocus()) return;
        SyncEditWithList(editSize, listSize);
    });
    QObject::connect(listFamily, &QListWidget::currentRowChanged, &dialog, [editFamily, listFamily]() {
        if(!listFamily->hasFocus()) return;
        SyncEditWithList(editFamily, listFamily);
    });

    QObject::connect(editFamily, &QLineEdit::textEdited, &dialog, [listFamily](const QString &text) {
        SelectRow(listFamily, FindFirstRowStartsWith(listFamily, text));
    });
    QObject::connect(editSize, &QLineEdit::textEdited, &dialog, [listSize](const QString &text) {
        SelectRow(listSize, FindNearestSizeRow(listSize, text));
    });
    QObject::connect(editSampleText, &QLineEdit::textChanged, &dialog, UpdatePreview);
    QObject::connect(comboSampleText, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog,
                     [comboSampleText, editSampleText, UpdatePreview](int index) {
        if(index < 0) return;
        const QString value = comboSampleText->itemData(index).toString();
        if(editSampleText->text() != value) editSampleText->setText(value);
        else UpdatePreview();
    });
    QObject::connect(listFamily, &QListWidget::currentRowChanged, &dialog, UpdatePreview);
    QObject::connect(listSize, &QListWidget::currentRowChanged, &dialog, UpdatePreview);
    QObject::connect(editFamily, &QLineEdit::textEdited, &dialog, UpdatePreview);
    QObject::connect(editSize, &QLineEdit::textEdited, &dialog, UpdatePreview);
    QObject::connect(chStrikeOut, &QCheckBox::toggled, &dialog, UpdatePreview);
    QObject::connect(chUnderline, &QCheckBox::toggled, &dialog, UpdatePreview);
    QObject::connect(chItalic, &QCheckBox::toggled, &dialog, UpdatePreview);
    QObject::connect(chBold, &QCheckBox::toggled, &dialog, UpdatePreview);

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

        result.font = BuildFontFromDialog();
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
    RebuildSizes((listFamily->currentItem() ? listFamily->currentItem()->text() : QString()), startPointSize);

    chStrikeOut->setChecked(startFont.strikeOut());
    chUnderline->setChecked(startFont.underline());
    chItalic->setChecked(startFont.italic());
    chBold->setChecked(startFont.bold());
    UpdatePreview();

    dialog.exec();
    return result;
}

std::vector<std::pair<QString, QString>> MyQFontDialog::SampleTexts()
{
	return {
		{QStringLiteral("Arabic"), QStringLiteral("ابتثجحخ دذرزسشصض")},
		{QStringLiteral("Armenian"), QStringLiteral("ԱաԲբԳգԴդԵեԶզ")},
		{QStringLiteral("Cyrillic"), QStringLiteral("АаБбВвГгДд ЕеЖжЗз")},
		{QStringLiteral("Digits"), QStringLiteral("0123456789")},
		{QStringLiteral("Greek"), QStringLiteral("ΑαΒβΓγΔδ ΕεΖζΗηΘθ")},
		{QStringLiteral("Hex"), QStringLiteral("0123456789ABCDEF abcdef")},
		{QStringLiteral("Latin"), QStringLiteral("AaBbYyZz")},
		{QStringLiteral("Latin Extended"), QStringLiteral("ÀàÄäÇçÑñØøŽž")},
		{QStringLiteral("Mixed"), QStringLiteral("AaБбΓγԱա123")},
		{QStringLiteral("Symbols"), QStringLiteral("@#&%!?+-=*/()[]{}")}
	};
}

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

//--------------------------------------------------------------------------------------------------------------------------
#endif
