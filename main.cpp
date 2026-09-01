#pragma once
#include "main.h" 
#include "Gamemodes/GameModes.h"
#include "Twitch/Twitch.h"

// Globals
bool bBotActive = false;
std::wstring ActiveMatchID = L"";
std::string GameName = "Marbles on Stream";
ENeuroState lastState = static_cast<ENeuroState>(-1);

bool IsMenuState(ENeuroState state)
{
	return state == STAGE_Welcome_Continue || state == STAGE_Gamemode_Select;
}

bool IsRaceState(ENeuroState state)
{

	return state >= STAGE_Race_Map_Select && state <= STAGE_Race_Game_At_Results;
}
bool IsRoyaleState(ENeuroState state)
{
	return false; // Implement 
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	Sleep(5000);
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	LoadConfig();
	Logger::Init();

	printf("==========================================\n");
	printf("     MARBLES ON STREAM - NEURO INTEGRATION\n");
	printf("==========================================\n");

	std::cout << "\n\nCurrently in WIDGET DEBUG mode!\nPress NUM1 to activate / deactivate the actual INTEGRATION!\n This will be removed on full release.\n\n";

	std::cout << "Connecting to websocket..." << std::endl;
	Mode_Race::InitComplexActions();
	MarblesGameState gameState{};
	NeuroMarbles client("ws://localhost:8000", GameName, gameState, &std::cout, &std::cerr);

	client.sendStartup();

	std::cout << "Websocket connection successful!" << std::endl;

	TwitchChatClient Twitch;
	std::string TwitchChannelName = GetConfigString("TwitchChannelname", "vedal987");
	Twitch.Start(TwitchChannelName);

	std::cout << "Twitch connection successful!" << std::endl;

	bool bWasNumpad1Pressed = false;
	bool bWasLeftClicked = false;
	static long int searchTick = 0;

	while (true)
	{
		// HARDWARE INPUTS
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			if (!bWasLeftClicked) { bWasLeftClicked = true; InspectUIWidgets(); }
		}
		else { bWasLeftClicked = false; }

		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
		{
			if (!bWasNumpad1Pressed)
			{
				bWasNumpad1Pressed = true;
				bBotActive = !bBotActive;
				printf("\n>>> Neuro Auto-Pilot: %s <<<\n", bBotActive ? "ENABLED" : "DISABLED");
			}
		}
		else { bWasNumpad1Pressed = false; }

		if (GetAsyncKeyState(VK_END) & 1) break;

		// THE ROUTER
		if (bBotActive)
		{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				Checking for Twitch manual override
// 
			TwitchOverride currentCmd;
			bool hasCommand = false;

			{
				std::lock_guard<std::mutex> lock(OverrideMutex);
				if (!OverrideQueue.empty())
				{
					currentCmd = OverrideQueue.front();
					OverrideQueue.pop();
					hasCommand = true;
				}
			}

			if (hasCommand)
			{
				if (currentCmd.CommandType == "STATE")
				{
					try
					{
						// "!override_state 5" forces STAGE_Race_Game_Started
						int newState = std::stoi(currentCmd.Payload);
						gameState.CurrentState = static_cast<ENeuroState>(newState);
						printf("[Twitch] Forced State Change to: %d\n", newState);
					}
					catch (...)
					{
						printf("[Twitch] Invalid state ID.\n");
					}
				}
				else if (currentCmd.CommandType == "BUTTON")
				{
					// "!press_button StartButton"
					Mode_Race::PressInGameButton(currentCmd.Payload);
					printf("[Twitch] Pressed Button: %s\n", currentCmd.Payload.c_str());
				}
				else if (currentCmd.CommandType == "ACTION")
				{
					// "!exec_action set_global_gravity"
					// might need fake JSON
					gameState.LastNeuroAction = currentCmd.Payload;
					gameState.NeuroDidAction = true;
					printf("[Twitch] Forced Action: %s\n", currentCmd.Payload.c_str());
				}
			}

//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (gameState.NeuroDidAction)
			{
				// Route Actions
				if (IsMenuState(gameState.CurrentState))
					Mode_Menu::ProcessAction(client, gameState);

				else if (IsRaceState(gameState.CurrentState))
					Mode_Race::ProcessAction(client, gameState);

				else if (IsRoyaleState(gameState.CurrentState))
					Mode_Royale::ProcessAction(client, gameState);
			}
			else
			{
				if (searchTick % 3500 == 0)
				{
					printf("[Neuro DEBUG] Waiting for Neuro at state: %d\n", gameState.CurrentState);
				}

				// Route Idle / Context gathering
				if (IsMenuState(gameState.CurrentState))
					Mode_Menu::ProcessIdle(client, gameState, searchTick);

				else if (IsRaceState(gameState.CurrentState))
					Mode_Race::ProcessIdle(client, gameState, searchTick);

				else if (IsRoyaleState(gameState.CurrentState))
					Mode_Royale::ProcessIdle(client, gameState, searchTick);
			}
		}

		searchTick++;
		Sleep(10);
	}

	printf("Ejecting...\n");

	Mode_Race::FreeComplexActions();
	Logger::Shutdown();
	fclose(fDummy);
	FreeConsole();
	FreeLibraryAndExitThread(static_cast<HMODULE>(lpReserved), 0);
	return TRUE;
}

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