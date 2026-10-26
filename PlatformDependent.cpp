#include "PlatformDependent.h"

#ifndef Q_OS_WIN
#error PlatformDependent is implemented only for Qt on Windows.

	// инклуды предложенные ИИ при добавлении GetParentProcessName
	#elif defined(Q_OS_LINUX)
	#include <unistd.h>
	#include <fstream>
	#include <string>
	#elif defined(Q_OS_MAC)
	#include <unistd.h>
	#include <libproc.h>
	//#endif

#endif

#include <qt_windows.h>
#include <tlhelp32.h>

#include <QApplication>
#include <QDebug>
#include <QDir>

#include "MyCppDifferent.h"
#include "MyQShortings.h"

qint64 PlatformDependent::GetCurrentProcessId()
{
	return QCoreApplication::applicationPid();
}

QString PlatformDependent::GetParentProcessName(qint64 processID) {
#if defined(Q_OS_WIN)
	if (processID < 0 || processID > (std::numeric_limits<DWORD>::max)()) {
		qDebug() << "GetParentProcessName: Некорректное значение (" << processID << ") для DWORD!";
		return "";
	}

	DWORD parentPid = 0;
	QString parentName = "";

	// 1. Создаем снимок процессов
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		qDebug() << "getParentProcessName [Win]: Failed to create snapshot. Error:" << GetLastError();
		return "";
	}

	any_guard::functions_caller handlerCloser({}, [snapshot](){ CloseHandle(snapshot); });

	PROCESSENTRY32W entry;
	ZeroMemory(&entry, sizeof(entry));
	entry.dwSize = sizeof(PROCESSENTRY32W);

	// 2. Ищем PID родителя
	bool foundParentPid = false;
	if(!Process32FirstW(snapshot, &entry)) {
		qDebug() << "getParentProcessName [Win]: Process32FirstW-1 failed. Error:" << GetLastError();
		return "";
	}

	do {
		if (entry.th32ProcessID == processID) {
			parentPid = entry.th32ParentProcessID;
			foundParentPid = true;
			break;
		}
	} while (Process32NextW(snapshot, &entry));

	if(!foundParentPid) {
		qDebug() << "getParentProcessName [Win]: PID (" << processID << ") not found in snapshot";
		return "";
	}

	if(parentPid == 0) {
		qDebug() << "getParentProcessName [Win]: parent PID is 0";
		return "";
	}

	/// 3. Ищем процесс родителя вторым проходом по snapshot и извлекаем его имя
	/// (второй проход нужен, потому что мы первым циклом мы дошли до текущего процесса,
	/// а процесс родителя будет ДО, вернуться назад нельзя).
	ZeroMemory(&entry, sizeof(entry));
	entry.dwSize = sizeof(PROCESSENTRY32W);

	bool foundParentName = false;
	if (!Process32FirstW(snapshot, &entry)) {
		qDebug() << "getParentProcessName [Win]: Process32FirstW-2 failed. Error:" << GetLastError();
		return "";
	}

	do {
		if (entry.th32ProcessID == parentPid) {
			// Используем безопасный конструктор QString для работы с WCharArray
			parentName = QString::fromWCharArray(entry.szExeFile);
			foundParentName = true;
			break;
		}
	} while (Process32NextW(snapshot, &entry));

	if (!foundParentName) {
		qDebug() << "getParentProcessName [Win]: Parent process with PID" << parentPid << "not found (likely terminated).";
	}

	return parentName.trimmed().toLower();

#elif defined(Q_OS_LINUX)
	// В Linux getppid() всегда безопасен
#error Требуется перепроверить
	pid_t parentPid = getppid();
	if (parentPid <= 0) {
		qDebug() << "getParentProcessName [Linux]: Invalid parent PID:" << parentPid;
		return "";
	}

	// Читаем /proc/[pid]/comm (содержит только имя, до 16 символов, без аргументов)
	std::string commPath = "/proc/" + std::to_string(parentPid) + "/comm";
	std::ifstream file(commPath);
	if (!file.is_open()) {
		qDebug() << "getParentProcessName [Linux]: Failed to open" << QString::fromStdString(commPath);
		return "";
	}

	std::string name;
	if (std::getline(file, name)) {
		return QString::fromStdString(name).trimmed().toLower();
	}

	qDebug() << "getParentProcessName [Linux]: File" << QString::fromStdString(commPath) << "is empty.";
	return "";

#elif defined(Q_OS_MAC)
#error Требуется перепроверить
	pid_t parentPid = getppid();
	if (parentPid <= 0) {
		qDebug() << "getParentProcessName [Mac]: Invalid parent PID:" << parentPid;
		return "";
	}

	char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
	ZeroMemory(pathBuffer, sizeof(pathBuffer));

	// proc_pidpath возвращает полный путь к бинарнику родителя
	int ret = proc_pidpath(parentPid, pathBuffer, sizeof(pathBuffer));
	if (ret <= 0) {
		qDebug() << "getParentProcessName [Mac]: proc_pidpath failed for PID" << parentPid;
		return "";
	}

	// Извлекаем только имя файла из полного пути
	QString fullPath = QString::fromUtf8(pathBuffer);
	int lastSlash = fullPath.lastIndexOf('/');
	if (lastSlash != -1) {
		return fullPath.mid(lastSlash + 1).trimmed().toLower();
	}
	return fullPath.trimmed().toLower();

#else
	qDebug() << "getParentProcessName: Unsupported Operating System.";
	return "";
#endif
}

QDateTime PlatformDependent::GetProcessStartTime(qint64 processID) {
	if (processID < 0 || processID > (std::numeric_limits<DWORD>::max)()) {
		qDebug() << "GetProcessStartTime: Некорректное значение (" << processID << ") для DWORD!";
		return {};
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if (hProcess == NULL) {
		auto err = GetLastError();
		qDebug() << "PlatformDependent::GetProcessStartTime: Не удалось открыть процесс с PID "
				+ QSn(processID) + ". Ошибка: " + QSn(err);
		return QDateTime(); // Возвращаем пустой QDateTime в случае ошибки
	}

	FILETIME creationTime, exitTime, kernelTime, userTime;

	if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
		// Преобразуем FILETIME в QDateTime
		ULARGE_INTEGER ull;
		ull.LowPart = creationTime.dwLowDateTime;
		ull.HighPart = creationTime.dwHighDateTime;

		// FILETIME представляет время в 100-наносекундных интервалах с 1 января 1601 года
		// Преобразуем в QDateTime
		QDateTime startTime = QDateTime::fromMSecsSinceEpoch(ull.QuadPart / 10000 - 11644473600000LL);
		CloseHandle(hProcess);
		return startTime;
	}
	else
	{
		auto err = GetLastError();
		qDebug() << "PlatformDependent::GetProcessStartTime: Не удалось получить время процесса. Ошибка: "
				+ QSn(err);
		CloseHandle(hProcess);
		return QDateTime(); // Возвращаем пустой QDateTime в случае ошибки
	}
}

bool PlatformDependent::IsProcessRunning(qint64 processID) {
	if (processID < 0 || processID > (std::numeric_limits<DWORD>::max)()) {
		qDebug() << "IsProcessRunning: Некорректное значение (" << processID << ") для DWORD!";
		return {};
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if(hProcess != NULL)
	{
		DWORD exitCode;
		if(GetExitCodeProcess(hProcess, &exitCode)) // Получаем код завершения процесса
		{
			CloseHandle(hProcess); // Закрываем дескриптор
			return (exitCode == STILL_ACTIVE); // Если код завершения равен STILL_ACTIVE, процесс все еще работает
		}
		else
		{
			auto err = GetLastError();
			qDebug() << "IsProcessRunning::GetExitCodeProcess error: " + QSn(err);
		}
		CloseHandle(hProcess); // Закрываем дескриптор в случае ошибки
	}
	else
	{
		auto err = GetLastError();
		if(err == ERROR_INVALID_PARAMETER) ; // ok, means not running
		else qDebug() << "IsProcessRunning::OpenProcess unknown error " + QSn(err);
	}
	return false; // Процесс не запущен или не может быть открыт
}

bool PlatformDependent::OpenWithDialog(const QString &filePath) {
	if (filePath.isEmpty()) return false;

	std::wstring nativePath = QDir::toNativeSeparators(filePath).toStdWString();

	SHELLEXECUTEINFOW sei {};
	sei.cbSize = sizeof(SHELLEXECUTEINFOW);
	sei.fMask = SEE_MASK_ASYNCOK | SEE_MASK_INVOKEIDLIST;
	sei.lpVerb = L"openas";
	sei.lpFile = nativePath.c_str();
	sei.nShow = SW_SHOWNORMAL;

	// от ИИ
	// Важно: ShellExecuteEx требует инициализации COM для некоторых функций
	// Если в приложении еще нет CoInitialize, добавьте:
	// CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	// дополнение от меня - с CoInitializeEx не линкуется, для него требуется какая-то dll

	return ShellExecuteExW(&sei);
}

bool PlatformDependent::IsFileLocked(const QString & file) {
	// Преобразуем QString в массив WCHAR для работы с WinAPI
	const WCHAR* pathPtr = reinterpret_cast<const WCHAR*>(file.utf16());

	// Пытаемся получить дескриптор файла БЕЗ прав на чтение/запись данных (0)
	// и БЕЗ разрешения другим процессам его использовать (dwShareMode = 0)
	HANDLE hFile = CreateFileW(
				pathPtr,
				DELETE,                      // ЗАПРАШИВАЕМ ПРАВО НА УДАЛЕНИЕ (как при rename)
				0,                           // Требуем эксклюзивности: если кто-то открыл файл, вызов вернет ошибку
				NULL,
				OPEN_EXISTING,               // Только для существующих файлов
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);

	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION || err == ERROR_ACCESS_DENIED) {
			return true; // Файл занят
		}
		else {
			qDebug() << "PlatformDependent::IsFileLocked: unknown error: " + QSn(err);
			return true; // Другая ошибка (например, файл не найден)
		}
	}

	// Если удалось открыть — файл свободен. Обязательно закрываем дескриптор!
	CloseHandle(hFile);
	return false;
}

PlatformDependent::CopyMoveFileRes PlatformDependent::CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode)
{
	SourceFile.replace('/','\\');
	Destination.replace('/','\\');
	wchar_t cFrom[MAX_PATH] = {0};
	wcscpy(cFrom, SourceFile.toStdWString().c_str());
	wchar_t cTo[MAX_PATH] = {0};
	wcscpy(cTo, Destination.toStdWString().c_str());
	SHFILEOPSTRUCT fos;
	memset(&fos, 0, sizeof(SHFILEOPSTRUCT));
	if(QApplication::activeWindow())
		fos.hwnd = (HWND)QApplication::activeWindow()->winId();
	else if(GetConsoleWindow())
		fos.hwnd = GetConsoleWindow();
	else fos.hwnd = nullptr;
	fos.wFunc = Mode == move ? FO_MOVE : FO_COPY;
	fos.pFrom = cFrom;
	fos.pTo = cTo;
	fos.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOF_SIMPLEPROGRESS;

	CopyMoveFileRes res;
	res.errorCode = SHFileOperation(&fos);

	if(res.errorCode == 0) res.success = true;
	else
	{
		res.success = false;
		res.errorText = "Не удалось переместить файл\n\n"+SourceFile+
				"\n\nв\n\n"+Destination+"!\n\nКод ошибки SHFileOperation: "+ QSn(res.errorCode);
		if(Mode == copy)
			res.errorText.replace("переместить", "скопировать");
	}

	return res;
}

bool PlatformDependent::IsPathOnFixedDrive(QString path)
{
	// 1. Проверка UNC-пути напрямую (например, \\server\share)
	path = QDir::toNativeSeparators(path);
	if (path.startsWith("\\\\")) return false;

	// 2. Проверка буквы дисков, запрашивая тип у ОС Windows
	if (path.length() >= 2 && path[1] == ':') {
		if(path.size() == 2) path += "\\";

		// Извлечение "C:\" или "Z:\"
		std::wstring root = path.left(3).toStdWString();

		// Получение типа диска
		UINT driveType = GetDriveTypeW(reinterpret_cast<LPCWSTR>(root.c_str()));

		// DRIVE_FIXED — константа Windows, указывающая на локальный диск
		return (driveType == DRIVE_FIXED);
	}

	qDebug() << "PlatformDependent::IsPathLocal invalid path:" << path;
	return false;
}

void PlatformDependent::SetTopMost(QWidget * w, bool topMost)
{
	HWND hwnd = reinterpret_cast<HWND>(w->winId());
	SetWindowPos(hwnd, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
				 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void PlatformDependent::SetTopMostFlash(QWidget *w)
{
	SetTopMost(w, true);
	SetTopMost(w, false);
}

void PlatformDependent::FlashClickOnTitle(QWidget *w)
{
	// Получаем дескриптор окна
	HWND hwndTool = reinterpret_cast<HWND>(w->winId());

	static int captionHeight = GetSystemMetrics(SM_CYCAPTION);

	// Получаем координаты заголовка окна
	RECT rect;
	GetWindowRect(hwndTool, &rect);
	int x = (rect.left + rect.right) / 2; // Центр окна по X
	int y = rect.top + captionHeight / 2; // Центр высоты заголовка окна

	// Запоминаем текущие координаты мыши
	POINT currentPos;
	GetCursorPos(&currentPos);

	// Перемещаем курсор мыши в нужную позицию
	SetCursorPos(x, y);

	// Эмулируем клик мышью
	INPUT input {};
	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.dy = 0;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT)); // Нажатие кнопки мыши

	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT)); // Отпускание кнопки мыши

	// Возвращаем курсор мыши на исходные координаты
	SetCursorPos(currentPos.x, currentPos.y);
}

void PlatformDependent::ShowPropertiesWindow(const QString &file)
{
	SHELLEXECUTEINFO sei = {};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_INVOKEIDLIST;
	sei.lpVerb = L"properties";
	sei.lpFile = reinterpret_cast<LPCWSTR>(file.utf16());
	sei.nShow = SW_SHOW;
	ShellExecuteEx(&sei);
}




