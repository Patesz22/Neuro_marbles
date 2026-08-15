#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS // error C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#include <winsock2.h>
#include <cstdio>
#include <string>
#include <chrono>
#include <time.h>
#include <windows.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <memory>

#include "NeuroGameSDK/NeuroGameSdkWebsocketpp.hpp"
#include "Client/NeuroMarblesClient.h"
#include "misc.h"
#include "json/json.hpp"
#include "GameState.h"
#include "Logging/Logging.h"
#include "SDK.hpp"
#include "Actions/Actions_Complex.h"
#include "Config/NeuroConfig.h"





