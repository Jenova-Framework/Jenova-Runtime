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
	static bool Prepare(jenova::json_t& data);
	static void SetProfilingMode(jenova::ProfilingMode profilingMode);
	static void StartRecording();
	static void StopRecording();
	static void ClearRecords();
	static bool AddStageRecord(const std::string& stageName, double duration);
	static bool AddExecutionRecord(const std::string& scriptPath, const std::string& functionName, double duration);
	static double GetStageRecord(const std::string& stageName);
	static double GetExecutionRecord(const std::string& scriptPath, const std::string& functionName);
	static void Frame();

public:
	static double MonitorReport(const String& scriptPath, const String& functionName);

private:
	static inline bool isRecording = false;
	static inline jenova::ProfilingMode profilingMode = jenova::ProfilingMode::Unknown;
};