
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

// Jenova Namespace
namespace jenova
{
    // Windows Compilers
    #ifdef TARGET_PLATFORM_WINDOWS

    // Jenova Microsoft Visual C++ Compiler Implementation
    class MicrosoftCompiler : public IJenovaCompiler
    {
    public:
        MicrosoftCompiler(bool useLLVM)
        {
            // Set Sub-Compiler Model
            compilerModel = useLLVM ? CompilerModel::ClangLLVMCompiler : CompilerModel::MicrosoftCompiler;
        }
        ~MicrosoftCompiler()
        {
        }
        bool InitializeCompiler(String compilerInstanceName = "<JenovaMSVCCompiler>")
        {
            // Microsoft Compiler Default Settings
            if (this->GetCompilerModel() == CompilerModel::MicrosoftCompiler)
            {
                // Initialize Tool Chain Settings
                internalDefaultSettings["instance_name"]                        = compilerInstanceName;
                internalDefaultSettings["instance_version"]                     = 1.4f;
                internalDefaultSettings["cpp_toolchain_path"]                   = "/Jenova/Compilers/JenovaMSVCCompiler";
                internalDefaultSettings["cpp_compiler_binary"]                  = "/Bin/cl.exe";
                internalDefaultSettings["cpp_linker_binary"]                    = "/Bin/link.exe";
                internalDefaultSettings["cpp_include_path"]                     = "/Include";
                internalDefaultSettings["cpp_library_path"]                     = "/Lib";
                internalDefaultSettings["cpp_symbols_path"]                     = "/Symbols";
                internalDefaultSettings["cpp_jenovasdk_path"]                   = "/Jenova/JenovaSDK";
                internalDefaultSettings["cpp_godotsdk_path"]                    = "/Jenova/GodotSDK";

                // MSVC Compiler Settings
                internalDefaultSettings["cpp_language_standards"]               = "cpp20";                          /* /std:c++20 [cpp20, cpp17] */
                internalDefaultSettings["cpp_clean_stack"]                      = true;                             /* /Gd */
                internalDefaultSettings["cpp_intrinsic_functions"]              = true;                             /* /Oi */
                internalDefaultSettings["cpp_open_mp_support"]                  = true;                             /* /openmp */
                internalDefaultSettings["cpp_multithreaded"]                    = true;                             /* /MT */
                internalDefaultSettings["cpp_debug_database"]                   = true;                             /* /Zi */
                internalDefaultSettings["cpp_conformance_mode"]                 = true;                             /* /permissive vs /permissive- */
                internalDefaultSettings["cpp_exception_handling"]               = 2;                                /* 1 : /EHsc 2: /EHa */
                internalDefaultSettings["cpp_use_task_system"]                  = false;                            /* Internal Multiprocessing vs Task System */
                internalDefaultSettings["cpp_extra_compiler"]                   = "/Ot /Ox /GR /bigobj";            /* Extra Compiler Options Like /Zc:threadSafeInit /Bt /Zc:tlsGuards /d1reportTime */
                internalDefaultSettings["cpp_definitions"]                      = "TYPED_METHOD_BIND;HOT_RELOAD_ENABLED;_WINDLL";

                // MSVC Linker Settings
                internalDefaultSettings["cpp_output_module"]                    = "Jenova.Module.jnv";              /* Must use .dll for Debug .jnv for Final*/
                internalDefaultSettings["cpp_output_map"]                       = "Jenova.Module.map";
                internalDefaultSettings["cpp_output_database"]                  = "Jenova.Module.Database.pdb";
                internalDefaultSettings["cpp_default_libs"]                     = "kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;delayimp.lib";
                internalDefaultSettings["cpp_native_libs"]                      = "libGodot.x64.lib;";
                internalDefaultSettings["cpp_delayed_dll"]                      = "/DELAYLOAD:\"Jenova.Runtime.Win64.dll\"";
                internalDefaultSettings["cpp_default_subsystem"]                = "Console";                        /* /SUBSYSTEM:CONSOLE [Console, GUI]*/
                internalDefaultSettings["cpp_machine_architecture"]             = "Win64";                          /* /MACHINE:X64 [Win64, Win32]*/
                internalDefaultSettings["cpp_machine_pe_type"]                  = "dll";                            /* /DLL [dll, exe]*/
                internalDefaultSettings["cpp_add_manifest"]                     = true;                             /* /MANIFEST */
                internalDefaultSettings["cpp_dynamic_base"]                     = true;                             /* /DYNAMICBASE */
                internalDefaultSettings["cpp_debug_symbol"]                     = true;                             /* /DEBUG:FULL */
                internalDefaultSettings["cpp_extra_linker"]                     = "/OPT:ICF /OPT:NOREF /LTCG";      /* Extra Linker Options */
            }

            // Microsoft Compiler LLVM Default Settings
            if (this->GetCompilerModel() == CompilerModel::ClangLLVMCompiler)
            {
                // Initialize Tool Chain Settings
                internalDefaultSettings["instance_name"]                        = compilerInstanceName;
                internalDefaultSettings["instance_version"]                     = 1.4f;
                internalDefaultSettings["cpp_toolchain_path"]                   = "/Jenova/Compilers/JenovaMSVCCompiler";
                internalDefaultSettings["cpp_compiler_binary"]                  = "/Bin/clang-cl.exe";
                internalDefaultSettings["cpp_linker_binary"]                    = "/Bin/lld-link.exe";
                internalDefaultSettings["cpp_include_path"]                     = "/Include";
                internalDefaultSettings["cpp_library_path"]                     = "/Lib";
                internalDefaultSettings["cpp_symbols_path"]                     = "/Symbols";
                internalDefaultSettings["cpp_jenovasdk_path"]                   = "/Jenova/JenovaSDK";
                internalDefaultSettings["cpp_godotsdk_path"]                    = "/Jenova/GodotSDK";

                // MSVC Compiler Settings
                internalDefaultSettings["cpp_language_standards"]               = "cpp20";                          /* /std:c++20 [cpp20, cpp17] */
                internalDefaultSettings["cpp_clean_stack"]                      = true;                             /* /Gd */
                internalDefaultSettings["cpp_intrinsic_functions"]              = true;                             /* /Oi */
                internalDefaultSettings["cpp_open_mp_support"]                  = true;                             /* /openmp */
                internalDefaultSettings["cpp_multithreaded"]                    = true;                             /* /MT */
                internalDefaultSettings["cpp_debug_database"]                   = true;                             /* /Zi */
                internalDefaultSettings["cpp_conformance_mode"]                 = true;                             /* /permissive vs /permissive- */
                internalDefaultSettings["cpp_exception_handling"]               = 2;                                /* 1 : /EHsc 2: /EHa */
                internalDefaultSettings["cpp_use_task_system"]                  = true;                             /* Internal Multiprocessing vs Task System */
                internalDefaultSettings["cpp_extra_compiler"]                   = "/Ot /Ox /GR /bigobj -w";         /* Extra Compiler Options Like /Zc:threadSafeInit /Bt /Zc:tlsGuards /d1reportTime */
                internalDefaultSettings["cpp_definitions"]                      = "TYPED_METHOD_BIND;HOT_RELOAD_ENABLED;_WINDLL";

                // MSVC Linker Settings
                internalDefaultSettings["cpp_output_module"]                    = "Jenova.Module.jnv";              /* Must use .dll for Debug .jnv for Final*/
                internalDefaultSettings["cpp_output_map"]                       = "Jenova.Module.map";
                internalDefaultSettings["cpp_output_database"]                  = "Jenova.Module.Database.pdb";
                internalDefaultSettings["cpp_default_libs"]                     = "kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;delayimp.lib";
                internalDefaultSettings["cpp_native_libs"]                      = "libGodot.x64.lib;";
                internalDefaultSettings["cpp_delayed_dll"]                      = "/DELAYLOAD:\"Jenova.Runtime.Win64.dll\"";
                internalDefaultSettings["cpp_default_subsystem"]                = "Console";                        /* /SUBSYSTEM:CONSOLE [Console, GUI]*/
                internalDefaultSettings["cpp_machine_architecture"]             = "Win64";                          /* /MACHINE:X64 [Win64, Win32]*/
                internalDefaultSettings["cpp_machine_pe_type"]                  = "dll";                            /* /DLL [dll, exe]*/
                internalDefaultSettings["cpp_add_manifest"]                     = true;                             /* /MANIFEST */
                internalDefaultSettings["cpp_dynamic_base"]                     = true;                             /* /DYNAMICBASE */
                internalDefaultSettings["cpp_debug_symbol"]                     = true;                             /* /DEBUG:FULL */
                internalDefaultSettings["cpp_extra_linker"]                     = "/OPT:ICF /OPT:NOREF /LTCG";      /* Extra Linker Options */
            }

            // All Good
            return true;
        }
        bool ReleaseCompiler()
        {
            // Release Resources
            internalDefaultSettings.clear();
            return true;
        }
        String PreprocessScript(Ref<CPPScript> cppScript, const Dictionary& preprocessorSettings)
        {
            // Get Original Source Code
            String scriptSourceCode = cppScript->get_source_code();

            // Reset Line Number
            scriptSourceCode = scriptSourceCode.insert(0, "#line 1\n");

            // Process And Extract Properties
            jenova::SerializedData propertiesMetadata = jenova::ProcessAndExtractPropertiesFromScript(scriptSourceCode, cppScript->GetScriptIdentity());
            if (!propertiesMetadata.empty() && propertiesMetadata != "null") jenova::WriteStdStringToFile(AS_STD_STRING(String(preprocessorSettings["PropertyMetadata"])), propertiesMetadata);

            // Preprocessor Definitions [Header]
            String preprocessorDefinitions = "// Jenova Preprocessor Definitions\n";

            // Preprocessor Definitions [Version]
            preprocessorDefinitions += String(jenova::Format("#define JENOVA_VERSION \"%d.%d.%d.%d\"\n",
                jenova::GlobalSettings::JenovaBuildVersion[0], jenova::GlobalSettings::JenovaBuildVersion[1],
                jenova::GlobalSettings::JenovaBuildVersion[2], jenova::GlobalSettings::JenovaBuildVersion[3]).c_str());

            // Preprocessor Definitions [Compiler]
            if (this->GetCompilerModel() == CompilerModel::MicrosoftCompiler)
            {
                preprocessorDefinitions += "#define JENOVA_COMPILER \"Microsoft Visual C++ Compiler\"\n";
                preprocessorDefinitions += "#define MSVC_COMPILER\n";
            }
            if (this->GetCompilerModel() == CompilerModel::ClangLLVMCompiler)
            {
                preprocessorDefinitions += "#define JENOVA_COMPILER \"Microsoft Visual C++ Compiler LLVM\"\n";
                preprocessorDefinitions += "#define MSVC_LLVM_COMPILER\n";
            }

            // Preprocessor Definitions [User]
            String userPreprocessorDefinitions = preprocessorSettings["PreprocessorDefinitions"];
            PackedStringArray userPreprocessorDefinitionsList = userPreprocessorDefinitions.split(";");
            for (const auto& definition : userPreprocessorDefinitionsList) if (!definition.is_empty()) preprocessorDefinitions += "#define " + definition + "\n";

            // Add Final Preprocessor Definitions
            scriptSourceCode = scriptSourceCode.insert(0, preprocessorDefinitions + "\n");

            // Replecements
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptToolIdentifier, "#define TOOL_SCRIPT");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockBeginIdentifier, "namespace JNV_" + cppScript->GetScriptIdentity() + " {");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockEndIdentifier, "}; using namespace JNV_" + cppScript->GetScriptIdentity() + ";");
            scriptSourceCode = scriptSourceCode.replace(" OnReady", " _ready");
            scriptSourceCode = scriptSourceCode.replace(" OnAwake", " _enter_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnDestroy", " _exit_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnProcess", " _process");
            scriptSourceCode = scriptSourceCode.replace(" OnPhysicsProcess", " _physics_process");
            scriptSourceCode = scriptSourceCode.replace(" OnInput", " _input");
            scriptSourceCode = scriptSourceCode.replace(" OnUserInterfaceInput", " _gui_input");

            // Return Preprocessed Source
            return scriptSourceCode;
        }
        CompileResult CompileScript(const String sourceCode)
        {
            return CompileScriptWithCustomSettings(sourceCode, this->internalDefaultSettings);
        }
        CompileResult CompileScriptWithCustomSettings(const String sourceCode, const Dictionary& compilerSettings)
        {
            CompileResult result;
            result.hasError = true;
            result.compileError = "This Compiler does not support Compilation from Memory!";
            return result;
        }
        CompileResult CompileScriptFromFile(const String scriptFilePath)
        {
            return CompileScriptFromFileWithCustomSettings(scriptFilePath, this->internalDefaultSettings);
        }
        CompileResult CompileScriptFromFileWithCustomSettings(const String scriptFilePath, const Dictionary& compilerSettings)
        {
            CompileResult result;
            result.hasError = true;
            result.compileError = "The requested compilation method is currently Not Implemented.";
            return result;
        }
        CompileResult CompileScriptModuleContainer(const ScriptModuleContainer& scriptModulesContainer)
        {
            return CompileScriptModuleWithCustomSettingsContainer(scriptModulesContainer, this->internalDefaultSettings);
        }
        CompileResult CompileScriptModuleWithCustomSettingsContainer(const ScriptModuleContainer& scriptModulesContainer, const Dictionary& compilerSettings)
        {
            // Create Compiler Result
            CompileResult result;

            // Solve Settings [Cache]
            if (!SolveCompilerSettings(compilerSettings))
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C666 : Unable to Solve Compiler Settings.";
                return result;
            };

            // Utilities
            auto GeneratePreprocessDefinitions = [](const godot::String& defsSetting) -> std::string 
            {
                std::string defs = AS_STD_STRING(defsSetting);
                if (defs.empty()) return "";
                if (defs.back() == ';') defs.pop_back();
                std::vector<std::string> defsArray;
                size_t start = 0;
                size_t end = defs.find(';');
                while (end != std::string::npos) 
                {
                    defsArray.push_back(defs.substr(start, end - start));
                    start = end + 1;
                    end = defs.find(';', start);
                }
                defsArray.push_back(defs.substr(start));
                std::string result;
                for (const auto& def : defsArray) result += "/D \"" + def + "\" ";
                return result;
            };
            auto GenerateAdditionalIncludeDirectories = [](const godot::String& additionalDirs) -> std::string
                {
                    std::string dirs = AS_STD_STRING(additionalDirs);
                    if (dirs.empty()) return "";
                    if (dirs.back() == ';') dirs.pop_back();
                    std::vector<std::string> dirArray;
                    size_t start = 0;
                    size_t end = dirs.find(';');
                    while (end != std::string::npos)
                    {
                        dirArray.push_back(dirs.substr(start, end - start));
                        start = end + 1;
                        end = dirs.find(';', start);
                    }
                    dirArray.push_back(dirs.substr(start));
                    std::string result;
                    for (const auto& dir : dirArray) result += "/I \"" + dir + "\" ";
                    return result;
                };

            // Generate Compiler Arguments
            std::string compilerArgument;
            compilerArgument += "\"" + this->compilerBinaryPath + "\" /c ";
            if (String(compilerSettings["cpp_language_standards"]) == "cpp23") compilerArgument += "/std:c++23 ";
            if (String(compilerSettings["cpp_language_standards"]) == "cpp20") compilerArgument += "/std:c++20 ";
            if (String(compilerSettings["cpp_language_standards"]) == "cpp17") compilerArgument += "/std:c++17 ";
            if (bool(compilerSettings["cpp_clean_stack"])) compilerArgument += "/Gd ";
            if (bool(compilerSettings["cpp_intrinsic_functions"])) compilerArgument += "/Oi ";
            if (bool(compilerSettings["cpp_open_mp_support"])) compilerArgument += "/openmp ";
            if (bool(compilerSettings["cpp_multithreaded"])) compilerArgument += "/MT ";
            if (bool(compilerSettings["cpp_debug_database"]) && bool(compilerSettings["cpp_generate_debug_info"]))
            {
                compilerArgument += "/Zi ";
                compilerArgument += "/Fd\"" + this->jenovaCachePath + AS_STD_STRING(String(compilerSettings["cpp_output_database"])) + "\" ";;
            }
            compilerArgument += bool(compilerSettings["cpp_conformance_mode"]) ? "/permissive- " : "/permissive ";
            if (int(compilerSettings["cpp_exception_handling"]) == 1) compilerArgument += "/EHsc ";
            if (int(compilerSettings["cpp_exception_handling"]) == 2) compilerArgument += "/EHa ";
            compilerArgument += GeneratePreprocessDefinitions(compilerSettings["cpp_definitions"]);
            compilerArgument += "/I \"./\" ";
            compilerArgument += "/I \"" + this->includePath + "\" ";
            compilerArgument += "/I \"" + this->jenovaSDKPath + "\" ";
            compilerArgument += "/I \"" + this->godotSDKPath + "\" ";

            // Force Include JenovaSDK Header
            if (jenova::GlobalSettings::ForceJenovaSDKHeader && jenova::GlobalStorage::UseBuiltinSDK) compilerArgument += "/FI JenovaSDK.h ";

            // Add Additional Include Directories
            compilerArgument += GenerateAdditionalIncludeDirectories(compilerSettings["cpp_extra_include_directories"]);

            // Add Packages Headers (Addons, Libraries etc.)
            for (const auto& addonConfig : jenova::GetInstalledAddons())
            {
                // Check For Addon Type
                if (addonConfig.Type == "RuntimeModule")
                {
                    if (!addonConfig.Header.empty())
                    {
                        if (addonConfig.Global)
                        {
                            std::string headerPath = addonConfig.Path + "/" + addonConfig.Header;
                            compilerArgument += "/FI \"" + headerPath + "\" ";
                        }
                        compilerArgument += "/I \"" + addonConfig.Path + "\" ";
                    }
                }
            }

            // Disable Logo
            compilerArgument += "/nologo ";

            // Load Cache if Exists
            bool buildCacheFileFound = false;
            jenova::json_t buildCacheDatabase;
            if (!std::filesystem::exists(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile))
            {
                // Cache Doesn't Exists Generate It [Required for Proxies]
                if (!jenova::CreateBuildCacheDatabase(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, scriptModulesContainer.scriptModules, compilerSettings["CppHeaderFiles"], true))
                {
                    result.compileResult = false;
                    result.hasError = true;
                    result.compileError = "C670 : Failed to Generate Build Cache Database.";
                    return result;
                }
            }

            // Parse Cache File
            try
            {
                std::ifstream buildCacheDatabaseReader(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, std::ios::binary);
                std::string buildCacheDatabaseContent(std::istreambuf_iterator<char>(buildCacheDatabaseReader), {});
                if (!buildCacheDatabaseContent.empty())
                {
                    buildCacheDatabase = jenova::json_t::parse(buildCacheDatabaseContent);
                    buildCacheFileFound = true;
                }
            }
            catch (const std::exception&) 
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C671 : Failed to Parse Build Cache Database.";
                return result;
            }

            // Check If Any Changes Applied to Headers
            if (buildCacheDatabase.contains("Headers"))
            {
                // Detect Changes
                bool detectedHeaderChanges = false;
                PackedStringArray cppHeaderFiles = compilerSettings["CppHeaderFiles"];
                for (const auto& cppHeaderFile : cppHeaderFiles)
                {
                    String cppHeaderUID = jenova::GenerateStandardUIDFromPath(cppHeaderFile);
                    String cppHeaderHash = jenova::GenerateMD5HashFromFile(cppHeaderFile);
                    if (buildCacheDatabase["Headers"].contains(AS_STD_STRING(cppHeaderUID)))
                    {
                        if (AS_STD_STRING(cppHeaderHash) != buildCacheDatabase["Headers"][AS_STD_STRING(cppHeaderUID)].get<std::string>())
                        {
                            detectedHeaderChanges = true;
                            break;
                        }
                    }
                }
                if (buildCacheDatabase.contains("HeaderCount"))
                {
                    int headerCount = buildCacheDatabase["HeaderCount"].get<int>();
                    if (headerCount != cppHeaderFiles.size()) detectedHeaderChanges = true;
                }

                // If Contains Changes Reset All Script Cache
                if (detectedHeaderChanges)
                {
                    if (buildCacheDatabase.contains("Modules")) for (const auto& scriptModule : buildCacheDatabase["Modules"].items()) scriptModule.value() = "No Hash";
                }
            }

            // Add Source/Output Based On Compile Model
            jenova::ModuleList compilationScripts;
            if (bool(compilerSettings["cpp_multi_threaded_compilation"]))
            {
                for (const auto& scriptModule : scriptModulesContainer.scriptModules)
                {
                    // Skip If File Hash Didn't Change
                    if (buildCacheDatabase.contains("Modules"))
                    {
                        if (buildCacheDatabase["Modules"].contains(AS_STD_STRING(scriptModule.scriptUID)))
                        {
                            if (AS_STD_STRING(scriptModule.scriptHash) == buildCacheDatabase["Modules"][AS_STD_STRING(scriptModule.scriptUID)].get<std::string>()) continue;
                        }
                    }

                    // Add Source
                    compilationScripts.push_back(scriptModule);

                    // Add Script Count
                    result.scriptsCount++;
                }
            }
            else
            {
                // If Script Has Changes Add to Compilation Scripts
                if (buildCacheDatabase.contains("Modules"))
                {
                    if (buildCacheDatabase["Modules"].contains(AS_STD_STRING(scriptModulesContainer.scriptModule.scriptUID)))
                    {
                        if (AS_STD_STRING(scriptModulesContainer.scriptModule.scriptHash) != buildCacheDatabase["Modules"][AS_STD_STRING(scriptModulesContainer.scriptModule.scriptUID)].get<std::string>())
                        {
                            compilationScripts.push_back(scriptModulesContainer.scriptModule);
                            result.scriptsCount++;
                        }
                    }
                }
            }

            // Skip Compile If Source Count is 0
            if (result.scriptsCount == 0)
            {
                result.compileResult = true;
                result.hasError = false;
                result.compileVerbose = "Cache System Detected No Script Requires to be Compiled.";
                return result;
            }

            // Add Extra Options
            compilerArgument += AS_STD_STRING(String(compilerSettings["cpp_extra_compiler"])) + " ";

            // Dump Compiler Command If Developer Mode Enabled
            if (jenova::GlobalStorage::DeveloperModeActivated) jenova::WriteStdStringToFile(jenovaCachePath + "CompilerCommand.txt", compilerArgument);

            // Handle Multi Process Compilation
            if (bool(compilerSettings["cpp_multi_threaded_compilation"]))
            {
                // Using Task System
                if (bool(compilerSettings["cpp_use_task_system"]))
                {
                    // Verbose Multi Processing Model
                    jenova::Output("Using Task System for Multi-Processing...");

                    // Compile Scripts
                    std::vector<jenova::TaskID> compilationTasks;
                    bool compilationFailed = false;
                    std::mutex compilationMutex;
                    std::vector<std::string> errorMessages;

                    // Spawn Tasks Per Script
                    for (const auto& scriptModule : compilationScripts)
                    {
                        std::string command = compilerArgument;
                        command += "\"" + AS_STD_STRING(scriptModule.scriptCacheFile) + "\" ";
                        command += "-o \"" + AS_STD_STRING(scriptModule.scriptObjectFile) + "\" ";

                        // Fix Paths in Command
                        jenova::ReplaceAllMatchesWithString(command, "\\", "/");
                        jenova::ReplaceAllMatchesWithString(command, "\\\\", "/");

                        // Initiate Compilation Task
                        jenova::TaskID taskID = JenovaTaskSystem::InitiateTask([command, &compilationFailed, &compilationMutex, &errorMessages]()
                        {
                            STARTUPINFOW si;
                            PROCESS_INFORMATION pi;
                            ZeroMemory(&si, sizeof(si));
                            si.cb = sizeof(si);
                            si.dwFlags |= STARTF_USESTDHANDLES;

                            HANDLE hStdOutRead, hStdOutWrite;
                            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
                            CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
                            SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
                            si.hStdOutput = hStdOutWrite;
                            si.hStdError = hStdOutWrite;

                            std::wstring wCommand(command.begin(), command.end());
                            if (!CreateProcessW(NULL, &wCommand[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
                            {
                                std::lock_guard<std::mutex> lock(compilationMutex);
                                compilationFailed = true;
                                errorMessages.push_back("Failed to start compilation process.");
                                return;
                            }

                            CloseHandle(hStdOutWrite);

                            std::vector<char> buffer(4096);
                            DWORD bytesRead;
                            std::string compilerOutput;
                            while (ReadFile(hStdOutRead, buffer.data(), buffer.size(), &bytesRead, NULL) && bytesRead > 0)
                            {
                                compilerOutput.append(buffer.data(), bytesRead);
                            }

                            CloseHandle(hStdOutRead);
                            WaitForSingleObject(pi.hProcess, INFINITE);
                            DWORD exitCode;
                            GetExitCodeProcess(pi.hProcess, &exitCode);
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);

                            if (exitCode != 0)
                            {
                                std::lock_guard<std::mutex> lock(compilationMutex);
                                compilationFailed = true;
                                errorMessages.push_back(compilerOutput);
                            }
                        });
                        compilationTasks.push_back(taskID);
                    }

                    // Wait for all tasks to complete
                    for (const auto& taskID : compilationTasks)
                    {
                        while (!JenovaTaskSystem::IsTaskComplete(taskID))
                        {
                            std::this_thread::yield();
                        }
                        JenovaTaskSystem::ClearTask(taskID);
                    }

                    // Check Compile Result
                    if (compilationFailed)
                    {
                        result.compileResult = false;
                        result.hasError = true;
                        result.compileError = "C668 : One or more compilation tasks failed.\n";
                        for (const auto& errorMsg : errorMessages)
                        {
                            result.compileError += AS_GD_STRING(errorMsg) + "\n";
                        }
                        return result;
                    }

                    // Set Compiler Result
                    result.compileResult = !compilationFailed;
                    result.hasError = !compilationFailed;

                    // Yield Engine
                    OS::get_singleton()->delay_msec(1);
                    std::this_thread::yield();

                    // Return Final Result
                    return result;
                }

                // Using Internal Multiprocessing
                else
                {
                    // Verbose Multi Processing Model
                    jenova::Output("Using Compiler Native Multi-Processing...");

                    // Set Compiler Multi-Processing Mode & Set Output
                    compilerArgument += "/MP ";
                    compilerArgument += "/Fo\"" + this->jenovaCachePath + "\" ";

                    // Add Compilation Scripts to Compiler Argument
                    for (const auto& scriptModule : compilationScripts)
                    {
                        // Add Source
                        compilerArgument += "\"" + AS_STD_STRING(scriptModule.scriptCacheFile) + "\" ";
                    }
                }
            }

            // Handle Single Thread Compilation
            if (!bool(compilerSettings["cpp_multi_threaded_compilation"]))
            {
                compilerArgument += "\"" + AS_STD_STRING(scriptModulesContainer.scriptModule.scriptCacheFile) + "\" ";
                compilerArgument += "/Fo\"" + AS_STD_STRING(scriptModulesContainer.scriptModule.scriptObjectFile) + "\" ";
            }

            // Run Compiler
            STARTUPINFOW si = { 0 };
            PROCESS_INFORMATION pi;
            si.cb = sizeof(si);
            si.dwFlags |= STARTF_USESTDHANDLES;

            // Create pipes for capturing output
            HANDLE hStdOutRead, hStdOutWrite;
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
            SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
            si.hStdOutput = hStdOutWrite;
            si.hStdError = hStdOutWrite;

            // Convert command to wide string
            std::wstring wCompilerArgument(compilerArgument.begin(), compilerArgument.end());

            // Execute the command
            if (!CreateProcessW(NULL, &wCompilerArgument[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C667 : Failed to Run Compiler!";
                return result;
            }

            // Close write end of the pipe
            CloseHandle(hStdOutWrite);

            // Read And Filter Output
            std::vector<char> buffer(jenova::GlobalSettings::BuildOutputBufferSize);
            DWORD bytesRead;
            std::string resultOutput, accumulatedOutput;
            while (ReadFile(hStdOutRead, buffer.data(), buffer.size(), &bytesRead, NULL) && bytesRead > 0)
            {
                accumulatedOutput.append(buffer.data(), bytesRead);
                std::istringstream stream(accumulatedOutput);
                std::string line;
                std::regex pathRegex("(.*)\\((\\d+)\\): (.*)");
                std::smatch match;
                std::string remainingOutput;

                // Parse Lines
                while (std::getline(stream, line))
                {
                    // If Incomplete, Save it for Next Read
                    if (stream.eof() && !line.empty() && accumulatedOutput.back() != '\n')
                    {
                        remainingOutput = line;
                        break;
                    }

                    // Checks for time() Lines
                    if (line.find("time(") != std::string::npos) continue;

                    // Check for Source File
                    if (line.size() >= 6 && line.compare(line.size() - 5, 5, ".cpp\r") == 0)
                    {
                        // Skip Source Lines
                        continue;
                    }

                    // Replace Paths with At Line
                    if (std::regex_search(line, match, pathRegex))
                    {
                        std::string proxyFileName = std::filesystem::path(match[1].str()).filename().string();
                        if (buildCacheDatabase["Proxies"].contains(proxyFileName))
                        {
                            std::string actualSourceFile = buildCacheDatabase["Proxies"][proxyFileName].get<std::string>();
                            std::string newLine = "Script [" + actualSourceFile + "] Line " + match[2].str() + " :: " + match[3].str() + "\n";
                            resultOutput.append(newLine);
                            continue;
                        }
                    }

                    // Other Lines
                    resultOutput.append(line + "\n");
                }

                // Save the remaining output for the next read
                accumulatedOutput = remainingOutput;
            }
            if (!resultOutput.empty() && resultOutput.back() == '\n') resultOutput.pop_back();

            // Wait for the process to finish
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Close handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(hStdOutRead);

            // Set the compiler result
            result.compileResult = (exitCode == 0);
            result.hasError = (exitCode != 0);
            result.compileError = AS_GD_STRING(resultOutput);
            result.compileVerbose = AS_GD_STRING(resultOutput);

            // Yield Engine
            OS::get_singleton()->delay_msec(1);
            std::this_thread::yield();

            // Return Final Result
            return result;
        }
        BuildResult BuildFinalModule(const jenova::ModuleList& scriptModules)
        {
            return BuildFinalModuleWithCustomSettings(scriptModules, internalDefaultSettings);
        }
        BuildResult BuildFinalModuleWithCustomSettings(const jenova::ModuleList& scriptModules, const Dictionary& linkerSettings)
        {
            // Create Build Result
            BuildResult result;

            // Solve Settings [Cache]
            if (!SolveCompilerSettings(linkerSettings))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L666 : Unable to Solve Linker Settings.";
                return result;
            };

            // Set Output Directory Path on Build Result
            result.buildPath = this->jenovaCachePath;

            // Set Compiler Model
            result.compilerModel = this->GetCompilerModel();

            // Set Debug Information Flag
            result.hasDebugInformation = bool(linkerSettings["cpp_generate_debug_info"]);

            // Generate Output Module Path
            std::string outputModule = this->jenovaCachePath + (result.hasDebugInformation ? "Jenova.Module.dll" : AS_STD_STRING((String)linkerSettings["cpp_output_module"]));
            std::string outputMap = this->jenovaCachePath + AS_STD_STRING((String)linkerSettings["cpp_output_map"]);

            // Utilities
            auto GenerateLibraries = [](const godot::String& libsSetting) -> std::string 
            {
                std::string libs = AS_STD_STRING(libsSetting);
                if (libs.empty()) return "";
                if (libs.back() == ';') libs.pop_back();
                std::vector<std::string> libsArray;
                size_t start = 0;
                size_t end = libs.find(';');
                while (end != std::string::npos) 
                {
                    libsArray.push_back(libs.substr(start, end - start));
                    start = end + 1;
                    end = libs.find(';', start);
                }
                libsArray.push_back(libs.substr(start));
                std::string result;
                for (const auto& lib : libsArray) result += "\"" + lib + "\" ";
                return result;
            };
            auto GenerateAdditionalLibraryDirectories = [](const godot::String& additionalDirs) -> std::string
                {
                    std::string dirs = AS_STD_STRING(additionalDirs);
                    if (dirs.empty()) return "";
                    if (dirs.back() == ';') dirs.pop_back();
                    std::vector<std::string> dirArray;
                    size_t start = 0;
                    size_t end = dirs.find(';');
                    while (end != std::string::npos)
                    {
                        dirArray.push_back(dirs.substr(start, end - start));
                        start = end + 1;
                        end = dirs.find(';', start);
                    }
                    dirArray.push_back(dirs.substr(start));
                    std::string result;
                    for (const auto& dir : dirArray) result += "/LIBPATH:\"" + dir + "\" ";
                    return result;
                };

            // Generate Linker Arguments
            std::string linkerArgument;
            linkerArgument += "\"" + this->linkerBinaryPath + "\" ";
            linkerArgument += "/OUT:\"" + outputModule + "\" ";
            linkerArgument += "/MAP:\"" + outputMap + "\" ";
            if (String(linkerSettings["cpp_default_subsystem"]) == "Console") linkerArgument += "/SUBSYSTEM:CONSOLE ";
            if (String(linkerSettings["cpp_default_subsystem"]) == "GUI") linkerArgument += "/SUBSYSTEM:WINDOWS ";
            if (String(linkerSettings["cpp_machine_architecture"]) == "Win64") linkerArgument += "/MACHINE:X64 ";
            if (String(linkerSettings["cpp_machine_architecture"]) == "Win32") linkerArgument += "/MACHINE:X86 ";
            if (String(linkerSettings["cpp_machine_pe_type"]) == "dll") linkerArgument += "/DLL ";
            if (String(linkerSettings["cpp_machine_pe_type"]) == "exe") linkerArgument += "/EXE ";
            if (bool(linkerSettings["cpp_add_manifest"])) linkerArgument += "/MANIFEST ";
            if (bool(linkerSettings["cpp_dynamic_base"])) linkerArgument += "/DYNAMICBASE ";
            if (bool(linkerSettings["cpp_debug_symbol"]) && result.hasDebugInformation) linkerArgument += "/DEBUG:FULL ";
            linkerArgument += GenerateLibraries(linkerSettings["cpp_default_libs"]);
            linkerArgument += GenerateLibraries(linkerSettings["cpp_native_libs"]);
            linkerArgument += GenerateLibraries(linkerSettings["cpp_extra_libs"]);
            linkerArgument += "/LIBPATH:\"./\" ";
            linkerArgument += "/LIBPATH:\"" + this->libraryPath + "\" ";
            linkerArgument += "/LIBPATH:\"" + this->jenovaSDKPath + "\" ";
            linkerArgument += "/LIBPATH:\"" + this->godotSDKPath + "\" ";

            // Add Additional Library Directories
            linkerArgument += GenerateAdditionalLibraryDirectories(linkerSettings["cpp_extra_library_directories"]);

            // Add Packages Libraries (Addons, Libraries etc.)
            for (const auto& addonConfig : jenova::GetInstalledAddons())
            {
                // Check For Addon Type
                if (addonConfig.Type == "RuntimeModule")
                {
                    if (!addonConfig.Header.empty())
                    {
                        std::string libraryPath = addonConfig.Path + "/" + addonConfig.Library;
                        linkerArgument += "\"" + libraryPath + "\" ";
                        linkerArgument += "/DELAYLOAD:\"" + addonConfig.Binary + "\" ";
                    }
                }
            }

            // Disable Logo
            linkerArgument += "/nologo ";

            // Ignore Warnings
            linkerArgument += "/IGNORE:4099 ";

            // Add Object Files
            for (const auto& scriptModule : scriptModules) linkerArgument += "\"" + AS_STD_STRING(scriptModule.scriptObjectFile) + "\" ";

            // Add Extra Options
            linkerArgument += AS_STD_STRING(String(linkerSettings["cpp_extra_linker"])) + " ";

            // Add Delayed DLLs
            linkerArgument += AS_STD_STRING(String(linkerSettings["cpp_delayed_dll"])) + " ";

            // Dump Linker Command If Developer Mode Enabled
            if (jenova::GlobalStorage::DeveloperModeActivated) jenova::WriteStdStringToFile(jenovaCachePath + "LinkerCommand.txt", linkerArgument);

            // Run Linker
            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.dwFlags |= STARTF_USESTDHANDLES;

            // Create pipes for capturing output
            HANDLE hStdOutRead, hStdOutWrite;
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
            SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
            si.hStdOutput = hStdOutWrite;
            si.hStdError = hStdOutWrite;

            // Convert linker command to wide string
            std::wstring wLinkerArgument(linkerArgument.begin(), linkerArgument.end());

            // Execute the linker command
            if (!CreateProcessW(NULL, &wLinkerArgument[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L667 : Failed to Run Linker!";
                return result;
            }

            // Close the write end of the pipe
            CloseHandle(hStdOutWrite);

            // Read the linker output
            std::vector<char> buffer(jenova::GlobalSettings::BuildOutputBufferSize);
            DWORD bytesRead;
            std::string resultOutput;
            while (ReadFile(hStdOutRead, buffer.data(), buffer.size(), &bytesRead, NULL) && bytesRead > 0) resultOutput.append(buffer.data(), bytesRead);

            // Wait for the process to finish
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code of the linker process
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Close handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(hStdOutRead);

            // Set the build result
            result.buildResult = (exitCode == 0);
            result.hasError = (exitCode != 0);
            result.buildError = AS_GD_STRING(resultOutput);
            result.buildVerbose = AS_GD_STRING(resultOutput);

            // Return if There's error
            if (!result.buildResult) return result;

            // Read module to buffer
            std::ifstream moduleReader(outputModule, std::ios::binary);
            result.builtModuleData = std::vector<uint8_t>(std::istreambuf_iterator<char>(moduleReader), {});

            // Validate Module Buffer
            if (result.builtModuleData.size() == 0)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L668 : Invalid Module Data.";
                return result;
            }

            // Generate Metadata
            result.moduleMetaData = JenovaInterpreter::GenerateModuleMetadata(outputMap, scriptModules, result);
            if (result.moduleMetaData.empty())
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L669 : Failed to Generate Module Meta Data.";
                return result;
            }

            // Generate Build Cache
            if (!jenova::CreateBuildCacheDatabase(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, scriptModules, linkerSettings["CppHeaderFiles"]))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L670 : Failed to Generate Build Cache Database.";
                return result;
            }

            // Yield Engine
            OS::get_singleton()->delay_msec(1);
            std::this_thread::yield();

            // Return Final Result
            return result;
        }
        bool SetCompilerOption(const String& optName, const Variant& optValue)
        {
            internalDefaultSettings[optName] = optValue;
            return true;
        }
        Variant GetCompilerOption(const String& optName) const
        {
            if (internalDefaultSettings.has(optName)) return internalDefaultSettings[optName];
            return Variant();
        }
        Variant ExecuteCommand(const String& commandName, const Dictionary& commandSettings)
        {
            // Process Commands
            if (commandName == "Solve-Compiler-Settings")
            {
                return SolveCompilerSettings(internalDefaultSettings);
            }

            // Invalid Command
            return Variant::NIL;
        }
        CompilerFeatures GetCompilerFeatures() const
        {
            return CanCompileFromFile | CanGenerateMappingData | CanGenerateModule | CanLinkObjectFiles;
        }
        CompilerModel GetCompilerModel() const
        {
            return compilerModel;
        }
        bool SolveCompilerSettings(const Dictionary& compilerSettings)
        {
            // Get Project Path
            String projectPath = jenova::GetJenovaProjectDirectory();
        
            // Collect Compiler & GodotKit Packages
            String selectedCompilerPath = jenova::GetInstalledCompilerPathFromPackages(compilerSettings["cpp_toolchain_path"], GetCompilerModel());
            String selectedGodotKitPath = jenova::GetInstalledGodotKitPathFromPackages(compilerSettings["cpp_godotsdk_path"]);

            // Validate Compiler & GodotKit Packages
            if (selectedCompilerPath == "Missing-Compiler-1.0.0")
            {
                jenova::Error("Jenova Microsoft Compiler", "No Microsoft Compiler Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }
            if (selectedGodotKitPath == "Missing-GodotKit-1.0.0")
            {
                jenova::Error("Jenova Microsoft Compiler", "No GodotSDK Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }

            // Globalize Paths
            selectedCompilerPath = ProjectSettings::get_singleton()->globalize_path(selectedCompilerPath);
            selectedGodotKitPath = ProjectSettings::get_singleton()->globalize_path(selectedGodotKitPath);

            // Solve Compiler Paths
            this->projectPath = std::filesystem::absolute(AS_STD_STRING(projectPath)).string();
            this->compilerBinaryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_compiler_binary"]))).string();
            this->linkerBinaryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_linker_binary"]))).string();
            this->includePath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_include_path"]))).string();
            this->libraryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_library_path"]))).string();
            this->jenovaSDKPath = std::filesystem::absolute(AS_STD_STRING(projectPath + String(compilerSettings["cpp_jenovasdk_path"]))).string();
            this->godotSDKPath = std::filesystem::absolute(AS_STD_STRING(selectedGodotKitPath)).string();
            this->jenovaCachePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory());

            // Store Solved Paths
            this->internalDefaultSettings["compiler_solved_binary_path"] = String(this->compilerBinaryPath.c_str());
            this->internalDefaultSettings["linker_solved_binary_path"] = String(this->linkerBinaryPath.c_str());

            // All Good
            return true;
        }

    private:
        CompilerModel compilerModel = CompilerModel::Unspecified;
        Dictionary internalDefaultSettings;
        std::string projectPath;
        std::string compilerBinaryPath;
        std::string linkerBinaryPath;
        std::string includePath;
        std::string libraryPath;
        std::string jenovaSDKPath;
        std::string godotSDKPath;
        std::string jenovaCachePath;
    };

    // Jenova MinGW Compiler Implementation
    class MinGWCompiler : public IJenovaCompiler
    {
    public:
        MinGWCompiler(bool useLLVM)
        {
            // Set Sub-Compiler Model
            compilerModel = useLLVM ? CompilerModel::MinGWClangCompiler : CompilerModel::MinGWCompiler;
        }
        ~MinGWCompiler()
        {
        }
        bool InitializeCompiler(String compilerInstanceName = "<JenovaMinGWCompiler>")
        {
            // MinGW GCC Compiler Default Settings
            if (this->GetCompilerModel() == CompilerModel::MinGWCompiler)
            {
                // Initialize GCC Tool Chain Settings
                internalDefaultSettings["instance_name"]                        = compilerInstanceName;
                internalDefaultSettings["instance_version"]                     = 1.1f;
                internalDefaultSettings["cpp_toolchain_path"]                   = "/Jenova/Compilers/JenovaMinGWCompiler"; // Placeholder
                internalDefaultSettings["cpp_compiler_binary"]                  = "/bin/g++.exe";
                internalDefaultSettings["cpp_linker_binary"]                    = "/bin/g++.exe";
                internalDefaultSettings["cpp_gdb_binary"]                       = "/bin/gdb.exe";
                internalDefaultSettings["cpp_include_path"]                     = "/include";
                internalDefaultSettings["cpp_library_path"]                     = "/lib";
                internalDefaultSettings["cpp_jenovasdk_path"]                   = "/Jenova/JenovaSDK";
                internalDefaultSettings["cpp_godotsdk_path"]                    = "/Jenova/GodotSDK"; // Placeholder

                // MinGW GCC Compiler Settings
                internalDefaultSettings["cpp_language_standards"]               = "cpp20";                          /* -std=c++20 [cpp20, cpp17] */
                internalDefaultSettings["cpp_open_mp_support"]                  = true;                             /* -fopenmp */
                internalDefaultSettings["cpp_multithreaded"]                    = true;                             /* -static */
                internalDefaultSettings["cpp_debug_database"]                   = true;                             /* -g */
                internalDefaultSettings["cpp_exception_handling"]               = true;                             /* -fexceptions */
                internalDefaultSettings["cpp_extra_compiler"]                   = "-O1 -pthread";                   /* Extra Compiler Options*/
                internalDefaultSettings["cpp_definitions"]                      = "TYPED_METHOD_BIND;HOT_RELOAD_ENABLED;_WINDLL"; /* REAL_T_IS_DOUBLE Removed for Now */

                // MinGW GCC Linker Settings
                internalDefaultSettings["cpp_output_module"]                    = "Jenova.Module.jnv";              /* Must use .dll for Debug, .jnv for Final*/
                internalDefaultSettings["cpp_debug_symbol"]                     = "Jenova.Module.pdb";              /* -Wl,--out-implib,Jenova.Module.pdb */
                internalDefaultSettings["cpp_output_map"]                       = "Jenova.Module.map";              /* -Wl,-Map,Jenova.Module.map */
                internalDefaultSettings["cpp_default_libs"]                     = "wldap32;bcrypt;dbghelp;delayimp";
                internalDefaultSettings["cpp_native_libs"]                      = "libGodot.x64.a";
                internalDefaultSettings["cpp_delayed_dll"]                      = "";
                internalDefaultSettings["cpp_default_subsystem"]                = "Console";                        /* -Wl,-subsystem,console / -Wl,-subsystem,windows */
                internalDefaultSettings["cpp_machine_architecture"]             = "Win64";                          /* -m64 [Win64, Win32]*/
                internalDefaultSettings["cpp_machine_pe_type"]                  = "dll";                            /* -shared [None for exe]*/
                internalDefaultSettings["cpp_statics_libs"]                     = "-static-libstdc++ -static-libgcc -Wl,-Bstatic -lpthread";
                internalDefaultSettings["cpp_extra_linker"]                     = "-fuse-ld=lld -flto";             /* Extra Linker Options */
            }

            // MinGW Clang Compiler Default Settings
            if (this->GetCompilerModel() == CompilerModel::MinGWClangCompiler)
            {
                // Initialize Clang Tool Chain Settings
                internalDefaultSettings["instance_name"]                        = compilerInstanceName;
                internalDefaultSettings["instance_version"]                     = 1.1f;
                internalDefaultSettings["cpp_toolchain_path"]                   = "/Jenova/Compilers/JenovaMinGWCompiler"; // Placeholder
                internalDefaultSettings["cpp_compiler_binary"]                  = "/bin/clang++.exe";
                internalDefaultSettings["cpp_linker_binary"]                    = "/bin/clang++.exe";
                internalDefaultSettings["cpp_gdb_binary"]                       = "/bin/gdb.exe";
                internalDefaultSettings["cpp_include_path"]                     = "/include";
                internalDefaultSettings["cpp_library_path"]                     = "/lib";
                internalDefaultSettings["cpp_jenovasdk_path"]                   = "/Jenova/JenovaSDK";
                internalDefaultSettings["cpp_godotsdk_path"]                    = "/Jenova/GodotSDK"; // Placeholder

                // MinGW Clang Compiler Settings
                internalDefaultSettings["cpp_language_standards"]               = "cpp20";                          /* -std=c++20 [cpp20, cpp17] */
                internalDefaultSettings["cpp_open_mp_support"]                  = true;                             /* -fopenmp */
                internalDefaultSettings["cpp_multithreaded"]                    = true;                             /* -static */
                internalDefaultSettings["cpp_debug_database"]                   = true;                             /* -g */
                internalDefaultSettings["cpp_exception_handling"]               = true;                             /* -fexceptions */
                internalDefaultSettings["cpp_extra_compiler"]                   = "-O3 -pthread -femulated-tls";    /* Extra Compiler Options*/
                internalDefaultSettings["cpp_definitions"]                      = "TYPED_METHOD_BIND;HOT_RELOAD_ENABLED;_WINDLL"; /* REAL_T_IS_DOUBLE Removed for Now */

                // MinGW Clang Linker Settings
                internalDefaultSettings["cpp_output_module"]                    = "Jenova.Module.jnv";              /* Must use .dll for Debug, .jnv for Final*/
                internalDefaultSettings["cpp_debug_symbol"]                     = "Jenova.Module.pdb";              /* -Wl,--out-implib,Jenova.Module.pdb */
                internalDefaultSettings["cpp_output_map"]                       = "Jenova.Module.map";              /* -Wl,-Map,Jenova.Module.map */
                internalDefaultSettings["cpp_default_libs"]                     = "wldap32;bcrypt;dbghelp;delayimp";
                internalDefaultSettings["cpp_native_libs"]                      = "libGodot.x64.a";
                internalDefaultSettings["cpp_delayed_dll"]                     = "-Xlinker /DELAYLOAD:Jenova.Runtime.Win64.dll";
                internalDefaultSettings["cpp_default_subsystem"]                = "Console";                        /* -Wl,-subsystem,console / -Wl,-subsystem,windows */
                internalDefaultSettings["cpp_machine_architecture"]             = "Win64";                          /* -m64 [Win64, Win32]*/
                internalDefaultSettings["cpp_machine_pe_type"]                  = "dll";                            /* -shared [None for exe]*/
                internalDefaultSettings["cpp_statics_libs"]                     = "-static-libstdc++ -static-libgcc -Wl,-Bstatic -lpthread";
                internalDefaultSettings["cpp_extra_linker"]                     = "-fuse-ld=lld";                   /* Extra Linker Options */
            }

            // All Good
            return true;
        }
        bool ReleaseCompiler()
        {
            // Release Resources
            internalDefaultSettings.clear();
            return true;
        }
        String PreprocessScript(Ref<CPPScript> cppScript, const Dictionary& preprocessorSettings)
        {
            // Get Original Source Code
            String scriptSourceCode = cppScript->get_source_code();

            // Reset Line Number
            scriptSourceCode = scriptSourceCode.insert(0, "#line 1\n");

            // Process And Extract Properties
            jenova::SerializedData propertiesMetadata = jenova::ProcessAndExtractPropertiesFromScript(scriptSourceCode, cppScript->GetScriptIdentity());
            if (!propertiesMetadata.empty() && propertiesMetadata != "null") jenova::WriteStdStringToFile(AS_STD_STRING(String(preprocessorSettings["PropertyMetadata"])), propertiesMetadata);

            // Preprocessor Definitions [Header]
            String preprocessorDefinitions = "// Jenova Preprocessor Definitions\n";

            // Preprocessor Definitions [Version]
            preprocessorDefinitions += String(jenova::Format("#define JENOVA_VERSION \"%d.%d.%d.%d\"\n",
                jenova::GlobalSettings::JenovaBuildVersion[0], jenova::GlobalSettings::JenovaBuildVersion[1],
                jenova::GlobalSettings::JenovaBuildVersion[2], jenova::GlobalSettings::JenovaBuildVersion[3]).c_str());

            // Preprocessor Definitions [Compiler]
            if (this->GetCompilerModel() == CompilerModel::MinGWCompiler)
            {
                preprocessorDefinitions += "#define JENOVA_COMPILER \"MinGW GCC Compiler\"\n";
                preprocessorDefinitions += "#define MINGW_CLANG_COMPILER\n";
            }
            if (this->GetCompilerModel() == CompilerModel::MinGWClangCompiler)
            {
                preprocessorDefinitions += "#define JENOVA_COMPILER \"MinGW Clang Compiler\"\n";
                preprocessorDefinitions += "#define MINGW_GCC_COMPILER\n";
            }

            // Preprocessor Definitions [User]
            String userPreprocessorDefinitions = preprocessorSettings["PreprocessorDefinitions"];
            PackedStringArray userPreprocessorDefinitionsList = userPreprocessorDefinitions.split(";");
            for (const auto& definition : userPreprocessorDefinitionsList) if (!definition.is_empty()) preprocessorDefinitions += "#define " + definition + "\n";

            // Add Final Preprocessor Definitions
            scriptSourceCode = scriptSourceCode.insert(0, preprocessorDefinitions + "\n");

            // Replecements
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptToolIdentifier, "#define TOOL_SCRIPT");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockBeginIdentifier, "namespace JNV_" + cppScript->GetScriptIdentity() + " {");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockEndIdentifier, "}; using namespace JNV_" + cppScript->GetScriptIdentity() + ";");
            scriptSourceCode = scriptSourceCode.replace(" OnReady", " _ready");
            scriptSourceCode = scriptSourceCode.replace(" OnAwake", " _enter_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnDestroy", " _exit_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnProcess", " _process");
            scriptSourceCode = scriptSourceCode.replace(" OnPhysicsProcess", " _physics_process");
            scriptSourceCode = scriptSourceCode.replace(" OnInput", " _input");
            scriptSourceCode = scriptSourceCode.replace(" OnUserInterfaceInput", " _gui_input");

            // Return Preprocessed Source
            return scriptSourceCode;
        }
        CompileResult CompileScript(const String sourceCode)
        {
            return CompileScriptWithCustomSettings(sourceCode, this->internalDefaultSettings);
        }
        CompileResult CompileScriptWithCustomSettings(const String sourceCode, const Dictionary& compilerSettings)
        {
            CompileResult result;
            result.hasError = true;
            result.compileError = "This Compiler does not support Compilation from Memory!";
            return result;
        }
        CompileResult CompileScriptFromFile(const String scriptFilePath)
        {
            return CompileScriptFromFileWithCustomSettings(scriptFilePath, this->internalDefaultSettings);
        }
        CompileResult CompileScriptFromFileWithCustomSettings(const String scriptFilePath, const Dictionary& compilerSettings)
        {
            CompileResult result;
            result.hasError = true;
            result.compileError = "The requested compilation method is currently Not Implemented.";
            return result;
        }
        CompileResult CompileScriptModuleContainer(const ScriptModuleContainer& scriptModulesContainer)
        {
            return CompileScriptModuleWithCustomSettingsContainer(scriptModulesContainer, this->internalDefaultSettings);
        }
        CompileResult CompileScriptModuleWithCustomSettingsContainer(const ScriptModuleContainer& scriptModulesContainer, const Dictionary& compilerSettings)
        {
            // Create Compiler Result
            CompileResult result;

            // Solve Settings [Cache]
            if (!SolveCompilerSettings(compilerSettings))
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C666 : Unable to Solve Compiler Settings.";
                return result;
            };

            // Utilities
            auto GeneratePreprocessDefinitions = [](const godot::String& defsSetting) -> std::string
            {
                std::string defs = AS_STD_STRING(defsSetting);
                if (defs.empty()) return "";
                if (defs.back() == ';') defs.pop_back();
                std::vector<std::string> defsArray;
                size_t start = 0;
                size_t end = defs.find(';');
                while (end != std::string::npos) {
                    defsArray.push_back(defs.substr(start, end - start));
                    start = end + 1;
                    end = defs.find(';', start);
                }
                defsArray.push_back(defs.substr(start));
                std::string result;
                for (const auto& def : defsArray) result += "-D" + def + " ";
                return result;
            };
            auto GenerateAdditionalIncludeDirectories = [](const godot::String& additionalDirs) -> std::string
            {
                std::string dirs = AS_STD_STRING(additionalDirs);
                if (dirs.empty()) return "";
                if (dirs.back() == ';') dirs.pop_back();
                std::vector<std::string> dirArray;
                size_t start = 0;
                size_t end = dirs.find(';');
                while (end != std::string::npos)
                {
                    dirArray.push_back(dirs.substr(start, end - start));
                    start = end + 1;
                    end = dirs.find(';', start);
                }
                dirArray.push_back(dirs.substr(start));
                std::string result;
                for (const auto& dir : dirArray) result += "-I" + dir + " ";
                return result;
            };

            // Generate Compiler Arguments
            std::string compilerArgument;
            compilerArgument += "\"" + this->compilerBinaryPath + "\" -c ";
            if (String(compilerSettings["cpp_language_standards"]) == "cpp20") compilerArgument += "-std=c++20 ";
            if (String(compilerSettings["cpp_language_standards"]) == "cpp17") compilerArgument += "-std=c++17 ";
            if (bool(compilerSettings["cpp_open_mp_support"])) compilerArgument += "-fopenmp ";
            if (bool(compilerSettings["cpp_multithreaded"])) compilerArgument += "-static ";
            if (bool(compilerSettings["cpp_debug_database"]) && bool(compilerSettings["cpp_generate_debug_info"])) compilerArgument += "-ggdb ";
            if (bool(compilerSettings["cpp_exception_handling"]) == true) compilerArgument += "-fexceptions ";
            compilerArgument += GeneratePreprocessDefinitions(compilerSettings["cpp_definitions"]);
            compilerArgument += "-I\"./\" ";
            compilerArgument += "-I\"" + this->projectPath + "\" ";
            compilerArgument += "-I\"" + this->includePath + "\" ";
            compilerArgument += "-I\"" + this->jenovaSDKPath + "\" ";
            compilerArgument += "-I\"" + this->godotSDKPath + "\" ";

            // Add Additional Include Directories
            compilerArgument += GeneratePreprocessDefinitions(compilerSettings["cpp_extra_include_directories"]);

            // Force Include JenovaSDK Header
            if (jenova::GlobalSettings::ForceJenovaSDKHeader && jenova::GlobalStorage::UseBuiltinSDK) compilerArgument += "-include JenovaSDK.h ";

            // Add Packages Headers (Addons, Libraries etc.)
            for (const auto& addonConfig : jenova::GetInstalledAddons())
            {
                // Check For Addon Type
                if (addonConfig.Type == "RuntimeModule")
                {
                    if (!addonConfig.Header.empty())
                    {
                        if (addonConfig.Global)
                        {
                            std::string headerPath = addonConfig.Path + "/" + addonConfig.Header;
                            compilerArgument += "-include \"" + headerPath + "\" ";
                        }
                        compilerArgument += "-I\"" + addonConfig.Path + "\" ";
                    }
                }
            }

            // Add Static Libraries
            compilerArgument += AS_STD_STRING(String(compilerSettings["cpp_statics_libs"])) + " ";

            // Add Extra Options
            compilerArgument += AS_STD_STRING(String(compilerSettings["cpp_extra_compiler"])) + " ";

            // Load Cache if Exists
            bool buildCacheFileFound = false;
            jenova::json_t buildCacheDatabase;
            if (!std::filesystem::exists(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile))
            {
                // Cache Doesn't Exists Generate It [Required for Proxies]
                if (!jenova::CreateBuildCacheDatabase(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, scriptModulesContainer.scriptModules, compilerSettings["CppHeaderFiles"], true))
                {
                    result.compileResult = false;
                    result.hasError = true;
                    result.compileError = "C670 : Failed to Generate Build Cache Database.";
                    return result;
                }
            }

            // Parse Cache File
            try
            {
                std::ifstream buildCacheDatabaseReader(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, std::ios::binary);
                std::string buildCacheDatabaseContent(std::istreambuf_iterator<char>(buildCacheDatabaseReader), {});
                if (!buildCacheDatabaseContent.empty())
                {
                    buildCacheDatabase = jenova::json_t::parse(buildCacheDatabaseContent);
                    buildCacheFileFound = true;
                }
            }
            catch (const std::exception&) 
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C671 : Failed to Parse Build Cache Database.";
                return result;
            }

            // Check If Any Changes Applied to Headers
            if (buildCacheDatabase.contains("Headers"))
            {
                // Detect Changes
                bool detectedHeaderChanges = false;
                PackedStringArray cppHeaderFiles = compilerSettings["CppHeaderFiles"];
                for (const auto& cppHeaderFile : cppHeaderFiles)
                {
                    String cppHeaderUID = jenova::GenerateStandardUIDFromPath(cppHeaderFile);
                    String cppHeaderHash = jenova::GenerateMD5HashFromFile(cppHeaderFile);
                    if (buildCacheDatabase["Headers"].contains(AS_STD_STRING(cppHeaderUID)))
                    {
                        if (AS_STD_STRING(cppHeaderHash) != buildCacheDatabase["Headers"][AS_STD_STRING(cppHeaderUID)].get<std::string>())
                        {
                            detectedHeaderChanges = true;
                            break;
                        }
                    }
                }
                if (buildCacheDatabase.contains("HeaderCount"))
                {
                    int headerCount = buildCacheDatabase["HeaderCount"].get<int>();
                    if (headerCount != cppHeaderFiles.size()) detectedHeaderChanges = true;
                }

                // If Contains Changes Reset All Script Cache
                if (detectedHeaderChanges)
                {
                    if (buildCacheDatabase.contains("Modules")) for (const auto& scriptModule : buildCacheDatabase["Modules"].items()) scriptModule.value() = "No Hash";
                }
            }

            // Add Source/Output Based On Compile Model
            jenova::ModuleList compilationScripts;
            if (bool(compilerSettings["cpp_multi_threaded_compilation"]))
            {
                for (const auto& scriptModule : scriptModulesContainer.scriptModules)
                {
                    // Skip If File Hash Didn't Change
                    if (buildCacheDatabase.contains("Modules"))
                    {
                        if (buildCacheDatabase["Modules"].contains(AS_STD_STRING(scriptModule.scriptUID)))
                        {
                            if (AS_STD_STRING(scriptModule.scriptHash) == buildCacheDatabase["Modules"][AS_STD_STRING(scriptModule.scriptUID)].get<std::string>()) continue;
                        }
                    }
               
                    // Add Source
                    compilationScripts.push_back(scriptModule);

                    // Add Script Count
                    result.scriptsCount++;
                }
            }
            else
            {
                // Skip If File Hash Didn't Change
                if (buildCacheDatabase.contains("Modules"))
                {
                    if (buildCacheDatabase["Modules"].contains(AS_STD_STRING(scriptModulesContainer.scriptModule.scriptUID)))
                    {
                        if (AS_STD_STRING(scriptModulesContainer.scriptModule.scriptHash) != buildCacheDatabase["Modules"][AS_STD_STRING(scriptModulesContainer.scriptModule.scriptUID)].get<std::string>())
                        {
                            compilationScripts.push_back(scriptModulesContainer.scriptModule);
                            result.scriptsCount++;
                        }
                    }
                }
            }

            // Skip Compile If Source Count is 0
            if (result.scriptsCount == 0)
            {
                result.compileResult = true;
                result.hasError = false;
                result.compileVerbose = "Cache System Detected No Script Requires to be Compiled.";
                return result;
            }

            // Dump Compiler Command If Developer Mode Enabled
            if (jenova::GlobalStorage::DeveloperModeActivated) jenova::WriteStdStringToFile(jenovaCachePath + "CompilerCommand.txt", compilerArgument);

            // Compile Scripts
            std::vector<jenova::TaskID> compilationTasks;
            bool compilationFailed = false;
            std::mutex compilationMutex;
            std::vector<std::string> errorMessages;

            // Spawn Tasks Per Script
            for (const auto& scriptModule : compilationScripts)
            {
                std::string command = compilerArgument;
                command += "\"" + AS_STD_STRING(scriptModule.scriptCacheFile) + "\" ";
                command += "-o \"" + AS_STD_STRING(scriptModule.scriptObjectFile) + "\" ";

                // Fix Paths in Command
                jenova::ReplaceAllMatchesWithString(command, "\\", "/");
                jenova::ReplaceAllMatchesWithString(command, "\\\\", "/");

                // Initiate Compilation Task
                jenova::TaskID taskID = JenovaTaskSystem::InitiateTask([command, &compilationFailed, &compilationMutex, &errorMessages]()
                {
                    STARTUPINFOW si;
                    PROCESS_INFORMATION pi;
                    ZeroMemory(&si, sizeof(si));
                    si.cb = sizeof(si);
                    si.dwFlags |= STARTF_USESTDHANDLES;

                    HANDLE hStdOutRead, hStdOutWrite;
                    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
                    CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
                    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
                    si.hStdOutput = hStdOutWrite;
                    si.hStdError = hStdOutWrite;

                    std::wstring wCommand(command.begin(), command.end());
                    if (!CreateProcessW(NULL, &wCommand[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
                    {
                        std::lock_guard<std::mutex> lock(compilationMutex);
                        compilationFailed = true;
                        errorMessages.push_back("Failed to start compilation process.");
                        return;
                    }

                    CloseHandle(hStdOutWrite);

                    std::vector<char> buffer(4096);
                    DWORD bytesRead;
                    std::string compilerOutput;
                    while (ReadFile(hStdOutRead, buffer.data(), buffer.size(), &bytesRead, NULL) && bytesRead > 0)
                    {
                        compilerOutput.append(buffer.data(), bytesRead);
                    }

                    CloseHandle(hStdOutRead);
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    DWORD exitCode;
                    GetExitCodeProcess(pi.hProcess, &exitCode);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);

                    if (exitCode != 0)
                    {
                        std::lock_guard<std::mutex> lock(compilationMutex);
                        compilationFailed = true;
                        errorMessages.push_back(compilerOutput);
                    }
                });
                compilationTasks.push_back(taskID);
            }

            // Wait for all tasks to complete
            for (const auto& taskID : compilationTasks)
            {
                while (!JenovaTaskSystem::IsTaskComplete(taskID))
                {
                    std::this_thread::yield();
                }
                JenovaTaskSystem::ClearTask(taskID);
            }

            // Check Compile Result
            if (compilationFailed)
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C668 : One or more compilation tasks failed.\n";
                for (const auto& errorMsg : errorMessages)
                {
                    result.compileError += AS_GD_STRING(errorMsg) + "\n";
                }
                return result;
            }

            // Set Compiler Result
            result.compileResult = !compilationFailed;
            result.hasError = !compilationFailed;

            // Yield Engine
            OS::get_singleton()->delay_msec(1);
            std::this_thread::yield();

            // Return Final Result
            return result;
        }
        BuildResult BuildFinalModule(const jenova::ModuleList& scriptModules)
        {
            return BuildFinalModuleWithCustomSettings(scriptModules, internalDefaultSettings);
        }
        BuildResult BuildFinalModuleWithCustomSettings(const jenova::ModuleList& scriptModules, const Dictionary& linkerSettings)
        {
            // Create Build Result
            BuildResult result;

            // Solve Settings [Cache]
            if (!SolveCompilerSettings(linkerSettings))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L666 : Unable to Solve Linker Settings.";
                return result;
            };

            // Set Output Directory Path on Build Result
            result.buildPath = this->jenovaCachePath;

            // Set Compiler Model
            result.compilerModel = this->GetCompilerModel();

            // Set Debug Information Flag
            result.hasDebugInformation = bool(linkerSettings["cpp_generate_debug_info"]);

            // Generate Output Module Path
            std::string outputModule = this->jenovaCachePath + (result.hasDebugInformation ? "Jenova.Module.dll" : AS_STD_STRING((String)linkerSettings["cpp_output_module"]));
            std::string outputDebugSymbol = this->jenovaCachePath + AS_STD_STRING((String)linkerSettings["cpp_debug_symbol"]);
            std::string outputMap = this->jenovaCachePath + AS_STD_STRING((String)linkerSettings["cpp_output_map"]);

            // Utilities
            auto GenerateLibraries = [](const godot::String& libsSetting, bool noPrefix = false) -> std::string
            {
                std::string libs = AS_STD_STRING(libsSetting);
                if (libs.empty()) return "";
                if (libs.back() == ';') libs.pop_back();
                std::vector<std::string> libsArray;
                size_t start = 0;
                size_t end = libs.find(';');
                while (end != std::string::npos)
                {
                    libsArray.push_back(libs.substr(start, end - start));
                    start = end + 1;
                    end = libs.find(';', start);
                }
                libsArray.push_back(libs.substr(start));
                std::string result;
                if (noPrefix) for (const auto& lib : libsArray) result += "-l:" + lib + " ";
                else for (const auto& lib : libsArray) result += "-l" + lib + " ";
                return result;
            };
            auto GenerateLibraryPaths = [](const godot::String& additionalDirs) -> std::string
            {
                std::string dirs = AS_STD_STRING(additionalDirs);
                if (dirs.empty()) return "";
                if (dirs.back() == ';') dirs.pop_back();
                std::vector<std::string> dirArray;
                size_t start = 0;
                size_t end = dirs.find(';');
                while (end != std::string::npos)
                {
                    dirArray.push_back(dirs.substr(start, end - start));
                    start = end + 1;
                    end = dirs.find(';', start);
                }
                dirArray.push_back(dirs.substr(start));
                std::string result;
                for (const auto& dir : dirArray) result += "-L\"" + dir + "\" ";
                return result;
            };

            // Generate Linker Arguments
            std::string linkerArgument;
            linkerArgument += "\"" + this->linkerBinaryPath + "\" ";
            linkerArgument += "-B\"" + this->binariesPath + "\" ";
            linkerArgument += "-o \"" + outputModule + "\" ";
            linkerArgument += "-Wl,-Map=\"" + outputMap + "\" ";
            linkerArgument += "-Wl,--pdb=\"" + outputDebugSymbol + "\" ";
            if (String(linkerSettings["cpp_default_subsystem"]) == "Console") linkerArgument += "-Wl,-subsystem,console ";
            if (String(linkerSettings["cpp_default_subsystem"]) == "GUI") linkerArgument += "-Wl,-subsystem,windows ";
            if (String(linkerSettings["cpp_machine_architecture"]) == "Win64") linkerArgument += "-m64 ";
            if (String(linkerSettings["cpp_machine_architecture"]) == "Win32") linkerArgument += "-m32 ";
            if (String(linkerSettings["cpp_machine_pe_type"]) == "dll") linkerArgument += "-shared ";
            if (String(linkerSettings["cpp_machine_pe_type"]) == "exe") linkerArgument += "-no-pie ";
            linkerArgument += "-L\"./\" ";
            linkerArgument += "-L\"" + this->projectPath + "\" ";
            linkerArgument += "-L\"" + this->libraryPath + "\" ";
            linkerArgument += "-L\"" + this->jenovaSDKPath + "\" ";
            linkerArgument += "-L\"" + this->godotSDKPath + "\" ";

            // Add Additional Library Directories
            linkerArgument += GenerateLibraryPaths(linkerSettings["cpp_extra_library_directories"]);

            // Add Packages Libraries (Addons, Libraries etc.)
            for (const auto& addonConfig : jenova::GetInstalledAddons())
            {
                if (addonConfig.Type == "RuntimeModule")
                { 
                    if (!addonConfig.Header.empty())
                    {
                        std::string libraryPath = addonConfig.Path + "/" + addonConfig.Library;
                        linkerArgument += "\"" + libraryPath + "\" ";
                        linkerArgument += "-Xlinker /DELAYLOAD:\"" + addonConfig.Binary + "\" ";
                    }
                }
            }

            // Add Object Files
            for (const auto& scriptModule : scriptModules) linkerArgument += "\"" + AS_STD_STRING(scriptModule.scriptObjectFile) + "\" ";

            // Add Static Libraries
            linkerArgument += AS_STD_STRING(String(linkerSettings["cpp_statics_libs"])) + " ";

            // Add Extra Options
            linkerArgument += AS_STD_STRING(String(linkerSettings["cpp_extra_linker"])) + " ";

            // Add Delayed DLLs
            linkerArgument += AS_STD_STRING(String(linkerSettings["cpp_delayed_dll"])) + " ";

            // Add Libraries [GCC Requires Libraries to be Added at the End]
            linkerArgument += GenerateLibraries(linkerSettings["cpp_native_libs"], true);
            linkerArgument += GenerateLibraries(linkerSettings["cpp_default_libs"]);
            linkerArgument += GenerateLibraries(linkerSettings["cpp_extra_libs"]);

            // Fix Paths in Command
            jenova::ReplaceAllMatchesWithString(linkerArgument, "\\", "/");
            jenova::ReplaceAllMatchesWithString(linkerArgument, "\\\\", "/");

            // Dump Linker Command If Developer Mode Enabled
            if (jenova::GlobalStorage::DeveloperModeActivated) jenova::WriteStdStringToFile(jenovaCachePath + "LinkerCommand.txt", linkerArgument);

            // Run Linker
            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.dwFlags |= STARTF_USESTDHANDLES;

            // Create pipes for capturing output
            HANDLE hStdOutRead, hStdOutWrite;
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
            SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
            si.hStdOutput = hStdOutWrite;
            si.hStdError = hStdOutWrite;

            // Convert linker command to wide string
            std::wstring wLinkerArgument(linkerArgument.begin(), linkerArgument.end());

            // Execute the linker command
            if (!CreateProcessW(NULL, &wLinkerArgument[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L667 : Failed to Run Linker!";
                return result;
            }

            // Close the write end of the pipe
            CloseHandle(hStdOutWrite);

            // Read the linker output
            std::vector<char> buffer(jenova::GlobalSettings::BuildOutputBufferSize);
            DWORD bytesRead;
            std::string resultOutput;
            while (ReadFile(hStdOutRead, buffer.data(), buffer.size(), &bytesRead, NULL) && bytesRead > 0) resultOutput.append(buffer.data(), bytesRead);

            // Wait for the process to finish
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code of the linker process
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Close handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(hStdOutRead);

            // Set the build result
            result.buildResult = (exitCode == 0);
            result.hasError = (exitCode != 0);
            result.buildError = AS_GD_STRING(resultOutput);
            result.buildVerbose = AS_GD_STRING(resultOutput);

            // Return if There's error
            if (!result.buildResult) return result;

            // Read module to buffer
            std::ifstream moduleReader(outputModule, std::ios::binary);
            result.builtModuleData = std::vector<uint8_t>(std::istreambuf_iterator<char>(moduleReader), {});

            // Validate Module Buffer
            if (result.builtModuleData.size() == 0)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L668 : Invalid Module Data.";
                return result;
            }

            // Generate Function Information
            std::string funcInfoCmd = R"(-q -batch -ex "set logging file %FUNC_INFO_FILE%" -ex "set logging enabled" -ex "info functions" -ex "quit" "%BINARY%" > NUL)";
            jenova::ReplaceAllMatchesWithString(funcInfoCmd, "%FUNC_INFO_FILE%", AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(outputMap).stem().string() + ".finfo");
            jenova::ReplaceAllMatchesWithString(funcInfoCmd, "%BINARY%", outputModule);
            if (jenova::ExecuteCommand(this->debuggerBinaryPath, funcInfoCmd) != 0)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L850 : Failed to Extract Module Function Information.";
                return result;
            }

            // Generate Variable Information
            std::string varInfoCmd = R"(-q -batch -ex "set logging file %VAR_INFO_FILE%" -ex "set logging enabled" -ex "info variables" -ex "quit" "%BINARY%" > NUL)";
            jenova::ReplaceAllMatchesWithString(varInfoCmd, "%VAR_INFO_FILE%", AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(outputMap).stem().string() + ".pinfo");
            jenova::ReplaceAllMatchesWithString(varInfoCmd, "%BINARY%", outputModule);
            if (jenova::ExecuteCommand(this->debuggerBinaryPath, varInfoCmd) != 0)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L860 : Failed to Extract Module Variable Information.";
                return result;
            }

            // Generate Metadata
            result.moduleMetaData = JenovaInterpreter::GenerateModuleMetadata(outputMap, scriptModules, result);
            if (result.moduleMetaData.empty())
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L669 : Failed to Generate Module Meta Data.";
                return result;
            }

            // Generate Build Cache
            if (!jenova::CreateBuildCacheDatabase(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, scriptModules, linkerSettings["CppHeaderFiles"]))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L670 : Failed to Generate Build Cache Database.";
                return result;
            }

            // Yield Engine
            OS::get_singleton()->delay_msec(1);
            std::this_thread::yield();

            // Return Final Result
            return result;
        }
        bool SetCompilerOption(const String& optName, const Variant& optValue)
        {
            internalDefaultSettings[optName] = optValue;
            return true;
        }
        Variant GetCompilerOption(const String& optName) const
        {
            if (internalDefaultSettings.has(optName)) return internalDefaultSettings[optName];
            return Variant();
        }
        Variant ExecuteCommand(const String& commandName, const Dictionary& commandSettings)
        {
            // Process Commands
            if (commandName == "Solve-Compiler-Settings")
            {
                return SolveCompilerSettings(internalDefaultSettings);
            }

            // Invalid Command
            return Variant::NIL;
        }
        CompilerFeatures GetCompilerFeatures() const
        {
            return CanCompileFromFile | CanGenerateMappingData | CanGenerateModule | CanLinkObjectFiles;
        }
        CompilerModel GetCompilerModel() const
        {
            return compilerModel;
        }
        bool SolveCompilerSettings(const Dictionary& compilerSettings)
        {
            // Get Project Path
            String projectPath = jenova::GetJenovaProjectDirectory();
        
            // Collect Compiler & GodotKit Packages
            String selectedCompilerPath = jenova::GetInstalledCompilerPathFromPackages(compilerSettings["cpp_toolchain_path"], GetCompilerModel());
            String selectedGodotKitPath = jenova::GetInstalledGodotKitPathFromPackages(compilerSettings["cpp_godotsdk_path"]);

            // Validate Compiler & GodotKit Packages
            if (selectedCompilerPath == "Missing-Compiler-1.0.0")
            {
                jenova::Error("Jenova MinGW Compiler", "No MinGW Compiler Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }
            if (selectedGodotKitPath == "Missing-GodotKit-1.0.0")
            {
                jenova::Error("Jenova MinGW Compiler", "No GodotSDK Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }

            // Globalize Paths
            selectedCompilerPath = ProjectSettings::get_singleton()->globalize_path(selectedCompilerPath);
            selectedGodotKitPath = ProjectSettings::get_singleton()->globalize_path(selectedGodotKitPath);

            // Solve Compiler Paths
            this->projectPath = std::filesystem::absolute(AS_STD_STRING(projectPath)).string();
            this->binariesPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + "/bin")).string();
            this->compilerBinaryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_compiler_binary"]))).string();
            this->linkerBinaryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_linker_binary"]))).string();
            this->debuggerBinaryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_gdb_binary"]))).string();
            this->includePath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_include_path"]))).string();
            this->libraryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + String(compilerSettings["cpp_library_path"]))).string();
            this->jenovaSDKPath = std::filesystem::absolute(AS_STD_STRING(projectPath + String(compilerSettings["cpp_jenovasdk_path"]))).string();
            this->godotSDKPath = std::filesystem::absolute(AS_STD_STRING(selectedGodotKitPath)).string();
            this->jenovaCachePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory());

            // Store Solved Paths
            this->internalDefaultSettings["compiler_solved_binary_path"] = String(this->compilerBinaryPath.c_str());
            this->internalDefaultSettings["linker_solved_binary_path"] = String(this->linkerBinaryPath.c_str());

            // All Good
            return true;
        }

    private:
        CompilerModel compilerModel = CompilerModel::Unspecified;
        Dictionary internalDefaultSettings;
        std::string projectPath;
        std::string binariesPath;
        std::string compilerBinaryPath;
        std::string linkerBinaryPath;
        std::string debuggerBinaryPath;
        std::string includePath;
        std::string libraryPath;
        std::string jenovaSDKPath;
        std::string godotSDKPath;
        std::string jenovaCachePath;
    };

    #endif // Windows Compilers

    // Linux Compilers
    #ifdef TARGET_PLATFORM_LINUX

    // Jenova GNU Compiler Implementation
    class GNUCompiler : public IJenovaCompiler
    {
    public:
        GNUCompiler()
        {
        }
        ~GNUCompiler()
        {
        }
        bool InitializeCompiler(String compilerInstanceName = "<JenovaGNUCompiler>")
        {
            // Initialize Tool Chain Settings
            internalDefaultSettings["instance_name"]                        = compilerInstanceName;
            internalDefaultSettings["instance_version"]                     = 1.0f;
            internalDefaultSettings["cpp_compiler_binary"]                  = "g++"; 
            internalDefaultSettings["cpp_linker_binary"]                    = "g++";
            internalDefaultSettings["cpp_include_path"]                     = "/include";                           // Default include path
            internalDefaultSettings["cpp_library_path"]                     = "/lib";                               // Default library path
            internalDefaultSettings["cpp_symbols_path"]                     = "/Symbols";
            internalDefaultSettings["cpp_jenova_path"]                      = "/Jenova/";        
            internalDefaultSettings["cpp_jenovasdk_path"]                   = "/Jenova/JenovaSDK";
            internalDefaultSettings["cpp_godotsdk_path"]                    = "/Jenova/GodotSDK";                   // Placeholder

            // GNU Compiler Settings
            internalDefaultSettings["cpp_language_standards"]               = "cpp20";                              // -std=c++20
            internalDefaultSettings["cpp_open_mp_support"]                  = true;                                 // -fopenmp
            internalDefaultSettings["cpp_debug_database"]                   = true;                                 // -g
            internalDefaultSettings["cpp_extra_compiler"]                   = "-O2 -march=native";                  // Extra Compiler Options
            internalDefaultSettings["cpp_definitions"]                      = "TYPED_METHOD_BIND;HOT_RELOAD_ENABLED";

            // GNU Linker Settings
            internalDefaultSettings["cpp_output_module"]                    = "Jenova.Module.so";
            internalDefaultSettings["cpp_output_map"]                       = "Jenova.Module.map";
            internalDefaultSettings["cpp_default_libs"]                     = "m;pthread;dl;rt";
            internalDefaultSettings["cpp_native_libs"]                      = "libGodot.x64.a;Jenova.Runtime.Linux64.so";
            internalDefaultSettings["cpp_machine_architecture"]             = "Linux64";                            // -m64
            internalDefaultSettings["cpp_machine_pe_type"]                  = "so";                                 // -shared
            internalDefaultSettings["cpp_dynamic_base"]                     = true;                                 // -fPIC
            internalDefaultSettings["cpp_debug_symbol"]                     = true;                                 // -ggdb
            internalDefaultSettings["cpp_strip_symbol"]                     = false;                                // -Wl,--strip-all         
            internalDefaultSettings["cpp_statics_libs"]                     = "-static-libstdc++ -static-libgcc";   // Static Libraries
            internalDefaultSettings["cpp_extra_linker"]                     = "";                                   // Extra Linker Options 

            // All Good
            return true;
        }
        bool ReleaseCompiler()
        {
            // Release Resources
            internalDefaultSettings.clear();
            return true;
        }
        String PreprocessScript(Ref<CPPScript> cppScript, const Dictionary& preprocessorSettings)
        {
            // Get Original Source Code
            String scriptSourceCode = cppScript->get_source_code();

            // Reset Line Number
            scriptSourceCode = scriptSourceCode.insert(0, "#line 1\n");

            // Process And Extract Properties
            jenova::SerializedData propertiesMetadata = jenova::ProcessAndExtractPropertiesFromScript(scriptSourceCode, cppScript->GetScriptIdentity());
            if (!propertiesMetadata.empty() && propertiesMetadata != "null") jenova::WriteStdStringToFile(AS_STD_STRING(String(preprocessorSettings["PropertyMetadata"])), propertiesMetadata);

            // Preprocessor Definitions [Header]
            String preprocessorDefinitions = "// Jenova Preprocessor Definitions\n";

            // Preprocessor Definitions [Version]
            preprocessorDefinitions += String(jenova::Format("#define JENOVA_VERSION \"%d.%d.%d.%d\"\n",
                jenova::GlobalSettings::JenovaBuildVersion[0], jenova::GlobalSettings::JenovaBuildVersion[1],
                jenova::GlobalSettings::JenovaBuildVersion[2], jenova::GlobalSettings::JenovaBuildVersion[3]).c_str());

            // Preprocessor Definitions [Compiler]
            preprocessorDefinitions += "#define JENOVA_COMPILER \"GNU Compiler Collection\"\n";
            preprocessorDefinitions += "#define GCC_COMPILER\n";

            // Preprocessor Definitions [User]
            String userPreprocessorDefinitions = preprocessorSettings["PreprocessorDefinitions"];
            PackedStringArray userPreprocessorDefinitionsList = userPreprocessorDefinitions.split(";");
            for (const auto& definition : userPreprocessorDefinitionsList) if (!definition.is_empty()) preprocessorDefinitions += "#define " + definition + "\n";

            // Add Final Preprocessor Definitions
            scriptSourceCode = scriptSourceCode.insert(0, preprocessorDefinitions + "\n");

            // Replecements
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptToolIdentifier, "#define TOOL_SCRIPT");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockBeginIdentifier, "namespace JNV_" + cppScript->GetScriptIdentity() + " {");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockEndIdentifier, "}; using namespace JNV_" + cppScript->GetScriptIdentity() + ";");
            scriptSourceCode = scriptSourceCode.replace(" OnReady", " _ready");
            scriptSourceCode = scriptSourceCode.replace(" OnAwake", " _enter_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnDestroy", " _exit_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnProcess", " _process");
            scriptSourceCode = scriptSourceCode.replace(" OnPhysicsProcess", " _physics_process");
            scriptSourceCode = scriptSourceCode.replace(" OnInput", " _input");
            scriptSourceCode = scriptSourceCode.replace(" OnUserInterfaceInput", " _gui_input");

            // Return Preprocessed Source
            return scriptSourceCode;
        }
        CompileResult CompileScript(const String sourceCode)
        {
            return CompileScriptWithCustomSettings(sourceCode, this->internalDefaultSettings);
        }
        CompileResult CompileScriptWithCustomSettings(const String sourceCode, const Dictionary& compilerSettings)
        {
            CompileResult result;
            result.hasError = true;
            result.compileError = "This Compiler does not support Compilation from Memory!";
            return result;
        }
        CompileResult CompileScriptFromFile(const String scriptFilePath)
        {
            return CompileScriptFromFileWithCustomSettings(scriptFilePath, this->internalDefaultSettings);
        }
        CompileResult CompileScriptFromFileWithCustomSettings(const String scriptFilePath, const Dictionary& compilerSettings)
        {
            CompileResult result;
            result.hasError = true;
            result.compileError = "The requested compilation method is currently Not Implemented.";
            return result;
        }
        CompileResult CompileScriptModuleContainer(const ScriptModuleContainer& scriptModulesContainer)
        {
            return CompileScriptModuleWithCustomSettingsContainer(scriptModulesContainer, this->internalDefaultSettings);
        }
        CompileResult CompileScriptModuleWithCustomSettingsContainer(const ScriptModuleContainer& scriptModulesContainer, const Dictionary& compilerSettings)
        {
            // Create Compiler Result
            CompileResult result;

            // Solve Settings [Cache]
            if (!SolveCompilerSettings(compilerSettings))
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C666 : Unable to Solve Compiler Settings.";
                return result;
            }
            
            // Utilities
            auto GeneratePreprocessDefinitions = [](const godot::String& defsSetting) -> std::string
            {
                std::string defs = AS_STD_STRING(defsSetting);
                if (defs.empty()) return "";
                if (defs.back() == ';') defs.pop_back();
                std::vector<std::string> defsArray;
                size_t start = 0;
                size_t end = defs.find(';');
                while (end != std::string::npos) {
                    defsArray.push_back(defs.substr(start, end - start));
                    start = end + 1;
                    end = defs.find(';', start);
                }
                defsArray.push_back(defs.substr(start));
                std::string result;
                for (const auto& def : defsArray) result += "-D" + def + " ";
                return result;
            };
            auto GenerateAdditionalIncludeDirectories = [](const godot::String& additionalDirs) -> std::string
            {
                std::string dirs = AS_STD_STRING(additionalDirs);
                if (dirs.empty()) return "";
                if (dirs.back() == ';') dirs.pop_back();
                std::vector<std::string> dirArray;
                size_t start = 0;
                size_t end = dirs.find(';');
                while (end != std::string::npos)
                {
                    dirArray.push_back(dirs.substr(start, end - start));
                    start = end + 1;
                    end = dirs.find(';', start);
                }
                dirArray.push_back(dirs.substr(start));
                std::string result;
                for (const auto& dir : dirArray) result += "-I" + dir + " ";
                return result;
            };

            // Load Cache if Exists
            bool buildCacheFileFound = false;
            jenova::json_t buildCacheDatabase;
            if (!std::filesystem::exists(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile))
            {
                // Cache Doesn't Exist, Generate It
                std::string cacheDatabaseFilePath = this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile;
                if (!jenova::CreateBuildCacheDatabase(cacheDatabaseFilePath, scriptModulesContainer.scriptModules, compilerSettings["CppHeaderFiles"], true))
                {
                    result.compileResult = false;
                    result.hasError = true;
                    result.compileError = "C670 : Failed to Generate Build Cache Database.";
                    return result;
                }
            }

            // Parse Cache File
            try
            {
                std::ifstream buildCacheDatabaseReader(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, std::ios::binary);
                std::string buildCacheDatabaseContent(std::istreambuf_iterator<char>(buildCacheDatabaseReader), {});
                if (!buildCacheDatabaseContent.empty())
                {
                    buildCacheDatabase = jenova::json_t::parse(buildCacheDatabaseContent);
                    buildCacheFileFound = true;
                }
            }
            catch (const std::exception&)
            {
                result.compileResult = false;
                result.hasError = true;
                result.compileError = "C671 : Failed to Parse Build Cache Database.";
                return result;
            }

            // Check If Any Changes Applied to Headers
            if (buildCacheDatabase.contains("Headers"))
            {
                // Detect Changes
                bool detectedHeaderChanges = false;
                PackedStringArray cppHeaderFiles = compilerSettings["CppHeaderFiles"];
                for (const auto& cppHeaderFile : cppHeaderFiles)
                {
                    String cppHeaderUID = jenova::GenerateStandardUIDFromPath(cppHeaderFile);
                    String cppHeaderHash = jenova::GenerateMD5HashFromFile(cppHeaderFile);
                    if (buildCacheDatabase["Headers"].contains(AS_STD_STRING(cppHeaderUID)))
                    {
                        if (AS_STD_STRING(cppHeaderHash) != buildCacheDatabase["Headers"][AS_STD_STRING(cppHeaderUID)].get<std::string>())
                        {
                            detectedHeaderChanges = true;
                            break;
                        }
                    }
                }
                if (buildCacheDatabase.contains("HeaderCount"))
                {
                    int headerCount = buildCacheDatabase["HeaderCount"].get<int>();
                    if (headerCount != cppHeaderFiles.size()) detectedHeaderChanges = true;
                }

                // If Contains Changes, Reset All Script Cache
                if (detectedHeaderChanges)
                {
                    if (buildCacheDatabase.contains("Modules")) for (auto& scriptModule : buildCacheDatabase["Modules"].items()) scriptModule.value() = "No Hash";
                }
            }

            // Create Task List for Parallel Compilation
            std::vector<TaskID> taskIDs; size_t taskIndex = 0;
            std::vector<int> taskResults(scriptModulesContainer.scriptModules.size(), -1);
            for (const auto& scriptModule : scriptModulesContainer.scriptModules)
            {
                // Skip If File Hash Didn't Change
                if (buildCacheDatabase.contains("Modules"))
                {
                    if (buildCacheDatabase["Modules"].contains(AS_STD_STRING(scriptModule.scriptUID)))
                    {
                        if (AS_STD_STRING(scriptModule.scriptHash) == buildCacheDatabase["Modules"][AS_STD_STRING(scriptModule.scriptUID)].get<std::string>()) continue;
                    }
                }

                // Generate Command for Each Script Module
                std::string compilerArgument = AS_STD_STRING(String(compilerSettings["cpp_compiler_binary"]));

                // Compile Without Linking
                compilerArgument += " -c ";

                // Language Standards
                if (String(compilerSettings["cpp_language_standards"]) == "cpp20") compilerArgument += "-std=c++20 ";
                if (String(compilerSettings["cpp_language_standards"]) == "cpp17") compilerArgument += "-std=c++17 ";

                // Debug Symbols
                if (bool(compilerSettings["cpp_debug_database"])) compilerArgument += "-g ";

                // Dynamic Base
                if (bool(compilerSettings["cpp_dynamic_base"])) compilerArgument += "-fPIC ";

                // Extra Compiler Flags
                compilerArgument += AS_STD_STRING(String(compilerSettings["cpp_extra_compiler"])) + " ";

                // Preprocessor Definitions
                compilerArgument += GeneratePreprocessDefinitions(compilerSettings["cpp_definitions"]);

                // Include Paths
                compilerArgument += "-I./ ";
                compilerArgument += "-I\"" + this->includePath + "\" ";
                compilerArgument += "-I\"" + this->jenovaSDKPath + "\" ";
                compilerArgument += "-I\"" + this->godotSDKPath + "\" ";

                // Force Include JenovaSDK Header
                if (jenova::GlobalSettings::ForceJenovaSDKHeader && jenova::GlobalStorage::UseBuiltinSDK) compilerArgument += "-include JenovaSDK.h ";

                // Add Additional Include Directories
                compilerArgument += GenerateAdditionalIncludeDirectories(compilerSettings["cpp_extra_include_directories"]);

                // Add Source File
                compilerArgument += "\"" + AS_STD_STRING(scriptModule.scriptCacheFile) + "\" ";

                // Specify Object File Output
                compilerArgument += "-o \"" + AS_STD_STRING(scriptModule.scriptObjectFile) + "\" ";

                // Store Task Index for Results
                size_t currentTaskIndex = taskIndex++;
                taskIDs.push_back(JenovaTaskSystem::InitiateTask([compilerArgument, &taskResults, currentTaskIndex, scriptModule]()
                {
                    // Run the compiler command using a process and capture its output
                    int pipefd[2];
                    if (pipe(pipefd) == -1)
                    {
                        taskResults[currentTaskIndex] = 1; // Non-zero indicates failure
                        jenova::Output("Failed to create pipe for capturing output.");
                        return;
                    }

                    pid_t pid = fork();
                    if (pid == -1)
                    {
                        close(pipefd[0]);
                        close(pipefd[1]);
                        taskResults[currentTaskIndex] = 1; // Non-zero indicates failure
                        jenova::Output("Failed to fork process for compilation.");
                        return;
                    }

                    if (pid == 0)
                    {
                        // Child process: Redirect output to pipe
                        dup2(pipefd[1], STDOUT_FILENO);
                        dup2(pipefd[1], STDERR_FILENO);
                        close(pipefd[0]); // Close unused read end
                        close(pipefd[1]);

                        // Execute compiler command
                        setenv("LANG", "C.UTF-8", 1);
                        setenv("LC_ALL", "C.UTF-8", 1);
                        execl("/bin/sh", "sh", "-c", compilerArgument.c_str(), nullptr);
                    }
                    else
                    {
                        // Parent process: Capture output
                        close(pipefd[1]); // Close unused write end
                        char buffer[128];
                        std::string resultOutput;

                        // Read output from the pipe
                        ssize_t bytesRead;
                        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
                        {
                            buffer[bytesRead] = '\0';
                            resultOutput += buffer;
                        }

                        // Close the read end
                        close(pipefd[0]); 

                        // Wait for the child process to finish
                        int status;
                        waitpid(pid, &status, 0);
                        taskResults[currentTaskIndex] = WIFEXITED(status) ? WEXITSTATUS(status) : 1;

                        // Log the output
                        if (!resultOutput.empty())
                        {
                            // Replace UTF-8 Smart Quotes With ASCII Equivalents
                            std::string sanitized;
                            for (size_t i = 0; i < resultOutput.size(); i++)
                            {
                                if (i + 2 < resultOutput.size() && static_cast<unsigned char>(resultOutput[i]) == 0xE2 && static_cast<unsigned char>(resultOutput[i + 1]) == 0x80)
                                {
                                    if (static_cast<unsigned char>(resultOutput[i + 2]) == 0x98 || static_cast<unsigned char>(resultOutput[i + 2]) == 0x99)
                                    {
                                        sanitized += '\'';
                                        i += 2;
                                        continue;
                                    }
                                }
                                sanitized += resultOutput[i];
                            }
                            jenova::Error("Jenova Compiler", "Compile Error : %s", sanitized.c_str());
                        }
                        if (taskResults[currentTaskIndex] != 0)
                        {
                            jenova::Output("Script [%s] Compilation Failed.", AS_C_STRING(scriptModule.scriptHash));
                        }
                    }
                }));

                // Increment Successfully Compiled Scripts
                result.scriptsCount++;

            }

            // Skip Compile If Source Count is 0
            if (result.scriptsCount == 0)
            {
                result.compileResult = true;
                result.hasError = false;
                result.compileVerbose = "Cache System Detected No Script Requires to be Compiled.";
                return result;
            }

            // Wait for All Tasks to Complete
            for (const auto& taskID : taskIDs)
            {
                while (!JenovaTaskSystem::IsTaskComplete(taskID))
                {
                    std::this_thread::yield();
                }
                JenovaTaskSystem::ClearTask(taskID);
            }

            // Aggregate Results
            for (size_t i = 0; i < taskResults.size(); i++)
            {
                // Skip tasks not executed (cached scripts)
                if (taskResults[i] == -1) continue;

                // Check for failures
                if (taskResults[i] != 0)
                {
                    result.compileResult = false;
                    result.hasError = true;
                    result.compileError = "C667 : Compilation Failed for One or More Script Modules.";
                    return result;
                }
            }

            // Compilation Successful
            result.compileResult = true;
            result.hasError = false;
            result.compileVerbose = "Compilation Successful for All Script Modules.";

            return result;
        }
        BuildResult BuildFinalModule(const jenova::ModuleList& scriptModules)
        {
            return BuildFinalModuleWithCustomSettings(scriptModules, internalDefaultSettings);
        }
        BuildResult BuildFinalModuleWithCustomSettings(const jenova::ModuleList& scriptModules, const Dictionary& linkerSettings)
        {
            // Create Build Result
            BuildResult result;

            // Solve Settings [Cache]
            if (!SolveCompilerSettings(linkerSettings))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L666 : Unable to Solve Linker Settings.";
                return result;
            }

            // Set Output Directory Path on Build Result
            result.buildPath = this->jenovaCachePath;

            // Set Compiler Model
            result.compilerModel = this->GetCompilerModel();

            // Set Debug Information Flag
            result.hasDebugInformation = bool(linkerSettings["cpp_debug_symbol"]);

            // Generate Output Module Path
            std::string outputModule = this->jenovaCachePath + AS_STD_STRING((String)linkerSettings["cpp_output_module"]);
            std::string outputMap = this->jenovaCachePath + AS_STD_STRING((String)linkerSettings["cpp_output_map"]);

            // Utilities
            auto GenerateLibraries = [](const godot::String& libsSetting, bool noPrefix = false) -> std::string
            {
                std::string libs = AS_STD_STRING(libsSetting);
                if (libs.empty()) return "";
                if (libs.back() == ';') libs.pop_back();
                std::vector<std::string> libsArray;
                size_t start = 0;
                size_t end = libs.find(';');
                while (end != std::string::npos)
                {
                    libsArray.push_back(libs.substr(start, end - start));
                    start = end + 1;
                    end = libs.find(';', start);
                }
                libsArray.push_back(libs.substr(start));
                std::string result;
                if (noPrefix) for (const auto& lib : libsArray) result += "-l:" + lib + " ";
                else for (const auto& lib : libsArray) result += "-l" + lib + " ";
                return result;
            };
            auto GenerateLibraryPaths = [](const godot::String& additionalDirs) -> std::string
            {
                std::string dirs = AS_STD_STRING(additionalDirs);
                if (dirs.empty()) return "";
                if (dirs.back() == ';') dirs.pop_back();
                std::vector<std::string> dirArray;
                size_t start = 0;
                size_t end = dirs.find(';');
                while (end != std::string::npos)
                {
                    dirArray.push_back(dirs.substr(start, end - start));
                    start = end + 1;
                    end = dirs.find(';', start);
                }
                dirArray.push_back(dirs.substr(start));
                std::string result;
                for (const auto& dir : dirArray) result += "-L\"" + dir + "\" ";
                return result;
            };

            // Generate Linker Arguments
            std::string linkerArgument = AS_STD_STRING(String(linkerSettings["cpp_linker_binary"]));
            linkerArgument += " -o \"" + outputModule + "\" ";
            if (result.hasDebugInformation && bool(linkerSettings["cpp_debug_symbol"])) linkerArgument += "-ggdb ";
            linkerArgument += "-Wl,-Map=\"" + outputMap + "\" ";
            linkerArgument += "-shared ";
            linkerArgument += "-fPIC ";

            // Machine Architecture
            if (String(linkerSettings["cpp_machine_architecture"]) == "Linux64") linkerArgument += "-m64 ";
            if (String(linkerSettings["cpp_machine_architecture"]) == "Linux32") linkerArgument += "-m32 ";

            // Add Library Paths
            linkerArgument += "-L./ ";
            linkerArgument += "-L\"" + this->libraryPath + "\" ";
            linkerArgument += "-L\"" + this->jenovaPath + "\" ";
            linkerArgument += "-L\"" + this->jenovaSDKPath + "\" ";
            linkerArgument += "-L\"" + this->godotSDKPath + "\" ";
            linkerArgument += GenerateLibraryPaths(linkerSettings["cpp_extra_library_directories"]);

            // Add Object Files
            for (const auto& scriptModule : scriptModules)
            {
                linkerArgument += "\"" + AS_STD_STRING(scriptModule.scriptObjectFile) + "\" ";
            }

            // Strip Symbols
            if (bool(linkerSettings["cpp_strip_symbol"])) linkerArgument += "-Wl,--strip-all ";

            // Add Dependency Path
            linkerArgument += "-Wl,-rpath,./Jenova ";

            // Add Extra Options
            linkerArgument += AS_STD_STRING(String(linkerSettings["cpp_extra_linker"])) + " ";

            // Add Libraries [GCC Requires Libraries to be Added at the End]
            linkerArgument += GenerateLibraries(linkerSettings["cpp_native_libs"], true);
            linkerArgument += GenerateLibraries(linkerSettings["cpp_default_libs"]);
            linkerArgument += GenerateLibraries(linkerSettings["cpp_extra_libs"]);

            // Dump Linker Command If Developer Mode Enabled
            if (jenova::GlobalStorage::DeveloperModeActivated) jenova::WriteStdStringToFile(this->jenovaCachePath + "LinkerCommand.txt", linkerArgument);

            // Run Linker Command Using Process Management
            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L667 : Failed to create pipe for linking process.";
                return result;
            }

            pid_t pid = fork();
            if (pid == -1)
            {
                close(pipefd[0]);
                close(pipefd[1]);
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L668 : Failed to fork process for linking.";
                return result;
            }

            if (pid == 0)
            {
                // Child process: Redirect output to pipe
                dup2(pipefd[1], STDOUT_FILENO);
                dup2(pipefd[1], STDERR_FILENO);
                close(pipefd[0]); // Close unused read end
                close(pipefd[1]);

                // Execute linker command
                setenv("LANG", "C.UTF-8", 1);
                setenv("LC_ALL", "C.UTF-8", 1);
                execl("/bin/sh", "sh", "-c", linkerArgument.c_str(), nullptr);

                // If execl fails
                _exit(127);
            }
            else
            {
                // Parent process: Capture output
                close(pipefd[1]); // Close unused write end
                char buffer[128];
                std::string resultOutput;

                // Read output from the pipe
                ssize_t bytesRead;
                while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
                {
                    buffer[bytesRead] = '\0';
                    resultOutput += buffer;
                }
                close(pipefd[0]); // Close the read end

                // Wait for the child process to finish
                int status;
                waitpid(pid, &status, 0);
                result.buildResult = WIFEXITED(status) ? (WEXITSTATUS(status) == 0) : false;
                result.hasError = !result.buildResult;

                // Log the linker output
                if (!resultOutput.empty())
                {
                    // Replace UTF-8 Smart Quotes With ASCII Equivalents
                    std::string sanitized;
                    for (size_t i = 0; i < resultOutput.size(); i++)
                    {
                        if (i + 2 < resultOutput.size() && static_cast<unsigned char>(resultOutput[i]) == 0xE2 && static_cast<unsigned char>(resultOutput[i + 1]) == 0x80)
                        {
                            if (static_cast<unsigned char>(resultOutput[i + 2]) == 0x98 || static_cast<unsigned char>(resultOutput[i + 2]) == 0x99)
                            {
                                sanitized += '\'';
                                i += 2;
                                continue;
                            }
                        }
                        sanitized += resultOutput[i];
                    }
                    jenova::Error("Jenova Linker", "Linker Error: %s", sanitized.c_str());
                }
                if (!result.buildResult)
                {
                    result.hasError = true;
                    result.buildError = AS_GD_STRING(resultOutput);
                    return result;
                }
            }

            // Read Module to Buffer
            std::ifstream moduleReader(outputModule, std::ios::binary);
            result.builtModuleData = std::vector<uint8_t>(std::istreambuf_iterator<char>(moduleReader), {});

            // Validate Module Buffer
            if (result.builtModuleData.empty())
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L670 : Invalid Module Data.";
                return result;
            }

            // Generate Function Information
            std::string funcInfoCmd = R"(gdb -q -batch -ex "set logging file "%FUNC_INFO_FILE%"" -ex "set logging on" -ex "info functions" -ex "quit" "%BINARY%" > /dev/null 2>&1)";
            jenova::ReplaceAllMatchesWithString(funcInfoCmd, "%FUNC_INFO_FILE%", AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(outputMap).stem().string() + ".finfo");
            jenova::ReplaceAllMatchesWithString(funcInfoCmd, "%BINARY%", outputModule);
            if (jenova::ExecuteCommand(std::string(), funcInfoCmd) != 0)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L850 : Failed to Extract Module Function Information.";
                return result;
            }

            // Generate Variable Information
            std::string varInfoCmd = R"(gdb -q -batch -ex "set logging file "%VAR_INFO_FILE%"" -ex "set logging on" -ex "info variables" -ex "quit" "%BINARY%" > /dev/null 2>&1)";
            jenova::ReplaceAllMatchesWithString(varInfoCmd, "%VAR_INFO_FILE%", AS_STD_STRING(jenova::GetJenovaCacheDirectory()) + std::filesystem::path(outputMap).stem().string() + ".pinfo");
            jenova::ReplaceAllMatchesWithString(varInfoCmd, "%BINARY%", outputModule);
            if (jenova::ExecuteCommand(std::string(), varInfoCmd) != 0)
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L860 : Failed to Extract Module Variable Information.";
                return result;
            }

            // Generate Metadata
            result.moduleMetaData = JenovaInterpreter::GenerateModuleMetadata(outputMap, scriptModules, result);
            if (result.moduleMetaData.empty())
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L671 : Failed to Generate Module Metadata.";
                return result;
            }

            // Generate Build Cache
            if (!jenova::CreateBuildCacheDatabase(this->jenovaCachePath + jenova::GlobalSettings::JenovaBuildCacheDatabaseFile, scriptModules, linkerSettings["CppHeaderFiles"]))
            {
                result.buildResult = false;
                result.hasError = true;
                result.buildError = "L672 : Failed to Generate Build Cache Database.";
                return result;
            }

            // Return Final Result
            return result;
        }
        bool SetCompilerOption(const String& optName, const Variant& optValue)
        {
            internalDefaultSettings[optName] = optValue;
            return true;
        }
        Variant GetCompilerOption(const String& optName) const
        {
            if (internalDefaultSettings.has(optName)) return internalDefaultSettings[optName];
            return Variant();
        }
        Variant ExecuteCommand(const String& commandName, const Dictionary& commandSettings)
        {
            // Process Commands
            if (commandName == "Solve-Compiler-Settings")
            {
                return SolveCompilerSettings(internalDefaultSettings);
            }

            // Invalid Command
            return Variant::NIL;
        }
        CompilerFeatures GetCompilerFeatures() const
        {
            return CanCompileFromFile | CanGenerateMappingData | CanGenerateModule | CanLinkObjectFiles;
        }
        CompilerModel GetCompilerModel() const
        {
            return CompilerModel::GNUCompiler;
        }
        bool SolveCompilerSettings(const Dictionary& compilerSettings)
        {
            // Get Project Path
            String projectPath = jenova::GetJenovaProjectDirectory();
                    
            // Collect Compiler & GodotKit Packages
            String selectedCompilerPath = jenova::GetInstalledCompilerPathFromPackages(compilerSettings["cpp_toolchain_path"], GetCompilerModel());
            String selectedGodotKitPath = jenova::GetInstalledGodotKitPathFromPackages(compilerSettings["cpp_godotsdk_path"]);

            // Validate Compiler & GodotKit Packages
            if (selectedCompilerPath == "Missing-Compiler-1.0.0")
            {
                jenova::Error("Jenova GNU Compiler", "No GNU Compiler Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }
            if (selectedGodotKitPath == "Missing-GodotKit-1.0.0")
            {
                jenova::Error("Jenova GNU Compiler", "No GodotSDK Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }

            // Globalize Paths
            selectedCompilerPath = ProjectSettings::get_singleton()->globalize_path(selectedCompilerPath);
            selectedGodotKitPath = ProjectSettings::get_singleton()->globalize_path(selectedGodotKitPath);

            // Solve Compiler Paths
            this->projectPath = std::filesystem::absolute(AS_STD_STRING(projectPath)).string();
            this->includePath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + (String)compilerSettings["cpp_include_path"])).string();
            this->libraryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + (String)compilerSettings["cpp_library_path"])).string();
            this->jenovaPath = std::filesystem::absolute(AS_STD_STRING(projectPath + (String)compilerSettings["cpp_jenova_path"])).string();           
            this->jenovaSDKPath = std::filesystem::absolute(AS_STD_STRING(projectPath + (String)compilerSettings["cpp_jenovasdk_path"])).string();
            this->godotSDKPath = std::filesystem::absolute(AS_STD_STRING(selectedGodotKitPath)).string();
            this->jenovaCachePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory());

            // Store Solved Paths
            this->internalDefaultSettings["compiler_solved_binary_path"] = String(internalDefaultSettings["cpp_compiler_binary"]);
            this->internalDefaultSettings["linker_solved_binary_path"] = String(internalDefaultSettings["cpp_linker_binary"]);

            // All Good
            return true;
        }

    protected:
        Dictionary internalDefaultSettings;
        std::string projectPath;
        std::string compilerBinaryPath;
        std::string linkerBinaryPath;
        std::string includePath;
        std::string libraryPath;
        std::string jenovaPath;      
        std::string jenovaSDKPath;
        std::string godotSDKPath;
        std::string jenovaCachePath;
    };

    // Jenova Clang Compiler Implementation
    class ClangCompiler : public GNUCompiler
    {
    public:
        ClangCompiler() 
        {   
        }
        ~ClangCompiler() 
        {
        }

        // Override only specific functions
        bool InitializeCompiler(String compilerInstanceName = "<JenovaClangCompiler>") override
        {
            // Initialzie GNU Compiler Settings
            if (!GNUCompiler::InitializeCompiler(compilerInstanceName)) return false;
            
            // Initialize Clang Tool Chain Settings
            internalDefaultSettings["instance_name"]                        = compilerInstanceName;
            internalDefaultSettings["cpp_compiler_binary"]                  = "clang++"; 
            internalDefaultSettings["cpp_linker_binary"]                    = "clang++";
            internalDefaultSettings["cpp_extra_linker"]                     = "-fuse-ld=bfd";

            // All Good
            return true;
        }
        String PreprocessScript(Ref<CPPScript> cppScript, const Dictionary& preprocessorSettings) override
        {
           // Get Original Source Code
            String scriptSourceCode = cppScript->get_source_code();

            // Reset Line Number
            scriptSourceCode = scriptSourceCode.insert(0, "#line 1\n");

            // Process And Extract Properties
            jenova::SerializedData propertiesMetadata = jenova::ProcessAndExtractPropertiesFromScript(scriptSourceCode, cppScript->GetScriptIdentity());
            if (!propertiesMetadata.empty() && propertiesMetadata != "null") jenova::WriteStdStringToFile(AS_STD_STRING(String(preprocessorSettings["PropertyMetadata"])), propertiesMetadata);

            // Preprocessor Definitions [Header]
            String preprocessorDefinitions = "// Jenova Preprocessor Definitions\n";

            // Preprocessor Definitions [Version]
            preprocessorDefinitions += String(jenova::Format("#define JENOVA_VERSION \"%d.%d.%d.%d\"\n",
                jenova::GlobalSettings::JenovaBuildVersion[0], jenova::GlobalSettings::JenovaBuildVersion[1],
                jenova::GlobalSettings::JenovaBuildVersion[2], jenova::GlobalSettings::JenovaBuildVersion[3]).c_str());

            // Preprocessor Definitions [Compiler]
            preprocessorDefinitions += "#define JENOVA_COMPILER \"LLVM Clang Compiler\"\n";
            preprocessorDefinitions += "#define CLANG_COMPILER\n";

            // Preprocessor Definitions [User]
            String userPreprocessorDefinitions = preprocessorSettings["PreprocessorDefinitions"];
            PackedStringArray userPreprocessorDefinitionsList = userPreprocessorDefinitions.split(";");
            for (const auto& definition : userPreprocessorDefinitionsList) if (!definition.is_empty()) preprocessorDefinitions += "#define " + definition + "\n";

            // Add Final Preprocessor Definitions
            scriptSourceCode = scriptSourceCode.insert(0, preprocessorDefinitions + "\n");

            // Replecements
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptToolIdentifier, "#define TOOL_SCRIPT");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockBeginIdentifier, "namespace JNV_" + cppScript->GetScriptIdentity() + " {");
            scriptSourceCode = scriptSourceCode.replace(jenova::GlobalSettings::ScriptBlockEndIdentifier, "}; using namespace JNV_" + cppScript->GetScriptIdentity() + ";");
            scriptSourceCode = scriptSourceCode.replace(" OnReady", " _ready");
            scriptSourceCode = scriptSourceCode.replace(" OnAwake", " _enter_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnDestroy", " _exit_tree");
            scriptSourceCode = scriptSourceCode.replace(" OnProcess", " _process");
            scriptSourceCode = scriptSourceCode.replace(" OnPhysicsProcess", " _physics_process");
            scriptSourceCode = scriptSourceCode.replace(" OnInput", " _input");
            scriptSourceCode = scriptSourceCode.replace(" OnUserInterfaceInput", " _gui_input");

            // Return Preprocessed Source
            return scriptSourceCode;
        }
        CompilerModel GetCompilerModel() const override
        {
            return CompilerModel::ClangCompiler;
        }
        bool SolveCompilerSettings(const Dictionary& compilerSettings) override
        {
            // Get Project Path
            String projectPath = jenova::GetJenovaProjectDirectory();
                    
            // Collect Compiler & GodotKit Packages
            String selectedCompilerPath = jenova::GetInstalledCompilerPathFromPackages(compilerSettings["cpp_toolchain_path"], GetCompilerModel());
            String selectedGodotKitPath = jenova::GetInstalledGodotKitPathFromPackages(compilerSettings["cpp_godotsdk_path"]);

            // Validate Compiler & GodotKit Packages
            if (selectedCompilerPath == "Missing-Compiler-1.0.0")
            {
                jenova::Error("Jenova Clang Compiler", "No Clang Compiler Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }
            if (selectedGodotKitPath == "Missing-GodotKit-1.0.0")
            {
                jenova::Error("Jenova Clang Compiler", "No GodotSDK Detected On Build System, Install At Least One From Package Manager!");
                return false;
            }

            // Globalize Paths
            selectedCompilerPath = ProjectSettings::get_singleton()->globalize_path(selectedCompilerPath);
            selectedGodotKitPath = ProjectSettings::get_singleton()->globalize_path(selectedGodotKitPath);

            // Solve Compiler Paths
            this->projectPath = std::filesystem::absolute(AS_STD_STRING(projectPath)).string();
            this->includePath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + (String)compilerSettings["cpp_include_path"])).string();
            this->libraryPath = std::filesystem::absolute(AS_STD_STRING(selectedCompilerPath + (String)compilerSettings["cpp_library_path"])).string();
            this->jenovaPath = std::filesystem::absolute(AS_STD_STRING(projectPath + (String)compilerSettings["cpp_jenova_path"])).string();           
            this->jenovaSDKPath = std::filesystem::absolute(AS_STD_STRING(projectPath + (String)compilerSettings["cpp_jenovasdk_path"])).string();
            this->godotSDKPath = std::filesystem::absolute(AS_STD_STRING(selectedGodotKitPath)).string();
            this->jenovaCachePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory());

            // Store Solved Paths
            this->internalDefaultSettings["compiler_solved_binary_path"] = String(internalDefaultSettings["cpp_compiler_binary"]);
            this->internalDefaultSettings["linker_solved_binary_path"] = String(internalDefaultSettings["cpp_linker_binary"]);

            // All Good
            return true;
        }
    };

    #endif // Linux Compilers

    // Compiler Factory
    IJenovaCompiler* CreateMicrosoftCompiler()
    {
        // Windows Implementation
        #ifdef TARGET_PLATFORM_WINDOWS
            return new MicrosoftCompiler(false);
        #endif

        // Not Supported
        return nullptr;
    }
    IJenovaCompiler* CreateClangCompiler()
    {
        // Windows Implementation
        #ifdef TARGET_PLATFORM_WINDOWS
            return new MicrosoftCompiler(true);
        #endif

        // Linux Implementation
        #ifdef TARGET_PLATFORM_LINUX
            return new ClangCompiler();
        #endif


        // Not Supported
        return nullptr;
    }
    IJenovaCompiler* CreateMinGWCompiler(bool useLLVM)
    {
        // Windows Implementation
        #ifdef TARGET_PLATFORM_WINDOWS
            return new MinGWCompiler(useLLVM);
        #endif

        // Not Supported
        return nullptr;
    }
    IJenovaCompiler* CreateGNUCompiler()
    {
        // Linux Implementation
        #ifdef TARGET_PLATFORM_LINUX
            return new GNUCompiler();
        #endif

        // Not Supported
        return nullptr;
    }
}