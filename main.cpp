#pragma once
#include "main.h" 
#include "Gamemodes/GameModes.h"

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

    Logger::Init();

    printf("==========================================\n");
    printf("     MARBLES ON STREAM - NEURO BOT\n");
    printf("==========================================\n");

    MarblesGameState gameState{};
    NeuroMarbles client("ws://localhost:8000", GameName, gameState, &std::cout, &std::cerr);

    client.sendStartup();

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

            if (gameState.bNeuroDidAction)
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
                if (searchTick % 2500 == 0) 
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