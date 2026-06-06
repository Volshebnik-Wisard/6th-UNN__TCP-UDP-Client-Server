// Отключает предупреждения устаревших функций
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <csignal>

#pragma comment(lib, "Ws2_32.lib") // Автоматическая линковка библиотеки

#define DEFAULT_BUFLEN 512

using namespace std;

SOCKET ServerSocket = INVALID_SOCKET; // Глобальный сокет сервера

// Функция для обработки сигналов (Ctrl + C)
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        cout << "Прерывание программы (Ctrl + C). Закрываем сервер." << endl;
        if (ServerSocket != INVALID_SOCKET) {
            closesocket(ServerSocket);
        }
        WSACleanup();
        exit(0); // Завершение программы
    }
    return TRUE;
}
//Корректное завершение программы при нажатии Ctrl+C

int main() {
    setlocale(LC_ALL, "Russian");
    WSADATA wsaData;
    int iResult;
    string host = "0.0.0.0";
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

    // Инициализация Winsock - Без этого сокеты не будут работать
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed: " << iResult << endl;
        return 1;
    }

    // Создание сокета
    ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ServerSocket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    /*
    Ключевые параметры:
    AF_INET - IPv4
    SOCK_DGRAM - датаграммы (UDP)
    IPPROTO_UDP - UDP протокол
    */

    // Настройка таймаута - сервер не будет вечно ждать сообщений, через 3 секунды получит ошибку таймаута
    int timeout = 3000; // 3 seconds
    setsockopt(ServerSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // Привязка к адресу
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.S_un.S_addr = inet_addr(host.c_str()); //лучше использовать inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr);
    serverAddress.sin_port = htons(port);

    // Привязка сокета к адресу
    iResult = bind(ServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR) {
        cerr << "bind failed: " << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    cout << "Сервер слушает на " << host << ":" << port << endl;

    char recvbuf[DEFAULT_BUFLEN];
    char sendbuf[DEFAULT_BUFLEN] = "Привет, клиент!"; //Фиксированный ответ:
    int recvbuflen = DEFAULT_BUFLEN;
    sockaddr_in clientAddress; // Адрес клиента заполняется при recvfrom()
    int clientAddressSize = sizeof(clientAddress);
    //Основной рабочий цикл
    //Логика обработки сообщений :
    while (true) {
        cout << "-------------------------" << endl;

        // 1. Прием сообщения от любого клиента
        iResult = recvfrom(ServerSocket, recvbuf, recvbuflen, 0, (SOCKADDR*)&clientAddress, &clientAddressSize);
        // recvfrom() заполняет clientAddress данными отправителя
        if (iResult > 0) {
            recvbuf[iResult] = '\0'; // Добавляем нуль-терминатор
            // 2. Вывод полученного сообщения
            cout << "Получено от клиента: " << recvbuf << endl;

            // 3. Отправка фиксированного ответа
            iResult = sendto(ServerSocket, sendbuf, strlen(sendbuf), 0, (SOCKADDR*)&clientAddress, clientAddressSize);
            // sendto() использует этот адрес для ответа
            if (iResult == SOCKET_ERROR) {
                cerr << "Ошибка отправки сообщения: " << WSAGetLastError() << endl;
                continue;
            }
            cout << "Ответ отправлен клиенту: " << sendbuf << endl;
        }
        else {
            cerr << "Ошибка приема сообщения: " << WSAGetLastError() << endl;
        }
    }
    /*
    Ключевые особенности UDP
    1. Без установления соединения
    -Нет accept(), listen(), connect()
    -Каждое сообщение обрабатывается независимо
    -Сервер не "знает" клиентов постоянно
    2. Работа с адресами
    3. Датиграммный подход
    -Сообщения могут теряться
    -Нет гарантии порядка доставки
    -Высокая производительность
    */
    // Закрытие сокета
    closesocket(ServerSocket);
    WSACleanup();
    return 0;
}

/*
## Рекомендации по улучшению

### Динамические ответы
```cpp
// Генерация ответа на основе полученного сообщения
string response = "Эхо: " + string(recvbuf);
sendto(ServerSocket, response.c_str(), response.length(), 0, ...);
```

### Обработка множественных клиентов
```cpp
// UDP по умолчанию поддерживает множественных клиентов
// Можно добавить логику для разных типов запросов
```

## Сравнение с TCP-сервером

| Аспект | UDP-сервер | TCP-сервер |
|--------|------------|------------|
| Соединение | Без соединения | С установкой соединения |
| Надежность | Нет гарантий доставки | Гарантированная доставка |
| Производительность | Выше | Ниже |
| Сложность | Проще | Сложнее |
| Использование | VoIP, игры, DNS | Веб-серверы, файловые передачи |

*/