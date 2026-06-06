#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_IP "192.168.9.68"
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

int main10()
{
	setlocale(LC_CTYPE, "ru");
	// Инициализация Winsock
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	else
	{
		//std::cout << "1.DLL загружена!" << std::endl;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	//Конфигурация подключения
	std::string host = DEFAULT_IP;
	std::string port = DEFAULT_PORT;

	std::cout << "IP(по умолчанию: 127.0.0.1): ";
	std::string input_host;
	std::getline(std::cin, input_host);
	if (!input_host.empty()) host = input_host;
	std::cout << "Порт(по умолчанию: 27015): ";
	std::string input_port;
	std::getline(std::cin, input_port);
	if (!input_port.empty()) port = (input_port);
	std::cout << host << ":" << port << std::endl;

	// Определите локальный адрес и порт, которые будут использоваться сервером. Преобразование доменного имени/IP в адресную структуру
	iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
// Попытайтесь подключиться к первому адресу, возвращенному вызовом getaddrinfo
	ptr = result;

	// Создайте сокет для подключения к серверу. Установление TCP-соединения с сервером
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	else
	{
		//std::cout << "2.Сокет для клиента успешно создан!" << std::endl;
	}
	// Подключитесь к серверу.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}


	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	std::string msgToSend;
	while (true)
	{
		// 1. Проверка состояния соединения. Если соединение потеряно - пробуем востановить
		if (iResult == SOCKET_ERROR) {
			// Пересоздаем сокет и соединение, попытка переподключения к серверу. Бесконечный цикл при потере соединения.
			closesocket(ConnectSocket);
			ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				std::cout << "Попытка переподключения..." << std::endl;
				continue;
			}
			else {
				std::cout << "Соединение восстановлено" << std::endl;
				iResult = send(ConnectSocket, msgToSend.c_str(), (int)strlen(msgToSend.c_str()), 0);
			}

		}
		//2.Отправка/получение данных
		if (msgToSend.empty())
		{
			do {
				std::cout << "КЛИЕНТ:\n";
				std::getline(std::cin, msgToSend);
			} while (msgToSend.empty());
			iResult = send(ConnectSocket, msgToSend.c_str(), (int)strlen(msgToSend.c_str()), 0);

		}

		if (iResult == SOCKET_ERROR) {
			std::cout << "Сервер не отвечает..." << std::endl;
			continue;
		}
		msgToSend = "";
		// 3. Получение ответа от сервера
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			std::cout << "СЕРВЕР:\n" << std::string(recvbuf).substr(0, iResult) << std::endl;
		}
		else if (iResult <= 0)
		{
			std::cout << "Потеряно соединение " << std::endl;
			continue;
		}

	}
	// Закрытие сокета
	/*
	Текущая логика:
	-Клиент отправляет сообщение
	-Ждет ответ от сервера
	-После получения ответа снова запрашивает ввод
	-Проблема: Если сервер отправит несколько сообщений подряд - они проигнорируются
	Правильная логика, на всякий случай:

while (true) {
	// 1. Проверка входящих сообщений
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		// Обработка полученного сообщения
	}

	// 2. Проверка пользовательского ввода (неблокирующая)
	if (есть_данные_от_пользователя) {
		// Отправка сообщения
	}
}
	*/
	closesocket(ConnectSocket);
	WSACleanup();
	system("pause");
	return 0;
}

/*
// Правильная обработка переподключения
bool Reconnect(SOCKET& socket, addrinfo* ptr) {
	if (socket != INVALID_SOCKET) {
		closesocket(socket);
	}

	socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket == INVALID_SOCKET) return false;

	return connect(socket, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR;
}

// Основной цикл с правильной логикой
std::string currentMessage;
while (true) {
	// Проверка входящих сообщений
	char buffer[DEFAULT_BUFLEN];
	int bytesReceived = recv(ConnectSocket, buffer, DEFAULT_BUFLEN, 0);

	if (bytesReceived > 0) {
		std::cout << "СЕРВЕР: " << std::string(buffer, bytesReceived) << std::endl;
	} else if (bytesReceived == 0) {
		std::cout << "Сервер закрыл соединение" << std::endl;
		break;
	} else {
		// Ошибка или таймаут
	}

	// Проверка пользовательского ввода (неблокирующая)
	if (/* есть ввод от пользователя *//*) {
		std::string message;
		std::getline(std::cin, message);
		send(ConnectSocket, message.c_str(), message.length(), 0);
	}
}

Разделить переменные для отправки и приема:
int sendResult, recvResult;  // Разные переменные для разных операций
Добавить задержки между попытками переподключения:
if (iResult == SOCKET_ERROR) {
	std::cout << "Попытка переподключения через 5 секунд..." << std::endl;
	Sleep(5000);  // Задержка 5 секунд
	// ... переподключение
}
*/