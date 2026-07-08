# Jenova Microsoft Compiler Bridge

## Overview

**Jenova MSVC Bridge (JMCB)** is a lightweight bridge that redirects compiler calls to a legitimate Microsoft Visual C++ (MSVC) installation.   
It replaces the need to redistribute MSVC binaries while maintaining full compatibility with your build system.

- **No MSVC Binaries Redistributed** - Fully compliant with [Microsoft Licensing of Build Tools](https://visualstudio.microsoft.com/license-terms/mt644918/).
- **Lightweight with Minimal Footprint** - Tiny bridge executables with no overhead.
- **Works with any MSVC version** - Eliminates the need to install different versions of the toolchain.

## How It Works

The bridge executables (`cl.exe` and `link.exe`) read `config.json` to locate your MSVC installation, then forward all commands to the real compiler/linker with the correct include and library paths. Simple as that!

## Prerequisites

- **[Microsoft Visual C++ Build Tools](https://aka.ms/vs/stable/vs_BuildTools.exe)** or **Microsoft Visual Studio** installed.
- **Windows SDK** (Installed with MSVC)

## Configuration

### 1. Locate Your MSVC Installation

Find your MSVC toolchain path. Typical locations:

| Edition          | Path                                                         |
| ---------------- | ------------------------------------------------------------ |
| **Build Tools**  | `C:\Program Files\Microsoft Visual Studio\20XX\BuildTools\VC\Tools\MSVC\XX.XX.XXXXX` |
| **Community**    | `C:\Program Files\Microsoft Visual Studio\20XX\Community\VC\Tools\MSVC\XX.XX.XXXXX` |
| **Professional** | `C:\Program Files\Microsoft Visual Studio\20XX\Professional\VC\Tools\MSVC\XX.XX.XXXXX` |
| **Enterprise**   | `C:\Program Files\Microsoft Visual Studio\20XX\Enterprise\VC\Tools\MSVC\XX.XX.XXXXX` |

> **Note :** `20XX` is the Visual Studio year (e.g., 2019, 2022, 2026) and `XX.XX.XXXXX` is the toolchain version number (e.g., 14.38.33130)

### 2. Find Your Windows SDK Version

Check your installed SDK versions:
```
C:\Program Files (x86)\Windows Kits\10\Include\
```
Common versions: `10.0.19041.0`, `10.0.22621.0`, `10.0.26100.0`

### 3. Create `config.json`

Place this file in the **same directory** as your bridge executables (`cl.exe`, `link.exe`).

```json
{
    "msvc": {
        "installPath": "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\MSVC\\14.38.33130"
    },
    "windowsSdk": {
        "version": "10.0.19041.0"
    }
}
```

> **Important:** Use **double backslashes** (`\\`) in the path.

### 4. Verify It Works

Run the bridge from command line:
```cmd
cl.exe /?
```

## Limitations
Microsoft Compiler Bridge cannot be used for IntelliSense in **Visual Studio Code** or **Jenova Code IDE** due to missing header files. To resolve this, you must place the MSVC and Windows SDK header files in a folder named `Include` located alongside the `Bin` directory or manually add the required include paths in `c_cpp_properties.json` exported from Jenova Exporter.

Alternatively, you can use the [Jenova MSVC Packer Wizard](https://jenova-framework.github.io/docs/pages/Misc/Auxiliaries#jenova-msvc-packer-wizard) to create your own **Jenova MSVC Compiler Package** from any MSVC installation.


## Troubleshooting

| Error | Cause(s) | Fix |
|-------|----------|-----|
| **Configuration File is Missing** | `config.json` not found | Place `config.json` in the same directory as the bridge executables |
| **Failed to Execute: 0x00000002** | MSVC path is incorrect, MSVC not installed at that path or Path typos (especially `\\` vs `\`) | Correct the MSVC path in `config.json` |
| **Cannot open include file: crtdbg.h** | Wrong Windows SDK version | Update `windowsSdk.version` to match your installed SDK |
| **Cannot open include file: windows.h** | Wrong Windows SDK version | Update `windowsSdk.version` to match your installed SDK |

## Notes

- The bridge **does not redistribute** any proprietary Microsoft binaries or source.
- Users must have a valid MSVC Build Tools installed by official installer.
- The bridge passes all arguments to the real MSVC tools acting as proxy.

## License

This project is part of the **[Jenova Framework](https://github.com/Jenova-Framework)** and is licensed under the **MIT License**.