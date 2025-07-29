# Jenova Runtime (Core)

This repository contains source code of **Jenova Runtime** for Godot Engine and It's a part of **[Projekt J.E.N.O.V.A](https://github.com/Jenova-Framework/J.E.N.O.V.A)**

![image](https://github.com/user-attachments/assets/013eed25-7047-407d-aef8-b964203e73b0)

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
### Windows x64

Building fully-featured Jenova Runtime for Windows x64 requires Visual Studio 2022 with C++20 Support. 

#### Using GigaChad Toolchain

For Windows, Jenova Framework provides a massive compiler collection including all standard compilers :

- Microsoft Visual C++ (msvc)
- Microsoft Visual C++ LLVM (clang-cl)
- LLVM Clang (clang/clang++)
- Minimalist GNU for Windows (gcc/g++)

You can use **Jenova Builder** to build Jenova Runtime for Windows using GigaChad Toolchain. Everything is automated and all dependencies will be compiled by Jenova Builder. The compiled dependencies from Jenova Builder can be used in Visual Studio as well.

```bash
# Running Builder
python3 ./Jenova.Builder.py

# Build using MSVC
python3 ./Jenova.Builder.py --compiler win-msvc --skip-banner

# Build using MSVC LLVM
python3 ./Jenova.Builder.py --compiler win-clangcl --skip-banner

# Build using MinGW Clang
python3 ./Jenova.Builder.py --compiler win-clang --skip-banner

# Build using MinGW GCC
python3 ./Jenova.Builder.py --compiler win-gcc --skip-banner
```

The toolchain is only 150MB and contains all compiler toolchains All-in-One. You don't need to install anything extra, including Visual Studio.

> **Note :** Using Jenova Builder with `win-msvc` also compiles a fully-featured Jenova Runtime. It serves as an alternative to Visual Studio 2022 with C++20.

### Linux x64

Building Jenova Runtime for Linux x64 can be done using **Jenova Builder**. To build using Jenova Builder, Install Python 3 and then use the following commands from VSCode or Terminal:

```bash
# Running Builder
python3 ./Jenova.Builder.py

# Build using Clang++
python3 ./Jenova.Builder.py --compiler linux-clang --skip-banner

# Build using G++
python3 ./Jenova.Builder.py --compiler linux-gcc --skip-banner
```

Jenova Runtime source code is fully compatible with both Clang++ (18+) and G++ (13+). Additionally, [Visual Studio Code](https://code.visualstudio.com/) compatibility files are provided.

> **Note :** Code::Blocks project files have been deprecated and removed following the significant update to Jenova Builder.

### Builder Prerequisites

Before using **Jenova Builder** you need to install following prerequisites on both Windows and Linux.
- Python (3.10+)
- CMake (3.20+)
- Ninja (1.11+)

Additionally, the following Python packages are required for Builder :
```sh
pip install requests py7zr colored
```

>  Full information and a detailed usage guide for the build system can be found [here](https://jenova-framework.github.io/docs/pages/Advanced/Build-Guide).

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
> The Jenova Runtime Proprietary version is no longer available for public access. The last version released was 0.3.6.0 Alpha.
> Open-Source version will continue to receive all future updates from the proprietary version, Except those related to security.

----
Developed & Designed By **Hamid.Memar (MemarDesign™ LLC.)**
