#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <csignal>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

using namespace std;

SOCKET ClientSocket = INVALID_SOCKET; // Глобальный UDP-сокет

// Функция для обработки сигналов (Ctrl + C)
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        cout << "Прерывание программы (Ctrl + C). Закрываем соединение." << endl;
        if (ClientSocket != INVALID_SOCKET) {
            closesocket(ClientSocket);
        }
        WSACleanup();
        exit(0); // Завершение программы
    }
    return TRUE;
}
//Корректное завершение программы при нажатии Ctrl+C с освобождением ресурсов.

int main() {
    setlocale(LC_ALL, "Russian");
    //Инициализация Winsock
    WSADATA wsaData;
    int iResult;
    string host = "192.168.9.68";
    int port = 8123;

    cout << "Хост(127.0.0.1): ";
    string input_host;
    getline(cin, input_host);
    if (!input_host.empty()) host = input_host;
    cout << endl;

    cout << "Порт(8123): ";
    string input_port;
    getline(cin, input_port);
    if (!input_port.empty()) port = stoi(input_port);
    cout << endl;

    cout << "Хост : Порт для подключения - " << host << ":" << port << endl;

    // Установка обработчика сигналов
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // Инициализация Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed: " << iResult << endl;
        return 1;
    }

    // Создание сокета
    ClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ClientSocket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Настройка таймаута. Клиент не будет вечно ждать ответа от сервера. Через 3 секунды получит ошибку таймаута.
    int timeout = 3000; // 3 seconds
    setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // Настройка адреса сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.S_un.S_addr = inet_addr(host.c_str());
    serverAddress.sin_port = htons(port);

    char sendbuf[DEFAULT_BUFLEN] = "Привет, сервер!"; //Фиксированное сообщение
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int serverAddressSize = sizeof(serverAddress);

    while (true) {
        cout << "-------------------------" << endl;
        Sleep(1000); // Пауза в 1 секунду

        // 1. Отправка сообщения серверу. Отправка с указанием адреса получателя
        iResult = sendto(ClientSocket, sendbuf, strlen(sendbuf), 0, (SOCKADDR*)&serverAddress, serverAddressSize);
        if (iResult == SOCKET_ERROR) {
            cerr << "Ошибка отправки сообщения: " << WSAGetLastError() << endl;
            continue;
        }
        cout << "Отправлено: " << sendbuf << endl;

        // 2. Прием ответа от сервера. Прием с получением адреса отправителя
        iResult = recvfrom(ClientSocket, recvbuf, recvbuflen, 0, (SOCKADDR*)&serverAddress, &serverAddressSize);
        if (iResult > 0) {
            recvbuf[iResult] = '\0'; // Добавляем нуль-терминатор
            cout << "Получено от сервера: " << recvbuf << endl;
        }
        else {
            cout << "Ошибка при получении сообщения: " << WSAGetLastError() << endl;
        }
    }
    /*
    Логика:
    -Пауза 1 секунда
    -Отправка фиксированного сообщения "Привет, сервер!"
    -Ожидание ответа с таймаутом 3 секунды
    -Вывод результата
    -Повтор цикла
    */
    // Закрытие сокета
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}

/*
Ввод сообщений:

string message;
cout << "Введите сообщение: ";
getline(cin, message);

// Отправка введенного сообщения
sendto(ClientSocket, message.c_str(), message.length(), 0, ...);

Буфер:
char recvbuf[DEFAULT_BUFLEN];
int bytesReceived = recvfrom(ClientSocket, recvbuf, DEFAULT_BUFLEN - 1, 0, ...);
if (bytesReceived > 0) {
    recvbuf[bytesReceived] = '\0';  // Безопасно - место гарантировано
}

Рекомендации по улучшению
1. Интерактивный режим:
while (true) {
    string message;
    cout << "Введите сообщение: ";
    getline(cin, message);

    // Отправка и прием...
}
2. Обработка длинных сообщений:
// Проверка размера сообщения перед отправкой
if (message.length() >= DEFAULT_BUFLEN) {
    cerr << "Сообщение слишком длинное" << endl;
    continue;
}
3. Более интеллектуальные таймауты:
// Увеличивать таймаут при повторных ошибках
static int currentTimeout = 3000;
if (iResult == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
    currentTimeout = min(currentTimeout * 2, 30000); // Макс 30 секунд
    setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&currentTimeout, sizeof(currentTimeout));
}
*/