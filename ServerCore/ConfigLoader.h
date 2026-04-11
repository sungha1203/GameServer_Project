#pragma once
#include <string>
#include <fstream>
#include <type_traits>

struct ConfigServer
{
	std::string ip;
	int port;
};

struct ConfigClient
{
	std::string ip;
	int port;
	int clientCnt;
};

template<typename T>
class ConfigLoader
{
public:
	static bool Load(const std::string& filePath, T& config)
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
			if (pos == std::string::npos)
				continue;

			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);

			if constexpr (std::is_same_v<T, ConfigServer>)
			{
				if (key == "IP")
					config.ip = value;
				else if (key == "PORT")
					config.port = std::stoi(value);
			}
			else if constexpr (std::is_same_v<T, ConfigClient>)
			{
				if (key == "IP")
					config.ip = value;
				else if (key == "PORT")
					config.port = std::stoi(value);
				else if (key == "CLIENT")
					config.clientCnt = std::stoi(value);
			}
		}

		if constexpr (std::is_same_v<T, ConfigServer>)
		{
			if (config.ip.empty() || config.port == 0)
				return false;
		}
		else if constexpr (std::is_same_v<T, ConfigClient>)
		{
			if (config.ip.empty() || config.port == 0 || config.clientCnt == 0)
				return false;
		}

		return true;
	}
};