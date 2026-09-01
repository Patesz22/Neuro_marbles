#pragma once
#include <json/json.hpp>
#include "main.h"

namespace Mode_Race
{
	extern NeuroIXWebsocket::Action* actRaceGetJoinedPlayers;
	extern NeuroIXWebsocket::Action* actSetGlobalGravity;
	extern NeuroIXWebsocket::Action* actSetMarbleMass;
	extern NeuroIXWebsocket::Action* actKickPlayer;
	extern NeuroIXWebsocket::Action* actSetMarbleSize;

	void InitComplexActions();

	void FreeComplexActions();
};
