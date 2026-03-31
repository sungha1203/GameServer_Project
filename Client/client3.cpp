#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;
using uint16 = unsigned short;

struct PacketHeader
{
    uint16 size;
    uint16 id;
};

enum : uint16
{
    PKT_CHAT = 1,
};

constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 7777;

constexpr int THREAD_COUNT = 10;
constexpr int SOCKETS_PER_THREAD = 1000;
constexpr int SEND_COUNT_PER_SOCKET = 10;

atomic<int> g_connectSuccess = 0;
atomic<long long> g_sendSuccess = 0;

bool SendAll(SOCKET sock, const char* data, int len)
{
    int totalSent = 0;

    while (totalSent < len)
    {
        int sent = send(sock, data + totalSent, len - totalSent, 0);
        if (sent == SOCKET_ERROR || sent == 0)
            return false;

        totalSent += sent;
    }

    return true;
}

bool SendChatPacket(SOCKET sock, const string& msg)
{
    PacketHeader header;
    header.size = 128;
    header.id = PKT_CHAT;

    vector<char> sendBuffer(header.size, 0);
    memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));

    int maxMsgSize = static_cast<int>(header.size) - static_cast<int>(sizeof(PacketHeader));
    int copySize = min(static_cast<int>(msg.size()), maxMsgSize);
    memcpy(sendBuffer.data() + sizeof(PacketHeader), msg.data(), copySize);

    bool ret = SendAll(sock, sendBuffer.data(), static_cast<int>(sendBuffer.size()));
    if(ret)
		g_sendSuccess++;

	// cout << "Send Packet : " << msg << endl;

    return ret;
}

bool ConnectToServer(SOCKET& sock)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return false;

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) != 1)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        cout << "연결 실패 : " << err << endl;

        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    ++g_connectSuccess;
    return true;
}

void CloseAllSockets(vector<SOCKET>& sockets)
{
    for (SOCKET& sock : sockets)
    {
        if (sock != INVALID_SOCKET)
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }
}

void RunThread(int threadIndex)
{
    const int threadN = threadIndex + 1;

    vector<SOCKET> sockets(SOCKETS_PER_THREAD, INVALID_SOCKET);
    vector<string> messages;
    messages.reserve(SOCKETS_PER_THREAD);

    for (int socketIndex = 0; socketIndex < SOCKETS_PER_THREAD; socketIndex++)
    {
        int socketN = socketIndex + 1;
        messages.push_back(to_string(threadN) + "/" + to_string(socketN));
    }

    while (true)
    {
        int connectedCount = 0;

        // 끊어진 소켓만 다시 연결
        for (int i = 0; i < SOCKETS_PER_THREAD; i++)
        {
            if (sockets[i] != INVALID_SOCKET)
            {
                ++connectedCount;
                continue;
            }

            if (ConnectToServer(sockets[i]))
                ++connectedCount;

            // 너무 한 번에 몰리지 않게 하기 위함
            if ((i + 1) % 200 == 0)
                this_thread::sleep_for(chrono::milliseconds(10));
        }

        // cout << "[Thread " << threadN << "] 연결 소켓 개수 : " << connectedCount << endl;

        // 각 소켓이 10번씩 전송
        for (int i = 0; i < SOCKETS_PER_THREAD; i++)
        {
            SOCKET& sock = sockets[i];
            if (sock == INVALID_SOCKET)
                continue;

            for (int sendRound = 0; sendRound < 10; ++sendRound)
            {
                if (!SendChatPacket(sock, messages[i]))
                {
                    closesocket(sock);
                    sock = INVALID_SOCKET;
                    break;
                }
            }

            // 10번 보내고 바로 소켓 닫기
            //if (sock != INVALID_SOCKET)
            //{
            //    closesocket(sock);
            //    sock = INVALID_SOCKET;
            //}

            // 너무 몰리지 않게 약간 쉬기
            if ((i + 1) % 200 == 0)
                this_thread::sleep_for(chrono::milliseconds(10));
        }

        cout << "[Thread " << threadN << "]"
            << " 누적 연결 성공 : " << g_connectSuccess.load()
            << ", send 성공 : " << g_sendSuccess.load() << endl;

        // 소켓을 닫으면 동시 접속자 수가 점점 떨어지는데 가용 port가 소진되서 TIME_WAIT 상태 예상
        // CloseAllSockets(sockets);

        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    vector<thread> workers;
    workers.reserve(THREAD_COUNT);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        workers.emplace_back(RunThread, i);
    }

    for (auto& t : workers)
    {
        t.join();
    }

    WSACleanup();
    return 0;
}