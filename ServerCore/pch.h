#pragma once
#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

using namespace std;