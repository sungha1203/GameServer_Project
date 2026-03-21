#include "pch.h"
#include "Server.h"

int main()
{
	Server server;
	if (!server.Init()) return 0;
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