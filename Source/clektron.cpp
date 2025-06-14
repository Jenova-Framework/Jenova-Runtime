
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

// Archive SDK
#define LIBARCHIVE_STATIC
#include "Archive/archive.h"
#include "Archive/archive_entry.h"

// Curl SDK
#define CURL_STATICLIB
#include "Curl/curl.h"

// Tiny C Compiler
#include <TinyCC/libtcc.h>

// Macros
#define UnreferenceParameter(P) (void)(P)
#define AddSymbol(symbol) tcc_add_symbol(tcc, #symbol, reinterpret_cast<const void*>(&ClektronSystem::API_##symbol))

// Stroage Units
std::map<const char*, std::shared_ptr<std::fstream>> fileInstances;

// Clektron Script Interface
extern "C" namespace ClektronSystem
{
    // Types
    typedef void* Instance;
    typedef void* Buffer;
    typedef void* FunctionPtr;
    typedef const char* CString;
    typedef int Size;

    // Enums
    enum AlertType
    {
        Information,
        Warning,
        Error
    };
    enum OperatingSystem
    {
        Unsupported,
        Windows64,
        Linux64
    };

    // Structures
    struct DownloaderMetadata
    {
        void* downloaderInstance;
        void* downloaderCallback;
        double downloadSpeed = 0;
    };

    // Utilities
    const char* InternalFormat(const char* fmt, ...)
    {
        char buffer[2048];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        return jenova::CloneString(buffer);
    }
    bool HasAdministratorAccess()
    {
        // Windows Implementation
        #ifdef TARGET_PLATFORM_WINDOWS
            SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
            PSID administratorsGroup = NULL;
            BOOL isAdmin = FALSE;
            if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup)) return false;
            if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin)) isAdmin = FALSE;
            if (administratorsGroup) FreeSid(administratorsGroup);
            return isAdmin != FALSE;

        #endif

        // Linux Implementation
        #ifdef TARGET_PLATFORM_LINUX

            if (geteuid() == 0) return true;
            struct passwd* userInfo = getpwuid(geteuid());
            if (!userInfo) return false;
            int groupCount = 0;
            getgrouplist(userInfo->pw_name, userInfo->pw_uid, NULL, &groupCount);
            if (groupCount <= 0) return false;
            gid_t* groups = (gid_t*)malloc(groupCount * sizeof(gid_t));
            if (!groups) return false;
            if (getgrouplist(userInfo->pw_name, userInfo->pw_uid, groups, &groupCount) == -1)
            {
                free(groups);
                return false;
            }
            for (int i = 0; i < groupCount; ++i)
            {
                struct group* grp = getgrgid(groups[i]);
                if (grp)
                {
                    if (strcmp(grp->gr_name, "sudo") == 0 ||
                        strcmp(grp->gr_name, "admin") == 0 ||
                        strcmp(grp->gr_name, "wheel") == 0)
                    {
                        free(groups);
                        return true;
                    }
                }
            }
            free(groups);
            return false;

        #endif

        // Platform not supported
        return false;
    }

    // Callbacks
    size_t OnDownloadBufferWrite(void* bufferPtr, size_t bufferSize, size_t newBufferSize, void* outBufferPtr)
    {
        size_t totalSize = bufferSize * newBufferSize;
        std::vector<uint8_t>* outBuffer = static_cast<std::vector<uint8_t>*>(outBufferPtr);
        outBuffer->insert(outBuffer->end(), static_cast<uint8_t*>(bufferPtr), static_cast<uint8_t*>(bufferPtr) + totalSize);
        return totalSize;
    }
    size_t OnDownloadProgress(void* metaDataPtr, size_t downloadTotal, size_t downloadNow, size_t uploadTotal, size_t uploadNow)
    {
        // Unreference Unused Parameters
        UnreferenceParameter(uploadTotal);
        UnreferenceParameter(uploadNow);

        // Update Download Progress
        if (downloadTotal > 0)
        {
            // Get Downloader Metadata
            DownloaderMetadata* metaData = static_cast<DownloaderMetadata*>(metaDataPtr);

            // Calculate Download Progress
            double percentage = (downloadNow * 100.0) / downloadTotal;

            // Calculate Download Speed
            std::string speedCString;
            constexpr size_t sizeBase = 1000;
            curl_easy_getinfo(metaData->downloaderInstance, CURLINFO_SPEED_DOWNLOAD, &(metaData->downloadSpeed));
            if (metaData->downloadSpeed < sizeBase) speedCString += std::to_string(static_cast<int>(metaData->downloadSpeed)) + " B/s";
            else if (metaData->downloadSpeed < sizeBase * sizeBase) speedCString += std::to_string(static_cast<int>(metaData->downloadSpeed / sizeBase)) + " KB/s";
            else speedCString += std::to_string(static_cast<int>(metaData->downloadSpeed / (sizeBase * sizeBase))) + " MB/s";

            // Update Status
            if (metaData->downloaderCallback)
            {
                typedef void(*DownloadCallback_t)(double percentage, Size downloadNow, Size downloadTotal, CString downloadSpeed);
                auto DownloadCallback = (DownloadCallback_t)metaData->downloaderCallback;
                DownloadCallback(percentage, Size(downloadNow), Size(downloadTotal), speedCString.c_str());
            }
        }
        return 0;
    }

    // User Interface API
    int API_Alert(CString message, CString title, AlertType type)
    {
        switch (type)
        {
        case ClektronSystem::Information:
            return jenova::ShowMessageBox(message, title, 0x00000040L);
            break;
        case ClektronSystem::Warning:
            return jenova::ShowMessageBox(message, title, 0x00000030L);
            break;
        case ClektronSystem::Error:
            return jenova::ShowMessageBox(message, title, 0x00000010L);
        default:
            break;
        }
        return jenova::ShowMessageBox(message, title, 0);
    }
    void API_Wait(int msec)
    {
        OS::get_singleton()->delay_msec(msec);
    }
    void API_Refresh()
    {
        jenova::DoApplicationEvents();
    }
    void API_Exit(int exitCode)
    {
        jenova::ExitWithCode(exitCode);
    }

    // File Manager API
    bool API_DoesFileExist(CString filePath)
    {
        return std::filesystem::exists(filePath);
    }
    bool API_DoesDirectoryExist(CString directoryPath)
    {
        return std::filesystem::exists(directoryPath);
    }
    bool API_WriteStringToFile(CString filePath, CString str)
    {
        std::ofstream outFile(filePath, std::ios::out | std::ios::binary);
        if (outFile.is_open())
        {
            outFile.write(str, strlen(str));
            outFile.close();
            return true;
        }
        else
        {
            return false;
        }
    }
    CString API_ReadStringFromFile(CString filePath)
    {
        std::ifstream inFile(filePath);
        if (inFile.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
            inFile.close();
            return jenova::CloneString(content.c_str());
        }
        else
        {
            return nullptr;
        }
    }
    bool API_MakeDirectory(CString directoryPath)
    {
        try
        {
            return std::filesystem::create_directories(directoryPath);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
    Size API_GetFileSize(CString filePath)
    {
        return std::filesystem::file_size(filePath);
    }
    bool API_RenameFile(CString filePath, CString newName)
    {
        try
        {
            // Extract directory from the current file path
            std::filesystem::path oldPath(filePath);
            std::filesystem::path newPath = oldPath.parent_path() / newName;

            // Perform the rename operation
            std::filesystem::rename(oldPath, newPath);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    bool API_RenameDirectory(CString directoryPath, CString newName)
    {
        try
        {
            if (std::filesystem::is_directory(directoryPath))
            {
                std::filesystem::rename(directoryPath, newName);
                return true;
            }
            return false; // Not a directory
        }
        catch (...)
        {
            return false;
        }
    }
    bool API_MoveFile(CString srcPath, CString dstPath)
    {
        try
        {
            std::filesystem::path source(srcPath);
            std::filesystem::path destination(dstPath);

            // If destination is a directory, append the filename from the source
            if (std::filesystem::is_directory(destination))
            {
                destination /= source.filename();
            }

            // Perform the move operation
            std::filesystem::rename(source, destination);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    bool API_MoveDirectory(CString srcPath, CString dstPath)
    {
        try
        {
            std::filesystem::path source(srcPath);
            std::filesystem::path destination(dstPath);

            // If destination is a directory, append the folder name from the source
            if (std::filesystem::is_directory(destination))
            {
                destination /= source.filename();
            }

            // Perform the move operation
            std::filesystem::rename(source, destination);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    bool API_RemoveFile(CString filePath)
    {
        try
        {
            return std::filesystem::remove(filePath);
        }
        catch (...)
        {
            return false;
        }
    }
    bool API_RemoveDirectory(CString directoryPath)
    {
        try
        {
            return std::filesystem::remove_all(directoryPath) > 0;
        }
        catch (...)
        {
            return false;
        }
    }
    Instance API_CreateFile(CString filePath)
    {
        auto file = std::make_shared<std::fstream>(filePath, std::ios::out | std::ios::binary);
        if (file->is_open())
        {
            fileInstances[filePath] = file;
            return static_cast<Instance>(file.get());
        }
        return nullptr;
    }
    Instance API_OpenFile(CString filePath)
    {
        auto file = std::make_shared<std::fstream>(filePath, std::ios::in | std::ios::out | std::ios::binary);
        if (file->is_open())
        {
            fileInstances[filePath] = file;
            return static_cast<Instance>(file.get());
        }
        return nullptr;
    }
    bool API_WriteFile(Instance fileInstance, Buffer bufferPtr, Size bufferSize)
    {
        auto file = static_cast<std::fstream*>(fileInstance);
        if (file && file->is_open())
        {
            file->write(static_cast<const char*>(bufferPtr), bufferSize);
            return file->good();
        }
        return false;
    }
    bool API_ReadFile(Instance fileInstance, Buffer bufferPtr, Size readPosition, Size readSize)
    {
        auto file = static_cast<std::fstream*>(fileInstance);
        if (file && file->is_open())
        {
            file->seekg(readPosition);
            file->read(static_cast<char*>(bufferPtr), readSize);
            return file->good();
        }
        return false;
    }
    bool API_CloseFile(Instance fileInstance)
    {
        auto file = static_cast<std::fstream*>(fileInstance);
        if (file && file->is_open())
        {
            file->close();
            for (auto it = fileInstances.begin(); it != fileInstances.end(); ++it)
            {
                if (it->second.get() == file)
                {
                    fileInstances.erase(it);
                    break;
                }
            }
            return true;
        }
        return false;
    }

    // Memory Manager API
     Buffer API_Allocate(Size size)
    {
        return malloc(size);
    }
    bool API_Free(Buffer bufferPtr)
    {
        if (!bufferPtr) return false;
        free(bufferPtr);
        return true;
    }
    void API_Delete(Instance targetPtr)
    {
        delete targetPtr;
    }
    Buffer API_CopyMemory(Buffer srcBuffer, Buffer dstBuffer, Size copySize)
    {
        return memcpy(dstBuffer, srcBuffer, copySize);
    }
    Buffer API_ZeroMemory(Buffer bufferPtr, Size bufferSize)
    {
        return memset(bufferPtr, 0, bufferSize);
    }
    Size API_CompareMemory(Buffer bufferA, Buffer bufferB, Size compareSize)
    {
        return memcmp(bufferA, bufferB, compareSize);
    }

    // Downloader API
    bool API_DownloadToBuffer(CString fileURL, Buffer* outBuffer, Size* bufferSize, FunctionPtr downloadCallback)
    {
        // Initialize CURL
        CURLcode res;
        curl_global_init(CURL_GLOBAL_ALL);
        CURL* curlHandle = curl_easy_init();
        if (!curlHandle)
        {
            jenova::Error("Clektron Script System","Failed to initialize CURL");
            return false;
        }

        // Create Buffer
        std::vector<uint8_t> buffer;

        // Create Downloader Metadata
        DownloaderMetadata metaData;
        metaData.downloaderInstance = curlHandle;
        metaData.downloaderCallback = downloadCallback;

        // Set Download URL
        curl_easy_setopt(curlHandle, CURLOPT_URL, fileURL);

        // Enable SSL
        curl_easy_setopt(curlHandle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 2L);

        // Set Write Function
        curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, OnDownloadBufferWrite);
        curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &buffer);

        // Configuring Curl
        curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curlHandle, CURLOPT_HTTPPROXYTUNNEL, 1L);
        curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, "Jenova Clektron Downloader/1.0 (Microsoft Windows) (x64) English");
        curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curlHandle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

        // Set Progress Function
        curl_easy_setopt(curlHandle, CURLOPT_XFERINFOFUNCTION, OnDownloadProgress);
        curl_easy_setopt(curlHandle, CURLOPT_XFERINFODATA, &metaData);
        curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0L);

        // Perform Download
        res = curl_easy_perform(curlHandle);

        // Check For Errors During Download
        if (res != CURLE_OK)
        {
            std::vector<uint8_t>().swap(buffer);
            curl_easy_cleanup(curlHandle);
            curlHandle = nullptr;
            jenova::Error("Clektron Script System",curl_easy_strerror(res));
            return false;
        }

        // Allocate Output Buffer
        *bufferSize = Size(buffer.size());
        *outBuffer = new(std::nothrow) uint8_t[*bufferSize];
        if (!*outBuffer)
        {
            std::vector<uint8_t>().swap(buffer);
            curl_easy_cleanup(curlHandle);
            curlHandle = nullptr;
            jenova::Error("Clektron Script System","Failed to allocate memory for download buffer.");
            return false;
        }

        // Copy Data from Temp Buffer into Raw Buffer
        std::memcpy(*outBuffer, buffer.data(), buffer.size());

        // Clean Up CURL
        std::vector<uint8_t>().swap(buffer);
        curl_easy_cleanup(curlHandle);
        curl_global_cleanup();
        curlHandle = nullptr;

        // All Good
        return true;
    }
    void API_FreeDownloadBuffer(Buffer* dataBuffer)
    {
        if (dataBuffer && *dataBuffer)
        {
            delete[] static_cast<uint8_t*>(*dataBuffer);
            *dataBuffer = nullptr;
        }
    }

    // Archive API
    bool API_ExtractArchive(CString archivePath, CString destinationPath, FunctionPtr extractionCallback)
    {
        // Create Destination If Not Exist
        if (!std::filesystem::exists(destinationPath))
        {
            if (!std::filesystem::create_directories(destinationPath))
            {
                jenova::Error("Clektron Script System","Failed to create destination directory.");
                return false;
            };
        }

        // Create Archvie Extractor
        struct archive* a;
        struct archive* ext;
        struct archive_entry* entry;
        int flags;
        int r;
        int entityIndex = 0;
        int totalEntityCount = 0;

        // Helper Function
        auto countEntities = [](const std::string& archivePath) -> int
            {
                struct archive* archive = archive_read_new();
                if (!archive) return -1;
                archive_read_support_format_all(archive);
                archive_read_support_filter_all(archive);
                if (archive_read_open_filename(archive, archivePath.c_str(), 10240) != ARCHIVE_OK)
                {
                    archive_read_free(archive);
                    return -1;
                }
                struct archive_entry* entry;
                int count = 0;
                while (archive_read_next_header(archive, &entry) == ARCHIVE_OK)
                {
                    if (archive_entry_filetype(entry) == AE_IFREG) ++count;
                    archive_read_data_skip(archive);
                }
                archive_read_close(archive);
                archive_read_free(archive);
                return count;
            };
        auto copyData = [&](struct archive* ar, struct archive* aw)
            {
                int r;
                const void* buff;
                size_t size;
                la_int64_t offset;
                for (;;)
                {
                    r = archive_read_data_block(ar, &buff, &size, &offset);
                    if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
                    if (r < ARCHIVE_OK) return (r);
                    r = archive_write_data_block(aw, buff, size, offset);
                    if (r < ARCHIVE_OK) return (r);
                }
            };

        // Configuration
        flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        // Get Extraction Callback
        typedef void(*ExtractionCallback_t)(CString stageName, CString stageData, Size extractedCount, Size totalCount);
        auto ExtractionCallback = (ExtractionCallback_t)extractionCallback;

        // Count Total Files
        totalEntityCount = countEntities(archivePath);

        // Extract Package
        a = archive_read_new();
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);
        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);
        archive_write_disk_set_standard_lookup(ext);
        if ((r = archive_read_open_filename(a, archivePath, 10240))) return false;
        for (;;)
        {
            // Read Next Header
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF) break;
            if (r < ARCHIVE_OK) jenova::Error("Clektron Script System",InternalFormat("Failed to extract archive, Reason [%d] : %s", __LINE__, archive_error_string(a)));
            if (r < ARCHIVE_WARN) return false;

            // Set Output Path
            std::string newFilePath = destinationPath + std::string("/") + std::string(archive_entry_pathname(entry));
            archive_entry_set_pathname(entry, newFilePath.c_str());

            // Extract Entity
            r = archive_write_header(ext, entry);
            if (r < ARCHIVE_OK) jenova::Error("Clektron Script System",InternalFormat("Failed to extract archive, Reason [%d] : %s", __LINE__, archive_error_string(a)));
            else if (archive_entry_size(entry) > 0)
            {
                r = copyData(a, ext);
                if (r < ARCHIVE_OK) jenova::Error("Clektron Script System",InternalFormat("Failed to extract archive, Reason [%d] : %s", __LINE__, archive_error_string(a)));
                if (r < ARCHIVE_WARN) return false;
            }
            r = archive_write_finish_entry(ext);
            if (r < ARCHIVE_OK) jenova::Error("Clektron Script System",InternalFormat("Failed to extract archive, Reason [%d] : %s", __LINE__, archive_error_string(a)));
            if (r < ARCHIVE_WARN) return false;

            // Count Extracted Files
            if (archive_entry_filetype(entry) == AE_IFREG)
            {
                if (ExtractionCallback) ExtractionCallback("Extracted", std::filesystem::path(archive_entry_pathname(entry)).filename().string().c_str(), entityIndex, totalEntityCount);
                entityIndex++;
            }
        }
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);

        // All Good
        return true;
    }

    // Utility API
    void API_Print(CString message)
    {
        printf("[ Electron-C ] :: ");
        printf(message);
        printf("\n");
    }
    void API_PrintError(CString stage, CString message)
    {
        jenova::Error(stage, message);
    }
    void API_PrintWarning(CString stage, CString message)
    {
        jenova::Warning(stage, message);
    }
    CString API_Format(CString fmt, ...)
    {
        char buffer[2048] = { 0 };
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        return jenova::CloneString(buffer);
    }
    int API_System(CString command)
    {
        return system(command);
    }
    CString API_GetFileMD5Hash(CString filePath)
    {
        return jenova::CloneString(AS_C_STRING(jenova::GenerateMD5HashFromFile(godot::String(filePath))));
    }
    OperatingSystem API_GetOperatingSystem()
    {
        return OperatingSystem::Unsupported;
    }
    CString API_GetOperatingSystemName(OperatingSystem os)
    {
        switch (os)
        {
        case OperatingSystem::Windows64:
            return "Windows (x64)";
        case OperatingSystem::Linux64:
            return "Linux (x64)";
        case OperatingSystem::Unsupported:
        default:
            return "Unsupported OS";
        }
    }
    bool API_HasAdministratorAccess()
    {
        return HasAdministratorAccess();
    }
    Instance API_LoadModule(CString modulePath)
    {
        return jenova::LoadModule(modulePath);
    }
    FunctionPtr API_GetModuleFunction(Instance moduleInstance, CString functionName)
    {
        return jenova::GetModuleFunction(moduleInstance, functionName);
    }
    bool API_FreeModule(Instance moduleInstance)
    {
        return jenova::ReleaseModule(moduleInstance);
    }
    CString API_CombineStrings(CString strA, CString strB)
    {
        std::string stringA(strA);
        std::string stringB(strB);
        std::string combinedCString = stringA + stringB;
        return jenova::CloneString(combinedCString.c_str());
    }
    bool API_CompareStrings(CString strA, CString strB)
    {
        std::string stringA(strA);
        std::string stringB(strB);
        return stringA == stringB;
    }
}

// Jenova Clektron Syntax Highlighter
class ClektronHighlighter : public CodeHighlighter
{
    GDCLASS(ClektronHighlighter, CodeHighlighter);

protected:
    static void _bind_methods() {}

public:
    ClektronHighlighter()
    {
        // Define Clektron Highlighter Colors
        Color keyword_color             = Color::html("#86e051");
        Color type_color                = Color::html("#66e051");
        Color boolean_color             = Color::html("#4cb8e6");
        Color enum_color                = Color::html("#8484f5");
        Color enum_value_color          = Color::html("#9dbef5");
        Color preprocessor_color        = Color::html("#fa5293");
        Color string_color              = Color::html("#f53854");
        Color char_color                = Color::html("#f5386e");
        Color comment_color             = Color::html("#96969696");
        Color function_color            = Color::html("#45ff92");
        Color number_color              = Color::html("#e0b83f");
        Color symbol_color              = Color::html("#f75c40");
        Color memeber_variable_color    = Color::html("#4f49c9");

        // Add Clektron Keywords
        const String keywords[] =
        {
            "auto", "break", "case", "const", "continue", "default", "do", "else", "enum", "extern", "for", "goto", "if", 
            "register", "return", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "volatile", "while", "jenova"
        };
        for (const String& _keyword : keywords) add_keyword_color(_keyword, keyword_color);

        // Add Clektron Types
        const String types[] =
        {
            "void", "char", "float", "int", "long", "double", "short", 
            "bool", "Instance", "Buffer", "FunctionPtr", "String", "Size"
        };
        for (const String& _type : types) add_keyword_color(_type, type_color);
        add_keyword_color("true", boolean_color);
        add_keyword_color("false", boolean_color);

        // Add Clektron Enums
        const String enums[] =
        {
            "AlertType", "TaskIcon", "OperatingSystem"
        };
        for (const String& _enum : enums) add_keyword_color(_enum, enum_color);

        // Add Clektron Enum Values
        const String enumValues[] =
        {
            "Alert_Info", "Alert_Warn", "Alert_Error", 
            "TaskIcon_Working", "TaskIcon_Warning", "TaskIcon_Error", "TaskIcon_Done", 
            "OS_Unsupported", "OS_Windows_x64", "OS_Linux_x64"
        };
        for (const String& _enumval : enumValues) add_keyword_color(_enumval, enum_value_color);

        // Add Clektron Regions
        add_color_region("#", "", preprocessor_color, true);
        add_color_region("\"", "\"", string_color);
        add_color_region("'", "'", char_color);
        add_color_region("//", "", comment_color, true);
        add_color_region("/*", "*/", comment_color);

        // Add Clektron Function, Number and Symbol etc.
        set_function_color(function_color);
        set_number_color(number_color);
        set_symbol_color(symbol_color);
        set_member_variable_color(memeber_variable_color);
    }
};

// Singleton Instance
Clektron* singleton = nullptr;

// Initializer/Deinitializer
void Clektron::init()
{
    // Register Class
    ClassDB::register_internal_class<Clektron>();

    // Register Syntax Highlighter
    ClassDB::register_class<ClektronHighlighter>();

    // Initialize Singleton
    singleton = memnew(Clektron);

    // Verbose
    jenova::Output("Jenova Asset Monitor Initialized.");
}
void Clektron::deinit()
{
    // Release Singleton
    if (singleton) memdelete(singleton);
}

// Bindings
void Clektron::_bind_methods()
{
    ClassDB::bind_static_method("Clektron", D_METHOD("GetInstance"), &Clektron::get_singleton);
    ClassDB::bind_method(D_METHOD("ExecuteScript"), static_cast<bool (Clektron::*)(const godot::String&, bool)>(&Clektron::ExecuteScript));
    ClassDB::bind_method(D_METHOD("ExecuteScriptFromFile"), static_cast<bool (Clektron::*)(const godot::String&, bool)>(&Clektron::ExecuteScriptFromFile));
}

// Singleton Handling
Clektron* Clektron::get_singleton()
{
    return singleton;
}

// Jenova Clektron Script Engine Implementation
Clektron::Clektron()
{

}
Clektron::~Clektron()
{

}
bool Clektron::ExecuteScript(const std::string& ctronScriptContent, bool noEntrypoint)
{
    // Validate Script Content
    if (ctronScriptContent.empty())
    {
        jenova::Error("Clektron Script Engine", "Invalid Script Content.");
        return false;
    }

    // Initialize TCC Compiler
    TCCState* tcc = tcc_new();
    if (!tcc)
    {
        jenova::Error("Clektron Script Engine", "Failed to Create Clektron Compiler.");
        return false;
    }

    // Create Error Handler
    auto tcc_error_handler = [](void* opaque, const char* msg) -> void
    {
        std::string errorMessage(msg);
        jenova::ReplaceAllMatchesWithString(errorMessage, "<string>:", "At Line ");
        jenova::Error("Clektron Script Engine", ClektronSystem::InternalFormat("Clektron Compiler Encountered Error :\n%s", errorMessage.c_str()));
    };
    tcc_set_error_func(tcc, this, tcc_error_handler);

    // Configure TCC Compiler
    tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
    tcc_set_options(tcc, "-nostdlib");

    // Register User Interface API Symbols
    AddSymbol(Alert);
    AddSymbol(Wait);
    AddSymbol(Refresh);
    AddSymbol(Exit);

    // Register File Manager API Symbols
    AddSymbol(DoesFileExist);
    AddSymbol(DoesDirectoryExist);
    AddSymbol(WriteStringToFile);
    AddSymbol(ReadStringFromFile);
    AddSymbol(MakeDirectory);
    AddSymbol(GetFileSize);
    AddSymbol(RenameFile);
    AddSymbol(RenameDirectory);
    AddSymbol(MoveFile);
    AddSymbol(MoveDirectory);
    AddSymbol(RemoveFile);
    AddSymbol(RemoveDirectory);
    AddSymbol(CreateFile);
    AddSymbol(OpenFile);
    AddSymbol(WriteFile);
    AddSymbol(ReadFile);
    AddSymbol(CloseFile);

    // Register Memory Manager API Symbols
    AddSymbol(Allocate);
    AddSymbol(Free);
    AddSymbol(Delete);
    AddSymbol(CopyMemory);
    AddSymbol(ZeroMemory);
    AddSymbol(CompareMemory);

    // Register Downloader API Symbols
    AddSymbol(DownloadToBuffer);
    AddSymbol(FreeDownloadBuffer);

    // Register Archive API Symbols
    AddSymbol(ExtractArchive);

    // Register Utility API Symbols
    AddSymbol(Print);
    AddSymbol(PrintError);
    AddSymbol(PrintWarning);
    AddSymbol(Format);
    AddSymbol(System);
    AddSymbol(GetFileMD5Hash);
    AddSymbol(GetOperatingSystem);
    AddSymbol(GetOperatingSystemName);
    AddSymbol(HasAdministratorAccess);
    AddSymbol(LoadModule);
    AddSymbol(GetModuleFunction);
    AddSymbol(FreeModule);
    AddSymbol(CombineStrings);
    AddSymbol(CompareStrings);

    // Generate Final Script Code
    std::string ctronFinalScript = 
    R"(
    /* Jenova Clektron Engine Developed By Hamid.Memar */

    // Macros
    #define bool                int
    #define false               0
    #define true                1
    #define Instance            void*
    #define Buffer              unsigned char*
    #define FunctionPtr         void*
    #define String              const char*
    #define Size                int

    // Enums
    typedef enum { Alert_Info, Alert_Warn, Alert_Error } AlertType;
    typedef enum { TaskIcon_Working, TaskIcon_Warning, TaskIcon_Error, TaskIcon_Done } TaskIcon;
    typedef enum { OS_Unsupported, OS_Windows_x64, OS_Linux_x64 } OperatingSystem;

    // User Interface API Symbols
    int Alert(String message, String title, AlertType type);
    void Wait(int msec);
    void Refresh();
    void Exit(int exitCode);

    // File Manager API Symbols
    bool DoesFileExist(String filePath);
    bool DoesDirectoryExist(String directoryPath);
    bool WriteStringToFile(String filePath, String str);
    String ReadStringFromFile(String filePath);
    bool MakeDirectory(String directoryPath);
    Size GetFileSize(String filePath);
    bool RenameFile(String filePath, String newName);
    bool RenameDirectory(String directoryPath, String newName);
    bool MoveFile(String srcPath, String dstPath);
    bool MoveDirectory(String srcPath, String dstPath);
    bool RemoveFile(String filePath);
    bool RemoveDirectory(String directoryPath);
    Instance CreateFile(String filePath);
    Instance OpenFile(String filePath);
    bool WriteFile(Instance fileInstance, Buffer bufferPtr, Size bufferSize);
    bool ReadFile(Instance fileInstance, Buffer bufferPtr, Size readPosition, Size readSize);
    bool CloseFile(Instance fileInstance);

    // Memory Manager API Symbols
    Buffer Allocate(Size size);
    bool Free(Buffer bufferPtr);
    void Delete(Instance targetPtr);
    Buffer CopyMemory(Buffer srcBuffer, Buffer dstBuffer, Size copySize);
    Buffer ZeroMemory(Buffer bufferPtr, Size bufferSize);
    Size CompareMemory(Buffer bufferA, Buffer bufferB, Size compareSize);

    // Downloader API Symbols
    bool DownloadToBuffer(String fileURL, Buffer* outBuffer, Size* bufferSize, FunctionPtr downloadCallback);
    void FreeDownloadBuffer(Buffer* dataBuffer);

    // Archive API Symbols
    bool ExtractArchive(String archivePath, String destinationPath, FunctionPtr extractionCallback);

    // Utility API Symbols
    void Print(String message);
    void PrintError(String stage, String message);
    void PrintWarning(String stage, String message);
    String Format(String fmt, ...);
    int System(String command);
    String GetFileMD5Hash(String filePath);
    OperatingSystem GetOperatingSystem();
    String GetOperatingSystemName(OperatingSystem os);
    bool HasAdministratorAccess();
    Instance LoadModule(String modulePath);
    FunctionPtr GetModuleFunction(Instance moduleInstance, String functionName);
    bool FreeModule(Instance moduleInstance);
    String CombineStrings(String strA, String strB);
    bool CompareStrings(String strA, String strB);

    // Reset Line Number
    #line 1
    )";

    // Add Entrypoint Start Automatically
    if (noEntrypoint)
    {
        ctronFinalScript += "bool ClektronMain() {\n";
        ctronFinalScript += "#line 1\n";
    }

    // Add Electron-C Script Content
    ctronFinalScript += ctronScriptContent;

    // Add Entrypoint End Automatically
    if (noEntrypoint)
    {
        ctronFinalScript += "\n return true;\n";
        ctronFinalScript += "}";
    }

    // Compile Clektron Code
    if (tcc_compile_string(tcc, ctronFinalScript.c_str()) == -1)
    {
        jenova::Error("Clektron Script Engine", "Failed to Compile Clektron Script.");
        tcc_delete(tcc);
        return false;
    }

    // Prepare For Execution
    if (tcc_relocate(tcc, TCC_RELOCATE_AUTO) < 0) {
        jenova::Error("Clektron Script Engine", "Failed to Resolve Clektron Script.");
        tcc_delete(tcc);
        return false;
    }

    // Get Compiled Caller Function
    using ClektronMainFunction = bool*(*)();
    ClektronMainFunction ctronEntrypoint = (ClektronMainFunction)tcc_get_symbol(tcc, "ClektronMain");
    if (!ctronEntrypoint)
    {
        jenova::Error("Clektron Script Engine", "Failed to Execute Clektron Script.");
        tcc_delete(tcc);
        return false;
    }

    // Execute Caller
    bool result = ctronEntrypoint();

    // Verbose If Failed
    if (!result) jenova::Warning("Clektron Script Engine", "Script Entrypoint Returned False.");

    // Clean up TCC
    tcc_delete(tcc);

    // Return Result
    return result;
}
bool Clektron::ExecuteScript(const godot::String& ctronScriptContent, bool noEntrypoint)
{
    return this->ExecuteScript(AS_STD_STRING(ctronScriptContent), noEntrypoint);
}
bool Clektron::ExecuteScriptFromFile(const std::string& ctronScriptFilePath, bool noEntrypoint)
{
    return this->ExecuteScript(jenova::ReadStdStringFromFile(ctronScriptFilePath), noEntrypoint);
}
bool Clektron::ExecuteScriptFromFile(const godot::String& ctronScriptFilePath, bool noEntrypoint)
{
    return this->ExecuteScriptFromFile(AS_STD_STRING(ctronScriptFilePath), noEntrypoint);
}
