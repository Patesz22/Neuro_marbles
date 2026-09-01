#pragma once
#include <string>
#include "main.h"
#include "misc.h"

// For future updates and for easier mod updates after a game update.
// Doesn't serve much purpose currently
 
namespace MemoryHelpers
{

    // Find any active Object/Widget by its string name
    inline SDK::UObject* FindObjectByName(const std::string& ObjectName, bool bRequireTransient = false)
    {
        for (int i = SDK::UObject::GObjects->Num() - 1; i >= 0; --i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            int32_t d; float df;
            if (!Obj || !SafeRead4Bytes(reinterpret_cast<uintptr_t>(Obj), &d, &df))
                continue;

            if (Obj->IsDefaultObject()) 
                continue;

            std::string fullName = Obj->GetFullName();

            if (fullName.find(ObjectName) != std::string::npos)
            {
                // If it's a UI widget, we usually only want it if it's currently on screen (Transient)
                if (bRequireTransient && fullName.find("Transient") == std::string::npos) 
                    continue;

                return Obj;
            }
        }
        return nullptr;
    }

    // Find any Blueprint Function in memory by its string name
    inline SDK::UFunction* FindFunctionByName(const std::string& FunctionName)
    {
        // Functions are loaded early, so we sweep forward!
        for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            if (!Obj) 
                continue;

            std::string fullName = Obj->GetFullName();
            if (fullName.find("Function") != std::string::npos && fullName.find(FunctionName) != std::string::npos)
            {
                return static_cast<SDK::UFunction*>(Obj);
            }
        }
        return nullptr;
    }
}

