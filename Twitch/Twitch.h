#pragma once

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

#include <string>
#include <iostream>
#include <algorithm> 

#include "GameState.h"
#include "Config/NeuroConfig.h"

class TwitchChatClient
{
private:
    ix::WebSocket m_webSocket;
    std::string m_channel;

    std::string SanitizeInput(const std::string& input)
    {
        std::string safe_str = input;

        // Erase anything that is NOT a letter, number, or underscore
        safe_str.erase(std::remove_if(safe_str.begin(), safe_str.end(), [](unsigned char c)
            {
                return !std::isalnum(c) && c != '_';
            }), safe_str.end());

        // Limit the maximum length to prevent memory exhaustion
        if (safe_str.length() > 64)
        {
            safe_str = safe_str.substr(0, 64);
        }

        return safe_str;
    }

    void SetupCallbacks()
    {
        m_webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
            {
                if (msg->type == ix::WebSocketMessageType::Open)
                {
                    std::cout << "[Twitch] Connected to IRC WSS!" << std::endl;

                    m_webSocket.sendText("PASS SCHMOOPIIE\r\n");
                    m_webSocket.sendText("NICK justinfan12345\r\n");
                    m_webSocket.sendText("JOIN #" + m_channel + "\r\n");
                }
                else if (msg->type == ix::WebSocketMessageType::Close)
                {
                    std::cout << "[Twitch] Connection Closed! Code: " << msg->closeInfo.code
                        << " Reason: " << msg->closeInfo.reason << std::endl;
                }
                else if (msg->type == ix::WebSocketMessageType::Error)
                {
                    std::cout << "[Twitch] Connection Error: " << msg->errorInfo.reason << std::endl;
                }
                else if (msg->type == ix::WebSocketMessageType::Message)
                {
                    std::string payload = msg->str;

                    // Handle Twitch Keep-Alive PINGs
                    if (payload.find("PING :tmi.twitch.tv") != std::string::npos)
                    {
                        m_webSocket.send("PONG :tmi.twitch.tv\r\n");
                        return;
                    }

                    size_t msgStart = payload.find("PRIVMSG #");
                    if (msgStart != std::string::npos)
                    {
                        // Extract the username from the raw IRC string
                        // Format: :username!username@username.tmi.twitch.tv PRIVMSG ...
                        std::string sender = "";
                        if (payload[0] == ':')
                        {
                            size_t exclaimPos = payload.find('!');
                            if (exclaimPos != std::string::npos)
                            {
                                sender = payload.substr(1, exclaimPos - 1);
                            }
                        }

                        // Extract message
                        size_t textStart = payload.find(" :", msgStart);
                        if (textStart != std::string::npos)
                        {
                            std::string text = payload.substr(textStart + 2);

                            text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
                            text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());

                            std::string allowedUsers = GetConfigString("AdminUsers", "");

                            // Wrap in commas to prevent partial matches ("sam" matching "neuro_sama")
                            std::string searchStr = "," + allowedUsers + ",";
                            std::string targetStr = "," + sender + ",";

                            bool isAuthorized = (searchStr.find(targetStr) != std::string::npos);

                            if (isAuthorized)
                            {
                                if (text.rfind("!#override_state ", 0) == 0)
                                {
                                    std::string rawPayload = text.substr(17);
                                    std::cout << "[Twitch] Incoming !#override_state" << std::endl;
                                    PushOverride("STATE", SanitizeInput(rawPayload));
                                }
                                else if (text.rfind("!#override_button ", 0) == 0)
                                {
                                    std::string rawPayload = text.substr(18);
                                    std::cout << "[Twitch] Incoming !#override_button" << std::endl;
                                    PushOverride("BUTTON", SanitizeInput(rawPayload));
                                }
                                else if (text.rfind("!#override_action ", 0) == 0)
                                {
                                    std::string rawPayload = text.substr(18);
                                    std::cout << "[Twitch] Incoming !#override_action" << std::endl;
                                    PushOverride("ACTION", SanitizeInput(rawPayload));
                                }
                            }
                        }
                    }
                }
                else if (msg->type == ix::WebSocketMessageType::Error)
                {
                    std::cout << "[Twitch] Connection Error: " << msg->errorInfo.reason << std::endl;
                }
            });
    }

public:
    void Start(const std::string& channel)
    {
        m_channel = channel;

        // Initialize Windows networking systems for IXWebSocket
        ix::initNetSystem();

        m_webSocket.setUrl("wss://irc-ws.chat.twitch.tv:443");
        m_webSocket.enableAutomaticReconnection();

        SetupCallbacks();
        
        m_webSocket.start();
    }

    void Stop()
    {
        m_webSocket.stop();
        ix::uninitNetSystem();
    }
};