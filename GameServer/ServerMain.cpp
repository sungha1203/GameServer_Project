#include "pch.h"
#include "Server.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	//_CrtSetBreakAlloc(2567);

	// 로그 파일 최대 1MB, 최대 5개까지 보관
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ServerLog.txt", 1024 * 1024, 5);

	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

	Server server;
	if (!server.Init()) return 0;
	server.Start();

	while (1)
	{
		string cmd;
		cin >> cmd;

		if (cmd == "서버종료")	break;
	}

	server.End();

	//_CrtDumpMemoryLeaks();

	// 받은 메시지			-> GameSession::OnRecvPacket(L21)
	// recv 개수			-> GameSession::OnRecvPacket(L25)
}