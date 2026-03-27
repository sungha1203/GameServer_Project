#include "pch.h"
#include "Server.h"

int main()
{
	// 로그 파일 최대 1MB, 최대 5개까지 보관
	plog::init(plog::debug, "ServerLog.txt", 1024 * 1024, 5);
	//PLOGI << "Server Start";
	cout << "Server Start" << endl;

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