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

// Clektron Console Definition
class Console : public Node
{
	GDCLASS(Console, Node);

private:
	Ref<Resource> consoleScheme;
	void SetConsoleScheme(const Ref<Resource>& scheme) { consoleScheme = scheme; }
	Ref<Resource> GetConsoleScheme() const { return consoleScheme; }
	template <typename T> T GetConfiguration(const String& configName)
	{
		return T(consoleScheme->get(configName));
	}

protected:
	static void _bind_methods();

public:
	static void init();

public:
	Console() {};
	~Console() {};

public:
	// Routines
	void _enter_tree() override;
	void _exit_tree() override;
	void _ready() override;
	void _process(double _delta) override;

private:
	void InitializeConsole();
	bool ExecuteCommand(String consoleCommand);
	void ShowHideConsole(bool visible);
	String GetInitialText() const;

private:
	// Callbacks
	void SetConsoleState(bool state);
	void HandleConsoleInputEvent(const Ref<InputEvent>& inputEvent);
	void HandleConsoleInputChange();

public:
	// Accessors
	RichTextLabel* GetConsoleOutput() { return consoleOutput; };
	CodeEdit* GetConsoleInput() { return consoleInput; };
	Vector<String> GetConsoleHistory() { return consoleHistory; };

public:
	// Exposed API
	bool Execute(const String& command);
	void AddLog(const String& logMessage);
	void AddLog(const String& logMessage, Color logColor);
	void ThrowError(const String& errorMessage);
	void Flush();
	String GetData() const;

private:
	// Internal Data
	Vector<String> consoleHistory;
	int currentHistoryIndex			= 0;
	bool isConsoleVisible			= true;
	bool isConsoleBeingAnimated		= false;

private:
	// Internal Nodes
	Panel* consolePanel				= nullptr;
	RichTextLabel* consoleOutput	= nullptr;
	CodeEdit* consoleInput			= nullptr;
};
