#pragma once

// Windows Module Loader
#ifdef TARGET_PLATFORM_WINDOWS

// Windows SDK
#include <Windows.h>
#include <ntstatus.h>

// Memory Module Shell Database
#include "InternalModules.h"

// Windows Loader Flags
#define LOAD_FLAGS_NOT_MAP_DLL						0x10000000
#define LOAD_FLAGS_USE_DLL_NAME						0x00000004
#define LOAD_FLAGS_NOT_HANDLE_TLS					0x00000008

// Loader Interface [Windows]
class JenovaLoader
{
public:
	static bool Initialize()
	{
		// Create Temporary Module Loader Shell
		char tempPath[MAX_PATH] = { 0 };
		GetTempPathA(MAX_PATH, tempPath);
		DWORD pid = GetCurrentProcessId();
		std::string modulePath = std::string(tempPath) + "Jenova.Loader." + std::to_string(pid) + ".jnv";
		std::ofstream outFile(modulePath, std::ios::binary);
		outFile.write(reinterpret_cast<const char*>(LIB_MEMORYMODULE_WIN64_SHELL), sizeof(LIB_MEMORYMODULE_WIN64_SHELL));
		outFile.close();

		// Load Module Loader Shell
		HMODULE memoryModuleShell = LoadLibraryA(modulePath.c_str());
		if (!memoryModuleShell) return false;

		// Solve Module Loader Shell Functions
		InitializeMemoryModuleLoader = (NTSTATUS(NTAPI*)())GetProcAddress(memoryModuleShell, "InitializeMemoryModuleLoader");
		if (!InitializeMemoryModuleLoader) return false;
		ReleaseMemoryModuleLoader = (NTSTATUS(NTAPI*)())GetProcAddress(memoryModuleShell, "ReleaseMemoryModuleLoader");
		if (!ReleaseMemoryModuleLoader) return false;
		SetAgressiveReleaseMode = (VOID(*)(bool))GetProcAddress(memoryModuleShell, "SetAgressiveReleaseMode");
		if (!SetAgressiveReleaseMode) return false;
		LoadLibraryMemory = (HMODULE(WINAPI*)(PVOID))GetProcAddress(memoryModuleShell, "LoadLibraryMemory");
		if (!LoadLibraryMemory) return false;
		LoadLibraryMemoryExA = (HMODULE(WINAPI*)(PVOID, size_t, LPCSTR, LPCSTR, DWORD))GetProcAddress(memoryModuleShell, "LoadLibraryMemoryExA");
		if (!LoadLibraryMemoryExA) return false;
		FreeLibraryMemory = (BOOL(WINAPI*)(HMODULE))GetProcAddress(memoryModuleShell, "FreeLibraryMemory");
		if (!FreeLibraryMemory) return false;

		// Initialize Module Loader Shell
		return ((InitializeMemoryModuleLoader()) >= 0);
	}
	static bool Release()
	{
		// Release Module Loader Shell
		NTSTATUS ntResult = ReleaseMemoryModuleLoader();
		bool result = (ntResult >= 0 || ntResult == STATUS_NOT_SUPPORTED);

		// Delete Temporary Module Loader Shell
		if (!QUERY_ENGINE_MODE(Editor))
		{
			char tempPath[MAX_PATH] = { 0 };
			GetTempPathA(MAX_PATH, tempPath);
			DWORD pid = GetCurrentProcessId();
			std::string modulePath = std::string(tempPath) + "Jenova.Loader." + std::to_string(pid) + ".jnv";
			FreeLibrary(GetModuleHandleA(modulePath.c_str()));
			std::filesystem::remove(modulePath);
		}

		// Return Result
		return result;
	}
	static bool SetAgressiveMode(bool agrState)
	{
		SetAgressiveReleaseMode(agrState);
		return true;
	}
	static jenova::ModuleHandle LoadModule(void* bufferPtr, size_t bufferSize, int flags = 0)
	{
		// Update Flags
		loaderFlags = flags;

		// Load From Memory
		return LoadLibraryMemory(bufferPtr);
	}
	static jenova::ModuleHandle LoadModuleAsVirtual(void* bufferPtr, size_t bufferSize, const char* moduleName, const char* modulePath, int flags = 0)
	{
		// Update Flags
		loaderFlags = flags;

		// If Debug Mode Required Load From Disk
		if ((loaderFlags & jenova::LoaderFlag::LoadInDebugMode) != 0)
		{
			return LoadLibraryA(jenova::CreateTemporaryModuleCache((uint8_t*)bufferPtr, bufferSize).c_str());
		}

		// Load From Memory
		return LoadLibraryMemoryExA(bufferPtr, bufferSize, moduleName, modulePath, LOAD_FLAGS_USE_DLL_NAME);
	}
	static jenova::ModuleAddress GetModuleBaseAddress(jenova::ModuleHandle moduleHandle)
	{
		return jenova::ModuleAddress(moduleHandle);
	}
	static void* GetVirtualFunction(jenova::ModuleHandle moduleHandle, const char* functionName)
	{
		return jenova::GetModuleFunction(moduleHandle, functionName);
	}
	static bool ReleaseModule(jenova::ModuleHandle moduleHandle)
	{
		return FreeLibraryMemory(HMODULE(moduleHandle));
	}

private:
	static inline jenova::LoaderFlags loaderFlags = 0;

private:
	static inline NTSTATUS(NTAPI*InitializeMemoryModuleLoader)() = nullptr;
	static inline NTSTATUS(NTAPI*ReleaseMemoryModuleLoader)() = nullptr;
	static inline VOID(*SetAgressiveReleaseMode)(bool) = nullptr;
	static inline HMODULE(WINAPI*LoadLibraryMemory)(PVOID) = nullptr;
	static inline HMODULE(WINAPI*LoadLibraryMemoryExA)(PVOID, size_t, LPCSTR, LPCSTR, DWORD) = nullptr;
	static inline BOOL(WINAPI*FreeLibraryMemory)(HMODULE) = nullptr;
};

#endif

// Linux Module Loader
#ifdef TARGET_PLATFORM_LINUX

// Loader Interface [Linux]
class JenovaLoader
{
public:
	static bool Initialize()
	{
		return true;
	}
	static bool Release()
	{
		return true;
	}
	static bool SetAgressiveMode(bool agrState)
	{
		aggressiveMode = agrState;
		return true;
	}
	static jenova::ModuleHandle LoadModule(void* bufferPtr, size_t bufferSize, int flags = 0)
	{
		// Generate Memory-Mapped File Name
		pid_t pid = getpid();
		char moduleName[64];
		snprintf((char*)moduleName, sizeof(moduleName), "jenova_module_%d", pid);

		// Create Memory-Mapped File
		int fd = memfd_create(moduleName, MFD_CLOEXEC | MFD_ALLOW_SEALING);
		if (fd == -1)
		{
			perror("[Jenova Loader] memfd_create failed.");
			return nullptr;
		}

		// Resize Memory-Mapped File
		if (ftruncate(fd, bufferSize) == -1)
		{
			perror("[Jenova Loader] ftruncate failed.");
			close(fd);
			return nullptr;
		}

		// Write Module to Memory-Mapped File
		ssize_t written = write(fd, bufferPtr, bufferSize);
		if (written != static_cast<ssize_t>(bufferSize))
		{
			perror("[Jenova Loader] write failed.");
			close(fd);
			return nullptr;
		}

		// Seal Memory-Mapped File
		if (fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE) == -1)
		{
			perror("[Jenova Loader] fcntl failed.");
			close(fd);
			return nullptr;
		}

		// Construct File Descriptor Path for Memory-Mapped File
		char fd_path[64];
		snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);

		// Load the Shared Object from Memory-Mapped File
		void* handle = dlopen(fd_path, RTLD_NOW | RTLD_GLOBAL | flags);
		if (!handle)
		{
			fprintf(stderr, "[Jenova Loader] dlopen failed: %s\n", dlerror());
			close(fd);
			return nullptr;
		}

		// Store Memory-Mapped File
		moduleFdMap[handle] = fd;

		// Return Loaded Module Handle
		return reinterpret_cast<jenova::ModuleHandle>(handle);
	}
	static jenova::ModuleHandle LoadModuleAsVirtual(void* bufferPtr, size_t bufferSize, const char* moduleName, const char* modulePath, int flags = 0)
	{
		return LoadModule(bufferPtr, bufferSize, flags);
	}
	static jenova::ModuleAddress GetModuleBaseAddress(jenova::ModuleHandle moduleHandle)
	{
		struct link_map* map;
		dlinfo(moduleHandle, RTLD_DI_LINKMAP, &map);
		return jenova::ModuleAddress(map->l_addr);
	}
	static void* GetVirtualFunction(jenova::ModuleHandle moduleHandle, const char* functionName)
	{
		return dlsym(moduleHandle, functionName);
	}
	static bool ReleaseModule(jenova::ModuleHandle moduleHandle)
	{
		// Validate
		if (!moduleHandle) return false;
		if (!aggressiveMode) return true;

		// Close Shared Object
		dlclose(moduleHandle);

		// Release Memory-Mapped File
		auto it = moduleFdMap.find(moduleHandle);
		if (it != moduleFdMap.end())
		{
			close(it->second);
			moduleFdMap.erase(it);
		}

		// All Good
		return true;
	}

private:
	static inline bool aggressiveMode = false;
	static inline std::unordered_map<jenova::ModuleHandle, int> moduleFdMap;
};

#endif