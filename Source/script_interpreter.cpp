
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

// Jenova Loader
#include "Jenova.Loader.h"

// AsmJIT
#define ASMJIT_STATIC
#include <AsmJIT/asmjit.h>

// Tiny C Compiler
#include <TinyCC/libtcc.h>

// Helper Macros
#define RESOLVE_PARAMETER(index) JenovaInterpreter::GetResolvedParameterPointer(objectPtr, functionParameters[index], functionParametersType[index + parameterOffset])

// Jenova Interpreter Implementation :: Boot
void JenovaInterpreter::BootInterpreter()
{
    // Initialize Interpreter
    if (!JenovaInterpreter::IsInterpreterInitialized())
    {
        if (!JenovaInterpreter::InitializeInterpreter())
        {
            jenova::Warning("Jenova Interpreter", "Jenova Interpreter Failed to Initialize!");
            jenova::ExitWithCode(jenova::ErrorCode::INTERPRETER_INIT_FAILED);
        }
    }

    // Load Module From Database
    if (JenovaInterpreter::IsDatabaseAvailable(jenova::GlobalSettings::DefaultModuleDatabaseFile))
    {
        if (!JenovaInterpreter::DeployFromDatabase(jenova::GlobalSettings::DefaultModuleDatabaseFile))
        {
            jenova::Warning("Jenova Interpreter", "Module Cache Cannot Be Deployed, Possible Corruption, Rebuild Project.");
        }
    }
    else
    {
        jenova::Warning("Jenova Interpreter", "Module Cache Cannot Be Found, Rebuild Project.");
    }
}

// Jenova Interpreter Implementation :: Initialization/Release
bool JenovaInterpreter::InitializeInterpreter()
{
    // Already Initialized
    if (isInitialized) return true;
    
    // Initialize Memory Module Loader
    if (!JenovaLoader::Initialize()) return false;

    // All Good
    isInitialized = true;
    return true;
}
bool JenovaInterpreter::IsInterpreterInitialized()
{
    return isInitialized;
}
bool JenovaInterpreter::ReleaseInterpreter()
{
    return true;
    // Doesn't Require Clean Up In Debug Mode
    if (executeInDebugMode) return true;

    // It's Not Initialized
    if (!isInitialized) return false;

    // Initialize Memory Module Loader
    if (!JenovaLoader::Release()) return false;

    // All Good
    return true;
}

// Jenova Interpreter Implementation :: Module Management
bool JenovaInterpreter::LoadModule(const uint8_t* moduleDataPtr, const size_t moduleSize, const jenova::SerializedData& metaData)
{
    // Check If A Module Is Already Loaded
    if (moduleBaseAddress) return false;

    // Update Metadata And Configuration
    if (!JenovaInterpreter::UpdateConfigurationsFromMetaData(metaData))
    {
        jenova::Error("Jenova Interpreter", "Failed to Update Interpreter Configurations from Metadata.");
        return false;
    }

    // Create Loader Flags
    jenova::LoaderFlags loaderFlags = 0;
    if (executeInDebugMode && !QUERY_ENGINE_MODE(Editor)) loaderFlags |= jenova::LoaderFlag::LoadInDebugMode;

    // Load And Map Module to Memory
    if (hasDebugInformation)
    {
        // Load Module As Virtual
        moduleHandle = JenovaLoader::LoadModuleAsVirtual((void*)moduleDataPtr, moduleSize, "Jenova.Module.dll", moduleDiskPath.c_str(), loaderFlags);

        // Load Debug Symbol If MSE Disabled
        if (!jenova::GlobalStorage::UseManagedSafeExecution)
        {
            jenova::LoadSymbolForModule(jenova::GetCurrentProcessHandle(), jenova::LongWord(moduleHandle), moduleDiskPath + "\\Jenova.Module.pdb", moduleSize);
        }
    }
    else
    {
        // Load Module As Regular
        moduleHandle = JenovaLoader::LoadModule((void*)moduleDataPtr, moduleSize, loaderFlags);
    }
    if (!moduleHandle) return false;

    // Get Module Base Address
    moduleBaseAddress = JenovaLoader::GetModuleBaseAddress(moduleHandle);
    if (!moduleBaseAddress) return false;

    // Update Property Storage From Metadata
    if (!JenovaInterpreter::UpdatePropertyStorageFromMetaData())
    {
        jenova::Error("Jenova Interpreter", "Failed to Update Interpreter Property Database from Metadata.");
        return false;
    }

    // Resolve And Load Addon Modules
    if (!jenova::ResolveAndLoadAddonModulesAtRuntime())
    {
        jenova::Error("Jenova Interpreter", "Failed to Resolve and Load Addon Modules.");
        return false;
    }

    // Solve Functions Inside Module
    if(!jenova::InitializeExtensionModule("InitializeJenovaModule", moduleHandle, jenova::ModuleCallMode::Virtual))
    {
        jenova::Error("Jenova Interpreter", "Failed to Initialize Jenova Module API Solver.");
        return false;
    }

    // Call Module Boot Event If Exists
    if (!jenova::CallModuleEvent(jenova::GlobalSettings::JenovaModuleBootEventName, moduleHandle, jenova::ModuleCallMode::Virtual))
    {
        jenova::Warning("Jenova Interpreter", "Module Boot Event Failed. Unexpected Behaviors May Occur.");
    }

    // Call Module Internal Boot Event
    if (!jenova::CallModuleEvent(jenova::Format("_%s", jenova::GlobalSettings::JenovaModuleBootEventName), moduleHandle, jenova::ModuleCallMode::Virtual))
    {
        jenova::Warning("Jenova Interpreter", "Module Internal Boot Event Failed. Unexpected Behaviors May Occur.");
    }

    // Enable Execution
    allowExecution = true;

    // All Good
	return true;
}
bool JenovaInterpreter::LoadModule(const jenova::BuildResult& buildResult)
{
    return LoadModule(buildResult.builtModuleData.data(), buildResult.builtModuleData.size(), buildResult.moduleMetaData);
}
bool JenovaInterpreter::ReloadModule(const uint8_t* moduleDataPtr, const size_t moduleSize, const jenova::SerializedData& metaData)
{
    // Reload Not Supported In Debug Mode
    if (executeInDebugMode) return false;

    // Unload Module
    if (!UnloadModule(jenova::ModuleUnloadStage::UnloadModuleToReload)) return false;

    // Load Module
    return LoadModule(moduleDataPtr, moduleSize, metaData);
}
bool JenovaInterpreter::ReloadModule(const jenova::BuildResult& buildResult)
{
    return ReloadModule(buildResult.builtModuleData.data(), buildResult.builtModuleData.size(), buildResult.moduleMetaData);
}
bool JenovaInterpreter::UnloadModule(const jenova::ModuleUnloadStage& unloadStage)
{
    // Adjust Agressive Mode [Disable For All For Now]
    JenovaLoader::SetAgressiveMode(!(QUERY_ENGINE_MODE(Editor) || QUERY_ENGINE_MODE(Debug) || QUERY_ENGINE_MODE(Runtime)));

    // Flush Property Storage
    if (!JenovaInterpreter::FlushPropertyStorage())
    {
        jenova::Error("Jenova Interpreter", "Failed to Flush Interpreter Property Database.");
        return false;
    }

    // Call Module Internal Shutdown Event
    if (!jenova::CallModuleEvent(jenova::Format("_%s", jenova::GlobalSettings::JenovaModuleShutdownEventName), moduleHandle, jenova::ModuleCallMode::Virtual))
    {
        jenova::Warning("Jenova Interpreter", "Module Internal Shutdown Event Failed. Unexpected Behaviors May Occur.");
    }

    // Call Module Shutdown Event If Exists
    if (!jenova::CallModuleEvent(jenova::GlobalSettings::JenovaModuleShutdownEventName, moduleHandle, jenova::ModuleCallMode::Virtual))
    {
        jenova::Warning("Jenova Interpreter", "Module Shutdown Event Failed. Unexpected Behaviors May Occur.");
    }

    // If Debug Mode is Activated Unload Module Loaded From Disk
    if (executeInDebugMode) return jenova::ReleaseTemporaryModuleCache();

    // If Unload Stage is at Shutdown Leave Unloading to the OS
    if (unloadStage == jenova::ModuleUnloadStage::UnloadModuleToShutdown) return true;

    // Unload Module
	if (!moduleHandle) return false;
	if (!moduleBaseAddress) return false;
    if (!JenovaLoader::ReleaseModule(moduleHandle)) return false;
    moduleHandle = nullptr;
	moduleBaseAddress = 0;
    moduleMetaData = "{}";

    // All Good
	return true;
}
bool JenovaInterpreter::LoadDebugSymbol(const std::string symbolFilePath)
{
    return jenova::LoadSymbolForModule(jenova::GetCurrentProcessHandle(), moduleBaseAddress, symbolFilePath.c_str(), moduleBinarySize);
}
intptr_t JenovaInterpreter::GetModuleBaseAddress()
{
	return moduleBaseAddress;
}
jenova::FunctionList JenovaInterpreter::GetFunctionsList(const std::string& scriptUID)
{
    try
    {
        // Create Function List
        jenova::FunctionList functionNames;
        
        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["methods"];

        // Add Functions to List
        for (const auto& functionName : scriptMetadata.items()) functionNames.push_back(functionName.key());

        // Return List
        return functionNames;
    }
    catch (const std::exception&)
    {
        return jenova::FunctionList();
    }
}
jenova::FunctionAddress JenovaInterpreter::GetFunctionAddress(const std::string& functionName, const std::string& scriptUID)
{
    try
    {
        // Validate Script UID
        if (!moduleMetaData["Scripts"].contains(scriptUID)) return 0;

        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["methods"];

        // Get Function Address
        for (const auto& funcName : scriptMetadata.items())
        {
            if (funcName.key() == functionName)
            {
                // Calculate Offset + BaseAddress And Return
                jenova::FunctionAddress functionOffset = funcName.value()["Offset"].get<jenova::FunctionAddress>();
                return moduleBaseAddress + functionOffset;
            }
        }
    }
    catch (const std::exception&)
    {
        // Error Happened
        return 0;
    }

    // Function Was Not Found
    return 0;
}
jenova::ParameterTypeList JenovaInterpreter::GetFunctionParameters(const std::string& functionName, const std::string& scriptUID)
{
    try
    {
        // Validate Script UID
        if (!moduleMetaData["Scripts"].contains(scriptUID)) return jenova::ParameterTypeList();

        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["methods"];

        // Get Function Parameters
        for (const auto& funcName : scriptMetadata.items())
        {
            if (funcName.key() == functionName)
            {
                jenova::ParameterTypeList parameterTypes;
                int paramCount = funcName.value()["ParamCount"].get<int>();
                for (int i = 1; i <= paramCount; ++i) parameterTypes.push_back(funcName.value()[jenova::Format("Param%02d", i)].get<std::string>());
                return parameterTypes;
            }
        }
    }
    catch (const std::exception&)
    {
        // Error Happened
        return jenova::ParameterTypeList();
    }

    // Not Found
    return jenova::ParameterTypeList();
}
std::string JenovaInterpreter::GetFunctionReturn(const std::string& functionName, const std::string& scriptUID)
{
    try
    {
        // Validate Script UID
        if (!moduleMetaData["Scripts"].contains(scriptUID)) return "Unknown";

        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["methods"];

        // Get Function Return Type
        for (const auto& funcName : scriptMetadata.items())
        {
            if (funcName.key() == functionName)
            {
                return funcName.value()["ReturnType"].get<std::string>();
            }
        }
    }
    catch (const std::exception&)
    {
        // Error Happened
        return "Unknown";
    }

    // Not Found
    return "Unknown";
}
uintptr_t JenovaInterpreter::GetResolvedParameterPointer(const godot::Object* objectPtr, const godot::Variant* functionParameter, const std::string& parameterType)
{
    void* valueAddress = (void*)functionParameter;
    return reinterpret_cast<uintptr_t>(valueAddress);
}
bool JenovaInterpreter::IsFunctionReturnable(const std::string& returnType)
{
    if (returnType == "void") return false;
    return true;
}
jenova::ScriptFunctionContainer JenovaInterpreter::GetFunctionContainer(const std::string& scriptUID)
{
    // Todo : For Faster Execution Implement Same as GetPropertyContainer

    // Create Function Container
    jenova::ScriptFunctionContainer functionContainer;
    functionContainer.scriptUID = AS_GD_STRING(scriptUID);

    // Collect Script Functions
    jenova::FunctionList jenovaMethods = GetFunctionsList(scriptUID);
    for (size_t fid = 0; fid < jenovaMethods.size(); fid++)
    {
        // Create Script Function
        jenova::ScriptFunction scriptFunction;
        scriptFunction.functionID = fid;
        scriptFunction.functionName = String(jenovaMethods[fid].c_str());
        scriptFunction.ownerScriptUID = functionContainer.scriptUID;

        // Create Method Information
        scriptFunction.methodInfo.name = StringName(scriptFunction.functionName);
        scriptFunction.methodInfo.flags = METHOD_FLAGS_DEFAULT;
        scriptFunction.methodInfo.id = scriptFunction.functionID;

        // Get Function Return Type
        Variant::Type returnType = jenova::GetVariantTypeFromStdString(GetFunctionReturn(jenovaMethods[fid], scriptUID));
        scriptFunction.methodInfo.return_val = PropertyInfo(returnType, "return");

        // Get Function Parameter Types
        jenova::ParameterTypeList paramTypes = GetFunctionParameters(jenovaMethods[fid], scriptUID);
        for (size_t pid = 0; pid < paramTypes.size(); pid++)
        {
            Variant::Type paramType = jenova::GetVariantTypeFromStdString(paramTypes[pid]);
            if (paramType != Variant::NIL) scriptFunction.methodInfo.arguments.push_back(PropertyInfo(paramType, "Param"));
        }

        // Name Added Arguments
        for (size_t argid = 0; argid < scriptFunction.methodInfo.arguments.size(); argid++)
        {
            scriptFunction.methodInfo.arguments[argid].name = String(jenova::Format("Param%d", argid + 1).c_str());
        }

        // Add New Function to Container
        functionContainer.scriptFunctions.push_back(scriptFunction);
    }

    // Return Function Container
    return functionContainer;
}
jenova::ScriptPropertyContainer JenovaInterpreter::GetPropertyContainer(const std::string& scriptUID)
{
    try
    {
        // Create Property Container
        jenova::ScriptPropertyContainer propertyContainer;

        // Check If Script has Database
        if (!moduleMetaData["Scripts"][scriptUID].contains("database")) return propertyContainer;

        // Get Script Database by UID
        jenova::json_t scriptDatabase = moduleMetaData["Scripts"][scriptUID]["database"];

        // Check If Script has Properties
        if (!scriptDatabase.contains("properties")) return propertyContainer;

        // Collect & Create Properties
        propertyContainer = jenova::CreatePropertyContainerFromMetadata(scriptDatabase["properties"].dump(), scriptUID);

        // Return List
        return propertyContainer;
    }
    catch (const std::exception&)
    {
        return jenova::ScriptPropertyContainer();
    }
}
Variant JenovaInterpreter::CallFunction(const godot::Object* objectPtr, const std::string& functionName, std::string& scriptUID, const Variant** functionParameters, const int functionParametersCount)
{
    // Validate Module
    if (!allowExecution) return GenerateFunctionCallError(functionName, "ERROR::EXECUTION_DENIED");
    if (!moduleHandle || !moduleBaseAddress) return GenerateFunctionCallError(functionName, "ERROR::INVALID_MODULE");

    // Create Profiler Checkpoint [Not Required For Now]
    /* JenovaTinyProfiler::CreateCheckpoint("InterpreterCallFunction"); */

    // Verbose
    jenova::VerboseByID(__LINE__, "Interpreter Calling Function [%s] From Script [%s] On Object [%p]", functionName.c_str(), scriptUID.c_str(), objectPtr);

    // Get Function Address Offset
    jenova::FunctionAddress functionAddress = JenovaInterpreter::GetFunctionAddress(functionName, scriptUID);
    if (!functionAddress) return GenerateFunctionCallError(functionName, "ERROR::FUNCTION_ADDRESS_NOT_FOUND");

    // Get Function Return Type
    std::string functionReturnType = JenovaInterpreter::GetFunctionReturn(functionName, scriptUID);
    if (functionReturnType == "Unknown") return GenerateFunctionCallError(functionName, "ERROR::FUNCTION_RETURN_TYPE_NOT_FOUND");

    // Get Function Parameters Type
    jenova::ParameterTypeList functionParametersType = JenovaInterpreter::GetFunctionParameters(functionName, scriptUID);
    if (functionParametersType.size() == 0) return GenerateFunctionCallError(functionName, "ERROR::FUNCTION_PARAMETERS_TYPE_NOT_FOUND");

    // Determine and Set Flags
    bool callMustReturn = JenovaInterpreter::IsFunctionReturnable(functionReturnType);
    bool callHasParameters = !(functionParametersType.size() == 1 && functionParametersType[0] == "void");
    bool needsPassingOwner = functionParametersType[0] == "jenova::sdk::Caller*";

    // Create Final Parameter List
    std::vector<uintptr_t> resolvedParameters;

    // Pass Owner
    std::shared_ptr<jenova::ScriptCaller> scriptHandle = nullptr;
    if (needsPassingOwner)
    {
        scriptHandle = std::make_shared<jenova::ScriptCaller>(objectPtr);
        resolvedParameters.push_back(reinterpret_cast<uintptr_t>(scriptHandle.get()));
    }

    // Add Godot Parameters
    int parameterOffset = needsPassingOwner ? 1 : 0;
    for (size_t i = 0; i < functionParametersCount; i++) resolvedParameters.push_back(RESOLVE_PARAMETER(i));

    // Calculate Final Size
    int resolvedParametersCount = callHasParameters ? resolvedParameters.size() : 0;

    // Generate Code And Call Using Backends
    if (interpreterBackend == jenova::InterpreterBackend::AsmJIT)
    {
        try
        {
            // Create a JIT Runtime
            asmjit::JitRuntime jitRuntime;

            // Create Code Holder
            asmjit::CodeHolder code;
            code.init(jitRuntime.environment());

            // Assembler to Emit Code
            asmjit::x86::Assembler assembler(&code);

            // Calculate Stack Size
            int stackAlignmentSize = 0x38; int stackAdjusterSize = 0x08;
            if (resolvedParametersCount > 4) stackAlignmentSize += (resolvedParametersCount - 4) * stackAdjusterSize;

            // Generate Assembly Caller Code
            {
                // Push Required Stack Size
                assembler.sub(asmjit::x86::rsp, stackAlignmentSize);

                // Pushing Parameters
                if (resolvedParametersCount > 0)
                {
                    // Microsoft Windows x64 Architecture
                    if (QUERY_PLATFORM(Windows))
                    {
                        // Handle First 4 Parameters in Registers
                        if (resolvedParametersCount > 0) assembler.mov(asmjit::x86::rcx, resolvedParameters[0]);
                        if (resolvedParametersCount > 1) assembler.mov(asmjit::x86::rdx, resolvedParameters[1]);
                        if (resolvedParametersCount > 2) assembler.mov(asmjit::x86::r8, resolvedParameters[2]);
                        if (resolvedParametersCount > 3) assembler.mov(asmjit::x86::r9, resolvedParameters[3]);

                        // Push Remaining Parameters Directly to Stack
                        for (int i = 4; i < resolvedParametersCount; ++i)
                        {
                            int offset = 32 + ((i - 4) * stackAdjusterSize); // Offset starts at 32 bytes after the 4th parameter
                            assembler.mov(asmjit::x86::qword_ptr(asmjit::x86::rsp, offset), resolvedParameters[i]);
                        }
                    }

                    // System V AMD64 ABI Architecture
                    if (QUERY_PLATFORM(Linux))
                    {
                        // Handle First 6 Parameters in Registers
                        if (resolvedParametersCount > 0) assembler.mov(asmjit::x86::rdi, resolvedParameters[0]);
                        if (resolvedParametersCount > 1) assembler.mov(asmjit::x86::rsi, resolvedParameters[1]);
                        if (resolvedParametersCount > 2) assembler.mov(asmjit::x86::rdx, resolvedParameters[2]);
                        if (resolvedParametersCount > 3) assembler.mov(asmjit::x86::rcx, resolvedParameters[3]);
                        if (resolvedParametersCount > 4) assembler.mov(asmjit::x86::r8, resolvedParameters[4]);
                        if (resolvedParametersCount > 5) assembler.mov(asmjit::x86::r9, resolvedParameters[5]);

                        // Push Remaining Parameters Directly to Stack
                        for (int i = 6; i < resolvedParametersCount; ++i)
                        {
                            int offset = 32 + ((i - 6) * stackAdjusterSize); // Offset starts at 32 bytes after the 6th parameter
                            assembler.mov(asmjit::x86::qword_ptr(asmjit::x86::rsp, offset), resolvedParameters[i]);
                        }
                    }
                }

                // Push Calling Address and Call
                assembler.mov(asmjit::x86::rax, functionAddress);
                assembler.call(asmjit::x86::rax);

                // Pop Required Stack Size And Return
                assembler.add(asmjit::x86::rsp, stackAlignmentSize);
                assembler.ret();
            }

            // Execution
            if (callMustReturn)
            {
                // Typedef for the Generated Function
                typedef Variant(*CallerFunction)();

                // Allocate and Run Generated Code
                CallerFunction callerFunction = nullptr;
                jitRuntime.add(&callerFunction, &code);

                // Call the JIT-generated Function
                Variant result = callerFunction();

                // Release Generated Code When Done
                jitRuntime.release(callerFunction);

                // Return the Result as a Variant
                if (result.get_type() == Variant::NIL) return Variant("RESULT::VOID");
                return result;
            }
            else
            {
                // Void Call
                typedef void(*CallerFunction)();
                CallerFunction callerFunction = nullptr;
                jitRuntime.add(&callerFunction, &code);
                callerFunction();
                jitRuntime.release(callerFunction);
                return Variant(true);
            }
        }
        catch (const std::exception&)
        {
            // If Failed, Return False
            return GenerateFunctionCallError(functionName, "ERROR::CALL_FAILED");
        }
    }
    if (interpreterBackend == jenova::InterpreterBackend::TinyCC)
    {
        // Create Pointer List
        jenova::PointerList ptrList;
        
        // Generate Caller Code
        std::string interpreterCallerCode;
        interpreterCallerCode += jenova::Format("struct Variant { unsigned char opaque[%d]; };\n", GODOT_CPP_VARIANT_SIZE);
        interpreterCallerCode += "typedef struct Variant Variant;\n";
        interpreterCallerCode += "Variant* MakeVariant(void*, char*);\n";
        interpreterCallerCode += "void* interpreter_call()\n";
        interpreterCallerCode += "{\n";
        interpreterCallerCode += "typedef " + jenova::ResolveReturnTypeForJIT(functionReturnType) + "(*function_t)(";
        if (needsPassingOwner) interpreterCallerCode  += "void*";
        for (size_t i = 0; i < functionParametersCount; i++)
        {
            if (i == 0 && needsPassingOwner) interpreterCallerCode += ",";
            interpreterCallerCode += jenova::ResolveVariantTypeAsString(functionParameters[i]);
            if (i != functionParametersCount - 1) interpreterCallerCode += ",";
        }
        interpreterCallerCode += ");\n";
        interpreterCallerCode += jenova::Format("function_t _func = (function_t)0x%llx;\n", functionAddress);
        if (callMustReturn) interpreterCallerCode += jenova::ResolveReturnTypeForJIT(functionReturnType) + " result = ";
        interpreterCallerCode += "_func(";
        if (needsPassingOwner) interpreterCallerCode += jenova::Format("(void*)0x%llx", resolvedParameters[0]);
        for (size_t i = 0; i < functionParametersCount; i++)
        {
            if (i == 0 && needsPassingOwner) interpreterCallerCode += ",";
            interpreterCallerCode += jenova::ResolveVariantValueAsString(functionParameters[i], ptrList);
            if (i != functionParametersCount - 1) interpreterCallerCode += ",";
        }
        interpreterCallerCode += ");\n";
        if (callMustReturn) interpreterCallerCode += "return MakeVariant(&result,\"" + functionReturnType + "\");\n";
        else interpreterCallerCode += "return 0;\n";
        interpreterCallerCode += "}";

        // Initialize TCC Compiler
        TCCState* tcc = tcc_new();
        if (!tcc) 
        {
            jenova::Error("Interpreter Backend", "Failed to Initialize JIT Interpreter.");
            return Variant(false);
        }

        // Create Error/Warning Reporter 
        if (jenova::GlobalStorage::DeveloperModeActivated)
        {
            jenova::VerboseByID(__LINE__, "JIT Execution Code : \n%s", interpreterCallerCode.c_str());
            auto tcc_error_handler = [](void* opaque, const char* msg) -> void
            {
                jenova::Error("Interpreter Backend", "%s", msg);
            };
            tcc_set_error_func(tcc, nullptr, tcc_error_handler);
        }

        // Configure TCC Compiler
        tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
        tcc_set_options(tcc, "-nostdlib");

        // Add Symbols
        tcc_add_symbol(tcc, "memmove", reinterpret_cast<const void*>(&jenova::RelocateMemory));
        tcc_add_symbol(tcc, "MakeVariant", reinterpret_cast<const void*>(&jenova::MakeVariantFromReturnType));

        // Compile Generated Code
        if (tcc_compile_string(tcc, interpreterCallerCode.c_str()) == -1) 
        {
            jenova::Error("Interpreter Backend", "Failed to Compile Interpreter Code.");
            for (void* ptr : ptrList) if (ptr) delete ptr;
            tcc_delete(tcc);
            return Variant(false);
        }

        // Prepare For Execution
        if (tcc_relocate(tcc, TCC_RELOCATE_AUTO) < 0) {
            jenova::Error("Interpreter Backend", "Failed to Resolve Interpreter Code.");
            for (void* ptr : ptrList) if (ptr) delete ptr;
            tcc_delete(tcc);
            return Variant(false);
        }

        // Get Compiled Caller Function
        using MetaCallerType = Variant*(*)();
        MetaCallerType interpreterCaller = (MetaCallerType)tcc_get_symbol(tcc, "interpreter_call");
        if (!interpreterCaller) 
        {
            jenova::Error("Interpreter Backend", "Failed to Get Interpreter JIT Caller.");
            for (void* ptr : ptrList) if (ptr) delete ptr;
            tcc_delete(tcc);
            return Variant(false);
        }

        // Execute Caller
        Variant* result = interpreterCaller();

        // Release Allocated Values
        for (void* ptr : ptrList) if (ptr) delete ptr;
        ptrList.clear();

        // Clean up
        tcc_delete(tcc);

        // Process Result
        if (callMustReturn)
        {
            if (result)
            {
                Variant finalResult(*result);
                delete result;
                return finalResult;
            }
        }
        return Variant(true);
    }
    if (interpreterBackend == jenova::InterpreterBackend::AkiraJIT)
    {
        // Removed
    }
    if (interpreterBackend == jenova::InterpreterBackend::SecureAngel)
    {
        // Removed
    }

    // No Valid Backend
    return GenerateFunctionCallError(functionName, "ERROR::INVALID_INTERPRETER_BACKEND");
}
void JenovaInterpreter::SetExecutionState(bool executionState)
{
    // Set Execution State
    allowExecution = executionState;
}
Variant JenovaInterpreter::GenerateFunctionCallError(const std::string& functionName, const String& errorReason)
{
    // Throw Error Message
    jenova::Error("[Jenova Interpreter]", "Failed to Interpret Function [%s] With Reason : %s", functionName.c_str(), AS_C_STRING(errorReason));

    // Return Error Reason
    return String(errorReason);
}
jenova::SerializedData JenovaInterpreter::GenerateModuleMetadata(const std::string& mapFilePath, const jenova::ModuleList& scriptModules, const jenova::BuildResult& buildResult)
{
    // Windows Compilers
    #ifdef TARGET_PLATFORM_WINDOWS

    // Microsoft Visual C++ Map Parser
    if (buildResult.compilerModel == jenova::CompilerModel::MicrosoftCompiler || buildResult.compilerModel == jenova::CompilerModel::ClangLLVMCompiler)
    {
        try
        {
            // Create JSON Serializer
            jenova::json_t serializer;

            // Variables to Store __ImageBase
            uint64_t imageBaseAddress = 0;

            // Serialize Script Modules
            for (const auto& scriptModule : scriptModules) serializer["Scripts"][AS_STD_STRING(scriptModule.scriptUID)] = jenova::json_t::object();

            // Open Map File
            if (!std::filesystem::exists(mapFilePath))
            {
                jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Open Map File.");
                return jenova::SerializedData();
            }
            std::ifstream mapfileReader(mapFilePath);
            if (!mapfileReader.is_open())
            {
                jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Read Map File.");
                return jenova::SerializedData();
            }

            // Parse Map File And Generate Metadata
            if (buildResult.compilerModel == jenova::CompilerModel::MicrosoftCompiler)
            {
                // Regex Patterns
                std::regex imageBasePattern(R"(^\s*\d+:\d+\s+__ImageBase\s+([0-9A-Fa-f]{16}))");
                std::regex jnvNameOffsetFuncPattern(R"(^\s*\d+:(\w+)\s+\?(.*?)@JNV_([a-f0-9]+)@@.*\s+([0-9A-Fa-f]{16})\s+f\s+.*$)");
                std::regex jnvNameOffsetPropPattern(R"(^\s*\d+:(\w+)\s+\?(.*?)@JNV_([a-f0-9]+)@@.*\s+([0-9A-Fa-f]{16})\s+\s+.*$)");
                std::regex jnvMangledNamePattern(R"(\?\w+@JNV_\w+@@\S+)");

                // Process Parsing
                std::string mapFileLine; std::smatch match;
                while (std::getline(mapfileReader, mapFileLine))
                {
                    // Extract __ImageBase
                    if (std::regex_search(mapFileLine, match, imageBasePattern))
                    {
                        imageBaseAddress = std::stoull(match[1], nullptr, 16);
                        serializer["ImageBaseAddress"] = imageBaseAddress;
                        continue;  // Skip to the next line after extracting __ImageBase
                    }

                    // Parse Functions Name and Offsets
                    if (std::regex_search(mapFileLine, match, jnvNameOffsetFuncPattern) && imageBaseAddress)
                    {
                        // Extract Parsed Data
                        std::string functionName = match[2];
                        std::string scriptUID = match[3];
                        std::string functionOffsetStr = match[4];

                        // Ignore Classed Functions
                        if (functionName.find("@") != std::string::npos) continue;

                        // Calculate Offset
                        uint64_t functionOffset = std::stoull(functionOffsetStr, nullptr, 16);
                        uint64_t actualOffset = functionOffset - imageBaseAddress;

                        // Check for duplicate function names under the same script UID
                        if (serializer["Scripts"].contains(scriptUID) && serializer["Scripts"][scriptUID].contains(functionName))
                        {
                            jenova::Error("Jenova Interpreter", "Duplicate Function Detected : [%s] Under Script UID: [%s]", functionName.c_str(), scriptUID.c_str());
                            return jenova::SerializedData();
                        }

                        // Create Function Metadata Serializer
                        jenova::json_t funcSerializer;
                        funcSerializer["Offset"] = actualOffset;

                        // Parse Functions Mangled Name And Extract Types
                        if (std::regex_search(mapFileLine, match, jnvMangledNamePattern) && imageBaseAddress)
                        {
                            // Extract Parsed Data And Demangle
                            std::string mangledFunctionSignature = match[0];
                            std::string demangledFunctionSignature = jenova::GetDemangledFunctionSignature(mangledFunctionSignature, buildResult.compilerModel);
                            if (demangledFunctionSignature.empty())
                            {
                                jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Demangle Function [%s] [%s]", 
                                    mangledFunctionSignature.c_str(), demangledFunctionSignature.c_str());
                                return jenova::SerializedData();
                            }

                            // Clean Function Signature
                            std::string cleanedFunctionSignature = jenova::CleanFunctionAndPropertySignature(demangledFunctionSignature, buildResult.compilerModel);
                    
                            // Exctract Return Type
                            std::string returnType = jenova::ExtractReturnTypeFromSignature(cleanedFunctionSignature, buildResult.compilerModel);
                            if (returnType.empty())
                            {
                                jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Extract Function Return Type [%s] [%s]",
                                    mangledFunctionSignature.c_str(), demangledFunctionSignature.c_str());
                                return jenova::SerializedData();
                            }
                            funcSerializer["ReturnType"] = returnType;
                            jenova::VerboseByID(__LINE__, "Extracted Return Type [%s]", returnType.c_str());

                            // Extract Parameter Types
                            jenova::ParameterTypeList parameterTypes = jenova::ExtractParameterTypesFromSignature(cleanedFunctionSignature, buildResult.compilerModel);
                            funcSerializer["ParamCount"] = parameterTypes.size();
                            jenova::VerboseByID(__LINE__, "Extracted Parameters Count [%d]", parameterTypes.size());
                            for (size_t i = 0; i < parameterTypes.size(); ++i)
                            {
                                funcSerializer[jenova::Format("Param%02d", i + 1)] = parameterTypes[i];
                                jenova::VerboseByID(__LINE__, "Extracted Parameter Type [%s]", parameterTypes[i].c_str());
                            }

                            // Verbose
                            jenova::VerboseByID(__LINE__, "[Map-Parser] Demangled Function Name: [%s], UID: [%s]", demangledFunctionSignature.c_str(), scriptUID.c_str());
                        }

                        // Store function name and metadata in the serializer
                        if (serializer["Scripts"].contains(scriptUID))
                        {
                            serializer["Scripts"][scriptUID]["methods"][functionName] = funcSerializer;
                        }
                        else 
                        {
                            serializer["Scripts"][scriptUID]["methods"] = { { functionName, funcSerializer } };
                        }

                        // Verbose
                        jenova::VerboseByID(__LINE__, "[Map-Parser] Function Name & Offset Extracted > Name: %s, UID: %s, Offset: %llx", functionName.c_str(), scriptUID.c_str(), actualOffset);
                    }

                    // Parse Properties Name and Offsets
                    if (std::regex_search(mapFileLine, match, jnvNameOffsetPropPattern) && imageBaseAddress)
                    {
                        // Extract Parsed Data
                        std::string propertyName = match[2];
                        std::string scriptUID = match[3];
                        std::string propertyOffsetStr = match[4];

                        // Clean Property Name
                        jenova::ReplaceAllMatchesWithString(propertyName, "__prop_", "");

                        // Ignore Classed Properties
                        if (propertyName.find("@") != std::string::npos) continue;

                        // Calculate Offset
                        uint64_t propertyOffset = std::stoull(propertyOffsetStr, nullptr, 16);
                        uint64_t actualOffset = propertyOffset - imageBaseAddress;

                        // Check for duplicate property names under the same script UID
                        if (serializer["Scripts"].contains(scriptUID) && serializer["Scripts"][scriptUID].contains(propertyName))
                        {
                            jenova::Error("Jenova Interpreter", "Duplicate Property Detected : [%s] Under Script UID: [%s]", propertyName.c_str(), scriptUID.c_str());
                            return jenova::SerializedData();
                        }

                        // Create Property Metadata Serializer
                        jenova::json_t propSerializer;
                        propSerializer["Offset"] = actualOffset;

                        // Parse Properties Mangled Name And Extract Type
                        if (std::regex_search(mapFileLine, match, jnvMangledNamePattern) && imageBaseAddress)
                        {
                            // Extract Parsed Data And Demangle
                            std::string mangledPropertySignature = match[0];
                            std::string demangledPropertySignature = jenova::GetDemangledFunctionSignature(mangledPropertySignature, buildResult.compilerModel);
                            if (demangledPropertySignature.empty())
                            {
                                jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Demangle Property [%s] [%s]",
                                    mangledPropertySignature.c_str(), demangledPropertySignature.c_str());
                                return jenova::SerializedData();
                            }

                            // Clean Property Signature
                            std::string cleanedPropertySignature = jenova::CleanFunctionAndPropertySignature(demangledPropertySignature, buildResult.compilerModel);

                            // Extract Type
                            std::string propertyType = jenova::ExtractPropertyTypeFromSignature(cleanedPropertySignature, buildResult.compilerModel);
                            if (propertyType.empty())
                            {
                                jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Extract Property Type [%s] [%s]",
                                    mangledPropertySignature.c_str(), demangledPropertySignature.c_str());
                                return jenova::SerializedData();
                            }
                            propSerializer["Type"] = propertyType;
                            jenova::VerboseByID(__LINE__, "Extracted Property Type [%s]", propertyType.c_str());

                            // Verbose
                            jenova::VerboseByID(__LINE__, "[Map-Parser] Demangled Property Name: [%s], UID: [%s]", demangledPropertySignature.c_str(), scriptUID.c_str());
                        }

                        // Store property name and metadata in the serializer
                        if (serializer["Scripts"].contains(scriptUID))
                        {
                            serializer["Scripts"][scriptUID]["properties"][propertyName] = propSerializer;
                        }
                        else
                        {
                            serializer["Scripts"][scriptUID]["properties"] = { { propertyName, propSerializer } };
                        }

                        // Verbose
                        jenova::VerboseByID(__LINE__, "[Map-Parser] Property Name & Offset Extracted > Name: %s, UID: %s, Offset: %llx", propertyName.c_str(), scriptUID.c_str(), actualOffset);
                    }
                }
            }
            if (buildResult.compilerModel == jenova::CompilerModel::ClangLLVMCompiler)
            {
                // Regex Patterns
                std::regex imageBasePattern(R"(^\s*\d+:\d+\s+__ImageBase\s+([0-9A-Fa-f]{16}))");
                std::regex jnvSymbolPattern(R"(^\s*\d+:(\w+)\s+\?(.*?)@JNV_([a-f0-9]+)@@.*\s+([0-9A-Fa-f]{16})\s+\s+.*$)");
                std::regex jnvMangledNamePattern(R"(\?\w+@JNV_\w+@@\S+)");

                // Process Parsing
                std::string mapFileLine; std::smatch match;
                while (std::getline(mapfileReader, mapFileLine))
                {
                    // Extract __ImageBase
                    if (std::regex_search(mapFileLine, match, imageBasePattern))
                    {
                        imageBaseAddress = std::stoull(match[1], nullptr, 16);
                        serializer["ImageBaseAddress"] = imageBaseAddress;
                        continue;  // Skip to the next line after extracting __ImageBase
                    }

                    // Parse Jenova Symbols
                    if (std::regex_search(mapFileLine, match, jnvSymbolPattern) && imageBaseAddress)
                    {
                        // Detect Property vs Function
                        if (match[2].str().find("__prop_") != std::string::npos)
                        {
                            // Extract Parsed Data
                            std::string propertyName = match[2];
                            std::string scriptUID = match[3];
                            std::string propertyOffsetStr = match[4];

                            // Clean Property Name
                            jenova::ReplaceAllMatchesWithString(propertyName, "__prop_", "");

                            // Ignore Classed Properties
                            if (propertyName.find("@") != std::string::npos) continue;

                            // Calculate Offset
                            uint64_t propertyOffset = std::stoull(propertyOffsetStr, nullptr, 16);
                            uint64_t actualOffset = propertyOffset - imageBaseAddress;

                            // Check for duplicate property names under the same script UID
                            if (serializer["Scripts"].contains(scriptUID) && serializer["Scripts"][scriptUID].contains(propertyName))
                            {
                                jenova::Error("Jenova Interpreter", "Duplicate Property Detected : [%s] Under Script UID: [%s]", propertyName.c_str(), scriptUID.c_str());
                                return jenova::SerializedData();
                            }

                            // Create Property Metadata Serializer
                            jenova::json_t propSerializer;
                            propSerializer["Offset"] = actualOffset;

                            // Parse Properties Mangled Name And Extract Type
                            if (std::regex_search(mapFileLine, match, jnvMangledNamePattern) && imageBaseAddress)
                            {
                                // Extract Parsed Data And Demangle
                                std::string mangledPropertySignature = match[0];
                                std::string demangledPropertySignature = jenova::GetDemangledFunctionSignature(mangledPropertySignature, buildResult.compilerModel);
                                if (demangledPropertySignature.empty())
                                {
                                    jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Demangle Property [%s] [%s]",
                                        mangledPropertySignature.c_str(), demangledPropertySignature.c_str());
                                    return jenova::SerializedData();
                                }

                                // Clean Property Signature
                                std::string cleanedPropertySignature = jenova::CleanFunctionAndPropertySignature(demangledPropertySignature, buildResult.compilerModel);

                                // Extract Type
                                std::string propertyType = jenova::ExtractPropertyTypeFromSignature(cleanedPropertySignature, buildResult.compilerModel);
                                if (propertyType.empty())
                                {
                                    jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Extract Property Type [%s] [%s]",
                                        mangledPropertySignature.c_str(), demangledPropertySignature.c_str());
                                    return jenova::SerializedData();
                                }
                                propSerializer["Type"] = propertyType;
                                jenova::VerboseByID(__LINE__, "Extracted Property Type [%s]", propertyType.c_str());

                                // Verbose
                                jenova::VerboseByID(__LINE__, "[Map-Parser] Demangled Property Name: [%s], UID: [%s]", demangledPropertySignature.c_str(), scriptUID.c_str());
                            }

                            // Store property name and metadata in the serializer
                            if (serializer["Scripts"].contains(scriptUID))
                            {
                                serializer["Scripts"][scriptUID]["properties"][propertyName] = propSerializer;
                            }
                            else
                            {
                                serializer["Scripts"][scriptUID]["properties"] = { { propertyName, propSerializer } };
                            }

                            // Verbose
                            jenova::VerboseByID(__LINE__, "[Map-Parser] Property Name & Offset Extracted > Name: %s, UID: %s, Offset: %llx", propertyName.c_str(), scriptUID.c_str(), actualOffset);
                        }
                        else
                        {
                            // Extract Parsed Data
                            std::string functionName = match[2];
                            std::string scriptUID = match[3];
                            std::string functionOffsetStr = match[4];

                            // Ignore Classed Functions
                            if (functionName.find("@") != std::string::npos) continue;

                            // Calculate Offset
                            uint64_t functionOffset = std::stoull(functionOffsetStr, nullptr, 16);
                            uint64_t actualOffset = functionOffset - imageBaseAddress;

                            // Check for duplicate function names under the same script UID
                            if (serializer["Scripts"].contains(scriptUID) && serializer["Scripts"][scriptUID].contains(functionName))
                            {
                                jenova::Error("Jenova Interpreter", "Duplicate Function Detected : [%s] Under Script UID: [%s]", functionName.c_str(), scriptUID.c_str());
                                return jenova::SerializedData();
                            }

                            // Create Function Metadata Serializer
                            jenova::json_t funcSerializer;
                            funcSerializer["Offset"] = actualOffset;

                            // Parse Functions Mangled Name And Extract Types
                            if (std::regex_search(mapFileLine, match, jnvMangledNamePattern) && imageBaseAddress)
                            {
                                // Extract Parsed Data And Demangle
                                std::string mangledFunctionSignature = match[0];
                                std::string demangledFunctionSignature = jenova::GetDemangledFunctionSignature(mangledFunctionSignature, buildResult.compilerModel);
                                if (demangledFunctionSignature.empty())
                                {
                                    jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Demangle Function [%s] [%s]",
                                        mangledFunctionSignature.c_str(), demangledFunctionSignature.c_str());
                                    return jenova::SerializedData();
                                }

                                // Double-Check If Extracted Symbol is Function
                                if (jenova::DetectSymbolSignatureType(mangledFunctionSignature, buildResult.compilerModel) != jenova::SymbolSignatureType::FunctionSymbol)
                                {
                                    jenova::VerboseByID(__LINE__, "Skipping Symbol Candidate [%s] due to a Non-Standard Signature.", demangledFunctionSignature.c_str());
                                    continue;
                                }

                                // Clean Function Signature
                                std::string cleanedFunctionSignature = jenova::CleanFunctionAndPropertySignature(demangledFunctionSignature, buildResult.compilerModel);

                                // Exctract Return Type
                                std::string returnType = jenova::ExtractReturnTypeFromSignature(cleanedFunctionSignature, buildResult.compilerModel);
                                if (returnType.empty())
                                {
                                    jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : Unable to Extract Function Return Type [%s] [%s]",
                                        mangledFunctionSignature.c_str(), demangledFunctionSignature.c_str());
                                    return jenova::SerializedData();
                                }
                                funcSerializer["ReturnType"] = returnType;
                                jenova::VerboseByID(__LINE__, "Extracted Return Type [%s]", returnType.c_str());

                                // Extract Parameter Types
                                jenova::ParameterTypeList parameterTypes = jenova::ExtractParameterTypesFromSignature(cleanedFunctionSignature, buildResult.compilerModel);
                                funcSerializer["ParamCount"] = parameterTypes.size();
                                jenova::VerboseByID(__LINE__, "Extracted Parameters Count [%d]", parameterTypes.size());
                                for (size_t i = 0; i < parameterTypes.size(); ++i)
                                {
                                    funcSerializer[jenova::Format("Param%02d", i + 1)] = parameterTypes[i];
                                    jenova::VerboseByID(__LINE__, "Extracted Parameter Type [%s]", parameterTypes[i].c_str());
                                }

                                // Verbose
                                jenova::VerboseByID(__LINE__, "[Map-Parser] Demangled Function Name: [%s], UID: [%s]", demangledFunctionSignature.c_str(), scriptUID.c_str());
                            }

                            // Store function name and metadata in the serializer
                            if (serializer["Scripts"].contains(scriptUID))
                            {
                                serializer["Scripts"][scriptUID]["methods"][functionName] = funcSerializer;
                            }
                            else
                            {
                                serializer["Scripts"][scriptUID]["methods"] = { { functionName, funcSerializer } };
                            }

                            // Verbose
                            jenova::VerboseByID(__LINE__, "[Map-Parser] Function Name & Offset Extracted > Name: %s, UID: %s, Offset: %llx", functionName.c_str(), scriptUID.c_str(), actualOffset);
                        }
                    }
                }
            }

            // Add Properties Definitions
            for (const auto& scriptModule : scriptModules)
            {
                if (scriptModule.scriptPropertiesFile.is_empty()) continue;
                if (std::filesystem::exists(AS_STD_STRING(scriptModule.scriptPropertiesFile)))
                {
                    std::string propDatabase = jenova::ReadStdStringFromFile(AS_STD_STRING(scriptModule.scriptPropertiesFile));
                    if (!propDatabase.empty())
                    {
                        serializer["Scripts"][AS_STD_STRING(scriptModule.scriptUID)]["database"]["properties"] = jenova::json_t::parse(propDatabase);
                    }
                }
            }

            // Add Extra Info
            if (buildResult.hasDebugInformation)
            {
                serializer["HasDebugInformation"] = true;
                serializer["BuildPath"] = buildResult.buildPath;
            }
            else
            {
                serializer["HasDebugInformation"] = false;
            }
            serializer["ModuleBinarySize"] = buildResult.builtModuleData.size();
            serializer["InterpreterBackend"] = JenovaInterpreter::GetInterpreterBackend();
            serializer["DeveloperMode"] = jenova::GlobalStorage::DeveloperModeActivated;
            serializer["ManagedSafeExecution"] = jenova::GlobalStorage::UseManagedSafeExecution;

            // Dump Metadata If Developer Mode Activated
            if (jenova::GlobalStorage::DeveloperModeActivated)
            {
                jenova::WriteStdStringToFile(AS_STD_STRING(jenova::GetJenovaCacheDirectory() + "Jenova.Metadata.json"), serializer.dump(4));
            }

            // Serialize Data
            return serializer.dump();
        }
        catch (const std::exception& err)
        {
            jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : %s", err.what());
        }
    }

    // MinGW Compiler Map Parser/LLVM Clang Map Parser
    if (buildResult.compilerModel == jenova::CompilerModel::MinGWCompiler || buildResult.compilerModel == jenova::CompilerModel::MinGWClangCompiler)
    {
        try
        {
            // Create JSON Serializer
            jenova::json_t serializer;

            // Generate Extra Paths
            std::string moduleFilePath = buildResult.buildPath + "Jenova.Module.so";
            std::string funcInfoFilePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(mapFilePath).stem().string() + ".finfo";
            std::string propInfoFilePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(mapFilePath).stem().string() + ".pinfo";

            // Parse Function Info File
            std::ifstream funcFile(funcInfoFilePath);
            if (!funcFile.is_open())
            {
                jenova::Error("Jenova Interpreter", "Unable to open function info file: %s", funcInfoFilePath.c_str());
                return jenova::SerializedData();
            }

            std::string line;
            std::regex funcPattern(R"(^\s*\d+:\s*(.*JNV_([a-f0-9]+)::(\w+)\((.*?)\));$)");
            while (std::getline(funcFile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, funcPattern))
                {
                    // Get Extracted Data
                    std::string funcSignature = match[1];
                    std::string funcName = match[3];
                    std::string scriptUID = match[2];

                    // Extract Function Information
                    std::string cleanedSignature = jenova::CleanFunctionAndPropertySignature(funcSignature, buildResult.compilerModel);
                    std::vector<std::string> params = jenova::ExtractParameterTypesFromSignature(cleanedSignature, buildResult.compilerModel);
                    std::string returnType = jenova::ExtractReturnTypeFromSignature(cleanedSignature, buildResult.compilerModel);

                    // If the function has no parameters, Add A Dummy Parameter
                    if (params.empty()) params.push_back("void");

                    // Add Function
                    if (!serializer["Scripts"].contains(scriptUID)) serializer["Scripts"][scriptUID]["methods"] = jenova::json_t::object();

                    // Add Parameter Count & Return Type
                    serializer["Scripts"][scriptUID]["methods"][funcName] = { {"ParamCount", params.size()}, {"ReturnType", returnType} };

                    // Add Parameter Types
                    for (size_t i = 0; i < params.size(); ++i) serializer["Scripts"][scriptUID]["methods"][funcName][jenova::Format("Param%02d", i + 1)] = params[i];
                }
            }

            // Parse Property Info File
            std::ifstream propFile(propInfoFilePath);
            if (!propFile.is_open())
            {
                jenova::Error("Jenova Interpreter", "Unable to open property info file: %s", propInfoFilePath.c_str());
                return jenova::SerializedData();
            }

            std::regex propPattern(R"(^\s*\d+:\s*(.*JNV_([a-f0-9]+)::(__prop_\w+));$)");
            while (std::getline(propFile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, propPattern))
                {
                    std::string propSignature = match[1];
                    std::string propName = match[3].str();
                    std::string scriptUID = match[2];

                    // Clean Property Name
                    jenova::ReplaceAllMatchesWithString(propName, "__prop_", "");

                    // Ignore Classed Properties
                    if (propName.find("@") != std::string::npos) continue;

                    // Extract Property Type From Signature
                    std::string propType = jenova::ExtractPropertyTypeFromSignature(propSignature, buildResult.compilerModel);

                    // Set Data
                    if (!serializer["Scripts"].contains(scriptUID)) serializer["Scripts"][scriptUID]["properties"] = jenova::json_t::object();
                    serializer["Scripts"][scriptUID]["properties"][propName] = { {"Type", propType} };
                }
            }

            // Parse Map File for Offsets
            std::ifstream mapFile(mapFilePath);
            if (!mapFile.is_open())
            {
                jenova::Error("Jenova Interpreter", "Unable to open map file: %s", mapFilePath.c_str());
                return jenova::SerializedData();
            }

            std::regex mapPattern(R"(^\s*([0-9a-fA-F]+)\s+\d+\s+\d+\s+JNV_([a-f0-9]+)::([\w:]+).*$)");
            while (std::getline(mapFile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, mapPattern))
                {
                    uint64_t offset = std::stoull(match[1], nullptr, 16);
                    std::string scriptUID = match[2];
                    std::string name = match[3];

                    if (serializer["Scripts"].contains(scriptUID))
                    {
                        if (serializer["Scripts"][scriptUID]["methods"].contains(name))
                        {
                            serializer["Scripts"][scriptUID]["methods"][name]["Offset"] = offset;
                        }
                        else if (serializer["Scripts"][scriptUID]["properties"].contains(name))
                        {
                            serializer["Scripts"][scriptUID]["properties"][name]["Offset"] = offset;
                        }
                    }
                }

                // Handle Property Offsets Explicitly
                std::regex propOffsetPattern(R"(^\s*([0-9a-fA-F]+)\s+\d+\s+\d+\s+JNV_([a-f0-9]+)::(__prop_[\w]+).*$)");
                if (std::regex_search(line, match, propOffsetPattern))
                {
                    uint64_t offset = std::stoull(match[1], nullptr, 16);
                    std::string scriptUID = match[2];
                    std::string propName = match[3].str();

                    // Clean Property Name
                    jenova::ReplaceAllMatchesWithString(propName, "__prop_", "");

                    if (serializer["Scripts"].contains(scriptUID) && serializer["Scripts"][scriptUID]["properties"].contains(propName))
                    {
                        serializer["Scripts"][scriptUID]["properties"][propName]["Offset"] = offset;
                    }
                }
            }

            // Add Properties Definitions
            for (const auto& scriptModule : scriptModules)
            {
                if (scriptModule.scriptPropertiesFile.is_empty()) continue;
                if (std::filesystem::exists(AS_STD_STRING(scriptModule.scriptPropertiesFile)))
                {
                    std::string propDatabase = jenova::ReadStdStringFromFile(AS_STD_STRING(scriptModule.scriptPropertiesFile));
                    if (!propDatabase.empty())
                    {
                        serializer["Scripts"][AS_STD_STRING(scriptModule.scriptUID)]["database"]["properties"] = jenova::json_t::parse(propDatabase);
                    }
                }
            }

            // Add Extra Info
            if (buildResult.hasDebugInformation)
            {
                serializer["HasDebugInformation"] = true;
                serializer["BuildPath"] = buildResult.buildPath;
            }
            else
            {
                serializer["HasDebugInformation"] = false;
            }
            serializer["ModuleBinarySize"] = buildResult.builtModuleData.size();
            serializer["InterpreterBackend"] = JenovaInterpreter::GetInterpreterBackend();
            serializer["DeveloperMode"] = jenova::GlobalStorage::DeveloperModeActivated;
            serializer["ManagedSafeExecution"] = jenova::GlobalStorage::UseManagedSafeExecution;

            // Dump Metadata If Developer Mode Activated
            if (jenova::GlobalStorage::DeveloperModeActivated)
            {
                jenova::WriteStdStringToFile(AS_STD_STRING(jenova::GetJenovaCacheDirectory() + "Jenova.Metadata.json"), serializer.dump(4));
            }

            // Serialize Data
            return serializer.dump();
        }
        catch (const std::exception& err)
        {
            jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : %s", err.what());
        }
    }

    #endif // Windows Compilers

    // Linux Compilers
    #ifdef TARGET_PLATFORM_LINUX

    // GNU Compiler Collection/LLVM Clang Map Parser
    if (buildResult.compilerModel == jenova::CompilerModel::GNUCompiler || buildResult.compilerModel == jenova::CompilerModel::ClangCompiler)
    {
        try
        {
            // Create JSON Serializer
            jenova::json_t serializer;

            // Generate Extra Paths
            std::string moduleFilePath = buildResult.buildPath + "Jenova.Module.so";
            std::string funcInfoFilePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(mapFilePath).stem().string() + ".finfo";
            std::string propInfoFilePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(mapFilePath).stem().string() + ".pinfo";

            // Parse Function Info File
            std::ifstream funcFile(funcInfoFilePath);
            if (!funcFile.is_open())
            {
                jenova::Error("Jenova Interpreter", "Unable to open function info file: %s", funcInfoFilePath.c_str());
                return jenova::SerializedData();
            }

            std::string line;
            std::regex funcPattern(R"(^\s*\d+:\s*(.*JNV_([a-f0-9]+)::(\w+)\((.*?)\));$)");
            while (std::getline(funcFile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, funcPattern))
                {
                    // Get Extracted Data
                    std::string funcSignature = match[1];
                    std::string funcName = match[3];
                    std::string scriptUID = match[2];

                    // Extract Function Information
                    std::string cleanedSignature = jenova::CleanFunctionAndPropertySignature(funcSignature, buildResult.compilerModel);
                    std::vector<std::string> params = jenova::ExtractParameterTypesFromSignature(cleanedSignature, buildResult.compilerModel);
                    std::string returnType = jenova::ExtractReturnTypeFromSignature(cleanedSignature, buildResult.compilerModel);

                    // If the function has no parameters, Add A Dummy Parameter
                    if (params.empty()) params.push_back("void");

                    // Add Function
                    if (!serializer["Scripts"].contains(scriptUID)) serializer["Scripts"][scriptUID]["methods"] = jenova::json_t::object();

                    // Add Parameter Count & Return Type
                    serializer["Scripts"][scriptUID]["methods"][funcName] = { {"ParamCount", params.size()}, {"ReturnType", returnType} };

                    // Add Parameter Types
                    for (size_t i = 0; i < params.size(); ++i) serializer["Scripts"][scriptUID]["methods"][funcName][jenova::Format("Param%02d", i + 1)] = params[i];
                }
            }

            // Parse Property Info File
            std::ifstream propFile(propInfoFilePath);
            if (!propFile.is_open())
            {
                jenova::Error("Jenova Interpreter", "Unable to open property info file: %s", propInfoFilePath.c_str());
                return jenova::SerializedData();
            }

            std::regex propPattern(R"(^\s*\d+:\s*(.*JNV_([a-f0-9]+)::(__prop_\w+));$)");
            while (std::getline(propFile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, propPattern))
                {
                    std::string propSignature = match[1];
                    std::string propName = match[3].str();
                    std::string scriptUID = match[2];

                    // Clean Property Name
                    jenova::ReplaceAllMatchesWithString(propName, "__prop_", "");

                    // Ignore Classed Properties
                    if (propName.find("@") != std::string::npos) continue;

                    // Extract Property Type From Signature
                    std::string propType = jenova::ExtractPropertyTypeFromSignature(propSignature, buildResult.compilerModel);

                    // Set Data
                    if (!serializer["Scripts"].contains(scriptUID)) serializer["Scripts"][scriptUID]["properties"] = jenova::json_t::object();
                    serializer["Scripts"][scriptUID]["properties"][propName] = {{"Type", propType}};
                }
            }

            // Parse Map File for Offsets
            std::ifstream mapFile(mapFilePath);
            if (!mapFile.is_open())
            {
                jenova::Error("Jenova Interpreter", "Unable to open map file: %s", mapFilePath.c_str());
                return jenova::SerializedData();
            }

            std::regex mapPattern(R"(\s*0x([0-9a-fA-F]+)\s*JNV_([a-f0-9]+)::(\w+).*$)");
            while (std::getline(mapFile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, mapPattern))
                {
                    uint64_t offset = std::stoull(match[1], nullptr, 16);
                    std::string scriptUID = match[2];
                    std::string name = match[3];

                    if (serializer["Scripts"].contains(scriptUID))
                    {
                        if (serializer["Scripts"][scriptUID]["methods"].contains(name))
                        {
                            serializer["Scripts"][scriptUID]["methods"][name]["Offset"] = offset;
                        }
                        else if (serializer["Scripts"][scriptUID]["properties"].contains(name))
                        {
                            serializer["Scripts"][scriptUID]["properties"][name]["Offset"] = offset;
                        }
                    }
                }

                // Handle Property Offsets Explicitly
                std::regex propOffsetPattern(R"(\s*0x([0-9a-fA-F]+)\s*JNV_([a-f0-9]+)::(__prop_\w+).*$)");
                if (std::regex_search(line, match, propOffsetPattern))
                {
                    uint64_t offset = std::stoull(match[1], nullptr, 16);
                    std::string scriptUID = match[2];
                    std::string propName = match[3].str();

                    // Clean Property Name
                    jenova::ReplaceAllMatchesWithString(propName, "__prop_", "");

                    if (serializer["Scripts"].contains(scriptUID) && serializer["Scripts"][scriptUID]["properties"].contains(propName))
                    {
                        serializer["Scripts"][scriptUID]["properties"][propName]["Offset"] = offset;
                    }
                }
            }

            // Add Properties Definitions
            for (const auto& scriptModule : scriptModules)
            {
                if (scriptModule.scriptPropertiesFile.is_empty()) continue;
                if (std::filesystem::exists(AS_STD_STRING(scriptModule.scriptPropertiesFile)))
                {
                    std::string propDatabase = jenova::ReadStdStringFromFile(AS_STD_STRING(scriptModule.scriptPropertiesFile));
                    if (!propDatabase.empty())
                    {
                        serializer["Scripts"][AS_STD_STRING(scriptModule.scriptUID)]["database"]["properties"] = jenova::json_t::parse(propDatabase);
                    }
                }
            }

            // Add Extra Info
            if (buildResult.hasDebugInformation)
            {
                serializer["HasDebugInformation"] = true;
                serializer["BuildPath"] = buildResult.buildPath;
            }
            else
            {
                serializer["HasDebugInformation"] = false;
            }
            serializer["ModuleBinarySize"] = buildResult.builtModuleData.size();
            serializer["InterpreterBackend"] = JenovaInterpreter::GetInterpreterBackend();
            serializer["DeveloperMode"] = jenova::GlobalStorage::DeveloperModeActivated;

            // Dump Metadata If Developer Mode Activated
            if (jenova::GlobalStorage::DeveloperModeActivated)
            {
                jenova::WriteStdStringToFile(AS_STD_STRING(jenova::GetJenovaCacheDirectory() + "Jenova.Metadata.json"), serializer.dump(4));
            }

            // Serialize Data
            return serializer.dump();
        }
        catch (const std::exception& err)
        {
            jenova::Error("Jenova Interpreter", "Failed to Parse Map and Generate Metadata, Parser Error : %s", err.what());
        }
    }

    #endif // Linux Compilers

	// Generation Failed, Return Empty Data
	return jenova::SerializedData();
}
bool JenovaInterpreter::UpdateConfigurationsFromMetaData(const jenova::SerializedData& metaData)
{
    try
    {
        // Parse and Set Interpreter Metadata
        moduleMetaData = jenova::json_t::parse(metaData);

        // Set Interpreter Backend
        if (moduleMetaData.contains("InterpreterBackend"))
        {
            SetInterpreterBackend(moduleMetaData["InterpreterBackend"].get<jenova::InterpreterBackend>());
        }

        // Set Has Debug Information If Present
        if (moduleMetaData["HasDebugInformation"].get<bool>() == true)
        {
            hasDebugInformation = true;
            moduleDiskPath = std::filesystem::absolute(moduleMetaData["BuildPath"].get<std::string>()).string();
        }

        // Set Debug Mode If Present
        if (moduleMetaData.contains("DebugMode"))
        {
            if (moduleMetaData["DebugMode"].get<bool>() == true)
            {
                executeInDebugMode = true;
            }
        }

        // Update Global Storage From Metadata
        if (!QUERY_ENGINE_MODE(Editor))
        {
            if (moduleMetaData.contains("DeveloperMode")) jenova::GlobalStorage::DeveloperModeActivated = moduleMetaData["DeveloperMode"].get<bool>();
            if (moduleMetaData.contains("ManagedSafeExecution")) jenova::GlobalStorage::UseManagedSafeExecution = moduleMetaData["ManagedSafeExecution"].get<bool>();
        }

        // Update Module Binary Size
        moduleBinarySize = moduleMetaData["ModuleBinarySize"].get<size_t>();

        // All Good
        return true;
    }
    catch (const std::exception& err)
    {
        jenova::Error("Jenova Interpreter", "Failed to Parse Metadata, Parser Error : %s", err.what());
        return false;
    }
}
bool JenovaInterpreter::UpdatePropertyStorageFromMetaData()
{
    // Clean Storage
    propertyStorage.clear();

    // Update Property Storage
    try
    {
        // Get Scripts
        jenova::json_t moduleScripts = moduleMetaData["Scripts"];

        // Extract Properties from Metadata
        for (const auto& moduleScript : moduleScripts.items())
        {
            std::string scriptUID = moduleScript.key();

            if (moduleScript.value().contains("database"))
            {
                if (moduleScript.value()["database"].contains("properties"))
                {
                    for (const auto& scriptProperty : moduleScript.value()["database"]["properties"])
                    {
                        // Get Property Name
                        std::string propertyName = scriptProperty["PropertyName"].get<std::string>();

                        // Allocate Property
                        void* propertyPtr = jenova::AllocateVariantBasedProperty(scriptProperty["PropertyType"].get<std::string>());

                        // Validate Property Pointer
                        if (!propertyPtr)
                        {
                            jenova::Error("Jenova Interpreter", "Failed to Allocate Property %s From Script %s", propertyName.c_str(), scriptUID.c_str());
                            return false;
                        }

                        // Create Property Key
                        std::string propertyKey = scriptUID + "_" + propertyName;

                        // Add Allocated Property
                        propertyStorage.insert(std::make_pair(propertyKey, propertyPtr));

                        // Verbose
                        jenova::VerboseByID(__LINE__, "Allocating Script [%s] Property [%s]", scriptUID.c_str(), propertyName.c_str());
                    }
                }
            }
        }

        // All Good
        return true;
    }
    catch (const std::exception& err)
    {
        jenova::Error("Jenova Interpreter", "Failed to Parse Metadata, Parser Error : %s", err.what());
        return false;
    }
}
bool JenovaInterpreter::FlushPropertyStorage()
{
    for (auto scriptProperty : propertyStorage)
    {
        if (scriptProperty.second) delete scriptProperty.second;
    }
    propertyStorage.clear();
    return true;
}
jenova::PropertyList JenovaInterpreter::GetPropertiesList(std::string& scriptUID)
{
    try
    {
        // Create Property List
        jenova::PropertyList propertyNames;

        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["properties"];

        // Add Property to List
        for (const auto& propertyName : scriptMetadata.items()) propertyNames.push_back(propertyName.key());

        // Return List
        return propertyNames;
    }
    catch (const std::exception&)
    {
        return jenova::PropertyList();
    }
}
std::string JenovaInterpreter::GetPropertyType(const std::string& propertyName, std::string& scriptUID)
{
    try
    {
        // Validate Script UID
        if (!moduleMetaData["Scripts"].contains(scriptUID)) return std::string();

        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["properties"];

        // Find and Return Property Type
        for (const auto& prop : scriptMetadata.items())
        {
            if (prop.key() == propertyName) return prop.value()["Type"].get<std::string>();
        }
    }
    catch (const std::exception&)
    {
        return std::string();
    }

    // Property was not found
    return std::string();
}
jenova::PropertyAddress JenovaInterpreter::GetPropertyAddress(const std::string& propertyName, std::string& scriptUID)
{
    try
    {
        // Validate Script UID
        if (!moduleMetaData["Scripts"].contains(scriptUID)) return 0;

        // Get Script Metadata by UID
        jenova::json_t scriptMetadata = moduleMetaData["Scripts"][scriptUID]["properties"];

        // Find and return Property Address
        for (const auto& prop : scriptMetadata.items())
        {
            if (prop.key() == propertyName)
            {
                // Calculate Offset + BaseAddress and Return the Address
                jenova::PropertyAddress propertyOffset = prop.value()["Offset"].get<jenova::PropertyAddress>();
                return moduleBaseAddress + propertyOffset;
            }
        }
    }
    catch (const std::exception&)
    {
        return 0;
    }

    // Property was not found
    return 0;
}
jenova::PropertyPointer JenovaInterpreter::GetPropertyPointer(const String& propertyName, const String& scriptUID)
{
    // Create Property Key
    std::string propertyKey = AS_STD_STRING(scriptUID + String("_") + propertyName.get_file());

    // Validate Property
    if (!propertyStorage.contains(propertyKey)) return nullptr;

    // Get Property Pointer from Storage
    jenova::PropertyPointer propertyPtr = propertyStorage[propertyKey];

    // Return Property Pointer
    return propertyPtr;
}
bool JenovaInterpreter::SetPropertyValueFromVariant(const String& propertyName, const Variant& propertyValue, const String& scriptUID)
{
    // Get Property Pointer from Storage
    jenova::PropertyPointer propertyPtr = GetPropertyPointer(propertyName, scriptUID);

    // Validate Property Pointer
    if (propertyPtr == nullptr) return false;

    // Set Property Value from Variant
    if (!jenova::SetPropertyPointerValueFromVariant(propertyPtr, propertyValue)) return false;

    // Get Property Address
    jenova::PropertyAddress propertyAddress = JenovaInterpreter::GetPropertyAddress(AS_STD_STRING(propertyName.get_file()), AS_STD_STRING(scriptUID));
    if (!propertyAddress) return false;

    // Set Property Pointer
    if (propertySetMethod == jenova::PropertySetMethod::DirectAssign) *(void**)propertyAddress = propertyPtr;
    if (propertySetMethod == jenova::PropertySetMethod::MemoryCopy) memcpy((void*)propertyAddress, &propertyPtr, sizeof(propertyPtr));

    // All Good
    return true;
}
jenova::InterpreterBackend JenovaInterpreter::GetInterpreterBackend()
{
    return interpreterBackend;
}
void JenovaInterpreter::SetInterpreterBackend(jenova::InterpreterBackend newBackend)
{
    interpreterBackend = newBackend;
}
jenova::FunctionPointer JenovaInterpreter::SolveVirtualFunction(jenova::ModuleHandle moduleHandle, const char* functionName)
{
    return JenovaLoader::GetVirtualFunction(moduleHandle, functionName);
}
void JenovaInterpreter::SetDebugModeExecutionState(bool debugModeState)
{
    executeInDebugMode = debugModeState;
}
jenova::ModuleHandle JenovaInterpreter::LoadShellModule(const uint8_t* moduleDataPtr, const size_t moduleSize)
{
    return JenovaLoader::LoadModule((void*)moduleDataPtr, moduleSize);
}

// Jenova Interpreter Implementation :: Module Database
bool JenovaInterpreter::CreateModuleDatabase(const std::string& moduleDatabaseName, const uint8_t* moduleDataPtr, const size_t moduleSize, const jenova::SerializedData& metaData)
{
    // Verbose
    jenova::VerboseByID(__LINE__, "Caching Jenova Compiled Module In Database...");

    // Validate Inputs
    if (moduleDatabaseName.empty()) return false;
    if (!moduleDataPtr || moduleSize == 0 || metaData.empty()) return false;

    // Create Header
    jenova::ModuleDatabaseHeader moduleDatabaseHeader;
    moduleDatabaseHeader.moduleSize = moduleSize;
    moduleDatabaseHeader.metaDataSize = metaData.size();
    moduleDatabaseHeader.databaseType = jenova::ModuleCacheType::OpenSource;

    // Set Database Version
    const unsigned char appVersionData[4] = { APP_VERSION_DATA };
    std::memcpy(moduleDatabaseHeader.databaseVersion, appVersionData, sizeof(appVersionData));

    // Compress Module Data
    jenova::MemoryBuffer databaseRawBuffer;
    databaseRawBuffer.insert(databaseRawBuffer.end(), moduleDataPtr, moduleDataPtr + moduleSize);
    databaseRawBuffer.insert(databaseRawBuffer.end(), metaData.begin(), metaData.end());
    jenova::MemoryBuffer compressedData = jenova::CompressBuffer(databaseRawBuffer.data(), databaseRawBuffer.size());

    // Update Compression Ratio
    moduleDatabaseHeader.compressionRatio = jenova::CalculateCompressionRatio(databaseRawBuffer.size(), compressedData.size());

    // Update Encoded Data Size
    moduleDatabaseHeader.encodedDataSize = compressedData.size();

    // Write Database to Disk
    std::string defaultModuleDatabasePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + moduleDatabaseName;
    std::fstream databaseWriter;
    databaseWriter.open(defaultModuleDatabasePath, std::ios::binary | std::ios::out);
    databaseWriter.write((char*)&moduleDatabaseHeader, sizeof(jenova::ModuleDatabaseHeader));
    databaseWriter.write((char*)compressedData.data(), compressedData.size());
    databaseWriter.close();

    // Release Buffers
    jenova::MemoryBuffer().swap(databaseRawBuffer);
    jenova::MemoryBuffer().swap(compressedData);

    // Verbose
    jenova::VerboseByID(__LINE__, "Code Compression Ratio : %02f%%", moduleDatabaseHeader.compressionRatio);
    jenova::VerboseByID(__LINE__, "Jenova Compiled Module Database Cached At (%s)", defaultModuleDatabasePath.c_str());

    // All Good
    return true;
}
bool JenovaInterpreter::CreateModuleDatabase(const std::string& moduleDatabaseName, const jenova::BuildResult& buildResult)
{
    return CreateModuleDatabase(moduleDatabaseName, buildResult.builtModuleData.data(), buildResult.builtModuleData.size(), buildResult.moduleMetaData);
}
bool JenovaInterpreter::DeployFromDatabase(const std::string& moduleDatabaseName)
{
    // Verbose
    jenova::VerboseByID(__LINE__, "Loading Jenova Compiled Module Database...");

    // Database Raw Data
    jenova::MemoryBuffer databaseRawData;

    // Create Database File Path
    String defaultModuleDatabasePath = String(jenova::GlobalSettings::DefaultJenovaBootPath) + AS_GD_STRING(moduleDatabaseName);

    // Update Database File Path if In Editor/Debugging
    if (QUERY_ENGINE_MODE(Editor) || QUERY_ENGINE_MODE(Debug) && !jenova::IsEngineRuntimeExport())
    {
        defaultModuleDatabasePath = String(jenova::GetJenovaCacheDirectory()) + AS_GD_STRING(moduleDatabaseName);
    }

    // Open & Validate File
    Ref<FileAccess> moduleDatabaseReader = FileAccess::open(defaultModuleDatabasePath, FileAccess::READ);
    if (!moduleDatabaseReader.is_valid()) return false;

    // Read File
    PackedByteArray databaseFileBytes = moduleDatabaseReader->get_buffer(moduleDatabaseReader->get_length());
    databaseRawData.assign(databaseFileBytes.ptr(), databaseFileBytes.ptr() + databaseFileBytes.size());
    databaseFileBytes.clear();
    moduleDatabaseReader->close();

    // Validate Buffer
    if (databaseRawData.size() == 0) return false;

    // Parse And Get Header
    jenova::ModuleDatabaseHeader* databaseHeader = (jenova::ModuleDatabaseHeader*)&databaseRawData[0];

    // Validate Header Magic Number
    const unsigned char magicNumber[16] = { 0x5F, 0x5F, 0x4A, 0x45, 0x4E, 0x4F, 0x56, 0x41, 0x5F, 0x43, 0x41, 0x43, 0x48, 0x45, 0x5F, 0x5F };
    if (memcmp(magicNumber, databaseHeader->magicNumber, sizeof(magicNumber)) != 0)
    {
        jenova::Error("Jenova Interpreter", "Jenova Module Database is Invalid!");
        return false;
    }

    // Validate Package Cache Type
    if (databaseHeader->databaseType != jenova::ModuleCacheType::OpenSource)
    {
        std::string moduleDatbaseType = "Unknown";
        if (databaseHeader->databaseType == jenova::ModuleCacheType::Proprietary) moduleDatbaseType = "Proprietary";
        jenova::Error("Jenova Interpreter", "Unable to Load Jenova Module Database.\n" \
            "Module Built with %s version of Jenova Framework while Runtime is Open-Source version.", moduleDatbaseType.c_str());
        return false;
    }

    // Validate Package Version
    const unsigned char appVersionData[4] = { APP_VERSION_DATA };
    if (memcmp(appVersionData, databaseHeader->databaseVersion, sizeof(appVersionData)) != 0)
    {
        // Warn User About Database Version Mismatch
        jenova::Warning("Jenova Interpreter", 
            "Jenova Module Database Version Mismatch (Runtime : %d.%d.%d.%d | Database : %d.%d.%d.%d)\nThis May Leads to Unexpected Behaviour! " \
            "Consider Updating Package With Latest Version.",
            appVersionData[0], appVersionData[1], appVersionData[2], appVersionData[3],
            databaseHeader->databaseVersion[0], databaseHeader->databaseVersion[1], databaseHeader->databaseVersion[2], databaseHeader->databaseVersion[3]);
    }

    // Decompress Data
    const uint8_t* databaseEncodedDataPtr = (const uint8_t*)&databaseRawData[sizeof(jenova::ModuleDatabaseHeader)];
    jenova::MemoryBuffer encodedRawBuffer;
    encodedRawBuffer.insert(encodedRawBuffer.end(), databaseEncodedDataPtr, databaseEncodedDataPtr + databaseHeader->encodedDataSize);
    jenova::MemoryBuffer decompressedData = jenova::DecompressBuffer(encodedRawBuffer.data(), encodedRawBuffer.size());
    if (decompressedData.size() == 0) return false;

    // Get Data Pointers
    const uint8_t* moduleDataPtr = decompressedData.data();
    const size_t moduleSize = databaseHeader->moduleSize;
    const char* metaDataDataPtr = (const char*)&decompressedData[databaseHeader->moduleSize];
    const jenova::SerializedData metaData(metaDataDataPtr, databaseHeader->metaDataSize);

    // Check If Module Is Already Loaded
    if (GetModuleBaseAddress() == 0)
    {
        // Load Module
        if (!LoadModule(moduleDataPtr, moduleSize, metaData))
        {
            jenova::Error("Jenova Interpreter", "Unable to Deploy and Load Compiled Jenova Module From Database.");
            std::vector<uint8_t>().swap(databaseRawData);
            return false;
        }
    }
    else
    {
        // Reload Module
        if (!JenovaInterpreter::ReloadModule(moduleDataPtr, moduleSize, metaData))
        {
            jenova::Error("Jenova Interpreter", "Unable to Deploy and Reload Compiled Jenova Module From Database.");
            return false;
        }
    }

    // Release Buffers
    jenova::MemoryBuffer().swap(databaseRawData);
    jenova::MemoryBuffer().swap(encodedRawBuffer);
    jenova::MemoryBuffer().swap(decompressedData);

    // Verbose
    jenova::VerboseByID(__LINE__, "Jenova Compiled Module Deployed and Loaded from Database Cache.");

    // All Good
    return true;
}
bool JenovaInterpreter::IsDatabaseAvailable(const std::string& moduleDatabaseName)
{
    // Verbose
    jenova::VerboseByID(__LINE__, "Validating Jenova Compiled Module Database...");

    // Create Database File Path
    String defaultModuleDatabasePath = String(jenova::GlobalSettings::DefaultJenovaBootPath) + AS_GD_STRING(moduleDatabaseName);

    // Update Database File Path only if in Editor or Debug mode, but NOT in Runtime
    if ((QUERY_ENGINE_MODE(Editor) || QUERY_ENGINE_MODE(Debug)) && !jenova::IsEngineRuntimeExport())
    {
        defaultModuleDatabasePath = String(jenova::GetJenovaCacheDirectory()) + AS_GD_STRING(moduleDatabaseName);
    }

    // Check If Database File Exists
    return FileAccess::file_exists(defaultModuleDatabasePath);
}