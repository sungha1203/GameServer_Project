#pragma once
#include <iostream>

#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

#include <thread>
#include <vector>

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

using namespace std;