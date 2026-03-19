#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup 실패" << endl;
        return 0;
    }

	SOCKET waitSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(waitSock == INVALID_SOCKET)
    {
        WSACleanup();
        return 0;
	}

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7777);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (::connect(waitSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "연결실패" << endl;
        closesocket(waitSock);
        WSACleanup();
        return 0;
    }

    for (int round = 1; round <= 5; ++round)
    {
        vector<SOCKET> sockets;
        sockets.reserve(100);

        cout << "===== Round " << round << " 시작 =====" << endl;

        for (int i = 0; i < 100; ++i)
        {
            SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock == INVALID_SOCKET)
            {
                cout << "socket 생성 실패 : " << WSAGetLastError() << endl;
                continue;
            }

            sockaddr_in serverAddr{};
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(7777);
            inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

            int ret = ::connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
            if (ret == SOCKET_ERROR)
            {
                closesocket(sock);
                continue;
            }

            sockets.push_back(sock);
        }

        cout << "연결 성공 소켓 수 : " << sockets.size() << endl;

        this_thread::sleep_for(chrono::seconds(1));

        for (SOCKET sock : sockets)
        {
            ::shutdown(sock, SD_BOTH);
            ::closesocket(sock);
        }

        sockets.clear();

        cout << "===== Round " << round << " 종료 =====" << endl << endl;

        this_thread::sleep_for(chrono::seconds(1));
    }
    
	// 서버에서 종료 메시지 수신 대기
    char buffer[128]{};
    int recvLen = recv(waitSock, buffer, sizeof(buffer) - 1, 0);

    if (recvLen > 0)
    {
        buffer[recvLen] = '\0';

		cout << "서버로부터 메시지 수신 : " << buffer << endl;

        if (strcmp(buffer, "서버종료") == 0)
        {
            cout << "클라이언트 종료." << endl;
        }
    }

    closesocket(waitSock);
    WSACleanup();
    return 0;
}