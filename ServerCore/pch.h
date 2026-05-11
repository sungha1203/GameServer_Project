#pragma once
#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <unordered_map>

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

using namespace std;