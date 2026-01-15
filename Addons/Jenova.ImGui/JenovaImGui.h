
/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|                         Jenova ImGui                         |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Jenova-ImGui SDK
#pragma once

// Imports
#include <string>
#include <functional>
#include "imgui.h"

// Jenova-ImGui API Import/Export
#ifdef JENOVA_IMGUI_SDK_BUILD
	#define JENOVAIMGUI_API	 __declspec(dllexport)
#else
	#define JENOVAIMGUI_API	 __declspec(dllimport)
#endif

// Jenova-ImGui Namespace
namespace JenovaImGui
{
	// Jenova-ImGui :: Type Definitions
	typedef const char* UIRenderEventID;
	typedef std::function<void()> UIRenderCallback;

	// Jenova-ImGui :: API Functions
	JENOVAIMGUI_API bool AddUserInterfaceRenderEvent(UIRenderEventID eventID, UIRenderCallback callbackPtr);
	JENOVAIMGUI_API bool RemoveUserInterfaceRenderEvent(UIRenderEventID eventID);
	JENOVAIMGUI_API void SetUserInterfaceRenderingState(bool renderState);
}