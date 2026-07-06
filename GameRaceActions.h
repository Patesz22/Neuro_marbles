#pragma once
#include "main.h"

bool SelectExperienceCard(const std::string& targetModeName);
bool ClickNativeRandomButton();
bool StartRaceMatch();
bool PressInGameButton(const std::string&);

int GetRaceTotalPlayerCount();
int GetRaceDeadPlayerCount();
int GetRaceFinishedPlayerCount();

void ForceProceedToNextMap();

