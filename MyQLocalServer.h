#ifndef MyQLocalServer_h
#define MyQLocalServer_h

#include <memory>
#include <functional>

#include <QLocalServer>
#include <QLocalSocket>

struct MyQLocalServer
{
	inline static std::shared_ptr<QLocalServer> InitServer(QString name, std::function<void(QByteArray data)> incommingWorker);

	inline static std::shared_ptr<QLocalSocket> InitSocket(QString serverName, std::function<void(QByteArray data)> incommingWorker);
};

//------------------------------------------------------------------------------------------------------------------------------------------

std::shared_ptr<QLocalServer> MyQLocalServer::InitServer(QString name, std::function<void(QByteArray data)> incommingWorker)
{
	if(!incommingWorker) {
		qDebug() << "QLocalServer::InitServer invalid incommingWorker";
		return nullptr;
	}

	std::shared_ptr<QLocalServer> server(new QLocalServer);
	QLocalServer *serverPtr = server.get();
	if (!server->listen(name)) {  // Имя канала (на Windows: \\.\pipe\%name%)
		qDebug() << "QLocalServer "+name+" creations error:" << server->errorString();
		return nullptr;
	}

	// Обработка нового подключения
	QObject::connect(server.get(), &QLocalServer::newConnection, [serverPtr, incommingWorker]() {
		QLocalSocket *client = serverPtr->nextPendingConnection();

		// Чтение данных от клиента
		QObject::connect(client, &QLocalSocket::readyRead, [client, incommingWorker]() {
			QByteArray data = client->readAll();
			incommingWorker(std::move(data));

			// Отправка ответа
			//client->write("Hello from Qt Server!");
			//client->flush();
		});

		// Обработка отключения клиента
		QObject::connect(client, &QLocalSocket::disconnected, [client]() {
			client->deleteLater();
		});
	});

	QObject::connect(server.get(), &QObject::destroyed, [name]() {
		qDebug() << "QLocalServer "+name+" destroyed";
	});

	return server;
}

std::shared_ptr<QLocalSocket> MyQLocalServer::InitSocket(QString serverName, std::function<void (QByteArray)> incommingWorker)
{
	if(!incommingWorker) {
		qDebug() << "QLocalServer::InitSocket invalid incommingWorker";
		return nullptr;
	}

	std::shared_ptr<QLocalSocket> socket(new QLocalSocket);
	socket->connectToServer(serverName);  // Имя канала

	if (!socket->waitForConnected(1000)) {
		qDebug() << "QLocalSocket error connection to server "+serverName+": " + socket->errorString();
		return nullptr;
	}
	return socket;
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------