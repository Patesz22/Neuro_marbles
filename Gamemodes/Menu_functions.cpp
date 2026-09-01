#include "../main.h"
#include "Menu.h"

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

namespace Mode_Menu
{

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

			// Handle Default Menu Buttons
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
			// Handle Small Menu Buttons
			else if (Obj->IsA(SDK::UW_MOSButton_Small_C::StaticClass()))
			{
				std::string fullName = Obj->GetFullName();

				if (fullName.find("Transient") != std::string::npos &&
					fullName.find(buttonName) != std::string::npos)
				{
					SDK::UW_MOSButton_Small_C* SmallBtn = static_cast<SDK::UW_MOSButton_Small_C*>(Obj);

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

					return true;
				}
			}
			// Handle Back Menu Buttons
			else if (Obj->IsA(SDK::UW_MOSButton_Back_C::StaticClass()))
			{
				std::string fullName = Obj->GetFullName();

				if (fullName.find("Transient") != std::string::npos &&
					fullName.find(buttonName) != std::string::npos)
				{
					SDK::UW_MOSButton_Back_C* BackBtn = static_cast<SDK::UW_MOSButton_Back_C*>(Obj);

					if (!BackBtn->IsVisible())
					{
						printf(">> [DEBUG] Ignored hidden button: %s <<\n", fullName.c_str());
						continue;
					}

					printf(">> [DEBUG] Clicking VISIBLE button: %s <<\n", fullName.c_str());

					if (bBailOutInstantly)
					{
						BackBtn->HandleButtonClicked();
					}
					else
					{
						BackBtn->BP_OnPressed();
						BackBtn->HandleButtonPressed();
						BackBtn->HandleButtonClicked();
						BackBtn->BP_OnReleased();
						BackBtn->HandleButtonReleased();
					}
					return true;
				}
			}
		}

		return false; // no visible button was found
	}


	bool SelectExperienceCard(const std::string& targetModeName)
	{
		SDK::UMOSUserFacingExperienceDescription* TargetData = nullptr;
		SDK::UW_UserFacingExperienceSelector_C* SelectorMenu = nullptr;

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

		// Find the Data Asset (Scan FORWARDS because Data Assets are persistent, not Transient)
		for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
		{
			SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
			if (!Obj || Obj->IsDefaultObject()) continue;

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

		// Native Engine Event!
		if (TargetData && SelectorMenu)
		{
			printf(">> [STAGE 2] Firing the literal UI Mouse-Click Event! <<\n");

			// Holy function name
			SelectorMenu->BndEvt__W_UserFacingExperienceSelector_ExperienceDescriptionsListView_K2Node_ComponentBoundEvent_0_SimpleListItemEventDynamic__DelegateSignature(TargetData);

			return true;
		}

		return false;
	}


	std::string ClickRandomizeButton()
	{
		SDK::UW_MOSButton_Default_C* ActiveRandomButton = nullptr;
		SDK::UObject* ActiveMapListWidget = nullptr;
		std::string mapName = "Unknown Map";

		// Grab the visibility function safely outside the loop
		SDK::UFunction* isVisibleFunc = MemoryHelpers::FindFunctionByName("IsVisible");

		int initialObjectCount = SDK::UObject::GObjects->Num();
		for (int i = initialObjectCount - 1; i >= 0; --i)
		{
			if (i >= SDK::UObject::GObjects->Num())
				break;

			SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
			int32_t d; float df;
			if (!Obj || !MemoryHelpers::SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df) || Obj->IsDefaultObject())
				continue;

			std::string fullName = Obj->GetFullName();

			// Ensure we only grab the VISIBLE Map List to avoid "Ghost" widgets!
			if (!ActiveMapListWidget &&
				fullName.find("W_RaceExperienceMapList") != std::string::npos &&
				fullName.find("Transient") != std::string::npos)
			{
				if (isVisibleFunc)
				{
					struct
					{
						bool ReturnValue;
					} Params = { false };
					Obj->ProcessEvent(isVisibleFunc, &Params);

					if (Params.ReturnValue)
					{
						ActiveMapListWidget = Obj;
					}
					else
					{
						printf(">> [DEBUG] Ignored hidden 'Ghost' MapList widget!\n");
					}
				}
			}

			// Ensure we grab the VISIBLE Random Button
			if (!ActiveRandomButton && Obj->IsA(SDK::UW_MOSButton_Default_C::StaticClass()))
			{
				if (fullName.find("Transient") != std::string::npos &&
					fullName.find("RandomButton") != std::string::npos)
				{
					SDK::UW_MOSButton_Default_C* Btn = static_cast<SDK::UW_MOSButton_Default_C*>(Obj);

					if (Btn->IsVisible())
					{
						ActiveRandomButton = Btn;
					}
				}
			}

			// found both active elements -> stop looping!
			if (ActiveRandomButton && ActiveMapListWidget)
				break;
		}

		if (!ActiveRandomButton)
		{
			printf("[ERROR] Could not find VISIBLE Random Button! <<\n");
			return mapName;
		}

		// This scrambles the Blueprint seed
		srand((unsigned int)time(NULL));
		int randomChurn = (rand() % 50) + 15;
		SDK::UKismetMathLibrary* MathLib = SDK::UKismetMathLibrary::GetDefaultObj();

		if (MathLib)
		{
			for (int i = 0; i < randomChurn; i++)
			{
				MathLib->RandomFloat();
			}
		}

		// Fire the click events
		ActiveRandomButton->BP_OnPressed();
		ActiveRandomButton->HandleButtonPressed();
		ActiveRandomButton->HandleButtonClicked();
		ActiveRandomButton->BP_OnReleased();
		ActiveRandomButton->HandleButtonReleased();

		if (ActiveMapListWidget)
		{
			SDK::UFunction* GetMapFunc = nullptr;

			int maxObjects = SDK::UObject::GObjects->Num();
			for (int i = maxObjects - 1; i >= 0; --i)
			{
				SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
				if (!Obj) continue;

				std::string fullName = Obj->GetFullName();

				if (fullName.find("Function") != std::string::npos &&
					fullName.find("GetSelectedStudioMap") != std::string::npos)
				{
					GetMapFunc = static_cast<SDK::UFunction*>(Obj);
					break;
				}
			}

			if (GetMapFunc)
			{
				struct
				{
					SDK::UObject* SelectedMap;
				} Params = { nullptr };

				ActiveMapListWidget->ProcessEvent(GetMapFunc, &Params);

				if (Params.SelectedMap)
				{
					SDK::UMOSUserFacingExperienceDefinition* MapDef = static_cast<SDK::UMOSUserFacingExperienceDefinition*>(Params.SelectedMap);

					if (MapDef)
					{
						mapName = MapDef->TileTitle.ToString();
						printf(">> [DEBUG] Successfully extracted real map name!\n");
					}
					else
					{
						printf(">> [DEBUG] Failed to cast to UMOSUserFacingExperienceDefinition!\n");
					}
				}
				else
				{
					printf(">> [DEBUG] GetSelectedStudioMap returned NULL!\n");
				}
			}
			else
			{
				printf(">> [DEBUG] CRITICAL: Function 'GetSelectedStudioMap' literally does not exist in memory!\n");
			}
		}
		else
		{
			printf(">> [DEBUG] CRITICAL: ActiveMapListWidget is NULL!\n");
		}

		printf("[STAGE 3] RNG Seed churned %d times. Selected Map: %s <<\n", randomChurn, mapName.c_str());

		return mapName;
	}



	int GetMaxLobbySize()
	{
		SDK::UW_RaceUserFacingExperience_C* ActiveRaceMenu = nullptr;

		// Sweep BACKWARDS to find the ACTIVE Race Menu widget
		for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
		{
			SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

			int32_t d; float df;
			if (!Obj || !MemoryHelpers::SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df)) continue;

			// Use the safe StaticClass() check to find the Race Menu
			if (!Obj->IsDefaultObject() && Obj->IsA(SDK::UW_RaceUserFacingExperience_C::StaticClass()))
			{
				// Ensure it's in the Transient package (meaning it is actively on screen)
				if (Obj->GetFullName().find("Transient") != std::string::npos)
				{
					ActiveRaceMenu = static_cast<SDK::UW_RaceUserFacingExperience_C*>(Obj);
					break;
				}
			}
		}

		// Call the built-in SDK function!
		if (ActiveRaceMenu)
		{
			// This natively executes the logic the game developers wrote 
			// to determine the lobby size, 100% immune to offset changes!
			int32_t maxPlayers = ActiveRaceMenu->GetMaxPlayers();

			printf("[DEBUG] Active Race Menu found -> MaxPlayers: %d\n", maxPlayers);
			return maxPlayers;
		}

		printf("[!] Could not find the active Race Menu widget in memory.\n");
		return -1;
	}


}