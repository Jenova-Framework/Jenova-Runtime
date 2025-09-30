#pragma once

/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|                        Jenova Runtime                        |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Jenova SDK
#include "Jenova.hpp"

// Jenova Interpreter Definitions
class JenovaProfiler
{
// Script Profiler API
public:
	static bool Initialize();
	static bool Shutdown();
	static bool IsEnabled();
	static void SetProfilingMode(jenova::ProfilingMode profilingMode);
	static void StartRecording();
	static void StopRecording();
	static void ClearRecords();
	static bool AddStageRecord(const StringName& stageName, double duration);
	static bool AddExecutionRecord(const StringName& scriptPath, const StringName& functionName, double duration);
	static double GetStageRecord(const StringName& stageName);
	static double GetExecutionRecord(const StringName& scriptPath, const StringName& functionName);
	static void Frame();

private:
	static inline bool isRecording = false;
	static inline jenova::ProfilingMode profilingMode = jenova::ProfilingMode::Unknown;
};