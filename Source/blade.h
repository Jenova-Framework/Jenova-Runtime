#pragma once

/*-------------------------------------------------------------+
|                                                              |
|           ______         _______ ______  _______             |
|           |_____] |      |_____| |     \ |______             |
|           |_____] |_____ |     | |_____/ |______             |
|                                                              |
|                        Blade Language                        |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Jenova SDK
#include "Jenova.hpp"

// Script Instance Base
#include "blade_instance.h"

// Configuration
namespace blade
{
	static const char* BladeVersion						= "0.3.0 [alpha-release]";

	static const char* BladeCore						= "Native-Extension";
	static const char* BladeScriptType					= "BladeScript";
	static const char* BladeLangaugeName				= "Blade";
	static const char* BladeExtension					= "blade";
	static const char* BladeCacheDirectory				= ".blade";
	static const char* BladeLibraryDirectory			= "BladeLib";
	static const char* BladeBindingsDatabasePath		= "Blade.Bindings.json";
	static const char* BladeBindingsDatabaseBundlePath	= "res://__BLADE__/_APICACHE_";
	static const char* BladeScriptCacheRuntimePath		= "res://__BLADE__/_CACHE_/";
	static const char* BladeExtensionAPIFilePath		= "extension_api.json";

	static const bool  CacheScriptMetadataOnDisk		= true;			// Required for Exporter
	static const bool  DumpPreprocessedScriptOnDisk		= false;		// Developer Mode
	static const bool  GenerateControlFlowWrappers		= true;			// Optional
	static const bool  ExtensionSystemEnabled			= true;			// Optional
}

// Forward Declarationss
class BladeScript;
struct BladeExtension;
class BladeEventWrapper;

// Blade Type Definitions
typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::unordered_map<std::string, BladeExtension> ExtensionStorage;
typedef std::function<String(String)> RecursiveRewriter;

// Blade Enums
enum class BladeState : uint8_t
{
	Unknown,
	Initialized,
	Error,
	Compiling,
	Compiled,
	Ready
};

// Blade Compiler Interface
class BladeCompiler
{
public:
    virtual ~BladeCompiler() {}

	virtual bool Validate() = 0;
	virtual bool AddIncludePath(const std::string& includePath) = 0;
    virtual bool AddSymbol(const std::string& name, const void* ptr) = 0;
    virtual bool Compile(const std::string& source) = 0;
    virtual bool Prepare() = 0;
    virtual void* GetSymbol(const std::string& symbolName) = 0;
};

// Blade Language Definition
class BladeLanguage : public ScriptLanguageExtension
{
	GDCLASS(BladeLanguage, ScriptLanguageExtension);

protected:
	static void _bind_methods() {}

public:
	static void init();
	static void deinit();
	static BladeLanguage* get_singleton();

	virtual String _get_name() const override;
	virtual void _init() override;
	virtual void _release();
	virtual String _get_type() const override;
	virtual String _get_extension() const override;
	virtual void _finish() override;
	virtual PackedStringArray _get_reserved_words() const override;
	virtual bool _is_control_flow_keyword(const String& p_keyword) const override;
	virtual PackedStringArray _get_comment_delimiters() const override;
	virtual PackedStringArray _get_doc_comment_delimiters() const override;
	virtual PackedStringArray _get_string_delimiters() const override;
	virtual Ref<Script> _make_template(const String& p_template, const String& p_class_name, const String& p_base_class_name) const override;
	virtual TypedArray<Dictionary> _get_built_in_templates(const StringName& p_object) const override;
	virtual bool _is_using_templates() override;
	virtual Dictionary _validate(const String& p_script, const String& p_path, bool p_validate_functions, bool p_validate_errors, bool p_validate_warnings, bool p_validate_safe_lines) const override;
	virtual String _validate_path(const String& p_path) const override;
	virtual Object* _create_script() const override;
	virtual bool _supports_builtin_mode() const override;
	virtual bool _supports_documentation() const override;
	virtual bool _can_inherit_from_file() const override;
	virtual int32_t _find_function(const String& p_function, const String& p_code) const override;
	virtual String _make_function(const String& p_class_name, const String& p_function_name, const PackedStringArray& p_function_args) const override;
	virtual Error _open_in_external_editor(const Ref<Script>& p_script, int32_t p_line, int32_t p_column) override;
	virtual bool _overrides_external_editor() override;
	virtual bool _can_make_function() const override;
	virtual Dictionary _complete_code(const String& p_code, const String& p_path, Object* p_owner) const override;
	virtual Dictionary _lookup_code(const String& p_code, const String& p_symbol, const String& p_path, Object* p_owner) const override;
	virtual String _auto_indent_code(const String& p_code, int32_t p_from_line, int32_t p_to_line) const override;
	virtual void _add_global_constant(const StringName& p_name, const Variant& p_value) override;
	virtual void _add_named_global_constant(const StringName& p_name, const Variant& p_value) override;
	virtual void _remove_named_global_constant(const StringName& p_name) override;
	virtual void _thread_enter() override;
	virtual void _thread_exit() override;
	virtual String _debug_get_error() const override;
	virtual int32_t _debug_get_stack_level_count() const override;
	virtual int32_t _debug_get_stack_level_line(int32_t p_level) const override;
	virtual String _debug_get_stack_level_function(int32_t p_level) const override;
	virtual Dictionary _debug_get_stack_level_locals(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth) override;
	virtual Dictionary _debug_get_stack_level_members(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth) override;
	virtual void* _debug_get_stack_level_instance(int32_t p_level) override;
	virtual Dictionary _debug_get_globals(int32_t p_max_subitems, int32_t p_max_depth) override;
	virtual String _debug_parse_stack_level_expression(int32_t p_level, const String& p_expression, int32_t p_max_subitems, int32_t p_max_depth) override;
	virtual TypedArray<Dictionary> _debug_get_current_stack_info() override;
	virtual void _reload_all_scripts() override;
	virtual void _reload_scripts(const Array& p_scripts, bool p_soft_reload);
	virtual void _reload_tool_script(const Ref<Script>& p_script, bool p_soft_reload) override;
	virtual PackedStringArray _get_recognized_extensions() const override;
	virtual TypedArray<Dictionary> _get_public_functions() const override;
	virtual Dictionary _get_public_constants() const override;
	virtual TypedArray<Dictionary> _get_public_annotations() const override;
	virtual void _profiling_start() override;
	virtual void _profiling_stop() override;
	virtual void _profiling_set_save_native_calls(bool p_enable) override;
	virtual int32_t _profiling_get_accumulated_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max) override;
	virtual int32_t _profiling_get_frame_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max) override;
	virtual void _frame() override;
	virtual bool _handles_global_class_type(const String& p_type) const override;
	virtual Dictionary _get_global_class_name(const String& p_path) const override;

	BladeLanguage() {}
	~BladeLanguage() {}

public:
	// Methods
	bool CreateBindings();
	bool LoadBindingsDatabase();
	const jenova::json_t GetBindingsDatabase() const;
	void GenerateFrameworkCode();
	String GetFrameworkCode();
	bool RegisterScript(Ref<BladeScript> script);
	bool UnregisterScript(Ref<BladeScript> script);
	void AddKeywordDefinition(const String& keyword, const Color& color);
	bool IsKeywordDefinition(const String& keyword);
	Color GetKeywordDefinitionColor(const String& keyword);

	// Events
	bool HandleRemoteMessage(const String& message, const Array& args);

private:
	bool							isInitialized = false;
	String							frameworkCode;
	jenova::json_t					bindingsDatabase;

private:
	// Storage
	TypedArray<Ref<BladeScript>>	activeScripts;
	HashMap<String, Color>			keywordDefinitionStorage;
};

// Blade Script Object Definition
class BladeScript : public ScriptExtension
{
	GDCLASS(BladeScript, ScriptExtension);

protected:
	static void _bind_methods();

private:
	mutable BladeCompiler*	scriptContext = nullptr;
	mutable jenova::json_t	scriptMetadata = jenova::json_t();
	mutable BladeState		scriptState = BladeState::Unknown;
	mutable Object*			baseObject = nullptr;

	mutable String			metaClassName;
	mutable String			metaClassBase;
	mutable bool			metaHasClassName = false;
	mutable bool			metaHasClassBase = false;
	mutable bool			metaToolEnabled = false;

	Ref<Mutex>				scriptMutex;
	String					sourceCode;
	int64_t					sourceHash = 0;
	TimePoint 				lastCompileTime;
	TypedArray<Dictionary>  scriptProperties;

public:
	// Class Information (Cached by get_class_category)
	StringName				scriptClassName;
	StringName				scriptClassType;
	String					scriptClassPath;
	
public:
	// Base Methods
	virtual bool _editor_can_reload_from_file() override;
	virtual void _placeholder_erased(void* p_placeholder) override;
	virtual bool _can_instantiate() const override;
	virtual Ref<Script> _get_base_script() const override;
	virtual StringName _get_global_name();
	virtual bool _inherits_script(const Ref<Script>& p_script) const override;
	virtual StringName _get_instance_base_type() const override;
	virtual void* _instance_create(Object* p_for_object) const override;
	virtual void* _placeholder_instance_create(Object* p_for_object) const override;
	virtual bool _instance_has(Object* p_object) const override;
	virtual bool _has_source_code() const override;
	virtual String _get_source_code() const override;
	virtual void _set_source_code(const String& p_code) override;
	virtual Error _reload(bool p_keep_state) override;
	virtual StringName _get_doc_class_name() const override;
	virtual TypedArray<Dictionary> _get_documentation() const override;
	virtual String _get_class_icon_path() const override;
	virtual bool _has_method(const StringName& p_method) const override;
	virtual bool _has_static_method(const StringName& p_method) const override;
	virtual Dictionary _get_method_info(const StringName& p_method) const override;
	virtual bool _is_tool() const override;
	virtual bool _is_valid() const override;
	virtual bool _is_abstract() const override;
	virtual ScriptLanguage* _get_language() const override;
	virtual bool _has_script_signal(const StringName& p_signal) const override;
	virtual TypedArray<Dictionary> _get_script_signal_list() const override;
	virtual bool _has_property_default_value(const StringName& p_property) const override;
	virtual Variant _get_property_default_value(const StringName& p_property) const override;
	virtual void _update_exports() override;
	virtual TypedArray<Dictionary> _get_script_method_list() const override;
	virtual TypedArray<Dictionary> _get_script_property_list() const override;
	virtual int32_t _get_member_line(const StringName& p_member) const override;
	virtual Dictionary _get_constants() const override;
	virtual TypedArray<StringName> _get_members() const override;
	virtual bool _is_placeholder_fallback_enabled() const override;
	virtual Variant _get_rpc_config() const override;

public:
	// Methods
	void Initialize() {}
	void ReloadSourceCode();
	void UpdateScriptFeatures() const;
	void BindScriptSymbols(BladeCompiler* compiler);
	String PreProcessScript(jenova::json_t& metadata);
	bool CompileScript(bool dryPass = false);
	bool PostProcessScript(BladeCompiler* compiler, jenova::json_t& metadata);
	void EnsureCompiled() const;
	void InvalidateContext();
	void PerformHotReload(const String& newSourceCode);
	const bool HasClassName() const { return metaHasClassName; }
	BladeCompiler* GetContext() const { return scriptContext; }
	const jenova::json_t& GetScriptMetadata() const { return scriptMetadata; }

	// Intializer/Destroyer
	BladeScript();
	~BladeScript();
};

// Blade Script Instance Definition
class BladeScriptInstance : public blade::ScriptInstanceExtension
{
protected:
	Ref<BladeScript> script;
	Ref<BladeEventWrapper> eventWrapper;
	Object* owner = nullptr;
	size_t refCount = 0;
	bool isDeleting = false;

private:
	mutable Dictionary instanceProperties;

public:
	// Base Methods
	bool set(const StringName& p_name, const Variant& p_value) override;
	bool get(const StringName& p_name, Variant& r_ret) const override;
	const GDExtensionPropertyInfo* get_property_list(uint32_t* r_count) const override;
	void free_property_list(const GDExtensionPropertyInfo* p_list, uint32_t p_count) const override;
	Variant::Type get_property_type(const StringName& p_name, bool* r_is_valid) const override;
	bool validate_property(GDExtensionPropertyInfo& p_property) const override;
	bool property_can_revert(const StringName& p_name) const override;
	bool property_get_revert(const StringName& p_name, Variant& r_ret) const override;
	Object* get_owner() override;
	void get_property_state(GDExtensionScriptInstancePropertyStateAdd p_add_func, void* p_userdata) override;
	const GDExtensionMethodInfo* get_method_list(uint32_t* r_count) const override;
	void free_method_list(const GDExtensionMethodInfo* p_list, uint32_t p_count) const override;
	bool has_method(const StringName& p_method) const override;
	int get_method_argument_count(const StringName& p_method, bool* r_is_valid = nullptr) const override;
	Variant callp(const StringName& p_method, const Variant** p_args, int p_argcount, GDExtensionCallError& r_error) override;
	void notification(int p_notification, bool p_reversed) override;
	String to_string(bool* r_valid) override;
	void refcount_incremented() override;
	bool refcount_decremented() override;
	Ref<Script> get_script() const override;
	bool is_placeholder() const override;
	void property_set_fallback(const StringName& p_name, const Variant& p_value, bool* r_valid) override;
	Variant property_get_fallback(const StringName& p_name, bool* r_valid) override;
	ScriptLanguage* _get_language() override;

public:
	// Events
	void OnScriptReloaded();

	// Initializer/Destructor
	BladeScriptInstance(Object* p_owner, const Ref<BladeScript> p_script);
	~BladeScriptInstance();
};

// Blade Resource Loader Difinition
class BladeResourceLoader : public ResourceFormatLoader
{
	GDCLASS(BladeResourceLoader, ResourceFormatLoader);

protected:
	static void _bind_methods() {}

public:
	static void init();
	static void deinit();
	virtual Variant _load(const String& path, const String& original_path, bool use_sub_threads, int32_t cache_mode) const override;
	virtual PackedStringArray _get_recognized_extensions() const override;
	virtual bool _handles_type(const StringName& type) const override;
	virtual String _get_resource_type(const String& p_path) const override;
};

// Blade Resource Saver Difinition
class BladeResourceSaver : public ResourceFormatSaver
{
	GDCLASS(BladeResourceSaver, ResourceFormatSaver);

protected:
	static void _bind_methods() {}

public:
	static void init();
	static void deinit();
	virtual Error _save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags) override;
	virtual Error _set_uid(const String& p_path, int64_t p_uid) override;
	virtual bool _recognize(const Ref<Resource>& p_resource) const override;
	virtual PackedStringArray _get_recognized_extensions(const Ref<Resource>& p_resource) const override;
	virtual bool _recognize_path(const Ref<Resource>& p_resource, const String& p_path) const override;
};

// Blade Event Wrapper
class BladeEventWrapper : public RefCounted
{
	GDCLASS(BladeEventWrapper, RefCounted);

protected:
	static void _bind_methods() {}

public:
	// Default
	BladeEventWrapper() {}

	// Instance Events
	BladeEventWrapper(BladeScriptInstance* _instance) : instance(_instance) {}
	void OnScriptReloaded() { instance->OnScriptReloaded(); }

private:
	BladeScriptInstance* instance = nullptr;
};

// Blade Editor Plugin Difinition
class BladePlugin : public EditorPlugin
{
	GDCLASS(BladePlugin, EditorPlugin);

protected:
	static void _bind_methods();

public:
	void _enter_tree() override;
	void _exit_tree() override;

public:
	// Methods
	static BladePlugin* GetSingleton() { return signleton; }
	Ref<EditorDebuggerPlugin> GetDebugger() const;

	// Events
	void HandleAction(const String& action);

private:
	static inline BladePlugin* signleton = nullptr;
};

// Blade Extension Metadata
struct BladeExtension
{
	std::string name;
	std::string kind;
	std::string receiverType;
	std::string returnType;
	bool isGeneric;

	std::vector<std::string> paramTypes;
	std::vector<std::string> paramNames;
	int paramCount = 0;

	union {
		Variant(*callv)(Variant, int, Variant*);
		Variant(*callo)(void*, int, Variant*);
	} fn;
};