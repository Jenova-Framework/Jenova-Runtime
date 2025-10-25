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

// Jenova Package Manager Difinition
class JenovaPackageManager : public RefCounted
{
	GDCLASS(JenovaPackageManager, RefCounted);

public:
	static JenovaPackageManager* get_singleton();

protected:
	static void _bind_methods();

private:
	Timer* taskTimer				= nullptr;
	Window* currentWindow			= nullptr;
	GridContainer* browserContent	= nullptr;
	RichTextLabel* statusLabel		= nullptr;
	jenova::TaskID currentTaskID	= 0;
	int64_t currentTabID			= 0;
	std::string currentDatabaseURL	= "";
	bool isBusy						= false;

public:
	static void init();
	static void deinit();

public:
	bool OpenPackageManager(const String& packageDatabaseURL);
	jenova::PackageList GetInstalledPackages(const jenova::PackageType& packageType);

private:
	Button* CreateToolbarItem(const String& toolName, const Ref<Texture2D>& toolIcon, const String& toolTip, Control* toolbar);
	VSeparator* CreateToolbarSeparator(Control* toolbar);
	Panel* CreatePackageItem(const jenova::JenovaPackage& jenovaPackage, const Ref<Theme> itemTheme);
	void ReloadEntireDatabase();
	bool FetchOnlinePackages(const String& packageDatabaseURL);
	bool ObtainInstalledPackages();
	bool CacheInstalledPackages();
	jenova::JenovaPackage GetOnlinePackage(const String& packageHash);
	String GetPackagePlatformName(jenova::PackagePlatform pkgPlatform);
	bool ExtractPackage(const String& packageFile, const String& destinationPath);
	bool DownloadPackage(const String& packageFileURL, const String& downloadFilePath);
	bool InstallPackage(const String& packageHash);
	bool UninstallPackage(const String& packageHash);
	bool InstallCustomPackage(const jenova::CustomPackageInstallerMode& installerMode);
	bool RemoveUnusedPackages();
	void UtilizeNewPackageTask(const String& taskName, const String& packageHash, Button* utilizerButton = nullptr);
	bool UpdateStatus(const String& newStatus);
	void FormatStatus(const String& colorHash, const char* newStatus, ...);
	void UpdatePackageList(const jenova::PackageType& packageType);
	void ForceUpdatePackageList();
	void RequestEditorRestart();
	void SetBusy(bool busyState, jenova::TaskID taskID = 0);
	Window* GetWindow() { return currentWindow; }
	bool CanClose() const;
	bool PreparePackageManager();
	void PrepareForClose();

private:
	void OnTaskTimerTick();

private:
	static size_t OnDownloadBufferWrite(void* bufferPtr, size_t bufferSize, size_t newBufferSize, void* streamPtr);
	static int OnDownloadProgress(void* clientPtr, size_t downloadTotal, size_t downloadNow, size_t uploadTotal, size_t uploadNow);
	static size_t OnFetchBufferWrite(void* contents, size_t size, size_t nmemb, std::string* buffer);

private:
	static inline void* curlHandle = nullptr;
	static inline double downloadSpeed = 0;
};