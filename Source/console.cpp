
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

// Internal/Built-In Sources
#include "InternalSources.h"

// Macros
#define JENOCON_VERSION         0.46f
#define JENOCON_TOGGLE_ACTION   "ConsoleToggle"
#define VALIDATE_EXECUTOR		if (!executorConsole) return

// Internal Console Script Interface
namespace ConsoleInterface
{
	// Executor Context
	static Console* executorConsole = nullptr;

	// Internal API
	static void ClearOutput()
	{
		VALIDATE_EXECUTOR;
		executorConsole->GetConsoleOutput()->clear();
	}
	static void ClearHistory()
	{
		if (!executorConsole) return;
		executorConsole->GetConsoleHistory().clear();
		executorConsole->GetConsoleOutput()->append_text("\n> [color=#8699f7]Update[/color] : Console history cleared.");
	}
	static void PrintOutput(const char* format, ...)
	{
		VALIDATE_EXECUTOR;
		char buffer[2048] = { 0 };
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
		executorConsole->GetConsoleOutput()->append_text("\n> [color=#aaffee]Print[/color] : " + String(buffer));
	}
	static void PrintTime()
	{
		VALIDATE_EXECUTOR;
		int64_t time_msec = godot::Time::get_singleton()->get_ticks_msec();
		String engineTime = String::num(static_cast<double>(time_msec) / 1000.0);
		executorConsole->GetConsoleOutput()->append_text("\n> [color=#8699f7]Time[/color] : " + engineTime);
	}
	static void QuitProcess()
	{
		VALIDATE_EXECUTOR;
		jenova::GetSceneTree()->quit();
	}

	// Binding Generator
	static void CreateBindings(Ref<CodeHighlighter> highlighter)
	{
		// Add Custom Keyword Colors
		highlighter->add_keyword_color("clear", Color("#9080e4"));
		highlighter->add_keyword_color("ch", Color("#9080e4"));
		highlighter->add_keyword_color("print", Color("#aaffee"));
		highlighter->add_keyword_color("quit", Color("#ff3958"));
		highlighter->add_keyword_color("exit", Color("#ff3958"));

		// Bind Symbols
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::ClearOutput, "clear");
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::ClearHistory, "clear_history");
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::ClearHistory, "ch");
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::PrintOutput, "print", "void", 2, "String", "...");
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::PrintTime, "time");
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::QuitProcess, "quit");
		Clektron::get_singleton()->BindSymbol(ConsoleInterface::QuitProcess, "exit");
	}
}

// Console Configuration Implementation
class ConsoleScheme : public Resource
{
	GDCLASS(ConsoleScheme, Resource);

public:
    Key visibilityToggleKey		= Key::KEY_QUOTELEFT;
	bool hideConsoleByDefault	= true;
	float caretBlinkInterval	= 0.2f;
	float animationDuration		= 0.3f;
	float darkShadePower		= 0.6f;
	int consoleHeight			= 210;
	int outputFontSize			= 14;
	int inputFontSize			= 16;

protected:
    static void _bind_methods()
    {
		// Key Mapping Data
		static const char ToggleKeyMapData[407] = {
		0x4B, 0x45, 0x59, 0x5F, 0x54, 0x41, 0x42, 0x3A, 0x34, 0x31, 0x39, 0x34,
		0x33, 0x30, 0x36, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x42, 0x41, 0x43, 0x4B,
		0x54, 0x41, 0x42, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x30, 0x37, 0x2C,
		0x4B, 0x45, 0x59, 0x5F, 0x48, 0x4F, 0x4D, 0x45, 0x3A, 0x34, 0x31, 0x39,
		0x34, 0x33, 0x31, 0x37, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x45, 0x4E, 0x44,
		0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x31, 0x38, 0x2C, 0x4B, 0x45, 0x59,
		0x5F, 0x50, 0x41, 0x47, 0x45, 0x55, 0x50, 0x3A, 0x34, 0x31, 0x39, 0x34,
		0x33, 0x32, 0x33, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x50, 0x41, 0x47, 0x45,
		0x44, 0x4F, 0x57, 0x4E, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x32, 0x34,
		0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x42, 0x52, 0x41, 0x43, 0x4B, 0x45, 0x54,
		0x4C, 0x45, 0x46, 0x54, 0x3A, 0x39, 0x31, 0x2C, 0x4B, 0x45, 0x59, 0x5F,
		0x42, 0x41, 0x43, 0x4B, 0x53, 0x4C, 0x41, 0x53, 0x48, 0x3A, 0x39, 0x32,
		0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x42, 0x52, 0x41, 0x43, 0x4B, 0x45, 0x54,
		0x52, 0x49, 0x47, 0x48, 0x54, 0x3A, 0x39, 0x33, 0x2C, 0x4B, 0x45, 0x59,
		0x5F, 0x53, 0x4C, 0x41, 0x53, 0x48, 0x3A, 0x34, 0x37, 0x2C, 0x4B, 0x45,
		0x59, 0x5F, 0x50, 0x4C, 0x55, 0x53, 0x3A, 0x34, 0x33, 0x2C, 0x4B, 0x45,
		0x59, 0x5F, 0x55, 0x4E, 0x44, 0x45, 0x52, 0x53, 0x43, 0x4F, 0x52, 0x45,
		0x3A, 0x39, 0x35, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x51, 0x55, 0x4F, 0x54,
		0x45, 0x4C, 0x45, 0x46, 0x54, 0x3A, 0x39, 0x36, 0x2C, 0x4B, 0x45, 0x59,
		0x5F, 0x46, 0x31, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x33, 0x32, 0x2C,
		0x4B, 0x45, 0x59, 0x5F, 0x46, 0x32, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33,
		0x33, 0x33, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x46, 0x33, 0x3A, 0x34, 0x31,
		0x39, 0x34, 0x33, 0x33, 0x34, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x46, 0x34,
		0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x33, 0x35, 0x2C, 0x4B, 0x45, 0x59,
		0x5F, 0x46, 0x35, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x33, 0x36, 0x2C,
		0x4B, 0x45, 0x59, 0x5F, 0x46, 0x36, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33,
		0x33, 0x37, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x46, 0x37, 0x3A, 0x34, 0x31,
		0x39, 0x34, 0x33, 0x33, 0x38, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x46, 0x38,
		0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x33, 0x39, 0x2C, 0x4B, 0x45, 0x59,
		0x5F, 0x46, 0x39, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x34, 0x30, 0x2C,
		0x4B, 0x45, 0x59, 0x5F, 0x46, 0x31, 0x30, 0x3A, 0x34, 0x31, 0x39, 0x34,
		0x33, 0x34, 0x31, 0x2C, 0x4B, 0x45, 0x59, 0x5F, 0x46, 0x31, 0x31, 0x3A,
		0x34, 0x31, 0x39, 0x34, 0x33, 0x34, 0x32, 0x2C, 0x4B, 0x45, 0x59, 0x5F,
		0x46, 0x31, 0x32, 0x3A, 0x34, 0x31, 0x39, 0x34, 0x33, 0x34, 0x33
		};

		// Bind Properties
        ClassDB::bind_method(D_METHOD("set_toggle_key", "key"), &ConsoleScheme::set_toggle_key);
        ClassDB::bind_method(D_METHOD("get_toggle_key"), &ConsoleScheme::get_toggle_key);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "toggle_key", PROPERTY_HINT_ENUM, ToggleKeyMapData), "set_toggle_key", "get_toggle_key");

        ClassDB::bind_method(D_METHOD("set_hide_console_by_default", "enabled"), &ConsoleScheme::set_hide_console_by_default);
        ClassDB::bind_method(D_METHOD("get_hide_console_by_default"), &ConsoleScheme::get_hide_console_by_default);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hide_console_by_default"), "set_hide_console_by_default", "get_hide_console_by_default");

        ClassDB::bind_method(D_METHOD("set_caret_blink_interval", "speed"), &ConsoleScheme::set_caret_blink_interval);
        ClassDB::bind_method(D_METHOD("get_caret_blink_interval"), &ConsoleScheme::get_caret_blink_interval);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "caret_blink_interval", PROPERTY_HINT_RANGE, "0.05,1"), "set_caret_blink_interval", "get_caret_blink_interval");

        ClassDB::bind_method(D_METHOD("set_animation_duration_ms", "speed"), &ConsoleScheme::set_animation_duration_ms);
        ClassDB::bind_method(D_METHOD("get_animation_duration_ms"), &ConsoleScheme::get_animation_duration_ms);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "animation_duration_ms", PROPERTY_HINT_RANGE, "0.05,2"), "set_animation_duration_ms", "get_animation_duration_ms");

        ClassDB::bind_method(D_METHOD("set_dark_shade_power", "power"), &ConsoleScheme::set_dark_shade_power);
        ClassDB::bind_method(D_METHOD("get_dark_shade_power"), &ConsoleScheme::get_dark_shade_power);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dark_shade_power", PROPERTY_HINT_RANGE, "0.1,1.0"), "set_dark_shade_power", "get_dark_shade_power");

        ClassDB::bind_method(D_METHOD("set_console_height", "size"), &ConsoleScheme::set_console_height);
        ClassDB::bind_method(D_METHOD("get_console_height"), &ConsoleScheme::get_console_height);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "console_height", PROPERTY_HINT_RANGE, "64,1440"), "set_console_height", "get_console_height");

		ClassDB::bind_method(D_METHOD("set_output_font_size", "size"), &ConsoleScheme::set_output_font_size);
		ClassDB::bind_method(D_METHOD("get_output_font_size"), &ConsoleScheme::get_output_font_size);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "output_font_size", PROPERTY_HINT_RANGE, "5,50"), "set_output_font_size", "get_output_font_size");

		ClassDB::bind_method(D_METHOD("set_input_font_size", "size"), &ConsoleScheme::set_input_font_size);
		ClassDB::bind_method(D_METHOD("get_input_font_size"), &ConsoleScheme::get_input_font_size);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "input_font_size", PROPERTY_HINT_RANGE, "5,30"), "set_input_font_size", "get_input_font_size");
    }

public:
    // Setters/Getters
    void set_toggle_key(const int key) { visibilityToggleKey = Key(key); }
    int get_toggle_key() const { return visibilityToggleKey; }
    void set_hide_console_by_default(bool enabled) { hideConsoleByDefault = enabled; }
    bool get_hide_console_by_default() const { return hideConsoleByDefault; }
    void set_caret_blink_interval(float speed) { caretBlinkInterval = speed; }
    float get_caret_blink_interval() const { return caretBlinkInterval; }
    void set_animation_duration_ms(float speed) { animationDuration = speed; }
    float get_animation_duration_ms() const { return animationDuration; }
    void set_dark_shade_power(float power) { darkShadePower = power; }
    float get_dark_shade_power() const { return darkShadePower; }
	void set_console_height(float power) { consoleHeight = power; }
	float get_console_height() const { return consoleHeight; }
	void set_output_font_size(int size) { outputFontSize = size; }
	int get_output_font_size() const { return outputFontSize; }
	void set_input_font_size(int size) { inputFontSize = size; }
	int get_input_font_size() const { return inputFontSize; }
};

// Initializer
void Console::init()
{
	// Register Classes
	ClassDB::register_class<ConsoleScheme>();
	ClassDB::register_class<Console>();

	// Verbose
	jenova::Verbose("Clektron Console (Jenocon) Initialized.");
}

// Bindings
void Console::_bind_methods()
{
	// Bind Console API Functions
	ClassDB::bind_method(D_METHOD("execute", "command"), &Console::Execute);
	ClassDB::bind_method(D_METHOD("log", "message"), static_cast<void (Console::*)(const String&)>(&Console::AddLog));
	ClassDB::bind_method(D_METHOD("logc", "message", "color"), static_cast<void (Console::*)(const String&, Color)>(&Console::AddLog));
	ClassDB::bind_method(D_METHOD("error", "message"), &Console::ThrowError);
	ClassDB::bind_method(D_METHOD("flush"), &Console::Flush);
	ClassDB::bind_method(D_METHOD("get_data"), &Console::GetData);

    // Bind Console Scheme Resource
    ClassDB::bind_method(D_METHOD("SetConsoleScheme", "consoleScheme"), &Console::SetConsoleScheme);
    ClassDB::bind_method(D_METHOD("GetConsoleScheme"), &Console::GetConsoleScheme);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "consoleScheme", PROPERTY_HINT_RESOURCE_TYPE, "ConsoleScheme"), "SetConsoleScheme", "GetConsoleScheme");
}

// Clektron Console Routines
void Console::_enter_tree()
{
    // Validate Engine Mode
    if (QUERY_ENGINE_MODE(Editor)) return;

    // Create Console Toggle Action
    if (!InputMap::get_singleton()->has_action(JENOCON_TOGGLE_ACTION)) InputMap::get_singleton()->add_action(JENOCON_TOGGLE_ACTION);

	// If Has No Configuration, Create Default
	if (consoleScheme.is_null())
	{
		Ref<ConsoleScheme> newScheme = Object::cast_to<ConsoleScheme>(ClassDB::instantiate("ConsoleScheme"));
		consoleScheme = newScheme;
	}

    // Add Console Toggle Action Event
    Ref<InputEventKey> actionEvent;
    actionEvent.instantiate();
    actionEvent->set_keycode(Key(GetConfiguration<int>("toggle_key")));
    InputMap::get_singleton()->action_add_event(JENOCON_TOGGLE_ACTION, actionEvent);
}
void Console::_exit_tree()
{
}
void Console::_ready()
{
    // Validate Engine Mode
    if (QUERY_ENGINE_MODE(Editor)) return;
    
    // Initialize Console
	this->InitializeConsole();
}
void Console::_process(double _delta)
{
    // Validate Engine Mode
    if (QUERY_ENGINE_MODE(Editor)) return;

    // Handle Console Toggle
    if (Input::get_singleton()->is_action_just_pressed(JENOCON_TOGGLE_ACTION))
    {
        isConsoleVisible = !isConsoleVisible;
        ShowHideConsole(isConsoleVisible);
    }
}

// Clektron Console Implementation
void Console::InitializeConsole()
{
	// Generate Configuration
	float outputFontSize = GetConfiguration<int>("output_font_size");
	float inputFontSize = GetConfiguration<int>("input_font_size");

	// Create Resources :: Highlighter
	Ref<CodeHighlighter> highlighter = Object::cast_to<CodeHighlighter>(ClassDB::instantiate("ClektronHighlighter"));
	highlighter->set_name("ConsoleHighlighter");

	// Create Resources :: Empty Box
	Ref<StyleBoxEmpty> emptyBox;
	emptyBox.instantiate();

	// Create Resources :: Output Box
	Ref<StyleBoxFlat> outputBox;
	outputBox.instantiate();
	outputBox->set_bg_color(Color(0, 0, 0, 0.2));
	outputBox->set_content_margin(SIDE_LEFT, 8.0);
	outputBox->set_content_margin(SIDE_TOP, 4.0);
	outputBox->set_corner_radius(CORNER_TOP_LEFT, 3);
	outputBox->set_corner_radius(CORNER_TOP_RIGHT, 3);
	outputBox->set_corner_radius(CORNER_BOTTOM_RIGHT, 3);
	outputBox->set_corner_radius(CORNER_BOTTOM_LEFT, 3);
	outputBox->set_corner_detail(6);

	// Create Resources :: Input Box
	Ref<StyleBoxFlat> inputBox;
	inputBox.instantiate();
	inputBox->set_bg_color(Color(0, 0, 0, 0.3882353));
	inputBox->set_content_margin(SIDE_LEFT, 12.0);
	inputBox->set_border_color(Color(0.182, 0.91, 0.57026666, 1));
	inputBox->set_border_width(SIDE_LEFT, 3);
	inputBox->set_corner_radius(CORNER_TOP_LEFT, 3);
	inputBox->set_corner_radius(CORNER_TOP_RIGHT, 3);
	inputBox->set_corner_radius(CORNER_BOTTOM_RIGHT, 3);
	inputBox->set_corner_radius(CORNER_BOTTOM_LEFT, 3);
	inputBox->set_corner_detail(1);

	// Create Resources :: Console Font
	Ref<Font> consoleFont = jenova::CreateFontFileFromByteArray(BUFFER_PTR_SIZE_PARAM(JENOVA_RESOURCE(FONT_SPACEMONO_REGULAR)));

	// Create Resources :: Console Panel Shader
	using namespace jenova::resources;
	String panelShaderCode = String(JenovaConsolePanelShader).replace("123.456", String::num(1.0 - GetConfiguration<float>("dark_shade_power"), 2));
	Ref<ShaderMaterial> panelShaderMaterial = jenova::CreateShaderMaterialFromString(panelShaderCode);

	// Create User Interface :: Console Panel
	consolePanel = memnew(Panel);
	consolePanel->set_name("ConsolePanel");
	consolePanel->set_material(panelShaderMaterial);
	consolePanel->set_anchors_preset(Control::PRESET_TOP_WIDE);
	consolePanel->set_custom_minimum_size(Vector2(0, GetConfiguration<float>("console_height")));
	this->add_child(consolePanel);

	// Create User Interface :: Console Output
	consoleOutput = memnew(RichTextLabel);
	consoleOutput->set_name("ConsoleOutput");
	consoleOutput->set_anchors_preset(Control::PRESET_FULL_RECT);
	consoleOutput->set_offset(Side::SIDE_LEFT, 8.0);
	consoleOutput->set_offset(Side::SIDE_TOP, 8.0);
	consoleOutput->set_offset(Side::SIDE_RIGHT, -8.0);
	consoleOutput->set_offset(Side::SIDE_BOTTOM, -46.0);
	consoleOutput->add_theme_color_override("selection_color", Color("#2ebc78"));
	consoleOutput->add_theme_font_override("normal_font", consoleFont);
	consoleOutput->add_theme_font_override("mono_font", consoleFont);
	consoleOutput->add_theme_font_size_override("normal_font_size", outputFontSize);
	consoleOutput->add_theme_font_size_override("bold_font_size", outputFontSize);
	consoleOutput->add_theme_font_size_override("bold_italics_font_size", outputFontSize);
	consoleOutput->add_theme_font_size_override("italics_font_size", outputFontSize);
	consoleOutput->add_theme_font_size_override("mono_font_size", outputFontSize);
	consoleOutput->add_theme_stylebox_override("normal", outputBox);
	consoleOutput->add_theme_stylebox_override("focus", emptyBox);
	consoleOutput->set_use_bbcode(true);
	consoleOutput->set_scroll_follow(true);
	consoleOutput->set_text(GetInitialText());
	consolePanel->add_child(consoleOutput);

	// Create User Interface :: Console Input
	consoleInput = memnew(CodeEdit);
	consoleInput->set_name("ConsoleInput");
	consoleInput->set_anchors_preset(Control::PRESET_BOTTOM_WIDE);
	consoleInput->set_custom_minimum_size(Vector2(0, 32));
	consoleInput->set_offset(Side::SIDE_LEFT, 8.0);
	consoleInput->set_offset(Side::SIDE_TOP, -40.0);
	consoleInput->set_offset(Side::SIDE_RIGHT, -8.0);
	consoleInput->set_offset(Side::SIDE_BOTTOM, -8.0);
	consoleInput->add_theme_color_override("selection_color", Color("#2ebc78"));
	consoleInput->add_theme_color_override("caret_color", Color(0.4, 1.0, 0.7, 0.6));
	consoleInput->add_theme_constant_override("caret_width", 10);
	consoleInput->add_theme_font_override("font", consoleFont);
	consoleInput->add_theme_font_size_override("font_size", inputFontSize);
	consoleInput->add_theme_stylebox_override("normal", inputBox);
	consoleInput->add_theme_stylebox_override("focus", emptyBox);
	consoleInput->set_placeholder("> Enter Command...");
	consoleInput->set_caret_blink_enabled(true);
	consoleInput->set_draw_spaces(true);
	consoleInput->set_syntax_highlighter(highlighter);
	consoleInput->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_NONE);
	consoleInput->set_caret_blink_interval(GetConfiguration<float>("caret_blink_interval"));
	consoleInput->set_auto_brace_completion_enabled(true);
	consolePanel->add_child(consoleInput);

	// Connect Signals
	consoleInput->connect("gui_input", callable_mp(this, &Console::HandleConsoleInputEvent));
	consoleInput->connect("text_changed", callable_mp(this, &Console::HandleConsoleInputChange));

	// Apply Settings
	if (GetConfiguration<bool>("hide_console_by_default"))
	{
		consolePanel->set_offset(Side::SIDE_TOP, -consolePanel->get_size().y);
		isConsoleVisible = false;
	}

	// Bind Console API
	ConsoleInterface::CreateBindings(highlighter);

	// Update Highlighter Cache
	highlighter->update_cache();
}
bool Console::ExecuteCommand(String consoleCommand)
{
	// Add Command Log
	consoleOutput->append_text("\n> [color=#e19166]Command[/color] : ");
	consoleOutput->add_text(consoleCommand);

	// Add to History
	if (consoleHistory.is_empty()) consoleHistory.push_back(consoleCommand);
	else if (consoleHistory[consoleHistory.size() - 1] != consoleCommand) consoleHistory.push_back(consoleCommand);

	// Update Current History Index
	currentHistoryIndex = consoleHistory.size();

	// Prepare Command
	if (!consoleCommand.ends_with(")") && !consoleCommand.contains("();")) consoleCommand += "();";
	if (!consoleCommand.ends_with(";") && !consoleCommand.contains("();")) consoleCommand += ";";

	// Set Executor Instance
	ConsoleInterface::executorConsole = this;

	// Execute Command
	if (!Clektron::get_singleton()->ExecuteScript(consoleCommand, true))
	{
		this->ThrowError("Invalid Command! Check engine output for more information.");
		return false;
	}

	// All Good
	return true;
}
void Console::ShowHideConsole(bool visible)
{
	// Animate Console Visiblity
	if (!isConsoleBeingAnimated)
	{
		isConsoleBeingAnimated = true;
		if (visible)
		{
			consolePanel->set_visible(true);
			Ref<Tween> consoleAnimator = this->create_tween();
			consoleAnimator->tween_property(consolePanel, "offset_top", 0, GetConfiguration<float>("animation_duration_ms"));
			consoleAnimator->connect("finished", callable_mp(this, &Console::SetConsoleState).bind(true));
		}
		else
		{
			consoleInput->release_focus();
			Ref<Tween> consoleAnimator = this->create_tween();
			consoleAnimator->tween_property(consolePanel, "offset_top", -consolePanel->get_size().y, GetConfiguration<float>("animation_duration_ms"));
			consoleAnimator->connect("finished", callable_mp(this, &Console::SetConsoleState).bind(false));
		}
	}
}
String Console::GetInitialText() const
{
	std::string initialText = jenova::Format("[color=#2ebc78]Jenova Console v%1.2f Alpha Initialized.[/color]", JENOCON_VERSION);
	return String(initialText.c_str());
}
void Console::SetConsoleState(bool state)
{
	// Set Focus
	if (state)
	{
		consoleInput->grab_focus();
	}
	else
	{
		consoleInput->release_focus();
		consolePanel->set_visible(false);
	}

	// Update Flags
	isConsoleBeingAnimated = false;
}
void Console::HandleConsoleInputEvent(const Ref<InputEvent>& inputEvent)
{
	// Check Input Event Type
	if (inputEvent.is_valid() && inputEvent->get_class() == "InputEventKey")
	{
		// Get Input Event Key
		Ref<InputEventKey> keyEvent = inputEvent;

		// Handle Command Accept
		if (keyEvent->get_keycode() == Key::KEY_ENTER && keyEvent->is_released())
		{
			if (!consoleInput->get_text().is_empty())
			{
				ExecuteCommand(consoleInput->get_text());
				consoleInput->clear();
			}
		}

		// Handle Commands History
		if (keyEvent->get_keycode() == Key::KEY_UP && keyEvent->is_released())
		{
			if (consoleHistory.size() != 0)
			{
				if (currentHistoryIndex > 0)
				{
					currentHistoryIndex--;
					currentHistoryIndex = ::CLAMP(currentHistoryIndex, 0, consoleHistory.size());
					if (currentHistoryIndex == consoleHistory.size()) currentHistoryIndex = consoleHistory.size() - 1;
				}
				consoleInput->set_text(consoleHistory[currentHistoryIndex]);
				int caretPosition = consoleInput->get_text().length();
				consoleInput->set_caret_column(caretPosition);
			}
		}
		if (keyEvent->get_keycode() == Key::KEY_DOWN && keyEvent->is_released())
		{
			if (consoleHistory.size() != 0)
			{
				if (currentHistoryIndex < consoleHistory.size())
				{
					currentHistoryIndex++;
					currentHistoryIndex = ::CLAMP(currentHistoryIndex, 0, consoleHistory.size() - 1);
				}
				consoleInput->set_text(consoleHistory[currentHistoryIndex]);
				int caretPosition = consoleInput->get_text().length();
				consoleInput->set_caret_column(caretPosition);
			}
		}
	}
}
void Console::HandleConsoleInputChange()
{
	// Limit to Single Line
	if (consoleInput->get_line_count() > 1)
	{
		consoleInput->set_text(consoleInput->get_text().replace("\n", ""));
	}

	// Prevent Toggle Key Inserting Character
	if (GetConfiguration<int>("toggle_key") >= 0 && GetConfiguration<int>("toggle_key") <= 127)
	{
		String forbiddenChar = String::chr(GetConfiguration<int>("toggle_key"));
		if (consoleInput->get_text().contains(forbiddenChar))
		{
			int caretPosition = consoleInput->get_caret_column();
			consoleInput->set_text(consoleInput->get_text().replace(forbiddenChar, ""));
			consoleInput->set_caret_column(caretPosition == 0 ? 0 : caretPosition - 1);
		}
	}
}

// Clektron Console API Implementation
bool Console::Execute(const String& command)
{
	return this->ExecuteCommand(command);
}
void Console::AddLog(const String& logMessage)
{
	consoleOutput->append_text("\n> " + logMessage);
}
void Console::AddLog(const String& logMessage, Color logColor)
{
	consoleOutput->append_text("\n> [color=" + logColor.to_html() + "]Error : " + logMessage + "[/color]");
}
void Console::ThrowError(const String& errorMessage)
{
	consoleOutput->append_text("\n> [color=#f83760]Error : " + errorMessage + "[/color]");
}
void Console::Flush()
{
	consoleOutput->clear();
}
String Console::GetData() const
{
	return consoleOutput->get_text();
}