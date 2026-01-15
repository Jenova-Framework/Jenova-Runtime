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
class Console : public Control
{
	GDCLASS(Console, Control);

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
	Console();
	~Console() {};

public:
	// Routines
	void _enter_tree() override;
	void _exit_tree() override;
	void _ready() override;
	void _process(double _delta) override;

private:
	void InitializeConsole();
	void ExecuteCommand(String consoleCommand);
	void ShowHideConsole(bool visible);
	String GetInitialText() const;

private:
	// Callbacks
	void SetConsoleState(bool state);
	void HandleConsoleInputEvent(const Ref<InputEvent>& inputEvent);
	void HandleConsoleInputChange();

public:
	// Getters
	RichTextLabel* GetConsoleOutput() { return consoleOutput; };
	CodeEdit* GetConsoleInput() { return consoleInput; };

private:
	// Internal Data
	Vector<String> consoleHistory;
	int currentHistoryIndex = 0;
	bool isConsoleVisible = true;
	bool isConsoleBeingAnimated = false;

private:
	// Internal Nodes
	RichTextLabel* consoleOutput = nullptr;
	CodeEdit* consoleInput = nullptr;
};
