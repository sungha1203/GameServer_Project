#include "pch.h"
#include "ChatClient.h"

int main()
{
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("ChatClientLog.txt", 1024 * 1024, 5);

	plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);

	ChatClient client;
	if (!client.Init())
		return 0;

	if (!client.Connect())
		return 0;

	client.Start();
	client.RunConsole();
	client.End();

	return 0;
}
