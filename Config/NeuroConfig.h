#pragma once
#include "Logging/Logging.h"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

extern std::map<std::string, std::string> GlobalConfig;

void LoadConfig();

std::string GetConfigString(const std::string& key, const std::string& defaultVal = "");
int GetConfigInt(const std::string& key, int defaultVal = 0);
float GetConfigFloat(const std::string& key, float defaultVal = 0.0f);
