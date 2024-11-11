# Projekt J.E.N.O.V.A :: Core (Public Access)

This repository contains the source code for the **Jenova Runtime** developed for Godot 4+.

![image](https://github.com/user-attachments/assets/013eed25-7047-407d-aef8-b964203e73b0)

## Overview

**J.E.N.O.V.A** is an extension library for the Godot 4 Game Engine that brings fully-featured C++ scripting directly into the Godot Editor. It allows the use of modern C++20 standards within the Godot Engine, similar to GDScript. With the Jenova Framework, there are no limits. You can integrate OpenCV, CUDA, Vulkan, OpenMP, and any other modern C++ features, all supported by the powerful MSVC backend.

A full feature list can be found [here](https://github.com/Jenova-Framework/J.E.N.O.V.A).

> [!IMPORTANT]  
> This source code is licensed under the MIT license.

## Issue/Bug Reports and Questions

If you would like to report an issue or bug, please create a new thread at [Issues](https://github.com/Jenova-Framework/Jenova-Runtime/issues).

If you have any questions, you can create a new thread at [Discussions](https://github.com/Jenova-Framework/J.E.N.O.V.A/discussions).

## Dependencies

Jenova Core has the following dependencies:

- [AsmJIT](https://github.com/asmjit/asmjit)
- [LibArchive](https://github.com/libarchive/libarchive)
- [LibCurl](https://github.com/curl/curl)
- [LibFastZLib](https://github.com/timotejroiko/fast-zlib)
- [LibTinyCC](http://download.savannah.gnu.org/releases/tinycc/)
- [LibVSWhere](https://github.com/TheAenema/libvswhere/tree/jenova-edition)
- [MemoryModule++](https://github.com/bb107/MemoryModulePP)
- [MinHook](https://github.com/TsudaKageyu/minhook)
- [JSON++](https://github.com/nlohmann/json)
- [FileWatch](https://github.com/ThomasMonkman/filewatch)
- [ArgParse++](https://github.com/p-ranav/argparse)
- [Base64++](https://github.com/zaphoyd/websocketpp/blob/master/websocketpp/base64/base64.hpp)

> [!NOTE]  
> - Edit the **base64.hpp** namespace to `base64`.  
> - Only the header file `libtcc.h` is required from TinyCC, besides the static library.  
> - In **FileWatch.hpp**, change `_callback(file.first, file.second);` to `_callback(_path + file.first, file.second);`.

> **Pre-built dependencies can be obtained from [here](https://drive.google.com/file/d/10qVZ3I0yVod3dSdpotCnNUcAs1jTDYG8/view?usp=sharing).**

## Build System

Jenova Core requires Visual Studio 2022 with C++20 support.  
You can always obtain pre-built binaries from [here](https://github.com/Jenova-Framework/J.E.N.O.V.A/releases).

## Open Source vs Proprietary

While the public source code of Jenova is ~99% identical to the proprietary version, a few specific features have been removed or disabled.

### These changes include:

- **Jenova Emulator Connector**: This is provided in full source code, but parts of the integration code for proprietary emulators have been removed. It remains functional if connected to another emulator module.
- **A.K.I.R.A JIT**: This component, responsible for executing obfuscated code using a proprietary, highly-secured VM, has been removed from the public source code.
- **Code Encryption and Key System**: This has been omitted from the public version to protect critical proprietary algorithms. However, code compression is fully included, and developers can add their own encryption on top of the existing buffering system.
- **Jenova Code Virtualizer/Sandbox**: This has been removed due to reliance on the proprietary SecureAngel™ 2.0 technology.

---

**Developed & Designed by**  
**Hamid.Memar**
