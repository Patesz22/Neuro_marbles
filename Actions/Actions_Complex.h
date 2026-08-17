#pragma once
#include <json/json.hpp>
#include "main.h"

namespace Mode_Race
{
	extern NeuroWebsocketpp::Action* actRaceGetJoinedPlayers;
	extern NeuroWebsocketpp::Action* actSetGlobalGravity;
	extern NeuroWebsocketpp::Action* actSetMarbleMass;
	extern NeuroWebsocketpp::Action* actKickPlayer;
	extern NeuroWebsocketpp::Action* actSetMarbleSize;

	void InitComplexActions();

	void FreeComplexActions();
};
