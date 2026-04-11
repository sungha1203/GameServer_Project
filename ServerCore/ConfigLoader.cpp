#include "pch.h"
#include "ConfigLoader.h"
#include <fstream>
#include <string>

bool ConfigLoader::Load(const std::string& filePath, Config& config)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		return false;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		size_t pos = line.find('=');
		if(pos == std::string::npos)
			continue;

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		if(key == "IP")
			config.ip = value;
		else if (key == "PORT")
			config.port = std::stoi(value);
	}

	if (config.ip.empty() || config.port == 0)
		return false;

	return true;
}
