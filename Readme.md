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
   - Download **dxgi.dll** from [Ultimate-ASI-Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) and put it beside the two newly created folders
   - Navigate to the **scripts** folder and put the downloaded mod .asi file there
6. Paste the config template into your **config.txt** and modify it to your needs <br>
```
# ============================================
# Marbles on Stream mod configuration
# ============================================

# WebSocket Connection Settings
# These settings are only used, if the "NEURO_SDK_WS_URL" env variable is equal to "" or does not exists.
WebSocketIP = 127.0.0.1
WebSocketPort = 8000

# File Logging
# Use forward slashes (/) or double backslashes (\\)
# LogDirectory = C:/Somefolder/Logs/
LogDirectory = ./logs

# Command Cooldowns (in seconds)
GravityCooldown = 60
MassCooldown = 15
KickCooldown = 1
```
