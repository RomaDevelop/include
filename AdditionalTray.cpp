#include "AdditionalTray.h"

#include "MyQString.h"
#include "MyQTextEdit.h"
#include "MyQLocalServer.h"

AdditionalTrayIcon::AdditionalTrayIcon(const QIcon &icon, QPoint globalPos, QWidget *parent)
	: ClickableQWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
	CreateLogsWidget(false);

	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setWindowFlag(Qt::WindowDoesNotAcceptFocus);
	setWindowFlag(Qt::NoDropShadowWindowHint);

	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

	QLabel *label = new QLabel(this);
	label->setPixmap(icon.pixmap(24, 24));
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(label);
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);

	adjustSize();
	move(globalPos);
	show();

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &AdditionalTrayIcon::SlotShowContextMenu);

	connect(this, &ClickableQWidget::clicked, [this](){ QTimer::singleShot(0,[this](){
			PlatformDependent::SetTopMost(this,true); }); });

	//QTimer *timerTopmoster = new QTimer(this);
	//connect(timerTopmoster, &QTimer::timeout, [this](){ PlatformDependent::SetTopMost(this,true); });
	//timerTopmoster->start(1000);

	existingIcons.insert(this);
}

AdditionalTrayIcon::~AdditionalTrayIcon()
{
	auto erased = existingIcons.erase(this);
	if(erased == 0) qdbg << "existingIcons didnt contained this";
	if(existingIcons.empty())
	{
		localServer1 = nullptr;
		localServer2 = nullptr;
		localClient = nullptr;
	}
}

void AdditionalTrayIcon::CreateMousePosShower(bool moveWithMouse, QWidget *parent)
{
	auto timerPosShow = new QTimer(parent);
	timerPosShow->start(300);
	connect(timerPosShow, &QTimer::timeout, [](){
		QPoint globalPos = QCursor::pos(); // Глобальные координаты
		QScreen *screen = QGuiApplication::screenAt(globalPos);

		if (screen)
		{
			QRect screenGeometry = screen->geometry();
			QPoint localPos = globalPos - screenGeometry.topLeft();
			qDebug() << "global:" << globalPos << "local:" << localPos;
		}
	});

	if(moveWithMouse)
	{
		auto timer = new QTimer(parent);
		timer->start(10);
		connect(timer, &QTimer::timeout, [this](){ this->move(QCursor::pos()); });
	}
}

#include <QPushButton>
void AdditionalTrayIcon::CreateLogsWidget(bool show)
{
	widget = std::make_unique<QWidget>();

	QVBoxLayout *vlo_main = new QVBoxLayout(widget.get());
	QHBoxLayout *hlo1 = new QHBoxLayout;
	QHBoxLayout *hlo2 = new QHBoxLayout;
	vlo_main->addLayout(hlo1);
	vlo_main->addLayout(hlo2);

	QPushButton *btnTestSend = new QPushButton("test write");
	hlo1->addWidget(btnTestSend);
	connect(btnTestSend,&QPushButton::clicked,[](){
		localClient->write("thing");
	});

	QPushButton *btnTestRequest = new QPushButton("test 2");
	hlo1->addWidget(btnTestRequest);
	connect(btnTestRequest,&QPushButton::clicked,[](){ });

	hlo1->addStretch();

	textEditLogs = new QTextEdit;
	hlo2->addWidget(textEditLogs);

	if(show) widget->show();

	widget->resize(800,600);
	QTimer::singleShot(100,[this](){ widget->activateWindow(); });
}

void AdditionalTrayIcon::Log(const QString &str)
{
	textEditLogs->append(str);
}

void AdditionalTrayIcon::Error(const QString &str)
{
	textEditLogs->append(str);
	MyQTextEdit::ColorizeLastCount(textEditLogs, Qt::red, str.size());
}

void AdditionalTrayIcon::Warning(const QString &str)
{
	textEditLogs->append(str);
	MyQTextEdit::ColorizeLastCount(textEditLogs, Qt::blue, str.size());
}
