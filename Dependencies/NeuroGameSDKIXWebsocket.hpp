#ifndef INCLUDE_NEURO_WEBSOCKET_LIBRARY_HPP
#define INCLUDE_NEURO_WEBSOCKET_LIBRARY_HPP

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

#include <utility>
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "json/json.hpp"
#include "Logging/Logging.h"
#include "Config/NeuroConfig.h"

namespace NeuroIXWebsocket
{

    enum Priority
    {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL,
    };

    inline std::string priorityToString(Priority priority)
    {
        switch (priority)
        {
            case LOW: return "low";
            case MEDIUM: return "medium";
            case HIGH: return "high";
            case CRITICAL: return "critical";
            default: return "low";
        }
    }

    class Action
    {
    public:
        Action(std::string  name, std::string  description, const nlohmann::json& schema) : name(std::move(name)), description(std::move(description)), schema(schema) {}

        std::string getName() const
        {
            return name;
        }
        std::string getDescription() const
        {
            return description;
        }
        nlohmann::json getSchema() const
        {
            return schema;
        }

    private:
        std::string name;
        std::string description;
        nlohmann::json schema;
    };

    class NeuroResponse
    {
    private:
        std::string command;
        std::string id;
        std::string name;
        std::string data;
        std::string sessionId;
        std::string characterId;
        std::string displayName;

    public:
        explicit NeuroResponse(const std::string& jsonStr);
        std::string getCommand() const
        {
            return command;
        }
        std::string getId() const
        {
            return id;
        }
        std::string getName() const
        {
            return name;
        }
        std::string getData() const
        {
            return data;
        }
        std::string getSessionId() const
        {
            return sessionId;
        }
        std::string getCharacterId() const
        {
            return characterId;
        }
        std::string getDisplayName() const
        {
            return displayName;
        }
    };

    inline NeuroResponse::NeuroResponse(const std::string& jsonStr)
    {
        try
        {
            if (jsonStr.empty())
            {
                command = id = name = data = sessionId = characterId = displayName = "";
                return;
            }
            nlohmann::json parsedJson = nlohmann::json::parse(jsonStr);

            if (parsedJson.contains("command") && parsedJson["command"].is_string())
            {
                command = parsedJson["command"];
            }
            else
            {
                throw std::invalid_argument("JSON is missing the 'command' field or it is not a string.");
            }

            if (parsedJson.contains("data") && parsedJson["data"].is_object())
            {
                auto& dataObj = parsedJson["data"];

                if (dataObj.contains("id") && dataObj["id"].is_string()) 
                    id = dataObj["id"];

                if (dataObj.contains("name") && dataObj["name"].is_string()) 
                    name = dataObj["name"];

                if (dataObj.contains("data"))
                {
                    if (dataObj["data"].is_string()) 
                        data = dataObj["data"].get<std::string>();
                    else 
                        data = dataObj["data"].dump();
                }

                if (dataObj.contains("session") && dataObj["session"].is_object())
                {
                    auto& sessionObj = dataObj["session"];

                    if (sessionObj.contains("sessionId") && sessionObj["sessionId"].is_string()) 
                        sessionId = sessionObj["sessionId"];

                    if (sessionObj.contains("characterId") && sessionObj["characterId"].is_string()) 
                        characterId = sessionObj["characterId"];

                    if (sessionObj.contains("displayName") && sessionObj["displayName"].is_string()) 
                        displayName = sessionObj["displayName"];
                }
            }
        }
        catch (const nlohmann::json::parse_error& e)
        {
            throw std::invalid_argument("Invalid JSON string: " + std::string(e.what()));
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Error processing JSON: " + std::string(e.what()));
        }
    }

    class NeuroGameClient
    {
    public:
        virtual ~NeuroGameClient()
        {
            shutting_down = true;
            if (connected)
            {
                ws_client.stop();
            }
            condition.notify_all();
        }

        NeuroGameClient(const std::string& uri, std::string game_name, std::ostream* output_stream = &std::cout, std::ostream* error_stream = &std::cerr, int timeout = -1, bool retry_on_fail = true)
            : game_name(std::move(game_name)), lastResponse(""), timeout(timeout), uri(uri), retry_on_fail(retry_on_fail)
        {

            output = output_stream;
            error = error_stream;

            setup_callbacks();
            _connect();
        }

        void _connect()
        {
            std::unique_lock<std::mutex> lock(reconnectMutex);
            if (shutting_down) 
                return;

            std::string finalUri;
            const char* env_url = std::getenv("NEURO_SDK_WS_URL");

            if (env_url != nullptr && std::string(env_url) != "")
            {
                finalUri = std::string(env_url);
                std::cout << "[DEBUG] Connecting via Environment Variable: " << finalUri << std::endl;
            }
            else
            {
                std::string wsIP = GetConfigString("WebSocketIP", "127.0.0.1");
                int wsPort = GetConfigInt("WebSocketPort", 8000);
                finalUri = "ws://" + wsIP + ":" + std::to_string(wsPort);
                std::cout << "[DEBUG] Connecting via Config File: " << finalUri << std::endl;
            }

            ws_client.setUrl(finalUri);

            if (retry_on_fail)
            {
                ws_client.enableAutomaticReconnection();
            }
            else
            {
                ws_client.disableAutomaticReconnection();
            }

            // Start the connection in a background thread
            ws_client.start();

            // Wait for the connection to establish
            if (timeout >= 0)
            {
                bool success = condition.wait_for(lock, std::chrono::seconds(timeout), [this]()
                    {
                        return connected || connection_failed || shutting_down;
                    });

                if (!success || connection_failed)
                {
                    *error << "Failed to connect to server" << std::endl;
                    throw std::runtime_error("Failed to connect to server");
                }
            }
            else
            {
                condition.wait(lock, [this]()
                    {
                        return connected || connection_failed || shutting_down;
                    });
                if (connection_failed && !retry_on_fail)
                {
                    *error << "Failed to connect to server" << std::endl;
                    throw std::runtime_error("Failed to connect to server");
                }
            }
        }

        void sendStartup()
        {
            nlohmann::json payload;
            payload["game"] = game_name;
            payload["command"] = "startup";

            std::unique_lock<std::mutex> lock(reconnectMutex);

            if (!(connected || shutting_down))
            {
                condition.wait(lock, [this]()
                    {
                        return connected || shutting_down;
                    });
            }
            lock.unlock();
            send(payload.dump());
        }

        void sendContext(const std::string& context_message, bool silent)
        {
            nlohmann::json payload;
            payload["game"] = game_name;
            payload["command"] = "context";
            payload["data"]["message"] = context_message;
            payload["data"]["silent"] = silent;
            send(payload.dump());
        }

        void sendRegisterActions(const std::vector<Action>& actions)
        {
            nlohmann::json payload;
            payload["game"] = game_name;
            payload["command"] = "actions/register";
            payload["data"]["actions"] = nlohmann::json::array();

            for (const auto& action : actions)
            {
                nlohmann::json action_json;
                action_json["name"] = action.getName();
                action_json["description"] = action.getDescription();
                action_json["schema"] = action.getSchema();
                payload["data"]["actions"].push_back(action_json);
            }
            send(payload.dump());
        }

        void sendUnregisterActions(const std::vector<std::string>& action_names)
        {
            nlohmann::json payload;
            payload["game"] = game_name;
            payload["command"] = "actions/unregister";
            nlohmann::json action_names_json;

            for (const auto& action : action_names)
            {
                action_names_json.push_back(action);
            }
            payload["data"]["action_names"] = action_names_json;
            send(payload.dump());
        }

        void sendForceActions(const std::string& state, const std::string& query, bool ephemeral, const std::vector<std::string>& actions, Priority priority = Priority::LOW)
        {
            nlohmann::json payload;
            payload["command"] = "actions/force";
            payload["game"] = game_name;
            payload["data"]["state"] = state;
            payload["data"]["query"] = query;
            payload["data"]["ephemeral_context"] = ephemeral;
            payload["data"]["priority"] = priorityToString(priority);
            payload["data"]["action_names"] = actions;
            send(payload.dump());
        }

        void sendActionResult(const NeuroResponse& neuroAction, bool success, const std::string& message)
        {
            nlohmann::json payload;
            payload["command"] = "action/result";
            payload["game"] = game_name;
            payload["data"]["id"] = neuroAction.getId();
            payload["data"]["success"] = success;
            payload["data"]["message"] = message;
            send(payload.dump());
        }

        void forceAction(const std::string& state, const std::string& query, bool ephemeral, const std::vector<std::string>& actions, Priority priority = Priority::LOW)
        {
            std::unique_lock<std::mutex> lock(mutex);
            forcedActions = actions;
            waitingForForcedAction = true;
            sendForceActions(state, query, ephemeral, actions, priority);

            if (timeout >= 0)
            {
                bool success = condition.wait_for(lock, std::chrono::seconds(timeout), [this]()
                    {
                        return !waitingForForcedAction || connection_failed || shutting_down;
                    });
                if (!success || connection_failed)
                {
                    *error << "Error waiting for forced action" << std::endl;
                    throw std::runtime_error("Error waiting for forced action");
                }
            }
            else
            {
                condition.wait(lock, [this]()
                    {
                        return !waitingForForcedAction || connection_failed || shutting_down;
                    });
                if (connection_failed)
                {
                    *error << "Error waiting for forced action" << std::endl;
                    throw std::runtime_error("Error waiting for forced action");
                }
            }
            forcedActions.clear();
        }

        void forceDisposableActions(const std::string& state, const std::string& query, bool ephemeral, const std::vector<Action>& actions, bool forceUnregister = false, Priority priority = Priority::LOW)
        {
            sendRegisterActions(actions);
            disposableActions = getActionNamesFromActions(actions);
            forceAction(state, query, ephemeral, getActionNamesFromActions(actions), priority);

            if (forceUnregister)
            {
                sendUnregisterActions(disposableActions);
            }
        }

        static std::vector<std::string> getActionNamesFromActions(const std::vector<Action>& actions)
        {
            std::vector<std::string> actionNames;

            for (const auto& action : actions)
            {
                actionNames.push_back(action.getName());
            }

            return actionNames;
        }

    protected:
        virtual void handleMessage(NeuroResponse const& response) = 0;

        void setup_callbacks()
        {
            ws_client.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
                {
                    if (msg->type == ix::WebSocketMessageType::Open)
                    {
                        on_open();
                    }
                    else if (msg->type == ix::WebSocketMessageType::Message)
                    {
                        on_message(msg->str);
                    }
                    else if (msg->type == ix::WebSocketMessageType::Close)
                    {
                        on_close();
                    }
                    else if (msg->type == ix::WebSocketMessageType::Error)
                    {
                        on_fail(msg->errorInfo.reason);
                    }
                });
        }

        void on_open()
        {
            *output << "Connection established!" << std::endl;
            {
                std::lock_guard<std::mutex> lock(reconnectMutex);
                connected = true;
                connection_failed = false;

                // Empty the message queue upon connection
                while (!messageQueue.empty())
                {
                    std::string msg = messageQueue.front();
                    messageQueue.pop();
                    ws_client.sendText(msg);
                }
            }
            condition.notify_all();
        }

        void on_message(const std::string& message)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                printf("[Neuro says] %s\n", message.c_str());

                auto JsonMessage = nlohmann::json::parse(message);
                if (JsonMessage["command"] == "actions/reregister_all") return;

                NeuroResponse const response = NeuroResponse(message);
                handleMessage(response);
                lastResponse = response;
            }
            condition.notify_all();
        }

        void on_close()
        {
            connected = false;
            *output << "Connection closed." << (retry_on_fail ? " IXWebSocket will auto-reconnect..." : "") << std::endl;
            condition.notify_all();
        }

        void on_fail(const std::string& error_msg)
        {
            std::lock_guard<std::mutex> lock(mutex);
            *error << "Connection failed! Error: " << error_msg << std::endl;

            if (!retry_on_fail)
            {
                connection_failed = true;
            }
            condition.notify_all();
        }

        void send(const std::string& message)
        {
            std::lock_guard<std::mutex> lock(reconnectMutex);
            if (connection_failed)
            {
                *error << "Trying to send message on a failed connection" << std::endl;
                throw std::runtime_error("Trying to send message on a failed connection");
            }
            if (!connected)
            {
                std::cerr << "Connection closed. Storing message in queue." << std::endl;
                messageQueue.push(message);
            }
            else
            {
                auto result = ws_client.sendText(message);
                printf("[Client sent] %s\n", message.c_str());
                if (!result.success)
                {
                    *error << "Error sending message. Storing message in queue." << std::endl;
                    messageQueue.push(message);
                }
            }
        }

        ix::WebSocket ws_client;
        std::ostream* output;
        std::ostream* error;
        std::string game_name;
        std::mutex mutex;
        std::mutex reconnectMutex;
        std::condition_variable condition;
        NeuroResponse lastResponse;

        bool waitingForForcedAction = false;
        bool connected = false;

    public:
        bool isConnected() const
        {
            return connected;
        }

    protected:
        std::vector<std::string> forcedActions;
        int timeout;
        bool connection_failed = false;
        std::vector<std::string> disposableActions;
        std::string uri;
        std::queue<std::string> messageQueue;
        bool retry_on_fail;
        bool shutting_down = false;
    };
}
#endif