#include "Gamemodes/GameModes.h"
#include "main.h"

void RegisterAStage(EActionRegistry operation, ENeuroState targetStage, NeuroMarbles& client)
{
	switch (targetStage)
	{
	case STAGE_Welcome_Continue:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Menu::actContinue });
		else
			client.sendUnregisterActions({ "click_welcome_continue" });
		break;

	case STAGE_Gamemode_Select:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Menu::actSelectRace }); // other gamemodes's actions here
		else
			client.sendUnregisterActions({ "select_race_mode" });
		break;

	case STAGE_Race_Map_Select:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Race::actRaceRandomMap, Mode_Race::actMenuGoBack });
		else
			client.sendUnregisterActions({ "randomize_race_map", "menu_go_back" });
		break;

	case STAGE_Race_Lobby_Start:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Race::actRaceStartLobby, Mode_Race::actMenuGoBack });
		else
			client.sendUnregisterActions({ "race_start_lobby", "menu_go_back" });
		break;

	case STAGE_Race_Game_Joining:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Race::actRaceJoinGame });
		else
			client.sendUnregisterActions({ "race_join_game" });
		break;

	case STAGE_Race_Game_Waiting:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Race::actRaceStartGame, *Mode_Race::actRaceGetJoinedPlayers, *Mode_Race::actKickPlayer });
		else
		{
			client.sendUnregisterActions({ "race_start_game", "get_joined_players", "kick_player" });
			Sleep(10000); // race start delay
		}
		break;

	case STAGE_Race_Game_Started:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Race::actRaceFocusFirst, Mode_Race::actRaceFocusSecond, Mode_Race::actRaceFocusThird, Mode_Race::actRotateCamera, *Mode_Race::actSetGlobalGravity, *Mode_Race::actSetMarbleMass, *Mode_Race::actSetMarbleSize });
		else
			client.sendUnregisterActions({ "race_focus_first_place", "race_focus_second_place", "race_focus_third_place", "rotate_cam", "set_global_gravity", "set_marble_mass", "set_marble_size" });
		break;

	case STAGE_Race_Game_At_Results:
		if (operation == EActionRegistry::Register)
			client.sendRegisterActions({ Mode_Race::actResultMainMenu, Mode_Race::actResultNextRandomMap });
		else
			client.sendUnregisterActions({ "result_exit_race_menu", "result_next_random_map" });
		break;
	}
}

void RegisterAnAction(EActionRegistry operation, ENeuroState previousStage, ENeuroState currentStage, NeuroMarbles& client)
{
	if (operation == EActionRegistry::Unregister)
	{
		// Because only one state is ever active at a time, we ONLY need to 
		// unregister the stage we are leaving (previousStage).
		if (previousStage != static_cast<ENeuroState>(-1))
		{
			RegisterAStage(EActionRegistry::Unregister, previousStage, client);
		}
	}
	else if (operation == EActionRegistry::Register)
	{
		// Always register the new stage we are entering
		RegisterAStage(EActionRegistry::Register, currentStage, client);
	}
}