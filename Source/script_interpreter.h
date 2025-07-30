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

// Jenova Interpreter Definitions
class JenovaInterpreter
{
// Module Management API
public:
    static void BootInterpreter();
    static bool InitializeInterpreter();
    static bool IsInterpreterInitialized();
    static bool ReleaseInterpreter();
    static bool LoadModule(const uint8_t* moduleDataPtr, const size_t moduleSize, const jenova::SerializedData& metaData);
    static bool LoadModule(const jenova::BuildResult& buildResult);
    static bool ReloadModule(const uint8_t* moduleDataPtr, const size_t moduleSize, const jenova::SerializedData& metaData);
    static bool ReloadModule(const jenova::BuildResult& buildResult);
    static bool UnloadModule(const jenova::ModuleUnloadStage& unloadStage);
    static bool LoadDebugSymbol(const std::string symbolFilePath);
    static intptr_t GetModuleBaseAddress();
    static jenova::FunctionList GetFunctionsList(const std::string& scriptUID);
    static jenova::FunctionAddress GetFunctionAddress(const std::string& functionName, const std::string& scriptUID);
    static jenova::ParameterTypeList GetFunctionParameters(const std::string& functionName, const std::string& scriptUID);
    static std::string GetFunctionReturn(const std::string& functionName, const std::string& scriptUID);
    static uintptr_t GetResolvedParameterPointer(const godot::Object* objectPtr, const godot::Variant* functionParameter, const std::string& parameterType);
    static bool IsFunctionReturnable(const std::string& returnType);
    static jenova::ScriptFunctionContainer GetFunctionContainer(const std::string& scriptUID);
    static jenova::ScriptPropertyContainer GetPropertyContainer(const std::string& scriptUID);
    static Variant CallFunction(const godot::Object* objectPtr, const std::string& functionName, std::string& scriptUID, const Variant** functionParameters, const int functionParametersCount);
    static void SetExecutionState(bool executionState);
    static Variant GenerateFunctionCallError(const std::string& functionName, const String& errorReason);
    static jenova::SerializedData GenerateModuleMetadata(const std::string& mapFilePath, const jenova::ModuleList& scriptModules, const jenova::BuildResult& buildResult);
    static bool UpdateConfigurationsFromMetaData(const jenova::SerializedData& metaData);
    static bool UpdatePropertyStorageFromMetaData();
    static bool FlushPropertyStorage();
    static jenova::PropertyList GetPropertiesList(std::string& scriptUID);
    static std::string GetPropertyType(const std::string& propertyName, std::string& scriptUID);
    static jenova::PropertyAddress GetPropertyAddress(const std::string& propertyName, std::string& scriptUID);
    static jenova::PropertyPointer GetPropertyPointer(const String& propertyName, const String& scriptUID);
    static bool SetPropertyValueFromVariant(const String& propertyName, const Variant& propertyValue, const String& scriptUID);
    static jenova::InterpreterBackend GetInterpreterBackend();
    static void SetInterpreterBackend(jenova::InterpreterBackend newBackend);
    static jenova::FunctionPointer SolveVirtualFunction(jenova::ModuleHandle moduleHandle, const char* functionName);
    static void SetDebugModeExecutionState(bool debugModeState);
    static jenova::ModuleHandle LoadShellModule(const uint8_t* moduleDataPtr, const size_t moduleSize);

// Module Database API
public:
    static bool CreateModuleDatabase(const std::string& moduleDatabaseName, const uint8_t* moduleDataPtr, const size_t moduleSize, const jenova::SerializedData& metaData);
    static bool CreateModuleDatabase(const std::string& moduleDatabaseName, const jenova::BuildResult& buildResult);
    static bool DeployFromDatabase(const std::string& moduleDatabaseName);
    static bool IsDatabaseAvailable(const std::string& moduleDatabaseName);

private:
    static inline bool                          isInitialized           = false;
    static inline bool                          allowExecution          = false;
    static inline bool                          isExecuting             = false;
    static inline jenova::ModuleHandle          moduleHandle            = nullptr;
    static inline jenova::ModuleAddress         moduleBaseAddress       = 0;
    static inline jenova::json_t                moduleMetaData          = "{}";
    static inline size_t                        moduleBinarySize        = 0;
    static inline bool                          hasDebugInformation     = false;
    static inline bool                          executeInDebugMode      = false;
    static inline std::string                   moduleDiskPath          = "";
    static inline jenova::InterpreterBackend    interpreterBackend      = jenova::InterpreterBackend::TinyCC;
    static inline jenova::PointerStorage        propertyStorage         = jenova::PointerStorage();
    static inline jenova::PropertySetMethod     propertySetMethod       = jenova::PropertySetMethod::DirectAssign;

};