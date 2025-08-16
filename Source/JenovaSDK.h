
/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|                          Jenova SDK                          |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

#pragma once

// Enable Jenova SDK
#define JENOVA_SDK

// Jenova API Import/Export
#if defined(_WIN32) || defined(_WIN64)
	#define JENOVA_API_EXPORT __declspec(dllexport)
	#define JENOVA_API_IMPORT __declspec(dllimport)
#else
	#define JENOVA_API_EXPORT __attribute__((visibility("default")))
	#define JENOVA_API_IMPORT 
#endif
#ifdef JENOVA_SDK_STATIC
    #define JENOVA_API
#else
	#if defined(JENOVA_STATIC_BUILD)
		#define JENOVA_API
		#define JENOVA_C_API extern "C"
	#elif defined(JENOVA_SDK_BUILD)
		#define JENOVA_API JENOVA_API_EXPORT
		#define JENOVA_C_API extern "C" JENOVA_API_EXPORT
	#else
		#define JENOVA_API JENOVA_API_IMPORT
		#define JENOVA_C_API extern "C" JENOVA_API_IMPORT
	#endif
#endif

// Jenova Definitions
#ifndef JENOVA_VERSION
	#define JENOVA_VERSION "Unknown Version"
#endif
#ifndef JENOVA_COMPILER
	#define JENOVA_COMPILER "Unknown Compiler"
#endif

// Jenova Utilities
#define JENOVA_EXPORT			extern "C" JENOVA_API_EXPORT
#define JENOVA_CALLBACK			(void*)static_cast<void(*)(void)>([]()

// Jenova Interface
#define JENOVA_WRAPPER			static inline
#define JENOVA_INTERNAL(fn)		virtual fn

// Jenova Configuration Macros
#define JENOVA_TOOL_SCRIPT

// Jenova Script Block Macros
#define JENOVA_SCRIPT_BEGIN
#define JENOVA_SCRIPT_END

// Jenova Virtual Machine Block Macros
#define JENOVA_VM_BEGIN
#define JENOVA_VM_END

// Jenova Property Macro
#ifndef JENOVA_PROPERTY
	#define JENOVA_PROPERTY(pType, pName, pValue, ...) pType pName = pValue;
#endif

// Jenova Class Name Macro
#ifndef JENOVA_CLASS_NAME
	#define JENOVA_CLASS_NAME(className)
#endif

// Jenova Activator Macro
#ifndef JENOVA_ACTIVATOR
	#define JENOVA_ACTIVATOR(name, regfn, uregfn)   \
		static struct name##SelfActivator {         \
			inline name##SelfActivator() {          \
				RegisterBootEvent(&regfn);          \
				RegisterShutdownEvent(&uregfn);     \
			}} inline _self;
#endif

// C++ Runtime Imports
#ifndef JENOVA_SDK_BUILD
	#include <cstdarg>
	#include <string>
	#include <thread>
	#include <functional>
#endif

// GodotSDK Imports
#ifndef JENOVA_SDK_BUILD
	#include <Godot/variant/variant.hpp>
	#include <Godot/variant/string.hpp>
	#include <Godot/variant/string_name.hpp>
	#include <Godot/classes/global_constants.hpp>
	#include <Godot/classes/object.hpp>
	#include <Godot/classes/node.hpp>
	#include <Godot/classes/scene_tree.hpp>
	#include <Godot/classes/texture2d.hpp>
#endif

// Jenova SDK Implementation
namespace jenova::sdk
{
	// Enumerators
	enum class EngineMode
	{
		Editor,
		Debug,
		Runtime,
		Unknown
	};
	enum class FileSystemEvent
	{
		Added,
		Removed,
		Modified,
		RenamedOld,
		RenamedNew
	};
	enum class RuntimeReloadMode
	{
		FullReset,
		HotReload,
		ForceReload
	};
	enum class RuntimeEvent
	{
		Initialized,
		Started,
		Stopped,
		Ready,
		EnterTree,
		ExitTree,
		FrameBegin,
		FrameIdle,
		FrameEnd,
		FramePresent,
		Process,
		PhysicsProcess,
		ReceivedDebuggerMessage
	};
	enum class ClassAccess 
	{
		Core,
		Editor,
		Extension,
		Editor_Extension,
		None
	};

	// Type Definitions
	typedef void*						FunctionPtr;
	typedef void*						NativePtr;
	typedef int64_t						IntPtr;
	typedef godot::Variant				ObjectPtr;
	typedef const char*					StringPtr;
	typedef const wchar_t*				WideStringPtr;
	typedef const char*					MemoryID;
	typedef const char*					VariableID;
	typedef unsigned short				TaskID;
	typedef int short					DriverResourceID;
	typedef std::function<void()>		TaskFunction;
	typedef void*						JenovaSDKInterface;

	// Function Definitions
	typedef void(*RuntimeCallback)(const RuntimeEvent& runtimeEvent, NativePtr dataPtr, size_t dataSize);
	typedef void(*FileSystemCallback)(const godot::String& targetPath, const FileSystemEvent& fsEvent);

	// Structures
	struct Caller
	{
		// Script Caller
		const godot::Object* self;
	};

	// JenovaSDK Interface
	struct JenovaSDK
	{
		// Helpers Utilities
		JENOVA_INTERNAL(bool IsEditor());
		JENOVA_INTERNAL(bool IsGame());
		JENOVA_INTERNAL(EngineMode GetEngineMode());
		JENOVA_INTERNAL(godot::Node* GetNodeByPath(const godot::String& nodePath));
		JENOVA_INTERNAL(godot::Node* FindNodeByName(godot::Node* parent, const godot::String& name));
		JENOVA_INTERNAL(StringPtr GetNodeUniqueID(godot::Node* node));
		JENOVA_INTERNAL(godot::SceneTree* GetTree());
		JENOVA_INTERNAL(double GetTime());
		JENOVA_INTERNAL(void Alert(StringPtr fmt, va_list args));
		JENOVA_INTERNAL(godot::String Format(StringPtr format, va_list args));
		JENOVA_INTERNAL(godot::String Format(WideStringPtr format, va_list args));
		JENOVA_INTERNAL(void Output(StringPtr format, va_list args));
		JENOVA_INTERNAL(void Output(WideStringPtr format, va_list args));
		JENOVA_INTERNAL(void DebugOutput(StringPtr format, va_list args));
		JENOVA_INTERNAL(void DebugOutput(WideStringPtr format, va_list args));
		JENOVA_INTERNAL(StringPtr GetCStr(const godot::String& godotStr));
		JENOVA_INTERNAL(WideStringPtr GetWCStr(const godot::String& godotStr));
		JENOVA_INTERNAL(ObjectPtr GetObjectPointer(NativePtr obj));
		JENOVA_INTERNAL(bool SetClassIcon(const godot::String& className, const godot::Ref<godot::Texture2D> iconImage));
		JENOVA_INTERNAL(double MatchScaleFactor(double inputSize));
		JENOVA_INTERNAL(godot::Error CreateSignalCallback(godot::Object* object, const godot::String& signalName, FunctionPtr callbackPtr));
		JENOVA_INTERNAL(bool CreateDirectoryMonitor(const godot::String& directoryPath));
		JENOVA_INTERNAL(bool CreateFileMonitor(const godot::String& filePath));
		JENOVA_INTERNAL(bool RegisterFileMonitorCallback(FileSystemCallback callbackPtr));
		JENOVA_INTERNAL(bool UnregisterFileMonitorCallback(FileSystemCallback callbackPtr));
		JENOVA_INTERNAL(bool ReloadJenovaRuntime(RuntimeReloadMode reloadMode));
		JENOVA_INTERNAL(void CreateCheckpoint(const godot::String& checkPointName));
		JENOVA_INTERNAL(double GetCheckpointTime(const godot::String& checkPointName));
		JENOVA_INTERNAL(void DeleteCheckpoint(const godot::String& checkPointName));
		JENOVA_INTERNAL(double GetCheckpointTimeAndDispose(const godot::String& checkPointName));
		JENOVA_INTERNAL(bool RegisterRuntimeCallback(RuntimeCallback callbackPtr));
		JENOVA_INTERNAL(bool UnregisterRuntimeCallback(RuntimeCallback callbackPtr));

		// Graphic Utilities
		JENOVA_INTERNAL(NativePtr GetGameWindowHandle());
		JENOVA_INTERNAL(StringPtr GetRenderingDriverName());
		JENOVA_INTERNAL(NativePtr GetRenderingDriverResource(DriverResourceID resourceType));

		// Hot-Reloading Utilities (Sakura)
		JENOVA_INTERNAL(bool SupportsReload());
		JENOVA_INTERNAL(void PrepareReload(const godot::String& className));
		JENOVA_INTERNAL(void FinishReload(const godot::String& className));
		JENOVA_INTERNAL(void Dispose(const godot::String& className));

		// Memory Management Utilities (Anzen)
		JENOVA_INTERNAL(NativePtr GetGlobalPointer(MemoryID id));
		JENOVA_INTERNAL(NativePtr SetGlobalPointer(MemoryID id, NativePtr ptr));
		JENOVA_INTERNAL(void DeleteGlobalPointer(MemoryID id));
		JENOVA_INTERNAL(NativePtr AllocateGlobalMemory(MemoryID id, size_t size));
		JENOVA_INTERNAL(void FreeGlobalMemory(MemoryID id));

		// Global Variable Storage Utilities (Anzen)
		JENOVA_INTERNAL(godot::Variant GetGlobalVariable(VariableID id));
		JENOVA_INTERNAL(void SetGlobalVariable(VariableID id, godot::Variant var));
		JENOVA_INTERNAL(void ClearGlobalVariables());

		// Task System Utilities
		JENOVA_INTERNAL(TaskID InitiateTask(TaskFunction function));
		JENOVA_INTERNAL(bool IsTaskComplete(TaskID taskID));
		JENOVA_INTERNAL(void ClearTask(TaskID taskID));

		// C Scripting Utilities (Clektron)
		JENOVA_INTERNAL(bool ExecuteScript(StringPtr ctronScript, bool noEntrypoint = false));
		JENOVA_INTERNAL(bool ExecuteScriptFromFile(StringPtr ctronScriptFile, bool noEntrypoint = false));
		JENOVA_INTERNAL(bool ExecuteScript(const godot::String& ctronScript, bool noEntrypoint = false));
		JENOVA_INTERNAL(bool ExecuteScriptFromFile(const godot::String& ctronScriptFile, bool noEntrypoint = false));

		// Interface Validator
		static bool ValidateInterface(void* bridgePtr)
		{
			if (!bridgePtr) return false;
			return true;
		}
	};

	// JenovaSDK Singleton
	extern JenovaSDK* bridge;

	// JenovaSDK Interface Management
	JENOVA_C_API JenovaSDKInterface GetSDKInterface();
	JENOVA_C_API FunctionPtr GetSDKFunction(StringPtr sdkFunctionName);

	// Helpers Utilities :: Wrappers
	JENOVA_WRAPPER bool IsEditor()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->IsEditor();
	}
	JENOVA_WRAPPER bool IsGame()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->IsGame();
	}
	JENOVA_WRAPPER EngineMode GetEngineMode()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return EngineMode::Unknown;
		return bridge->GetEngineMode();
	}
	JENOVA_WRAPPER godot::Node* GetNodeByPath(const godot::String& nodePath)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetNodeByPath(nodePath);
	}
	JENOVA_WRAPPER godot::Node* FindNodeByName(godot::Node* parent, const godot::String& name)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->FindNodeByName(parent, name);
	}
	JENOVA_WRAPPER StringPtr GetNodeUniqueID(godot::Node* node)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetNodeUniqueID(node);
	}
	JENOVA_WRAPPER godot::SceneTree* GetTree()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetTree();
	}
	JENOVA_WRAPPER double GetTime()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->GetTime();
	}
	JENOVA_WRAPPER void Alert(StringPtr fmt, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, fmt);
		bridge->Alert(fmt, args);
		va_end(args);
	}
	JENOVA_WRAPPER godot::String Format(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::String();
		va_list args;
		va_start(args, format);
		godot::String result = bridge->Format(format, args);
		va_end(args);
		return result;
	}
	JENOVA_WRAPPER godot::String Format(WideStringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::String();
		va_list args;
		va_start(args, format);
		godot::String result = bridge->Format(format, args);
		va_end(args);
		return result;
	}
	JENOVA_WRAPPER void Output(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->Output(format, args);
		va_end(args);
	}
	JENOVA_WRAPPER void Output(WideStringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->Output(format, args);
		va_end(args);
	}
	JENOVA_WRAPPER void DebugOutput(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->DebugOutput(format, args);
		va_end(args);
	}
	JENOVA_WRAPPER void DebugOutput(WideStringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->DebugOutput(format, args);
		va_end(args);
	}
	JENOVA_WRAPPER StringPtr GetCStr(const godot::String& godotStr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetCStr(godotStr);
	}
	JENOVA_WRAPPER WideStringPtr GetWCStr(const godot::String& godotStr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetWCStr(godotStr);
	}
	JENOVA_WRAPPER ObjectPtr GetObjectPointer(NativePtr obj)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetObjectPointer(obj);
	}
	JENOVA_WRAPPER bool SetClassIcon(const godot::String& className, const godot::Ref<godot::Texture2D> iconImage)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->SetClassIcon(className, iconImage);
	}
	JENOVA_WRAPPER double MatchScaleFactor(double inputSize)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->MatchScaleFactor(inputSize);
	}
	JENOVA_WRAPPER godot::Error CreateSignalCallback(godot::Object* object, const godot::String& signalName, FunctionPtr callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::ERR_INVALID_PARAMETER;
		return bridge->CreateSignalCallback(object, signalName, callbackPtr);
	}
	JENOVA_WRAPPER bool CreateDirectoryMonitor(const godot::String& directoryPath)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->CreateDirectoryMonitor(directoryPath);
	}
	JENOVA_WRAPPER bool CreateFileMonitor(const godot::String& filePath)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->CreateFileMonitor(filePath);
	}
	JENOVA_WRAPPER bool RegisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->RegisterFileMonitorCallback(callbackPtr);
	}
	JENOVA_WRAPPER bool UnregisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->UnregisterFileMonitorCallback(callbackPtr);
	}
	JENOVA_WRAPPER bool ReloadJenovaRuntime(RuntimeReloadMode reloadMode)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->ReloadJenovaRuntime(reloadMode);
	}
	JENOVA_WRAPPER void CreateCheckpoint(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->CreateCheckpoint(checkPointName);
	}
	JENOVA_WRAPPER double GetCheckpointTime(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->GetCheckpointTime(checkPointName);
	}
	JENOVA_WRAPPER void DeleteCheckpoint(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->DeleteCheckpoint(checkPointName);
	}
	JENOVA_WRAPPER double GetCheckpointTimeAndDispose(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->GetCheckpointTimeAndDispose(checkPointName);
	}
	JENOVA_WRAPPER bool RegisterRuntimeCallback(RuntimeCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->RegisterRuntimeCallback(callbackPtr);
	}
	JENOVA_WRAPPER bool UnregisterRuntimeCallback(RuntimeCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->UnregisterRuntimeCallback(callbackPtr);
	}

	// Graphic Utilities :: Wrappers
	JENOVA_WRAPPER NativePtr GetGameWindowHandle()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetGameWindowHandle();
	}
	JENOVA_WRAPPER StringPtr GetRenderingDriverName()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetRenderingDriverName();
	}
	JENOVA_WRAPPER NativePtr GetRenderingDriverResource(DriverResourceID resourceType)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetRenderingDriverResource(resourceType);
	}

	// Hot-Reloading Utilities (Sakura) :: Wrappers
	namespace sakura
	{
		JENOVA_WRAPPER bool SupportsReload()
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->SupportsReload();
		}
		JENOVA_WRAPPER void PrepareReload(const godot::String& className)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return;
			bridge->PrepareReload(className);
		}
		JENOVA_WRAPPER void FinishReload(const godot::String& className)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return;
			bridge->FinishReload(className);
		}
		JENOVA_WRAPPER void Dispose(const godot::String& className)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return;
			bridge->Dispose(className);
		}
	}

	// Memory Management Utilities (Anzen) :: Wrappers
	JENOVA_WRAPPER NativePtr GetGlobalPointer(MemoryID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetGlobalPointer(id);
	}
	JENOVA_WRAPPER NativePtr SetGlobalPointer(MemoryID id, NativePtr ptr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->SetGlobalPointer(id, ptr);
	}
	JENOVA_WRAPPER void DeleteGlobalPointer(MemoryID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->DeleteGlobalPointer(id);
	}
	JENOVA_WRAPPER NativePtr AllocateGlobalMemory(MemoryID id, size_t size)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->AllocateGlobalMemory(id, size);
	}
	JENOVA_WRAPPER void FreeGlobalMemory(MemoryID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->FreeGlobalMemory(id);
	}

	// Global Variable Storage Utilities (Anzen) :: Wrappers
	JENOVA_WRAPPER godot::Variant GetGlobalVariable(VariableID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::Variant();
		return bridge->GetGlobalVariable(id);
	}
	JENOVA_WRAPPER void SetGlobalVariable(VariableID id, godot::Variant var)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->SetGlobalVariable(id, var);
	}
	JENOVA_WRAPPER void ClearGlobalVariables()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->ClearGlobalVariables();
	}

	// Task System Utilities :: Wrappers
	JENOVA_WRAPPER TaskID InitiateTask(TaskFunction function)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return TaskID();
		return bridge->InitiateTask(function);
	}
	JENOVA_WRAPPER bool IsTaskComplete(TaskID taskID)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->IsTaskComplete(taskID);
	}
	JENOVA_WRAPPER void ClearTask(TaskID taskID)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->ClearTask(taskID);
	}

	// C Scripting Utilities (Clektron)
	namespace clektron
	{
		JENOVA_WRAPPER bool ExecuteScript(StringPtr ctronScript, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScript, noEntrypoint);
		}
		JENOVA_WRAPPER bool ExecuteScriptFromFile(StringPtr ctronScriptFile, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScriptFile, noEntrypoint);
		}
		JENOVA_WRAPPER bool ExecuteScript(const godot::String& ctronScript, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScript, noEntrypoint);
		}
		JENOVA_WRAPPER bool ExecuteScriptFromFile(const godot::String& ctronScriptFile, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScriptFile, noEntrypoint);
		}
	}

	// Template Helpers
	template <typename T> T* GetSelf(Caller* caller)
	{
		return (T*)(caller->self);
	}
	template <typename T> T* GetNode(const godot::String& nodePath)
	{ 
		return static_cast<T*>(GetNodeByPath(nodePath));
	}
	template <typename T> T* FindNode(godot::Node* parent, const godot::String& nodeName)
	{
		return static_cast<T*>(FindNodeByName(parent, nodeName));
	}
	template <typename T> T* GlobalPointer(MemoryID id)
	{ 
		return static_cast<T*>(GetGlobalPointer(id));
	}
	template <typename T> T GlobalGet(MemoryID id)
	{
		return *(static_cast<T*>(GetGlobalPointer(id)));
	}
	template <typename T> void GlobalSet(MemoryID id, const T& newValue)
	{
		T* ptr = static_cast<T*>(GetGlobalPointer(id));
		if (ptr) *ptr = newValue;
	}
	template <typename T> T GlobalVariable(VariableID id)
	{
		return T(GetGlobalVariable(id));
	}
	template <typename T> T* GetObjectFromIntPtr(IntPtr ptr)
	{
		return reinterpret_cast<T*>(ptr);
	}
	template <typename T> T* GetObjectFromIntPtr(godot::Variant variantPtr)
	{
		return reinterpret_cast<T*>(IntPtr(variantPtr));
	}

	// Internal Module Functions
	bool RegisterBootEvent(jenova::sdk::FunctionPtr funcionPtr, int index = -1);
	bool RegisterShutdownEvent(jenova::sdk::FunctionPtr funcionPtr, int index = -1);
	bool UnregisterBootEvent(jenova::sdk::FunctionPtr funcionPtr);
	bool UnregisterShutdownEvent(jenova::sdk::FunctionPtr funcionPtr);
}