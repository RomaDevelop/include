#include "AdditionalTray.h"

#include "MyQString.h"
#include "MyQTextEdit.h"
#include "MyQLocalServer.h"

std::shared_ptr<MyQLocalServer> localServer;
std::shared_ptr<QLocalSocket> localClient;

QString netName = "AdditionalTrayIcon";
std::function<void(QString log)> logWorkerSock = [](QString log){ qdbg << "sock: "+log; };
std::function<void(QString log)> logWorkerServ = [](QString log){ qdbg << "serv: "+log; };

const char * request_for_pos = "request_for_pos";
QString command_set_pos = "command_set_pos:";
const char * not_need_add_tray = "not_need_add_tray";

bool InitClient()
{
	auto worker = [](QByteArray arr){
		if(arr == not_need_add_tray)
		{
			AdditionalTrayIcon::CallFnClientGetCommandSetPos({});
		}
		else if(arr.startsWith(command_set_pos.toUtf8()))
		{
			if(not arr.endsWith(";")) { QMbError("Not finished data: "+arr); return; }

			QString answ_pos_str = arr;
			answ_pos_str.remove(0, command_set_pos.size());
			answ_pos_str.chop(1);

			auto xy = answ_pos_str.split(":");
			if(xy.size() != 2) { QMbError("Invalind data (count): "+arr); return; }

			bool ok1, ok2;
			int x = xy[0].toInt(&ok1);
			int y = xy[1].toInt(&ok2);
			if(not ok1 or not ok2)  { QMbError("Invalind data (value): "+arr); return; }
			QPoint posFromServer(x,y);

			AdditionalTrayIcon::CallFnClientGetCommandSetPos(posFromServer);
			//qdbg << "correct get" << posFromServer;
		}
		else QMbError("Invalid data from server "+netName+"\ndata:"+arr);
	};

	localClient = MyQLocalServer::InitSocket(netName, 300, worker,
	logWorkerSock);

	if(localClient)
	{
		if(not localClient->isOpen()) localClient = {};
	}

	bool connected = localClient and localClient->isOpen();

	return connected;
}

std::map<QLocalSocket *, QPoint> distributedPoses;

const char * abort_send = "abort_send";

QString GetPosForClient(QLocalSocket *client, bool forceResetAll = false)
{
	if(not localServer){
		qdbg << "GetPosForClient called, but localServer is null";
		return "";
	}

	bool wereErased = false;

	auto screens = QGuiApplication::screens();
	if(screens.size() < 2) { return not_need_add_tray; }
	else
	{
		QPoint globalPosForIcon;
		for(auto &node:distributedPoses)
			if(localServer->activeClients.count(node.first) < 1)
			{
				//distributedPoses.erase(node.first); // удаление неактуальных
				wereErased = true;
				break;
			}

		if(wereErased or forceResetAll)
		{
			distributedPoses.clear();
			for(auto &node:localServer->activeClientsByConnectedTime)
			{
				auto clientToReset = node.second;
				auto posMsg = GetPosForClient(clientToReset);
				clientToReset->write(posMsg.toUtf8());
			}
			return abort_send;
		}

		const int fixed_y = 1001;
		const int step = 30;
		int x;
		if(distributedPoses.empty()) // нет существующих
		{
			auto screen2 = screens[1];
			const int startX = 1890;
			x = startX;
			QPoint posOnSct2(x, fixed_y);
			globalPosForIcon = screen2->geometry().topLeft() + posOnSct2;
		}
		else
		{
			int minX = INT32_MAX;
			for(auto &node:distributedPoses)
			{
				int x = node.second.x();
				minX = std::min(minX, x);
			}
			x = minX-step;
			globalPosForIcon = QPoint(x, fixed_y);
		}

		//qdbg << "GetPosForClient" << globalPosForIcon;
		distributedPoses.insert({client, globalPosForIcon});
		return command_set_pos+QSn(globalPosForIcon.x())+":"+QSn(globalPosForIcon.y())+";";
	}
}

bool InitServer()
{
	auto worker = [](QByteArray data, QLocalSocket *fromClient)
	{
		if(data == request_for_pos)
		{
			auto msg = GetPosForClient(fromClient).toUtf8();
			if(msg != abort_send) fromClient->write(msg);
		}

		//qdbg << "get from client: " << data;
	};

	localServer = MyQLocalServer::InitServer(netName, worker, logWorkerServ);
	if(localServer) localServer->cbClientDisconnectFinished = [](QLocalSocket *client){
		GetPosForClient(client, true);
	};

	return localServer->IsUp();
}

void InitLocalNet()
{
	auto initClientRes = InitClient();
	if(not initClientRes)
	{
		auto initSrvRes = InitServer();
		if(not initSrvRes) { QMbError("Error init server in net "+netName); return; }

		auto initClientRes2 = InitClient();
		if(not initClientRes2) { QMbError("Error init client in net "+netName); return; }
	}

	localClient->write(request_for_pos);
}

AdditionalTrayIcon::AdditionalTrayIcon(const QIcon &icon)
	: ClickableQWidget({}, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
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
	move({1871, 1001});
	show();

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &AdditionalTrayIcon::SlotShowContextMenu);

	connect(this, &ClickableQWidget::clicked, [this](){ QTimer::singleShot(0,[this](){
			PlatformDependent::SetTopMost(this,true); }); });

	//QTimer *timerTopmoster = new QTimer(this);
	//connect(timerTopmoster, &QTimer::timeout, [this](){ PlatformDependent::SetTopMost(this,true); });
	//timerTopmoster->start(1000);

	existingIcons.insert(this);

	InitLocalNet();

	AdditionalTrayIcon::fnClientGetCommandSetPos = [this](QPoint pos){
		if(pos.isNull()) {
			hide();
		}
		else {
			move(pos);
			show();
		}
	};
}

AdditionalTrayIcon::~AdditionalTrayIcon()
{
	//qdbg << "~AdditionalTrayIcon";
	auto erased = existingIcons.erase(this);
	if(erased == 0) qdbg << "existingIcons didnt contained this";
	if(existingIcons.empty())
	{
		localServer = {};
		localClient = {};
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
	widgetLogs = std::make_unique<QWidget>();

	QVBoxLayout *vlo_main = new QVBoxLayout(widgetLogs.get());
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

	if(show) widgetLogs->show();

	widgetLogs->resize(800,600);
	QTimer::singleShot(100,[this](){ widgetLogs->activateWindow(); });
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

void AdditionalTrayIcon::CallFnClientGetCommandSetPos(QPoint pos)
{
	if(not fnClientGetCommandSetPos) { QMbError("AdditionalTrayIcon: null fnClientGetCommandSetPos"); return; }
	fnClientGetCommandSetPos(pos);
}
