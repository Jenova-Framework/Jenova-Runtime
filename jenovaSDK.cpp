
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

// Widnows SDK
#include <Windows.h>
#include <iostream>

// Godot SDK
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Jenova System SDK
#include "JenovaSDK.h"

// Namespaces
using namespace std;

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

// System SDK Implementation
namespace jenova::sdk
{
	// Helpers Utilities
	bool IsEditor()
	{
		return godot::Engine::get_singleton()->is_editor_hint();
	}
	bool IsGame()
	{
		if (IsEditor()) return false;
		if (godot::OS::get_singleton()->is_debug_build()) return false;
		return true;
	}
	godot::Node* GetNodeByPath(const godot::String& path)
	{
		godot::SceneTree* scene_tree = dynamic_cast<godot::SceneTree*>(godot::Engine::get_singleton()->get_main_loop());
		if (scene_tree) return scene_tree->get_root()->get_node<godot::Node>(godot::NodePath(path));
		return nullptr;
	}
	godot::SceneTree* GetTree()
	{
		godot::SceneTree* scene_tree = dynamic_cast<godot::SceneTree*>(godot::Engine::get_singleton()->get_main_loop());
		return scene_tree;
	}
	double GetTime()
	{
		int64_t time_msec = godot::Time::get_singleton()->get_ticks_msec();
		return static_cast<double>(time_msec) / 1000.0f;
	}
	void Alert(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		MessageBoxA(0, buffer, "[JENOVA-SDK]", 0);
	}
	godot::String Format(const char* format, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
		return godot::String(buffer);
	}
	void Output(const char* format, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
		godot::UtilityFunctions::print(godot::String("[JENOVA-SDK] > ") + godot::String(buffer));
	}
	const char* GetCStr(const godot::String& godotStr)
	{
		std::string str((char*)godotStr.utf8().ptr(), godotStr.utf8().size());
		if (!str.empty() && str.back() == '\0') str.pop_back();
		return _strdup(str.c_str());
	}
	bool SetClassIcon(const godot::String& className, const godot::Ref<godot::Texture2D> iconImage)
	{
		if (!godot::ClassDB::class_exists(className)) return false;
		godot::Ref<godot::Theme> editor_theme = godot::EditorInterface::get_singleton()->get_editor_theme();
		if (editor_theme->has_icon(className, "EditorIcons")) return false;
		editor_theme->set_icon(className, "EditorIcons", iconImage);
		return true;
	}
	godot::Error CreateSignalCallback(godot::Object* object, const godot::String& signalName, FunctionPtr callbackPtr)
	{
		return object->connect(signalName, callable_mp(memnew(EventCallback(callbackPtr)), &EventCallback::OnEventCall));
	}

	// Hot-Reloading Utilities (Sakura)
	namespace sakura
	{
		bool SupportsReload()
		{
			if (IsEditor()) return godot::EditorInterface::get_singleton()->has_method("get_open_scenes_roots");
			return true;
		}
		void PrepareReload(const godot::String& className)
		{
			// Validate
			if (!godot::ClassDB::class_exists(className)) return;
	
			// Validate Scene Tree [Required!]
			if (!GetTree()) return;

			// Deselect Nodes
			if (IsEditor()) godot::EditorInterface::get_singleton()->get_selection()->clear();

			// Get Opened Scenes
			godot::Array openedScenes;
			if (IsEditor()) openedScenes = godot::EditorInterface::get_singleton()->get_open_scenes_roots();
			else openedScenes.push_back(GetTree()->get_root());
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
		void FinishReload(const godot::String& className)
		{
			// Validate
			if (!godot::ClassDB::class_exists(className)) return;

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

				// Check for Feature
				if (!IsEditor())
				{
					godot::Dictionary versionInfo = godot::Engine::get_singleton()->get_version_info();
					if (godot::String(versionInfo["build"]) != "jenova")
					{
						godot::UtilityFunctions::push_error("[Jenova::Sakura] Runtime Hot-Reloading for GDExtension Classes Only is Supported in Jenova Editions of Godot.");
						auto* placeholderNode = memnew(godot::Node);
						nodeBackup.dummyNode->replace_by(placeholderNode, true);
						memdelete(nodeBackup.dummyNode);
						memdelete(nodeBackup.nodeBackup);
						nodeBackups.remove_at(i);
						--i;
						continue;
					}
				}

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
		void Dispose(const godot::String& className)
		{
			godot::StringName classNameStr(className);
			if (!godot::ClassDB::class_exists(classNameStr)) return;
			godot::internal::gdextension_interface_classdb_unregister_extension_class(godot::internal::library, classNameStr._native_ptr());
		}
	}

	// Memory Management Utilities (Anzen)
	void* GetGlobalPointer(MemoryID id)
	{
		auto it = globalMemoryMap.find(id);
		if (it != globalMemoryMap.end()) return it->second;
		return nullptr;
	}
	void* SetGlobalPointer(MemoryID id, void* ptr)
	{
		auto it = globalMemoryMap.find(id);
		if (it != globalMemoryMap.end()) 
		{
			void* oldPtr = it->second;
			it->second = ptr;
			return oldPtr;
		}
		else {
			globalMemoryMap[id] = ptr;
			return ptr;
		}
	}
	void DeleteGlobalPointer(MemoryID id)
	{
		globalMemoryMap.erase(id);
	}
	void* AllocateGlobalMemory(MemoryID id, size_t size)
	{
		void* mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		if (!mem) return nullptr;
		globalMemoryMap[id] = mem;
		return mem;
	}
	void FreeGlobalMemory(MemoryID id)
	{
		auto it = globalMemoryMap.find(id);
		if (it != globalMemoryMap.end())
		{
			HeapFree(GetProcessHeap(), 0, it->second);
			globalMemoryMap.erase(it);
		}
	}
}

// Jenova Runtime SDK
#include "Jenova.hpp"

// Runtime SDK Implementation
namespace jenova::sdk
{
	// Helpers Utilities
	jenova::sdk::EngineMode GetEngineMode()
	{
		return jenova::sdk::EngineMode(jenova::GlobalStorage::CurrentEngineMode);
	}
	bool CreateDirectoryMonitor(const String& directoryPath)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->AddDirectory(directoryPath)) return false;
		return true;
	}
	bool CreateFileMonitor(const String& filePath)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->AddFile(filePath)) return false;
		return true;
	}
	bool RegisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->RegisterCallback(jenova::AssetMonitor::AssetMonitorCallback(callbackPtr))) return false;
		return true;
	}
	bool UnregisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaAssetMonitor::get_singleton()) return false;
		if (!JenovaAssetMonitor::get_singleton()->UnregisterCallback(jenova::AssetMonitor::AssetMonitorCallback(callbackPtr))) return false;
		return true;
	}
	bool ReloadJenovaRuntime(RuntimeReloadMode reloadMode)
	{
		jenova::sdk::Output("ReloadJenovaRuntime -> Not Implemented Yet");
		return false;
	}
	void CreateCheckpoint(const godot::String& checkPointName)
	{
		JenovaTinyProfiler::CreateCheckpoint(AS_STD_STRING(checkPointName));
	}
	double GetCheckpointTime(const godot::String& checkPointName)
	{
		return JenovaTinyProfiler::GetCheckpointTime(AS_STD_STRING(checkPointName));
	}
	void DeleteCheckpoint(const godot::String& checkPointName)
	{
		JenovaTinyProfiler::DeleteCheckpoint(AS_STD_STRING(checkPointName));
	}
	double GetCheckpointTimeAndDispose(const godot::String& checkPointName)
	{
		return JenovaTinyProfiler::GetCheckpointTimeAndDispose(AS_STD_STRING(checkPointName));
	}
}