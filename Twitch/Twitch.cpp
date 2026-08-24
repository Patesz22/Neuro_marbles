#include "GameState.h"

std::queue<TwitchOverride> OverrideQueue;
std::mutex OverrideMutex;

void PushOverride(const std::string& type, const std::string& payload)
{
    std::lock_guard<std::mutex> lock(OverrideMutex);
    OverrideQueue.push({ type, payload });
}