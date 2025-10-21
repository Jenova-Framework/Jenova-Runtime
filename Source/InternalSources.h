#pragma once
namespace jenova::resources {
static constexpr char JenovaModuleInitializerCPP[] = R"(

	// Imports
	#include <vector>
	#include <algorithm>

	// Godot SDK
	#include <Godot/godot.hpp>

	// Jenova SDK
	#ifndef JENOVA_SDK
	#include <JenovaSDK.h>
	#endif

	// Namespaces
	using namespace godot;
	using namespace jenova::sdk;

	// Dummy Callbacks
	void DummyCallback(ModuleInitializationLevel initLevel) {}

	// Jenova SDK Interface
	namespace jenova::sdk
	{
		/* Singleton Accessed From Scripts */
		JenovaSDK* bridge = nullptr;
	}

	// Extension Initialization Data
	struct InitializerData
	{
		GDExtensionInterfaceGetProcAddress		godotGetProcAddress;
		GDExtensionClassLibraryPtr				godotExtensionClassLibraryPtr;
		GDExtensionInitialization*				godotExtensionInitialization;
		jenova::sdk::JenovaSDKInterface			jenovaSDKInterface;
	};

	// Extension Initializer
	JENOVA_EXPORT GDExtensionBool InitializeJenovaModule(InitializerData* initializerData)
	{
		// Initialize Jenova SDK Interface
		jenova::sdk::bridge = (jenova::sdk::JenovaSDK*)initializerData->jenovaSDKInterface;

		// Initailize Object & Solve API Functions
		GDExtensionBinding::InitObject init_obj(initializerData->godotGetProcAddress, 
												initializerData->godotExtensionClassLibraryPtr, 
												initializerData->godotExtensionInitialization);
		init_obj.register_initializer(DummyCallback);
		init_obj.register_terminator(DummyCallback);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
		return init_obj.init();
	}

	// Autoload Event Storage
	static std::vector<FunctionPtr> bootFunctions;
	static std::vector<FunctionPtr> shutdownFunctions;
	static bool InsertEvent(std::vector<FunctionPtr>& list, FunctionPtr funcPtr, int index)
	{
		if (std::find(list.begin(), list.end(), funcPtr) != list.end()) return false;
		if (index < 0 || index >= static_cast<int>(list.size())) list.push_back(funcPtr);
		else list.insert(list.begin() + index, funcPtr);
		return true;
	}
	static bool RemoveEvent(std::vector<FunctionPtr>& list, FunctionPtr funcPtr)
	{
		auto it = std::find(list.begin(), list.end(), funcPtr);
		if (it == list.end()) return false;
		list.erase(it);
		return true;
	}

	// Autoload Event Manager
	bool jenova::sdk::RegisterBootEvent(FunctionPtr funcPtr, int index)
	{
		return InsertEvent(bootFunctions, funcPtr, index);
	}
	bool jenova::sdk::RegisterShutdownEvent(FunctionPtr funcPtr, int index)
	{
		return InsertEvent(shutdownFunctions, funcPtr, index);
	}
	bool jenova::sdk::UnregisterBootEvent(FunctionPtr funcPtr)
	{
		return RemoveEvent(bootFunctions, funcPtr);
	}
	bool jenova::sdk::UnregisterShutdownEvent(FunctionPtr funcPtr)
	{
		return RemoveEvent(shutdownFunctions, funcPtr);
	}

	// Internal Module Events
	JENOVA_EXPORT bool _JenovaBoot()
	{
		// Execute Registered Boot Functions
		for (void* event : bootFunctions) reinterpret_cast<void(*)()>(event)();

		// All Good
		return true;
	}
	JENOVA_EXPORT bool _JenovaShutdown()
	{
		// Execute Registered Shutdown Functions
		for (void* event : shutdownFunctions) reinterpret_cast<void(*)()>(event)();

		// All Good
		return true;
	}

)";
static constexpr char JenovaGitIngoreTemplate[] = "# Godot 4+ Jenova specific ignores\n.vs/\n.vscode/\n.idea/\n.godot/\n.jenova/\nJenova/\n/vc*.pdb\ncmake-build-*/\nCMakeFiles/\nCMakeCache.txt\nMakefile\ncmake_install.cmake\ncompile_commands.json\ncompile_flags.txt\n";
}