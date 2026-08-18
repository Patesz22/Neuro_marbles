#pragma once
#include "GameModes.h"
#include "main.h"
#include "Menu.h"
#include "Race.h"
#include "misc.h"

extern ENeuroState lastState;

namespace Mode_Race
{

    static int joinedPlayers = 0;
    static bool lobbyFull = false;
    static bool joinableSent = false;
    static int maxLobbySize = -1;
    static bool bIsWaitingForResults = false;
    static bool sentFinished1st = false;

    static std::wstring currentObservedLeader = L"";
    static std::wstring previousLeader = L"";
    static int consecutiveLeaderSeconds = 0;
    static std::wstring lastAnnouncedLeader = L"";
    static int lastAnnouncementTick = 0;
    static int lastDeadCount = 0;

    // Race-Specific Neuro Actions
    nlohmann::json empty_schema;
    NeuroWebsocketpp::Action actMenuGoBack("menu_go_back", "Go back to the gamemode selection menu screen.", empty_schema);

    NeuroWebsocketpp::Action actRaceRandomMap("randomize_race_map", "Pick a random track to race on!", empty_schema);
    NeuroWebsocketpp::Action actRaceStartLobby("race_start_lobby", "Start the lobby so players can join!", empty_schema);
    NeuroWebsocketpp::Action actRaceJoinGame("race_join_game", "Spawn your own marble into the race!", empty_schema);
    NeuroWebsocketpp::Action actRaceStartGame("race_start_game", "Start the game on the current map, but WAIT until the lobby is full!", empty_schema);

    NeuroWebsocketpp::Action actRaceFocusFirst("race_focus_first_place", "Focus the first place player in view", empty_schema);
    NeuroWebsocketpp::Action actRaceFocusSecond("race_focus_second_place", "Focus the second place player in view", empty_schema);
    NeuroWebsocketpp::Action actRaceFocusThird("race_focus_third_place", "Focus the third place player in view", empty_schema);
    NeuroWebsocketpp::Action actRotateCamera("rotate_cam", "Rotate the camera so chat can see", empty_schema);
    NeuroWebsocketpp::Action actResultMainMenu("result_exit_race_menu", "Exit to main menu after seeing the results.", empty_schema);
    NeuroWebsocketpp::Action actResultNextRandomMap("result_next_random_map", "Start the next map, randomly", empty_schema);


    void ProcessAction(NeuroMarbles& client, MarblesGameState& state)
    {
        if (state.LastNeuroAction.compare("menu_go_back") == 0)
        {
            if (Mode_Menu::ClickGenericButton("BackButton"))
            {
                printf("[Action]: STAGE 1: Back button pressed.\n");
                lastState = state.CurrentState;
                state.CurrentState = STAGE_Gamemode_Select;
                RegisterAnAction(EActionRegistry::Unregister, lastState, state.CurrentState, client);
                RegisterAnAction(EActionRegistry::Register, lastState, state.CurrentState, client);
                Sleep(1500);
            }
            state.NeuroDidAction = false;
        }

        switch (state.CurrentState)
        {
        case STAGE_Race_Map_Select:
            if (Mode_Menu::ClickRandomizeButton())
            {
                printf("[Action]: STAGE 3: Map selected.\n");
                state.CurrentState = STAGE_Race_Lobby_Start;
                state.NeuroDidAction = false;
                Sleep(1000);
            }
            break;

        case STAGE_Race_Lobby_Start:
            maxLobbySize = Mode_Menu::GetMaxLobbySize();
            printf("[Action] Lobby size is: %d\n", maxLobbySize);

            if (StartRaceMatch())
            {
                printf("[Action]: STAGE 4: Start initiated! Entering match...\n");
                state.NeuroDidAction = false;

                while (!ClickAcknowledged) 
                { Sleep(100); }

                printf("[Action] Waiting for map to load...\n");
                state.CurrentState = STAGE_Race_Game_Joining;

                // Reset lobby variables for the new race
                joinedPlayers = 0;
                lobbyFull = false;
                sentFinished1st = false;
                currentObservedLeader = L"";
                previousLeader = L"";
                consecutiveLeaderSeconds = 0;
                lastAnnouncedLeader = L"";
                lastAnnouncementTick = 0;
                lastDeadCount = 0;
            }
            break;

        case STAGE_Race_Game_Joining:
            if (PressInGameButton("JoinButton"))
            {
                printf("[Action]: STAGE 5: Neuro joined the game!\n");
                state.CurrentState = STAGE_Race_Game_Waiting;
                state.NeuroDidAction = false;
                sentFinished1st = false;
                currentObservedLeader = L"";
                previousLeader = L"";
                consecutiveLeaderSeconds = 0;
                lastAnnouncedLeader = L"";
                lastAnnouncementTick = 0;
            }
            break;

        case STAGE_Race_Game_Waiting:
            if (state.LastNeuroAction.compare("race_start_game") == 0)
            {
                if (PressInGameButton("StartButton"))
                {
                    state.CurrentState = STAGE_Race_Game_Started;
                    state.NeuroDidAction = false;
                }
            }
            else if (state.LastNeuroAction.compare("get_joined_players") == 0)
            {
                int requestedAmount = 10; // Default fallback

                if (!state.lastData.empty())
                {
                    try
                    {
                        nlohmann::json parsedData = nlohmann::json::parse(state.lastData);

                        if (parsedData.is_string())
                        {
                            parsedData = nlohmann::json::parse(parsedData.get<std::string>());
                        }

                        // Safely verify the key exists before extracting
                        if (parsedData.is_object() && parsedData.contains("amount") && parsedData["amount"].is_number_integer())
                        {
                            requestedAmount = parsedData["amount"].get<int>();
                        }
                    }
                    catch (const std::exception& e)
                    {
                        printf("[DEBUG] JSON Parse Error in get_joined_players: %s\n", e.what());
                    }
                }

                std::string playersList = GetJoinedPlayers(requestedAmount);

                printf("[Action]: Fetched %d joined players: %s\n", requestedAmount, playersList.c_str());
                std::string tmp = "Joined players: ";
                tmp.append(playersList);
                client.sendContext(tmp, false);
                tmp.clear();

                state.NeuroDidAction = false;
            }
            else if (state.LastNeuroAction.compare("kick_player") == 0)
            {
                try
                {
                    nlohmann::json parsedData = nlohmann::json::parse(state.lastData);
                    if (parsedData.is_string()) parsedData = nlohmann::json::parse(parsedData.get<std::string>());

                    std::string requestedUsername = parsedData["username"].get<std::string>();

                    SDK::AMarbleRaceGameMode* ActiveGameMode = nullptr;

                    // Find the active GameMode 
                    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
                    {
                        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
                        if (!Obj || Obj->IsDefaultObject()) continue;

                        if (Obj->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
                        {
                            ActiveGameMode = static_cast<SDK::AMarbleRaceGameMode*>(Obj);
                            break;
                        }
                    }

                    if (ActiveGameMode)
                    {
                        // Convert to Unreal Wide String
                        std::wstring wUsername(requestedUsername.begin(), requestedUsername.end());
                        SDK::FString ueUsername(wUsername.c_str());

                        // The KickPlayer function expects both Username and DisplayName.
                        // Supplying the username to both works perfectly.
                        ActiveGameMode->KickPlayer(ueUsername, ueUsername);

                        printf("[Action]: Kicked player: %s\n", requestedUsername.c_str());
                    }
                    else
                    {
                        printf("[Action]: Could not find active GameMode to kick player: %s\n", requestedUsername.c_str());
                    }
                }
                catch (const std::exception& e)
                {
                    printf("[DEBUG] JSON Parse Error in kick_player execution: %s\n", e.what());
                }

                state.NeuroDidAction = false;
            }
            break;

        case STAGE_Race_Game_Started:
        {
            SDK::AMarbleRaceGameMode* GameMode = GetMarbleGameMode();
            if (GameMode && GameMode->bRaceFinished)
            {
                state.CurrentState = STAGE_Race_Game_Finished;
                break;
            }

            if (state.LastNeuroAction.compare("race_focus_first_place") == 0)
            {
                PressKey('1');

                std::string tmp = "Currently viewing ";
                tmp.append(GetSpectatedPlayerName());
                client.sendContext(tmp, false);
                tmp.clear();

                state.NeuroDidAction = false;
            }
            else if (state.LastNeuroAction.compare("race_focus_second_place") == 0)
            {
                PressKey('2');
                std::string tmp = "Currently viewing ";
                tmp.append(GetSpectatedPlayerName());
                client.sendContext(tmp, false);
                tmp.clear();

                state.NeuroDidAction = false;
            }
            else if (state.LastNeuroAction.compare("race_focus_third_place") == 0)
            {
                PressKey('3');
                std::string tmp = "Currently viewing ";
                tmp.append(GetSpectatedPlayerName());
                client.sendContext(tmp, false);
                tmp.clear();

                state.NeuroDidAction = false;
            }
            else if (state.LastNeuroAction.compare("set_global_gravity") == 0)
            {
                try
                {
                    nlohmann::json parsedData = nlohmann::json::parse(state.lastData);

                    if (parsedData.is_string())
                    {
                        parsedData = nlohmann::json::parse(parsedData.get<std::string>());
                    }

                    double requestedAmount = parsedData["amount"].get<double>();
                    int requestedDuration = parsedData["duration"].get<int>();

                    float previousGravity = ApplyGravity(static_cast<float>(requestedAmount));

                    // Only overwrite OriginalGravityZ if we aren't ALREADY overriding it
                    if (!state.IsGravityModified)
                    {
                        state.OriginalGravityZ = previousGravity;
                    }

                    state.IsGravityModified = true;

                    // Set the timer so the game knows when to turn it off!
                    state.GravityResetTime = std::chrono::steady_clock::now() + std::chrono::seconds(requestedDuration);
                }
                catch (const std::exception& e)
                {
                    printf("[DEBUG] JSON Parse Error in race.cpp: %s\n", e.what());
                }

                state.NeuroDidAction = false;
            }
            else if (state.LastNeuroAction.compare("set_marble_mass") == 0)
            {
                try
                {
                    nlohmann::json parsedData = nlohmann::json::parse(state.lastData);
                    if (parsedData.is_string()) parsedData = nlohmann::json::parse(parsedData.get<std::string>());

                    std::string requestedUsername = parsedData["username"].get<std::string>();
                    double requestedAmount = parsedData["amount"].get<double>();

                    SDK::AMarble* TargetMarble = FindMarble(requestedUsername);

                    if (TargetMarble)
                    {
                        float startingMass = 1.0f; // Fallback
                        if (TargetMarble->PrimitiveRootComponent)
                        {
                            startingMass = TargetMarble->PrimitiveRootComponent->GetMass();
                        }

                        TargetMarble->SetMassInKgs(static_cast<float>(requestedAmount));
                        printf("[Action]: Set %s's mass to %.2f\n", requestedUsername.c_str(), requestedAmount);

                        // Add to the list with a fixed 30-second duration!
                        state.ActiveMassModifiers.push_back({
                            TargetMarble,
                            startingMass,
                            std::chrono::steady_clock::now() + std::chrono::seconds(30)
                            });
                    }
                    else
                    {
                        printf("[Action]: Could not find marble for user: %s\n", requestedUsername.c_str());
                    }
                }
                catch (const std::exception& e)
                {
                    printf("[DEBUG] JSON Parse Error in set_marble_mass execution: %s\n", e.what());
                }

                state.NeuroDidAction = false;
            }
            else if (state.LastNeuroAction.compare("set_marble_size") == 0)
            {
                try
                {
                    nlohmann::json parsedData = nlohmann::json::parse(state.lastData);
                    if (parsedData.is_string()) parsedData = nlohmann::json::parse(parsedData.get<std::string>());

                    std::string requestedUsername = parsedData["username"].get<std::string>();
                    double requestedAmount = parsedData["amount"].get<double>();

                    SDK::AMarble* TargetMarble = FindMarble(requestedUsername);

                    if (TargetMarble)
                    {
                        // Build the FVector for uniform scaling
                        SDK::FVector newScale;
                        newScale.X = static_cast<float>(requestedAmount);
                        newScale.Y = static_cast<float>(requestedAmount);
                        newScale.Z = static_cast<float>(requestedAmount);

                        TargetMarble->SetActorScale3D(newScale);
                        printf("[Action]: Set %s's size to %.2f\n", requestedUsername.c_str(), requestedAmount);

                        // 1.0f is default scale, set for a 30-second duration
                        state.ActiveSizeModifiers.push_back(MarblesGameState::ActiveSizeModifier{
                            TargetMarble,
                            1.0f,
                            std::chrono::steady_clock::now() + std::chrono::seconds(30)
                            });
                    }
                    else
                    {
                        printf("[Action]: Could not find marble for user: %s\n", requestedUsername.c_str());
                    }
                }
                catch (const std::exception& e)
                {
                    printf("[DEBUG] JSON Parse Error in set_marble_size execution: %s\n", e.what());
                }

                state.NeuroDidAction = false;
                }
            else if (state.LastNeuroAction.compare("rotate_cam") == 0)
            {
                TurnCamera();
                state.NeuroDidAction = false;
            }
            break;

        }
            
        case STAGE_Race_Game_Finished:
            break;

        case STAGE_Race_Game_At_Results:
            if (state.LastNeuroAction.compare("result_exit_race_menu") == 0)
            {
                if (ClickReturnToRaceMenu()) 
                {
                    state.CurrentState = STAGE_Race_Map_Select;
                }
                state.NeuroDidAction = false;
                Sleep(1500);
            }
            else if (state.LastNeuroAction.compare("result_next_random_map") == 0)
            {
                if (ClickNextRandomTrack()) 
                {
                    state.NeuroDidAction = false;
                }
                state.CurrentState = STAGE_Race_Game_Joining;
            }
            break;
        }

        // State Machine Cleanup
        if (!state.NeuroDidAction)
        {
            state.LastNeuroAction = "";
            state.lastData = "";
        }
    }

    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick)
    {
        if ((state.CurrentState == STAGE_Race_Game_Joining) && IsRaceJoinable() && !joinableSent)
        {
            client.sendContext("The race has loaded, the lobby is joinable!", false);
            joinableSent = true;
        }

        if ((state.CurrentState == STAGE_Race_Game_Waiting) && !lobbyFull)
        {
            if ((joinedPlayers + 1) <= GetRaceTotalPlayerCount())
            {
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "Chat is joining... %d have joined out of %d.", GetRaceTotalPlayerCount(), maxLobbySize);
                client.sendContext(std::string(buffer), false);
                joinedPlayers = GetRaceTotalPlayerCount();
            }
            else if (joinedPlayers == maxLobbySize && maxLobbySize > 0)
            {
                client.sendContext("The lobby is full! Please start the map!", false);
                lobbyFull = true;
            }
        }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//          Checking cooldowns
// 

        if (state.IsGravityModified)
        {
            // Check if the duration has expired
            if (std::chrono::steady_clock::now() >= state.GravityResetTime)
            {
                ApplyGravity(state.OriginalGravityZ);
                state.IsGravityModified = false;

                std::cout << "[Action]: Gravity duration expired. Reset to normal" << state.OriginalGravityZ << std::endl;
            }
        }

        for (auto it = state.ActiveMassModifiers.begin(); it != state.ActiveMassModifiers.end(); )
        {
            if (std::chrono::steady_clock::now() >= it->ResetTime)
            {
                int32_t d; float df;
                if (it->MarblePtr && SafeRead4Bytes(reinterpret_cast<uintptr_t>(it->MarblePtr), &d, &df))
                {
                    // Restore the exact original mass we saved earlier!
                    it->MarblePtr->SetMassInKgs(it->OriginalMass);
                    std::cout << "[Action]: Marble mass duration expired. Reset to original" << it->OriginalMass << std::endl;
                }

                it = state.ActiveMassModifiers.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (auto it = state.ActiveSizeModifiers.begin(); it != state.ActiveSizeModifiers.end(); )
        {
            if (std::chrono::steady_clock::now() >= it->ResetTime)
            {
                int32_t d; float df;
                // Validate the pointer is still alive before touching it
                if (it->MarblePtr && SafeRead4Bytes(reinterpret_cast<uintptr_t>(it->MarblePtr), &d, &df))
                {
                    SDK::FVector normalScale;
                    normalScale.X = it->OriginalScale;
                    normalScale.Y = it->OriginalScale;
                    normalScale.Z = it->OriginalScale;

                    it->MarblePtr->SetActorScale3D(normalScale);
                    std::cout << "[Action]: Marble size duration expired. Reset to original" << it->OriginalScale << std::endl;
                }

                it = state.ActiveSizeModifiers.erase(it);
            }
            else
            {
                ++it;
            }
        }

//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (state.CurrentState != lastState)
        {
            RegisterAnAction(EActionRegistry::Unregister, lastState, state.CurrentState, client);
            RegisterAnAction(EActionRegistry::Register, lastState, state.CurrentState, client);

            switch (state.CurrentState)
            {
            case STAGE_Race_Map_Select:
                client.sendContext("We need to pick a track to race on. Let's make it random! Or, if you changed your mind, you can go back to mode selection.", true);
                break;
            case STAGE_Race_Lobby_Start:
                client.sendContext("The track has been selected. Start the lobby so chat can join, or go back if you want a different map!", true);
                break;
            case STAGE_Race_Game_Joining:
                client.sendContext("The lobby is opening... Chat can join by typing !play in chat! Don't forget to join the game yourself!", true);
                break;
            case STAGE_Race_Game_At_Results:
                client.sendContext("The race results are on screen. Would you like to return to menu or play another random map?", true);
                break;
            }

            lastState = state.CurrentState;
        }


        switch (state.CurrentState)
        {
            case STAGE_Race_Game_Started: 
            {

                if (!bIsWaitingForResults && ((GetRaceFinishedPlayerCount() + GetRaceDeadPlayerCount()) == GetRaceTotalPlayerCount()))
                {
                    state.CurrentState = STAGE_Race_Game_Finished;
                    break;
                }

                if (!sentFinished1st) 
                {
                    std::string firstplace = GetFirstPlaceFinishedPlayer();
                    if (firstplace.compare("") != 0) 
                    {
                        std::string tmp = "We have a first place finisher: ";
                        tmp.append(firstplace);
                        client.sendContext(tmp, false);
                        sentFinished1st = true;
                    }
                }
                if (searchTick % 100 == 0)
                {
                    int currentDead = GetRaceDeadPlayerCount();

                    // MASS CARNAGE
                    // If 5 or more marbles die within 2 second
                    if (currentDead - lastDeadCount >= 5 && (searchTick - lastAnnouncementTick) > 200)
                    {
                        char buffer[256];
                        snprintf(buffer, sizeof(buffer),
                            "%d marbles were just eliminated all at once!",
                            (currentDead - lastDeadCount));

                        client.sendContext(std::string(buffer), true);

                        lastAnnouncementTick = searchTick;
                        lastDeadCount = currentDead;
                        break;
                    }
                    lastDeadCount = currentDead;

                    // LEAD CHANGES & BATTLES
                    std::vector<RaceResult> topPlayers = ExtractLiveScoreboard(10);

                    if (!topPlayers.empty() && topPlayers[0].bIsValid)
                    {
                        std::wstring polledLeader = topPlayers[0].PlayerName;

                        // The Debounce: Did they hold the lead?
                        if (polledLeader == currentObservedLeader)
                        {
                            consecutiveLeaderSeconds++;
                        }
                        else
                        {
                            previousLeader = currentObservedLeader;
                            currentObservedLeader = polledLeader;
                            consecutiveLeaderSeconds = 1;
                        }

                        // The Announcement Trigger (Held for 3s + Not recently announced + 5s global cooldown)
                        if (consecutiveLeaderSeconds >= 3 &&
                            currentObservedLeader != lastAnnouncedLeader &&
                            (searchTick - lastAnnouncementTick) > 50)
                        {
                            char buffer[512];

                            // An overtake
                            if (!previousLeader.empty() && previousLeader != currentObservedLeader)
                            {
                                snprintf(buffer, sizeof(buffer),
                                    "%S just overtook %S for 1st place!",
                                    currentObservedLeader.c_str(), previousLeader.c_str());
                            }
                            // A close battle between 1st and 2nd
                            else if (topPlayers.size() >= 2 && topPlayers[1].bIsValid)
                            {
                                snprintf(buffer, sizeof(buffer),
                                    "%S has broken away from the pack and is leading the race, but %S is chasing right behind them in 2nd!",
                                    currentObservedLeader.c_str(), topPlayers[1].PlayerName.c_str());
                            }
                            // Dominating lead
                            else
                            {
                                snprintf(buffer, sizeof(buffer),
                                    "%S has taken a dominating lead!",
                                    currentObservedLeader.c_str());
                            }

                            // Send the prompt!
                            client.sendContext(std::string(buffer), false);
                            printf("[Neuro Context] %s\n", buffer);

                            lastAnnouncedLeader = currentObservedLeader;
                            lastAnnouncementTick = searchTick;
                        }
                    }
                }
                else if (searchTick % 500 == 0)
                {
                    char buffer[512];
                    snprintf(buffer, sizeof(buffer),
                        "There are %d players alive, %d dead players and %d players have already finished.",
                        GetRaceTotalPlayerCount() - GetRaceDeadPlayerCount() - GetRaceFinishedPlayerCount(), GetRaceDeadPlayerCount(), GetRaceFinishedPlayerCount());
                    client.sendContext(std::string(buffer), (searchTick % 100 != 0)); // Send true/false alternately
                }
                //Sleep(100);
                break;
            }
            

            case STAGE_Race_Game_Finished:
            {
                Sleep(3000);
                //PressInGameButton("ContinueButton"); // doesn't work

                ForceProceedToResults();
                printf("[Action]: Race finished, continue button pressed.\n");

                Sleep(1000);
                std::string top10 = ProcessRaceResults(10);

                std::string msg;
                msg.append("The race has finished. Out of the ");
                msg.append(std::to_string(GetRaceTotalPlayerCount()));
                msg.append(" players, only ");
                msg.append(std::to_string(GetRaceFinishedPlayerCount()));
                msg.append(" players have finished, ");
                msg.append(std::to_string(GetRaceDeadPlayerCount()));
                msg.append(" have died. The top 10 finishers are:\n");
                msg.append(top10);
                client.sendContext(msg, false);

                // Destroy the saved UE pointers
                state.ActiveMassModifiers.clear();
                state.ActiveSizeModifiers.clear();
                state.IsGravityModified = false;

                state.CurrentState = STAGE_Race_Game_At_Results;
                break;
            }
            

            case STAGE_Race_Game_At_Results:
                AutoScrollRaceResults(searchTick);
                break;
            }
        
    }


}
