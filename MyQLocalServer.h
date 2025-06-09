#ifndef MyQLocalServer_h
#define MyQLocalServer_h

#include <memory>
#include <functional>

#include <QLocalServer>
#include <QLocalSocket>

#include "MyQShortings.h"

struct MyQLocalServer
{
	inline static std::shared_ptr<QLocalServer> InitServer(QString name,
	                                                       std::function<void(QByteArray data)> incommingWorker,
	                                                       std::function<void(QString)> logWorker);

	inline static std::shared_ptr<QLocalSocket> InitSocket(QString serverName, int waitForConnected,
	                                                       std::function<void(QByteArray data)> incommingWorker,
	                                                       std::function<void(QString)> logWorker);
};

//------------------------------------------------------------------------------------------------------------------------------------------

std::shared_ptr<QLocalServer> MyQLocalServer::InitServer(QString name,
                                                         std::function<void(QByteArray data)> incommingWorker,
                                                         std::function<void(QString)> logWorker)
{
	if(!incommingWorker) {
		if(logWorker) logWorker("QLocalServer::InitServer invalid incommingWorker");
		else qDebug() << "QLocalServer::InitServer invalid incommingWorker";
	}

	std::shared_ptr<QLocalServer> server(new QLocalServer);
	QLocalServer *serverPtr = server.get();
	if (!server->listen(name)) {  // Имя канала (на Windows: \\.\pipe\%name%)
		if(logWorker) logWorker("QLocalServer "+name+" creations error: " + server->errorString());
		else qDebug() << "QLocalServer "+name+" creations error: " + server->errorString();
		return nullptr;
	}

	// Обработка нового подключения
	QObject::connect(serverPtr, &QLocalServer::newConnection, [serverPtr, incommingWorker, logWorker, name]() {
		QLocalSocket *client = serverPtr->nextPendingConnection();

		if(logWorker) logWorker("client connected to " + name);
		else qDebug() << "client connected to " + name;

		// Чтение данных от клиента
		QObject::connect(client, &QLocalSocket::readyRead, [client, incommingWorker]() {
			QByteArray data = client->readAll();
			if(incommingWorker) incommingWorker(std::move(data));
		});

		// Обработка отключения клиента
		QObject::connect(client, &QLocalSocket::disconnected, [client, logWorker, name]() {
			client->deleteLater();
			if(logWorker) logWorker("client disconnected from " + name);
			else qDebug() << "client disconnected from " + name;
		});
	});

	QObject::connect(serverPtr, &QObject::destroyed, [name, logWorker]() {
		qdbg << "before error (because QLocalServer is global, but logWorker works with destroyed widgets)";
		if(logWorker) logWorker("QLocalServer " + name + "destroyed");
		else qDebug() << "QLocalServer " + name + "destroyed";
		qdbg << "after error";
	});

	return server;
}

std::shared_ptr<QLocalSocket> MyQLocalServer::InitSocket(QString serverName, int waitForConnected,
                                                         std::function<void (QByteArray)> incommingWorker,
                                                         std::function<void(QString)> logWorker)
{
	if(!incommingWorker) {
		if(logWorker) logWorker("MyQLocalServer::InitSocket invalid incommingWorker");
		else qDebug() << "MyQLocalServer::InitSocket invalid incommingWorker";
	}

	std::shared_ptr<QLocalSocket> socket(new QLocalSocket);
	socket->connectToServer(serverName);  // Имя канала

	if (!socket->waitForConnected(waitForConnected)) {
		if(logWorker) logWorker("MyQLocalServer::InitSocket error connection to server "+serverName+": " + socket->errorString());
		else qDebug() << "MyQLocalServer::InitSocket error connection to server "+serverName+": " + socket->errorString();
		return nullptr;
	}

	QObject::connect(socket.get(), &QLocalSocket::disconnected, [logWorker, serverName](){
		if(logWorker) logWorker("client disconnected from " + serverName);
		else qDebug() << "client disconnected from " + serverName;
	});

	return socket;
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
