#pragma once

struct Config
{
	std::string ip;
	int port;
};

class ConfigLoader
{
public:
	static bool Load(const std::string& filePath, Config& config);
};

