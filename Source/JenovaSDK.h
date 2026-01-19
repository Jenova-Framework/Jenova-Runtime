
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

// Jenova Utilities
#define JENOVA_EXPORT				extern "C" JENOVA_API_EXPORT
#define JENOVA_CALLBACK				(void*)static_cast<void(*)(void)>([]()

// Jenova API Interface
#define JNVAPI_WRAPPER				static inline
#define JNVAPI_INTERNAL(fn)			virtual fn

// Jenova Configuration Macros
#define JENOVA_TOOL_SCRIPT

// Jenova Profiler Macros
#define JENOVA_SCRIPT_RECORD(n,t)	sentinel::CommitScriptRecord(__FILE__, n, t)

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
	#define JENOVA_ACTIVATOR(name, regfn, uregfn)					\
		static struct name##SelfActivator {							\
			inline name##SelfActivator() {							\
				RegisterBootEvent((FunctionPtr)&regfn);				\
				RegisterShutdownEvent((FunctionPtr)&uregfn);		\
			}} inline name##_self;
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
		/* Must Match to Jenova Runtime */
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

	// Caller Structure
	struct Caller
	{
		// Script Caller
		const godot::Object* self;
	};

	// Class Wrappers
	struct Console
	{
		godot::Node* consoleNode = nullptr;
		Console(godot::Node* _consoleNode) : consoleNode(_consoleNode) {};
		bool Execute(const godot::String& command)
		{
			return bool(consoleNode->call("execute", command));
		}
		void Log(const godot::String& logMessage, godot::Color logColor = godot::Color(12, 34, 56))
		{
			if (logColor == godot::Color(12, 34, 56)) consoleNode->call("log", logMessage);
			else consoleNode->call("logc", logMessage, logColor);
		}
		void Error(const godot::String& errorMessage)
		{
			consoleNode->call("error", errorMessage);
		}
		void Flush()
		{
			consoleNode->call("flush");
		}
	};

	// JenovaSDK Interface
	struct JenovaSDK
	{
		// Helpers Utilities
		JNVAPI_INTERNAL(bool IsEditor());
		JNVAPI_INTERNAL(bool IsGame());
		JNVAPI_INTERNAL(EngineMode GetEngineMode());
		JNVAPI_INTERNAL(godot::Node* GetNodeByPath(const godot::String& nodePath));
		JNVAPI_INTERNAL(godot::Node* FindNodeByName(godot::Node* parent, const godot::String& name));
		JNVAPI_INTERNAL(StringPtr GetNodeUniqueID(godot::Node* node));
		JNVAPI_INTERNAL(godot::SceneTree* GetTree());
		JNVAPI_INTERNAL(double GetTime());
		JNVAPI_INTERNAL(void Alert(StringPtr format, va_list args));
		JNVAPI_INTERNAL(godot::String Format(StringPtr format, va_list args));
		JNVAPI_INTERNAL(godot::String Format(WideStringPtr format, va_list args));
		JNVAPI_INTERNAL(void Output(StringPtr format, va_list args));
		JNVAPI_INTERNAL(void Output(WideStringPtr format, va_list args));
		JNVAPI_INTERNAL(void DebugOutput(StringPtr format, va_list args));
		JNVAPI_INTERNAL(void DebugOutput(WideStringPtr format, va_list args));
		JNVAPI_INTERNAL(StringPtr GetCStr(const godot::String& godotStr));
		JNVAPI_INTERNAL(WideStringPtr GetWCStr(const godot::String& godotStr));
		JNVAPI_INTERNAL(ObjectPtr GetObjectPointer(NativePtr obj));
		JNVAPI_INTERNAL(bool SetClassIcon(const godot::String& className, const godot::Ref<godot::Texture2D> iconImage));
		JNVAPI_INTERNAL(double MatchScaleFactor(double inputSize));
		JNVAPI_INTERNAL(godot::Error CreateSignalCallback(godot::Object* object, const godot::String& signalName, FunctionPtr callbackPtr));
		JNVAPI_INTERNAL(bool CreateDirectoryMonitor(const godot::String& directoryPath));
		JNVAPI_INTERNAL(bool CreateFileMonitor(const godot::String& filePath));
		JNVAPI_INTERNAL(bool RegisterFileMonitorCallback(FileSystemCallback callbackPtr));
		JNVAPI_INTERNAL(bool UnregisterFileMonitorCallback(FileSystemCallback callbackPtr));
		JNVAPI_INTERNAL(bool ReloadJenovaRuntime(RuntimeReloadMode reloadMode));
		JNVAPI_INTERNAL(void CreateCheckpoint(const godot::String& checkPointName));
		JNVAPI_INTERNAL(double GetCheckpointTime(const godot::String& checkPointName));
		JNVAPI_INTERNAL(void DeleteCheckpoint(const godot::String& checkPointName));
		JNVAPI_INTERNAL(double GetCheckpointTimeAndDispose(const godot::String& checkPointName));
		JNVAPI_INTERNAL(godot::String GetPackageRepositoryPath(bool globalize = false));
		JNVAPI_INTERNAL(bool RegisterRuntimeCallback(RuntimeCallback callbackPtr));
		JNVAPI_INTERNAL(bool UnregisterRuntimeCallback(RuntimeCallback callbackPtr));

		// Graphic Utilities
		JNVAPI_INTERNAL(NativePtr GetGameWindowHandle());
		JNVAPI_INTERNAL(StringPtr GetRenderingDriverName());
		JNVAPI_INTERNAL(NativePtr GetRenderingDriverResource(DriverResourceID resourceType));

		// Hot-Reloading Utilities (Sakura)
		JNVAPI_INTERNAL(bool SupportsReload());
		JNVAPI_INTERNAL(void PrepareReload(const godot::String& className));
		JNVAPI_INTERNAL(void FinishReload(const godot::String& className));
		JNVAPI_INTERNAL(void Dispose(const godot::String& className));

		// Memory Management Utilities (Anzen)
		JNVAPI_INTERNAL(NativePtr GetGlobalPointer(MemoryID id));
		JNVAPI_INTERNAL(NativePtr SetGlobalPointer(MemoryID id, NativePtr ptr));
		JNVAPI_INTERNAL(void DeleteGlobalPointer(MemoryID id));
		JNVAPI_INTERNAL(NativePtr AllocateGlobalMemory(MemoryID id, size_t size));
		JNVAPI_INTERNAL(void FreeGlobalMemory(MemoryID id));

		// Global Variable Storage Utilities (Anzen)
		JNVAPI_INTERNAL(godot::Variant GetGlobalVariable(VariableID id));
		JNVAPI_INTERNAL(void SetGlobalVariable(VariableID id, godot::Variant var));
		JNVAPI_INTERNAL(void ClearGlobalVariables());

		// Task System Utilities
		JNVAPI_INTERNAL(TaskID InitiateTask(TaskFunction function));
		JNVAPI_INTERNAL(bool IsTaskComplete(TaskID taskID));
		JNVAPI_INTERNAL(void ClearTask(TaskID taskID));

		// C Scripting Utilities (Clektron)
		JNVAPI_INTERNAL(bool ExecuteScript(StringPtr ctronScript, bool noEntrypoint = false));
		JNVAPI_INTERNAL(bool ExecuteScriptFromFile(StringPtr ctronScriptFile, bool noEntrypoint = false));
		JNVAPI_INTERNAL(bool BindSymbol(FunctionPtr symbolPtr, StringPtr symbolName, StringPtr returnType, int paramCount, va_list args));
		JNVAPI_INTERNAL(bool ExecuteScript(const godot::String& ctronScript, bool noEntrypoint = false));
		JNVAPI_INTERNAL(bool ExecuteScriptFromFile(const godot::String& ctronScriptFile, bool noEntrypoint = false));
		JNVAPI_INTERNAL(bool BindSymbol(FunctionPtr symbolPtr, const godot::String& symbolName, const godot::String& returnType, int paramCount, va_list args));

		// Profiling Utilities (Sentinel)
		JNVAPI_INTERNAL(bool IsProfilerEnabled());
		JNVAPI_INTERNAL(bool CommitRecord(StringPtr recordName, double recordTime));
		JNVAPI_INTERNAL(bool CommitScriptRecord(StringPtr fileName, StringPtr recordName, double recordTime));

		// Interface Validator
		static bool ValidateInterface(NativePtr bridgePtr)
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
	JNVAPI_WRAPPER bool IsEditor()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->IsEditor();
	}
	JNVAPI_WRAPPER bool IsGame()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->IsGame();
	}
	JNVAPI_WRAPPER EngineMode GetEngineMode()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return EngineMode::Unknown;
		return bridge->GetEngineMode();
	}
	JNVAPI_WRAPPER godot::Node* GetNodeByPath(const godot::String& nodePath)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetNodeByPath(nodePath);
	}
	JNVAPI_WRAPPER godot::Node* FindNodeByName(godot::Node* parent, const godot::String& name)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->FindNodeByName(parent, name);
	}
	JNVAPI_WRAPPER StringPtr GetNodeUniqueID(godot::Node* node)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetNodeUniqueID(node);
	}
	JNVAPI_WRAPPER godot::SceneTree* GetTree()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetTree();
	}
	JNVAPI_WRAPPER double GetTime()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->GetTime();
	}
	JNVAPI_WRAPPER void Alert(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->Alert(format, args);
		va_end(args);
	}
	JNVAPI_WRAPPER godot::String Format(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::String();
		va_list args;
		va_start(args, format);
		godot::String result = bridge->Format(format, args);
		va_end(args);
		return result;
	}
	JNVAPI_WRAPPER godot::String Format(WideStringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::String();
		va_list args;
		va_start(args, format);
		godot::String result = bridge->Format(format, args);
		va_end(args);
		return result;
	}
	JNVAPI_WRAPPER void Output(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->Output(format, args);
		va_end(args);
	}
	JNVAPI_WRAPPER void Output(WideStringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->Output(format, args);
		va_end(args);
	}
	JNVAPI_WRAPPER void DebugOutput(StringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->DebugOutput(format, args);
		va_end(args);
	}
	JNVAPI_WRAPPER void DebugOutput(WideStringPtr format, ...)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		va_list args;
		va_start(args, format);
		bridge->DebugOutput(format, args);
		va_end(args);
	}
	JNVAPI_WRAPPER StringPtr GetCStr(const godot::String& godotStr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetCStr(godotStr);
	}
	JNVAPI_WRAPPER WideStringPtr GetWCStr(const godot::String& godotStr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetWCStr(godotStr);
	}
	JNVAPI_WRAPPER ObjectPtr GetObjectPointer(NativePtr obj)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetObjectPointer(obj);
	}
	JNVAPI_WRAPPER bool SetClassIcon(const godot::String& className, const godot::Ref<godot::Texture2D> iconImage)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->SetClassIcon(className, iconImage);
	}
	JNVAPI_WRAPPER double MatchScaleFactor(double inputSize)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->MatchScaleFactor(inputSize);
	}
	JNVAPI_WRAPPER godot::Error CreateSignalCallback(godot::Object* object, const godot::String& signalName, FunctionPtr callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::ERR_INVALID_PARAMETER;
		return bridge->CreateSignalCallback(object, signalName, callbackPtr);
	}
	JNVAPI_WRAPPER bool CreateDirectoryMonitor(const godot::String& directoryPath)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->CreateDirectoryMonitor(directoryPath);
	}
	JNVAPI_WRAPPER bool CreateFileMonitor(const godot::String& filePath)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->CreateFileMonitor(filePath);
	}
	JNVAPI_WRAPPER bool RegisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->RegisterFileMonitorCallback(callbackPtr);
	}
	JNVAPI_WRAPPER bool UnregisterFileMonitorCallback(FileSystemCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->UnregisterFileMonitorCallback(callbackPtr);
	}
	JNVAPI_WRAPPER bool ReloadJenovaRuntime(RuntimeReloadMode reloadMode)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->ReloadJenovaRuntime(reloadMode);
	}
	JNVAPI_WRAPPER void CreateCheckpoint(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->CreateCheckpoint(checkPointName);
	}
	JNVAPI_WRAPPER double GetCheckpointTime(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->GetCheckpointTime(checkPointName);
	}
	JNVAPI_WRAPPER void DeleteCheckpoint(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->DeleteCheckpoint(checkPointName);
	}
	JNVAPI_WRAPPER double GetCheckpointTimeAndDispose(const godot::String& checkPointName)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return 0.0;
		return bridge->GetCheckpointTimeAndDispose(checkPointName);
	}
	JNVAPI_WRAPPER godot::String GetPackageRepositoryPath(bool globalize = false)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::String();
		return bridge->GetPackageRepositoryPath(globalize);
	}
	JNVAPI_WRAPPER bool RegisterRuntimeCallback(RuntimeCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->RegisterRuntimeCallback(callbackPtr);
	}
	JNVAPI_WRAPPER bool UnregisterRuntimeCallback(RuntimeCallback callbackPtr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->UnregisterRuntimeCallback(callbackPtr);
	}

	// Graphic Utilities :: Wrappers
	JNVAPI_WRAPPER NativePtr GetGameWindowHandle()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetGameWindowHandle();
	}
	JNVAPI_WRAPPER StringPtr GetRenderingDriverName()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetRenderingDriverName();
	}
	JNVAPI_WRAPPER NativePtr GetRenderingDriverResource(DriverResourceID resourceType)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetRenderingDriverResource(resourceType);
	}

	// Hot-Reloading Utilities (Sakura) :: Wrappers
	namespace sakura
	{
		JNVAPI_WRAPPER bool SupportsReload()
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->SupportsReload();
		}
		JNVAPI_WRAPPER void PrepareReload(const godot::String& className)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return;
			bridge->PrepareReload(className);
		}
		JNVAPI_WRAPPER void FinishReload(const godot::String& className)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return;
			bridge->FinishReload(className);
		}
		JNVAPI_WRAPPER void Dispose(const godot::String& className)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return;
			bridge->Dispose(className);
		}
	}

	// Memory Management Utilities (Anzen) :: Wrappers
	JNVAPI_WRAPPER NativePtr GetGlobalPointer(MemoryID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->GetGlobalPointer(id);
	}
	JNVAPI_WRAPPER NativePtr SetGlobalPointer(MemoryID id, NativePtr ptr)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->SetGlobalPointer(id, ptr);
	}
	JNVAPI_WRAPPER void DeleteGlobalPointer(MemoryID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->DeleteGlobalPointer(id);
	}
	JNVAPI_WRAPPER NativePtr AllocateGlobalMemory(MemoryID id, size_t size)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return nullptr;
		return bridge->AllocateGlobalMemory(id, size);
	}
	JNVAPI_WRAPPER void FreeGlobalMemory(MemoryID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->FreeGlobalMemory(id);
	}

	// Global Variable Storage Utilities (Anzen) :: Wrappers
	JNVAPI_WRAPPER godot::Variant GetGlobalVariable(VariableID id)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return godot::Variant();
		return bridge->GetGlobalVariable(id);
	}
	JNVAPI_WRAPPER void SetGlobalVariable(VariableID id, godot::Variant var)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->SetGlobalVariable(id, var);
	}
	JNVAPI_WRAPPER void ClearGlobalVariables()
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->ClearGlobalVariables();
	}

	// Task System Utilities :: Wrappers
	JNVAPI_WRAPPER TaskID InitiateTask(TaskFunction function)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return TaskID();
		return bridge->InitiateTask(function);
	}
	JNVAPI_WRAPPER bool IsTaskComplete(TaskID taskID)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return false;
		return bridge->IsTaskComplete(taskID);
	}
	JNVAPI_WRAPPER void ClearTask(TaskID taskID)
	{
		if (!JenovaSDK::ValidateInterface(bridge)) return;
		bridge->ClearTask(taskID);
	}

	// C Scripting Utilities (Clektron)
	namespace clektron
	{
		JNVAPI_WRAPPER bool ExecuteScript(StringPtr ctronScript, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScript, noEntrypoint);
		}
		JNVAPI_WRAPPER bool ExecuteScriptFromFile(StringPtr ctronScriptFile, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScriptFile, noEntrypoint);
		}
		JNVAPI_WRAPPER bool BindSymbol(FunctionPtr symbolPtr, StringPtr symbolName, StringPtr returnType = "void", int paramCount = 0, ...)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			va_list args;
			va_start(args, paramCount);
			bool result = bridge->BindSymbol(symbolPtr, symbolName, returnType, paramCount, args);
			va_end(args);
			return result;
		}
		JNVAPI_WRAPPER bool ExecuteScript(const godot::String& ctronScript, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScript, noEntrypoint);
		}
		JNVAPI_WRAPPER bool ExecuteScriptFromFile(const godot::String& ctronScriptFile, bool noEntrypoint = false)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->ExecuteScript(ctronScriptFile, noEntrypoint);
		}
		JNVAPI_WRAPPER bool BindSymbol(FunctionPtr symbolPtr, const godot::String& symbolName, const godot::String& returnType, int paramCount = 0, ...)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			va_list args;
			va_start(args, paramCount);
			bool result = bridge->BindSymbol(symbolPtr, symbolName, returnType, paramCount, args);
			va_end(args);
			return result;
		}
	}

	// Profiling Utilities (Sentinel)
	namespace sentinel
	{
		JNVAPI_WRAPPER bool IsProfilerEnabled()
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->IsProfilerEnabled();
		}
		JNVAPI_WRAPPER bool CommitRecord(StringPtr recordName, double recordTime = 0)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->CommitRecord(recordName, recordTime);
		}
		JNVAPI_WRAPPER bool CommitScriptRecord(StringPtr fileName, StringPtr recordName, double recordTime)
		{
			if (!JenovaSDK::ValidateInterface(bridge)) return false;
			return bridge->CommitScriptRecord(fileName, recordName, recordTime);
		}
	}

	// Template Helpers
	template <typename T> T* GetSelf(Caller* caller)
	{
		return (T*)(caller->self);
	}
	template <typename T> T* GetSelf(godot::Variant self)
	{
		return godot::Object::cast_to<T>(self);
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
	template <typename T> T* Instantiate()
	{
		godot::String typeName(typeid(T).name());
		typeName = typeName.substr(typeName.find("::") + 2);
		return godot::Object::cast_to<T>(godot::ClassDB::instantiate(typeName));
	}
	template <typename T> T* Instantiate(const godot::String& className)
	{
		return godot::Object::cast_to<T>(godot::ClassDB::instantiate(className));
	}
	template <typename T> godot::Ref<T> InstantiateAsRef()
	{
		godot::String typeName(typeid(T).name());
		typeName = typeName.substr(typeName.find("::") + 2);
		return godot::Ref<T>(godot::Object::cast_to<T>(godot::ClassDB::instantiate(typeName)));
	}
	template <typename T> godot::Ref<T> InstantiateAsRef(const godot::String& className)
	{
		return godot::Ref<T>(godot::Object::cast_to<T>(godot::ClassDB::instantiate(className)));
	}

	// Internal Module Functions
	bool RegisterBootEvent(jenova::sdk::FunctionPtr funcionPtr, int index = -1);
	bool RegisterShutdownEvent(jenova::sdk::FunctionPtr funcionPtr, int index = -1);
	bool UnregisterBootEvent(jenova::sdk::FunctionPtr funcionPtr);
	bool UnregisterShutdownEvent(jenova::sdk::FunctionPtr funcionPtr);
}