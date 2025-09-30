
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

// Code Templates
#include "CodeTemplates.h"

// Shared Values/Objects
static std::chrono::steady_clock::time_point lastScriptReloadTime = std::chrono::steady_clock::now();

// CPPScript Object Implementation
void CPPScript::_bind_methods()
{
}
bool CPPScript::_editor_can_reload_from_file()
{
	// Needs Investigation
	// It Seems This is for Tracking Changes to Files When They Are Open?
	return true;
}
void CPPScript::_placeholder_erased(void* p_placeholder)
{
}
bool CPPScript::_can_instantiate() const
{
	return true;
}
Ref<Script> CPPScript::_get_base_script() const
{
	return Ref<Script>();
}
StringName CPPScript::_get_global_name()
{
	return StringName("JenovaScript_" + GetScriptIdentity());
}
bool CPPScript::_inherits_script(const Ref<Script>& p_script) const
{
	return false;
}
StringName CPPScript::_get_instance_base_type() const
{
	return StringName(jenova::GlobalSettings::JenovaScriptType);
}
void* CPPScript::_instance_create(Object* p_for_object) const
{
	godot::Node* parentNode = godot::Object::cast_to<godot::Node>(p_for_object);
	jenova::VerboseByID(__LINE__, "Adding C++ Script (%s) Instance to (%s)", AS_C_STRING(this->get_path()), AS_C_STRING(parentNode->get_name()));
	CPPScriptInstance* instance = memnew(CPPScriptInstance(p_for_object, Ref<CPPScript>(this)));
	return CPPScriptInstance::create_native_instance(instance);
}
void* CPPScript::_placeholder_instance_create(Object* p_for_object) const
{
	return nullptr; // Not Supported Yet
}
bool CPPScript::_instance_has(Object* p_object) const
{
	return false;
}
bool CPPScript::_has_source_code() const
{
	return !source_code.is_empty();
}
String CPPScript::_get_source_code() const
{
	if (!this->get_path().is_empty()) jenova::VerboseByID(__LINE__, "Get C++ Script Source (%s) [%p]", AS_C_STRING(this->get_path()), this);
	return source_code;
}
void CPPScript::_set_source_code(const String& p_code)
{
	if (!this->get_path().is_empty()) jenova::VerboseByID(__LINE__, "Set C++ Script Source (%s) [%p]", AS_C_STRING(this->get_path()), this);
	source_code = p_code;

	// Detect Tool Macro
	String cleanedSource = jenova::RemoveCommentsFromSource(p_code);
	this->IsTool = jenova::ContainsExactString(cleanedSource, jenova::GlobalSettings::ScriptToolIdentifier);
}
Error CPPScript::_reload(bool p_keep_state)
{
	jenova::VerboseByID(__LINE__, "Reloading C++ Script (%s)", AS_C_STRING(this->get_path()));
	if (!jenova::UpdateGlobalStorageFromEditorSettings()) return Error::FAILED;
	if (jenova::GlobalStorage::CurrentChangesTriggerMode == jenova::ChangesTriggerMode::TriggerOnScriptReload)
	{
		scriptMutex->lock();
		auto now = std::chrono::steady_clock::now();
		if (now - lastScriptReloadTime >= std::chrono::milliseconds(jenova::GlobalSettings::ScriptReloadCooldown))
		{
			lastScriptReloadTime = now;
			if (jenova::QueueProjectBuild())
			{
				scriptMutex->unlock();
				return Error::OK;
			}
		}
		scriptMutex->unlock();
	}
	return Error::ERR_UNAVAILABLE;
}
StringName CPPScript::_get_doc_class_name() const
{
	/* Needs to be Implemented */
	return "CPPScript";
}
TypedArray<Dictionary> CPPScript::_get_documentation() const
{
	/* 
		Due to the bad mechanisim for Script Documentation in Godot
		This function is just used to trigger Documentation Reload,
		Instead of parsing sources we use a XML beside each Script.
	*/
	jenova::UpdateScriptsDocumentation();
	return TypedArray<Dictionary>();
}
String CPPScript::_get_class_icon_path() const
{
	String iconPath = get_path().replace(get_path().get_extension(), "svg");
	if (FileAccess::file_exists(iconPath)) return iconPath;
	return String(); // No Icon Provided
}
bool CPPScript::_has_method(const StringName& p_method) const
{
	std::string scriptUID = AS_STD_STRING(GetScriptIdentity());
	jenova::FunctionList jenovaMethods = JenovaInterpreter::GetFunctionsList(scriptUID);
	for (const auto& method : jenovaMethods)
	{
		if (p_method == StringName(method.c_str())) return true;
	}
	return false;
}
bool CPPScript::_has_static_method(const StringName& p_method) const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_has_static_method (%s)", AS_C_STRING(p_method));
	return false; // Not Supported Yet
}
Dictionary CPPScript::_get_method_info(const StringName& p_method) const
{
	TypedArray<Dictionary> methodsList;
	std::string scriptUID = AS_STD_STRING(GetScriptIdentity());
	auto functionContainer = JenovaInterpreter::GetFunctionContainer(scriptUID);
	for (const auto& function : functionContainer.scriptFunctions)
	{
		if (function.functionName == p_method) return function.methodInfo;
	}
	return Dictionary();
}
bool CPPScript::_is_tool() const
{
	return IsTool;
}
bool CPPScript::_is_valid() const
{
	// Ask From Interpreter
	return true;
}
bool CPPScript::_is_abstract() const
{
	return false;
}
ScriptLanguage* CPPScript::_get_language() const
{
	return CPPScriptLanguage::get_singleton();
}
bool CPPScript::_has_script_signal(const StringName& p_signal) const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_has_script_signal (%s)", AS_C_STRING(p_signal));
	return false; // Not Supported Yet
}
TypedArray<Dictionary> CPPScript::_get_script_signal_list() const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_get_script_signal_list");
	return TypedArray<Dictionary>();
}
bool CPPScript::_has_property_default_value(const StringName& p_property) const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_has_property_default_value (%s)", AS_C_STRING(p_property));
	// This will cause property change flag on assigned node!
	return false;
}
Variant CPPScript::_get_property_default_value(const StringName& p_property) const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_get_property_default_value (%s)", AS_C_STRING(p_property));
	return Variant();  // Not Supported Yet
}
void CPPScript::_update_exports()
{
	jenova::VerboseByID(__LINE__, "CPPScript::_update_exports");
}
TypedArray<Dictionary> CPPScript::_get_script_method_list() const
{
	TypedArray<Dictionary> methodsList;
	std::string scriptUID = AS_STD_STRING(GetScriptIdentity());
	auto functionContainer = JenovaInterpreter::GetFunctionContainer(scriptUID);
	for (const auto& function : functionContainer.scriptFunctions) methodsList.push_back(Dictionary(function.methodInfo));
	return methodsList;
}
TypedArray<Dictionary> CPPScript::_get_script_property_list() const
{
	TypedArray<Dictionary> propertyList;
	std::string scriptUID = AS_STD_STRING(GetScriptIdentity());
	auto propertyContainer = JenovaInterpreter::GetPropertyContainer(scriptUID);
	for (const auto& property : propertyContainer.scriptProperties) propertyList.push_back(Dictionary(property.propertyInfo));
	return propertyList;
}
int32_t CPPScript::_get_member_line(const StringName& p_member) const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_get_member_line (%s)", AS_C_STRING(p_member));
	return 0;  // Not Supported Yet
}
Dictionary CPPScript::_get_constants() const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_get_constants");
	return Dictionary();  // Not Supported Yet
}
TypedArray<StringName> CPPScript::_get_members() const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_get_members");
	return TypedArray<StringName>();  // Not Supported Yet
}
bool CPPScript::_is_placeholder_fallback_enabled() const
{
	return false;  // Not Supported Yet
}
Variant CPPScript::_get_rpc_config() const
{
	jenova::VerboseByID(__LINE__, "CPPScript::_get_rpc_config");
	return Variant();  // Not Supported Yet
}

// CPPScript Extended Implementation
void CPPScript::SetDefaultSourceCode()
{
	// Set Default Source Code
	source_code = CODE_TEMPLATE(CODE_TEMPLATE_DEFAULT);
}
jenova::ScriptIdentifier CPPScript::GetScriptIdentity() const
{
	return jenova::GenerateStandardUIDFromPath(this);
}
void CPPScript::ReloadScriptSourceCode()
{
	source_code = FileAccess::get_file_as_string(this->get_path());
}

// CPPScript Initializer/Destructor
CPPScript::CPPScript()
{
	// Initialize Objects
	scriptMutex.instantiate();

	// Register Script Object to Manager
	JenovaScriptManager::get_singleton()->add_script_object(this);

	// Set Default Source Code
	SetDefaultSourceCode();
}
CPPScript::~CPPScript()
{
	// Release Objects
	scriptMutex.unref();

	// Unregister Script Object to Manager
	JenovaScriptManager::get_singleton()->remove_script_object(this);
}

// CPPHeader Object Implementation
void CPPHeader::_bind_methods() { }
bool CPPHeader::_editor_can_reload_from_file() { return false; }
void CPPHeader::_placeholder_erased(void* p_placeholder) { }
bool CPPHeader::_can_instantiate() const { return false; }
Ref<Script> CPPHeader::_get_base_script() const { return Ref<Script>(); }
StringName CPPHeader::_get_global_name()
{
	return StringName("Jenova C++ Header");
}
bool CPPHeader::_inherits_script(const Ref<Script>& p_script) const
{
	return false;
}
StringName CPPHeader::_get_instance_base_type() const
{
	return StringName(jenova::GlobalSettings::JenovaHeaderType);
}
void* CPPHeader::_instance_create(Object* p_for_object) const { return nullptr; }
void* CPPHeader::_placeholder_instance_create(Object* p_for_object) const { return nullptr; }
bool CPPHeader::_instance_has(Object* p_object) const { return false; }
bool CPPHeader::_has_source_code() const { return !source_code.is_empty(); }
String CPPHeader::_get_source_code() const { return this->source_code; }
void CPPHeader::_set_source_code(const String& p_code) { this->source_code = p_code; }
Error CPPHeader::_reload(bool p_keep_state) { return Error::ERR_UNAVAILABLE; }
StringName CPPHeader::_get_doc_class_name() const
{
	return StringName();
}
TypedArray<Dictionary> CPPHeader::_get_documentation() const { return TypedArray<Dictionary>(); }
String CPPHeader::_get_class_icon_path() const { return String(); }
bool CPPHeader::_has_method(const StringName& p_method) const { return false; }
bool CPPHeader::_has_static_method(const StringName& p_method) const { return false; }
Dictionary CPPHeader::_get_method_info(const StringName& p_method) const { return Dictionary(); }
bool CPPHeader::_is_tool() const { return false; }
bool CPPHeader::_is_valid() const { return false; }
bool CPPHeader::_is_abstract() const { return true; }
ScriptLanguage* CPPHeader::_get_language() const
{
	return CPPHeaderLanguage::get_singleton();
}
bool CPPHeader::_has_script_signal(const StringName& p_signal) const { return false; }
TypedArray<Dictionary> CPPHeader::_get_script_signal_list() const { return TypedArray<Dictionary>(); }
bool CPPHeader::_has_property_default_value(const StringName& p_property) const { return false; }
Variant CPPHeader::_get_property_default_value(const StringName& p_property) const { return false; }
void CPPHeader::_update_exports() { }
TypedArray<Dictionary> CPPHeader::_get_script_method_list() const { return TypedArray<Dictionary>(); }
TypedArray<Dictionary> CPPHeader::_get_script_property_list() const { return TypedArray<Dictionary>(); }
int32_t CPPHeader::_get_member_line(const StringName& p_member) const { return 0; }
Dictionary CPPHeader::_get_constants() const { return Dictionary(); }
TypedArray<StringName> CPPHeader::_get_members() const { return TypedArray<StringName>(); }
bool CPPHeader::_is_placeholder_fallback_enabled() const { return false; }
Variant CPPHeader::_get_rpc_config() const { return Variant(); }
void CPPHeader::ReloadHeaderSourceCode()
{
	source_code = FileAccess::get_file_as_string(this->get_path());
}
CPPHeader::CPPHeader()
{
	this->source_code = "#pragma once\n";
}