#include "pch.h"
#include "ChatServer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ChatServerLog.txt", 1024 * 1024, 5);

	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

	ChatServer server;
	if (!server.Init()) return 0;
	server.Start();

	while (1)
	{
		string cmd;
		cin >> cmd;

		if (cmd == "quit" || cmd == "exit")
			break;
	}

	server.End();
	return 0;
}
