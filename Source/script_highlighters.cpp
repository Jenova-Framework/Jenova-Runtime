
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

// Minimal C++ Lexer
#include "Lexers/Minilex.hpp"

// Initializer/Deinitializer
void CPPSyntaxHighlighter::init()
{
	// Register Class
	ClassDB::register_internal_class<CPPSyntaxHighlighter>();

	// Register Syntax Highlighter
	Ref<CPPSyntaxHighlighter> highlighter; highlighter.instantiate();
	EditorInterface::get_singleton()->get_script_editor()->register_syntax_highlighter(highlighter);
}
void CPPSyntaxHighlighter::deinit()
{
	// Unregister Syntax Highlighter
	Ref<CPPSyntaxHighlighter> highlighter; highlighter.instantiate();
	EditorInterface::get_singleton()->get_script_editor()->unregister_syntax_highlighter(highlighter);
}

// Jenova Editor Syntax Highlighter Implementation
String CPPSyntaxHighlighter::_get_name() const
{
	return CPPScriptLanguage::get_singleton()->_get_friendly_name();
}
PackedStringArray CPPSyntaxHighlighter::_get_supported_languages() const
{
	return PackedStringArray{ CPPScriptLanguage::get_singleton()->_get_name(), CPPHeaderLanguage::get_singleton()->_get_name() };
}
Ref<EditorSyntaxHighlighter> CPPSyntaxHighlighter::_create() const
{
	Ref<CPPSyntaxHighlighter> highlighter;
	highlighter.instantiate();
	return highlighter;
}
Dictionary CPPSyntaxHighlighter::_get_line_syntax_highlighting(int32_t p_line) const
{
	PerformHighlighting();
	if (p_line >= 0 && p_line < lexerCache.size()) return lexerCache[p_line];
	return Dictionary();
}
void CPPSyntaxHighlighter::_update_cache()
{
	PerformHighlighting();
}
void CPPSyntaxHighlighter::_clear_highlighting_cache()
{
	ResetHighlighting();
}
void CPPSyntaxHighlighter::PerformHighlighting() const
{
	// Get Script Text Editor
	TextEdit* textEdit = get_text_edit();
	if (!textEdit) return;

	// Check for Content Changes
	int64_t contentHash = textEdit->get_text().hash();
	if (contentHash == lastCacheHash) return;
	lastCacheHash = contentHash;

	// C++ Word Database
	PackedStringArray cppTypes =
	{
		"char", "char8_t", "char16_t", "char32_t", "double", "float", "int", "int8_t", "uint8_t", "int16_t", "int32_t", "int64_t", "uint16_t", "uint32_t", "uint64_t", 
		"long", "size_t", "short", "signed", "unsigned", "void", "wchar_t", "__int64", "__int32", "nullptr_t", "ptrdiff_t", "max_align_t", "byte", "json_t", "string",
		"vector", "array", "map", "set", "unordered_map", "unordered_set", "list", "deque", "queue", "stack", "pair", "tuple", "optional", "variant", "any", "unique_ptr", 
		"shared_ptr", "weak_ptr"
	};
	PackedStringArray cppKeywords =
	{
		"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "class", "compl", "concept",
		"const", "consteval", "constexpr", "const_cast", "co_await", "co_return", "co_yield", "decltype", "default", "delete", "new", "dynamic_cast", "enum", "explicit",
		"export", "extern", "false", "friend", "inline", "mutable", "namespace", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected",
		"public", "reflexpr", "register", "reinterpret_cast", "requires", "sizeof", "static", "static_assert", "static_cast", "struct", "synchronized", "template", "this",
		"thread_local", "throw", "true", "typedef", "typeid", "typename", "union", "using", "virtual", "volatile", "xor", "xor_eq", "include", "ifdef", "endif", "pragma", 
		"final", "override"
	};
	PackedStringArray cppControlFlow =
	{
		"break", "case", "catch", "continue", "default", "do", "else",
		"for", "goto", "if", "return", "switch", "throw", "try", "while",
		"co_await", "co_return", "co_yield"
	};
	PackedStringArray specialKeywords =
	{
		"jenova", "sdk", "godot", "std", "Ref",
		"Caller", "Variant", "JenovaSDK",
		"EngineMode", "FileSystemEvent", "RuntimeReloadMode", "RuntimeEvent", "ClassAccess",
		"FunctionPtr", "NativePtr", "IntPtr", "BufferPtr", "ObjectPtr", "StringPtr", "WideStringPtr",
		"ImageSize", "MemoryID", "VariableID", "TaskID", "DriverResourceID", "TaskFunction", "JenovaSDKInterface",
		"GetSelf", "GetNode", "FindNode", "GlobalPointer", "GlobalGet", "GlobalSet",
		"GlobalVariable", "GetObjectFromIntPtr", "Instantiate", "InstantiateAsRef",
		jenova::GlobalSettings::ScriptToolIdentifier,
		jenova::GlobalSettings::ScriptBlockBeginIdentifier,
		jenova::GlobalSettings::ScriptBlockEndIdentifier,
		jenova::GlobalSettings::ScriptVMBeginIdentifier,
		jenova::GlobalSettings::ScriptVMEndIdentifier,
		jenova::GlobalSettings::ScriptSignalCallbackIdentifier,
		jenova::GlobalSettings::ScriptPropertyIdentifier,
		jenova::GlobalSettings::ScriptClassNameIdentifier,
		jenova::GlobalSettings::ScriptActivatorIdentifier,
		jenova::GlobalSettings::ScriptFunctionExportIdentifier
	};

	// Clear Cache
	ResetHighlighting();

	// Prepare for Parse
	int lineCount = textEdit->get_line_count();
	lexerCache.resize(lineCount);
	Color defaultColor = textEdit->get_theme_color("font_color", "TextEdit");

	// Helpers
	auto processIdentifier = [&](const String& word, int tokenStart, int tokenLength, const std::string& currentLine, Color& currentColor) -> void
	{
		if (cppControlFlow.has(word)) currentColor = cpp_controlFlowColor;
		else if (cppKeywords.has(word))
		{
			if (word == "true" || word == "false") currentColor = cpp_booleanColor;
			else if (word == "nullptr") currentColor = cpp_pointerColor;
			else currentColor = cpp_keywordColor;
		}
		else if (cppTypes.has(word)) currentColor = cpp_typeColor;
		else if (specialKeywords.has(word)) currentColor = cpp_specialColor;
		else if (ClassDB::class_exists(word)) currentColor = cpp_classColor;
		else
		{
			int lookAhead = tokenStart + tokenLength;
			while (lookAhead < int(currentLine.length()) && is_space(currentLine[lookAhead])) lookAhead++;
			if (lookAhead < int(currentLine.length()) && currentLine[lookAhead] == '(') currentColor = cpp_functionColor;
			else currentColor = defaultColor;
		}
	};

	// Process All Lines
	for (int lineNum = 0; lineNum < lineCount; lineNum++)
	{
		String line = textEdit->get_line(lineNum);
		Dictionary colorMap;

		// Validate Line
		if (line.length() == 0)
		{
			lexerCache.set(lineNum, colorMap);
			continue;
		}

		// Handle Multi-Line Comment
		if (inMultiLineComment)
		{
			int commentEnd = line.find("*/");
			if (commentEnd != -1)
			{
				Dictionary infoPrimary;
				infoPrimary["color"] = cpp_commentColor;
				colorMap[0] = infoPrimary;
				inMultiLineComment = false;

				// Handle Remaining Line
				String remainingLine = line.substr(commentEnd + 2);
				if (remainingLine.length() > 0)
				{
					std::string cppRemaining = AS_STD_STRING(remainingLine);
					Lexer lexer(cppRemaining.c_str());
					Token token = lexer.next();
					Color previousColor = cpp_commentColor;

					// Handle Tokens
					while (token.kind() != Token::Kind::End)
					{
						Color currentColor = defaultColor;
						int tokenStart = token.lexeme().data() - cppRemaining.c_str();
						int adjustedStart = commentEnd + 2 + tokenStart;

						switch (token.kind())
						{
							case Token::Kind::Comment:
								currentColor = cpp_commentColor;
								break;
							case Token::Kind::DoubleQuote:
							case Token::Kind::SingleQuote:
								currentColor = cpp_stringColor;
								break;
							case Token::Kind::Number:
								currentColor = cpp_numberColor;
								break;
							case Token::Kind::Identifier:
								processIdentifier(String(std::string(token.lexeme()).c_str()), tokenStart, token.lexeme().length(), cppRemaining, currentColor);
								break;
							case Token::Kind::Asterisk:
								currentColor = cpp_pointerColor;
								break;
							default:
								currentColor = cpp_operatorColor;
								break;
						}

						// Apply Color if Changed
						if (currentColor != previousColor)
						{
							Dictionary infoSecondary;
							infoSecondary["color"] = currentColor;
							colorMap[adjustedStart] = infoSecondary;
							previousColor = currentColor;
						}
						token = lexer.next();
					}
				}
				lexerCache.set(lineNum, colorMap);
				continue;
			}
			else
			{
				// It's Fully Comment
				Dictionary info;
				info["color"] = cpp_commentColor;
				colorMap[0] = info;
				lexerCache.set(lineNum, colorMap);
				continue;
			}
		}

		// Prepare Lexer
		std::string cppLine = AS_STD_STRING(line);
		Lexer lexer(cppLine.c_str());
		Token token = lexer.next();
		Color previousColor = defaultColor;

		// Handle Tokens
		while (token.kind() != Token::Kind::End)
		{
			Color currentColor = defaultColor;
			int tokenStart = token.lexeme().data() - cppLine.c_str();
			int tokenLength = token.lexeme().length();

			if (token.kind() == Token::Kind::Hash)
			{
				currentColor = cpp_preprocessorColor;
				Dictionary infoPrimary;
				infoPrimary["color"] = currentColor;
				colorMap[tokenStart] = infoPrimary;
				previousColor = currentColor;
				int lookAhead = tokenStart + tokenLength;
				while (lookAhead < int(cppLine.length()))
				{
					if (cppLine[lookAhead] == '\n' || cppLine[lookAhead] == '\0') break;
					if (is_identifier_char(cppLine[lookAhead]))
					{
						int wordStart = lookAhead;
						while (lookAhead < int(cppLine.length()) && is_identifier_char(cppLine[lookAhead])) lookAhead++;

						Dictionary infoSecondary;
						infoSecondary["color"] = cpp_preprocessorColor;
						colorMap[wordStart] = infoSecondary;
						previousColor = cpp_preprocessorColor;
					}
					else lookAhead++;
				}
				while (token.kind() != Token::Kind::End) token = lexer.next();
				continue;
			}

			switch (token.kind())
			{
				case Token::Kind::Comment:
				{
					currentColor = cpp_commentColor;
					std::string commentText = std::string(token.lexeme());
					if (commentText.find("/*") != std::string::npos && commentText.find("*/") == std::string::npos) inMultiLineComment = true;
					break;
				}
				case Token::Kind::DoubleQuote:
				case Token::Kind::SingleQuote:
					currentColor = cpp_stringColor;
					break;
				case Token::Kind::Number:
					currentColor = cpp_numberColor;
					break;
				case Token::Kind::Identifier:
					processIdentifier(String(std::string(token.lexeme()).c_str()), tokenStart, tokenLength, cppLine, currentColor);
					break;
				case Token::Kind::Asterisk:
					currentColor = cpp_pointerColor;
					break;
				case Token::Kind::Minus:
					if (token.lexeme().length() > 1 && token.lexeme()[1] == '>') currentColor = cpp_operatorColor;
					else currentColor = cpp_operatorColor;
					break;
				default:
					currentColor = cpp_operatorColor;
					break;
			}

			// Apply Color if Changed
			if (currentColor != previousColor)
			{
				Dictionary info;
				info["color"] = currentColor;
				colorMap[tokenStart] = info;
				previousColor = currentColor;
			}

			token = lexer.next();
		}

		lexerCache.set(lineNum, colorMap);
	}
}
void CPPSyntaxHighlighter::ResetHighlighting() const
{
	lexerCache.clear();
	lastCacheHash = 0;
	inMultiLineComment = false;
}