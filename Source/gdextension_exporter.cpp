
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

// Jenova GDExtension Exporter Initializer
GDExtensionExporter::GDExtensionExporter(GDExtensionTarget target, GDExtensionType type) : extensionTarget(target), extensionType(type) {}

// Jenova GDExtension Exporter Implementation
void GDExtensionExporter::Initialize()
{
	// Open File Dialog
	this->PickExportOutputFile();
}
bool GDExtensionExporter::Export()
{
	// Build Project
	if (!jenova::QueueProjectBuild(false))
	{
		jenova::Error("Jenova Module Exporter", "Project Build failed, Correct errors and try again.");
		return false;
	}

	// Windows x64 GDExtension Binary Exporter
	if (extensionTarget == GDExtensionTarget::Windows64 && extensionType == GDExtensionType::Binary)
	{
		// Windows Implementation
		#ifdef TARGET_PLATFORM_WINDOWS

			// Find Built Module
			std::string jenovaCachePath = AS_STD_STRING(jenova::GetJenovaCacheDirectory());
			std::string jenovaModulePath = "Not-Found";
			if (std::filesystem::exists(jenovaCachePath + "Jenova.Module.jnv")) jenovaModulePath = jenovaCachePath + "Jenova.Module.jnv";
			if (std::filesystem::exists(jenovaCachePath + "Jenova.Module.dll")) jenovaModulePath = jenovaCachePath + "Jenova.Module.dll";
			if (jenovaModulePath == "Not-Found")
			{
				jenova::Error("Jenova Module Exporter", "Cannot find compiled Jenova Module.");
				return false;
			}

			// Read Built Module
			jenova::MemoryBuffer jenovaModuleBuffer = jenova::ReadMemoryBufferFromFile(jenovaModulePath);
			if (jenovaModuleBuffer.size() == 0)
			{
				jenova::Error("Jenova Module Exporter", "Cannot access compiled Jenova Module.");
				return false;
			}

			// Compress Built Module 
			jenova::MemoryBuffer compressedData = jenova::CompressBuffer(jenovaModuleBuffer.data(), jenovaModuleBuffer.size());

			// Create Section Data
			struct JenovaModuleEntity { size_t originalModuleSize, compressedModuleSize; } jenovaModuleEntity { jenovaModuleBuffer.size(), compressedData.size() };
			jenova::MemoryBuffer sectionData(sizeof(JenovaModuleEntity) + compressedData.size());
			memcpy(sectionData.data(), &jenovaModuleEntity, sizeof(JenovaModuleEntity));
			memcpy(&sectionData.data()[sizeof(JenovaModuleEntity)], compressedData.data(), compressedData.size());

			// Create Win64 Extension PE Clone
			const jenova::MemoryBuffer& extensionHostWin64Base = RESOURCE_BUFFER(GDEXTENSION_HOST_WIN64);
			jenova::MemoryBuffer extensionHostWin64(extensionHostWin64Base.size());
			memcpy(extensionHostWin64.data(), extensionHostWin64Base.data(), extensionHostWin64.size());

			// x64 PE Editing Functions
			auto PE64_Align = [&](DWORD size, DWORD align, DWORD addr) -> DWORD
			{
				if (!(size % align)) return addr + size;
				return addr + (size / align + 1) * align;
			};
			auto PE64_AddSection = [&](jenova::MemoryBuffer& inputModule, const jenova::MemoryBuffer& sectionBuffer, const std::string& sectionName) -> void
			{
				// Prepare Data Information
				BYTE* dataPtr = (BYTE*)inputModule.data();
				DWORD fileSize = inputModule.size();

				// Get DOS Header
				PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)inputModule.data();
				if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return;

				// Get Secondary Headers (NT Headers)
				PIMAGE_FILE_HEADER	   FH = (PIMAGE_FILE_HEADER)(dataPtr + dosHeader->e_lfanew + sizeof(DWORD));
				PIMAGE_OPTIONAL_HEADER OH = (PIMAGE_OPTIONAL_HEADER)(dataPtr + dosHeader->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
				PIMAGE_SECTION_HEADER  SH = (PIMAGE_SECTION_HEADER)(dataPtr + dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

				// Allocate and Initialize New Section
				ZeroMemory(&SH[FH->NumberOfSections], sizeof(IMAGE_SECTION_HEADER));
				CopyMemory(&SH[FH->NumberOfSections].Name, sectionName.c_str(), 8);
				SH[FH->NumberOfSections].Misc.VirtualSize = PE64_Align(sectionBuffer.size(), OH->SectionAlignment, 0);

				// Align New Segment
				if (!(SH[FH->NumberOfSections - 1].Misc.VirtualSize % OH->SectionAlignment)) 
					SH[FH->NumberOfSections].VirtualAddress = SH[FH->NumberOfSections - 1].Misc.VirtualSize + SH[FH->NumberOfSections - 1].VirtualAddress;
				else 
					SH[FH->NumberOfSections].VirtualAddress = SH[FH->NumberOfSections - 1].VirtualAddress + ((SH[FH->NumberOfSections - 1].Misc.VirtualSize / OH->SectionAlignment) + 1) * OH->SectionAlignment;

				// Update New Section Info
				SH[FH->NumberOfSections].SizeOfRawData = PE64_Align(sectionBuffer.size(), OH->FileAlignment, 0);
				SH[FH->NumberOfSections].PointerToRawData = PE64_Align(SH[FH->NumberOfSections - 1].SizeOfRawData, OH->FileAlignment, SH[FH->NumberOfSections - 1].PointerToRawData);
				SH[FH->NumberOfSections].Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
			
				// Update New Section Data
				inputModule.resize(SH[FH->NumberOfSections].PointerToRawData + SH[FH->NumberOfSections].SizeOfRawData);
				memcpy(inputModule.data() + SH[FH->NumberOfSections].PointerToRawData, sectionBuffer.data(), sectionBuffer.size());

				// Update Size of Image and Number of Sections
				OH->SizeOfImage = SH[FH->NumberOfSections].VirtualAddress + SH[FH->NumberOfSections].Misc.VirtualSize;
				FH->NumberOfSections += 1;

				// Restore Original Data
				memcpy(inputModule.data(), dataPtr, fileSize);
			};

			// Build GDExtension
			PE64_AddSection(extensionHostWin64, sectionData, ".jnvext");

			// Write GDExtension On Disk
			if (!jenova::WriteMemoryBufferToFile(this->exportOutputFile, extensionHostWin64))
			{
				jenova::Error("Jenova Module Exporter", "Cannot write generated GDExtension on disk.");
				return false;
			};

			// Clean Up
			jenova::ReleaseMemoryBuffer(sectionData);
			jenova::ReleaseMemoryBuffer(compressedData);
			jenova::ReleaseMemoryBuffer(jenovaModuleBuffer);
			jenova::ReleaseMemoryBuffer(extensionHostWin64);

			// Create .gdextension File
			std::string win64GDXTemplate;
			win64GDXTemplate += "[configuration]\n";
			win64GDXTemplate += "entry_symbol = \"InitializeExtension\"\n";
			win64GDXTemplate += "compatibility_minimum = \"4.1\"\n";
			win64GDXTemplate += "reloadable = false\n\n";
			win64GDXTemplate += "[libraries]\n";
			win64GDXTemplate += "windows.debug.x86_64 = \"res://" + std::filesystem::path(this->exportOutputFile).filename().string() + "\"\n";
			win64GDXTemplate += "windows.release.x86_64 = \"res://" + std::filesystem::path(this->exportOutputFile).filename().string() + "\"\n";

			// Write GDExtension On Disk
			if (!jenova::WriteStdStringToFile(jenova::ReplaceAllMatchesWithStringAndReturn(this->exportOutputFile, ".dll", ".gdextension"), win64GDXTemplate))
			{
				jenova::Warning("Jenova Module Exporter", "Failed to generate '.gdextension' file.");
			};

			// All Good
			return true;

		#endif
	}

	// Failed
	return false;
}
void GDExtensionExporter::PickExportOutputFile()
{
	// Get Scale Factor
	double scaleFactor = EditorInterface::get_singleton()->get_editor_scale();

	// Create File Dialog Instance
	FileDialog* file_dialog = memnew(FileDialog);
	file_dialog->set_file_mode(FileDialog::FILE_MODE_SAVE_FILE);
	file_dialog->set_access(FileDialog::ACCESS_FILESYSTEM);
	file_dialog->set_title("Select a Path for GDExtension");
	file_dialog->set_size(Vector2i(SCALED(950), SCALED(580)));

	// Add Filters
	PackedStringArray filters;
	if (extensionTarget == GDExtensionTarget::Windows64 && extensionType == GDExtensionType::Binary) filters.push_back("*.dll ; GDExtension (Windows x64)");
	file_dialog->set_filters(filters);

	// Define Internal UI Callback
	class FileDialogEvent : public Object
	{
	private:
		FileDialog* fileDialogInstance;
		GDExtensionExporter* exporterInstance;

	public:
		// Constructor
		FileDialogEvent(FileDialog* _fileDialogInstance, GDExtensionExporter* _exporterInstance) : fileDialogInstance(_fileDialogInstance), exporterInstance(_exporterInstance) {}

		// Events
		void OnExportOutputFileSelected(const String& path)
		{
			// Set Export Output File
			exporterInstance->exportOutputFile = AS_STD_STRING(ProjectSettings::get_singleton()->globalize_path(path));

			// Perform Export
			if (exporterInstance->Export())
			{
				jenova::OutputColored("#03fc6b", "Module Exported to GDExtension Successfully!");
			}
			else
			{
				jenova::Error("Jenova Module Exporter", "GDExtension Exporter Encountered an Internal Error.");
			}

			// Clean Up
			ReleaseEverything();
		}
		void OnDialogClosed()
		{
			// Verbose
			jenova::Output("Module Export Operation Aborted by User.");

			// Clean Up
			ReleaseEverything();
		}

		// Releaser
		void ReleaseEverything()
		{
			fileDialogInstance->queue_free();
			memdelete(this);
			delete exporterInstance;
		}
	};

	// Add File Dialog to Tree
	dynamic_cast<godot::SceneTree*>(godot::Engine::get_singleton()->get_main_loop())->get_root()->add_child(file_dialog);

	// Connect Signals
	auto file_dialog_event = memnew(FileDialogEvent(file_dialog, this));
	file_dialog->connect("file_selected", callable_mp(file_dialog_event, &FileDialogEvent::OnExportOutputFileSelected));
	file_dialog->connect("canceled", callable_mp(file_dialog_event, &FileDialogEvent::OnDialogClosed));

	// Display File Picker
	file_dialog->popup_centered();
}