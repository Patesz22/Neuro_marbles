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
---

## Building from Source

This project requires a C++ compiler compatible with C++17 or higher.

1. Clone the repository.
2. Ensure your build environment is configured for the target architecture (x64).
3. Link the necessary dependencies (see Acknowledgements).
4. Build the executable.

---
## Acknowledgements

* **[neuro-sdk](https://github.com/VedalAI/neuro-sdk) by Vedal987**
* **[neuro-sdk-websocketpp](https://github.com/chris-pie/neuro-sdk-websocketpp) by chris-pie**
* **[WebSocket++](https://github.com/zaphoyd/websocketpp) by zaphoyd**
* **[nlohmann/json](https://github.com/nlohmann/json) by nlohmann**
* **[Dumper-7](https://github.com/Encryqed/Dumper-7) by Encryqed** 

---
## License

[MIT License](LICENSE)