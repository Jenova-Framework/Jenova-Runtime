#pragma once
#include <Windows.h>

typedef HMODULE HMEMORYMODULE;

#define MemoryModuleToModule(_hMemoryModule_) (_hMemoryModule_)

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#define LOAD_FLAGS_NOT_MAP_DLL						0x10000000
#define LOAD_FLAGS_NOT_FAIL_IF_HANDLE_TLS			0x20000000
#define LOAD_FLAGS_PASS_IMAGE_CHECK					0x40000000
#define LOAD_FLAGS_NOT_ADD_INVERTED_FUNCTION		0x00000001
#define LOAD_FLAGS_NOT_USE_REFERENCE_COUNT			0x00000002
#define LOAD_FLAGS_USE_DLL_NAME						0x00000004
#define LOAD_FLAGS_NOT_HANDLE_TLS					0x00000008
#define LOAD_FLAGS_HOOK_DOT_NET						0x00000010

HMEMORYMODULE WINAPI LoadLibraryMemory(_In_ PVOID BufferAddress);

HMEMORYMODULE WINAPI LoadLibraryMemoryExA(
	_In_ PVOID BufferAddress,
	_In_ size_t Reserved,
	_In_opt_ LPCSTR DllBaseName,
	_In_opt_ LPCSTR DllFullName,
	_In_ DWORD Flags
);

HMEMORYMODULE WINAPI LoadLibraryMemoryExW(
	_In_ PVOID BufferAddress,
	_In_ size_t Reserved,
	_In_opt_ LPCWSTR DllBaseName,
	_In_opt_ LPCWSTR DllFullName,
	_In_ DWORD Flags
);

BOOL WINAPI FreeLibraryMemory(_In_ HMEMORYMODULE hMemoryModule);

NTSTATUS NTAPI InitializeMemoryModuleLoader();
NTSTATUS NTAPI ReleaseMemoryModuleLoader();
VOID SetAgressiveReleaseMode(bool arState);

#define NtLoadDllMemory						LdrLoadDllMemory
#define NtLoadDllMemoryExA					LdrLoadDllMemoryExA
#define NtLoadDllMemoryExW					LdrLoadDllMemoryExW
#define NtUnloadDllMemory					LdrUnloadDllMemory
#define NtUnloadDllMemoryAndExitThread		LdrUnloadDllMemoryAndExitThread
#define FreeLibraryMemoryAndExitThread		LdrUnloadDllMemoryAndExitThread
#define NtQuerySystemMemoryModuleFeatures	LdrQuerySystemMemoryModuleFeatures

#ifdef UNICODE
#define LdrLoadDllMemoryEx LdrLoadDllMemoryExW
#define LoadLibraryMemoryEx LoadLibraryMemoryExW
#else
#define LdrLoadDllMemoryEx LdrLoadDllMemoryExA
#define LoadLibraryMemoryEx LoadLibraryMemoryExA
#endif
#define NtLoadDllMemoryEx LdrLoadDllMemoryEx