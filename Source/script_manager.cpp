
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

// Resources
#include "IconDatabase.h"
#include "FontDatabase.h"

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

	// Register Script Manager Window
	scriptManager->initialize_script_manager_window();
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
class ScriptManagerWindow : public Window
{
	GDCLASS(ScriptManagerWindow, Window);

	// Datatypes
	enum ScriptType
	{
		UNKNOWN,
		ACTIVE,
		INACTIVE,
		HYBRID,
		EXTENSION
	};
	struct RecordItemData
	{
		VBoxContainer* recordItem = nullptr;
		Color baseColor;
	};

private:
	// Storages 
	jenova::json_t profilerDatabase;
	std::unordered_map<std::string, FoldableContainer*> loadedScripts;
	std::unordered_map<std::string, std::unordered_map<std::string, RecordItemData>> recordItems;

	// Values
	int currentLoadedFrame = 0;
	int totalLoadedFrames = 0;
	double scaleFactor = 1.0;

	// Objects
	VBoxContainer* _ScriptList = nullptr;
	HBoxContainer* _Toolbar = nullptr;
	RichTextLabel* _FrameStatus = nullptr;
	Button* reloadProfilerCacheTool = nullptr;
	Button* profilerPreviousFrameTool = nullptr;
	Button* profilerNextFrameTool = nullptr;
	Button* foldAllScriptsTool = nullptr;

	// Resources
	Ref<Theme> editorTheme;
	Ref<StyleBoxFlat> scriptItemStyle;
	Ref<StyleBoxFlat> scriptItemHoverStyle;
	Ref<StyleBoxFlat> scriptItemFoldedStyle;
	Ref<StyleBoxFlat> scriptItemFoldedHoverStyle;
	Ref<StyleBoxFlat> scriptItemPanelStyle;
	Ref<StyleBoxFlat> profilerRecordsStyle;
	Ref<StyleBoxFlat> profilerRecordsHoverStyle;
	Ref<StyleBoxFlat> functionRecordStyle;
	Ref<StyleBoxFlat> stageRecordStyle;
	Ref<StyleBoxFlat> connectionCircleStyle;
	Ref<StyleBoxEmpty> panelEmptyStyle;
	Ref<ImageTexture> arrowOpenedImage;
	Ref<ImageTexture> arrowClosedImage;
	Ref<ImageTexture> connectionMiddleImage;
	Ref<ImageTexture> connectionLastImage;
	Ref<ImageTexture> reloadToolbarImage;
	Ref<ImageTexture> backwardFrameImage;
	Ref<ImageTexture> forwardFrameImage;
	Ref<ImageTexture> foldAllImage;
	Ref<Font> cascadiaFont;
	Ref<Font> spaceMonoRegularFont;
	Ref<Font> spaceMonoItalicFont;
	Ref<Font> spaceMonoBoldFont;
	Ref<Font> spaceMonoBoldItalicFont;

protected:
	static void _bind_methods() {}

public:
	// Constructor/Deconstructor
	ScriptManagerWindow()
	{
		currentInstance = this;
	}
	~ScriptManagerWindow()
	{
		currentInstance = nullptr;
	}

public:
	// Routines
	void _ready() override
	{
		// Initialize & Create UI
		InitializeResources();
		InitializeUserInterface();
		IntializeToolbarButtons();

		// Update Script Items
		UpdateScriptItems();

		// Load Profiler Database If Exist
		ReloadProfilerDatabase();
	}
	void _exit_tree() override
	{
		// Release Resource
		ReleaseResources();
	}

private:
	// Signals
	bool MoveProfilerFrame(int offset)
	{
		if (totalLoadedFrames == 0) return true;
		if (Input::get_singleton()->is_key_pressed(KEY_SHIFT)) offset *= 10;
		int newFrameIndex = ::CLAMP(currentLoadedFrame + (offset), 0, totalLoadedFrames - 1);
		if (newFrameIndex == currentLoadedFrame) return true;
		currentLoadedFrame = newFrameIndex;
		ClearProfilerRecords();
		return LoadProfilerFrame(currentLoadedFrame);
	}
	void ReloadProfilerDatabase()
	{
		ClearProfilerRecords();
		String reportPath = jenova::GetJenovaCacheDirectory() + jenova::GlobalSettings::JenovaProfilerReportDatabaseFile;
		if (std::filesystem::exists(AS_STD_STRING(reportPath)))
		{
			if (!LoadProfilerDatbase(reportPath)) jenova::Error("Jenova Script Manager", "Failed to Load Profiler Configuration Cache.");
		}
	}
	void FoldAllScriptItems()
	{
		for (auto scriptItem : loadedScripts) scriptItem.second->fold();
	}

private:
	// Methods
	void InitializeResources()
	{
		// Obtain Editor Theme
		editorTheme = EditorInterface::get_singleton()->get_editor_theme();

		// Get Scale Factor
		scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

		// Create Colors
		Color accentColor = editorTheme->get_color("accent_color", "Editor");
		Color scriptItemFoldColor = Color(accentColor.r, accentColor.g, accentColor.b, 0.5);
		Color scriptItemFoldHoverColor = Color(accentColor.r, accentColor.g, accentColor.b, 0.75);

		// Create Style Box Resources
		scriptItemStyle.instantiate();
		scriptItemStyle->set_content_margin(Side::SIDE_TOP, 10.0);
		scriptItemStyle->set_content_margin(Side::SIDE_BOTTOM, 6.0);
		scriptItemStyle->set_bg_color(scriptItemFoldColor);
		scriptItemStyle->set_border_width(Side::SIDE_TOP, 4);
		scriptItemStyle->set_border_color(accentColor);
		scriptItemStyle->set_corner_detail(1);

		scriptItemHoverStyle.instantiate();
		scriptItemHoverStyle->set_content_margin(Side::SIDE_TOP, 10.0);
		scriptItemHoverStyle->set_content_margin(Side::SIDE_BOTTOM, 6.0);
		scriptItemHoverStyle->set_bg_color(scriptItemFoldHoverColor);
		scriptItemHoverStyle->set_border_width(Side::SIDE_TOP, 4);
		scriptItemHoverStyle->set_border_color(accentColor);
		scriptItemHoverStyle->set_corner_detail(1);

		scriptItemFoldedStyle.instantiate();
		scriptItemFoldedStyle->set_content_margin(Side::SIDE_TOP, 8.0);
		scriptItemFoldedStyle->set_content_margin(Side::SIDE_BOTTOM, 8.0);
		scriptItemFoldedStyle->set_bg_color(Color(0.0018, 0.0018, 0.0018, 0.74));
		scriptItemFoldedStyle->set_corner_detail(1);

		scriptItemFoldedHoverStyle.instantiate();
		scriptItemFoldedHoverStyle->set_content_margin(Side::SIDE_TOP, 8.0);
		scriptItemFoldedHoverStyle->set_content_margin(Side::SIDE_BOTTOM, 8.0);
		scriptItemFoldedHoverStyle->set_bg_color(Color(0.1477, 0.1477, 0.1477, 0.74));
		scriptItemFoldedHoverStyle->set_corner_detail(1);

		scriptItemPanelStyle.instantiate();
		scriptItemPanelStyle->set_content_margin(Side::SIDE_LEFT, 4.0);
		scriptItemPanelStyle->set_content_margin(Side::SIDE_TOP, 4.0);
		scriptItemPanelStyle->set_content_margin(Side::SIDE_RIGHT, 4.0);
		scriptItemPanelStyle->set_content_margin(Side::SIDE_BOTTOM, 4.0);
		scriptItemPanelStyle->set_bg_color(Color(0.3704, 0.3704, 0.3704, 0.298));
		scriptItemPanelStyle->set_corner_radius(Corner::CORNER_BOTTOM_RIGHT, 3);
		scriptItemPanelStyle->set_corner_radius(Corner::CORNER_BOTTOM_LEFT, 3);
		scriptItemPanelStyle->set_corner_detail(5);

		profilerRecordsStyle.instantiate();
		profilerRecordsStyle->set_content_margin(Side::SIDE_TOP, 5.0);
		profilerRecordsStyle->set_content_margin(Side::SIDE_BOTTOM, 5.0);
		profilerRecordsStyle->set_bg_color(Color(0, 0, 0, 0.3765));
		profilerRecordsStyle->set_border_width(Side::SIDE_LEFT, 8);
		profilerRecordsStyle->set_border_color(Color(0.0728, 0.0728, 0.0728, 1));

		profilerRecordsHoverStyle.instantiate();
		profilerRecordsHoverStyle->set_content_margin(Side::SIDE_TOP, 5.0);
		profilerRecordsHoverStyle->set_content_margin(Side::SIDE_BOTTOM, 5.0);
		profilerRecordsHoverStyle->set_bg_color(Color(0.2629, 0.2629, 0.2629, 0.3765));
		profilerRecordsHoverStyle->set_border_width(Side::SIDE_LEFT, 8);

		functionRecordStyle.instantiate();
		functionRecordStyle->set_content_margin(Side::SIDE_LEFT, 20.0);
		functionRecordStyle->set_content_margin(Side::SIDE_TOP, 4.0);
		functionRecordStyle->set_content_margin(Side::SIDE_RIGHT, 4.0);
		functionRecordStyle->set_content_margin(Side::SIDE_BOTTOM, 8.0);
		functionRecordStyle->set_bg_color(Color(0, 0, 0, 0.82));
		functionRecordStyle->set_border_width(Side::SIDE_LEFT, 8);
		functionRecordStyle->set_border_color(Color(1, 1, 1, 1));
		functionRecordStyle->set_corner_detail(1);

		stageRecordStyle.instantiate();
		stageRecordStyle->set_content_margin(Side::SIDE_LEFT, SCALED(20.0));
		stageRecordStyle->set_content_margin(Side::SIDE_TOP, SCALED(4.0));
		stageRecordStyle->set_content_margin(Side::SIDE_RIGHT, SCALED(4.0));
		stageRecordStyle->set_content_margin(Side::SIDE_BOTTOM, SCALED(8.0));
		stageRecordStyle->set_bg_color(Color(0, 0, 0, 0.698));
		stageRecordStyle->set_border_width(Side::SIDE_LEFT, 8);
		stageRecordStyle->set_border_color(Color(1, 1, 1, 0.4));
		stageRecordStyle->set_corner_detail(1);

		connectionCircleStyle.instantiate();
		connectionCircleStyle->set_bg_color(Color("#ffffff"));
		connectionCircleStyle->set_corner_radius(Corner::CORNER_TOP_LEFT, 5);
		connectionCircleStyle->set_corner_radius(Corner::CORNER_TOP_RIGHT, 5);
		connectionCircleStyle->set_corner_radius(Corner::CORNER_BOTTOM_RIGHT, 5);
		connectionCircleStyle->set_corner_radius(Corner::CORNER_BOTTOM_LEFT, 5);

		panelEmptyStyle.instantiate();

		// Create Image Resources
		arrowOpenedImage = MAKE_IMAGE_FROM_BUFFER(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_FOLD_OPENED_ARROW_ICON)));
		arrowClosedImage = MAKE_IMAGE_FROM_BUFFER(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_FOLD_CLOSED_ARROW_ICON)));
		connectionMiddleImage = MAKE_IMAGE_FROM_BUFFER(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_CONNECTION_BEGIN_ICON)));
		connectionLastImage = MAKE_IMAGE_FROM_BUFFER(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_CONNECTION_END_ICON)));

		// Create Toolbar Icon Image Resources
		reloadToolbarImage = jenova::CreateMenuItemIconFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_RELOAD_ICON)));
		backwardFrameImage = jenova::CreateMenuItemIconFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_BACKWARD_FRAME_ICON)));
		forwardFrameImage = jenova::CreateMenuItemIconFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_FORWARD_FRAME_ICON)));
		foldAllImage = jenova::CreateMenuItemIconFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(SVG_FOLD_ALL_ICON)));

		// Create Font Resources
		cascadiaFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_CASCADIACODE_REGULAR)));
		spaceMonoRegularFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_SPACEMONO_REGULAR)));
		spaceMonoItalicFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_SPACEMONO_ITALIC)));
		spaceMonoBoldFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_SPACEMONO_BOLD)));
		spaceMonoBoldItalicFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_SPACEMONO_BOLD_ITALIC)));
	}
	void InitializeUserInterface()
	{
		ColorRect* _Background = memnew(ColorRect);
		_Background->set_anchors_preset(Control::PRESET_FULL_RECT);
		_Background->set_color(editorTheme->get_color("dark_color_1", "Editor"));

		Panel* _Stage = memnew(Panel);
		_Stage->set_anchors_preset(Control::PRESET_FULL_RECT);
		_Stage->add_theme_stylebox_override("panel", panelEmptyStyle);
		_Stage->set_anchor(Side::SIDE_RIGHT, 1.0);
		_Stage->set_anchor(Side::SIDE_BOTTOM, 1.0);
		_Stage->set_offset(Side::SIDE_LEFT, SCALED(10.0));
		_Stage->set_offset(Side::SIDE_TOP, SCALED(10.0));
		_Stage->set_offset(Side::SIDE_RIGHT, SCALED(-10.0));
		_Stage->set_offset(Side::SIDE_BOTTOM, SCALED(-10.0));
		_Background->add_child(_Stage);

		_Toolbar = memnew(HBoxContainer);
		_Toolbar->set_custom_minimum_size(Vector2(0, SCALED(32)));
		_Toolbar->set_anchors_preset(Control::PRESET_TOP_WIDE);
		_Toolbar->set_anchor(Side::SIDE_RIGHT, 1.0);
		_Toolbar->set_offset(Side::SIDE_BOTTOM, SCALED(32.0));
		_Toolbar->set_h_grow_direction(Control::GROW_DIRECTION_BOTH);
		_Stage->add_child(_Toolbar);

		_FrameStatus = memnew(RichTextLabel);
		_FrameStatus->set_name("FrameStatus");
		_FrameStatus->set_text("Frame 0/0");
		_FrameStatus->set_anchors_preset(Control::PRESET_TOP_RIGHT);
		_FrameStatus->set_offset(Side::SIDE_TOP, SCALED(10.0));
		_FrameStatus->set_offset(Side::SIDE_RIGHT, SCALED(-10.0));
		_FrameStatus->set_autowrap_mode(TextServer::AutowrapMode::AUTOWRAP_OFF);
		_FrameStatus->set_fit_content(true);
		_FrameStatus->set_h_grow_direction(Control::GROW_DIRECTION_BEGIN);
		_FrameStatus->set_theme(nullptr);
		_FrameStatus->add_theme_stylebox_override("normal", panelEmptyStyle);
		_Stage->add_child(_FrameStatus);

		Panel* _ManagerArea = memnew(Panel);
		_ManagerArea->set_anchors_preset(Control::PRESET_FULL_RECT);
		_ManagerArea->set_anchor(Side::SIDE_RIGHT, 1.0);
		_ManagerArea->set_anchor(Side::SIDE_BOTTOM, 1.0);
		_ManagerArea->set_offset(Side::SIDE_TOP, SCALED(32.0));
		_Stage->add_child(_ManagerArea);

		ScrollContainer* _ScriptListView = memnew(ScrollContainer);
		_ScriptListView->set_anchors_preset(Control::PRESET_FULL_RECT);
		_ScriptListView->set_anchor(Side::SIDE_RIGHT, 1.0);
		_ScriptListView->set_anchor(Side::SIDE_BOTTOM, 1.0);
		_ScriptListView->set_offset(Side::SIDE_TOP, SCALED(10.0));
		_ScriptListView->set_offset(Side::SIDE_BOTTOM, SCALED(-10.0));
		_ManagerArea->add_child(_ScriptListView);

		_ScriptList = memnew(VBoxContainer);
		_ScriptList->set_h_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
		_ScriptListView->add_child(_ScriptList);

		this->add_child(_Background);
	}
	void IntializeToolbarButtons()
	{
		// Create Toolbar Items
		reloadProfilerCacheTool = CreateNewToolbarItem("ReloadDatabse", reloadToolbarImage, "Reload Profiler Database", _Toolbar);
		profilerPreviousFrameTool = CreateNewToolbarItem("FrameBackward", backwardFrameImage, "Back to Previous Frame (Hold Shift to 10x)", _Toolbar);
		profilerNextFrameTool = CreateNewToolbarItem("FrameForward", forwardFrameImage, "Forward to Next Frame (Hold Shift to 10x)", _Toolbar);
		CreateNewToolbarSeparator(_Toolbar);
		foldAllScriptsTool = CreateNewToolbarItem("FoldAllScriptItems", foldAllImage, "Fold All Script Items", _Toolbar);

		// Assign Signals
		reloadProfilerCacheTool->connect("pressed", callable_mp(this, &ScriptManagerWindow::ReloadProfilerDatabase));
		profilerPreviousFrameTool->connect("pressed", callable_mp(this, &ScriptManagerWindow::MoveProfilerFrame).bind(-1));
		profilerNextFrameTool->connect("pressed", callable_mp(this, &ScriptManagerWindow::MoveProfilerFrame).bind(1));
		foldAllScriptsTool->connect("pressed", callable_mp(this, &ScriptManagerWindow::FoldAllScriptItems));
	}
	void ReleaseResources()
	{
		// Clean Up
		loadedScripts.clear();
	}
	bool UpdateScriptItems()
	{
		// Collect C++ Scripts
		std::unordered_map<std::string, Ref<CPPScript>> usedScripts;
		for (size_t i = 0; i < ScriptManager::get_singleton()->get_script_object_count(); i++)
		{
			Ref<CPPScript> scriptObject = ScriptManager::get_singleton()->get_script_object(i);
			usedScripts.insert(std::make_pair(AS_STD_STRING(scriptObject->GetScriptIdentity()), scriptObject));
		}
		jenova::ResourceCollection cppResources;
		if (!jenova::CollectScriptsFromFileSystemAndScenes("res://", "cpp", cppResources))
		{
			jenova::Error("Jenova Script Manager", "Failed to Collect C++ Scripts from Project.");
			return false;
		};
		if (cppResources.size() == 0) return false;
		for (const auto& cppResource : cppResources)
		{
			if (cppResource->is_class(jenova::GlobalSettings::JenovaScriptType))
			{
				// Get C++ Script Object
				Ref<CPPScript> scriptResource = Object::cast_to<CPPScript>(cppResource.ptr());

				// Get Script Function and Property Count
				std::string scriptUID = AS_STD_STRING(scriptResource->GetScriptIdentity());
				int scriptFunctionCount = JenovaInterpreter::GetFunctionContainer(scriptUID).scriptFunctions.size();
				int scriptPropertyCount = JenovaInterpreter::GetPropertyContainer(scriptUID).scriptProperties.size();

				// Detect Script Type
				ScriptType scriptType = ScriptType::UNKNOWN;
				String scriptContent = scriptResource->get_source_code();
				bool isScriptActive = usedScripts.contains(AS_STD_STRING(scriptResource->GetScriptIdentity()));
				bool hasScriptBlock = scriptContent.contains("JENOVA_SCRIPT_BEGIN");
				bool hasExtension = scriptContent.contains("GDCLASS");
				if (isScriptActive && hasScriptBlock) scriptType = ScriptType::ACTIVE;
				if (!isScriptActive && hasScriptBlock) scriptType = ScriptType::INACTIVE;
				if (hasExtension) scriptType = ScriptType::EXTENSION;
				if (hasExtension && hasScriptBlock) scriptType = ScriptType::HYBRID;

				// Create New Script Item
				AddNewScriptItem(CreateNewScriptItem(scriptResource->get_path(), scriptType, scriptFunctionCount, scriptPropertyCount));
			}
		}

		// All Good
		return true;
	}
	bool LoadProfilerDatbase(String databaseFile)
	{
		try
		{
			// Parse Profiler Database
			profilerDatabase = jenova::json_t::parse(jenova::ReadStdStringFromFile(AS_STD_STRING(databaseFile)));

			// Set Total Frame Count
			totalLoadedFrames = profilerDatabase["RecordedFrames"].get<int>();

			// Load First Frame
			return LoadProfilerFrame(0);
		}
		catch (const std::exception& error)
		{
			return false;
		}
	}
	bool LoadProfilerFrame(int frameNumber)
	{
		try
		{
			// Reset Color Generator
			jenova::GenerateColorVariation(Color(), -1);

			// Add Execution Record Items
			jenova::json_t executionRecords = profilerDatabase["ExecutionFrameRecords"][frameNumber];
			for (const auto& executionRecord : executionRecords.items())
			{
				std::string scriptPath = executionRecord.key();
				if (loadedScripts.contains(scriptPath))
				{
					FoldableContainer* scriptItem = loadedScripts[scriptPath];
					for (const auto& functionRecord : executionRecord.value().items())
					{
						std::string functionName = functionRecord.key();
						double functionTime = functionRecord.value();
						Color colorBase = jenova::GenerateColorVariation(editorTheme->get_color("accent_color", "Editor"), totalLoadedFrames);
						VBoxContainer* newRecord = CreateNewRecordItem(String(functionName.c_str()), functionTime, colorBase);
						recordItems[scriptPath][functionName] = { newRecord, colorBase };
						AddNewRecordItem(scriptItem, newRecord);
					}
				}
			}

			// Add Stage Record items
			jenova::json_t stagesRecords = profilerDatabase["StageFrameRecords"][frameNumber];
			for (const auto& stagesRecord : stagesRecords.items())
			{
				std::string scriptPath = stagesRecord.key();
				if (loadedScripts.contains(scriptPath))
				{
					FoldableContainer* scriptItem = loadedScripts[scriptPath];
					auto stagesRecordItems = stagesRecord.value().items();
					for (const auto& stageRecord : stagesRecordItems)
					{
						std::string stageFullName = stageRecord.key();
						stageFullName = stageFullName.substr(stageFullName.find('$') + 1);
						double stageTime = stageRecord.value();
						size_t pos = stageFullName.find("::");
						if (pos != std::string::npos)
						{
							std::string functionName = stageFullName.substr(0, pos);
							std::string stageName = stageFullName.substr(pos + 2);

							if (recordItems.contains(scriptPath))
							{
								if (recordItems[scriptPath].contains(functionName))
								{
									bool isFirst = (stageRecord == stagesRecordItems.begin());
									bool isLast = (std::next(stageRecord) == stagesRecordItems.end());
									bool isMiddle = (!isLast && !isFirst);
									RecordItemData recordItem = recordItems[scriptPath][functionName];
									Panel* newStageRecord = CreateNewStageRecordItem(String(stageName.c_str()), stageTime, recordItem.baseColor, isLast, isMiddle);
									AddNewStageRecordItem(newStageRecord, recordItem.recordItem);
								}
							}
						}
					}
				}
			}

			// Update Status
			currentLoadedFrame = frameNumber;
			_FrameStatus->set_text(jenova::Format(String("Frame %d/%d"), currentLoadedFrame + 1, totalLoadedFrames));

			// All Good
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
	FoldableContainer* CreateNewScriptItem(String scriptPath, ScriptType scriptType, int funcCount, int propCount)
	{
		FoldableContainer* _ScriptItem = memnew(FoldableContainer);
		_ScriptItem->set_name("ScriptItem");
		_ScriptItem->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		_ScriptItem->add_theme_font_override("font", cascadiaFont);
		_ScriptItem->add_theme_icon_override("expanded_arrow", arrowOpenedImage);
		_ScriptItem->add_theme_icon_override("folded_arrow", arrowClosedImage);
		_ScriptItem->add_theme_stylebox_override("title_panel", scriptItemStyle);
		_ScriptItem->add_theme_stylebox_override("title_hover_panel", scriptItemHoverStyle);
		_ScriptItem->add_theme_stylebox_override("title_collapsed_panel", scriptItemFoldedStyle);
		_ScriptItem->add_theme_stylebox_override("title_collapsed_hover_panel", scriptItemFoldedHoverStyle);
		_ScriptItem->add_theme_stylebox_override("panel", scriptItemPanelStyle);
		_ScriptItem->set_folded(false);
		_ScriptItem->set_title(scriptPath.replace("res://", ""));
		_ScriptItem->set_title_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);

		VBoxContainer* _ScriptItemContent = memnew(VBoxContainer);
		_ScriptItemContent->set_name("ScriptItemContent");
		_ScriptItem->add_child(_ScriptItemContent);

		HBoxContainer* _ScriptDetails = memnew(HBoxContainer);
		_ScriptDetails->set_name("ScriptDetails");
		_ScriptDetails->set_custom_minimum_size(Vector2(0, SCALED(40)));
		_ScriptItemContent->add_child(_ScriptDetails);

		ColorRect* _RightSpace = memnew(ColorRect);
		_RightSpace->set_custom_minimum_size(Vector2(SCALED(15), 0));
		_RightSpace->set_color(Color(1, 1, 1, 0));
		_ScriptDetails->add_child(_RightSpace);

		RichTextLabel* _ScriptUID = memnew(RichTextLabel);
		_ScriptUID->set_name("ScriptPath");
		_ScriptUID->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		_ScriptUID->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
		_ScriptUID->add_theme_font_override("normal_font", spaceMonoRegularFont);
		_ScriptUID->add_theme_font_override("bold_font", spaceMonoBoldFont);
		_ScriptUID->add_theme_font_override("bold_italics_font", spaceMonoBoldItalicFont);
		_ScriptUID->add_theme_font_override("italics_font", spaceMonoItalicFont);
		_ScriptUID->add_theme_font_size_override("normal_font_size", SCALED(13));
		_ScriptUID->add_theme_stylebox_override("normal", panelEmptyStyle);
		_ScriptUID->set_use_bbcode(true);
		_ScriptUID->set_text("[b]UID[/b] : " + jenova::GenerateStandardUIDFromPath(scriptPath));
		_ScriptUID->set_fit_content(true);
		_ScriptUID->set_scroll_active(false);
		_ScriptUID->set_autowrap_mode(TextServer::AUTOWRAP_OFF);
		_ScriptDetails->add_child(_ScriptUID);

		RichTextLabel* _ScriptProperties = memnew(RichTextLabel);
		_ScriptProperties->set_name("ScriptProperties");
		_ScriptProperties->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		_ScriptProperties->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
		_ScriptProperties->add_theme_font_override("normal_font", spaceMonoRegularFont);
		_ScriptProperties->add_theme_font_override("bold_font", spaceMonoBoldFont);
		_ScriptProperties->add_theme_font_override("bold_italics_font", spaceMonoBoldItalicFont);
		_ScriptProperties->add_theme_font_override("italics_font", spaceMonoItalicFont);
		_ScriptProperties->add_theme_font_size_override("normal_font_size", SCALED(13));
		_ScriptProperties->add_theme_stylebox_override("normal", panelEmptyStyle);
		_ScriptProperties->set_use_bbcode(true);
		String scriptProperties = jenova::Format(String("[b]Type[/b] : %s        "), GetScriptTypeName(scriptType).c_str());
		scriptProperties += jenova::Format(String("[b]Functions[/b] : [color=gray]%d[/color]        "), funcCount);
		scriptProperties += jenova::Format(String("[b]Properties[/b] : [color=gray]%d[/color]"), propCount);
		_ScriptProperties->set_text(scriptProperties);
		_ScriptProperties->set_fit_content(true);
		_ScriptProperties->set_scroll_active(false);
		_ScriptProperties->set_autowrap_mode(TextServer::AUTOWRAP_OFF);
		_ScriptProperties->set_horizontal_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_RIGHT);
		_ScriptDetails->add_child(_ScriptProperties);

		ColorRect* _LeftSpace = memnew(ColorRect);
		_LeftSpace->set_custom_minimum_size(Vector2(SCALED(15), 0));
		_LeftSpace->set_color(Color(1, 1, 1, 0));
		_ScriptDetails->add_child(_LeftSpace);

		FoldableContainer* _ProfilerRecords = memnew(FoldableContainer);
		_ProfilerRecords->set_name("ProfilerRecords");
		_ProfilerRecords->set("theme_override_colors/font_color", Color(1, 1, 1, 0.7));
		_ProfilerRecords->add_theme_font_override("font", cascadiaFont);
		_ProfilerRecords->add_theme_font_size_override("font_size", SCALED(12));
		_ProfilerRecords->add_theme_icon_override("expanded_arrow", arrowOpenedImage);
		_ProfilerRecords->add_theme_icon_override("folded_arrow", arrowClosedImage);
		_ProfilerRecords->add_theme_stylebox_override("title_panel", profilerRecordsStyle);
		_ProfilerRecords->add_theme_stylebox_override("title_hover_panel", profilerRecordsHoverStyle);
		_ProfilerRecords->add_theme_stylebox_override("title_collapsed_panel", profilerRecordsStyle);
		_ProfilerRecords->add_theme_stylebox_override("title_collapsed_hover_panel", profilerRecordsHoverStyle);
		_ProfilerRecords->add_theme_stylebox_override("panel", panelEmptyStyle);
		_ProfilerRecords->set_folded(false);
		_ProfilerRecords->set_title("Profiler Records");
		_ProfilerRecords->set_title_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
		_ScriptItemContent->add_child(_ProfilerRecords);

		VBoxContainer* _ProfilerRecordsList = memnew(VBoxContainer);
		_ProfilerRecordsList->set_name("ProfilerRecordsList");
		_ProfilerRecordsList->add_theme_constant_override("separation", 0);
		_ProfilerRecords->add_child(_ProfilerRecordsList);

		loadedScripts[AS_STD_STRING(scriptPath)] = _ScriptItem;

		return _ScriptItem;
	}
	VBoxContainer* CreateNewRecordItem(String functionName, double functionRecord, Color baseColor) const
	{
		VBoxContainer* _RecordItem = memnew(VBoxContainer);
		_RecordItem->set_name("RecordItem");
		_RecordItem->add_theme_constant_override("separation", 0);

		Panel* _FunctionRecord = memnew(Panel);
		_FunctionRecord->set_name("FunctionRecord");
		_FunctionRecord->set_custom_minimum_size(Vector2(0, SCALED(35)));
		_FunctionRecord->add_theme_stylebox_override("panel", functionRecordStyle);
		_FunctionRecord->set_self_modulate(baseColor);
		_RecordItem->add_child(_FunctionRecord);

		Label* _Name = memnew(Label);
		_Name->set_anchors_preset(Control::PRESET_CENTER_LEFT);
		_Name->set_anchor(Side::SIDE_TOP, 0.5);
		_Name->set_anchor(Side::SIDE_BOTTOM, 0.5);
		_Name->set_offset(Side::SIDE_LEFT, SCALED(20.0));
		_Name->set_offset(Side::SIDE_TOP, SCALED(-12.0));
		_Name->set_offset(Side::SIDE_RIGHT, SCALED(101.0));
		_Name->set_offset(Side::SIDE_BOTTOM, SCALED(9.0));
		_Name->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
		_Name->add_theme_font_override("font", spaceMonoBoldItalicFont);
		_Name->add_theme_color_override("font_color", Color(1, 1, 1, 0.62));
		_Name->set_text(functionName);
		if (functionRecord == 0) _Name->set_self_modulate(Color(1, 1, 1, 0.4));
		_FunctionRecord->add_child(_Name);

		Label* _Duration = memnew(Label);
		_Duration->set_anchors_preset(Control::PRESET_CENTER_RIGHT);
		_Duration->set_anchor(Side::SIDE_LEFT, 1.0);
		_Duration->set_anchor(Side::SIDE_TOP, 0.5);
		_Duration->set_anchor(Side::SIDE_RIGHT, 1.0);
		_Duration->set_anchor(Side::SIDE_BOTTOM, 0.5);
		_Duration->set_offset(Side::SIDE_LEFT, SCALED(-67.0));
		_Duration->set_offset(Side::SIDE_TOP, SCALED(-11.5));
		_Duration->set_offset(Side::SIDE_RIGHT, SCALED(-16.0));
		_Duration->set_offset(Side::SIDE_BOTTOM, SCALED(11.5));
		_Duration->set_h_grow_direction(Control::GROW_DIRECTION_BEGIN);
		_Duration->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
		_Duration->add_theme_font_override("font", spaceMonoBoldFont);
		_Duration->set_text(jenova::Format(String("%.4f (ms)"), functionRecord));
		if (functionRecord == 0) _Duration->set_self_modulate(Color(1, 1, 1, 0.2));
		_Duration->set_horizontal_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_RIGHT);
		_FunctionRecord->add_child(_Duration);

		VBoxContainer* _StageRecordsList = memnew(VBoxContainer);
		_StageRecordsList->set_name("StageRecordsList");
		_StageRecordsList->add_theme_constant_override("separation", 0);
		_RecordItem->add_child(_StageRecordsList);

		return _RecordItem;
	}
	Panel* CreateNewStageRecordItem(String stageName, double stageTime, Color baseColor, bool lastItem = false, bool middleItem = false) const
	{
		Panel* _StageRecordItem = memnew(Panel);
		_StageRecordItem->set_name("StageRecordItem");
		_StageRecordItem->set_custom_minimum_size(Vector2(0, SCALED(35)));
		_StageRecordItem->add_theme_stylebox_override("panel", stageRecordStyle);
		_StageRecordItem->set_self_modulate(baseColor);

		Label* _Name = memnew(Label);
		_Name->set_name("Name");
		_Name->set_anchors_preset(Control::PRESET_CENTER_LEFT);
		_Name->set_anchor(Side::SIDE_TOP, 0.5);
		_Name->set_anchor(Side::SIDE_BOTTOM, 0.5);
		_Name->set_offset(Side::SIDE_LEFT, SCALED(65.0));
		_Name->set_offset(Side::SIDE_TOP, SCALED(-13.0));
		_Name->set_offset(Side::SIDE_RIGHT, SCALED(291.0));
		_Name->set_offset(Side::SIDE_BOTTOM, SCALED(11.0));
		_Name->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
		_Name->add_theme_color_override("font_color", Color(1, 1, 1, 0.60));
		_Name->add_theme_font_override("font", spaceMonoItalicFont);
		_Name->add_theme_font_size_override("font_size", SCALED(14));
		_Name->set_text(stageName);
		if (stageTime == 0) _Name->set_self_modulate(Color(1, 1, 1, 0.4));
		_Name->set_vertical_alignment(VerticalAlignment::VERTICAL_ALIGNMENT_CENTER);
		_StageRecordItem->add_child(_Name);

		Label* _Duration = memnew(Label);
		_Duration->set_name("Duration");
		_Duration->set_anchors_preset(Control::PRESET_CENTER_RIGHT);
		_Duration->set_anchor(Side::SIDE_LEFT, 1.0);
		_Duration->set_anchor(Side::SIDE_TOP, 0.5);
		_Duration->set_anchor(Side::SIDE_RIGHT, 1.0);
		_Duration->set_anchor(Side::SIDE_BOTTOM, 0.5);
		_Duration->set_offset(Side::SIDE_LEFT, SCALED(-67.0));
		_Duration->set_offset(Side::SIDE_TOP, SCALED(-11.5));
		_Duration->set_offset(Side::SIDE_RIGHT, SCALED(-16.0));
		_Duration->set_offset(Side::SIDE_BOTTOM, SCALED(11.5));
		_Duration->set_h_grow_direction(Control::GROW_DIRECTION_BEGIN);
		_Duration->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
		_Duration->add_theme_color_override("font_color", Color(1, 1, 1, 0.55));
		_Duration->add_theme_font_override("font", spaceMonoRegularFont);
		_Duration->set_text(jenova::Format(String("%.4f (ms)"), stageTime));
		if (stageTime == 0) _Duration->set_self_modulate(Color(1, 1, 1, 0.2));
		_Duration->set_horizontal_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_RIGHT);
		_StageRecordItem->add_child(_Duration);

		TextureRect* _Connection = memnew(TextureRect);
		_Connection->set_name("Connection");
		_Connection->set_offset(Side::SIDE_LEFT, SCALED(25.0));
		_Connection->set_offset(Side::SIDE_TOP, lastItem ? SCALED(-7.0) : SCALED(-4.0));
		_Connection->set_offset(Side::SIDE_RIGHT, SCALED(53.0));
		_Connection->set_offset(Side::SIDE_BOTTOM, lastItem ? SCALED(25.0) : SCALED(33.0));
		_Connection->set_texture(lastItem ? connectionLastImage : connectionMiddleImage);
		_Connection->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
		_StageRecordItem->add_child(_Connection);

		if (!middleItem)
		{
			Panel* _ConnectionCircle = memnew(Panel);
			_ConnectionCircle->set_name("ConnectionCircle");
			_ConnectionCircle->set_offset(Side::SIDE_LEFT, lastItem ? SCALED(48.0) : SCALED(27.0));
			_ConnectionCircle->set_offset(Side::SIDE_TOP, lastItem ? SCALED(17.0) : SCALED(-4.0));
			_ConnectionCircle->set_offset(Side::SIDE_RIGHT, lastItem ? SCALED(55.0) : SCALED(34.0));
			_ConnectionCircle->set_offset(Side::SIDE_BOTTOM, lastItem ? SCALED(25.0) : SCALED(3.0));
			_ConnectionCircle->add_theme_stylebox_override("panel", connectionCircleStyle);
			_ConnectionCircle->set_modulate(baseColor);
			_StageRecordItem->add_child(_ConnectionCircle);
		}

		return _StageRecordItem;
	}
	void AddNewScriptItem(Control* scriptItem)
	{
		_ScriptList->add_child(scriptItem);
	}
	void AddNewRecordItem(Control* scriptItem, Control* recordItem)
	{
		scriptItem->get_node<VBoxContainer>("ScriptItemContent/ProfilerRecords/ProfilerRecordsList")->add_child(recordItem);
	}
	void AddNewStageRecordItem(Control* stageRecordItem, Control* recordItem)
	{
		recordItem->get_node<VBoxContainer>("StageRecordsList")->add_child(stageRecordItem);
	}
	std::string GetScriptTypeName(ScriptType scriptType)
	{
		switch (scriptType)
		{
			case ScriptManagerWindow::ACTIVE:
				return "[color=green]Active[/color]";
			case ScriptManagerWindow::INACTIVE:
				return "[color=red]Passive[/color]";
			case ScriptManagerWindow::HYBRID:
				return "[color=orange]Hybrid[/color]";
			case ScriptManagerWindow::EXTENSION:
				return "[color=cyan]Extension[/color]";
			case ScriptManagerWindow::UNKNOWN:
			default:
				return "[color=gray]Unknown[/color]";
		}
	}
	Button* CreateNewToolbarItem(const String& toolName, const Ref<Texture2D>& toolIcon, const String& toolTip, Control* toolbar) const
	{
		Button* toolbar_item = memnew(Button);
		toolbar_item->set_name(toolName);
		toolbar_item->set_custom_minimum_size(Vector2(SCALED(32), SCALED(32)));
		toolbar_item->set_flat(false);
		toolbar_item->set_tooltip_text(toolTip);
		toolbar_item->set_theme_type_variation("RunBarButton");
		toolbar_item->add_theme_stylebox_override("focus", memnew(StyleBoxEmpty));
		toolbar_item->set_button_icon(toolIcon);
		toolbar_item->set_icon_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
		toolbar_item->add_theme_constant_override("icon_max_width", SCALED(20));
		toolbar->add_child(toolbar_item);
		return toolbar_item;
	}
	VSeparator* CreateNewToolbarSeparator(Control* toolbar) const
	{
		VSeparator* toolbar_separator = memnew(VSeparator);
		toolbar_separator->set_v_size_flags(Control::SizeFlags::SIZE_SHRINK_CENTER);
		toolbar_separator->set_custom_minimum_size(Vector2(1, SCALED(20)));
		toolbar_separator->add_theme_constant_override("separation", SCALED(2));
		toolbar->add_child(toolbar_separator);
		return toolbar_separator;
	}
	void ClearProfilerRecords()
	{
		for (auto& recordItem : recordItems) for (auto& recordItemData : recordItem.second) recordItemData.second.recordItem->queue_free();
		recordItems.clear();
	}

public:
	// Instance
	static inline ScriptManagerWindow* currentInstance = nullptr;
};

// Jenova Script Manager Window Creator
void JenovaScriptManager::initialize_script_manager_window()
{
	// Regsiter Only for Editor
	if (QUERY_ENGINE_MODE(Editor)) ClassDB::register_internal_class<ScriptManagerWindow>();
}
bool JenovaScriptManager::open_script_manager_window()
{
	// Get Scale Factor
	double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

	// Create A New Script Manager Window
	if (ScriptManagerWindow::currentInstance == nullptr)
	{
		ScriptManagerWindow* scriptManWindow = memnew(ScriptManagerWindow);
		if (!scriptManWindow) return false;
		scriptManWindow->set_name("JenovaScriptManager");
		scriptManWindow->set_title("Script Manager");
		scriptManWindow->set_size(Vector2(SCALED(780), 1200));
		scriptManWindow->set_min_size(Vector2i(SCALED(780), 1200));
		scriptManWindow->connect("close_requested", callable_mp((Node*)scriptManWindow, &Window::queue_free));

		// Show Script Manager Window
		scriptManWindow->hide();
		scriptManWindow->popup_exclusive_centered(EditorInterface::get_singleton()->get_base_control());
		scriptManWindow->set_owner(EditorInterface::get_singleton()->get_base_control()->get_window());
	}
	else
	{
		ScriptManagerWindow::currentInstance->grab_focus();
	}

	// All Good
	return true;
}