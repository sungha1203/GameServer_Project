#pragma once

#include "DummyConfig.h"

class DummyConfigLoader
{
public:
	static bool Load(const std::string& filePath, DummyConfig& config);

private:
	static std::string Trim(const std::string& value);
};
