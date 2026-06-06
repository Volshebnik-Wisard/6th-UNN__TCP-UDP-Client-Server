// Отключение редко используемых компонентов Windows
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_IP "0.0.0.0"
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512



int main10()
{
	setlocale(LC_CTYPE, "ru");

	WSADATA wsaData;
	int iResult;
	// Инициализация Winsock - Загружается библиотека Winsock 2.2
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	else
	{
		std::cout << "1.DLL загружена!" << std::endl;
	}


	//задаем IP и port (конфигурацию сервера) и настройка сокета
	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;  // IPv4
	hints.ai_socktype = SOCK_STREAM;  // Потоковый сокет (TCP)
	hints.ai_protocol = IPPROTO_TCP;  // TCP протокол

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

	// Определите локальный адрес и порт, которые будут использоваться сервером
	iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	// Создание и настройка слушающего сокета
	// Создайте СОКЕТ для сервера, который будет прослушивать клиентские подключения
	//Этапы:
	//	socket() - создание сокета
	//	bind() - привязка к IP и порту
	//	listen() - начало прослушивания порта
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "2.Сокет для прослушивания клиентов создан! " << std::endl;
	}
	// Настройка прослушивающего сокета TCP
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "3.Сокет успешно привязан к IP и порту!" << std::endl;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "4.Сокет начинает прослушивание!" << std::endl;
		std::cout << "================================" << std::endl;
	}


	char recvbuf[DEFAULT_BUFLEN];
	//Возможно переполнение буфера и длинные сообщения обрежутся - 512 байт
	int recvbuflen = DEFAULT_BUFLEN;
	// Основной цикл сервера - Сервер бесконечно ожидает новых подключений.
	while (true)
	{
		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL); // Обработка клиента
		// Сервер может обрабатывать только ОДНОГО клиента одновременно
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			continue;
		}
		std::cout << "Принято соединение" << std::endl;
		// Обработка клиентского соединения
		while (true)
		// Пока обрабатывается один клиент, другие ждут
		// Новые клиенты не принимаются, пока сервер общается с текущим клиентом
		// Нет возможности одновременной работы с несколькими клиентами
		{
			// Получение сообщения от клиента
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			// Если клиент отправил сообщение и отключился, сервер все равно попытается отправить ответ
			if (iResult > 0)
			{
				// Вывод сообщения и затем ОБЯЗАТЕЛЬНО запрос ответа и отправка
				std::cout << "КЛИЕНТ:\n" << std::string(recvbuf).substr(0, iResult) << std::endl;
				
			}
			else if (iResult <= 0)
			{
				std::cout << "Потеряно соединение с клиентом" << std::endl;
				break;
			}

			std::string msgToSend;
			do {
				std::cout << "СЕРВЕР:\n";
				std::getline(std::cin, msgToSend);
			} while (msgToSend.empty());
			// Отправка ответа клиенту
			iResult = send(ClientSocket, msgToSend.c_str(), (int)strlen(msgToSend.c_str()), 0);
			if (iResult == SOCKET_ERROR) {
				std::cout << "Ошибка отправки" << std::endl;
				break;
			}
		}
		closesocket(ClientSocket);

	}
	/*
	 
	Цикл обмена сообщениями:
	1.Сервер ждет сообщение от клиента
	2.Выводит полученное сообщение
	3.Ждет ввод ответа от пользователя
	4.Отправляет ответ клиенту

	*/
	closesocket(ListenSocket);
	WSACleanup();
	system("pause");
	return 0;
}

/*
## Рекомендации по улучшению

### 1. Многопоточная обработка
```cpp
while (true) {
	SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
	std::thread clientThread(handleClient, ClientSocket);
	clientThread.detach();
}
```

### 2. Неблокирующие сокеты
```cpp
// Использование select() или WSAPoll для множественных соединений
```

### 3. Обработка длинных сообщений
```cpp
std::vector<char> buffer;
while (true) {
	char chunk[512];
	int received = recv(ClientSocket, chunk, sizeof(chunk), 0);
	// Собираем полное сообщение из чанков
}
```

## Вывод
Это базовый учебный TCP-сервер, который демонстрирует основные принципы сетевого программирования, но **не пригоден для production-использования** из-за блокирующей архитектуры и ограниченной функциональности. Для реальных задач требуется реализация многопоточности или асинхронных операций.
*/