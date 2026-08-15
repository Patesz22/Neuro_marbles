#include "NeuroConfig.h"

std::map<std::string, std::string> GlobalConfig;

void LoadConfig()
{
    std::ifstream file("config.txt");
    if (!file.is_open())
    {
        printf("[WARNING] Could not open config.txt! Falling back to default settings.\n");
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Strip # lines
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos)
        {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            GlobalConfig[key] = value;
            printf("[DEBUG] Config Loaded: %s = %s\n", key.c_str(), value.c_str());
        }
    }
    file.close();
}


std::string GetConfigString(const std::string& key, const std::string& defaultVal)
{
    auto it = GlobalConfig.find(key);
    return (it != GlobalConfig.end()) ? it->second : defaultVal;
}

int GetConfigInt(const std::string& key, int defaultVal)
{
    auto it = GlobalConfig.find(key);
    if (it != GlobalConfig.end())
    {
        try { return std::stoi(it->second); }
        catch (...) {}
    }
    return defaultVal;
}

float GetConfigFloat(const std::string& key, float defaultVal)
{
    auto it = GlobalConfig.find(key);
    if (it != GlobalConfig.end())
    {
        try { return std::stof(it->second); }
        catch (...) {}
    }
    return defaultVal;
}
