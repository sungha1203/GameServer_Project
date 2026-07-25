#pragma once

#include <string>

struct DummyConfig
{
	std::string ip = "172.30.1.48";
	int port = 7778;
	int clientCount = 1000;
	int connectThreadCount = 4;
	int roomCount = 10;
	int sendPerSecond = 1;
	int testSeconds = 60;
	int packetProcessorThreadCount = 4;
};
