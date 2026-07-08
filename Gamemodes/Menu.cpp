#pragma once
#include "Menu.h"
#include "GameModes.h"

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
            if (ClickGenericButton("ContinueButton"))
            {
                printf("[Neuro] STAGE 1: Bypassed Promo Screen.\n");
                state.CurrentState = STAGE_Gamemode_Select;
                state.bNeuroDidAction = false;
                Sleep(1000);
            }
            break;

        case STAGE_Gamemode_Select:
            if (SelectExperienceCard("Race"))
            {
                printf("[Neuro] STAGE 2: Race mode selected.\n");
                state.CurrentState = STAGE_Race_Map_Select;
                state.bNeuroDidAction = false;
                Sleep(1000);
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
        switch (state.CurrentState)
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
                "Please select a game mode for chat. (Race or Royale)",
                false, { actSelectRace /*, actSelectRoyale */ }
            );
            break;
        }
    }

}

