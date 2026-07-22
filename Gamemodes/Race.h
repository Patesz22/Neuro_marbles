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

    struct RaceResult 
    {
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
    std::vector<RaceResult> ExtractLiveScoreboard(int maxPlayersToFetch);
    std::vector<RaceResult> ExtractResultsFromListView(int maxPlayersToFetch);
    std::string ProcessRaceResults(const int playersWanted);
    std::string GetFirstPlaceFinishedPlayer();

    void TurnCamera();

    SDK::AMarbleRaceGameMode* GetMarbleGameMode();
    bool IsRaceJoinable();
    bool ClickNextRandomTrack();
    bool ClickReturnToRaceMenu();

    bool AutoScrollRaceResults(int searchTick);

}
