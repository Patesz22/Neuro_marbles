#include "misc.h"
#include "GameState.h"
#include <windows.h>


void PressKey(char key)
{
    HWND gameWindow = FindWindowA("UnrealWindow", "Marbles On Stream");
    if (!gameWindow)
    {
        gameWindow = FindWindowA("UnrealWindow", NULL); // Fallback
    }

    if (gameWindow)
    {
        UINT scanCode = MapVirtualKeyA(key, MAPVK_VK_TO_VSC);

        // Construct the exact 32-bit lParam that Unreal Engine requires
        LPARAM lParamDown = 1 | (scanCode << 16);
        LPARAM lParamUp = 1 | (scanCode << 16) | (1 << 30) | (1 << 31);

        // Send the formatted messages
        PostMessage(gameWindow, WM_KEYDOWN, (WPARAM)key, lParamDown);
        Sleep(50); // Important: Give UE's tick rate time to process the Down state
        PostMessage(gameWindow, WM_KEYUP, (WPARAM)key, lParamUp);

        printf("[Action]: Sent UE-Formatted Background Key: %c\n", key);
    }
    else
    {
        printf("[ERROR] Could not find the UnrealWindow!\n");
    }
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

void InspectUIWidgets()
{
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject()) continue;

        if (Obj->IsA(SDK::UUserWidget::StaticClass()))
        {
            SDK::UUserWidget* Widget = static_cast<SDK::UUserWidget*>(Obj);

            if (Widget->IsHovered())
            {
                printf("\n[UI WIDGET CLICKED]\n");
                printf("Instance: %s\n", Widget->GetName().c_str());
                printf("Class:    %s\n", Widget->GetFullName().c_str());
            }
        }
    }
}
