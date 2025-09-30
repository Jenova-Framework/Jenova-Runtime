
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

// Jenova Script Language Implementation
static CPPScriptLanguage* cpp_language;
CPPScriptLanguage* CPPScriptLanguage::get_singleton()
{
	return cpp_language;
}
void CPPScriptLanguage::init()
{
	cpp_language = memnew(CPPScriptLanguage);
	Engine::get_singleton()->register_script_language(cpp_language);
}
void CPPScriptLanguage::deinit()
{
	Engine::get_singleton()->unregister_script_language(cpp_language);
	memdelete(cpp_language);
}
String CPPScriptLanguage::_get_name() const
{
	return "C++ Script";
}
void CPPScriptLanguage::_init() {}
void CPPScriptLanguage::_finish() {}
String CPPScriptLanguage::_get_type() const
{
	return jenova::GlobalSettings::JenovaScriptType;
}
String CPPScriptLanguage::_get_extension() const
{
	return jenova::GlobalSettings::JenovaScriptExtension;
}
PackedStringArray CPPScriptLanguage::_get_reserved_words() const
{
	static const PackedStringArray reserved_words
	{
		"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept",
		"auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
		"class", "compl", "concept", "const", "consteval", "constexpr", "const_cast", "continue", "co_await",
		"co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else",
		"enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline",
		"int", "int32_t", "int64_t", "uint32_t", "uint64_t", "long", "size_t", "mutable", "namespace", "new",
		"noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
		"reflexpr", "register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static",
		"static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local",
		"throw", "true", "try", "typedef", "typeid","typename", "union", "unsigned", "using", "virtual", "void",
		"volatile", "wchar_t", "while", "xor", "xor_eq", "jenova", "godot", "__int64", "__int32", "Caller", "Variant",
		"include", "ifdef", "else", "endif", "pragma",
		"GlobalGet", "GlobalSet", "GlobalPointer", "GlobalVariable", "GetNode", "FindNode", "GetSelf",
		jenova::GlobalSettings::ScriptToolIdentifier,
		jenova::GlobalSettings::ScriptBlockBeginIdentifier,
		jenova::GlobalSettings::ScriptBlockEndIdentifier,
		jenova::GlobalSettings::ScriptVMBeginIdentifier,
		jenova::GlobalSettings::ScriptVMEndIdentifier,
		jenova::GlobalSettings::ScriptSignalCallbackIdentifier,
		jenova::GlobalSettings::ScriptPropertyIdentifier,
		jenova::GlobalSettings::ScriptClassNameIdentifier,
		jenova::GlobalSettings::ScriptActivatorIdentifier,
		jenova::GlobalSettings::ScriptFunctionExportIdentifier
	};
	return reserved_words;
}
bool CPPScriptLanguage::_is_control_flow_keyword(const String& p_keyword) const
{
	static const std::unordered_set<std::string> control_flow_keywords{
		"if", "else", "switch", "case", "default", "while", "do", "for", "break", "continue", "return", "goto", "try", "catch", "throw", "co_await",
		"co_return", "co_yield", "jenova", "godot", "include", "ifdef", "else", "endif", "pragma",
		jenova::GlobalSettings::ScriptToolIdentifier,
		jenova::GlobalSettings::ScriptBlockBeginIdentifier,
		jenova::GlobalSettings::ScriptBlockEndIdentifier,
		jenova::GlobalSettings::ScriptVMBeginIdentifier,
		jenova::GlobalSettings::ScriptVMEndIdentifier,
		jenova::GlobalSettings::ScriptSignalCallbackIdentifier,
		jenova::GlobalSettings::ScriptPropertyIdentifier,
		jenova::GlobalSettings::ScriptClassNameIdentifier,
		jenova::GlobalSettings::ScriptActivatorIdentifier,
		jenova::GlobalSettings::ScriptFunctionExportIdentifier
	};
	return control_flow_keywords.find(p_keyword.utf8().get_data()) != control_flow_keywords.end();
}
PackedStringArray CPPScriptLanguage::_get_comment_delimiters() const
{
	PackedStringArray comment_delimiters;
	comment_delimiters.push_back("/* */");
	comment_delimiters.push_back("//");
	return comment_delimiters;
}
PackedStringArray CPPScriptLanguage::_get_doc_comment_delimiters() const
{
	PackedStringArray doc_comment_delimiters;
	doc_comment_delimiters.push_back("///");
	doc_comment_delimiters.push_back("/** */");
	return doc_comment_delimiters;
}
PackedStringArray CPPScriptLanguage::_get_string_delimiters() const
{
	PackedStringArray string_delimiters;
	string_delimiters.push_back("' '");
	string_delimiters.push_back("\" \"");
	return string_delimiters;
}
Ref<Script> CPPScriptLanguage::_make_template(const String& p_template, const String& p_class_name, const String& p_base_class_name) const
{
	// Create New C++ Script
	Ref<CPPScript> script;
	script.instantiate();

	// Set Template
	if (!p_template.is_empty())
	{
		// Create A Copy from Template
		String templateCode = p_template;

		// Process Template Macros
		templateCode = templateCode.replace("$BASE_TYPE$", p_base_class_name);
		templateCode = templateCode.replace("$BASE_TYPE_HEADER$", jenova::GenerateHeaderNameFromClassName(p_base_class_name));
		templateCode = templateCode.replace("$BASE_CLASS_NAME$", jenova::GenerateClassNameFromBaseName(p_class_name));

		// Set Processed Template
		script->set_source_code(templateCode);
	}

	// Return Created Script
	return script;
}
TypedArray<Dictionary> CPPScriptLanguage::_get_built_in_templates(const StringName& p_object) const
{
	// Global Templates [Future : Maybe Merge to Object Class Templates?]
	if (p_object == StringName("Object")) return JenovaTemplateManager::get_singleton()->GetGlobalScriptTemplates();

	// Class Templates
	return JenovaTemplateManager::get_singleton()->GetClassScriptTemplates(p_object);

	// No Template Found
	return TypedArray<Dictionary>();
}
bool CPPScriptLanguage::_is_using_templates()
{
	return true;
}
Dictionary CPPScriptLanguage::_validate(const String& p_script, const String& p_path, bool p_validate_functions, bool p_validate_errors, bool p_validate_warnings, bool p_validate_safe_lines) const
{
	return Dictionary();
}
String CPPScriptLanguage::_validate_path(const String& p_path) const
{
	return String();
}
Object* CPPScriptLanguage::_create_script() const
{
	CPPScript* script = memnew(CPPScript);
	return script;
}
bool CPPScriptLanguage::_supports_builtin_mode() const
{
	return true; // Experimental
}
bool CPPScriptLanguage::_supports_documentation() const
{
	return true;
}
bool CPPScriptLanguage::_can_inherit_from_file() const
{
	return false;
}
int32_t CPPScriptLanguage::_find_function(const String& p_function, const String& p_code) const
{
	return -1;
}
String CPPScriptLanguage::_make_function(const String& p_class_name, const String& p_function_name, const PackedStringArray& p_function_args) const
{
	return String();
}
Error CPPScriptLanguage::_open_in_external_editor(const Ref<Script>& p_script, int32_t p_line, int32_t p_column)
{
	return Error::OK;
}
bool CPPScriptLanguage::_overrides_external_editor()
{
	return false;
}
Dictionary CPPScriptLanguage::_complete_code(const String& p_code, const String& p_path, Object* p_owner) const
{
	Dictionary completeCodeResult;
	completeCodeResult["result"] = Variant(false);
	completeCodeResult["force"] = Variant(false);
	completeCodeResult["call_hint"] = Variant(false);
	return completeCodeResult;
}
Dictionary CPPScriptLanguage::_lookup_code(const String& p_code, const String& p_symbol, const String& p_path, Object* p_owner) const
{
	Dictionary lookUpResult;
	lookUpResult["result"] = Variant(false);
	lookUpResult["type"] = GDEXTENSION_VARIANT_TYPE_NIL;
	return lookUpResult;
}
String CPPScriptLanguage::_auto_indent_code(const String& p_code, int32_t p_from_line, int32_t p_to_line) const
{
	return String();
}
bool CPPScriptLanguage::_can_make_function() const
{
	return true;
}
void CPPScriptLanguage::_add_global_constant(const StringName& p_name, const Variant& p_value) {}
void CPPScriptLanguage::_add_named_global_constant(const StringName& p_name, const Variant& p_value) {}
void CPPScriptLanguage::_remove_named_global_constant(const StringName& p_name) {}
void CPPScriptLanguage::_thread_enter() {}
void CPPScriptLanguage::_thread_exit() {}
String CPPScriptLanguage::_debug_get_error() const
{
	// jenova::Output("_debug_get_error");
	return String();
}
int32_t CPPScriptLanguage::_debug_get_stack_level_count() const
{
	// jenova::Output("_debug_get_stack_level_count");
	return 0;
}
int32_t CPPScriptLanguage::_debug_get_stack_level_line(int32_t p_level) const
{
	// jenova::Output("_debug_get_stack_level_line %d", p_level);
	return 0;
}
String CPPScriptLanguage::_debug_get_stack_level_function(int32_t p_level) const
{
	// jenova::Output("_debug_get_stack_level_function %d", p_level);
	return String();
}
Dictionary CPPScriptLanguage::_debug_get_stack_level_locals(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth)
{
	// jenova::Output("_debug_get_stack_level_locals %d %d %d", p_level, p_max_subitems, p_max_depth);
	return Dictionary();
}
Dictionary CPPScriptLanguage::_debug_get_stack_level_members(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth)
{
	// jenova::Output("_debug_get_stack_level_members %d %d %d", p_level, p_max_subitems, p_max_depth);
	return Dictionary();
}
void* CPPScriptLanguage::_debug_get_stack_level_instance(int32_t p_level)
{
	// jenova::Output("_debug_get_stack_level_instance %d", p_level);
	return nullptr;
}
Dictionary CPPScriptLanguage::_debug_get_globals(int32_t p_max_subitems, int32_t p_max_depth)
{
	// jenova::Output("_debug_get_globals %d", p_max_subitems, p_max_depth);
	return Dictionary();
}
String CPPScriptLanguage::_debug_parse_stack_level_expression(int32_t p_level, const String& p_expression, int32_t p_max_subitems, int32_t p_max_depth)
{
	// jenova::Output("_debug_parse_stack_level_expression %d %s %d %d", p_level, p_max_depth, AS_C_STRING(p_expression), p_max_subitems, p_max_depth);
	return String();
}
TypedArray<Dictionary> CPPScriptLanguage::_debug_get_current_stack_info()
{
	// jenova::Output("_debug_get_current_stack_info");
	return TypedArray<Dictionary>();
}
void CPPScriptLanguage::_reload_all_scripts()
{
	jenova::VerboseByID(__LINE__, "_reload_all_scripts Called");
}
void CPPScriptLanguage::_reload_scripts(const Array& p_scripts, bool p_soft_reload)
{
	// Called when A Script gets Edited While Game is Running In Debug Mode
	jenova::VerboseByID(__LINE__, "_reload_scripts Called");
}
void CPPScriptLanguage::_reload_tool_script(const Ref<Script>& p_script, bool p_soft_reload)
{
	jenova::VerboseByID(__LINE__, "_reload_tool_script Called");
}
PackedStringArray CPPScriptLanguage::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.push_back(jenova::GlobalSettings::JenovaScriptExtension);
	return array;
}
TypedArray<Dictionary> CPPScriptLanguage::_get_public_functions() const
{
	return TypedArray<Dictionary>();
}
Dictionary CPPScriptLanguage::_get_public_constants() const
{
	return Dictionary();
}
TypedArray<Dictionary> CPPScriptLanguage::_get_public_annotations() const
{
	return TypedArray<Dictionary>();
}
void CPPScriptLanguage::_profiling_start()
{
	JenovaProfiler::StartRecording();
}
void CPPScriptLanguage::_profiling_stop()
{
	JenovaProfiler::StopRecording();
}
void CPPScriptLanguage::_profiling_set_save_native_calls(bool p_enable)
{
	jenova::VerboseByID(__LINE__, "_profiling_set_save_native_calls : %s", p_enable ? "True" : "False");
}
int32_t CPPScriptLanguage::_profiling_get_accumulated_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max)
{
	return 0;
}
int32_t CPPScriptLanguage::_profiling_get_frame_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max)
{
	return 0;
}
void CPPScriptLanguage::_frame()
{
	JenovaProfiler::Frame();
}
bool CPPScriptLanguage::_handles_global_class_type(const String& p_type) const
{
	jenova::VerboseByID(__LINE__, "_handles_global_class_type [%s]", AS_C_STRING(p_type));
	return p_type == _get_type();
}
Dictionary CPPScriptLanguage::_get_global_class_name(const String& p_path) const
{
	// Remove
	jenova::VerboseByID(__LINE__, "_get_global_class_name [%s]", AS_C_STRING(p_path));

	// Return Global Class Name
	Dictionary classInfo;
	Ref<Resource> resource = ResourceLoader::get_singleton()->load(p_path);
	Ref<CPPScript> cppScript = Object::cast_to<CPPScript>(resource.ptr());
	if (cppScript.is_valid())
	{
		// Check for User-Defined Class Name
		if (cppScript->has_source_code())
		{
			std::string className = jenova::ParseClassNameFromScriptSource(AS_STD_STRING(cppScript->get_source_code()));
			if (!className.empty()) classInfo["name"] = AS_GD_STRING(className);
			else classInfo["name"] = "NotFound";
		}

		// Create Class Info
		if (classInfo["name"] == String("NotFound")) classInfo["name"] = " Jenova C++ Script";
		classInfo["base_type"] = jenova::GlobalSettings::JenovaScriptType;
		String iconPath = p_path.replace(p_path.get_extension(), "svg");
		if (FileAccess::file_exists(iconPath)) classInfo["icon_path"] = iconPath;

		// Create Global Class If Not Exists
		ProjectSettings::get_singleton()->get_global_class_list();
	}
	return classInfo;
}

// Jenova Script Language Implementation
static CPPHeaderLanguage* header_language;
CPPHeaderLanguage* CPPHeaderLanguage::get_singleton()
{
	return header_language;
}
void CPPHeaderLanguage::init()
{
	header_language = memnew(CPPHeaderLanguage);
	Engine::get_singleton()->register_script_language(header_language);
}
void CPPHeaderLanguage::deinit()
{
	Engine::get_singleton()->unregister_script_language(header_language);
	memdelete(header_language);
}
String CPPHeaderLanguage::_get_name() const
{
	return "C++ Header";
}
void CPPHeaderLanguage::_init() {}
void CPPHeaderLanguage::_finish() {}
String CPPHeaderLanguage::_get_type() const
{
	return jenova::GlobalSettings::JenovaHeaderType;
}
String CPPHeaderLanguage::_get_extension() const
{
	return String("hpp"); // How to add .h and .hh?
}
PackedStringArray CPPHeaderLanguage::_get_reserved_words() const
{
	return CPPScriptLanguage::get_singleton()->_get_reserved_words();
}
bool CPPHeaderLanguage::_is_control_flow_keyword(const String& p_keyword) const
{
	return CPPScriptLanguage::get_singleton()->_is_control_flow_keyword(p_keyword);
}
PackedStringArray CPPHeaderLanguage::_get_comment_delimiters() const
{
	return CPPScriptLanguage::get_singleton()->_get_comment_delimiters();
}
PackedStringArray CPPHeaderLanguage::_get_doc_comment_delimiters() const
{
	return CPPScriptLanguage::get_singleton()->_get_doc_comment_delimiters();
}
PackedStringArray CPPHeaderLanguage::_get_string_delimiters() const
{
	return CPPScriptLanguage::get_singleton()->_get_string_delimiters();
}
Ref<Script> CPPHeaderLanguage::_make_template(const String& p_template, const String& p_class_name, const String& p_base_class_name) const
{
	CPPHeader* script = memnew(CPPHeader);
	return Ref<Script>(script);
}
TypedArray<Dictionary> CPPHeaderLanguage::_get_built_in_templates(const StringName& p_object) const
{
	return TypedArray<Dictionary>();
}
bool CPPHeaderLanguage::_is_using_templates()
{
	return false;
}
Dictionary CPPHeaderLanguage::_validate(const String& p_script, const String& p_path, bool p_validate_functions, bool p_validate_errors, bool p_validate_warnings, bool p_validate_safe_lines) const
{
	return Dictionary();
}
String CPPHeaderLanguage::_validate_path(const String& p_path) const
{
	return String();
}
Object* CPPHeaderLanguage::_create_script() const
{
	CPPHeader* script = memnew(CPPHeader);
	return script;
}
bool CPPHeaderLanguage::_supports_builtin_mode() const
{
	return false;
}
bool CPPHeaderLanguage::_supports_documentation() const
{
	return false;
}
bool CPPHeaderLanguage::_can_inherit_from_file() const
{
	return false;
}
int32_t CPPHeaderLanguage::_find_function(const String& p_function, const String& p_code) const
{
	return -1;
}
String CPPHeaderLanguage::_make_function(const String& p_class_name, const String& p_function_name, const PackedStringArray& p_function_args) const
{
	return String();
}
Error CPPHeaderLanguage::_open_in_external_editor(const Ref<Script>& p_script, int32_t p_line, int32_t p_column)
{
	return Error::OK;
}
bool CPPHeaderLanguage::_overrides_external_editor()
{
	return false;
}
Dictionary CPPHeaderLanguage::_complete_code(const String& p_code, const String& p_path, Object* p_owner) const
{
	return Dictionary();
}
Dictionary CPPHeaderLanguage::_lookup_code(const String& p_code, const String& p_symbol, const String& p_path, Object* p_owner) const
{
	return Dictionary();
}
String CPPHeaderLanguage::_auto_indent_code(const String& p_code, int32_t p_from_line, int32_t p_to_line) const
{
	return String();
}
bool CPPHeaderLanguage::_can_make_function() const
{
	return false;
}
void CPPHeaderLanguage::_add_global_constant(const StringName& p_name, const Variant& p_value) {}
void CPPHeaderLanguage::_add_named_global_constant(const StringName& p_name, const Variant& p_value) {}
void CPPHeaderLanguage::_remove_named_global_constant(const StringName& p_name) {}
void CPPHeaderLanguage::_thread_enter() {}
void CPPHeaderLanguage::_thread_exit() {}
String CPPHeaderLanguage::_debug_get_error() const
{
	return String();
}
int32_t CPPHeaderLanguage::_debug_get_stack_level_count() const
{
	return 0;
}
int32_t CPPHeaderLanguage::_debug_get_stack_level_line(int32_t p_level) const
{
	return 0;
}
String CPPHeaderLanguage::_debug_get_stack_level_function(int32_t p_level) const
{
	return String();
}
Dictionary CPPHeaderLanguage::_debug_get_stack_level_locals(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth)
{
	return Dictionary();
}
Dictionary CPPHeaderLanguage::_debug_get_stack_level_members(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth)
{
	return Dictionary();
}
void* CPPHeaderLanguage::_debug_get_stack_level_instance(int32_t p_level)
{
	return nullptr;
}
Dictionary CPPHeaderLanguage::_debug_get_globals(int32_t p_max_subitems, int32_t p_max_depth)
{
	return Dictionary();
}
String CPPHeaderLanguage::_debug_parse_stack_level_expression(int32_t p_level, const String& p_expression, int32_t p_max_subitems, int32_t p_max_depth)
{
	return String();
}
TypedArray<Dictionary> CPPHeaderLanguage::_debug_get_current_stack_info()
{
	return TypedArray<Dictionary>();
}
void CPPHeaderLanguage::_reload_all_scripts() {}
void CPPHeaderLanguage::_reload_scripts(const Array& p_scripts, bool p_soft_reload) {}
void CPPHeaderLanguage::_reload_tool_script(const Ref<Script>& p_script, bool p_soft_reload) {}
PackedStringArray CPPHeaderLanguage::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.push_back("h");
	array.push_back("hh");
	array.push_back("hpp");
	return array;
}
TypedArray<Dictionary> CPPHeaderLanguage::_get_public_functions() const
{
	return TypedArray<Dictionary>();
}
Dictionary CPPHeaderLanguage::_get_public_constants() const
{
	return Dictionary();
}
TypedArray<Dictionary> CPPHeaderLanguage::_get_public_annotations() const
{
	return TypedArray<Dictionary>();
}
void CPPHeaderLanguage::_profiling_start() {}
void CPPHeaderLanguage::_profiling_stop() {}
void CPPHeaderLanguage::_profiling_set_save_native_calls(bool p_enable) {}
int32_t CPPHeaderLanguage::_profiling_get_accumulated_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max)
{
	return 0;
}
int32_t CPPHeaderLanguage::_profiling_get_frame_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max)
{
	return 0;
}
void CPPHeaderLanguage::_frame() {}
bool CPPHeaderLanguage::_handles_global_class_type(const String& p_type) const
{
	return false;
}
Dictionary CPPHeaderLanguage::_get_global_class_name(const String& p_path) const
{
	return Dictionary();
}

// Deprecated
#ifndef LITHIUM_EDITION
bool CPPScriptLanguage::_has_named_classes() const
{
	return false; // Not Supprted Yet
}
bool CPPHeaderLanguage::_has_named_classes() const
{
	return false;
}
#endif