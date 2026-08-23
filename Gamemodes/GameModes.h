#pragma once
#include "../GameState.h"
#include "Client/NeuroMarblesClient.h"

// Menu Routing
namespace Mode_Menu
{
    extern NeuroIXWebsocket::Action actContinue;
    extern NeuroIXWebsocket::Action actSelectRace;

    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);

    int GetMaxLobbySize();
}

// Race Routing
namespace Mode_Race
{
    extern NeuroIXWebsocket::Action actMenuGoBack;

    extern NeuroIXWebsocket::Action actRaceRandomMap;
    extern NeuroIXWebsocket::Action actRaceStartLobby;
    extern NeuroIXWebsocket::Action actRaceJoinGame;
    extern NeuroIXWebsocket::Action actRaceStartGame;
    extern NeuroIXWebsocket::Action actRaceFocusFirst;
    extern NeuroIXWebsocket::Action actRaceFocusSecond;
    extern NeuroIXWebsocket::Action actRaceFocusThird;
    extern NeuroIXWebsocket::Action actRotateCamera;
    extern NeuroIXWebsocket::Action actResultMainMenu;
    extern NeuroIXWebsocket::Action actResultNextRandomMap;

    extern NeuroIXWebsocket::Action* actRaceGetJoinedPlayers;
    extern NeuroIXWebsocket::Action* actSetGlobalGravity;
    extern NeuroIXWebsocket::Action* actSetMarbleMass;
    extern NeuroIXWebsocket::Action* actKickPlayer;

    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);
}

// Royale Routing
namespace Mode_Royale
{
    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);
}

// Enum for the Registry
enum class EActionRegistry
{
    Register,
    Unregister
};

void RegisterAnAction(EActionRegistry operation, ENeuroState previousStage, ENeuroState currentStage, NeuroMarbles& client);