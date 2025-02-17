
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
#pragma once

// String Resources
#define APP_NAME						"Jenova Runtime for Godot Engine"
#define APP_COMPANYNAME					"MemarDesign™ LLC."
#define APP_DESCRIPTION					"Real-Time C++ Scripting System for Godot Game Engine, Developed By Hamid.Memar."
#define APP_COPYRIGHT					"Copyright MemarDesign™ LLC. (©) 2024-2025, All Rights Reserved."
#define APP_VERSION						"0.3.6.5"
#define APP_VERSION_MIDDLEFIX			" "
#define APP_VERSION_POSTFIX				"Alpha"
#define APP_VERSION_SINGLECHAR			"a"
#define APP_VERSION_DATA				0, 3, 6, 5
#define APP_VERSION_BUILD				"0"
#define APP_VERSION_NAME				"Dragon"

#ifndef NO_JENOVA_RUNTIME_SDK

// Define Target Platform
#if defined(_WIN64) || defined(_WIN32)
#define TARGET_PLATFORM_WINDOWS 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::Windows
#define APP_ARCH "Win64"
#elif defined(__linux__)
#define TARGET_PLATFORM_LINUX 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::Linux
#define APP_ARCH "Linux64"
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define TARGET_PLATFORM_IOS 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::iOS
#define APP_ARCH "iOS"
#elif TARGET_OS_MAC
#define TARGET_PLATFORM_MACOS 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::MacOS
#define APP_ARCH "MacOS"
#endif
#elif defined(__ANDROID__)
#define TARGET_PLATFORM_ANDROID 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::Android
#define APP_ARCH "Android"
#elif defined(__EMSCRIPTEN__)
#define TARGET_PLATFORM_WEB 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::Web
#define APP_ARCH "Web"
#else
#define TARGET_PLATFORM_UNKNOWN 1
#define TARGET_PLATFORM_CURRENT jenova::TargetPlatform::Unknown
#define APP_ARCH "Unknown"
#endif

// Jenova API Import/Export
#if defined(_WIN32) || defined(_WIN64)
#define JENOVA_API_EXPORT __declspec(dllexport)
#define JENOVA_API_IMPORT __declspec(dllimport)
#else
#define JENOVA_API_EXPORT __attribute__((visibility("default")))
#define JENOVA_API_IMPORT 
#endif

// Windows SDK
#ifdef TARGET_PLATFORM_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#include <psapi.h>
#endif

// Linux SDK
#ifdef TARGET_PLATFORM_LINUX
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <utime.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <link.h>
#include <cxxabi.h>
#endif

// C++ SDK
#include <stddef.h>
#include <stdarg.h>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <thread>
#include <regex>
#include <string>
#include <cstring>
#include <vector>
#include <random>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <filesystem>

// Godot SDK
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/font_variation.hpp>
#include <godot_cpp/classes/resource_importer_dynamic_font.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_mouse.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/property_tweener.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/shortcut.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/menu_bar.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/editor_plugin_registration.hpp>
#include <godot_cpp/classes/editor_export_platform.hpp>
#include <godot_cpp/classes/editor_export_plugin.hpp>
#include <godot_cpp/classes/editor_import_plugin.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/editor_debugger_session.hpp>
#include <godot_cpp/classes/editor_debugger_plugin.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_scene_post_import_plugin.hpp>
#include <godot_cpp/classes/editor_resource_conversion_plugin.hpp>
#include <godot_cpp/classes/editor_feature_profile.hpp>
#include <godot_cpp/classes/packed_data_container.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/grid_container.hpp>
#include <godot_cpp/classes/box_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/sub_viewport_container.hpp>
#include <godot_cpp/classes/tab_bar.hpp>
#include <godot_cpp/templates/list.hpp>
#include <godot_cpp/templates/pair.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/classes/reg_ex_match.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_format_saver.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/list.hpp>
#include <godot_cpp/templates/pair.hpp>
#include <godot_cpp/templates/self_list.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/file_dialog.hpp>
#include <godot_cpp/classes/hashing_context.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/placeholder_texture2d.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/script_editor.hpp>
#include <godot_cpp/classes/script_editor_base.hpp>
#include <godot_cpp/classes/script_extension.hpp>
#include <godot_cpp/classes/script_language.hpp>
#include <godot_cpp/classes/script_language_extension.hpp>
#include <godot_cpp/classes/confirmation_dialog.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Shared Third-Party
#include <Parsers/json.hpp>
#include <Base64/base64.hpp>

// Namespaces Import
using namespace godot;

// Logging Macros
#define jenova_log(fmt,...)					printf(fmt "\n", ##__VA_ARGS__);

// Helper Macros
#define JENOVA_API							extern "C" JENOVA_API_EXPORT
#define FUNCTION_CHECK						jenova::Output("%s | %p", __FUNCSIG__, this);
#define LINE_CHECK							jenova::Output("%d", __LINE__);
#define LINE_CHECK_THIS						jenova::Output("%d | %p", __LINE__, this);
#define AS_STD_STRING(gstr)					(*jenova::ConvertToStdString(gstr).str)
#define AS_C_STRING(gstr)					((*jenova::ConvertToStdString(gstr).str).c_str())
#define AS_STD_WSTRING(gstr)				(*jenova::ConvertToWideStdString(gstr).wstr)
#define EDITOR_MENU_ID(id)					int32_t(jenova::EditorMenuID::id)
#define BUFFER_PTR_SIZE_PARAM(buffer)		buffer, sizeof(buffer)
#define JENOVA_RESOURCE(key)				jenova::resources::key
#define CODE_TEMPLATE(id)					String(std::string(jenova::templates::id, sizeof(jenova::templates::id)).c_str())
#define VALIDATE_FUNCTION(func)				if (!func) { jenova::Output("System Failure : %d", __LINE__); jenova::ExitWithCode(__LINE__); }
#define CREATE_SVG_MENU_ICON(buffer)		jenova::CreateMenuItemIconFromByteArray(BUFFER_PTR_SIZE_PARAM(buffer), jenova::ImageCreationFormat::SVG)
#define CREATE_PNG_MENU_ICON(buffer)		jenova::CreateMenuItemIconFromByteArray(BUFFER_PTR_SIZE_PARAM(buffer), jenova::ImageCreationFormat::PNG)
#define CREATE_GLOBAL_TEMPLATE(a,b,c)		JenovaTemplateManager::get_singleton()->RegisterNewGlobalScriptTemplate(a, CODE_TEMPLATE(b), c);
#define CREATE_CLASS_TEMPLATE(a,b,c,d)		JenovaTemplateManager::get_singleton()->RegisterNewClassScriptTemplate(a, b, CODE_TEMPLATE(c), d);
#define QUERY_ENGINE_MODE(mode)				(jenova::GlobalStorage::CurrentEngineMode == jenova::EngineMode::mode)
#define QUERY_SDK_LINKING_MODE(mode)		(jenova::GlobalStorage::SDKLinkingMode == jenova::SDKLinkingMode::mode)
#define QUERY_PLATFORM(platform)			(TARGET_PLATFORM_CURRENT == jenova::TargetPlatform::platform)
#define SCALED(value)						(value * scaleFactor)

// Helper Markers
#define InParam
#define OutParam

// Jenova Namespace
namespace jenova
{
	// Forward Declarations
	struct ScriptModule;
	struct JenovaPackage;
	struct AddonConfig;

	// Type Definitions
	typedef void* GenericHandle;
	typedef void* ModuleHandle;
	typedef void* WindowHandle;
	typedef void* FileHandle;
	typedef String ScriptIdentifier;
	typedef uint32_t CompilerFeatures;
	typedef uint32_t LoaderFlags;
	typedef std::string RootPath;
	typedef std::string EncodedData;
	typedef std::string DecodedData;
	typedef std::string SerializedData;
	typedef std::vector<jenova::ScriptModule> ModuleList;
	typedef PackedStringArray HeaderList;
	typedef std::vector<std::string> ArgumentsArray;
	typedef std::vector<std::string> FunctionList;
	typedef std::vector<std::string> ParameterTypeList;
	typedef std::vector<std::string> IdentityList;
	typedef std::vector<std::filesystem::path> PathList;
	typedef std::vector<std::string> FileList;
	typedef std::vector<std::string> DirecotryList;
	typedef std::vector<std::string> TokenList;
	typedef std::vector<void*> PointerList;
	typedef std::vector<JenovaPackage> PackageList;
	typedef std::vector<size_t> IndexList;
	typedef std::vector<uint8_t> MemoryBuffer;
	typedef std::vector<AddonConfig> InstalledAddons;
	typedef std::string StringBuffer;
	typedef std::unordered_map<std::string, void*> PointerStorage;
	typedef Vector<Ref<Resource>> ResourceCollection;
	typedef intptr_t ModuleAddress;
	typedef intptr_t FunctionAddress;
	typedef intptr_t PropertyAddress;
	typedef uint64_t LongWord;
	typedef uint16_t TaskID;
	typedef std::function<void()> TaskFunction;
	typedef void(*VoidFunc_t)();
	typedef struct { uint32_t LowDateTime, HighDateTime; } FileTime;
	typedef struct SmartString { std::string* str; ~SmartString() { if (str) delete str; }} SmartString;
	typedef struct SmartWstring { std::wstring* wstr; ~SmartWstring() { if (wstr) delete wstr; }} SmartWstring;
	typedef void* FunctionPointer;
	typedef void* PropertyPointer;

	// Enumerators
	enum class TargetPlatform
	{
		Windows,
		Linux,
		MacOS,
		Android,
		iOS,
		Web,
		Unknown
	};
	enum class EngineMode
	{
		Editor,
		Debug,
		Runtime,
		Unknown
	};
	enum class ModuleLoadStage
	{
		LoadModuleAtRuntimeStart,
		LoadModuleAtInitialization,
		LoadModuleManually
	};
	enum class ImageCreationFormat
	{
		PNG,
		JPG,
		SVG,
	};
	enum class BuildToolButtonPlacement
	{
		BeforeMenu,
		AfterMenu,
		BeforeStage,
		AfterStage,
		BeforeRunBar,
		AfterRunbar,
		AfterRendeMethod
	};
	enum class CompilerModel
	{
		#ifdef TARGET_PLATFORM_WINDOWS
		MicrosoftCompiler,
		ClangLLVMCompiler,
		MinGWCompiler,
		MinGWClangCompiler,
		#endif
		#ifdef TARGET_PLATFORM_LINUX
		GNUCompiler,
		ClangCompiler,
		#endif
		Unspecified
	};
	enum class InterpreterBackend
	{
		AsmJIT,
		TinyCC,
		AkiraJIT,
		SecureAngel,
		Unknown
	};
	enum class BuildAndRunMode
	{
		RunOnBuildSuccess,
		BuildBeforeRun,
		DoNothing
	};
	enum class ChangesTriggerMode
	{
		TriggerOnScriptReload,
		TriggerOnScriptChange,
		TriggerOnWatchdogInvoke,
		DoNothing
	};
	enum class EditorVerboseOutput
	{
		StandardOutput,
		JenovaTerminal,
		Disabled
	};
	enum class ScriptModuleType
	{
		Unknown,
		UsedScript,
		UnusedScript,
		InternalScript,
		BuiltinScript,
		EntityScript,
		BuiltinEntityScript,
		BootstrapScript,
		EmbeddedScript // Reserved
	};
	enum class EditorMenuID
	{
		BuildSolution,
		RebuildSolution,
		CleanSolution,
		ConfigureBuild,
		ExportToVisualStudio,
		ExportToVisualStudioCode,
		ExportJenovaModule,
		DeveloperMode,
		ClearCacheDatabase,
		GenerateEncryptionKey,
		BackupCurrentEncryptionKey,
		OpenAddonExplorer,
		OpenScriptManager,
		OpenPackageManager,
		Documentation,
		DiscordServer,
		CheckForUpdates,
		AboutJenova,
		Unknown
	};
	enum class PackageType
	{
		Compiler,
		GodotKit,
		Library,
		SampleProject,
		CodeTemplate,
		Addon,
		Tool,
		All
	};
	enum class PackagePlatform
	{
		WindowsAMD64,
		LinuxAMD64,
		WindowsARM64,
		LinuxARM64,
		AndroidARM64,
		iOSARM64,
		MacOSARM64,
		Universal,
		Unknown
	};
	enum class PropertySetMethod
	{
		MemoryCopy,
		DirectAssign
	};
	enum class ModuleCallMode
	{
		Actual,
		Virtual
	};
	enum class SDKLinkingMode
	{
		None,
		Dynamically,
		Statically
	};
	enum class ModuleCacheType : short
	{
		Proprietary						= 0x5250,
		OpenSource						= 0x534F,
		Unknown							= 0x0000,
	};
	enum class SymbolSignatureType
	{
		FunctionSymbol,
		PropertySymbol,
		UnknownSymbol
	};

	// Flags
	enum CompilerFeature : CompilerFeatures
	{
		CanCompileFromSourceCode		= 0x01 << 0,
		CanCompileFromFile				= 0x01 << 1,
		CanLinkObjectFiles				= 0x01 << 2,
		CanGenerateMappingData			= 0x01 << 3,
		CanGenerateModule				= 0x01 << 4
	};
	enum LoaderFlag : LoaderFlags
	{
		LoadInDebugMode					= 0x01 << 0,
		InitializeProtector				= 0x01 << 1,
	};


	// Structures
	struct ScriptModule
	{
		String scriptUID;
		String scriptFilename;
		String scriptCacheFile;
		String scriptObjectFile;
		String scriptPropertiesFile;
		String scriptSource;
		String scriptHash;
		ScriptModuleType scriptType = ScriptModuleType::Unknown;
	};
	struct ScriptModuleContainer
	{
		ScriptModule scriptModule;
		ModuleList scriptModules;
		ScriptModuleContainer(const ScriptModule& _scriptModule, const ModuleList& _scriptModules) : scriptModule(_scriptModule), scriptModules(_scriptModules) {}
		ScriptModuleContainer(const ModuleList& _scriptModules) : scriptModules(_scriptModules), scriptModule() {}
	};
	struct ScriptEntityContainer
	{
		RootPath rootPath;
		ModuleList scriptModules;

		FileList scriptFilesFullPath;
		FileList scriptFilesReleative;
		IdentityList scriptIdentities;
		IndexList entityDirectoryIndex;
		size_t entityCount = 0;
		size_t rootedCount = 0;
		size_t builtinCount = 0;

		DirecotryList scriptDirectoriesFullPath;
		DirecotryList scriptDirectoriesReleative;
		size_t directoryCount = 0;
	};
	struct ScriptProperty
	{
		String propertyName;
		String ownerScriptUID;
		PropertyInfo propertyInfo;
		Variant defaultValue;
	};
	struct ScriptPropertyContainer
	{
		String scriptUID;
		Vector<ScriptProperty> scriptProperties;
	};
	struct ScriptFileState
	{
		bool isValid = false;
		FileTime creationTime;
		FileTime accessTime;
		FileTime writeTime;
	};
	struct CompileResult
	{
		bool hasError = false;
		bool compileResult = false;
		String compileWarnings = "";
		String compileError = "";
		String compileVerbose = "";
		int scriptsCount = 0;
	};
	struct BuildResult
	{
		bool hasError = false;
		bool buildResult = false;
		String buildWarnings = "";
		String buildError = "";
		String buildVerbose = "";
		SerializedData moduleMetaData;
		std::vector<uint8_t> builtModuleData;
		std::string buildPath;
		CompilerModel compilerModel = CompilerModel::Unspecified;
		bool hasDebugInformation = false;
	};
	struct ModuleDatabaseHeader
	{
		const unsigned char magicNumber[16]		= { 0x5F, 0x5F, 0x4A, 0x45, 0x4E, 0x4F, 0x56, 0x41, 0x5F, 0x43, 0x41, 0x43, 0x48, 0x45, 0x5F, 0x5F };
		size_t moduleSize						= 0;
		size_t metaDataSize						= 0;
		size_t encodedDataSize					= 0;
		float compressionRatio					= 100.0f;
		ModuleCacheType databaseType			= ModuleCacheType::Unknown;
		unsigned char databaseVersion[4]		= { 0 };
		unsigned char reserved[14]				= { 0 };
	};
	struct ScriptCaller
	{
		const void* self;

		// Initializer
		ScriptCaller(const void* _self) : self(_self) {}
	};
	struct ExtensionInitializerData
	{
		GDExtensionInterfaceGetProcAddress		godotGetProcAddress;
		GDExtensionClassLibraryPtr				godotExtensionClassLibraryPtr;
		GDExtensionInitialization*				godotExtensionInitialization;
	};
	struct VisualStudioInstance
	{
		String instanceName		= "";
		String instanceVersion	= "";
		String platformToolset	= "";
		String majorVersion		= "";
		String productName		= "";
		String productYear		= "";
	};
	struct JenovaPackage
	{
		// Elements
		String				pkgName;
		String				pkgVersion;
		String				pkgDescription;
		String				pkgHash;
		Ref<ImageTexture>	pkgImage;
		PackageType			pkgType;
		PackagePlatform		pkgPlatform;
		uint32_t			pkgSize;
		String				pkgDate;
		String				pkgURL;
		String				pkgDestination;
		bool				pkgInstallScript;
		bool				pkgUninstallScript;

		// Operators
		bool operator==(const JenovaPackage& other) const
		{
			return pkgHash == other.pkgHash;
		}
	};
	struct AddonConfig
	{
		// Parsed Configurations
		std::string Name;
		std::string Version;
		std::string License;
		std::string Type;
		std::string Arch;
		std::string Header;
		std::string Binary;
		std::string Library;
		std::string Dependencies;
		std::string Path;
		bool Global;
		bool AutoLoad;

		// Serialized Data
		SerializedData Data;
	};

	// Global Settings
	namespace GlobalSettings
	{
		constexpr ModuleInitializationLevel PluginInitializationLevel(MODULE_INITIALIZATION_LEVEL_SCENE);

		constexpr bool VerboseEnabled							= false;
		constexpr bool ScriptingEnabled							= true;
		constexpr bool BuildInternalSources						= true;
		constexpr bool SafeExitOnPluginUnload					= true;
		constexpr bool HandlePreLaunchErrors					= true;
		constexpr bool AskAboutOpeningVisualStudio				= true;
		constexpr bool AskAboutOpeningVSCode					= true;
		constexpr bool CreateSymbolicAddonModules				= true;
		constexpr bool CopyRuntimeModuleOnExport				= true;
		constexpr bool RespectSourceFilesEncoding				= true;
		constexpr bool RegisterGlobalCrashHandler				= false;
		constexpr bool CreateDumpOnExecutionCrash				= false;
		constexpr bool UpdatePropertiesAfterCall				= true;
		constexpr bool DisableBuildAndRunWhileDebug				= true;

		constexpr size_t PrintOutputBufferSize					= 8192;
		constexpr size_t BuildOutputBufferSize					= PrintOutputBufferSize;
		constexpr size_t FormatBufferSize						= 4096;
		constexpr size_t ScriptReloadCooldown					= 200;
		constexpr size_t ScriptChangeCooldown					= 200;

		constexpr char* JenovaRuntimeModuleName					= "Jenova.Runtime";
		constexpr char* JenovaScriptExtension					= "cpp";
		constexpr char* JenovaHeaderExtension					= "hpp";
		constexpr char* JenovaScriptType						= "CPPScript";
		constexpr char* JenovaHeaderType						= "CPPHeader";
		constexpr char* JenovaCacheDirectory					= "/Jenova_Cache/";
		constexpr char* ScriptToolIdentifier					= "JENOVA_TOOL_SCRIPT";
		constexpr char* ScriptBlockBeginIdentifier				= "JENOVA_SCRIPT_BEGIN";
		constexpr char* ScriptBlockEndIdentifier				= "JENOVA_SCRIPT_END";
		constexpr char* ScriptVMBeginIdentifier					= "JENOVA_VM_BEGIN";
		constexpr char* ScriptVMEndIdentifier					= "JENOVA_VM_END";
		constexpr char* ScriptSignalCallbackIdentifier			= "JENOVA_CALLBACK";
		constexpr char* ScriptPropertyIdentifier				= "JENOVA_PROPERTY";
		constexpr char* ScriptClassNameIdentifier				= "JENOVA_CLASS_NAME";
		constexpr char* DefaultModuleDatabaseFile				= "JenovaRuntime.jdb";
		constexpr char* DefaultModuleConfigFile					= "JenovaRuntime.cfg";
		constexpr char* DefaultJenovaBootPath					= "res://J.E.N.O.V.A/";
		constexpr char* JenovaModuleBootEventName				= "JenovaBoot";
		constexpr char* JenovaModuleShutdownEventName			= "JenovaShutdown";
		constexpr char* JenovaBuildCacheDatabaseFile			= "Jenova.Build.json";
		constexpr char* JenovaInstalledPackagesFile				= "Jenova.Runtime.Packages.json";
		constexpr char* JenovaGodotSDKHeaderCacheFile			= "GodotSDK.auto";
		constexpr char* JenovaConfigurationFile					= "Jenova.config";
		constexpr char* VisualStudioSolutionFile				= "Jenova.Framework.sln";
		constexpr char* VisualStudioProjectFile					= "Jenova.Module.vcxproj";
		constexpr char* VisualStudioWatchdogFile				= "Jenova.VisualStudio.jwd";
		constexpr char* JenovaPackageDatabaseURL				= "https://raw.githubusercontent.com";
		constexpr char* JenovaPackageRepositoryPath				= "Jenova/Packages/";

		constexpr int JenovaTerminalLogFontSize					= 12;

		constexpr char JenovaBuildVersion[4]					= { APP_VERSION_DATA };

		constexpr ModuleLoadStage DefaultModuleLoadStage		= ModuleLoadStage::LoadModuleAtInitialization;
	}

	// Global Storage
	namespace GlobalStorage
	{
		extern ExtensionInitializerData							ExtensionInitData;
		extern jenova::EngineMode								CurrentEngineMode;
		extern jenova::BuildAndRunMode							CurrentBuildAndRunMode;
		extern jenova::ChangesTriggerMode						CurrentChangesTriggerMode;
		extern jenova::EditorVerboseOutput						CurrentEditorVerboseOutput;
		extern jenova::SDKLinkingMode							SDKLinkingMode;
		extern std::string										CurrentJenovaCacheDirectory;
		extern std::string										CurrentJenovaGeneratedConfiguration;
		extern std::string										CurrentJenovaRuntimeModulePath;
		extern bool												DeveloperModeActivated;
		extern bool												UseHotReloadAtRuntime;
		extern bool												UseMonospaceFontForTerminal;
		extern bool												UseManagedSafeExecution;
		extern int												TerminalDefaultFontSize;
	}

	// Error Codes
	namespace ErrorCode
	{
		constexpr int RUNTIME_INIT_FAILED						= 0xE0C0;
		constexpr int RUNTIME_START_FAILED						= 0xE0C1;
		constexpr int RUNTIME_DEINIT_FAILED						= 0xE0C2;
		constexpr int INTERPRETER_INIT_FAILED					= 0xE0C3;
	}

	// Operating System Abstraction Layer
	#pragma region JenovaOS
	jenova::ModuleHandle LoadModule(const char* libPath);
	bool ReleaseModule(jenova::ModuleHandle moduleHandle);
	void* GetModuleFunction(jenova::ModuleHandle moduleHandle, const char* functionName);
	bool SetWindowState(jenova::WindowHandle windowHandle, bool windowState);
	int ShowMessageBox(const char* msg, const char* title, int flags);
	bool RunFile(const char* filePath);
	bool OpenURL(const char* url);
	void* AllocateMemory(size_t memorySize);
	void* RelocateMemory(void* dest, const void* src, std::size_t count);
	bool FreeMemory(void* memoryPtr);
	int GetEnvironmentEntity(const char* entityName, char* bufferPtr, size_t bufferSize);
	bool SetEnvironmentEntity(const char* entityName, const char* entityValue);
	bool AddEnvironmentPath(const char* path, const char* pathCollection);
	jenova::GenericHandle GetCurrentProcessHandle();
	bool CreateSymbolicFile(const char* srcFile, const char* dstFile);
	int ExecuteCommand(const std::string& app, const std::string& command);
	void ExitWithCode(int exitCode);
	#pragma endregion

	// Utilities & Helpers
	#pragma region JenovaUtilities
	void Alert(const char* fmt, ...);
	std::string Format(const char* fmt, ...);
	std::string FormatSafe(const char* fmt, ...);
	void Output(const char* fmt, ...);
	void Output(const wchar_t* fmt, ...);
	void OutputColored(const char* colorHash, const char* fmt, ...);
	void Verbose(const char* fmt, ...);
	void VerboseByID(int id, const char* fmt, ...);
	void Error(const char* stageName, const char* fmt, ...);
	void Warning(const char* stageName, const char* fmt, ...);
	void ErrorMessage(const char* title, const char* fmt, ...);
	jenova::SmartString ConvertToStdString(const godot::String& gstr);
	jenova::SmartString ConvertToStdString(const godot::StringName& gstr);
	jenova::SmartWstring ConvertToWideStdString(const godot::String& gstr);
	std::string GetNameFromPath(godot::String gstr);
	String GenerateStandardUIDFromPath(String resourcePath);
	String GenerateStandardUIDFromPath(Resource* resourcePtr);
	std::string GenerateRandomHashString();
	std::string GenerateTerminalLogTime();
	jenova::EngineMode GetCurrentEngineInstanceMode();
	String GetCurrentEngineInstanceModeAsString();
	Ref<ImageTexture> CreateImageTextureFromByteArray(const uint8_t* imageDataPtr, size_t imageDataSize, ImageCreationFormat imageFormat = ImageCreationFormat::PNG);
	Ref<ImageTexture> CreateImageTextureFromByteArrayEx(const uint8_t* imageDataPtr, size_t imageDataSize, const Vector2i& imageSize = Vector2i(), ImageCreationFormat imageFormat = ImageCreationFormat::PNG);
	Ref<ImageTexture> CreateMenuItemIconFromByteArray(const uint8_t* imageDataPtr, size_t imageDataSize, ImageCreationFormat imageFormat = ImageCreationFormat::PNG);
	Ref<FontFile> CreateFontFileFromByteArray(const uint8_t* fontDataPtr, size_t fontDataSize);
	bool CollectResourcesFromFileSystem(const String& rootPath, const String& extensions, jenova::ResourceCollection& collectedResources, bool respectGDIgnore = true);
	bool CollectScriptsFromFileSystemAndScenes(const String& rootPath, const String& extension, jenova::ResourceCollection& collectedResources, bool respectGDIgnore = true);
	void RegisterDocumentationFromByteArray(const char* xmlDataPtr, size_t xmlDataSize);
	void CopyStringToClipboard(const String& str);
	String GetStringFromClipboard();
	void CopyStdStringToClipboard(const std::string& str);
	std::string GetStdStringFromClipboard();
	ArgumentsArray CreateArgumentsArrayFromString(const std::string& str, char delimiter);
	std::string GetExecutablePath();
	void ResetCurrentDirectoryToRoot();
	void DoApplicationEvents();
	bool QueueProjectBuild(bool deferred = true);
	bool UpdateGlobalStorageFromEditorSettings();
	std::string GetNotificationString(int p_what);
	String GetJenovaCacheDirectory();
	String GetJenovaProjectDirectory();
	String RemoveCommentsFromSource(const String& sourceCode);
	bool ContainsExactString(const String& srcStr, const String& matchStr);
	std::string GetDemangledFunctionSignature(std::string mangledName, CompilerModel compilerModel);
	std::string CleanFunctionAndPropertySignature(const std::string& functionSignature, jenova::CompilerModel compilerModel);
	ParameterTypeList ExtractParameterTypesFromSignature(const std::string& functionSignature, jenova::CompilerModel compilerModel);
	std::string ExtractReturnTypeFromSignature(const std::string& functionSignature, jenova::CompilerModel compilerModel);
	std::string ExtractPropertyTypeFromSignature(const std::string& propertySignature, jenova::CompilerModel compilerModel);
	jenova::SymbolSignatureType DetectSymbolSignatureType(const std::string& symbolSignature, jenova::CompilerModel compilerModel);
	bool LoadSymbolForModule(jenova::GenericHandle process, jenova::LongWord baseAddress, const std::string& pdbPath, size_t dllSize);
	bool InitializeExtensionModule(const char* initFuncName, jenova::ModuleHandle moduleBase, jenova::ModuleCallMode callType);
	bool CallModuleEvent(const char* eventFuncName, jenova::ModuleHandle moduleBase, jenova::ModuleCallMode callType);
	ScriptModule CreateScriptModuleFromInternalSource(const std::string& sourceName, const std::string& sourceCode);
	bool CreateFileFromInternalSource(const std::string& sourceFile, const std::string& sourceCode);
	bool CreateBuildCacheDatabase(const std::string& cacheFile, const ModuleList& scriptModules, const jenova::HeaderList& scriptHeaders, bool skipHashes = false);
	std::string GetLoadedModulePath(jenova::ModuleHandle moduleHandle);
	MemoryBuffer CompressBuffer(void* bufferPtr, size_t bufferSize);
	MemoryBuffer DecompressBuffer(void* bufferPtr, size_t bufferSize);
	float CalculateCompressionRatio(size_t baseSize, size_t compressedSize);
	Ref<Texture2D> GetEditorIcon(const String& iconName);
	bool DumpThemeColors(const Ref<Theme> theme);
	ArgumentsArray ProcessDeployerArguments(const std::string& cmdLine);
	bool WriteStringToFile(const String& filePath, const String& str);
	String ReadStringFromFile(const String& filePath);
	bool WriteStdStringToFile(const std::string& filePath, const std::string& str);
	std::string ReadStdStringFromFile(const std::string& filePath);
	bool WriteWideStdStringToFile(const std::wstring& filePath, const std::wstring& str);
	std::wstring ReadWideStdStringFromFile(const std::wstring& filePath);
	void ReplaceAllMatchesWithString(std::string& targetString, const std::string& from, const std::string& to);
	std::string ReplaceAllMatchesWithStringAndReturn(std::string targetString, const std::string& from, const std::string& to);
	ArgumentsArray SplitStdStringToArguments(const std::string& str, char delimiter = ';');
	ScriptEntityContainer CreateScriptEntityContainer(const String& rootPath);
	std::string GenerateFilterUniqueIdentifier(std::string& filterName, bool addBrackets = false);
	bool CompareFilePaths(const std::string& sourcePath, const std::string& destinationPath);
	bool RemoveFileEncodingInStdString(std::string& fileContent);
	bool ApplyFileEncodingFromReferenceFile(const std::string& sourceFile, const std::string& destinationFile);
	EncodedData CreateCompressedBase64FromStdString(const std::string& srcStr);
	std::string CreateStdStringFromCompressedBase64(const EncodedData& base64);
	bool WriteMemoryBufferToFile(const std::string& filePath, const MemoryBuffer& memoryBuffer);
	MemoryBuffer ReadMemoryBufferFromFile(const std::string& filePath);
	std::string ExtractMajorVersionFromFullVersion(const std::string& fullVersion);
	std::string GetVisualStudioPlatformToolsetFromVersion(const std::string& versionNumber);
	bool CreateFontFileDataPackageFromAsset(const String& assetPath, const String& packagePath);
	String CreateSecuredBase64StringFromString(const String& srcStr);
	String RetriveStringFromSecuredBase64String(const String& securedStr);
	jenova::WindowHandle GetWindowNativeHandle(const Window* targetWindow);
	jenova::WindowHandle GetMainWindowNativeHandle();
	bool AssignPopUpWindow(const Window* targetWindow);
	bool ReleasePopUpWindow(const Window* targetWindow);
	String FormatBytesSize(size_t byteSize);
	String GenerateMD5HashFromFile(const String& targetFile);
	jenova::PackageList GetInstalledAddonPackages();
	jenova::PackageList GetInstalledCompilerPackages(const jenova::CompilerModel& compilerModel);
	jenova::PackageList GetInstalledGodotKitPackages();
	jenova::InstalledAddons GetInstalledAddones();
	String GetInstalledCompilerPathFromPackages(const String& compilerIdentity, const jenova::CompilerModel& compilerModel);
	String GetInstalledGodotKitPathFromPackages(const String& godotKitIdentity);
	std::string ResolveVariantValueAsString(const Variant* variantValue, jenova::PointerList& ptrList);
	std::string ResolveVariantTypeAsString(const Variant* variantValue);
	std::string ResolveReturnTypeForJIT(const std::string& returnType);
	Variant* MakeVariantFromReturnType(Variant* variantPtr, const char* returnType);
	uint32_t GetPropertyEnumFlagFromString(const std::string enumFlagStr);
	jenova::SerializedData ProcessAndExtractPropertiesFromScript(OutParam std::string& scriptSource, const std::string& scriptUID);
	jenova::SerializedData ProcessAndExtractPropertiesFromScript(OutParam String& scriptSource, const String& scriptUID);
	Variant::Type GetVariantTypeFromStdString(const std::string& typeName);
	jenova::ScriptPropertyContainer CreatePropertyContainerFromMetadata(const jenova::SerializedData& propertyMetadata, const std::string& scriptUID);
	void CleanVariantTypeName(std::string& typeName);
	void* AllocateVariantBasedProperty(const std::string& typeName);
	bool SetPropertyPointerValueFromVariant(jenova::PropertyPointer propertyPointer, const Variant& variantValue);
	bool GetVariantFromPropertyPointer(const jenova::PropertyPointer propertyPointer, godot::Variant& variantValue, const Variant::Type& variantType);
	std::string ParseClassNameFromScriptSource(const std::string& sourceCode);
	jenova::ScriptFileState BackupScriptFileState(const std::string& scriptFilePath);
	bool RestoreScriptFileState(const std::string& scriptFilePath, const jenova::ScriptFileState& scriptFileState);
	void RandomWait(int minWaitTime, int maxWaitTime);
	void CopyAddonBinariesToEngineDirectory(bool createSymbolic = false);
	bool ExecutePackageScript(const std::string& packageScriptFile);
	bool ProcessCommandLineArguments();
	godot::SceneTree* GetSceneTree();
	std::string FindScriptPathFromPreprocessedFile(const std::string& preprocessedFile);
	bool RegisterRuntimeEventCallback(jenova::FunctionPointer runtimeCallback);
	bool UnregisterRuntimeEventCallback(jenova::FunctionPointer runtimeCallback);
	jenova::SerializedData GenerateRuntimeModuleConfiguration();
	jenova::SerializedData ObtainRuntimeModuleConfiguration();
	bool ResolveAndLoadAddonModulesAtRuntime();
	std::string CreateTemporaryModuleCache(const uint8_t* moduleDataPtr, const size_t moduleSize);
	bool ReleaseTemporaryModuleCache();
	std::string GetVisualStudioInstancesMetadata(std::string arguments);
	std::string GetRuntimeCompilerName();
	#pragma endregion

	// Crash Handlers
	#ifdef TARGET_PLATFORM_WINDOWS
	bool GenerateMiniMemoryDump(EXCEPTION_POINTERS* exceptionInfo);
	std::string GetExceptionDescription(EXCEPTION_POINTERS* exceptionInfo);
	static LONG WINAPI JenovaGlobalCrashHandler(EXCEPTION_POINTERS* exceptionInfo);
	LONG WINAPI JenovaExecutionCrashHandler(EXCEPTION_POINTERS* exceptionInfo);
	#endif
}

// Jenova Tools
#include "tiny_profiler.h"
#include "task_system.h"
#include "asset_monitor.h"
#include "package_manager.h"

// Jenova C++ Script Engine
#include "script_object.h"
#include "script_resource.h"
#include "script_templates.h"
#include "script_language.h"
#include "script_interpreter.h"
#include "script_instance_base.h"
#include "script_instance.h"
#include "script_manager.h"
#include "script_compiler.h"

// Jenova Exporters
#include "gdextension_exporter.h"

#endif // NO_JENOVA_RUNTIME_SDK
