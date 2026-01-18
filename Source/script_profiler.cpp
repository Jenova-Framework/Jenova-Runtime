
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

// Types
template <typename T> using FrameRecord = std::vector<T>;
typedef std::unordered_map<std::string, double> FunctionRecord;
typedef std::unordered_map<std::string, FunctionRecord> ScriptRecord;
typedef std::unordered_map<int, jenova::SteadyTimePoint> SpanRecord;

// Storages
std::vector<StringName>			activeMonitors;
FunctionRecord					stageTimings;
FrameRecord<FunctionRecord>		stageFrameRecords;
ScriptRecord					executionTimings;
FrameRecord<ScriptRecord>		executionFrameRecords;
SpanRecord						spansStorage;

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
	// Generate Profiling Report
	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Echo)
	{
		if (!GenerateReportDatabase()) jenova::Error("Jenova Profiler", "Failed to Generate Performance Report.");
	}

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
	if (!IsEnabled()) return false;

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
				StringName monitorID = String(scriptPath.c_str()).replace("res://", "").replace("/", "\\") + "/" + String(functionName.c_str()) + "_(µs)";
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
	if (!IsEnabled()) return;
	if (isRecording == true) return;
	isRecording = true;
}
void JenovaProfiler::StopRecording()
{
	if (!IsEnabled()) return;
	if (isRecording == false) return;
	isRecording = false;
}
void JenovaProfiler::ClearRecords()
{
	// Clear Storage
	stageTimings.clear();
	stageFrameRecords.clear();
	executionTimings.clear();
	executionFrameRecords.clear();
}
bool JenovaProfiler::GenerateReportDatabase()
{
	// Generate Performance Report Database
	jenova::json_t reportDatabase;
	reportDatabase["StageFrameRecords"] = stageFrameRecords;
	reportDatabase["ExecutionFrameRecords"] = executionFrameRecords;

	// Write Performance Report Database On Disk
	String reportPath = jenova::GetJenovaCacheDirectory() + jenova::GlobalSettings::JenovaProfilerReportDatabaseFile;
	if (!jenova::WriteStdStringToFile(AS_STD_STRING(reportPath), reportDatabase.dump(2))) return false;

	// All Good
	return true;
}
void JenovaProfiler::SetCurrentExecutionContext(const std::string& scriptPath, const std::string& functionName)
{
	// Set Current Context
	currentContext.script = scriptPath;
	currentContext.function = functionName;

	// Set Context Check Time
	contextCheckTime = std::chrono::steady_clock::now();
}
bool JenovaProfiler::AddStageRecord(const std::string& stageName, double duration)
{
	if (!IsEnabled()) return false;
	if (duration == 0)
	{
		auto now = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration<double, std::milli>(now - contextCheckTime).count();
		contextCheckTime = now;
	}
	stageTimings[stageName] = duration;
	return true;
}
bool JenovaProfiler::AddStageRecord(const std::string& scriptPath, const std::string& stageName, double duration)
{
	if (!IsEnabled()) return false;
	stageTimings[stageName] = duration;
	return true;
}
bool JenovaProfiler::AddExecutionRecord(const std::string& scriptPath, const std::string& functionName, double duration)
{
	if (!IsEnabled()) return false;
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
	if (!IsEnabled()) return;
	if (isRecording == false) return;

	// Increase Profiler Tick
	profilerTick++;

	if (JenovaProfiler::profilingMode == jenova::ProfilingMode::Echo)
	{
		// Add Frame Records
		stageFrameRecords.push_back(stageTimings);
		executionFrameRecords.push_back(executionTimings);

		// Reset Records
		/*
		 * Currently we're keeping non-executed functions as records, In future it can be improved.
		 * Records can be just cleared and in data processing if the record hasn't the function we count it as 0.0
		 * This will reduce the size of record database, also maybe use sql instead of json? :d
		*/
		for (auto& [stage, record] : stageTimings) record = 0;
		for (auto& [script, records] : executionTimings) for (auto& [function, record] : records) record = 0;
	}
}
class Ref<AutoSpan> JenovaProfiler::BeginAutoSpan(const std::string& friendlyName)
{
	Ref<AutoSpan> autospan;
	autospan.instantiate();
	autospan->Initialize(String(friendlyName.c_str()));
	return autospan;
}
void JenovaProfiler::BeginSpan(int spanID)
{
	if (!IsEnabled()) return;
	spansStorage[spanID] = std::chrono::high_resolution_clock::now();
}
void JenovaProfiler::EndSpan(int spanID, const std::string& friendlyName)
{
	if (!IsEnabled()) return;
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration<double, std::milli>(now - spansStorage[spanID]).count();
	AddStageRecord(friendlyName, duration);
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

// Auto Span Implementation
void AutoSpan::Initialize(const String& autoSpanName)
{
	JenovaProfiler::BeginSpan(HASH_CSTR(AS_C_STRING(autoSpanName)));
	spanName = autoSpanName;
}
AutoSpan::~AutoSpan()
{
	JenovaProfiler::EndSpan(HASH_CSTR(AS_C_STRING(spanName)), AS_STD_STRING(spanName));
}
