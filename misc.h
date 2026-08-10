#pragma once
#include "main.h"

void PressKey(char key);

bool SafeRead4Bytes(uintptr_t address, int32_t* outInt, float* outFloat);
bool SafeCopyMemory(void* destination, void* source, size_t size);

void InspectUIWidgets();

