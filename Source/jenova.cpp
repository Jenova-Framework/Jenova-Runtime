
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
#include "BuiltinFonts.h"
#include "Documentation.h"
#include "JenovaIcon64.h"
#include "TypesIcons.h"
#include "AboutImage.h"

// Internal/Built-In Sources
#include "InternalSources.h"
#include "InternalModules.h"

// Internal/Built-In Templates
#include "VisualStudioTemplates.h"

// Third-Party
#include <Parsers/argparse.hpp>
#include <Zlib/zlib.h>

// Namespaces
using namespace std;

// Jenova Core Implementations
namespace jenova
{
	// Global Storage
	namespace GlobalStorage
	{
		// Holders
		ExtensionInitializerData ExtensionInitData = {0};

		// Configurations
		jenova::EngineMode CurrentEngineMode = jenova::EngineMode::Unknown;
		jenova::ProfilingMode CurrentProfilingMode = jenova::ProfilingMode::Disabled;
		jenova::BuildAndRunMode CurrentBuildAndRunMode = jenova::BuildAndRunMode::DoNothing;
		jenova::ChangesTriggerMode CurrentChangesTriggerMode = jenova::ChangesTriggerMode::DoNothing;
		jenova::EditorVerboseOutput CurrentEditorVerboseOutput = jenova::EditorVerboseOutput::StandardOutput;

		// Database
		std::string CurrentJenovaCacheDirectory = "";
		std::string CurrentJenovaGeneratedConfiguration = "";
		std::string CurrentJenovaRuntimeModulePath = "";

		// Flags
		bool DeveloperModeActivated = jenova::GlobalSettings::VerboseEnabled;
		bool UseHotReloadAtRuntime = true;
		bool UseMonospaceFontForTerminal = true;
		bool UseManagedSafeExecution = true;
		bool UseBuiltinSDK = true;
		bool RefreshSceneTreeAfterBuild = false;

		// Values
		int TerminalDefaultFontSize = 12;
	}

	// Forward Declarations
	bool WriteStdStringToFile(const std::string& filePath, const std::string& str);

	// Utilities & Helpers
	void ExitWithCode(int exitCode)
	{
		std::quick_exit(exitCode);
	}
	std::string Format(const char* fmt, ...)
	{
		char buffer[jenova::GlobalSettings::FormatBufferSize];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		return std::string(buffer);
	}
	std::string GetNameFromPath(godot::String gstr)
	{
		std::string path((char*)gstr.to_utf8_buffer().ptr(), gstr.to_utf8_buffer().size());
		size_t pos = path.find("res://");
		if (pos != std::string::npos){ path.erase(pos, 6); }
		return path;
	}
	std::string GenerateRandomHashString()
	{
		const int hashLength = 16;
		const char hexChars[] = "0123456789ABCDEF";
		std::stringstream ss;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 15);
		for (int i = 0; i < hashLength; ++i) ss << hexChars[dis(gen)];
		return ss.str();
	}
	std::string GenerateTerminalLogTime()
	{
		auto now = std::chrono::system_clock::now();
		auto now_time_t = std::chrono::system_clock::to_time_t(now);
		auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		std::tm now_tm;
		#if defined(_WIN32) || defined(_WIN64)
			localtime_s(&now_tm, &now_time_t);
		#else
			localtime_r(&now_time_t, &now_tm);
		#endif

		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(2) << now_tm.tm_hour << ":"
			<< std::setw(2) << now_tm.tm_min << ":"
			<< std::setw(2) << now_tm.tm_sec << ":"
			<< std::setw(4) << now_ms.count();
		return oss.str();
	}
	jenova::ArgumentsArray CreateArgumentsArrayFromString(const std::string& str, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		bool insideQuotes = false;
		std::stringstream ss;
		for (char c : str)
		{
			if (c == '"')
			{
				insideQuotes = !insideQuotes;
			}
			else if (c == delimiter && !insideQuotes)
			{
				if (!ss.str().empty())
				{
					tokens.push_back(ss.str());
					ss.str("");
					ss.clear();
				}
			}
			else
			{
				ss << c;
			}
		}
		if (!ss.str().empty()) tokens.push_back(ss.str());
		return tokens;
	}
	String RemoveCommentsFromSource(const String& sourceCode) 
	{
		String comment_pattern = R"(\/\/[^\n]*|\/\*[\s\S]*?\*\/)";
		Ref<RegEx> regex = RegEx::create_from_string(comment_pattern);
		return regex->sub(sourceCode, "", true);
	}
	bool ContainsExactString(const String& srcStr, const String& matchStr) 
	{
		String pattern = "\\b" + matchStr + "\\b";
		Ref<RegEx> regex = RegEx::create_from_string(pattern);
		return regex->search(srcStr).is_valid();
	}
	bool CreateFileFromInternalSource(const std::string& sourceFile, const std::string& sourceCode)
	{
		return WriteStdStringToFile(sourceFile, sourceCode);
	}
	jenova::MemoryBuffer CompressBuffer(void* bufferPtr, size_t bufferSize)
	{
		try
		{
			jenova::MemoryBuffer buffer;
			const size_t BUFSIZE = 128 * 1024;
			std::unique_ptr<uint8_t[]> temp_buffer(new uint8_t[BUFSIZE]);

			z_stream strm;
			strm.zalloc = 0;
			strm.zfree = 0;
			strm.next_in = reinterpret_cast<uint8_t*>(bufferPtr);
			strm.avail_in = uInt(bufferSize);
			strm.next_out = temp_buffer.get();
			strm.avail_out = BUFSIZE;

			deflateInit(&strm, Z_BEST_COMPRESSION);

			while (strm.avail_in != 0)
			{
				int res = deflate(&strm, Z_NO_FLUSH);
				assert(res == Z_OK);
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
			}

			int deflate_res = Z_OK;
			while (deflate_res == Z_OK)
			{
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
				deflate_res = deflate(&strm, Z_FINISH);
			}

			assert(deflate_res == Z_STREAM_END);
			buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE - strm.avail_out);
			deflateEnd(&strm);

			return buffer;
		}
		catch (const std::exception&)
		{
			return jenova::MemoryBuffer();
		}
	}
	jenova::MemoryBuffer DecompressBuffer(void* bufferPtr, size_t bufferSize)
	{
		try
		{
			jenova::MemoryBuffer buffer;
			const size_t BUFSIZE = 128 * 1024;
			std::unique_ptr<uint8_t[]> temp_buffer(new uint8_t[BUFSIZE]);

			z_stream strm;
			strm.zalloc = 0;
			strm.zfree = 0;
			strm.next_in = reinterpret_cast<uint8_t*>(bufferPtr);
			strm.avail_in = uInt(bufferSize);
			strm.next_out = temp_buffer.get();
			strm.avail_out = BUFSIZE;

			inflateInit(&strm);

			while (strm.avail_in != 0)
			{
				int res = inflate(&strm, Z_NO_FLUSH);
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
			}

			int inflate_res = Z_OK;
			while (inflate_res == Z_OK)
			{
				if (strm.avail_out == 0)
				{
					buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE);
					strm.next_out = temp_buffer.get();
					strm.avail_out = BUFSIZE;
				}
				inflate_res = inflate(&strm, Z_FINISH);
			}

			assert(inflate_res == Z_STREAM_END);
			buffer.insert(buffer.end(), temp_buffer.get(), temp_buffer.get() + BUFSIZE - strm.avail_out);
			inflateEnd(&strm);

			return buffer;
		}
		catch (const std::exception&)
		{
			return jenova::MemoryBuffer();
		}
	}
	jenova::ArgumentsArray ProcessDeployerArguments(const std::string& cmdLine)
	{
		std::regex paramRegex(R"(/([^:\s]+):\"([^\"]+)\")");
		std::string processedCmdLine = std::regex_replace(cmdLine, paramRegex, "--$1 \"$2\"");
		return jenova::CreateArgumentsArrayFromString(processedCmdLine, ' ');
	}
	bool WriteStringToFile(const String& filePath, const String& str)
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
	String ReadStringFromFile(const String& filePath)
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
	bool WriteStdStringToFile(const std::string& filePath, const std::string& str)
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
	std::string ReadStdStringFromFile(const std::string& filePath)
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
	bool WriteWideStdStringToFile(const std::wstring& filePath, const std::wstring& str)
	{
		// Create Stream
		#if defined(TARGET_PLATFORM_WINDOWS) && defined(_MSC_VER)
			std::wofstream outFile(filePath, std::ios::out | std::ios::binary);
		#else
			std::wofstream outFile(jenova::Format("%S", filePath.c_str()).c_str(), std::ios::out | std::ios::binary);
		#endif

		// Write File
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
	std::wstring ReadWideStdStringFromFile(const std::wstring& filePath)
	{
		// Create Stream
		#ifdef defined(TARGET_PLATFORM_WINDOWS) && defined(_MSC_VER)
			std::wifstream inFile(filePath);
		#else
			std::wifstream inFile(jenova::Format("%S", filePath.c_str()).c_str());
		#endif

		// Read File
		if (inFile.is_open())
		{
			std::wstring content((std::istreambuf_iterator<wchar_t>(inFile)), std::istreambuf_iterator<wchar_t>());
			inFile.close();
			return content;
		}
		else
		{
			return L"";
		}
	}
	void ReplaceAllMatchesWithString(std::string& targetString, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while ((start_pos = targetString.find(from, start_pos)) != std::string::npos)
		{
			targetString.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}
	std::string ReplaceAllMatchesWithStringAndReturn(std::string targetString, const std::string& from, const std::string& to)
	{
		ReplaceAllMatchesWithString(targetString, from, to);
		return targetString;
	}
	jenova::ArgumentsArray SplitStdStringToArguments(const std::string& str, char delimiter)
	{
		ArgumentsArray arguments;
		std::stringstream ss(str);
		std::string argument;
		while (std::getline(ss, argument, delimiter)) if (!argument.empty()) arguments.push_back(argument);
		return arguments;
	}
	std::string NormalizePath(const std::string& input)
	{
		std::string result;
		char prevChar = '\0';
		for (char c : input)
		{
			if (c == '/' && prevChar == '/')
				continue;
			result += (c == '\\') ? '/' : c;
			prevChar = c;
		}
		return result;
	}
	bool RemoveFileEncodingInStdString(std::string& fileContent) 
	{
		// Define BOMs for UTF Encodings
		static const unsigned char UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };
		static const unsigned char UTF16_LE_BOM[] = { 0xFF, 0xFE };
		static const unsigned char UTF16_BE_BOM[] = { 0xFE, 0xFF };
		static const unsigned char UTF32_LE_BOM[] = { 0xFF, 0xFE, 0x00, 0x00 };
		static const unsigned char UTF32_BE_BOM[] = { 0x00, 0x00, 0xFE, 0xFF };

		// Check for UTF-8 BOM
		if (fileContent.size() >= 3 && std::memcmp(fileContent.data(), UTF8_BOM, 3) == 0)
		{
			fileContent.erase(0, 3);
			return true;
		}

		// Check for UTF-16 LE BOM
		if (fileContent.size() >= 2 && std::memcmp(fileContent.data(), UTF16_LE_BOM, 2) == 0)
		{
			fileContent.erase(0, 2);
			return true;
		}

		// Check for UTF-16 BE BOM
		if (fileContent.size() >= 2 && std::memcmp(fileContent.data(), UTF16_BE_BOM, 2) == 0)
		{
			fileContent.erase(0, 2);
			return true;
		}

		// Check for UTF-32 LE BOM
		if (fileContent.size() >= 4 && std::memcmp(fileContent.data(), UTF32_LE_BOM, 4) == 0)
		{
			fileContent.erase(0, 4);
			return true;
		}

		// Check for UTF-32 BE BOM
		if (fileContent.size() >= 4 && std::memcmp(fileContent.data(), UTF32_BE_BOM, 4) == 0)
		{
			fileContent.erase(0, 4);
			return true;
		}

		// No BOM Detected
		return false;
	}
	bool ApplyFileEncodingFromReferenceFile(const std::string& sourceFile, const std::string& destinationFile)
	{
		// Helper Functions
		auto DetectEncodingBOM = [](const std::vector<unsigned char>& bytes) -> std::string
		{
			if (bytes.size() >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
			{
				return "UTF-8";
			}
			else if (bytes.size() >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
			{
				return "UTF-16LE";
			}
			else if (bytes.size() >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF)
			{
				return "UTF-16BE";
			}
			else if (bytes.size() >= 4 && bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xFE && bytes[3] == 0xFF)
			{
				return "UTF-32BE";
			}
			else if (bytes.size() >= 4 && bytes[0] == 0xFF && bytes[1] == 0xFE && bytes[2] == 0x00 && bytes[3] == 0x00)
			{
				return "UTF-32LE";
			}
			return "None"; // No BOM detected
		};
		auto GetBOMBytes = [](const std::string& encoding) -> std::vector<unsigned char>
		{
			if (encoding == "UTF-8")
			{
				return { 0xEF, 0xBB, 0xBF };
			}
			else if (encoding == "UTF-16LE")
			{
				return { 0xFF, 0xFE };
			}
			else if (encoding == "UTF-16BE")
			{
				return { 0xFE, 0xFF };
			}
			else if (encoding == "UTF-32BE")
			{
				return { 0x00, 0x00, 0xFE, 0xFF };
			}
			else if (encoding == "UTF-32LE")
			{
				return { 0xFF, 0xFE, 0x00, 0x00 };
			}
			return {}; // No BOM
		};

		// Read First 5 bytes of Source File
		std::ifstream srcFile(sourceFile, std::ios::binary);
		if (!srcFile.is_open()) return false;
		std::vector<unsigned char> srcBytes(5);
		srcFile.read(reinterpret_cast<char*>(srcBytes.data()), 5);
		srcFile.close();

		// Get Source Encoding
		std::string sourceEncoding = DetectEncodingBOM(srcBytes);

		// Read First 5 Bytes of Destination File
		std::ifstream destFile(destinationFile, std::ios::binary);
		if (!destFile.is_open()) return false;
		std::vector<unsigned char> destBytes(5);
		destFile.read(reinterpret_cast<char*>(destBytes.data()), 5);
		destFile.close();

		// Get Destination Encoding
		std::string destinationEncoding = DetectEncodingBOM(destBytes);

		// If Destination File has a BOM Replace it, If it Doesn't Add the source BOM at Beginning
		std::vector<unsigned char> newBOM = GetBOMBytes(sourceEncoding);
		std::ifstream inDest(destinationFile, std::ios::binary);
		if (!inDest.is_open()) return false;
		std::vector<unsigned char> fileContent((std::istreambuf_iterator<char>(inDest)), std::istreambuf_iterator<char>());
		inDest.close();

		// Replace BOM
		if (!destinationEncoding.empty() && destinationEncoding != "None")
		{
			size_t bomSize = GetBOMBytes(destinationEncoding).size();
			fileContent.erase(fileContent.begin(), fileContent.begin() + bomSize);
		}

		// Prepend Source BOM
		fileContent.insert(fileContent.begin(), newBOM.begin(), newBOM.end());

		// Write Back Updated Content
		std::ofstream outDest(destinationFile, std::ios::binary);
		if (!outDest.is_open()) return false;
		outDest.write(reinterpret_cast<const char*>(fileContent.data()), fileContent.size());
		outDest.close();

		// All Good
		return true;
	}
	jenova::EncodedData CreateCompressedBase64FromStdString(const std::string& srcStr)
	{
		jenova::MemoryBuffer compressedData = CompressBuffer((void*)srcStr.data(), srcStr.size());
		jenova::EncodedData base64Data = base64::base64_encode(compressedData.data(), compressedData.size());
		jenova::MemoryBuffer().swap(compressedData);
		return base64Data;
	}
	std::string CreateStdStringFromCompressedBase64(const jenova::EncodedData& base64)
	{
		DecodedData decodedData = base64::base64_decode(base64);
		jenova::MemoryBuffer decompressedData = DecompressBuffer((void*)decodedData.data(), decodedData.size());
		std::string decodedString((char*)decompressedData.data(), decompressedData.size());
		jenova::MemoryBuffer().swap(decompressedData);
		return decodedString;
	}
	jenova::SerializedData ProcessAndExtractPropertiesFromScript(std::string& scriptSource, const std::string& scriptUID)
	{
		// Property Metadata Serializer
		jenova::json_t propertiesMetadata;

		// Helpers
		auto isCommented = [](const std::string& line)
			{
				std::regex commentRegex(R"(^\s*(//|/\*|\*/))");
				return std::regex_search(line, commentRegex);
			};

		// Function to parse arguments with key-value pairs
		auto parseArguments = [](const std::string& argsString)
			{
				std::vector<std::string> args;
				std::string currentArg;
				int parenDepth = 0;
				bool inQuotes = false;
				for (size_t i = 0; i < argsString.size(); ++i)
				{
					char c = argsString[i];
					if (c == '"') inQuotes = !inQuotes;
					else if (c == '(' && !inQuotes) ++parenDepth;
					else if (c == ')' && !inQuotes && parenDepth > 0) --parenDepth;
					if (c == ',' && parenDepth == 0 && !inQuotes)
					{
						// Trim whitespace around currentArg
						currentArg.erase(currentArg.find_last_not_of(" \t\n\r\f\v") + 1);
						currentArg.erase(0, currentArg.find_first_not_of(" \t\n\r\f\v"));
						args.push_back(currentArg);
						currentArg.clear();
					}
					else
					{
						currentArg += c;
					}
				}
				if (!currentArg.empty())
				{
					currentArg.erase(currentArg.find_last_not_of(" \t\n\r\f\v") + 1);
					currentArg.erase(0, currentArg.find_first_not_of(" \t\n\r\f\v"));
					args.push_back(currentArg);
				}
				return args;
			};

		// Helper to parse key-value pairs
		auto parseKeyValuePairs = [](const std::vector<std::string>& args, size_t startIndex)
			{
				std::unordered_map<std::string, std::string> keyValuePairs;
				std::regex kvRegex(R"(\s*([\w]+)\s*:\s*(.*))");

				for (size_t i = startIndex; i < args.size(); ++i)
				{
					std::smatch match;
					if (std::regex_match(args[i], match, kvRegex))
					{
						std::string key = match[1].str();
						std::string value = match[2].str();
						// Remove surrounding quotes if present
						if (!value.empty() && value.front() == '"' && value.back() == '"')
						{
							value = value.substr(1, value.size() - 2);
						}
						keyValuePairs[key] = value;
					}
				}
				return keyValuePairs;
			};

		// Split scriptSource into lines for processing
		std::istringstream scriptStream(scriptSource);
		std::string line, paramHandlers;
		int lineNumber = 0;
		int paramCount = 0;

		// Flags
		bool isHeaderCommentAdded = false;

		// Extract properties and replace lines
		while (std::getline(scriptStream, line))
		{
			lineNumber++;

			// Skip if line is commented
			if (isCommented(line)) continue;

			// Detect JENOVA_PROPERTY line
			std::regex propertyRegex(R"(JENOVA_PROPERTY\s*\((.*)\))");
			std::smatch match;
			if (std::regex_search(line, match, propertyRegex))
			{
				paramCount++;

				// Parse arguments
				std::vector<std::string> args = parseArguments(match[1].str());
				if (args.size() >= 3)
				{
					// Set Property Data
					jenova::json_t propertyMetadata;
					propertyMetadata["PropertyName"] = args[1];
					propertyMetadata["PropertyType"] = args[0];
					propertyMetadata["PropertyDefault"] = args[2];

					// Parse additional key-value parameters and add them to the metadata
					auto extraParams = parseKeyValuePairs(args, 3);
					for (const auto& kv : extraParams)
					{
						if (kv.first == "Group")
						{
							propertyMetadata["PropertyGroup"] = kv.second;
							continue;
						}
						if (kv.first == "Hint")
						{
							propertyMetadata["PropertyHint"] = kv.second;
							continue;
						}
						if (kv.first == "HintString")
						{
							propertyMetadata["PropertyHintString"] = kv.second;
							continue;
						}
						if (kv.first == "ClassName")
						{
							propertyMetadata["PropertyClassName"] = kv.second;
							continue;
						}
						if (kv.first == "Usage")
						{
							propertyMetadata["PropertyUsage"] = kv.second;
							continue;
						}
						propertyMetadata[kv.first] = kv.second;
					}

					propertiesMetadata.push_back(propertyMetadata);
				}

				// Add Header Comment
				if (!isHeaderCommentAdded)
				{
					paramHandlers += "// Script Properties Handlers\n";
					isHeaderCommentAdded = true;
				}

				// Generate Properties & Handlers
				size_t lineStartPos = scriptSource.find(line);
				if (lineStartPos != std::string::npos)
				{
					scriptSource.replace(lineStartPos, line.length(), jenova::Format("%s* __prop_%s = nullptr;", args[0].c_str(), args[1].c_str()));
					paramHandlers += jenova::Format("#define %s (*__prop_%s)\n", args[1].c_str(), args[1].c_str());
				}
			}
		}

		// Add Handlers to Source
		if (paramCount != 0)
		{
			paramHandlers += "\n";
			scriptSource.insert(0, paramHandlers);
		}

		// Return Metadata
		return propertiesMetadata.dump();
	}
	
	// Deployer (Called to Perform Tasks)
	namespace Deployer
	{
		// Windows Implementation
		#if defined(TARGET_PLATFORM_WINDOWS) && !defined(JENOVA_STATIC_BUILD)
			JENOVA_API void CALLBACK Deploy(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
			{
				// Parse Arguments And Split to Argument Array
				jenova::ArgumentsArray deployerArguments = jenova::ProcessDeployerArguments(lpszCmdLine);
				deployerArguments.insert(deployerArguments.begin(), "jenova.exe");

				try
				{
					// Create Argument Parser
					argparse::ArgumentParser program("Jenova Deployer");

					// Add Arguments to Parser
					program.add_argument("--command").required().help("Deployment Command");
					program.add_argument("--in").help("Input File Path");
					program.add_argument("--out").help("Output File Path");
					program.add_argument("--cache").help("Jenova Cache Directory");
					program.add_argument("--compiler").help("Jenova Compiler Model");
					program.add_argument("--identity").help("Script Unique Identity");
					program.add_argument("--configuration").help("Jenova Build Configuration Data");
					program.add_argument("--watchdog-invoker").help("Build System Watchdog Invoker Name");

					// Parse Arguments
					program.parse_args(deployerArguments);

					// Get Parsed Values
					std::string command = program.get<std::string>("--command");

					// Build System Process Commands
					if (command == "prepare")
					{
						// Get Input/Output Values & Files
						std::string compilerModel = program.get<std::string>("--compiler");

						// Initialize Build Tools
						if (compilerModel == "msvc")
						{
							jenova_log("[Jenova Deployer] Jenova %s (%s) Visual Studio Build Tools Initialized.", APP_VERSION, APP_VERSION_POSTFIX);
							jenova_log("[Jenova Deployer] Starting Build...");
							jenova::ExitWithCode(EXIT_SUCCESS);
						}
					}
					if (command == "preprocess")
					{
						// Get Input/Output Values & Files
						std::string inputFile = std::filesystem::absolute(program.get<std::string>("--in")).string();
						std::string cacheDirectory = std::filesystem::absolute(program.get<std::string>("--cache")).string();
						std::string compilerModel = program.get<std::string>("--compiler");
						std::string sourceIdentity = program.get<std::string>("--identity");
						std::string configuration = program.get<std::string>("--configuration");

						// Create Configuration
						jenova::json_t jenovaConfiguration;

						// Read And Parse Configuration
						try
						{
							jenovaConfiguration = jenova::json_t::parse(jenova::CreateStdStringFromCompressedBase64(configuration));
							if (jenovaConfiguration.empty()) throw std::runtime_error("Invalid Jenova Configuration Data.");
						}
						catch (const std::exception&)
						{
							jenova_log("[Jenova Deployer] Error : Failed to Read or Parse Jenova Configuration Data.");
							jenova::ExitWithCode(EXIT_FAILURE);
						}

						// Preprocess C++ Source
						{
							std::string scriptSourceCode = jenova::ReadStdStringFromFile(inputFile);
							if (scriptSourceCode.empty())
							{
								jenova_log("[Jenova Deployer] Error : Preprocessing Failed, Invalid Input Source.");
								jenova::ExitWithCode(EXIT_FAILURE);
							}

							// Remove Encoding From File Content
							RemoveFileEncodingInStdString(scriptSourceCode);

							// Line Directive
							std::string referenceSourceFile = inputFile;
							jenova::ReplaceAllMatchesWithString(referenceSourceFile, "\\", "\\\\");
							scriptSourceCode = scriptSourceCode.insert(0, jenova::Format("#line 1 \"%s\"\n", referenceSourceFile.c_str()));

							// Process And Extract Properties
							jenova::SerializedData propertiesMetadata = jenova::ProcessAndExtractPropertiesFromScript(scriptSourceCode, sourceIdentity);
							if (!propertiesMetadata.empty() && propertiesMetadata != "null")
							{
								std::string propFile = cacheDirectory + std::filesystem::path(inputFile).stem().string() + "_" + sourceIdentity + ".props";
								if (!jenova::WriteStdStringToFile(propFile, propertiesMetadata))
								{
									jenova_log("[Jenova Deployer] Error : Failed to Write Property Metadata On Disk.");
									jenova::ExitWithCode(EXIT_FAILURE);
								}
							}

							// Preprocessor Definitions [Header]
							std::string preprocessorDefinitions = "// Jenova Preprocessor Definitions\n";

							// Preprocessor Definitions [Version]
							preprocessorDefinitions += jenova::Format("#define JENOVA_VERSION \"%d.%d.%d.%d\"\n",
								jenova::GlobalSettings::JenovaBuildVersion[0], jenova::GlobalSettings::JenovaBuildVersion[1],
								jenova::GlobalSettings::JenovaBuildVersion[2], jenova::GlobalSettings::JenovaBuildVersion[3]);

							// Preprocessor Definitions [Compiler]
							preprocessorDefinitions += "#define JENOVA_COMPILER \"Microsoft Visual C++ Compiler\"\n";
							preprocessorDefinitions += "#define MSVC_COMPILER\n";

							// Preprocessor Definitions [User]
							auto userPreprocessorDefinitions = jenova::SplitStdStringToArguments(jenovaConfiguration["PreprocessorDefinitions"].get<std::string>(), ';');
							for (const auto& definition : userPreprocessorDefinitions) if (!definition.empty()) preprocessorDefinitions += "#define " + definition + "\n";

							// Add Final Preprocessor Definitions
							scriptSourceCode = scriptSourceCode.insert(0, preprocessorDefinitions + "\n");

							// Replecements
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, jenova::GlobalSettings::ScriptToolIdentifier, "#define TOOL_SCRIPT");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, jenova::GlobalSettings::ScriptBlockBeginIdentifier, "namespace JNV_" + sourceIdentity + "{");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, jenova::GlobalSettings::ScriptBlockEndIdentifier, "}; using namespace JNV_" + sourceIdentity + ";");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnReady", " _ready");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnAwake", " _enter_tree");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnDestroy", " _exit_tree");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnProcess", " _process");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnPhysicsProcess", " _physics_process");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnInput", " _input");
							jenova::ReplaceAllMatchesWithString(scriptSourceCode, " OnUserInterfaceInput", " _gui_input");

							// Write Preprocessed Source
							std::string outputPath = cacheDirectory + std::filesystem::path(inputFile).stem().string() + "_" + sourceIdentity + ".cpp";
							if (!jenova::WriteStdStringToFile(outputPath, scriptSourceCode))
							{
								jenova_log("[Jenova Deployer] Error : Failed to Write Preprocessed Source On Disk.");
								jenova::ExitWithCode(EXIT_FAILURE);
							}

							// Apply Reference File Encoding
							if (jenova::GlobalSettings::RespectSourceFilesEncoding)
							{
								if (!jenova::ApplyFileEncodingFromReferenceFile(inputFile, outputPath))
								{
									jenova_log("[Jenova Deployer] Warning : Failed to Apply Encoding to Source File.");
								}
							}
						}

						// All Good
						jenova_log("[Jenova Deployer] C++ Source '%s' Preprocessed.", std::filesystem::path(inputFile).filename().string().c_str());
						jenova::ExitWithCode(EXIT_SUCCESS);
					}
					if (command == "create-internal-scripts")
					{
						// Get Input/Output Values & Files
						std::string cacheDirectory = std::filesystem::absolute(program.get<std::string>("--cache")).string();

						// Create Internal Source
						if (!jenova::CreateFileFromInternalSource(cacheDirectory + "JenovaModuleLoader.cpp", std::string(JENOVA_RESOURCE(JenovaModuleInitializerCPP))))
						{
							jenova_log("[Jenova Deployer] Error : %s", "Unable to Create Internal Script 'JenovaModuleLoader'");
							jenova::ExitWithCode(EXIT_FAILURE);
						};

						// All Good
						jenova_log("[Jenova Deployer] Internal Scripts Successfully Generated.");
						jenova::ExitWithCode(EXIT_SUCCESS);
					}
					if (command == "generate-watchdog")
					{
						// Get Input/Output Values & Files
						std::string cacheDirectory = std::filesystem::absolute(program.get<std::string>("--cache")).string();
						std::string configuration = program.get<std::string>("--configuration");
						std::string watchdogInvoker = program.get<std::string>("--watchdog-invoker");

						// Create Known Watchdog
						if (watchdogInvoker == "vs")
						{
							// Generate Watchdog Path
							std::string visualStudioWatchdogFile = cacheDirectory + jenova::GlobalSettings::VisualStudioWatchdogFile;

							// If Watchdog Exists Remove It
							if (std::filesystem::exists(visualStudioWatchdogFile))
							{
								if (!std::filesystem::remove(visualStudioWatchdogFile))
								{
									jenova_log("[Jenova Deployer] Error : Could Not Remove Existing Watchdog Cache.");
									jenova::ExitWithCode(EXIT_FAILURE);
								}
							}

							// Decode Jenova Configuration
							std::string jenovaConfiguration = jenova::CreateStdStringFromCompressedBase64(configuration);

							// Create New Watchdog
							if (!jenova::WriteStdStringToFile(visualStudioWatchdogFile, jenovaConfiguration))
							{
								jenova_log("[Jenova Deployer] Error : Failed to Generate Visual Studio Watchdog Invoker.");
								jenova::ExitWithCode(EXIT_FAILURE);
							}

							// All Good
							jenova_log("[Jenova Deployer] Visual Studio Watchdog Invoked Successfully.");
							jenova::ExitWithCode(EXIT_SUCCESS);
						}

						// No Valid Command Executed
						jenova_log("[Jenova Deployer] Error : Invalid Watchdog Invoker Detected.");
						jenova::ExitWithCode(EXIT_FAILURE);
					}
					if (command == "finalize")
					{
						// Get Input/Output Values & Files
						std::string compilerModel = program.get<std::string>("--compiler");

						// Finalize Build Tools
						if (compilerModel == "msvc")
						{
							jenova_log("[Jenova Deployer] Build Completed.");
							jenova::ExitWithCode(EXIT_SUCCESS);
						}
					}

					// Internal Process Commands
					if (command == "increment-build-number")
					{
						// Get Input/Output Values & Files
						std::string inputFile = std::filesystem::absolute(program.get<std::string>("--in")).string();

						// Read Source File
						std::string sourceContent = jenova::ReadStdStringFromFile(inputFile);
						if (sourceContent.empty())
						{
							jenova_log("[Jenova Deployer] Error : Source File is Empty!");
							jenova::ExitWithCode(EXIT_FAILURE);
						}

						// Find and Increase Build Number
						std::regex versionBuildRegex(R"((\#define\s+APP_VERSION_BUILD\s+\")(\d+)(\"))");
						std::smatch match;
						if (std::regex_search(sourceContent, match, versionBuildRegex))
						{
							// Increment Build Number
							int buildNumber = std::stoi(match[2]);
							buildNumber++;
							std::string newVersion = match[1].str() + std::to_string(buildNumber) + match[3].str();
							sourceContent = std::regex_replace(sourceContent, versionBuildRegex, newVersion, std::regex_constants::format_first_only);
							if (!jenova::WriteStdStringToFile(inputFile, sourceContent))
							{
								jenova_log("[Jenova Deployer] Error: Can't Write Source File!");
								jenova::ExitWithCode(EXIT_FAILURE);
							};

							// Success
							jenova_log("[Jenova Deployer] Build Number Incremented.");
							jenova::ExitWithCode(EXIT_SUCCESS);
						}
						else
						{
							jenova_log("[Jenova Deployer] Error: APP_VERSION_BUILD Not Found!");
							jenova::ExitWithCode(EXIT_FAILURE);
						}
					}

					// No Valid Command Executed
					jenova_log("[Jenova Deployer] Error : Deployment Command Not Found.");
					jenova::ExitWithCode(EXIT_FAILURE);
				}
				catch (const std::exception& error)
				{
					// Argument Parse Failed
					jenova_log("[Jenova Deployer] Error : %s", error.what());
					jenova::ExitWithCode(EXIT_FAILURE);
				}
			}
		#endif
	}

}