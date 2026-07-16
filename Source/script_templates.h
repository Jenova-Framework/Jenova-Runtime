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

// Jenova Script Template Manager Difinition
class JenovaTemplateManager : public Object
{
	GDCLASS(JenovaTemplateManager, Object);

public:
	static JenovaTemplateManager* get_singleton();

protected:
	static void _bind_methods();

public:
	static void init();
	static void deinit();

public:
	bool RegisterNewGlobalScriptTemplate(const String& templateName, const String& templateCode, const String& templateDescription);
	bool RegisterNewClassScriptTemplate(const String& templateName, const String& className, const String& templateCode, const String& templateDescription);
	TypedArray<Dictionary> GetGlobalScriptTemplates();
	String GetGlobalScriptTemplateSource(const String& templateName);
	TypedArray<Dictionary> GetClassScriptTemplates(const String& className);
	String GetClassScriptTemplateSource(const String& templateName, const String& className);
	void ClearScriptTemplates();

public:
	bool InstallBuiltInScriptTemplates();
	bool UpdateScriptTemplates();
};