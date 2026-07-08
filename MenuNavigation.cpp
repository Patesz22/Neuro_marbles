#pragma once
#include "MenuNavigation.h"


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

