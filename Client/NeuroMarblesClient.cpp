#pragma once
#include "NeuroMarblesClient.h"

// Constructor implementation
NeuroMarbles::NeuroMarbles(const std::string& uri, const std::string& game_name, MarblesGameState& gameState,
    std::ostream* output_stream, std::ostream* error_stream)
    : NeuroGameClient(uri, game_name, output_stream, error_stream), state(gameState)
{}

bool NeuroMarbles::isWaitingForForcedAction() const
{
    return waitingForForcedAction; // Grabs the protected variable from the SDK
}

void NeuroMarbles::handleMessage(const NeuroWebsocketpp::NeuroResponse& response)
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
            state.bNeuroDidAction = true;
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
        else if (actionName == "result_exit_main_menu" && state.CurrentState == STAGE_Race_Game_At_Results)
        {
            resp = "I have exited to the main menu.";
            bSuccess = true;
        }
        else if (actionName == "result_next_random_map" && state.CurrentState == STAGE_Race_Game_At_Results)
        {
            resp = "Starting the next race...";
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
            state.bNeuroDidAction = true;
        }

        sendActionResult(response, bSuccess, resp);
    }
}