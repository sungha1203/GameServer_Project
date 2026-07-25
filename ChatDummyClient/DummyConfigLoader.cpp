#include "pch.h"
#include "DummyConfigLoader.h"

#include <fstream>

bool DummyConfigLoader::Load(const std::string& filePath, DummyConfig& config)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		return false;

	std::string line;
	while (std::getline(file, line))
	{
		line = Trim(line);
		if (line.empty() || line[0] == '#')
			continue;

		const size_t pos = line.find('=');
		if (pos == std::string::npos)
			continue;

		const std::string key = Trim(line.substr(0, pos));
		const std::string value = Trim(line.substr(pos + 1));

		if (key == "IP")
			config.ip = value;
		else if (key == "PORT")
			config.port = std::stoi(value);
		else if (key == "CLIENT_COUNT")
			config.clientCount = std::stoi(value);
		else if (key == "CONNECT_THREAD_COUNT")
			config.connectThreadCount = std::stoi(value);
		else if (key == "ROOM_COUNT")
			config.roomCount = std::stoi(value);
		else if (key == "SEND_PER_SECOND")
			config.sendPerSecond = std::stoi(value);
		else if (key == "TEST_SECONDS")
			config.testSeconds = std::stoi(value);
		else if (key == "PACKET_PROCESSOR_THREAD_COUNT")
			config.packetProcessorThreadCount = std::stoi(value);
	}

	return true;
}

std::string DummyConfigLoader::Trim(const std::string& value)
{
	const size_t begin = value.find_first_not_of(" \t\r\n");
	if (begin == std::string::npos)
		return "";

	const size_t end = value.find_last_not_of(" \t\r\n");
	return value.substr(begin, end - begin + 1);
}
