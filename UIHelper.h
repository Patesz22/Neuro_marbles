#pragma once
#include "main.h"

struct RaceResult 
{
    int Rank;
    int Score;
    float RaceTime;
    std::wstring PlayerName;
    bool bIsValid = false;
};


void InspectUIWidgets();
int GetMaxLobbySize();
bool ClickGenericButton(const std::string& buttonName, bool bBailOutInstantly = false);
std::vector<RaceResult> ExtractResultsFromListView(int maxPlayersToFetch);
void ProcessRaceResults(const std::string& actionId, const int playersWanted);

void TurnCamera();
void PressKey(int virtualKeyCode);

bool SafeRead4Bytes(uintptr_t address, int32_t* outInt, float* outFloat);
bool SafeProcessEvent(SDK::UObject* TargetObject, SDK::UFunction* Function, void* Params);
bool SafeCopyMemory(void* destination, void* source, size_t size);