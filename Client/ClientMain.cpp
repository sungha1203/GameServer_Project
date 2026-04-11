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
	client.Start();

	while (1)
	{
		auto clientSession = std::static_pointer_cast<ClientSession>(client.GetSession());
		clientSession->SendChat("ㅎㅇ");
		this_thread::sleep_for(chrono::seconds(1));
	}

	client.End();
	WSACleanup();
}