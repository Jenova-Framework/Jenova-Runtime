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

// Jenova Script Instance Wrapper Definition
namespace godot
{
	class ScriptInstanceExtension
	{
		static GDExtensionScriptInstanceInfo3 script_instance_info;

	public:
		GDExtensionScriptInstanceDataPtr instance = nullptr;

	public:
		static GDExtensionScriptInstancePtr create_native_instance(ScriptInstanceExtension* p_instance)
		{
			p_instance->instance = ::godot::gdextension_interface::script_instance_create3(&script_instance_info, p_instance);
			return p_instance->instance;
		}
		static GDExtensionScriptInstanceInfo3* get_script_insatnce_info()
		{
			return &script_instance_info;
		}

		virtual bool set(const StringName& p_name, const Variant& p_value) = 0;
		virtual bool get(const StringName& p_name, Variant& r_ret) const = 0;
		virtual const GDExtensionPropertyInfo* get_property_list(uint32_t* r_count) const = 0;
		virtual void free_property_list(const GDExtensionPropertyInfo* p_list, uint32_t p_count) const = 0;
		virtual Variant::Type get_property_type(const StringName& p_name, bool* r_is_valid) const = 0;
		virtual bool validate_property(GDExtensionPropertyInfo& p_property) const = 0;
		virtual bool get_class_category(GDExtensionPropertyInfo& r_class_category) const;
		virtual bool property_can_revert(const StringName& p_name) const = 0;
		virtual bool property_get_revert(const StringName& p_name, Variant& r_ret) const = 0;
		virtual Object* get_owner() = 0;
		virtual void get_property_state(GDExtensionScriptInstancePropertyStateAdd p_add_func, void* p_userdata) = 0;
		virtual const GDExtensionMethodInfo* get_method_list(uint32_t* r_count) const = 0;
		virtual void free_method_list(const GDExtensionMethodInfo* p_list, uint32_t p_count) const = 0;
		virtual bool has_method(const StringName& p_method) const = 0;
		virtual int get_method_argument_count(const StringName& p_method, bool* r_is_valid = nullptr) const = 0;
		virtual Variant callp(const StringName& p_method, const Variant** p_args, int p_argcount, GDExtensionCallError& r_error) = 0;
		virtual void notification(int p_notification, bool p_reversed) = 0;
		virtual String to_string(bool* r_valid) = 0;
		virtual void refcount_incremented() = 0;
		virtual bool refcount_decremented() = 0;
		virtual Ref<Script> get_script() const = 0;
		virtual bool is_placeholder() const = 0;
		virtual void property_set_fallback(const StringName& p_name, const Variant& p_value, bool* r_valid) = 0;
		virtual Variant property_get_fallback(const StringName& p_name, bool* r_valid) = 0;
		virtual ScriptLanguage* _get_language() = 0;

		virtual ~ScriptInstanceExtension() {};
	};
};
