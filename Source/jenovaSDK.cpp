
/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|							Jenova SDK                         |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Windows SDK
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

// C++ SDK
#include <stdarg.h>
#include <iostream>
#include <functional>

// Godot SDK
#include <gdextension_interface.h>
#include <godot.hpp>
#include <core/defs.hpp>
#include <core/class_db.hpp>
#include <core/object.hpp>
#include <classes/os.hpp>
#include <classes/time.hpp>
#include <classes/engine.hpp>
#include <classes/editor_interface.hpp>
#include <classes/editor_selection.hpp>
#include <classes/theme.hpp>
#include <classes/window.hpp>
#include <classes/scene_tree.hpp>
#include <classes/resource_saver.hpp>
#include <classes/resource_loader.hpp>
#include <classes/texture2d.hpp>
#include <templates/vector.hpp>
#include <variant/string.hpp>
#include <variant/node_path.hpp>
#include <variant/array.hpp>
#include <variant/utility_functions.hpp>

// GodotSDK/LithiumSDK
#ifdef LITHIUM_EDITION
#include <classes/component.hpp>
#include <classes/element.hpp>
#include <Misc/lithium-compatibility.hpp>
#else
#include <classes/packed_scene.hpp>
#endif

// Jenova System SDK
#include "JenovaSDK.h"

// Namespaces
using namespace std;

// Import External Functions
namespace jenova
{
	extern void Alert(const char* fmt, ...);
	extern void Error(const char* stageName, const char* fmt, ...);
	extern void Warning(const char* stageName, const char* fmt, ...);
	extern int ShowMessageBox(const char* msg, const char* title, int flags);
};

// Internal Structs
struct NodeBackup
{
	godot::String nodeName;
	godot::String nodeClass;
	godot::String scenePath;
	godot::PackedScene* nodeBackup = nullptr;
	godot::Node* dummyNode = nullptr;
	godot::Node* sceneRoot = nullptr;
};

// Internal Classes
class EventCallback : public godot::RefCounted
{
private:
	void* callback;

public:
	void OnEventCall()
	{
		if (!this->callback) return;
		typedef void(*callbackFunc)(void);
		callbackFunc callbackFunction = (callbackFunc)this->callback;
		if (!callbackFunction) return;
		callbackFunction();
		memdelete(this);
	}
	EventCallback(void* callbackPtr) : callback(callbackPtr) {}
};

// Storages
godot::Vector<NodeBackup> nodeBackups;
std::unordered_map<std::string, void*> globalMemoryMap;
std::unordered_map<std::string, godot::Variant> globalVariables;

// Internal Helpers
static void CollectNodesByClassName(godot::Node* node, const godot::String& class_name, godot::Vector<godot::Node*>& result)
{
	if (node->is_class(class_name))
	{
		result.push_back(node);
	}
	for (int i = 0; i < node->get_child_count(); ++i) 
	{
		CollectNodesByClassName(node->get_child(i), class_name, result);
	}
}
static bool OverrideClassAPIType(const godot::StringName& className, jenova::sdk::ClassAccess apiType)
{
	// Validate Class
	if (!godot::ClassDB::class_exists(className)) return true; 

	// Check If Engine Build Support API Override Feature
	if (godot::ClassDBSingleton::get_singleton()->has_method("class_override_api_type"))
	{
		auto result = godot::ClassDBSingleton::get_singleton()->call("class_override_api_type", className, int(apiType));
		if (godot::Error(int(result)) != godot::Error::OK)
		{
			jenova::Error("Sakura", "Failed to Override API of Class '%s'", jenova::sdk::GetCStr(className));
			return false;
		};

		// All Good
		return true;
	}
	else
	{
		// Warn User About Lack of Nested Extensions Runtime Hot-Reload Capability
		jenova::ShowMessageBox("You are attempting to Hot-Reload runtime with Nested Extensions and "
			"your engine build does not contain the required feature.\n\n"
			"To use this feature you need a compatible build.\n\n"
			"Godot Jenova Compatible, Godot Core, Deicidium and Lithium IDE support this feature.\n", "Warning", 0x00000030L);

		// Failed
		return false;
	}
}
static godot::TypedArray<godot::Node> GetOpenedScenes()
{
	if (godot::EditorInterface::get_singleton()->has_method("get_open_scene_roots"))
	{
		return godot::TypedArray<godot::Node>(godot::EditorInterface::get_singleton()->call("get_open_scene_roots"));
	}
	if (godot::EditorInterface::get_singleton()->has_method("get_open_scenes_roots")) 
	{
		return godot::TypedArray<godot::Node>(godot::EditorInterface::get_singleton()->call("get_open_scenes_roots"));
	}
	return godot::TypedArray<godot::Node>();
}

// System SDK Implementation
namespace jenova::sdk
{
	// Helpers Utilities
	bool JenovaSDK::IsEditor()
	{
		return godot::Engine::get_singleton()->is_editor_hint();
	}
	bool JenovaSDK::IsGame()
	{
		if (IsEditor()) return false;
		if (godot::OS::get_singleton()->is_debug_build()) return false;
		return true;
	}
	godot::Node* JenovaSDK::GetNodeByPath(const godot::String& path)
	{
		godot::SceneTree* scene_tree = dynamic_cast<godot::SceneTree*>(godot::Engine::get_singleton()->get_main_loop());
		if (scene_tree) return scene_tree->get_root()->get_node<godot::Node>(godot::NodePath(path));
		return nullptr;
	}
	godot::Node* JenovaSDK::FindNodeByName(godot::Node* parent, const godot::String& name)
	{
		if (!parent) return nullptr;
		if (parent->get_name() == name) return parent;
		for (int i = 0; i < parent->get_child_count(); ++i)
		{
			godot::Node* child = parent->get_child(i);
			godot::Node* result = FindNodeByName(child, name);
			if (result) return result;
		}
		return nullptr;
	}
	StringPtr JenovaSDK::GetNodeUniqueID(godot::Node* node)
	{
		return GetCStr(godot::String(node->get_path()).md5_text());
	}
	godot::SceneTree* JenovaSDK::GetTree()
	{
		godot::SceneTree* scene_tree = dynamic_cast<godot::SceneTree*>(godot::Engine::get_singleton()->get_main_loop());
		return scene_tree;
	}
	double JenovaSDK::GetTime()
	{
		int64_t time_msec = godot::Time::get_singleton()->get_ticks_msec();
		return static_cast<double>(time_msec) / 1000.0;
	}
	godot::String JenovaSDK::Format(StringPtr format, va_list args)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, args);
		return godot::String(buffer);
	}
	godot::String JenovaSDK::Format(WideStringPtr format, va_list args)
	{
		wchar_t buffer[1024];
		vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), format, args);
		return godot::String(buffer);
	}
	void JenovaSDK::Output(StringPtr format, va_list args)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, args);
		godot::UtilityFunctions::print(godot::String("[JENOVA-SDK] > ") + godot::String(buffer));
	}
	void JenovaSDK::Output(WideStringPtr format, va_list args)
	{
		wchar_t buffer[1024];
		vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), format, args);
		godot::UtilityFunctions::print(godot::String(L"[JENOVA-SDK] > ") + godot::String(buffer));
	}
	void JenovaSDK::DebugOutput(StringPtr format, va_list args)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, args);
		std::string debugMessage = "[JENOVA-SDK] ::> ";
		debugMessage += buffer;

		// Debug Print
		#if defined(_WIN32) || defined(_WIN64)
			OutputDebugStringA(debugMessage.c_str());
		#else
			std::clog << debugMessage << std::endl;
		#endif
	}
	void JenovaSDK::DebugOutput(WideStringPtr format, va_list args)
	{
		wchar_t buffer[1024];
		vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), format, args);
		std::wstring debugMessage = L"[JENOVA-SDK] ::> ";
		debugMessage += buffer;

		// Debug Print
		#if defined(_WIN32) || defined(_WIN64)
			OutputDebugStringW(debugMessage.c_str());
		#else
			std::wclog << debugMessage << std::endl;
		#endif
	}
	StringPtr JenovaSDK::GetCStr(const godot::String& godotStr)
	{
		std::string str((char*)godotStr.utf8().ptr(), godotStr.utf8().size());
		if (!str.empty() && str.back() == '\0') str.pop_back();

		// Bad Approach, Needs Improvement
		#if defined(_WIN32) || defined(_WIN64)
			return _strdup(str.c_str());
		#else
			return strdup(str.c_str());
		#endif
	}
	WideStringPtr JenovaSDK::GetWCStr(const godot::String& godotStr)
	{
		godot::PackedByteArray wchar_buffer = godotStr.to_wchar_buffer();
		size_t length = wchar_buffer.size() / sizeof(wchar_t);
		std::wstring str((wchar_t*)wchar_buffer.ptr(), length);
		if (!str.empty() && str.back() == L'\0') str.pop_back();

		// Bad Approach, Needs Improvement
		#if defined(_WIN32) || defined(_WIN64)
				return _wcsdup(str.c_str());
		#else
				return wcsdup(str.c_str());
		#endif
	}
	ObjectPtr JenovaSDK::GetObjectPointer(NativePtr obj)
	{
		return ObjectPtr(IntPtr(obj));
	}
	bool JenovaSDK::SetClassIcon(const godot::String& className, const godot::Ref<godot::Texture2D> iconImage)
	{
		if (!godot::ClassDB::class_exists(className)) return false;
		godot::Ref<godot::Theme> editor_theme = godot::EditorInterface::get_singleton()->get_editor_theme();
		if (editor_theme->has_icon(className, "EditorIcons")) return false;
		editor_theme->set_icon(className, "EditorIcons", iconImage);
		return true;
	}
	double JenovaSDK::MatchScaleFactor(double inputSize)
	{
		if (IsEditor())
		{
			double scaleFactor = godot::EditorInterface::get_singleton()->get_editor_scale();
			return inputSize * scaleFactor;
		}
		else
		{
			return inputSize;
		}
	}
	godot::Error JenovaSDK::CreateSignalCallback(godot::Object* object, const godot::String& signalName, FunctionPtr callbackPtr)
	{
		return object->connect(signalName, callable_mp(memnew(EventCallback(callbackPtr)), &EventCallback::OnEventCall));
	}
}

// Jenova Runtime SDK
#include "Jenova.hpp"

// Runtime SDK Implementation
namespace jenova::sdk
{
	// Helpers Utilities
	void JenovaSDK::Alert(StringPtr fmt, va_list args)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		ShowMessageBox(buffer, "[JENOVA-SDK]", 0);
	}
	jenova::sdk::EngineMode JenovaSDK::GetEngineMode()
	{
		return jenova::sdk::EngineMode(jenova::GlobalStorage::CurrentEngineMode);
	}
	bool JenovaSDK::CreateDirectoryMonitor(const String& directoryPath)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->AddDirectory(directoryPath)) return false;
		return true;
	}
	bool JenovaSDK::CreateFileMonitor(const String& filePath)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->AddFile(filePath)) return false;
		return true;
	}
	bool JenovaSDK::RegisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->RegisterCallback(jenova::AssetMonitor::AssetMonitorCallback(callbackPtr))) return false;
		return true;
	}
	bool JenovaSDK::UnregisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->UnregisterCallback(jenova::AssetMonitor::AssetMonitorCallback(callbackPtr))) return false;
		return true;
	}
	bool JenovaSDK::ReloadJenovaRuntime(RuntimeReloadMode reloadMode)
	{
		jenova::sdk::Output("ReloadJenovaRuntime -> Not Implemented Yet");
		return false;
	}
	void JenovaSDK::CreateCheckpoint(const godot::String& checkPointName)
	{
		JenovaTinyProfiler::CreateCheckpoint(AS_STD_STRING(checkPointName));
	}
	double JenovaSDK::GetCheckpointTime(const godot::String& checkPointName)
	{
		return JenovaTinyProfiler::GetCheckpointTime(AS_STD_STRING(checkPointName));
	}
	void JenovaSDK::DeleteCheckpoint(const godot::String& checkPointName)
	{
		JenovaTinyProfiler::DeleteCheckpoint(AS_STD_STRING(checkPointName));
	}
	double JenovaSDK::GetCheckpointTimeAndDispose(const godot::String& checkPointName)
	{
		return JenovaTinyProfiler::GetCheckpointTimeAndDispose(AS_STD_STRING(checkPointName));
	}
	godot::String JenovaSDK::GetPackageRepositoryPath(bool globalize)
	{
		if (!IsEditor()) return godot::String();
		godot::String path = godot::String(jenova::GetEditorSetting(jenova::JenovaSettings::PackageRepositoryPathConfigPath)).replace("\\", "/");
		if (path.contains("res://")) path = globalize ? ProjectSettings::get_singleton()->globalize_path(path) : path;
		if (!path.ends_with("/")) path += "/";
		return path;
	}
	bool JenovaSDK::RegisterRuntimeCallback(RuntimeCallback callbackPtr)
	{
		return jenova::RegisterRuntimeEventCallback((jenova::FunctionPointer)callbackPtr);
	}
	bool JenovaSDK::UnregisterRuntimeCallback(RuntimeCallback callbackPtr)
	{
		return jenova::UnregisterRuntimeEventCallback((jenova::FunctionPointer)callbackPtr);
	}

	// Graphic Utilities
	NativePtr JenovaSDK::GetGameWindowHandle()
	{
		return NativePtr(jenova::GetMainWindowNativeHandle());
	}
	StringPtr JenovaSDK::GetRenderingDriverName()
	{
		auto projectSetting = ProjectSettings::get_singleton();
		#if defined(_WIN32) || defined(_WIN64)
			return GetCStr(String(projectSetting->get_setting("rendering/rendering_device/driver.windows")));
		#else
			return GetCStr(String(projectSetting->get_setting("rendering/rendering_device/driver")));
		#endif
	}
	NativePtr JenovaSDK::GetRenderingDriverResource(DriverResourceID resourceType)
	{
		godot::RenderingDevice* device = godot::RenderingServer::get_singleton()->get_rendering_device();
		if (device) return reinterpret_cast<NativePtr>(device->get_driver_resource(godot::RenderingDevice::DriverResource(resourceType), RID(), 0));
		return nullptr;
	}

	// Memory Management Utilities (Anzen)
	NativePtr JenovaSDK::GetGlobalPointer(MemoryID id)
	{
		auto it = globalMemoryMap.find(id);
		if (it != globalMemoryMap.end()) return it->second;
		return nullptr;
	}
	NativePtr JenovaSDK::SetGlobalPointer(MemoryID id, NativePtr ptr)
	{
		auto it = globalMemoryMap.find(id);
		if (it != globalMemoryMap.end())
		{
			NativePtr oldPtr = it->second;
			it->second = ptr;
			return oldPtr;
		}
		else {
			globalMemoryMap[id] = ptr;
			return ptr;
		}
	}
	void JenovaSDK::DeleteGlobalPointer(MemoryID id)
	{
		globalMemoryMap.erase(id);
	}
	NativePtr JenovaSDK::AllocateGlobalMemory(MemoryID id, size_t size)
	{
		NativePtr mem = jenova::AllocateMemory(size);
		if (!mem) return nullptr;
		globalMemoryMap[id] = mem;
		return mem;
	}
	void JenovaSDK::FreeGlobalMemory(MemoryID id)
	{
		auto it = globalMemoryMap.find(id);
		if (it != globalMemoryMap.end())
		{
			jenova::FreeMemory(it->second);
			globalMemoryMap.erase(it);
		}
	}

	// Global Variable Storage Utilities (Anzen)
	godot::Variant JenovaSDK::GetGlobalVariable(VariableID id)
	{
		return globalVariables[id];
	}
	void JenovaSDK::SetGlobalVariable(VariableID id, godot::Variant var)
	{
		globalVariables[id] = var;
	}
	void JenovaSDK::ClearGlobalVariables()
	{
		globalVariables.clear();
	}

	// Task System Utilities
	TaskID JenovaSDK::InitiateTask(TaskFunction function)
	{
		return JenovaTaskSystem::InitiateTask(function);
	}
	bool JenovaSDK::IsTaskComplete(TaskID taskID)
	{
		return JenovaTaskSystem::IsTaskComplete(taskID);
	}
	void JenovaSDK::ClearTask(TaskID taskID)
	{
		JenovaTaskSystem::ClearTask(taskID);
	}

	// Hot-Reloading Utilities (Sakura)
	bool JenovaSDK::SupportsReload()
	{
		if (IsEditor()) { if (godot::EditorInterface::get_singleton()->has_method("get_open_scene_roots")) return true; }
		if (IsEditor()) { if (godot::EditorInterface::get_singleton()->has_method("get_open_scenes_roots")) return true; }
		return false;
	}
	void JenovaSDK::PrepareReload(const godot::String& className)
	{
		// Disable Hot-Reloading In Static SDK
		#ifdef JENOVA_SDK_STATIC
			return;
		#endif

		// Validate
		if (!godot::ClassDB::class_exists(className)) return;
	
		// Validate Scene Tree [Required!]
		if (!GetTree()) return;

		// Override Nested Node Class API Type
		OverrideClassAPIType(className, jenova::sdk::ClassAccess::Extension);

		// Deselect Nodes
		if (IsEditor()) godot::EditorInterface::get_singleton()->get_selection()->clear();

		// Collect Opened Scenes
		godot::TypedArray<godot::Node> openedScenes;
		if (IsEditor()) 
		{
			// Validate Hot-Reloading
			int openedScenesCount = godot::EditorInterface::get_singleton()->get_open_scenes().size();
			if (openedScenesCount == 1)
			{
				// Only One Scene is Open in Editor, Add Active Scene
				openedScenes.push_back(GetTree()->get_root());
			}
			else if (openedScenesCount > 1)
			{
				// Multiple Scenes Are Open in Editor, Collect Opened Scenes If Supported
				if (sakura::SupportsReload())
				{
					openedScenes = GetOpenedScenes();
				}
				else
				{
					// Warn User About Lack of Multiple Scenes Hot-Reload Capability
					ShowMessageBox("You are attempting to Hot-Reload multiple opened scenes and "
						"your engine build does not contain the required feature.\n\n"
						"To use this feature you need a compatible build, "
						"If you have any nested nodes in your other opened scene, It will lead to a crash when switching to them.\n\n"
						"Godot Jenova Compatible, Godot Core, Deicidium and Lithium IDE support this feature.\n", "Warning", 0x00000030L);

					// Fallback to Active Scene
					openedScenes.push_back(GetTree()->get_root());
				}
			}
		}
		else
		{
			// At Runtime/Debug Mode Only One Scene is Opened
			openedScenes.push_back(GetTree()->get_root());
		}

		// Process Nested Node Instances From Scenes
		for (size_t i = 0; i < openedScenes.size(); i++)
		{
			// Get Scene Root
			godot::Node* sceneRoot = (godot::Node*)openedScenes[i]._native_ptr();

			// Validate Scene Root
			if (!sceneRoot) continue;

			// Collect Nodes With Class Name
			godot::Vector<godot::Node*> classNodes;
			CollectNodesByClassName(sceneRoot, className, classNodes);

			// Backup Nodes
			for (const auto& classNode : classNodes)
			{
				// Create Node Backup
				NodeBackup nodebackup;
				nodebackup.nodeName = classNode->get_name();
				nodebackup.nodeClass = godot::String(className);
				nodebackup.sceneRoot = sceneRoot;
				nodebackup.scenePath = sceneRoot->get_scene_file_path();

				// Duplicate Node to Backup [Due to Issue #81982]
				godot::Node* classNodeClone = classNode->duplicate();

				// Pack Current Scene
				nodebackup.nodeBackup = memnew(godot::PackedScene);
				nodebackup.nodeBackup->pack(classNodeClone);
				memdelete(classNodeClone); // classNodeClone->queue_free();

				// Replace With Dummy Node
				nodebackup.dummyNode = memnew(godot::Node);
				classNode->replace_by(nodebackup.dummyNode, true);
				memdelete(classNode); // classNode->queue_free();

				// Add to Dummy Nodes
				nodeBackups.push_back(nodebackup);
			}
		}
	}
	void JenovaSDK::FinishReload(const godot::String& className)
	{
		// Disable Hot-Reloading In Static SDK
		#ifdef JENOVA_SDK_STATIC
			return;
		#endif

		// Validate
		if (!godot::ClassDB::class_exists(className)) return;

		// Override Nested Node Class API Type
		OverrideClassAPIType(className, jenova::sdk::ClassAccess::Extension);

		// Create Class Name
		godot::String backupClassName(className);

		// Restore Nodes
		for (int i = 0; i < nodeBackups.size(); ++i)
		{
			// Get Node Backup
			NodeBackup nodeBackup = nodeBackups[i];

			// Validate Node Class
			if (nodeBackup.nodeClass != backupClassName) return;

			// Validate Backup Data
			if (!nodeBackup.sceneRoot) return;

			// Restore Nodes From Backup
			godot::Node* originalNode = nodeBackup.nodeBackup->instantiate(godot::PackedScene::GenEditState::GEN_EDIT_STATE_DISABLED);
			if (originalNode)
			{
				nodeBackup.dummyNode->replace_by(originalNode, true);
				memdelete(nodeBackup.dummyNode);
				memdelete(nodeBackup.nodeBackup);
				nodeBackups.remove_at(i);
				--i;
			}
		}
	}
	void JenovaSDK::Dispose(const godot::String& className)
	{
		godot::StringName classNameStr(className);
		if (!godot::ClassDB::class_exists(classNameStr)) return;
		godot::internal::gdextension_interface_classdb_unregister_extension_class(godot::internal::library, classNameStr._native_ptr());
	}

	// C Scripting Utilities (Clektron)
	bool JenovaSDK::ExecuteScript(StringPtr ctronScript, bool noEntrypoint)
	{
		return Clektron::get_singleton()->ExecuteScript(std::string(ctronScript), noEntrypoint);
	}
	bool JenovaSDK::ExecuteScriptFromFile(StringPtr ctronScriptFile, bool noEntrypoint)
	{
		return Clektron::get_singleton()->ExecuteScriptFromFile(std::string(ctronScriptFile), noEntrypoint);
	}
	bool JenovaSDK::ExecuteScript(const godot::String& ctronScript, bool noEntrypoint)
	{
		return Clektron::get_singleton()->ExecuteScript(ctronScript, noEntrypoint);
	}
	bool JenovaSDK::ExecuteScriptFromFile(const godot::String& ctronScriptFile, bool noEntrypoint)
	{
		return Clektron::get_singleton()->ExecuteScriptFromFile(ctronScriptFile, noEntrypoint);
	}
}

// Jenova SDK Management
namespace jenova
{
	// JenovaSDK Interface Singleton
	namespace sdk { JenovaSDK* bridge = nullptr; }

	// Create/Release SDK Interface
	JenovaSDKInterface CreateJenovaSDKInterface()
	{
		JenovaSDKInterface sdkInterface = new sdk::JenovaSDK();
		sdk::bridge = (sdk::JenovaSDK*)sdkInterface;
		return sdkInterface;
	}
	void* GetJenovaSDKFunctionSolver()
	{
		return reinterpret_cast<void*>(&jenova::sdk::GetSDKFunction);
	}
	bool ReleaseJenovaSDKInterface(JenovaSDKInterface sdkInterface)
	{
		if (!sdkInterface) return false;
		delete sdkInterface;
		sdk::bridge = nullptr;
		return true;
	}

	// Exported API
	JenovaSDKInterface sdk::GetSDKInterface()
	{
		return bridge;
	}
	sdk::FunctionPtr sdk::GetSDKFunction(StringPtr sdkFunctionName)
	{
		// Validate Bridge
		if (bridge == nullptr) return nullptr;

		// Solve Helpers Utilities Functions
		if (string(sdkFunctionName) == "IsEditor") return FunctionPtr(&IsEditor);
		if (string(sdkFunctionName) == "IsGame") return FunctionPtr(&IsGame);
		if (string(sdkFunctionName) == "GetEngineMode") return FunctionPtr(&GetEngineMode);
		if (string(sdkFunctionName) == "GetNodeByPath") return FunctionPtr(&GetNodeByPath);
		if (string(sdkFunctionName) == "FindNodeByName") return FunctionPtr(&FindNodeByName);
		if (string(sdkFunctionName) == "GetNodeUniqueID") return FunctionPtr(&GetNodeUniqueID);
		if (string(sdkFunctionName) == "GetTree") return FunctionPtr(&GetTree);
		if (string(sdkFunctionName) == "GetTime") return FunctionPtr(&GetTime);
		if (string(sdkFunctionName) == "Alert") return FunctionPtr(&Alert);
		if (string(sdkFunctionName) == "FormatA") return FunctionPtr((godot::String(*)(StringPtr, ...))(Format));
		if (string(sdkFunctionName) == "FormatW") return FunctionPtr((godot::String(*)(WideStringPtr, ...))(Format));
		if (string(sdkFunctionName) == "OutputA") return FunctionPtr((void(*)(StringPtr, ...))(Output));
		if (string(sdkFunctionName) == "OutputW") return FunctionPtr((void(*)(WideStringPtr, ...))(Output));
		if (string(sdkFunctionName) == "DebugOutputA") return FunctionPtr((void(*)(StringPtr, ...))(DebugOutput));
		if (string(sdkFunctionName) == "DebugOutputW") return FunctionPtr((void(*)(WideStringPtr, ...))(DebugOutput));
		if (string(sdkFunctionName) == "GetCStr") return FunctionPtr(&GetCStr);
		if (string(sdkFunctionName) == "GetWCStr") return FunctionPtr(&GetWCStr);
		if (string(sdkFunctionName) == "GetObjectPointer") return FunctionPtr(&GetObjectPointer);
		if (string(sdkFunctionName) == "SetClassIcon") return FunctionPtr(&SetClassIcon);
		if (string(sdkFunctionName) == "MatchScaleFactor") return FunctionPtr(&MatchScaleFactor);
		if (string(sdkFunctionName) == "CreateSignalCallback") return FunctionPtr(&CreateSignalCallback);
		if (string(sdkFunctionName) == "CreateDirectoryMonitor") return FunctionPtr(&CreateDirectoryMonitor);
		if (string(sdkFunctionName) == "CreateFileMonitor") return FunctionPtr(&CreateFileMonitor);
		if (string(sdkFunctionName) == "RegisterFileMonitorCallback") return FunctionPtr(&RegisterFileMonitorCallback);
		if (string(sdkFunctionName) == "UnregisterFileMonitorCallback") return FunctionPtr(&UnregisterFileMonitorCallback);
		if (string(sdkFunctionName) == "ReloadJenovaRuntime") return FunctionPtr(&ReloadJenovaRuntime);
		if (string(sdkFunctionName) == "CreateCheckpoint") return FunctionPtr(&CreateCheckpoint);
		if (string(sdkFunctionName) == "GetCheckpointTime") return FunctionPtr(&GetCheckpointTime);
		if (string(sdkFunctionName) == "DeleteCheckpoint") return FunctionPtr(&DeleteCheckpoint);
		if (string(sdkFunctionName) == "GetCheckpointTimeAndDispose") return FunctionPtr(&GetCheckpointTimeAndDispose);
		if (string(sdkFunctionName) == "GetPackageRepositoryPath") return FunctionPtr(&GetPackageRepositoryPath);
		if (string(sdkFunctionName) == "RegisterRuntimeCallback") return FunctionPtr(&RegisterRuntimeCallback);
		if (string(sdkFunctionName) == "UnregisterRuntimeCallback") return FunctionPtr(&UnregisterRuntimeCallback);

		// Solve Graphic Utilities Functions
		if (string(sdkFunctionName) == "GetGameWindowHandle") return FunctionPtr(&GetGameWindowHandle);
		if (string(sdkFunctionName) == "GetRenderingDriverName") return FunctionPtr(&GetRenderingDriverName);
		if (string(sdkFunctionName) == "GetRenderingDriverResource") return FunctionPtr(&GetRenderingDriverResource);

		// Solve Hot-Reloading Utilities (Sakura) Functions
		if (string(sdkFunctionName) == "SupportsReload") return FunctionPtr(&sakura::SupportsReload);
		if (string(sdkFunctionName) == "PrepareReload") return FunctionPtr(&sakura::PrepareReload);
		if (string(sdkFunctionName) == "FinishReload") return FunctionPtr(&sakura::FinishReload);
		if (string(sdkFunctionName) == "Dispose") return FunctionPtr(&sakura::Dispose);

		// Solve Memory Management Utilities (Anzen) Functions
		if (string(sdkFunctionName) == "GetGlobalPointer") return FunctionPtr(&GetGlobalPointer);
		if (string(sdkFunctionName) == "SetGlobalPointer") return FunctionPtr(&SetGlobalPointer);
		if (string(sdkFunctionName) == "DeleteGlobalPointer") return FunctionPtr(&DeleteGlobalPointer);
		if (string(sdkFunctionName) == "AllocateGlobalMemory") return FunctionPtr(&AllocateGlobalMemory);
		if (string(sdkFunctionName) == "FreeGlobalMemory") return FunctionPtr(&FreeGlobalMemory);

		// Solve Global Variable Storage Utilities (Anzen) Functions
		if (string(sdkFunctionName) == "GetGlobalVariable") return FunctionPtr(&GetGlobalVariable);
		if (string(sdkFunctionName) == "SetGlobalVariable") return FunctionPtr(&SetGlobalVariable);
		if (string(sdkFunctionName) == "ClearGlobalVariables") return FunctionPtr(&ClearGlobalVariables);

		// Solve Task System Utilities Functions
		if (string(sdkFunctionName) == "InitiateTask") return FunctionPtr(&InitiateTask);
		if (string(sdkFunctionName) == "IsTaskComplete") return FunctionPtr(&IsTaskComplete);
		if (string(sdkFunctionName) == "ClearTask") return FunctionPtr(&ClearTask);

		// Solve C Scripting Utilities (Clektron) Functions
		if (string(sdkFunctionName) == "ExecuteScript") return FunctionPtr((bool(*)(StringPtr, bool))(&clektron::ExecuteScript));
		if (string(sdkFunctionName) == "ExecuteScriptFromFile") return FunctionPtr((bool(*)(StringPtr, bool))(&clektron::ExecuteScriptFromFile));

		// Invalid Function
		return nullptr;
	}
}

// Static Runtime SDK Implementation [Deprecated]
#ifdef JENOVA_SDK_STATIC
namespace jenova
{
	/*
		Following APIs Are Replicates from jenova.hpp
		And Are Only Available in Static Build of SDK.
	*/
	void Output(sdk::StringPtr fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		godot::UtilityFunctions::print(godot::String("[JENOVA-SDK] > ") + godot::String(buffer));
	}
	std::string ConvertToStdString(const godot::String& gstr)
	{
		std::string str((char*)gstr.utf8().ptr(), gstr.utf8().size());
		if (!str.empty() && str.back() == '\0') str.pop_back();
		return str;
	}
	bool RegisterRuntimeEventCallback(jenova::FunctionPointer runtimeCallback)
	{
		UtilityFunctions::push_error(
			"API Function `RegisterRuntimeEventCallback` in JenovaSDK is not available when using SDK static linking.\n"
			"Please ensure dynamic linking is enabled or refer to the SDK documentation for alternative solutions."
		);
		return false;
	}
	bool UnregisterRuntimeEventCallback(jenova::FunctionPointer runtimeCallback)
	{
		UtilityFunctions::push_error(
			"API Function `UnregisterRuntimeEventCallback` in JenovaSDK is not available when using SDK static linking.\n"
			"Please ensure dynamic linking is enabled or refer to the SDK documentation for alternative solutions."
		);
		return false;
	}
	HWND GetMainWindowNativeHandle()
	{
		return HWND(DisplayServer::get_singleton()->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE));
	}
}
#endif