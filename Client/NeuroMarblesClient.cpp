#include "NeuroMarblesClient.h"
#include "Gamemodes/Menu.h"
#include "Gamemodes/GameModes.h"
#include "Gamemodes/Race.h"

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
    state.lastData = response.getData();
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

    try
    {
        actionName = response.getName();
    }
    catch (const std::exception& e)
    {
        std::string resp = std::string("JSON validation failed: ") + e.what();
        std::cout << resp << std::endl;
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
    std::cout << "State: " << state.CurrentState << std::endl;
    if (actionName == "click_welcome_continue" && state.CurrentState == STAGE_Welcome_Continue)
    {
        resp = "We are continuing to the main menu!";
        bSuccess = true;
    }
    else if (actionName == "menu_go_back" && (state.CurrentState == STAGE_Race_Map_Select || state.CurrentState == STAGE_Race_Lobby_Start))
    {
        resp = "Returning to the gamemode selection screen!";
        bSuccess = true;
    }
    else if (actionName == "click_welcome_continue" && state.CurrentState == STAGE_Welcome_Continue)
    {
        resp = "Bypassed welcome screen!";
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
        if (Mode_Race::IsRaceJoinable()) 
        {
            resp = "I have joined the race!";
            bSuccess = true;
        }
        else 
        {
            resp = "The race has not finished loading yet, please wait a bit.";
            bSuccess = false;
        }
        
    }
    else if (actionName == "race_start_game" && state.CurrentState == STAGE_Race_Game_Waiting)
    {
        resp = "I have started the race! Let's roll! Don't forget to rotate the camera next, so chat can see the race better.";
        bSuccess = true;
    }
    else if (actionName == "get_joined_players" && state.CurrentState == STAGE_Race_Game_Waiting)
    {
        bool correct = false;
        int requestedAmount = 0;

        if (!state.lastData.empty())
        {
            try
            {
                nlohmann::json parsedData = nlohmann::json::parse(state.lastData);

                if (parsedData.contains("amount") && parsedData["amount"].is_number())
                {
                    requestedAmount = parsedData["amount"].get<int>();

                    // Validate bounds
                    if (requestedAmount >= 1 && requestedAmount <= 1000)
                    {
                        correct = true;
                    }
                    else
                    {
                        printf("[DEBUG] Amount out of bounds: %d\n", requestedAmount);
                    }
                }
                else
                {
                    printf("[DEBUG] Payload missing 'amount' or not a number.\n");
                }
            }
            catch (const std::exception& e)
            {
                // Print the exact error so we know if the parse fails!
                printf("[DEBUG] JSON Validation Exception: %s\n", e.what());
                correct = false;
            }
        }
        else
        {
            printf("[DEBUG] state.lastData was empty!\n");
        }

        //validate here
        if (correct) // if successfully validated
        {
            resp = "The data is correct, getting the requested number of players.";
            bSuccess = true;
        }
        else // if incorrent
        {
            resp = "The data format is incorrect, please try again!";
            bSuccess = false;
        }

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
    else if (actionName == "set_global_gravity" && state.CurrentState == STAGE_Race_Game_Started)
    {
        bool correct = false;
        double requestedAmount = 0.0;
        int requestedDuration = 0;
        bool onCooldown = false;

        if (!state.lastData.empty())
        {
            try
            {
                nlohmann::json parsedData = nlohmann::json::parse(state.lastData);

                if (parsedData.is_string())
                {
                    parsedData = nlohmann::json::parse(parsedData.get<std::string>());
                }

                if (parsedData.is_object() &&
                    parsedData.contains("amount") && parsedData["amount"].is_number() &&
                    parsedData.contains("duration") && parsedData["duration"].is_number_integer())
                {
                    requestedAmount = parsedData["amount"].get<double>();
                    requestedDuration = parsedData["duration"].get<int>();

                    // Validate bounds
                    if (requestedAmount >= -1500.0 && requestedAmount <= 100.0 && requestedDuration >= 1 && requestedDuration <= 5)
                    {
                        long long currentTime = GetCurrentTimeMs();
                        long long cooldownExpiry = 0;

                        if (GetGlobalCooldowns().contains("gravity_cooldown"))
                        {
                            cooldownExpiry = GetGlobalCooldowns()["gravity_cooldown"].get<long long>();
                        }

                        if (currentTime < cooldownExpiry)
                        {
                            int remainingSeconds = (cooldownExpiry - currentTime) / 1000;
                            bSuccess = false;
                            onCooldown = true;
                        }
                        else
                        {
                            correct = true; // Validation passed, not on cooldown!
                            onCooldown = false;
                        }
                    }
                    else
                    {
                        printf("[DEBUG] Amount or duration out of bounds: %f, %d\n", requestedAmount, requestedDuration);
                    }
                }
                else
                {
                    printf("[DEBUG] JSON is missing 'amount' or 'duration', or they are the wrong types.\n");
                }
            }
            catch (const std::exception& e)
            {
                printf("[DEBUG] JSON Parse Error: %s\n", e.what());
            }
        }

        if (correct)
        {
            long long currentTime = GetCurrentTimeMs();
            long long newCooldownExpiry = currentTime + (requestedDuration * 1000) + (60 * 1000); //+ 60s cooldown
            GetGlobalCooldowns()["gravity_cooldown"] = newCooldownExpiry;

            resp = "Global gravity has been altered successfully!";
            bSuccess = true;
            onCooldown = false;
        }
        else if (!correct && onCooldown)
        {
            resp = "I can't do that right now! The gravity command is on cooldown!";
            bSuccess = false;
        }
        else
        {
            resp = "The data format is incorrect, please try again!";
            bSuccess = false;
        }
    }
    else if (actionName == "set_marble_mass" && state.CurrentState == STAGE_Race_Game_Started)
    {
        bool correct = false;
        std::string requestedUsername = "";
        double requestedAmount = 0.0;

        if (!state.lastData.empty())
        {
            try
            {
                nlohmann::json parsedData = nlohmann::json::parse(state.lastData);
                if (parsedData.is_string()) 
                    parsedData = nlohmann::json::parse(parsedData.get<std::string>());

                if (parsedData.is_object() &&
                    parsedData.contains("username") && parsedData["username"].is_string() &&
                    parsedData.contains("amount") && parsedData["amount"].is_number())
                {
                    requestedUsername = parsedData["username"].get<std::string>();
                    requestedAmount = parsedData["amount"].get<double>();

                    long long currentTime = GetCurrentTimeMs();
                    long long cooldownExpiry = 0;

                    if (GetGlobalCooldowns().contains("mass_cooldown"))
                    {
                        cooldownExpiry = GetGlobalCooldowns()["mass_cooldown"].get<long long>();
                    }

                    if (currentTime < cooldownExpiry)
                    {
                        int remainingSeconds = (cooldownExpiry - currentTime) / 1000;
                        resp = "I can't do that right now! The mass command is on cooldown for another " + std::to_string(remainingSeconds) + " seconds.";
                        bSuccess = false;
                    }
                    else
                    {
                        correct = true;
                    }
                }
            }
            catch (const std::exception& e)
            {
                printf("[DEBUG] JSON Parse Error: %s\n", e.what());
            }
        }

        if (correct)
        {
            // fixed 15s cooldown!
            long long currentTime = GetCurrentTimeMs();
            GetGlobalCooldowns()["mass_cooldown"] = currentTime + (15 * 1000);

            resp = "Attempting to change the mass for " + requestedUsername + "!";
            bSuccess = true;
        }
        else
        {
            resp = "The data format is incorrect. Please provide a string 'username' and numeric 'amount'.";
            bSuccess = false;
        }
    }
    else if (actionName == "result_exit_race_menu" && state.CurrentState == STAGE_Race_Game_At_Results)
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
        resp = "That's not a valid move for this state!";
        bSuccess = false;
    }

    if (bSuccess)
    {
        state.LastNeuroAction = actionName;
        state.NeuroDidAction = true;
    }

    // Send the API response back to Neuro
    sendActionResult(response, bSuccess, resp);
    
}