
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

// Helper Functions
static String* AllocateString(const String& p_str) 
{
	String* ptr = memnew(String);
	*ptr = p_str;
	return ptr;
}
static StringName* AllocateStringName(const String& p_str) 
{
	StringName* ptr = memnew(StringName);
	*ptr = p_str;
	return ptr;
}
static GDExtensionPropertyInfo CreatePropertyType(const Dictionary& p_src) 
{
	GDExtensionPropertyInfo p_dst;
	p_dst.type = GDExtensionVariantType::GDEXTENSION_VARIANT_TYPE_NIL;
	p_dst.name = AllocateStringName(p_src["name"]);
	p_dst.class_name = AllocateStringName(p_src["class_name"]);
	p_dst.hint = p_src["hint"];
	p_dst.hint_string = AllocateString(p_src["hint_string"]);
	p_dst.usage = p_src["usage"];
	return p_dst;
}
static void AddState(GDExtensionConstStringNamePtr p_name, GDExtensionConstVariantPtr p_value, void* p_userdata) 
{
	List<Pair<StringName, Variant>>* list = reinterpret_cast<List<Pair<StringName, Variant>>*>(p_userdata);
	list->push_back({ *(const StringName*)p_name, *(const Variant*)p_value });
}
static GDExtensionMethodInfo CreateMethodInfo(const MethodInfo& methodInfo, CPPScriptInstance* instance)
{
	// Create Extension Parameters Info
	std::vector<GDExtensionPropertyInfo>* parametersInfo = new std::vector<GDExtensionPropertyInfo>();
	instance->methodInfoPointers.push_back(parametersInfo);

	for (const auto& parameter : methodInfo.arguments)
	{
		parametersInfo->push_back(GDExtensionPropertyInfo
		{
			GDExtensionVariantType(parameter.type),
			AllocateStringName(parameter.name),
			AllocateStringName(parameter.class_name),
			parameter.hint,
			AllocateString(parameter.hint_string),
			parameter.usage
		});
	}

	// Create Extension Method Info
	return GDExtensionMethodInfo
	{
		AllocateStringName(methodInfo.name),
		GDExtensionPropertyInfo
		{
			GDExtensionVariantType(methodInfo.return_val.type),
			AllocateStringName(methodInfo.return_val.name),
			AllocateStringName(methodInfo.return_val.class_name),
			methodInfo.return_val.hint,
			AllocateString(methodInfo.return_val.hint_string),
			methodInfo.return_val.usage
		},
		methodInfo.flags,
		methodInfo.id,
		(uint32_t)methodInfo.arguments.size(),
		parametersInfo->data(),
		0, nullptr // Default Arguments Not Supported Yet
	};
}
template<typename T> T* memnew_with_size(int p_size)
{
	uint64_t size = sizeof(T) * p_size;
	void* ptr = memalloc(size + sizeof(int));
	*((int*)ptr) = p_size;
	return (T*)((int*)ptr + 1);
}
template<typename T> void memdelete_with_size(const T* p_ptr)
{
	memfree((int*)p_ptr - 1);
}
template<typename T> int memnew_ptr_size(const T* p_ptr)
{
	return !p_ptr ? 0 : *((int*)p_ptr - 1);
}
static void FreePropertyList(const GDExtensionPropertyInfo& p_property)
{
	memdelete((StringName*)p_property.name);
	memdelete((StringName*)p_property.class_name);
	memdelete((String*)p_property.hint_string);
}

// Internal Godot Functions
static const jenova::FunctionList godot_functions =
{
	"_get_editor_name",
	"_hide_script_from_inspector",
	"_is_read_only",
};

// C++ Script Instance Implementation
bool CPPScriptInstance::set(const StringName &p_name, const Variant &p_value) 
{
	// Remove
	jenova::VerboseByID(__LINE__, "Setting Property (%s)...", AS_C_STRING(p_name));

	// Set Embedded Source [Internal]
	if (p_name == StringName("cpp/source_code") && script.is_valid() && script->is_built_in())
	{
		script->set_source_code(jenova::RetriveStringFromSecuredBase64String(p_value));
		return true;
	}

	// Set Interpreted Properties [Optimize This!]
	if (instanceProperties.has(p_name))
	{
		instanceProperties[p_name] = p_value;
		return true;
	}
	else
	{
		auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
		for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
		{
			if (p_name == propContainer.scriptProperties[i].propertyInfo.name)
			{
				instanceProperties[p_name] = p_value;
				return true;
			}
		}
	}

	// Not Handled
	return false;
}
bool CPPScriptInstance::get(const StringName &p_name, Variant &r_ret) const
 {
	// Remove
	jenova::VerboseByID(__LINE__, "Getting Property (%s)...", AS_C_STRING(p_name));

	// Get Script
	if (p_name == StringName("script")) 
	{
		r_ret = script;
		return true;
	}

	// Get Embedded Source [Internal]
	if (p_name == StringName("cpp/source_code") && script.is_valid() && script->is_built_in())
	{
		r_ret = jenova::CreateSecuredBase64StringFromString(script->get_source_code());
		return true;
	}

	// Get Interpreted Properties [Optimize This!]
	if (instanceProperties.has(p_name))
	{
		r_ret = instanceProperties[p_name];
		return true;
	}
	else
	{
		auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
		for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
		{
			if (p_name == propContainer.scriptProperties[i].propertyInfo.name)
			{
				r_ret = propContainer.scriptProperties[i].defaultValue;
				return true;
			}
		}
	}

	// Not Handled
	return false;
}
godot::String CPPScriptInstance::to_string(bool *r_is_valid) 
{
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::to_string");
	*r_is_valid = true;
	return String(jenova::Format("<JenovaScript:%s>", AS_C_STRING(get_identity())).c_str());
}
void CPPScriptInstance::notification(int p_notification, bool p_reversed)
{
	if (p_notification == Object::NOTIFICATION_PREDELETE) isDeleting = true;
}
Variant CPPScriptInstance::callp(const StringName &p_method, const Variant **p_args, const int p_argument_count, GDExtensionCallError &r_error) 
{
	// Validate Instance & Script
	if (isDeleting || !this->script.is_valid())
	{
		r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	// Verbose Call
	String script_name = this->script->get_path().get_file();
	String owner_name = godot::Object::cast_to<godot::Node>(this->owner)->get_name();
	jenova::VerboseByID(__LINE__, "Executing Script (%s | %s)[%s][%d] from (%s | %p) ...", 
		AS_C_STRING(script_name), AS_C_STRING(scriptInstanceIdentity), AS_C_STRING(p_method), p_argument_count,AS_C_STRING(owner_name), this->instance);

	// Handle Internal Methods
	if (p_method == StringName("_get_editor_name"))
	{
		r_error.error = GDEXTENSION_CALL_OK;
		return Variant(String(jenova::Format("[ %s · Powered by Jenova ]", AS_C_STRING(godot::Object::cast_to<godot::Node>(this->owner)->get_name())).c_str()));
	}
	else if (p_method == StringName("_hide_script_from_inspector")) 
	{
		r_error.error = GDEXTENSION_CALL_OK;
		return false;
	}
	else if (p_method == StringName("_is_read_only")) 
	{
		r_error.error = GDEXTENSION_CALL_OK;
		return false;
	}

	// Abort Call In Editor If Script is Not Tool
	if (QUERY_ENGINE_MODE(Editor) && !script->is_tool())
	{
		r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	// Initial Update for Properties
	if (p_method == StringName("_enter_world"))
	{
		auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
		for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
		{
			jenova::ScriptProperty scriptProperty = propContainer.scriptProperties[i];
			if (!this->instanceProperties.has(scriptProperty.propertyInfo.name)) this->instanceProperties[scriptProperty.propertyInfo.name] = scriptProperty.defaultValue;
		}
	}

	// Update Interpreter Properties
	if (this->instanceProperties.size() != 0)
	{
		Array instancePropertiesKeys = this->instanceProperties.keys();
		for (size_t i = 0; i < instancePropertiesKeys.size(); i++)
		{
			if (!JenovaInterpreter::SetPropertyValueFromVariant(instancePropertiesKeys[i], this->instanceProperties[instancePropertiesKeys[i]], scriptInstanceIdentity))
			{
				jenova::Error("Jenova Interpreter", "Failed to Update Interpreter Property Storage Value!");
				r_error.error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
				return Variant();
			}
		}
	}

	// Call to Interpreter
	bool hasMethod = false;
	jenova::FunctionList jenovaMethods = JenovaInterpreter::GetFunctionsList(AS_STD_STRING(scriptInstanceIdentity));
	for (auto& function : jenovaMethods)
	{
		if (p_method == StringName(function.c_str()))
		{
			hasMethod = true;
			break;
		}
	}
	if (hasMethod)
	{
		// Invoke Function & Call
		Variant callResult = JenovaInterpreter::CallFunction(this->owner, AS_STD_STRING(p_method), AS_STD_STRING(scriptInstanceIdentity), p_args, p_argument_count);

		// Update Properties
		if (jenova::GlobalSettings::UpdatePropertiesAfterCall)
		{
			// Update Interpreter Properties
			if (this->instanceProperties.size() != 0)
			{
				Array instancePropertiesKeys = instanceProperties.keys();
				for (size_t i = 0; i < instancePropertiesKeys.size(); i++)
				{
					Variant variantValue = Variant::NIL;
					jenova::PropertyPointer propertyPointer = JenovaInterpreter::GetPropertyPointer(instancePropertiesKeys[i], scriptInstanceIdentity);
					if (!jenova::GetVariantFromPropertyPointer(propertyPointer, variantValue, instanceProperties[instancePropertiesKeys[i]].get_type()))
					{
						jenova::Error("Jenova Interpreter", "Failed to Update Property Storage Value From Interpreter Property!");
						r_error.error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
						return Variant();
					}
					instanceProperties[instancePropertiesKeys[i]] = variantValue;
				}
			}
		}

		// Return Result
		r_error.error = GDEXTENSION_CALL_OK;
		return callResult;
	}

	// Default Result
	r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
	return Variant();
}
void CPPScriptInstance::update_methods() const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::update_methods (%s)", AS_C_STRING(scriptInstanceIdentity));

	// Validate Script
	if (script.is_null()) return;

	// Update Script Methods
	methodsInfo.clear();
	auto functionContainer = JenovaInterpreter::GetFunctionContainer(AS_STD_STRING(scriptInstanceIdentity));
	for (auto& scriptFunction : functionContainer.scriptFunctions)
	{
		this->methodsInfo.push_back(scriptFunction.methodInfo);
	}
}
const GDExtensionMethodInfo* CPPScriptInstance::get_method_list(uint32_t *r_count) const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::get_method_list (%s)", AS_C_STRING(scriptInstanceIdentity));

	// Validate Script
	if (script.is_null()) 
	{
		*r_count = 0;
		return nullptr;
	}

	// Update Methods
	this->update_methods();

	// Create Method List
	const int size = methodsInfo.size();
	GDExtensionMethodInfo* list = memnew_arr(GDExtensionMethodInfo, size);
	int i = 0;
	for (auto& methodInfo : methodsInfo) 
	{
		CPPScriptInstance* self = const_cast<CPPScriptInstance*>(this);
		list[i] = CreateMethodInfo(methodInfo, self);
		i++;
	}

	// Remove
	jenova::VerboseByID(__LINE__, "Method List returned %d functions", size);

	// Return Methods
	*r_count = size;
	return list;
}
void CPPScriptInstance::free_method_list(const GDExtensionMethodInfo* p_list, uint32_t p_count) const
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::free_method_list");

	if (p_list) memdelete_arr(p_list);
}
const GDExtensionPropertyInfo* CPPScriptInstance::get_property_list(uint32_t *r_count) const
{
	// Create Property List
	LocalVector<GDExtensionPropertyInfo> propertiesInfo;

	// Add Properties
	if (script.is_valid() && script->is_built_in())
	{
		GDExtensionPropertyInfo sourceCodeProperty = {};
		sourceCodeProperty.type = GDEXTENSION_VARIANT_TYPE_STRING;
		sourceCodeProperty.name = AllocateStringName("cpp/source_code");
		sourceCodeProperty.class_name = AllocateStringName("Variant");
		sourceCodeProperty.hint_string = AllocateString("");
		sourceCodeProperty.hint = PROPERTY_HINT_NONE;
		sourceCodeProperty.usage = PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL;
		propertiesInfo.push_back(sourceCodeProperty);
	}
	
	// Add Jenova Script Interpreted Properties
	auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
	for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
	{
		GDExtensionPropertyInfo sourceCodeProperty = {};
		sourceCodeProperty.type = GDExtensionVariantType(propContainer.scriptProperties[i].propertyInfo.type);
		sourceCodeProperty.name = AllocateStringName(propContainer.scriptProperties[i].propertyInfo.name);
		sourceCodeProperty.class_name = AllocateStringName(propContainer.scriptProperties[i].propertyInfo.class_name);
		sourceCodeProperty.hint_string = AllocateString(propContainer.scriptProperties[i].propertyInfo.hint_string);
		sourceCodeProperty.hint = propContainer.scriptProperties[i].propertyInfo.hint;
		sourceCodeProperty.usage = propContainer.scriptProperties[i].propertyInfo.usage;
		propertiesInfo.push_back(sourceCodeProperty);
	}

	// Add Jenova Script User-Defined Properties

	// Set Properties Size
	*r_count = propertiesInfo.size();
	if (propertiesInfo.size() == 0) return nullptr;

	// Create Property Final List
	GDExtensionPropertyInfo* list = memnew_with_size<GDExtensionPropertyInfo>(propertiesInfo.size());
	memcpy(list, propertiesInfo.ptr(), sizeof(GDExtensionPropertyInfo) * propertiesInfo.size());
	return list;
}
void CPPScriptInstance::free_property_list(const GDExtensionPropertyInfo *p_list, uint32_t p_count) const 
{
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::free_property_list");
	if (p_list)
	{
		int size = memnew_ptr_size<GDExtensionPropertyInfo>(p_list);
		for (int i = 0; i < size; i++) FreePropertyList(p_list[i]);
		memdelete_with_size<GDExtensionPropertyInfo>(p_list);
	}
}
Variant::Type CPPScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const 
{
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::get_property_type");

	// Handle Internal Properties
	if (p_name == StringName("cpp/source_code"))
	{
		if (r_is_valid) *r_is_valid = true;
		return Variant::STRING;
	}

	// Handle Interpreted Properties
	auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
	for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
	{
		if (p_name == propContainer.scriptProperties[i].propertyInfo.name)
		{
			if (r_is_valid) *r_is_valid = true;
			return propContainer.scriptProperties[i].propertyInfo.type;
		}
	}

	// Not Found
	*r_is_valid = false;
	return Variant::NIL;
}
void CPPScriptInstance::get_property_state(GDExtensionScriptInstancePropertyStateAdd p_add_func, void *p_userdata) 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::get_property_state");

	p_add_func = AddState; // Needs Investigation
}
bool CPPScriptInstance::validate_property(GDExtensionPropertyInfo &p_property) const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::validate_property %s", AS_C_STRING(*(StringName*)p_property.name));

	// Verify & Get Property Name
	if (!p_property.name) return false;
	StringName propertyName = *(StringName*)p_property.name;

	// Handle Interpreted Properties
	if (instanceProperties.has(propertyName))
	{
		return true;
	}
	else
	{
		auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
		for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
		{
			if (propertyName == propContainer.scriptProperties[i].propertyInfo.name) return true;
		}
	}

	// Not Implemented Yet
	return false;
}
bool CPPScriptInstance::has_method(const StringName &p_name) const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::has_method (%s) [%s]", AS_C_STRING(scriptInstanceIdentity), AS_C_STRING(p_name));

	// Validate Script
	if (!script.is_valid()) return false;
	if (script.is_null()) return false;
	bool result = false;

	// Search Over Pre-Defined Functions [These will be not filtered by Tool Mode]
	for (auto& function : godot_functions) 
	{
		if (p_name == StringName(function.c_str()))
		{
			result = true;
			break;
		}
	}
	
	// Jenova Module Functions Handling
	if (!result)
	{
		// Get Jenova Function List And Search Over User Defined Functions
		jenova::FunctionList jenovaMethods = JenovaInterpreter::GetFunctionsList(AS_STD_STRING(scriptInstanceIdentity));
		for (auto& function : jenovaMethods)
		{
			if (p_name == StringName(function.c_str()))
			{
				result = true;
				break;
			}
		}

		// In Editor and Tool Mode We Return All Functions As True
		if (!result && QUERY_ENGINE_MODE(Editor) && script->is_tool()) result = true;
	}

	// Temp Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::has_method (%s) [%s] returned %s", AS_C_STRING(scriptInstanceIdentity), AS_C_STRING(p_name), result ? "TRUE" : "FALSE");
	return result;
}
int CPPScriptInstance::get_method_argument_count(const StringName& p_method, bool* r_is_valid) const
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::get_method_argument_count");
	*r_is_valid = false;
	return 0;
}
bool CPPScriptInstance::property_can_revert(const StringName &p_name) const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::property_can_revert");

	// Handle Interpreted Properties
	auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
	for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
	{
		if (p_name == propContainer.scriptProperties[i].propertyInfo.name) return true;
	}

	// Not Found
	return false;
}
bool CPPScriptInstance::property_get_revert(const StringName &p_name, Variant &r_ret) const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::property_get_revert");

	// Handle Interpreted Properties
	auto propContainer = JenovaInterpreter::GetPropertyContainer(AS_STD_STRING(this->scriptInstanceIdentity));
	for (size_t i = 0; i < propContainer.scriptProperties.size(); i++)
	{
		if (p_name == propContainer.scriptProperties[i].propertyInfo.name)
		{
			r_ret = propContainer.scriptProperties[i].defaultValue;
			return true;
		}
	}

	// Not Found
	return false;
}
void CPPScriptInstance::refcount_incremented() 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::refcount_incremented");

	refCount++;
}
bool CPPScriptInstance::refcount_decremented() 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::refcount_decremented");

	refCount--;
	return false;
}
Object* CPPScriptInstance::get_owner() 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::get_owner");

	return owner;
}
Ref<Script> CPPScriptInstance::get_script() const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::get_script");
	return script;
}
bool CPPScriptInstance::is_placeholder() const 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::is_placeholder");

	return false;
}
void CPPScriptInstance::property_set_fallback(const StringName &p_name, const Variant &p_value, bool *r_valid) 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::property_set_fallback");

	*r_valid = false;
}
Variant CPPScriptInstance::property_get_fallback(const StringName &p_name, bool *r_valid) 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::property_get_fallback");

	*r_valid = false;
	return Variant::NIL;
}
ScriptLanguage* CPPScriptInstance::_get_language() 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::_get_language");

	return CPPScriptLanguage::get_singleton();
}
String CPPScriptInstance::get_identity()
{
	return scriptInstanceIdentity;
}

// C++ Script Instance Initializer/Destructor
CPPScriptInstance::CPPScriptInstance(Object *p_owner, const Ref<CPPScript> p_script) :
		owner(p_owner), script(p_script) 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::CPPScriptInstance");

	// Validate Script Object
	if (p_script.is_null() || !p_script.is_valid())
	{
		jenova::VerboseByID(__LINE__, "ERROR : Null Script Passed to Instance.");
		return;
	}

	// Verbose Creation
	godot::Node* parentNode = godot::Object::cast_to<godot::Node>(p_owner);
	jenova::VerboseByID(__LINE__, "Creating Script Instance from (%s) Owner : %s", AS_C_STRING(p_script.ptr()->get_name()), AS_C_STRING(parentNode->get_name()));

	// Generate Script Identifier Hash
	scriptInstanceIdentity = jenova::GenerateStandardUIDFromPath(p_script.ptr());

	// Register Script Instance to Manager
	JenovaScriptManager::get_singleton()->add_script_instance(this);
}
CPPScriptInstance::~CPPScriptInstance() 
{
	// Remove
	jenova::VerboseByID(__LINE__, "CPPScriptInstance::~CPPScriptInstance (%s)", AS_C_STRING(this->get_identity()));

	// Unregister Script Instance to Manager
	JenovaScriptManager::get_singleton()->remove_script_instance(this);

	// Release Pointers
	for (size_t i = 0; i < this->methodInfoPointers.size(); i++) delete this->methodInfoPointers[i];
	std::vector<std::vector<GDExtensionPropertyInfo>*>().swap(this->methodInfoPointers);
}