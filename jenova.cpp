﻿
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

// Resources
#include "IconDatabase.h"
#include "BuiltinFonts.h"
#include "Documentation.h"
#include "JenovaIcon64.h"
#include "TypesIcons.h"
#include "AboutImage.h"

// Internal/Built-In Sources
#include "InternalSources.h"
#include "CodeTemplates.h"

// Internal/Built-In Templates
#include "VisualStudioTemplates.h"

// Third-Party
#include <Parsers/argparse.hpp>
#include <Zlib/zlib.h>

// Namespaces
using namespace std;

// Global Objects
HINSTANCE jenovaRuntimeInstance = nullptr;

// Entrypoint
static BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
	// Handle Events
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
		// Disable Thread Calls
		jenovaRuntimeInstance = hinstDLL;
        DisableThreadLibraryCalls(jenovaRuntimeInstance);

		// Update Module Path
		jenova::GlobalStorage::CurrentJenovaRuntimeModulePath = jenova::GetLoadedModulePath(jenovaRuntimeInstance);
    }

	// All Good
    return TRUE;
}

// Jenova Core Implementations
namespace jenova
{
	// Plugin Implementation
	namespace plugin
	{
		// Forward Declarations
		class JenovaEditorPlugin;
		class JenovaExportPlugin;
		class JenovaImportPlugin;
		class JenovaDebuggerPlugin;

		// Global Instances
		static JenovaEditorPlugin* jenovaEditorPlugin = nullptr;

		// Editor Plugins
		class JenovaEditorPlugin : public EditorPlugin 
		{
			GDCLASS(JenovaEditorPlugin, EditorPlugin);

		private:
			 String JenovaEditorPluginName								= "J.E.N.O.V.A";
			 String JenovaEditorSettingsCategory						= "JenovaSettings";
			 String RemoveSourcesFromBuildEditorConfigPath				= "jenova/remove_source_codes_from_build";
			 String CompilerModelConfigPath								= "jenova/compiler_model";
			 String MultiThreadedCompilationConfigPath					= "jenova/multi_threaded_compilation";
			 String GenerateDebugInformationConfigPath					= "jenova/generate_debug_information";
			 String InterpreterBackendConfigPath						= "jenova/interpreter_backend";
			 String BuildAndRunModeConfigPath							= "jenova/build_and_run_mode";
			 String PreprocessorDefinitionsConfigPath					= "jenova/preprocessor_definitions";
			 String AdditionalIncludeDirectoriesConfigPath				= "jenova/additional_include_directories";
			 String AdditionalLibraryDirectoriesConfigPath				= "jenova/additional_library_directories";
			 String AdditionalDependenciesConfigPath					= "jenova/additional_dependencies";
			 String ExternalChangesTriggerModeConfigPath				= "jenova/external_changes_trigger_mode";
			 String UseHotReloadAtRuntimeConfigPath						= "jenova/use_hot_reload_at_runtime";
			 String EditorVerboseOutputConfigPath						= "jenova/editor_verbose_output";
			 String UseMonospaceFontForTerminalConfigPath				= "jenova/use_monospace_font_for_terminal";
			 String TerminalDefaultFontSizeConfigPath					= "jenova/terminal_default_font_size";
			 String CompilerPackageConfigPath							= "jenova/compiler_package";
			 String GodotKitPackageConfigPath							= "jenova/godot_kit_package";
			 String BuildToolButtonEditorConfigPath						= "jenova/build_tool_button_placement";

		private:
			// Default Settings
			const jenova::BuildToolButtonPlacement BuildToolButtonDefaultPlacement = jenova::BuildToolButtonPlacement::AfterRunbar;
			const jenova::CompilerModel CompilerDefaultModel = jenova::CompilerModel::MicrosoftCompiler;
			const jenova::BuildAndRunMode BuildAndRunDefaultMode = jenova::BuildAndRunMode::DoNothing;
			const jenova::ChangesTriggerMode ExternalChangesDefaultTriggerMode = jenova::ChangesTriggerMode::DoNothing;
			const jenova::EditorVerboseOutput EditorVerboseDefaultOutput = jenova::EditorVerboseOutput::JenovaTerminal;
			const jenova::InterpreterBackend InterpreterBackendDefaultMode = jenova::InterpreterBackend::AsmJIT;

		private:
			// Internal Objects
			bool isEditorPluginInitialized = false;
			Ref<JenovaExportPlugin> exportPlugin;
			Ref<JenovaImportPlugin> importPlugin;
			Ref<JenovaDebuggerPlugin> debuggerPlugin;
			jenova::ModuleList scriptModules;
			jenova::IJenovaCompiler* jenovaCompiler = nullptr;
			Button* buildToolButton = nullptr;
			PopupMenu* jenovaMenu = nullptr;
			PopupMenu* toolsMenu = nullptr;
			Control* jenovaTerminal = nullptr;
			RichTextLabel* jenovaLogOutput = nullptr;
			Ref<Shortcut> developerModeShortcut;
			std::vector<VisualStudioInstance> vsInstances;

		protected:

			// Bind Methods
			static void _bind_methods() 
			{
				ClassDB::bind_static_method("JenovaEditorPlugin", D_METHOD("GetInstance"), &JenovaEditorPlugin::get_singleton);
				ClassDB::bind_method(D_METHOD("BuildProject"), &JenovaEditorPlugin::BuildProject);
				ClassDB::bind_method(D_METHOD("CleanProject"), &JenovaEditorPlugin::CleanProject);
				ClassDB::bind_method(D_METHOD("BootstrapModule"), &JenovaEditorPlugin::BootstrapModule);
				ClassDB::bind_method(D_METHOD("VerboseLog"), &JenovaEditorPlugin::VerboseLog);
				ClassDB::bind_method(D_METHOD("ClearLogs"), &JenovaEditorPlugin::ClearLogs);
			}
	
		public:

			// Singleton Handling
			static JenovaEditorPlugin* get_singleton()
			{
				return jenovaEditorPlugin;
			}

		public:

			// Plugin Implementation
			String _get_plugin_name() const override { return " " + JenovaEditorPluginName; }
			Ref<Texture2D> _get_plugin_icon() const override 
			{ 
				Vector2i iconSize(20, 20);
				iconSize *= EditorInterface::get_singleton()->get_editor_scale();
				return jenova::CreateImageTextureFromByteArrayEx(BUFFER_PTR_SIZE_PARAM(jenova::resources::PNG_JENOVA_ICON_64), iconSize);
			}
			bool _has_main_screen() const override { return false; }
			bool _handles(Object* p_object) const override { return false; }

			// Tree Events
			void _enter_tree() override 
			{
				// Initialize Editor Plugin Instance
				VALIDATE_FUNCTION(InitializeEditorPlugin());

				// Register Editor Settings
				VALIDATE_FUNCTION(RegisterEditorSettings());

				// Update Storage From Editor Settings
				VALIDATE_FUNCTION(UpdateStorageConfigurations());

				// Register Editor Terminal Panel
				VALIDATE_FUNCTION(RegisterEditorTerminalPanel());

				// Register Editor Plugin Events
				VALIDATE_FUNCTION(RegisterEditorPluginEvents());

				// Set Editor Feature Profile
				VALIDATE_FUNCTION(SetEditorFeatureProfile());

				// Register Export Plugins
				VALIDATE_FUNCTION(RegisterExportPlugins());

				// Register Import Plugins
				VALIDATE_FUNCTION(RegisterImportPlugins());

				// Register Debugger Plugins
				VALIDATE_FUNCTION(RegisterDebuggerPlugins());

				// Register Built-In Script Templates
				VALIDATE_FUNCTION(RegisterBuiltInScriptTemplates());

				// Initialize Custom Types Icons
				VALIDATE_FUNCTION(InitialzeClassesIcons());

				// Initialize Documentation
				VALIDATE_FUNCTION(InitializeDocumentation());

				// Initialize Editor Menu
				VALIDATE_FUNCTION(InitializeEditorMenu());

				// Create 'Build' Tool Button
				VALIDATE_FUNCTION(CreateBuildToolButton());

				// Register Asset Monitors
				VALIDATE_FUNCTION(RegisterAssetMonitors());

				// Update Flag
				isEditorPluginInitialized = true;

				// Verbose
				jenova::OutputColored("#2ebc78", "[b]J.E.N.O.V.A[/b] System Initialized. Version : %s%s%s / Arch : Win64", APP_VERSION, APP_VERSION_MIDDLEFIX, APP_VERSION_POSTFIX);
			}
			void _exit_tree() override
			{
				// Unregister Editor Plugin Events
				VALIDATE_FUNCTION(UnRegisterEditorPluginEvents());

				// Unregister Asset Monitors
				VALIDATE_FUNCTION(UnRegisterAssetMonitors());

				// Remove 'Build' Tool Button
				VALIDATE_FUNCTION(RemoveBuildToolButton());

				// Destroy Editor Menu
				VALIDATE_FUNCTION(DestroyEditorMenu());

				// Unregister Export Plugins
				VALIDATE_FUNCTION(UnRegisterExportPlugins());

				// Unregister Import Plugins
				VALIDATE_FUNCTION(UnRegisterImportPlugins());

				// Unregister Import Plugins
				VALIDATE_FUNCTION(UnRegisterDebuggerPlugins());

				// Unregister Editor Terminal Panel
				VALIDATE_FUNCTION(UnRegisterEditorTerminalPanel());

				// Uninitialize Editor Plugin Instance
				VALIDATE_FUNCTION(UninitializeEditorPlugin());
			}

			// Input Events
			void _unhandled_key_input(const Ref<InputEvent>& p_event) override
			{
				// Can Also Use : if (Input::get_singleton()->is_key_pressed(KEY_ALT))
				Ref<InputEventKey> key_event = p_event;
				if (key_event.is_valid())
				{
					if (key_event->is_pressed() && key_event->get_keycode() == KEY_ALT)
					{
						if (buildToolButton)
						{
							buildToolButton->set_tooltip_text("Clean Jenova Project");
							buildToolButton->set_button_icon(jenova::GetEditorIcon("Clear"));
						}
					}
					else if (!key_event->is_pressed() && key_event->get_keycode() == KEY_ALT)
					{
						if (buildToolButton)
						{
							buildToolButton->set_tooltip_text("Build Jenova Project");
							buildToolButton->set_button_icon(jenova::GetEditorIcon("PluginScript"));
						}
					}
				}
			}

			// Build Events
			bool _build() override
			{
				// Reset Environment Flag
				SetEnvironmentVariableA("JENOVA_PRE_LAUNCH_ERROR", "NO_ERROR");

				// Update Settings
				if (!UpdateStorageConfigurations())
				{
					// Set Environment Flag
					SetEnvironmentVariableA("JENOVA_PRE_LAUNCH_ERROR", "WRONG_CONFIGURATIONS");

					// Always Return True, We Handle It Using Environment Value
					return true;
				}

				// Build Project If Required
				if (jenova::GlobalStorage::CurrentBuildAndRunMode == jenova::BuildAndRunMode::BuildBeforeRun)
				{
					if (!BuildProject())
					{
						// Set Environment Flag
						SetEnvironmentVariableA("JENOVA_PRE_LAUNCH_ERROR", "BUILD_FAILED");

						// Always Return True, We Handle It Using Environment Value
						return true;
					}
				}

				// All Good
				return true;
			}

			// Scene Events
			void _clear() override
			{
				jenova::VerboseByID(__LINE__, "All States Have Been Cleared And Reset.");
			}
			void _apply_changes() override
			{
				jenova::VerboseByID(__LINE__, "All Pending State Changes Applied.");
			}

			// Methods
			bool InitializeEditorPlugin()
			{
				// Set Global Singleton
				jenovaEditorPlugin = this;

				// Connect Signals
				get_editor_interface()->get_editor_settings()->connect("settings_changed", callable_mp(this, &JenovaEditorPlugin::OnEditorSettingsChanged));
				get_editor_interface()->get_base_control()->connect("theme_changed", callable_mp(this, &JenovaEditorPlugin::OnEditorThemeChanged));

				// Verbose
				jenova::VerboseByID(__LINE__, "Editor Plugin Initialized At %p", jenovaEditorPlugin);

				// All Good
				return true;
			}
			bool UninitializeEditorPlugin()
			{
				// Unset Global Singleton
				jenovaEditorPlugin = nullptr;

				// Disconnect Signals
				get_editor_interface()->get_editor_settings()->disconnect("settings_changed", callable_mp(this, &JenovaEditorPlugin::OnEditorSettingsChanged));
				get_editor_interface()->get_base_control()->disconnect("theme_changed", callable_mp(this, &JenovaEditorPlugin::OnEditorThemeChanged));

				// Verbose
				jenova::VerboseByID(__LINE__, "Editor Plugin Uninitialized");

				// All Good
				return true;
			}
			bool RegisterEditorPluginEvents()
			{
				// Register Window Events
				godot::Window* root_window = get_tree()->get_root()->get_window();
				if (!root_window) return false;
				root_window->connect("focus_entered", callable_mp(this, &JenovaEditorPlugin::OnWindowGainedFocus));
				root_window->connect("focus_exited", callable_mp(this, &JenovaEditorPlugin::OnWindowLostFocus));

				// All Good
				return true;
			}
			bool UnRegisterEditorPluginEvents()
			{
				// Unregister Window Events
				godot::Window* root_window = get_tree()->get_root()->get_window();
				if (!root_window) return false;
				root_window->disconnect("focus_entered", callable_mp(this, &JenovaEditorPlugin::OnWindowGainedFocus));
				root_window->disconnect("focus_exited", callable_mp(this, &JenovaEditorPlugin::OnWindowLostFocus));

				// All Good
				return true;
			}
			bool SetEditorFeatureProfile()
			{
				// All Good
				return true;
			}
			bool RegisterExportPlugins()
			{
				// Add Export Plugin to Editor
				exportPlugin.instantiate();
				this->add_export_plugin(exportPlugin);

				// All Good
				return true;
			}
			bool UnRegisterExportPlugins()
			{
				// Remove Export Plugin to Editor
				this->remove_export_plugin(exportPlugin);
				exportPlugin.unref();

				// All Good
				return true;
			}
			bool RegisterImportPlugins()
			{
				// Add Import Plugin to Editor
				// importPlugin.instantiate();
				// this->add_import_plugin(importPlugin);

				// All Good
				return true;
			}
			bool UnRegisterImportPlugins()
			{
				// Remove Import Plugin to Editor
				// this->remove_import_plugin(importPlugin);
				// importPlugin.unref();

				// All Good
				return true;
			}
			bool RegisterDebuggerPlugins()
			{
				// Add Debugger Plugin to Editor
				debuggerPlugin.instantiate();
				this->add_debugger_plugin(debuggerPlugin);

				// All Good
				return true;
			}
			bool UnRegisterDebuggerPlugins()
			{
				// Remove Debugger Plugin to Editor
				 this->remove_debugger_plugin(debuggerPlugin);
				 debuggerPlugin.unref();

				// All Good
				return true;
			}
			bool RegisterEditorSettings() const
			{
				EditorInterface* editor_interface = EditorInterface::get_singleton();
				if (editor_interface) 
				{
					Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
					if (!editor_settings.is_null()) 
					{
						// Check if the Settings Already Exists, If Not Define Them
						if (!editor_settings->has_setting(RemoveSourcesFromBuildEditorConfigPath)) editor_settings->set(RemoveSourcesFromBuildEditorConfigPath, true);
						if (!editor_settings->has_setting(CompilerModelConfigPath)) editor_settings->set(CompilerModelConfigPath, int32_t(CompilerDefaultModel));
						if (!editor_settings->has_setting(MultiThreadedCompilationConfigPath)) editor_settings->set(MultiThreadedCompilationConfigPath, true);
						if (!editor_settings->has_setting(GenerateDebugInformationConfigPath)) editor_settings->set(GenerateDebugInformationConfigPath, true);
						if (!editor_settings->has_setting(InterpreterBackendConfigPath)) editor_settings->set(InterpreterBackendConfigPath, int32_t(InterpreterBackendDefaultMode));
						if (!editor_settings->has_setting(BuildAndRunModeConfigPath)) editor_settings->set(BuildAndRunModeConfigPath, int32_t(BuildAndRunDefaultMode));
						if (!editor_settings->has_setting(PreprocessorDefinitionsConfigPath)) editor_settings->set(PreprocessorDefinitionsConfigPath, "JENOVA_CUSTOM");
						if (!editor_settings->has_setting(AdditionalIncludeDirectoriesConfigPath)) editor_settings->set(AdditionalIncludeDirectoriesConfigPath, "");
						if (!editor_settings->has_setting(AdditionalLibraryDirectoriesConfigPath)) editor_settings->set(AdditionalLibraryDirectoriesConfigPath, "");
						if (!editor_settings->has_setting(AdditionalDependenciesConfigPath)) editor_settings->set(AdditionalDependenciesConfigPath, "");
						if (!editor_settings->has_setting(ExternalChangesTriggerModeConfigPath)) editor_settings->set(ExternalChangesTriggerModeConfigPath, int32_t(ExternalChangesDefaultTriggerMode));
						if (!editor_settings->has_setting(UseHotReloadAtRuntimeConfigPath)) editor_settings->set(UseHotReloadAtRuntimeConfigPath, true);
						if (!editor_settings->has_setting(EditorVerboseOutputConfigPath)) editor_settings->set(EditorVerboseOutputConfigPath, int32_t(EditorVerboseDefaultOutput));
						if (!editor_settings->has_setting(UseMonospaceFontForTerminalConfigPath)) editor_settings->set(UseMonospaceFontForTerminalConfigPath, true);
						if (!editor_settings->has_setting(TerminalDefaultFontSizeConfigPath)) editor_settings->set(TerminalDefaultFontSizeConfigPath, jenova::GlobalSettings::JenovaTerminalLogFontSize);
						if (!editor_settings->has_setting(CompilerPackageConfigPath)) editor_settings->set(CompilerPackageConfigPath, "Latest");
						if (!editor_settings->has_setting(GodotKitPackageConfigPath)) editor_settings->set(GodotKitPackageConfigPath, "Latest");
						if (!editor_settings->has_setting(BuildToolButtonEditorConfigPath)) editor_settings->set(BuildToolButtonEditorConfigPath, int32_t(BuildToolButtonDefaultPlacement));
				
						// Add the Setting Descriptions to The Editor Settings
						PropertyInfo RemoveSourcesFromBuildProperty(Variant::BOOL, RemoveSourcesFromBuildEditorConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(RemoveSourcesFromBuildProperty);
						editor_settings->set_initial_value(RemoveSourcesFromBuildEditorConfigPath, true, false);

						// Compiler Model Property
						PropertyInfo CompilerModelProperty(Variant::INT, CompilerModelConfigPath, 
							PropertyHint::PROPERTY_HINT_ENUM, "Microsoft Visual C++ (MSVC),LLVM CLang Toolchain,MinGW Standard",
							PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(CompilerModelProperty);
						editor_settings->set_initial_value(CompilerModelConfigPath, int32_t(CompilerDefaultModel), false);

						// Multi-Threaded Compilation Property
						PropertyInfo MultiThreadedCompilationProperty(Variant::BOOL, MultiThreadedCompilationConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(MultiThreadedCompilationProperty);
						editor_settings->set_initial_value(MultiThreadedCompilationConfigPath, true, false);

						// Generate Debug Information Property
						PropertyInfo CompilerGenerateDebugInformationProperty(Variant::BOOL, GenerateDebugInformationConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(CompilerGenerateDebugInformationProperty);
						editor_settings->set_initial_value(GenerateDebugInformationConfigPath, true, false);

						// Interpreter Backend Property
						PropertyInfo InterpreterBackendProperty(Variant::INT, InterpreterBackendConfigPath,
							PropertyHint::PROPERTY_HINT_ENUM, "NitroJIT (Fastest),Meteora (Experimental),A.K.I.R.A (Unavailable),AngelVM (Unavailable)",
							PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(InterpreterBackendProperty);
						editor_settings->set_initial_value(InterpreterBackendConfigPath, int32_t(InterpreterBackendDefaultMode), false);

						// Build And Run Mode Property
						PropertyInfo BuildAndRunModeProperty(Variant::INT, BuildAndRunModeConfigPath, 
							PropertyHint::PROPERTY_HINT_ENUM, "Run Game After Successful Build,Build Project Before Running Game,Don't Take Any Action",
							PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(BuildAndRunModeProperty);
						editor_settings->set_initial_value(BuildAndRunModeConfigPath, int32_t(BuildAndRunDefaultMode), false);

						// Preprocessor Definitions Property
						PropertyInfo PreprocessorDefinitionsProperty(Variant::STRING, PreprocessorDefinitionsConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(PreprocessorDefinitionsProperty);
						editor_settings->set_initial_value(PreprocessorDefinitionsConfigPath, "JENOVA_CUSTOM", false);

						// Additional Include Directories Property
						PropertyInfo AdditionalIncludeDirectoriesProperty(Variant::STRING, AdditionalIncludeDirectoriesConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(AdditionalIncludeDirectoriesProperty);
						editor_settings->set_initial_value(AdditionalIncludeDirectoriesConfigPath, "", false);

						// Additional Library Directories Property
						PropertyInfo AdditionalLibraryDirectoriesProperty(Variant::STRING, AdditionalLibraryDirectoriesConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(AdditionalLibraryDirectoriesProperty);
						editor_settings->set_initial_value(AdditionalLibraryDirectoriesConfigPath, "", false);

						// Additional Dependencies Property
						PropertyInfo AdditionalDependenciesProperty(Variant::STRING, AdditionalDependenciesConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(AdditionalDependenciesProperty);
						editor_settings->set_initial_value(AdditionalDependenciesConfigPath, "", false);

						// External Changes Trigger Mode Property
						PropertyInfo ExternalChangesTriggerModeProperty(Variant::INT, ExternalChangesTriggerModeConfigPath, 
							PropertyHint::PROPERTY_HINT_ENUM, "Build Project On Script Reload,Build Project On Watchdog Invoke,Don't Take Any Action",
							PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(ExternalChangesTriggerModeProperty);
						editor_settings->set_initial_value(ExternalChangesTriggerModeConfigPath, int32_t(ExternalChangesDefaultTriggerMode), false);
						
						// Use Hot Reload At Runtime Property
						PropertyInfo UseHotReloadAtRuntimeProperty(Variant::BOOL, UseHotReloadAtRuntimeConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(UseHotReloadAtRuntimeProperty);
						editor_settings->set_initial_value(UseHotReloadAtRuntimeConfigPath, true, false);
						
						// Editor Verbose Output Property
						PropertyInfo EditorVerboseOutputProperty(Variant::INT, EditorVerboseOutputConfigPath, 
							PropertyHint::PROPERTY_HINT_ENUM, "Standard Editor Output,Jenova Built-In Terminal,Disabled",
							PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(EditorVerboseOutputProperty);
						editor_settings->set_initial_value(EditorVerboseOutputConfigPath, int32_t(EditorVerboseDefaultOutput), false);

						// Use Monospace Font For Terminal Property
						PropertyInfo UseMonospaceFontForTerminalProperty(Variant::BOOL, UseMonospaceFontForTerminalConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(UseMonospaceFontForTerminalProperty);
						editor_settings->set_initial_value(UseMonospaceFontForTerminalConfigPath, true, false);

						// Terminal Default Font Size Property
						PropertyInfo TerminalDefaultFontSizeProperty(Variant::INT, TerminalDefaultFontSizeConfigPath, 
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(TerminalDefaultFontSizeProperty);
						editor_settings->set_initial_value(TerminalDefaultFontSizeConfigPath, jenova::GlobalSettings::JenovaTerminalLogFontSize, false);

						// Compiler Package Property
						PropertyInfo CompilerPackageProperty(Variant::STRING, CompilerPackageConfigPath,
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(CompilerPackageProperty);
						editor_settings->set_initial_value(CompilerPackageConfigPath, "Latest", false);

						// GodotKit Package Property
						PropertyInfo GodotKitPackageProperty(Variant::STRING, GodotKitPackageConfigPath,
							PropertyHint::PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(GodotKitPackageProperty);
						editor_settings->set_initial_value(GodotKitPackageConfigPath, "Latest", false);

						// Build Tool Button Placement Property
						PropertyInfo BuildToolButtonPlacementProperty(Variant::INT, BuildToolButtonEditorConfigPath, 
							PropertyHint::PROPERTY_HINT_ENUM, "Before Main Menu,After Main Menu,Before Stage Selector,After Stage Selector,Before Run Bar,After Run Bar,After Render Method",
							PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED, JenovaEditorSettingsCategory);
						editor_settings->add_property_info(BuildToolButtonPlacementProperty);
						editor_settings->set_initial_value(BuildToolButtonEditorConfigPath, int32_t(BuildToolButtonDefaultPlacement), false);

						// All Good
						return true;
					}
				}

				// Failed
				return false;
			}
			bool GetEditorSetting(const String& setting_name, Variant& value) 
			{
				EditorInterface* editor_interface = EditorInterface::get_singleton();
				if (editor_interface) 
				{
					Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
					if (!editor_settings.is_null()) 
					{
						// Check if the setting exists
						if (editor_settings->has_setting(setting_name)) 
						{
							// Get the setting value
							value = editor_settings->get_setting(setting_name);

							// All Good
							return true;
						}
					}
				}
				return false;
			}
			bool UpdateStorageConfigurations()
			{
				// Update Jenova Cache Directory
				jenova::GlobalStorage::CurrentJenovaCacheDirectory = AS_STD_STRING(jenova::GetJenovaCacheDirectory());

				// Update Interpreter Backend
				Variant interpreterBackend;
				if (!GetEditorSetting(InterpreterBackendConfigPath, interpreterBackend)) return false;
				JenovaInterpreter::SetInterpreterBackend(jenova::InterpreterBackend(int32_t(interpreterBackend)));

				// Update Build And Run Mode
				Variant buildAndRunMode;
				if (!GetEditorSetting(BuildAndRunModeConfigPath, buildAndRunMode)) return false;
				jenova::GlobalStorage::CurrentBuildAndRunMode = jenova::BuildAndRunMode(int32_t(buildAndRunMode));

				// Update External Changes Trigger Mode
				Variant changesTriggerMode;
				if (!GetEditorSetting(ExternalChangesTriggerModeConfigPath, changesTriggerMode)) return false;
				jenova::GlobalStorage::CurrentChangesTriggerMode = jenova::ChangesTriggerMode(int32_t(changesTriggerMode));

				// Update Use Hot-Reload At Runtime
				Variant useHotReloadAtRuntime;
				if (!GetEditorSetting(UseHotReloadAtRuntimeConfigPath, useHotReloadAtRuntime)) return false;
				jenova::GlobalStorage::UseHotReloadAtRuntime = bool(useHotReloadAtRuntime);

				// Update Editor Verbose Output
				Variant editorVerboseOutput;
				if (!GetEditorSetting(EditorVerboseOutputConfigPath, editorVerboseOutput)) return false;
				jenova::GlobalStorage::CurrentEditorVerboseOutput = jenova::EditorVerboseOutput(int32_t(editorVerboseOutput));

				// Update Monospace Font For Terminal
				Variant useMonospaceFontForTerminal;
				if (!GetEditorSetting(UseMonospaceFontForTerminalConfigPath, useMonospaceFontForTerminal)) return false;
				jenova::GlobalStorage::UseMonospaceFontForTerminal = bool(useMonospaceFontForTerminal);

				// Update Terminal Default Font Size
				Variant terminalDefaultFontSize;
				if (!GetEditorSetting(TerminalDefaultFontSizeConfigPath, terminalDefaultFontSize)) return false;
				jenova::GlobalStorage::TerminalDefaultFontSize = int(terminalDefaultFontSize);

				// All Good
				return true;
			}
			bool CreateBuildToolButton()
			{
				// Create Button
				buildToolButton = memnew(Button);
				buildToolButton->set_flat(false);
				buildToolButton->set_tooltip_text("Build Jenova Project");
				buildToolButton->set_button_icon(jenova::GetEditorIcon("PluginScript"));
				buildToolButton->set_focus_mode(Control::FOCUS_NONE);
				buildToolButton->set_shortcut_in_tooltip(false);
				buildToolButton->set_icon_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_RIGHT);
				buildToolButton->set_theme_type_variation("RunBarButton");

				// Connect Button Press to Method
				buildToolButton->connect("pressed", callable_mp(this, &JenovaEditorPlugin::OnBuildButtonPressed));

				// Add Button to Editor's Run Bar
				add_control_to_container(CONTAINER_TOOLBAR, buildToolButton);

				// Load Placement Settings
				Variant BuildToolButtonPlacement;
				if (!GetEditorSetting(BuildToolButtonEditorConfigPath, BuildToolButtonPlacement)) BuildToolButtonPlacement = int32_t(BuildToolButtonDefaultPlacement);

				// Move the button to the desired position
				buildToolButton->get_parent()->move_child(buildToolButton, BuildToolButtonPlacement);

				// All Good
				return true;
			}
			bool RemoveBuildToolButton()
			{
				if (buildToolButton && buildToolButton->get_parent()) 
				{
					// Remove Button from Parent
					buildToolButton->get_parent()->remove_child(buildToolButton);

					// Delete Allocated Button
					memdelete(buildToolButton);
					buildToolButton = nullptr;

					// All Good
					return true;
				}
				return false;
			}
			bool InitialzeClassesIcons()
			{
				// Obtain Theme
				auto* editor_interface = EditorInterface::get_singleton();
				Ref<Theme> editor_theme = editor_interface->get_editor_theme();
				double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

				// Validate Theme
				if (!editor_theme.is_valid()) return false;

				// Register Emulator Class Icon
				if (jenova::GlobalSettings::EmulatorEnabled)
				{
					if (!editor_theme->has_icon("JenovaEmulator", "EditorIcons"))
					{
						Ref<ImageTexture> iconImage = jenova::CreateImageTextureFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(PNG_JENOVA_ICON_64)));
						if (iconImage.is_valid())
						{
							editor_theme->set_icon("JenovaEmulator", "EditorIcons", iconImage);
						}
						else
						{
							jenova::Error("Jenova Plugin", "Cannot Load Emulator Icon.");
							return false;
						}
					}
				}

				// Register C++ Script Icon
				if (jenova::GlobalSettings::ScriptingEnabled)
				{
					if (!editor_theme->has_icon(jenova::GlobalSettings::JenovaScriptType, "EditorIcons"))
					{
						Ref<ImageTexture> iconImage =
							jenova::CreateImageTextureFromByteArrayEx(BUFFER_PTR_SIZE_PARAM(jenova::resources::SVG_CPP_SCRIPT_ICON),
							Vector2i(SCALED(19), SCALED(18)), jenova::ImageCreationFormat::SVG);

						if (iconImage != nullptr)
						{
							editor_theme->set_icon(jenova::GlobalSettings::JenovaScriptType, "EditorIcons", iconImage);
						}
						else
						{
							jenova::Error("Jenova Plugin", "Cannot Load C++ Script Icon.");
							return false;
						}
					}
				}

				// Register C++ Header Icon
				if (jenova::GlobalSettings::ScriptingEnabled)
				{
					if (!editor_theme->has_icon(jenova::GlobalSettings::JenovaHeaderType, "EditorIcons"))
					{
						Ref<ImageTexture> iconImage =  
							jenova::CreateImageTextureFromByteArrayEx(BUFFER_PTR_SIZE_PARAM(jenova::resources::SVG_CPP_HEADER_ICON), 
							Vector2i(SCALED(18), SCALED(18)), jenova::ImageCreationFormat::SVG);

						if (iconImage != nullptr)
						{
							editor_theme->set_icon(jenova::GlobalSettings::JenovaHeaderType, "EditorIcons", iconImage);
						}
						else
						{
							jenova::Error("Jenova Plugin", "Cannot Load C++ Header Icon.");
							return false;
						}
					}
				}

				// All Good
				return true;
			}
			bool InitializeDocumentation()
			{
				// Register Nodes Documentation
				if (jenova::GlobalSettings::EmulatorEnabled) jenova::RegisterDocumentationFromByteArray(BUFFER_PTR_SIZE_PARAM(jenova::documentation::JenovaEmulatorXML));

				// Register Settings Documentation [ Disabled : This will replace entire Editor Settings ]
				// jenova::RegisterDocumentationFromByteArray(BUFFER_PTR_SIZE_PARAM(jenova::documentation::EditorSettingsXML));

				// All Good
				return true;
			}
			bool InitializeGodotSDKData()
			{
				// Get Project Path
				String projectPath = ProjectSettings::get_singleton()->globalize_path("res://");
				std::string godotSDKPath = std::filesystem::absolute(AS_STD_STRING(projectPath) + "/Jenova/GodotSDK").string();

				// Clear Data
				jenova::GlobalStorage::CurrentJenovaGodotSDKGeneratedData.clear();

				// Collect Files And Generate Data
				try
				{
					if (std::filesystem::exists(godotSDKPath) && std::filesystem::is_directory(godotSDKPath))
					{
						for (const auto& entry : std::filesystem::recursive_directory_iterator(godotSDKPath))
						{
							if (entry.is_regular_file() && entry.path().extension() == ".hpp")
							{
								std::filesystem::path relativePath = std::filesystem::relative(entry.path(), godotSDKPath);
								std::string includePath = "#include <" + relativePath.string();
								std::replace(includePath.begin(), includePath.end(), '\\', '/');
								jenova::GlobalStorage::CurrentJenovaGodotSDKGeneratedData += includePath + ">\n";
							}
						}
					}
					else
					{
						// Failed
						return false;
					}
				}
				catch (const std::filesystem::filesystem_error& e)
				{
					// Failed
					return false;
				}

				// All Good
				return true;
			}
			bool InitializeEditorMenu()
			{
				// Get Editor Node
				Control* editorNode = GetEditorContainerNode();
				if (!editorNode) return false;
				
				// Get Title Bar
				HBoxContainer* titleBar = (HBoxContainer*)FindFirstControlByClassName(editorNode, "EditorTitleBar");
				if (!titleBar) return false;

				// Get Main Menu
				MenuBar* mainMenu = (MenuBar*)FindFirstControlByClassName(editorNode, "MenuBar");
				if (!mainMenu) return false;

				// Create Jenova Menu
				jenovaMenu = memnew(PopupMenu);
				jenovaMenu->set_name("Jenova");

				// Create Tools Submenu
				toolsMenu = memnew(PopupMenu);
				toolsMenu->set_name("Tools");

				// Add Main Menu Items
				jenovaMenu->add_shortcut(CreateShortcut("  Build Solution", Key(KEY_MASK_CTRL | KEY_MASK_SHIFT | KEY_B)), EDITOR_MENU_ID(BuildSolution));
				jenovaMenu->add_shortcut(CreateShortcut("  Rebuild Solution", Key(KEY_MASK_CTRL | KEY_MASK_SHIFT | KEY_R)), EDITOR_MENU_ID(RebuildSolution));
				jenovaMenu->add_shortcut(CreateShortcut("  Clean Solution", Key(KEY_MASK_CTRL | KEY_MASK_SHIFT | KEY_K)), EDITOR_MENU_ID(CleanSolution));
				jenovaMenu->add_separator();
				jenovaMenu->add_item("  Configure Build...  ", EDITOR_MENU_ID(ConfigureBuild));
				jenovaMenu->add_separator();
				jenovaMenu->add_shortcut(CreateShortcut("  Export to Visual Studio...  ", Key(KEY_MASK_CTRL | KEY_MASK_SHIFT | KEY_E)), EDITOR_MENU_ID(ExportToVisualStudio));
				jenovaMenu->add_item("  Export Jenova Module...  ", EDITOR_MENU_ID(ExportJenovaModule));
				jenovaMenu->add_separator();

				// Add Developer Mode Menu Items
				String developerModeMenuItemText = jenova::GlobalSettings::VerboseEnabled ? "  Disable Developer Mode" : "  Enable Developer Mode";
				developerModeShortcut = CreateShortcut(developerModeMenuItemText, Key(KEY_MASK_CTRL | KEY_MASK_SHIFT | KEY_D));
				jenovaMenu->add_shortcut(developerModeShortcut, EDITOR_MENU_ID(DeveloperMode));
				jenovaMenu->add_separator();

				// Add Tools Menu Items
				toolsMenu->add_item("  Clear Cache Database", EDITOR_MENU_ID(ClearCacheDatabase));
				toolsMenu->add_separator();
				toolsMenu->add_item("  Generate Encryption Key...  ", EDITOR_MENU_ID(GenerateEncryptionKey));
				toolsMenu->add_item("  Backup Current Encryption Key...  ", EDITOR_MENU_ID(BackupCurrentEncryptionKey));
				toolsMenu->add_separator();
				toolsMenu->add_item("  Open Script Manager...  ", EDITOR_MENU_ID(OpenScriptManager));
				toolsMenu->add_item("  Open Package Manager...  ", EDITOR_MENU_ID(OpenPackageManager));

				// Add Tools Submenu
				jenovaMenu->add_child(toolsMenu);
				jenovaMenu->add_submenu_item("  Tools", toolsMenu->get_name());

				// Add Extra Menu Items
				jenovaMenu->add_separator();
				jenovaMenu->add_item("  Documentation", EDITOR_MENU_ID(Documentation));
				jenovaMenu->add_item("  Discord Server", EDITOR_MENU_ID(DiscordServer));
				jenovaMenu->add_separator();
				jenovaMenu->add_item("  Check for Updates", EDITOR_MENU_ID(CheckForUpdates));
				jenovaMenu->add_item("  About Projekt Jenova", EDITOR_MENU_ID(AboutJenova));

				// Load Menu Icons
				auto jenovaIcon = CREATE_PNG_MENU_ICON(JENOVA_RESOURCE(PNG_JENOVA_ICON_64));
				auto vsIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_VISUAL_STUDIO_ICON));
				auto codeTealIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_CODEBLOCK_TEAL_ICON));
				auto codeRedIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_CODEBLOCK_RED_ICON));
				auto discordIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_DISCORD_ICON));
				auto diamondIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_DIAMOND_ICON));
				auto lightningIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_LIGHTNING_ICON));
				auto eraserIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_ERASER_ICON));
				auto updateIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_UPDATE_ICON));
				auto keyTealIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_KEY_TEAL_ICON));
				auto keyMaroonIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_KEY_MAROON_ICON));
				auto compilerIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_COMPILER_ICON));
				auto packageIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_PACKAGE_ICON));
				auto configureIcon = CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_CONFIGURE_ICON));

				// Set Menu Icons
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(BuildSolution)), jenova::GetEditorIcon("PluginScript"));
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(RebuildSolution)), jenova::GetEditorIcon("PreviewRotate"));
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(CleanSolution)), jenova::GetEditorIcon("Clear"));
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(ConfigureBuild)), configureIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(ExportToVisualStudio)), vsIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(ExportJenovaModule)), lightningIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(DeveloperMode)), jenova::GlobalSettings::VerboseEnabled ? codeTealIcon : codeRedIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(DiscordServer)), discordIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(AboutJenova)), jenovaIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(Documentation)), diamondIcon);
				jenovaMenu->set_item_icon(jenovaMenu->get_item_index(EDITOR_MENU_ID(CheckForUpdates)), updateIcon);
				toolsMenu->set_item_icon(toolsMenu->get_item_index(EDITOR_MENU_ID(ClearCacheDatabase)), eraserIcon);
				toolsMenu->set_item_icon(toolsMenu->get_item_index(EDITOR_MENU_ID(GenerateEncryptionKey)), keyTealIcon);
				toolsMenu->set_item_icon(toolsMenu->get_item_index(EDITOR_MENU_ID(BackupCurrentEncryptionKey)), keyMaroonIcon);
				toolsMenu->set_item_icon(toolsMenu->get_item_index(EDITOR_MENU_ID(OpenScriptManager)), compilerIcon);
				toolsMenu->set_item_icon(toolsMenu->get_item_index(EDITOR_MENU_ID(OpenPackageManager)), packageIcon);

				// Add Jenova Menu to Main Menu
				mainMenu->add_child(jenovaMenu);

				// Set Jenova Menu Position And Data
				int jenovaMenuPosition = mainMenu->get_child_count() - 2; // Before Help
				mainMenu->move_child(jenovaMenu, jenovaMenuPosition);
				mainMenu->set_menu_tooltip(jenovaMenuPosition, "Operations & Utilities of Jenvoa Framework");

				// Bind Menu Items Press Signals
				jenovaMenu->connect("id_pressed", callable_mp(this, &JenovaEditorPlugin::OnMenuItemPressed));
				toolsMenu->connect("id_pressed", callable_mp(this, &JenovaEditorPlugin::OnMenuItemPressed));

				// All Good
				return true;
			}
			bool DestroyEditorMenu()
			{
				// Remove Menus
				toolsMenu->get_parent()->remove_child(toolsMenu);
				jenovaMenu->get_parent()->remove_child(jenovaMenu);

				// Release Menus
				memdelete(toolsMenu);
				memdelete(jenovaMenu);

				// All Good
				return true;
			}
			bool RegisterAssetMonitors()
			{
				// Register Project Directory Monitor
				if (!JenovaAssetMonitor::get_singleton()->AddDirectory(ProjectSettings::get_singleton()->globalize_path("res://"))) return false;

				// Register Jenova Cache Directory Monitor
				if (!JenovaAssetMonitor::get_singleton()->AddDirectory(GetJenovaCacheDirectory())) return false;

				// Register Callback
				if (!JenovaAssetMonitor::get_singleton()->RegisterCallback(JenovaEditorPlugin::OnAssetChanged)) return false;

				// All Good
				return true;
			}
			bool UnRegisterAssetMonitors()
			{
				// Prepare for Shutdown
				if (!JenovaAssetMonitor::get_singleton()->PrepareForShutdown()) return false;

				// All Good
				return true;
			}
			bool RegisterBuiltInScriptTemplates()
			{
				// Register Global Script Templates
				CREATE_GLOBAL_TEMPLATE("Default", CODE_TEMPLATE_DEFAULT, "Base C++ Script");
				CREATE_GLOBAL_TEMPLATE("Boot", CODE_TEMPLATE_BOOT, "Module Boot/Entrypoint");
				CREATE_GLOBAL_TEMPLATE("Export", CODE_TEMPLATE_EXPORT, "Export Functions");

				// Register Node Script Templates
				CREATE_CLASS_TEMPLATE("Node Base", "Node", CODE_TEMPLATE_NODE_BASE, "Base Node Script");

				// Register Control Script Templates
				CREATE_CLASS_TEMPLATE("Control Base", "Control", CODE_TEMPLATE_CONTROL_BASE, "Base Control Script");

				// Register Label Script Templates
				CREATE_CLASS_TEMPLATE("Label FPS Visualizer", "Label", CODE_TEMPLATE_LABEL_FPS, "Display FPS On Label");

				// All Good
				return true;
			}
			bool RegisterEditorTerminalPanel()
			{
				// Get Viewport
				Viewport* editorViewport = this->get_viewport();
				if (!editorViewport) return false;

				// Create Terminal Control
				jenovaTerminal = memnew(Control);
				jenovaTerminal->set_name("JenovaTerminalControl");
				jenovaTerminal->set_anchors_preset(Control::PRESET_FULL_RECT);
				jenovaTerminal->set_custom_minimum_size(Vector2i(0, editorViewport->get_visible_rect().get_size().height * 0.2f));

				// Log Screen
				jenovaLogOutput = memnew(RichTextLabel);
				jenovaLogOutput->set_name("TerminalLog");
				jenovaLogOutput->set_threaded(true);
				jenovaLogOutput->set_use_bbcode(true);
				jenovaLogOutput->set_scroll_follow(true);
				jenovaLogOutput->set_selection_enabled(true);
				jenovaLogOutput->set_context_menu_enabled(true);
				jenovaLogOutput->set_focus_mode(Control::FOCUS_CLICK);
				jenovaLogOutput->set_anchors_preset(Control::PRESET_FULL_RECT);
				jenovaLogOutput->set_deselect_on_focus_loss_enabled(false);
				jenovaLogOutput->set_autowrap_mode(TextServer::AutowrapMode::AUTOWRAP_WORD_SMART);
				jenovaTerminal->add_child(jenovaLogOutput);

				// Clear Button
				Button* clearButton = memnew(Button);
				clearButton->set_name("TerminalClearButton");
				clearButton->set_flat(false);
				clearButton->set_tooltip_text("Clear Terminal Log");
				clearButton->set_button_icon(jenova::GetEditorIcon("Clear"));
				clearButton->set_focus_mode(Control::FOCUS_NONE);
				clearButton->set_shortcut_in_tooltip(false);
				clearButton->set_icon_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
				clearButton->set_theme_type_variation("RunBarButton");
				clearButton->set_size(Vector2(40, 40));
				clearButton->set_anchors_preset(Control::PRESET_BOTTOM_RIGHT);
				clearButton->set_position(Vector2(jenovaTerminal->get_size().x - 110, jenovaTerminal->get_size().y - 50));
				jenovaTerminal->add_child(clearButton);

				// Clear Button
				Button* copyButton = memnew(Button);
				copyButton->set_name("TerminalCopyButton");
				copyButton->set_flat(false);
				copyButton->set_tooltip_text("Copy Terminal Log");
				copyButton->set_button_icon(jenova::GetEditorIcon("ActionCopy"));
				copyButton->set_focus_mode(Control::FOCUS_NONE);
				copyButton->set_shortcut_in_tooltip(false);
				copyButton->set_icon_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
				copyButton->set_theme_type_variation("RunBarButton");
				copyButton->set_size(Vector2(40, 40));
				copyButton->set_anchors_preset(Control::PRESET_BOTTOM_RIGHT);
				copyButton->set_position(Vector2(jenovaTerminal->get_size().x - 60, jenovaTerminal->get_size().y - 50));
				jenovaTerminal->add_child(copyButton);

				// Assign Events
				clearButton->connect("pressed", callable_mp(this, &JenovaEditorPlugin::ClearLogs));
				copyButton->connect("pressed", callable_mp(this, &JenovaEditorPlugin::CopyLogs));

				// Add Terminal to Bottom Panel
				Button* terminalButton = this->add_control_to_bottom_panel(jenovaTerminal, " Terminal");
				if (!terminalButton) return false;
				terminalButton->set_tooltip_text("Jenova Built-In Terminal System");

				// Set Terminal Icon
				auto jenovaIcon = jenova::CreateImageTextureFromByteArrayEx(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(PNG_JENOVA_ICON_64)), Vector2i(15, 15));
				terminalButton->set_button_icon(jenovaIcon);

				// Update Terminal
				UpdateTerminal();

				// All Good
				return true;
			}
			bool UnRegisterEditorTerminalPanel()
			{
				if (jenovaTerminal)
				{
					// Remove Terminal from Bottom Panel
					this->remove_control_from_bottom_panel(jenovaTerminal);

					// Remove Log Output from Terminal
					jenovaTerminal->remove_child(jenovaLogOutput);

					// Delete Allocated Log Output
					memdelete(jenovaLogOutput);
					jenovaLogOutput = nullptr;

					// Delete Allocated Control
					memdelete(jenovaTerminal);
					jenovaTerminal = nullptr;

					// All Good
					return true;
				}
				return false;
			}

			// Instance Obtainers
			Ref<JenovaExportPlugin> GetExportPluginInstance()
			{
				return exportPlugin;
			}
			Ref<JenovaImportPlugin> GetImportPluginInstance()
			{
				return importPlugin;
			}
			Ref<JenovaDebuggerPlugin> GetDebuggerPluginInstance()
			{
				return debuggerPlugin;
			}

			// UI Events
			void OnEditorThemeChanged()
			{
				// Validate Editor Plugin
				if (!isEditorPluginInitialized) return;

				// Re-Initialize Custom Types Icons
				VALIDATE_FUNCTION(InitialzeClassesIcons());
			}
			void OnWindowGainedFocus()
			{
			}
			void OnWindowLostFocus()
			{
				// Update Build Button Icon
				if (buildToolButton)
				{
					buildToolButton->set_tooltip_text("Build Jenova Project");
					buildToolButton->set_button_icon(jenova::GetEditorIcon("PluginScript"));
				}
			}
			void OnBuildButtonPressed()
			{
				if (Input::get_singleton()->is_key_pressed(KEY_ALT)) CleanProject();
				else BuildProject();
			}
			void OnMenuItemPressed(int32_t menuID)
			{
				// Get Menu Item Index And ID
				int menuItemIndex = jenovaMenu->get_item_index(int32_t(menuID));
				jenova::EditorMenuID menuItemID = jenova::EditorMenuID(menuID);

				// Handle Menu Actions
				switch (menuItemID)
				{
				case jenova::EditorMenuID::BuildSolution:
					BuildProject();
					break;
				case jenova::EditorMenuID::RebuildSolution:
					CleanProject();
					BuildProject();
					break;
				case jenova::EditorMenuID::CleanSolution:
					CleanProject();
					break;
				case jenova::EditorMenuID::ConfigureBuild:
					OpenBuildConfigurationWindow();
					break;
				case jenova::EditorMenuID::ExportToVisualStudio:
					OpenVisualStudioSelectorWindow();
					break;
				case jenova::EditorMenuID::ExportJenovaModule:
					jenova::Error("Jenova Main Menu", "Feature Not Implemented Yet");
					break;
				case jenova::EditorMenuID::DeveloperMode:
					jenova::GlobalStorage::DeveloperModeActivated = !jenova::GlobalStorage::DeveloperModeActivated;
					jenovaMenu->set_item_text(menuItemIndex, jenova::GlobalStorage::DeveloperModeActivated ? "  Disable Developer Mode" : "  Enable Developer Mode");
					if (jenova::GlobalStorage::DeveloperModeActivated) jenovaMenu->set_item_icon(menuItemIndex, CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_CODEBLOCK_TEAL_ICON)));
					if (!jenova::GlobalStorage::DeveloperModeActivated) jenovaMenu->set_item_icon(menuItemIndex, CREATE_SVG_MENU_ICON(JENOVA_RESOURCE(SVG_CODEBLOCK_RED_ICON)));
					jenova::Output("Developer Mode %s", jenova::GlobalStorage::DeveloperModeActivated ? "Enabled" : "Disabled");
					break;
				case jenova::EditorMenuID::ClearCacheDatabase:
					if (std::filesystem::exists(AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile))
					{
						if (std::filesystem::remove(AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile))
						{
							jenova::Output("Jenova Module Cache Database Cleared.");
						}
						else
						{
							jenova::Error("[Cache System]", "Jenova Module Cache Database Cannot Be Cleared.");
						};
					}
					else jenova::Output("Jenova Module Cache Doesn't Exists, Operation Aborted.");
					break;
				case jenova::EditorMenuID::GenerateEncryptionKey:
					jenova::Error("Jenova Main Menu", "Feature is Removed.");
					break;
				case jenova::EditorMenuID::BackupCurrentEncryptionKey:
					jenova::Error("Jenova Main Menu", "Feature is Removed.");
					break;
				case jenova::EditorMenuID::OpenScriptManager:
					jenova::Error("Jenova Main Menu", "Feature Not Implemented Yet");
					break;
				case jenova::EditorMenuID::OpenPackageManager:
					if (JenovaPackageManager::get_singleton())
					{
						if (!JenovaPackageManager::get_singleton()->OpenPackageManager(String(jenova::GlobalSettings::JenovaPackageDatabaseURL)))
						{
							jenova::Error("Jenova Framework", "Package Manager Failed to Open.");
						}
					}
					break;
				case jenova::EditorMenuID::Documentation:
					ShellExecute(0, 0, L"https://jenova-framework.github.io/docs/", 0, 0, SW_SHOW);
					break;
				case jenova::EditorMenuID::DiscordServer:
					ShellExecute(0, 0, L"https://discord.gg/p7zAf6aBPz", 0, 0, SW_SHOW);
					break;
				case jenova::EditorMenuID::CheckForUpdates:
					jenova::Error("Jenova Main Menu", "Feature Not Implemented Yet");
					break;
				case jenova::EditorMenuID::AboutJenova:
					OpenAboutJenovaProject();
					break;
				default:
					break;
				}
			}

			// Settings Events
			void OnEditorSettingsChanged()
			{
				// Update New Settings
				if (!UpdateStorageConfigurations())
				{
					jenova::Warning("Jenova Settings", "Unable to Update Storage Configurations!");
				}

				// Take Action On Changed Settings
				auto changedEditorSettings = EditorInterface::get_singleton()->get_editor_settings()->get_changed_settings();
				for (const auto& changedEditorSetting : changedEditorSettings)
				{
					// Warn User to Restart for Compiler Model Change
					if (changedEditorSetting == CompilerModelConfigPath)
					{
						// Todo : Store A Global Value And Compare, If changed Emit Once!
						jenova::Warning("Jenova Settings", "Compiler Model has Changed. While not Absolutely Required, It's Recommended To Restart The Editor For Stablity.");
					}

					// Update Terminal If Options Changed
					if (changedEditorSetting == UseMonospaceFontForTerminalConfigPath || changedEditorSetting == TerminalDefaultFontSizeConfigPath)
					{
						UpdateTerminal();
					}
				}
			}

			// Build Events
			void OnBuildSuccess()
			{
				// Start Game
				if (jenova::GlobalStorage::CurrentBuildAndRunMode == jenova::BuildAndRunMode::RunOnBuildSuccess) EditorInterface::get_singleton()->play_main_scene();
			}

			// Project Actions
			bool BuildProject()
			{
				// Check If Editor Running Project
				if (EditorInterface::get_singleton()->is_playing_scene())
				{
					jenova::Error("Jenova Builder", "Jenova cannot build while the Editor is running project, Stop the project and try again.");
				
					// Prompt User for Retry
					ConfirmationDialog* dialog = memnew(ConfirmationDialog);
					dialog->set_title("[ Build Error ]");
					dialog->set_text("Jenova cannot build while the Editor is running project. \nWould you like to stop the project and try again?");
					dialog->get_ok_button()->set_text("Confirm");
					dialog->get_cancel_button()->set_text("Abort Build");
				
					// Define Internal UI Callback
					class OnConfirmedEvent : public RefCounted
					{
					private:
						JenovaEditorPlugin* pluginInstance;
				
					public:
						OnConfirmedEvent(JenovaEditorPlugin* _plugin) { pluginInstance = _plugin; }
						void ProcessEvent()
						{
							EditorInterface::get_singleton()->stop_playing_scene();
							pluginInstance->BuildProject();
							memdelete(this);
						}
					};

					// Create & Assign UI Callback to Dialog
					dialog->connect("confirmed", callable_mp(memnew(OnConfirmedEvent(this)), &OnConfirmedEvent::ProcessEvent));
					dialog->connect("confirmed", callable_mp((Node*)dialog, &ConfirmationDialog::queue_free));
					dialog->connect("canceled", callable_mp((Node*)dialog, &ConfirmationDialog::queue_free));

					// Add Dialog to Engine & Show
					add_child(dialog);
					dialog->popup_centered();

					// Relaunched Ignore Faliure
					return true;
				}
			
				// Switch to Jenova Terminal Tab
				if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal && jenovaTerminal)
				{
					this->make_bottom_panel_item_visible(jenovaTerminal);
				}

				// Stop Interpreter Execution
				JenovaInterpreter::SetExecutionState(false);

				// Update Settings
				if (!UpdateStorageConfigurations())
				{
					jenova::Error("Jenova Builder", "Failed to Update Configurations From Editor Settings.");
					return false;
				}

				// Create Project Build Profiler Checkpoint
				JenovaTinyProfiler::CreateCheckpoint("JenovaProjectBuild");

				// Verbose Build
				jenova::Output("Building Project C++ Scripts...");

				// Create Compiler
				if (!CreateCompiler()) return false;

				// Create Cache Folder If Doesn't Exists
				if(!filesystem::exists(AS_STD_STRING(jenova::GetJenovaCacheDirectory())))
				{
					try
					{
						filesystem::create_directories(AS_STD_STRING(jenova::GetJenovaCacheDirectory()));
					}
					catch (const std::filesystem::filesystem_error& e)
					{
						jenova::Error("Jenova Builder", "Failed to Create Jenova Cache Directory.");
						DisposeCompiler();
						return false;
					}
				}

				// Create GodotSDK Auto-Header
				if (!jenova::GlobalStorage::CurrentJenovaGodotSDKGeneratedData.empty())
				{
					// Write Database to Disk
					std::string headerCachePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + jenova::GlobalSettings::JenovaGodotSDKHeaderCacheFile;
					std::fstream headerCacheWritter;
					headerCacheWritter.open(headerCachePath, std::ios::binary | std::ios::out);
					headerCacheWritter.write(jenova::GlobalStorage::CurrentJenovaGodotSDKGeneratedData.data(), jenova::GlobalStorage::CurrentJenovaGodotSDKGeneratedData.size());
					headerCacheWritter.close();
				}

				// Collect Current Used Script
				jenova::Output("Collecting ([color=#53b5ab]%lld[/color]) C++ Script Object In Use...", ScriptManager::get_singleton()->get_script_object_count());
				unordered_map<string, Ref<CPPScript>> usedScripts;
				for (size_t i = 0; i < ScriptManager::get_singleton()->get_script_object_count(); i++)
				{
					Ref<CPPScript> scriptObject = ScriptManager::get_singleton()->get_script_object(i);
					if (!scriptObject->HasValidScriptIdentity()) scriptObject->GenerateScriptIdentity();
					usedScripts.insert(std::make_pair(AS_STD_STRING(scriptObject->GetScriptIdentity()), scriptObject));
					jenova::Output("C++ Script Object In Use ([color=#91b553]%s[/color]) Collected.", AS_C_STRING(scriptObject->GetScriptIdentity()));
				}

				// Collect All Script Files In File System
				jenova::Output("Collecting Project C++ Scripts...");
				jenova::ResourceCollection cppResources;
				if (!jenova::CollectScriptsFromFileSystemAndScenes("res://", "cpp", cppResources))
				{
					jenova::Error("Jenova Builder", "Failed to Collect C++ Scripts from Project.");
					DisposeCompiler();
					return false;
				};

				// If There's No Script Abort Build
				if (cppResources.size() == 0)
				{
					jenova::Error("Jenova Builder", "No C++ Script Found In Project, Build Aborted.");
					DisposeCompiler();
					return false;
				}

				// Collect Header Scripts
				jenova::Output("Collecting Project C++ Headers...");
				jenova::ResourceCollection headerResources;
				PackedStringArray cppHeaderFiles;
				if (!jenova::CollectResourcesFromFileSystem("res://", "h hh hpp", headerResources))
				{
					jenova::Error("Jenova Builder", "Failed to Collect C++ Headers from Project.");
					DisposeCompiler();
					return false;
				};
				for (const auto& headerResource : headerResources)
				{
					if (headerResource->is_class(jenova::GlobalSettings::JenovaHeaderType))
					{
						// Get C++ Header Object
						Ref<CPPHeader> cppHeader = Object::cast_to<CPPHeader>(headerResource.ptr());
						cppHeaderFiles.push_back(ProjectSettings::get_singleton()->globalize_path(cppHeader->get_path()));
						jenova::Output("C++ Header File ([color=#70a9d4]%s[/color]) Added to Build System.", AS_C_STRING(cppHeader->get_path()));
					}
				}
				if (!jenovaCompiler->SetCompilerOption("CppHeaderFiles", cppHeaderFiles))
				{
					jenova::Error("Jenova Builder", "Failed to Set C++ Headers as Compiler Settings.");
					DisposeCompiler();
					return false;
				};

				// Create Preprocessor Settings
				godot::Dictionary preprocessorSettings;
				{
					Variant preprocessorDefinitions;
					if (!GetEditorSetting(PreprocessorDefinitionsConfigPath, preprocessorDefinitions))
					{ 
						jenova::Error("Jenova Builder", "Failed to Obtain Preprocessor Definitions.");
						DisposeCompiler();
						return false;
					}
					preprocessorSettings["PreprocessorDefinitions"] = preprocessorDefinitions;
				}

				// Collect Scripts & Preprocess Them
				jenova::Output("Preparing ([color=#53b5ab]%lld[/color]) C++ Script Resource From Project...", cppResources.size());
				scriptModules.clear();
				for (const auto& cppResource : cppResources)
				{
					if (cppResource->is_class(jenova::GlobalSettings::JenovaScriptType)) 
					{
						// Get C++ Script Object
						Ref<CPPScript> scriptResource = Object::cast_to<CPPScript>(cppResource.ptr());
						if (!scriptResource->HasValidScriptIdentity()) scriptResource->GenerateScriptIdentity();
						bool isUsedScript = usedScripts.contains(AS_STD_STRING(scriptResource->GetScriptIdentity()));

						// Verbose
						jenova::Output("Preprocessing C++ Script Resource ([color=#70a9d4]%s[/color]) [[color=#91b553]%s[/color]] [%s]",
							AS_C_STRING(scriptResource->get_path()), AS_C_STRING(scriptResource->GetScriptIdentity()), 
							isUsedScript ? "[color=#24ed49]Used[/color]" : "[color=#ed2456]Unused[/color]");

						// Set Per-Script Preprocessor Settings
						preprocessorSettings["PropertyMetadata"] = jenova::GetJenovaCacheDirectory() + scriptResource->get_path().get_file().get_basename() + "_" + scriptResource->GetScriptIdentity() + ".props";

						// Preprocess Source
						String preprocessedSource = jenovaCompiler->PreprocessScript(cppResource, preprocessorSettings);

						// Create Script Module
						jenova::ScriptModule scriptModule;
						scriptModule.scriptFilename = scriptResource->get_path();
						scriptModule.scriptUID = scriptResource->GetScriptIdentity();
						scriptModule.scriptType = isUsedScript ? jenova::ScriptModuleType::UsedScript : jenova::ScriptModuleType::UnusedScript;
						scriptModule.scriptSource = preprocessedSource;
						scriptModule.scriptHash = scriptModule.scriptSource.md5_text();

						// Detect Embedded Built-In Scripts
						if (scriptResource->is_built_in()) scriptModule.scriptType = jenova::ScriptModuleType::BuiltinScript;

						// Generate Script Cache and Object Filenames
						scriptModule.scriptCacheFile = jenova::GetJenovaCacheDirectory() + scriptResource->get_path().get_file().get_basename() + "_" + scriptResource->GetScriptIdentity() + ".cpp";
						scriptModule.scriptObjectFile = jenova::GetJenovaCacheDirectory() + scriptResource->get_path().get_file().get_basename() + "_" + scriptResource->GetScriptIdentity() + ".obj";
						scriptModule.scriptPropertiesFile = preprocessorSettings["PropertyMetadata"];

						// Create Preprocessed Source File
						Ref<FileAccess> handle = FileAccess::open(scriptModule.scriptCacheFile, FileAccess::ModeFlags::WRITE);
						if (handle.is_valid())
						{
							// Cache Preprocessed On Disk
							handle->store_string(scriptModule.scriptSource);
							handle->close();

							// Verbose
							jenova::Verbose("C++ Script ([color=#70a9d4]%s[/color]) Successfully Preprocessed.", AS_C_STRING(scriptResource->get_path()));
						}
						else
						{
							jenova::Error("Jenova Builder", "Failed to Preprocess Script File : [color=#70a9d4]%s[/color]", AS_C_STRING(scriptModule.scriptFilename));
							DisposeCompiler();
							return false;
						}

						// Add Script Module
						scriptModules.push_back(scriptModule);
					}
				}
				jenova::Output("All ([color=#53b5ab]%lld[/color]) C++ Script Resources Successfully Preprocessed.", cppResources.size());

				// Add Internal Sources
				if (jenova::GlobalSettings::BuildInternalSources)
				{
					// Add Module Loader Source [Godot Function Solver]
					jenova::ScriptModule moduleLoaderScript = jenova::CreateScriptModuleFromInternalSource("JenovaModuleLoader", std::string(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(JenovaModuleInitializerCPP))));
					if (moduleLoaderScript.scriptType != jenova::ScriptModuleType::InternalScript)
					{
						jenova::Error("Jenova Builder", "Failed to Create Function Solver Internal Script.");
						DisposeCompiler();
						return false;
					}
					scriptModules.push_back(moduleLoaderScript);
				}

				// Compile Scripts [Multi-Thread/Single-Thread]
				if (bool(jenovaCompiler->GetCompilerOption("cpp_multi_threaded_compilation")))
				{
					jenova::Output("Compiling ([color=#53b5ab]%lld[/color]) C++ Script Module%s ([color=#7834f7]Multi-Thread[/color])...", scriptModules.size(), scriptModules.size() == 1 ? "" : "s");

					// Create Compile Profiler Checkpoint
					JenovaTinyProfiler::CreateCheckpoint("JenovaCompileMT");

					// Compile Scripts By Module
					jenova::CompileResult compilerResult = jenovaCompiler->CompileScriptModuleContainer(jenova::ScriptModuleContainer(scriptModules));

					// Check for Compiler Result
					if (!compilerResult.compileResult)
					{
						if (compilerResult.hasError)
						{
							jenova::Error("Jenova Builder", "Compile Error :\n%s", AS_C_STRING(compilerResult.compileError));
						}

						// Compile Failed
						DisposeCompiler();
						return false;
					}

					// Check If Any Compile Was Done
					if (compilerResult.scriptsCount == 0)
					{
						jenova::OutputColored("#2ebc78", "Falling Back to Previous Build, No Change Detected. Fallback Time : [color=#c8e38a]%f ms[/color]", JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaCompileMT"));
					
						// Start Interpreter Execution
						JenovaInterpreter::SetExecutionState(true);

						// Call Build Success
						OnBuildSuccess();

						// Release Compiler
						DisposeCompiler();

						// Abort Build
						return true;
					}

					// Verbose
					jenova::Output("All Script Modules Compiled, Compile Time : [color=#c8e38a]%f ms[/color]", JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaCompileMT"));
				}
				else
				{
					jenova::Output("Compiling ([color=#53b5ab]%lld[/color]) C++ Script Module%s ([color=#2d2ded]Single-Thread[/color])...", scriptModules.size(), scriptModules.size() == 1 ? "" : "s");
					for (const auto& scriptModule : scriptModules)
					{
						// Create Compile Profiler Checkpoint
						JenovaTinyProfiler::CreateCheckpoint("JenovaCompileST");

						// Compile Script By Module
						jenova::CompileResult compilerResult = jenovaCompiler->CompileScriptModuleContainer(jenova::ScriptModuleContainer(scriptModule, scriptModules));

						// Check for Compiler Result
						if (!compilerResult.compileResult)
						{
							if (compilerResult.hasError)
							{
								jenova::Error("Jenova Builder", "Compile Error :\n%s", AS_C_STRING(compilerResult.compileError));
							}

							// Compile Failed
							DisposeCompiler();
							return false;
						}

						// Skip Verbosing Built-in Scripts
						if (scriptModule.scriptType == jenova::ScriptModuleType::InternalScript) continue;

						// Verbose
						jenova::Output("Script Module [%s] [%s] [%s] Compiled, Compile Time : %f ms",
							AS_C_STRING(scriptModule.scriptFilename), AS_C_STRING(scriptModule.scriptUID),
							scriptModule.scriptType == jenova::ScriptModuleType::UsedScript ? "Used" : "Unused",
							JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaCompileST"));
					}
				}

				// Create Build Profiler Checkpoint
				JenovaTinyProfiler::CreateCheckpoint("JenovaBuild");
				
				// Link And Generate Final Binary
				jenova::Output("Generating Module...");
				jenova::BuildResult buildResult = jenovaCompiler->BuildFinalModule(scriptModules);

				// Check for Build Result
				if (!buildResult.buildResult)
				{
					if (buildResult.hasError)
					{
						jenova::Error("Jenova Builder", "Build Error :\n%s", AS_C_STRING(buildResult.buildError));
					}

					// Build (Link) Failed
					DisposeCompiler();
					return false;
				}

				// Verbose Build Success
				jenova::Output("Module Generated, Generate Time : [color=#c8e38a]%f ms[/color]", JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaBuild"));
				jenova::OutputColored("#2ebc78", "Project Build Completed! Project Build Time : [color=#eb9234]%f ms[/color]", JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaProjectBuild"));

				// Cache Module To Database
				if (!JenovaInterpreter::CreateModuleDatabase(jenova::GlobalSettings::DefaultModuleDatabaseFile, buildResult))
				{
					jenova::Error("Jenova Interpreter", "Unable to Cache Jenova Module to Database.");
					DisposeCompiler();
					return false;
				}

				// Initialize Interpreter If Not Initialized Yet
				if (!JenovaInterpreter::IsInterpreterInitialized())
				{
					if (!JenovaInterpreter::InitializeInterpreter())
					{
						jenova::Error("Jenova Interpreter", "Jenova Interpreter Failed to Initialize!");
						quick_exit(jenova::ErrorCode::INTERPRETER_INIT_FAILED);
					}
				}

				// Load Built Module
				if (JenovaInterpreter::GetModuleBaseAddress() == 0)
				{
					if (!JenovaInterpreter::LoadModule(buildResult))
					{
						jenova::Error("Jenova Interpreter", "Unable to Load Compiled Jenova Module, Check for Missing Dependencies.");
						DisposeCompiler();
						return false;
					}
				}
				else
				{
					// Pause Tree
					get_tree()->set_pause(true);

					// Reloading
					if (!JenovaInterpreter::ReloadModule(buildResult))
					{
						jenova::Error("Jenova Interpreter", "Unable to Reload Compiled Jenova Module, Check for Missing Dependencies.");
						DisposeCompiler();
						return false;
					}
				
					// Resume Tree
					get_tree()->set_pause(false);
				}

				// Update Script Instances 
				if (!ReloadJenovaScriptInstances())
				{
					jenova::Warning("Jenova Builder", "Failed to Reload Script Instances!");
				}

				// Call Build Success
				OnBuildSuccess();

				// Release Compiler & Clean Up
				DisposeCompiler();

				// All Good
				return true;
			}
			void CleanProject()
			{
				// Switch to Jenova Terminal Tab
				if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal && jenovaTerminal)
				{
					this->make_bottom_panel_item_visible(jenovaTerminal);
				}

				// Create Project Build Profiler Checkpoint
				JenovaTinyProfiler::CreateCheckpoint("JenovaProjectClean");

				// Verbose
				jenova::Output("Cleaning Project Cache...");
			
				// Get Jenova Cache Path
				std::string jenovaCacheDirectory = AS_STD_STRING(jenova::GetJenovaCacheDirectory());
			
				// Validate Jenova Cache Folder
				if (!std::filesystem::exists(jenovaCacheDirectory))
				{
					jenova::Output("No Cache Folder to Delete.");
					JenovaTinyProfiler::DeleteCheckpoint("JenovaProjectClean");
					return;
				}

				// Check If Any Cache Exist
				if (std::filesystem::is_empty(jenovaCacheDirectory))
				{
					jenova::Output("No Cache Files to Delete. The Cache is Already Empty.");
					JenovaTinyProfiler::DeleteCheckpoint("JenovaProjectClean");
					return;
				}

				// Delete Files And Directories
				try 
				{
					for (const auto& entry : std::filesystem::directory_iterator(jenovaCacheDirectory)) 
					{
						if (entry.is_regular_file()) 
						{
							std::filesystem::remove(entry.path());
							jenova::Output("Cache File ([color=#70a9d4]%s[/color]) Deleted.", entry.path().string().c_str());
						}
						else if (entry.is_directory())
						{
							std::filesystem::remove_all(entry.path());
							jenova::Output("Cache Directory ([color=#70a9d4]%s[/color]) Deleted.", entry.path().string().c_str());
						}
					}
				}
				catch (const std::filesystem::filesystem_error& e) 
				{
					jenova::Error("Jenova Builder", "Failed to Clean Jenova Cache Directory.");
				}

				// Verbose
				jenova::OutputColored("#4287f5", "Project Cleaned, Cleaning Time : [color=#eb9234]%f ms[/color]", JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaProjectClean"));
			}

			// Module Bootstraper
			bool BootstrapModule(const String& jenovaConfig)
			{
				// Switch to Jenova Terminal Tab
				if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal && jenovaTerminal)
				{
					this->make_bottom_panel_item_visible(jenovaTerminal);
				}

				// Stop Interpreter Execution
				JenovaInterpreter::SetExecutionState(false);

				// Verbose
				jenova::Output("Bootstrapping Generated Module...");
				
				// Update Settings
				if (!UpdateStorageConfigurations())
				{
					jenova::Error("Jenova Bootstraper", "Failed to Update Configurations From Editor Settings.");
					return false;
				}

				// Create Module Bootstrap Profiler Checkpoint
				JenovaTinyProfiler::CreateCheckpoint("JenovaBootstrapModule");

				// Parse And Progress Jenova Configuration
				nlohmann::json jenovaConfiguration;
				try
				{
					// Deserialize Configuration
					jenovaConfiguration = nlohmann::json::parse(AS_STD_STRING(jenovaConfig));
				}
				catch (const std::exception& error)
				{
					jenova::Error("Jenova Bootstraper", "Failed to Bootstrap Jenova Module, Configuration Parsing Failed.");
					return false;
				}

				// Reload All Scripts Resources
				jenova::ResourceCollection projectScripts;
				jenova::CollectResourcesFromFileSystem("res://", "cpp", projectScripts);
				for (const auto& projectScript : projectScripts)
				{
					if (projectScript->is_class(jenova::GlobalSettings::JenovaScriptType))
					{
						Ref<CPPScript> cppScript = Object::cast_to<CPPScript>(projectScript.ptr());
						String newSourceCode = jenova::ReadStringFromFile(cppScript->get_path());
						if (!newSourceCode.is_empty()) cppScript->set_source_code(newSourceCode);
						std::string cppScriptFilePath = AS_STD_STRING(ProjectSettings::get_singleton()->globalize_path(cppScript->get_path()));
						jenova::ScriptFileState cppScriptFileState = jenova::BackupScriptFileState(cppScriptFilePath);
						ResourceSaver::get_singleton()->save(cppScript, cppScript->get_path());
						jenova::RestoreScriptFileState(cppScriptFilePath, cppScriptFileState);
						cppScript->emit_changed();
					}
				}

				// Bootstrap Module
				try
				{
					// Get Configuration From Database
					bool hasDebugInformation = jenovaConfiguration["GenerateDebugInformation"].get<bool>();
					int scriptCount = jenovaConfiguration["ScriptsCount"].get<int>();

					// Generate Module And Map Paths
					std::string modulePath = jenova::GlobalStorage::CurrentJenovaCacheDirectory + "Jenova.Module" + (hasDebugInformation ? ".dll" : ".jnv");
					std::string mapPath = jenova::GlobalStorage::CurrentJenovaCacheDirectory + "Jenova.Module.map";

					// Validate Files
					if (!std::filesystem::exists(modulePath) || !std::filesystem::exists(mapPath))
					{
						jenova::Error("Jenova Bootstraper", "Failed to Validate Module or Map File, Aborted.");
						return false;
					}

					// Read Module to Memory Buffer
					jenova::MemoryBuffer moduleData = jenova::ReadMemoryBufferFromFile(modulePath);
					if (moduleData.size() == 0)
					{
						jenova::Error("Jenova Bootstraper", "Failed to Read Module Data, Aborted.");
						return false;
					}

					// Get Extra Required Values
					String intermediatePath(std::filesystem::absolute(jenovaConfiguration["IntermediatePath"].get<std::string>()).string().c_str());

					// Create Script Modules
					jenova::ModuleList scriptModules;
					for (int i = 0; i < scriptCount; i++)
					{
						// Get Script Info
						String scriptFilePath(jenovaConfiguration["ScriptEntities"][i]["ScriptFile"].get<std::string>().c_str());

						// Create Script Module
						ScriptModule scriptModule;
						scriptModule.scriptUID = String(jenovaConfiguration["ScriptEntities"][i]["ScriptIdentity"].get<std::string>().c_str());
						scriptModule.scriptFilename = intermediatePath + scriptFilePath.get_basename().get_file() + "_" + scriptModule.scriptUID + ".cpp";
						scriptModule.scriptPropertiesFile = intermediatePath + scriptFilePath.get_basename().get_file() + "_" + scriptModule.scriptUID + ".props";
						scriptModule.scriptType = jenova::ScriptModuleType::BootstrapScript;
						scriptModules.push_back(scriptModule);
					}

					// Create Build Result
					jenova::BuildResult buildResult;
					buildResult.hasError = false;
					buildResult.buildResult = true;
					buildResult.builtModuleData = moduleData;
					buildResult.buildPath = jenova::GlobalStorage::CurrentJenovaCacheDirectory;
					buildResult.compilerModel = jenova::CompilerModel::MicrosoftCompiler;
					buildResult.hasDebugInformation = hasDebugInformation;

					// Generate Metadata
					buildResult.moduleMetaData = JenovaInterpreter::GenerateModuleMetadata(mapPath, scriptModules, buildResult);
					if (buildResult.moduleMetaData.empty())
					{
						jenova::Error("Jenova Bootstraper", "Failed to Genereate Module Metadata, Aborted.");
						return false;
					}

					// Cache Module To Database
					if (!JenovaInterpreter::CreateModuleDatabase(jenova::GlobalSettings::DefaultModuleDatabaseFile, buildResult))
					{
						jenova::Error("Jenova Interpreter", "Unable to Cache Bootstrapped Jenova Module to Database.");
						return false;
					}

					// Load Built Module
					if (JenovaInterpreter::GetModuleBaseAddress() == 0)
					{
						if (!JenovaInterpreter::LoadModule(buildResult))
						{
							jenova::Error("Jenova Interpreter", "Unable to Load Bootstrapped Jenova Module, Check for Missing Dependencies.");
							return false;
						}
					}
					else
					{
						// Pause Tree
						get_tree()->set_pause(true);

						// Reloading
						if (!JenovaInterpreter::ReloadModule(buildResult))
						{
							jenova::Error("Jenova Interpreter", "Unable to Reload Bootstrapped Jenova Module, Check for Missing Dependencies.");
							return false;
						}

						// Resume Tree
						get_tree()->set_pause(false);
					}

					// Release Buffers
					jenova::MemoryBuffer().swap(moduleData);
				}
				catch (const std::exception& err)
				{
					jenova::Error("Jenova Bootstraper", "Failed to Bootstrap Jenova Module, Something Went Wrong! > %s", err.what());
					return false;
				}

				// Update Script Instances 
				if (!ReloadJenovaScriptInstances())
				{
					jenova::Warning("Jenova Builder", "Failed to Reload Script Instances!");
				}

				// Verbose
				jenova::OutputColored("#2ebc78", "Build Bootstrapping Completed! Bootstrap Time : [color=#eb9234]%f ms[/color]", JenovaTinyProfiler::GetCheckpointTimeAndDispose("JenovaBootstrapModule"));

				// Call Build Success
				OnBuildSuccess();

				// If Debug Build Running Notify It
				if (jenova::GlobalStorage::UseHotReloadAtRuntime && EditorInterface::get_singleton()->is_playing_scene())
				{
					Array currentSessions = ((Ref<EditorDebuggerPlugin>)debuggerPlugin)->get_sessions();
					for (size_t i = 0; i < currentSessions.size(); i++)
					{
						Ref<EditorDebuggerSession> debuggerSession = currentSessions[i];
						if (debuggerSession.is_valid() && debuggerSession->is_active()) debuggerSession->send_message("Jenova-Runtime:Reload");
					}
				}

				// All Good
				return true;
			}

			// Compiler Helpers
			bool CreateCompiler()
			{
				// Get Compiler Model
				Variant compilerModel;
				if (!GetEditorSetting(CompilerModelConfigPath, compilerModel)) compilerModel = int32_t(CompilerDefaultModel);

				// Get Compiler Extra Settings
				Variant useMultiThreading;
				if (!GetEditorSetting(MultiThreadedCompilationConfigPath, useMultiThreading)) return false;
				Variant generateDebugInformation;
				if (!GetEditorSetting(GenerateDebugInformationConfigPath, generateDebugInformation)) return false;
				Variant additionalIncludeDirectories;
				if (!GetEditorSetting(AdditionalIncludeDirectoriesConfigPath, additionalIncludeDirectories)) return false;
				Variant additionalLibraryDirectories;
				if (!GetEditorSetting(AdditionalLibraryDirectoriesConfigPath, additionalLibraryDirectories)) return false;
				Variant additionalDependencies;
				if (!GetEditorSetting(AdditionalDependenciesConfigPath, additionalDependencies)) return false;
				Variant compilerPackage;
				if (!GetEditorSetting(CompilerPackageConfigPath, compilerPackage)) return false;
				Variant godotKitPackage;
				if (!GetEditorSetting(GodotKitPackageConfigPath, godotKitPackage)) return false;

				// Initialize Compiler Compiler
				switch (jenova::CompilerModel(int32_t(compilerModel)))
				{
				case jenova::CompilerModel::MicrosoftCompiler:
					jenova::Output("Creating Microsoft Visual C++ (MSVC) Compiler...");
					jenovaCompiler = new jenova::MicrosoftCompiler();
					jenova::Output("New Microsoft Visual C++ (MSVC) Compiler Implemented at [color=#44e376]%p[/color]", jenovaCompiler);
					break;
				case jenova::CompilerModel::ClangCompiler:
					jenova::Output("Creating LLVM CLang Compiler...");
					jenovaCompiler = nullptr; // Not Implemented Yet
					break;
				case jenova::CompilerModel::MinGWCompiler:
					jenova::Output("Creating MinGW Compiler...");
					jenovaCompiler = nullptr; // Not Implemented Yet
					break;
				default:
					jenova::Error("Jenova Builder", "Invalid Compiler Model detected, Build aborted.");
					return false;
				}

				// Validate Compiler
				if (!jenovaCompiler)
				{
					jenova::Error("Jenova Builder", "No valid Compiler has been detected, Build aborted.");
					return false;
				}

				// Initialize Compiler
				if (!jenovaCompiler->InitializeCompiler())
				{
					jenova::Error("Jenova Builder", "Compiler failed to initialize! Build aborted.");
					DisposeCompiler();
					return false;
				};

				// Set Compiler Extra Settings
				if (!jenovaCompiler->SetCompilerOption("cpp_multi_threaded_compilation", bool(useMultiThreading)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'Multi-Threaded Compilation'");
					DisposeCompiler();
					return false;
				};
				if (!jenovaCompiler->SetCompilerOption("cpp_generate_debug_info", bool(generateDebugInformation)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'Generate Debug Information'");
					DisposeCompiler();
					return false;
				};
				if (!jenovaCompiler->SetCompilerOption("cpp_extra_include_directories", String(additionalIncludeDirectories)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'Additional Include Directories'");
					DisposeCompiler();
					return false;
				};
				if (!jenovaCompiler->SetCompilerOption("cpp_extra_library_directories", String(additionalLibraryDirectories)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'Additional Library Directories'");
					DisposeCompiler();
					return false;
				};
				if (!jenovaCompiler->SetCompilerOption("cpp_extra_libs", String(additionalDependencies)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'Additional Dependencies'");
					DisposeCompiler();
					return false;
				};
				if (!jenovaCompiler->SetCompilerOption("cpp_toolchain_path", String(compilerPackage)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'Compiler Package'");
					DisposeCompiler();
					return false;
				};
				if (!jenovaCompiler->SetCompilerOption("cpp_godotsdk_path", String(godotKitPackage)))
				{
					jenova::Error("Jenova Builder", "Failed to Set Compiler Setting 'GodotKit Package'");
					DisposeCompiler();
					return false;
				};

				// All Good
				return true;
			}
			void DisposeCompiler()
			{
				if (jenovaCompiler != nullptr)
				{
					jenovaCompiler->ReleaseCompiler();
					delete jenovaCompiler;
					jenovaCompiler = nullptr;
				}
			}

			// Terminal Actions
			void UpdateTerminal()
			{
				// Validate Log Output
				if (!jenovaLogOutput) return;

				// Customize Font
				if (jenova::GlobalStorage::UseMonospaceFontForTerminal)
				{
					// Create Terminal Font
					Ref<FontFile> terminalNormalFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_TERMINAL_NORMAL)));
					terminalNormalFont->set_antialiasing(TextServer::FontAntialiasing::FONT_ANTIALIASING_GRAY);
					terminalNormalFont->set_subpixel_positioning(TextServer::SubpixelPositioning::SUBPIXEL_POSITIONING_DISABLED);
					Ref<FontFile> terminalBoldFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_TERMINAL_BOLD)));
					terminalBoldFont->set_antialiasing(TextServer::FontAntialiasing::FONT_ANTIALIASING_GRAY);
					terminalBoldFont->set_subpixel_positioning(TextServer::SubpixelPositioning::SUBPIXEL_POSITIONING_DISABLED);

					// Set New Fonts and Font Size
					jenovaLogOutput->add_theme_font_override("normal_font", terminalNormalFont);
					jenovaLogOutput->add_theme_font_override("bold_font", terminalBoldFont);
					jenovaLogOutput->add_theme_font_size_override("normal_font_size", jenova::GlobalStorage::TerminalDefaultFontSize * EditorInterface::get_singleton()->get_editor_scale());
					jenovaLogOutput->add_theme_font_size_override("bold_font_size", jenova::GlobalStorage::TerminalDefaultFontSize * EditorInterface::get_singleton()->get_editor_scale());
				}
				else
				{
					// Reset Fonts Back
					jenovaLogOutput->remove_theme_font_override("normal_font");
					jenovaLogOutput->remove_theme_font_override("bold_font");
					jenovaLogOutput->remove_theme_font_size_override("normal_font_size");
					jenovaLogOutput->remove_theme_font_size_override("bold_font_size");
				}
			}
			void VerboseLog(const String& logMessage)
			{
				if (jenovaLogOutput) 
				{
					// Add Timstamp
					jenovaLogOutput->append_text(" [color=#787a76][" + String(jenova::GenerateTerminalLogTime().c_str()) + "][/color]");

					// Add Log Message
					jenovaLogOutput->append_text(logMessage);
					jenovaLogOutput->append_text("\n");
				}
			}
			void ClearLogs()
			{
				if (jenovaLogOutput)
				{
					jenovaLogOutput->clear();
				}
			}
			void CopyLogs()
			{
				if (jenovaLogOutput)
				{
					jenova::CopyStringToClipboard(jenovaLogOutput->get_parsed_text());
					jenova::OutputColored("#40403f", "Terminal Logs Copied to Clipboard.");
				}
			}

			// User Interface Helpers
			Control* GetEditorContainerNode()
			{
				return Object::cast_to<Control>(EditorInterface::get_singleton()->get_base_control());
			}
			Control* FindFirstControlByClassName(Node* currentNode, const String& className)
			{
				if (currentNode->get_class() == className) return Object::cast_to<Control>(currentNode);
				for (int i = 0; i < currentNode->get_child_count(); ++i)
				{
					Control* result = FindFirstControlByClassName(currentNode->get_child(i), className);
					if (result) return result;
				}
				return nullptr;
			}
			Ref<Shortcut> CreateShortcut(const String& p_name, Key p_keycode = Key::KEY_NONE, bool p_physical = false)
			{
				// Create New Shortcut
				Ref<Shortcut> newShortcut;
				newShortcut.instantiate();
				newShortcut->set_name(p_name);

				// If Contains HotKey Create Event and Assign It
				if (p_keycode != Key::KEY_NONE) 
				{
					// Create Input Event
					Ref<InputEventKey> inputEvent = CreateInputEventKeyReference(p_keycode, p_physical);

					// Set Events
					Array events;
					events.push_back(inputEvent);
					newShortcut->set_events(events);
				}

				// Return Created Shortcut
				return newShortcut;
			}
			Ref<InputEventKey> CreateInputEventKeyReference(Key p_keycode, bool p_physical) 
			{
				// Create New Input Event Key
				Ref<InputEventKey> ie;
				ie.instantiate();

				// Set Physical/Virtual Key
				if (p_physical) ie->set_physical_keycode(Key(p_keycode & KeyModifierMask::KEY_CODE_MASK));
				else ie->set_keycode(Key(p_keycode & KeyModifierMask::KEY_CODE_MASK));

				// Set Unicode
				char32_t ch = char32_t(p_keycode & KeyModifierMask::KEY_CODE_MASK);
				if (ch < 0xd800 || (ch > 0xdfff && ch <= 0x10ffff)) ie->set_unicode(ch);

				// Update Modifier Keys
				if ((p_keycode & KeyModifierMask::KEY_MASK_SHIFT) != Key::KEY_NONE) ie->set_shift_pressed(true);
				if ((p_keycode & KeyModifierMask::KEY_MASK_ALT) != Key::KEY_NONE) ie->set_alt_pressed(true);
				if ((p_keycode & KeyModifierMask::KEY_MASK_CMD_OR_CTRL) != Key::KEY_NONE) ie->set_command_or_control_autoremap(true);
				if ((p_keycode & KeyModifierMask::KEY_MASK_CMD_OR_CTRL) == Key::KEY_NONE)
				{
					if ((p_keycode & KeyModifierMask::KEY_MASK_CTRL) != Key::KEY_NONE) ie->set_ctrl_pressed(true);
					if ((p_keycode & KeyModifierMask::KEY_MASK_META) != Key::KEY_NONE) ie->set_meta_pressed(true);
				}

				// Return Created Input Event Key
				return ie;
			}

			// Utiltiies
			bool CreateJenovaConfiguration(const jenova::ScriptEntityContainer& scriptEntityContainer, bool exportOnDisk = false)
			{
				// Jenova Configurations Serializer
				nlohmann::json jenovaConfiguration;

				// Get User-Defined Macros
				Variant preprocessorDefinitions;
				if (!GetEditorSetting(PreprocessorDefinitionsConfigPath, preprocessorDefinitions))
				{
					jenova::Error("Jenova Utilities", "Failed to Obtain Preprocessor Definitions.");
					return false;
				}
				Variant additionalIncludeDirectories;
				if (!GetEditorSetting(AdditionalIncludeDirectoriesConfigPath, additionalIncludeDirectories)) return false;
				Variant additionalLibraryDirectories;
				if (!GetEditorSetting(AdditionalLibraryDirectoriesConfigPath, additionalLibraryDirectories)) return false;
				Variant additionalDependencies;
				if (!GetEditorSetting(AdditionalDependenciesConfigPath, additionalDependencies)) return false;
				Variant generateDebugInformation;
				if (!GetEditorSetting(GenerateDebugInformationConfigPath, generateDebugInformation)) return false;

				// Serialize Configuration
				try
				{
					// Serialize Configurations
					jenovaConfiguration["PreprocessorDefinitions"] = AS_STD_STRING(String(preprocessorDefinitions));
					jenovaConfiguration["AdditionalIncludeDirectories"] = AS_STD_STRING(String(additionalIncludeDirectories));
					jenovaConfiguration["AdditionalLibraryDirectories"] = AS_STD_STRING(String(additionalLibraryDirectories));
					jenovaConfiguration["AdditionalDependencies"] = AS_STD_STRING(String(additionalDependencies));
					jenovaConfiguration["GenerateDebugInformation"] = bool(generateDebugInformation);

					// Serialize Scripts Count
					jenovaConfiguration["ScriptsCount"] = scriptEntityContainer.entityCount;

					// Serialize Scripts Paths & Identities
					jenovaConfiguration["ScriptEntities"] = nlohmann::json::array();
					for (size_t i = 0; i < scriptEntityContainer.entityCount; i++)
					{
						nlohmann::json::object_t scriptEntitySerializer;
						scriptEntitySerializer["ScriptIdentity"] = AS_STD_STRING(scriptEntityContainer.scriptModules[i].scriptUID);
						std::string scriptPathEncoded(scriptEntityContainer.scriptFilesReleative[i]);
						jenova::ReplaceAllMatchesWithString(scriptPathEncoded, "\\", "/");
						scriptEntitySerializer["ScriptFile"] = scriptPathEncoded;
						jenovaConfiguration["ScriptEntities"].push_back(scriptEntitySerializer);
					}

					// Serialize Jenova Version
					jenovaConfiguration["RuntimeVersion"] = std::string(APP_VERSION);
				}
				catch (const std::exception& error)
				{
					jenova::Error("Jenova Utilities", "Failed to Serialize Jenova Configurations. Error : %s", error.what());
					return false;
				}

				// Export Jenova Configurations
				jenova::GlobalStorage::CurrentJenovaGeneratedConfiguration = jenovaConfiguration.dump(2);
				String configurationFilePath = jenova::GetJenovaCacheDirectory() + String(jenova::GlobalSettings::JenovaConfigurationFile);

				// If Required to be Written On Disk
				if (exportOnDisk) return jenova::WriteStringToFile(configurationFilePath, String(jenova::GlobalStorage::CurrentJenovaGeneratedConfiguration.c_str()));

				// All Good
				return true;
			}
			bool ReloadJenovaScriptInstances()
			{
				// All Good
				return true;
			}
			String GetEditorSettingStringPath(const std::string& setting_key)
			{
				if (setting_key == std::string("remove_source_codes_from_build")) return RemoveSourcesFromBuildEditorConfigPath;
				if (setting_key == std::string("compiler_model")) return CompilerModelConfigPath;
				if (setting_key == std::string("multi_threaded_compilation")) return MultiThreadedCompilationConfigPath;
				if (setting_key == std::string("generate_debug_information")) return GenerateDebugInformationConfigPath;
				if (setting_key == std::string("interpreter_backend")) return InterpreterBackendConfigPath;
				if (setting_key == std::string("build_and_run_mode")) return BuildAndRunModeConfigPath;
				if (setting_key == std::string("preprocessor_definitions")) return PreprocessorDefinitionsConfigPath;
				if (setting_key == std::string("additional_include_directories")) return AdditionalIncludeDirectoriesConfigPath;
				if (setting_key == std::string("additional_library_directories")) return AdditionalLibraryDirectoriesConfigPath;
				if (setting_key == std::string("additional_dependencies")) return AdditionalDependenciesConfigPath;
				if (setting_key == std::string("external_changes_trigger_mode")) return ExternalChangesTriggerModeConfigPath;
				if (setting_key == std::string("use_hot_reload_at_runtime")) return UseHotReloadAtRuntimeConfigPath;
				if (setting_key == std::string("editor_verbose_output")) return EditorVerboseOutputConfigPath;
				if (setting_key == std::string("use_monospace_font_for_terminal")) return UseMonospaceFontForTerminalConfigPath;
				if (setting_key == std::string("terminal_default_font_size")) return TerminalDefaultFontSizeConfigPath;
				if (setting_key == std::string("compiler_package")) return CompilerPackageConfigPath;
				if (setting_key == std::string("godot_kit_package")) return GodotKitPackageConfigPath;
				if (setting_key == std::string("build_toolbutton_placement")) return BuildToolButtonEditorConfigPath;
				return String("jenova/unknown");
			}

			// Visual Studio Integration
			void OpenVisualStudioSelectorWindow()
			{
				// Collect Visual Studio Instances
				try
				{
					// Clear Detected Visual Studio Instances
					vsInstances.clear();

					// Get Installed Visual Studios Metadata
					nlohmann::json vsMetadata = nlohmann::json::parse(GetVistualStudioMetadata());

					// Create Data
					for (const auto& vsInstanceData : vsMetadata["Instances"])
					{
						VisualStudioInstance vsInstance;
						vsInstance.instanceName = String(vsInstanceData["Name"].get<std::string>().c_str());
						vsInstance.instanceVersion = String(vsInstanceData["Version"].get<std::string>().c_str());
						vsInstance.majorVersion = String(jenova::ExtractMajorVersionFromFullVersion(AS_STD_STRING(vsInstance.instanceVersion)).c_str());
						vsInstance.platformToolset = String(jenova::GetVisualStudioPlatformToolsetFromVersion(AS_STD_STRING(vsInstance.majorVersion)).c_str());
						vsInstance.productName = String(vsInstanceData["Product"].get<std::string>().c_str());
						vsInstance.productYear = String(vsInstanceData["Year"].get<std::string>().c_str());
						vsInstances.push_back(vsInstance);
					}
				}
				catch (const std::exception&)
				{
					jenova::Error("Jenova Vistual Studio Locator", "Failed to Parse Visual Studio Metadata.");
					return;
				}

				// Check If No Instance Exist
				if (vsInstances.size() == 0)
				{
					jenova::Warning("Jenova Vistual Studio Locator", "No Visual Studio Instance Detected, Generating With Default Settings.");
					VisualStudioInstance vsInstance;
					vsInstance.instanceName = "Visual Studio Enterprise 2022";
					vsInstance.instanceVersion = "17.8.34330.188";
					vsInstance.platformToolset = "v143";
					vsInstance.majorVersion = "17";
					if (!ExportVisualStudioProject(vsInstance)) jenova::Error("Jenova Utilities", "Failed to Export Project to Visual Studio Solution.");
					return;
				}

				// Get Scale Factor
				double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

				// Create Window
				Window* vs_selector_window = memnew(Window);
				vs_selector_window->set_title("Visual Studio Selector");
				vs_selector_window->set_size(Vector2i(SCALED(400), SCALED(270)));
				vs_selector_window->set_flag(Window::Flags::FLAG_RESIZE_DISABLED, true);
				vs_selector_window->set_flag(Window::Flags::FLAG_POPUP, true);

				// Show Window [Must Be Here]
				vs_selector_window->popup_exclusive_centered(EditorInterface::get_singleton()->get_base_control());
				if (!vs_selector_window->get_flag(Window::Flags::FLAG_POPUP)) EditorInterface::get_singleton()->get_base_control()->get_tree()->set_pause(true);

				// Get Editor Theme
				Ref<Theme> editor_theme = EditorInterface::get_singleton()->get_editor_theme();
				if (!editor_theme.is_valid())
				{
					jenova::Error("Jenova Vistual Studio Locator", "Failed to Obtain Engine Theme.");
					return;
				}

				// Create Window Controls
				Control* root_control = memnew(Control);
				root_control->set_name("VsSelectorWindow");
				root_control->set_anchors_preset(Control::PRESET_FULL_RECT);
				vs_selector_window->add_child(root_control);
				
				ColorRect* background = memnew(ColorRect);
				background->set_name("Background");
				background->set_anchors_preset(Control::PRESET_FULL_RECT);
				background->set_color(editor_theme->get_color("base_color", "Editor"));
				root_control->add_child(background);

				Control* window_surface = memnew(Control);
				window_surface->set_name("WindowSurface");
				window_surface->set_anchors_and_offsets_preset(Control::PRESET_TOP_LEFT);
				window_surface->set_anchor_and_offset(Side::SIDE_LEFT, 0.0, SCALED(30.0));
				window_surface->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(15.0));
				window_surface->set_anchor_and_offset(Side::SIDE_RIGHT, 0.0, SCALED(370.0));
				window_surface->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(250.0));
				root_control->add_child(window_surface);

				Label* instances_label = memnew(Label);
				instances_label->set_name("InstancesLabel");
				instances_label->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				instances_label->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(22.0));
				instances_label->add_theme_font_size_override("font_size", SCALED(15));
				instances_label->set_text("Available Instances");
				window_surface->add_child(instances_label);

				OptionButton* instances_selector = memnew(OptionButton);
				instances_selector->set_name("InstancesSelector");
				instances_selector->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				instances_selector->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(29.0));
				instances_selector->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(74.0));
				instances_selector->add_theme_color_override("font_color", editor_theme->get_color("accent_color", "Editor"));
				for (const auto& vsInstance : vsInstances) instances_selector->add_item("  " + vsInstance.instanceName);
				instances_selector->select(instances_selector->get_item_count() - 1);
				window_surface->add_child(instances_selector);

				Label* toolset_label = memnew(Label);
				toolset_label->set_name("ToolsetLabel");
				toolset_label->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				toolset_label->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(90.0));
				toolset_label->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(112.0));
				toolset_label->add_theme_font_size_override("font_size", SCALED(15));
				toolset_label->set_text("Platform Toolset");
				window_surface->add_child(toolset_label);

				OptionButton* toolset_selector = memnew(OptionButton);
				toolset_selector->set_name("ToolsetSelector");
				toolset_selector->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				toolset_selector->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(119.0));
				toolset_selector->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(164.0));
				toolset_selector->add_theme_color_override("font_color", editor_theme->get_color("accent_color", "Editor"));
				for (const auto& vsInstance : vsInstances)
				{
					String toolsetItem = "  " + vsInstance.productName + " " + vsInstance.productYear + " (" + vsInstance.platformToolset + ")";
					toolset_selector->add_item(toolsetItem);
				}
				toolset_selector->select(toolset_selector->get_item_count() - 1);
				window_surface->add_child(toolset_selector);

				Button* generate_button = memnew(Button);
				generate_button->set_name("GenerateButton");
				generate_button->set_anchors_and_offsets_preset(Control::PRESET_CENTER_BOTTOM);
				generate_button->set_anchor_and_offset(Side::SIDE_LEFT, 0.5, SCALED(-85.0));
				generate_button->set_anchor_and_offset(Side::SIDE_TOP, 1.0, SCALED(-50.0));
				generate_button->set_anchor_and_offset(Side::SIDE_RIGHT, 0.5, SCALED(85.0));
				generate_button->set_anchor_and_offset(Side::SIDE_BOTTOM, 1.0, SCALED(1.0));
				generate_button->set_text("   Generate Solution   ");
				window_surface->add_child(generate_button);

				// Define Internal UI Callback Handler
				class VSSelectorEventManager : public RefCounted
				{
				private:
					JenovaEditorPlugin* pluginInstance = nullptr;
					Window* window = nullptr;

				public:
					VSSelectorEventManager(JenovaEditorPlugin* _plugin, Window* _window) : pluginInstance(_plugin), window(_window) { }
					void OnGenerateButtonClick()
					{
						OptionButton* instances_selector = window->get_node<OptionButton>("VsSelectorWindow/WindowSurface/InstancesSelector");
						VisualStudioInstance& vsInstance = pluginInstance->GetVisualStudioInstance(instances_selector->get_selected_id());
						if (!pluginInstance->ExportVisualStudioProject(vsInstance))
						{
							jenova::Error("Jenova Utilities", "Failed to Export Project to Visual Studio Solution.");
						}
						EditorInterface::get_singleton()->get_base_control()->get_tree()->set_pause(false);
						window->queue_free();
						memdelete(this);
					}
					void OnWindowClose()
					{
						EditorInterface::get_singleton()->get_base_control()->get_tree()->set_pause(false);
						window->queue_free();
						memdelete(this);
					}
				};

				// Create & Assign Callbacks
				generate_button->connect("pressed", callable_mp(memnew(VSSelectorEventManager(this, vs_selector_window)), &VSSelectorEventManager::OnGenerateButtonClick));
				vs_selector_window->connect("close_requested", callable_mp(memnew(VSSelectorEventManager(this, vs_selector_window)), &VSSelectorEventManager::OnWindowClose));

				// Prepare Pop Up Window
				if (!jenova::AssignPopUpWindow(vs_selector_window))
				{
					vs_selector_window->queue_free();
				}
			}
			bool ExportVisualStudioProject(const VisualStudioInstance& vsInstance)
			{
				// Verbose
				jenova::Output("Initializing Visual Studio Solution Exporter...");
				jenova::Output("Targeting [color=#c78fe3]%s ([color=#b765e0]%s[/color])[/color]", AS_C_STRING(vsInstance.instanceName), AS_C_STRING(vsInstance.instanceVersion));

				// Generate Script Collection
				jenova::ScriptEntityContainer scriptCollection = jenova::CreateScriptEntityContainer("res://");

				// Update Jenova Configuration File
				if (!CreateJenovaConfiguration(scriptCollection)) return false;

				// Create Visual Studio Files Path
				std::string solutionFile		= scriptCollection.rootPath + jenova::GlobalSettings::VisualStudioSolutionFile;
				std::string projectFile			= scriptCollection.rootPath + jenova::GlobalSettings::VisualStudioProjectFile;
				std::string projectFiltersFile	= scriptCollection.rootPath + jenova::GlobalSettings::VisualStudioProjectFile + ".filters";
				std::string projectUserFile		= scriptCollection.rootPath + jenova::GlobalSettings::VisualStudioProjectFile + ".user";
				std::string gitIgnoreFile		= scriptCollection.rootPath + ".gitignore";

				// Generate Solution File
				jenova::Output("Generating Visual Studio Solution...");
				std::string soultionTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_SOLUTION_TEMPLATE));
				jenova::ReplaceAllMatchesWithString(soultionTemplate, "@@VISUAL_STUDIO_VERSION@@", AS_C_STRING(vsInstance.majorVersion));
				jenova::ReplaceAllMatchesWithString(soultionTemplate, "@@VISUAL_STUDIO_VERSION_FULL@@", Format("%s", AS_C_STRING(vsInstance.instanceVersion)));
				if (!jenova::WriteStdStringToFile(solutionFile, soultionTemplate)) return false;

				// Create Compiler [For Obtaining Settings Only]
				if (!CreateCompiler()) return false;

				// Get Settings From Compiler
				bool generateDebugInformation = bool(jenovaCompiler->GetCompilerOption("cpp_generate_debug_info"));
				bool openMPSupport = bool(jenovaCompiler->GetCompilerOption("cpp_open_mp_support"));
				std::string languageStandard = AS_STD_STRING(String(jenovaCompiler->GetCompilerOption("cpp_language_standards")));
				std::string cpp_definitions = AS_STD_STRING(String(jenovaCompiler->GetCompilerOption("cpp_definitions")));
				std::string extraIncludeDirectories = AS_STD_STRING(String(jenovaCompiler->GetCompilerOption("cpp_extra_include_directories")));
				std::string extralibraryDirectories = AS_STD_STRING(String(jenovaCompiler->GetCompilerOption("cpp_extra_library_directories")));
				std::string extraLibraries = AS_STD_STRING(String(jenovaCompiler->GetCompilerOption("cpp_extra_libs")));
				
				// Solve GodotKit Path
				String selectedGodotKitPath = jenova::GetInstalledGodotKitPathFromPackages(jenovaCompiler->GetCompilerOption("cpp_godotsdk_path"));
				if (selectedGodotKitPath == "Missing-GodotKit-1.0.0")
				{
					jenova::Error("Visual Studio Exporter", "No GodotSDK Detected On Build System, Install At Least One From Package Manager!");
					return false;
				}
				std::string solvedGodotKitPath = "./" + AS_STD_STRING(selectedGodotKitPath.replace("res://", ""));

				// Adjust Compiler Settings
				if (!extraIncludeDirectories.empty() && extraIncludeDirectories.back() != ';') extraIncludeDirectories.push_back(';');
				if (!extralibraryDirectories.empty() && extralibraryDirectories.back() != ';') extralibraryDirectories.push_back(';');
				if (!extraLibraries.empty() && extraLibraries.back() != ';') extraLibraries.push_back(';');

				// Generate Output Path (Must Added From VS Generator Settings [Absoulte/Releative])
				std::string outputPath = std::filesystem::relative(AS_STD_STRING(jenova::GetJenovaCacheDirectory())).string() + "\\";
				std::string intermediatePath = outputPath + "VisualStudio\\";
				std::string builtinPath = outputPath + "\\BuiltInScripts\\";

				// Create Builtin Cache Directory If Required
				if (scriptCollection.builtinCount != 0)
				{
					if (!std::filesystem::exists(builtinPath))
					{
						if (!std::filesystem::create_directories(builtinPath)) return false;
					}
				}

				// Generate Jenova Configuration Data As Compressed Base64
				jenova::EncodedData jenovaConfigurationBase64 = "";
				try
				{
					nlohmann::json jenovaConfiguration = nlohmann::json::parse(jenova::GlobalStorage::CurrentJenovaGeneratedConfiguration);
					jenovaConfiguration["OutputPath"] = outputPath;
					jenovaConfiguration["IntermediatePath"] = intermediatePath;
					jenovaConfiguration["BuiltinPath"] = builtinPath;
					jenovaConfigurationBase64 = jenova::CreateCompressedBase64FromStdString(jenovaConfiguration.dump(2));
				}
				catch (const std::exception&)
				{
					return false;
				}

				// Create Project File
				jenova::Output("Generating Visual C++ Project...");
				std::string projectTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_PROJECT_TEMPLATE));
				std::string projectCompileItems;
				jenova::TokenList projectCompileTokens, projectBuiltinCompileToken;
				for (size_t i = 0; i < scriptCollection.entityCount; i++)
				{
					// Add Bultin Compile Tokens
					if (scriptCollection.scriptModules[i].scriptType == jenova::ScriptModuleType::BuiltinEntityScript)
					{
						// Create Temporary Files for Built-In (Embedded) Scripts
						std::string embeddedScriptFile = builtinPath + "Builtin_" + scriptCollection.scriptIdentities[i] + ".cpp";
						if (!jenova::WriteStdStringToFile(embeddedScriptFile, AS_STD_STRING(scriptCollection.scriptModules[i].scriptSource))) return false;

						// Create Builtin Compile Item
						std::string compileItemTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_COMPILE_ITEM_TEMPLATE));
						jenova::ReplaceAllMatchesWithString(compileItemTemplate, "@@SCRIPT_PATH@@", "$(BuiltInDir)Builtin_" + scriptCollection.scriptIdentities[i] + ".cpp");
						jenova::ReplaceAllMatchesWithString(compileItemTemplate, "@@SCRIPT_IDENTITY@@", scriptCollection.scriptIdentities[i]);
						projectBuiltinCompileToken.push_back(compileItemTemplate);

						// Handled
						continue;
					}
					
					// Add External Compile Tokens
					std::string compileItemTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_COMPILE_ITEM_TEMPLATE));
					jenova::ReplaceAllMatchesWithString(compileItemTemplate, "@@SCRIPT_PATH@@", scriptCollection.scriptFilesReleative[i]);
					jenova::ReplaceAllMatchesWithString(compileItemTemplate, "@@SCRIPT_IDENTITY@@", scriptCollection.scriptIdentities[i]);
					projectCompileTokens.push_back(compileItemTemplate);
				}
				projectCompileTokens.insert(projectCompileTokens.end(), projectBuiltinCompileToken.begin(), projectBuiltinCompileToken.end());
				for (auto it = projectCompileTokens.begin(); it != projectCompileTokens.end(); ++it)
				{
					projectCompileItems += *it;
					if (std::next(it) != projectCompileTokens.end()) projectCompileItems += "\n";
				}
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@COMPILE_ITEMS@@", projectCompileItems);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@JENOVA_CONFIGURATION@@", jenovaConfigurationBase64);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@JENOVA_VERSION@@", std::string(APP_VERSION));
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@PLATFORM_TOOLSET@@", Format("%s", AS_C_STRING(vsInstance.platformToolset)));
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@OUT_DIRECTORY@@", outputPath);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@INT_DIRECTORY@@", intermediatePath);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@BUILTIN_DIRECTORY@@", builtinPath);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@MODULE_EXTENSION@@", generateDebugInformation ? ".dll" : ".jnv");
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@LANGUAGE_STANDARD@@", "std" + languageStandard);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@OPEN_MP_SUPPORT@@", openMPSupport ? "true" : "false");
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@PREPROCESSOR_DEFINITIONS@@", cpp_definitions);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@ADDITIONALINCLUDEDIRECTORIES@@", "./;./Jenova/JenovaSDK;" + solvedGodotKitPath + ";" + extraIncludeDirectories);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@ADDITIONALLIBRARYDIRECTORIES@@", "./;./Jenova/JenovaSDK;" + solvedGodotKitPath + ";" + extralibraryDirectories);
				jenova::ReplaceAllMatchesWithString(projectTemplate, "@@ADDITIONALDEPENDENCIES@@", "libGodot.x64.lib;Jenova.SDK.x64.lib;" + extraLibraries + "%(AdditionalDependencies)");
				if (!jenova::WriteStdStringToFile(projectFile, projectTemplate))
				{
					DisposeCompiler();
					return false;
				}

				// Dispose Compiler
				DisposeCompiler();

				// Create Project Filters File
				std::string projectFiltersTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_PROJECT_FILTERS_TEMPLATE));
				std::string projectFilters, projectScriptItems;
				jenova::TokenList projectScriptTokens, projectBuiltinScriptToken;
				for (size_t i = 0; i < scriptCollection.directoryCount; i++)
				{
					// Create Filters
					std::string filterItemTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_FILTER_ITEM_TEMPLATE));
					jenova::ReplaceAllMatchesWithString(filterItemTemplate, "@@SCRIPT_DIRECTORY@@", scriptCollection.scriptDirectoriesReleative[i]);
					jenova::ReplaceAllMatchesWithString(filterItemTemplate, "@@SCRIPT_DIRECTORY_IDENTITY@@", jenova::GenerateFilterUniqueIdentifier(scriptCollection.scriptDirectoriesReleative[i]));
					projectFilters += filterItemTemplate;
					if (i != scriptCollection.directoryCount - 1) projectFilters += "\n";
				}
				for (size_t i = 0; i < scriptCollection.entityCount; i++)
				{
					// Add Built-in Scripts Filter Items
					if (scriptCollection.scriptModules[i].scriptType == jenova::ScriptModuleType::BuiltinEntityScript)
					{
						std::string scriptItemTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_SCRIPT_ITEM_TEMPLATE));
						jenova::ReplaceAllMatchesWithString(scriptItemTemplate, "@@SCRIPT_PATH@@", "$(BuiltInDir)Builtin_" + scriptCollection.scriptIdentities[i] + ".cpp");
						jenova::ReplaceAllMatchesWithString(scriptItemTemplate, "@@SCRIPT_FILTER@@", "Embedded");
						projectBuiltinScriptToken.push_back(scriptItemTemplate);
						continue;
					}

					// Skip Root Scripts
					if (scriptCollection.entityDirectoryIndex[i] == -1) continue;

					// Create Script Filter Items
					std::string scriptItemTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_SCRIPT_ITEM_TEMPLATE));
					jenova::ReplaceAllMatchesWithString(scriptItemTemplate, "@@SCRIPT_PATH@@", scriptCollection.scriptFilesReleative[i]);
					jenova::ReplaceAllMatchesWithString(scriptItemTemplate, "@@SCRIPT_FILTER@@", scriptCollection.scriptDirectoriesReleative[scriptCollection.entityDirectoryIndex[i]]);
					projectScriptTokens.push_back(scriptItemTemplate);
				}
				projectScriptTokens.insert(projectScriptTokens.end(), projectBuiltinScriptToken.begin(), projectBuiltinScriptToken.end());
				for (auto it = projectScriptTokens.begin(); it != projectScriptTokens.end(); ++it)
				{
					projectScriptItems += *it;
					if (std::next(it) != projectScriptTokens.end()) projectScriptItems += "\n";
				}
				jenova::ReplaceAllMatchesWithString(projectFiltersTemplate, "@FILTERS@", projectFilters);
				jenova::ReplaceAllMatchesWithString(projectFiltersTemplate, "@@SCRIPT_ITEMS@@", projectScriptItems);
				if (!jenova::WriteStdStringToFile(projectFiltersFile, projectFiltersTemplate)) return false;

				// Generate Project User File
				std::string projectUserTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_PROJECT_USER_TEMPLATE));
				if (!jenova::WriteStdStringToFile(projectUserFile, projectUserTemplate)) return false;

				// Generate Source Control Git Ignore
				std::string gitIgnoreTemplate = std::string(BUFFER_PTR_SIZE_PARAM(jenova::visualstudio::VS_SCRIPT_GIT_IGNORE));
				if (!jenova::WriteStdStringToFile(gitIgnoreFile, gitIgnoreTemplate)) return false;

				// Check If User Wants to Launch VS
				if (jenova::GlobalSettings::AskAboutOpeningVisualStudio)
				{
					// Prompt User for Opening Visual Studio
					ConfirmationDialog* dialog = memnew(ConfirmationDialog);
					dialog->set_title("[ Visual Studio Integration ]");
					dialog->set_text("Visual Studio Solution Successfully Generated. Would you like to open it now?");
					dialog->get_ok_button()->set_text("Open Visual Studio");
					dialog->get_cancel_button()->set_text("No Thanks!");

					// Define Internal UI Callback
					class OnConfirmedEvent : public RefCounted
					{
					private:
						JenovaEditorPlugin* pluginInstance;

					public:
						OnConfirmedEvent(JenovaEditorPlugin* _plugin) { pluginInstance = _plugin; }
						void ProcessEvent()
						{
							EditorInterface::get_singleton()->stop_playing_scene();
							pluginInstance->OpenProjectInVisualStudio();
							memdelete(this);
						}
					};

					// Create & Assign UI Callback to Dialog
					dialog->connect("confirmed", callable_mp(memnew(OnConfirmedEvent(this)), &OnConfirmedEvent::ProcessEvent));
					dialog->connect("confirmed", callable_mp((Node*)dialog, &ConfirmationDialog::queue_free));
					dialog->connect("canceled", callable_mp((Node*)dialog, &ConfirmationDialog::queue_free));

					// Add Dialog to Engine & Show
					add_child(dialog);
					dialog->popup_centered();
				}

				// Verbose
				jenova::OutputColored("#8d42f5", "Jenova Project Has Been Successfully Exported to Visual Studio %s Solution.", AS_C_STRING(vsInstance.productYear));

				// All Good
				return true;
			}
			bool OpenProjectInVisualStudio()
			{
				// Get Project Path
				std::string projectPath = AS_STD_STRING(ProjectSettings::get_singleton()->globalize_path("res://"));
				std::string solutionFile = projectPath + jenova::GlobalSettings::VisualStudioSolutionFile;
				if (!std::filesystem::exists(solutionFile)) return false;
				ShellExecuteA(0, 0, solutionFile.c_str(), 0, 0, SW_SHOW);
				return true;
			}
			jenova::SerializedData GetVistualStudioMetadata()
			{
				// Get Metadata from Visual Studio Locator
				std::string jsonData = jenova::libraries::GetVisualStudioInstancesMetadata("-format json");
				if (jsonData.empty()) return jenova::SerializedData();

				// Parse Metadata
				try
				{
					// Create Serializer/Deserializer
					nlohmann::json vsMetadata;
					nlohmann::json vsInstancesMetadata = nlohmann::json::parse(jsonData);
					vsMetadata["InstancesNumber"] = vsInstancesMetadata.size();
					vsMetadata["Instances"] = nlohmann::json::object();

					// Collect Instances
					for (const auto& vsInstance : vsInstancesMetadata)
					{
						// Get Instance Information
						std::string instanceID = vsInstance["catalog"]["id"].get<std::string>();
						
						// Add New Instance From Extracted Information
						vsMetadata["Instances"][instanceID] = nlohmann::json::object();
						vsMetadata["Instances"][instanceID]["Name"] = vsInstance["displayName"].get<std::string>();
						vsMetadata["Instances"][instanceID]["Version"] = vsInstance["catalog"]["buildVersion"].get<std::string>();
						vsMetadata["Instances"][instanceID]["Product"] = vsInstance["catalog"]["productName"].get<std::string>();
						vsMetadata["Instances"][instanceID]["Year"] = vsInstance["catalog"]["productLineVersion"].get<std::string>();
					}

					// Serialize Metadata and Return
					return vsMetadata.dump(3);
				}
				catch (const std::exception& error)
				{
					jenova::Error("Jenova Vistual Studio Locator", "Failed to Generate Visual Studio Metadata.");
				}

				// Failed
				return jenova::SerializedData();
			}
			VisualStudioInstance GetVisualStudioInstance(int instanceIndex)
			{
				return vsInstances[instanceIndex];
			}

			// Build Configuration
			void OpenBuildConfigurationWindow()
			{
				// Get Current Compiler Model
				Variant compilerModelSetting; 
				if (!GetEditorSetting(CompilerModelConfigPath, compilerModelSetting))
				{
					jenova::Error("Jenova Build Configurer", "Failed to Get Compiler Model.");
					return;
				}
				auto compilerModel = jenova::CompilerModel(int32_t(compilerModelSetting));

				// Collect Compiler & GodotKit Packages
				auto compilerPackages = jenova::GetInstalledCompilerPackages(compilerModel);
				auto godotKitPackages = jenova::GetInstalledGodotKitPackages();

				// Validate Compiler & GodotKit Packages
				if (compilerPackages.size() == 0)
				{
					jenova::Warning("Jenova Build Configurer", "No Compiler for Selected Compiler Model Detected On Build System, Install At Least One From Package Manager!");
				}
				if (godotKitPackages.size() == 0)
				{
					jenova::Error("Jenova Build Configurer", "No GodotSDK Detected On Build System, Install At Least One From Package Manager!");
					return;
				}

				// Get Scale Factor
				double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

				// Create Window
				Window* configure_build_window = memnew(Window);
				configure_build_window->set_title("Build Configuration");
				configure_build_window->set_size(Vector2i(SCALED(400), SCALED(270)));
				configure_build_window->set_flag(Window::Flags::FLAG_RESIZE_DISABLED, true);
				configure_build_window->set_flag(Window::Flags::FLAG_POPUP, true);

				// Show Window [Must Be Here]
				configure_build_window->popup_exclusive_centered(EditorInterface::get_singleton()->get_base_control());

				// Get Editor Theme
				Ref<Theme> editor_theme = EditorInterface::get_singleton()->get_editor_theme();
				if (!editor_theme.is_valid())
				{
					jenova::Error("Jenova Build Configurer", "Failed to Obtain Engine Theme.");
					return;
				}

				// Create Window Controls
				Control* root_control = memnew(Control);
				root_control->set_name("ConfigureBuildWindow");
				root_control->set_anchors_preset(Control::PRESET_FULL_RECT);
				configure_build_window->add_child(root_control);

				ColorRect* background = memnew(ColorRect);
				background->set_name("Background");
				background->set_anchors_preset(Control::PRESET_FULL_RECT);
				background->set_color(editor_theme->get_color("base_color", "Editor"));
				root_control->add_child(background);

				Control* window_surface = memnew(Control);
				window_surface->set_name("WindowSurface");
				window_surface->set_anchors_and_offsets_preset(Control::PRESET_TOP_LEFT);
				window_surface->set_anchor_and_offset(Side::SIDE_LEFT, 0.0, SCALED(30.0));
				window_surface->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(15.0));
				window_surface->set_anchor_and_offset(Side::SIDE_RIGHT, 0.0, SCALED(370.0));
				window_surface->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(250.0));
				root_control->add_child(window_surface);

				Label* compiler_package_label = memnew(Label);
				compiler_package_label->set_name("CompilersLabel");
				compiler_package_label->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				compiler_package_label->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(22.0));
				compiler_package_label->add_theme_font_size_override("font_size", SCALED(15));
				compiler_package_label->set_text("Available Compilers");
				window_surface->add_child(compiler_package_label);

				OptionButton* compiler_selector = memnew(OptionButton);
				compiler_selector->set_name("CompilerSelector");
				compiler_selector->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				compiler_selector->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(29.0));
				compiler_selector->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(74.0));
				compiler_selector->add_theme_color_override("font_color", editor_theme->get_color("accent_color", "Editor"));
				for (const auto& compilerPackage : compilerPackages) compiler_selector->add_item(" " + compilerPackage.pkgDestination.get_file());
				if(compilerPackages.size() != 0) compiler_selector->add_item(" Latest");
				if (compilerPackages.size() == 0) compiler_selector->add_item(" No Compiler Detected");
				if (compilerPackages.size() == 0) compiler_selector->set_disabled(true);
				compiler_selector->select(compiler_selector->get_item_count() - 1);
				window_surface->add_child(compiler_selector);
	

				Label* godotkit_package_label = memnew(Label);
				godotkit_package_label->set_name("GodotKitLabel");
				godotkit_package_label->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				godotkit_package_label->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(90.0));
				godotkit_package_label->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(112.0));
				godotkit_package_label->add_theme_font_size_override("font_size", SCALED(15));
				godotkit_package_label->set_text("Available GodotKits");
				window_surface->add_child(godotkit_package_label);

				OptionButton* godotkit_selector = memnew(OptionButton);
				godotkit_selector->set_name("GodotKitSelector");
				godotkit_selector->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
				godotkit_selector->set_anchor_and_offset(Side::SIDE_TOP, 0.0, SCALED(119.0));
				godotkit_selector->set_anchor_and_offset(Side::SIDE_BOTTOM, 0.0, SCALED(164.0));
				godotkit_selector->add_theme_color_override("font_color", editor_theme->get_color("accent_color", "Editor"));
				for (const auto& godotKitPackage : godotKitPackages) godotkit_selector->add_item(" " + godotKitPackage.pkgDestination.get_file());
				godotkit_selector->add_item(" Latest");
				godotkit_selector->select(godotkit_selector->get_item_count() - 1);
				window_surface->add_child(godotkit_selector);

				Button* configure_button = memnew(Button);
				configure_button->set_name("ConfigureButton");
				configure_button->set_anchors_and_offsets_preset(Control::PRESET_CENTER_BOTTOM);
				configure_button->set_anchor_and_offset(Side::SIDE_LEFT, 0.5, SCALED(-85.0));
				configure_button->set_anchor_and_offset(Side::SIDE_TOP, 1.0, SCALED(-50.0));
				configure_button->set_anchor_and_offset(Side::SIDE_RIGHT, 0.5, SCALED(85.0));
				configure_button->set_anchor_and_offset(Side::SIDE_BOTTOM, 1.0, SCALED(1.0));
				configure_button->set_text("   Configure Build   ");
				window_surface->add_child(configure_button);

				// Define Internal UI Callback Handler
				class ConfigureBuildEventManager : public RefCounted
				{
				private:
					JenovaEditorPlugin* pluginInstance = nullptr;
					Window* window = nullptr;

				public:
					ConfigureBuildEventManager(JenovaEditorPlugin* _plugin, Window* _window) : pluginInstance(_plugin), window(_window) { }
					void OnConfigureButtonClick()
					{
						OptionButton* compilerSelector = window->get_node<OptionButton>("ConfigureBuildWindow/WindowSurface/CompilerSelector");
						OptionButton* godotKitSelector = window->get_node<OptionButton>("ConfigureBuildWindow/WindowSurface/GodotKitSelector");
						if (compilerSelector && godotKitSelector)
						{
							if (!compilerSelector->is_disabled())
							{
								String selectedCompiler = compilerSelector->get_item_text(compilerSelector->get_selected_id()).replace(" ", "");
								EditorInterface::get_singleton()->get_editor_settings()->set_setting(pluginInstance->GetEditorSettingStringPath("compiler_package"), selectedCompiler);
							}
							if (!godotKitSelector->is_disabled())
							{
								String selectedGodotKit = godotKitSelector->get_item_text(godotKitSelector->get_selected_id()).replace(" ", "");
								EditorInterface::get_singleton()->get_editor_settings()->set_setting(pluginInstance->GetEditorSettingStringPath("godot_kit_package"), selectedGodotKit);
							}
							jenova::OutputColored("#2ebc78", "Project Build Configuration Updated Successfully!");
						}
						window->queue_free();
						memdelete(this);
					}
					void OnWindowClose()
					{
						window->queue_free();
						memdelete(this);
					}
				};

				// Create & Assign Callbacks
				configure_button->connect("pressed", callable_mp(memnew(ConfigureBuildEventManager(this, configure_build_window)), &ConfigureBuildEventManager::OnConfigureButtonClick));
				configure_build_window->connect("close_requested", callable_mp(memnew(ConfigureBuildEventManager(this, configure_build_window)), &ConfigureBuildEventManager::OnWindowClose));

				// Prepare Pop Up Window
				if (!jenova::AssignPopUpWindow(configure_build_window))
				{
					configure_build_window->queue_free();
				}
			}

			// Misc Windows
			void OpenAboutJenovaProject()
			{
				// Get Scale Factor
				double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

				// Create Window
				Window* jenva_about_window = memnew(Window);
				jenva_about_window->set_title("About Projekt Jenova");
				jenva_about_window->set_size(Vector2i(SCALED(750), SCALED(600)));
				jenva_about_window->set_flag(Window::Flags::FLAG_RESIZE_DISABLED, true);
				jenva_about_window->set_flag(Window::Flags::FLAG_POPUP, true);

				// Show Window [Must Be Here]
				jenva_about_window->popup_exclusive_centered(EditorInterface::get_singleton()->get_base_control());

				// Create UI Stage
				Control* jenova_about_ui = memnew(Control);
				jenova_about_ui->set_name("JenovaAboutUI");
				jenova_about_ui->set_anchors_preset(Control::PRESET_FULL_RECT);
				jenova_about_ui->set_h_size_flags(Control::SIZE_EXPAND_FILL);
				jenova_about_ui->set_v_size_flags(Control::SIZE_EXPAND_FILL);
				jenva_about_window->add_child(jenova_about_ui);

				// Add Background ColorRect
				ColorRect* background = memnew(ColorRect);
				background->set_name("Background");
				background->set_anchors_preset(Control::PRESET_FULL_RECT);
				background->set_h_size_flags(Control::SIZE_EXPAND_FILL);
				background->set_v_size_flags(Control::SIZE_EXPAND_FILL);
				background->set_color(Color(0.0666667, 0.0666667, 0.105882, 1));
				jenova_about_ui->add_child(background);

				// Add AboutImage TextureRect (using a placeholder for now)
				TextureRect* about_image = memnew(TextureRect);
				about_image->set_name("AboutImage");
				about_image->set_anchors_preset(Control::PRESET_LEFT_WIDE);
				about_image->set_offset(Side::SIDE_LEFT, SCALED(30.0));
				about_image->set_offset(Side::SIDE_TOP, SCALED(30.0));
				about_image->set_offset(Side::SIDE_RIGHT, SCALED(300.0));
				about_image->set_offset(Side::SIDE_BOTTOM, SCALED(-30.0));
				about_image->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
				about_image->set_expand_mode(TextureRect::ExpandMode::EXPAND_FIT_HEIGHT); 
				about_image->set_texture(jenova::CreateImageTextureFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(PNG_ABOUT_IMAGE))));
				jenova_about_ui->add_child(about_image);

				// Add Title Label
				Label* title = memnew(Label);
				title->set_name("Title");
				title->set_offset(Side::SIDE_LEFT, SCALED(323.0));
				title->set_offset(Side::SIDE_TOP, SCALED(20.0));
				title->set_offset(Side::SIDE_RIGHT, SCALED(665.0));
				title->set_offset(Side::SIDE_BOTTOM, SCALED(77.0));
				title->add_theme_color_override("font_color", Color(0.0820475, 0.763784, 0.615328, 1));
				title->add_theme_font_size_override("font_size", SCALED(35));
				title->set_text("Projekt J.E.N.O.V.A");
				jenova_about_ui->add_child(title);

				// Add Description Label
				Label* description = memnew(Label);
				description->set_name("Description");
				description->set_offset(Side::SIDE_LEFT, SCALED(327.0));
				description->set_offset(Side::SIDE_TOP, SCALED(130.0));
				description->set_offset(Side::SIDE_RIGHT, SCALED(733.0));
				description->set_offset(Side::SIDE_BOTTOM, SCALED(450.0));
				description->add_theme_color_override("font_color", Color(1, 1, 1, 0.533333));
				description->set_text(
					"Projekt J.E.N.O.V.A is an extension library for the Godot 4 Game Engine "
					"that brings fully-featured C++ scripting directly into the Godot Editor. "
					"It allows the use of modern C++20 standards within the Godot Engine, similar to GDScript.\n\n"
					"With Projekt J.E.N.O.V.A, there are no limits.\n"
					"You can create anything! From Desktop Software to AAA Quality Games. "
					"It's a full toolset with all the features C++ compilers provide.\n\n"
					"For More Information Visit Official Website:\n"
				);
				description->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
				description->add_theme_font_size_override("font_size", SCALED(16));
				jenova_about_ui->add_child(description);

				// Add Version Label
				Label* version = memnew(Label);
				version->set_name("Version");
				version->set_offset(Side::SIDE_LEFT, SCALED(327.0));
				version->set_offset(Side::SIDE_TOP, SCALED(73.0));
				version->set_offset(Side::SIDE_RIGHT, SCALED(724.0));
				version->set_offset(Side::SIDE_BOTTOM, SCALED(106.0));
				version->set_text("Version " + String(APP_VERSION) + " (" + String(APP_VERSION_POSTFIX) + ") Build " + String(APP_VERSION_BUILD) + "\n" + __TIMESTAMP__);
				version->set_autowrap_mode(TextServer::AUTOWRAP_WORD); 
				version->add_theme_font_size_override("font_size", SCALED(14));
				jenova_about_ui->add_child(version);

				// Add Author Label
				Label* author = memnew(Label);
				author->set_name("Author");
				author->set_anchors_preset(Control::PRESET_CENTER_BOTTOM);
				author->set_anchor(Side::SIDE_LEFT, 0.5);
				author->set_anchor(Side::SIDE_TOP, 1.0);
				author->set_anchor(Side::SIDE_RIGHT, 0.5);
				author->set_anchor(Side::SIDE_BOTTOM, 1.0);
				author->set_offset(Side::SIDE_LEFT, SCALED(-48.0));
				author->set_offset(Side::SIDE_TOP, SCALED(-75.0));
				author->set_offset(Side::SIDE_RIGHT, SCALED(349.0));
				author->set_offset(Side::SIDE_BOTTOM, SCALED(-42.0));
				author->set_h_size_flags(Control::SIZE_EXPAND_FILL);
				author->set_v_size_flags(Control::SIZE_SHRINK_BEGIN);
				author->add_theme_color_override("font_color", Color(0.00392157, 0.768627, 0.627451, 1));
				author->set_text("Developed & Designed By Hamid.Memar");
				author->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
				jenova_about_ui->add_child(author);

				// Add Company Label
				Label* company = memnew(Label);
				company->set_name("Company");
				company->set_anchors_preset(Control::PRESET_CENTER_BOTTOM);
				company->set_anchor(Side::SIDE_LEFT, 0.5);
				company->set_anchor(Side::SIDE_TOP, 1.0);
				company->set_anchor(Side::SIDE_RIGHT, 0.5);
				company->set_anchor(Side::SIDE_BOTTOM, 1.0);
				company->set_offset(Side::SIDE_LEFT, SCALED(-48.0));
				company->set_offset(Side::SIDE_TOP, SCALED(-50.0));
				company->set_offset(Side::SIDE_RIGHT, SCALED(349.0));
				company->set_offset(Side::SIDE_BOTTOM, SCALED(-17.0));
				company->set_h_size_flags(Control::SIZE_EXPAND_FILL);
				company->set_v_size_flags(Control::SIZE_SHRINK_BEGIN);
				company->set_text("MemarDesign LLC.");
				company->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
				jenova_about_ui->add_child(company);

				// Add Button
				Button* open_web_button = memnew(Button);
				open_web_button->set_name("OpenWebButton");
				open_web_button->set_anchors_preset(Control::PRESET_CENTER);
				open_web_button->set_anchor(Side::SIDE_LEFT, 0.5);
				open_web_button->set_anchor(Side::SIDE_TOP, 0.5);
				open_web_button->set_anchor(Side::SIDE_RIGHT, 0.5);
				open_web_button->set_anchor(Side::SIDE_BOTTOM, 0.5);
				open_web_button->set_offset(Side::SIDE_LEFT, SCALED(-49.0));
				open_web_button->set_offset(Side::SIDE_TOP, SCALED(146.0));
				open_web_button->set_offset(Side::SIDE_RIGHT, SCALED(351.0));
				open_web_button->set_offset(Side::SIDE_BOTTOM, SCALED(201.0));
				open_web_button->set_h_size_flags(Control::SIZE_EXPAND_FILL);
				open_web_button->set_v_size_flags(Control::SIZE_EXPAND_FILL);
				open_web_button->add_theme_color_override("font_color", Color(0.427828, 0.675155, 0.933394, 1));
				open_web_button->set_text("Open Projekt Jenova Website");
				jenova_about_ui->add_child(open_web_button);

				// Define Internal UI Callback Handler
				class AboutEventManager : public RefCounted
				{
				private:
					Window* window = nullptr;

				public:
					AboutEventManager(Window* _window) : window(_window) { }
					void OnWebButtonClick()
					{
						ShellExecute(0, 0, L"https://jenova-framework.github.io", 0, 0, SW_SHOW);
						window->queue_free();
						memdelete(this);
					}
					void OnWindowClose()
					{
						window->queue_free();
						memdelete(this);
					}
				};

				// Create Event Manager
				AboutEventManager* aboutEventMan = memnew(AboutEventManager(jenva_about_window));

				// Create & Assign Callbacks
				open_web_button->connect("pressed", callable_mp(aboutEventMan, &AboutEventManager::OnWebButtonClick));
				jenva_about_window->connect("close_requested", callable_mp(aboutEventMan, &AboutEventManager::OnWindowClose));

				// Prepare Pop Up Window
				if (!jenova::AssignPopUpWindow(jenva_about_window))
				{
					jenva_about_window->queue_free();
				}
			}

		private:

			// Asset Monitor Events
			static void OnAssetChanged(const String& targetPath, const jenova::AssetMonitor::CallbackEvent& callbackEvent)
			{
				// Convert to Absolute Path
				std::string targetFullPath = std::filesystem::absolute(AS_STD_STRING(targetPath)).string();

				// Predefined Files
				static std::string visualStudioWatchdogFile = jenova::GlobalStorage::CurrentJenovaCacheDirectory + jenova::GlobalSettings::VisualStudioWatchdogFile;

				// Handle Added Files
				if (callbackEvent == jenova::AssetMonitor::CallbackEvent::Added)
				{
					// Check for Watchdog Invokers
					if (jenova::GlobalStorage::CurrentChangesTriggerMode == jenova::ChangesTriggerMode::TriggerOnWatchdogInvoke)
					{
						// Check for Visual Studio Watchdog
						if (jenova::CompareFilePaths(targetFullPath, visualStudioWatchdogFile))
						{
							// Wait For File to Be Written
							std::this_thread::sleep_for(std::chrono::milliseconds(60));

							// Launch Bootstrapper
							if (jenovaEditorPlugin) jenovaEditorPlugin->call_deferred("BootstrapModule", jenova::ReadStringFromFile(targetPath));
							std::filesystem::remove(targetFullPath);
						}
					}
				}
			}
		};
		class JenovaExportPlugin : public EditorExportPlugin 
		{
			GDCLASS(JenovaExportPlugin, EditorExportPlugin);

		private:
			String JenovaExportPluginName = "JenovaGodotExportPlugin";

		private:
			bool ExcludeSourcesFromBuild = true;
			static void _bind_methods() {}

		public:
			String _get_name() const override { return JenovaExportPluginName; }
			bool _supports_platform(const Ref<EditorExportPlatform>& p_platform) const override
			{
				if (p_platform->get_os_name() == "Windows") return true;
				return false;
			}

			// Events
			void _export_begin(const PackedStringArray& p_features, bool p_is_debug, const String& p_path, uint32_t p_flags) override
			{
				// Build Verbose
				jenova::Output("[color=#729bed][Build][/color] Building Jenova Runtime...");

				// Validate Editor Plugin Instance
				if (jenovaEditorPlugin == nullptr)
				{
					jenova::Error("Jenova Exporter", "Export Plugin Encountered a Fatal Error, Editor Plugin is Null.");
					return;
				}

				// Check for Source Remove from Build Option
				Variant RemoveSourcesFromBuild;
				if (!jenovaEditorPlugin->GetEditorSetting(jenovaEditorPlugin->GetEditorSettingStringPath("remove_source_codes_from_build"), RemoveSourcesFromBuild)) RemoveSourcesFromBuild = true;
				ExcludeSourcesFromBuild = RemoveSourcesFromBuild;

				// Add Configuration File
				jenova::Output("[color=#729bed][Build][/color] Generating Jenova Runtime Data...");
				std::string runtimeData = "[JENOVA RUNTIME CONFIGURATION]";
				PackedByteArray packed_svg_data;
				packed_svg_data.resize(runtimeData.size());
				memcpy(packed_svg_data.ptrw(), runtimeData.data(), runtimeData.size());
				String runtimeConfigPath = String(jenova::GlobalSettings::DefaultJenovaBootPath) + String(jenova::GlobalSettings::DefaultModuleConfigFile);
				this->add_file(runtimeConfigPath, packed_svg_data, false);
				packed_svg_data.clear();

				// Add Module Cache File
				String runtimeCachePath = String(jenova::GlobalSettings::DefaultJenovaBootPath) + String(jenova::GlobalSettings::DefaultModuleDatabaseFile);
				String defaultModuleDatabasePath = jenova::GetJenovaCacheDirectory() + String(jenova::GlobalSettings::DefaultModuleDatabaseFile);
				if (FileAccess::file_exists(defaultModuleDatabasePath))
				{
					Ref<FileAccess> fileAccess = FileAccess::open(defaultModuleDatabasePath, FileAccess::READ);
					PackedByteArray moduleCacheData = fileAccess->get_buffer(fileAccess->get_length());
					this->add_file(runtimeCachePath, moduleCacheData, false);
					moduleCacheData.clear();
				}
				else
				{
					jenova::Warning("Jenova Deployer", "Module Cache Not Found. Your Build Will Not Work Properly, Build Jenova Project Before Exporting!");
				}
			}
			void _export_end() override
			{
				jenova::Output("[color=#729bed][Build][/color] Jenova Runtime Successfully Generated.");
			}
			void _export_file(const String& p_path, const String& p_type, const PackedStringArray& p_features) override 
			{
				// Exclude Source Files From Build If Requiresd
				if (ExcludeSourcesFromBuild)
				{
					// Exclude Script Files
					if (p_path.to_lower().ends_with(".cpp")) skip();

					// Exclude Header Files
					if (p_path.to_lower().ends_with(".h")) skip();
					if (p_path.to_lower().ends_with(".hh")) skip();
					if (p_path.to_lower().ends_with(".hpp")) skip();
				}
			}
		};
		class JenovaImportPlugin : public EditorImportPlugin
		{
			GDCLASS(JenovaImportPlugin, EditorImportPlugin);

		private:
			String JenovaImportPluginName = "JenovaGodotCPPImportPlugin";
			String JenovaImportVisibleName = "Jenova C++ Script";

		private:
			static void _bind_methods() {}

		public:
			String _get_importer_name() const override { return JenovaImportPluginName; }
			String _get_visible_name() const override { return JenovaImportVisibleName; }
			int32_t _get_preset_count() const override
			{
				return 1;
			}
			String _get_preset_name(int32_t preset_index = 0) const override
			{
				switch (preset_index)
				{
				case 0:
				default:
					return "Default";
				}
			}
			TypedArray<Dictionary> _get_import_options(const String& p_path, int32_t p_preset_index) const override 
			{ 
				Dictionary optCppLanguageStd;
				optCppLanguageStd["name"] = "c++_language_standard";
				optCppLanguageStd["usage"] = PROPERTY_USAGE_DEFAULT;
				optCppLanguageStd["default_value"] = 0;
				optCppLanguageStd["property_hint"] = PropertyHint::PROPERTY_HINT_ENUM;
				optCppLanguageStd["hint_string"] = "Default,ISO C++14 Standard,ISO C++17 Standard,ISO C++20 Standard";

				Dictionary optRuntimeMachine;
				optRuntimeMachine["name"] = "runtime_machine";
				optRuntimeMachine["usage"] = PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED;
				optRuntimeMachine["default_value"] = 0;
				optRuntimeMachine["property_hint"] = PropertyHint::PROPERTY_HINT_ENUM;
				optRuntimeMachine["hint_string"] = "Jenova VM,Native VM";

				TypedArray<Dictionary> options;
				options.push_back(optCppLanguageStd);
				options.push_back(optRuntimeMachine);
				return options;
			}
			PackedStringArray _get_recognized_extensions() const override
			{
				PackedStringArray array;
				array.push_back(jenova::GlobalSettings::JenovaScriptExtension);
				return array;
			}
			String _get_resource_type() const override
			{
				return jenova::GlobalSettings::JenovaScriptType;
			}
			String _get_save_extension() const override
			{
				return jenova::GlobalSettings::JenovaScriptExtension;
			}
			double _get_priority() const override
			{
				return 1;
			}
			int32_t _get_import_order() const override
			{
				return IMPORT_ORDER_DEFAULT;
			}
			bool _get_option_visibility(const String& path, const StringName& option_name, const Dictionary& options) const override
			{
				return true;
			}
			godot::Error _import(const String& source_file, const String& save_path, const Dictionary& options, const TypedArray<String>& platform_variants, const TypedArray<String>& gen_files) const
			{
				jenova::Output("[Importer] C++ Script File (%s) Imported.", AS_C_STRING(source_file));
				CPPScript* newCppScript = memnew(CPPScript);
				newCppScript->_set_source_code(FileAccess::get_file_as_string(source_file));
				return ResourceSaver::get_singleton()->save(newCppScript, "" + save_path + "." + _get_save_extension(), 0);
			}
			bool _can_import_threaded() const override
			{
				return false;
			}
		};
		class JenovaDebuggerPlugin : public EditorDebuggerPlugin
		{
			GDCLASS(JenovaDebuggerPlugin, EditorDebuggerPlugin);

		private:
			String JenovaDebuggerPluginName = "JenovaGodotDebuggerPlugin";

		private:
			int32_t currentSessionID = -1;
			static void _bind_methods() {}

		public:
			void _setup_session(int32_t p_session_id) override
			{
				currentSessionID = p_session_id;
			}
			bool _has_capture(const String& p_capture) const override
			{
				jenova::Output("_has_capture %s", AS_C_STRING(p_capture));
				return false;
			}
			bool _capture(const String& p_message, const Array& p_data, int32_t p_session_id) override
			{
				return false;
			}
		};

		// Runtime Plugins
		class JenovaRuntime : public Object 
		{
			GDCLASS(JenovaRuntime, Object)

		private:
			// Singleton Instance
			static inline JenovaRuntime* singleton = nullptr;

		protected:
			// Method Binding
			static void _bind_methods()
			{

			}

		public:
			// Initializer/Deinitializer
			static void init()
			{
				// Register Class
				ClassDB::register_internal_class<JenovaRuntime>();

				// Initialize Singleton
				singleton = memnew(JenovaRuntime);

				// Start Runtime
				if (!singleton->StartRuntime())
				{
					jenova::Error("Jenova Runtime", "Fatal Error :: Jenova Runtime Failed to Start!");
					quick_exit(jenova::ErrorCode::RUNTIME_INIT_FAILED);
				}

				// Verbose
				jenova::Output("Jenova Runtime (%s%s%s) Initialized.", APP_VERSION, APP_VERSION_MIDDLEFIX, APP_VERSION_POSTFIX);
			}
			static void deinit()
			{
				// Start Runtime
				if (!singleton->StopRuntime())
				{
					jenova::Error("Jenova Runtime", "Fatal Error :: Jenova Runtime Failed to Stop!");
					quick_exit(jenova::ErrorCode::RUNTIME_DEINIT_FAILED);
				}

				// Release Singleton
				if (singleton) memdelete(singleton);

				// Verbose
				jenova::Output("Jenova Runtime (%s%s%s) Uninitialized.", APP_VERSION, APP_VERSION_MIDDLEFIX, APP_VERSION_POSTFIX);
			}

			// Singleton Handling
			static JenovaRuntime* get_singleton()
			{
				return singleton;
			}

		public:
			// Methods
			bool StartRuntime()
			{
				// Register Debugger Message Receiver
				if (QUERY_ENGINE_MODE(Debug))
				{
					EngineDebugger::get_singleton()->register_message_capture("Jenova-Runtime", callable_mp(this, &JenovaRuntime::OnDebuggerMessageReceived));
					jenova::Output("Debugging Tools Detected, Runtime will be running in Debug Mode.");
				}

				// All Good
				return true;
			}
			bool StopRuntime()
			{
				// Unregister Debugger Message Receiver
				if (QUERY_ENGINE_MODE(Debug))
				{
					EngineDebugger::get_singleton()->unregister_message_capture("Jenova-Runtime");
				}

				// All Good
				return true;
			}
			void StartNetworkListener(const Dictionary& listenerSettings);
			void StartNetworkPeer(const Dictionary& peerSettings);

		private:
			// Events
			bool OnDebuggerMessageReceived(const String& msgCommand, const Array& args)
			{
				// Handle Remote Commands
				if (msgCommand == "Reload" && QUERY_ENGINE_MODE(Debug))
				{
					if (!JenovaInterpreter::DeployFromDatabase(jenova::GlobalSettings::DefaultModuleDatabaseFile))
					{
						jenova::Warning("Jenova Runtime", "Module Cache Cannot Be Deployed, Possible Corruption.");
						return false;
					}
				}

				// All Good
				return true;
			}
		};

		// Callbacks
		static void OnExtensionInitialize()
		{
			/* Note: This Function Must Update Data Only If They Are Not Already Set!*/

			// Set Engine Mode
			if (QUERY_ENGINE_MODE(Unknown))
			{
				if (OS::get_singleton()->is_debug_build()) jenova::GlobalStorage::CurrentEngineMode = jenova::EngineMode::Debug;
				else jenova::GlobalStorage::CurrentEngineMode = jenova::EngineMode::Runtime;
				if (Engine::get_singleton()->is_editor_hint()) jenova::GlobalStorage::CurrentEngineMode = jenova::EngineMode::Editor;

				// Verbose Mode
				jenova::Output("Running Jenova Core in [%s] Engine Mode.", AS_C_STRING(jenova::GetCurrentEngineInstanceModeAsString()));
			}
		}
		static void OnEnvironmentBoot()
		{
			// Handle Pre-Launch Errors
			if (jenova::GlobalSettings::HandlePreLaunchErrors)
			{
				// Ignore Editor
				if (QUERY_ENGINE_MODE(Editor)) return;

				// Get Environment Value & Process
				char buffer[256];
				DWORD result = GetEnvironmentVariableA("JENOVA_PRE_LAUNCH_ERROR", buffer, 256);
				if (result == 0) return;
				if (strcmp(buffer, "WRONG_CONFIGURATIONS") == 0) 
				{
					ERR_PRINT("[ Jenova Runtime ] ::> Corrupted Configuration Detected, Launch Aborted.");
					quick_exit(0);
				}
				else if (strcmp(buffer, "BUILD_FAILED") == 0) 
				{
					ERR_PRINT("[ Jenova Runtime ] ::> Project Build Failed, Check Console for Build Errors.");
					quick_exit(0);
				}
			}
		}
		static void OnRuntimeStarted()
		{
			// Load Module At Runtime Start 
			if (jenova::GlobalSettings::DefaultModuleLoadStage == ModuleLoadStage::LoadModuleAtRuntimeStart) JenovaInterpreter::BootInterpreter();
		}

		// Events
		static void InitializeModule(ModuleInitializationLevel p_level)
		{
			// Editor Initialize Event
			if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
			{
				// Initialize Extension
				OnExtensionInitialize();

				// Register Editor Sub Plugins (Seems like this is not required in gdmodule)
				GDREGISTER_INTERNAL_CLASS(JenovaExportPlugin);
				GDREGISTER_INTERNAL_CLASS(JenovaImportPlugin);
				GDREGISTER_INTERNAL_CLASS(JenovaDebuggerPlugin);

				// Register Editor Plugins
				GDREGISTER_INTERNAL_CLASS(JenovaEditorPlugin);
				EditorPlugins::add_by_type<JenovaEditorPlugin>();

				// Initialize Classes
				JenovaPackageManager::init();
				JenovaTemplateManager::init();
				JenovaAssetMonitor::init();

				// Load Module At Initialization [Editor]
				if (QUERY_ENGINE_MODE(Editor))
				{
					if (jenova::GlobalSettings::DefaultModuleLoadStage == ModuleLoadStage::LoadModuleAtInitialization) JenovaInterpreter::BootInterpreter();
				}

				// Verbose
				jenova::Output("Jenova Editor Plugin Initialized.");
			}

			// Scene Initialize Event
			if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
			{
				// Initialize Extension
				OnExtensionInitialize();

				// Rise Environment Boot Callback
				OnEnvironmentBoot();

				// Register Classes
				ClassDB::register_class<CPPScript>();
				ClassDB::register_class<CPPHeader>();
				ClassDB::register_class<CPPScriptLanguage>();
				ClassDB::register_class<CPPHeaderLanguage>();
				ClassDB::register_class<CPPScriptResourceLoader>();
				ClassDB::register_class<CPPScriptResourceSaver>();
				ClassDB::register_class<CPPHeaderResourceLoader>();
				ClassDB::register_class<CPPHeaderResourceSaver>();
				ClassDB::register_class<JenovaScriptManager>();

				// Initialize Classes
				CPPScriptLanguage::init();
				CPPHeaderLanguage::init();
				CPPScriptResourceLoader::init();
				CPPScriptResourceSaver::init();
				CPPHeaderResourceLoader::init();
				CPPHeaderResourceSaver::init();
				JenovaScriptManager::init();

				// Initialize Emulator System If Enabled
				if (jenova::GlobalSettings::EmulatorEnabled) JenovaFirewall::init();
				if (jenova::GlobalSettings::EmulatorEnabled) JenovaEmulator::init();

				// Load Module At Initialization [Debug/Runtime]
				if (QUERY_ENGINE_MODE(Debug) || QUERY_ENGINE_MODE(Runtime))
				{
					if (jenova::GlobalSettings::DefaultModuleLoadStage == ModuleLoadStage::LoadModuleAtInitialization) JenovaInterpreter::BootInterpreter();
				}

				// Register Callbacks
				JenovaScriptManager::get_singleton()->register_runtime_start_event(&OnRuntimeStarted);

				// Set the Custom Crash Handler
				SetUnhandledExceptionFilter(jenova::JenovaCrashHandler);

				// Uninitialize Runtime
				JenovaRuntime::init();
			}
		}
		static void UninitializeModule(ModuleInitializationLevel p_level)
		{
			// Editor UnInitialize Event
			if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
			{
				// Unregister Editor Plugins
				EditorPlugins::remove_by_type<JenovaEditorPlugin>();

				// UnInitialize Classes
				JenovaPackageManager::deinit();
				JenovaTemplateManager::deinit();
				JenovaAssetMonitor::deinit();

				// Verbose
				jenova::Output("Jenova Editor Plugin Uninitialized.");
			}

			// Scene UnInitialize Event
			if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
			{
				// Uninitialize Classes
				CPPScriptLanguage::deinit();
				CPPHeaderLanguage::deinit();
				CPPScriptResourceLoader::deinit();
				CPPScriptResourceSaver::deinit();
				CPPHeaderResourceLoader::deinit();
				CPPHeaderResourceSaver::deinit();

				// Uninitialize Emulator System If Enabled
				if (jenova::GlobalSettings::EmulatorEnabled) JenovaEmulator::deinit();
				if (jenova::GlobalSettings::EmulatorEnabled) JenovaFirewall::deinit();

				// Unload Module
				VALIDATE_FUNCTION(JenovaInterpreter::UnloadModule());

				// Release Interpreter
				VALIDATE_FUNCTION(JenovaInterpreter::ReleaseInterpreter());

				// Uninitialize Runtime
				JenovaRuntime::deinit();

				// Exit (Temp Fix for TLS Handling Failure)
				if (jenova::GlobalSettings::SafeExitOnPluginUnload && !QUERY_ENGINE_MODE(Editor)) quick_exit(EXIT_SUCCESS);
			}
		}

		// Wrapper
		static bool CheckWrapperInitialization()
		{
			std::string currentModuleName = std::filesystem::path(GlobalStorage::CurrentJenovaRuntimeModulePath).filename().string();
			if (currentModuleName == std::string(GlobalSettings::JenovaRuntimeModuleName)) return false;
			return true;
		}
		static GDExtensionBool InitializeAsWrapper(ExtensionInitializerData initializerData)
		{
			std::string wrapperDirectory = std::filesystem::path(GlobalStorage::CurrentJenovaRuntimeModulePath).parent_path().string();
			std::string originalRuntimeModulePath = wrapperDirectory + "\\" + std::string(GlobalSettings::JenovaRuntimeModuleName);
			if (!std::filesystem::exists(originalRuntimeModulePath)) return false;
			HMODULE jenovaRuntimeModule = LoadLibraryA(originalRuntimeModulePath.c_str());
			if (!jenovaRuntimeModule) return false;
			typedef GDExtensionBool(*InitializeJenovaRuntimeFunc)(GDExtensionInterfaceGetProcAddress, const GDExtensionClassLibraryPtr, GDExtensionInitialization*);
			InitializeJenovaRuntimeFunc InitializeJenovaRuntime = (InitializeJenovaRuntimeFunc)GetProcAddress(jenovaRuntimeModule, "InitializeRuntime");
			if (!InitializeJenovaRuntime) return false;
			return InitializeJenovaRuntime(initializerData.godotGetProcAddress, initializerData.godotExtensionClassLibraryPtr, initializerData.godotExtensionInitialization);
		}

		// Extension Initialization (Solve Godot SDK Functions)
		JENOVA_API GDExtensionBool InitializeRuntime(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization)
		{
			// Store Godot Internal Extension Initialzier Data
			jenova::GlobalStorage::ExtensionInitData.godotGetProcAddress = p_get_proc_address;
			jenova::GlobalStorage::ExtensionInitData.godotExtensionClassLibraryPtr = p_library;
			jenova::GlobalStorage::ExtensionInitData.godotExtensionInitialization = r_initialization;

			// Check for Wrapper Instance
			if (CheckWrapperInitialization())
			{
				// Instance is Wrapper, Fallback to Main
				return InitializeAsWrapper(jenova::GlobalStorage::ExtensionInitData);
			}

			// Initailize Object & Solve API Functions
			GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
			init_obj.register_initializer(InitializeModule);
			init_obj.register_terminator(UninitializeModule);
			init_obj.set_minimum_library_initialization_level(jenova::GlobalSettings::PluginInitializationLevel);
			return init_obj.init();
		}

		// Deployer (Called to Perform Tasks)
		JENOVA_API void CALLBACK Deploy(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
		{
			// Parse Arguments And Split to Argument Array
			jenova::ArgumentsArray deployerArguments = jenova::ProcessDeployerArguments(lpszCmdLine);
			deployerArguments.insert(deployerArguments.begin(), "jenova.exe");

			try
			{
				// Create Argument Parser
				argparse::ArgumentParser program("Jenova Deployer");

				// Add Arguments to Parser
				program.add_argument("--command").required().help("Deployment Command");
				program.add_argument("--in").help("Input File Path");
				program.add_argument("--out").help("Output File Path");
				program.add_argument("--cache").help("Jenova Cache Directory");
				program.add_argument("--compiler").help("Jenova Compiler Model");
				program.add_argument("--identity").help("Script Unique Identity");
				program.add_argument("--configuration").help("Jenova Build Configuration Data");
				program.add_argument("--watchdog-invoker").help("Build System Watchdog Invoker Name");

				// Parse Arguments
				program.parse_args(deployerArguments);

				// Get Parsed Values
				std::string command = program.get<std::string>("--command");

				// Process Commands
				if (command == "prepare")
				{
					// Get Input/Output Values & Files
					std::string compilerModel = program.get<std::string>("--compiler");

					// Initialize Build Tools
					if (compilerModel == "msvc")
					{
						jenova_log("[Jenova Deployer] Jenova %s (%s) Visual Studio Build Tools Initialized.", APP_VERSION, APP_VERSION_POSTFIX);
						jenova_log("[Jenova Deployer] Starting Build...");
						quick_exit(EXIT_SUCCESS);
					}
				}
				if (command == "preprocess")
				{
					// Get Input/Output Values & Files
					std::string inputFile = std::filesystem::absolute(program.get<std::string>("--in")).string();
					std::string cacheDirectory = std::filesystem::absolute(program.get<std::string>("--cache")).string();
					std::string compilerModel = program.get<std::string>("--compiler");
					std::string sourceIdentity = program.get<std::string>("--identity");
					std::string configuration = program.get<std::string>("--configuration");

					// Create Configuration
					nlohmann::json jenovaConfiguration;

					// Read And Parse Configuration
					try
					{
						jenovaConfiguration = nlohmann::json::parse(jenova::CreateStdStringFromCompressedBase64(configuration));
						if (jenovaConfiguration.empty()) throw std::runtime_error("Invalid Jenova Configuration Data.");
					}
					catch (const std::exception&)
					{
						jenova_log("[Jenova Deployer] Error : Failed to Read or Parse Jenova Configuration Data.");
						quick_exit(EXIT_FAILURE);
					}

					// Preprocess C++ Source
					{
						std::string scriptSourceCode = jenova::ReadStdStringFromFile(inputFile);
						if (scriptSourceCode.empty())
						{
							jenova_log("[Jenova Deployer] Error :", "Preprocessing Failed, Invalid Input Source.");
							quick_exit(EXIT_FAILURE);
						}

						// Line Directive
						std::string referenceSourceFile = inputFile;
						jenova::ReplaceAllMatchesWithString(referenceSourceFile, "\\", "\\\\");
						scriptSourceCode = scriptSourceCode.insert(0, jenova::Format("#line 1 \"%s\"\n", referenceSourceFile.c_str()));

						// Process And Extract Properties
						jenova::SerializedData propertiesMetadata = jenova::ProcessAndExtractPropertiesFromScript(scriptSourceCode, sourceIdentity);
						if (!propertiesMetadata.empty() && propertiesMetadata != "null")
						{
							std::string propFile = cacheDirectory + std::filesystem::path(inputFile).stem().string() + "_" + sourceIdentity + ".props";
							if (!jenova::WriteStdStringToFile(propFile, propertiesMetadata))
							{
								jenova_log("[Jenova Deployer] Error : Failed to Write Property Metadata On Disk.");
								quick_exit(EXIT_FAILURE);
							}
						}

						// Preprocessor Definitions [Header]
						std::string preprocessorDefinitions = "// Jenova Preprocessor Definitions\n";

						// Preprocessor Definitions [Version]
						preprocessorDefinitions += jenova::Format("#define JENOVA_VERSION \"%d.%d.%d.%d\"\n",
							jenova::GlobalSettings::JenovaBuildVersion[0], jenova::GlobalSettings::JenovaBuildVersion[1],
							jenova::GlobalSettings::JenovaBuildVersion[2], jenova::GlobalSettings::JenovaBuildVersion[3]);

						// Preprocessor Definitions [Compiler]
						preprocessorDefinitions += "#define JENOVA_COMPILER \"Microsoft Visual C++ Compiler\"\n";
						preprocessorDefinitions += "#define MSVC_COMPILER\n";

						// Preprocessor Definitions [USER]
						auto userPreprocessorDefinitions = jenova::SplitStdStringToArguments(jenovaConfiguration["PreprocessorDefinitions"].get<std::string>(), ';');
						for (const auto& definition : userPreprocessorDefinitions) if (!definition.empty()) preprocessorDefinitions += "#define " + definition + "\n";

						// Add Final Preprocessor Definitions
						scriptSourceCode = scriptSourceCode.insert(0, preprocessorDefinitions + "\n");

						// Replecements
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, jenova::GlobalSettings::ScriptToolIdentifier, "#define TOOL_SCRIPT");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, jenova::GlobalSettings::ScriptBlockBeginIdentifier, "namespace JNV_" + sourceIdentity + "{");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, jenova::GlobalSettings::ScriptBlockEndIdentifier, "}; using namespace JNV_" + sourceIdentity + ";");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnReady", " _ready");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnAwake", " _enter_tree");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnDestroy", " _exit_tree");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnProcess", " _process");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnPhysicsProcess", " _physics_process");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnInput", " _input");
						jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnUserInterfaceInput", " _gui_input");

						// Write Preprocessed Source
						std::string outputPath = cacheDirectory + std::filesystem::path(inputFile).stem().string() + "_" + sourceIdentity + ".cpp";
						if (!jenova::WriteStdStringToFile(outputPath, scriptSourceCode))
						{
							jenova_log("[Jenova Deployer] Error : Failed to Write Preprocessed Source On Disk.");
							quick_exit(EXIT_FAILURE);
						}
					}

					// All Good
					jenova_log("[Jenova Deployer] C++ Source '%s' Preprocessed.", std::filesystem::path(inputFile).filename().string().c_str());
					quick_exit(EXIT_SUCCESS);
				}
				if (command == "create-internal-scripts")
				{
					// Get Input/Output Values & Files
					std::string cacheDirectory = std::filesystem::absolute(program.get<std::string>("--cache")).string();

					// Create Internal Source
					if (!jenova::CreateFileFromInternalSource(cacheDirectory + "JenovaModuleLoader.cpp", std::string(JENOVA_RESOURCE(JenovaModuleInitializerCPP))))
					{
						jenova_log("[Jenova Deployer] Error : %s", "Unable to Create Internal Script 'JenovaModuleLoader'");
						quick_exit(EXIT_FAILURE);
					};

					// All Good
					jenova_log("[Jenova Deployer] Internal Scripts Successfully Generated.");
					quick_exit(EXIT_SUCCESS);
				}
				if (command == "generate-watchdog")
				{
					// Get Input/Output Values & Files
					std::string cacheDirectory = std::filesystem::absolute(program.get<std::string>("--cache")).string();
					std::string configuration = program.get<std::string>("--configuration");
					std::string watchdogInvoker = program.get<std::string>("--watchdog-invoker");

					// Create Known Watchdog
					if (watchdogInvoker == "vs")
					{
						// Generate Watchdog Path
						std::string visualStudioWatchdogFile = cacheDirectory + jenova::GlobalSettings::VisualStudioWatchdogFile;

						// If Watchdog Exists Remove It
						if (std::filesystem::exists(visualStudioWatchdogFile))
						{
							if (!std::filesystem::remove(visualStudioWatchdogFile))
							{
								jenova_log("[Jenova Deployer] Error : Could Not Remove Existing Watchdog Cache.");
								quick_exit(EXIT_FAILURE);
							}
						}

						// Decode Jenova Configuration
						std::string jenovaConfiguration = jenova::CreateStdStringFromCompressedBase64(configuration);

						// Create New Watchdog
						if (!jenova::WriteStdStringToFile(visualStudioWatchdogFile, jenovaConfiguration))
						{
							jenova_log("[Jenova Deployer] Error : Failed to Generate Visual Studio Watchdog Invoker.");
							quick_exit(EXIT_FAILURE);
						}

						// All Good
						jenova_log("[Jenova Deployer] Visual Studio Watchdog Invoked Successfully.");
						quick_exit(EXIT_SUCCESS);
					}

					// No Valid Command Executed
					jenova_log("[Jenova Deployer] Error : Invalid Watchdog Invoker Detected.");
					quick_exit(EXIT_FAILURE);
				}
				if (command == "finalize")
				{
					// Get Input/Output Values & Files
					std::string compilerModel = program.get<std::string>("--compiler");

					// Finalize Build Tools
					if (compilerModel == "msvc")
					{
						jenova_log("[Jenova Deployer] Build Completed.");
						quick_exit(EXIT_SUCCESS);
					}
				}

				// No Valid Command Executed
				jenova_log("[Jenova Deployer] Error : Deployment Command Not Found.");
				quick_exit(EXIT_FAILURE);
			}
			catch (const std::exception& error)
			{
				// Argument Parse Failed
				jenova_log("[Jenova Deployer] Error : %s", error.what());
				quick_exit(EXIT_FAILURE);
			}
		}
	}

	// Global Storage
	namespace GlobalStorage
	{
		// Holders
		ExtensionInitializerData ExtensionInitData = {0};

		// Configurations
		jenova::EngineMode CurrentEngineMode = jenova::EngineMode::Unknown;
		jenova::BuildAndRunMode CurrentBuildAndRunMode = jenova::BuildAndRunMode::DoNothing;
		jenova::ChangesTriggerMode CurrentChangesTriggerMode = jenova::ChangesTriggerMode::DoNothing;
		jenova::EditorVerboseOutput CurrentEditorVerboseOutput = jenova::EditorVerboseOutput::StandardOutput;

		// Database
		std::string CurrentJenovaCacheDirectory = "";
		std::string CurrentJenovaGeneratedConfiguration = "";
		std::string CurrentJenovaGodotSDKGeneratedData = "";
		std::string CurrentJenovaRuntimeModulePath = "";

		// Flags
		bool DeveloperModeActivated = jenova::GlobalSettings::VerboseEnabled;
		bool UseHotReloadAtRuntime = true;
		bool UseMonospaceFontForTerminal = true;

		// Values
		int TerminalDefaultFontSize = 12;
	}

	// Utilities & Helpers
	#pragma region JenovaUtilities
	void Alert(const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::FormatBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		MessageBoxA(0, buffer, "[JENOVA]", 0);
	}
	std::string Format(const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::FormatBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		return std::string(buffer);
	}
	std::string FormatSafe(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		// Calculate the size of the formatted string
		va_list args_copy;
		va_copy(args_copy, args);
		int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
		va_end(args_copy);

		if (size < 0) {
			va_end(args);
			throw std::runtime_error("Error During Formatting.");
		}

		// Create a string with the required size
		std::vector<char> buffer(size + 1);
		std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
		va_end(args);

		return std::string(buffer.data(), size);
	}
	void Output(const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::PrintOutputBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Suppress if Disabled
		if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::Disabled) return;

		// Handle Verbose In Different Modes
		if (QUERY_ENGINE_MODE(Editor))
		{
			// Jenvoa Terminal Log
			if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal)
			{
				if (jenova::plugin::JenovaEditorPlugin::get_singleton())
				{
					jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", String(" [color=#ed266c]>[/color] ") + String(buffer));
					return;
				}
			}

			// Standard Log
			UtilityFunctions::print_rich(String("[b][JENOVA][/b] [color=#ed266c]>[/color] ") + String(buffer));
		}
		else
		{
			UtilityFunctions::print(String("[JENOVA] > ") + String(buffer));
		}
	}
	void OutputColored(const char* colorHash, const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::PrintOutputBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Suppress if Disabled
		if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::Disabled) return;

		// Handle Verbose In Different Modes
		if (QUERY_ENGINE_MODE(Editor))
		{
			// Jenvoa Terminal Log
			if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal)
			{
				if (jenova::plugin::JenovaEditorPlugin::get_singleton())
				{
					jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", "[color=" + String(colorHash) + "] > " + String(buffer) + "[/color]");
					return;
				}
			}

			// Standard Log
			UtilityFunctions::print_rich("[color=" + String(colorHash) + "][b][JENOVA][/b] > " + String(buffer) + "[/color]");
		}
		else
		{
			UtilityFunctions::print(String("[JENOVA] > ") + String(buffer));
		}
	}
	void Verbose(const char* fmt, ...)
	{
		if (!jenova::GlobalStorage::DeveloperModeActivated) return;
		char buffer[jenova::GlobalSettings::PrintOutputBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Suppress if Disabled
		if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::Disabled) return;

		// Handle Verbose In Different Modes
		if (QUERY_ENGINE_MODE(Editor))
		{
			// Jenvoa Terminal Log
			if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal)
			{
				if (jenova::plugin::JenovaEditorPlugin::get_singleton())
				{
					jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", String(" [color=#fcb603]^[/color] ") + String(buffer));
					return;
				}
			}

			// Standard Log
			UtilityFunctions::print_rich(String("[b][JENOVA][/b] [color=#fcb603]^[/color] ") + String(buffer));
		}
		else
		{
			UtilityFunctions::print(String("[JENOVA] ^ ") + String(buffer));
		}
	}
	void VerboseByID(int id, const char* fmt, ...)
	{
		if (!jenova::GlobalStorage::DeveloperModeActivated) return;
		char buffer[jenova::GlobalSettings::PrintOutputBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Suppress if Disabled
		if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::Disabled) return;

		// Generate A Unique Color based on id (Knuth's multiplicative hash, Mod 360 for HUE)
		unsigned int hash = id * 2654435761 % 360;

		// Convert HUE to RGB
		float hue = static_cast<float>(hash);
		float saturation = 0.8f; // Saturation of 80%
		float value = 0.9f; // Brightness of 90%
		int hi = static_cast<int>(hue / 60) % 6;
		float f = hue / 60 - hi;
		float p = value * (1 - saturation);
		float q = value * (1 - f * saturation);
		float t = value * (1 - (1 - f) * saturation);
		float r, g, b;
		switch (hi) 
		{
			case 0: r = value; g = t; b = p; break;
			case 1: r = q; g = value; b = p; break;
			case 2: r = p; g = value; b = t; break;
			case 3: r = p; g = q; b = value; break;
			case 4: r = t; g = p; b = value; break;
			case 5: r = value; g = p; b = q; break;
		}

		// Convert RGB to hex
		std::stringstream colorHash;
		colorHash << "#" << std::hex 
			<< std::setw(2) << std::setfill('0') << static_cast<int>(r * 255)
			<< std::setw(2) << std::setfill('0') << static_cast<int>(g * 255)
			<< std::setw(2) << std::setfill('0') << static_cast<int>(b * 255);


		// Handle Verbose In Different Modes
		if (QUERY_ENGINE_MODE(Editor))
		{
			// Jenvoa Terminal Log
			if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal)
			{
				if (jenova::plugin::JenovaEditorPlugin::get_singleton())
				{
					jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", "[color=" + String(colorHash.str().c_str()) + String("] ^ ") + String(buffer) + "[/color]");
					return;
				}
			}

			// Standard Log
			UtilityFunctions::print_rich("[color=" + String(colorHash.str().c_str()) + String("][JENOVA] ^ ") + String(buffer) + "[/color]");
		}
		else
		{
			UtilityFunctions::print(String("[JENOVA] ^ ") + String(buffer));
		}
	}
	void Error(const char* stageName, const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::PrintOutputBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Handle Verbose In Different Modes
		if (QUERY_ENGINE_MODE(Editor))
		{
			// Jenvoa Terminal Log
			if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal)
			{
				if (jenova::plugin::JenovaEditorPlugin::get_singleton())
				{
					// Print Error Header
					jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", " [color=#f70f32]> [b]Error at Stage [" + String(stageName) + "] [/b][/color]");

					// Split Error Lines
					std::istringstream errorStream(buffer);
					std::string errorline;
					while (std::getline(errorStream, errorline)) jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", " [color=#f70f4d]  " + String(errorline.c_str()) + "[/color]");
					return;
				}
			}

			// Standard Log
			ERR_PRINT(String("[ ") + String(stageName) + " ] " + String(buffer));
		}
		else
		{
			UtilityFunctions::push_error(String("[Runtime Error] [ ") + String(stageName) + " ] " + String(buffer));
		}
	}
	void Warning(const char* stageName, const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::PrintOutputBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Handle Verbose In Different Modes
		if (QUERY_ENGINE_MODE(Editor))
		{
			// Jenvoa Terminal Log
			if (jenova::GlobalStorage::CurrentEditorVerboseOutput == jenova::EditorVerboseOutput::JenovaTerminal)
			{
				if (jenova::plugin::JenovaEditorPlugin::get_singleton())
				{
					// Print Error Header
					jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", " [color=#f7a60f]> [b]Warning at Stage [" + String(stageName) + "] [/b][/color]");

					// Split Error Lines
					std::istringstream warningStream(buffer);
					std::string warningline;
					while (std::getline(warningStream, warningline)) jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("VerboseLog", " [color=#f7b90f]  " + String(warningline.c_str()) + "[/color]");
					return;
				}
			}

			// Standard Log
			WARN_PRINT(String("[ ") + String(stageName) + " ] " + String(buffer));
		}
		else
		{
			UtilityFunctions::push_warning(String("[Runtime Warning] [ ") + String(stageName) + " ] " + String(buffer));
		}
	}
	void ErrorMessage(const char* title, const char* fmt, ...)
	{
		// Genereate Error Message
		char buffer[2048];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		// Show Error Message
		HWND godotWindow = (HWND)DisplayServer::get_singleton()->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE);
		MessageBoxA(godotWindow, buffer, title, MB_ICONERROR);
	}
	std::string ConvertToStdString(const godot::String& gstr)
	{
		std::string str((char*)gstr.utf8().ptr(), gstr.utf8().size());
		if (!str.empty() && str.back() == '\0') str.pop_back();
		return str;
	}
	std::string ConvertToStdString(const godot::StringName& gstr)
	{
		std::string str((char*)gstr.to_utf8_buffer().ptr(), gstr.to_utf8_buffer().size());
		if (!str.empty() && str.back() == '\0') str.pop_back();
		return str;
	}
	std::wstring ConvertToWideStdString(const godot::String& gstr)
	{
		std::string str((char*)gstr.utf8().ptr(), gstr.utf8().size());
		if (!str.empty() && str.back() == '\0') str.pop_back();
		return std::wstring (str.begin(), str.end());
	}
	std::string GetNameFromPath(godot::String gstr)
	{
		std::string path((char*)gstr.to_utf8_buffer().ptr(), gstr.to_utf8_buffer().size());
		size_t pos = path.find("res://");
		if (pos != std::string::npos){ path.erase(pos, 6); }
		return path;
	}
	String GenerateStandardUIDFromPath(String resourcePath)
	{
		return resourcePath.sha1_text().substr(0, 20);
	}
	String GenerateStandardUIDFromPath(Resource* resourcePtr)
	{
		return GenerateStandardUIDFromPath(resourcePtr->get_path());
	}
	std::string GenerateRandomHashString()
	{
		const int hashLength = 16;
		const char hexChars[] = "0123456789ABCDEF";
		std::stringstream ss;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 15);
		for (int i = 0; i < hashLength; ++i) ss << hexChars[dis(gen)];
		return ss.str();
	}
	std::string GenerateTerminalLogTime()
	{
		auto now = std::chrono::system_clock::now();
		auto now_time_t = std::chrono::system_clock::to_time_t(now);
		auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		std::tm now_tm; localtime_s(&now_tm, &now_time_t);
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(2) << now_tm.tm_hour << ":"
			<< std::setw(2) << now_tm.tm_min << ":"
			<< std::setw(2) << now_tm.tm_sec << ":"
			<< std::setw(4) << now_ms.count();
		return oss.str();
	}
	jenova::EngineMode GetCurrentEngineInstanceMode()
	{
		return jenova::GlobalStorage::CurrentEngineMode;
	}
	String GetCurrentEngineInstanceModeAsString()
	{
		switch (jenova::GlobalStorage::CurrentEngineMode)
		{
		case EngineMode::Editor:
			return "Editor";
		case EngineMode::Debug:
			return "Debug";
		case EngineMode::Runtime:
			return "Runtime";
		default:
			return "Unknown";
		}
	}
	Ref<ImageTexture> CreateImageTextureFromByteArray(const ::byte* imageDataPtr, size_t imageDataSize, ImageCreationFormat imageFormat)
	{
		return CreateImageTextureFromByteArrayEx(imageDataPtr, imageDataSize, Vector2i(), imageFormat);
	}
	Ref<ImageTexture> CreateImageTextureFromByteArrayEx(const ::byte* imageDataPtr, size_t imageDataSize, const Vector2i& imageSize, jenova::ImageCreationFormat imageFormat)
	{
		PackedByteArray imageDataPackedBytes;
		imageDataPackedBytes.resize(imageDataSize);
		memcpy(imageDataPackedBytes.ptrw(), imageDataPtr, imageDataSize);
		Ref<Image> createdImage = memnew(Image);
		godot::Error loadResult = godot::Error::FAILED;
		switch (imageFormat)
		{
		case jenova::ImageCreationFormat::PNG:
			loadResult = createdImage->load_png_from_buffer(imageDataPackedBytes);
			break;
		case jenova::ImageCreationFormat::JPG:
			loadResult = createdImage->load_jpg_from_buffer(imageDataPackedBytes);
			break;
		case jenova::ImageCreationFormat::SVG:
			loadResult = createdImage->load_svg_from_buffer(imageDataPackedBytes);
			break;
		default:
			break;
		}
		imageDataPackedBytes.clear();
		if (loadResult == godot::Error::OK)
		{
			if (imageSize != Vector2i(0, 0)) createdImage->resize(imageSize.width, imageSize.height, Image::INTERPOLATE_LANCZOS);
			Ref<ImageTexture> createdImageTexture = ImageTexture::create_from_image(createdImage);
			return createdImageTexture;
		}
		else 
		{
			jenova::Error("Texture/Image Loader", "Failed to Load Image From Memory Buffer.");
			createdImage.unref();
		}
		return nullptr;
	}
	Ref<ImageTexture> CreateMenuItemIconFromByteArray(const ::byte* imageDataPtr, size_t imageDataSize, jenova::ImageCreationFormat imageFormat)
	{
		PackedByteArray imageDataPackedBytes;
		imageDataPackedBytes.resize(imageDataSize);
		memcpy(imageDataPackedBytes.ptrw(), imageDataPtr, imageDataSize);
		Ref<Image> createdImage = memnew(Image);
		godot::Error loadResult = godot::Error::FAILED;
		switch (imageFormat)
		{
		case jenova::ImageCreationFormat::PNG:
			loadResult = createdImage->load_png_from_buffer(imageDataPackedBytes);
			break;
		case jenova::ImageCreationFormat::JPG:
			loadResult = createdImage->load_jpg_from_buffer(imageDataPackedBytes);
			break;
		case jenova::ImageCreationFormat::SVG:
			loadResult = createdImage->load_svg_from_buffer(imageDataPackedBytes);
			break;
		default:
			break;
		}
		imageDataPackedBytes.clear();
		if (loadResult == godot::Error::OK)
		{
			int menuIconSize = 16.0f * EditorInterface::get_singleton()->get_editor_scale();
			createdImage->resize(menuIconSize, menuIconSize, Image::INTERPOLATE_LANCZOS);
			Ref<ImageTexture> createdImageTexture = ImageTexture::create_from_image(createdImage);
			return createdImageTexture;
		}
		else
		{
			jenova::Error("Texture/Image Loader", "Failed to Load Menu Icon From Memory Buffer.");
			createdImage.unref();
		}
		return nullptr;
	}
	Ref<FontFile> CreateFontFileFromByteArray(const ::byte* fontDataPtr, size_t fontDataSize)
	{
		// Create Byte Array
		PackedByteArray fontDataPackedBytes;
		fontDataPackedBytes.resize(fontDataSize);
		memcpy(fontDataPackedBytes.ptrw(), fontDataPtr, fontDataSize);

		// Create New Font File
		Ref<FontFile> newFont;
		newFont.instantiate();

		// Set Data
		newFont->set_data(fontDataPackedBytes);

		// Return Loaded Font
		return newFont;
	}
	bool CollectResourcesFromFileSystem(const String& rootPath, const String& extensions, jenova::ResourceCollection& collectedResources, bool respectGDIgnore)
	{
		// Create Extension List
		PackedStringArray extension_list = extensions.split(" ");

		// Open Root
		Ref<DirAccess> dir = DirAccess::open(rootPath);
		if (!dir.is_valid()) return false;

		// Start Iterating
		dir->list_dir_begin();

		// Iteration
		String file_name = dir->get_next();
		while (!file_name.is_empty())
		{
			if (dir->current_is_dir())
			{
				if (file_name != "." && file_name != "..")
				{
					// Check for .gdignore file in the directory
					if (respectGDIgnore)
					{
						Ref<DirAccess> subdir = DirAccess::open("" + rootPath + "/" + file_name);
						if (subdir.is_valid() && subdir->file_exists(".gdignore"))
						{
							file_name = dir->get_next();
							continue;
						}
					}

					// Recursive call
					CollectResourcesFromFileSystem("" + rootPath + "/" + file_name, extensions, collectedResources, respectGDIgnore);
				}
			}
			else
			{
				// Check if file has one of the allowed extensions
				String file_extension = file_name.get_extension();
				if (extension_list.has(file_extension))
				{
					Ref<Resource> resource = ResourceLoader::get_singleton()->load("" + rootPath + "/" + file_name);
					if (resource.is_valid()) collectedResources.push_back(resource);
				}
			}
			file_name = dir->get_next();
		}

		// Stop Iteration
		dir->list_dir_end();

		// All Good
		return true;
	}
	bool CollectScriptsFromFileSystemAndScenes(const String& rootPath, const String& extension, jenova::ResourceCollection& collectedResources, bool respectGDIgnore)
	{
		// Open Root
		Ref<DirAccess> dir = DirAccess::open(rootPath);
		if (!dir.is_valid()) return false;

		// Internal Functions
		std::function<void(Node*, jenova::ResourceCollection&)> CollectScriptsFromNodes;
		auto CollectEmbeddedScriptsFromScene = [&](Ref<PackedScene> scene, jenova::ResourceCollection& collectedResources)
		{
			Node* root = scene->instantiate();
			if (!root) return;
			CollectScriptsFromNodes(root, collectedResources);
			memdelete(root); // Must Not Use `queue_free`
			scene->unreference();
		};
		CollectScriptsFromNodes = [&](Node* node, jenova::ResourceCollection& collectedResources)
		{
			Ref<Script> script = node->get_script();
			if (script.is_valid())
			{
				String scriptPath = script->get_path();
				if (script->is_class(jenova::GlobalSettings::JenovaScriptType) && scriptPath.contains("::") && !script->get_source_code().is_empty())
				{
					if (!collectedResources.has(script)) collectedResources.push_back(script);
				}
			}
			for (int i = 0; i < node->get_child_count(); i++) CollectScriptsFromNodes(node->get_child(i), collectedResources);
		};

		// Start Iterating
		dir->list_dir_begin();

		// Iteration
		String file_name = dir->get_next();
		while (!file_name.is_empty())
		{
			if (file_name == "." || file_name == "..")
			{
				file_name = dir->get_next();
				continue;
			}

			String full_path = rootPath.path_join(file_name);

			if (dir->current_is_dir())
			{
				if (respectGDIgnore)
				{
					Ref<DirAccess> subdir = DirAccess::open(full_path);
					if (subdir.is_valid() && subdir->file_exists(".gdignore"))
					{
						file_name = dir->get_next();
						continue;
					}
				}

				CollectScriptsFromFileSystemAndScenes(full_path, extension, collectedResources, respectGDIgnore);
			}
			else
			{
				// Check for Scene files (.tscn or .scn)
				if (file_name.get_extension() == "tscn" || file_name.get_extension() == "scn")
				{
					Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(full_path);
					if (scene.is_valid())
					{
						CollectEmbeddedScriptsFromScene(scene, collectedResources);
					}
				}
				else if (file_name.get_extension() == extension)
				{
					Ref<Resource> resource = ResourceLoader::get_singleton()->load(full_path);
					if (resource.is_valid())
					{
						collectedResources.push_back(resource);
					}
				}
			}

			file_name = dir->get_next();
		}

		// Stop Iteration
		dir->list_dir_end();

		// All Good
		return true;
	}
	void RegisterDocumentationFromByteArray(const char* xmlDataPtr, size_t xmlDataSize)
	{
		std::string documentationData(xmlDataPtr, xmlDataSize);
		internal::gdextension_interface_editor_help_load_xml_from_utf8_chars_and_len(documentationData.data(), documentationData.size());
	}
	void CopyStringToClipboard(const String& str)
	{
		DisplayServer::get_singleton()->clipboard_set(str);
	}
	String GetStringFromClipboard()
	{
		return DisplayServer::get_singleton()->clipboard_get();
	}
	void CopyStdStringToClipboard(const std::string& str)
	{
		if (!OpenClipboard(nullptr)) return;
		EmptyClipboard();
		HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, str.size() + 1);
		if (!hClipboardData)
		{
			CloseClipboard();
			return;
		}
		char* pchData = static_cast<char*>(GlobalLock(hClipboardData));
		if (pchData != nullptr)
		{
			strcpy_s(pchData, str.size() + 1, str.c_str());
			GlobalUnlock(hClipboardData);
		}
		SetClipboardData(CF_TEXT, hClipboardData);
		CloseClipboard();
	}
	std::string GetStdStringFromClipboard()
	{
		return ConvertToStdString(DisplayServer::get_singleton()->clipboard_get());
	}
	jenova::ArgumentsArray CreateArgumentsArrayFromString(const std::string& str, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		bool insideQuotes = false;
		std::stringstream ss;
		for (char c : str)
		{
			if (c == '"')
			{
				insideQuotes = !insideQuotes;
			}
			else if (c == delimiter && !insideQuotes)
			{
				if (!ss.str().empty())
				{
					tokens.push_back(ss.str());
					ss.str("");
					ss.clear();
				}
			}
			else
			{
				ss << c;
			}
		}
		if (!ss.str().empty()) tokens.push_back(ss.str());
		return tokens;
	}
	std::string GetExecutablePath()
	{
		char pathBuffer[MAX_PATH];
		GetModuleFileNameA(NULL, pathBuffer, MAX_PATH);
		return std::string(pathBuffer);
	}
	void ResetCurrentDirectoryToRoot()
	{
		char buffer[MAX_PATH];
		GetModuleFileNameA(nullptr, buffer, MAX_PATH);
		std::string fullPath(buffer);
		size_t pos = fullPath.find_last_of("\\/");
		std::string folder = fullPath.substr(0, pos);
		SetCurrentDirectoryA(folder.c_str());
	}
	bool GenerateMiniMemoryDump(EXCEPTION_POINTERS* exceptionInfo)
	{
		// Create a directory named "MaxFusionCrashData" in the Windows temp directory
		wchar_t tempPath[MAX_PATH];
		GetTempPath(MAX_PATH, tempPath);
		std::wstring crashDir = std::wstring(tempPath) + L"JenovaCrashData\\";
		CreateDirectory(crashDir.c_str(), NULL);

		// Generate a random hash ID for the crash dump file
		std::string crashHash = GenerateRandomHashString();
		wchar_t dumpFileName[MAX_PATH];
		wsprintf(dumpFileName, L"%sJenovaCore_Crash_Dump_%hs.dmp", crashDir.c_str(), crashHash.c_str());

		// Open the dump file for writing
		HANDLE hDumpFile = CreateFile(dumpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			// Set up the MiniDumpWriteDump options
			MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
			dumpInfo.ThreadId = GetCurrentThreadId();
			dumpInfo.ExceptionPointers = exceptionInfo;
			dumpInfo.ClientPointers = TRUE;

			// Write the mini dump file
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

			// Close the dump file handle
			CloseHandle(hDumpFile);

			// All Good
			return true;
		}

		// Something Went Wrong
		return false;
	}
	void DoApplicationEvents()
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else break;
		}
	}
	bool QueueProjectBuild()
	{
		if (!jenova::plugin::JenovaEditorPlugin::get_singleton()) return false;
		jenova::plugin::JenovaEditorPlugin::get_singleton()->call_deferred("BuildProject");
		return true;
	}
	bool UpdateGlobalStorageFromEditorSettings()
	{
		if (!jenova::plugin::JenovaEditorPlugin::get_singleton()) return false;
		return jenova::plugin::JenovaEditorPlugin::get_singleton()->UpdateStorageConfigurations();
	}
	std::string GetNotificationString(int p_what)
	{
		switch (p_what)
		{
			case 0:  return "NOTIFICATION_POSTINITIALIZE";
			case 1:  return "NOTIFICATION_PREDELETE";
			case 2:  return "NOTIFICATION_EXTENSION_RELOADED";
			case 3:  return "NOTIFICATION_PREDELETE_CLEANUP";
			case 10: return "NOTIFICATION_ENTER_TREE";
			case 11: return "NOTIFICATION_EXIT_TREE";
			case 12: return "NOTIFICATION_MOVED_IN_PARENT";
			case 13: return "NOTIFICATION_READY";
			case 14: return "NOTIFICATION_PAUSED";
			case 15: return "NOTIFICATION_UNPAUSED";
			case 16: return "NOTIFICATION_PHYSICS_PROCESS";
			case 17: return "NOTIFICATION_PROCESS";
			case 18: return "NOTIFICATION_PARENTED";
			case 19: return "NOTIFICATION_UNPARENTED";
			case 20: return "NOTIFICATION_SCENE_INSTANTIATED";
			case 21: return "NOTIFICATION_DRAG_BEGIN";
			case 22: return "NOTIFICATION_DRAG_END";
			case 23: return "NOTIFICATION_PATH_RENAMED";
			case 24: return "NOTIFICATION_CHILD_ORDER_CHANGED";
			case 25: return "NOTIFICATION_INTERNAL_PROCESS";
			case 26: return "NOTIFICATION_INTERNAL_PHYSICS_PROCESS";
			case 27: return "NOTIFICATION_POST_ENTER_TREE";
			case 28: return "NOTIFICATION_DISABLED";
			case 29: return "NOTIFICATION_ENABLED";
			case 30: return "NOTIFICATION_DRAW";
			case 31: return "NOTIFICATION_VISIBILITY_CHANGED";
			case 32: return "NOTIFICATION_ENTER_CANVAS";
			case 33: return "NOTIFICATION_EXIT_CANVAS";
			case 35: return "NOTIFICATION_LOCAL_TRANSFORM_CHANGED";
			case 36: return "NOTIFICATION_WORLD_2D_CHANGED";
			case 40: return "NOTIFICATION_RESIZED";
			case 41: return "NOTIFICATION_MOUSE_ENTER";
			case 42: return "NOTIFICATION_MOUSE_EXIT";
			case 43: return "NOTIFICATION_FOCUS_ENTER";
			case 44: return "NOTIFICATION_FOCUS_EXIT";
			case 45: return "NOTIFICATION_THEME_CHANGED";
			case 47: return "NOTIFICATION_SCROLL_BEGIN";
			case 48: return "NOTIFICATION_SCROLL_END";
			case 49: return "NOTIFICATION_LAYOUT_DIRECTION_CHANGED";
			case 60: return "NOTIFICATION_MOUSE_ENTER_SELF";
			case 61: return "NOTIFICATION_MOUSE_EXIT_SELF";
			case 2000: return "NOTIFICATION_TRANSFORM_CHANGED";
			case 2001: return "NOTIFICATION_RESET_PHYSICS_INTERPOLATION";
			case 9001: return "NOTIFICATION_EDITOR_PRE_SAVE";
			case 9002: return "NOTIFICATION_EDITOR_POST_SAVE";
			case 1002: return "NOTIFICATION_WM_MOUSE_ENTER";
			case 1003: return "NOTIFICATION_WM_MOUSE_EXIT";
			case 1004: return "NOTIFICATION_WM_WINDOW_FOCUS_IN";
			case 1005: return "NOTIFICATION_WM_WINDOW_FOCUS_OUT";
			case 1006: return "NOTIFICATION_WM_CLOSE_REQUEST";
			case 1007: return "NOTIFICATION_WM_GO_BACK_REQUEST";
			case 1008: return "NOTIFICATION_WM_SIZE_CHANGED";
			case 1009: return "NOTIFICATION_WM_DPI_CHANGE";
			case 1010: return "NOTIFICATION_VP_MOUSE_ENTER";
			case 1011: return "NOTIFICATION_VP_MOUSE_EXIT";
			case 2009: return "NOTIFICATION_OS_MEMORY_WARNING";
			case 2010: return "NOTIFICATION_TRANSLATION_CHANGED";
			case 2011: return "NOTIFICATION_WM_ABOUT";
			case 2012: return "NOTIFICATION_CRASH";
			case 2013: return "NOTIFICATION_OS_IME_UPDATE";
			case 2014: return "NOTIFICATION_APPLICATION_RESUMED";
			case 2015: return "NOTIFICATION_APPLICATION_PAUSED";
			case 2016: return "NOTIFICATION_APPLICATION_FOCUS_IN";
			case 2017: return "NOTIFICATION_APPLICATION_FOCUS_OUT";
			case 2018: return "NOTIFICATION_TEXT_SERVER_CHANGED";
			default: return "UNKNOWN_NOTIFICATION";
		}
	}
	String GetJenovaCacheDirectory()
	{
		// Generate Cache Directory Path
		String jenovaCacheDirectory = OS::get_singleton()->get_cache_dir() + String(jenova::GlobalSettings::JenovaCacheDirectory);

		// Try to Create It If Doesn't Exist
		if (!filesystem::exists(AS_STD_STRING(jenovaCacheDirectory)))
		{
			try
			{
				filesystem::create_directories(AS_STD_STRING(jenovaCacheDirectory));
			}
			catch (const std::filesystem::filesystem_error& e) { }
		}

		// Return Path
		return jenovaCacheDirectory;
	}
	String RemoveCommentsFromSource(const String& sourceCode) 
	{
		String comment_pattern = R"(\/\/[^\n]*|\/\*[\s\S]*?\*\/)";
		Ref<RegEx> regex = RegEx::create_from_string(comment_pattern);
		return regex->sub(sourceCode, "", true);
	}
	bool ContainsExactString(const String& srcStr, const String& matchStr) 
	{
		String pattern = "\\b" + matchStr + "\\b";
		Ref<RegEx> regex = RegEx::create_from_string(pattern);
		return regex->search(srcStr).is_valid();
	}
	std::string GetDemangledFunctionSignature(std::string mangledName, jenova::CompilerModel compilerModel)
	{
		// Demangled MSVC Function
		if (compilerModel == jenova::CompilerModel::MicrosoftCompiler)
		{
			char demangled_name[1024];
			if (UnDecorateSymbolName(mangledName.c_str(), demangled_name, sizeof(demangled_name), UNDNAME_COMPLETE)) return std::string(demangled_name);
		}
		
		// Unknown Compiler, Return Empty String
		return std::string();
	}
	std::string CleanFunctionSignature(const std::string& functionSignature) 
	{
		std::string cleanedSignature = functionSignature;

		// Remove calling conventions
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("__cdecl\\s+|__stdcall\\s+|__fastcall\\s+"), "");

		// Remove "class " prefix before classes
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("class\\s+"), "");

		// Remove "struct " prefix before classes
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("struct\\s+"), "");

		// Remove "__ptrXX" keyword
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*__ptr32"), "");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*__ptr64"), "");

		// Convert Types
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*unsigned __int32"), "uint32");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*unsigned __int64"), "uint64");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*__int32"), "int32");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*__int64"), "int64");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*unsigned int"), "uint32");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*unsigned char"), "byte");

		// Remove volatile qualifiers
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("volatile\\s+"), "");

		// Ensure space between type and pointer/reference
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("const\\s*\\*"), "*");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("const\\s*\\&"), "&");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*\\*"), "*");
		cleanedSignature = std::regex_replace(cleanedSignature, std::regex("\\s*\\&"), "&");

		return cleanedSignature;
	}
	jenova::ParameterTypeList ExtractParameterTypesFromSignature(const std::string& functionSignature)
	{
		ParameterTypeList parameterTypes;

		// Regex to match the function signature's parameters inside parentheses
		std::regex parameterRegex(R"(\(([^)]+)\))");
		std::smatch match;

		if (std::regex_search(functionSignature, match, parameterRegex))
		{
			// Extract Parameter Substring
			std::string parameterString = match[1]; 
			if (!parameterString.empty())
			{
				// Split Parameters
				std::string line;
				std::stringstream ss(parameterString);
				while (std::getline(ss, line, ',')) parameterTypes.push_back(line);
			}
		}

		// Return Parameter Types
		return parameterTypes;
	}
	std::string ExtractReturnTypeFromSignature(const std::string& functionSignature) 
	{
		std::regex funcRegex(R"(^\s*([^\s]+)\s+\w+::\w+\()");
		std::smatch match;
		if (std::regex_search(functionSignature, match, funcRegex)) return match[1];
		return std::string();
	}
	std::string ExtractPropertyTypeFromSignature(const std::string& propertySignature)
	{
		std::regex propRegex(R"(^\s*([^\s]+)\s+\w+::\w+$)");
		std::smatch match;
		if (std::regex_search(propertySignature, match, propRegex)) return match[1];
		return std::string();
	}
	bool LoadSymbolForModule(HANDLE process, DWORD64 baseAddress, const std::string& pdbPath, size_t dllSize)
	{
		// Initialize the symbol handler for the process
		if (!SymInitialize(process, NULL, FALSE)) return false;

		// Load the symbols for the memory-mapped DLL
		DWORD64 moduleBase = SymLoadModuleEx(process, NULL, pdbPath.c_str(), NULL, baseAddress, dllSize, NULL, 0);
		if (moduleBase == 0) { SymCleanup(process); return false; }

		// Verbose
		if (QUERY_ENGINE_MODE(Editor))
			jenova::Output("Symbol [[color=#db2e59]%p[/color]] Loaded for Jenova Module [[color=#44e376]%p[/color]] at [[color=#44e376]%p[/color]]", pdbPath, baseAddress, moduleBase);

		// All Good
		return moduleBase == baseAddress;
	}
	bool InitializeExtensionModule(const char* initFuncName, HMODULE moduleBase)
	{
		// Initializer Function Typedef
		typedef GDExtensionBool(*InitializeExtensionModuleFunc)(ExtensionInitializerData*);

		// Get Initializer Function
		InitializeExtensionModuleFunc InitializeModule = (InitializeExtensionModuleFunc)GetProcAddress(moduleBase, initFuncName);
		if (!InitializeModule) return false;

		// Create A Clone from Initializer Data
		ExtensionInitializerData extInitData = jenova::GlobalStorage::ExtensionInitData;
		GDExtensionInitialization initData = {};
		extInitData.godotExtensionInitialization = &initData;

		// Execute Initializer Function
		if (!InitializeModule(&extInitData)) return false;

		// All Good
		return true;
	}
	bool CallModuleEvent(const char* eventFuncName, HMODULE moduleBase)
	{
		// Initializer Function Typedef
		typedef bool(*ModuleEventFun)(void);

		// Get Initializer Function
		ModuleEventFun ModuleEvent = (ModuleEventFun)GetProcAddress(moduleBase, eventFuncName);

		// Event Functions Are Optional, If Doesn't Exist We Ignore
		if (!ModuleEvent) return true;

		// Execute Module Event Function
		return ModuleEvent();
	}
	jenova::ScriptModule CreateScriptModuleFromInternalSource(const std::string& sourceName, const std::string& sourceCode)
	{
		// Create Module
		jenova::ScriptModule builtinModule;
		builtinModule.scriptFilename = String(sourceName.c_str());
		builtinModule.scriptUID = jenova::GenerateStandardUIDFromPath(builtinModule.scriptFilename);
		builtinModule.scriptType = jenova::ScriptModuleType::InternalScript;
		builtinModule.scriptSource = String(sourceCode.c_str());
		builtinModule.scriptHash = builtinModule.scriptSource.md5_text();
		builtinModule.scriptCacheFile = jenova::GetJenovaCacheDirectory() + builtinModule.scriptFilename + "_" + builtinModule.scriptUID + ".cpp";
		builtinModule.scriptObjectFile = jenova::GetJenovaCacheDirectory() + builtinModule.scriptFilename + "_" + builtinModule.scriptUID + ".obj";

		// Create C++ File
		Ref<FileAccess> handle = FileAccess::open(builtinModule.scriptCacheFile, FileAccess::ModeFlags::WRITE);
		if (handle.is_valid())
		{
			handle->store_string(builtinModule.scriptSource);
			handle->close();
		}
		else
		{
			jenova::Error("Jenova Builder", "Failed to Create Internal Script File : %s", sourceName.c_str());
			builtinModule.scriptType = jenova::ScriptModuleType::Unknown;
		}

		// Return New Module
		return builtinModule;
	}
	bool CreateFileFromInternalSource(const std::string& sourceFile, const std::string& sourceCode)
	{
		return WriteStdStringToFile(sourceFile, sourceCode);
	}
	bool CreateBuildCacheDatabase(const std::string& cacheFile, const jenova::ModuleList& scriptModules, const jenova::HeaderList& scriptHeaders, bool skipHashes)
	{
		try
		{
			// Create JSON Serializer
			nlohmann::json serializer;

			// Cache Script Modules Hashes
			for (const auto& scriptModule : scriptModules)
			{
				// Update Hashes
				serializer["Modules"][AS_STD_STRING(scriptModule.scriptUID)] = skipHashes ? "Skipped" : AS_STD_STRING(scriptModule.scriptHash);

				// Update Proxies
				serializer["Proxies"][AS_STD_STRING(scriptModule.scriptCacheFile.get_file())] = AS_STD_STRING(scriptModule.scriptFilename);
			}
			serializer["ModuleCount"] = scriptModules.size();

			// Cache Script Headers Hashes
			for (const auto& scriptHeader : scriptHeaders)
			{
				// Update Hashes
				serializer["Headers"][AS_STD_STRING(jenova::GenerateStandardUIDFromPath(scriptHeader))] = AS_STD_STRING(jenova::GenerateMD5HashFromFile(scriptHeader));
			}
			serializer["HeaderCount"] = scriptHeaders.size();

			// Add Timestamp
			serializer["BuildTimestamp"] = time(NULL);

			// Write Cache File On Disk
			Ref<FileAccess> handle = FileAccess::open(String(cacheFile.c_str()), FileAccess::ModeFlags::WRITE);
			if (handle.is_valid())
			{
				handle->store_string(String(serializer.dump(2).c_str()));
				handle->close();

				// Verbose
				jenova::VerboseByID(__LINE__, "Build Cache Database Saved At (%s)", cacheFile);

				// All Good
				return true;
			}
			else
			{
				return false;
			}
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
	std::string GetLoadedModulePath(HINSTANCE hInstance)
	{
		char dllPath[MAX_PATH];
		GetModuleFileNameA(hInstance, dllPath, MAX_PATH);
		return std::string(dllPath);
	}
	jenova::MemoryBuffer CompressBuffer(void* bufferPtr, size_t bufferSize)
	{
		try
		{
			jenova::MemoryBuffer buffer;
			const size_t BUFSIZE = 128 * 1024;
			std::unique_ptr<uint8_t[]> temp_buffer(new uint8_t[BUFSIZE]);

			z_stream strm;
			strm.zalloc = 0;
			strm.zfree = 0;
			strm.next_in = reinterpret_cast<uint8_t*>(bufferPtr);
			strm.avail_in = uInt(bufferSize);
			strm.next_out = temp_buffer.get();
			strm.avail_out = BUFSIZE;

			deflateInit(&strm, Z_BEST_COMPRESSION);

			while (strm.avail_in != 0)
			{
				int res = deflate(&strm, Z_NO_FLUSH);
				assert(res == Z_OK);
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
			}

			int deflate_res = Z_OK;
			while (deflate_res == Z_OK)
			{
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
				deflate_res = deflate(&strm, Z_FINISH);
			}

			assert(deflate_res == Z_STREAM_END);
			buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE - strm.avail_out);
			deflateEnd(&strm);

			return buffer;
		}
		catch (const std::exception&)
		{
			return jenova::MemoryBuffer();
		}
	}
	jenova::MemoryBuffer DecompressBuffer(void* bufferPtr, size_t bufferSize)
	{
		try
		{
			jenova::MemoryBuffer buffer;
			const size_t BUFSIZE = 128 * 1024;
			std::unique_ptr<uint8_t[]> temp_buffer(new uint8_t[BUFSIZE]);

			z_stream strm;
			strm.zalloc = 0;
			strm.zfree = 0;
			strm.next_in = reinterpret_cast<uint8_t*>(bufferPtr);
			strm.avail_in = uInt(bufferSize);
			strm.next_out = temp_buffer.get();
			strm.avail_out = BUFSIZE;

			inflateInit(&strm);

			while (strm.avail_in != 0)
			{
				int res = inflate(&strm, Z_NO_FLUSH);
				assert(res == Z_OK);
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
			}

			int inflate_res = Z_OK;
			while (inflate_res == Z_OK)
			{
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
				inflate_res = inflate(&strm, Z_FINISH);
			}

			assert(inflate_res == Z_STREAM_END);
			buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE - strm.avail_out);
			inflateEnd(&strm);

			return buffer;
		}
		catch (const std::exception&)
		{
			return jenova::MemoryBuffer();
		}
	}
	float CalculateCompressionRatio(size_t baseSize, size_t compressedSize) 
	{
		if (baseSize == 0) return 100.0f;
		return (1.0 - static_cast<float>(compressedSize) / baseSize) * 100;
	}
	Ref<Texture2D> GetEditorIcon(const String& iconName)
	{
		return EditorInterface::get_singleton()->get_editor_theme()->get_icon(iconName, "EditorIcons");
	}
	bool DumpThemeColors(const Ref<Theme> theme)
	{
		if (!theme.is_valid()) return false;
		for (const auto& coltype : theme->get_color_type_list())
		{
			for (const auto& color : theme->get_color_list(coltype))
			{
				Color col = theme->get_color(color, coltype);
				jenova::OutputColored(("#" + AS_STD_STRING(col.to_html())).c_str(), "[b]Dumped Color [%s][%s][%s][/b] ",
					AS_C_STRING(coltype), AS_C_STRING(color), AS_C_STRING(col.to_html()));
			}
		}
		return true;
	}
	jenova::ArgumentsArray ProcessDeployerArguments(const std::string& cmdLine)
	{
		std::regex paramRegex(R"(/([^:\s]+):\"([^\"]+)\")");
		std::string processedCmdLine = std::regex_replace(cmdLine, paramRegex, "--$1 \"$2\"");
		return jenova::CreateArgumentsArrayFromString(processedCmdLine, ' ');
	}
	bool WriteStringToFile(const String& filePath, const String& str)
	{
		// Write String On Disk
		Ref<FileAccess> handle = FileAccess::open(filePath, FileAccess::ModeFlags::WRITE);
		if (handle.is_valid())
		{
			handle->store_string(str);
			handle->close();
			return true;
		}
		else
		{
			return false;
		}
	}
	String ReadStringFromFile(const String& filePath)
	{
		// Read String From Disk
		Ref<FileAccess> handle = FileAccess::open(filePath, FileAccess::ModeFlags::READ);
		if (handle.is_valid())
		{
			String content = handle->get_as_text();
			handle->close();
			return content;
		}
		else
		{
			return "";
		}
	}
	bool WriteStdStringToFile(const std::string& filePath, const std::string& str)
	{
		std::ofstream outFile(filePath, std::ios::out | std::ios::binary);
		if (outFile.is_open())
		{
			outFile.write(str.c_str(), str.size());
			outFile.close();
			return true;
		}
		else
		{
			return false;
		}
	}
	std::string ReadStdStringFromFile(const std::string& filePath)
	{
		std::ifstream inFile(filePath);
		if (inFile.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
			inFile.close();
			return content;
		}
		else
		{
			return "";
		}
	}
	void ReplaceAllMatchesWithString(std::string& targetString, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while ((start_pos = targetString.find(from, start_pos)) != std::string::npos)
		{
			targetString.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}
	std::string ReplaceAllMatchesWithStringAndReturn(std::string targetString, const std::string& from, const std::string& to)
	{
		ReplaceAllMatchesWithString(targetString, from, to);
		return targetString;
	}
	jenova::ArgumentsArray SplitStdStringToArguments(std::string& str, char delimiter)
	{
		ArgumentsArray arguments;
		std::stringstream ss(str);
		std::string argument;
		while (std::getline(ss, argument, delimiter)) if (!argument.empty()) arguments.push_back(argument);
		return arguments;
	}
	jenova::ScriptEntityContainer CreateScriptEntityContainer(const String& rootPath)
	{
		// Create Entity List
		jenova::ScriptEntityContainer scriptCollection;

		// Get Project Path
		std::filesystem::path projectPath = std::filesystem::absolute(AS_STD_STRING(ProjectSettings::get_singleton()->globalize_path(rootPath)));
		scriptCollection.rootPath = projectPath.string();

		// Collect C++ Scripts [No Support for Builtin Yet]
		jenova::ResourceCollection cppResources;
		if (!jenova::CollectScriptsFromFileSystemAndScenes(rootPath, "cpp", cppResources)) return jenova::ScriptEntityContainer();

		// Create C++ File List
		for (const auto& cppResource : cppResources)
		{
			if (cppResource->is_class(jenova::GlobalSettings::JenovaScriptType))
			{
				// Get C++ Script Object
				Ref<CPPScript> cppScript = Object::cast_to<CPPScript>(cppResource.ptr());

				// Update Identity
				if (!cppScript->HasValidScriptIdentity()) cppScript->GenerateScriptIdentity();

				// Get Paths
				std::filesystem::path cppFilePath = std::filesystem::absolute(AS_STD_STRING(ProjectSettings::get_singleton()->globalize_path(cppScript->get_path())));
				std::filesystem::path relativePath = std::filesystem::relative(cppFilePath, projectPath);

				// Create Script Module
				jenova::ScriptModule scriptModule;
				scriptModule.scriptFilename = cppScript->get_path();
				scriptModule.scriptUID = cppScript->GetScriptIdentity();
				scriptModule.scriptType = cppScript->is_built_in() ? jenova::ScriptModuleType::BuiltinEntityScript : jenova::ScriptModuleType::EntityScript;
				scriptModule.scriptHash = cppScript->get_source_code().md5_text();

				// Store Source Code For Builtin Scripts
				if (cppScript->is_built_in()) scriptModule.scriptSource = cppScript->get_source_code();

				// Add to Collection
				scriptCollection.scriptModules.push_back(scriptModule);
				scriptCollection.scriptFilesFullPath.push_back(cppFilePath.string());
				scriptCollection.scriptFilesReleative.push_back(relativePath.string());
				scriptCollection.scriptIdentities.push_back(AS_STD_STRING(cppScript->GetScriptIdentity()));

				// Collect Directories
				if (!cppScript->is_built_in())
				{
					std::filesystem::path directory = cppFilePath.parent_path();
					while (!directory.empty() && directory != projectPath.parent_path())
					{
						std::string directoryStr = directory.string();
						auto index = std::find(scriptCollection.scriptDirectoriesFullPath.begin(), scriptCollection.scriptDirectoriesFullPath.end(), directoryStr) - scriptCollection.scriptDirectoriesFullPath.begin();
						if (index == scriptCollection.scriptDirectoriesFullPath.size())
						{
							scriptCollection.scriptDirectoriesFullPath.push_back(directoryStr);
							scriptCollection.scriptDirectoriesReleative.push_back(std::filesystem::relative(directory, projectPath).string());
							scriptCollection.directoryCount++;
						}
						directory = directory.parent_path();
					}
				}

				// Update Builtin Count
				if (cppScript->is_built_in()) scriptCollection.builtinCount++;

				// Update Entity Count
				scriptCollection.entityCount++;
			}
		}

		// Sort directories [!]
		std::sort(scriptCollection.scriptDirectoriesReleative.begin(), scriptCollection.scriptDirectoriesReleative.end());
		std::sort(scriptCollection.scriptDirectoriesFullPath.begin(), scriptCollection.scriptDirectoriesFullPath.end());

		// Assign Directory Indices
		for (const auto& filePath : scriptCollection.scriptFilesFullPath)
		{
			std::filesystem::path directory = std::filesystem::path(filePath).parent_path();
			auto it = std::find(scriptCollection.scriptDirectoriesFullPath.begin(), scriptCollection.scriptDirectoriesFullPath.end(), directory.string());
			if (it != scriptCollection.scriptDirectoriesFullPath.end())
			{
				size_t index = std::distance(scriptCollection.scriptDirectoriesFullPath.begin(), it);
				scriptCollection.entityDirectoryIndex.push_back(index);
			}
			else
			{
				// Not Found Or It's In Root
				scriptCollection.entityDirectoryIndex.push_back(-1);
				scriptCollection.rootedCount++;
			}
		}

		// Add Virtual Embedded Directory If There's Any Builtin Directory
		if (scriptCollection.builtinCount != 0)
		{
			scriptCollection.scriptDirectoriesFullPath.push_back("Embedded");
			scriptCollection.scriptDirectoriesReleative.push_back("Embedded");
			scriptCollection.directoryCount++;
		}

		// Return Collection
		return scriptCollection;
	}
	std::string GenerateFilterUniqueIdentifier(std::string& filterName, bool addBrackets)
	{
		std::string md5Hash = AS_STD_STRING(String(filterName.c_str()).md5_text());
		std::string formattedHash = md5Hash.substr(0, 8) + "-" + md5Hash.substr(8, 4) + "-" + md5Hash.substr(12, 4) + "-" + md5Hash.substr(16, 4) + "-" + md5Hash.substr(20);
		if (addBrackets) formattedHash = "{" + formattedHash + "}";
		return formattedHash;
	}
	bool CompareFilePaths(const std::string& sourcePath, const std::string& destinationPath)
	{
		if (sourcePath.size() != destinationPath.size()) return false;
		for (size_t i = 0; i < sourcePath.size(); ++i)
		{
			char c1 = (sourcePath[i] == '\\') ? '/' : sourcePath[i];
			char c2 = (destinationPath[i] == '\\') ? '/' : destinationPath[i];
			if (c1 != c2) return false;
		}
		return true;
	}
	jenova::EncodedData CreateCompressedBase64FromStdString(const std::string& srcStr)
	{
		jenova::MemoryBuffer compressedData = CompressBuffer((void*)srcStr.data(), srcStr.size());
		jenova::EncodedData base64Data = base64::base64_encode(compressedData.data(), compressedData.size());
		jenova::MemoryBuffer().swap(compressedData);
		return base64Data;
	}
	std::string CreateStdStringFromCompressedBase64(const jenova::EncodedData& base64)
	{
		DecodedData decodedData = base64::base64_decode(base64);
		jenova::MemoryBuffer decompressedData = DecompressBuffer((void*)decodedData.data(), decodedData.size());
		std::string decodedString((char*)decompressedData.data(), decompressedData.size());
		jenova::MemoryBuffer().swap(decompressedData);
		return decodedString;
	}
	bool WriteMemoryBufferToFile(const std::string& filePath, const jenova::MemoryBuffer& memoryBuffer)
	{
		std::ofstream binaryFileWritter(filePath, std::ios::out | std::ios::binary);
		if (binaryFileWritter.is_open())
		{
			binaryFileWritter.write((char*)memoryBuffer.data(), memoryBuffer.size());
			binaryFileWritter.close();
			return true;
		}
		else
		{
			return false;
		}
	}
	jenova::MemoryBuffer ReadMemoryBufferFromFile(const std::string& filePath)
	{
		std::ifstream binaryFileReader(filePath, std::ios::binary);
		if (binaryFileReader.is_open())
		{
			jenova::MemoryBuffer memoryBuffer(std::istreambuf_iterator<char>(binaryFileReader), {});
			binaryFileReader.close();
			return memoryBuffer;
		}
		else
		{
			return jenova::MemoryBuffer();
		}
	}
	std::string ExtractMajorVersionFromFullVersion(const std::string& fullVersion)
	{
		size_t pos = fullVersion.find('.');
		std::string majorVersion = fullVersion.substr(0, pos);
		return majorVersion;
	}
	std::string GetVisualStudioPlatformToolsetFromVersion(const std::string& versionNumber)
	{
		if (versionNumber == "18") return "v144";
		if (versionNumber == "17") return "v143";
		if (versionNumber == "16") return "v142";
		if (versionNumber == "15") return "v141";
		if (versionNumber == "14") return "v140";
	}
	bool CreateFontFileDataPackageFromAsset(const String& assetPath, const String& packagePath)
	{
		// Load Font From Assets
		Ref<FontFile> loadedFont = ResourceLoader::get_singleton()->load(assetPath);
		if (!loadedFont.is_valid()) return false;

		// Write Package On Disk
		Ref<FileAccess> handle = FileAccess::open(packagePath, FileAccess::ModeFlags::WRITE);
		if (handle.is_valid())
		{
			handle->store_buffer(loadedFont->get_data());
			handle->close();
			return true;
		}
		return false;
	}
	String CreateSecuredBase64StringFromString(const String& srcStr)
	{
		// Feature is Removed Due to Proprietary Code
		return srcStr;
	}
	String RetriveStringFromSecuredBase64String(const String& securedStr)
	{
		// Feature is Removed Due to Proprietary Code
		return securedStr;
	}
	HWND GetWindowNativeHandle(const Window* targetWindow)
	{
		return HWND(DisplayServer::get_singleton()->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE, targetWindow->get_window_id()));
	}
	HWND GetMainWindowNativeHandle()
	{
		return HWND(DisplayServer::get_singleton()->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE));
	}
	bool AssignPopUpWindow(const Window* targetWindow)
	{
		HWND windowHandle = jenova::GetWindowNativeHandle(targetWindow);
		if (windowHandle)
		{
			LONG style = GetWindowLong(windowHandle, GWL_STYLE);
			style &= ~WS_MINIMIZEBOX;
			SetWindowLong(windowHandle, GWL_STYLE, style);
			SetWindowLongPtr(windowHandle, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(jenova::GetMainWindowNativeHandle()));
			return true;
		}
		return false;
	}
	bool ReleasePopUpWindow(const Window* targetWindow)
	{
		HWND windowHandle = jenova::GetWindowNativeHandle(targetWindow);
		if (windowHandle)
		{
			SetWindowLongPtr(windowHandle, GWLP_HWNDPARENT, NULL);
			return true;
		}
		return false;
	}
	String FormatBytesSize(size_t byteSize)
	{
		const size_t sizeBase = 1000;
		if (byteSize == 0)
		{
			return "Empty";
		}
		else if (byteSize < sizeBase)
		{
			return String::num_int64(byteSize) + " B";
		}
		else if (byteSize < sizeBase * sizeBase)
		{
			double sizeKB = static_cast<double>(byteSize) / sizeBase;
			return sizeKB == floor(sizeKB) ? vformat("%.1f KB", sizeKB) : vformat("%.3f KB", sizeKB);
		}
		else if (byteSize < sizeBase * sizeBase * sizeBase)
		{
			double sizeMB = static_cast<double>(byteSize) / (sizeBase * sizeBase);
			return sizeMB == floor(sizeMB) ? vformat("%.1f MB", sizeMB) : vformat("%.3f MB", sizeMB);
		}
		else if (byteSize < 2LL * sizeBase * sizeBase * sizeBase)
		{
			double sizeGB = static_cast<double>(byteSize) / (sizeBase * sizeBase * sizeBase);
			return sizeGB == floor(sizeGB) ? vformat("%.1f GB", sizeGB) : vformat("%.3f GB", sizeGB);
		}
		else
		{
			return "Huge";
		}
	}
	String GenerateMD5HashFromFile(const String& targetFile)
	{
		// Open the file
		Ref<FileAccess> file = FileAccess::open(targetFile, FileAccess::ModeFlags::READ);
		if (!file.is_valid()) return "00000000000000000000000000000000";
		PackedByteArray file_data = file->get_buffer(file->get_length());
		Ref<HashingContext> md5_ctx;
		md5_ctx.instantiate();
		md5_ctx->start(HashingContext::HASH_MD5);
		md5_ctx->update(file_data);
		PackedByteArray md5_hash = md5_ctx->finish();
		String md5_hex = md5_hash.hex_encode();
		md5_ctx.unref();
		file_data.clear();
		file->close();
		return md5_hex;
	}
	jenova::PackageList GetInstalledCompilerPackages(const jenova::CompilerModel& compilerModel)
	{
		// Collect Compiler Packages
		auto compilerPackages = JenovaPackageManager::get_singleton()->GetInstalledPackages(jenova::PackageType::Compiler);
		jenova::PackageList filteredCompilerPackages;
		for (const auto& compilerPackage : compilerPackages)
		{
			if (compilerModel == jenova::CompilerModel::MicrosoftCompiler && compilerPackage.pkgDestination.contains("JenovaMSVCCompiler"))
			{
				filteredCompilerPackages.push_back(compilerPackage);
			}
		}

		// Sort Package Collections
		std::sort(filteredCompilerPackages.begin(), filteredCompilerPackages.end(),
			[](const jenova::JenovaPackage& a, const jenova::JenovaPackage& b) { return a.pkgDestination < b.pkgDestination; });

		// Return Package List
		return filteredCompilerPackages;
	}
	jenova::PackageList GetInstalledGodotKitPackages()
	{
		// Collect GodotKit Packages
		auto godotKitPackages = JenovaPackageManager::get_singleton()->GetInstalledPackages(jenova::PackageType::GodotKit);

		// Sort Package Collections
		std::sort(godotKitPackages.begin(), godotKitPackages.end(),
			[](const jenova::JenovaPackage& a, const jenova::JenovaPackage& b) { return a.pkgDestination < b.pkgDestination; });

		// Return Package List
		return godotKitPackages;
	}
	String GetInstalledCompilerPathFromPackages(const String& compilerIdentity, const jenova::CompilerModel& compilerModel)
	{
		auto compilerPackages = jenova::GetInstalledCompilerPackages(compilerModel);
		if (compilerPackages.size() > 0)
		{
			if (compilerIdentity.to_lower() == "latest") return compilerPackages.back().pkgDestination;
			for (const auto& compilerPackage : compilerPackages)
			{
				if (compilerPackage.pkgDestination.get_file() == compilerIdentity) return compilerPackage.pkgDestination;
			}
		}
		return "Missing-Compiler-1.0.0";
	}
	String GetInstalledGodotKitPathFromPackages(const String& godotKitIdentity)
	{
		auto godotKitPackages = jenova::GetInstalledGodotKitPackages();
		if (godotKitPackages.size() > 0)
		{
			if (godotKitIdentity.to_lower() == "latest") return godotKitPackages.back().pkgDestination;
			for (const auto& godotKitPackage : godotKitPackages)
			{
				if (godotKitPackage.pkgDestination.get_file() == godotKitIdentity) return godotKitPackage.pkgDestination;
			}
		}
		return "Missing-GodotKit-1.0.0";
	}
	std::string ResolveVariantValueAsString(const Variant* variantValue, jenova::PointerList& ptrList)
	{
		// Atomic types
		if (variantValue->get_type() == Variant::BOOL) return (bool(*variantValue)) ? "true" : "false";
		if (variantValue->get_type() == Variant::FLOAT) return jenova::Format("%lf", double(*variantValue));
		if (variantValue->get_type() == Variant::INT) return jenova::Format("%lld", int64_t(*variantValue));
		if (variantValue->get_type() == Variant::STRING)
		{
			String* value = new String(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}

		// Math types
		if (variantValue->get_type() == Variant::VECTOR2)
		{
			Vector2* value = new Vector2(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::VECTOR2I)
		{
			Vector2i* value = new Vector2i(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::VECTOR3)
		{
			Vector3* value = new Vector3(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::VECTOR3I)
		{
			Vector3i* value = new Vector3i(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::RECT2)
		{
			Rect2* value = new Rect2(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::RECT2I)
		{
			Rect2i* value = new Rect2i(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::VECTOR4)
		{
			Vector4* value = new Vector4(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::VECTOR4I)
		{
			Vector4i* value = new Vector4i(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::TRANSFORM2D)
		{
			Transform2D* value = new Transform2D(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PLANE)
		{
			Plane* value = new Plane(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::QUATERNION)
		{
			Quaternion* value = new Quaternion(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::AABB)
		{
			AABB* value = new AABB(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::BASIS)
		{
			Basis* value = new Basis(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::TRANSFORM3D)
		{
			Transform3D* value = new Transform3D(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PROJECTION)
		{
			Projection* value = new Projection(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}

		// Misc types
		if (variantValue->get_type() == Variant::COLOR)
		{
			Color* value = new Color(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::STRING_NAME)
		{
			StringName* value = new StringName(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::NODE_PATH)
		{
			NodePath* value = new NodePath(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::RID)
		{
			RID* value = new RID(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::OBJECT)
		{
			return jenova::Format("(void*)0x%llx", (void*)variantValue);
		}
		if (variantValue->get_type() == Variant::CALLABLE)
		{
			Callable* value = new Callable(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::SIGNAL)
		{
			Signal* value = new Signal(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::DICTIONARY)
		{
			Dictionary* value = new Dictionary(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::ARRAY)
		{
			Array* value = new Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}

		// Typed arrays
		if (variantValue->get_type() == Variant::PACKED_BYTE_ARRAY)
		{
			PackedByteArray* value = new PackedByteArray(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_INT32_ARRAY)
		{
			PackedInt32Array* value = new PackedInt32Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_INT64_ARRAY)
		{
			PackedInt64Array* value = new PackedInt64Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_FLOAT32_ARRAY)
		{
			PackedFloat32Array* value = new PackedFloat32Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_FLOAT64_ARRAY)
		{
			PackedFloat64Array* value = new PackedFloat64Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_STRING_ARRAY)
		{
			PackedStringArray* value = new PackedStringArray(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_VECTOR2_ARRAY)
		{
			PackedVector2Array* value = new PackedVector2Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_VECTOR3_ARRAY)
		{
			PackedVector3Array* value = new PackedVector3Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_VECTOR4_ARRAY)
		{
			PackedVector4Array* value = new PackedVector4Array(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}
		if (variantValue->get_type() == Variant::PACKED_COLOR_ARRAY)
		{
			PackedColorArray* value = new PackedColorArray(*variantValue);
			ptrList.push_back(value);
			return jenova::Format("(void*)0x%llx", value);
		}

		// Default Case
		return jenova::Format("(void*)0x%llx", (void*)variantValue);
	}
	std::string ResolveVariantTypeAsString(const Variant* variantValue)
	{
		if (variantValue->get_type() == Variant::BOOL) return "bool";
		if (variantValue->get_type() == Variant::FLOAT) return "double";
		if (variantValue->get_type() == Variant::INT) return "long long int";
		return "void*";
	}
	std::string ResolveReturnTypeForJIT(const std::string& returnType)
	{
		// No Return
		if (returnType == "void") return returnType;

		// Basic Types
		if (returnType == "bool" || returnType == "float" || 
			returnType == "int" || returnType == "double") 
			return returnType;

		// Other Types
		return "Variant";
	}
	Variant* MakeVariantFromReturnType(Variant* variantObject, const char* returnType)
	{
		// Validate Return Type
		if (returnType == 0) return new Variant(Variant::NIL);

		// Atomic types
		if (strcmp(returnType, "bool") == 0)
		{
			bool value = *(bool*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "int") == 0)
		{
			int64_t value = *(int64_t*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "float") == 0)
		{
			float value = *(float*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "double") == 0)
		{
			double value = *(double*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::String") == 0)
		{
			String value = *(String*)variantObject;
			return new Variant(value);
		}

		// Math types
		if (strcmp(returnType, "godot::Vector2") == 0)
		{
			Vector2 value = *(Vector2*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Vector2i") == 0)
		{
			Vector2i value = *(Vector2i*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Rect2") == 0)
		{
			Rect2 value = *(Rect2*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Rect2i") == 0)
		{
			Rect2i value = *(Rect2i*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Vector3") == 0)
		{
			Vector3 value = *(Vector3*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Vector3i") == 0)
		{
			Vector3i value = *(Vector3i*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Transform2D") == 0)
		{
			Transform2D value = *(Transform2D*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Vector4") == 0)
		{
			Vector4 value = *(Vector4*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Vector4i") == 0)
		{
			Vector4i value = *(Vector4i*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Plane") == 0)
		{
			Plane value = *(Plane*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Quaternion") == 0)
		{
			Quaternion value = *(Quaternion*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::AABB") == 0)
		{
			AABB value = *(AABB*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Basis") == 0)
		{
			Basis value = *(Basis*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Transform3D") == 0)
		{
			Transform3D value = *(Transform3D*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Projection") == 0)
		{
			Projection value = *(Projection*)variantObject;
			return new Variant(value);
		}

		// Misc types
		if (strcmp(returnType, "godot::Color") == 0)
		{
			Color value = *(Color*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::StringName") == 0)
		{
			StringName value = *(StringName*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::NodePath") == 0)
		{
			NodePath value = *(NodePath*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::RID") == 0)
		{
			RID value = *(RID*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Object") == 0)
		{
			Object* value = *(Object**)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Callable") == 0)
		{
			Callable value = *(Callable*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Signal") == 0)
		{
			Signal value = *(Signal*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Dictionary") == 0)
		{
			Dictionary value = *(Dictionary*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::Array") == 0)
		{
			Array value = *(Array*)variantObject;
			return new Variant(value);
		}

		// Typed arrays
		if (strcmp(returnType, "godot::PackedByteArray") == 0)
		{
			PackedByteArray value = *(PackedByteArray*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedInt32Array") == 0)
		{
			PackedInt32Array value = *(PackedInt32Array*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedInt64Array") == 0)
		{
			PackedInt64Array value = *(PackedInt64Array*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedFloat32Array") == 0)
		{
			PackedFloat32Array value = *(PackedFloat32Array*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedFloat64Array") == 0)
		{
			PackedFloat64Array value = *(PackedFloat64Array*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedStringArray") == 0)
		{
			PackedStringArray value = *(PackedStringArray*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedVector2Array") == 0)
		{
			PackedVector2Array value = *(PackedVector2Array*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedVector3Array") == 0)
		{
			PackedVector3Array value = *(PackedVector3Array*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedColorArray") == 0)
		{
			PackedColorArray value = *(PackedColorArray*)variantObject;
			return new Variant(value);
		}
		if (strcmp(returnType, "godot::PackedVector4Array") == 0)
		{
			PackedVector4Array value = *(PackedVector4Array*)variantObject;
			return new Variant(value);
		}

		// Variant
		if (strcmp(returnType, "godot::Variant") == 0)
		{
			return new Variant(variantObject);
		}

		// Default case (unsupported type)
		return new Variant(*variantObject);
	}
	uint32_t GetPropertyEnumFlagFromString(const std::string enumFlagStr)
	{
		// Check if the input is an integer
		if (std::all_of(enumFlagStr.begin(), enumFlagStr.end(), ::isdigit)) return static_cast<uint32_t>(std::stoi(enumFlagStr));

		// Property Hint Flags
		if (enumFlagStr == "PROPERTY_HINT_NONE") return PROPERTY_HINT_NONE;
		else if (enumFlagStr == "PROPERTY_HINT_RANGE") return PROPERTY_HINT_RANGE;
		else if (enumFlagStr == "PROPERTY_HINT_ENUM") return PROPERTY_HINT_ENUM;
		else if (enumFlagStr == "PROPERTY_HINT_ENUM_SUGGESTION") return PROPERTY_HINT_ENUM_SUGGESTION;
		else if (enumFlagStr == "PROPERTY_HINT_EXP_EASING") return PROPERTY_HINT_EXP_EASING;
		else if (enumFlagStr == "PROPERTY_HINT_LINK") return PROPERTY_HINT_LINK;
		else if (enumFlagStr == "PROPERTY_HINT_FLAGS") return PROPERTY_HINT_FLAGS;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_2D_RENDER") return PROPERTY_HINT_LAYERS_2D_RENDER;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_2D_PHYSICS") return PROPERTY_HINT_LAYERS_2D_PHYSICS;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_2D_NAVIGATION") return PROPERTY_HINT_LAYERS_2D_NAVIGATION;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_3D_RENDER") return PROPERTY_HINT_LAYERS_3D_RENDER;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_3D_PHYSICS") return PROPERTY_HINT_LAYERS_3D_PHYSICS;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_3D_NAVIGATION") return PROPERTY_HINT_LAYERS_3D_NAVIGATION;
		else if (enumFlagStr == "PROPERTY_HINT_LAYERS_AVOIDANCE") return PROPERTY_HINT_LAYERS_AVOIDANCE;
		else if (enumFlagStr == "PROPERTY_HINT_FILE") return PROPERTY_HINT_FILE;
		else if (enumFlagStr == "PROPERTY_HINT_DIR") return PROPERTY_HINT_DIR;
		else if (enumFlagStr == "PROPERTY_HINT_GLOBAL_FILE") return PROPERTY_HINT_GLOBAL_FILE;
		else if (enumFlagStr == "PROPERTY_HINT_GLOBAL_DIR") return PROPERTY_HINT_GLOBAL_DIR;
		else if (enumFlagStr == "PROPERTY_HINT_RESOURCE_TYPE") return PROPERTY_HINT_RESOURCE_TYPE;
		else if (enumFlagStr == "PROPERTY_HINT_MULTILINE_TEXT") return PROPERTY_HINT_MULTILINE_TEXT;
		else if (enumFlagStr == "PROPERTY_HINT_EXPRESSION") return PROPERTY_HINT_EXPRESSION;
		else if (enumFlagStr == "PROPERTY_HINT_PLACEHOLDER_TEXT") return PROPERTY_HINT_PLACEHOLDER_TEXT;
		else if (enumFlagStr == "PROPERTY_HINT_COLOR_NO_ALPHA") return PROPERTY_HINT_COLOR_NO_ALPHA;
		else if (enumFlagStr == "PROPERTY_HINT_OBJECT_ID") return PROPERTY_HINT_OBJECT_ID;
		else if (enumFlagStr == "PROPERTY_HINT_TYPE_STRING") return PROPERTY_HINT_TYPE_STRING;
		else if (enumFlagStr == "PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE") return PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE;
		else if (enumFlagStr == "PROPERTY_HINT_OBJECT_TOO_BIG") return PROPERTY_HINT_OBJECT_TOO_BIG;
		else if (enumFlagStr == "PROPERTY_HINT_NODE_PATH_VALID_TYPES") return PROPERTY_HINT_NODE_PATH_VALID_TYPES;
		else if (enumFlagStr == "PROPERTY_HINT_SAVE_FILE") return PROPERTY_HINT_SAVE_FILE;
		else if (enumFlagStr == "PROPERTY_HINT_GLOBAL_SAVE_FILE") return PROPERTY_HINT_GLOBAL_SAVE_FILE;
		else if (enumFlagStr == "PROPERTY_HINT_INT_IS_OBJECTID") return PROPERTY_HINT_INT_IS_OBJECTID;
		else if (enumFlagStr == "PROPERTY_HINT_INT_IS_POINTER") return PROPERTY_HINT_INT_IS_POINTER;
		else if (enumFlagStr == "PROPERTY_HINT_ARRAY_TYPE") return PROPERTY_HINT_ARRAY_TYPE;
		else if (enumFlagStr == "PROPERTY_HINT_LOCALE_ID") return PROPERTY_HINT_LOCALE_ID;
		else if (enumFlagStr == "PROPERTY_HINT_LOCALIZABLE_STRING") return PROPERTY_HINT_LOCALIZABLE_STRING;
		else if (enumFlagStr == "PROPERTY_HINT_NODE_TYPE") return PROPERTY_HINT_NODE_TYPE;
		else if (enumFlagStr == "PROPERTY_HINT_HIDE_QUATERNION_EDIT") return PROPERTY_HINT_HIDE_QUATERNION_EDIT;
		else if (enumFlagStr == "PROPERTY_HINT_PASSWORD") return PROPERTY_HINT_PASSWORD;
		else if (enumFlagStr == "PROPERTY_HINT_MAX") return PROPERTY_HINT_MAX;

		// Property Usage Flags
		if (enumFlagStr == "PROPERTY_USAGE_NONE") return PROPERTY_USAGE_NONE;
		else if (enumFlagStr == "PROPERTY_USAGE_STORAGE") return PROPERTY_USAGE_STORAGE;
		else if (enumFlagStr == "PROPERTY_USAGE_EDITOR") return PROPERTY_USAGE_EDITOR;
		else if (enumFlagStr == "PROPERTY_USAGE_INTERNAL") return PROPERTY_USAGE_INTERNAL;
		else if (enumFlagStr == "PROPERTY_USAGE_CHECKABLE") return PROPERTY_USAGE_CHECKABLE;
		else if (enumFlagStr == "PROPERTY_USAGE_CHECKED") return PROPERTY_USAGE_CHECKED;
		else if (enumFlagStr == "PROPERTY_USAGE_GROUP") return PROPERTY_USAGE_GROUP;
		else if (enumFlagStr == "PROPERTY_USAGE_CATEGORY") return PROPERTY_USAGE_CATEGORY;
		else if (enumFlagStr == "PROPERTY_USAGE_SUBGROUP") return PROPERTY_USAGE_SUBGROUP;
		else if (enumFlagStr == "PROPERTY_USAGE_CLASS_IS_BITFIELD") return PROPERTY_USAGE_CLASS_IS_BITFIELD;
		else if (enumFlagStr == "PROPERTY_USAGE_NO_INSTANCE_STATE") return PROPERTY_USAGE_NO_INSTANCE_STATE;
		else if (enumFlagStr == "PROPERTY_USAGE_RESTART_IF_CHANGED") return PROPERTY_USAGE_RESTART_IF_CHANGED;
		else if (enumFlagStr == "PROPERTY_USAGE_SCRIPT_VARIABLE") return PROPERTY_USAGE_SCRIPT_VARIABLE;
		else if (enumFlagStr == "PROPERTY_USAGE_STORE_IF_NULL") return PROPERTY_USAGE_STORE_IF_NULL;
		else if (enumFlagStr == "PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED") return PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED;
		else if (enumFlagStr == "PROPERTY_USAGE_SCRIPT_DEFAULT_VALUE") return PROPERTY_USAGE_SCRIPT_DEFAULT_VALUE;
		else if (enumFlagStr == "PROPERTY_USAGE_CLASS_IS_ENUM") return PROPERTY_USAGE_CLASS_IS_ENUM;
		else if (enumFlagStr == "PROPERTY_USAGE_NIL_IS_VARIANT") return PROPERTY_USAGE_NIL_IS_VARIANT;
		else if (enumFlagStr == "PROPERTY_USAGE_ARRAY") return PROPERTY_USAGE_ARRAY;
		else if (enumFlagStr == "PROPERTY_USAGE_ALWAYS_DUPLICATE") return PROPERTY_USAGE_ALWAYS_DUPLICATE;
		else if (enumFlagStr == "PROPERTY_USAGE_NEVER_DUPLICATE") return PROPERTY_USAGE_NEVER_DUPLICATE;
		else if (enumFlagStr == "PROPERTY_USAGE_HIGH_END_GFX") return PROPERTY_USAGE_HIGH_END_GFX;
		else if (enumFlagStr == "PROPERTY_USAGE_NODE_PATH_FROM_SCENE_ROOT") return PROPERTY_USAGE_NODE_PATH_FROM_SCENE_ROOT;
		else if (enumFlagStr == "PROPERTY_USAGE_RESOURCE_NOT_PERSISTENT") return PROPERTY_USAGE_RESOURCE_NOT_PERSISTENT;
		else if (enumFlagStr == "PROPERTY_USAGE_KEYING_INCREMENTS") return PROPERTY_USAGE_KEYING_INCREMENTS;
		else if (enumFlagStr == "PROPERTY_USAGE_DEFERRED_SET_RESOURCE") return PROPERTY_USAGE_DEFERRED_SET_RESOURCE;
		else if (enumFlagStr == "PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT") return PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT;
		else if (enumFlagStr == "PROPERTY_USAGE_EDITOR_BASIC_SETTING") return PROPERTY_USAGE_EDITOR_BASIC_SETTING;
		else if (enumFlagStr == "PROPERTY_USAGE_READ_ONLY") return PROPERTY_USAGE_READ_ONLY;
		else if (enumFlagStr == "PROPERTY_USAGE_SECRET") return PROPERTY_USAGE_SECRET;
		else if (enumFlagStr == "PROPERTY_USAGE_DEFAULT") return PROPERTY_USAGE_DEFAULT;
		else if (enumFlagStr == "PROPERTY_USAGE_NO_EDITOR") return PROPERTY_USAGE_NO_EDITOR;

		// Invalid/Unsupported
		return 0;
	}
	jenova::SerializedData ProcessAndExtractPropertiesFromScript(OUT std::string& scriptSource, const std::string& scriptUID)
	{
		// Property Metadata Serializer
		nlohmann::json propertiesMetadata;

		// Helpers
		auto isCommented = [](const std::string& line)
		{
			std::regex commentRegex(R"(^\s*(//|/\*|\*/))");
			return std::regex_search(line, commentRegex);
		};

		// Function to parse arguments with key-value pairs
		auto parseArguments = [](const std::string& argsString)
		{
			std::vector<std::string> args;
			std::string currentArg;
			int parenDepth = 0;
			bool inQuotes = false;
			for (size_t i = 0; i < argsString.size(); ++i)
			{
				char c = argsString[i];
				if (c == '"') inQuotes = !inQuotes;
				else if (c == '(' && !inQuotes) ++parenDepth;
				else if (c == ')' && !inQuotes && parenDepth > 0) --parenDepth;
				if (c == ',' && parenDepth == 0 && !inQuotes)
				{
					// Trim whitespace around currentArg
					currentArg.erase(currentArg.find_last_not_of(" \t\n\r\f\v") + 1);
					currentArg.erase(0, currentArg.find_first_not_of(" \t\n\r\f\v"));
					args.push_back(currentArg);
					currentArg.clear();
				}
				else
				{
					currentArg += c;
				}
			}
			if (!currentArg.empty())
			{
				currentArg.erase(currentArg.find_last_not_of(" \t\n\r\f\v") + 1);
				currentArg.erase(0, currentArg.find_first_not_of(" \t\n\r\f\v"));
				args.push_back(currentArg);
			}
			return args;
		};

		// Helper to parse key-value pairs
		auto parseKeyValuePairs = [](const std::vector<std::string>& args, size_t startIndex)
		{
			std::unordered_map<std::string, std::string> keyValuePairs;
			std::regex kvRegex(R"(\s*([\w]+)\s*:\s*(.*))");

			for (size_t i = startIndex; i < args.size(); ++i)
			{
				std::smatch match;
				if (std::regex_match(args[i], match, kvRegex))
				{
					std::string key = match[1].str();
					std::string value = match[2].str();
					// Remove surrounding quotes if present
					if (!value.empty() && value.front() == '"' && value.back() == '"')
					{
						value = value.substr(1, value.size() - 2);
					}
					keyValuePairs[key] = value;
				}
			}
			return keyValuePairs;
		};

		// Split scriptSource into lines for processing
		std::istringstream scriptStream(scriptSource);
		std::string line, paramHandlers;
		int lineNumber = 0;
		int paramCount = 0;

		// Flags
		bool isHeaderCommentAdded = false;

		// Extract properties and replace lines
		while (std::getline(scriptStream, line))
		{
			lineNumber++;

			// Skip if line is commented
			if (isCommented(line)) continue;

			// Detect JENOVA_PROPERTY line
			std::regex propertyRegex(R"(JENOVA_PROPERTY\s*\((.*)\))");
			std::smatch match;
			if (std::regex_search(line, match, propertyRegex))
			{
				paramCount++;

				// Parse arguments
				std::vector<std::string> args = parseArguments(match[1].str());
				if (args.size() >= 3)
				{
					// Set Property Data
					nlohmann::json propertyMetadata;
					propertyMetadata["PropertyName"] = args[1];
					propertyMetadata["PropertyType"] = args[0];
					propertyMetadata["PropertyDefault"] = args[2];

					// Parse additional key-value parameters and add them to the metadata
					auto extraParams = parseKeyValuePairs(args, 3);
					for (const auto& kv : extraParams)
					{
						if (kv.first == "Group")
						{
							propertyMetadata["PropertyGroup"] = kv.second;
							continue;
						}
						if (kv.first == "Hint")
						{
							propertyMetadata["PropertyHint"] = kv.second;
							continue;
						}
						if (kv.first == "HintString")
						{
							propertyMetadata["PropertyHintString"] = kv.second;
							continue;
						}
						if (kv.first == "ClassName")
						{
							propertyMetadata["PropertyClassName"] = kv.second;
							continue;
						}
						if (kv.first == "Usage")
						{
							propertyMetadata["PropertyUsage"] = kv.second;
							continue;
						}
						propertyMetadata[kv.first] = kv.second;
					}

					propertiesMetadata.push_back(propertyMetadata);

					// Verbose
					jenova::VerboseByID(__LINE__, "Property Extracted >> Name : [%s] Type : [%s]  Default Value :[%s]", args[0].c_str(), args[1].c_str(), args[2].c_str());
				}

				// Add Header Comment
				if (!isHeaderCommentAdded)
				{
					paramHandlers += "// Script Properties Handlers\n";
					isHeaderCommentAdded = true;
				}

				// Generate Properties & Handlers
				size_t lineStartPos = scriptSource.find(line);
				if (lineStartPos != std::string::npos)
				{
					scriptSource.replace(lineStartPos, line.length(), jenova::Format("%s* __prop_%s = nullptr;", args[0].c_str(), args[1].c_str()));
					paramHandlers += jenova::Format("#define %s *__prop_%s\n", args[1].c_str(), args[1].c_str());
				}
			}
		}

		// Add Handlers to Source
		if (paramCount != 0)
		{
			paramHandlers += "\n";
			scriptSource.insert(0, paramHandlers);
		}

		// Return Metadata
		return propertiesMetadata.dump();
	}
	jenova::SerializedData ProcessAndExtractPropertiesFromScript(OUT String& scriptSource, const String& scriptUID)
	{
		std::string sourceStdStr = AS_STD_STRING(scriptSource);
		jenova::SerializedData propertiesMetadata = ProcessAndExtractPropertiesFromScript(sourceStdStr, AS_STD_STRING(scriptUID));
		scriptSource = String(sourceStdStr.c_str());
		return propertiesMetadata;
	}
	Variant::Type GetVariantTypeFromStdString(const std::string& typeName)
	{
		// Remove Reference & Pointer Symbols & Namespace
		std::string typeNameCleaned(typeName);
		jenova::ReplaceAllMatchesWithString(typeNameCleaned, "*", "");
		jenova::ReplaceAllMatchesWithString(typeNameCleaned, "&", "");
		jenova::ReplaceAllMatchesWithString(typeNameCleaned, "godot::", "");

		// Atomic types
		if (typeNameCleaned == "bool") return Variant::Type::BOOL;
		if (typeNameCleaned == "int" || typeNameCleaned == "int32_t" || typeNameCleaned == "int64_t") return Variant::Type::INT;
		if (typeNameCleaned == "float" || typeNameCleaned == "double") return Variant::Type::FLOAT;
		if (typeNameCleaned == "String") return Variant::Type::STRING;

		// Math types
		if (typeNameCleaned == "Vector2") return Variant::Type::VECTOR2;
		if (typeNameCleaned == "Vector2i") return Variant::Type::VECTOR2I;
		if (typeNameCleaned == "Rect2") return Variant::Type::RECT2;
		if (typeNameCleaned == "Rect2i") return Variant::Type::RECT2I;
		if (typeNameCleaned == "Vector3") return Variant::Type::VECTOR3;
		if (typeNameCleaned == "Vector3i") return Variant::Type::VECTOR3I;
		if (typeNameCleaned == "Transform2D") return Variant::Type::TRANSFORM2D;
		if (typeNameCleaned == "Vector4") return Variant::Type::VECTOR4;
		if (typeNameCleaned == "Vector4i") return Variant::Type::VECTOR4I;
		if (typeNameCleaned == "Plane") return Variant::Type::PLANE;
		if (typeNameCleaned == "Quaternion") return Variant::Type::QUATERNION;
		if (typeNameCleaned == "AABB") return Variant::Type::AABB;
		if (typeNameCleaned == "Basis") return Variant::Type::BASIS;
		if (typeNameCleaned == "Transform3D") return Variant::Type::TRANSFORM3D;
		if (typeNameCleaned == "Projection") return Variant::Type::PROJECTION;

		// Misc types
		if (typeNameCleaned == "Color") return Variant::Type::COLOR;
		if (typeNameCleaned == "StringName") return Variant::Type::STRING_NAME;
		if (typeNameCleaned == "NodePath") return Variant::Type::NODE_PATH;
		if (typeNameCleaned == "RID") return Variant::Type::RID;
		if (typeNameCleaned == "Object") return Variant::Type::OBJECT;
		if (typeNameCleaned == "Callable") return Variant::Type::CALLABLE;
		if (typeNameCleaned == "Signal") return Variant::Type::SIGNAL;
		if (typeNameCleaned == "Dictionary") return Variant::Type::DICTIONARY;
		if (typeNameCleaned == "Array") return Variant::Type::ARRAY;

		// Typed arrays
		if (typeNameCleaned == "PackedByteArray") return Variant::Type::PACKED_BYTE_ARRAY;
		if (typeNameCleaned == "PackedInt32Array") return Variant::Type::PACKED_INT32_ARRAY;
		if (typeNameCleaned == "PackedInt64Array") return Variant::Type::PACKED_INT64_ARRAY;
		if (typeNameCleaned == "PackedFloat32Array") return Variant::Type::PACKED_FLOAT32_ARRAY;
		if (typeNameCleaned == "PackedFloat64Array") return Variant::Type::PACKED_FLOAT64_ARRAY;
		if (typeNameCleaned == "PackedStringArray") return Variant::Type::PACKED_STRING_ARRAY;
		if (typeNameCleaned == "PackedVector2Array") return Variant::Type::PACKED_VECTOR2_ARRAY;
		if (typeNameCleaned == "PackedVector3Array") return Variant::Type::PACKED_VECTOR3_ARRAY;
		if (typeNameCleaned == "PackedColorArray") return Variant::Type::PACKED_COLOR_ARRAY;
		if (typeNameCleaned == "PackedVector4Array") return Variant::Type::PACKED_VECTOR4_ARRAY;

		// Default Case
		return Variant::Type::OBJECT;
	}
	jenova::ScriptPropertyContainer CreatePropertyContainerFromMetadata(const jenova::SerializedData& propertyMetadata, const std::string& scriptUID)
	{
		try
		{
			// Create Property Metadata Parser
			nlohmann::json propertyMetadataParser = nlohmann::json::parse(propertyMetadata);

			// Create Property Container
			jenova::ScriptPropertyContainer propertyContainer;
			propertyContainer.scriptUID = String(scriptUID.c_str());

			// Create Script Properties
			for (const auto& scriptProperty : propertyMetadataParser)
			{
				jenova::ScriptProperty scriptProp;
				scriptProp.ownerScriptUID = String(scriptUID.c_str());
				scriptProp.propertyName = String(scriptProperty["PropertyName"].get<std::string>().c_str());
				scriptProp.propertyInfo.type = jenova::GetVariantTypeFromStdString(scriptProperty["PropertyType"].get<std::string>());
				scriptProp.defaultValue = UtilityFunctions::str_to_var(String(scriptProperty["PropertyDefault"].get<std::string>().c_str()));
				scriptProp.propertyInfo.name = scriptProperty.contains("PropertyGroup") ? 
					String(scriptProperty["PropertyGroup"].get<std::string>().c_str()) + "/" + String(scriptProperty["PropertyName"].get<std::string>().c_str()) :
					StringName(scriptProperty["PropertyName"].get<std::string>().c_str());
				if (scriptProperty.contains("PropertyHint")) scriptProp.propertyInfo.hint = jenova::GetPropertyEnumFlagFromString(scriptProperty["PropertyHint"].get<std::string>());
				else scriptProp.propertyInfo.hint = godot::PropertyHint::PROPERTY_HINT_NONE;
				if (scriptProperty.contains("PropertyHintString")) scriptProp.propertyInfo.hint_string = String(scriptProperty["PropertyHintString"].get<std::string>().c_str());
				else scriptProp.propertyInfo.hint_string = String("");
				if (scriptProperty.contains("PropertyClassName")) scriptProp.propertyInfo.class_name = StringName(scriptProperty["PropertyClassName"].get<std::string>().c_str());
				else scriptProp.propertyInfo.class_name = StringName("Variant");
				if (scriptProperty.contains("PropertyUsage")) scriptProp.propertyInfo.usage = jenova::GetPropertyEnumFlagFromString(scriptProperty["PropertyUsage"].get<std::string>());
				else scriptProp.propertyInfo.usage = PropertyUsageFlags::PROPERTY_USAGE_DEFAULT | PropertyUsageFlags::PROPERTY_USAGE_SCRIPT_VARIABLE;
				propertyContainer.scriptProperties.push_back(scriptProp);
			}

			// Return Container
			return propertyContainer;
		}
		catch (const std::exception&)
		{
			return jenova::ScriptPropertyContainer();
		}
	}
	void* AllocateVariantBasedProperty(const std::string& typeName)
	{
		// Remove Reference & Pointer Symbols & Namespace
		std::string typeNameCleaned(typeName);
		jenova::ReplaceAllMatchesWithString(typeNameCleaned, "*", "");
		jenova::ReplaceAllMatchesWithString(typeNameCleaned, "&", "");
		jenova::ReplaceAllMatchesWithString(typeNameCleaned, "godot::", "");

		// Atomic types
		if (typeNameCleaned == "bool") return new bool();
		if (typeNameCleaned == "int" || typeNameCleaned == "int32_t" || typeNameCleaned == "int64_t") return new int64_t();
		if (typeNameCleaned == "double" || typeNameCleaned == "float") return new double();
		if (typeNameCleaned == "String") return new godot::String();

		// Math types
		if (typeNameCleaned == "Vector2") return new godot::Vector2();
		if (typeNameCleaned == "Vector2i") return new godot::Vector2i();
		if (typeNameCleaned == "Rect2") return new godot::Rect2();
		if (typeNameCleaned == "Rect2i") return new godot::Rect2i();
		if (typeNameCleaned == "Vector3") return new godot::Vector3();
		if (typeNameCleaned == "Vector3i") return new godot::Vector3i();
		if (typeNameCleaned == "Transform2D") return new godot::Transform2D();
		if (typeNameCleaned == "Vector4") return new godot::Vector4();
		if (typeNameCleaned == "Vector4i") return new godot::Vector4i();
		if (typeNameCleaned == "Plane") return new godot::Plane();
		if (typeNameCleaned == "Quaternion") return new godot::Quaternion();
		if (typeNameCleaned == "AABB") return new godot::AABB();
		if (typeNameCleaned == "Basis") return new godot::Basis();
		if (typeNameCleaned == "Transform3D") return new godot::Transform3D();
		if (typeNameCleaned == "Projection") return new godot::Projection();

		// Misc types
		if (typeNameCleaned == "Color") return new godot::Color();
		if (typeNameCleaned == "StringName") return new godot::StringName();
		if (typeNameCleaned == "NodePath") return new godot::NodePath();
		if (typeNameCleaned == "RID") return new godot::RID();
		if (typeNameCleaned == "Object") return new godot::Object * ();
		if (typeNameCleaned == "Callable") return new godot::Callable();
		if (typeNameCleaned == "Signal") return new godot::Signal();
		if (typeNameCleaned == "Dictionary") return new godot::Dictionary();
		if (typeNameCleaned == "Array") return new godot::Array();

		// Typed arrays
		if (typeNameCleaned == "PackedByteArray") return new godot::PackedByteArray();
		if (typeNameCleaned == "PackedInt32Array") return new godot::PackedInt32Array();
		if (typeNameCleaned == "PackedInt64Array") return new godot::PackedInt64Array();
		if (typeNameCleaned == "PackedFloat32Array") return new godot::PackedFloat32Array();
		if (typeNameCleaned == "PackedFloat64Array") return new godot::PackedFloat64Array();
		if (typeNameCleaned == "PackedStringArray") return new godot::PackedStringArray();
		if (typeNameCleaned == "PackedVector2Array") return new godot::PackedVector2Array();
		if (typeNameCleaned == "PackedVector3Array") return new godot::PackedVector3Array();
		if (typeNameCleaned == "PackedColorArray") return new godot::PackedColorArray();
		if (typeNameCleaned == "PackedVector4Array") return new godot::PackedVector4Array();

		// Default case (unsupported type)
		return nullptr;
	}
	bool SetPropertyPointerValueFromVariant(void* propertyPointer, const Variant& variantValue)
	{
		// Get Property Information
		Variant::Type variantType = variantValue.get_type();

		// Set Values based on type
		switch (variantType) {
		case Variant::Type::BOOL: 
		{
			bool* valuePtr = static_cast<bool*>(propertyPointer);
			*valuePtr = bool(variantValue);
			return true;
		}
		case Variant::Type::INT: 
		{
			int64_t* valuePtr = static_cast<int64_t*>(propertyPointer);
			*valuePtr = int64_t(variantValue);
			return true;
		}
		case Variant::Type::FLOAT: 
		{
			double* valuePtr = static_cast<double*>(propertyPointer);
			*valuePtr = double(variantValue);
			return true;
		}
		case Variant::Type::STRING: 
		{
			godot::String* valuePtr = static_cast<godot::String*>(propertyPointer);
			*valuePtr = godot::String(variantValue);
			return true;
		}
		case Variant::Type::VECTOR2: 
		{
			godot::Vector2* valuePtr = static_cast<godot::Vector2*>(propertyPointer);
			*valuePtr = godot::Vector2(variantValue);
			return true;
		}
		case Variant::Type::VECTOR2I: 
		{
			godot::Vector2i* valuePtr = static_cast<godot::Vector2i*>(propertyPointer);
			*valuePtr = godot::Vector2i(variantValue);
			return true;
		}
		case Variant::Type::RECT2: 
		{
			godot::Rect2* valuePtr = static_cast<godot::Rect2*>(propertyPointer);
			*valuePtr = godot::Rect2(variantValue);
			return true;
		}
		case Variant::Type::RECT2I: 
		{
			godot::Rect2i* valuePtr = static_cast<godot::Rect2i*>(propertyPointer);
			*valuePtr = godot::Rect2i(variantValue);
			return true;
		}
		case Variant::Type::VECTOR3: 
		{
			godot::Vector3* valuePtr = static_cast<godot::Vector3*>(propertyPointer);
			*valuePtr = godot::Vector3(variantValue);
			return true;
		}
		case Variant::Type::VECTOR3I: 
		{
			godot::Vector3i* valuePtr = static_cast<godot::Vector3i*>(propertyPointer);
			*valuePtr = godot::Vector3i(variantValue);
			return true;
		}
		case Variant::Type::TRANSFORM2D: 
		{
			godot::Transform2D* valuePtr = static_cast<godot::Transform2D*>(propertyPointer);
			*valuePtr = godot::Transform2D(variantValue);
			return true;
		}
		case Variant::Type::VECTOR4: 
		{
			godot::Vector4* valuePtr = static_cast<godot::Vector4*>(propertyPointer);
			*valuePtr = godot::Vector4(variantValue);
			return true;
		}
		case Variant::Type::VECTOR4I: 
		{
			godot::Vector4i* valuePtr = static_cast<godot::Vector4i*>(propertyPointer);
			*valuePtr = godot::Vector4i(variantValue);
			return true;
		}
		case Variant::Type::PLANE: 
		{
			godot::Plane* valuePtr = static_cast<godot::Plane*>(propertyPointer);
			*valuePtr = godot::Plane(variantValue);
			return true;
		}
		case Variant::Type::QUATERNION: 
		{
			godot::Quaternion* valuePtr = static_cast<godot::Quaternion*>(propertyPointer);
			*valuePtr = godot::Quaternion(variantValue);
			return true;
		}
		case Variant::Type::AABB:
		{
			godot::AABB* valuePtr = static_cast<godot::AABB*>(propertyPointer);
			*valuePtr = godot::AABB(variantValue);
			return true;
		}
		case Variant::Type::BASIS:
		{
			godot::Basis* valuePtr = static_cast<godot::Basis*>(propertyPointer);
			*valuePtr = godot::Basis(variantValue);
			return true;
		}
		case Variant::Type::TRANSFORM3D:
		{
			godot::Transform3D* valuePtr = static_cast<godot::Transform3D*>(propertyPointer);
			*valuePtr = godot::Transform3D(variantValue);
			return true;
		}
		case Variant::Type::COLOR:
		{
			godot::Color* valuePtr = static_cast<godot::Color*>(propertyPointer);
			*valuePtr = godot::Color(variantValue);
			return true;
		}
		case Variant::Type::STRING_NAME:
		{
			godot::StringName* valuePtr = static_cast<godot::StringName*>(propertyPointer);
			*valuePtr = godot::StringName(variantValue);
			return true;
		}
		case Variant::Type::NODE_PATH:
		{
			godot::NodePath* valuePtr = static_cast<godot::NodePath*>(propertyPointer);
			*valuePtr = godot::NodePath(variantValue);
			return true;
		}
		case Variant::Type::RID:
		{
			godot::RID* valuePtr = static_cast<godot::RID*>(propertyPointer);
			*valuePtr = godot::RID(variantValue);
			return true;
		}
		case Variant::Type::OBJECT:
		{
			godot::Object** valuePtr = static_cast<godot::Object**>(propertyPointer);
			*valuePtr = Object::cast_to<godot::Object>(variantValue);
			return true;
		}
		case Variant::Type::CALLABLE:
		{
			godot::Callable* valuePtr = static_cast<godot::Callable*>(propertyPointer);
			*valuePtr = godot::Callable(variantValue);
			return true;
		}
		case Variant::Type::DICTIONARY:
		{
			godot::Dictionary* valuePtr = static_cast<godot::Dictionary*>(propertyPointer);
			*valuePtr = godot::Dictionary(variantValue);
			return true;
		}
		case Variant::Type::ARRAY:
		{
			godot::Array* valuePtr = static_cast<godot::Array*>(propertyPointer);
			*valuePtr = godot::Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_BYTE_ARRAY:
		{
			godot::PackedByteArray* valuePtr = static_cast<godot::PackedByteArray*>(propertyPointer);
			*valuePtr = godot::PackedByteArray(variantValue);
			return true;
		}
		case Variant::Type::PACKED_INT32_ARRAY:
		{
			godot::PackedInt32Array* valuePtr = static_cast<godot::PackedInt32Array*>(propertyPointer);
			*valuePtr = godot::PackedInt32Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_INT64_ARRAY:
		{
			godot::PackedInt64Array* valuePtr = static_cast<godot::PackedInt64Array*>(propertyPointer);
			*valuePtr = godot::PackedInt64Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_FLOAT32_ARRAY:
		{
			godot::PackedFloat32Array* valuePtr = static_cast<godot::PackedFloat32Array*>(propertyPointer);
			*valuePtr = godot::PackedFloat32Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_FLOAT64_ARRAY:
		{
			godot::PackedFloat64Array* valuePtr = static_cast<godot::PackedFloat64Array*>(propertyPointer);
			*valuePtr = godot::PackedFloat64Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_STRING_ARRAY:
		{
			godot::PackedStringArray* valuePtr = static_cast<godot::PackedStringArray*>(propertyPointer);
			*valuePtr = godot::PackedStringArray(variantValue);
			return true;
		}
		case Variant::Type::PACKED_VECTOR2_ARRAY:
		{
			godot::PackedVector2Array* valuePtr = static_cast<godot::PackedVector2Array*>(propertyPointer);
			*valuePtr = godot::PackedVector2Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_VECTOR3_ARRAY:
		{
			godot::PackedVector3Array* valuePtr = static_cast<godot::PackedVector3Array*>(propertyPointer);
			*valuePtr = godot::PackedVector3Array(variantValue);
			return true;
		}
		case Variant::Type::PACKED_COLOR_ARRAY:
		{
			godot::PackedColorArray* valuePtr = static_cast<godot::PackedColorArray*>(propertyPointer);
			*valuePtr = godot::PackedColorArray(variantValue);
			return true;
		}
		case Variant::Type::PACKED_VECTOR4_ARRAY:
		{
			godot::PackedVector4Array* valuePtr = static_cast<godot::PackedVector4Array*>(propertyPointer);
			*valuePtr = godot::PackedVector4Array(variantValue);
			return true;
		}
		default:
			return false; // Unsupported type
		}
	}
	std::string ParseClassNameFromScriptSource(const std::string& sourceCode)
	{
		std::regex pattern(R"(JENOVA_CLASS_NAME\s*\(\s*\"([^\"]+)\"\s*\))");
		std::smatch match;
		std::istringstream stream(sourceCode);
		std::string line;
		while (std::getline(stream, line)) 
		{
			if (line.find("//") == std::string::npos && line.find("/*") == std::string::npos) 
			{
				if (std::regex_search(line, match, pattern)) return match[1].str();
			}
		}
		return "";
	}
	jenova::ScriptFileState BackupScriptFileState(const std::string& scriptFilePath)
	{
		ScriptFileState scriptFileState;
		HANDLE hFile = CreateFileA(scriptFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) return scriptFileState;
		if (GetFileTime(hFile, &scriptFileState.creationTime, &scriptFileState.accessTime, &scriptFileState.writeTime)) scriptFileState.isValid = true;
		CloseHandle(hFile);
		return scriptFileState;
	}
	bool RestoreScriptFileState(const std::string& scriptFilePath, const jenova::ScriptFileState& scriptFileState)
	{
		if (!scriptFileState.isValid) return false;
		HANDLE hFile = CreateFileA(scriptFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) return false;
		bool success = SetFileTime(hFile, &scriptFileState.creationTime, &scriptFileState.accessTime, &scriptFileState.writeTime);
		CloseHandle(hFile);
		return success;
	}
	void RandomWait(int minWaitTime, int maxWaitTime)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(minWaitTime, maxWaitTime);
		int waitTime = dis(gen);
		std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	}

	#pragma endregion
	
	// Core Reimplementation
	void* CoreMemoryMove(void* dest, const void* src, std::size_t count)
	{
		char* pDest = static_cast<char*>(dest);
		const char* pSrc = static_cast<const char*>(src);
		if (pDest < pSrc)
		{
			std::copy(pSrc, pSrc + count, pDest);
		}
		else
		{
			for (std::size_t i = count; i > 0; --i)
			{
				pDest[i - 1] = pSrc[i - 1];
			}
		}
		return dest;
	}

	// Handlers
	LONG WINAPI JenovaCrashHandler(EXCEPTION_POINTERS* exceptionInfo)
	{
		// Generate Crash Dump
		jenova::GenerateMiniMemoryDump(exceptionInfo);

		// Show Crash Dialog
		MessageBoxA(0, "Jenova™ Core Crashed! Sorry about that... Hmm...\nNot really, Coding is hard man! :)", "Jenova :: Fatal Error", MB_ICONERROR);

		// Suppress the Exception
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// Deprecateds
	jenova::SerializedData ProcessAndExtractPropertiesFromScriptDeprecated(OUT std::string& scriptSource, const std::string& scriptUID)
	{
		// Property Metadata Serializer
		nlohmann::json propertiesMetadata;

		// Helpers
		auto isCommented = [](const std::string& line)
		{
			std::regex commentRegex(R"(^\s*(//|/\*|\*/))");
			return std::regex_search(line, commentRegex);
		};
		auto parseArguments = [](const std::string& argsString)
		{
			std::vector<std::string> args;
			std::string currentArg;
			int parenDepth = 0;
			bool inQuotes = false;
			for (size_t i = 0; i < argsString.size(); ++i)
			{
				char c = argsString[i];
				if (c == '"') inQuotes = !inQuotes;
				else if (c == '(') if (!inQuotes) ++parenDepth;
				else if (c == ')') if (!inQuotes) --parenDepth;
				if (c == ',' && parenDepth == 0 && !inQuotes)
				{
					// Trim whitespace around currentArg
					currentArg.erase(currentArg.find_last_not_of(" \t\n\r\f\v") + 1);
					currentArg.erase(0, currentArg.find_first_not_of(" \t\n\r\f\v"));
					args.push_back(currentArg);
					currentArg.clear();
				}
				else
				{
					currentArg += c;
				}
			}
			if (!currentArg.empty())
			{
				currentArg.erase(currentArg.find_last_not_of(" \t\n\r\f\v") + 1);
				currentArg.erase(0, currentArg.find_first_not_of(" \t\n\r\f\v"));
				args.push_back(currentArg);
			}
			return args;
		};

		// Split scriptSource into lines for processing
		std::istringstream scriptStream(scriptSource);
		std::string line, paramHandlers;
		int lineNumber = 0;
		int paramCount = 0;

		// Flags
		bool isHeaderCommentAdded = false;

		// Extract properties and replace lines
		while (std::getline(scriptStream, line))
		{
			lineNumber++;

			// Skip if line is commented
			if (isCommented(line)) continue;

			// Detect JENOVA_PROPERTY line
			std::regex propertyRegex(R"(JENOVA_PROPERTY\s*\((.*)\))");
			std::smatch match;
			if (std::regex_search(line, match, propertyRegex))
			{
				paramCount++;

				// Parse arguments
				std::vector<std::string> args = parseArguments(match[1].str());
				if (args.size() >= 3)
				{
					// Set Property Data
					nlohmann::json propertyMetadata;
					propertyMetadata["PropertyName"] = args[1];
					propertyMetadata["PropertyType"] = args[0];
					propertyMetadata["PropertyDefault"] = args[2];
					if (args.size() >= 4) propertyMetadata["PropertyGroup"] = jenova::ReplaceAllMatchesWithStringAndReturn(args[3].substr(1, args[3].length() - 2), " ", "_");
					if (args.size() >= 5) propertyMetadata["PropertyHint"] = args[4];
					if (args.size() >= 6) propertyMetadata["PropertyHintString"] = args[5];
					if (args.size() >= 7) propertyMetadata["PropertyClassName"] = args[6];
					propertiesMetadata.push_back(propertyMetadata);

					// Verbose
					jenova::VerboseByID(__LINE__, "Property Extracted >> Name : [%s] Type : [%s]  Default Value :[%s]", args[0].c_str(), args[1].c_str(), args[2].c_str());
				}

				// Add Header Comment
				if (!isHeaderCommentAdded)
				{
					paramHandlers += "// Script Properties Handlers\n";
					isHeaderCommentAdded = true;
				}

				// Generate Properties & Handlers
				size_t lineStartPos = scriptSource.find(line);
				if (lineStartPos != std::string::npos)
				{
					scriptSource.replace(lineStartPos, line.length(), jenova::Format("%s* __prop_%s = nullptr;", args[0], args[1]));
					paramHandlers += jenova::Format("#define %s *__prop_%s\n", args[1], args[1]);
				}
			}
		}

		// Add Handlers to Source
		if (paramCount != 0)
		{
			paramHandlers += "\n";
			scriptSource.insert(0, paramHandlers);
		}

		// Return Metadata
		return propertiesMetadata.dump();
	}
}