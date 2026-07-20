
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
	// Validation
	if (jnvrm_singleton != nullptr) return;

    // Register Class
    ClassDB::register_internal_class<JenovaResourceManager>();

    // Initialize Singleton
	jnvrm_singleton = memnew(JenovaResourceManager);

	// Load Default Asset Package
	if (!jnvrm_singleton->CreateDatabaseFromArchive(nullptr, 0))
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
		jnvrm_singleton = nullptr;
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
bool JenovaResourceManager::CreateDatabaseFromArchive(const uint8_t* archivePtr, size_t archiveSize)
{
	// Default Package
	if (archivePtr == nullptr || archiveSize == 0)
	{
		archivePtr = JENOVA_RESOURCE(JENOVA_ASSET_PACK_LZMA2);
		archiveSize = sizeof(JENOVA_RESOURCE(JENOVA_ASSET_PACK_LZMA2));
	}

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
	if ((r = archive_read_open_memory(a, archivePtr, archiveSize))) return false;
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
            const void* buff = nullptr;
            size_t buffSize = 0;
            la_int64_t offset = 0;
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
			String key = fullPath.get_file();
			int dotPos = key.find(".");
			if (dotPos != -1) key = key.substr(0, dotPos);
			database[key] = std::move(buffer);
		}
		else archive_read_data_skip(a);
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
	for (auto& pair : database) jenova::ReleaseMemoryBuffer(pair.second);
	database.clear();

	// All Good
	return true;
}
const jenova::MemoryBuffer& JenovaResourceManager::GetResourceRawBuffer(const String& dataID) const
{
	static const jenova::MemoryBuffer emptyBuffer;
	if (!areResourcesLoaded) return emptyBuffer;
	if (database.contains(dataID)) return database[dataID];
	jenova::ErrorMessage("Jenova Resource Manager", "Invalid Resource '%s' Requested, Returning Empty Buffer.", AS_C_STRING(dataID));
	return emptyBuffer;
}
const uint8_t* JenovaResourceManager::GetResourceRawFileData(const String& dataID) const
{
	if (!areResourcesLoaded) return nullptr;
	if (database.contains(dataID)) return database[dataID].data();
	jenova::ErrorMessage("Jenova Resource Manager", "Invalid Resource '%s' Requested, Returning Null.", AS_C_STRING(dataID));
	return nullptr;
}
size_t JenovaResourceManager::GetResourceRawFileSize(const String& dataID) const
{
	if (!areResourcesLoaded) return 0;
	if (database.contains(dataID)) return database[dataID].size();
	jenova::ErrorMessage("Jenova Resource Manager", "Invalid Resource '%s' Requested, Returning Zero.", AS_C_STRING(dataID));
	return 0;
}

// Standalone Extractor (For Resource Manager)
jenova::MemoryBuffer JenovaResourceManager::PullEntity(const uint8_t* archivePtr, size_t archiveSize, const std::string& entityID)
{
    // Default Package
    if (archivePtr == nullptr || archiveSize == 0)
    {
        archivePtr = JENOVA_RESOURCE(JENOVA_ASSET_PACK_LZMA2);
        archiveSize = sizeof(JENOVA_RESOURCE(JENOVA_ASSET_PACK_LZMA2));
    }

    // Result Buffer
    jenova::MemoryBuffer result;

    // Unpack & Find Entity
    struct archive* a;
    struct archive_entry* entry;
    int r;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    if ((r = archive_read_open_memory(a, archivePtr, archiveSize)))
    {
        jenova_log("[Jenova Resource Manager] Error : %s", "Failed to Open Archive");
        return result;
    }

    for (;;)
    {
        // Read Next Header
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r < ARCHIVE_OK)
        {
            jenova_log("[Jenova Resource Manager] Error : Failed to Parse Archive, Reason [%d] : %s", __LINE__, archive_error_string(a));
            archive_read_close(a);
            archive_read_free(a);
            return result;
        }
        if (r < ARCHIVE_WARN)
        {
            archive_read_close(a);
            archive_read_free(a);
            return result;
        }

        // Skip Directories
        if (archive_entry_filetype(entry) == AE_IFDIR)
        {
            archive_read_data_skip(a);
            continue;
        }

        // Prepare Entity File Name
        std::string fullPath = archive_entry_pathname(entry);
        std::string filename = fullPath;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) filename = filename.substr(lastSlash + 1);
        size_t dotPos = filename.find_last_of(".");
        if (dotPos != std::string::npos) filename = filename.substr(0, dotPos);

        // Entity ID Check
        if (filename == entityID)
        {
            // Extract File Data
            size_t fileSize = archive_entry_size(entry);
            if (fileSize > 0)
            {
                result.resize(fileSize);
                const void* buff = nullptr;
                size_t buffSize = 0;
                la_int64_t offset = 0;
                size_t totalRead = 0;
                for (;;)
                {
                    r = archive_read_data_block(a, &buff, &buffSize, &offset);
                    if (r == ARCHIVE_EOF) break;
                    if (r < ARCHIVE_OK)
                    {
                        jenova_log("[Jenova Resource Manager] Error : Failed to Read Data Block, Reason [%d] : %s", __LINE__, archive_error_string(a));
                        result.clear();
                        archive_read_close(a);
                        archive_read_free(a);
                        return result;
                    }
                    if (r < ARCHIVE_WARN)
                    {
                        result.clear();
                        archive_read_close(a);
                        archive_read_free(a);
                        return result;
                    }
                    memcpy(result.data() + totalRead, buff, buffSize);
                    totalRead += buffSize;
                }
            }
            archive_read_close(a);
            archive_read_free(a);
            return result;
        }
        archive_read_data_skip(a);
    }

    // Entity Not Found
    archive_read_close(a);
    archive_read_free(a);
    jenova_log("[Jenova Resource Manager] Error : Entity Not Found '%s'", entityID.c_str());
    return result;
}