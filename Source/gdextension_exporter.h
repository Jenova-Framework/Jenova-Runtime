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

// Jenova GDExtension Enumerators
enum class GDExtensionTarget
{
	Windows64,
	Linux64,
	Unknown
};
enum class GDExtensionType
{
	Binary,
	Source,
	Unknown
};

// Jenova GDExtension Exporter Definitions
class GDExtensionExporter
{
public:
	GDExtensionExporter(GDExtensionTarget target, GDExtensionType type);

// Public Methods
public:
	void Initialize();
	bool Export();

// Internal Methods
private:
	void PickExportOutputFile();

// Internal Objects
private:
	std::string exportOutputFile				= "";
	GDExtensionTarget extensionTarget			= GDExtensionTarget::Unknown;
	GDExtensionType extensionType				= GDExtensionType::Unknown;
};