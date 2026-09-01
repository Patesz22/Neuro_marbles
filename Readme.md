# Neuro Marbles On Stream game integration based on <u>[neuro-sdk](https://github.com/VedalAI/neuro-sdk)</u> with <u>[neuro-sdk-c++](https://github.com/chris-pie/neuro-sdk-websocketpp)</u>

## Features
- todo

---
## Usage and Installation
1. Download Marbles On Stream from Steam.
2. Run and set the game up, log in with your twitch account.
3. Download the [latest release](https://github.com/Patesz22/Neuro_marbles/releases) from the releases page.
4. Open the game's folder
   - *Right click the game -> Manage -> Browse local files*
   - **or** *Right click the game -> Properties -> Installed Files -> Browse*
5. In the opened folder navigate to *MarblesOnStream/Binaries/Win64/*
   - Create a **scripts** folder and a **config.txt** here
   - Download **dxgi.dll** from [Ultimate-ASI-Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) and put it beside the two newly created folder and txt
   - Navigate to the **scripts** folder and put the downloaded mod .asi file there
6. Paste the config template into your **config.txt** and modify it to your needs <u>(names and values are case sensitive)</u><br>
```
# ============================================
# Nuero Marbles on Stream mod configuration
# ============================================

### WebSocket Connection Settings
# These settings are only used, if the "NEURO_SDK_WS_URL" env variable is equal to "" or does not exists.
# 0 is false, 1 is true
WebSocketTLS1_2 = 0
WebSocketIP = 127.0.0.1
WebSocketPort = 8000

### File Logging
# Use forward slashes (/) or double backslashes (\\)
# LogDirectory = C:/Somefolder/Logs/
LogDirectory = ./logs

### Command Cooldowns (in seconds)
GravityCooldown = 60
MassCooldown = 15
KickCooldown = 1
SizeCooldown = 15

### Twitch configs for manual override from twitch chat in case something happens

# The username of the channel from which the bot will read the messages
# only lowercase
TwitchChannelname = patesz_3
# Who can trigger *any* of the manual actions
# only lowercase
AdminUsers = patesz_3,vedal987
```
---

## Building from Source

This project requires a C++ compiler compatible with C++17 or higher.

1. Clone the repository.
2. Ensure your build environment is configured for the target architecture (x64).
3. Link the necessary dependencies (see Acknowledgements).
4. Build the executable.

---
## Acknowledgements

* **[neuro-sdk](https://github.com/VedalAI/neuro-sdk) by Vedal987**, under MIT License
* **A rewrite of [neuro-sdk-websocketpp](https://github.com/chris-pie/neuro-sdk-websocketpp) by chris-pie** under MIT License, using IXWebSocket for support of newer OpenSSL versions
* **[IXWebSocket](https://github.com/machinezone/IXWebSocket) by machinezone**, under BSD 3-Clause License
* **[nlohmann/json](https://github.com/nlohmann/json) by nlohmann**, under MIT License
* **[Dumper-7](https://github.com/Encryqed/Dumper-7) by Encryqed** 

---
## License

[MIT License](LICENSE)

## Todo

- [ ] more gamemodes
- [x] ~~Organize project files~~
- [ ] support for community maps
- [x] ~~change "force action menu navigation" to normal action based~~
- [x] ~~send neuro context on race results (finished, DNF) - without context flooding (top 10 sent)~~
- [x] ~~make camera automatically rotate on race start~~ **manual action**
- [x] ~~action for "exit to main menu" after results~~
- [x] ~~action for "next random race" after results~~
- [ ] make the code "neuro proof"
- [x] ~~send context on what's happening in game, so neuro can sort of "commentate"~~ **improvements needed**
- [x] ~~increase the frequency of neuro getting context on alive/dead/finished players (without context flooding)~~ **improvements needed**
- [ ] add modifier actions
  - [x] ~~I've seen a SetGravity function eg.~~
  - [x] ~~SetMassInKgs~~
  - [x] ~~SetMarbleSize~~
  - [ ] Investigate UCheatManager class
  - [ ] GodMode
  - [ ] BoostMarble
  - [ ] DestroyMarble

- [x] ~~action for kicking player(s) + ~~~~send neuro context on joined players (action)~~
- [x] ~~add config file support~~
- [ ] increase the lobby size
  - [ ] there is a function for this, but the official max is 1000 players so increaseing it to 2k would be detected server side (ban probably)
  - [ ] maybe ask the devs to allow us to use lobbies with more players
- [x] ~~Add twitch chat manual override for specific user~~
- [x] ~~Add console and file logging~~
