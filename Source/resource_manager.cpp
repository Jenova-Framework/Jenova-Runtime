
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

// Archive SDK
#define LIBARCHIVE_STATIC
#include "Archive/archive.h"
#include "Archive/archive_entry.h"

// Include Storage
#include "Storage.h"

// Global Database
jenova::ResourceDatabase database;

// Singleton Instance
JenovaResourceManager* jnvrm_singleton = nullptr;

// Initializer/Deinitializer
void JenovaResourceManager::init()
{
    // Register Class
    ClassDB::register_internal_class<JenovaResourceManager>();

    // Initialize Singleton
	jnvrm_singleton = memnew(JenovaResourceManager);

	// Load Default Asset Package
	if (!jnvrm_singleton->CreateDatabaseFromArchive(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(JENOVA_ASSET_PACK_LZMA2))))
	{
		jenova::Warning("Jenova Resource Manager", "Failed to Load Default Asset Package.");
	}

    // Verbose
    jenova::Output("Jenova Resource Manager Initialized.");
}
void JenovaResourceManager::deinit()
{
    // Release Singleton
	if (jnvrm_singleton)
	{
		jnvrm_singleton->ReleaseDatabase();
		memdelete(jnvrm_singleton);
	}
}

// Bindings
void JenovaResourceManager::_bind_methods() {}

// Singleton Handling
JenovaResourceManager* JenovaResourceManager::get_singleton()
{
    return jnvrm_singleton;
}

// Jenova Resource Manager Implementation
bool JenovaResourceManager::CreateDatabaseFromArchive(const uint8_t* archviePtr, const size_t archiveSize)
{
    // Clear Database
	areResourcesLoaded = false;
    ReleaseDatabase();

    // Unpack & Store Files In Database
	struct archive* a;
	struct archive_entry* entry;
	int r;

	a = archive_read_new();
	archive_read_support_format_all(a);
	archive_read_support_filter_all(a);
	if ((r = archive_read_open_memory(a, archviePtr, archiveSize))) return false;
	for (;;)
	{
		// Read Next Header
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF) break;
		if (r < ARCHIVE_OK) jenova::Error("Jenova Resource Manager", "Failed to Parse Archive, Reason [%d] : %s", __LINE__, archive_error_string(a));
		if (r < ARCHIVE_WARN) return false;

		// Skip Directories
		if (archive_entry_filetype(entry) == AE_IFDIR)
		{
			archive_read_data_skip(a);
			continue;
		}

		// Extract File Data
		size_t fileSize = archive_entry_size(entry);
		if (fileSize > 0)
		{
			jenova::MemoryBuffer buffer;
			buffer.resize(fileSize);
			const void* buff;
			size_t buffSize;
			la_int64_t offset;
			size_t totalRead = 0;
			for (;;)
			{
				r = archive_read_data_block(a, &buff, &buffSize, &offset);
				if (r == ARCHIVE_EOF) break;
				if (r < ARCHIVE_OK)
				{
					jenova::Error("Jenova Resource Manager", "Failed to Read Data Block, Reason [%d] : %s", __LINE__, archive_error_string(a));
					return false;
				}
				if (r < ARCHIVE_WARN) return false;

				memcpy(buffer.data() + totalRead, buff, buffSize);
				totalRead += buffSize;
			}
			String fullPath = String(archive_entry_pathname(entry));
			String key = fullPath.get_file().get_basename();
			database[key] = std::move(buffer);
		}
		else
		{
			archive_read_data_skip(a);
		}
	}
	archive_read_close(a);
	archive_read_free(a);

	// All Good
	areResourcesLoaded = true;
	return areResourcesLoaded;
}
bool JenovaResourceManager::ReleaseDatabase() const
{
	// Validation
	if (!areResourcesLoaded) return false;

	// Release Buffers
	for (auto& pair : database) jenova::MemoryBuffer().swap(pair.second);
	database.clear();

	// All Good
	return true;
}
const uint8_t* JenovaResourceManager::GetResourceRawFileData(const String& dataID) const
{
	if (!areResourcesLoaded) return nullptr;
	if (database.contains(dataID)) return database[dataID].data();
	return nullptr;
}
size_t JenovaResourceManager::GetResourceRawFileSize(const String& dataID) const
{
	if (!areResourcesLoaded) return 0;
	if (database.contains(dataID)) return database[dataID].size();
	return 0;
}
const jenova::MemoryBuffer& JenovaResourceManager::GetResourceRawBuffer(const String& dataID) const
{
	if (!areResourcesLoaded) return jenova::MemoryBuffer();
	if (database.contains(dataID)) return database[dataID];
	return jenova::MemoryBuffer();
}