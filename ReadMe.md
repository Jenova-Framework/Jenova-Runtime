<img width="1200" height="208" alt="Jenova-Framework-Banner" src="https://github.com/user-attachments/assets/79c4bd1d-dfe3-48dc-925d-e8bbddcf28ca" />

# Jenova Runtime (Core)

This repository contains source code of **Jenova Runtime** for Godot Engine and It's a part of **[Projekt J.E.N.O.V.A](https://github.com/Jenova-Framework/J.E.N.O.V.A)**

![Jenova-Screenshot](https://github.com/user-attachments/assets/9db1d96a-cd2c-4733-9465-1dc434ec1543)

<div align="center">
  <span>
    <img src="https://github.com/Jenova-Framework/Jenova-Runtime/actions/workflows/runtime-windows-x64-build.yml/badge.svg" alt="Jenova Runtime (Windows-x64)">
  </span>
  <span>
    <img src="https://github.com/Jenova-Framework/Jenova-Runtime/actions/workflows/runtime-linux-x64-build.yml/badge.svg" alt="Jenova Runtime (Linux-64)">
  </span>
</div>

## Overview

**Projekt J.E.N.O.V.A** is a comprehensive collection of modular components for the Godot Engine, designed to deliver major enhancements and break through the limitations of current development workflows. By bringing fully-featured C++ scripting directly into the Godot Editor, it enables the use of modern C++20/C++23 standards within Godot in a manner similar to GDScript.

**Jenova Framework** empowers developers to build entire games or applications natively in C/C++ with ease and stability. It supports seamless integration of cutting-edge technologies and allowing direct integration of OpenCV, CUDA, Vulkan, OpenMP and any other modern C++ features seamlessly, All supported by the powerful backend.

For more information and to view the full feature list, check out the details [here](https://github.com/Jenova-Framework/J.E.N.O.V.A#%EF%B8%8F-current-features).

### Jenova Runtime (Core)

This repository contains the full source code of **Jenova Runtime**, a full-featured scripting backend with a wide range of capabilities including a Build System, Compiler Interface, Script Objects, Script Language, Script Instances, Script Interpreter and more.

## Issue/Bug Reports and Questions
- If you want to report an issue or bug create a new thread at [Issues](https://github.com/Jenova-Framework/Jenova-Runtime/issues).
- If you have any questions you can create a new thread at [Discussions](https://github.com/Jenova-Framework/J.E.N.O.V.A/discussions).
- More details on the build process can be found at [Documentation](https://jenova-framework.github.io/docs/pages/Advanced/Build-Guide/).

## Dependencies

Jenova Core has following dependencies :

- [AsmJIT](https://github.com/asmjit/asmjit)
- [LibArchive](https://github.com/libarchive/libarchive)
- [LibLZMA](https://github.com/ShiftMediaProject/liblzma)
- [LibCurl](https://github.com/curl/curl)
- [LibFastZLib](https://github.com/gildor2/fast_zlib)
- [LibTinyCC](http://download.savannah.gnu.org/releases/tinycc/)
- [LibPThread](https://github.com/GerHobbelt/pthread-win32)
- [JSON++](https://github.com/nlohmann/json)
- [FileWatch](https://github.com/ThomasMonkman/filewatch)
- [ArgParse++](https://github.com/p-ranav/argparse)
- [Base64++](https://github.com/zaphoyd/websocketpp/blob/master/websocketpp/base64/base64.hpp)

> [!IMPORTANT]
> - Edit **base64.hpp** namespace to `base64`
> - Only header file `libtcc.h` is required from TinyCC beside static library
> - In **FileWatch.hpp** change `_callback(file.first, file.second);` to `_callback(_path + "/" + file.first, file.second);`
> - **By using Jenova Builder, All the dependencies are downloaded, manipulated and compiled automatically.**

## Build Systems

Jenova Runtime can be built on Windows x64 and Linux x64 using **Jenova Builder**.

**Prerequisites:**

*   Python (3.10+)
*   CMake (3.20+)
*   Ninja (1.11+)
*   Python packages: `pip install requests py7zr colored`

**Windows x64:** Requires Visual Studio (2022+) with C++20 support or the AiO Toolchain.

**Linux x64:** Compatible with Clang++ (18+) and G++ (13+).

For detailed build instructions and more information, see the [Build Guide](https://jenova-framework.github.io/docs/pages/Advanced/Build-Guide).

## Godot Compatibility
As of Jenova Runtime 0.4.0.0 LTS, **Godot 4.7 Stable** is the minimum required version due to breaking changes in [godot-cpp](https://github.com/godotengine/godot-cpp). While it is still possible to build 0.4.0.0+ for Godot 4.2–4.6 with minor modifications, the official builder is now fully migrated and fine-tuned for Godot 4.7 only.

## Open Source vs Proprietary

While the public source code of Jenova is ~90% identical to the proprietary version, a few specific features have been removed or disabled.
### These changes include :
- **Jenova Emulator Connector** is removed and will be made available later as an addon in the Package Manager.

- **A.K.I.R.A JIT** is removed from the public source code. This component was responsible for executing obfuscated code using a proprietary highly secured VM.

- **Code Encryption and Key System** has been omitted from the public version to protect critical proprietary algorithms. However, Code Compression is fully included, Developers can add their own encryption on top of the existing buffering system.

- **Jenova Code Virtualizer/Sandbox** removed due to reliance on the proprietary SecureAngel™ 2.0 technology.

### Cross-Platform

Jenova Proprietary version includes only Microsoft Visual C++ (MSVC) and Microsoft LLVM Clang (Clang-cl) compilers and is compatible only with Windows. Open-Source version, however, is fully ported to Linux and includes support for MSVC, Clang-cl, MinGW GCC and LLVM on Windows as well as GCC and LLVM Clang on Linux.

> [!IMPORTANT]  
> The proprietary version has been deprecated and is no longer maintained. All future development efforts are now focused exclusively on the open-source version, which will continue to incorporate enhancements from the previous proprietary releases, with the exception of security-related features.

![RepoBeats](https://repobeats.axiom.co/api/embed/292d48b5da1eb7a8a7db9362fe92577877ec5b51.svg "Repobeats Analytics Image")

----
Developed & Designed By **Hamid.Memar (MemarDesign™ LLC.)**
