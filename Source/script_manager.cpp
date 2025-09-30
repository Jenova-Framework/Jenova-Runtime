
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

// Singleton Instance
JenovaScriptManager* scriptManager = nullptr;

// Jenova Script Manager Implementation
JenovaScriptManager::JenovaScriptManager()
{
	scriptObjects.clear();
	scriptInstances.clear();
}
JenovaScriptManager::~JenovaScriptManager()
{

}
void JenovaScriptManager::_bind_methods()
{

}
void JenovaScriptManager::init()
{
	// Initialize Singleton
	scriptManager = memnew(JenovaScriptManager);
}
void JenovaScriptManager::deinit()
{
	// Release Singleton
	if (scriptManager) memdelete(scriptManager);
}
JenovaScriptManager* JenovaScriptManager::get_singleton()
{
	return scriptManager;
}
bool JenovaScriptManager::add_script_object(CPPScript* scriptObject)
{
	jenova::VerboseByID(__LINE__, "New Script Object Added to Manager : %p", scriptObject);
	scriptObjects.push_back(scriptObject);
	return true;
}
bool JenovaScriptManager::remove_script_object(CPPScript* scriptObject)
{
	jenova::VerboseByID(__LINE__, "Script Object Removed from Manager : %p", scriptObject);
	auto it = std::find(scriptObjects.begin(), scriptObjects.end(), scriptObject);
	if (it != scriptObjects.end()) 
	{
		scriptObjects.erase(it);
		return true;
	}
	return false;
}
size_t JenovaScriptManager::get_script_object_count()
{
	return scriptObjects.size();
}
Ref<CPPScript> JenovaScriptManager::get_script_object(size_t index)
{
	return Ref<CPPScript>(scriptObjects[index]);
}
bool JenovaScriptManager::add_script_instance(CPPScriptInstance* scriptInstance)
{
	// Rise Events
	if (JenovaScriptManager::IsScriptRuntimeStarted == false)
	{
		for (const auto& callBack : runtimeStartEvents) reinterpret_cast<void(*)()>(callBack)();
		JenovaScriptManager::IsScriptRuntimeStarted = true;
	}

	// Add Script Instance
	scriptInstances.push_back(scriptInstance);
	return true;
}
bool JenovaScriptManager::remove_script_instance(CPPScriptInstance* scriptInstance)
{
	auto it = std::find(scriptInstances.begin(), scriptInstances.end(), scriptInstance);
	if (it != scriptInstances.end()) 
	{
		scriptInstances.erase(it);
		return true;
	}
	return false;
}
size_t JenovaScriptManager::get_script_instance_count()
{
	return scriptInstances.size();
}
CPPScriptInstance* JenovaScriptManager::get_script_instance(size_t index)
{
	return scriptInstances[index];
}
bool JenovaScriptManager::register_script_runtime_start_event(jenova::VoidFunc_t callbackPtr)
{
	runtimeStartEvents.push_back(callbackPtr);
	return true;
}


// Jenova Script Manager Window Implementation
bool JenovaScriptManager::open_script_manager_window()
{
	// All Good
	return true;
}