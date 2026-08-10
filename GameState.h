
#pragma once
#include <string>

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
    STAGE_Race_Game_At_Results
};

// Global variable declarations
extern bool bBotActive;
extern std::wstring ActiveMatchID;
extern std::string GameName;

extern volatile bool bShouldClickStart;
extern volatile bool bClickAcknowledged;
extern volatile bool bMatchIDIntercepted;

// Holds our current state so both the Main Thread and the WebSocket thread can see it
class MarblesGameState
{
public:
    ENeuroState CurrentState = STAGE_Welcome_Continue;
    bool bNeuroDidAction = false; // Flips to true when Neuro successfully sends a command
    bool successfulLastAction = false;

    std::string LastNeuroAction = "";
    std::string ActiveCharacterId;
    std::string ActiveCharacterName;
    std::string lastData = "";
};