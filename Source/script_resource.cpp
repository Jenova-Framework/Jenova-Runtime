
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

// References
static Ref<CPPScriptResourceLoader>		cpp_loader;
static Ref<CPPScriptResourceSaver>		cpp_saver;

// Script Resource Loader Implementation
void CPPScriptResourceLoader::init()
{
	cpp_loader.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(cpp_loader);
}
void CPPScriptResourceLoader::deinit()
{
	ResourceLoader::get_singleton()->remove_resource_format_loader(cpp_loader);
	cpp_loader.unref();
}
Variant CPPScriptResourceLoader::_load(const String& p_path, const String& original_path, bool use_sub_threads, int32_t cache_mode) const
{
	Ref<CPPScript> cppScript = memnew(CPPScript);
	cppScript->_set_source_code(FileAccess::get_file_as_string(p_path));

	// Verbose
	jenova::VerboseByID(__LINE__, "C++ Script (%s) Loaded.", AS_C_STRING(p_path));

	return cppScript;
}
PackedStringArray CPPScriptResourceLoader::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.push_back(jenova::GlobalSettings::JenovaScriptExtension);
	return array;
}
bool CPPScriptResourceLoader::_handles_type(const StringName& type) const
{
	String type_str = type;
	return type_str == jenova::GlobalSettings::JenovaScriptType || type_str == "Script";
}
String CPPScriptResourceLoader::_get_resource_type(const String& p_path) const
{
	String el = p_path.get_extension().to_lower();
	if (el == jenova::GlobalSettings::JenovaScriptExtension) return jenova::GlobalSettings::JenovaScriptType;
	return "";
}

// Script Resource Saver Implementation
void CPPScriptResourceSaver::init()
{
	cpp_saver.instantiate();
	ResourceSaver::get_singleton()->add_resource_format_saver(cpp_saver);
}
void CPPScriptResourceSaver::deinit()
{
	ResourceSaver::get_singleton()->remove_resource_format_saver(cpp_saver);
	cpp_saver.unref();
}
Error CPPScriptResourceSaver::_save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags)
{
	jenova::VerboseByID(__LINE__, "Saving C++ Script (%s)...", AS_C_STRING(p_path));

	CPPScript* script = Object::cast_to<CPPScript>(p_resource.ptr());
	if (script != nullptr)
	{
		Ref<FileAccess> handle = FileAccess::open(p_path, FileAccess::ModeFlags::WRITE);
		if (handle.is_valid())
		{
			handle->store_string(script->_get_source_code());
			handle->close();

			// Verbose
			jenova::VerboseByID(__LINE__, "C++ Script (%s) Saved.", AS_C_STRING(p_path));

			return Error::OK;
		}
		else
		{
			return Error::ERR_FILE_CANT_OPEN;
		}
	}
	return Error::ERR_SCRIPT_FAILED;
}
Error CPPScriptResourceSaver::_set_uid(const String& p_path, int64_t p_uid)
{
	return Error::OK;
}
bool CPPScriptResourceSaver::_recognize(const Ref<Resource>& p_resource) const
{
	return Object::cast_to<CPPScript>(p_resource.ptr()) != nullptr;
}
PackedStringArray CPPScriptResourceSaver::_get_recognized_extensions(const Ref<Resource>& p_resource) const
{
	PackedStringArray array;
	if (Object::cast_to<CPPScript>(p_resource.ptr()) != nullptr) array.push_back(jenova::GlobalSettings::JenovaScriptExtension);
	return array;
}
bool CPPScriptResourceSaver::_recognize_path(const Ref<Resource>& p_resource, const String& p_path) const
{
	return Object::cast_to<CPPScript>(p_resource.ptr()) != nullptr;
}

// References
static Ref<CPPHeaderResourceLoader>		h_loader;
static Ref<CPPHeaderResourceSaver>		h_saver;

// Header Resource Loader Implementation
void CPPHeaderResourceLoader::init()
{
	h_loader.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(h_loader);
}
void CPPHeaderResourceLoader::deinit()
{
	ResourceLoader::get_singleton()->remove_resource_format_loader(h_loader);
	h_loader.unref();
}
Variant CPPHeaderResourceLoader::_load(const String& p_path, const String& original_path, bool use_sub_threads, int32_t cache_mode) const
{
	Ref<CPPHeader> cppHeader = memnew(CPPHeader);
	cppHeader->_set_source_code(FileAccess::get_file_as_string(p_path));
	return cppHeader;
}
PackedStringArray CPPHeaderResourceLoader::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.push_back("h");
	array.push_back("hh");
	array.push_back("hpp");
	return array;
}
bool CPPHeaderResourceLoader::_handles_type(const StringName& type) const
{
	String type_str = type;
	return type_str == jenova::GlobalSettings::JenovaHeaderType || type_str == "Script";
}
String CPPHeaderResourceLoader::_get_resource_type(const String& p_path) const
{
	String el = p_path.get_extension().to_lower();
	if (el == "h" || el == "hh" || el == "hpp") return jenova::GlobalSettings::JenovaHeaderType;
	return "";
}

// Header Resource Saver Implementation
void CPPHeaderResourceSaver::init()
{
	h_saver.instantiate();
	ResourceSaver::get_singleton()->add_resource_format_saver(h_saver);
}
void CPPHeaderResourceSaver::deinit()
{
	ResourceSaver::get_singleton()->remove_resource_format_saver(h_saver);
	h_saver.unref();
}
Error CPPHeaderResourceSaver::_save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags)
{
	CPPHeader* header = Object::cast_to<CPPHeader>(p_resource.ptr());
	if (header != nullptr)
	{
		Ref<FileAccess> handle = FileAccess::open(p_path, FileAccess::ModeFlags::WRITE);
		if (handle.is_valid())
		{
			handle->store_string(header->_get_source_code());
			handle->close();
			return Error::OK;
		}
		else
		{
			return Error::ERR_FILE_CANT_OPEN;
		}
	}
	return Error::ERR_SCRIPT_FAILED;
}
Error CPPHeaderResourceSaver::_set_uid(const String& p_path, int64_t p_uid)
{
	return Error::OK;
}
bool CPPHeaderResourceSaver::_recognize(const Ref<Resource>& p_resource) const
{
	return Object::cast_to<CPPHeader>(p_resource.ptr()) != nullptr;
}
PackedStringArray CPPHeaderResourceSaver::_get_recognized_extensions(const Ref<Resource>& p_resource) const
{
	PackedStringArray array;
	if (Object::cast_to<CPPHeader>(p_resource.ptr()) != nullptr) array.push_back("h");
	if (Object::cast_to<CPPHeader>(p_resource.ptr()) != nullptr) array.push_back("hh");
	if (Object::cast_to<CPPHeader>(p_resource.ptr()) != nullptr) array.push_back("hpp");
	return array;
}
bool CPPHeaderResourceSaver::_recognize_path(const Ref<Resource>& p_resource, const String& p_path) const
{
	return Object::cast_to<CPPHeader>(p_resource.ptr()) != nullptr;
}