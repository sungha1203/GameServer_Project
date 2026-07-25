#include "pch.h"
#include "ChatDummyClient.h"

int main()
{
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ChatDummyClientLog.txt", 1024 * 1024, 5);

	plog::init(plog::info, &fileAppender).addAppender(&consoleAppender);

	ChatDummyClient client;
	if (!client.Init())
		return 0;

	if (!client.Start())
		return 0;

	client.Run();
	client.End();

	return 0;
}
