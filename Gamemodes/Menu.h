#pragma once
#include "../main.h"

namespace Mode_Menu
{
	bool ClickGenericButton(const std::string& buttonName, bool bBailOutInstantly = false);
	bool SelectExperienceCard(const std::string& targetModeName);
	bool ClickRandomizeButton();

	int GetMaxLobbySize();
}