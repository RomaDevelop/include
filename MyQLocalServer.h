#ifndef MyQLocalServer_h
#define MyQLocalServer_h

#include <set>
#include <memory>
#include <functional>

#include <QDateTime>
#include <QLocalServer>
#include <QLocalSocket>

#include "MyQShortings.h"

struct MyQLocalServer
{
	inline static std::shared_ptr<MyQLocalServer> InitServer(QString name,
											std::function<void(QByteArray data, QLocalSocket *fromClient)> incommingWorker,
											std::function<void(QString)> logWorker);

	inline static std::shared_ptr<QLocalSocket> InitSocket(QString serverName, int waitForConnected,
	                                                       std::function<void(QByteArray data)> incommingWorker,
	                                                       std::function<void(QString)> logWorker);

	std::shared_ptr<QLocalServer> qserver;	// shared_ptr потому что конструктор перемещения не доступен
	std::map<QLocalSocket*, QDateTime> activeClients; // second = connected_time
	std::map<QDateTime, QLocalSocket*> activeClientsByConnectedTime;

	std::function<void(QLocalSocket *client)> cbClientAboutToDisconnect;
	std::function<void(QLocalSocket *client)> cbClientDisconnectFinished;

	void Down() { qserver = {}; }

	bool IsUp() const { return (bool)qserver; }

	MyQLocalServer();
	MyQLocalServer(const MyQLocalServer&) = delete;
	MyQLocalServer& operator=(const MyQLocalServer&) = delete;
	MyQLocalServer(MyQLocalServer&&) = delete;
	MyQLocalServer& operator=(MyQLocalServer&&) = delete;

	static void Log(const std::function<void(QString)> &logWorker, const QString &log)
	{
		if(logWorker) logWorker(log);
		else qDebug() << log;
	}

	inline static const char *error_connection_starting = "MyQLocalServer::InitSocket error connection to server ";
};

//--------------------------------------------------------------------------------------------------------------------------

MyQLocalServer::MyQLocalServer():
	qserver { std::make_shared<QLocalServer>() }
{

}

std::shared_ptr<MyQLocalServer> MyQLocalServer::InitServer(QString name,
									std::function<void(QByteArray data, QLocalSocket *fromClient)> incommingWorker,
									std::function<void(QString)> logWorker)
{
	if(!incommingWorker) {
		Log(logWorker, "QLocalServer::InitServer invalid incommingWorker");
	}

	std::shared_ptr<MyQLocalServer> server (new MyQLocalServer);

	MyQLocalServer *serverPtr = server.get();
	QLocalServer *qserverPtr = server->qserver.get();
	if (!qserverPtr->listen(name)) {  // Имя канала (на Windows: \\.\pipe\%name%)
		Log(logWorker, "QLocalServer "+name+" creations error: " + qserverPtr->errorString());
		return {};
	}

	// Обработка нового подключения
	QObject::connect(qserverPtr, &QLocalServer::newConnection, qserverPtr, [serverPtr, qserverPtr, incommingWorker, logWorker, name]() {
		QLocalSocket *client = qserverPtr->nextPendingConnection();

		auto dt = QDateTime::currentDateTime();
		serverPtr->activeClients.insert({client, dt});
		serverPtr->activeClientsByConnectedTime.insert({dt, client});

		Log(logWorker, "client connected to " + name);

		// Чтение данных от клиента
		QObject::connect(client, &QLocalSocket::readyRead, qserverPtr, [client, incommingWorker]() {
			QByteArray data = client->readAll();
			if(incommingWorker) incommingWorker(std::move(data), client);
		});

		// Обработка отключения клиента
		QObject::connect(client, &QLocalSocket::disconnected, qserverPtr, [serverPtr, client, logWorker, name]() {

			if(serverPtr->cbClientAboutToDisconnect) serverPtr->cbClientAboutToDisconnect(client);

			auto dtConnedcted = serverPtr->activeClients[client];
			serverPtr->activeClients.erase(client);
			auto findRes = serverPtr->activeClientsByConnectedTime.find(dtConnedcted);
			if(findRes != serverPtr->activeClientsByConnectedTime.end())
			{
				serverPtr->activeClientsByConnectedTime.erase(dtConnedcted);
			}
			else
			{
				Log(logWorker, "not found in map activeClientsByConnectedTime by "+dtConnedcted.toString());
				for(auto &node:serverPtr->activeClientsByConnectedTime)
				{
					if(node.second == client)
					{
						serverPtr->activeClientsByConnectedTime.erase(node.first);
						Log(logWorker, "found in cycle, date was activeClientsByConnectedTime by "+node.first.toString());
						break;
					}
				}
				Log(logWorker, "not found in map activeClientsByConnectedTime even by cycle");
			}

			client->deleteLater();
			Log(logWorker, "client 0x"+QSn((uintptr_t)client, 16)+" disconnected from " + name);

			if(serverPtr->cbClientDisconnectFinished) serverPtr->cbClientDisconnectFinished(client);
		});
	});

	QObject::connect(qserverPtr, &QObject::destroyed, qserverPtr, [name, logWorker]() {
		//qdbg << "before error (because QLocalServer is global, but logWorker works with destroyed widgets)";
		Log(logWorker, "QLocalServer " + name + "destroyed");
		//qdbg << "after error";
	});

	return server;
}

std::shared_ptr<QLocalSocket> MyQLocalServer::InitSocket(QString serverName, int waitForConnected,
                                                         std::function<void (QByteArray)> incommingWorker,
                                                         std::function<void(QString)> logWorker)
{
	if(!incommingWorker) {
		Log(logWorker, "MyQLocalServer::InitSocket invalid incommingWorker");
	}

	std::shared_ptr<QLocalSocket> socket(new QLocalSocket);
	socket->connectToServer(serverName);  // Имя канала

	if (!socket->waitForConnected(waitForConnected)) {
		Log(logWorker, error_connection_starting+serverName+": " + socket->errorString());
		return nullptr;
	}

	if (incommingWorker) {
		QObject::connect(socket.get(), &QLocalSocket::readyRead, [socket = socket.get(), incommingWorker]() {
			// Читаем все доступные данные из сокета
			QByteArray data = socket->readAll();
			if (!data.isEmpty()) {
				incommingWorker(data);
			}
		});
	}

	QObject::connect(socket.get(), &QLocalSocket::disconnected, [logWorker, serverName](){
		Log(logWorker, "client disconnected from " + serverName);
	});

	return socket;
}

//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------
