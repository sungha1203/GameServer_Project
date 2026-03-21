#pragma once

struct ServerConfig
{
	std::string ip;
	int port = 7777;
};

class ConfigLoader
{
public:
	static bool Load(const std::string& filePath, ServerConfig& config);
};

