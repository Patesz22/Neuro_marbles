#pragma once
#include <cstdint>

void PressKey(char key);

namespace MemoryHelpers
{
    bool SafeRead4Bytes(uintptr_t address, int32_t* outInt, float* outFloat);
    bool SafeCopyMemory(void* destination, void* source, size_t size);
}

void InspectUIWidgets();

