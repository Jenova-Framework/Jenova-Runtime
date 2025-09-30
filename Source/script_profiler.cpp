
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
std::unordered_map<std::string, double> stageTimings;
std::unordered_map<std::string, std::unordered_map<std::string, double>> executionTimings;
std::vector<StringName> activeMonitors;

// Jenova Profiler Implementation :: Main
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

	// Release Monitors
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Monitor)
	{
		Performance* perf = Performance::get_singleton();
		for (const StringName& id : activeMonitors)
		{
			if (perf->has_custom_monitor(id)) perf->remove_custom_monitor(id);
		}
		activeMonitors.clear();
	}

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
bool JenovaProfiler::Prepare(jenova::json_t& data)
{
	// Validate Profiler
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return false;

	// Prepare for Monitor Mode
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Monitor)
	{
		Performance* perf = Performance::get_singleton();
		activeMonitors.clear();
		for (auto& [scriptUID, scriptData] : data["Scripts"].items())
		{
			std::string scriptPath = scriptData["path"];
			auto& methods = scriptData["methods"];
			for (auto& [functionName, methodData] : methods.items())
			{
				StringName monitorID = String(scriptPath.c_str()).replace("res://", "") + "/" + String(functionName.c_str()) + "_(µs)";
				if (perf->has_custom_monitor(monitorID)) perf->remove_custom_monitor(monitorID);
				Callable monitorCallable = callable_mp_static(&JenovaProfiler::MonitorReport).bind(String(scriptPath.c_str()), String(functionName.c_str()));
				perf->add_custom_monitor(monitorID, monitorCallable);
				activeMonitors.push_back(monitorID);
			}
		}
	}

	// All Good
	return true;
}
void JenovaProfiler::SetProfilingMode(jenova::ProfilingMode profilingMode)
{
	// Update Profiling Mode
	JenovaProfiler::profilingMode = QUERY_ENGINE_MODE(Editor) ? jenova::ProfilingMode::Disabled : profilingMode;
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
bool JenovaProfiler::AddStageRecord(const std::string& stageName, double duration)
{
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return false;
	stageTimings[stageName] = duration;
	return true;
}
bool JenovaProfiler::AddExecutionRecord(const std::string& scriptPath, const std::string& functionName, double duration)
{
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return false;
	executionTimings[scriptPath][functionName] = duration;
	return true;
}
double JenovaProfiler::GetStageRecord(const std::string& stageName)
{
	if (stageTimings.contains(stageName)) return stageTimings[stageName];
	return 0.0;
}
double JenovaProfiler::GetExecutionRecord(const std::string& scriptPath, const std::string& functionName)
{
	if (executionTimings.contains(scriptPath))
	{
		const auto& functionMap = executionTimings.at(scriptPath);
		if (functionMap.contains(functionName)) return functionMap.at(functionName);
	}
	return 0.0;
}
void JenovaProfiler::Frame()
{
	// Validate Profiler
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Disabled) return;
	if (JenovaProfiler::isRecording == false) return;
}

// Jenova Profiler Implementation :: Callbacks
double JenovaProfiler::MonitorReport(const String& scriptPath, const String& functionName)
{
	auto scriptIt = executionTimings.find(AS_STD_STRING(scriptPath));
	if (scriptIt != executionTimings.end())
	{
		auto& functionMap = scriptIt->second;
		auto funcIt = functionMap.find(AS_STD_STRING(functionName));
		if (funcIt != functionMap.end())
		{
			double value = funcIt->second;
			funcIt->second = 0.0;
			return value * 1000.0;
		}
	}
	return 0.0;
}