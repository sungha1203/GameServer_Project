#include "pch.h"
#include "Client.h"

int main()
{
	// 로그 파일 최대 1MB, 최대 5개까지 보관
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ClientLog.txt", 1024 * 1024, 5);

	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

	Client client;
	if(!client.Init()) return 0;
	if (!client.ConnectClients()) return 0;
	client.Start();

	while (1)
	{
		std::string cmd;
		std::cin >> cmd;
		if (cmd == "클라종료")
		{
			break;
		}
	}
	client.End();

	WSACleanup();
}