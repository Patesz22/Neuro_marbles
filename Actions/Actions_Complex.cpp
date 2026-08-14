#include "Actions_Complex.h"

namespace Mode_Race
{
    NeuroWebsocketpp::Action* actRaceGetJoinedPlayers = nullptr;
    NeuroWebsocketpp::Action* actSetGlobalGravity = nullptr;

    void InitComplexActions()
    {
        // Initialize get_joined_players
        if (!actRaceGetJoinedPlayers)
        {
            actRaceGetJoinedPlayers = new NeuroWebsocketpp::Action(
                "get_joined_players",
                "Returns the specified amount of currently joined players. Input range is 1-1000, single integer.",
                nlohmann::json::parse(R"(
                    {
                        "type": "object",
                        "properties": {
                            "amount": {
                                "type": "integer",
                                "description": "The number of players to fetch. Input range is 1-1000.",
                                "minimum": 1,
                                "maximum": 1000
                            }
                        },
                        "required": ["amount"]
                    }
                )")
            );
        }

        // Initialize set_global_gravity
        if (!actSetGlobalGravity)
        {
            actSetGlobalGravity = new NeuroWebsocketpp::Action(
                "set_global_gravity",
                "Changes the global race gravity for the specified duration.",
                nlohmann::json::parse(R"(
                    {
                        "type": "object",
                        "properties": {
                            "amount": {
                                "type": "number",
                                "description": "The new gravity value. Default gravity is -3920. Use positive numbers to make marbles float up! Minimum is -5500, maximum is 50. The cooldown is 60s.",
                                "minimum": -5500,
                                "maximum": 50
                            },
                            "duration": {
                                "type": "integer",
                                "description": "How long the gravity change lasts in seconds.",
                                "minimum": 1,
                                "maximum": 5
                            }
                        },
                        "required": ["amount", "duration"]
                    }
                )")
            );
        }
    }

    void FreeComplexActions()
    {
        if (actRaceGetJoinedPlayers)
        {
            delete actRaceGetJoinedPlayers;
            actRaceGetJoinedPlayers = nullptr;
        }

        if (actSetGlobalGravity)
        {
            delete actSetGlobalGravity;
            actSetGlobalGravity = nullptr;
        }
    }
};
