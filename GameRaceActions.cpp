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


bool SelectExperienceCard(const std::string& targetModeName)
{
    SDK::UMOSUserFacingExperienceDescription* TargetData = nullptr;
    SDK::UW_UserFacingExperienceSelector_C* SelectorMenu = nullptr;

    // 1. Find the Selector Menu (Scan BACKWARDS for the active Transient UI)
    for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        if (Obj->IsA(SDK::UW_UserFacingExperienceSelector_C::StaticClass()))
        {
            std::string fullName = Obj->GetFullName();
            if (fullName.find("Transient") != std::string::npos)
            {
                SelectorMenu = static_cast<SDK::UW_UserFacingExperienceSelector_C*>(Obj);
                break; // Found the active menu container!
            }
        }
    }

    // 2. Find the Data Asset (Scan FORWARDS because Data Assets are persistent, not Transient)
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        // Note the class change: We are looking for UMOS... not UW_...
        if (Obj->IsA(SDK::UMOSUserFacingExperienceDescription::StaticClass()))
        {
            std::string objName = Obj->GetName();

            // Check if the internal data name contains "Race"
            if (objName.find(targetModeName) != std::string::npos)
            {
                printf(">> [STAGE 2] Locked onto Data Asset: %s <<\n", objName.c_str());
                TargetData = static_cast<SDK::UMOSUserFacingExperienceDescription*>(Obj);
                break; // Found the data!
            }
        }
    }

    // 3. Fire the Native Engine Event!
    if (TargetData && SelectorMenu)
    {
        printf(">> [STAGE 2] Firing the literal UI Mouse-Click Event! <<\n");

        // Holy function name
        SelectorMenu->BndEvt__W_UserFacingExperienceSelector_ExperienceDescriptionsListView_K2Node_ComponentBoundEvent_0_SimpleListItemEventDynamic__DelegateSignature(TargetData);

        return true;
    }

    return false;
}


bool ClickNativeRandomButton()
{
    SDK::UObject* ParentMapList = nullptr;
    SDK::UObject* RandomButton = nullptr;

    // 1. Find BOTH the MapList Container and the physical Random Button
    for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        if (Obj->IsA(SDK::UUserWidget::StaticClass()))
        {
            std::string fullName = Obj->GetFullName();

            if (!ParentMapList &&
                fullName.find("W_RaceExperienceMapList_C") != std::string::npos &&
                fullName.find("Transient") != std::string::npos)
            {
                ParentMapList = Obj;
            }

            if (!RandomButton &&
                fullName.find("RandomButton") != std::string::npos &&
                fullName.find("Transient") != std::string::npos)
            {
                RandomButton = Obj;
            }

            if (ParentMapList && RandomButton) break;
        }
    }

    if (!ParentMapList || !RandomButton) return false;

    // 2. Fetch the exact Developer Function from global memory
    SDK::UFunction* TargetFunc = nullptr;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (Obj && Obj->IsA(SDK::UFunction::StaticClass()))
        {
            if (Obj->GetName().find("BndEvt__W_RaceUserFacingExperience_RandomButton_K2Node_ComponentBoundEvent_3_CommonButtonBaseClicked__DelegateSignature") != std::string::npos)
            {
                TargetFunc = static_cast<SDK::UFunction*>(Obj);
                break;
            }
        }
    }

    // 3. The "Russian Roulette" Execution
    if (TargetFunc)
    {
        struct FCommonButtonBaseClicked_Params
        {
            SDK::UObject* Button;
        };

        FCommonButtonBaseClicked_Params Params;
        Params.Button = RandomButton;

        // Seed C++ with your PC's real-time clock
        srand((unsigned int)time(NULL));

        // Pick a random number of clicks between 15 and 65
        int randomClicks = (rand() % 50) + 15;

        printf(">> [STAGE 3] Defeating Static Seed! Firing Random Button %d times... <<\n", randomClicks);

        // Rapid-fire the native function to scramble the game's internal RNG state
        for (int i = 0; i < randomClicks; i++)
        {
            ParentMapList->ProcessEvent(TargetFunc, &Params);
        }

        printf(">> [STAGE 3] Map successfully randomized! <<\n");
        return true;
    }

    return false;
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


void ForceProceedToNextMap()
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
