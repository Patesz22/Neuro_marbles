#include "Actions_Complex.h"

namespace Mode_Race
{
	NeuroIXWebsocket::Action* actRaceGetJoinedPlayers = nullptr;
	NeuroIXWebsocket::Action* actSetGlobalGravity = nullptr;
	NeuroIXWebsocket::Action* actSetMarbleMass = nullptr;
	NeuroIXWebsocket::Action* actKickPlayer = nullptr;
	NeuroIXWebsocket::Action* actSetMarbleSize = nullptr;

	void InitComplexActions()
	{
		if (!actRaceGetJoinedPlayers)
		{
			actRaceGetJoinedPlayers = new NeuroIXWebsocket::Action(
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
			actSetGlobalGravity = new NeuroIXWebsocket::Action(
				"set_global_gravity",
				"Changes the global race gravity for the specified duration. Default gravity is -3920. Use positive numbers to make marbles float up! Minimum is -5500, maximum is 50. Duration is between 1-5 sec. The cooldown is 60s.",
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
			actSetMarbleMass = new NeuroIXWebsocket::Action(
				"set_marble_mass",
				"Changes the weight of a specific viewer's marble. The mass 37.88 is default. >37.88 is heavy, <37.88 is light.",
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
			actKickPlayer = new NeuroIXWebsocket::Action(
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

		if (!actSetMarbleSize)
		{
			actSetMarbleSize = new NeuroIXWebsocket::Action(
				"set_marble_size",
				"Changes the physical size of a specific viewer's marble. The multiplier: 1.0 is normal. 2.0 is double size, 0.5 is half size.",
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
                            "description": "The new size multiplier. 1.0 is normal. 2.0 is double size, 0.5 is half size.",
                            "minimum": 0.1,
                            "maximum": 5.0
                        }
                    },
                    "required": ["username", "amount"]
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

		if (actSetMarbleSize)
		{
			delete actSetMarbleSize;
			actSetMarbleSize = nullptr;
		}
	}
};
