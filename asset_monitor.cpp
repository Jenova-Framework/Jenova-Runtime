
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

// Third-Party SDK
#include <FileWatch/FileWatch.hpp>

/*
	Note : To make FileWatch provide full path instead of releative path to initialized path I've added `_path` to _callback(_path + file.first, file.second);
*/

// Helpers
static String GetCallbackEventStringName(jenova::AssetMonitor::CallbackEvent callbackEvent)
{
	switch (callbackEvent)
	{
	case jenova::AssetMonitor::CallbackEvent::Added:
		return "Added";
	case jenova::AssetMonitor::CallbackEvent::Removed:
		return "Removed";
	case jenova::AssetMonitor::CallbackEvent::Modified:
		return "Modified";
	case jenova::AssetMonitor::CallbackEvent::RenamedOld:
		return "RenamedOld";
	case jenova::AssetMonitor::CallbackEvent::RenamedNew:
		return "RenamedNew";
	default:
		return "Unknown";
	}
}

// Global Storage
Vector<filewatch::FileWatch<std::string>*> assetMonitors;
Vector<jenova::AssetMonitor::AssetMonitorCallback> monitorCallbacks;
std::unordered_map<std::string, std::chrono::system_clock::time_point> lastRead;

// Singleton Instance
JenovaAssetMonitor* singleton = nullptr;

// Initializer/Deinitializer
void JenovaAssetMonitor::init()
{
    // Register Class
    ClassDB::register_internal_class<JenovaAssetMonitor>();

    // Initialize Singleton
    singleton = memnew(JenovaAssetMonitor);

    // Verbose
    jenova::Output("Jenova Asset Monitor Initialized.");
}
void JenovaAssetMonitor::deinit()
{
    // Release Singleton
    if (singleton) memdelete(singleton);
}

// Bindings
void JenovaAssetMonitor::_bind_methods()
{
	ClassDB::bind_static_method("JenovaAssetMonitor", D_METHOD("GetInstance"), &JenovaAssetMonitor::get_singleton);
	ClassDB::bind_method(D_METHOD("AddDirectory"), &JenovaAssetMonitor::AddDirectory);
	ClassDB::bind_method(D_METHOD("AddDirectories"), &JenovaAssetMonitor::AddDirectories);
	ClassDB::bind_method(D_METHOD("AddFile"), &JenovaAssetMonitor::AddFile);
	ClassDB::bind_method(D_METHOD("AddFiles"), &JenovaAssetMonitor::AddFiles);
	ADD_SIGNAL(MethodInfo("callback", PropertyInfo(Variant::STRING, "targetPath"), PropertyInfo(Variant::STRING, "callbackEvent")));
}

// Singleton Handling
JenovaAssetMonitor* JenovaAssetMonitor::get_singleton()
{
    return singleton;
}

// Jenova Asset Monitor Implementation
bool JenovaAssetMonitor::AddDirectory(const String& directoryPath) 
{
	// Create New Asset Monitor
	auto assetMonitor = new filewatch::FileWatch<std::string>(AS_STD_STRING(directoryPath), [](const std::string& path, const filewatch::Event change_type)
	{
		if (change_type == filewatch::Event::modified)
		{
			auto lastWriteTime = std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(path));

			if (lastRead.find(path) == lastRead.end() || std::chrono::duration_cast<std::chrono::milliseconds>(lastWriteTime - lastRead[path]).count() > 100)
			{
				lastRead[path] = lastWriteTime;
				for (const auto& callback : monitorCallbacks) callback(String(path.c_str()), jenova::AssetMonitor::CallbackEvent(change_type));
				JenovaAssetMonitor::get_singleton()->emit_signal("callback", String(path.c_str()), GetCallbackEventStringName(jenova::AssetMonitor::CallbackEvent(change_type)));
			}
		}
		else
		{
			for (const auto& callback : monitorCallbacks) callback(String(path.c_str()), jenova::AssetMonitor::CallbackEvent(change_type));
			JenovaAssetMonitor::get_singleton()->emit_signal("callback", String(path.c_str()), GetCallbackEventStringName(jenova::AssetMonitor::CallbackEvent(change_type)));
		}
	});

	// Add to Monitors
	assetMonitors.push_back(assetMonitor);

	// All Good
	return true;
}
bool JenovaAssetMonitor::AddDirectories(const PackedStringArray& directoryList) 
{
	// Fallback to AddDirectory
	for (const auto& directory : directoryList) if (!AddDirectory(directory)) return false;

	// All Good
	return true;
}
bool JenovaAssetMonitor::AddFile(const String& filePath) 
{
	// Current Implementation Doesn't Require Any Different Behaviour Between Directories and Files
	return AddDirectory(filePath);
}
bool JenovaAssetMonitor::AddFiles(const PackedStringArray& fileList) 
{
	// Fallback to AddDirectory
	for (const auto& file : fileList) if (!AddFile(file)) return false;

	// All Good
	return true;
}
bool JenovaAssetMonitor::RegisterCallback(jenova::AssetMonitor::AssetMonitorCallback callback)
{
	// Add Callback to Storage
	monitorCallbacks.push_back(callback);

	// All Good
	return true;
}
bool JenovaAssetMonitor::UnregisterCallback(jenova::AssetMonitor::AssetMonitorCallback callback)
{
	// Remove Callback from Storage
	monitorCallbacks.erase(callback);

	// All Good
	return true;
}
bool JenovaAssetMonitor::PrepareForShutdown()
{
	// Clear Resources
	monitorCallbacks.clear();
	for (const auto& assetMonitor : assetMonitors) delete assetMonitor;
	
	// All Good
	return true;
}