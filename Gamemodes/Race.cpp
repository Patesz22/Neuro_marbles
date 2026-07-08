#pragma once
#include "GameModes.h"
#include "../main.h"
#include "Menu.h"
#include "Race.h"

namespace Mode_Race
{

    static int joinedPlayers = 0;
    static bool lobbyFull = false;
    static int maxLobbySize = -1;
    static bool bIsWaitingForResults = false;

    // Race-Specific Neuro Actions
    nlohmann::json empty_schema;
    NeuroWebsocketpp::Action actRaceRandomMap("randomize_race_map", "Pick a random track to race on!", empty_schema);
    NeuroWebsocketpp::Action actRaceStartLobby("race_start_lobby", "Start the lobby so players can join!", empty_schema);
    NeuroWebsocketpp::Action actRaceJoinGame("race_join_game", "Spawn your own marble into the race!", empty_schema);
    NeuroWebsocketpp::Action actRaceStartGame("race_start_game", "Start the game on the current map, but WAIT until the lobby is full!", empty_schema);
    NeuroWebsocketpp::Action actRaceFocusFirst("race_focus_first_place", "Focus the first place player in view", empty_schema);
    NeuroWebsocketpp::Action actRaceFocusSecond("race_focus_second_place", "Focus the second place player in view", empty_schema);
    NeuroWebsocketpp::Action actRaceFocusThird("race_focus_third_place", "Focus the third place player in view", empty_schema);
    NeuroWebsocketpp::Action actRotateCamera("rotate_cam", "Rotate the camera so chat can see", empty_schema);

    void ProcessAction(NeuroMarbles& client, MarblesGameState& state)
    {
        switch (state.CurrentState)
        {
        case STAGE_Race_Map_Select:
            if (Mode_Menu::ClickRandomizeButton())
            {
                printf("[Neuro] STAGE 3: Map selected.\n");
                state.CurrentState = STAGE_Race_Lobby_Start;
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            break;

        case STAGE_Race_Lobby_Start:
            maxLobbySize = Mode_Menu::GetMaxLobbySize();
            printf("Lobby size is: %d\n", maxLobbySize);

            if (StartRaceMatch())
            {
                printf("[Neuro] STAGE 4: Start initiated! Entering match...\n");
                state.bNeuroDidAction = false;

                while (!bClickAcknowledged) { Sleep(100); }

                printf("[Bot] Blueprint Injected! Waiting for map to load...\n");
                Sleep(8000);
                printf("Waiting finished! Skipping intros...\n");
                PressKey(VK_SPACE);
                state.CurrentState = STAGE_Race_Game_Joining;

                // Reset lobby variables for the new race
                joinedPlayers = 0;
                lobbyFull = false;
            }
            break;

        case STAGE_Race_Game_Joining:
            if (PressInGameButton("JoinButton"))
            {
                printf("[Neuro] STAGE 5: Neuro joined the game!\n");
                state.CurrentState = STAGE_Race_Game_Waiting;
                state.bNeuroDidAction = false;
                client.sendRegisterActions({ actRaceStartGame });
            }
            break;

        case STAGE_Race_Game_Waiting:
            if (PressInGameButton("StartButton"))
            {
                state.CurrentState = STAGE_Race_Game_Started;
                state.bNeuroDidAction = false;
                client.sendUnregisterActions({ "race_start_game" });
                client.sendRegisterActions({ actRaceFocusFirst, actRaceFocusSecond, actRaceFocusThird, actRotateCamera });
            }
            break;

        case STAGE_Race_Game_Started:
            if (strcmp(state.LastNeuroAction.c_str(), "race_focus_first_place") == 0)
            {
                PressKey('1');
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            else if (strcmp(state.LastNeuroAction.c_str(), "race_focus_second_place") == 0)
            {
                PressKey('2');
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            else if (strcmp(state.LastNeuroAction.c_str(), "race_focus_third_place") == 0)
            {
                PressKey('3');
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            else if (strcmp(state.LastNeuroAction.c_str(), "rotate_cam") == 0)
            {
                TurnCamera();
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            break;

        case STAGE_Race_Game_Finished:
        case STAGE_Race_Game_At_Results:
            // Handled in Idle / Post-Match logic
            break;
        }
    }

    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick)
    {
        // 1. High-frequency Context Updates
        if (searchTick % 80 == 0)
        {
            if ((state.CurrentState == STAGE_Race_Game_Waiting) && !lobbyFull)
            {
                if ((joinedPlayers + 1) <= GetRaceTotalPlayerCount())
                {
                    char buffer[128];
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
        }

        // 2. Standard State Handling
        switch (state.CurrentState)
        {
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
            Sleep(100);
            break;

        case STAGE_Race_Game_Started:
            if (!bIsWaitingForResults && ((GetRaceFinishedPlayerCount() + GetRaceDeadPlayerCount()) == GetRaceTotalPlayerCount()))
            {
                state.CurrentState = STAGE_Race_Game_Finished;
                client.sendUnregisterActions({ "race_focus_first_place", "race_focus_second_place", "race_focus_third_place", "rotate_cam" });
            }
            else if (searchTick % 100 == 0)
            {
                char buffer[256];
                snprintf(buffer, sizeof(buffer),
                    "There are %d players alive, %d dead players and %d players have already finished.",
                    GetRaceTotalPlayerCount() - GetRaceDeadPlayerCount() - GetRaceFinishedPlayerCount(), GetRaceDeadPlayerCount(), GetRaceFinishedPlayerCount());
                client.sendContext(std::string(buffer), (searchTick % 200 != 0)); // Send true/false alternately
            }
            Sleep(100);
            break;

        case STAGE_Race_Game_Finished:
            Sleep(3000);
            PressInGameButton("ContinueButton");
            printf("[Neuro] Race finished, continue button pressed.\n");

            Sleep(1000);
            ProcessRaceResults(maxLobbySize);
            state.CurrentState = STAGE_Race_Game_At_Results;
            break;

        case STAGE_Race_Game_At_Results:
            client.sendRegisterActions({});
            Sleep(100);
            break;
        }
    }


}
