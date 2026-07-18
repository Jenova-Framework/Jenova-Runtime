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
#include "Jenova.hpp""

// Jenova Script Syntax Highlighter Definition
class CPPSyntaxHighlighter : public EditorSyntaxHighlighter
{
	GDCLASS(CPPSyntaxHighlighter, EditorSyntaxHighlighter)

private:
	// Internal Values
	mutable Vector<Dictionary> lexerCache;
	mutable int64_t lastCacheHash = 0;
	mutable bool inMultiLineComment = false;

public:
    // Define C++ Highlighter Colors
	Color cpp_typeColor					= Color::html("#ffc9b4");
	Color cpp_keywordColor				= Color::html("#ffa978");
	Color cpp_controlFlowColor			= Color::html("#7cf0ff");
	Color cpp_classColor				= Color::html("#ffd78b");
	Color cpp_booleanColor				= Color::html("#4cb8e6");
	Color cpp_preprocessorColor			= Color::html("#beacff");
	Color cpp_stringColor				= Color::html("#ff3758");
	Color cpp_functionColor				= Color::html("#a3ff9b");
	Color cpp_numberColor				= Color::html("#ca88ff");
	Color cpp_pointerColor				= Color::html("#ffbf47");
	Color cpp_commentColor				= Color::html("#a6a6a6aa");
	Color cpp_operatorColor				= Color::html("#d1fff0");
	Color cpp_specialColor				= Color::html("#47ff6f");

protected:
	static void _bind_methods() {}

public:
	static void init();
	static void deinit();

	virtual String _get_name() const override;
	virtual PackedStringArray _get_supported_languages() const override;
	virtual Ref<EditorSyntaxHighlighter> _create() const override;
	virtual Dictionary _get_line_syntax_highlighting(int32_t p_line) const override;
	virtual void _clear_highlighting_cache() override;
	virtual void _update_cache() override;

private:
	// Internal Methods
	void PerformHighlighting() const;
	void ResetHighlighting() const;
};