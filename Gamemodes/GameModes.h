#pragma once
#include "../GameState.h"
#include "Client/NeuroMarblesClient.h"

// Menu Routing
namespace Mode_Menu 
{
    extern NeuroWebsocketpp::Action actContinue;
    extern NeuroWebsocketpp::Action actSelectRace;

    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);

    int GetMaxLobbySize();
}

// Race Routing
namespace Mode_Race 
{
    extern NeuroWebsocketpp::Action actMenuGoBack;

    extern NeuroWebsocketpp::Action actRaceRandomMap;
    extern NeuroWebsocketpp::Action actRaceStartLobby;
    extern NeuroWebsocketpp::Action actRaceJoinGame;
    extern NeuroWebsocketpp::Action actRaceStartGame;
    extern NeuroWebsocketpp::Action actRaceFocusFirst;
    extern NeuroWebsocketpp::Action actRaceFocusSecond;
    extern NeuroWebsocketpp::Action actRaceFocusThird;
    extern NeuroWebsocketpp::Action actRotateCamera;
    extern NeuroWebsocketpp::Action actResultMainMenu;
    extern NeuroWebsocketpp::Action actResultNextRandomMap;

    extern NeuroWebsocketpp::Action* actRaceGetJoinedPlayers;
    extern NeuroWebsocketpp::Action* actSetGlobalGravity;
    extern NeuroWebsocketpp::Action* actSetMarbleMass;

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
enum class EActionRegistry {
    Register,
    Unregister
};

void RegisterAnAction(EActionRegistry operation, ENeuroState previousStage, ENeuroState currentStage, NeuroMarbles& client);