
/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|               Jenova Microsoft Compiler Bridge               |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Windows SDK
#include <Windows.h>

// C++ Runtime SDK
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <codecvt>
#include <locale>

// Third-Party
#include <Parsers/json.hpp>

// Configuration
#define DEVELOPER "Hamid.Memar"
#define CONFIG_FILE L"config.json"
#define OUTPUT_BUFFER_SIZE 8192
#ifdef COMPILER_BRIDGE
    #define BRIDGE_TYPE "Compiler"
    #define TARGET_EXE "cl.exe"
    #define BRIDGE_NAME "Jenova MSVC Compiler Bridge"
    #define MSVC_COMPILER_BRIDGE
#elif defined(LINKER_BRIDGE)
    #define BRIDGE_TYPE "Linker"
    #define TARGET_EXE "link.exe"
    #define BRIDGE_NAME "Jenova MSVC Linker Bridge"
    #define MSVC_LINKER_BRIDGE
#else
    #error "Bridge Mode Undefined"
#endif

// Namespaces
using namespace std;
namespace fs = std::filesystem;

// Macros
#define bridge_log(fmt,...) printf("[Jenova-Bridge] > " fmt "\n", __VA_ARGS__);

// Typedefs
typedef nlohmann::json          json_t;
typedef std::wstring            String;
typedef std::vector<String>     ArgumentList;

// Utilities
static void ShowError(const wchar_t* fmt, ...)
{
    static wchar_t buffer[1024];
    va_list args;
    va_start(args, fmt);
    int charsWritten = vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), fmt, args);
    if (charsWritten < 0)  wcscpy_s(buffer, sizeof(buffer) / sizeof(wchar_t), L"Error Formatting Message");
    va_end(args);
    MessageBoxW(NULL, buffer, L"[Bridge Error]", MB_OK | MB_ICONERROR | MB_TOPMOST);
}
static String GetConfigurationFile()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    String configPath(exePath);
    size_t pos = configPath.find_last_of(L"\\/");
    if (pos != String::npos) configPath = configPath.substr(0, pos + 1);
    configPath += CONFIG_FILE;
    return configPath;
}
static String ReadTextDataFromFile(String filePath)
{
    std::wifstream inFile(filePath);
    if (inFile.is_open())
    {
        String content((std::istreambuf_iterator<wchar_t>(inFile)), std::istreambuf_iterator<wchar_t>());
        inFile.close();
        return content;
    }
    else
    {
        return L"";
    }
}
static int RunExecutable(const String& exePath, const String& arguments)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    HANDLE hStdoutRead, hStdoutWrite;
    HANDLE hStderrRead, hStderrWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0);
    CreatePipe(&hStderrRead, &hStderrWrite, &sa, 0);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hStdoutWrite;
    si.hStdError = hStderrWrite;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    String cmdLine = L"\"" + exePath + L"\" " + arguments;
    if (!CreateProcessW(NULL, &cmdLine[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        DWORD error = GetLastError();
        CloseHandle(hStdoutRead);
        CloseHandle(hStdoutWrite);
        CloseHandle(hStderrRead);
        CloseHandle(hStderrWrite);
        ShowError(L"Failed to Execute: %s\nError: 0x%08X", exePath.c_str(), error);
        return EXIT_FAILURE;
    }
    CloseHandle(hStdoutWrite);
    CloseHandle(hStderrWrite);
    char buffer[OUTPUT_BUFFER_SIZE] = {};
    DWORD bytesRead;
    HANDLE handles[2] = { hStdoutRead, hStderrRead };
    while (true)
    {
        DWORD result = WaitForMultipleObjects(2, handles, FALSE, 100);
        if (result == WAIT_OBJECT_0 || result == WAIT_OBJECT_0 + 1)
        {
            HANDLE readHandle = handles[result - WAIT_OBJECT_0];
            if (readHandle != NULL)
            {
                if (ReadFile(readHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    printf("%s", buffer);
                }
                else
                {
                    CloseHandle(readHandle);
                    handles[result - WAIT_OBJECT_0] = NULL;
                }
            }
        }
        else if (result == WAIT_TIMEOUT)
        {
            DWORD exitCode;
            if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE) break;
        }
        else if (result == WAIT_FAILED) break;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(hStdoutRead);
    CloseHandle(hStderrRead);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return exitCode;
}
static String CreateStringFromStdString(const std::string& str)
{
    return String(str.begin(), str.end());
}

// Entrypoint
int wmain(int argc, wchar_t* argv[])
{
    // Set Title [For Fun]
	SetConsoleTitleA(BRIDGE_NAME " v1.0 | Developed By " DEVELOPER);

    // Get Arguments
    ArgumentList args;
    args.reserve(argc - 1);
    int startIndex = 0;
    if (argc > 0 && wcsstr(argv[0], L".exe") != NULL) startIndex = 1;
    for (int i = startIndex; i < argc; ++i) args.push_back(String(argv[i]));
    
    // Read Configuration
    String configPath = GetConfigurationFile();
    if (fs::exists(configPath))
    {
        try
        {
            // Parse Configuration
            json_t configuration = json_t::parse(ReadTextDataFromFile(configPath));

            // Get Configuration Data
            String msvcInstallPath = CreateStringFromStdString(configuration["msvc"]["installPath"].get<std::string>());
            String sdkVersion = CreateStringFromStdString(configuration["windowsSdk"]["version"].get<std::string>());

            // Generate Toolchain/WinSDK Paths
            String msvcBinPath = msvcInstallPath + L"\\bin\\Hostx64\\x64";
            String msvcIncludePath = msvcInstallPath + L"\\include";
            String msvcLibPath = msvcInstallPath + L"\\lib\\x64";
            String sdkRoot = L"C:\\Program Files (x86)\\Windows Kits\\10";
            String sdkUcrtIncludePath = sdkRoot + L"\\Include\\" + sdkVersion + L"\\ucrt";
            String sdkUmIncludePath = sdkRoot + L"\\Include\\" + sdkVersion + L"\\um";
            String sdkLibPath = sdkRoot + L"\\Lib\\" + sdkVersion + L"\\ucrt\\x64";
            String sdkUmLibPath = sdkRoot + L"\\Lib\\" + sdkVersion + L"\\um\\x64";
            String sdkSharedIncludePath = sdkRoot + L"\\Include\\" + sdkVersion + L"\\shared";

            // Generate Binary Path
            #ifdef COMPILER_BRIDGE
                String binaryPath = msvcBinPath + L"\\cl.exe";
            #endif
            #ifdef LINKER_BRIDGE
                String binaryPath = msvcBinPath + L"\\link.exe";
            #endif

            // Build Arguments
            String arguments;
            for (size_t i = 0; i < args.size(); ++i) arguments += L" " + args[i];

            #ifdef COMPILER_BRIDGE
                arguments += L" /I \"" + msvcIncludePath + L"\"";
                arguments += L" /I \"" + sdkUcrtIncludePath + L"\"";
                arguments += L" /I \"" + sdkUmIncludePath + L"\"";
                arguments += L" /I \"" + sdkSharedIncludePath + L"\"";
            #endif
            #ifdef LINKER_BRIDGE
                arguments += L" /LIBPATH:\"" + msvcLibPath + L"\"";
                arguments += L" /LIBPATH:\"" + sdkLibPath + L"\"";
                arguments += L" /LIBPATH:\"" + sdkUmLibPath + L"\"";
            #endif

            // Proxify & Invoke Real MSVC
            return RunExecutable(binaryPath, arguments);
        }
        catch (const std::exception& error)
        {
            ShowError(L"Failed to Parse Configuration, Parser Error : %S", error.what());
            return EXIT_FAILURE;
        }
    }
    else
    {
        ShowError(L"Configuration File is Missing.");
        return EXIT_FAILURE;
    }
}