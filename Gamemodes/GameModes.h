#pragma once
#include "../GameState.h"
#include "Client/NeuroMarblesClient.h"

// Menu Routing
namespace Mode_Menu 
{
    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);
}

// Race Routing
namespace Mode_Race 
{
    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);
}

// Royale Routing
namespace Mode_Royale 
{
    void ProcessAction(NeuroMarbles& client, MarblesGameState& state);
    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick);
}