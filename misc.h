#pragma once
#include "main.h"

void PressKey(int virtualKeyCode);

bool SafeRead4Bytes(uintptr_t address, int32_t* outInt, float* outFloat);
bool SafeProcessEvent(SDK::UObject* TargetObject, SDK::UFunction* Function, void* Params);
bool SafeCopyMemory(void* destination, void* source, size_t size);

