#pragma once
#include "Menu.h"
#include "GameModes.h"

extern ENeuroState lastState;

namespace Mode_Menu 
{

    // Define Menu-specific actions
    nlohmann::json empty_schema;
    NeuroWebsocketpp::Action actContinue("click_welcome_continue", "Click to bypass the promo screen!", empty_schema);
    NeuroWebsocketpp::Action actSelectRace("select_race_mode", "Select the standard Race Game Mode!", empty_schema);

    // NeuroWebsocketpp::Action actSelectRoyale("select_royale_mode", "Select the Royale Game Mode!", empty_schema);

    void ProcessAction(NeuroMarbles& client, MarblesGameState& state)
    {
        switch (state.CurrentState)
        {
        case STAGE_Welcome_Continue:
            if (state.LastNeuroAction.compare("click_welcome_continue") == 0)
            {
                if (ClickGenericButton("ContinueButton"))
                {
                    printf("[Neuro] Back to Gamemode selection!\n");
                    state.CurrentState = STAGE_Gamemode_Select;
                    Sleep(1000);
                }
                state.bNeuroDidAction = false;
            }
            break;

        case STAGE_Gamemode_Select:
            if (state.LastNeuroAction.compare("select_race_mode") == 0)
            {
                if (SelectExperienceCard("Race"))
                {
                    printf("[Neuro] STAGE 2: Race mode selected.\n");
                    state.CurrentState = STAGE_Race_Map_Select;
                    Sleep(1000);
                }
                state.bNeuroDidAction = false;
                
            }
            /*
            else if (SelectExperienceCard("Royale"))
            {
                printf("[Neuro] STAGE 2: Royale mode selected.\n");
                state.CurrentState = STAGE_Royale_Map_Select;
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            */
            break;
        }
    }

    void ProcessIdle(NeuroMarbles& client, MarblesGameState& state, int searchTick)
    {
        if (state.CurrentState != lastState)
        {
            RegisterAnAction(EActionRegistry::Unregister, lastState, state.CurrentState, client);
            RegisterAnAction(EActionRegistry::Register, lastState, state.CurrentState, client);
            
            switch (state.CurrentState)
            {
            case STAGE_Welcome_Continue:
                client.sendContext("We are currently on the title screen. Use your action to click continue and get us to the main menu!", true);
                break;

            case STAGE_Gamemode_Select:
                client.sendContext("We are at the game mode selection screen. Please select the gamemode for chat to play!", true);
                break;
            }

            lastState = state.CurrentState;
        }

        /*if (searchTick % 1500 == 0)
        {
            switch (state.CurrentState)
            {
            case STAGE_Welcome_Continue:
                client.sendContext("We are waiting on the title screen. Can you click continue?", false);
                break;
            case STAGE_Gamemode_Select:
                client.sendContext("We need to pick a game mode. Please use your action to select a gamemode!", false);
                break;
            }
        }*/
    }

}

