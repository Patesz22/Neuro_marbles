#include "Actions_Complex.h"

namespace Mode_Race
{
    NeuroWebsocketpp::Action* actRaceGetJoinedPlayers = nullptr;
    NeuroWebsocketpp::Action* actSetGlobalGravity = nullptr;
    NeuroWebsocketpp::Action* actSetMarbleMass = nullptr;
    NeuroWebsocketpp::Action* actKickPlayer = nullptr;

    void InitComplexActions()
    {
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

        if (!actSetMarbleMass)
        {
            actSetMarbleMass = new NeuroWebsocketpp::Action(
                "set_marble_mass",
                "Changes the weight of a specific viewer's marble.",
                nlohmann::json::parse(R"(
                {
                    "type": "object",
                    "properties": {
                        "username": {
                            "type": "string",
                            "description": "The exact Twitch username of the player."
                        },
                        "amount": {
                            "type": "number",
                            "description": "The new mass. 37.88 is default. >37.88 is heavy, <37.88 is light.",
                            "minimum": 1,
                            "maximum": 80.0
                        }
                    },
                    "required": ["username", "amount"]
                }
                )")
            );
        }

        if (!actKickPlayer)
        {
            actKickPlayer = new NeuroWebsocketpp::Action(
                "kick_player",
                "Kicks a specific viewer from the current race.",
                nlohmann::json::parse(R"(
                {
                    "type": "object",
                    "properties": {
                        "username": {
                            "type": "string",
                            "description": "The exact Twitch username of the player to kick."
                        }
                    },
                    "required": ["username"]
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

        if (actSetMarbleMass)
        {
            delete actSetMarbleMass;
            actSetMarbleMass = nullptr;
        }

        if (actKickPlayer)
        {
            delete actKickPlayer;
            actKickPlayer = nullptr;
        }
    }
};
