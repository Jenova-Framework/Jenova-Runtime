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

// Jenova Resource Manager Difinition
class JenovaResourceManager : public Object
{
	GDCLASS(JenovaResourceManager, Object);

public:
	static JenovaResourceManager* get_singleton();

protected:
	static void _bind_methods();

private:
	bool areResourcesLoaded = false;

public:
	static void init();
	static void deinit();

public:
	bool CreateDatabaseFromArchive(const uint8_t* archviePtr, const size_t archiveSize);
	bool ReleaseDatabase() const;
	const jenova::MemoryBuffer& GetResourceRawBuffer(const String& dataID) const;
	const uint8_t* GetResourceRawFileData(const String& dataID) const;
	size_t GetResourceRawFileSize(const String& dataID) const;
};