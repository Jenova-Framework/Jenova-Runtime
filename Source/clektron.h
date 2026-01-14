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

// Jenova Clektron Script Engine Definition
class Clektron : public RefCounted
{
	GDCLASS(Clektron, RefCounted);

protected:
	static void _bind_methods();

public:
	static void init();
	static void deinit();
	static Clektron* get_singleton();

public:
	Clektron();
	~Clektron();

public:
	bool ExecuteScript(const std::string& ctronScriptContent, bool noEntrypoint = false);
	bool ExecuteScript(const godot::String& ctronScriptContent, bool noEntrypoint = false);
	bool ExecuteScriptFromFile(const std::string& ctronScriptFilePath, bool noEntrypoint = false);
	bool ExecuteScriptFromFile(const godot::String& ctronScriptFilePath, bool noEntrypoint = false);
	bool BindSymbol(void* symbolPtr, const std::string& symbolName, const std::string& returnType, std::vector<std::string>& parameters);
	bool BindSymbol(void* symbolPtr, const std::string& symbolName, const std::string& returnType, int parameterCount, ...);
};