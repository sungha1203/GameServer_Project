#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
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

    vector<char> sendBuffer(header.size);
    memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));

    int maxMsgSize = 128 - sizeof(PacketHeader);
    int copySize = min((int)msg.size(), maxMsgSize);
    memcpy(sendBuffer.data() + sizeof(PacketHeader), msg.data(), copySize);

    return SendAll(sock, sendBuffer.data(), static_cast<int>(sendBuffer.size()));
}

void RunClient(int clientIndex)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return;

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7777);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        return;
    }

    vector<string> quotes =
    {
        "Dream big. Work hard. Stay focused.",
        "The best view comes after the hardest climb.",
        "Believe you can and you're halfway there.",
        "It always seems impossible until it's done.",
        "Seize the day."
    };

    string myMsg = quotes[clientIndex - 1];

    while (true)
    {
        string msg = "[Client" + to_string(clientIndex) + "] " + myMsg;

        if (!SendChatPacket(sock, msg))
            break;

        this_thread::sleep_for(chrono::seconds(1));
    }

    closesocket(sock);
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    vector<thread> clients;

    for (int i = 1; i <= 5; i++)
    {
        clients.emplace_back(RunClient, i);
    }

    for (auto& t : clients)
    {
        t.join();
    }

    WSACleanup();
    return 0;
}