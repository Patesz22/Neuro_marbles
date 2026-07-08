#pragma once
#include "GameRaceActions.h"
#include "MOSAPI_classes.hpp"

volatile bool bShouldClickStart = false;
volatile bool bClickAcknowledged = false;
volatile bool bMatchIDIntercepted = false;
bool bIsMenuHooked = false;
bool bIsHeartbeatHooked = false;

typedef void(__fastcall* tProcessEvent)(const SDK::UObject*, SDK::UFunction*, void*);
tProcessEvent OriginalProcessEvent = nullptr;

void __fastcall Hooked_ProcessEvent(const SDK::UObject* pThis, SDK::UFunction* Function, void* Parms)
{
    if (bShouldClickStart)
    {
        bShouldClickStart = false;

        // Safely find the RaceMenu natively on the Game Thread
        SDK::UW_RaceUserFacingExperience_C* RaceMenu = nullptr;
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

            if (Obj->IsA(SDK::UW_RaceUserFacingExperience_C::StaticClass()) && !Obj->IsDefaultObject())
            {
                if (Obj->GetFullName().find("Transient") != std::string::npos)
                {
                    RaceMenu = static_cast<SDK::UW_RaceUserFacingExperience_C*>(Obj);
                    break;
                }
            }
        }

        if (RaceMenu && RaceMenu->StartButton)
        {
            SDK::UFunction* BlueprintClickEvent = SDK::UObject::FindObject<SDK::UFunction>("Function W_RaceUserFacingExperience.W_RaceUserFacingExperience_C.BndEvt__W_RaceUserFacingExperience_StartButton_K2Node_ComponentBoundEvent_6_CommonButtonBaseClicked__DelegateSignature");

            if (BlueprintClickEvent && OriginalProcessEvent)
            {
                struct FClickParams { void* Button; };
                FClickParams Params;
                Params.Button = RaceMenu->StartButton;

                printf("[Neuro] Engine Heartbeat Triggered! Injecting Blueprint...\n");
                OriginalProcessEvent(RaceMenu, BlueprintClickEvent, &Params);
                bClickAcknowledged = true;
            }
        }
    }

    // 3. Always call the original function!
    if (OriginalProcessEvent)
    {
        OriginalProcessEvent(pThis, Function, Parms);
    }
}

bool StartRaceMatch()
{
    SDK::UW_RaceUserFacingExperience_C* RaceMenu = nullptr;
    SDK::APlayerController* HeartbeatController = nullptr;

    // 1. Find both the Menu and the PlayerController safely
    for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

        int32_t dummy; float dummyF;
        if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &dummy, &dummyF)) continue;

        // Find Menu
        if (!RaceMenu && Obj->IsA(SDK::UW_RaceUserFacingExperience_C::StaticClass()) && !Obj->IsDefaultObject())
        {
            if (Obj->GetFullName().find("Transient") != std::string::npos)
            {
                RaceMenu = static_cast<SDK::UW_RaceUserFacingExperience_C*>(Obj);
            }
        }
        // Find PlayerController
        else if (!HeartbeatController && Obj->IsA(SDK::APlayerController::StaticClass()) && !Obj->IsDefaultObject())
        {
            HeartbeatController = static_cast<SDK::APlayerController*>(Obj);
        }

        // Stop searching early if we found both!
        if (RaceMenu && HeartbeatController) break;
    }

    if (!RaceMenu)
    {
        printf(">> [STAGE 4 ERROR] Could not find active Race Menu.\n");
        return false;
    }
    if (!HeartbeatController)
    {
        printf(">> [STAGE 4 ERROR] Could not find PlayerController for Heartbeat.\n");
        return false;
    }

    // 2. Hook the Race Menu (For MatchID Sniffing)
    if (!bIsMenuHooked)
    {
        void** VTable = *(void***)RaceMenu;
        OriginalProcessEvent = (tProcessEvent)VTable[SDK::Offsets::ProcessEventIdx];

        DWORD oldProtect;
        VirtualProtect(&VTable[SDK::Offsets::ProcessEventIdx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        VTable[SDK::Offsets::ProcessEventIdx] = &Hooked_ProcessEvent;
        VirtualProtect(&VTable[SDK::Offsets::ProcessEventIdx], sizeof(void*), oldProtect, &oldProtect);

        bIsMenuHooked = true;
        printf(">> [STAGE 4] VTable Hook successfully installed into Race Menu.\n");
    }

    // 3. Hook the Player Controller (For the Engine Heartbeat)
    if (!bIsHeartbeatHooked)
    {
        void** VTable = *(void***)HeartbeatController;

        DWORD oldProtect;
        VirtualProtect(&VTable[SDK::Offsets::ProcessEventIdx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        VTable[SDK::Offsets::ProcessEventIdx] = &Hooked_ProcessEvent;
        VirtualProtect(&VTable[SDK::Offsets::ProcessEventIdx], sizeof(void*), oldProtect, &oldProtect);

        bIsHeartbeatHooked = true;
        printf(">> [STAGE 4] VTable Hook successfully installed into Player Controller (Heartbeat Active).\n");
    }

    // 4. Queue the click for the Game Thread to process
    printf(">> [STAGE 4] Flag raised. Heartbeat will process click instantly...\n");
    bShouldClickStart = true;
    bClickAcknowledged = false;

    // The mouse_event hack is GONE. The PlayerController will trigger the hook on the very next frame natively.

    return true;
}


bool PressInGameButton(const std::string& buttonName)
{
    // Cache the array size to prevent out-of-bounds loops
    int initialObjectCount = SDK::UObject::GObjects->Num();

    // Iterate backwards to hit the active, top-layer UI first
    for (int i = initialObjectCount - 1; i >= 0; --i)
    {
        if (i >= SDK::UObject::GObjects->Num()) break;

        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        // Target the specific Button Class from your log
        if (Obj->IsA(SDK::UW_MOSButton_Small_C::StaticClass()))
        {
            std::string fullName = Obj->GetFullName();

            // Filter for the active Join Button in the Race Form
            if (fullName.find(buttonName) != std::string::npos &&
                fullName.find("RaceFormWidget") != std::string::npos &&
                fullName.find("Transient") != std::string::npos)
            {
                printf(">> [STAGE 5] Executing Native Click. <<\n");

                // Cast to the small button class
                SDK::UW_MOSButton_Small_C* JoinBtn = static_cast<SDK::UW_MOSButton_Small_C*>(Obj);

                // Because we are in a stable lobby, it is safe to play the full visual animation sequence
                JoinBtn->HandleButtonPressed();
                JoinBtn->HandleButtonClicked();
                JoinBtn->HandleButtonReleased();

                return true;
            }
        }
    }

    return false;
}


int GetRaceTotalPlayerCount()
{
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    if (!World) return 0;

    SDK::AGameModeBase* CurrentGameMode = World->AuthorityGameMode;

    // Check if we are in the standard Marble Race mode
    if (CurrentGameMode && CurrentGameMode->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
    {
        // Cast to the standard mode
        SDK::AMarbleRaceGameMode* RaceMode = static_cast<SDK::AMarbleRaceGameMode*>(CurrentGameMode);

        // Access the variable directly!
        return RaceMode->PlayersJoined;
    }

    return 0;
}


int GetRaceDeadPlayerCount()
{
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    if (!World) return 0;

    SDK::AGameModeBase* CurrentGameMode = World->AuthorityGameMode;

    // Check if we are in the standard Marble Race mode
    if (CurrentGameMode && CurrentGameMode->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
    {
        // Cast to the standard mode
        SDK::AMarbleRaceGameMode* RaceMode = static_cast<SDK::AMarbleRaceGameMode*>(CurrentGameMode);

        // Access the variable directly!
        return RaceMode->PlayersEliminated;
    }

    return 0;
}


int GetRaceFinishedPlayerCount()
{
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    if (!World) return 0;

    SDK::AGameModeBase* CurrentGameMode = World->AuthorityGameMode;

    // Check if we are in the standard Marble Race mode
    if (CurrentGameMode && CurrentGameMode->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
    {
        // Cast to the standard mode
        SDK::AMarbleRaceGameMode* RaceMode = static_cast<SDK::AMarbleRaceGameMode*>(CurrentGameMode);

        // Access the variable directly!
        return RaceMode->PlayersFinished;
    }

    return 0;
}


void ForceProceedToResults()
{
    SDK::URaceWinnerWidgetY2* winnerWidget = nullptr;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (Obj && Obj->IsA(SDK::URaceWinnerWidgetY2::StaticClass()) && !Obj->IsDefaultObject())
        {
            winnerWidget = static_cast<SDK::URaceWinnerWidgetY2*>(Obj);
            break;
        }
    }

    if (winnerWidget)
    {
        printf("[Neuro] Forcing game out of 'Awaiting' state and triggering Continue...\n");

        winnerWidget->SetIsAwaitingMatchResults(false);
        winnerWidget->OnContinueButtonClicked();
    }
    else
    {
        printf("RaceWinnerWidgetY2 not found in memory.\n");
    }
}


std::vector<RaceResult> ExtractResultsFromListView(int maxPlayersToFetch)
{
    std::vector<RaceResult> resultsArray(maxPlayersToFetch);
    SDK::URaceResultsWidgetY2* resultsWidget = nullptr;

    // 1. Sweep BACKWARDS to find the LIVE Results Widget (Ignores Ghosts!)
    for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

        int32_t d; float df;
        if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

        if (Obj->IsA(SDK::URaceResultsWidgetY2::StaticClass()) && !Obj->IsDefaultObject())
        {
            // Ensure we are grabbing the Transient (on-screen) widget
            if (Obj->GetFullName().find("Transient") != std::string::npos)
            {
                resultsWidget = static_cast<SDK::URaceResultsWidgetY2*>(Obj);
                break;
            }
        }
    }

    if (!resultsWidget || !resultsWidget->RaceResultsListView)
    {
        printf("[!] Could not find active Race Results Widget.\n");
        return resultsArray;
    }

    auto* listView = resultsWidget->RaceResultsListView;
    int totalPlayers = listView->ListItems.Num();

    // 2. Loop through the players
    for (int i = 0; i < totalPlayers; i++)
    {
        SDK::UObject* rawItem = listView->ListItems[i];
        if (!rawItem) continue;

        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(rawItem);

        int32_t rank = *reinterpret_cast<int32_t*>(baseAddr + 0xC8);
        float metricValue = *reinterpret_cast<float*>(baseAddr + 0xCC); // Time OR Distance
        int32_t statusFlag = *reinterpret_cast<int32_t*>(baseAddr + 0xF0);

        int placementIndex = rank - 1;

        if (placementIndex >= 0 && placementIndex < maxPlayersToFetch)
        {
            resultsArray[placementIndex].Rank = rank;

            // Optional: Print raw memory for the top 3 to verify status flags
            if (rank <= 3)
            {
                printf("[DEBUG] Rank %d Raw -> StatusFlag: %d | Metric: %.3f\n", rank, statusFlag, metricValue);
            }

            // Status Logic (Assuming > 0 means finished)
            if (statusFlag > 0)
            {
                resultsArray[placementIndex].RaceTime = metricValue;

                int minutes = static_cast<int>(std::floor(metricValue / 60.0f));
                int seconds = static_cast<int>(std::floor(std::fmod(metricValue, 60.0f)));
                int milliseconds = static_cast<int>(std::round(std::fmod(metricValue, 1.0f) * 1000.0f));

                printf("[Neuro] Player %d finished! Time: %02d:%02d.%03d\n", rank, minutes, seconds, milliseconds);
            }
            else
            {
                resultsArray[placementIndex].RaceTime = -1.0f;
                printf("[Neuro] Player %d DNF'd! Distance: %.2f\n", rank, metricValue);
            }

            // RAW MEMORY STRING EXTRACTION
            uintptr_t primaryNameAddr = baseAddr + 0x38;
            uintptr_t fallbackNameAddr = baseAddr + 0x28;

            wchar_t* nameData = *reinterpret_cast<wchar_t**>(primaryNameAddr);
            int32_t nameCount = *reinterpret_cast<int32_t*>(primaryNameAddr + 8);

            bool bNameFound = false;

            // Try to extract the Primary Display Name (0x38)
            if (nameData != nullptr && nameCount > 0 && nameCount < 100)
            {
                wchar_t tempBuffer[128] = { 0 };
                size_t bytesToCopy = (nameCount - 1) * sizeof(wchar_t);

                if (SafeCopyMemory(tempBuffer, nameData, bytesToCopy))
                {
                    resultsArray[placementIndex].PlayerName = std::wstring(tempBuffer, nameCount - 1);
                    bNameFound = true;
                }
            }

            // Try the Fallback Internal Name (0x28) if the Primary failed
            if (!bNameFound)
            {
                wchar_t* fallbackData = *reinterpret_cast<wchar_t**>(fallbackNameAddr);
                int32_t fallbackCount = *reinterpret_cast<int32_t*>(fallbackNameAddr + 8);

                if (fallbackData != nullptr && fallbackCount > 0 && fallbackCount < 100)
                {
                    wchar_t tempBuffer[128] = { 0 };
                    size_t bytesToCopy = (fallbackCount - 1) * sizeof(wchar_t);

                    if (SafeCopyMemory(tempBuffer, fallbackData, bytesToCopy))
                    {
                        resultsArray[placementIndex].PlayerName = std::wstring(tempBuffer, fallbackCount - 1);
                        bNameFound = true;
                    }
                }
            }

            if (!bNameFound)
            {
                resultsArray[placementIndex].PlayerName = L"Unknown";
            }

            resultsArray[placementIndex].bIsValid = true;
        }
    }

    return resultsArray;
}


void ProcessRaceResults(const std::string& actionId, const int playersWanted)
{
    std::vector<RaceResult> topPlayers = ExtractResultsFromListView(playersWanted);

    printf("\n========== MARBLES ON STREAM: RACE RESULTS ==========\n");

    int validCount = 0;
    for (int i = 0; i < playersWanted; i++)
    {
        if (topPlayers[i].bIsValid)
        {
            validCount++;
            printf("#%d - %S (Score: %d | Time: %.2fs)\n",
                topPlayers[i].Rank,
                topPlayers[i].PlayerName.c_str(),
                topPlayers[i].Score,
                topPlayers[i].RaceTime);
        }
    }

    if (validCount == 0) {
        printf("[!] No results found. Is the race finished?\n");
    }
    printf("=====================================================\n\n");
}


void TurnCamera()
{
    SDK::APlayerCameraManager* cameraManager = nullptr;

    // 1. Find the active Camera Manager
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (Obj && Obj->IsA(SDK::APlayerCameraManager::StaticClass()) && !Obj->IsDefaultObject())
        {
            cameraManager = static_cast<SDK::APlayerCameraManager*>(Obj);
            break;
        }
    }

    if (cameraManager && cameraManager->PCOwner)
    {
        SDK::APlayerController* playerController = cameraManager->PCOwner;

        SDK::FRotator currentRot = playerController->K2_GetActorRotation();

        printf("Pitch: %lf, Yaw: %lf, Roll: %lf", currentRot.Pitch, currentRot.Yaw, currentRot.Roll);
        currentRot.Pitch = -89.0f; // top down view
        currentRot.Yaw = -113.0f;

        // ?? always look in the ball roll direction ??

        cameraManager->FreeCamDistance = 1500.0f;
        //// Force the controller's physical body to rotate
        playerController->K2_SetActorRotation(currentRot, false);

        //// Force the camera manager's physical body to rotate
        cameraManager->K2_SetActorRotation(currentRot, false);

        // Force the internal engine "Look" rotation (This is usually the most important one in Free Cam!)
        playerController->SetControlRotation(currentRot);
    }
    else
    {
        printf("[!] Camera Manager or Player Controller not found.\n");
    }
}