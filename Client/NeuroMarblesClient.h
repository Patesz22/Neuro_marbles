#pragma once
#include "main.h"
#include "GameState.h"

class NeuroMarbles : public NeuroIXWebsocket::NeuroGameClient
{
private:
    MarblesGameState& state;

    // Delete copy constructors
    NeuroMarbles(const NeuroMarbles&) = delete;
    NeuroMarbles& operator=(const NeuroMarbles&) = delete;

public:
    NeuroMarbles(const std::string& uri, const std::string& game_name, MarblesGameState& gameState,
        std::ostream* output_stream = &std::cout, std::ostream* error_stream = &std::cerr);

    bool isWaitingForForcedAction() const;

protected:
    void handleMessage(const NeuroIXWebsocket::NeuroResponse& response) override;
};