#include "Race.h"
#include "../main.h"

namespace Mode_Race
{
    volatile bool bShouldClickStart = false;
    volatile bool ClickAcknowledged = false;
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
                    ClickAcknowledged = true;
                }
            }
        }

        if (OriginalProcessEvent)
        {
            OriginalProcessEvent(pThis, Function, Parms);
        }
    }


    bool StartRaceMatch()
    {
        SDK::UW_RaceUserFacingExperience_C* RaceMenu = nullptr;
        SDK::APlayerController* HeartbeatController = nullptr;

        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t dummy; float dummyF;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &dummy, &dummyF)) continue;

            if (!RaceMenu && Obj->IsA(SDK::UW_RaceUserFacingExperience_C::StaticClass()) && !Obj->IsDefaultObject())
            {
                if (Obj->GetFullName().find("Transient") != std::string::npos)
                {
                    RaceMenu = static_cast<SDK::UW_RaceUserFacingExperience_C*>(Obj);
                }
            }
            else if (!HeartbeatController && Obj->IsA(SDK::APlayerController::StaticClass()) && !Obj->IsDefaultObject())
            {
                HeartbeatController = static_cast<SDK::APlayerController*>(Obj);
            }

            if (RaceMenu && HeartbeatController) break;
        }

        if (!RaceMenu || !HeartbeatController)
        {
            printf(">> [STAGE 4 ERROR] Could not find Menu or PlayerController.\n");
            return false;
        }

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

        if (!bIsHeartbeatHooked)
        {
            void** VTable = *(void***)HeartbeatController;
            DWORD oldProtect;
            VirtualProtect(&VTable[SDK::Offsets::ProcessEventIdx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            VTable[SDK::Offsets::ProcessEventIdx] = &Hooked_ProcessEvent;
            VirtualProtect(&VTable[SDK::Offsets::ProcessEventIdx], sizeof(void*), oldProtect, &oldProtect);

            bIsHeartbeatHooked = true;
            printf(">> [STAGE 4] VTable Hook successfully installed into Player Controller.\n");
        }

        printf(">> [STAGE 4] Flag raised. Heartbeat will process click instantly...\n");
        bShouldClickStart = true;
        ClickAcknowledged = false;

        return true;
    }

    bool PressInGameButton(const std::string& buttonName)
    {
        bool bClickedAtLeastOne = false;
        int initialObjectCount = SDK::UObject::GObjects->Num();

        for (int i = initialObjectCount - 1; i >= 0; --i)
        {
            if (i >= SDK::UObject::GObjects->Num()) break;

            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;
            if (Obj->IsDefaultObject()) continue;

            // Skip garbage collected items
            std::string objName = Obj->GetName();
            if (objName.find("TRASH") != std::string::npos ||
                objName.find("Default__") != std::string::npos ||
                objName.find("REINST") != std::string::npos)
            {
                continue;
            }

            if (Obj->IsA(SDK::UW_MOSButton_Small_C::StaticClass()))
            {
                std::string fullName = Obj->GetFullName();

                // Loose matching: As long as it's the right button inside the Race Form
                if (fullName.find(buttonName) != std::string::npos &&
                    fullName.find("RaceFormWidget") != std::string::npos)
                {
                    SDK::UW_MOSButton_Small_C* JoinBtn = static_cast<SDK::UW_MOSButton_Small_C*>(Obj);

                    printf(">> [STAGE 5] Forcing click on: %s\n", fullName.c_str());

                    // Click it!
                    JoinBtn->HandleButtonPressed();
                    JoinBtn->HandleButtonClicked();
                    JoinBtn->HandleButtonReleased();

                    bClickedAtLeastOne = true;

                }
            }
        }

        return bClickedAtLeastOne;
    }

    int GetRaceTotalPlayerCount()
    {
        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (!World) return 0;
        SDK::AGameModeBase* CurrentGameMode = World->AuthorityGameMode;
        if (CurrentGameMode && CurrentGameMode->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
        {
            return static_cast<SDK::AMarbleRaceGameMode*>(CurrentGameMode)->PlayersJoined;
        }
        return 0;
    }

    int GetRaceDeadPlayerCount()
    {
        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (!World) return 0;
        SDK::AGameModeBase* CurrentGameMode = World->AuthorityGameMode;
        if (CurrentGameMode && CurrentGameMode->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
        {
            return static_cast<SDK::AMarbleRaceGameMode*>(CurrentGameMode)->PlayersEliminated;
        }
        return 0;
    }

    int GetRaceFinishedPlayerCount()
    {
        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (!World) return 0;
        SDK::AGameModeBase* CurrentGameMode = World->AuthorityGameMode;
        if (CurrentGameMode && CurrentGameMode->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
        {
            return static_cast<SDK::AMarbleRaceGameMode*>(CurrentGameMode)->PlayersFinished;
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

        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

            if (Obj->IsA(SDK::URaceResultsWidgetY2::StaticClass()) && !Obj->IsDefaultObject())
            {
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

        for (int i = 0; i < totalPlayers; i++)
        {
            SDK::UObject* rawItem = listView->ListItems[i];
            if (!rawItem) continue;

            uintptr_t baseAddr = reinterpret_cast<uintptr_t>(rawItem);
            int32_t rank = *reinterpret_cast<int32_t*>(baseAddr + 0xC8);
            float metricValue = *reinterpret_cast<float*>(baseAddr + 0xCC);
            int32_t statusFlag = *reinterpret_cast<int32_t*>(baseAddr + 0xF0);
            int placementIndex = rank - 1;

            if (placementIndex >= 0 && placementIndex < maxPlayersToFetch)
            {
                resultsArray[placementIndex].Rank = rank;

                if (statusFlag > 0)
                {
                    resultsArray[placementIndex].RaceTime = metricValue;
                    int minutes = static_cast<int>(std::floor(metricValue / 60.0f));
                    int seconds = static_cast<int>(std::floor(std::fmod(metricValue, 60.0f)));
                    int milliseconds = static_cast<int>(std::round(std::fmod(metricValue, 1.0f) * 1000.0f));
                }
                else
                {
                    resultsArray[placementIndex].RaceTime = -1.0f;
                }

                uintptr_t primaryNameAddr = baseAddr + 0x38;
                uintptr_t fallbackNameAddr = baseAddr + 0x28;
                wchar_t* nameData = *reinterpret_cast<wchar_t**>(primaryNameAddr);
                int32_t nameCount = *reinterpret_cast<int32_t*>(primaryNameAddr + 8);
                bool bNameFound = false;

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

                if (!bNameFound) resultsArray[placementIndex].PlayerName = L"Unknown";
                resultsArray[placementIndex].bIsValid = true;
            }
        }
        return resultsArray;
    }

    std::string ProcessRaceResults(const int playersWanted)
    {
        // add logging
        std::vector<RaceResult> topPlayers = ExtractResultsFromListView(playersWanted);
        std::string finalResults = "";
        printf("\n========== RACE RESULTS ==========\n");

        int validCount = 0;
        for (int i = 0; i < playersWanted; i++)
        {
            if (topPlayers[i].bIsValid)
            {
                validCount++;
                std::string playerNameStr;
                for (wchar_t c : topPlayers[i].PlayerName) {
                    playerNameStr += static_cast<char>(c);
                }

                finalResults.append("#");
                finalResults.append(std::to_string(topPlayers[i].Rank));
                finalResults.append(": ");
                finalResults.append(playerNameStr);

                // Add Race Time nicely formatted
                if (topPlayers[i].RaceTime > 0.0f)
                {
                    char timeBuf[64];
                    // Format float to 2 decimal places (e.g., 65.43s)
                    snprintf(timeBuf, sizeof(timeBuf), " - Time: %.2fs", topPlayers[i].RaceTime);
                    finalResults.append(timeBuf);
                }
                else
                {
                    finalResults.append(" - DNF"); // Did Not Finish
                }

                finalResults.append(" | ");
            }
        }

        if (validCount == 0)
        {
            return "[!] No results found. Is the race finished?";
        }

        return finalResults;
    }

    void TurnCamera()
    {
        SDK::APlayerCameraManager* cameraManager = nullptr;
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

            currentRot.Pitch = -89.0f;
            currentRot.Yaw = -113.0f;

            cameraManager->FreeCamDistance = 1500.0f;
            playerController->K2_SetActorRotation(currentRot, false);
            cameraManager->K2_SetActorRotation(currentRot, false);
            playerController->SetControlRotation(currentRot);
        }
        else
        {
            printf("[!] Camera Manager or Player Controller not found.\n");
        }
    }

    bool ClickNextRandomTrack()
    {
        // Scan backwards to find the live HUD
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            if (i >= SDK::UObject::GObjects->Num()) break;

            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t dummy; float dummyF;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &dummy, &dummyF)) continue;
            if (Obj->IsDefaultObject()) continue;

            // Ghost Object Filter
            std::string objName = Obj->GetName();
            if (objName.find("TRASH") != std::string::npos ||
                objName.find("Default__") != std::string::npos ||
                objName.find("REINST") != std::string::npos)
            {
                continue;
            }

            // Find the active HUD
            if (Obj->IsA(SDK::AMarbleRaceHUDY2::StaticClass()))
            {
                SDK::AMarbleRaceHUDY2* HUD = static_cast<SDK::AMarbleRaceHUDY2*>(Obj);

                // Access the HUD's official, verified pointer to the active Race Widget!
                if (HUD->MarbleRaceWidget)
                {
                    printf(">> [STAGE 5] Safely firing Random Track via the HUD! <<\n");
                    HUD->MarbleRaceWidget->OnRandomTrackButtonClicked();

                    // Return immediately so we don't accidentally double-click!
                    return true;
                }
            }
        }

        printf("[!] Could not find active HUD or MarbleRaceWidget.\n");
        return false;
    }

    bool ClickReturnToRaceMenu()
    {
        // Scan backwards to find the live HUD
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            if (i >= SDK::UObject::GObjects->Num()) break;

            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t dummy; float dummyF;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &dummy, &dummyF)) continue;
            if (Obj->IsDefaultObject()) continue;

            // Ghost Object Filter
            std::string objName = Obj->GetName();
            if (objName.find("TRASH") != std::string::npos ||
                objName.find("Default__") != std::string::npos ||
                objName.find("REINST") != std::string::npos)
            {
                continue;
            }

            // Find the active HUD
            if (Obj->IsA(SDK::AMarbleRaceHUDY2::StaticClass()))
            {
                SDK::AMarbleRaceHUDY2* HUD = static_cast<SDK::AMarbleRaceHUDY2*>(Obj);

                // Access the HUD's official, verified pointer to the active Race Widget!
                if (HUD->MarbleRaceWidget)
                {
                    printf(">> [STAGE 5] Safely firing Main Menu via the HUD! <<\n");
                    HUD->MarbleRaceWidget->OnMainMenuButtonClicked();

                    // Return immediately so we don't accidentally double-click!
                    return true;
                }
            }
        }

        printf("[!] Could not find active HUD or MarbleRaceWidget.\n");
        return false;
    }

    std::vector<RaceResult> ExtractLiveScoreboard(int maxPlayersToFetch)
    {
        std::vector<RaceResult> liveResults;
        SDK::UTopRacePositionsWidgetY2* TopPositionsHUD = nullptr;

        // Find the Live Race HUD
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t dummy; float dummyF;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &dummy, &dummyF)) continue;
            if (Obj->IsDefaultObject()) continue;

            if (Obj->IsA(SDK::UTopRacePositionsWidgetY2::StaticClass()))
            {
                if (Obj->GetFullName().find("Transient") != std::string::npos)
                {
                    TopPositionsHUD = static_cast<SDK::UTopRacePositionsWidgetY2*>(Obj);
                    break;
                }
            }
        }

        if (!TopPositionsHUD || !TopPositionsHUD->PositionsPanel)
        {
            return liveResults;
        }

        // Extract from the Vertical Box
        // UVerticalBox inherits from UPanelWidget, which contains the children array.
        int childCount = TopPositionsHUD->PositionsPanel->GetChildrenCount();
        int fetchCount = (maxPlayersToFetch < childCount) ? maxPlayersToFetch : childCount;

        for (int i = 0; i < fetchCount; i++)
        {
            // Get the raw child widget from the UI slot
            SDK::UWidget* rawChild = TopPositionsHUD->PositionsPanel->GetChildAt(i);
            if (!rawChild || !rawChild->IsA(SDK::UTopRacePositionEntryWidgetY2::StaticClass())) continue;

            // Cast it to the Entry Widget you dumped
            SDK::UTopRacePositionEntryWidgetY2* Entry = static_cast<SDK::UTopRacePositionEntryWidgetY2*>(rawChild);

            RaceResult result;
            result.Rank = i + 1; // 0th index is 1st place, 1st index is 2nd place, etc.
            result.bIsValid = true;

            if (Entry->PlayerNameTextBlock)
            {
                // Get the raw FText from the UI element
                // (UCommonTextBlock inherits GetText() from UTextBlock)
                SDK::FText rawFText = Entry->PlayerNameTextBlock->GetText();

                // Convert FText to FString safely via the Engine's Kismet Library
                SDK::FString stringName = SDK::UKismetTextLibrary::Conv_TextToString(rawFText);

                result.PlayerName = stringName.ToWString();
            }

            if (result.PlayerName.empty())
            {
                result.PlayerName = L"Unknown";
            }

            liveResults.push_back(result);
        }

        return liveResults;
    }

    bool AutoScrollRaceResults(int searchTick)
    {
        SDK::URaceResultsWidgetY2* resultsWidget = nullptr;

        // Find the Live Results HUD
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;
            if (Obj->IsDefaultObject()) continue;

            if (Obj->IsA(SDK::URaceResultsWidgetY2::StaticClass()))
            {
                if (Obj->GetFullName().find("Transient") != std::string::npos)
                {
                    resultsWidget = static_cast<SDK::URaceResultsWidgetY2*>(Obj);
                    break;
                }
            }
        }

        if (!resultsWidget || !resultsWidget->RaceResultsListView)
            return false;

        if (searchTick % 20 == 0)
        {
            static int currentItemIndex = 0;
            int totalItems = resultsWidget->RaceResultsListView->ListItems.Num();

            if (totalItems > 0)
            {
                // Bring the next item into view
                resultsWidget->RaceResultsListView->ScrollIndexIntoView(currentItemIndex);

                currentItemIndex++;

                // Loop back to the top when it reaches the bottom
                if (currentItemIndex >= totalItems)
                {
                    currentItemIndex = 0;
                }
            }
            return true;
        }

        return false;
    }


    std::string GetFirstPlaceFinishedPlayer()
    {
        // Get the GameMode
        SDK::AMarbleRaceGameMode* GameMode = GetMarbleGameMode(); // Uses your helper from earlier
        if (!GameMode) return "";

        // Ask the Engine for the finished marble at position 1
        SDK::AMarble* FirstPlaceMarble = GameMode->FindMarbleAtPosition(1);

        if (FirstPlaceMarble)
        {
            // Read from the persistent APlayerState (Safest)
            if (FirstPlaceMarble->PlayerState)
            {
                SDK::FString fName = FirstPlaceMarble->PlayerState->GetPlayerName();
                std::wstring wName = fName.ToWString();
                std::string cleanName = "";

                for (wchar_t c : wName)
                {
                    if (c >= 32 && c <= 126) cleanName += static_cast<char>(c);
                }

                if (!cleanName.empty())
                {
                    return cleanName;
                }
            }

            // Fallback to the raw internal _Username variable
            if (FirstPlaceMarble->_Username.IsValid())
            {
                std::wstring wName = FirstPlaceMarble->_Username.ToWString();
                std::string cleanName = "";

                for (wchar_t c : wName)
                {
                    if (c >= 32 && c <= 126) cleanName += static_cast<char>(c);
                }

                if (!cleanName.empty())
                {
                    return cleanName;
                }
            }
        }

        // Return empty if the race isn't over yet, or no one has finished
        return "";
    }

    SDK::AMarbleRaceGameMode* GetMarbleGameMode()
    {
        // Scan backwards for the active GameMode
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            if (i >= SDK::UObject::GObjects->Num()) break;
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

            if (Obj->IsA(SDK::AMarbleRaceGameMode::StaticClass()) && !Obj->IsDefaultObject())
            {
                return static_cast<SDK::AMarbleRaceGameMode*>(Obj);
            }
        }
        return nullptr;
    }


    bool IsRaceJoinable()
    {
        // Scan backwards to find the active GameMode
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            if (i >= SDK::UObject::GObjects->Num()) break;

            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;

            // Memory safety check
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;
            if (Obj->IsDefaultObject()) continue;

            // Find the specific Marbles Game Mode
            if (Obj->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
            {
                SDK::AMarbleRaceGameMode* GameMode = static_cast<SDK::AMarbleRaceGameMode*>(Obj);

                // This is false during the Map Intro, and flips to true the moment the intro ends
                return GameMode->bCanJoinRace;
            }
        }

        // Return false if the Game Mode hasn't loaded into memory yet
        return false;
    }

    std::string GetSpectatedPlayerName()
    {
        // Scan memory for the Local Player Controller
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            if (i >= SDK::UObject::GObjects->Num()) break;

            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;
            if (Obj->IsDefaultObject()) continue;

            if (Obj->IsA(SDK::APlayerController::StaticClass()))
            {
                SDK::APlayerController* PC = static_cast<SDK::APlayerController*>(Obj);

                // Ask the Engine which Actor the camera is currently attached to!
                SDK::AActor* ViewTarget = PC->GetViewTarget();

                if (ViewTarget && ViewTarget->IsA(SDK::AMarble::StaticClass()))
                {
                    SDK::AMarble* SpectatedMarble = static_cast<SDK::AMarble*>(ViewTarget);

                    if (SpectatedMarble->PlayerState)
                    {
                        SDK::FString fName = SpectatedMarble->PlayerState->GetPlayerName();
                        std::wstring wName = fName.ToWString();
                        std::string cleanName = "";

                        for (wchar_t c : wName)
                        {
                            if (c >= 32 && c <= 126) cleanName += static_cast<char>(c);
                        }

                        if (!cleanName.empty()) return cleanName;
                    }

                    // Fallback: If PlayerState is missing, check the internal _Username we found earlier
                    if (SpectatedMarble->_Username.IsValid())
                    {
                        std::wstring wName = SpectatedMarble->_Username.ToWString();
                        std::string cleanName = "";

                        for (wchar_t c : wName)
                        {
                            if (c >= 32 && c <= 126) cleanName += static_cast<char>(c);
                        }

                        return cleanName;
                    }
                }
            }
        }

        return "";
    }

    std::string GetJoinedPlayers(int amount)
    {
        if (amount <= 0) return "Invalid amount requested.";
        if (amount > 1000) amount = 1000;

        SDK::AMOSGameState* ActiveGameState = nullptr;

        // Find the active Game State
        for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            if (!Obj) continue;

            if (!Obj->IsDefaultObject() && Obj->IsA(SDK::AMOSGameState::StaticClass()))
            {
                ActiveGameState = static_cast<SDK::AMOSGameState*>(Obj);
                break; // Found it! Stop searching.
            }
        }

        if (!ActiveGameState) return "Could not find active game state.";

        // Grab the native array of viewers!
        SDK::TArray<SDK::AMOSPlayerState*> ViewersArray = ActiveGameState->GetViewers();

        if (ViewersArray.Num() == 0) return "No players have joined the lobby yet.";

        std::string result = "";
        int limit = amount < ViewersArray.Num() ? amount : ViewersArray.Num();

        // Iterate through their array
        for (int i = 0; i < limit; ++i)
        {
            SDK::AMOSPlayerState* PlayerState = ViewersArray[i];
            if (!PlayerState) continue;

            // Extract and sanitize the name
            std::string cleanName = "";
            SDK::FString ueName = PlayerState->GetPlayerName(); // Assuming standard APlayerState inherited function
            std::wstring wName = ueName.ToWString();

            for (wchar_t c : wName)
            {
                if (c >= 32 && c <= 126) cleanName += static_cast<char>(c);
            }

            if (!cleanName.empty())
            {
                result += cleanName;
                if (i < limit - 1) result += ", ";
            }
        }

        return result;
    }

    float ApplyGravity(float newGravity)
    {
        float oldGravity = -3920.0f; // fallback

        for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

            // Target the WorldSettings class where GlobalGravityZ lives
            if (!Obj->IsDefaultObject() && Obj->IsA(SDK::AWorldSettings::StaticClass()))
            {
                SDK::AWorldSettings* WorldSettings = static_cast<SDK::AWorldSettings*>(Obj);

                // Grab the original gravity before overwriting (only if we haven't already modified it)
                oldGravity = WorldSettings->GlobalGravityZ;

                // Apply new gravity
                WorldSettings->GlobalGravityZ = newGravity;
                WorldSettings->bGlobalGravitySet = true;
            }
        }

        return oldGravity;
    }

    SDK::AMarble* FindMarble(const std::string& targetUsername)
    {
        SDK::AMarbleRaceGameMode* ActiveGameMode = nullptr;

        for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            if (!Obj || Obj->IsDefaultObject()) continue;

            if (Obj->IsA(SDK::AMarbleRaceGameMode::StaticClass()))
            {
                ActiveGameMode = static_cast<SDK::AMarbleRaceGameMode*>(Obj);
                break;
            }
        }

        if (!ActiveGameMode)
        {
            return nullptr; // Race hasn't started or GameMode couldn't be found
        }

        // Convert standard C++ string into a wide string
        std::wstring wUsername(targetUsername.begin(), targetUsername.end());

        // Unreal FString 
        SDK::FString ueUsername(wUsername.c_str());

        return ActiveGameMode->FindMarble(ueUsername);
    }

}


