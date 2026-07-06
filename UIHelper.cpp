#pragma once
#include "UIHelper.h"


bool SafeRead4Bytes(uintptr_t address, int32_t* outInt, float* outFloat)
{
    __try
    {
        *outInt = *reinterpret_cast<int32_t*>(address);
        *outFloat = *reinterpret_cast<float*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool SafeProcessEvent(SDK::UObject* TargetObject, SDK::UFunction* Function, void* Params)
{
    if (!TargetObject || !Function) return false;

    __try
    {
        TargetObject->ProcessEvent(Function, Params);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool SafeCopyMemory(void* destination, void* source, size_t size)
{
    if (!source || !destination || size <= 0) return false;

    __try
    {
        memcpy(destination, source, size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}


void InspectUIWidgets()
{
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        if (Obj->IsA(SDK::UUserWidget::StaticClass()))
        {
            SDK::UUserWidget* Widget = static_cast<SDK::UUserWidget*>(Obj);

            if (Widget->IsHovered())
            {
                printf("\n[UI WIDGET CLICKED]\n");
                printf("Instance: %s\n", Widget->GetName().c_str());
                printf("Class:    %s\n", Widget->GetFullName().c_str());
            }
        }
    }
}


/* Indexes for gamemode selection
0: Race
1: Grand Prix
2: Royale
3: Tilted
4: Marble Up
5: Bloop
6: Build
7: Dust
*/
bool ClickGenericButton(const std::string& buttonName, bool bBailOutInstantly)
{
    int initialObjectCount = SDK::UObject::GObjects->Num();

    for (int i = initialObjectCount - 1; i >= 0; --i)
    {
        if (i >= SDK::UObject::GObjects->Num())
        {
            printf(">> [WARNING] Memory shifted! <<\n");
            break;
        }

        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        // 1. Handle Default Menu Buttons
        if (Obj->IsA(SDK::UW_MOSButton_Default_C::StaticClass()))
        {
            std::string fullName = Obj->GetFullName();

            if (fullName.find("Transient") != std::string::npos &&
                fullName.find(buttonName) != std::string::npos)
            {
                SDK::UW_MOSButton_Default_C* DefBtn = static_cast<SDK::UW_MOSButton_Default_C*>(Obj);

                if (!DefBtn->IsVisible())
                {
                    printf(">> [DEBUG] Ignored hidden button: %s <<\n", fullName.c_str());
                    continue; // Skip to the next object
                }

                printf(">> [DEBUG] Clicking VISIBLE button: %s <<\n", fullName.c_str());

                if (bBailOutInstantly)
                {
                    DefBtn->HandleButtonClicked();
                }
                else
                {
                    // Call the Blueprint events if they exist, to ensure proper state logic triggers
                    DefBtn->BP_OnPressed();
                    DefBtn->HandleButtonPressed();
                    DefBtn->HandleButtonClicked();
                    DefBtn->BP_OnReleased();
                    DefBtn->HandleButtonReleased();
                }

                // We found the real one and clicked it. Stop looking!
                return true;
            }
        }
        // 2. Handle Small Menu Buttons
        else if (Obj->IsA(SDK::UW_MOSButton_Small_C::StaticClass()))
        {
            std::string fullName = Obj->GetFullName();

            if (fullName.find("Transient") != std::string::npos &&
                fullName.find(buttonName) != std::string::npos)
            {
                SDK::UW_MOSButton_Small_C* SmallBtn = static_cast<SDK::UW_MOSButton_Small_C*>(Obj);

                // ==========================================
                // THE FIX: Do not click hidden/collapsed buttons!
                // ==========================================
                if (!SmallBtn->IsVisible())
                {
                    printf(">> [DEBUG] Ignored hidden button: %s <<\n", fullName.c_str());
                    continue; // Skip to the next object
                }

                printf(">> [DEBUG] Clicking VISIBLE button: %s <<\n", fullName.c_str());

                if (bBailOutInstantly)
                {
                    SmallBtn->HandleButtonClicked();
                }
                else
                {
                    SmallBtn->BP_OnPressed();
                    SmallBtn->HandleButtonPressed();
                    SmallBtn->HandleButtonClicked();
                    SmallBtn->BP_OnReleased();
                    SmallBtn->HandleButtonReleased();
                }

                // We found the real one and clicked it. Stop looking!
                return true;
            }
        }
    }

    return false; // If we get here, no visible button was found
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


// press a key in the background
void PressKey(int virtualKeyCode)
{
    keybd_event(virtualKeyCode, 0, 0, 0);
    Sleep(100);
    keybd_event(virtualKeyCode, 0, KEYEVENTF_KEYUP, 0);
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


int GetMaxLobbySize()
{
    SDK::UObject* ActiveMaxPlayersWidget = nullptr;

    // 1. Sweep BACKWARDS to find the ACTIVE UI widget, bypassing old ghost menus!
    for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

        int32_t d; float df;
        if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

        // Find the widget blueprint
        if (!Obj->IsDefaultObject() && Obj->GetFullName().find("W_MaxPlayers_C") != std::string::npos)
        {
            // CRITICAL: Ensure it's in the Transient package (meaning it is currently on screen)
            if (Obj->GetFullName().find("Transient") != std::string::npos)
            {
                ActiveMaxPlayersWidget = Obj;
                break; // Found the live one! Stop searching.
            }
        }
    }

    // 2. Extract the values using your exact dump offsets
    if (ActiveMaxPlayersWidget)
    {
        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(ActiveMaxPlayersWidget);

        int32_t lastCommittedValue = *reinterpret_cast<int32_t*>(baseAddr + 0x0308);
        int32_t maxPlayers = *reinterpret_cast<int32_t*>(baseAddr + 0x030C);
        int32_t defaultPlayers = *reinterpret_cast<int32_t*>(baseAddr + 0x0314);

        printf("[DEBUG] Active W_MaxPlayers_C -> LastCommitted: %d | MaxPlayers: %d | Default: %d\n",
            lastCommittedValue, maxPlayers, defaultPlayers);

        // Logic: Return the most relevant number. 
        // In UE's Slate UI, "LastCommittedValue" is usually what updates when a user finishes typing in a box.
        if (lastCommittedValue > 0)
        {
            return lastCommittedValue;
        }
        if (maxPlayers > 0)
        {
            return maxPlayers;
        }
        if (defaultPlayers > 0)
        {
            return defaultPlayers;
        }
    }

    printf("[!] Could not find the active W_MaxPlayers_C widget in memory.\n");
    return -1;
}

