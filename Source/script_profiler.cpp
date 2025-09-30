
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

// Storage
HashMap<StringName, double> stageTimings;
HashMap<StringName, HashMap<StringName, double>> executionTimings;

// Jenova Profiler Implementation
bool JenovaProfiler::Initialize()
{
	// Verbose
	jenova::Output("Jenova Profiler Initialized.");

	// All Good
	return true;
}
bool JenovaProfiler::Shutdown()
{
	// Clear Storage
	ClearRecords();

	// All Good
	return true;
}
bool JenovaProfiler::IsEnabled()
{
	switch (JenovaProfiler::profilingMode)
	{
	case jenova::ProfilingMode::Echo:
	case jenova::ProfilingMode::Sentinel:
	case jenova::ProfilingMode::Monitor:
		return true;
	case jenova::ProfilingMode::Disabled:
	case jenova::ProfilingMode::Unknown:
	default:
		return false;
	}
}
void JenovaProfiler::SetProfilingMode(jenova::ProfilingMode profilingMode)
{
	// Update Profiling Mode
	JenovaProfiler::profilingMode = profilingMode;
}
void JenovaProfiler::StartRecording()
{
	isRecording = true;
}
void JenovaProfiler::StopRecording()
{
	isRecording = false;
}
void JenovaProfiler::ClearRecords()
{
	// Clear Storage
	stageTimings.clear();
	executionTimings.clear();
}
bool JenovaProfiler::AddStageRecord(const StringName& stageName, double duration)
{
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return;
	stageTimings[stageName] = duration;
	return true;
}
bool JenovaProfiler::AddExecutionRecord(const StringName& scriptPath, const StringName& functionName, double duration)
{
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return;
	executionTimings[scriptPath][functionName] = duration;
	return true;
}
double JenovaProfiler::GetStageRecord(const StringName& stageName)
{
	if (stageTimings.has(stageName)) return stageTimings[stageName];
	return 0.0
}
double JenovaProfiler::GetExecutionRecord(const StringName& scriptPath, const StringName& functionName)
{
	if (executionTimings.has(scriptPath))
	{
		const HashMap<StringName, double>& functionMap = executionTimings[scriptPath];
		if (functionMap.has(functionName)) return functionMap[functionName];
	}
	return 0.0;
}
void JenovaProfiler::Frame()
{
	// Validate Profiler
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return;
	if (JenovaProfiler::isRecording == false) return;
}