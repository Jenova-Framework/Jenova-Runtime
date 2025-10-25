
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

// Internal/Built-In Sources
#include "CodeTemplates.h"

// Archive SDK
#define LIBARCHIVE_STATIC
#include "Archive/archive.h"
#include "Archive/archive_entry.h"

// Macros
#define BUILT_IN_TEMPLATE 0

// Global Storage
Vector<Dictionary> globalTemplates;
Vector<Dictionary> classTemplates;

// Singleton Instance
JenovaTemplateManager* jnvtm_singleton = nullptr;

// Initializer/Deinitializer
void JenovaTemplateManager::init()
{
	// Register Class
	ClassDB::register_internal_class<JenovaTemplateManager>();

	// Initialize Singleton
	jnvtm_singleton = memnew(JenovaTemplateManager);

	// Verbose
	jenova::Output("Jenova Template Manager Initialized.");
}
void JenovaTemplateManager::deinit()
{
	// Release Singleton
	if (jnvtm_singleton) memdelete(jnvtm_singleton);
}

// Bindings
void JenovaTemplateManager::_bind_methods()
{
}

// Singleton Handling
JenovaTemplateManager* JenovaTemplateManager::get_singleton()
{
	return jnvtm_singleton;
}

// Jenova Script Template Manager Implementation
bool JenovaTemplateManager::RegisterNewGlobalScriptTemplate(const String& templateName, const String& templateCode, const String& templateDescription)
{
	Dictionary newTemplateData;
	newTemplateData["inherit"] = "";
	newTemplateData["name"] = templateName;
	newTemplateData["description"] = templateDescription;
	newTemplateData["content"] = templateCode;
	newTemplateData["id"] = globalTemplates.size();
	newTemplateData["origin"] = BUILT_IN_TEMPLATE;
	globalTemplates.push_back(newTemplateData);
	return true;
}
bool JenovaTemplateManager::RegisterNewClassScriptTemplate(const String& templateName, const String& className, const String& templateCode, const String& templateDescription)
{
	Dictionary newTemplateData;
	newTemplateData["inherit"] = className;
	newTemplateData["name"] = templateName;
	newTemplateData["description"] = templateDescription;
	newTemplateData["content"] = templateCode;
	newTemplateData["id"] = classTemplates.size();
	newTemplateData["origin"] = BUILT_IN_TEMPLATE;
	classTemplates.push_back(newTemplateData);
	return true;
}
TypedArray<Dictionary> JenovaTemplateManager::GetGlobalScriptTemplates()
{
	TypedArray<Dictionary> scriptTemplates;
	for (const auto& globalTemplate : globalTemplates) scriptTemplates.push_back(globalTemplate);
	return scriptTemplates;
}
String JenovaTemplateManager::GetGlobalScriptTemplateSource(const String& templateName)
{
	for (const auto& globalTemplate : globalTemplates) 
if (String(globalTemplate["name"]) == templateName) return globalTemplate["content"];
	return String("/* Global Template Not Found */");
}
TypedArray<Dictionary> JenovaTemplateManager::GetClassScriptTemplates(const String& className)
{
	TypedArray<Dictionary> scriptTemplates;
	for (const auto& classTemplate : classTemplates)
		if (String(classTemplate["inherit"]) == className) scriptTemplates.push_back(classTemplate);
	return scriptTemplates;
}
String JenovaTemplateManager::GetClassScriptTemplateSource(const String& templateName, const String& className)
{
	for (const auto& classTemplate : classTemplates)
		if (String(classTemplate["name"]) == templateName && String(classTemplate["inherit"]) == className) return classTemplate["content"];
	return String("/* Class Template Not Found */");
}
void JenovaTemplateManager::ClearScriptTemplates()
{
	globalTemplates.clear();
	classTemplates.clear();
}

// Jenova Script Template Manager Utilities Implementation
bool JenovaTemplateManager::InstallBuiltInScriptTemplates()
{
	// Create Templates Directory
	std::string templatesDirectory = AS_STD_STRING(ProjectSettings::get_singleton()->globalize_path(jenova::GlobalSettings::JenovaScriptTemplatesPath));
	if (!templatesDirectory.empty() && (templatesDirectory.back() == '\\' || templatesDirectory.back() == '/')) templatesDirectory.pop_back();

	// Validate Templates Directory Path
	if (!std::filesystem::exists(templatesDirectory))
	{
		// Create Destination If Not Exist
		if (!std::filesystem::create_directories(templatesDirectory))
		{
			jenova::Error("Jenova Template Installer", "Failed to Create Destination Directory.");
			return false;
		};

		// Setup Archive Extraction
		struct archive* a = archive_read_new();
		struct archive* ext = archive_write_disk_new();
		struct archive_entry* entry = nullptr;
		int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

		// Enable Support
		archive_read_support_format_all(a);
		archive_read_support_filter_all(a);
		archive_write_disk_set_options(ext, flags);
		archive_write_disk_set_standard_lookup(ext);

		// Open Archive From memory
		const void* buffer = jenova::templates::database::BUILTIN_TEMPLATES_ARCHIVE;
		size_t bufferSize = sizeof(jenova::templates::database::BUILTIN_TEMPLATES_ARCHIVE);

		if (archive_read_open_memory(a, buffer, bufferSize) != ARCHIVE_OK)
		{
			jenova::Error("Jenova Template Installer", "Failed to Open In-Memory Archive: %s", archive_error_string(a));
			archive_read_free(a);
			archive_write_free(ext);
			return false;
		}

		// Extraction Helper
		auto copyData = [&](struct archive* ar, struct archive* aw) -> int
			{
				const void* buff;
				size_t size;
				la_int64_t offset;
				int r;
				while (true)
				{
					r = archive_read_data_block(ar, &buff, &size, &offset);
					if (r == ARCHIVE_EOF) return ARCHIVE_OK;
					if (r < ARCHIVE_OK) return r;
					r = archive_write_data_block(aw, buff, size, offset);
					if (r < ARCHIVE_OK) return r;
				}
			};

		// Extract All Entries
		while (true)
		{
			int r = archive_read_next_header(a, &entry);
			if (r == ARCHIVE_EOF) break;
			if (r < ARCHIVE_OK) jenova::Error("Jenova Template Installer", "Extraction warning: %s", archive_error_string(a));
			if (r < ARCHIVE_WARN) break;
			std::string newPath = templatesDirectory + "/" + archive_entry_pathname(entry);
			archive_entry_set_pathname(entry, newPath.c_str());
			r = archive_write_header(ext, entry);
			if (r < ARCHIVE_OK) jenova::Error("Jenova Template Installer", "Header write warning: %s", archive_error_string(ext));
			else if (archive_entry_size(entry) > 0)
			{
				r = copyData(a, ext);
				if (r < ARCHIVE_OK)
				{
					jenova::Error("Jenova Template Installer", "Data Write Failed: %s", archive_error_string(ext));
					break;
				}
			}
			r = archive_write_finish_entry(ext);
			if (r < ARCHIVE_OK)
			{
				jenova::Error("Jenova Template Installer", "Finish Entry Failed: %s", archive_error_string(ext));
				break;
			}
		}

		// Cleanup
		archive_read_close(a);
		archive_read_free(a);
		archive_write_close(ext);
		archive_write_free(ext);

		// All Good
		return true;
	}

	// Already exists
	return true;
}
bool JenovaTemplateManager::UpdateScriptTemplates()
{
	// Create Templates Directory
	String templatesDirectory = ProjectSettings::get_singleton()->globalize_path(jenova::GlobalSettings::JenovaScriptTemplatesPath);

	// Validate Templates Directory Path
	if (!std::filesystem::exists(AS_STD_STRING(templatesDirectory)))
	{
		jenova::Warning("Jenova Template Loader", "No Script Templates Directory Detected.");
		return false;
	}

	// Clear Script Templates Database
	this->ClearScriptTemplates();

	// Collect Templates & Add Them to Template Manager
	try
	{
		// Collected Templates
		std::vector<std::string> templateFiles;

		// Collect Templates
		for (const auto& entry : std::filesystem::recursive_directory_iterator(AS_STD_STRING(templatesDirectory)))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".json") templateFiles.push_back(entry.path().string());
		}

		// Parse & Add Templates to Template Manager
		for (const auto& templateFile : templateFiles)
		{
			// Parse Template Metadata
			jenova::json_t templateMetadata = jenova::json_t::parse(jenova::ReadStdStringFromFile(templateFile));

			// Get Template Source Code File
			std::string templateSourceFile = std::filesystem::path(templateFile).replace_extension(".cpp").string();
			if (!std::filesystem::exists(templateSourceFile))
			{
				jenova::Warning("Jenova Template Loader", "Template File (%s) Missing.", templateSourceFile.c_str());
				return false;
			}

			// Get Template Category
			std::string templateCategory = templateMetadata["Category"].get<std::string>();

			// Process Templates Based On Category
			if (templateCategory == "Global")
			{
				// Get Template Properties
				std::string backend = templateMetadata["Backend"].get<std::string>();
				std::string name = templateMetadata["Name"].get<std::string>();
				std::string description = templateMetadata["Description"].get<std::string>();
				std::string author = templateMetadata["Author"].get<std::string>();
				float version = templateMetadata["Version"].get<float>();

				// Skip If Backend Mismatch
				if (backend != "Universal")
				{
					jenova::InterpreterBackend requiredBackend = jenova::InterpreterBackend::Unknown;
					if (backend == "NitroJIT") requiredBackend = jenova::InterpreterBackend::AsmJIT;
					if (backend == "Meteora") requiredBackend = jenova::InterpreterBackend::TinyCC;
					if (JenovaInterpreter::GetInterpreterBackend() != requiredBackend) continue;
				}

				// Add Global Script Template
				this->RegisterNewGlobalScriptTemplate(AS_GD_STRING(name), jenova::ReadStringFromFile(AS_GD_STRING(templateSourceFile)), AS_GD_STRING(description));
			}
			if (templateCategory == "Inherited")
			{
				// Get Template Properties
				std::string backend = templateMetadata["Backend"].get<std::string>();
				std::string inherit = templateMetadata["Inherit"].get<std::string>();
				std::string name = templateMetadata["Name"].get<std::string>();
				std::string description = templateMetadata["Description"].get<std::string>();
				std::string author = templateMetadata["Author"].get<std::string>();
				float version = templateMetadata["Version"].get<float>();

				// Skip If Backend Mismatch
				if (backend != "Universal")
				{
					jenova::InterpreterBackend requiredBackend = jenova::InterpreterBackend::Unknown;
					if (backend == "NitroJIT") requiredBackend = jenova::InterpreterBackend::AsmJIT;
					if (backend == "Meteora") requiredBackend = jenova::InterpreterBackend::TinyCC;
					if (JenovaInterpreter::GetInterpreterBackend() != requiredBackend) continue;
				}

				// Add Class Script Template
				this->RegisterNewClassScriptTemplate(AS_GD_STRING(name), AS_GD_STRING(inherit), jenova::ReadStringFromFile(AS_GD_STRING(templateSourceFile)), AS_GD_STRING(description));
			}
		}
	}
	catch (const std::exception&)
	{
		jenova::Warning("Jenova Template Loader", "An Error Occurred while Adding Script Templates.");
		return false;
	}

	// All Good
	return true;
}