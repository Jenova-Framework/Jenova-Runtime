
/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|                       Markova Fragment                       |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Markdown Parser
#include "Parsers/mumd.hpp"

// Markova Implementation
class Markova : public RichTextLabel
{
	GDCLASS(Markova, RichTextLabel);

	// Internal Types
	enum class ElementType
	{
		HEADER,
		PARAGRAPH,
		BOLD,
		ITALIC,
		STRIKETHROUGH,
		INLINE_CODE,
		LINK,
		IMAGE,
		LIST,
		ORDERED_LIST,
		LIST_ITEM,
		ORDERED_LIST_ITEM,
		BLOCKQUOTE,
		CODE_BLOCK,
		HORIZONTAL_RULE,
		TABLE,
		TABLE_ROW,
		TABLE_CELL,
		TEXT
	};
	struct Element
	{
		ElementType type;
		String content;
		int level = 0;
		std::vector<Element> children;
	};
	struct ParseState
	{
		std::vector<Element> elements;
		std::vector<Element*> stack;
		String textBuffer;
	};

private:
	// Properties
	Color inlineCode				= Color("#42f5a4ff");
	Color hyperLink					= Color("#22f5ccff");
	Color codeBlockBorder			= Color("#444444a1");
	Color codeBlockBackground		= Color("#1e1e1e8e");
	Color codeBlockText				= Color("#f28862ff");
	int codeBlockPadding			= 12;
	Color blockquoteBorder			= Color("#666666a5");
	Color blockquoteBackground		= Color("#2a2a2a9a");
	Color blockquoteText			= Color("#ccccccff");
	int blockquotePadding			= 10;
	Color tableBorder				= Color("#ffffff94");
	Color tableBackground			= Color("#222222ad");
	Color tableHeaderBackground		= Color("#444466e9");
	int tablePadding				= 8;

protected:
	static void _bind_methods()
	{
		// Function Bindings
		ClassDB::bind_method(D_METHOD("ParseMarkdown", "markdown"), &Markova::ParseMarkdown);
		ClassDB::bind_method(D_METHOD("GenerateMarkup"), &Markova::GenerateMarkup);
		ClassDB::bind_method(D_METHOD("DumpElementsCache"), &Markova::DumpElementsCache);

		// Properties Bindings
		ClassDB::bind_method(D_METHOD("set_inline_code_color", "color"), &Markova::set_inline_code_color);
		ClassDB::bind_method(D_METHOD("get_inline_code_color"), &Markova::get_inline_code_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "inline_code_color"), "set_inline_code_color", "get_inline_code_color");

		ClassDB::bind_method(D_METHOD("set_hyper_link_color", "color"), &Markova::set_hyper_link_color);
		ClassDB::bind_method(D_METHOD("get_hyper_link_color"), &Markova::get_hyper_link_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "hyper_link_color"), "set_hyper_link_color", "get_hyper_link_color");

		ClassDB::bind_method(D_METHOD("set_code_block_border_color", "color"), &Markova::set_code_block_border_color);
		ClassDB::bind_method(D_METHOD("get_code_block_border_color"), &Markova::get_code_block_border_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "code_block_border_color"), "set_code_block_border_color", "get_code_block_border_color");

		ClassDB::bind_method(D_METHOD("set_code_block_background_color", "color"), &Markova::set_code_block_background_color);
		ClassDB::bind_method(D_METHOD("get_code_block_background_color"), &Markova::get_code_block_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "code_block_background_color"), "set_code_block_background_color", "get_code_block_background_color");

		ClassDB::bind_method(D_METHOD("set_code_block_text_color", "color"), &Markova::set_code_block_text_color);
		ClassDB::bind_method(D_METHOD("get_code_block_text_color"), &Markova::get_code_block_text_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "code_block_text_color"), "set_code_block_text_color", "get_code_block_text_color");

		ClassDB::bind_method(D_METHOD("set_code_block_padding", "padding"), &Markova::set_code_block_padding);
		ClassDB::bind_method(D_METHOD("get_code_block_padding"), &Markova::get_code_block_padding);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "code_block_padding", PROPERTY_HINT_RANGE, "0,100"), "set_code_block_padding", "get_code_block_padding");

		ClassDB::bind_method(D_METHOD("set_blockquote_border_color", "color"), &Markova::set_blockquote_border_color);
		ClassDB::bind_method(D_METHOD("get_blockquote_border_color"), &Markova::get_blockquote_border_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "blockquote_border_color"), "set_blockquote_border_color", "get_blockquote_border_color");

		ClassDB::bind_method(D_METHOD("set_blockquote_background_color", "color"), &Markova::set_blockquote_background_color);
		ClassDB::bind_method(D_METHOD("get_blockquote_background_color"), &Markova::get_blockquote_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "blockquote_background_color"), "set_blockquote_background_color", "get_blockquote_background_color");

		ClassDB::bind_method(D_METHOD("set_blockquote_text_color", "color"), &Markova::set_blockquote_text_color);
		ClassDB::bind_method(D_METHOD("get_blockquote_text_color"), &Markova::get_blockquote_text_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "blockquote_text_color"), "set_blockquote_text_color", "get_blockquote_text_color");

		ClassDB::bind_method(D_METHOD("set_blockquote_padding", "padding"), &Markova::set_blockquote_padding);
		ClassDB::bind_method(D_METHOD("get_blockquote_padding"), &Markova::get_blockquote_padding);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "blockquote_padding", PROPERTY_HINT_RANGE, "0,100"), "set_blockquote_padding", "get_blockquote_padding");

		ClassDB::bind_method(D_METHOD("set_table_border_color", "color"), &Markova::set_table_border_color);
		ClassDB::bind_method(D_METHOD("get_table_border_color"), &Markova::get_table_border_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "table_border_color"), "set_table_border_color", "get_table_border_color");

		ClassDB::bind_method(D_METHOD("set_table_background_color", "color"), &Markova::set_table_background_color);
		ClassDB::bind_method(D_METHOD("get_table_background_color"), &Markova::get_table_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "table_background_color"), "set_table_background_color", "get_table_background_color");

		ClassDB::bind_method(D_METHOD("set_table_header_background_color", "color"), &Markova::set_table_header_background_color);
		ClassDB::bind_method(D_METHOD("get_table_header_background_color"), &Markova::get_table_header_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "table_header_background_color"), "set_table_header_background_color", "get_table_header_background_color");

		ClassDB::bind_method(D_METHOD("set_table_padding", "padding"), &Markova::set_table_padding);
		ClassDB::bind_method(D_METHOD("get_table_padding"), &Markova::get_table_padding);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "table_padding", PROPERTY_HINT_RANGE, "0,100"), "set_table_padding", "get_table_padding");
	}

public:
	Markova()
	{
		// Configure Self
		this->set_use_bbcode(true);
		this->set_fit_content(true);
		this->set_selection_enabled(true);
		this->add_theme_constant_override("line_separation", -1.0);

		// Connect Signals
		this->connect("meta_clicked", callable_mp(this, &Markova::OnMetaClicked));
	}
	void RenderMarkdown(const String& markdown)
	{
		ParseMarkdown(markdown);
		this->set_text(GenerateMarkup());
	}
	void ParseMarkdown(const String& markdown)
	{
		pasreState.elements.clear();
		pasreState.stack.clear();
		pasreState.textBuffer = "";

		CharString utf8 = markdown.utf8();
		mumd_parse(utf8.get_data(), utf8.length(), [this](const md_node node) { this->OnMarkdownNode(node); });

		Flush();
		parsedElements = std::move(pasreState.elements);
	}
	String GenerateMarkup() const
	{
		// Get Scale Factor
		double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

		// BBCode Generator
		std::function<String(const Element&)> GenerateBBCode = [&](const Element& elem) -> String
		{
			switch (elem.type)
			{
				case ElementType::TEXT:
					return elem.content;
				case ElementType::BOLD:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return "[b]" + content + "[/b]";
				}
				case ElementType::ITALIC:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return "[i]" + content + "[/i]";
				}
				case ElementType::STRIKETHROUGH:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return "[s]" + content + "[/s]";
				}
				case ElementType::INLINE_CODE:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return "[color=" + inlineCode.to_html() + "]" + content + "[/color]";
				}
				case ElementType::LINK:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return "[color=" + hyperLink.to_html() + "][url=" + elem.content + "]" + content + "[/url][/color]";
				}
				case ElementType::IMAGE:
				{
					// Images Are Not Supported Yet
					return "";
				}
				case ElementType::HEADER:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					int size = (elem.level == 1) ? SCALED(22) : (elem.level == 2) ? SCALED(20) : SCALED(16);
					String prefix = (/*elem.level == 2 || */ elem.level == 3) ? "\n\r" : "";
					return prefix + "[font_size=" + String::num_int64(size) + "][b]" + content + "[/b][/font_size]\n";
				}
				case ElementType::PARAGRAPH:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return content.replace("   ", "\r\n") + "\n\r";
				}
				case ElementType::CODE_BLOCK:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					String styleGen = jenova::Format(String("[table=1][cell border=%s bg=%s padding=%d,%d,%d,%d][color=%s][code]"),
						AS_C_STRING(codeBlockBorder.to_html()), AS_C_STRING(codeBlockBackground.to_html()),
						codeBlockPadding, codeBlockPadding, codeBlockPadding, codeBlockPadding,
						AS_C_STRING(codeBlockText.to_html()));
					return styleGen + content + "[/code][/color][/cell][/table]\n\r";
				}
				case ElementType::LIST:
				{
					String result;
					for (const auto& child : elem.children)
					{
						if (child.type == ElementType::LIST_ITEM)
						{
							String content = GenerateBBCode(child);
							result += U"\u2022 " + content + "\n";
						}
					}
					return result + "\n\r";
				}
				case ElementType::ORDERED_LIST:
				{
					String result;
					int num = 1;
					for (const auto& child : elem.children)
					{
						if (child.type == ElementType::LIST_ITEM)
						{
							String content = GenerateBBCode(child);
							result += String::num_int64(num) + ". " + content + "\n";
							num++;
						}
					}
					return result + "\n\r";
				}
				case ElementType::LIST_ITEM:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					return content;
				}
				case ElementType::BLOCKQUOTE:
				{
					String content;
					for (const auto& child : elem.children) content += GenerateBBCode(child);
					if (content.is_empty()) content = elem.content;
					if (content.begins_with(" ")) content = content.substr(1);
					PackedStringArray lines = content.split("\n");
					String quoted;
					for (int i = 0; i < lines.size(); i++) if (!lines[i].is_empty()) quoted += lines[i] + "\n";
					String styleGen = jenova::Format(String("[table=1][cell border=%s bg=%s padding=%d,%d,%d,%d][color=%s]"),
						AS_C_STRING(blockquoteBorder.to_html()), AS_C_STRING(blockquoteBackground.to_html()),
						blockquotePadding, blockquotePadding, blockquotePadding, blockquotePadding,
						AS_C_STRING(blockquoteText.to_html()));
					return styleGen + quoted + "[/color][/cell][/table]\n\r";
				}
				case ElementType::HORIZONTAL_RULE:
					return "[hr]\n";
				case ElementType::TABLE:
				{
					if (elem.children.empty()) break;
					int cols = 0;
					for (const auto& row : elem.children)
					{
						if (row.type == ElementType::TABLE_ROW)
						{
							int cellCount = 0;
							for (const auto& cell : row.children) if (cell.type == ElementType::TABLE_CELL) cellCount++;
							if (cellCount > cols) cols = cellCount;
						}
					}
					if (cols == 0) break;
					String result = "[table=" + String::num_int64(cols) + "]";
					for (const auto& row : elem.children)
					{
						if (row.type == ElementType::TABLE_ROW)
						{
							int cellIndex = 0;
							for (const auto& cell : row.children)
							{
								if (cell.type == ElementType::TABLE_CELL)
								{
									String cellContent;
									for (const auto& child : cell.children) cellContent += GenerateBBCode(child);
									if (cellContent.is_empty()) cellContent = cell.content;
									String styleGen = jenova::Format(String("[cell border=%s bg=%s padding=%d,%d,%d,%d]"),
										AS_C_STRING(tableBorder.to_html()), AS_C_STRING(tableBackground.to_html()),
										tablePadding, tablePadding, tablePadding, tablePadding);
									result += styleGen + cellContent + "[/cell]";
									cellIndex++;
								}
							}
							while (cellIndex < cols)
							{
								String styleGen = jenova::Format(String("[cell border=%s bg=%s padding=%d,%d,%d,%d]"),
									AS_C_STRING(tableBorder.to_html()), AS_C_STRING(tableBackground.to_html()),
									tablePadding, tablePadding, tablePadding, tablePadding);
								result += styleGen + "[/cell]";
								cellIndex++;
							}
						}
					}
					return result + "[/table]\n\r";
				}
				default:
					break;
				}
			return "";
		};

		// Create Result
		String result;
		for (const auto& elem : parsedElements)
		{
			result += GenerateBBCode(elem);
			result += "\n";
		}
		while (result.find("\n\n") != -1) result = result.replace("\n\n", "\n");
		return result;
	}
	void DumpElementsCache() const
	{
		print_line("============== PARSED ELEMENTS ==============");
		print_line("Total Elements: " + String::num_int64(parsedElements.size()) + "\n");
		std::function<void(const Element&, int)> dumpElement = [&](const Element& elem, int depth)
		{
			String indent = String("  ").repeat(depth);
			String typeString;
			switch (elem.type)
			{
				case ElementType::HEADER: typeString = "HEADER"; break;
				case ElementType::PARAGRAPH: typeString = "PARAGRAPH"; break;
				case ElementType::BOLD: typeString = "BOLD"; break;
				case ElementType::ITALIC: typeString = "ITALIC"; break;
				case ElementType::STRIKETHROUGH: typeString = "STRIKETHROUGH"; break;
				case ElementType::INLINE_CODE: typeString = "INLINE_CODE"; break;
				case ElementType::LINK: typeString = "LINK"; break;
				case ElementType::IMAGE: typeString = "IMAGE"; break;
				case ElementType::LIST: typeString = "LIST"; break;
				case ElementType::ORDERED_LIST: typeString = "ORDERED_LIST"; break;
				case ElementType::LIST_ITEM: typeString = "LIST_ITEM"; break;
				case ElementType::ORDERED_LIST_ITEM: typeString = "ORDERED_LIST_ITEM"; break;
				case ElementType::BLOCKQUOTE: typeString = "BLOCKQUOTE"; break;
				case ElementType::CODE_BLOCK: typeString = "CODE_BLOCK"; break;
				case ElementType::HORIZONTAL_RULE: typeString = "HORIZONTAL_RULE"; break;
				case ElementType::TABLE: typeString = "TABLE"; break;
				case ElementType::TABLE_ROW: typeString = "TABLE_ROW"; break;
				case ElementType::TABLE_CELL: typeString = "TABLE_CELL"; break;
				case ElementType::TEXT: typeString = "TEXT"; break;
				default: typeString = "UNKNOWN"; break;
			}
			String output = indent + "[" + typeString + "]";
			if (!elem.content.is_empty())
			{
				String displayContent = elem.content;
				if (displayContent.length() > 50) displayContent = displayContent.substr(0, 30) + "...";
				displayContent = displayContent.replace("\n", "\\n");
				output += " Content: \"" + displayContent + "\"";
			}
			if (elem.level > 0) output += " Level: " + String::num_int64(elem.level);
			print_line(output);
			for (const auto& child : elem.children) dumpElement(child, depth + 1);
		};
		for (const auto& elem : parsedElements) dumpElement(elem, 0);
		print_line("================ END ELEMENTS ===============");
	}

private:
	// Internal Functions
	void Flush()
	{
		if (!pasreState.textBuffer.is_empty())
		{
			Element elem;
			elem.type = ElementType::TEXT;
			elem.content = pasreState.textBuffer;
			pasreState.textBuffer = "";
			if (!pasreState.stack.empty()) pasreState.stack.back()->children.push_back(elem);
			else pasreState.elements.push_back(elem);
		}
	}
	void PushElement(ElementType type, const String& content = "", int level = 0)
	{
		Flush();
		Element elem;
		elem.type = type;
		elem.content = content;
		elem.level = level;
		if (!pasreState.stack.empty())
		{
			pasreState.stack.back()->children.push_back(elem);
			pasreState.stack.push_back(&pasreState.stack.back()->children.back());
		}
		else
		{
			pasreState.elements.push_back(elem);
			pasreState.stack.push_back(&pasreState.elements.back());
		}
	}
	void PopElement()
	{
		Flush();
		if (!pasreState.stack.empty()) pasreState.stack.pop_back();
	}

private:
	// Events
	void OnMarkdownNode(const md_node node)
	{
		switch (node.type)
		{
			case md_text:
				pasreState.textBuffer += String::utf8(node.text, node.length);
				break;
			case md_bold_start:
				PushElement(ElementType::BOLD);
				break;
			case md_bold_end:
				PopElement();
				break;
			case md_italics_start:
				PushElement(ElementType::ITALIC);
				break;
			case md_italics_end:
				PopElement();
				break;
			case md_strikethrough_start:
				PushElement(ElementType::STRIKETHROUGH);
				break;
			case md_strikethrough_end:
				PopElement();
				break;
			case md_code_start:
				PushElement(ElementType::INLINE_CODE);
				break;
			case md_code_end:
				PopElement();
				break;
			case md_link_start:
				PushElement(ElementType::LINK, String::utf8(node.text, node.length));
				break;
			case md_link_end:
			{
				String linkText = String::utf8(node.text, node.length);
				if (!pasreState.stack.empty() && pasreState.stack.back()->type == ElementType::LINK)
				{
					if (pasreState.stack.back()->children.empty())
					{
						Element textElem;
						textElem.type = ElementType::TEXT;
						textElem.content = linkText;
						pasreState.stack.back()->children.push_back(textElem);
					}
				}
				PopElement();
				break;
			}
			case md_image_start:
				PushElement(ElementType::IMAGE, String::utf8(node.text, node.length));
				break;
			case md_image_end:
				PopElement();
				break;
			case md_head_1_start:
				PushElement(ElementType::HEADER, "", 1);
				break;
			case md_head_1_end:
				PopElement();
				break;
			case md_head_2_start:
				PushElement(ElementType::HEADER, "", 2);
				break;
			case md_head_2_end:
				PopElement();
				break;
			case md_head_3_start:
				PushElement(ElementType::HEADER, "", 3);
				break;
			case md_head_3_end:
				PopElement();
				break;
			case md_head_4_start:
				PushElement(ElementType::HEADER, "", 4);
				break;
			case md_head_4_end:
				PopElement();
				break;
			case md_head_5_start:
				PushElement(ElementType::HEADER, "", 5);
				break;
			case md_head_5_end:
				PopElement();
				break;
			case md_head_6_start:
				PushElement(ElementType::HEADER, "", 6);
				break;
			case md_head_6_end:
				PopElement();
				break;
			case md_table_start:
				PushElement(ElementType::TABLE);
				break;
			case md_table_end:
				PopElement();
				break;
			case md_table_row_start:
				PushElement(ElementType::TABLE_ROW);
				break;
			case md_table_row_end:
				PopElement();
				break;
			case md_table_cell_header_start:
				PushElement(ElementType::TABLE_CELL);
				break;
			case md_table_cell_header_end:
				PopElement();
				break;
			case md_table_cell_start:
				PushElement(ElementType::TABLE_CELL);
				break;
			case md_table_cell_end:
				PopElement();
				break;
			case md_paragraph_start:
				PushElement(ElementType::PARAGRAPH);
				break;
			case md_paragraph_end:
				PopElement();
				break;
			case md_code_block_start:
				PushElement(ElementType::CODE_BLOCK, String::utf8(node.text, node.length));
				break;
			case md_code_block_end:
				PopElement();
				break;
			case md_quote_start:
				PushElement(ElementType::BLOCKQUOTE);
				break;
			case md_quote_end:
				PopElement();
				break;
			case md_unordered_list_start:
				PushElement(ElementType::LIST);
				break;
			case md_unordered_list_end:
				PopElement();
				break;
			case md_ordered_list_start:
				PushElement(ElementType::ORDERED_LIST);
				break;
			case md_ordered_list_end:
				PopElement();
				break;
			case md_list_item_start:
				PushElement(ElementType::LIST_ITEM);
				break;
			case md_list_item_end:
				PopElement();
				break;
			case md_horizontal_rule:
			{
				Flush();
				Element elem;
				elem.type = ElementType::HORIZONTAL_RULE;
				if (!pasreState.stack.empty()) pasreState.stack.back()->children.push_back(elem);
				else pasreState.elements.push_back(elem);
				break;
			}
		default:
			break;
		}
	}
	void OnMetaClicked(const String& p_meta)
	{
		if (p_meta.begins_with("http://") || p_meta.begins_with("https://"))
		{
			// Maybe Add Warning Like Jenova Installer?
			OS::get_singleton()->shell_open(p_meta);
		}
	}

public:
	// Setters/Getters
	void set_inline_code_color(Color color) { inlineCode = color; }
	Color get_inline_code_color() const { return inlineCode; }
	void set_hyper_link_color(Color color) { hyperLink = color; }
	Color get_hyper_link_color() const { return hyperLink; }
	void set_code_block_border_color(Color color) { codeBlockBorder = color; }
	Color get_code_block_border_color() const { return codeBlockBorder; }
	void set_code_block_background_color(Color color) { codeBlockBackground = color; }
	Color get_code_block_background_color() const { return codeBlockBackground; }
	void set_code_block_text_color(Color color) { codeBlockText = color; }
	Color get_code_block_text_color() const { return codeBlockText; }
	void set_code_block_padding(int padding) { codeBlockPadding = padding; }
	int get_code_block_padding() const { return codeBlockPadding; }
	void set_blockquote_border_color(Color color) { blockquoteBorder = color; }
	Color get_blockquote_border_color() const { return blockquoteBorder; }
	void set_blockquote_background_color(Color color) { blockquoteBackground = color; }
	Color get_blockquote_background_color() const { return blockquoteBackground; }
	void set_blockquote_text_color(Color color) { blockquoteText = color; }
	Color get_blockquote_text_color() const { return blockquoteText; }
	void set_blockquote_padding(int padding) { blockquotePadding = padding; }
	int get_blockquote_padding() const { return blockquotePadding; }
	void set_table_border_color(Color color) { tableBorder = color; }
	Color get_table_border_color() const { return tableBorder; }
	void set_table_background_color(Color color) { tableBackground = color; }
	Color get_table_background_color() const { return tableBackground; }
	void set_table_header_background_color(Color color) { tableHeaderBackground = color; }
	Color get_table_header_background_color() const { return tableHeaderBackground; }
	void set_table_padding(int padding) { tablePadding = padding; }
	int get_table_padding() const { return tablePadding; }

private:
	ParseState pasreState;
	std::vector<Element> parsedElements;
	String markdownSource;
};
