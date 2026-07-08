#pragma once
#include "misc.h"


void PressKey(int virtualKeyCode)
{
    keybd_event(virtualKeyCode, 0, 0, 0);
    Sleep(100);
    keybd_event(virtualKeyCode, 0, KEYEVENTF_KEYUP, 0);
}

bool SafeRead4Bytes(uintptr_t address, int32_t* outInt, float* outFloat)
{
    __try
    {
        *outInt = *reinterpret_cast<int32_t*>(address);
        *outFloat = *reinterpret_cast<float*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool SafeProcessEvent(SDK::UObject* TargetObject, SDK::UFunction* Function, void* Params)
{
    if (!TargetObject || !Function) return false;

    __try
    {
        TargetObject->ProcessEvent(Function, Params);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool SafeCopyMemory(void* destination, void* source, size_t size)
{
    if (!source || !destination || size <= 0) return false;

    __try
    {
        memcpy(destination, source, size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}