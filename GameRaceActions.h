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


bool StartRaceMatch();
bool PressInGameButton(const std::string&);

int GetRaceTotalPlayerCount();
int GetRaceDeadPlayerCount();
int GetRaceFinishedPlayerCount();

void ForceProceedToResults();

std::vector<RaceResult> ExtractResultsFromListView(int maxPlayersToFetch);
void ProcessRaceResults(const std::string& actionId, const int playersWanted);

void TurnCamera();

