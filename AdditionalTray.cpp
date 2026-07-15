#include "AdditionalTray.h"

#include <QComboBox>
#include <QToolTip>

#include "MyQString.h"
#include "MyQTextEdit.h"
#include "MyQLocalServer.h"
#include "AppDataWork.h"

static std::shared_ptr<MyQLocalServer> localServer;
static QByteArray serverBuffer;
static std::shared_ptr<QLocalSocket> localClient;
static QByteArray clientBuffer;
static bool localClientIsNextServer = false;

static QString netName = "AdditionalTrayIcon";
static std::function<void(QString log)> logWorkerSocket = [](QString log){ qdbg << "sock: "+log; };
static std::function<void(QString log)> logWorkerServer = [](QString log){ qdbg << "serv: "+log; };

static const char * you_are_next_server = "__yans__";
static const char * you_are_NOT_next_server = "__yaNOTns__";


static const char endCommandChar = ';';
static const char *endCommandStr = ";";

static const char * request_for_pos = "request_for_pos";
static QByteArray request_for_monitor = "request_for_monitor";
static QString command_set_pos = "command_set_pos:";
static const char * not_need_add_tray = "not_need_add_tray";
static const char * set_monitor = "set_monitor:";

static QByteArray answ_suffix = "_answ:";

static QEventLoop& monitor_loop() { static QEventLoop loop; return loop; }
static QString request_for_monitor_answValue;

static QString monitorFileName = "monitor.txt";

static std::vector<const char*> expectedLogs {
	MyQLocalServer::error_connection_starting,
	MyQLocalServer::log_client_disconnected,
};

struct LocalNet
{
	static void InitLocalNet();
	static bool InitClient(bool firstTry);
	static bool InitServer();
};

bool LocalNet::InitClient(bool firstTry)
{
	auto incommingWorker = [](QByteArray arr){
		//qdbg << arr;
		clientBuffer += arr;

		if(not clientBuffer.endsWith(endCommandChar))
		{
			if(clientBuffer.size() > 10000) { QMbError("clientBuffer overflow"); clientBuffer.clear(); }
			return;
		}

		clientBuffer.chop(1);
		auto arrs = clientBuffer.split(endCommandChar);
		clientBuffer.clear();

		for(auto &arr:arrs)
		{
			if(0) {}
			else if(arr == you_are_next_server)
			{
				localClientIsNextServer = true;
				qdbg << "localClientIsNextServer = true";
			}
			else if(arr == you_are_NOT_next_server)
			{
				localClientIsNextServer = false;
				qdbg << "localClientIsNextServer = false";
			}
			else if(arr == not_need_add_tray)
			{
				AdditionalTrayIcon::CallFnClientGetCommandSetPos({});
			}
			else if(arr.startsWith(command_set_pos.toUtf8()))
			{
				QString answ_pos_str = arr;
				answ_pos_str.remove(0, command_set_pos.size());

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
			else if(arr.startsWith(request_for_monitor))
			{
				auto arrCopy = arr;
				arrCopy.remove(0, request_for_monitor.size());
				if(not arrCopy.startsWith(answ_suffix)) { QMbError("missing answ suffix "+netName+"\ndata:"+arr); }

				arrCopy.remove(0, answ_suffix.size());

				request_for_monitor_answValue = arrCopy;
				monitor_loop().quit();
			}
			else QMbError("Invalid data from server "+netName+"\ndata:"+arr);
		}
	};

	auto logCopy = logWorkerSocket;
	if(firstTry) logCopy = [](QString log){
		for(auto &expectedLog:expectedLogs)
			if(log.startsWith(expectedLog)) return;
		qdbg << "UNEXPECTED MyQLocalServer log: "+log;
	};

	auto disconnectWorker = [](){
		if(localClientIsNextServer)
		{
			localClientIsNextServer = false;
			QTimer::singleShot(0, [](){
				qdbg << "this application will became new "+netName+" server";
				InitLocalNet();
			});
		}
		else
		{
			QTimer::singleShot(100, [](){
				qdbg << "this application will try connect to new "+netName+" server";
				InitLocalNet();
			});
		}
	};

	localClient = MyQLocalServer::InitSocket(netName, 300, incommingWorker, disconnectWorker, logCopy);

	if(localClient)
	{
		if(not localClient->isOpen()) localClient = {};
	}

	bool connected = localClient and localClient->isOpen();

	return connected;
}

static std::map<QLocalSocket *, QPoint> distributedPoses;

static int monitorForPlacing = 0;

static void SendCmdSetPos(QLocalSocket *client);

static void SendCmdSetPosToAll()
{
	distributedPoses.clear();
	for(auto &node:localServer->activeClientsByConnectedTime)
	{
		auto clientToReset = node.second;
		SendCmdSetPos(clientToReset);
	}
}

static void SendCmdSetPos(QLocalSocket *client)
{
	if(not localServer) {
		qdbg << "GetPosForClient called, but localServer is null";
		return;
	}

	if(monitorForPlacing == -1)
	{
		client->write(not_need_add_tray);
		client->write(endCommandStr);
		return;
	}

	auto screensList = QGuiApplication::screens();

	if(monitorForPlacing < 0 or monitorForPlacing >= screensList.size()) {
		QMbError("Invalid monitorForPlacing = "+QSn(monitorForPlacing)+", monitors count is "+QSn(screensList.size()));
		client->write(not_need_add_tray);
		client->write(endCommandStr);
		return;
	}

	auto &screen = screensList[monitorForPlacing];

	bool wereErased = false;
	for(auto &node:distributedPoses)
		if(localServer->activeClients.count(node.first) < 1)
		{
			//distributedPoses.erase(node.first); // удаление неактуальных
				// почему закомментировано?
			wereErased = true;
			break;
		}

	if(wereErased)
	{
		SendCmdSetPosToAll();
		return;
	}

	QPoint globalPosForIcon;
	if(distributedPoses.empty()) // нет существующих
	{
		const int fixed_y = 1001;
		const int start_x = 1870;
		QPoint posOnSct2(start_x, fixed_y);
		globalPosForIcon = screen->geometry().topLeft() + posOnSct2;
	}
	else
	{
		int minX = INT32_MAX;
		for(auto &node:distributedPoses)
		{
			minX = std::min(minX, node.second.x());
		}
		const int step = 30;
		int x = minX-step;
		globalPosForIcon = QPoint(x, distributedPoses.begin()->second.y());
	}

	//qdbg << "GetPosForClient" << globalPosForIcon;
	distributedPoses.insert({client, globalPosForIcon});
	client->write((command_set_pos+QSn(globalPosForIcon.x())+":"+QSn(globalPosForIcon.y())).toUtf8());
	client->write(endCommandStr);
}

static void SendClientHeIsNotNextServer(QLocalSocket &client)
{
	client.write(you_are_NOT_next_server);
	client.write(endCommandStr);
}

static void SendClientsWhoIsNextServer(QLocalSocket &next_server_client)
{
	next_server_client.write(you_are_next_server);
	next_server_client.write(endCommandStr);
	for(auto &node:localServer->activeClients)
	{
		if(node.first != &next_server_client)
		{
			SendClientHeIsNotNextServer(*node.first);
		}
	}
}

bool LocalNet::InitServer()
{
	auto incommingWorker = [](QByteArray data, QLocalSocket *fromClient)
	{
		serverBuffer += data;

		if(not data.endsWith(endCommandChar))
		{
			if(serverBuffer.size() > 10000) { QMbError("serverBuffer overflow"); serverBuffer.clear(); }
			return;
		}
		serverBuffer.chop(1);
		auto arrs = serverBuffer.split(endCommandChar);
		serverBuffer.clear();

		for(auto &data:arrs)
		{
			if(data == request_for_pos)
			{
				SendCmdSetPos(fromClient);
			}
			else if(data == request_for_monitor)
			{
				fromClient->write((request_for_monitor+answ_suffix+QSn(monitorForPlacing)).toUtf8());
				fromClient->write(endCommandStr);
			}
			else if(data.startsWith(set_monitor))
			{
				data.remove(0, QByteArray(set_monitor).length());
				bool ok;
				int monitor = data.toInt(&ok);
				if(not ok) { QMbError(QString("Invalid ")+set_monitor+" value: "+data); return; }

				monitorForPlacing = monitor;
				SendCmdSetPosToAll();

				auto forlder = AppDataWork::MakeFolderInAppData(AppDataWorkNames::RomaDevelop, AppDataWorkNames::AdditionalTray);
				auto writeRes = MyQFileDir::WriteFile(forlder+"/"+monitorFileName, QSn(monitorForPlacing));
				if(not writeRes) QMbError("Can't write file "+forlder+"/"+monitorFileName);
			}
			else QMbError("Invalid data from client\ndata:"+data);
		}

		//qdbg << "get from client: " << data;
	};

	localServer = MyQLocalServer::InitServer(netName, incommingWorker, logWorkerServer);
	if(not localServer) return false;

	static QLocalSocket *clientToBecomeServer {};
	static QLocalSocket *currentProgrammClient {};
	localServer->cbClientConnected = [](QLocalSocket *client){
		if(not currentProgrammClient) currentProgrammClient = client;
		if(currentProgrammClient == client) return;
		/// если это первый клиент, т.е. мы сами - ничего не делаем

		// если следующий сервер не знадан - выставляем
		if(not clientToBecomeServer)
		{
			clientToBecomeServer = client;
			SendClientsWhoIsNextServer(*client);
		}
		else SendClientHeIsNotNextServer(*client);
	};

	// При отключении клиента
	localServer->cbClientDisconnectFinished = [](QLocalSocket *client) {
		// если отключился следующий сервер - обнуляем
		if(client == clientToBecomeServer)
		{
			clientToBecomeServer = {};
			for(auto &node:localServer->activeClientsByConnectedTime)
			{
				if(node.second != currentProgrammClient)
				{
					clientToBecomeServer = node.second;
					SendClientsWhoIsNextServer(*node.second);
					break;
				}
			}
		}

		// всем высылаются обновлённые позиции
		SendCmdSetPosToAll();
	};

	return localServer->IsUp();
}

void LocalNet::InitLocalNet()
{
	if(localServer)
	{
		QMbError("Unexpected case, InitLocalNet called, but localServer is not null");
		return;
	}

	localServer = {};
	localClient = {};
	auto initClientRes = InitClient(true);
	if(not initClientRes)
	{
		auto initSrvRes = InitServer();
		if(not initSrvRes) { QMbError("Error init server in net "+netName); return; }

		auto initClientRes2 = InitClient(false);
		if(not initClientRes2) { QMbError("Error init client in net "+netName); return; }
	}

	localClient->write(request_for_pos);
	localClient->write(endCommandStr);
}

AdditionalTrayIcon::AdditionalTrayIcon(const QIcon &icon)
	: ClickableQWidget({}, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
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

	setToolTipDuration(0);
	setMouseTracking(true);
	timerToolTip.setSingleShot(true);
	connect(&timerToolTip, &QTimer::timeout, this, [this](){ QToolTip::showText(QCursor::pos(), toolTip(), this); });

	existingIcons.insert(this);

	auto monitorFile = AppDataWork::BuildPathFolderInAppData(AppDataWorkNames::RomaDevelop, AppDataWorkNames::AdditionalTray)
			+"/"+monitorFileName;
	if(QFileInfo(monitorFile).isFile())
	{
		auto readRes = MyQFileDir::ReadFile2(monitorFile);
		if(not readRes.success) QMbError("Error read "+monitorFile);
		else
		{
			bool ok;
			int monitor = readRes.content.toInt(&ok);
			if(ok) monitorForPlacing = monitor;
			else QMbError("Invalid value = "+readRes.content+" in file "+monitorFile);
		}
	}
	else
	{
		if(QGuiApplication::screens().size() > 1)
			monitorForPlacing = 1;
	}

	LocalNet::InitLocalNet();

	AdditionalTrayIcon::fnClientGetCommandSetPos = [this](QPoint pos){
		if(pos.isNull()) {
			hide();
		}
		else {
			move(pos);
			show();
//			QTimer::singleShot(0, [pos](){
//				QMbInfo("moved to "+MyQString::AsDebug(pos));
//			});
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
		expectedLogs.push_back(MyQLocalServer::log_client_disconnected);
		localServer = {};
		localClient = {};
	}
}

static const char *no_add_icon = "Без доп. иконки";

void AdditionalTrayIcon::CreateSettingsCombo(QGridLayout *glo, int row, int col, AdditionalTrayIcon *additionalTrayIcon)
{
	auto comboMonitor = new QComboBox();
	int monitorsCnt = QGuiApplication::screens().size();
	comboMonitor->addItem(no_add_icon);
	for(int i=0; i<monitorsCnt; i++) comboMonitor->addItem(QSn(i));
	int currentAddTrayMonitor = additionalTrayIcon->currentMonitor();
	comboMonitor->setCurrentText(QSn(currentAddTrayMonitor));

	glo->addWidget(new QLabel("Монитор доп. иконки:"), row, col);
	glo->addWidget(comboMonitor, row, 1);

	if(additionalTrayIcon)
	{
		connect(comboMonitor, &QComboBox::currentTextChanged, additionalTrayIcon, [additionalTrayIcon, comboMonitor]()
		{
			int monitorVal = comboMonitor->currentText().toInt();
			if(comboMonitor->currentText() == no_add_icon) monitorVal = -1;
			additionalTrayIcon->setMonitor(monitorVal);
		});
	}
	else
	{
		comboMonitor->setDisabled(true);
	}
}

void AdditionalTrayIcon::setMonitor(int monitor)
{
	if(not localClient) { QMbError("Icons client is null"); return;  }
	localClient->write(QString(set_monitor+QSn(monitor)).toUtf8());
	localClient->write(endCommandStr);
}

int AdditionalTrayIcon::currentMonitor()
{
	if(localServer) return monitorForPlacing;

	if(not localClient) { QMbError("Icons client is null"); return 0;  }

	request_for_monitor_answValue.clear();
	localClient->write(request_for_monitor);
	localClient->write(endCommandStr);

	{
		QObject singleShotAborter;
		QTimer::singleShot(300, &singleShotAborter, [](){
			monitor_loop().quit();
			QMbError("Can't get currentMonitor, timeout");
			request_for_monitor_answValue = "0";
		});
		monitor_loop().exec();
	}

	bool ok;
	int monitor = request_for_monitor_answValue.toInt(&ok);
	if(not ok) QMbError("Invalit request_for_monitor_answValue: "+request_for_monitor_answValue);

	return monitor;
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

void AdditionalTrayIcon::enterEvent(QEvent * event)
{
	startTooltipTimer();
	QWidget::enterEvent(event);
}

void AdditionalTrayIcon::leaveEvent(QEvent * event)
{
	timerToolTip.stop();
	QWidget::leaveEvent(event);
}

void AdditionalTrayIcon::mouseMoveEvent(QMouseEvent * event)
{
	startTooltipTimer(); // restart tooltip wait when mouse moved
	QWidget::mouseMoveEvent(event);
}

void AdditionalTrayIcon::startTooltipTimer()
{
	int delay = style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay, nullptr, this);
	timerToolTip.start(delay);
}
