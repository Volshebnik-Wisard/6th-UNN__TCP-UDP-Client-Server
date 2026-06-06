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
#include <format>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512


DWORD WINAPI ThreadProc()
{
	return 0;
}
int main1()
{
	setlocale(LC_CTYPE, "ru");

	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	else
	{
		std::cout << "1.DLL загружена!" << std::endl;
	}



	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "2.Сокет для прослушивания клиентов создан!" << std::endl;
	}
	// Setup the TCP listening socket
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
	}

	std::vector<SOCKET>ALL_connections;
	int clients_number = 0;
	

	while (true)
	{
		SOCKET ClientSocket;
		ClientSocket = INVALID_SOCKET;

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		else
		{
			clients_number++;
			ALL_connections.push_back(ClientSocket);

			
			std::cout << "Принят запрос от клиента на подключение! Всего клиентов " << clients_number << std::endl;
		}


		char recvbuf[DEFAULT_BUFLEN];
		int iSendResult;


		// Receive until the peer shuts down the connection
		do {

			iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0) {
				std::cout << "Принято сообщение " << recvbuf << std::endl;
				std::cout << "Введите сообщение" << std::endl;
				std::string msg;
				int msg_lenght = msg.length();
				std::getline(std::cin, msg);

				iSendResult = send(ClientSocket, msg.c_str(), (int)strlen(msg.c_str()), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}


			}
			else if (iResult == 0)
				printf("Закрывается соединение...\n");
			else {
				printf("recv failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}

		} while (iResult > 0);
	}
		
	

	


	system("pause");
	return 0;
}