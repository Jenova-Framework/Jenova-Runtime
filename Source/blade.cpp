
/*-------------------------------------------------------------+
|                                                              |
|           ______         _______ ______  _______             |
|           |_____] |      |_____| |     \ |______             |
|           |_____] |_____ |     | |_____/ |______             |
|                                                              |
|                        Blade Language                        |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Standard Library Imports
#include <stdarg.h>
#include <vector>
#include <random>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>

// Platform Imports
#ifdef TARGET_PLATFORM_WINDOWS
#include <windows.h>
#endif

// Blade Language
#include "Blade.h"

// Tiny C Compiler
#include <TinyCC/libtcc.h>

// Resources
#include "IconDatabase.h"

// Macros
#define BLADESYSAPI 		extern "C"
#define BLADEFUNCTION(f) 	(const void*)(f)

// String Converters
static String GetStr(const std::string& stdStr)
{
	return String(stdStr.c_str());
}
static const char* GetCStr(const String& godotStr)
{
	std::string str((char*)godotStr.utf8().ptr(), godotStr.utf8().size());
	if (!str.empty() && str.back() == '\0') str.pop_back();
	return _strdup(str.c_str());
}
static std::string GetStdStr(const String& godotStr)
{
	std::string str((char*)godotStr.utf8().ptr(), godotStr.utf8().size());
	if (!str.empty() && str.back() == '\0') str.pop_back();
	return std::string(str.c_str());
}

// Utilities
static String Format(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef _MSC_VER
	int size = _vscprintf(fmt, args) + 1;
#else
	int size = vsnprintf(nullptr, 0, fmt, args) + 1;
#endif
	va_end(args);
	std::vector<char> buf(size);
	va_start(args, fmt);
	vsnprintf(buf.data(), size, fmt, args);
	va_end(args);
	return String(buf.data());
}
static String Gradient(String sig)
{
    Color cl_from = Color::html("#f79b45");
    Color cl_to   = Color::html("#f75a45");

    String rich;
    int len = sig.length();
    for (int i = 0; i < len; i++)
    {
        float t = (float)i / (float)(len - 1);
        Color cl = cl_from.lerp(cl_to, t);
        String hex = cl.to_html(false);
        rich += Format("[color=%s]%c[/color]", GetStdStr(hex).c_str(), sig[i]);
    }
    return rich;
}
static void Verbose(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	godot::UtilityFunctions::print_rich(Gradient("[BLADE] > ") + godot::String(buffer));
}
static void DevLog(const char* fmt, ...)
{
#if (DEV_BUILD == 1)
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	godot::UtilityFunctions::print_rich(Gradient("[BLADE-DEV] > ") + godot::String(buffer));
#endif
}
static void ErrorLog(const char* stage, const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	UtilityFunctions::push_error(String("[BLADE] [" + String(stage) + "] Error > ") + String(buffer));
}
static void DevError(const char* stage, const char* fmt, ...)
{
#if (DEV_BUILD == 1)
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	UtilityFunctions::push_error(String("[BLADE] [" + String(stage) + "] Developer Error > ") + String(buffer));
#endif
}
static String GetScriptUID(Ref<BladeScript> script)
{
	return script->get_path().md5_text().substr(0, 12);
}
static String CreateUniqueID(int length)
{
	if (length <= 0) return godot::String("hailsatan");
	static std::mt19937 rng{ std::random_device{}() };
	static std::uniform_int_distribution<int> dist{ 0, 25 };
	constexpr const char* letters = "abcdefghijklmnopqrstuvwxyz";
	std::string s(length, ' ');
	for (int i = 0; i < length; ++i) s[i] = letters[dist(rng)];
	return godot::String(s.c_str());
}
static Color ColorFromHex(const std::string &hex)
{
    std::string h = hex[0] == '#' ? hex.substr(1) : hex;
    uint32_t r = 255, g = 255, b = 255, a = 255;
    if (h.size() >= 6)
    {
        r = std::stoul(h.substr(0, 2), nullptr, 16);
        g = std::stoul(h.substr(2, 2), nullptr, 16);
        b = std::stoul(h.substr(4, 2), nullptr, 16);
        if (h.size() == 8) a = std::stoul(h.substr(6, 2), nullptr, 16);
    }
    return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
static String Snakify(String s)
{
	String out;
	int len = s.length();
	for (int i = 0; i < len; i++)
	{
		char32_t c = s[i];
		bool isUpper = (c >= 'A' && c <= 'Z');
		bool prevLower = (i > 0 && s[i - 1] >= 'a' && s[i - 1] <= 'z');
		bool nextLower = (i + 1 < len && s[i + 1] >= 'a' && s[i + 1] <= 'z');
		if (i > 0 && isUpper && (prevLower || nextLower)) out += "_";
		out += String::chr(isUpper ? c + 32 : c);
	}
	out = out.replace("3_d", "3d");
	out = out.replace("2_d", "2d");
	return out;
}
static String StripQuotes(String s)
{
	if (s.begins_with("\"") && s.ends_with("\"")) return s.substr(1, s.length() - 2);
	return s;
};
static bool WriteStringToFile(const String& filePath, const String& str)
{
	// Write String On Disk
	Ref<FileAccess> handle = FileAccess::open(filePath, FileAccess::ModeFlags::WRITE);
	if (handle.is_valid())
	{
		handle->store_string(str);
		handle->close();
		return true;
	}
	else
	{
		return false;
	}
}
static String ReadStringFromFile(const String& filePath)
{
	// Read String From Disk
	Ref<FileAccess> handle = FileAccess::open(filePath, FileAccess::ModeFlags::READ);
	if (handle.is_valid())
	{
		String content = handle->get_as_text();
		handle->close();
		return content;
	}
	else
	{
		return "";
	}
}
static bool WriteStdStringToFile(const std::string& filePath, const std::string& str)
{
	std::ofstream outFile(filePath, std::ios::out | std::ios::binary);
	if (outFile.is_open())
	{
		outFile.write(str.c_str(), str.size());
		outFile.close();
		return true;
	}
	else
	{
		return false;
	}
}
static std::string ReadStdStringFromFile(const std::string& filePath)
{
	std::ifstream inFile(filePath);
	if (inFile.is_open())
	{
		std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
		inFile.close();
		return content;
	}
	else
	{
		return "";
	}
}
static void ReplaceAllMatchesWithString(std::string& targetString, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = targetString.find(from, start_pos)) != std::string::npos)
	{
		targetString.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}
static Ref<ImageTexture> CreateSVGFromByteArray(const uint8_t* imageDataPtr, size_t imageDataSize, const Vector2i& imageSize)
{
	PackedByteArray imageDataPackedBytes;
	imageDataPackedBytes.resize(imageDataSize);
	memcpy(imageDataPackedBytes.ptrw(), imageDataPtr, imageDataSize);
	Ref<Image> createdImage = memnew(Image);
	godot::Error loadResult = godot::Error::FAILED;
	loadResult = createdImage->load_svg_from_buffer(imageDataPackedBytes);
	imageDataPackedBytes.clear();
	if (loadResult == godot::Error::OK)
	{
		if (imageSize != Vector2i(0, 0))
		{
			createdImage->resize(imageSize.width, imageSize.height, Image::INTERPOLATE_LANCZOS);
		}
		Ref<ImageTexture> createdImageTexture = ImageTexture::create_from_image(createdImage);
		return createdImageTexture;
	}
	else
	{
		createdImage.unref();
		return nullptr;
	}
}
static bool IsRuntimeExecution()
{
	if (OS::get_singleton()->has_feature("template")) return true;
	return false;
}
static String GetBladeCacheDirectory()
{
	// Initialize Directory Path
	String bladeCacheDirectory = "";

	// Generate Cache Directory Path
	bladeCacheDirectory = ProjectSettings::get_singleton()->globalize_path("res://") + String(blade::BladeCacheDirectory);

	// Clean Up Path
	bladeCacheDirectory = bladeCacheDirectory.replace("//", "/");

	// Try to Create It If Doesn't Exist
	if (!std::filesystem::exists(GetStdStr(bladeCacheDirectory)) && Engine::get_singleton()->is_editor_hint())
	{
		try
		{
			std::filesystem::create_directories(GetStdStr(bladeCacheDirectory));

			// Create Ignore Files
			WriteStringToFile(bladeCacheDirectory + "/" + ".gdignore", "*");
			WriteStringToFile(bladeCacheDirectory + "/" + ".gitignore", "*");
		}
		catch (const std::filesystem::filesystem_error& e)
		{
		}
	}

	// Return Path
	return bladeCacheDirectory;
}
static String GetBladeLibraryDirectory()
{
	String libraryPath = GetBladeCacheDirectory() + "/" + blade::BladeLibraryDirectory;
	if (Engine::get_singleton()->is_editor_hint()) std::filesystem::create_directories(GetStdStr(libraryPath));
	return libraryPath;
}
static String GetBladeBindingsDatabasePath()
{
	return GetBladeCacheDirectory().path_join(blade::BladeBindingsDatabasePath);
}
static String GetScriptCachePath(Ref<BladeScript> script, bool runtime)
{
	if (runtime) return blade::BladeScriptCacheRuntimePath + GetScriptUID(script) + ".bladecache";
	return GetBladeCacheDirectory() + "/" + GetScriptUID(script) + ".bladecache";
}
PackedStringArray CollectBladeCacheFiles()
{
	String cachePath = GetBladeCacheDirectory();
    PackedStringArray result;
    Ref<DirAccess> dir = DirAccess::open(cachePath);
    if (dir.is_valid())
	{
        dir->list_dir_begin();
        String file_name = dir->get_next();
        while (!file_name.is_empty())
		{
            if (!dir->current_is_dir() && file_name.ends_with(".bladecache")) result.push_back(cachePath.path_join(file_name));
            file_name = dir->get_next();
        }
        dir->list_dir_end();
    }
    return result;
}
static Variant::Type MapBladeType(const std::string &t)
{
    if (t == "Bool") return Variant::BOOL;
    if (t == "Int") return Variant::INT;
    if (t == "Float") return Variant::FLOAT;
    if (t == "String") return Variant::STRING;
    if (t == "Vector2") return Variant::VECTOR2;
    if (t == "Vector2i") return Variant::VECTOR2I;
    if (t == "Rect2") return Variant::RECT2;
    if (t == "Rect2i") return Variant::RECT2I;
    if (t == "Vector3") return Variant::VECTOR3;
    if (t == "Vector3i") return Variant::VECTOR3I;
    if (t == "Transform2D") return Variant::TRANSFORM2D;
    if (t == "Vector4") return Variant::VECTOR4;
    if (t == "Vector4i") return Variant::VECTOR4I;
    if (t == "Plane") return Variant::PLANE;
    if (t == "Quaternion") return Variant::QUATERNION;
    if (t == "AABB") return Variant::AABB;
    if (t == "Basis") return Variant::BASIS;
    if (t == "Transform3D") return Variant::TRANSFORM3D;
    if (t == "Projection") return Variant::PROJECTION;
    if (t == "Color") return Variant::COLOR;
    if (t == "StringName") return Variant::STRING_NAME;
    if (t == "NodePath") return Variant::NODE_PATH;
    if (t == "RID") return Variant::RID;
    if (t == "Instance") return Variant::OBJECT;
    if (t == "Callable") return Variant::CALLABLE;
    if (t == "Signal") return Variant::SIGNAL;
    if (t == "Dictionary") return Variant::DICTIONARY;
    if (t == "Array") return Variant::ARRAY;
    if (t == "PackedByteArray") return Variant::PACKED_BYTE_ARRAY;
    if (t == "PackedInt32Array") return Variant::PACKED_INT32_ARRAY;
    if (t == "PackedInt64Array") return Variant::PACKED_INT64_ARRAY;
    if (t == "PackedFloat32Array") return Variant::PACKED_FLOAT32_ARRAY;
    if (t == "PackedFloat64Array") return Variant::PACKED_FLOAT64_ARRAY;
    if (t == "PackedStringArray") return Variant::PACKED_STRING_ARRAY;
    if (t == "PackedVector2Array") return Variant::PACKED_VECTOR2_ARRAY;
    if (t == "PackedVector3Array") return Variant::PACKED_VECTOR3_ARRAY;
    if (t == "PackedColorArray") return Variant::PACKED_COLOR_ARRAY;
    if (t == "PackedVector4Array") return Variant::PACKED_VECTOR4_ARRAY;
    return Variant::NIL;
}
static Variant::Type MapBladeType(const String& t)
{
	return MapBladeType(GetStdStr(t));
}
static std::string MapVariantType(Variant::Type t)
{
	switch (t)
	{
		case Variant::BOOL: return "Bool";
		case Variant::INT: return "Int";
		case Variant::FLOAT: return "Float";
		case Variant::STRING: return "String";
		case Variant::VECTOR2: return "Vector2";
		case Variant::VECTOR2I: return "Vector2i";
		case Variant::RECT2: return "Rect2";
		case Variant::RECT2I: return "Rect2i";
		case Variant::VECTOR3: return "Vector3";
		case Variant::VECTOR3I: return "Vector3i";
		case Variant::TRANSFORM2D: return "Transform2D";
		case Variant::VECTOR4: return "Vector4";
		case Variant::VECTOR4I: return "Vector4i";
		case Variant::PLANE: return "Plane";
		case Variant::QUATERNION: return "Quaternion";
		case Variant::AABB: return "AABB";
		case Variant::BASIS: return "Basis";
		case Variant::TRANSFORM3D: return "Transform3D";
		case Variant::PROJECTION: return "Projection";
		case Variant::COLOR: return "Color";
		case Variant::STRING_NAME: return "StringName";
		case Variant::NODE_PATH: return "NodePath";
		case Variant::RID: return "RID";
		case Variant::OBJECT: return "Instance";
		case Variant::CALLABLE: return "Callable";
		case Variant::SIGNAL: return "Signal";
		case Variant::DICTIONARY: return "Dictionary";
		case Variant::ARRAY: return "Array";
		case Variant::PACKED_BYTE_ARRAY: return "PackedByteArray";
		case Variant::PACKED_INT32_ARRAY: return "PackedInt32Array";
		case Variant::PACKED_INT64_ARRAY: return "PackedInt64Array";
		case Variant::PACKED_FLOAT32_ARRAY: return "PackedFloat32Array";
		case Variant::PACKED_FLOAT64_ARRAY: return "PackedFloat64Array";
		case Variant::PACKED_STRING_ARRAY: return "PackedStringArray";
		case Variant::PACKED_VECTOR2_ARRAY: return "PackedVector2Array";
		case Variant::PACKED_VECTOR3_ARRAY: return "PackedVector3Array";
		case Variant::PACKED_COLOR_ARRAY: return "PackedColorArray";
		case Variant::PACKED_VECTOR4_ARRAY: return "PackedVector4Array";
		default: return "Nil";
	}
}
static String GetVariantTypeFromBladeType(const String& type)
{
	if (type == "NIL") return "TYPE_NIL";
	if (type == "Bool") return "TYPE_BOOL";
	if (type == "Int") return "TYPE_INT";
	if (type == "Float") return "TYPE_FLOAT";
	if (type == "String") return "TYPE_STRING";
	if (type == "Vector2") return "TYPE_VECTOR2";
	if (type == "Vector2i") return "TYPE_VECTOR2I";
	if (type == "Rect2") return "TYPE_RECT2";
	if (type == "Rect2i") return "TYPE_RECT2I";
	if (type == "Vector3") return "TYPE_VECTOR3";
	if (type == "Vector3i") return "TYPE_VECTOR3I";
	if (type == "Transform2D") return "TYPE_TRANSFORM2D";
	if (type == "Vector4") return "TYPE_VECTOR4";
	if (type == "Vector4i") return "TYPE_VECTOR4I";
	if (type == "Plane") return "TYPE_PLANE";
	if (type == "Quaternion") return "TYPE_QUATERNION";
	if (type == "AABB") return "TYPE_AABB";
	if (type == "Basis") return "TYPE_BASIS";
	if (type == "Transform3D") return "TYPE_TRANSFORM3D";
	if (type == "Projection") return "TYPE_PROJECTION";
	if (type == "Color") return "TYPE_COLOR";
	if (type == "StringName") return "TYPE_STRING_NAME";
	if (type == "NodePath") return "TYPE_NODE_PATH";
	if (type == "RID") return "TYPE_RID";
	if (type == "Instance") return "TYPE_OBJECT";
	if (type == "Object") return "TYPE_OBJECT";
	if (type == "Callable") return "TYPE_CALLABLE";
	if (type == "Signal") return "TYPE_SIGNAL";
	if (type == "Dictionary") return "TYPE_DICTIONARY";
	if (type == "Array") return "TYPE_ARRAY";
	if (type == "PackedByteArray") return "TYPE_PACKED_BYTE_ARRAY";
	if (type == "PackedInt32Array") return "TYPE_PACKED_INT32_ARRAY";
	if (type == "PackedInt64Array") return "TYPE_PACKED_INT64_ARRAY";
	if (type == "PackedFloat32Array") return "TYPE_PACKED_FLOAT32_ARRAY";
	if (type == "PackedFloat64Array") return "TYPE_PACKED_FLOAT64_ARRAY";
	if (type == "PackedStringArray") return "TYPE_PACKED_STRING_ARRAY";
	if (type == "PackedVector2Array") return "TYPE_PACKED_VECTOR2_ARRAY";
	if (type == "PackedVector3Array") return "TYPE_PACKED_VECTOR3_ARRAY";
	if (type == "PackedColorArray") return "TYPE_PACKED_COLOR_ARRAY";
	if (type == "PackedVector4Array") return "TYPE_PACKED_VECTOR4_ARRAY";
	return "TYPE_OBJECT"; // Or Maybe TYPE_NIL?
}
static inline GDExtensionConstTypePtr EncodeUtilityFunctionArgument(const Variant &v, uint64_t &slot)
{
    switch (v.get_type())
    {
        case Variant::FLOAT:
        {
            double val = (double)v;
            memcpy(&slot, &val, sizeof(double));
            return &slot;
        }
        case Variant::INT:
        {
            int64_t val = (int64_t)v;
            memcpy(&slot, &val, sizeof(int64_t));
            return &slot;
        }
        case Variant::BOOL:
        {
            int8_t val = (bool)v ? 1 : 0;
            memcpy(&slot, &val, sizeof(int8_t));
            return &slot;
        }
        case Variant::RID:
        {
            RID rid = v;
            int64_t id = rid.get_id();
            memcpy(&slot, &id, sizeof(int64_t));
            return &slot;
        }
        case Variant::OBJECT:
        {
            Object *obj = v;
            memcpy(&slot, &obj, sizeof(Object *));
            return &slot;
        }
        default:
        {
            return v._native_ptr();
        }
    }
}
static inline Variant DecodeUtilityFunctionReturn(Variant::Type ret_type, GDExtensionPtrUtilityFunction func, GDExtensionConstTypePtr *mb_args, int argc)
{
    switch (ret_type)
    {
        case Variant::FLOAT:
        {
            double ret = 0.0;
            func(&ret, mb_args, argc);
            return Variant(ret);
        }
        case Variant::INT:
        {
            int64_t ret = 0;
            func(&ret, mb_args, argc);
            return Variant(ret);
        }
        case Variant::BOOL:
        {
            int8_t ret = 0;
            func(&ret, mb_args, argc);
            return Variant(bool(ret != 0));
        }
        case Variant::RID:
        {
            int64_t ret_id = 0;
            func(&ret_id, mb_args, argc);
            RID rid = UtilityFunctions::rid_from_int64(ret_id);
            return Variant(rid);
        }
        case Variant::OBJECT:
        {
            Object *ret = nullptr;
            func(&ret, mb_args, argc);
            return Variant(ret);
        }
        default:
        {
			// Godot Expects a Variant retunr Buffer For Anything Else
            Variant ret;
            func(ret._native_ptr(), mb_args, argc);
            return ret;
        }
    }
}
static void SendRemoteMessage(const String& message, const Array& args = Array())
{
	if (EditorInterface::get_singleton()->is_playing_scene())
	{
		BladePlugin* bladePlugin = BladePlugin::GetSingleton();
		if (!bladePlugin) return;
		Ref<EditorDebuggerPlugin> debuggerPlugin = bladePlugin->GetDebugger();
		if (!debuggerPlugin.is_valid()) return;
		Array currentSessions = debuggerPlugin->get_sessions();
		for (size_t i = 0; i < currentSessions.size(); i++)
		{
			Ref<EditorDebuggerSession> debuggerSession = currentSessions[i];
			if (debuggerSession.is_valid() && debuggerSession->is_active()) debuggerSession->send_message(message, args);
		}
	}
}

// Banner Printer
static void PrintBladeBanner()
{
    static Color cl_from = Color::html("#f79b45");
    static Color cl_to   = Color::html("#f75a45");

	std::string versionInfo = GetStdStr(Format(".:: Blade Language Runtime %s (x64-Windows) ::.", blade::BladeVersion));

    const char* lines[] =
	{
        "===================================================================",
		" ",
        "     ...     ..            ..                ..                  ",
        "  .=*6666x <\"?66h.   x .d66\"               dF                    ",
        " X>  '6666H> '6666    5666R               '66bu.                 ",
        "'666. `6666   6666    '666R         u     '*66666bu        .u    ",
        "'6666 '6666    \"66>    666R      us666u.    ^\"*6666N    ud6666.  ",
        " `666 '6666.xH666x.    666R   .@66 \"6666\"  beWE \"666L :666'6666. ",
        "   X\" :66*~  `*6666>   666R   9666  9666   666E  666E d666 '66%\" ",
        " ~\"   !\"`      \"666>   666R   9666  9666   666E  666E 6666.+\"    ",
        "  .H6666h.      ?66    666R   9666  9666   666E  666F 6666L      ",
        " :\"^\"B6666h.    '!    .666B . 9666  9666  .666N..666  '6666c. .+ ",
        " ^    \"66666hx.+\"     ^*666%  \"666*\"\"666\"  `\"666*\"\"    \"66666%   ",
        "        ^\"**\"\"          \"%     ^Y\"   ^Y'      \"\"         \"YP'    ",
		" ",
        "===================================================================",
		versionInfo.c_str(),
		"            Developed by Hamid.Memar (MemarDesign LLC.)            ",
        "==================================================================="
    };

    int line_count = sizeof(lines) / sizeof(lines[0]);
    for (int i = 0; i < line_count; i++)
	{
        float t = (float)i / (float)(line_count - 1);
        Color cl = cl_from.lerp(cl_to, t);
        String hex = cl.to_html(false);
        UtilityFunctions::print_rich(Format("[color=%s]%s[/color]", GetStdStr(hex).c_str(), lines[i]));
    }
    UtilityFunctions::print("\n");
}

// Helper Utilities
static String* AllocateString(const String& p_str)
{
	String* ptr = memnew(String);
	*ptr = p_str;
	return ptr;
}
static StringName* AllocateStringName(const String& p_str)
{
	StringName* ptr = memnew(StringName);
	*ptr = p_str;
	return ptr;
}
static GDExtensionPropertyInfo CreatePropertyType(const Dictionary& p_src)
{
	GDExtensionPropertyInfo p_dst;
	p_dst.type = GDExtensionVariantType::GDEXTENSION_VARIANT_TYPE_NIL;
	p_dst.name = AllocateStringName(p_src["name"]);
	p_dst.class_name = AllocateStringName(p_src["class_name"]);
	p_dst.hint = p_src["hint"];
	p_dst.hint_string = AllocateString(p_src["hint_string"]);
	p_dst.usage = p_src["usage"];
	return p_dst;
}
static void AddState(GDExtensionConstStringNamePtr p_name, GDExtensionConstVariantPtr p_value, void* p_userdata)
{
	List<Pair<StringName, Variant>>* list = reinterpret_cast<List<Pair<StringName, Variant>>*>(p_userdata);
	list->push_back({ *(const StringName*)p_name, *(const Variant*)p_value });
}
static GDExtensionMethodInfo CreateMethodInfo(const MethodInfo& methodInfo, BladeScriptInstance* instance)
{
	// Create Extension Parameters Info
	std::vector<GDExtensionPropertyInfo>* parametersInfo = new std::vector<GDExtensionPropertyInfo>();

	for (const auto& parameter : methodInfo.arguments)
	{
		parametersInfo->push_back(GDExtensionPropertyInfo{
				GDExtensionVariantType(parameter.type),
				AllocateStringName(parameter.name),
				AllocateStringName(parameter.class_name),
				parameter.hint,
				AllocateString(parameter.hint_string),
				parameter.usage });
	}

	// Create Extension Method Info
	return GDExtensionMethodInfo{
		AllocateStringName(methodInfo.name),
		GDExtensionPropertyInfo{
				GDExtensionVariantType(methodInfo.return_val.type),
				AllocateStringName(methodInfo.return_val.name),
				AllocateStringName(methodInfo.return_val.class_name),
				methodInfo.return_val.hint,
				AllocateString(methodInfo.return_val.hint_string),
				methodInfo.return_val.usage },
		methodInfo.flags,
		methodInfo.id,
		(uint32_t)methodInfo.arguments.size(),
		parametersInfo->data(),
		0, nullptr // Default Arguments Not Supported Yet
	};
}
static void FreePropertyList(const GDExtensionPropertyInfo& p_property)
{
	memdelete((StringName*)p_property.name);
	memdelete((StringName*)p_property.class_name);
	memdelete((String*)p_property.hint_string);
}
template <typename T> T* memnew_with_size(int p_size)
{
	uint64_t size = sizeof(T) * p_size;
	void* ptr = memalloc(size + sizeof(int));
	*((int*)ptr) = p_size;
	return (T*)((int*)ptr + 1);
}
template <typename T> void memdelete_with_size(const T* p_ptr)
{
	memfree((int*)p_ptr - 1);
}
template <typename T> int memnew_ptr_size(const T* p_ptr)
{
	return !p_ptr ? 0 : *((int*)p_ptr - 1);
}
template <typename T> T* cast_ptr(void* p_ptr)
{
	return (T*)(p_ptr);
}

// References
static BladeLanguage*					bladeLanguage = nullptr;
static Ref<BladeResourceLoader>			bladeScriptLoader;
static Ref<BladeResourceSaver>			bladeScriptSaver;

// Storages
static ExtensionStorage					extensionStorage;

// Blade Language System (Kernel)
namespace blade
{
	// Types
	extern "C" struct BladeObject
	{
		void* ptr		= nullptr;
		char type[64]	= {};
		bool ref		= false;
		bool singleton	= false;
	};
	extern "C" struct BladeVariant
	{
		uint8_t opaque[GODOT_CPP_VARIANT_SIZE]{ 0 };
	};
	extern "C" struct BladeProperty
	{
		int type;
		char name[64];
		char class_name[64];
		int hint;
		char hint_string[128];
		int usage;
		char default_value[128];
		char group[64];
	};

	// Functions
	namespace system
	{
		// Core Functions
		BLADESYSAPI Variant blade_exec(BladeObject* bobj, const char* method, int argc, Variant* args)
		{
			// Get Object
			Object* obj = static_cast<Object*>(bobj->ptr);

			// Handle Extension Object Calls
			if (strcmp(method, "as_obj") == 0)
			{
				return Variant((Object*)bobj->ptr);
			}

			// Handle User-Defined Extension Object Calls
			if (blade::ExtensionSystemEnabled)
			{
				auto it = extensionStorage.find(method);
				if (it != extensionStorage.end() && it->second.kind == "object")
				{
					// Extension Metadata
					const auto& ext = it->second;

					// Validate Type-Locked Extension
					if (!ext.isGeneric)
					{
						bool typeMismatch = !obj->is_class(GetStr(ext.receiverType));
						bool argCountMismatch = (argc != ext.paramCount);

						if (typeMismatch || argCountMismatch)
						{
							if (typeMismatch)
							{
								ErrorLog("Runtime",
									"Object extension expects receiver type '%s' but was called on '%s'.",
									ext.receiverType.c_str(), GetStdStr(obj->get_class()).c_str());
							}
							if (argCountMismatch)
							{
								ErrorLog("Runtime",
									"Object extension '%s' expects %d parameters but received %d.",
									ext.name.c_str(), ext.paramCount, argc);
							}
							return Variant::NIL;
						}
					}

					// Validate Function
					if (ext.fn.callo == nullptr)
					{
						ErrorLog("Runtime", "Object extension '%s' pointer is corrupted.", ext.name.c_str());
						return Variant::NIL;
					}
					
					// Call Extension
					return ext.fn.callo(bobj, argc, args);
				}
			}

			// Handle Object Call
			Array arr; arr.resize(argc);
			for (int i = 0; i < argc; i++) arr[i] = args[i];
			Variant ret = obj->callv(StringName(method), arr);
			return ret;
		}
		BLADESYSAPI Variant blade_vcall(Variant* varptr, const char* method, int argc, Variant* args)
		{
			// Get Variant
			Variant variant = *varptr;

			// Handle User-Defined Extension Variant Calls
			if (blade::ExtensionSystemEnabled)
			{
				auto it = extensionStorage.find(method);
				if (it != extensionStorage.end() && it->second.kind == "variant")
				{
					// Extension Metadata
					const auto& ext = it->second;

					// Validate Type-Locked Extension
					if (!ext.isGeneric)
					{
						bool typeMismatch = (variant.get_type() != MapBladeType(ext.receiverType));
						bool argCountMismatch = (argc != ext.paramCount);
						if (typeMismatch || argCountMismatch)
						{
							if (typeMismatch)
							{
								ErrorLog("Runtime",
									"Variant extension expects receiver type '%s' but was called on '%s'.",
									ext.receiverType.c_str(), MapVariantType(variant.get_type()).c_str());
							}
							if (argCountMismatch)
							{
								ErrorLog("Runtime",
									"Variant extension '%s' expects %d parameters but received %d.",
									ext.name.c_str(), ext.paramCount, argc);
							}
							return Variant::NIL;
						}
					}

					// Validate Function
					if (ext.fn.callv == nullptr)
					{
						ErrorLog("Runtime", "Variant extension '%s' pointer is corrupted.", ext.name.c_str());
						return Variant::NIL;
					}

					// Call Extension
					return ext.fn.callv(variant, argc, args);
				}
			}

			// Handle Extension Variant Calls
			if (strcmp(method, "as_string") == 0)
			{
				if (!Variant::can_convert(variant.get_type(), Variant::STRING))
				{
					ErrorLog("Runtime", "Backend: [varcall] failed! Cannot convert from %s to String.", GetStdStr(variant.get_type_name(variant.get_type())).c_str());
					return String("<invalid>");
				}
				return String(variant);
			}
			if (strcmp(method, "field_access") == 0)
			{
				// Validations
				if (argc != 1) 
				{
					ErrorLog("Runtime", "Backend: [varcall] Invalid Arguments, Needs 1 Got %d.", argc);
					return Variant::NIL;
				}
				if (args[0].get_type() != Variant::STRING)
				{
					ErrorLog("Runtime", "Backend: [varcall] Invalid Arguments, Method Expects String for Field Name.");
					return Variant::NIL;
				}
				
				// Field Access
				bool success = false, oob = false; 
				String field = args[0];
				Variant result;
				if (field.is_valid_int())
				{
					result = variant.get_indexed(field.to_int(), success, oob);
					if (!success || oob)
					{ 
						ErrorLog("Runtime", "Backend: [varcall] Invalid array index %lld.%s", size_t(field.to_int()), oob ? " Out of Bounds." : "");
						return Variant::NIL;
					}
					return result;
				}
				else if (variant.has_key(field)) result = variant.get_keyed(field, success);
				else result = variant.get_named(field, success);
				if (!success)
				{
					std::string variantType = GetStdStr(variant.get_type_name(variant.get_type()));
					ErrorLog("Runtime", "Backend: [varcall] Variant `%s` doesn't has the requested field '%s' to access.", variantType.c_str(), GetStdStr(String(args[0])).c_str());
					return Variant::NIL;
				}
				return result;
			}
			if (strcmp(method, "field_assign") == 0)
			{
				// Validations
				if (argc != 2)
				{
					ErrorLog("Runtime", "Backend: [varcall] Invalid Arguments, Needs 2 Got %d.", argc);
					return Variant::NIL;
				}
				if (args[0].get_type() != Variant::STRING)
				{
					ErrorLog("Runtime", "Backend: [varcall] Invalid Arguments, Method Expects String for Field Name.");
					return Variant::NIL;
				}

				bool success = false, oob = false; 
				Variant result = variant;
				String field = args[0];
				if (field.is_valid_int()) 
				{
					result.set_indexed(field.to_int(), args[1], success, oob);
					if (!success || oob)
					{ 
						ErrorLog("Runtime", "Backend: [varcall] Invalid array index %lld for assignment.", size_t(field.to_int()));
					}
					*varptr = result;
					return result;
				}
				if (result.has_key(field)) result.set_keyed(field, args[1], success);
				else result.set_named(field, args[1], success);
				std::string variantType = GetStdStr(variant.get_type_name(variant.get_type()));
				if (!success) ErrorLog("Runtime", "Backend: [varcall] Variant `%s` doesn't has the requested field '%s' to assign.", variantType.c_str(), GetStdStr(String(args[0])).c_str());
				*varptr = result;
				return result;
			}

			// Handle Variant Call
			std::vector<Variant> vargs;
			vargs.reserve(argc);
			for (int i = 0; i < argc; i++) vargs.push_back(args[i]);
			std::vector<const Variant*> argptrs;
			argptrs.resize(argc);
			for (int i = 0; i < argc; i++) argptrs[i] = &vargs[i];
			Variant result;
			GDExtensionCallError error;
			variant.callp(StringName(method), argptrs.data(), argc, result, error);
			if (error.error != GDEXTENSION_CALL_OK)
			{
				const char* typeName = variant.get_type_name(variant.get_type()).to_lower().utf8().get_data();
				ErrorLog("Runtime", "Backend: [varcall] Execution failed! (m=%s t=%d (%s) e=%d arg=%d exp=%d)", method, variant.get_type(), typeName, error.error, error.argument, error.expected);
				return Variant::NIL;
			}
			return result;
		}
		BLADESYSAPI Variant blade_ucall(const char* name, uint32_t hash, int returnType, int argc, Variant* args)
		{
			// Utility Function Cache Storage
			static std::unordered_map<uint32_t, GDExtensionPtrUtilityFunction> utilityCache;

			// Get Utility Function Pointer
			auto it = utilityCache.find(hash);
			GDExtensionPtrUtilityFunction func = nullptr;
			if (it != utilityCache.end())
			{
				func = it->second;
			}
			else
			{
				func = GDX_GET_UTILITY_FUNC_PTR(StringName(name)._native_ptr(), hash);
				if (func)
				{
					utilityCache[hash] = func;
				}
				else
				{
					ErrorLog("Runtime", "Backend: [utilcall] Invalid Utility, Requested Utility Function Doesn't Exist.");
					return Variant::NIL;
				}
			}

			// Create Arguments
			std::vector<uint64_t> encoded_args(argc);
			std::vector<GDExtensionConstTypePtr> mb_args(argc);
			for (int i = 0; i < argc; ++i) mb_args[i] = EncodeUtilityFunctionArgument(args[i], encoded_args[i]);
			
			// Perform Call And Return
			return DecodeUtilityFunctionReturn((Variant::Type)returnType, func, mb_args.data(), argc);
		}
		BLADESYSAPI Variant blade_eval(Variant a, Variant b, Variant::Operator op)
		{
			Variant r;
			bool ok = false;
			Variant::evaluate(op, a, b, r, ok);
			return r;
		}
		BLADESYSAPI BladeObject* blade_birth(BladeObject* bladeObj)
		{
			// Create Object
			Object* godotObj = nullptr;
			if (bladeObj->singleton) godotObj = Engine::get_singleton()->get_singleton(StringName(bladeObj->type));
			else
			{
				Variant newObject = ClassDB::instantiate(StringName(bladeObj->type));
				if (newObject.get_type() == Variant::NIL) return nullptr;

				Object* obj = newObject;
				if (bladeObj->ref)
				{
					RefCounted* rc = Object::cast_to<RefCounted>(obj);
					if (!rc) return nullptr;
					rc->reference();
					godotObj = rc;
				}
				else
				{
					godotObj = obj;
				}
			}

			// Create Wrapper
			BladeObject* bobj = new BladeObject();
			bobj->ptr = godotObj;
			strcpy_s(bobj->type, sizeof bobj->type, bladeObj->type);
			bobj->ref = bladeObj->ref;
			bobj->singleton = bladeObj->singleton;
			return bobj;
		}
		BLADESYSAPI void blade_death(BladeObject* bobj)
		{
			if (!bobj) return;
			Object* obj = static_cast<Object*>(bobj->ptr);
			if (bobj->singleton)
			{
				delete bobj;
				return;
			}
			if (bobj->ref)
			{
				RefCounted* rc = Object::cast_to<RefCounted>(obj);
				if (rc) rc->unreference();
			}
			else
			{
				if (obj)
				{
					if (obj->is_class("Node"))
					{
						Node* node = static_cast<Node*>(bobj->ptr);
						node->queue_free();
					}
					else
					{
						// Todo : Validate This Mode
						GDX_DESTROY_OBJECT(obj);
					}
				}
			}
			delete bobj;
		}
		BLADESYSAPI bool blade_cond(Variant opResult)
		{
			if (opResult.get_type() == Variant::BOOL)
			{
				return bool(opResult);
			}
			return false;
		}

		// Variant Creators
		BLADESYSAPI Variant blade_make_string(const char* str)
		{
			return Variant(str);
		}
		BLADESYSAPI Variant blade_make_int32(int32_t v)
		{
			return Variant(v);
		}
		BLADESYSAPI Variant blade_make_int64(int64_t v)
		{
			return Variant(v);
		}
		BLADESYSAPI Variant blade_make_float(float v)
		{
			return Variant(v);
		}
		BLADESYSAPI Variant blade_make_double(double v)
		{
			return Variant(v);
		}
		BLADESYSAPI Variant blade_make_bool(int v)
		{
			return Variant(bool(v));
		}
		BLADESYSAPI Variant blade_make_nil()
		{
			return Variant::NIL;
		}
		BLADESYSAPI Variant blade_make_object(BladeObject* bobj)
		{
			return Variant((Object*)bobj->ptr);
		}
		BLADESYSAPI Variant blade_make_array(Variant arraySize)
		{
			Array newArray;
			newArray.resize(arraySize);
			return Variant(newArray);
		}
		BLADESYSAPI Variant blade_make_dict()
		{
			return Variant(Dictionary());
		}
		BLADESYSAPI Variant blade_make_bytes(const uint8_t* data, int len)
		{
			PackedByteArray arr;
			arr.resize(len);
			memcpy(arr.ptrw(), data, len);
			return Variant(arr);
		}

		// Math Types Creators
		BLADESYSAPI Variant blade_make_vector2(Variant x, Variant y)
		{
			return Variant(Vector2(x, y));
		}
		BLADESYSAPI Variant blade_make_vector2i(Variant x, Variant y)
		{
			return Variant(Vector2i(x, y));
		}
		BLADESYSAPI Variant blade_make_vector3(Variant x, Variant y, Variant z)
		{
			return Variant(Vector3(x, y, z));
		}
		BLADESYSAPI Variant blade_make_vector3i(Variant x, Variant y, Variant z)
		{
			return Variant(Vector3i(x, y, z));
		}
		BLADESYSAPI Variant blade_make_vector4(Variant x, Variant y, Variant z, Variant w)
		{
			return Variant(Vector4(x, y, z, w));
		}
		BLADESYSAPI Variant blade_make_vector4i(Variant x, Variant y, Variant z, Variant w)
		{
			return Variant(Vector4i(x, y, z, w));
		}
		BLADESYSAPI Variant blade_make_color_rgb(Variant r, Variant g, Variant b)
		{
			return Variant(Color(r, g, b));
		}
		BLADESYSAPI Variant blade_make_color_rgba(Variant r, Variant g, Variant b, Variant a)
		{
			return Variant(Color(r, g, b, a));
		}
		BLADESYSAPI Variant blade_make_color_hash(Variant hash)
		{
			return Variant(Color(String(hash)));
		}
		BLADESYSAPI Variant blade_make_rect2(Variant x, Variant y, Variant w, Variant h)
		{
			return Variant(Rect2(x, y, w, h));
		}
		BLADESYSAPI Variant blade_make_transform2d(Variant rot, Variant v2_pos)
		{
			return Variant(Transform2D(rot, v2_pos));
		}
		BLADESYSAPI Variant blade_make_transform3d(Variant basis, Variant v3_origin)
		{
			return Variant(Transform3D(basis, v3_origin));
		}
		BLADESYSAPI Variant blade_make_quaternion(Variant x, Variant y, Variant z, Variant w)
		{
			return Variant(Quaternion(x, y, z, w));
		}
		BLADESYSAPI Variant blade_make_plane(Variant a, Variant b, Variant c, Variant d)
		{
			return Variant(Plane(a, b, c, d));
		}
		BLADESYSAPI Variant blade_make_aabb(Variant v3_pos, Variant v3_size)
		{
			return Variant(AABB(v3_pos, v3_size));
		}
		BLADESYSAPI Variant blade_make_basis(Variant v3_x_axis, Variant v3_y_axis, Variant v3_z_axis)
		{
			/* Note : Need to use Vector3() Wrapper Because Compiler Can't Detect Correct Type :) */
			return Variant(Basis(Vector3(v3_x_axis), Vector3(v3_y_axis), Vector3(v3_z_axis)));
		}
	
		// Typed Array Creators
		BLADESYSAPI Variant blade_make_byte_array()
		{
			return Variant(PackedByteArray());
		}
		BLADESYSAPI Variant blade_make_int32_array()
		{
			return Variant(PackedInt32Array());
		}
		BLADESYSAPI Variant blade_make_int64_array()
		{
			return Variant(PackedInt64Array());
		}
		BLADESYSAPI Variant blade_make_float32_array()
		{
			return Variant(PackedFloat32Array());
		}
		BLADESYSAPI Variant blade_make_float64_array()
		{
			return Variant(PackedFloat64Array());
		}
		BLADESYSAPI Variant blade_make_string_array()
		{
			return Variant(PackedStringArray());
		}
		BLADESYSAPI Variant blade_make_vector2_array()
		{
			return Variant(PackedVector2Array());
		}
		BLADESYSAPI Variant blade_make_vector3_array()
		{
			return Variant(PackedVector3Array());
		}
		BLADESYSAPI Variant blade_make_vector4_array()
		{
			return Variant(PackedVector4Array());
		}
		BLADESYSAPI Variant blade_make_color_array()
		{
			return Variant(PackedColorArray());
		}
	
		// C Native Type Casters
		BLADESYSAPI bool blade_c_bool(Variant v)
		{
			return bool(v);
		}
		BLADESYSAPI char blade_c_char(Variant v)
		{
			return char(int(v));
		}
		BLADESYSAPI int8_t blade_c_int8(Variant v)
		{
			return int8_t(v);
		}
		BLADESYSAPI uint8_t blade_c_uint8(Variant v)
		{
			return uint8_t(v);
		}
		BLADESYSAPI int16_t blade_c_int16(Variant v)
		{
			return int16_t(v);
		}
		BLADESYSAPI uint16_t blade_c_uint16(Variant v)
		{
			return uint16_t(v);
		}
		BLADESYSAPI int32_t blade_c_int32(Variant v)
		{
			return int32_t(v);
		}
		BLADESYSAPI uint32_t blade_c_uint32(Variant v)
		{
			return uint32_t(v);
		}
		BLADESYSAPI int64_t blade_c_int64(Variant v)
		{
			return int64_t(v);
		}
		BLADESYSAPI uint64_t blade_c_uint64(Variant v)
		{
			return uint64_t(uint64_t(v));
		}
		BLADESYSAPI float blade_c_float(Variant v)
		{
			return float(v);
		}
		BLADESYSAPI double blade_c_double(Variant v)
		{
			return double(v);
		}
		BLADESYSAPI const char* blade_c_str(Variant v)
		{
			static thread_local std::string buffers[20];
			static thread_local int index = 0;
			buffers[index] = GetStdStr(v);
			const char* result = buffers[index].c_str();
			index = (index + 1) % 20;
			return result;
		}
		BLADESYSAPI void* blade_c_ptr(Variant v)
		{
			switch (v.get_type())
			{
				case Variant::OBJECT:
				{
					Object* obj = v.operator Object*();
					return obj;
				}
				case Variant::ARRAY:
				{
					Array arr = v;
					if (arr.size() > 0) return &arr[0];
					return nullptr;
				}
				default:
				{
					return nullptr;
				}
			}
		}
		BLADESYSAPI const void* blade_c_const_ptr(Variant v)
		{
			switch (v.get_type())
			{
				case Variant::OBJECT:
				{
					Object* obj = v.operator Object*();
					return static_cast<const void*>(obj);
				}
				case Variant::ARRAY:
				{
					Array arr = v;
					if (arr.size() > 0) return &arr[0];
					return nullptr;
				}
				default:
				{
					return nullptr;
				}
			}
		}
		BLADESYSAPI uintptr_t blade_c_handle(Variant v)
		{
			int64_t raw = int64_t(v);
			return static_cast<uintptr_t>(raw);
		}
		BLADESYSAPI const uintptr_t blade_c_const_handle(Variant v)
		{
			int64_t raw = int64_t(v);
			return static_cast<const uintptr_t>(raw);
		}
	
		// Misc Functions
		BLADESYSAPI void blade_chkstk()
		{
			/* Stack Maximum Size Can Be Set in x86_64-gen.c */
		}
	}
	namespace helpers
	{
		BLADESYSAPI Variant blade_format(Variant format, int argc, Variant* args)
		{
			String fmt = String(format);
			String out;
			int arg_index = 0;
			for (int i = 0; i < fmt.length(); i++)
			{
				char32_t c = fmt[i];
				if (c != '%')
				{
					out += String::chr(c);
					continue;
				}
				if (i + 1 < fmt.length() && fmt[i + 1] == '%')
				{
					out += "%";
					i++;
					continue;
				}
				if (arg_index >= argc)
				{
					out += "<missing>";
					continue;
				}
				Variant& v = args[arg_index++];
				i++;
				bool zero_pad = false;
				int width = 0;
				int precision = -1;
				if (fmt[i] == '0')
				{
					zero_pad = true;
					i++;
				}
				while (i < fmt.length() && fmt[i] >= '0' && fmt[i] <= '9')
				{
					width = width * 10 + (fmt[i] - '0');
					i++;
				}
				if (i < fmt.length() && fmt[i] == '.')
				{
					i++;
					precision = 0;
					while (i < fmt.length() && fmt[i] >= '0' && fmt[i] <= '9')
					{
						precision = precision * 10 + (fmt[i] - '0');
						i++;
					}
				}
				String spec;
				int remaining = fmt.length() - i;
				if (remaining >= 3)
				{
					String s3 = fmt.substr(i, 3);
					if (s3 == "v2i" || s3 == "v3i") spec = s3;
				}
				if (spec.is_empty() && remaining >= 2)
				{
					String s2 = fmt.substr(i, 2);
					if (s2 == "lf" || s2 == "v2" || s2 == "v3") spec = s2;
				}
				if (spec.is_empty() && remaining >= 1) spec = fmt.substr(i, 1);
				String s;
				if (spec == "b")
				{
					bool bv = v;
					s = bv ? "True" : "False";
					i += 0;
				}
				else if (spec == "d")
				{
					int64_t iv = v;
					s = String::num_int64(iv);
					i += 0;
				}
				else if (spec == "f")
				{
					float fv = float(v);
					s = (precision >= 0) ? String::num(fv, precision) : String::num(fv);
					i += 0;
				}
				else if (spec == "lf")
				{
					double dv = double(v);
					s = (precision >= 0) ? String::num(dv, precision) : String::num(dv);
					i += 1;
				}
				else if (spec == "s")
				{
					s = v.operator String();
					i += 0;
				}
				else if (spec == "v2")
				{
					Vector2 vec = v;
					s = "(" + String::num(vec.x) + ", " + String::num(vec.y) + ")";
					i += 1;
				}
				else if (spec == "v3")
				{
					Vector3 vec = v;
					s = "(" + String::num(vec.x) + ", " + String::num(vec.y) + ", " + String::num(vec.z) + ")";
					i += 1;
				}
				else if (spec == "v2i")
				{
					Vector2i vec = v;
					s = "(" + itos(vec.x) + ", " + itos(vec.y) + ")";
					i += 2;
				}
				else if (spec == "v3i")
				{
					Vector3i vec = v;
					s = "(" + itos(vec.x) + ", " + itos(vec.y) + ", " + itos(vec.z) + ")";
					i += 2;
				}
				else
				{
					s = v.stringify();
					i -= 1;
				}
				if (width > s.length())
				{
					int pad = width - s.length();
					String padstr;
					padstr.resize(pad);
					for (int k = 0; k < pad; k++) padstr[k] = zero_pad ? '0' : ' ';
					s = padstr + s;
				}
				out += s;
			}
			return Variant(out);
		}
		BLADESYSAPI void blade_print(Variant format, int argc, Variant* args)
		{
			String out = blade_format(format, argc, args);
			UtilityFunctions::print_rich(out);
		}
		BLADESYSAPI BladeObject blade_get_singleton(BladeObject* bladeObj)
		{
			StringName singleton = StringName(bladeObj->type);
			if (!Engine::get_singleton()->has_singleton(singleton))
			{
				ErrorLog("Runtime", "Singleton [%s] Doesn't Exist.", bladeObj->type);
				return BladeObject();
			}

			// Get Singleton
			Object* obj = nullptr;
			if (singleton == StringName("Time"))
			{
				/* Somehow, Getting Time Singleton from Engine::get_singleton leads to crash at exit in Debug Mode :| */
				obj = Time::get_singleton();
			}
			else
			{
				obj = Engine::get_singleton()->get_singleton(singleton);
			}

			// Validate Object
			if (!obj)
			{
				ErrorLog("Runtime", "Failed to Obtain Singleton [%s].", bladeObj->type);
				return BladeObject();
			}
			
			// Create Wrapper
			BladeObject bobj;
			bobj.ptr = obj;
			strcpy_s(bobj.type, sizeof bobj.type, bladeObj->type);
			bobj.ref = bladeObj->ref;
			bobj.singleton = bladeObj->singleton;
			return bobj;
		}
		BLADESYSAPI BladeObject blade_get_object(Variant instance, BladeObject* bladeObj)
		{
			// Get Object
			Object* obj = instance.operator Object*();
			if (!obj)
			{
				ErrorLog("Runtime", "Failed to Convert Instance Blade Object [%s]. Invalid Pointer.", bladeObj->type);
				return BladeObject();
			}

			// Create Wrapper
			BladeObject bobj;
			bobj.ptr = obj;
			strcpy_s(bobj.type, sizeof bobj.type, bladeObj->type);
			bobj.ref = bladeObj->ref;
			bobj.singleton = bladeObj->singleton;
			return bobj;
		}
		BLADESYSAPI Variant blade_make_callable(BladeObject* bladeObj, Variant functionName)
		{
			// Get Object
			Object* obj = static_cast<Object*>(bladeObj->ptr);
			
			// Create New Callable
			return Callable(obj, String(functionName));
		}
		BLADESYSAPI void* blade_load_library(Variant libraryName)
		{
			#ifdef PLATFORM_WINDOWS
				return (void*)LoadLibraryA(GetStdStr(libraryName).c_str());
			#endif
			return nullptr;
		}
		BLADESYSAPI void* blade_get_function(void* library, Variant functionName)
		{
			#ifdef PLATFORM_WINDOWS
				return (void*)GetProcAddress(HMODULE(library), GetStdStr(functionName).c_str());
			#endif
			return nullptr;
		}
	}
}

// Blade Syntax, Configuration & Parser
namespace blade
{
	namespace configuration
	{
		static bool bl_print_banner				= false;
		static bool bl_use_64bit_integer		= true;
		static bool bl_use_64bit_float			= true;
		static bool bl_support_structs			= true;
		static bool bl_use_mini_constructors	= true;
		static bool bl_gen_utility_functions	= true;

		static int bl_max_feature_line_limit	= 50;
	
		static Color bl_typeColor          		= ColorFromHex("#ffc9b4");
		static Color bl_keywordColor          	= ColorFromHex("#ffa978");
		static Color bl_controlFlowColor      	= ColorFromHex("#7cf0ff");
		static Color bl_classColor            	= ColorFromHex("#ffd78b");
		static Color bl_booleanColor          	= ColorFromHex("#4cb8e6");
		static Color bl_utilsColor          	= ColorFromHex("#beacff");	
		static Color bl_stringColor           	= ColorFromHex("#ff3758");
		static Color bl_functionColor         	= ColorFromHex("#a3ff9b");
		static Color bl_numberColor           	= ColorFromHex("#ca88ff");
		static Color bl_symbolColor           	= ColorFromHex("#d1fff0");
		static Color bl_memberVariableColor   	= ColorFromHex("#9f9aff");
		static Color bl_variantAccessColor    	= ColorFromHex("#ffbf47");
		static Color bl_objectAccessColor		= ColorFromHex("#47ff6f");
		static Color bl_propOptionColor       	= ColorFromHex("#98ffc7");
		static Color bl_propOptionValueColor  	= ColorFromHex("#ffe498");
		static Color bl_switchesColor 			= ColorFromHex("#ff7b23");
		static Color bl_commentColor          	= ColorFromHex("#c2c2c296");
	}
	namespace syntax
	{
		// Definer Keywords
		static std::string bl_inherits			= "inherits";
		static std::string bl_class				= "class";
		static std::string bl_tool				= "tool";
		static std::string bl_use				= "use";
		static std::string bl_property			= "prop";
		static std::string bl_function			= "func";
		static std::string bl_signal			= "signal";
		static std::string bl_ext_obj			= "ext_obj";
		static std::string bl_ext_var			= "ext_var";
		static std::string bl_key_def			= "key_def";
		static std::string bl_blade_on			= "blade_on";
		static std::string bl_blade_off			= "blade_off";

		// Definer Keywords IDs
		enum class DefinerKeyword : uint8_t
		{
			dk_inherits,
			dk_class,
			dk_tool,
			dk_use,
			dk_property,
			dk_function,
			dk_signal,
			dk_ext_obj,
			dk_ext_var,
			dk_key_def,
			dk_blade_on,
			dk_blade_off
		};

		// Definer Keywords Accessors
		static String GetDefinerKeyword(DefinerKeyword dk)
		{
			switch (dk)
			{
			case blade::syntax::DefinerKeyword::dk_inherits:
				return Format("%s ", bl_inherits.c_str());
			case blade::syntax::DefinerKeyword::dk_class:
				return Format("%s ", bl_class.c_str());
			case blade::syntax::DefinerKeyword::dk_tool:
				return Format("%s ", bl_tool.c_str());
			case blade::syntax::DefinerKeyword::dk_use:
				return Format("%s ", bl_use.c_str());
			case blade::syntax::DefinerKeyword::dk_property:
				return Format("%s ", bl_property.c_str());
			case blade::syntax::DefinerKeyword::dk_function:
				return Format("%s ", bl_function.c_str());
			case blade::syntax::DefinerKeyword::dk_signal:
				return Format("%s ", bl_signal.c_str());
			case blade::syntax::DefinerKeyword::dk_ext_obj:
				return Format("%s ", bl_ext_obj.c_str());
			case blade::syntax::DefinerKeyword::dk_ext_var:
				return Format("%s ", bl_ext_var.c_str());
			case blade::syntax::DefinerKeyword::dk_key_def:
				return Format("%s ", bl_key_def.c_str());
			case blade::syntax::DefinerKeyword::dk_blade_on:
				return Format("%s", bl_blade_on.c_str());
			case blade::syntax::DefinerKeyword::dk_blade_off:
				return Format("%s", bl_blade_off.c_str());
			default:
				return "non ";
			}
		}
	
		// Global Functions Keywords
		static std::string bl_birth				= "birth";
		static std::string bl_death				= "death";
		static std::string bl_print				= "print";
		static std::string bl_format			= "format";
		static std::string bl_get_singleton		= "get_singleton";
		static std::string bl_get_object		= "get_object";
		static std::string bl_make_callable		= "callable";
		static std::string bl_load_library		= "load_library";
		static std::string bl_get_function		= "get_function";


		// Global Functions IDs
		enum class GlobalFunctions : uint8_t
		{
			gf_birth,
			gf_death,
			gf_print,
			gf_format,
			gf_get_singleton,
			gf_get_object,
			gf_make_callable,
			gf_load_library,
			gf_get_function,
		};
	
		// Misc Keywords
		static std::string bl_this				= "this";
		static std::string bl_self				= "self";
		static std::string bl_return			= "return";
		static std::string bl_continue			= "continue";
		static std::string bl_break				= "break";
		static std::string bl_true				= "true";
		static std::string bl_false				= "false";
		static std::string bl_null				= "null";

		// Misc Keywords IDs
		enum class MiscKeywords : uint8_t
		{
			mk_this,
			mk_self,
			mk_return,
			mk_continue,
			mk_break,
			mk_true,
			mk_false,
			mk_null
		};
	}
	namespace tast
	{
		/* Tiny AST (TAST) Parser for Blade - v1.0*/

		// Enumerators
		enum TokenKind
		{
			TOK_EOF,
			TOK_IDENT,
			TOK_NUMBER,
			TOK_OP,
			TOK_LPAREN,
			TOK_RPAREN,
			TOK_DOT,
			TOK_FIELD,
			TOK_STRING
		};
		enum ExprKind
		{
			EXPR_LITERAL,
			EXPR_IDENT,
			EXPR_UNARY,
			EXPR_BINARY,
			EXPR_MEMBER,
			EXPR_FIELD,
			EXPR_CALL,
			EXPR_OBJCALL,
			EXPR_VARCALL,
			EXPR_POST_INCREMENT,
			EXPR_POST_DECREMENT
		};

		// Structures
		struct Token
		{
			TokenKind kind;
			String text;
		};
		struct Expr
		{
			ExprKind kind;
			String op;
			String value;
			Expr* left;
			Expr* right;
			Vector<Expr*> arguments;
			Expr() : kind(EXPR_LITERAL), left(nullptr), right(nullptr) {}
		};

		// Tiny Lexer & Tiny Parser
		class Lexer
		{
		public:
			Lexer(const String& s)
			{
				src = s;
				pos = 0;
				len = s.length();
			}
			bool eof() const { return pos >= len; }
			char peek() const { return eof() ? 0 : src[pos]; }
			char get() { return eof() ? 0 : src[pos++]; }
			void SkipWS()
			{
				while (!eof() && isspace(peek())) pos++;
			}
			Token next()
			{
				SkipWS();
				if (eof())
				{
					Token t;
					t.kind = TOK_EOF;
					return t;
				}
				char c = peek();
				if (isalpha(c) || c == '_' || c == '$')
				{
					int start = pos;
					get();
					while (!eof())
					{
						char d = peek();
						if (isalnum(d) || d == '_' || d == '$') get();
						else break;
					}
					Token t;
					t.kind = TOK_IDENT;
					t.text = src.substr(start, pos - start);
					return t;
				}
				if (isdigit(c) || (c == '.' && pos + 1 < len && isdigit(src[pos + 1])))
				{
					int start = pos;
					get();
					while (!eof())
					{
						char d = peek();
						if (isalnum(d) || d == '.' || d == 'f' || d == 'F') get();
						else break;
					}
					Token t;
					t.kind = TOK_NUMBER;
					t.text = src.substr(start, pos - start);
					return t;
				}
				if (c == '"' || c == '\'')
				{
					char quote = get();
					int start = pos;
					bool escaped = false;
					while (!eof())
					{
						char d = get();
						if (!escaped && d == quote) break;
						if (d == '\\' && !escaped) escaped = true;
						else escaped = false;
					}
					String text = src.substr(start - 1, pos - (start - 1));
					Token t;
					t.kind = TOK_STRING;
					t.text = text;
					return t;
				}
				if (c == '.')
				{
					get();
					Token t;
					t.kind = TOK_DOT;
					t.text = ".";
					return t;
				}
				if (c == ':')
				{
					get();
					Token t;
					t.kind = TOK_FIELD;
					t.text = ":";
					return t;
				}
				if (c == '(')
				{
					get();
					Token t;
					t.kind = TOK_LPAREN;
					t.text = "(";
					return t;
				}
				if (c == ')')
				{
					get();
					Token t;
					t.kind = TOK_RPAREN;
					t.text = ")";
					return t;
				}
				if (c == '-' && pos + 1 < len && src[pos + 1] == '>')
				{
					get();
					get();
					Token t;
					t.kind = TOK_OP;
					t.text = "->";
					return t;
				}
				if (c == '<' && pos + 1 < len && src[pos + 1] == '-')
				{
					get();
					get();
					Token t;
					t.kind = TOK_OP;
					t.text = "<-";
					return t;
				}
				int start = pos;
				get();
				if (!eof())
				{
					String two = String::chr(src[start]) + String::chr(src[pos]);
					if (two == "==" || two == "!=" || two == "<=" || two == ">=" ||
						two == "&&" || two == "||" || two == "<<" || two == ">>")
					{
						get();
						Token t;
						t.kind = TOK_OP;
						t.text = two;
						return t;
					}
				}
				Token t;
				t.kind = TOK_OP;
				t.text = String::chr(src[start]);
				return t;
			}
		
		private:
			String src;
			int pos = 0;
			int len = 0;
		};

		// Tiny Parser
		class Parser
		{
		public:
			Parser(const String& s) : lex(s)
			{
				cur = lex.next();
			}
			void advance() { cur = lex.next(); }
			bool match(TokenKind k, const String& txt = String())
			{
				if (cur.kind != k) return false;
				if (txt.length() && cur.text != txt) return false;
				return true;
			}
			Expr* parse_primary()
			{
				Expr* e = nullptr;

				if (match(TOK_IDENT))
				{
					e = memnew(Expr);
					e->kind = EXPR_IDENT;
					e->value = cur.text;
					advance();
				}
				else if (match(TOK_NUMBER))
				{
					e = memnew(Expr);
					e->kind = EXPR_LITERAL;
					e->value = cur.text;
					advance();
				}
				else if (match(TOK_STRING))
				{
					e = memnew(Expr);
					e->kind = EXPR_LITERAL;
					e->value = cur.text;
					advance();
				}
				else if (match(TOK_LPAREN))
				{
					advance();
					e = parse_expr(0);
					if (match(TOK_RPAREN)) advance();
				}
				return e ? parse_postfix(e) : nullptr;
			}
			Expr* parse_postfix(Expr* expr)
			{
				if (!expr) return nullptr;
				while (true)
				{
					if (match(TOK_OP) && cur.text == "->")
					{
						advance();
						if (!match(TOK_IDENT)) break;
						Expr* e = memnew(Expr);
						e->kind = EXPR_OBJCALL;
						e->left = expr;
						e->value = cur.text;
						advance();
						expr = e;
						continue;
					}
					if (match(TOK_OP) && cur.text == "<-")
					{
						advance();
						if (!match(TOK_IDENT)) break;
						Expr* e = memnew(Expr);
						e->kind = EXPR_VARCALL;
						e->left = expr;
						e->value = cur.text;
						advance();
						expr = e;
						continue;
					}
					if (match(TOK_LPAREN))
					{
						advance();
						Expr* e = memnew(Expr);
						e->kind = EXPR_CALL;
						e->left = expr;
						if (!match(TOK_RPAREN))
						{
							while (true)
							{
								Expr* arg = parse_expr(0);
								if (!arg) break;
								e->arguments.push_back(arg);
								if (match(TOK_OP) && cur.text == ",")
								{
									advance();
									continue;
								}
								break;
							}
						}
						if (match(TOK_RPAREN)) advance();
						expr = e;
						continue;
					}
					if (match(TOK_OP))
					{
						if (cur.text == "++")
						{
							advance();
							Expr* e = memnew(Expr);
							e->kind = EXPR_POST_INCREMENT;
							e->left = expr;
							expr = e;
							continue;
						}
						if (cur.text == "--")
						{
							advance();
							Expr* e = memnew(Expr);
							e->kind = EXPR_POST_DECREMENT;
							e->left = expr;
							expr = e;
							continue;
						}
					}
					if (match(TOK_DOT))
					{
						advance();
						if (!match(TOK_IDENT)) break;

						Expr* e = memnew(Expr);
						e->kind = EXPR_MEMBER;
						e->left = expr;
						e->value = cur.text;
						advance();
						expr = e;
						continue;
					}
					if (match(TOK_FIELD))
					{
						advance();
						if (!match(TOK_IDENT)) break;

						Expr* e = memnew(Expr);
						e->kind = EXPR_FIELD;
						e->left = expr;
						e->value = cur.text;
						advance();
						expr = e;
						continue;
					}
					break;
				}
				return expr;
			}
			Expr* parse_unary()
			{
				if (match(TOK_OP) && (cur.text == "-" || cur.text == "+" || cur.text == "!" || cur.text == "~"))
				{
					String op = cur.text;
					advance();
					Expr* rhs = parse_unary();
					if (!rhs) return nullptr;
					Expr* e = memnew(Expr);
					e->kind = EXPR_UNARY;
					e->op = op;
					e->right = rhs;
					return e;
				}
				return parse_primary();
			}
			Expr* parse_expr(int min_prec)
			{
				Expr* lhs = parse_unary();
				if (!lhs) return nullptr;
				auto OperatorPrecedence = [](const String & op) -> int
				{
					if (op == "||") return 1;
					if (op == "&&") return 2;
					if (op == "|") return 3;
					if (op == "^") return 4;
					if (op == "&") return 5;
					if (op == "==" || op == "!=") return 6;
					if (op == "<" || op == "<=" || op == ">" || op == ">=") return 7;
					if (op == "<<" || op == ">>") return 8;
					if (op == "+" || op == "-") return 9;
					if (op == "*" || op == "/" || op == "%") return 10;
					return 0;
				};
				while (match(TOK_OP))
				{
					String op = cur.text;
					int prec = OperatorPrecedence(op);
					if (prec < min_prec || prec == 0) break;
					advance();
					Expr* rhs = parse_expr(prec + 1);
					if (!rhs) return lhs;
					Expr* e = memnew(Expr);
					e->kind = EXPR_BINARY;
					e->op = op;
					e->left = lhs;
					e->right = rhs;
					lhs = e;
				}
				return lhs;
			}

		private:
			Lexer lex;
			Token cur;
		};

		// Expression Emitter
		static String EmitExper(Expr* e)
		{
			if (!e) return String();
			auto OperatorEnum = [](const String& op, bool unary) -> String
			{
				if (unary)
				{
					if (op == "-") return "OP_NEGATE";
					if (op == "+") return "OP_POSITIVE";
					if (op == "!") return "OP_NOT";
					if (op == "~") return "OP_BIT_NEGATE";
				}
				else
				{
					if (op == "+") return "OP_ADD";
					if (op == "-") return "OP_SUBTRACT";
					if (op == "*") return "OP_MULTIPLY";
					if (op == "/") return "OP_DIVIDE";
					if (op == "%") return "OP_MODULE";
					if (op == "==") return "OP_EQUAL";
					if (op == "!=") return "OP_NOT_EQUAL";
					if (op == "<") return "OP_LESS";
					if (op == "<=") return "OP_LESS_EQUAL";
					if (op == ">") return "OP_GREATER";
					if (op == ">=") return "OP_GREATER_EQUAL";
					if (op == "&&") return "OP_AND";
					if (op == "||") return "OP_OR";
					if (op == "&") return "OP_BIT_AND";
					if (op == "|") return "OP_BIT_OR";
					if (op == "^") return "OP_BIT_XOR";
					if (op == "<<") return "OP_SHIFT_LEFT";
					if (op == ">>") return "OP_SHIFT_RIGHT";
				}
				return "OP_MAX";
			};
			if (e->kind == EXPR_LITERAL || e->kind == EXPR_IDENT)
			{
				return e->value;
			}
			if (e->kind == EXPR_UNARY)
			{
				String a = EmitExper(e->right);
				return "eval(" + a + ", " + GetStr(blade::syntax::bl_null) + ", " + OperatorEnum(e->op, true) + ")";
			}
			if (e->kind == EXPR_BINARY)
			{
				String a = EmitExper(e->left);
				String b = EmitExper(e->right);
				return "eval(" + a + ", " + b + ", " + OperatorEnum(e->op, false) + ")";
			}
			if (e->kind == EXPR_MEMBER)
			{
				return EmitExper(e->left) + "." + e->value;
			}
			if (e->kind == EXPR_FIELD)
			{
				return EmitExper(e->left) + ":" + e->value;
			}
			if (e->kind == EXPR_CALL)
			{
				String callee = EmitExper(e->left);
				String args_str;
				for (int j = 0; j < e->arguments.size(); ++j)
				{
					if (j > 0) args_str += ", ";
					args_str += EmitExper(e->arguments[j]);
				}
				return callee + "(" + args_str + ")";
			}
			if (e->kind == EXPR_OBJCALL)
			{
				return EmitExper(e->left) + "->" + e->value;
			}
			if (e->kind == EXPR_VARCALL)
			{
				return EmitExper(e->left) + "<-" + e->value;
			}
			if (e->kind == EXPR_POST_INCREMENT)
			{
				String var = EmitExper(e->left);
				return var + " = eval(" + var + ", 1, OP_ADD)";
			}
			if (e->kind == EXPR_POST_DECREMENT)
			{
				String var = EmitExper(e->left);
				return var + " = eval(" + var + ", 1, OP_SUBTRACT)";
			}
			return String();
		}
		static String RewriteExprToEval(const String& expr)
		{
			Parser p(expr);
			Expr* root = p.parse_expr(0);
			if (!root) return expr;
			return EmitExper(root);
		}

		// Processing Passes
        static String ProcessSourceExpressions(const String& src)
        {
            using namespace blade::syntax;
            enum class Context
            {
                None,
                IfWhile,
                ForLoop
            };
            String out;
            int len = src.length();
            int i = 0;
            bool transpileEnabled = true;
            auto IsIdentChar = [&](char c) -> bool
            {
                return isalnum(c) || c == '_' || c == '$';
            };
            auto FindMatchingBracket = [&](int open_pos) -> int
            {
                int depth = 1;
                for (int j = open_pos + 2; j < len; ++j)
                {
                    char c = src[j];
                    if (c == '[') depth++;
                    else if (c == ']')
                    {
                        depth--;
                        if (depth == 0) return j;
                    }
                }
                return -1;
            };
            auto DetectContext = [&](int hash_pos) -> Context
            {
                int p = hash_pos - 1;
                while (p >= 0 && isspace(src[p])) p--;
                if (p < 0 || src[p] != '(') return Context::None;
                int k = p - 1;
                while (k >= 0 && isspace(src[k])) k--;
                if (k >= 1 && src[k - 1] == 'i' && src[k] == 'f') return Context::IfWhile;
                if (k >= 4 && src.substr(k - 4, 5) == "while") return Context::IfWhile;
                if (k >= 2 && src.substr(k - 2, 3) == "for") return Context::ForLoop;
                return Context::None;
            };
            auto RewriteForHeader = [&](const String& inside) -> String
            {
                Vector<String> parts;
                int depth = 0;
                int start = 0;
                int n = inside.length();
                for (int k = 0; k < n; ++k)
                {
                    char c = inside[k];
                    if (c == '(') depth++;
                    else if (c == ')') depth--;
                    else if (c == ';' && depth == 0)
                    {
                        parts.push_back(inside.substr(start, k - start).strip_edges());
                        start = k + 1;
                    }
                }
                parts.push_back(inside.substr(start).strip_edges());
                String init  = parts.size() > 0 ? parts[0] : "";
                String cond  = parts.size() > 1 ? parts[1] : "";
                String incr  = parts.size() > 2 ? parts[2] : "";
                if (init.length())
                {
                    String original_init = init.strip_edges();
                    if (original_init.find("=") != -1)
                    {
                        int eq = original_init.find("=");
                        String var = original_init.substr(0, eq).strip_edges();
                        String rhs = original_init.substr(eq + 1).strip_edges();
                        String rhs2 = RewriteExprToEval(rhs);
                        init = var + " = " + rhs2;
                    }
                    else
                    {
                        init = RewriteExprToEval(original_init);
                    }
                }
                if (cond.length())
                {
                    String c2 = RewriteExprToEval(cond);
                    cond = "condition(" + c2 + ")";
                }
                if (incr.length())
                {
                    String original_incr = incr.strip_edges();
                    if (original_incr.ends_with("++") || original_incr.ends_with("--"))
                    {
                        String var_part = original_incr.substr(0, original_incr.length() - 2).strip_edges();
                        String op_const = original_incr.ends_with("++") ? "OP_ADD" : "OP_SUBTRACT";
                        incr = var_part + " = eval(" + var_part + ", 1, " + op_const + ")";
                    }
                    else if (original_incr.find("=") != -1)
                    {
                        int eq = original_incr.find("=");
                        String var = original_incr.substr(0, eq).strip_edges();
                        String rhs = original_incr.substr(eq + 1).strip_edges();
                        String rhs2 = RewriteExprToEval(rhs);
                        incr = var + " = " + rhs2;
                    }
                    else
                    {
                        incr = RewriteExprToEval(original_incr);
                    }
                }
                String rebuilt;
                rebuilt += init;
                rebuilt += "; ";
                rebuilt += cond;
                rebuilt += "; ";
                rebuilt += incr;
                return rebuilt;
            };
            while (i < len)
            {
                if (i + bl_blade_off.length() <= len && src.substr(i, bl_blade_off.length()) == GetDefinerKeyword(DefinerKeyword::dk_blade_off))
                {
                    transpileEnabled = false;
                    out += GetDefinerKeyword(DefinerKeyword::dk_blade_off);
                    i += bl_blade_off.length();
                    continue;
                }
                if (i + bl_blade_on.length() <= len && src.substr(i, bl_blade_on.length()) == GetDefinerKeyword(DefinerKeyword::dk_blade_on))
                {
                    transpileEnabled = true;
                    out += GetDefinerKeyword(DefinerKeyword::dk_blade_on);
                    i += bl_blade_on.length();
                    continue;
                }
                if (!transpileEnabled)
                {
                    out += src[i++];
                    continue;
                }
                if (i + 1 < len && (src.substr(i, 2) == "++" || src.substr(i, 2) == "--"))
                {
                    String op = src.substr(i, 2);
                    int op_pos = i;

                    int var_end = op_pos;
                    int var_start = op_pos - 1;
                    while (var_start >= 0 && IsIdentChar(src[var_start]))
                        var_start--;
                    var_start++;

                    String var = src.substr(var_start, var_end - var_start).strip_edges();
                    if (var.length() > 0)
                    {
                        String op_const = (op == "++") ? "OP_ADD" : "OP_SUBTRACT";
                        out += " = eval(" + var + ", 1, " + op_const + ")";
                        i += 2;
                        continue;
                    }
                }
                if (i + 1 < len && src[i] == '#' && src[i + 1] == '[')
                {
                    int close_pos = FindMatchingBracket(i);
                    if (close_pos != -1)
                    {
                        int expr_start = i + 2;
                        String inside = src.substr(expr_start, close_pos - expr_start);
                        if (inside.find("#[") != -1) ErrorLog("Parser", "Nested #[ ] is not allowed");
                        Context ctx = DetectContext(i);
                        if (ctx == Context::ForLoop)
                        {
                            String rebuilt = RewriteForHeader(inside.strip_edges());
                            out += rebuilt;
                        }
                        else
                        {
                            String rewritten = RewriteExprToEval(inside.strip_edges());
                            if (ctx == Context::IfWhile) out += "condition(" + rewritten + ")";
                            else out += rewritten;
                        }
                        i = close_pos + 1;
                        continue;
                    }
                }
                out += src[i++];
            }
            return out;
        }
	}
}

// Blade Compiler : TinyCC Implementation
class TinyCCompiler : public BladeCompiler
{
protected:
    TCCState* state;

public:
    TinyCCompiler(void* scriptObject)
    {
        state = tcc_new();
        if (!state) return;

        tcc_set_error_func(state, scriptObject, CompilerErrorHandler);
        tcc_set_include_func(state, scriptObject, CompilerIncludeHandler);
        tcc_set_output_type(state, TCC_OUTPUT_MEMORY);
        tcc_set_options(state, "-nostdlib");
    }
    ~TinyCCompiler() override
    {
        if (state)
        {
            tcc_delete(state);
			state = nullptr;
        }
    }

public:
	bool Validate() override
	{
		return state != nullptr;
	}
	bool AddIncludePath(const std::string& includePath) override
	{
		if (!state) return false;
        return tcc_add_include_path(state, includePath.c_str()) == 0;
	}
    bool AddSymbol(const std::string& name, const void* ptr) override
    {
		if (!state) return false;
        return tcc_add_symbol(state, name.c_str(), ptr) == 0;
    }
    bool Compile(const std::string& source) override
    {
		if (!state) return false;
        return tcc_compile_string(state, source.c_str()) != -1;
    }
    bool Prepare() override
    {
		if (!state) return false;
        return tcc_relocate(state, TCC_RELOCATE_AUTO) >= 0;
    }
    void* GetSymbol(const std::string& symbolName) override
    {
		if (!state) return nullptr;
        return tcc_get_symbol(state, symbolName.c_str());
    }

private:
	static void CompilerErrorHandler(void* opaque, const char* msg)
	{
		// Get Script
		BladeScript* script = static_cast<BladeScript*>(opaque);
		std::string scriptFile = GetStdStr(script->get_path());

		// Handle Message
		std::string compilerMessage(msg);
		std::regex pattern(R"(^<[^>]+>:(\d+):\s*(error|warning):\s*(.*)$)");
		std::smatch match;
		if (std::regex_match(compilerMessage, match, pattern))
		{
			int line     		 = atoi(match[1].str().c_str());
			std::string type     = match[2];
			std::string message  = match[3];

			// Ignored Messages
			if (message == "assignment discards qualifiers from pointer target type") return;

			// Error Redirections
			if (message == "operation on a struct") message = "Invalid Expression, Use #[<exp>] instead.";

			// Print Warning & Errors
			if (type == "warning")
			{
				godot::_err_print_error("BladeCompiler", scriptFile.c_str(), line, Format("Warning : %s", message.c_str()), false, true);
			}
			else if (type == "error")
			{
				godot::_err_print_error("BladeCompiler", scriptFile.c_str(), line, Format("Error : %s", message.c_str()));
			}
		}
		else
		{
			// Unhandled Message
			ErrorLog("Compiler", "Unhanled Message : %s", compilerMessage.c_str());
		}
	};
	static const char* CompilerIncludeHandler(void* opaque, const char* filename, size_t* len)
	{
		// Check for Header in Virtual File System
		const jenova::json_t bindingsDatabase = BladeLanguage::get_singleton()->GetBindingsDatabase();
		if (bindingsDatabase.contains(filename))
		{
			DevLog("Loading Header from Database [%s]", filename);
			static std::string sourceContent = "";
			sourceContent = base64::base64_decode(bindingsDatabase[filename].get<std::string>());
			*len = sourceContent.length();
			return sourceContent.c_str();
		}
		
		// Generate from ClassDB

		// Not Handled, Fallback On Disk
		*len = 0;
		return nullptr;
	};
};

// Blade Language Implementation
BladeLanguage* BladeLanguage::get_singleton()
{
	return bladeLanguage;
}
void BladeLanguage::init()
{
	bladeLanguage = memnew(BladeLanguage);
	Engine::get_singleton()->register_script_language(bladeLanguage);
}
void BladeLanguage::deinit()
{
	Engine::get_singleton()->unregister_script_language(bladeLanguage);
	memdelete(bladeLanguage);
}
String BladeLanguage::_get_name() const
{
	return blade::BladeLangaugeName;
}
void BladeLanguage::_init()
{
	// Create Framework Script
	GenerateFrameworkCode();

	// Load Bindings Database
	if (!LoadBindingsDatabase())
	{
		ErrorLog("Bindings Generator", "Bindings Database Failed to Load, Make Sure You Generated It, Use Project > Tools > Generate Blade Bindings Database");
	}

	// Register Remote Message Handler
	EngineDebugger* engineDebugger = EngineDebugger::get_singleton();
	if (engineDebugger) engineDebugger->register_message_capture(String(blade::BladeLangaugeName), callable_mp(this, &BladeLanguage::HandleRemoteMessage));

	// Verbose
	Verbose("[color=#24ffab][Success][/color] Blade Langauge%s Initialized.", IsRuntimeExecution() ? " Runtime" : "");

	// All Good
	bladeLanguage->isInitialized = true;
}
void BladeLanguage::_release()
{
	// Unregister Remote Message Handler
	EngineDebugger* engineDebugger = EngineDebugger::get_singleton();
	if (engineDebugger) engineDebugger->unregister_message_capture(String(blade::BladeLangaugeName));

	// Verbose
	Verbose("[color=#24ffab][Success][/color] Blade Langauge%s Uninitialized Gracefully.", IsRuntimeExecution() ? " Runtime" : "");

	// All Good
	bladeLanguage->isInitialized = false;
}
void BladeLanguage::_finish()
{
}
String BladeLanguage::_get_type() const
{
	return String(blade::BladeScriptType);
}
String BladeLanguage::_get_extension() const
{
	return blade::BladeExtension;
}
PackedStringArray BladeLanguage::_get_reserved_words() const
{
	using namespace blade::syntax;
	static const PackedStringArray reserved_words
	{
		// Blade Meta
		GetStr(bl_inherits), GetStr(bl_class), GetStr(bl_tool), GetStr(bl_use),
		GetStr(bl_property), GetStr(bl_function), GetStr(bl_signal), GetStr(bl_ext_obj), GetStr(bl_ext_var), GetStr(bl_key_def),
		GetStr(bl_blade_on), GetStr(bl_blade_off),

		// Control Flow
		"if", "else", "switch", "case", "default", "while", "do", "for", "__var", "__obj",
		"struct", "typedef", "pragma", "define", "ifdef", "endif", "include", "const",
		GetStr(bl_break), GetStr(bl_continue), GetStr(bl_return),

		// Literals
		GetStr(bl_true), GetStr(bl_false), GetStr(bl_null),

		// Built‑ins
		GetStr(bl_birth), GetStr(bl_death),
		GetStr(bl_this), GetStr(bl_self), GetStr(bl_format), GetStr(bl_print),
		GetStr(bl_get_singleton), GetStr(bl_get_object), GetStr(bl_make_callable), GetStr(bl_load_library), GetStr(bl_get_function)
	};
	return reserved_words;
}
bool BladeLanguage::_is_control_flow_keyword(const String& p_keyword) const
{
	using namespace blade::syntax;
	static const std::unordered_set<std::string> control_flow_keywords
	{
		bl_inherits, bl_class, bl_tool, bl_use,
		bl_property, bl_function, bl_signal, bl_ext_obj, bl_ext_var, bl_key_def,

		"if", "else", "switch", "case", "default", "while", "do", "for",
		bl_break, bl_continue, bl_return,
	};
	return control_flow_keywords.count(GetStdStr(p_keyword)) > 0;
}
PackedStringArray BladeLanguage::_get_comment_delimiters() const
{
	PackedStringArray comment_delimiters;
	comment_delimiters.push_back("//");
	return comment_delimiters;
}
PackedStringArray BladeLanguage::_get_doc_comment_delimiters() const
{
	return PackedStringArray();
}
PackedStringArray BladeLanguage::_get_string_delimiters() const
{
	PackedStringArray string_delimiters;
	string_delimiters.push_back("' '");
	string_delimiters.push_back("\" \"");
	return string_delimiters;
}
Ref<Script> BladeLanguage::_make_template(const String& p_template, const String& p_class_name, const String& p_base_class_name) const
{
	BladeScript* script = memnew(BladeScript);
	return Ref<Script>(script);
}
TypedArray<Dictionary> BladeLanguage::_get_built_in_templates(const StringName& p_object) const
{
	return TypedArray<Dictionary>();
}
bool BladeLanguage::_is_using_templates()
{
	return false;
}
Dictionary BladeLanguage::_validate(const String& p_script, const String& p_path, bool p_validate_functions, bool p_validate_errors, bool p_validate_warnings, bool p_validate_safe_lines) const
{
	/* Called to Check Syntax, Exports etc. */
	return Dictionary();
}
String BladeLanguage::_validate_path(const String& p_path) const
{
	/* Called from Create Dialog to Check for Path */
	return String();
}
Object* BladeLanguage::_create_script() const
{
	BladeScript* script = memnew(BladeScript);
	return script;
}
bool BladeLanguage::_supports_builtin_mode() const
{
	return false;
}
bool BladeLanguage::_supports_documentation() const
{
	return false;
}
bool BladeLanguage::_can_inherit_from_file() const
{
	return false;
}
int32_t BladeLanguage::_find_function(const String& p_function, const String& p_code) const
{
	return -1;
}
String BladeLanguage::_make_function(const String& p_class_name, const String& p_function_name, const PackedStringArray& p_function_args) const
{
	return String();
}
Error BladeLanguage::_open_in_external_editor(const Ref<Script>& p_script, int32_t p_line, int32_t p_column)
{
	return Error::OK;
}
bool BladeLanguage::_overrides_external_editor()
{
	return false;
}
Dictionary BladeLanguage::_complete_code(const String& p_code, const String& p_path, Object* p_owner) const
{
	Dictionary completeCodeResult;
	completeCodeResult["result"] = false;
	completeCodeResult["force"] = false;
	completeCodeResult["call_hint"] = false;
	return Dictionary();
}
Dictionary BladeLanguage::_lookup_code(const String& p_code, const String& p_symbol, const String& p_path, Object* p_owner) const
{
	Dictionary lookUpResult;
	lookUpResult["result"] = false;
	lookUpResult["type"] = GDEXTENSION_VARIANT_TYPE_NIL;
	return lookUpResult;
}
String BladeLanguage::_auto_indent_code(const String& p_code, int32_t p_from_line, int32_t p_to_line) const
{
	return String();
}
bool BladeLanguage::_can_make_function() const
{
	return false;
}
void BladeLanguage::_add_global_constant(const StringName& p_name, const Variant& p_value)
{
}
void BladeLanguage::_add_named_global_constant(const StringName& p_name, const Variant& p_value)
{
}
void BladeLanguage::_remove_named_global_constant(const StringName& p_name)
{
}
void BladeLanguage::_thread_enter()
{
}
void BladeLanguage::_thread_exit()
{
}
String BladeLanguage::_debug_get_error() const
{
	return String();
}
int32_t BladeLanguage::_debug_get_stack_level_count() const
{
	return 0;
}
int32_t BladeLanguage::_debug_get_stack_level_line(int32_t p_level) const
{
	return 0;
}
String BladeLanguage::_debug_get_stack_level_function(int32_t p_level) const
{
	return String();
}
Dictionary BladeLanguage::_debug_get_stack_level_locals(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth)
{
	return Dictionary();
}
Dictionary BladeLanguage::_debug_get_stack_level_members(int32_t p_level, int32_t p_max_subitems, int32_t p_max_depth)
{
	return Dictionary();
}
void* BladeLanguage::_debug_get_stack_level_instance(int32_t p_level)
{
	return nullptr;
}
Dictionary BladeLanguage::_debug_get_globals(int32_t p_max_subitems, int32_t p_max_depth)
{
	return Dictionary();
}
String BladeLanguage::_debug_parse_stack_level_expression(int32_t p_level, const String& p_expression, int32_t p_max_subitems, int32_t p_max_depth)
{
	return String();
}
TypedArray<Dictionary> BladeLanguage::_debug_get_current_stack_info()
{
	return TypedArray<Dictionary>();
}
void BladeLanguage::_reload_all_scripts()
{
	/* Called by the remote debugger on “reload_all_scripts” command and by GDExtensionManager after extensions change.*/
}
void BladeLanguage::_reload_scripts(const Array& p_scripts, bool p_soft_reload)
{
	/* Called by the remote debugger on “reload_scripts” command and by tool script wrappers. */
}
void BladeLanguage::_reload_tool_script(const Ref<Script>& p_script, bool p_soft_reload)
{
	/* Called by script resource savers on save (if reload-on-save is enabled) and by Script::reload_from_file for tool scripts. */
}
PackedStringArray BladeLanguage::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.push_back(blade::BladeExtension);
	array.push_back("bl");
	return array;
}
TypedArray<Dictionary> BladeLanguage::_get_public_functions() const
{
	return TypedArray<Dictionary>();
}
Dictionary BladeLanguage::_get_public_constants() const
{
	return Dictionary();
}
TypedArray<Dictionary> BladeLanguage::_get_public_annotations() const
{
	return TypedArray<Dictionary>();
}
void BladeLanguage::_profiling_start()
{
}
void BladeLanguage::_profiling_stop()
{
}
void BladeLanguage::_profiling_set_save_native_calls(bool p_enable)
{
}
int32_t BladeLanguage::_profiling_get_accumulated_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max)
{
	return 0;
}
int32_t BladeLanguage::_profiling_get_frame_data(ScriptLanguageExtensionProfilingInfo* p_info_array, int32_t p_info_max)
{
	return 0;
}
void BladeLanguage::_frame()
{
}
bool BladeLanguage::_handles_global_class_type(const String& p_type) const
{
	return p_type == blade::BladeScriptType;
}
Dictionary BladeLanguage::_get_global_class_name(const String& p_path) const
{
	Ref<BladeScript> script = ResourceLoader::get_singleton()->load(p_path);
	Dictionary result;
	if (script.is_valid())
	{
		// Validate
		StringName className = script->_get_global_name();
		if (!script->HasClassName()) return result;
		
		// Create Global Class Info
		result["name"] = className;
		result["base_type"] = script->_get_instance_base_type();
		result["icon_path"] = script->_get_class_icon_path();
		result["is_abstract"] = script->_is_abstract();
		result["is_tool"] = script->_is_tool();
	}
	return result;
}
bool BladeLanguage::CreateBindings()
{
	// Run Engine to Generate Bindings
	if (OS::get_singleton()->execute(OS::get_singleton()->get_executable_path(), { "--path", GetBladeCacheDirectory(), "--dump-extension-api" }) != 0) return false;
	String extensionAPIPath = GetBladeCacheDirectory() + "/" + blade::BladeExtensionAPIFilePath;
	if (!FileAccess::file_exists(extensionAPIPath))
	{
		ErrorLog("Bindings Generator", "Extension API File was not found, Run the engine with --dump-extension-api and place generated file in .blade directory.");
		return false;
	}

	// Read & Parse Extension API File
	try
	{
		jenova::json_t extensionAPI = jenova::json_t::parse(ReadStdStringFromFile(GetStdStr(extensionAPIPath)));
		jenova::json_t generatedDatabase;

		// Helpers
		auto StoreContent = [&](const std::string& key, std::string& content)
		{
			generatedDatabase[key] = base64::base64_encode((uint8_t*)content.data(), content.size());
			DevLog("Generated Virtual File %s", key.c_str());
		};
		auto IsSingleton = [&](const std::string& name)
		{
			for (const auto& s : extensionAPI.at("singletons"))
			{
				if (s.at("type").get<std::string>() == name) return true;
			}
			return false;
		};
		auto GenStruct = [&](const std::string& name, bool isRef)
		{
			std::string out;

			out += "typedef struct " + name + " {\n";
			out += "    void* ptr;\n";
			out += "    char type[64];\n";
			out += "    bool ref;\n";
			out += "    bool singleton;\n";
			out += "} " + name + ";\n\n";

			out += "static const " + name + " " + name + "_t = {\n";
			out += "    .ptr = nullptr,\n";
			out += "    .type = \"" + name + "\",\n";
			out += "    .ref = " + std::string(isRef ? "true" : "false") + ",\n";
			out += "    .singleton = " + std::string(IsSingleton(name) ? "true" : "false") + "\n";
			out += "};\n";

			return out;
		};
		auto GenEnums = [&](const jenova::json_t& cls)
		{
			std::string out;

			// Class Constants
			if (cls.contains("constants"))
			{
				for (const auto& c : cls["constants"])
				{
					std::string name = c["name"].get<std::string>();
					int value = c["value"].get<int>();
					out += "#define " + name + " " + std::to_string(value) + "\n";
				}
				out += "\n";
			}

			// Class Enums
			if (cls.contains("enums"))
			{
				for (const auto& e : cls["enums"])
				{
					std::string enumName = e["name"].get<std::string>();
					out += "typedef enum " + enumName + " {\n";
					for (const auto& v : e["values"])
					{
						std::string valName = v["name"].get<std::string>();
						int val = v["value"].get<int>();
						out += "    " + valName + " = " + std::to_string(val) + ",\n";
					}
					out += "} " + enumName + ";\n\n";
				}
			}
			return out;
		};
		auto GenHeader = [&](const std::string& name, const jenova::json_t& cls, bool isRef)
		{
			String file = Snakify(String(name.c_str())) + ".h";
			std::string content = "#pragma once\n\n";
			content += GenStruct(name, isRef);
			content += "\n";
			content += GenEnums(cls);
			StoreContent(GetStdStr(file), content);
		};
		auto GenGlobalEnums = [&](const jenova::json_t& api)
		{
			std::string out;
			if (api.contains("global_enums"))
			{
				for (const auto& e : api["global_enums"])
				{
					std::string enumName = e.at("name").get<std::string>();
					enumName.erase(std::remove(enumName.begin(), enumName.end(), '.'), enumName.end());
					out += "typedef enum " + enumName + " {\n";
					for (const auto& v : e.at("values"))
					{
						std::string valName = v.at("name").get<std::string>();
						int val = v.at("value").get<int>();
						out += "    " + valName + " = " + std::to_string(val) + ",\n";
					}
					out += "} " + enumName + ";\n\n";
				}
			}
			StoreContent("globals.inc", out);
			return out;
		};
		auto GenUtilityFunctions = [&](const jenova::json_t& api)
		{
			std::string out;
			if (!api.contains("utility_functions")) return out;

			for (const auto& u : api["utility_functions"])
			{
				if (!u.contains("name")) continue;
				if (!u.contains("hash")) continue;
				if (!u.contains("is_vararg")) continue;
				std::string name = u.at("name").get<std::string>();
				std::string funcName = name;

				// Handle Special Functons
				if (funcName == "typeof") funcName = "type_of";
				if (funcName == "print") funcName = "gdprint";

				bool has_ret = u.contains("return_type");
				std::string ret = has_ret ? u.at("return_type").get<std::string>() : "";
				uint32_t hash    = u.at("hash").get<uint32_t>();
				bool is_vararg   = u.at("is_vararg").get<bool>();
				std::string retTypeEnum;
				if (has_ret)
				{
					if      (ret == "float")              retTypeEnum = "TYPE_FLOAT";
					else if (ret == "int")                retTypeEnum = "TYPE_INT";
					else if (ret == "bool")               retTypeEnum = "TYPE_BOOL";
					else if (ret == "String")             retTypeEnum = "TYPE_STRING";
					else if (ret == "Vector2")            retTypeEnum = "TYPE_VECTOR2";
					else if (ret == "Vector2i")           retTypeEnum = "TYPE_VECTOR2I";
					else if (ret == "Rect2")              retTypeEnum = "TYPE_RECT2";
					else if (ret == "Rect2i")             retTypeEnum = "TYPE_RECT2I";
					else if (ret == "Vector3")            retTypeEnum = "TYPE_VECTOR3";
					else if (ret == "Vector3i")           retTypeEnum = "TYPE_VECTOR3I";
					else if (ret == "Transform2D")        retTypeEnum = "TYPE_TRANSFORM2D";
					else if (ret == "Vector4")            retTypeEnum = "TYPE_VECTOR4";
					else if (ret == "Vector4i")           retTypeEnum = "TYPE_VECTOR4I";
					else if (ret == "Plane")              retTypeEnum = "TYPE_PLANE";
					else if (ret == "Quaternion")         retTypeEnum = "TYPE_QUATERNION";
					else if (ret == "AABB")               retTypeEnum = "TYPE_AABB";
					else if (ret == "Basis")              retTypeEnum = "TYPE_BASIS";
					else if (ret == "Transform3D")        retTypeEnum = "TYPE_TRANSFORM3D";
					else if (ret == "Projection")         retTypeEnum = "TYPE_PROJECTION";
					else if (ret == "Color")              retTypeEnum = "TYPE_COLOR";
					else if (ret == "StringName")         retTypeEnum = "TYPE_STRING_NAME";
					else if (ret == "NodePath")           retTypeEnum = "TYPE_NODE_PATH";
					else if (ret == "RID")                retTypeEnum = "TYPE_RID";
					else if (ret == "Object")             retTypeEnum = "TYPE_OBJECT";
					else if (ret == "Callable")           retTypeEnum = "TYPE_CALLABLE";
					else if (ret == "Signal")             retTypeEnum = "TYPE_SIGNAL";
					else if (ret == "Dictionary")         retTypeEnum = "TYPE_DICTIONARY";
					else if (ret == "Array")              retTypeEnum = "TYPE_ARRAY";
					else if (ret == "PackedByteArray")    retTypeEnum = "TYPE_PACKED_BYTE_ARRAY";
					else if (ret == "PackedInt32Array")   retTypeEnum = "TYPE_PACKED_INT32_ARRAY";
					else if (ret == "PackedInt64Array")   retTypeEnum = "TYPE_PACKED_INT64_ARRAY";
					else if (ret == "PackedFloat32Array") retTypeEnum = "TYPE_PACKED_FLOAT32_ARRAY";
					else if (ret == "PackedFloat64Array") retTypeEnum = "TYPE_PACKED_FLOAT64_ARRAY";
					else if (ret == "PackedStringArray")  retTypeEnum = "TYPE_PACKED_STRING_ARRAY";
					else if (ret == "PackedVector2Array") retTypeEnum = "TYPE_PACKED_VECTOR2_ARRAY";
					else if (ret == "PackedVector3Array") retTypeEnum = "TYPE_PACKED_VECTOR3_ARRAY";
					else if (ret == "PackedColorArray")   retTypeEnum = "TYPE_PACKED_COLOR_ARRAY";
					else if (ret == "PackedVector4Array") retTypeEnum = "TYPE_PACKED_VECTOR4_ARRAY";
					else                                  continue; // Unknown Type → Skip Emitting
				}
				if (is_vararg)
				{
					out += (has_ret ? "Variant " : "void ") + funcName + "(int argc, Variant* args)\n";
					out += "{\n";
					if (has_ret) out += "    return utilcall(\"" + name + "\", " + std::to_string(hash) + ", " + retTypeEnum + ", argc, args);\n";
					else out += "    utilcall(\"" + name + "\", " + std::to_string(hash) + ", TYPE_NIL, argc, args);\n";
					out += "}\n\n";
					continue;
				}
				bool has_args = u.contains("arguments");
				const auto& args = has_args ? u.at("arguments") : jenova::json_t::array();
				int argc = has_args ? (int)args.size() : 0;
				out += (has_ret ? "Variant " : "void ") + funcName + "(";
				if (argc > 0)
				{
					for (int i = 0; i < argc; i++)
					{
						std::string atype = args[i].at("type").get<std::string>();
						std::string aname = args[i].at("name").get<std::string>();

						if      (atype == "float")  atype = "Float";
						else if (atype == "int")    atype = "Int";
						else if (atype == "bool")   atype = "Bool";
						else if (atype == "RID")    atype = "RID";
						else if (atype == "Object") atype = "Instance";
						else                        atype = "Variant";

						out += atype + " " + aname;
						if (i < argc - 1) out += ", ";
					}
				}
				out += ")\n";
				out += "{\n";
				if (argc > 0)
				{
					out += "    Variant args[" + std::to_string(argc) + "];\n";
					for (int i = 0; i < argc; i++)
					{
						std::string aname = args[i].at("name").get<std::string>();
						out += "    args[" + std::to_string(i) + "] = " + aname + ";\n";
					}
				}
				if (has_ret)
				{
					out += "    return utilcall(\"" + name + "\", " + std::to_string(hash) + ", " + retTypeEnum + ", " + std::to_string(argc) + ", ";
					out += (argc > 0 ? "args" : "nullptr");
					out += ");\n";
				}
				else
				{
					out += "    utilcall(\"" + name + "\", " + std::to_string(hash) + ", TYPE_NIL, " + std::to_string(argc) + ", ";
					out += (argc > 0 ? "args" : "nullptr");
					out += ");\n";
				}
				out += "}\n\n";
			}

			StoreContent("utils.inc", out);
			return out;
		};

		// Generate Classes Headers
		for (const auto& cls : extensionAPI["classes"])
		{
			std::string name = cls["name"].get<std::string>();
			if (name == "Object") continue; /* We skip Object Because it Conflicts With BladeObject */
			bool isRef = cls["is_refcounted"].get<bool>();
			GenHeader(name, cls, isRef);
		}
		
		// Generate Globals
		GenGlobalEnums(extensionAPI);

		// Generate Utility Functions
		if (blade::configuration::bl_gen_utility_functions) GenUtilityFunctions(extensionAPI);

		// Generate Database File
		if (!WriteStringToFile(GetBladeBindingsDatabasePath(), GetStr(generatedDatabase.dump(1))))
		{
			ErrorLog("Bindings Generator", "Failed to Create Generated Binding Database File.");
			return false;
		}
	}
	catch (const std::exception& parse_error)
	{
		ErrorLog("Bindings Generator", "Failed to Parse Extension API File. Aborting Bindings Database Generation Process. Parser Error : %s", parse_error.what());
		return false;
	}

	// Verbose
	Verbose("[color=#24ffab][Success][/color] Binding Database Cache Generated.");

	// All Good
	return LoadBindingsDatabase();
}
bool BladeLanguage::LoadBindingsDatabase()
{
	// Binding Database Content
	String bindingsDatabaseContent = "";

	// Load From Runtime Package
	if (FileAccess::file_exists(blade::BladeBindingsDatabaseBundlePath))
	{
		Ref<FileAccess> reader = FileAccess::open(blade::BladeBindingsDatabaseBundlePath, FileAccess::ModeFlags::READ);
		if (reader.is_valid())
		{
			bindingsDatabaseContent = reader->get_as_text();
			reader->close();
		}
	}
	else
	{
		// Load From Disk
		bindingsDatabaseContent = ReadStringFromFile(GetBladeBindingsDatabasePath());
	}

	// Validate Content
	if (bindingsDatabaseContent.is_empty()) return false;

	// Parse And Store
	try
	{
		bindingsDatabase = jenova::json_t::parse(GetStdStr(bindingsDatabaseContent));
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
const jenova::json_t BladeLanguage::GetBindingsDatabase() const
{
	return bindingsDatabase;
}
void BladeLanguage::GenerateFrameworkCode()
{
	// Banner
	frameworkCode += "/* Blade Script - Developed By Hamid.Memar */\n";

	// Do Nothing Macros
	frameworkCode += Format("#define %s\n", blade::syntax::bl_blade_on.c_str());
	frameworkCode += Format("#define %s\n", blade::syntax::bl_blade_off.c_str());

	// Language Features
	frameworkCode += "void printc(const char*, ...);\n";
	frameworkCode += "void* malloc(unsigned long);\n";
	frameworkCode += "void free(void*);\n";
	frameworkCode += "void* memset(void*, int, unsigned long);\n";
	frameworkCode += "void* memmove(void*, const void*, unsigned long);\n";

	// Base Types Macros
	frameworkCode += Format("#define %s 1\n", blade::syntax::bl_true.c_str());
	frameworkCode += Format("#define %s 0\n", blade::syntax::bl_false.c_str());
	frameworkCode += Format("#define %s NIL\n", blade::syntax::bl_null.c_str());
	frameworkCode += "#define nullptr 0\n"; // Internal

	// Control Flow Wrappers
	if (blade::GenerateControlFlowWrappers)
	{
		frameworkCode += "#define if IF\n";
		frameworkCode += "#define else ELSE\n";
		frameworkCode += "#define while WHILE\n";
		frameworkCode += "#define for FOR\n";
		frameworkCode += "#define do DO\n";
		frameworkCode += "#define break BREAK\n";
		frameworkCode += "#define continue CONTINUE\n";
		frameworkCode += "#define return RETURN\n";
		frameworkCode += "#define switch SWITCH\n";
		frameworkCode += "#define case CASE\n";
		frameworkCode += "#define default DEFAULT\n";
		frameworkCode += "#define goto GOTO\n";
	}

	// Special Types Macros
	frameworkCode += "#define Void void\n";
	frameworkCode += "#define Handle Variant\n";

	// Variant Types Macros
	frameworkCode += "#define Bool Variant\n";
	frameworkCode += "#define Int Variant\n";
	frameworkCode += "#define Float Variant\n";
	frameworkCode += "#define String Variant\n";
	frameworkCode += "#define Vector2 Variant\n";
	frameworkCode += "#define Vector2i Variant\n";
	frameworkCode += "#define Rect2 Variant\n";
	frameworkCode += "#define Rect2i Variant\n";
	frameworkCode += "#define Vector3 Variant\n";
	frameworkCode += "#define Vector3i Variant\n";
	frameworkCode += "#define Transform2D Variant\n";
	frameworkCode += "#define Vector4 Variant\n";
	frameworkCode += "#define Vector4i Variant\n";
	frameworkCode += "#define Plane Variant\n";
	frameworkCode += "#define Quaternion Variant\n";
	frameworkCode += "#define AABB Variant\n";
	frameworkCode += "#define Basis Variant\n";
	frameworkCode += "#define Transform3D Variant\n";
	frameworkCode += "#define Projection Variant\n";
	frameworkCode += "#define Color Variant\n";
	frameworkCode += "#define StringName Variant\n";
	frameworkCode += "#define NodePath Variant\n";
	frameworkCode += "#define RID Variant\n";
	frameworkCode += "#define Instance Variant\n";
	frameworkCode += "#define Callable Variant\n";
	frameworkCode += "#define Signal Variant\n";
	frameworkCode += "#define Dictionary Variant\n";
	frameworkCode += "#define Array Variant\n";
	frameworkCode += "#define PackedByteArray Variant\n";
	frameworkCode += "#define PackedInt32Array Variant\n";
	frameworkCode += "#define PackedInt64Array Variant\n";
	frameworkCode += "#define PackedFloat32Array Variant\n";
	frameworkCode += "#define PackedFloat64Array Variant\n";
	frameworkCode += "#define PackedStringArray Variant\n";
	frameworkCode += "#define PackedVector2Array Variant\n";
	frameworkCode += "#define PackedVector3Array Variant\n";
	frameworkCode += "#define PackedColorArray Variant\n";
	frameworkCode += "#define PackedVector4Array Variant\n";

	// Variant Type Makers Macros
	frameworkCode += Format("#define array mkarr\n"); // Todo : Must Be Customized
	frameworkCode += Format("#define dictionary mkdict\n"); // Todo : Must Be Customized

	// Math Type Makers Macros
	frameworkCode += "#define vec2 mkvec2\n";
	frameworkCode += "#define vec2i mkvec2i\n";
	frameworkCode += "#define vec3 mkvec3\n";
	frameworkCode += "#define vec3i mkvec3i\n";
	frameworkCode += "#define vec4 mkvec4\n";
	frameworkCode += "#define vec4i mkvec4i\n";
	frameworkCode += "#define rgb mkcolor3\n";
	frameworkCode += "#define rgba mkcolor4\n";
	frameworkCode += "#define color mkcolor4\n";
	frameworkCode += "#define htmlc mkcolorh\n";
	frameworkCode += "#define rect2 mkrect2\n";
	frameworkCode += "#define transform2d mktransform2d\n";
	frameworkCode += "#define transform3d mktransform3d\n";
	frameworkCode += "#define quat mkquat\n";
	frameworkCode += "#define plane mkplane\n";
	frameworkCode += "#define aabb mkaabb\n";
	frameworkCode += "#define basis mkbasis\n";
	
	// Packed Array Makers Macros
	frameworkCode += "#define byte_array mkbytearr\n";
	frameworkCode += "#define int32_array mkint32arr\n";
	frameworkCode += "#define int64_array mkint64arr\n";
	frameworkCode += "#define float32_array mkf32arr\n";
	frameworkCode += "#define float64_array mkf64arr\n";
	frameworkCode += "#define string_array mkstrarr\n";
	frameworkCode += "#define vector2_array mkvec2arr\n";
	frameworkCode += "#define vector3_array mkvec3arr\n";
	frameworkCode += "#define vector4_array mkvec4arr\n";
	frameworkCode += "#define color_array mkcolorarr\n";

	// Minimal Constructors
	if (blade::configuration::bl_use_mini_constructors) 
	{
		frameworkCode += "#define v2 mkvec2\n";
		frameworkCode += "#define v2i mkvec2i\n";
		frameworkCode += "#define v3 mkvec3\n";
		frameworkCode += "#define v3i mkvec3i\n";
		frameworkCode += "#define v4 mkvec4\n";
		frameworkCode += "#define v4i mkvec4i\n";
		frameworkCode += "#define c3 mkcolor3\n";
		frameworkCode += "#define c4 mkcolor4\n";
		frameworkCode += "#define r2 mkrect2\n";
		frameworkCode += "#define t2d mktransform2d\n";
		frameworkCode += "#define t3d mktransform3d\n";
		frameworkCode += "#define qt mkquat\n";
	}

	// Base Types
	frameworkCode += "typedef char bool;\n";
	frameworkCode += "typedef unsigned char byte;\n";

	// Generic Macros
	frameworkCode += Format("#define excall _exec\n"); // Internal
	frameworkCode += Format("#define varcall _vcall\n"); // Internal
	frameworkCode += Format("#define utilcall _ucall\n"); // Internal	
	frameworkCode += Format("#define eval _eval\n"); // Internal
	frameworkCode += Format("#define condition _cond\n"); // Internal
	frameworkCode += Format("#define make_obj mkobj\n"); // Internal
	frameworkCode += Format("#define %s(T) _birth(&T##_t)\n", blade::syntax::bl_birth.c_str());
	frameworkCode += Format("#define %s _death\n", blade::syntax::bl_death.c_str());
	frameworkCode += Format("#define %s _format\n", blade::syntax::bl_format.c_str());
	frameworkCode += Format("#define %s _print\n", blade::syntax::bl_print.c_str());
	frameworkCode += Format("#define %s(T) (T*)&_get_singleton(&T##_t)\n", blade::syntax::bl_get_singleton.c_str());
	frameworkCode += Format("#define %s(I, T) (T*)&_get_object(I, &T##_t)\n", blade::syntax::bl_get_object.c_str());
	frameworkCode += Format("#define %s _make_callable\n", blade::syntax::bl_make_callable.c_str());
	frameworkCode += Format("#define %s _load_library\n", blade::syntax::bl_load_library.c_str());
	frameworkCode += Format("#define %s _get_function\n", blade::syntax::bl_get_function.c_str());

	// Include Globals
	frameworkCode += "#include <globals.inc>\n";

	// Struct Types
	frameworkCode += "typedef struct Object { void* ptr; char type[64]; bool ref; bool singleton; } Object;\n";
	frameworkCode += Format("typedef struct Variant { byte opaque[%d]; } Variant;\n", GODOT_CPP_VARIANT_SIZE);
	frameworkCode += "typedef struct Property { int type; char name[64]; char class_name[64]; int hint; char hint_string[128]; int usage; char default_value[128]; char group[64]; } Property;\n";

	// System Symbols
	frameworkCode += "Variant _exec(void* bobj, const char* method, int argc, Variant* args);\n";
	frameworkCode += "Variant _vcall(Variant* variant, const char* method, int argc, Variant* args);\n";
	frameworkCode += "Variant _ucall(const char* name, int method, int returnType, int argc, Variant* args);\n";
	frameworkCode += "Variant _eval(const Variant a, const Variant b, VariantOperator op);\n";
	frameworkCode += "void* _birth(void* bobj);\n";
	frameworkCode += "void _death(void* bobj);\n";
	frameworkCode += "bool _cond(Variant opResult);\n";

	// Include Utilities
	if (blade::configuration::bl_gen_utility_functions) frameworkCode += "#include <utils.inc>\n";

	// Variant Makers
	frameworkCode += "Variant mkstr(const char* str);\n";
	frameworkCode += "Variant mkint(int v);\n";
	frameworkCode += "Variant mkint64(long long v);\n";
	frameworkCode += "Variant mkfl(float v);\n";
	frameworkCode += "Variant mkdbl(double v);\n";
	frameworkCode += "Variant mkbln(bool v);\n";
	frameworkCode += "Variant mkobj(void* bobj);\n";
	frameworkCode += "Variant mkarr(Int size);\n";
	frameworkCode += "Variant mkdict();\n";

	// Math Type Makers
	frameworkCode += "Variant mkvec2(Variant x, Variant y);\n";
	frameworkCode += "Variant mkvec2i(Variant x, Variant y);\n";
	frameworkCode += "Variant mkvec3(Variant x, Variant y, Variant z);\n";
	frameworkCode += "Variant mkvec3i(Variant x, Variant y, Variant z);\n";
	frameworkCode += "Variant mkvec4(Variant x, Variant y, Variant z, Variant w);\n";
	frameworkCode += "Variant mkvec4i(Variant x, Variant y, Variant z, Variant w);\n";
	frameworkCode += "Variant mkcolor3(Variant r, Variant g, Variant b);\n";
	frameworkCode += "Variant mkcolor4(Variant r, Variant g, Variant b, Variant a);\n";
	frameworkCode += "Variant mkcolorh(Variant hash);\n";
	frameworkCode += "Variant mkrect2(Variant x, Variant y, Variant w, Variant h);\n";
	frameworkCode += "Variant mktransform2d(Variant rot, Variant pos);\n";
	frameworkCode += "Variant mktransform3d(Variant basis, Variant origin);\n";
	frameworkCode += "Variant mkquat(Variant x, Variant y, Variant z, Variant w);\n";
	frameworkCode += "Variant mkplane(Variant a, Variant b, Variant c, Variant d);\n";
	frameworkCode += "Variant mkaabb(Variant pos, Variant size);\n";
	frameworkCode += "Variant mkbasis(Variant x_axis, Variant y_axis, Variant z_axis);\n";

	// Packed Array Makers
	frameworkCode += "Variant mkbytearr();\n";
	frameworkCode += "Variant mkint32arr();\n";
	frameworkCode += "Variant mkint64arr();\n";
	frameworkCode += "Variant mkf32arr();\n";
	frameworkCode += "Variant mkf64arr();\n";
	frameworkCode += "Variant mkstrarr();\n";
	frameworkCode += "Variant mkvec2arr();\n";
	frameworkCode += "Variant mkvec3arr();\n";
	frameworkCode += "Variant mkvec4arr();\n";
	frameworkCode += "Variant mkcolorarr();\n";

	// C Native Type Casters
	frameworkCode += "bool c_bool(Variant value);\n";
	frameworkCode += "char c_char(Variant value);\n";
	frameworkCode += "signed char c_int8(Variant value);\n";
	frameworkCode += "unsigned char c_uint8(Variant value);\n";
	frameworkCode += "short c_int16(Variant value);\n";
	frameworkCode += "unsigned short c_uint16(Variant value);\n";
	frameworkCode += "int c_int32(Variant value);\n";
	frameworkCode += "unsigned int c_uint32(Variant value);\n";
	frameworkCode += "long long c_int64(Variant value);\n";
	frameworkCode += "unsigned long long c_uint64(Variant value);\n";
	frameworkCode += "float c_float(Variant value);\n";
	frameworkCode += "double c_double(Variant value);\n";
	frameworkCode += "const char* c_str(Variant value);\n";
	frameworkCode += "void* c_ptr(Variant value);\n";
	frameworkCode += "const void* c_const_ptr(Variant value);\n";
	frameworkCode += "unsigned long long c_handle(Variant value);\n";
	frameworkCode += "unsigned long long c_const_handle(Variant value);\n";

	// Helper Symbols
	frameworkCode += "String _format(Variant format, int argc, Variant* args);\n";
	frameworkCode += "void _print(Variant format, int argc, Variant* args);\n";
	frameworkCode += "Object _get_singleton(void* bobj);\n";
	frameworkCode += "Object _get_object(Variant instance, void* bobj);\n";
	frameworkCode += "Variant _make_callable(void* bobj, String funcName);\n";
	frameworkCode += "void* _load_library(String libName);\n";
	frameworkCode += "void* _get_function(void* library, String funcName);\n";
	
	// Pre-Defined Values
	frameworkCode += Format("static const Variant NIL = { .opaque = {0} };\n");

	// Line Reset
	frameworkCode += "#line 1\n";
}
String BladeLanguage::GetFrameworkCode()
{
	return frameworkCode;
}
bool BladeLanguage::RegisterScript(Ref<BladeScript> script)
{
	// Assign Script
	activeScripts.push_back(script);

	// All Good
	return true;
}
bool BladeLanguage::UnregisterScript(Ref<BladeScript> script)
{
	// Remove Script
	if (activeScripts.has(script)) activeScripts.erase(script);

	// All Good
	return true;
}
bool BladeLanguage::HandleRemoteMessage(const String& message, const Array& args)
{
	// Handle Hot-Reload
	if (message == "Reload")
	{
		for (Ref<BladeScript> script : activeScripts)
		{
			if (script.is_valid() && script->get_path() == String(args[0]))
			{
				script->PerformHotReload(String(args[1]));
				break;
			}
		}
	}

	// All Good
	return true;
}
void BladeLanguage::AddKeywordDefinition(const String& keyword, const Color& color)
{
	keywordDefinitionStorage[keyword] = color;
}
bool BladeLanguage::IsKeywordDefinition(const String& keyword)
{
	return keywordDefinitionStorage.has(keyword);
}
Color BladeLanguage::GetKeywordDefinitionColor(const String& keyword)
{
	return keywordDefinitionStorage[keyword];
}

// Blade Object Implementation
BladeScript::BladeScript()
{
	// Initialize Objects
	scriptMutex.instantiate();

	// Set Default Script
	String code;
	code += "inherits Node3D\n";
	code += "\n";
	code += "// Imports\n";
	code += "use Node3D\n";
	code += "\n";
	code += "// Routines\n";
	code += "func Void _ready()\n";
	code += "{\n";
	code += "    // Called When Node and All Its Childred are Initialized //\n";
	code += "}\n";
	code += "func Void _process(Float delta)\n";
	code += "{\n";
	code += "    // Called Each Frame //\n";
	code += "}\n";
	this->sourceCode = code;

	// Register Script
	BladeLanguage::get_singleton()->RegisterScript(this);

	// Update Script State
	scriptState = BladeState::Initialized;
}
BladeScript::~BladeScript()
{
	// Release Objects
	scriptMutex.unref();

	// Unregister Script
	BladeLanguage::get_singleton()->UnregisterScript(this);

	// Release Context
	InvalidateContext();
}
void BladeScript::_bind_methods()
{
	ADD_SIGNAL(MethodInfo("ScriptCompiled"));
	ClassDB::bind_method(D_METHOD("CompileScript"), &BladeScript::CompileScript);
}
bool BladeScript::_editor_can_reload_from_file()
{
	return true;
}
void BladeScript::_placeholder_erased(void* p_placeholder)
{
}
bool BladeScript::_can_instantiate() const
{
	return true;
}
Ref<Script> BladeScript::_get_base_script() const
{
	return Ref<Script>();
}
StringName BladeScript::_get_global_name()
{
	UpdateScriptFeatures();
	return metaClassName;
}
bool BladeScript::_inherits_script(const Ref<Script>& p_script) const
{
	return false;
}
StringName BladeScript::_get_instance_base_type() const
{
	UpdateScriptFeatures();
	return metaClassBase;
}
void* BladeScript::_instance_create(Object* p_for_object) const
{
	// Compile Script
	EnsureCompiled();

	// Set Base Object
	baseObject = p_for_object;

	// Create Script Instance
	Node* parentNode = Object::cast_to<Node>(p_for_object);
	BladeScriptInstance* instance = memnew(BladeScriptInstance(p_for_object, Ref<BladeScript>(this)));
	return BladeScriptInstance::create_native_instance(instance);
}
void* BladeScript::_placeholder_instance_create(Object* p_for_object) const
{
	return nullptr;
}
bool BladeScript::_instance_has(Object* p_object) const
{
	ErrorLog("Debug", "Instance has %s", GetCStr(p_object->get_class()));
	return true;
}
bool BladeScript::_has_source_code() const
{
	return !sourceCode.is_empty();
}
String BladeScript::_get_source_code() const
{
	return this->sourceCode;
}
void BladeScript::_set_source_code(const String& p_code)
{
	this->sourceCode = p_code;
	_reload(true);
}
Error BladeScript::_reload(bool p_keep_state)
{
	if (Engine::get_singleton()->is_editor_hint())
	{
		/* Reload Script In Editor if It's Tool */
		if (is_tool()) CompileScript();
		else CompileScript(true /* <- Dry Run for Editor */);

		// Trigger Hot-Reload
		Array hotreloadArgs;
		hotreloadArgs.push_back(get_path());
		hotreloadArgs.push_back(sourceCode);
		SendRemoteMessage(String(blade::BladeLangaugeName) + ":Reload", hotreloadArgs);

		// All Good
		return Error::OK;
	}
	else
	{
		/* Gets Compiled and Created by Instance Creation */
		/* Hot-Reload Handled from Another Method */
		return Error::ERR_UNAVAILABLE;
	}
}
StringName BladeScript::_get_doc_class_name() const
{
	return StringName(blade::BladeScriptType);
}
TypedArray<Dictionary> BladeScript::_get_documentation() const
{
	return TypedArray<Dictionary>();
}
String BladeScript::_get_class_icon_path() const
{
	UpdateScriptFeatures();
	return String();
}
bool BladeScript::_has_method(const StringName& p_method) const
{
	const std::string method = GetStdStr(p_method);
	if (!GetScriptMetadata().contains("Functions")) return false;
	const jenova::json_t& funcs = GetScriptMetadata()["Functions"];
	for (const auto& f : funcs)
	{
		if (!f.contains("name")) continue;
		if (f["name"] == method) return true;
	}
	return false;
}
bool BladeScript::_has_static_method(const StringName& p_method) const
{
	return false;
}
Dictionary BladeScript::_get_method_info(const StringName& p_method) const
{
	const std::string method = GetStdStr(p_method);
	if (!GetScriptMetadata().contains("Functions")) return Dictionary();
	const jenova::json_t& funcs = GetScriptMetadata()["Functions"];
	if (!funcs.contains(method)) return Dictionary();
	const jenova::json_t& fn = funcs[method];
	MethodInfo mi;
	mi.name = p_method;
	std::string ret_str = fn["return"].get<std::string>();
	Variant::Type ret_type = MapBladeType(ret_str);
	mi.return_val = PropertyInfo(ret_type, ret_str.c_str());
	mi.return_val_metadata = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;
	for (const auto& p : fn["params"])
	{
		std::string type_str = p["type"].get<std::string>();
		std::string name_str = p["name"].get<std::string>();
		Variant::Type arg_type = MapBladeType(type_str);
		mi.arguments.push_back(PropertyInfo(arg_type, name_str.c_str()));
		mi.arguments_metadata.push_back(GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE);
	}
	return (Dictionary)mi;
}
bool BladeScript::_is_tool() const
{
	UpdateScriptFeatures();
	return metaToolEnabled;
}
bool BladeScript::_is_valid() const
{
	return scriptState == BladeState::Compiled;
}
bool BladeScript::_is_abstract() const
{
	UpdateScriptFeatures();
	return false;
}
ScriptLanguage* BladeScript::_get_language() const
{
	return BladeLanguage::get_singleton();
}
bool BladeScript::_has_script_signal(const StringName& p_signal) const
{
	if (!GetScriptMetadata().contains("Signals")) return false;
	const jenova::json_t& sigs = GetScriptMetadata()["Signals"];
	for (const auto& sig : sigs)
	{
		std::string sig_name = sig["name"].get<std::string>();
		if (sig_name == GetStdStr(p_signal)) return true;
	}
	return false;
}
TypedArray<Dictionary> BladeScript::_get_script_signal_list() const
{
	TypedArray<Dictionary> list;
	if (!GetScriptMetadata().contains("Signals")) return list;
	const jenova::json_t& sigs = GetScriptMetadata()["Signals"];
	for (const auto& sig : sigs)
	{
		std::string sig_name = sig["name"].get<std::string>();
		MethodInfo mi;
		mi.name = StringName(sig_name.c_str());
		mi.return_val = PropertyInfo(Variant::NIL, "");
		mi.return_val_metadata = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;
		for (const auto& p : sig["params"])
		{
			std::string type_str = p["type"].get<std::string>();
			std::string name_str = p["name"].get<std::string>();
			Variant::Type arg_type = MapBladeType(type_str);
			mi.arguments.push_back(PropertyInfo(arg_type, name_str.c_str()));
			mi.arguments_metadata.push_back(GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE);
		}
		list.push_back((Dictionary)mi);
	}
	return list;
}
bool BladeScript::_has_property_default_value(const StringName& p_property) const
{
	const std::string prop = GetStdStr(p_property);
	if (!GetScriptMetadata().contains("Properties")) return false;
	const jenova::json_t& props = GetScriptMetadata()["Properties"];
	for (const auto& p : props)
	{
		if (p.contains("solved_name") && p["solved_name"] == prop) return p.contains("default");
	}
	return false;
}
Variant BladeScript::_get_property_default_value(const StringName& p_property) const
{
	const std::string prop = GetStdStr(p_property);
	if (!GetScriptMetadata().contains("Properties")) return Variant();
	const jenova::json_t& props = GetScriptMetadata()["Properties"];
	for (const auto& p : props)
	{
		if (!p.contains("solved_name")) continue;
		if (p["solved_name"] != prop) continue;
		if (!p.contains("default")) return Variant();
		std::string def_str = p["default"].get<std::string>();
		return UtilityFunctions::str_to_var(GetStr(def_str));
	}
	return Variant();
}
void BladeScript::_update_exports()
{
}
TypedArray<Dictionary> BladeScript::_get_script_method_list() const
{
	TypedArray<Dictionary> list;
	if (!GetScriptMetadata().contains("Functions")) return list;
	const jenova::json_t& funcs = GetScriptMetadata()["Functions"];
	for (const auto& fn : funcs)
	{
		std::string fn_name = fn["name"].get<std::string>();

		MethodInfo mi;
		mi.name = StringName(fn_name.c_str());

		// Return type
		std::string ret_str = fn["return"].get<std::string>();
		Variant::Type ret_type = MapBladeType(ret_str);
		mi.return_val = PropertyInfo(ret_type, ret_str.c_str());
		mi.return_val_metadata = GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE;

		// Parameters
		for (const auto& p : fn["params"])
		{
			std::string type_str = p["type"].get<std::string>();
			std::string name_str = p["name"].get<std::string>();
			Variant::Type arg_type = MapBladeType(type_str);
			mi.arguments.push_back(PropertyInfo(arg_type, name_str.c_str()));
			mi.arguments_metadata.push_back(GDEXTENSION_METHOD_ARGUMENT_METADATA_NONE);
		}

		list.push_back((Dictionary)mi);
	}
	return list;
}
TypedArray<Dictionary> BladeScript::_get_script_property_list() const
{
	return scriptProperties;
}
int32_t BladeScript::_get_member_line(const StringName& p_member) const
{
	return 0;
}
Dictionary BladeScript::_get_constants() const
{
	return Dictionary();
}
TypedArray<StringName> BladeScript::_get_members() const
{
	return TypedArray<StringName>();
}
bool BladeScript::_is_placeholder_fallback_enabled() const
{
	return false;
}
Variant BladeScript::_get_rpc_config() const
{
	return Variant();
}
void BladeScript::ReloadSourceCode()
{
	sourceCode = FileAccess::get_file_as_string(this->get_path());
}
void BladeScript::UpdateScriptFeatures() const
{
	// Check Source Code
	if (!has_source_code()) return;

	// Use Namespace
	using namespace blade::syntax;

	// Split Source for Feature Scan
	PackedStringArray lines = sourceCode.split("\n");
	
	// Reset Features
	metaHasClassName = false;
	metaClassName = blade::BladeScriptType;
	metaHasClassBase = false;
	metaClassBase = "Object";
	metaToolEnabled = false;

	// Quick Scan for Features
	for (int i = 0; i < lines.size(); i++)
	{
		String line = lines[i];
		String trimmed = line.strip_edges();
		String prefix = "";

		// Ignore Lines
		if (trimmed.begins_with("//")) {  continue; };

		// Handle Base Class
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_inherits)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			metaClassBase = parts[1];
			metaHasClassBase = true;
			continue;
		}

		// Handle Class Name
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_class)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2)
			{
				metaClassName = StripQuotes(parts[1]);
				metaHasClassName = true;
			}
			continue;
		}

		// Handle Tool Option
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_tool)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2) metaToolEnabled = parts[1] == "true" ? true : false;
			continue;
		}

		// Limit Scan
		if (i > blade::configuration::bl_max_feature_line_limit) break;
	}
}
void BladeScript::BindScriptSymbols(BladeCompiler* compiler)
{
	// Add CBlade System Symbols
	compiler->AddSymbol("_exec", BLADEFUNCTION(&blade::system::blade_exec));
	compiler->AddSymbol("_vcall", BLADEFUNCTION(&blade::system::blade_vcall));
	compiler->AddSymbol("_ucall", BLADEFUNCTION(&blade::system::blade_ucall));
	compiler->AddSymbol("_eval", BLADEFUNCTION(&blade::system::blade_eval));
	compiler->AddSymbol("_birth", BLADEFUNCTION(&blade::system::blade_birth));
	compiler->AddSymbol("_death", BLADEFUNCTION(&blade::system::blade_death));
	compiler->AddSymbol("_cond", BLADEFUNCTION(&blade::system::blade_cond));

	// Add CBlade Variant Maker Symbols
	compiler->AddSymbol("mkstr", BLADEFUNCTION(&blade::system::blade_make_string));
	compiler->AddSymbol("mkint", BLADEFUNCTION(&blade::system::blade_make_int32));
	compiler->AddSymbol("mkint64", BLADEFUNCTION(&blade::system::blade_make_int64));
	compiler->AddSymbol("mkfl", BLADEFUNCTION(&blade::system::blade_make_float));
	compiler->AddSymbol("mkdbl", BLADEFUNCTION(&blade::system::blade_make_double));
	compiler->AddSymbol("mkbln", BLADEFUNCTION(&blade::system::blade_make_bool));
	compiler->AddSymbol("mkobj", BLADEFUNCTION(&blade::system::blade_make_object));
	compiler->AddSymbol("mkarr", BLADEFUNCTION(&blade::system::blade_make_array));
	compiler->AddSymbol("mkdict", BLADEFUNCTION(&blade::system::blade_make_dict));

	// Add CBlade Math Type Maker Symbols
	compiler->AddSymbol("mkvec2", BLADEFUNCTION(&blade::system::blade_make_vector2));
	compiler->AddSymbol("mkvec2i", BLADEFUNCTION(&blade::system::blade_make_vector2i));
	compiler->AddSymbol("mkvec3", BLADEFUNCTION(&blade::system::blade_make_vector3));
	compiler->AddSymbol("mkvec3i", BLADEFUNCTION(&blade::system::blade_make_vector3i));
	compiler->AddSymbol("mkvec4", BLADEFUNCTION(&blade::system::blade_make_vector4));
	compiler->AddSymbol("mkvec4i", BLADEFUNCTION(&blade::system::blade_make_vector4i));
	compiler->AddSymbol("mkcolor3", BLADEFUNCTION(&blade::system::blade_make_color_rgb));
	compiler->AddSymbol("mkcolor4", BLADEFUNCTION(&blade::system::blade_make_color_rgba));
	compiler->AddSymbol("mkcolorh", BLADEFUNCTION(&blade::system::blade_make_color_hash));
	compiler->AddSymbol("mkrect2", BLADEFUNCTION(&blade::system::blade_make_rect2));
	compiler->AddSymbol("mktransform2d", BLADEFUNCTION(&blade::system::blade_make_transform2d));
	compiler->AddSymbol("mktransform3d", BLADEFUNCTION(&blade::system::blade_make_transform3d));
	compiler->AddSymbol("mkquat", BLADEFUNCTION(&blade::system::blade_make_quaternion));
	compiler->AddSymbol("mkplane", BLADEFUNCTION(&blade::system::blade_make_plane));
	compiler->AddSymbol("mkaabb", BLADEFUNCTION(&blade::system::blade_make_aabb));
	compiler->AddSymbol("mkbasis", BLADEFUNCTION(&blade::system::blade_make_basis));

	// Add CBlade Typed Array Maker Symbols
	compiler->AddSymbol("mkbytearr", BLADEFUNCTION(&blade::system::blade_make_byte_array));
	compiler->AddSymbol("mkint32arr", BLADEFUNCTION(&blade::system::blade_make_int32_array));
	compiler->AddSymbol("mkint64arr", BLADEFUNCTION(&blade::system::blade_make_int64_array));
	compiler->AddSymbol("mkf32arr", BLADEFUNCTION(&blade::system::blade_make_float32_array));
	compiler->AddSymbol("mkf64arr", BLADEFUNCTION(&blade::system::blade_make_float64_array));
	compiler->AddSymbol("mkstrarr", BLADEFUNCTION(&blade::system::blade_make_string_array));
	compiler->AddSymbol("mkvec2arr", BLADEFUNCTION(&blade::system::blade_make_vector2_array));
	compiler->AddSymbol("mkvec3arr", BLADEFUNCTION(&blade::system::blade_make_vector3_array));
	compiler->AddSymbol("mkvec4arr", BLADEFUNCTION(&blade::system::blade_make_vector4_array));
	compiler->AddSymbol("mkcolorarr", BLADEFUNCTION(&blade::system::blade_make_color_array));
	
	// Add CBlade Native Type Casters Symbols
	compiler->AddSymbol("c_bool", BLADEFUNCTION(&blade::system::blade_c_bool));
	compiler->AddSymbol("c_char", BLADEFUNCTION(&blade::system::blade_c_char));
	compiler->AddSymbol("c_int8", BLADEFUNCTION(&blade::system::blade_c_int8));
	compiler->AddSymbol("c_uint8", BLADEFUNCTION(&blade::system::blade_c_uint8));
	compiler->AddSymbol("c_int16", BLADEFUNCTION(&blade::system::blade_c_int16));
	compiler->AddSymbol("c_uint16", BLADEFUNCTION(&blade::system::blade_c_uint16));
	compiler->AddSymbol("c_int32", BLADEFUNCTION(&blade::system::blade_c_int32));
	compiler->AddSymbol("c_uint32", BLADEFUNCTION(&blade::system::blade_c_uint32));
	compiler->AddSymbol("c_int64", BLADEFUNCTION(&blade::system::blade_c_int64));
	compiler->AddSymbol("c_uint64", BLADEFUNCTION(&blade::system::blade_c_uint64));
	compiler->AddSymbol("c_float", BLADEFUNCTION(&blade::system::blade_c_float));
	compiler->AddSymbol("c_double", BLADEFUNCTION(&blade::system::blade_c_double));
	compiler->AddSymbol("c_str", BLADEFUNCTION(&blade::system::blade_c_str));
	compiler->AddSymbol("c_ptr", BLADEFUNCTION(&blade::system::blade_c_ptr));
	compiler->AddSymbol("c_const_ptr", BLADEFUNCTION(&blade::system::blade_c_const_ptr));
	compiler->AddSymbol("c_handle", BLADEFUNCTION(&blade::system::blade_c_handle));
	compiler->AddSymbol("c_const_handle",BLADEFUNCTION(&blade::system::blade_c_const_handle));

	// System Misc Symbols
	compiler->AddSymbol("__chkstk", BLADEFUNCTION(&blade::system::blade_chkstk));

	// Add CBlade Helper Symbols
	compiler->AddSymbol("_format", BLADEFUNCTION(&blade::helpers::blade_format));
	compiler->AddSymbol("_print", BLADEFUNCTION(&blade::helpers::blade_print));
	compiler->AddSymbol("_get_singleton", BLADEFUNCTION(&blade::helpers::blade_get_singleton));
	compiler->AddSymbol("_get_object", BLADEFUNCTION(&blade::helpers::blade_get_object));
	compiler->AddSymbol("_make_callable", BLADEFUNCTION(&blade::helpers::blade_make_callable));
	compiler->AddSymbol("_load_library", BLADEFUNCTION(&blade::helpers::blade_load_library));
	compiler->AddSymbol("_get_function", BLADEFUNCTION(&blade::helpers::blade_get_function));

	// Add Additional Symbols
	compiler->AddSymbol("printc", BLADEFUNCTION(&::Verbose));
	compiler->AddSymbol("malloc", BLADEFUNCTION(&::malloc));
	compiler->AddSymbol("free", BLADEFUNCTION(&::free));
	compiler->AddSymbol("memset", BLADEFUNCTION(&::memset));
	compiler->AddSymbol("memmove", BLADEFUNCTION(&::memmove));
}
String BladeScript::PreProcessScript(jenova::json_t& metadata)
{
	// Use Namespace
	using namespace blade::syntax;

	// Prepare Metadata Arrays
	jenova::json_t includes = jenova::json_t::array();
	jenova::json_t properties = jenova::json_t::array();
	jenova::json_t functions = jenova::json_t::array();
	jenova::json_t signals = jenova::json_t::array();
	jenova::json_t extensions = jenova::json_t::array();

	// Property Mappings
	HashMap<String, String> propertyMappings;

	// Helpers
	auto SolvePropertyName = [&](String s)->String
	{
		if (propertyMappings.has(s)) return propertyMappings[s];
		return s;
	};

	// Rewritters (Transpile)
	RecursiveRewriter RewriteProperties = [&](String line) -> String
    {
        String out;
        auto isIdent = [](char32_t ch) { return (ch == '_' || ch == '/' || (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')); };
        bool inString = false;
        for (int i = 0; i < line.length(); i++)
        {
            char32_t c = line[i];
            if (c == '"' && (i == 0 || line[i - 1] != '\\'))
            {
                inString = !inString;
                out += c;
                continue;
            }
            if (!inString && c == '$')
            {
                int start = i + 1;
                int j = start;
                while (j < line.length() && isIdent(line[j])) j++;
                String raw = line.substr(start, j - start);
                String solved = SolvePropertyName(raw);
                int k = j;
                while (k < line.length() && isspace((int)line[k])) k++;
                bool is_assign =(k < line.length() && line[k] == '=' && !(k + 1 < line.length() && line[k + 1] == '='));
                if (is_assign)
                {
                    k++;
                    while (k < line.length() && isspace((int)line[k])) k++;
                    int expr_start = k;
                    bool inStr2 = false;
                    int depth = 0;
                    int m = expr_start;
                    for (; m < line.length(); m++)
                    {
                        char32_t ch2 = line[m];
                        if (ch2 == '"' && (m == 0 || line[m - 1] != '\\'))
                        {
                            inStr2 = !inStr2;
                            continue;
                        }
                        if (inStr2) continue;
                        if (ch2 == '(') depth++;
                        else if (ch2 == ')') depth--;
                        else if (ch2 == ';' && depth == 0) break;
                    }

					// Rewrite Assignment
                    String expr = line.substr(expr_start, m - expr_start).strip_edges();
                    expr = RewriteProperties(expr);
                    out += "self->set(\"" + solved + "\", " + expr + ")";
                    if (m < line.length() && line[m] == ';')out += ";";
                    i = m;
                    continue;
                }
                else
                {
					// Rewrite Access
                    out += "self->get(\"" + solved + "\")";
                    i = j - 1;
                    continue;
                }
            }
            out += c;
        }
        return out;
    };
	auto RewriteFields = [&](String line) -> String
	{
		if (line.strip_edges().begins_with("//")) return line;

		auto isIdent = [&](char32_t ch)
		{
			return (ch == '_' || (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
		};
		auto isObjChar = [&](char32_t ch)
		{
			return !(ch == ' ' || ch == '\t' || ch == '\n' || ch == ';' || ch == ',' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '\0');
		};
		auto isNameChar = [&](char32_t ch)
		{
			return (ch == '_' || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
		};

	restartScan:
		String out;
		bool inString = false;
		char fieldSymbol = blade::configuration::bl_support_structs ? ':' : '.';
		for (int i = 0; i < line.length(); i++)
		{
			char32_t c = line[i];
			if (c == '"' && (i == 0 || line[i - 1] != '\\'))
			{
				inString = !inString;
				out += c;
				continue;
			}
			if (!inString && c == fieldSymbol)
			{
				if (i == 0)
				{
					out += c;
					continue;
				}
				char32_t prev = line[i - 1];
				if (!(isNameChar(prev) || prev == ')' || prev == ']'))
				{
					out += c;
					continue;
				}
				int obj_end = i - 1;
				int obj_start = obj_end;
				while (obj_start >= 0 && isObjChar(line[obj_start])) obj_start--;
				obj_start++;
				if (obj_start > obj_end)
				{
					out += c;
					continue;
				}
				if (obj_start > 0 && isIdent(line[obj_start - 1]))
				{
					out += c;
					continue;
				}
				String obj = line.substr(obj_start, obj_end - obj_start + 1);
				int fld_start = i + 1;
				int fld_end = fld_start;
				while (fld_end < line.length() && isIdent(line[fld_end])) fld_end++;
				if (fld_end == fld_start)
				{
					out += c;
					continue;
				}
				String field = line.substr(fld_start, fld_end - fld_start);
				int k = fld_end;
				while (k < line.length() && isspace((int)line[k])) k++;
				bool is_assign = (k < line.length() && line[k] == '=' && !(k + 1 < line.length() && line[k + 1] == '='));
				if (is_assign)
				{
					k++;
					while (k < line.length() && isspace((int)line[k])) k++;
					int expr_start = k;
					bool inStr2 = false;
					int depth = 0;
					int m = expr_start;
					for (; m < line.length(); m++)
					{
						char32_t ch2 = line[m];

						if (ch2 == '"' && (m == 0 || line[m - 1] != '\\'))
						{
							inStr2 = !inStr2;
							continue;
						}
						if (inStr2) continue;
						if (ch2 == '(') depth++;
						else if (ch2 == ')') depth--;
						else if (ch2 == ';' && depth == 0) break;
					}
					String expr = line.substr(expr_start, m - expr_start).strip_edges();
					out = line.substr(0, obj_start) + obj + "<-field_assign(\"" + field + "\", " + expr + ")";
					if (m < line.length() && line[m] == ';') out += ";";
					out += line.substr(m + 1);
					line = out;
					goto restartScan;
				}
				else
				{
					out = line.substr(0, obj_start) + obj + "<-field_access(\"" + field + "\")" + line.substr(fld_end);
					line = out;
					goto restartScan;
				}
			}
			out += c;
		}
		return out;
	};
	auto RewriteLiterals = [&](const String& line) -> String
	{
		String out;
		int i = 0;
		int n = line.length();
		auto isDigit = [&](char c) { return c >= '0' && c <= '9'; };
		auto isIdentStart = [&](char c) { return (c == '_') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
		auto isIdentChar = [&](char c) { return isIdentStart(c) || isDigit(c); };
		while (i < n)
		{
			char c = line[i];
			if (c == '"')
			{
				int start = i;
				i++;
				bool escaped = false;
				while (i < n)
				{
					char d = line[i];
					if (!escaped && d == '\\')
					{
						escaped = true;
						i++;
					}
					else if (!escaped && d == '"')
					{
						i++;
						break;
					}
					else
					{
						escaped = false;
						i++;
					}
				}
				String lit = line.substr(start, i - start);
				out += "mkstr(";
				out += lit;
				out += ")";
				continue;
			}
			if (c == '/' && i + 1 < n && line[i + 1] == '/') {
				out += line.substr(i);
				break;
			}
			if (isIdentStart(c))
			{
				int start = i;
				i++;
				while (i < n && isIdentChar(line[i])) i++;
				String ident = line.substr(start, i - start);
				if (ident == "true" || ident == "false") out += "mkbln(" + ident + ")";
				else out += ident;
				continue;
			}
			if (isDigit(c) || (c == '-' && i + 1 < n && (isDigit(line[i + 1]) || line[i + 1] == '.')))
			{
				int start = i;
				bool is_hex = false;
				bool has_dot = false;
				bool has_exp = false;
				bool has_f = false;
				if (c == '-') i++;
				if (i + 1 < n && line[i] == '0' && (line[i + 1] == 'x' || line[i + 1] == 'X'))
				{
					is_hex = true;
					i += 2;
					while (i < n && std::isxdigit((unsigned char)line[i])) i++;
				}
				else
				{
					while (i < n && isDigit(line[i])) i++;
					if (i < n && line[i] == '.')
					{
						has_dot = true;
						i++;
						while (i < n && isDigit(line[i])) i++;
					}
					if (i < n && (line[i] == 'e' || line[i] == 'E'))
					{
						has_exp = true;
						i++;
						if (i < n && (line[i] == '+' || line[i] == '-')) i++;
						while (i < n && isDigit(line[i])) i++;
					}
					if (i < n && (line[i] == 'f' || line[i] == 'F'))
					{
						has_f = true;
						i++;
					}
				}
				String lit = line.substr(start, i - start);
				if (is_hex)
				{
					if (blade::configuration::bl_use_64bit_integer) out += "mkint64(" + lit + ")";
					else out += "mkint(" + lit + ")";
				}
				else if (has_dot || has_exp)
				{
					if (has_f)
					{
						out += "mkfl(" + lit + ")";
					}
					else
					{
						if (blade::configuration::bl_use_64bit_float) out += "mkdbl(" + lit + ")";
						else out += "mkfl(" + lit + ")";
					}
				}
				else
				{
					if (blade::configuration::bl_use_64bit_integer) out += "mkint64(" + lit + ")";
					else out += "mkint(" + lit + ")";
				}
				continue;
			}
			out += String::chr(c);
			i++;
		}
		return out;
	};
    auto RewriteCalls = [&](String line, String& prefix) -> String
    {
        // Helpers
        auto FindCallSymbol = [&](const String& s, int start) -> int
        {
            bool inString = false;
            for (int i = start; i < s.length() - 1; i++)
            {
                char c = s[i];
                if (c == '"' && (i == 0 || s[i - 1] != '\\')) inString = !inString;
                if (!inString && ((c == '-' && s[i + 1] == '>') || (c == '<' && s[i + 1] == '-'))) return i;
            }
            return -1;
        };
        auto FindReceiverStart = [&](const String& s, int arrowPos) -> int
        {
            int p = arrowPos - 1;
            if (p < 0) return 0;
            if (s[p] == ')')
            {
                int depth = 1;
                p--;
                while (p >= 0 && depth > 0)
                {
                    if (s[p] == ')') depth++;
                    else if (s[p] == '(') depth--;
                    p--;
                }
                p++;
                p--;
                while (p >= 0 && (isalnum(s[p]) || s[p] == '_')) p--;
                p++;
            }
            else
            {
                while (p >= 0 && (isalnum(s[p]) || s[p] == '_')) p--;
                p++;
            }
            return p;
        };

        // Perform Calls Rewrite
        int pos = FindCallSymbol(line, 0);
        while (pos != -1)
        {
            bool is_excall = (line[pos + 1] == '>');
            String call_prefix = is_excall ? "excall" : "varcall";
            int obj_start = FindReceiverStart(line, pos);
            String obj = line.substr(obj_start, pos - obj_start);
            if (obj == GetStr(blade::syntax::bl_this)) obj = "self";
            int fn_start = pos + 2;
            int fn_end = line.find("(", fn_start);
            String func = line.substr(fn_start, fn_end - fn_start);
            int arg_start = fn_end + 1;
            int i = arg_start;
            int depth = 1;
            while (i < line.length() && depth > 0)
            {
                if (line[i] == '(') depth++;
                if (line[i] == ')') depth--;
                i++;
            }
            int arg_end = i - 1;
            String args = line.substr(arg_start, arg_end - arg_start);
            Vector<String> arg_list;
            {
                int d = 0;
                int last = 0;
                for (int k = 0; k < args.length(); k++)
                {
                    char c = args[k];
                    if (c == '(') d++;
                    else if (c == ')') d--;
                    else if (c == ',' && d == 0)
                    {
                        arg_list.push_back(args.substr(last, k - last).strip_edges());
                        last = k + 1;
                    }
                }
                if (last < args.length()) arg_list.push_back(args.substr(last).strip_edges());
            }
            Vector<String> tr_args;
            for (int k = 0; k < arg_list.size(); k++)
            {
                String sub_prefix = "";
                String sub_line = arg_list[k];
                int sub_pos = FindCallSymbol(sub_line, 0);
                while (sub_pos != -1)
                {
                    bool sub_is_excall = (sub_line[sub_pos + 1] == '>');
                    String sub_call_prefix = sub_is_excall ? "excall" : "varcall";
                    int sub_obj_start = FindReceiverStart(sub_line, sub_pos);
                    String sub_obj = sub_line.substr(sub_obj_start, sub_pos - sub_obj_start);
                    if (sub_obj == GetStr(blade::syntax::bl_this)) sub_obj = "self";

                    int sub_fn_start = sub_pos + 2;
                    int sub_fn_end = sub_line.find("(", sub_fn_start);
                    String sub_func = sub_line.substr(sub_fn_start, sub_fn_end - sub_fn_start);

                    int sub_arg_start = sub_fn_end + 1;
                    int j = sub_arg_start;
                    int sub_depth = 1;
                    while (j < sub_line.length() && sub_depth > 0)
                    {
                        if (sub_line[j] == '(') sub_depth++;
                        if (sub_line[j] == ')') sub_depth--;
                        j++;
                    }
                    int sub_arg_end = j - 1;
                    String sub_args = sub_line.substr(sub_arg_start, sub_arg_end - sub_arg_start);
                    int sub_argc = 0;
                    int sd = 0;
                    for (int t = 0; t < sub_args.length(); t++)
                    {
                        char c = sub_args[t];
                        if (c == '(') sd++;
                        else if (c == ')') sd--;
                        else if (c == ',' && sd == 0) sub_argc++;
                    }
                    if (!sub_args.strip_edges().is_empty()) sub_argc++;
                    String sub_uid = "args_" + CreateUniqueID(6);
                    String sub_arr = (sub_argc > 0) ? "Variant " + sub_uid + "[" + itos(sub_argc) + "] = { " + sub_args + " }; " : "";
                    String sub_call = (sub_argc > 0) ? sub_call_prefix + "(" + (sub_is_excall ? sub_obj : "&" + sub_obj) + ", \"" + sub_func + "\", " + itos(sub_argc) + ", " + sub_uid + ")" : sub_call_prefix + "(" + (sub_is_excall ? sub_obj : "&" + sub_obj) + ", \"" + sub_func + "\", 0, nullptr)";
                    sub_prefix = sub_prefix + sub_arr;
                    sub_line = sub_line.substr(0, sub_obj_start) + sub_call + sub_line.substr(sub_arg_end + 1);
                    sub_pos = FindCallSymbol(sub_line, sub_obj_start + sub_call.length());
                }
                prefix = prefix + sub_prefix;
                tr_args.push_back(sub_line.strip_edges());
            }

            String rebuilt_args;
            for (int k = 0; k < tr_args.size(); k++)
            {
                if (k > 0) rebuilt_args += ", ";
                rebuilt_args += tr_args[k];
            }

            int argc = tr_args.size();
            String uid = "args_" + CreateUniqueID(6);
            String storage = (argc > 0) ? "Variant " + uid + "[" + itos(argc) + "] = { " + rebuilt_args + " }; " : "";
            String call = (argc > 0) ? call_prefix + "(" + (is_excall ? obj : "&" + obj) + ", \"" + func + "\", " + itos(argc) + ", " + uid + ")" : call_prefix + "(" + (is_excall ? obj : "&" + obj) + ", \"" + func + "\", 0, nullptr)";
            prefix += storage;
            line = line.substr(0, obj_start) + call + line.substr(arg_end + 1);
            pos = FindCallSymbol(line, obj_start + call.length());
        }
        return line.strip_edges();
    };
	auto RewriteVariadics = [&](String line, String function, String& prefix) -> String
	{
		String detect = function + "(";
		int pos = line.find(detect);
		while (pos != -1)
		{
			int start = pos;
			int open = line.find("(", start);
			if (open == -1) break;
			int i = open + 1;
			int depth = 1;
			while (i < line.length() && depth > 0)
			{
				if (line[i] == '(') depth++;
				else if (line[i] == ')') depth--;
				i++;
			}
			if (depth != 0) break;
			int close = i - 1;
			String inside = line.substr(open + 1, close - open - 1).strip_edges();
			Vector<String> parts;
			{
				int d = 0;
				int last = 0;
				for (int k = 0; k < inside.length(); k++)
				{
					char c = inside[k];
					if (c == '(') d++;
					else if (c == ')') d--;
					else if (c == ',' && d == 0)
					{
						parts.push_back(inside.substr(last, k - last).strip_edges());
						last = k + 1;
					}
				}
				if (last < inside.length()) parts.push_back(inside.substr(last).strip_edges());
			}
			if (parts.size() < 1)
			{
				pos = line.find(detect, pos + 1);
				continue;
			}
			String fmt = parts[0];
			int argc = parts.size() - 1;
			String hash = "args_" + CreateUniqueID(6);
			String storage = "Variant " + hash + "[" + itos(argc) + "] = { ";
			for (int p = 1; p < parts.size(); p++)
			{
				if (p > 1) storage += ", ";
				storage += parts[p];
			}
			storage += " }; ";
			prefix += storage;
			String replacement = detect + fmt + ", " + itos(argc) + ", " + hash + ")";
			line = line.substr(0, start) + replacement + line.substr(close + 1);
			pos = line.find(detect, start + replacement.length());
		}
		return line;
	};
	auto RewriteObjects = [&](const String& line) -> String
	{
		String out = "";
		bool in_string = false;
		for (int i = 0; i < line.length(); )
		{
			char c = line[i];
			if (c == '"' && (i == 0 || line[i - 1] != '\\')) in_string = !in_string;
			if (!in_string && c == '@')
			{
				int start = i + 1;
				int end = start;
				while (end < line.length() && (isalnum(line[end]) || line[end] == '_')) end++;
				if (end == start)
				{
					out += '@';
					i++;
					continue;
				}
				String name = line.substr(start, end - start);
				out += "make_obj(" + name + ")";
				i = end;
			}
			else
			{
				out += c;
				i++;
			}
		}
		return out;
	};

	// Generators
	auto GenerateProxies = [&]() -> String
	{
		std::string out = "/* BladeLang Auto-Generated Proxies */\n";
		std::string baseClass = GetStdStr(_get_instance_base_type());

		const jenova::json_t& funcs = functions;
		for (const auto& fn : funcs)
		{
			std::string fname = fn["name"].get<std::string>();
			std::string ret = fn["return"].get<std::string>();
			const jenova::json_t& params = fn["params"];
			std::string proxyName = "_" + fname + "___proxy";
			out += "Variant " + proxyName + "(" + baseClass + "* instance, int argc, Variant* args)\n{\n";
			out += "\tif (argc != " + std::to_string(params.size()) + ") {return null;}\n";
			if (ret == "void" || ret == "Void" || ret == "VOID")
			{
				out += "\t" + fname + "(instance";
				for (size_t i = 0; i < params.size(); i++) out += ", args[" + std::to_string(i) + "]";
				out += "); return null;\n";
			}
			else
			{
				out += "\treturn " + fname + "(instance";
				for (size_t i = 0; i < params.size(); i++) out += ", args[" + std::to_string(i) + "]";
				out += ");\n";
			}
			out += "}\n";
		}
		const jenova::json_t& exts = extensions;
		for (const auto& ext : exts)
		{
			std::string fname = ext["name"].get<std::string>();
			std::string ret = ext["return"].get<std::string>();
			std::string kind = ext["kind"].get<std::string>();
			std::string recv = ext["receiverType"].get<std::string>();
			const jenova::json_t& params = ext["params"];
			std::string proxyName = "__" + fname + "___proxy";
			if (kind == "variant")
			{
				out += "Variant " + proxyName + "(Variant __var, int argc, Variant* args)\n{\n";
				out += "\tif (argc != " + std::to_string(params.size()) + ") {return null;}\n";
				out += "\t";
				if (ret == "void" || ret == "Void" || ret == "VOID")
				{
					out += fname + "(__var";
					for (size_t i = 0; i < params.size(); i++) out += ", args[" + std::to_string(i) + "]";
					out += "); return null;\n";
				}
				else
				{
					out += "return " + fname + "(__var";
					for (size_t i = 0; i < params.size(); i++) out += ", args[" + std::to_string(i) + "]";
					out += ");\n";
				}
				out += "}\n";
			}
			else if (kind == "object")
			{
				out += "Variant " + proxyName + "(Object* __obj, int argc, Variant* args)\n{\n";
				out += "\tif (argc != " + std::to_string(params.size()) + ") {return null;}\n";

				out += "\t";
				if (ret == "void" || ret == "Void" || ret == "VOID")
				{
					out += fname + "((" + recv + "*)__obj";
					for (size_t i = 0; i < params.size(); i++) out += ", args[" + std::to_string(i) + "]";
					out += "); return null;\n";
				}
				else
				{
					out += "return " + fname + "((" + recv + "*)__obj";
					for (size_t i = 0; i < params.size(); i++) out += ", args[" + std::to_string(i) + "]";
					out += ");\n";
				}
				out += "}\n";
			}
		}
		return GetStr(out);
	};

	// Special Passes
	String processedSource = sourceCode;
	processedSource = blade::tast::ProcessSourceExpressions(processedSource);

	// Corrections
	processedSource = processedSource.replace("vec2()", "vec2(0, 0)");
	processedSource = processedSource.replace("vec3()", "vec3(0, 0, 0)");
	processedSource = processedSource.replace("vec4()", "vec4(0, 0, 0, 0)");
	processedSource = processedSource.replace("array()", "array(0)");

	// Split Source for Scanline Processor
	PackedStringArray lines = processedSource.split("\n");

	// Run Validations
	for (int i = 0; i < lines.size(); i++)
	{
		String l = lines[i];
		int open = 0;
		for (int k = 0; k < l.length(); k++)
		{
			if (l[k] == '(') open++;
			if (l[k] == ')') open--;
		}
		if (open > 0) 
		{
			ErrorLog("Preprocessor", "Parse Error: Multi-line calls are not allowed. Offending line: %d", i +1);
			return "<Parse-Error>";
		}
	}

	// Parse & Process Script (Scanline)
	String preprocessed;
	bool transpileEnabled = true;
	for (int i = 0; i < lines.size(); i++)
	{
		String line = lines[i];
		String trimmed = line.strip_edges();
		String prefix = "";

		// Ignore Lines
		if (trimmed.begins_with("//")) {  preprocessed += "\n"; continue; };
		if (trimmed.begins_with("#include")) { preprocessed += line + "\n"; continue; };

        // Handle Transpiler Switch
        if (trimmed == GetDefinerKeyword(DefinerKeyword::dk_blade_off)) { transpileEnabled = false; preprocessed += line + "\n"; continue; }
        if (trimmed == GetDefinerKeyword(DefinerKeyword::dk_blade_on)) { transpileEnabled = true; preprocessed += line + "\n"; continue; }
        if (!transpileEnabled) { preprocessed += line + "\n"; continue; }
		
		// Handle Base Class
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_inherits)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2) metadata["BaseClass"] = GetStdStr(parts[1]);
			preprocessed += "/*" + line + "*/\n";
			continue;
		}

		// Handle Class Name
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_class)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2)
			{
				String cname = StripQuotes(parts[1]);
				metadata["ClassName"] = GetStdStr(cname);
			}
			preprocessed += "/*" + line + "*/\n";
			continue;
		}

		// Handle Tool Option
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_tool)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2) metadata["FortySix&2"] = parts[1] == "true" ? true : false;
			preprocessed += "/*" + line + "*/\n";
			continue;
		}

		// Handle Imports
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_use)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2)
			{
				String className = parts[1];
				String header = Snakify(className) + ".h";
				includes.push_back(GetStdStr(header));
				preprocessed += "#include <" + header + ">\n";
			}
			continue;
		}

		// Handle Properties
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_property)))
		{
			String rest = trimmed.substr(GetDefinerKeyword(DefinerKeyword::dk_property).length()).strip_edges();
			int arrow = rest.find("->");
			String left = rest;
			String meta = "";
			if (arrow != -1)
			{
				left = rest.substr(0, arrow).strip_edges();
				meta = rest.substr(arrow + 2).strip_edges();
			}
			PackedStringArray parts = left.split(" ", false);
			if (parts.size() >= 2)
			{
				String type = parts[0];
				String name = parts[1];
				int eq = left.find("=");
				String def = "";
				if (eq != -1) def = left.substr(eq + 1).strip_edges();
				String orig_def = def;
				def = def.replace("\"", "\\\"");
				String prop_id = "__prop__" + Format("%03d", properties.size() + 1);
				String cprop = "const Property " + prop_id + " = { .type = " + GetVariantTypeFromBladeType(type) + ", .name = \"" + name + "\", .default_value = \"" + def + "\"";
				if (meta.begins_with("[") && meta.ends_with("]"))
				{
					String inside = meta.substr(1, meta.length() - 2).strip_edges();
					PackedStringArray metas;
					bool in_string = false;
					String current = "";
					for (int i = 0; i < inside.length(); i++)
					{
						char32_t c = inside[i];
						if (c == '"' && (i == 0 || inside[i - 1] != '\\')) in_string = !in_string;
						if (c == ',' && !in_string)
						{
							metas.push_back(current.strip_edges());
							current = "";
						}
						else current += c;
					}
					if (current.length() > 0) metas.push_back(current.strip_edges());
					for (int i = 0; i < metas.size(); i++)
					{
						String m = metas[i].strip_edges();
						int eqm = m.find("=");
						if (eqm != -1)
						{
							String key = m.substr(0, eqm).strip_edges();
							String val = m.substr(eqm + 1).strip_edges();

							if (key == "hint" || key == "usage")
							{
								PackedStringArray tokens = val.split("|", false);
								String out = "";
								for (int t = 0; t < tokens.size(); t++)
								{
									String tok = tokens[t].strip_edges();
									String snake = "";
									for (int k = 0; k < tok.length(); k++)
									{
										char32_t ch = tok[k];
										if (ch >= 'A' && ch <= 'Z' && k > 0) snake += "_";
										snake += String::chr(ch).to_lower();
									}
									snake = snake.to_upper();
									if (key == "hint") out += "PROPERTY_HINT_" + snake;
									else out += "PROPERTY_USAGE_" + snake;
									if (t < tokens.size() - 1) out += " | ";
								}
								val = out;
							}
							if (key == "hint_string" || key == "group")
							{ 
								if (key == "group")
								{
									String g = val.strip_edges();
									if (g.begins_with("\"") && g.ends_with("\"")) g = g.substr(1, g.length() - 2);
									g = g.to_lower().replace(" ", "_");
									String solvedName;
									if (g.is_empty()) solvedName = name; 
									else solvedName = g + "/" + name;
									propertyMappings[name] = solvedName;
								}
								if (!val.begins_with("\"")) val = "\"" + val + "\"";
							}
							cprop += ", ." + key + " = " + val;
						}
					}
				}
				cprop += " };";
				preprocessed += cprop + "\n";
				jenova::json_t prop;
				prop["name"] = GetStdStr(name);
				prop["type"] = GetStdStr(type);
				prop["default"] = GetStdStr(orig_def);
				properties.push_back(prop);
			}
			continue;
		}

		// Handle Functions
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_function)))
		{
			String rest = trimmed.substr(GetDefinerKeyword(DefinerKeyword::dk_function).length()).strip_edges();
			int space = rest.find(" ");
			String returnType = rest.substr(0, space);
			String after = rest.substr(space + 1).strip_edges();
			int paren = after.find("(");
			String fname = after.substr(0, paren).strip_edges();
			String params = after.substr(paren + 1, after.rfind(")") - paren - 1);
			preprocessed += returnType + " " + fname + "(" + _get_instance_base_type() + "* self" + (params.is_empty() ? "" : ", " + params) + ")\n";

			// Parse Function Parameters
			jenova::json_t paramList = jenova::json_t::array();
			PackedStringArray p = params.split(",", false);
			for (int k = 0; k < p.size(); k++)
			{
				String param = p[k].strip_edges();
				if (param == "") continue;
				PackedStringArray pp = param.split(" ", false);
				if (pp.size() >= 2)
				{
					jenova::json_t pmeta;
					pmeta["type"] = GetStdStr(pp[0]);
					pmeta["name"] = GetStdStr(pp[1]);
					paramList.push_back(pmeta);
				}
			}
			
			// Create Metadata
			jenova::json_t fn;
			fn["name"] = GetStdStr(fname);
			fn["return"] = GetStdStr(returnType);
			fn["params"] = paramList;
			functions.push_back(fn);
			continue;
		}

		// Handle Signals
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_signal)))
		{
			String rest = trimmed.substr(GetDefinerKeyword(DefinerKeyword::dk_signal).length()).strip_edges();
			int paren = rest.find("(");
			String sname = rest.substr(0, paren).strip_edges();
			String params = rest.substr(paren + 1, rest.rfind(")") - paren - 1);
			preprocessed += "void " + sname + "(" + _get_instance_base_type() + "* self" + (params.is_empty() ? "" : ", " + params) + ");\n";

			// Parse Signal Parameters
			jenova::json_t paramList = jenova::json_t::array();
			PackedStringArray p = params.split(",", false);
			for (int k = 0; k < p.size(); k++)
			{
				String param = p[k].strip_edges();
				if (param == "") continue;
				PackedStringArray pp = param.split(" ", false);
				if (pp.size() >= 2)
				{
					jenova::json_t pmeta;
					pmeta["type"] = GetStdStr(pp[0]);
					pmeta["name"] = GetStdStr(pp[1]);
					paramList.push_back(pmeta);
				}
			}

			// Create Metadata
			jenova::json_t sig;
			sig["name"] = GetStdStr(sname);
			sig["params"] = paramList;
			signals.push_back(sig);
			continue;
		}

		// Handle Extensions
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_ext_var)) ||
			trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_ext_obj)))
		{
			bool is_var_ext = trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_ext_var));
			String keyword = GetDefinerKeyword(is_var_ext ? DefinerKeyword::dk_ext_var : DefinerKeyword::dk_ext_obj);
			String rest = trimmed.substr(keyword.length()).strip_edges();
			int arrowPos = rest.find("->");
			String signature = arrowPos == -1 ? rest : rest.substr(0, arrowPos).strip_edges();
			String extType = arrowPos == -1 ? "" : rest.substr(arrowPos + 2).strip_edges();
			int space = signature.find(" ");
			String returnType = signature.substr(0, space).strip_edges();
			String after = signature.substr(space + 1).strip_edges();
			int paren = after.find("(");
			String fname = after.substr(0, paren).strip_edges();
			String params = after.substr(paren + 1, after.rfind(")") - paren - 1);

			String receiverType;
			bool isGeneric = extType.is_empty();
			if (is_var_ext) receiverType = isGeneric ? "Variant" : extType;
			else receiverType = isGeneric ? "Object" : (extType);

			preprocessed += returnType + " " + fname + "(" + receiverType + (is_var_ext ? " " : "* ") + (is_var_ext ? "__var" : "__obj") + (params.is_empty() ? "" : ", " + params) + ")\n";

			jenova::json_t paramList = jenova::json_t::array();
			PackedStringArray p = params.split(",", false);
			for (int k = 0; k < p.size(); k++)
			{
				String param = p[k].strip_edges();
				if (param == "") continue;

				PackedStringArray pp = param.split(" ", false);
				if (pp.size() >= 2)
				{
					jenova::json_t pmeta;
					pmeta["type"] = GetStdStr(pp[0]);
					pmeta["name"] = GetStdStr(pp[1]);
					paramList.push_back(pmeta);
				}
			}

			jenova::json_t ext;
			ext["name"] = GetStdStr(fname);
			ext["return"] = GetStdStr(returnType);
			ext["params"] = paramList;
			ext["kind"] = is_var_ext ? "variant" : "object";
			ext["receiverType"] = GetStdStr(receiverType);
			ext["generic"] = isGeneric;

			extensions.push_back(ext);
			continue;
		}

		// Handle Keyword Definitions
		if (trimmed.begins_with(GetDefinerKeyword(DefinerKeyword::dk_key_def)))
		{
			PackedStringArray parts = trimmed.split(" ", false);
			if (parts.size() >= 2)
			{
				// Default Color
				BladeLanguage::get_singleton()->AddKeywordDefinition(parts[1], blade::configuration::bl_symbolColor);

				// Custom Color
				if (parts.size() == 3 && StripQuotes(parts[2]).is_valid_html_color()) BladeLanguage::get_singleton()->AddKeywordDefinition(parts[1], Color(StripQuotes(parts[2])));
			}
			preprocessed += "/*" + line + "*/\n";
			continue;
		}

		// Rewrite Propertiesz
		line = RewriteProperties(line);

		// Rewrite Fields
		line = RewriteFields(line);	
		
		// Rewrite Literals
		line = RewriteLiterals(line);

		// Rewrite Objects
		line = RewriteObjects(line);

		// Rewrite Calls
		line = RewriteCalls(line, prefix);

		// Rewrite Variadics
		line = RewriteVariadics(line, GetStr(blade::syntax::bl_print), prefix);
		line = RewriteVariadics(line, GetStr(blade::syntax::bl_format), prefix);

		// Handle Rest
		preprocessed += prefix + line + "\n";
	}

	// Add Framework Header
	preprocessed = preprocessed.insert(0, BladeLanguage::get_singleton()->GetFrameworkCode());

	// Add Proxy Functions
	preprocessed += GenerateProxies();

	// Store Metadata
	metadata["Source"] = GetStdStr(preprocessed);
	metadata["Path"] = GetStdStr(get_path());
	metadata["Includes"] = includes;
	metadata["Properties"] = properties;
	metadata["Functions"] = functions;
	metadata["Signals"] = signals;
	metadata["Extensions"] = extensions;

	// Return Final Code
	return preprocessed;
}
bool BladeScript::CompileScript(bool dryPass)
{
	// Cooldown -> Yeah, Godot Sucks! :)
	scriptMutex->lock();
	TimePoint now = std::chrono::steady_clock::now();
	if (now - lastCompileTime < std::chrono::milliseconds(200))
	{
		scriptMutex->unlock();
		return true;
	}
	lastCompileTime = now;
	scriptMutex->unlock();

	// Check Hash Cache for Changes
	if (sourceHash != 0 && sourceHash == sourceCode.hash())
	{
		DevLog("Script [%s] Has No Changes, Compile Aborted.", GetCStr(this->get_path())); 
		return true;
	}

	// Validate Script Path
	if (this->get_path().is_empty())
	{
		ErrorLog("Compiler", "Script Has No Path, Compile Failed.");
		return false;
	}
	std::string scriptPath = GetStdStr(this->get_path());

	// Verbose
	DevLog("Compiling Script [%s] Using Blade Compiler...", scriptPath.c_str());

	// Update Script State
	scriptState = BladeState::Compiling;

	// Create Script Cache
	jenova::json_t newMetadata = jenova::json_t();

	// Load Metadata From Package
	if (IsRuntimeExecution())
	{
		String cachePath = GetScriptCachePath(this, true);
		try
		{
			newMetadata = jenova::json_t::parse(GetStdStr(ReadStringFromFile(cachePath)));
		}
		catch(const std::exception&)
		{
			ErrorLog("Runtime", "Failed to Load or Parse Script Metadata Cache for %s.", scriptPath.c_str());
			return false;
		}
	}
	else
	{
		// Preprocess Source Code
		String preprocessedSourceCode = PreProcessScript(newMetadata);
		if (preprocessedSourceCode == "<Parse-Error>")
		{
			ErrorLog("Compiler", "Failed to Parse Script [%s]", scriptPath.c_str());
			InvalidateContext();
			scriptState = BladeState::Error;
			return false;
		}

		// Dump Preprocessed Script (Developer Build Only)
		if (blade::DumpPreprocessedScriptOnDisk)
		{
			WriteStdStringToFile(GetStdStr(GetBladeCacheDirectory() + "/" + GetScriptUID(this) + ".c"), GetStdStr(preprocessedSourceCode));
		}
	}

	// Prepare Script Compiler
	if (scriptContext && !dryPass) InvalidateContext();
	BladeCompiler* compiler = new TinyCCompiler(this);
	if (!compiler->Validate())
	{
		ErrorLog("Compiler", "Compile Failed, Couldn't create C Compiler.");
		scriptState = BladeState::Error;
		return false;
	}
	
	// Configure Compiler
	compiler->AddIncludePath(GetStdStr(GetBladeLibraryDirectory()));

	// Add Symbols
	BindScriptSymbols(compiler);

	// Compile CBlade Script
	if (!compiler->Compile(newMetadata["Source"].get<std::string>()))
	{
		ErrorLog("Compiler", "Failed to Compile CBlade Script [%s].", scriptPath.c_str());
		delete compiler;
		scriptState = BladeState::Error;
		return false;
	}

	// Prepare For Execution
	if (!compiler->Prepare())
	{
		ErrorLog("Compiler", "Failed to Prepare CBlade Code for Execution.");
		delete compiler;
		scriptState = BladeState::Error;
		return false;
	}

	// Post Process Script
	if (!PostProcessScript(compiler, newMetadata))
	{
		ErrorLog("Compiler", "Failed to Post Process CBlade Script.");
		delete compiler;
		scriptState = BladeState::Error;
		return false;
	}

	// Genereate Script Metadata Cache
	if (blade::CacheScriptMetadataOnDisk && !IsRuntimeExecution())
	{
		std::string cachePath = GetStdStr(GetScriptCachePath(this, false));
		if (!WriteStdStringToFile(cachePath, newMetadata.dump(2)))
		{
			ErrorLog("Compiler", "Failed to Store Cache for Script %s On Disk.", scriptPath.c_str());
			delete compiler;
			scriptState = BladeState::Error;
			return false;
		}
	}

	// Verbose
	DevLog("Script [%s]%s Compiled Successfully.", GetStdStr(this->get_path()).c_str(), dryPass ? " Dry" : "");

	// All Good
	if (dryPass) delete compiler; else scriptContext = compiler;
	scriptMetadata = newMetadata;
	sourceHash = sourceCode.hash();
	scriptState = BladeState::Compiled;
	emit_signal("ScriptCompiled");
	return true;
}
bool BladeScript::PostProcessScript(BladeCompiler* compiler, jenova::json_t& metadata)
{
	// Get Script Compiler
	if (!compiler->Validate()) return false;

	// Update Property Storage
	scriptProperties.clear();
	const jenova::json_t& props = metadata["Properties"];
	int count = int(props.size());
	for (int i = 0; i < count; i++)
	{
		char symbolName[32];
		snprintf(symbolName, sizeof(symbolName), "__prop__%03d", i + 1);
		void* sym = compiler->GetSymbol(symbolName);
		if (!sym) continue;
		blade::BladeProperty* bp = reinterpret_cast<blade::BladeProperty*>(sym);

		String group = bp->group;
		if (!group.is_empty())
		{
			group = group.to_lower();
			group = group.replace(" ", "_");
		}

		String finalName = bp->name;
		if (!group.is_empty()) finalName = group + "/" + bp->name;

		PropertyInfo pi;
		pi.type = Variant::Type(bp->type);
		pi.name = finalName;
		if (bp->class_name[0] != '\0') pi.class_name = bp->class_name;

		if (bp->hint == PROPERTY_HINT_RANGE && bp->hint_string[0] == '\0') pi.hint = PROPERTY_HINT_NONE;
		else pi.hint = bp->hint;
		if (bp->hint_string[0] != '\0') pi.hint_string = bp->hint_string;
		int usage = bp->usage;
		if (usage == 0) usage = PROPERTY_USAGE_DEFAULT;
		pi.usage = usage;

		// Add Property to Script
		scriptProperties.push_back(Dictionary(pi));

		// Update Metadata
		metadata["Properties"][i]["solved_name"] = GetStdStr(finalName);
		metadata["Properties"][i]["type"] = bp->type;
		metadata["Properties"][i]["usage"] = pi.usage;
		metadata["Properties"][i]["hint"] = pi.hint;
		metadata["Properties"][i]["hint_string"] = GetStdStr(pi.hint_string);
		if (!group.is_empty()) metadata["Properties"][i]["group"] = GetStdStr(group);
	}

	// Create Extension Metadata
	const jenova::json_t& exts = metadata["Extensions"];
	for (const auto& ext : exts)
	{
		BladeExtension be;

		be.name = ext["name"].get<std::string>();
		be.kind = ext["kind"].get<std::string>();
		be.receiverType = ext["receiverType"].get<std::string>();
		be.returnType = ext["return"].get<std::string>();
		be.isGeneric = ext["generic"].get<bool>();

		const jenova::json_t& params = ext["params"];
		be.paramCount = (int)params.size();
		for (const auto& p : params)
		{
			be.paramTypes.push_back(p["type"].get<std::string>());
			be.paramNames.push_back(p["name"].get<std::string>());
		}

		// Resolve Proxy Function Pointer
		std::string proxyName = "__" + be.name + "___proxy";
		void* sym = compiler->GetSymbol(proxyName);
		if (!sym)
		{
			ErrorLog("Post Processor", "Failed to Resolve Extension Proxy: %s", proxyName.c_str());
			continue;
		}

		if (be.kind == "variant") be.fn.callv = (Variant(*)(Variant, int, Variant*))sym;
		else be.fn.callo = (Variant(*)(void*, int, Variant*))sym;

		// Add/Replace New Extension
		std::string key = be.name;
		extensionStorage[key] = be;
	}

	// All Good
	return true;
}
void BladeScript::EnsureCompiled() const
{
	if (scriptState != BladeState::Compiled) const_cast<BladeScript*>(this)->CompileScript();
}
void BladeScript::InvalidateContext()
{
    if (scriptContext)
	{
        delete scriptContext;
        scriptContext = nullptr;
    }
}
void BladeScript::PerformHotReload(const String& newSourceCode)
{
	this->set_source_code(newSourceCode);
	this->CompileScript();
	DevLog("Script [%s] Hot-Reloaded.", GetStdStr(get_path()).c_str());
}

// Blade Script Instance Implementation
BladeScriptInstance::BladeScriptInstance(Object* p_owner, const Ref<BladeScript> p_script) : owner(p_owner), script(p_script)
{
	// Initialize Event Wrapper
	eventWrapper = Ref<BladeEventWrapper>(memnew(BladeEventWrapper(this)));

	// Connect Events
	script->connect("ScriptCompiled", callable_mp(eventWrapper.ptr(), &BladeEventWrapper::OnScriptReloaded));
}
BladeScriptInstance::~BladeScriptInstance()
{
	// Disconnect Events
	if (script.is_valid()) script->disconnect("ScriptCompiled", callable_mp(eventWrapper.ptr(), &BladeEventWrapper::OnScriptReloaded));

	// Release Event Wrapper
	eventWrapper.unref();
}
bool BladeScriptInstance::set(const StringName& p_name, const Variant& p_value)
{
	// Handle Blade Properties
	if (script->_is_valid())
	{
		const std::string target = GetStdStr(p_name);
		for (const auto& p : script->GetScriptMetadata()["Properties"])
		{
			// Check If It's Blade Property
			if (p.contains("solved_name") && p["solved_name"] == target)
			{
				instanceProperties[p_name] = p_value;
				return true;
			}
		}
	}

	// Not Handled
	return false;
}
bool BladeScriptInstance::get(const StringName& p_name, Variant& r_ret) const
{
	// Get Script [Special]
	if (p_name == StringName("script"))
	{
		r_ret = script;
		return true;
	}

	// Instance-Level Property
	if (instanceProperties.has(p_name))
	{
		r_ret = instanceProperties[p_name];
		return true;
	}

	// Handle Blade Properties
	if (script->_is_valid())
	{
		const std::string target = GetStdStr(p_name);
		for (const auto& p : script->GetScriptMetadata()["Properties"])
		{
			if (p.contains("solved_name") && p["solved_name"] == target)
			{
				r_ret = script->_get_property_default_value(p_name);
				instanceProperties[p_name] = r_ret;
				return true;
			}
		}
	}
	
	// Not Handled
	return false;
}
String BladeScriptInstance::to_string(bool* r_is_valid)
{
	*r_is_valid = true;
	return String("BladeScript::" + script->get_global_name());
}
void BladeScriptInstance::notification(int p_notification, bool p_reversed)
{
	if (p_notification == Object::NOTIFICATION_PREDELETE)
	{
		isDeleting = true;
	}
}
Variant BladeScriptInstance::callp(const StringName& p_method, const Variant** p_args, const int p_argument_count, GDExtensionCallError& r_error)
{

	// Validate Instance & Script
	if (isDeleting || !this->script.is_valid())
	{
		r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	// Verbose [Temp] -> Use this for setting props maybe?
	if (p_method == StringName("_enter_tree"))
	{
		Node* parentNode = Object::cast_to<Node>(owner);
		DevLog("Script Instance for %s Entered Tree.", GetCStr(parentNode->get_name()));
	}

	// Handle Internal Methods
	if (p_method == StringName("_get_editor_name"))
	{
		r_error.error = GDEXTENSION_CALL_OK;
		return Variant(script->get_global_name());
	}
	else if (p_method == StringName("_hide_script_from_inspector"))
	{
		r_error.error = GDEXTENSION_CALL_OK;
		return false;
	}
	else if (p_method == StringName("_is_read_only"))
	{
		r_error.error = GDEXTENSION_CALL_OK;
		return false;
	}

	// Handle Tool Script
	if (Engine::get_singleton()->is_editor_hint() && !script->is_tool())
	{
		r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	// Get Script Execution Context
	BladeCompiler* compiler = script->GetContext();
	if (!compiler || !compiler->Validate())
	{
		DevError("Runtime", "Function `%s` Failed to Call, Instance is Null.", GetCStr(p_method));
		r_error.error = GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL;
		return Variant();
	}

	// Get Function
	std::string proxyName = "_" + GetStdStr(p_method) + "___proxy";
	void* sym = compiler->GetSymbol(proxyName);
	if (!sym)
	{
		r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	// Build Wrapper for Owner
	blade::BladeObject bladeObj;
	bladeObj.ptr = owner;
	strcpy_s(bladeObj.type, sizeof bladeObj.type, GetStdStr(owner->get_class()).c_str());
	bladeObj.ref = owner->is_class("RefCounted");
	bladeObj.singleton = Engine::get_singleton()->has_singleton(owner->get_class());

	// Build Parameters Array
	std::vector<Variant> parameters;
	for (int i = 0; i < p_argument_count; i++) parameters.push_back(*p_args[i]);

	// Invoke Function Using proxy
	using ProxyFunc = Variant(*)(blade::BladeObject*, int, Variant*);
	ProxyFunc func = (ProxyFunc)sym;
	if (func) 
	{
		Variant result = func(&bladeObj, p_argument_count, parameters.data());
		r_error.error = GDEXTENSION_CALL_OK; 
		return result;
	}

	// Default Result [GDEXTENSION_CALL_OK : Handled By Language / GDEXTENSION_CALL_ERROR_INVALID_METHOD : Handle Default]
	r_error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
	return Variant();
}
const GDExtensionMethodInfo* BladeScriptInstance::get_method_list(uint32_t* r_count) const
{
	if (script.is_null())
	{
		*r_count = 0;
		return nullptr;
	}
	TypedArray<Dictionary> method_arr = script->_get_script_method_list();
	const int method_count = method_arr.size();
	if (method_count == 0)
	{
		*r_count = 0;
		return nullptr;
	}
	GDExtensionMethodInfo* list = memnew_arr(GDExtensionMethodInfo, method_count);
	int idx = 0;
	for (int i = 0; i < method_count; i++)
	{
		Dictionary d = method_arr[i];
		MethodInfo mi = MethodInfo::from_dict(d);

		BladeScriptInstance* self = const_cast<BladeScriptInstance*>(this);
		list[idx++] = CreateMethodInfo(mi, self);
	}
	*r_count = method_count;
	return list;
}
void BladeScriptInstance::free_method_list(const GDExtensionMethodInfo* p_list, uint32_t p_count) const
{
	if (p_list) memdelete_arr(p_list);
}
const GDExtensionPropertyInfo* BladeScriptInstance::get_property_list(uint32_t* r_count) const
{
	if (script.is_null())
	{
		*r_count = 0;
		return nullptr;
	}

	TypedArray<Dictionary> arr = script->_get_script_property_list();
	const int size = arr.size();

	if (size == 0)
	{
		*r_count = 0;
		return nullptr;
	}

	GDExtensionPropertyInfo* list = memnew_with_size<GDExtensionPropertyInfo>(size);

	for (int i = 0; i < size; i++)
	{
		Dictionary d = arr[i];
		PropertyInfo pi = PropertyInfo::from_dict(d);

		GDExtensionPropertyInfo info = {};
		info.type = (GDExtensionVariantType)pi.type;
		info.name = AllocateStringName(pi.name);
		info.class_name = AllocateStringName(pi.class_name);
		info.hint = pi.hint;
		info.hint_string = AllocateString(pi.hint_string);
		info.usage = pi.usage;
		list[i] = info;
	}

	*r_count = size;
	return list;
}
void BladeScriptInstance::free_property_list(const GDExtensionPropertyInfo* p_list, uint32_t p_count) const
{
	if (p_list)
	{
		int size = memnew_ptr_size<GDExtensionPropertyInfo>(p_list);
		for (int i = 0; i < size; i++)
		{
			FreePropertyList(p_list[i]);
		}
		memdelete_with_size<GDExtensionPropertyInfo>(p_list);
	}
}
Variant::Type BladeScriptInstance::get_property_type(const StringName& p_name, bool* r_is_valid) const
{
	*r_is_valid = false;
	if (script.is_null()) return Variant::NIL;
	const std::string target = GetStdStr(p_name);
	TypedArray<Dictionary> arr = script->_get_script_property_list();
	for (int i = 0; i < arr.size(); i++)
	{
		PropertyInfo pi = PropertyInfo::from_dict(arr[i]);
		if (GetStdStr(pi.name) == target)
		{
			*r_is_valid = true;
			return pi.type;
		}
	}
	return Variant::NIL;
}
void BladeScriptInstance::get_property_state(GDExtensionScriptInstancePropertyStateAdd p_add_func, void* p_userdata)
{
	p_add_func = AddState; // Needs Investigation
}
bool BladeScriptInstance::validate_property(GDExtensionPropertyInfo& p_property) const
{
	if (!p_property.name) return false;
	StringName property_name = *(StringName*)p_property.name;
	if (instanceProperties.has(property_name)) return true;
	TypedArray<Dictionary> props = script->_get_script_property_list();
	for (int i = 0; i < props.size(); i++)
	{
		PropertyInfo pi = PropertyInfo::from_dict(props[i]);
		if (pi.name == property_name) return true;
	}
	return false;
}
bool BladeScriptInstance::has_method(const StringName& p_name) const
{
	// Internal Godot Functions
	static const Vector<String> godot_functions =
	{
		"_get_editor_name",
		"_hide_script_from_inspector",
		"_is_read_only",
	};
	for (auto& function : godot_functions)
	{
		if (p_name == StringName(function)) return true;
	}

	// Script-Defined Methods
	TypedArray<Dictionary> arr = script->_get_script_method_list();
	for (int i = 0; i < arr.size(); i++)
	{
		MethodInfo mi = MethodInfo::from_dict(arr[i]);
		if (mi.name == p_name) return true;
	}

	// Not Found
	return false;
}
int BladeScriptInstance::get_method_argument_count(const StringName& p_method, bool* r_is_valid) const
{
	*r_is_valid = false;
	if (script.is_null()) return 0;
	const std::string target = GetStdStr(p_method);
	TypedArray<Dictionary> arr = script->_get_script_method_list();
	for (int i = 0; i < arr.size(); i++)
	{
		MethodInfo mi = MethodInfo::from_dict(arr[i]);
		if (GetStdStr(mi.name) == target)
		{
			*r_is_valid = true;
			return mi.arguments.size();
		}
	}
	return 0;
}
bool BladeScriptInstance::property_can_revert(const StringName& p_name) const
{
	if (script.is_null()) return false;
	return script->_has_property_default_value(p_name);
}
bool BladeScriptInstance::property_get_revert(const StringName& p_name, Variant& r_ret) const
{
	if (script.is_null()) return false;
	if (!script->_has_property_default_value(p_name)) return false;
	r_ret = script->_get_property_default_value(p_name);
	return true;
}
void BladeScriptInstance::refcount_incremented()
{
	refCount++;
}
bool BladeScriptInstance::refcount_decremented()
{
	refCount--;
	return false;
}
Object* BladeScriptInstance::get_owner()
{
	return owner;
}
Ref<Script> BladeScriptInstance::get_script() const
{
	return script;
}
bool BladeScriptInstance::is_placeholder() const
{
	return false;
}
void BladeScriptInstance::property_set_fallback(const StringName& p_name, const Variant& p_value, bool* r_valid)
{
	*r_valid = false;
}
Variant BladeScriptInstance::property_get_fallback(const StringName& p_name, bool* r_valid)
{
	*r_valid = false;
	return Variant::NIL;
}
ScriptLanguage* BladeScriptInstance::_get_language()
{
	return BladeLanguage::get_singleton();
}
void BladeScriptInstance::OnScriptReloaded()
{
	DevLog("Reloading Properties on %s", GetCStr(((Node*)owner)->get_name()));
	owner->notify_property_list_changed();
}

// Blade Resource Loader Implementation
void BladeResourceLoader::init()
{
	bladeScriptLoader.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(bladeScriptLoader);
}
void BladeResourceLoader::deinit()
{
	ResourceLoader::get_singleton()->remove_resource_format_loader(bladeScriptLoader);
	bladeScriptLoader.unref();
}
Variant BladeResourceLoader::_load(const String& p_path, const String& original_path, bool use_sub_threads, int32_t cache_mode) const
{
	Ref<BladeScript> Blade;
	Blade.instantiate();
	Blade->set_path(p_path);
	Blade->set_source_code(FileAccess::get_file_as_string(p_path));
	return Blade;
}
PackedStringArray BladeResourceLoader::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.push_back(blade::BladeExtension);
	array.push_back("bl");
	return array;
}
bool BladeResourceLoader::_handles_type(const StringName& type) const
{
	String type_str = type;
	return type_str == StringName(blade::BladeScriptType) || type_str == "Script";
}
String BladeResourceLoader::_get_resource_type(const String& p_path) const
{
	String el = p_path.get_extension().to_lower();
	if (el == String(blade::BladeExtension) || el == "bl") return String(blade::BladeScriptType);
	return "";
}

// Blade Resource Saver Implementation
void BladeResourceSaver::init()
{
	bladeScriptSaver.instantiate();
	ResourceSaver::get_singleton()->add_resource_format_saver(bladeScriptSaver);
}
void BladeResourceSaver::deinit()
{
	ResourceSaver::get_singleton()->remove_resource_format_saver(bladeScriptSaver);
	bladeScriptSaver.unref();
}
Error BladeResourceSaver::_save(const Ref<Resource>& p_resource, const String& p_path, uint32_t p_flags)
{
	BladeScript* script = Object::cast_to<BladeScript>(p_resource.ptr());
	if (script != nullptr)
	{
		Ref<FileAccess> handle = FileAccess::open(p_path, FileAccess::ModeFlags::WRITE);
		if (handle.is_valid())
		{
			handle->store_string(script->_get_source_code());
			handle->close();
			return Error::OK;
		}
		else
		{
			return Error::ERR_FILE_CANT_OPEN;
		}
	}
	return Error::ERR_SCRIPT_FAILED;
}
Error BladeResourceSaver::_set_uid(const String& p_path, int64_t p_uid)
{
	return Error::OK;
}
bool BladeResourceSaver::_recognize(const Ref<Resource>& p_resource) const
{
	return Object::cast_to<BladeScript>(p_resource.ptr()) != nullptr;
}
PackedStringArray BladeResourceSaver::_get_recognized_extensions(const Ref<Resource>& p_resource) const
{
	PackedStringArray array;
	if (Object::cast_to<BladeScript>(p_resource.ptr()) != nullptr)
	{
		array.push_back(blade::BladeExtension);
		array.push_back("bl");
	}
	return array;
}
bool BladeResourceSaver::_recognize_path(const Ref<Resource>& p_resource, const String& p_path) const
{
	return Object::cast_to<BladeScript>(p_resource.ptr()) != nullptr;
}

// Blade Syntax Highlighter Implementation
class BladeEditorSyntaxHighlighter : public EditorSyntaxHighlighter
{
    GDCLASS(BladeEditorSyntaxHighlighter, EditorSyntaxHighlighter);

protected:
    static void _bind_methods() {}

private:
    PackedStringArray typesWords;
    PackedStringArray reservedWords;
    PackedStringArray controlFlowWords;
    PackedStringArray commentDelimiters;
    PackedStringArray stringDelimiters;

public:
    BladeEditorSyntaxHighlighter()
    {
		typesWords =
		{
			"Void", "void", "const",
			"Bool", "bool",
			"Int", "int",
			"Float", "float", "double",
			"String", "char", "wchar_t",
			"Vector2",
			"Vector2i",
			"Rect2",
			"Rect2i",
			"Vector3",
			"Vector3i",
			"Transform2D",
			"Vector4",
			"Vector4i",
			"Plane",
			"Quaternion",
			"AABB",
			"Basis",
			"Transform3D",
			"Projection",
			"Color",
			"StringName",
			"NodePath",
			"RID",
			"Instance",
			"Callable",
			"Signal",
			"Dictionary",
			"Array",
			"PackedByteArray",
			"PackedInt32Array",
			"PackedInt64Array",
			"PackedFloat32Array",
			"PackedFloat64Array",
			"PackedStringArray",
			"PackedVector2Array",
			"PackedVector3Array",
			"PackedColorArray",
			"PackedVector4Array"
        };
    }

public:
    String _get_name() const override
    {
        return "Blade Language";
    }
    PackedStringArray _get_supported_languages() const override
    {
        return PackedStringArray{ blade::BladeLangaugeName, blade::BladeExtension };
    }
    Ref<EditorSyntaxHighlighter> _create() const override
    {
        Ref<BladeEditorSyntaxHighlighter> highlighter;
        highlighter.instantiate();
        return highlighter;
    }
    Dictionary _get_line_syntax_highlighting(int32_t p_line) const override
    {
		// Import Configuration
		using namespace blade::configuration;

		// Final Color
        Dictionary colorMap;

		// Get Content
        TextEdit *textEdit = get_text_edit();
        if (!textEdit) return colorMap;
        String line = textEdit->get_line(p_line);
        int length = line.length();
        if (length == 0) return colorMap;

		// Detect Special Lines
		bool isPropLine = line.strip_edges().begins_with("prop ");
		bool inPropOptions = false;

		// Perform Highlighting
        Color defaultColor = textEdit->get_theme_color("font_color", "TextEdit");
        Color previousColor = defaultColor;
        int index = 0;
        while (index < length)
        {
            Color currentColor = defaultColor;
            char32_t ch = line[index];
			for (int i = 0; i < commentDelimiters.size(); i++)
			{
				const String &delim = commentDelimiters[i];
				int dlen = delim.length();
				if (dlen > 0 && index + dlen <= length && line.substr(index, dlen) == delim)
				{
					currentColor = bl_commentColor;
					if (currentColor != previousColor)
					{
						Dictionary info;
						info["color"] = currentColor;
						colorMap[index] = info;
						previousColor = currentColor;
					}
					return colorMap;
				}
			}
			bool matchedString = false;
			for (int i = 0; i < stringDelimiters.size(); i++)
			{
				const String &delim = stringDelimiters[i];
				if (delim.length() < 1) continue;
				char32_t quote = delim[0];
				if (ch == quote)
				{
					matchedString = true;
					Color currentColor = inPropOptions ? bl_propOptionValueColor : bl_stringColor;
					if (currentColor != previousColor)
					{
						Dictionary info;
						info["color"] = currentColor;
						colorMap[index] = info;
						previousColor = currentColor;
					}
					int stringEnd = index + 1;
					while (stringEnd < length && line[stringEnd] != quote) stringEnd++;
					if (stringEnd < length && line[stringEnd] == quote) stringEnd++;
					index = stringEnd;
					if (index < length)
					{
						Color resetColor = defaultColor;
						if (resetColor != previousColor)
						{
							Dictionary info2;
							info2["color"] = resetColor;
							colorMap[index] = info2;
							previousColor = resetColor;
						}
					}
					break;
				}
			}
			if (matchedString) continue;
			if (ch == '[') inPropOptions = true;
			if (ch == ']') inPropOptions = false;
			if (ch == '$' || ch == '@')
			{
				currentColor = bl_memberVariableColor;
				if (currentColor != previousColor)
				{
					Dictionary info;
					info["color"] = currentColor;
					colorMap[index] = info;
					previousColor = currentColor;
				}
				index++;
				while (index < length && isIdentifierChar(line[index])) index++;
				continue;
			}
			if (isDigit(ch))
			{
				currentColor = inPropOptions ? bl_propOptionValueColor : bl_numberColor;
				if (currentColor != previousColor)
				{
					Dictionary info;
					info["color"] = currentColor;
					colorMap[index] = info;
					previousColor = currentColor;
				}
				index++;
				bool hasDot = false;
				while (index < length)
				{
					char32_t c2 = line[index];
					if (isDigit(c2)) { index++; continue; }
					if (c2 == '.' && !hasDot) { hasDot = true; index++; continue; }
					break;
				}
				continue;
			}
			if (isIdentifierStart(ch))
			{
				int start = index;
				index++;
				while (index < length && isIdentifierChar(line[index])) index++;
				String word = line.substr(start, index - start);

				if (isPropLine && inPropOptions)
				{
					if (word == "hint" || word == "hint_string" || word == "usage" || word == "group")
					{
						currentColor = bl_propOptionColor;
					}
					else
					{
						currentColor = bl_propOptionValueColor;
					}
				}
				else
				{
					if (typesWords.has(word))
					{
						currentColor = bl_typeColor;
					}
					else if (reservedWords.has(word))
					{
						currentColor = bl_keywordColor;
						if (controlFlowWords.has(word)) currentColor = bl_controlFlowColor;
						if (word == GetStr(blade::syntax::bl_true) || word == GetStr(blade::syntax::bl_false)) currentColor = bl_booleanColor;
						if (word == GetStr(blade::syntax::bl_blade_on) || word == GetStr(blade::syntax::bl_blade_off)) currentColor = bl_switchesColor;					
						if (word == GetStr(blade::syntax::bl_format) || word == GetStr(blade::syntax::bl_print) || word == GetStr(blade::syntax::bl_get_singleton)
							|| word == GetStr(blade::syntax::bl_get_object) || word == GetStr(blade::syntax::bl_make_callable) || word == GetStr(blade::syntax::bl_load_library) 
							|| word == GetStr(blade::syntax::bl_get_function)) 
							currentColor = bl_utilsColor;
					}
					else if (ClassDB::class_exists(word))
					{
						currentColor = isPropLine ? bl_typeColor : bl_classColor;
					}
					else if (BladeLanguage::get_singleton()->IsKeywordDefinition(word))
					{
						currentColor = BladeLanguage::get_singleton()->GetKeywordDefinitionColor(word);
					}
				}
				int look = index;
				while (look < length && isSpace(line[look])) look++;
				if (look < length && line[look] == '(' && currentColor == defaultColor)
				{
					currentColor = bl_functionColor;
				}
				if (currentColor != previousColor)
				{
					Dictionary info;
					info["color"] = currentColor;
					colorMap[start] = info;
					previousColor = currentColor;
				}
				continue;
			}
			if (index + 1 < length)
			{
				if (line[index] == '<' && line[index + 1] == '-')
				{
					Color currentColor = bl_variantAccessColor;
					if (currentColor != previousColor)
					{
						Dictionary info;
						info["color"] = currentColor;
						colorMap[index] = info;
						previousColor = currentColor;
					}
					index += 2;
					continue;
				}
				if (line[index] == '-' && line[index + 1] == '>')
				{
					Color currentColor = bl_objectAccessColor;
					if (currentColor != previousColor)
					{
						Dictionary info;
						info["color"] = currentColor;
						colorMap[index] = info;
						previousColor = currentColor;
					}
					index += 2;
					continue;
				}
			}
			if (isSymbol(ch))
			{
				currentColor = bl_symbolColor;
				if (currentColor != previousColor)
				{
					Dictionary info;
					info["color"] = currentColor;
					colorMap[index] = info;
					previousColor = currentColor;
				}
				index++;
				continue;
			}
			if (currentColor != previousColor)
			{
				Dictionary info;
				info["color"] = currentColor;
				colorMap[index] = info;
				previousColor = currentColor;
			}
			index++;
		}
		return colorMap;
	}
    void _clear_highlighting_cache() override
    {
    }
	void _update_cache() override
	{
		BladeLanguage* lang = BladeLanguage::get_singleton();
		if (!lang) return;
		reservedWords = lang->_get_reserved_words();
		controlFlowWords.clear();
		for (int i = 0; i < reservedWords.size(); i++)
		{
			String w = reservedWords[i];
			if (lang->_is_control_flow_keyword(w)) controlFlowWords.push_back(w);
		}
		commentDelimiters = lang->_get_comment_delimiters();
		stringDelimiters = lang->_get_string_delimiters();
	}

private:
    static bool isDigit(char32_t ch)
    {
        return ch >= '0' && ch <= '9';
    }
    static bool isLetter(char32_t ch)
    {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    }
    static bool isIdentifierStart(char32_t ch)
    {
        return isLetter(ch) || ch == '_';
    }
    static bool isIdentifierChar(char32_t ch)
    {
        return isLetter(ch) || isDigit(ch) || ch == '_';
    }
    static bool isSpace(char32_t ch)
    {
        return ch == ' ' || ch == '\t';
    }
    static bool isSymbol(char32_t ch)
    {
        switch (ch)
        {
            case '{':
            case '}':
            case '(':
            case ')':
            case '[':
            case ']':
            case '<':
            case '>':
            case '+':
            case '-':
            case '*':
            case '/':
            case '=':
            case '!':
            case '&':
            case '|':
            case ',':
            case ';':
            case ':':
            case '#':
                return true;
            default:
                return false;
        }
    }
};

// Blade Debugger Plugin Implementation
class BladeDebuggerPlugin : public EditorDebuggerPlugin
{
	GDCLASS(BladeDebuggerPlugin, EditorDebuggerPlugin);

private:
	static void _bind_methods() {}

public:
	static inline Ref<BladeDebuggerPlugin> singleton;
};

// Blade Exporter Plugin Implementation
class BladeExporterPlugin : public EditorExportPlugin 
{
	GDCLASS(BladeExporterPlugin, EditorExportPlugin);

private:
	std::string exportDirectory = "";
	static void _bind_methods() {}

public:
	String _get_name() const override { return "Blade Runtime Exporter"; }
	bool _supports_platform(const Ref<EditorExportPlatform>& p_platform) const override
	{
		// Supports All Platforms
		return true;
	}

	// Events
	void _export_begin(const PackedStringArray& p_features, bool p_is_debug, const String& p_path, uint32_t p_flags) override
	{
		// Verbose Build Start
		Verbose("[color=#729bed][Builder][/color] Exporting Blade Runtime Package...");

		// Set Export Directory
		exportDirectory = std::filesystem::absolute(std::filesystem::path(GetStdStr(p_path)).parent_path()).string() + "/";

		// Bundle API Database
		String bindingsDatabaseContent = ReadStringFromFile(GetBladeBindingsDatabasePath());
		if (bindingsDatabaseContent.is_empty())
		{
			ErrorLog("Blade Exporter", "Failed to Bundle API Database, Make Sure it's Generated and Exists.");
			return;
		}
		add_file(blade::BladeBindingsDatabaseBundlePath, bindingsDatabaseContent.to_utf8_buffer(), false);
		Verbose("[color=#729bed][Builder][/color] API Database Bundled.");

		// Bundle Scripts
		int scriptCount = 0;
		for (const auto& scriptCache : CollectBladeCacheFiles())
		{
			String scriptCacheContent = ReadStringFromFile(scriptCache);
			if (scriptCacheContent.is_empty())
			{
				ErrorLog("Blade Exporter", "Failed to Bundle Script Cache %s.", GetStdStr(scriptCache).c_str());
			}
			add_file(blade::BladeScriptCacheRuntimePath + scriptCache.get_file(), scriptCacheContent.to_utf8_buffer(), false);
			scriptCount++;
		}
		Verbose("[color=#729bed][Builder][/color] %d Blade Scripts Bundled.", scriptCount);
	}
	void _export_end() override
	{
		// Verbose Build Success
		Verbose("[color=#729bed][Builder][/color] Blade Runtime Package Successfully Built and Exported.");
	}
	void _export_file(const String& p_path, const String& p_type, const PackedStringArray& p_features) override 
	{
		// Exclude Blade Script Files
		if (p_path.to_lower().ends_with(".blade")) skip();
	}

public:
	static inline Ref<BladeExporterPlugin> singleton;
};

// Blade Editor Plugin Implementation
void BladePlugin::_bind_methods()
{
}
void BladePlugin::_enter_tree()
{
	// Assign Singleton
	this->signleton = this;

	// Obtain Theme
	Ref<Theme> editor_theme = get_editor_interface()->get_editor_theme();
	double scaleFactor = get_editor_interface()->get_editor_scale();

	// Register Blade Script Icon
	if (!editor_theme->has_icon(blade::BladeScriptType, "EditorIcons"))
	{
		Vector2 iconSize = Vector2i(18 * scaleFactor, 18 * scaleFactor);
		Ref<ImageTexture> iconImage = CreateSVGFromByteArray(jenova::resources::SVG_BLADE_SCRIPT_ICON, sizeof jenova::resources::SVG_BLADE_SCRIPT_ICON, iconSize);
		if (iconImage != nullptr) editor_theme->set_icon(blade::BladeScriptType, "EditorIcons", iconImage);
	}

	// Register Debugger Plugin
	BladeDebuggerPlugin::singleton.instantiate();
	add_debugger_plugin(BladeDebuggerPlugin::singleton);

	// Register Exporter Plugin
	BladeExporterPlugin::singleton.instantiate();
	add_export_plugin(BladeExporterPlugin::singleton);

	// Add Tool Menus
	add_tool_menu_item("Generate Blade Bindings Database", callable_mp(this, &BladePlugin::HandleAction).bind("GenHeaders"));

	// Register Syntax Highlighter
	Ref<BladeEditorSyntaxHighlighter> highlighter; highlighter.instantiate();
	get_editor_interface()->get_script_editor()->register_syntax_highlighter(highlighter);

	// Verbose
	Verbose("[color=#24ffab][Success][/color] Blade Editor Plugin Initialized.");
}
void BladePlugin::_exit_tree()
{
	// Unregister Debugger Plugin
	remove_debugger_plugin(BladeDebuggerPlugin::singleton);
	BladeDebuggerPlugin::singleton.unref();

	// Unregister Exporter Plugin
	remove_export_plugin(BladeExporterPlugin::singleton);
	BladeExporterPlugin::singleton.unref();

	// Remove Tool Menus
	remove_tool_menu_item("Generate Blade Bindings Database");

	// Unregister Syntax Highlighter
	Ref<BladeEditorSyntaxHighlighter> highlighter; highlighter.instantiate();
	EditorInterface::get_singleton()->get_script_editor()->unregister_syntax_highlighter(highlighter);

	// Unassign Singleton
	this->signleton = nullptr;
}
Ref<EditorDebuggerPlugin> BladePlugin::GetDebugger() const
{
	return BladeDebuggerPlugin::singleton;
}
void BladePlugin::HandleAction(const String& action)
{
	if (action == "GenHeaders")
	{
		if (!BladeLanguage::get_singleton()->CreateBindings())
		{
			ErrorLog("Bindings Generator", "Failed to Generate Blade Bindings.");
		};
	}
}

// Register/Unregister Routines
void RegisterBlade(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		// Verbose [Jenova]
		jenova::Output("Initializing Blade Language...");

		// Register Classes
		ClassDB::register_internal_class<BladeScript>();
		ClassDB::register_internal_class<BladeLanguage>();
		ClassDB::register_internal_class<BladeResourceLoader>();
		ClassDB::register_internal_class<BladeResourceSaver>();
		ClassDB::register_internal_class<BladeEventWrapper>();

		// Initialize Classes
		BladeLanguage::init();
		BladeResourceLoader::init();
		BladeResourceSaver::init();

		// Print Banner
		if(blade::configuration::bl_print_banner) PrintBladeBanner();

		// Verbose [Blade]
		Verbose("[color=#24ffab][Success][/color] Blade Runtime %s Initialized.", blade::BladeVersion);
	}
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
	{
		// Register Editor Classes
		ClassDB::register_internal_class<BladePlugin>();
		ClassDB::register_internal_class<BladeDebuggerPlugin>();
		ClassDB::register_internal_class<BladeExporterPlugin>();
		ClassDB::register_internal_class<BladeEditorSyntaxHighlighter>();	

		// Register Editor Plugins
		EditorPlugins::add_by_type<BladePlugin>();
	}
}
void UnregisterBlade(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		// Release Blade Language
		if (BladeLanguage::get_singleton()) BladeLanguage::get_singleton()->_release();

		// Uninitialize Classes
		BladeLanguage::deinit();
		BladeResourceLoader::deinit();
		BladeResourceSaver::deinit();
	}
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
	{
		// Unregister Editor Plugins
		EditorPlugins::remove_by_type<BladePlugin>();
	}
}
