#pragma once
#include "main.h"

void InspectUIWidgets();

bool ClickGenericButton(const std::string& buttonName, bool bBailOutInstantly = false);
bool SelectExperienceCard(const std::string& targetModeName);
bool ClickNativeRandomButton();

int GetMaxLobbySize();

