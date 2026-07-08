#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS // NeuroGameSdkWebsocketpp.hpp(175,20): error C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#include <winsock2.h>
#include <cstdio>
#include <string>
#include <time.h>

#include "SDK.hpp"
#include "MenuNavigation.h"
#include "GameRaceActions.h"
#include "misc.h"
#include "json.hpp"
#include <cmath>

#include <windows.h>


extern bool bBotActive;
extern std::string GameName;
extern std::wstring ActiveMatchID;
extern volatile bool bShouldClickStart;
extern volatile bool bClickAcknowledged;
extern volatile bool bMatchIDIntercepted;


// State Machine
enum ENeuroState {
    STAGE_Welcome_Continue = 0,
    STAGE_Gamemode_Select,
    STAGE_Race_Map_Select,
    STAGE_Race_Lobby_Start,
    STAGE_Race_Game_Joining,
    STAGE_Race_Game_Waiting,
    STAGE_Race_Game_Started,
    STAGE_Race_Game_Finished,
    STAGE_Race_Game_Finished_Waiting
};


