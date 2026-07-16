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

// Jenova Asset Monitor Values & Types
namespace jenova::AssetMonitor
{
	// Enumerators
	enum class CallbackEvent
	{
		Added,
		Removed,
		Modified,
		RenamedOld,
		RenamedNew
	};

	// Function Definitions
	typedef void(*AssetMonitorCallback)(const String& targetPath, const CallbackEvent& callbackEvent);
}

// Jenova Asset Monitor Difinition
class JenovaAssetMonitor : public Object
{
	GDCLASS(JenovaAssetMonitor, Object);

public:
	static JenovaAssetMonitor* get_singleton();

protected:
	static void _bind_methods();

private:
	bool isMonitorActive = false;

public:
	static void init();
	static void deinit();

public:
	bool AddDirectory(const String& directoryPath);
	bool AddDirectories(const PackedStringArray& directoryList);
	bool AddFile(const String& filePath);
	bool AddFiles(const PackedStringArray& fileList);
	bool RegisterCallback(jenova::AssetMonitor::AssetMonitorCallback callback);
	bool UnregisterCallback(jenova::AssetMonitor::AssetMonitorCallback callback);
	bool PrepareForShutdown();
};