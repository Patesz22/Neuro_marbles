#pragma once
#include <string>
#include <chrono>
#include <json/json.hpp>

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
extern volatile bool ClickAcknowledged;
extern volatile bool bMatchIDIntercepted;

inline nlohmann::json& GetGlobalCooldowns()
{
    static nlohmann::json instance = nlohmann::json::object();
    return instance;
}

inline long long GetCurrentTimeMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// FORWARD DECLARATION
namespace SDK
{
    class AMarble;
}
// Holds our current state so both the Main Thread and the WebSocket thread can see it
class MarblesGameState
{
public:
    ENeuroState CurrentState = STAGE_Welcome_Continue;
    bool NeuroDidAction = false; // Flips to true when Neuro successfully sends a command
    bool successfulLastAction = false;

    bool IsGravityModified = false;
    float OriginalGravityZ = -980.0f; // Default Unreal gravity
    std::chrono::time_point<std::chrono::steady_clock> GravityResetTime;

    struct ActiveMassModifier
    {
        SDK::AMarble* MarblePtr;
        float OriginalMass;
        std::chrono::time_point<std::chrono::steady_clock> ResetTime;
    };
    std::vector<ActiveMassModifier> ActiveMassModifiers;

    std::string LastNeuroAction = "";
    std::string ActiveCharacterId;
    std::string ActiveCharacterName;
    std::string lastData = "";
};

