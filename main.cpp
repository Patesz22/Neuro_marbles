#include "main.h"
#include "websocketpp/client.hpp"
#include "NeuroGameSdkWebsocketpp.hpp"

bool bBotActive = false;
std::wstring ActiveMatchID = L"";

std::string GameName = "Marbles on Stream";

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
};


class NeuroMarbles : public NeuroWebsocketpp::NeuroGameClient 
{
private:
    MarblesGameState& state;
    NeuroMarbles(const NeuroMarbles&) = delete;
    NeuroMarbles& operator=(const NeuroMarbles&) = delete;

public:
    NeuroMarbles(const std::string& uri, const std::string& game_name, MarblesGameState& gameState,
        std::ostream* output_stream = &std::cout, std::ostream* error_stream = &std::cerr) : NeuroGameClient(uri, game_name, output_stream, error_stream), state(gameState) {}

    bool isWaitingForForcedAction() const 
    {
        return waitingForForcedAction; // Grabs the protected variable from the SDK
    }

protected:
    void handleMessage(const NeuroWebsocketpp::NeuroResponse& response) override
    {
        // Intercept startup acknowledgement
        if (response.getCommand() == "startup")
        {
            state.ActiveCharacterId = response.getCharacterId();
            state.ActiveCharacterName = response.getDisplayName();
            std::cout << "Neuro connected! Character: " << response.getDisplayName() << " (Session: " << response.getSessionId() << ")" << std::endl;
            return;
        }

        if (response.getCommand() != "action")
        {
            std::cout << "Not an action!";
            return;
        }

        std::string raw_data = response.getData();
        std::string actionName = "";
        nlohmann::json parsed_data;

        if (raw_data.empty() || raw_data == "undefined" || raw_data == "null")
        {
            // {} empty schema 
            parsed_data = nlohmann::json::object();
        }
        else
        {
            try
            {
                parsed_data = nlohmann::json::parse(raw_data);
            }
            catch (const nlohmann::json::parse_error& e)
            {
                std::string resp = "Your JSON data is malformed and could not be parsed.";
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                sendActionResult(response, false, resp);
                return;
            }
        }

        // Validate the Schema based on Action Name
        try
        {
            actionName = response.getName();

            if (actionName == "example_complex_action")
            {
                if (!parsed_data.is_object() || !parsed_data.contains("target_id") || !parsed_data["target_id"].is_string())
                {
                    throw std::invalid_argument("Missing or invalid parameter: 'target_id' (expected string).");
                }
            }
            else
            {
                // DEFAULT ROUTE: Schema {}
            }
        }
        catch (const std::exception& e)
        {
            std::string resp = std::string("JSON validation failed: ") + e.what();
            std::cerr << resp << std::endl;
            sendActionResult(response, false, resp);
            return;
        }

        std::string resp = "";
        bool bSuccess = false;

        if (state.LastNeuroAction.data())
        {
            printf("%s\n", state.LastNeuroAction.data());
        }

        // Check if her action matches the stage we are currently in
        if (waitingForForcedAction)
        {
            if (actionName == "click_welcome_continue" && state.CurrentState == STAGE_Welcome_Continue)
            {
                resp = "We are continuing to the main menu!";
                bSuccess = true;
            }
            else if (actionName == "select_race_mode" && state.CurrentState == STAGE_Gamemode_Select)
            {
                resp = "Selected Race mode!";
                bSuccess = true;
            }
            else if (actionName == "randomize_race_map" && state.CurrentState == STAGE_Race_Map_Select)
            {
                resp = "Spinning the wheel for a random map!";
                bSuccess = true;
            }
            else if (actionName == "race_start_lobby" && state.CurrentState == STAGE_Race_Lobby_Start)
            {
                resp = "Opening the lobby!";
                bSuccess = true;
            }
            else if (actionName == "race_join_game" && state.CurrentState == STAGE_Race_Game_Joining)
            {
                resp = "I have joined the race!";
                bSuccess = true;
            }
            else
            {
                resp = "That's not a valid move for this menu screen!";
                bSuccess = false;
            }

            if (bSuccess)
            {
                sendUnregisterActions(disposableActions);
                state.LastNeuroAction = actionName;
                state.bNeuroDidAction = true; // Tell the main thread to execute the click!
                waitingForForcedAction = false;
            }
            sendActionResult(response, bSuccess, resp);
        }
        else
        {
            if (actionName == "race_start_game" && state.CurrentState == STAGE_Race_Game_Waiting)
            {
                resp = "I have started the race! Let's roll!";
                bSuccess = true;
            }
            else if (actionName == "race_focus_first_place" && state.CurrentState == STAGE_Race_Game_Started)
            {
                resp = "I am currently watching first place. Let's cheer for them!";
                bSuccess = true;
            }
            else if (actionName == "race_focus_second_place" && state.CurrentState == STAGE_Race_Game_Started)
            {
                resp = "I am currently watching second place. Let's cheer for them!";
                bSuccess = true;
            }
            else if (actionName == "race_focus_third_place" && state.CurrentState == STAGE_Race_Game_Started)
            {
                resp = "I am currently watching third place. Let's cheer for them!";
                bSuccess = true;
            }
            else if (actionName == "rotate_cam" && state.CurrentState == STAGE_Race_Game_Started)
            {
                resp = "Rotating the cam so chat can see better.";
                bSuccess = true;
            }
            else
            {
                resp = "That's not a valid move for this menu screen!";
                bSuccess = false;
            }

            if (bSuccess)
            {
                state.LastNeuroAction = actionName;
                state.bNeuroDidAction = true; // Tell your game thread to process this action
            }

            sendActionResult(response, bSuccess, resp);
        }
    }
};


DWORD WINAPI MainThread(LPVOID lpReserved)
{
    // Wait 5 seconds for the game and ASI loader to fully initialize into RAM
    Sleep(5000);

    // Setup the Console safely
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);

    printf("==========================================\n");
    printf("      MARBLES ON STREAM - TOOL SUITE      \n");
    printf("==========================================\n");
    printf("- Left Click: Inspect Hovered UI Widget\n");
    printf("- Numpad 1:   Toggle Neuro Auto-Pilot\n");
    printf("- End Key:    Eject Mod\n");
    printf("------------------------------------------\n");

    // ==========================================
    // NEURO SDK INITIALIZATION
    // ==========================================
    MarblesGameState gameState{};
    printf("1\n");
    NeuroMarbles client("ws://localhost:8000", "Marbles on Stream", gameState, &std::cout, &std::cerr);
    printf("2\n");
    // Define empty schema (no complex parameters required for simple button clicks)
    //nlohmann::json empty_schema;
    //empty_schema["type"] = "object";

    // Define Neuro's Available Actions
    NeuroWebsocketpp::Action actContinue("click_welcome_continue", "Click to bypass the promo screen!", {});
    NeuroWebsocketpp::Action actSelectRace("select_race_mode", "Select the standard Race Game Mode!", {});
    NeuroWebsocketpp::Action actRaceRandomMap("randomize_race_map", "Pick a random track to race on!", {});
    NeuroWebsocketpp::Action actRaceStartLobby("race_start_lobby", "Start the lobby so players can join!", {});
    NeuroWebsocketpp::Action actRaceJoinGame("race_join_game", "Spawn your own marble into the race!", {});
    NeuroWebsocketpp::Action actRaceStartGame("race_start_game", "Start the game on the current map, but WAIT until the lobby is full!", {});

    NeuroWebsocketpp::Action actRaceFocusFirst("race_focus_first_place", "Focus the first place player in view", {});
    NeuroWebsocketpp::Action actRaceFocusSecond("race_focus_second_place", "Focus the second place player in view", {});
    NeuroWebsocketpp::Action actRaceFocusThird("race_focus_third_place", "Focus the third place player in view", {});
    NeuroWebsocketpp::Action actRotateCamera("rotate_cam", "Rotate the camera so chat can see", {});

    // Connect and Handshake
    client.sendStartup();
    printf("3\n");

    bool bWasNumpad1Pressed = false;
    bool bWasNumpad2Pressed = false;
    bool bWasNumpad3Pressed = false;

    bool bWasLeftClicked = false;
    static int searchTick = 0;
    int joinedPlayers = 0;
    int lobbyFull = false;
    int maxLobbySize = -1;

    static bool bIsWaitingForResults = false;
    static ULONGLONG raceFinishTime = 0;

    std::cout << gameState.ActiveCharacterId << gameState.ActiveCharacterName << std::endl;

    while (true)
    {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
        {
            if (!bWasLeftClicked)
            {
                bWasLeftClicked = true;
                InspectUIWidgets();
            }
        }
        else
        {
            bWasLeftClicked = false;
        }

        // Toggle Neuro
        if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
        {
            if (!bWasNumpad1Pressed)
            {
                bWasNumpad1Pressed = true;
                PressInGameButton("StartButton");
                bBotActive = !bBotActive;
                printf("\n>>> Neuro Auto-Pilot: %s <<<\n", bBotActive ? "ENABLED" : "DISABLED");
            }
        }
        else if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
        {
            if (!bWasNumpad2Pressed)
            {
                bWasNumpad2Pressed = true;
                printf("\nNum2 pressed\n");
                PressKey('1');
            }
        }
        else if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
        {
            if (!bWasNumpad3Pressed)
            {
                bWasNumpad3Pressed = true;
                printf("\nNum3 pressed\n");
            }
        }
        else
        {
            bWasNumpad1Pressed = false;
            bWasNumpad2Pressed = false;
            bWasNumpad3Pressed = false;
        }

        // ------------------------------------------
        // NEURO STATE MACHINE
        // ------------------------------------------
        if (bBotActive)
        {
            // If Neuro is currently deciding what to do, do not interrupt her.
            if (client.isWaitingForForcedAction())
            {
                Sleep(100); // Just idle the thread
                continue;
            }

            // If Neuro just authorized an action
            if (gameState.bNeuroDidAction)
            {
                switch (gameState.CurrentState)
                {
                case STAGE_Welcome_Continue:
                    if (ClickGenericButton("ContinueButton"))
                    {
                        printf("[Neuro] STAGE 1: Bypassed Promo Screen.\n");
                        gameState.CurrentState = STAGE_Gamemode_Select;
                        gameState.bNeuroDidAction = false; // Reset flag
                        Sleep(1000); // Wait for ui update
                    }
                    break;

                case STAGE_Gamemode_Select:
                    if (SelectExperienceCard("Race"))
                    {
                        printf("[Neuro] STAGE 2: Game mode selected.\n");
                        gameState.CurrentState = STAGE_Race_Map_Select;
                        gameState.bNeuroDidAction = false;
                        Sleep(1000);
                    }
                    break;

                case STAGE_Race_Map_Select:
                    if (ClickNativeRandomButton())
                    {
                        printf("[Neuro] STAGE 3: Map selected.\n");
                        gameState.CurrentState = STAGE_Race_Lobby_Start;
                        gameState.bNeuroDidAction = false;
                        Sleep(1000);
                    }
                    break;

                case STAGE_Race_Lobby_Start:
                {
                    
                    maxLobbySize = GetMaxLobbySize();
                    printf("Lobby size is: %d", maxLobbySize);

                    if (StartRaceMatch())
                    {
                        printf("[Neuro] STAGE 4: Start initiated! Entering match...\n");
                        gameState.bNeuroDidAction = false;

                        while (!bClickAcknowledged)
                        {
                            Sleep(100);
                        }

                        printf("[Bot] Blueprint Injected! Waiting for map to load...\n");

                        Sleep(8000);
                        printf("Waiting finished! Skipping intros...\n");
                        PressKey(VK_SPACE);
                        gameState.CurrentState = STAGE_Race_Game_Joining;
                    
                    }
                    break;
                }

                case STAGE_Race_Game_Joining:
                    if (PressInGameButton("JoinButton"))
                    {
                        printf("[Neuro] STAGE 5: Neuro joined the game!\n");
                        gameState.CurrentState = STAGE_Race_Game_Waiting;
                        gameState.bNeuroDidAction = false;
                        client.sendRegisterActions({ actRaceStartGame });
                        
                    }
                    break;

                case STAGE_Race_Game_Waiting:
                {
                    if (PressInGameButton("StartButton"))
                    {
                        gameState.CurrentState = STAGE_Race_Game_Started;
                        gameState.bNeuroDidAction = false;
                        client.sendUnregisterActions({ "race_start_game" });
                        client.sendRegisterActions({actRaceFocusFirst, actRaceFocusSecond, actRaceFocusThird, actRotateCamera});
                    }
                    break;
                }
                    
                case STAGE_Race_Game_Started:
                {
                    if (strcmp(gameState.LastNeuroAction.data(), "race_focus_first_place") == 0)
                    {
                        PressKey('1');
                        gameState.bNeuroDidAction = false;
                        Sleep(1000);
                        break;
                    }
                    else if (strcmp(gameState.LastNeuroAction.data(), "race_focus_second_place") == 0)
                    {
                        PressKey('2');
                        gameState.bNeuroDidAction = false;
                        Sleep(1000);
                        break;
                    }
                    else if (strcmp(gameState.LastNeuroAction.data(), "race_focus_third_place") == 0)
                    {
                        PressKey('3');
                        gameState.bNeuroDidAction = false;
                        Sleep(1000);
                        break;
                    }
                    else if (strcmp(gameState.LastNeuroAction.data(), "rotate_cam") == 0)
                    {
                        TurnCamera();
                        Sleep(1000);
                        gameState.bNeuroDidAction = false;
                        break;
                    }
                    // action to focus the first (second, third) player(s)
                    break;
                }
                    
                case STAGE_Race_Game_Finished:
                {
                    break;
                }

                }
            }

            // If Neuro hasn't done anything yet, prompt her based on where we are.
            else
            {
                if (searchTick % 80 == 0) // every 8 sec
                {
                    char buffer[128];
                    printf("[Neuro DEBUG] Prompting/Waiting Neuro for state: %d\n", gameState.CurrentState);
                    if ((gameState.CurrentState == STAGE_Race_Game_Waiting) && !lobbyFull)
                    {
                        if ((joinedPlayers + 1) <= GetRaceTotalPlayerCount())
                        {
                            snprintf(buffer, sizeof(buffer), "Chat is joining... %d have joined out of %d.", GetRaceTotalPlayerCount(), maxLobbySize);
                            client.sendContext(std::string(buffer), false);
                            joinedPlayers = GetRaceTotalPlayerCount();
                        }
                        else if (joinedPlayers == maxLobbySize)
                        {
                            std::string str = "The lobby is full! Please start the map!";
                            client.sendContext(str, false);
                            lobbyFull = true;
                        }
                    }
                }

                switch (gameState.CurrentState)
                {
                case STAGE_Welcome_Continue:
                    client.forceDisposableActions(
                        "We are currently on the title screen.",
                        "Can you click continue to get us to the main menu?",
                        false, { actContinue }
                    );
                    break;

                case STAGE_Gamemode_Select:
                    client.forceDisposableActions(
                        "We are at the game mode selection screen.",
                        "Please select the standard Race mode for chat.",
                        false, { actSelectRace }
                    );
                    break;

                case STAGE_Race_Map_Select:
                    client.forceDisposableActions(
                        "We need to pick a track to race on.",
                        "Let's make it random! Click the randomize map button.",
                        false, { actRaceRandomMap }
                    );
                    break;

                case STAGE_Race_Lobby_Start:
                    client.forceDisposableActions(
                        "The track has been selected.",
                        "Start the lobby so chat can join!",
                        false, { actRaceStartLobby }
                    );
                    break;

                case STAGE_Race_Game_Joining:
                    client.forceDisposableActions(
                        "The lobby is open. Chat can join by typeing !play in chat!",
                        "Don't forget to join the game yourself!",
                        false, { actRaceJoinGame }
                    );
                    break;

                case STAGE_Race_Game_Waiting:
                {
                    Sleep(100);
                    break;
                }
                    
                case STAGE_Race_Game_Started:
                {
                    if (!bIsWaitingForResults && ((GetRaceFinishedPlayerCount() + GetRaceDeadPlayerCount()) == GetRaceTotalPlayerCount()))
                    {
                        Sleep(3000);
                        printf("[Neuro] Race finished, continue button pressed.\n");

                        // Sometimes it gets stuck at "Awaiting results"
                        //ForceProceedToNextMap();
                        PressInGameButton("ContinueButton");

                        client.sendUnregisterActions({ "race_focus_first_place", "race_focus_second_place", "race_focus_third_place", "rotate_cam"});
                        gameState.CurrentState = STAGE_Race_Game_Finished;
                        Sleep(1000); // Wait for ui update
                        
                    }
                    else if (searchTick % 100 == 0) // every 10 sec (10tick / sec)
                    {
                        char buffer[256];
                        snprintf(buffer, sizeof(buffer),
                            "There are %d players alive, %d dead players and %d players have already finished.",
                            GetRaceTotalPlayerCount() - GetRaceDeadPlayerCount() - GetRaceFinishedPlayerCount(), GetRaceDeadPlayerCount(), GetRaceFinishedPlayerCount());
                        std::string str = buffer;
                        client.sendContext(str, true);
                    }
                    else if (searchTick % 250 == 0) // every 25 sec
                    {
                        char buffer[256];
                        snprintf(buffer, sizeof(buffer),
                            "There are %d players alive, %d dead players and %d players have already finished.",
                            GetRaceTotalPlayerCount() - GetRaceDeadPlayerCount() - GetRaceFinishedPlayerCount(), GetRaceDeadPlayerCount(), GetRaceFinishedPlayerCount());
                        std::string str = buffer;
                        client.sendContext(str, false);
                    }

                    Sleep(100);
                    break;
                }

                case STAGE_Race_Game_Finished:
                {
                    ProcessRaceResults("none", maxLobbySize);
                    gameState.CurrentState = STAGE_Race_Game_Finished_Waiting;
                    break;
                }

                case STAGE_Race_Game_Finished_Waiting:
                {

                    Sleep(100);
                    break;
                }

                }
            }
        }

        // ------------------------------------------
        // EJECTION HANDLER
        // ------------------------------------------
        if (GetAsyncKeyState(VK_END) & 1)
            break;

        searchTick++;
        Sleep(100); // 10 ticks a second
    }

    // Cleanup and Eject
    printf("Ejecting...\n");
    fclose(fDummy);
    FreeConsole();
    FreeLibraryAndExitThread(static_cast<HMODULE>(lpReserved), 0);
    return TRUE;
}

// ==========================================
// DLL ENTRY POINT
// ==========================================
BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hMod);
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}
