#include "AdditionalTray.h"

#include "MyQString.h"
#include "MyQTextEdit.h"
#include "MyQLocalServer.h"

AdditionalTrayIcon::AdditionalTrayIcon(const QIcon &icon, QPoint globalPos, QWidget *parent)
	: ClickableQWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
	qdbg << "AdditionalTrayIcon to do: "
/*			""
			"в конструктор не должны передаваться координаты "
			"должен передаваться дисплей на котором создать иконки "
			"а координаты должны расчитываться автоматом "
			"если при этом приложение с таким модулем должно локально опрашивать других - есть ли их иконки "
			"чтобы они не накладывались друг на друга "
			""
			"а вообще можно придумать как-то чтобы этот доп.трей был отдельным приложением, но как он тогда"
			"будет вызывать конкретные функции из этого приложения?"
			""
			"например так: должна работать программа AddTray(сервер)"
			"наша программа(клиент) регистрирует в ней трей иконку с нужными параметрами"
			"AddTray добавляет её как надо"
			"затем при взаимодействии с той иконкой в нашу программу приходят сведения что нужно сделать"
			"при отключении сервер удаляет иконку"
			"	тот же минус - еще одна программа, еще один процесс"*/;

	CreateLogsWidget(false);

	if(!localClient && !localServer1)
	{
		localClient = MyQLocalServer::InitSocket("AdditionalTrayServer", 100,
													[this](QByteArray data){Log("localClient get " + data); },
													[this](QString log){ Log(std::move(log)); });
		if(localClient) Log("localClient created " + MyQString::AsDebug(localClient.get()));
		else
		{
			localServer1 = MyQLocalServer::InitServer("AdditionalTrayServer",
														[this](QByteArray data){ Log("localServer1 get " + data); },
														[this](QString log){ Log(std::move(log)); });
			if(localServer1) Log("localServer1 created " + MyQString::AsDebug(localServer1.get()));
			else Error("cant create sever and client");
		}
	}

	/// MyQLocalServer переделать на u_ptr
	///
	/// в программе должен быть и сервер и клиент
	/// если клиент получает сигнал о том, что он отвалился, он пробует завести сервер
	/// проблема - это сделают одновременно несколько клиентов
	///		решение 1 сервер не в клиенте, а стороннее приложение, которое клиент инициирует
	///						сервер должен сам выключаться если он никому не нужен
	///		решение 2 сервер в клиенте, но он своим клиентам назначает тайминги ожидания
	///						для попыток переподключения и подъема сервера -
	///		решение 2 предпочтительнее,
	///			во первых не нужно создавать отдельное приложение,
	///			во вторых систему будет меньше грузить
	///			в третьих в диспетчере задач меньше говна
	///		далее сделать функцию - запрос всех координат иконок на сервер
	///		сервер опрашивает всех клиентов
	///		клиенты отвечают
	///		сервер передает обратно запросившему
	///		на основании полученного запросивший располагает иконку


	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setWindowFlag(Qt::WindowDoesNotAcceptFocus);
	setWindowFlag(Qt::NoDropShadowWindowHint);

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

	connect(this, &ClickableQWidget::clicked, [this](){ QTimer::singleShot(0,[this](){ PlatformDependent::SetTopMost(this,true); }); });

	QTimer *timerTopmoster = new QTimer(this);
	connect(timerTopmoster, &QTimer::timeout, [this](){ PlatformDependent::SetTopMost(this,true); });
	timerTopmoster->start(1000);

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
