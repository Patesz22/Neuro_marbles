#pragma once
#include "main.h"
#include "MOSAPI_classes.hpp"

extern bool SafeCopyMemory(void* destination, void* source, size_t size);

namespace Mode_Race
{
    extern volatile bool bShouldClickStart;
    extern volatile bool bClickAcknowledged;
    extern volatile bool bMatchIDIntercepted;
    extern bool bIsMenuHooked;
    extern bool bIsHeartbeatHooked;

    struct RaceResult {
        int Rank;
        std::wstring PlayerName;
        int Score;
        float RaceTime;
        bool bIsValid;
    };



    bool StartRaceMatch();
    bool PressInGameButton(const std::string& buttonName);

    int GetRaceTotalPlayerCount();
    int GetRaceDeadPlayerCount();
    int GetRaceFinishedPlayerCount();

    void ForceProceedToResults();
    std::vector<RaceResult> ExtractResultsFromListView(int maxPlayersToFetch);
    void ProcessRaceResults(const int playersWanted);

    void TurnCamera();
    bool ClickNextRandomTrack();
}
