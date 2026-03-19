#include "pch.h"
#include "Server.h"

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;

	Server server;
	server.Init();
	server.Start();
	
	while (1)
	{
		string cmd;
		cin >> cmd;

		if (cmd == "서버종료")
		{
			server.ShutDown(cmd.c_str());
			break;
		}
	}

	server.End();
	WSACleanup();
}