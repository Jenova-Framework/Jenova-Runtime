# Jenova Runtime Build System Script
# Developed by Hamid.Memar (2024-2025)
# Usage : python3 ./Jenova.Builder.py --compiler win-msvc --skip-banner
# Use python3 ./Jenova.Builder.py --help For More Information.

# Imports
import os
import sys
import json
import time
import shutil
import hashlib
import requests
import argparse
import platform
import subprocess
import py7zr
from concurrent.futures import ThreadPoolExecutor
from colored import fg, attr

# Set Encoding
sys.stdout.reconfigure(encoding='utf-8')
sys.stderr.reconfigure(encoding='utf-8')

# Shared Database
flags = [
    "NDEBUG",
    "JENOVA_RUNTIME",
    "JENOVA_SDK_BUILD",
    "TYPED_METHOD_BIND"
]
directories = [
    "Libs",
    "Libs/GodotSDK",
    "Libs/GodotSDK/godot_cpp",
    "Libs/LithiumSDK",
    "Libs/LithiumSDK/lithium_cpp",   
    "Libs/Archive",
    "Libs/Curl",
    "Source"
]
sources = [
    "Source/jenova.cpp",
    "Source/jenovaSDK.cpp",
    "Source/script_instance_base.cpp",
    "Source/script_object.cpp",
    "Source/script_instance.cpp",
    "Source/script_resource.cpp",
    "Source/script_interpreter.cpp",
    "Source/script_templates.cpp",
    "Source/script_language.cpp",
    "Source/script_compiler.cpp",
    "Source/script_profiler.cpp",
    "Source/script_manager.cpp",
    "Source/clektron.cpp",
    "Source/tiny_profiler.cpp",
    "Source/task_system.cpp",
    "Source/package_manager.cpp",
    "Source/asset_monitor.cpp",
    "Source/gdextension_exporter.cpp"
]

# Global Options
builder_version     = "2.8"
deps_version        = "4.5"
double_precision    = False
static_build        = False
skip_deps           = False
skip_cache          = False
skip_packaging      = False
deploy_mode         = False
lithium_edition     = False

# Global Functions
def rgb_print(hex_color, output, inplace = False):
    hex_color = hex_color.lstrip('#')
    rgb = tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))
    ansi_color = 16 + (36 * (rgb[0] // 51) + 6 * (rgb[1] // 51) + (rgb[2] // 51))
    color_code = fg(ansi_color)
    reset_code = attr('reset')
    if inplace:
        print(f"\r{color_code}{output}{reset_code}", end="")
    else:
        print(f"{color_code}{output}{reset_code}")
def format_duration(seconds):
    milliseconds = int((seconds - int(seconds)) * 1000)
    seconds = int(seconds)
    minutes, seconds = divmod(seconds, 60)
    hours, minutes = divmod(minutes, 60)
    return f"{hours}h {minutes}m {seconds}s {milliseconds}ms"
def guard_path(path):
    return '\"' + path + '\"'
def get_os_name():
    system_name = platform.system()
    architecture = "x64" if platform.machine() in ["AMD64", "x86_64"] else platform.machine()
    if system_name == "Windows":
        import sys
        win_ver = sys.getwindowsversion()
        if win_ver.build >= 26000: return f"Microsoft Windows 12 {architecture}"
        if win_ver.build >= 22000: return f"Microsoft Windows 11 {architecture}"
        if win_ver.build >= 19000: return f"Microsoft Windows 10 {architecture}"
        if win_ver.build >= 9600: return f"Microsoft Windows 8.1 {architecture}"
        if win_ver.build >= 9200: return f"Microsoft Windows 8 {architecture}"
        if win_ver.build >= 7600: return f"Microsoft Windows 7 {architecture}"
        return f"Microsoft Windows {platform.win32_ver()[0]} {architecture}"
    elif system_name == "Linux":
        try:
            import distro
            return f"{distro.name(pretty=True) or distro.id().capitalize() or 'Linux'} {architecture}"
        except ImportError:
            return f"{system_name} {platform.release()} {architecture}"
    return f"{system_name} {platform.release()} {architecture}"
def get_toolchain_name(buildMode):
    if buildMode == "win-msvc": return "msvc"
    if buildMode == "win-gcc": return "mingw"
    if buildMode == "win-clangcl": return "msvc-llvm"
    if buildMode == "win-clang": return "mingw-llvm"
    if buildMode == "linux-clang": return "llvm"
    if buildMode == "linux-gcc": return "gnu"
def print_banner():
    banner = r"""
===========================================================================
       _________   ______ _    _____       ____        _ __    __         
      / / ____/ | / / __ \ |  / /   |     / __ )__  __(_) /___/ /__  _____
 __  / / __/ /  |/ / / / / | / / /| |    / __  / / / / / / __  / _ \/ ___/
/ /_/ / /___/ /|  / /_/ /| |/ / ___ |   / /_/ / /_/ / / / /_/ /  __/ /    
\____/_____/_/ |_/\____/ |___/_/  |_|  /_____/\__,_/_/_/\__,_/\___/_/     
                                                                          
===========================================================================
                        Developed by Hamid.Memar
===========================================================================
    """
    rgb_print("#42f569", banner)
    rgb_print("#2942ff", f".:: Jenova Build System v{builder_version} ::.\n")
def get_compiler_choice():
    global compiler, linker
    rgb_print("#ff2474", "[ ? ] Select Supported Compiler :\n")
    rgb_print("#4eb8ff", "----------------- Windows x64 -----------------")
    print("1. Microsoft Visual C++ (win-msvc) [Recommended]")
    print("2. Microsoft Visual C++ LLVM (win-clangcl)")
    print("3. LLVM Clang (win-clang)")
    print("4. Minimalist GNU for Windows (win-gcc)")
    rgb_print("#fff06e", "\n------------------ Linux x64 ------------------")
    print("5. LLVM Clang (linux-clang) [Recommended]")
    print("6. GNU Compiler Collection (linux-gcc)")
    rgb_print("#ff6666", f"\nYou are currently running {get_os_name()} using ({os.cpu_count()} Cores)") 
    rgb_print("#666666", "\nEnter Build Mode: ")
    choice = input("")
    if choice == '1':
        build_windows("cl.exe", "link.exe", "win-msvc", "Microsoft Visual C++")
    elif choice == '2':
        build_windows("clang-cl.exe", "lld-link.exe", "win-clangcl", "Microsoft Visual C++ LLVM")
    elif choice == '3':
        build_windows("clang++.exe", "clang++.exe", "win-clang", "LLVM Clang")
    elif choice == '4':
        build_windows("g++.exe", "g++.exe", "win-gcc", "Minimalist GNU")
    elif choice == '5':
        build_linux("clang++", "clang++", "linux-clang", "LLVM Clang")
    elif choice == '6':
        build_linux("g++", "g++", "linux-gcc", "GNU Compiler Collection")
    else:
        rgb_print("#e02626", "[ x ] Error : Invalid Choice.")
        exit(-1)
def generate_gdsdk(output_dir):
    rgb_print("#367fff", "[ ^ ] Generating GodotSDK Package...")
    source_sdk_path = "./Libs/GodotSDK"
    target_sdk_path = os.path.join(output_dir, "GodotSDK")

    if os.path.exists(target_sdk_path):
        shutil.rmtree(target_sdk_path)
    shutil.copytree(source_sdk_path, target_sdk_path)

    for root, _, files in os.walk(target_sdk_path):
        for file in files:
            if file.endswith(".hpp"):
                file_path = os.path.join(root, file)
                with open(file_path, "r", encoding="utf-8") as f:
                    lines = f.readlines()
                patched_lines = [
                    line.replace('<godot_cpp', '<Godot') for line in lines
                ]
                with open(file_path, "w", encoding="utf-8") as f:
                    f.writelines(patched_lines)

    old_folder = os.path.join(target_sdk_path, "godot_cpp")
    new_folder = os.path.join(target_sdk_path, "Godot")
    if os.path.exists(old_folder):
        os.rename(old_folder, new_folder)

    for ext in [".lib", ".a"]:
        lib_name = f"libgodotcpp-static-x86_64{ext}"
        src_lib_path = os.path.join("./Libs", lib_name)
        if os.path.exists(src_lib_path):
            dst_lib_name = f"libGodot.x64{ext}"
            dst_lib_path = os.path.join(target_sdk_path, dst_lib_name)
            shutil.copy2(src_lib_path, dst_lib_path)

    rgb_print("#38f227", "[ √ ] Godot SDK Package Successfully Generated.")
def generate_jnvsdk(output_file):
    with open("./Source/JenovaSDK.h", "rb") as f:
        data = f.read()
        size = len(data)
    with open(output_file, "w") as out:
        out.write('#include <cstddef>\n')
        out.write('namespace jenova {\nnamespace plugin {\n')    
        out.write('static const char JenovaSDKHeader[] = {\n')
        out.write(', '.join(f'0x{byte:02x}' for byte in data))
        out.write('\n};\n')
        out.write(f'static const size_t JenovaSDKHeaderSize = {size};\n')
        out.write('const char* GetJenovaSDKHeaderData() { return JenovaSDKHeader; }\n')
        out.write(f'size_t GetJenovaSDKHeaderSize() {{ return JenovaSDKHeaderSize; }}\n')
        out.write('}\n}\n')

# Generic Build Functions
def compute_md5(file_path):
    hasher = hashlib.md5()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()
def load_cache(cacheDB):
    if skip_cache: return {}
    if os.path.exists(cacheDB):
        with open(cacheDB, "r") as f:
            return json.load(f)
    return {}
def save_cache(cache, cacheDB):
    with open(cacheDB, "w") as f:
        json.dump(cache, f, indent=4)
def should_compile(source, cache):
    source_path = os.path.abspath(source)
    if source_path not in cache:
        return True
    cached_entry = cache[source_path]
    current_timestamp = os.path.getmtime(source)
    current_md5 = compute_md5(source)
    return cached_entry["timestamp"] != current_timestamp or cached_entry["md5"] != current_md5
def update_cache_entry(source, cache):
    source_path = os.path.abspath(source)
    cache[source_path] = {
        "timestamp": os.path.getmtime(source),
        "md5": compute_md5(source)
    }
def run_compile_command(command, source_name, compiler):
    try:
        rgb_print("#ff2474", f"[ > ] Compiling Source File '{os.path.basename(source_name)}' Using {compiler}...")
        subprocess.check_call(command, shell=True)
        rgb_print("#38f227", f"[ √ ] Source File '{os.path.basename(source_name)}' Compiled.")
    except subprocess.CalledProcessError as e:
        rgb_print("#e02626", f"[ x ] Source File '{os.path.basename(source_name)}' Failed to Compile, Error : {e}")
        exit(1)
def run_linker_command(command, linker):
    try:
        rgb_print("#ff2474", f"[ > ] Linking Compiled Object Files using {linker}...")
        subprocess.check_call(command, shell=True)
        rgb_print("#38f227", f"[ √ ] All Object Files Linked Successfully.")
    except subprocess.CalledProcessError as e:
        rgb_print("#e02626", f"[ x ] Failed to Link Object Files, Error : {e}")
        exit(1)    
def install_dependencies():
    
    # Validate Dependencies
    if os.path.exists("./Dependencies"): return

    # Dependencies Package URL
    dependencies_pkg_url = f"https://jenova-framework.github.io/download/development/Jenova-Runtime-Dependencies-Universal-{deps_version}.jnvpkg"
    
    # Downloading Dependencies Package
    rgb_print("#367fff", "[ ^ ] Downloading Dependencies Package...")
    response = requests.get(dependencies_pkg_url, stream=True)
    if response.status_code == 200:
        total_size = int(response.headers.get('content-length', 0))
        block_size = 1024
        max_bar_length = 50
        progress_length = 0
        with open("Jenova-Runtime-Dependencies-Universal.jnvpkg", "wb") as f:
            for data in response.iter_content(block_size):
                f.write(data)
                progress_length += len(data)
                done = progress_length
                bar_length = int((done / total_size) * max_bar_length)
                progress_bar = '[ ' + '=' * bar_length + ' ' * (max_bar_length - bar_length) + ' ]'
                if not deploy_mode: rgb_print("#367fff", f"[ + ] Downloading : {done/total_size:.2%} {progress_bar}", True)

        rgb_print("#38f227", "\n[ √ ] Dependencies Package Download Complete.")
    else:
        rgb_print("#e02626", "[ x ] Error : Failed to Download Dependencies Package.")
        return

    # Extracting Dependencies Package
    rgb_print("#367fff","[ ^ ] Installing Dependencies Package...")
    with py7zr.SevenZipFile("Jenova-Runtime-Dependencies-Universal.jnvpkg", mode='r') as z: z.extractall(path="./Dependencies")
    rgb_print("#38f227", "[ √ ] Dependencies Package Installed.")

    # Delete Downloaded Package
    os.remove("Jenova-Runtime-Dependencies-Universal.jnvpkg")
def build_with_ninja(buildPath):
    if platform.system() == "Windows": subprocess.run(["ninja.exe", "-C", buildPath, "-j", f"{os.cpu_count()}"], check=True)
    if platform.system() == "Linux": subprocess.run(["ninja", "-C", buildPath, "-j", f"{os.cpu_count()}"], check=True)
def create_distribution_package(package_files, output_archive):

    # Initialize Archive
    rgb_print("#367fff", "[ ^ ] Creating Distribution Package...")

    # Set Compression Settings
    compression_options = {
        'method': 'lzma2',
        'level': 9,
        'filters': [
            {
                'id': py7zr.FILTER_LZMA2,
                'preset': 9
            }
        ]
    }
    
    # Create Archive
    with py7zr.SevenZipFile(output_archive, 'w', filters=compression_options['filters']) as archive:
        for file in package_files:
            src_path = file['src']
            dst_path = file['dst']
            if os.path.exists(src_path):
                file_name = os.path.basename(src_path)
                if file_name != ".gitignore": rgb_print("#764be3", f"[ * ] Packing File {file_name}...")
                archive.write(src_path, os.path.join(dst_path, file_name))

    # Verbose Success
    rgb_print("#38f227", "[ √ ] Jenova Runtime Distribution Package Created.")
def clean_up_build(deepMode):
    if os.path.exists("./Libs"): shutil.rmtree("./Libs")
    if os.path.exists("./Win64"): shutil.rmtree("./Win64")
    if os.path.exists("./Linux64"): shutil.rmtree("./Linux64")
    if deepMode:
        if os.path.exists("./Toolchain"): shutil.rmtree("./Toolchain")
        if os.path.exists("./Dependencies"): shutil.rmtree("./Dependencies")
    rgb_print("#03fc6f", f"[ √ ] Clean Up Successfully Finished.")
    exit(0)

# Linux Build Functions
def initialize_toolchain_linux():
    # No Toolchain Provided Yet
    return
def build_dependencies_linux(buildMode, cacheDir):
    
    # Install Dependencies
    install_dependencies()

    # Create Library
    os.makedirs("Libs", exist_ok=True)
    shutil.copytree("./Dependencies/libjenova", "./Libs", dirs_exist_ok=True)

    # Set Compiler/Linker Configuration
    os.environ['CC'] = 'clang' if buildMode == "linux-clang" else 'gcc'
    os.environ['CXX'] = 'clang++' if buildMode == "linux-clang" else 'g++'
    os.environ['CFLAGS'] = '-fPIC -w -m64'
    os.environ['CXXFLAGS'] = '-std=c++20 -w -fPIC -m64'
    os.environ['LDFLAGS'] = '-static-libstdc++ -static-libgcc -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -ldl -lrt -m64'
    os.environ['C_COMPILER'] = 'clang' if buildMode == "linux-clang" else 'gcc'
    os.environ['CXX_COMPILER'] = 'clang++' if buildMode == "linux-clang" else 'g++'

    # Set Specific Build System Configuration
    if buildMode == "linux-clang":
        os.environ['CFLAGS'] += ' -Qunused-arguments -w -Wno-unused-command-line-argument'
        os.environ['CXXFLAGS'] += ' -Qunused-arguments -w -Wno-unused-command-line-argument'

    # Build AsmJIT
    if not os.path.exists("./Libs/libasmjit-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/asmjit"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libasmjit",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DASMJIT_STATIC=ON"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/libasmjit.a", "./Libs/libasmjit-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'AsmJIT' Compiled Successfully.")

    # Build Curl
    if not os.path.exists("./Libs/libcurl-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/curl"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libcurl",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DCURL_BROTLI=OFF",
            "-DCURL_USE_LIBPSL=OFF"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/lib/libcurl.a", "./Libs/libcurl-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'Curl' Compiled Successfully.")

    # Build LZMA
    if not os.path.exists("./Libs/liblzma-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/lzma"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/liblzma",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/liblzma.a", "./Libs/liblzma-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'LZMA' Compiled Successfully.")

    # Build Archive
    if not os.path.exists("./Libs/libarchive-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/archive"
        lzmaInclude = os.path.abspath('./Dependencies/liblzma/src/liblzma/api')
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libarchive",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DENABLE_LZMA=ON",
            "-DHAVE_LZMA_H=ON",
            "-DBUILD_TESTING=OFF",
            "-DENABLE_TEST=OFF",
            "-DENABLE_LZ4=OFF",
            "-DENABLE_TAR=OFF",
            "-DENABLE_CAT=OFF",
            "-DENABLE_CPIO=OFF",
            "-DENABLE_EXPAT=OFF",
            "-DENABLE_BZip2=OFF",
            "-DENABLE_TEST=OFF",
            f"-DCMAKE_C_FLAGS={os.environ['CFLAGS'] + ' -DLZMA_API_STATIC ' + f'-I{guard_path(lzmaInclude)}'}"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/libarchive/libarchive.a", "./Libs/libarchive-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'Archive' Compiled Successfully.")

    # Build XML2
    if not os.path.exists("./Libs/libxml2-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/xml2"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libxml2",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DLIBXML2_WITH_MODULES=OFF",
            "-DLIBXML2_WITH_PYTHON=OFF",
            "-DLIBXML2_WITH_TESTS=OFF",
            "-DLIBXML2_WITH_ICONV=OFF"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/libxml2.a", "./Libs/libxml2-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'XML2' Compiled Successfully.")

    # Build ZLIB
    if not os.path.exists("./Libs/libzlib-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/zlib"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        os.makedirs(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libzlib",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DZLIB_BUILD_TESTING=OFF",
            "-DZLIB_BUILD_SHARED=OFF",
            "-DZLIB_BUILD_STATIC=ON"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/libz.a", "./Libs/libzlib-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'ZLIB' Compiled Successfully.")

    # Build TinyCC
    if not os.path.exists("./Libs/libtcc-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/tinycc"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libtinycc",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
        ], check=True)
        build_with_ninja(buildPath)
        shutil.copyfile(buildPath + "/liblibtcc.a", "./Libs/libtcc-static-x86_64.a")
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'TinyCC' Compiled Successfully.")

    # Build GodotSDK
    if not os.path.exists("./Libs/libgodotcpp-static-x86_64.a"):
        buildPath = cacheDir + "/Dependencies/godotcpp"
        sdkPath = "./Libs/GodotSDK"
        if os.path.exists(buildPath): shutil.rmtree(buildPath)
        subprocess.run([
            "cmake",
            "-S", "./Dependencies/libgodot",
            "-B", buildPath,
            "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=MinSizeRel",
            "-DGODOTCPP_USE_HOT_RELOAD=ON",
            "-DGODOTCPP_TARGET=editor",
            f"-DGODOTCPP_PRECISION={'double' if double_precision else 'single'}",
            "-DBUILD_SHARED_LIBS=OFF"
        ], check=True)
        build_with_ninja(buildPath)
        binary_path = buildPath + f"/bin/libgodot-cpp.linux.editor{'.double' if double_precision else ''}.x86_64.a"
        shutil.copyfile(binary_path, "./Libs/libgodotcpp-static-x86_64.a")
        if os.path.exists(sdkPath): shutil.rmtree(sdkPath)
        os.makedirs(sdkPath, exist_ok=True)
        shutil.copyfile("./Dependencies/libgodot/gdextension/gdextension_interface.h", sdkPath + "/gdextension_interface.h")
        shutil.copytree("./Dependencies/libgodot/include", sdkPath, dirs_exist_ok=True)
        shutil.copytree(buildPath + "/gen/include", sdkPath, dirs_exist_ok=True)
        rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'GodotSDK' Compiled Successfully.")
def build_linux(compilerBinary, linkerBinary, buildMode, buildSystem):

    # Verbose Build
    rgb_print("#fff06e", f"[ > ] Building Jenova Runtime for Linux using {buildSystem} Toolchain...")

    # Dependencies
    libs = [
        "Libs/libzlib-static-x86_64.a",
        "Libs/libtcc-static-x86_64.a",
        "Libs/libgodotcpp-static-x86_64.a",
        "Libs/libcurl-static-x86_64.a",
        "Libs/libasmjit-static-x86_64.a",
        "Libs/libarchive-static-x86_64.a",
        "Libs/liblzma-static-x86_64.a",
        "Libs/libxml2-static-x86_64.a"
    ]

    # Configuration
    compiler = compilerBinary
    linker = linkerBinary
    outputDir = "Linux64"
    outputName = "Jenova.Runtime.Linux64.so"
    mapFileName = "Jenova.Runtime.Linux64.map"
    cacheDir = f"{outputDir}/Cache"
    sdkDir = f"{outputDir}/JenovaSDK"
    CacheDB = f"{cacheDir}/Build.db"

    # Ensure Required Directories Exist
    rgb_print("#367fff", "[ ^ ] Validating Paths...")
    os.makedirs(outputDir, exist_ok=True)
    os.makedirs(cacheDir, exist_ok=True)
    os.makedirs(sdkDir, exist_ok=True)

    # Initialize Toolchain
    initialize_toolchain_linux()

    # Build Dependencies
    if not skip_deps:
        rgb_print("#367fff", "[ ^ ] Building Dependencies...")
        build_dependencies_linux(buildMode, cacheDir)
    else:
        rgb_print("#367fff", "[ ^ ] Building Dependencies Skipped by User.")

    # Load Cache
    rgb_print("#367fff", "[ ^ ] Loading Cache...")
    cache = load_cache(CacheDB)

    # Generate Compile Commands
    rgb_print("#367fff", "[ ^ ] Generating Compiler Commands...")
    compile_commands = {}
    object_files = []
    for source in sources:
        object_file = f"{cacheDir}/{os.path.basename(source).replace('.cpp', '.o')}"
        object_files.append(object_file)
        compile_command = (
            f"{compiler} -m64 -O3 -fPIC -pipe -w -std=c++20 -pthread -fexceptions "
            f"{' '.join([f'-D{flag}' for flag in flags])} "
            f"{' '.join([f'-I{directory}' for directory in directories])} "
            f"-c {source} -o {object_file}"
        )
        compile_commands[source] = compile_command

    # Compile Source Code
    rgb_print("#367fff", "[ ^ ] Compiling Jenova Runtime Source Code...")
    with ThreadPoolExecutor(max_workers=len(sources)) as executor:
        futures = []
        for source, command in compile_commands.items():
            if should_compile(source, cache):
                futures.append(executor.submit(run_compile_command, command, source, compiler))
                update_cache_entry(source, cache)
            else:
                rgb_print("#f59b42", f"[ ! ] Skipping Compilation for {os.path.basename(source)}, No Changes Detected.")
        for future in futures: future.result()

    # Save Cache
    rgb_print("#367fff", "[ ^ ] Saving Cache...")
    save_cache(cache, CacheDB)

    # Generate Linker Command
    rgb_print("#367fff", "[ ^ ] Generating Linker Command...")
    link_command = (
        f"{linker} {'-static' if static_build else '-shared'} -fPIC {' '.join(object_files)} -o {outputDir}/{outputName} -m64 "
        f"-static-libstdc++ -static-libgcc -L/usr/lib64 {' '.join(libs)} -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -ldl -lrt -lzstd "
        f"-Wl,-Map,{outputDir}/{mapFileName}"
    )

    # Link Object Files
    rgb_print("#367fff", "[ ^ ] Linking Jenova Runtime Binary...")
    run_linker_command(link_command, linker)

    # Prepare Release
    open(f"{sdkDir}/.gitignore", "w").write("*")
    shutil.copy2("./Source/JenovaSDK.h", f"{sdkDir}/JenovaSDK.h")
    shutil.copy2("./Jenova.Runtime.gdextension", f"{outputDir}/Jenova.Runtime.gdextension")

    # Generate Godot SDK Package
    if args.generate_gdsdk: generate_gdsdk(outputDir)

    # Create Package
    if skip_packaging: return
    packageFiles = [
        {"src": f"{outputDir}/{outputName}", "dst": "./Jenova"},
        {"src": f"{outputDir}/Jenova.Runtime.gdextension", "dst": "./Jenova"},
        {"src": f"{sdkDir}/JenovaSDK.h", "dst": "./Jenova/JenovaSDK"},
        {"src": f"{sdkDir}/.gitignore", "dst": "./Jenova/JenovaSDK"}
    ]
    os.makedirs(f"{outputDir}/Distribution", exist_ok=True)
    toolchainName = get_toolchain_name(buildMode)
    create_distribution_package(packageFiles, f"{outputDir}/Distribution/Jenova-Framework-Linux64-{toolchainName}.7z")

# Windows Build Functions
def initialize_toolchain_windows():

    # Validate Toolchain
    if not os.path.exists("./Toolchain"): 

        # GigaChad Toolchain Package URL
        gigachad_toolchain_pkg_pt1_url = "https://jenova-framework.github.io/download/development/GigaChad-Toolchain-v1.0-Win64-U1.jnvpkg"
        gigachad_toolchain_pkg_pt2_url = "https://jenova-framework.github.io/download/development/GigaChad-Toolchain-v1.0-Win64-U2.jnvpkg"

        # Downloading GigaChad Toolchain Package
        rgb_print("#367fff", "[ ^ ] Downloading GigaChad Toolchain Package...")
        response = requests.get(gigachad_toolchain_pkg_pt1_url, stream=True)
        if response.status_code == 200:
            total_size = int(response.headers.get('content-length', 0))
            block_size = 1024
            max_bar_length = 50
            progress_length = 0
            with open("GigaChad-Toolchain-v1.0-Win64-U1.jnvpkg", "wb") as f:
                for data in response.iter_content(block_size):
                    f.write(data)
                    progress_length += len(data)
                    done = progress_length
                    bar_length = int((done / total_size) * max_bar_length)
                    progress_bar = '[ ' + '=' * bar_length + ' ' * (max_bar_length - bar_length) + ' ]'
                    if not deploy_mode: rgb_print("#367fff", f"[ + ] Downloading First Unit: {done/total_size:.2%} {progress_bar}", True)
            rgb_print("#38f227", "\n[ √ ] GigaChad Toolchain Package First Unit Download Complete.")
        else:
            rgb_print("#e02626", "[ x ] Error : Failed to Download GigaChad Toolchain Package First Unit.")
            return
        response = requests.get(gigachad_toolchain_pkg_pt2_url, stream=True)
        if response.status_code == 200:
            total_size = int(response.headers.get('content-length', 0))
            block_size = 1024
            max_bar_length = 50
            progress_length = 0
            with open("GigaChad-Toolchain-v1.0-Win64-U2.jnvpkg", "wb") as f:
                for data in response.iter_content(block_size):
                    f.write(data)
                    progress_length += len(data)
                    done = progress_length
                    bar_length = int((done / total_size) * max_bar_length)
                    progress_bar = '[ ' + '=' * bar_length + ' ' * (max_bar_length - bar_length) + ' ]'
                    if not deploy_mode: rgb_print("#367fff", f"[ + ] Downloading Second Unit : {done/total_size:.2%} {progress_bar}", True)
            rgb_print("#38f227", "\n[ √ ] GigaChad Toolchain Package Second Unit Download Complete.")
        else:
            rgb_print("#e02626", "[ x ] Error : Failed to Download GigaChad Toolchain Package Second Unit.")
            return

        # Merge Gigachad Toolchain Package Units
        parts = ["GigaChad-Toolchain-v1.0-Win64-U1.jnvpkg", "GigaChad-Toolchain-v1.0-Win64-U2.jnvpkg"]
        with open("GigaChad-Toolchain-v1.0-Win64.jnvpkg", "wb") as merged_file:
            for part in parts:
                with open(part, "rb") as part_file: merged_file.write(part_file.read())

        # Extracting Gigachad Toolchain Package
        rgb_print("#367fff","[ ^ ] Installing GigaChad Toolchain Package...")
        with py7zr.SevenZipFile("GigaChad-Toolchain-v1.0-Win64.jnvpkg", mode='r') as z: z.extractall(path="./Toolchain")
        rgb_print("#38f227", "[ √ ] GigaChad Toolchain Package Installed.")

        # Delete Downloaded Package
        os.remove("GigaChad-Toolchain-v1.0-Win64.jnvpkg")
        os.remove("GigaChad-Toolchain-v1.0-Win64-U1.jnvpkg")
        os.remove("GigaChad-Toolchain-v1.0-Win64-U2.jnvpkg")

    # Configuring Toolchain
    rgb_print("#367fff", "[ ^ ] Configuring Toolchain...")
    toolchain_path = os.path.abspath("./Toolchain/bin")
    current_path = os.environ.get("PATH", "")
    if toolchain_path not in current_path: os.environ["PATH"] = f"{toolchain_path};{current_path}"
    
    # Set Toolchain Path
    os.environ['TOOLCHAIN_PATH'] = os.path.abspath("./Toolchain")

    # Verbose
    rgb_print("#38f227",f"[ √ ] Toolchain ({os.environ['TOOLCHAIN_PATH']}) has been Configured Successfully.")
def build_dependencies_windows(buildMode, cacheDir):

    # Install Dependencies
    install_dependencies()

    # Create Library
    os.makedirs("Libs", exist_ok=True)
    shutil.copytree("./Dependencies/libjenova", "./Libs", dirs_exist_ok=True)

    # Build Dependencies for MSVC & LLVM
    if buildMode == "win-msvc" or buildMode == "win-clangcl":
        
        # Set Compiler/Linker Configuration
        os.environ['CC'] = 'cl.exe' if buildMode == "win-msvc" else 'clang-cl.exe'
        os.environ['CXX'] = 'cl.exe' if buildMode == "win-msvc" else 'clang-cl.exe'
        os.environ['CLFLAGS'] = '/MT /std:c++20 '
        os.environ['LDFLAGS'] = '/LTCG:incremental'
        os.environ["INCLUDE"] = f"{os.environ['TOOLCHAIN_PATH']}/x86_64-msvc/Include"
        os.environ["LIB"] = f"{os.environ['TOOLCHAIN_PATH']}/x86_64-msvc/Lib"
        os.environ["LIBPATH"] = f"{os.environ['TOOLCHAIN_PATH']}/x86_64-msvc/Lib"     

        # Set Specific Build System Configuration
        if buildMode == "win-clangcl":
            os.environ['CLFLAGS'] += '-w'
            os.environ['CFLAGS'] = '-w'
            os.environ['CXXFLAGS'] = '-w'

        # Build AsmJIT
        if not os.path.exists("./Libs/libasmjit-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/asmjit"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libasmjit",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DASMJIT_STATIC=ON",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
                f"-DCMAKE_CXX_FLAGS={os.environ['CLFLAGS']} /wd5054"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/asmjit.lib", "./Libs/libasmjit-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'AsmJIT' Compiled Successfully.")
            
        # Build Curl
        if not os.path.exists("./Libs/libcurl-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/curl"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libcurl",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCURL_USE_OPENSSL=OFF",
                "-DCURL_USE_SCHANNEL=ON",
                "-DCURL_USE_LIBPSL=OFF",
                "-DBUILD_LIBCURL_DOCS=OFF",
                "-DBUILD_MISC_DOCS=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
            ], check=True)

            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/lib/libcurl.lib", "./Libs/libcurl-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'Curl' Compiled Successfully.")

        # Build LZMA
        if not os.path.exists("./Libs/liblzma-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/lzma"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/liblzma",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/lzma.lib", "./Libs/liblzma-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'LZMA' Compiled Successfully.")

        # Build Archive
        if not os.path.exists("./Libs/libarchive-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/archive"
            lzmaInclude = os.path.abspath('./Dependencies/liblzma/src/liblzma/api')
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libarchive",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DENABLE_LZMA=ON",
                "-DHAVE_LZMA_H=ON",
                "-DBUILD_TESTING=OFF",
                "-DENABLE_TEST=OFF",
                "-DENABLE_LZ4=OFF",
                "-DENABLE_TAR=OFF",
                "-DENABLE_CAT=OFF",
                "-DENABLE_CPIO=OFF",
                "-DENABLE_EXPAT=OFF",
                "-DENABLE_BZip2=OFF",
                "-DENABLE_TEST=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
                f"-DCMAKE_C_FLAGS={os.environ['CLFLAGS'] + ' -DLZMA_API_STATIC ' + f'/I{guard_path(lzmaInclude)}'}"
            ], check=True)
            os.environ["INCLUDE"] += f";"
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libarchive/archive.lib", "./Libs/libarchive-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'Archive' Compiled Successfully.")

        # Build XML2
        if not os.path.exists("./Libs/libxml2-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/xml2"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libxml2",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DLIBXML2_WITH_MODULES=OFF",
                "-DLIBXML2_WITH_PYTHON=OFF",
                "-DLIBXML2_WITH_TESTS=OFF",
                "-DLIBXML2_WITH_ICONV=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libxml2s.lib", "./Libs/libxml2-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'XML2' Compiled Successfully.")

        # Build ZLIB
        if not os.path.exists("./Libs/libzlib-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/zlib"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            os.makedirs(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libzlib",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DZLIB_BUILD_TESTING=OFF",
                "-DZLIB_BUILD_SHARED=OFF",
                "-DZLIB_BUILD_STATIC=ON",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/zs.lib", "./Libs/libzlib-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'ZLIB' Compiled Successfully.")

        # Build PThread
        if not os.path.exists("./Libs/libpthread-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/pthread"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libpthread",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
                "-DCMAKE_C_FLAGS=/w",
                "-DCMAKE_CXX_FLAGS=/w"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/pthreadVSE3.lib", "./Libs/libpthread-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'PThread' Compiled Successfully.")
         
        # Build TinyCC
        if not os.path.exists("./Libs/libtcc-static-x86_64.lib"):
            buildPath = cacheDir + "/Dependencies/tinycc"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libtinycc",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libtcc.lib", "./Libs/libtcc-static-x86_64.lib")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'TinyCC' Compiled Successfully.")
     
        # Build GodotSDK
        if not os.path.exists("./Libs/libgodotcpp-static-x86_64.lib"):
            os.environ["GODOT_MSVC_STATIC_RUNTIME"] = "true"
            buildPath = cacheDir + "/Dependencies/godotcpp"
            sdkPath = "./Libs/GodotSDK"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libgodot",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DGODOTCPP_USE_HOT_RELOAD=ON",
                "-DGODOTCPP_TARGET=editor",
                f"-DGODOTCPP_PRECISION={'double' if double_precision else 'single'}",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
                f"-DCMAKE_CXX_FLAGS={os.environ['CLFLAGS']}"
            ], check=True)
            build_with_ninja(buildPath)
            binary_path = buildPath + f"/bin/libgodot-cpp.windows.editor{'.double' if double_precision else ''}.x86_64.lib"
            shutil.copyfile(binary_path, "./Libs/libgodotcpp-static-x86_64.lib")
            if os.path.exists(sdkPath): shutil.rmtree(sdkPath)
            os.makedirs(sdkPath, exist_ok=True)
            shutil.copyfile("./Dependencies/libgodot/gdextension/gdextension_interface.h", sdkPath + "/gdextension_interface.h")
            shutil.copytree("./Dependencies/libgodot/include", sdkPath, dirs_exist_ok=True)
            shutil.copytree(buildPath + "/gen/include", sdkPath, dirs_exist_ok=True)
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'GodotSDK' Compiled Successfully.")

    # Build Dependencies for Clang & GNU
    if buildMode == "win-clang" or buildMode == "win-gcc":

        # Set Compiler/Linker Configuration
        os.environ['CC'] = 'clang.exe' if buildMode == "win-clang" else 'gcc.exe'
        os.environ['CXX'] = 'clang++.exe' if buildMode == "win-clang" else 'g++.exe'
        os.environ['CFLAGS'] = '-static -w -m64'
        os.environ['CXXFLAGS'] = '-static -std=c++20 -w -m64'
        os.environ['LDFLAGS'] = '-static -static-libgcc -static-libstdc++ -pthread -m64'
        os.environ['C_COMPILER'] = 'clang' if buildMode == "win-clang" else 'gcc'
        os.environ['CXX_COMPILER'] = 'clang++' if buildMode == "win-clang" else 'g++'

        # Set Specific Build System Configuration
        if buildMode == "win-clang":
            os.environ['CFLAGS'] += ' -Qunused-arguments'
            os.environ['CXXFLAGS'] += ' -Qunused-arguments -femulated-tls'

        # Build AsmJIT
        if not os.path.exists("./Libs/libasmjit-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/asmjit"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libasmjit",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DASMJIT_STATIC=ON"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libasmjit.a", "./Libs/libasmjit-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'AsmJIT' Compiled Successfully.")

        # Build Curl
        if not os.path.exists("./Libs/libcurl-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/curl"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libcurl",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCURL_USE_OPENSSL=OFF",
                "-DCURL_USE_SCHANNEL=ON",
                "-DBUILD_LIBCURL_DOCS=OFF",
                "-DBUILD_MISC_DOCS=OFF",
                "-DCURL_USE_LIBPSL=OFF"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/lib/libcurl.a", "./Libs/libcurl-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'Curl' Compiled Successfully.")

        # Build LZMA
        if not os.path.exists("./Libs/liblzma-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/lzma"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake",
                "-S", "./Dependencies/liblzma",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/liblzma.a", "./Libs/liblzma-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'LZMA' Compiled Successfully.")

        # Build Archive
        if not os.path.exists("./Libs/libarchive-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/archive"
            lzmaInclude = os.path.abspath('./Dependencies/liblzma/src/liblzma/api')
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libarchive",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DENABLE_LZMA=ON",
                "-DHAVE_LZMA_H=ON",
                "-DBUILD_TESTING=OFF",
                "-DENABLE_TEST=OFF",
                "-DENABLE_LZ4=OFF",
                "-DENABLE_TAR=OFF",
                "-DENABLE_CAT=OFF",
                "-DENABLE_CPIO=OFF",
                "-DENABLE_EXPAT=OFF",
                "-DENABLE_BZip2=OFF",
                "-DENABLE_TEST=OFF",
                f"-DCMAKE_C_FLAGS={os.environ['CFLAGS'] + ' -DLZMA_API_STATIC ' + f'-I{guard_path(lzmaInclude)}'}"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libarchive/libarchive.a", "./Libs/libarchive-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'Archive' Compiled Successfully.")

        # Build XML2
        if not os.path.exists("./Libs/libxml2-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/xml2"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake",
                "-S", "./Dependencies/libxml2",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DLIBXML2_WITH_MODULES=OFF",
                "-DLIBXML2_WITH_PYTHON=OFF",
                "-DLIBXML2_WITH_TESTS=OFF",
                "-DLIBXML2_WITH_ICONV=OFF"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libxml2.a", "./Libs/libxml2-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'XML2' Compiled Successfully.")

        # Build ZLIB
        if not os.path.exists("./Libs/libzlib-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/zlib"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            os.makedirs(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libzlib",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DZLIB_BUILD_TESTING=OFF",
                "-DZLIB_BUILD_SHARED=OFF",
                "-DZLIB_BUILD_STATIC=ON"
            ], check=True)
            build_with_ninja(buildPath)
            shutil.copyfile(buildPath + "/libzs.a", "./Libs/libzlib-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'ZLIB' Compiled Successfully.")

        # Build TinyCC
        if not os.path.exists("./Libs/libtcc-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/tinycc"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            os.makedirs(buildPath, exist_ok=True)
            shutil.copytree("./Dependencies/libtinycc", buildPath, dirs_exist_ok=True)
            subprocess.run([
                "busybox.exe",
                "sh",
                "./configure",
                f"--cc={os.environ['C_COMPILER']}",
                "--config-mingw32",
                "--enable-static",
                "--cpu=x86_64"
            ], cwd=buildPath, check=True)
            subprocess.run(["mingw32-make.exe"], cwd=buildPath, env=os.environ.copy(), check=True)
            shutil.copyfile(os.path.join(buildPath, "libtcc.a"), "./Libs/libtcc-static-x86_64.a")
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'TinyCC' Compiled Successfully.")

        # Build GodotSDK
        if not os.path.exists("./Libs/libgodotcpp-static-x86_64.a"):
            buildPath = cacheDir + "/Dependencies/godotcpp"
            sdkPath = "./Libs/GodotSDK"
            if os.path.exists(buildPath): shutil.rmtree(buildPath)
            subprocess.run([
                "cmake.exe",
                "-S", "./Dependencies/libgodot",
                "-B", buildPath,
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DGODOTCPP_USE_HOT_RELOAD=ON",
                "-DGODOTCPP_TARGET=editor",
                f"-DGODOTCPP_PRECISION={'double' if double_precision else 'single'}",
                "-DBUILD_SHARED_LIBS=OFF"
            ], check=True)
            build_with_ninja(buildPath)
            binary_path = buildPath + f"/bin/libgodot-cpp.windows.editor{'.double' if double_precision else ''}.x86_64.a"
            shutil.copyfile(binary_path, "./Libs/libgodotcpp-static-x86_64.a")
            if os.path.exists(sdkPath): shutil.rmtree(sdkPath)
            os.makedirs(sdkPath, exist_ok=True)
            shutil.copyfile("./Dependencies/libgodot/gdextension/gdextension_interface.h", sdkPath + "/gdextension_interface.h")
            shutil.copytree("./Dependencies/libgodot/include", sdkPath, dirs_exist_ok=True)
            shutil.copytree(buildPath + "/gen/include", sdkPath, dirs_exist_ok=True)
            rgb_print("#38f227", "[ √ ] Jenova Runtime Dependency 'GodotSDK' Compiled Successfully.")
def build_windows(compilerBinary, linkerBinary, buildMode, buildSystem):

    # Verbose Build
    rgb_print("#4eb8ff", f"[ > ] Building Jenova Runtime for Windows using {buildSystem} Toolchain...")

    # Configuration
    compiler = compilerBinary
    linker = linkerBinary
    outputDir = "Win64"
    symbolFileName = "Jenova.Runtime.Win64.pdb"
    mapFileName = "Jenova.Runtime.Win64.map"
    resourceFileName = "Jenova.Runtime.rc"
    cacheDir = f"{outputDir}/Cache"
    sdkDir = f"{outputDir}/JenovaSDK"
    CacheDB = f"{cacheDir}/Build.db"

    # Create Output File
    outputName = "Jenova.Runtime.Win64.lib" if static_build else "Jenova.Runtime.Win64.dll"

    # Ensure Required Directories Exist
    rgb_print("#367fff", "[ ^ ] Validating Paths...")
    os.makedirs(outputDir, exist_ok=True)
    os.makedirs(sdkDir, exist_ok=True) 
    os.makedirs(cacheDir, exist_ok=True)

    # Initialize Toolchain
    initialize_toolchain_windows()

    # Build Dependencies
    if not skip_deps:
        rgb_print("#367fff", "[ ^ ] Building Dependencies...")
        build_dependencies_windows(buildMode, cacheDir)
    else:
        rgb_print("#367fff", "[ ^ ] Building Dependencies Skipped by User.")

    # Build Runtime Using MSVC & LLVM
    if buildMode == "win-msvc" or buildMode == "win-clangcl":

        # Verbose Build Start
        rgb_print("#367fff", "[ ^ ] Building Fully-Featured Jenova Runtime...")

        # Set Compiler/Linker Configuration
        os.environ["INCLUDE"] = f"{os.environ['TOOLCHAIN_PATH']}/x86_64-msvc/Include"
        os.environ["LIB"] = f"{os.environ['TOOLCHAIN_PATH']}/x86_64-msvc/Lib"
        os.environ["LIBPATH"] = f"{os.environ['TOOLCHAIN_PATH']}/x86_64-msvc/Lib"    

        # Dependencies
        libs = [
            "Libs/libgodotcpp-static-x86_64.lib",
            "Libs/libasmjit-static-x86_64.lib",
            "Libs/libzlib-static-x86_64.lib",
            "Libs/liblzma-static-x86_64.lib",
            "Libs/libarchive-static-x86_64.lib",
            "Libs/libcurl-static-x86_64.lib",
            "Libs/libtcc-static-x86_64.lib",
            "Libs/libpthread-static-x86_64.lib",
            "Libs/libxml2-static-x86_64.lib",
            "comctl32.lib",
            "Dbghelp.lib",
            "Ws2_32.lib",
            "Wldap32.lib"
        ]
        default_libs = [
            "comctl32.lib",
            "Dbghelp.lib",
            "Ws2_32.lib",
            "Wldap32.lib"
        ]

        # Add Default Libraries
        if not static_build: libs += default_libs

        # Add Extra Directories
        directories.append("Libs/Threading")

        # Add Compatibility Sources
        if buildMode == "win-clangcl":
            sources.append("Libs/Misc/clangcl-compatibility.cpp")

        # Generate Built-in JenovaSDK
        if static_build:
            jnvsdkHeader = os.path.join(cacheDir, "jenovaSDK_builtin.cpp")
            generate_jnvsdk(jnvsdkHeader)
            sources.append(jnvsdkHeader)

        # Load Cache
        rgb_print("#367fff", "[ ^ ] Loading Cache...")
        cache = load_cache(CacheDB)

        # Determine Changed Sources
        changedSources = [source for source in sources if should_compile(source, cache)]

        # Check If Compile Required
        if changedSources:

            # Generate Compiler Command
            rgb_print("#367fff", "[ ^ ] Generating Compiler Command...")
            if buildMode == "win-msvc":
                compile_command = (
                    f"{compiler} /c /MP /permissive /ifcOutput:\"{cacheDir}/\" /GS /GL /W0 /Gy /Zc:wchar_t "
                    f"{' '.join([f'/I{guard_path(directory)}' for directory in directories])} "
                    f"/Zi /Gm- /O2 /sdl /Fd\"{cacheDir}/vc143.pdb\" /Zc:inline /fp:precise "
                    f"{' '.join([f'/D{flag}' for flag in flags])} "
                    f"/errorReport:prompt /WX- /Zc:forScope /GR /Gd /Oi /MT /openmp /std:c++20 /FC "
                    f"/EHa /nologo /Fo\"{cacheDir}/\" /permissive /diagnostics:column "
                    f"{' '.join(changedSources)}"
                )
            if buildMode == "win-clangcl":
                compile_commands = {}
                for source in sources:
                    object_file = f"{cacheDir}/{os.path.basename(source).replace('.cpp', '.obj')}"
                    compile_command = (
                        f"{compiler} /c /permissive /GS /GL /W0 /Gy /Zc:wchar_t "
                        f"{' '.join([f'/I{guard_path(directory)}' for directory in directories])} "
                        f"/Zi /Gm- /O2 /sdl /Fd\"{cacheDir}/vc143.pdb\" /Zc:inline /fp:precise "
                        f"{' '.join([f'/D{flag}' for flag in flags])} "
                        f"/errorReport:prompt /WX- /Zc:forScope /GR /Gd /Oi /MT /openmp /std:c++20 /FC "
                        f"/EHa /nologo /Fo\"{cacheDir}/\" /permissive /diagnostics:column -Xclang -Wftime-trace "
                        f"{source}"
                    )
                    compile_commands[source] = compile_command

            # Compile Source Code
            rgb_print("#367fff", "[ ^ ] Compiling Jenova Runtime Source Code...")
            if buildMode == "win-msvc":
                run_compile_command(compile_command, "MSVC Compilation", compiler)
            if buildMode == "win-clangcl":
                with ThreadPoolExecutor(max_workers=len(sources)) as executor:
                    futures = []
                    for source, command in compile_commands.items():
                        if should_compile(source, cache):
                            futures.append(executor.submit(run_compile_command, command, source, compiler))
                            update_cache_entry(source, cache)
                        else:
                            rgb_print("#f59b42", f"[ ! ] Skipping Compilation for {os.path.basename(source)}, No Changes Detected.")
                    for future in futures: future.result()

            # Update Cache
            for source in sources: update_cache_entry(source, cache)

            # Save Cache
            rgb_print("#367fff", "[ ^ ] Saving Cache...")
            save_cache(cache, CacheDB)

            # Generate Object Files List
            object_files = []
            for source in sources:
                object_file = f"{cacheDir}/{os.path.basename(source).replace('.cpp', '.obj')}"
                object_files.append(object_file)

            # Compile Resource File
            if not static_build:
                rgb_print("#367fff", "[ ^ ] Generating Resource Compiler Command...")
                compiled_resource_file = f"{cacheDir}/Jenova.Runtime.res"
                if buildMode == "win-msvc": resource_compiler = "rc.exe"
                if buildMode == "win-clangcl": resource_compiler = "llvm-rc.exe"
                compile_resource_command = (
                    f"{resource_compiler} -D_UNICODE -DUNICODE -c65001 /NOLOGO /fo "
                    f"\"{compiled_resource_file}\" \"{resourceFileName}\""
                )
                run_compile_command(compile_resource_command, "Jenova.Runtime.rc", resource_compiler)

            # Generate Linker Command
            rgb_print("#367fff", "[ ^ ] Generating Linker Command...")
            if static_build:
                    linker = "lib.exe"
                    global skip_packaging
                    skip_packaging = True
                    link_command = (
                        f"{linker} /OUT:\"{outputDir}/{outputName}\" "
                        f"{' '.join(object_files)} "
                        f"{' '.join(libs)} "
                        f"/NOLOGO /IGNORE:4006"
                    )
            else:
                link_command = (
                    f"{linker} /OUT:\"{outputDir}/{outputName}\" /MANIFEST /LTCG:incremental /NXCOMPAT "
                    f"/PDB:\"{outputDir}/{symbolFileName}\" /DYNAMICBASE "
                    f"{' '.join(object_files)} "
                    f"{' '.join(libs)} "
                    f"\"{compiled_resource_file}\" "
                    f"comctl32.lib Dbghelp.lib Ws2_32.lib Wldap32.lib kernel32.lib user32.lib Crypt32.lib "
                    f"gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib "
                    f"bcrypt.lib uuid.lib odbc32.lib odbccp32.lib delayimp.lib "
                    f"/DEBUG:FULL /DLL /MACHINE:X64 /OPT:REF /INCREMENTAL:NO "
                    f"/SUBSYSTEM:CONSOLE /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" "
                    f"/ManifestFile:\"{cacheDir}/{outputName}.intermediate.manifest\" "
                    f"/LTCGOUT:\"{cacheDir}/{outputName}.iobj\" /OPT:ICF /ERRORREPORT:PROMPT "
                    f"/ILK:\"{cacheDir}/{outputName}.ilk\" /NOLOGO /IMPLIB:\"{sdkDir}/Jenova.SDK.x64.lib\" "
                    f"/DELAYLOAD:\"dbghelp.dll\" /DELAYLOAD:\"Wldap32.dll\" "
                    f"/DELAYLOAD:\"bcrypt.dll\" /DELAYLOAD:\"Crypt32.dll\" "
                    f"/TLBID:1 /IGNORE:4098 /IGNORE:4286 /IGNORE:4099 "
                )

            # Link Object Files
            rgb_print("#367fff", "[ ^ ] Linking Jenova Runtime Binary...")
            run_linker_command(link_command, linker)
        else:
            rgb_print("#f59b42", "[ ! ] No Changes Detected, Skipping Compilation.")
    
    # Build Runtime Using Clang & GNU
    if buildMode == "win-clang" or buildMode == "win-gcc":
        
        # Verbose Build Start
        rgb_print("#367fff", "[ ^ ] Building Jenova Runtime...")

        # Dependencies
        libs = [
            "Libs/libzlib-static-x86_64.a",
            "Libs/libtcc-static-x86_64.a",
            "Libs/libgodotcpp-static-x86_64.a",
            "Libs/libcurl-static-x86_64.a",
            "Libs/libasmjit-static-x86_64.a",
            "Libs/liblzma-static-x86_64.a",
            "Libs/libarchive-static-x86_64.a",
            "Libs/libxml2-static-x86_64.a"
        ]

        # Load Cache
        rgb_print("#367fff", "[ ^ ] Loading Cache...")
        cache = load_cache(CacheDB)

        # Generate Compile Commands
        rgb_print("#367fff", "[ ^ ] Generating Compiler Commands...")
        compile_commands = {}
        object_files = []
        if buildMode == "win-clang":
            for source in sources:
                object_file = f"{cacheDir}/{os.path.basename(source).replace('.cpp', '.o')}"
                object_files.append(object_file)
                compile_command = (
                    f"{compiler} -m64 -O3 -g -gcodeview -fno-use-linker-plugin "
                    "-pipe -w -std=c++20 -pthread -fexceptions -femulated-tls "
                    f"{' '.join([f'-D{flag}' for flag in flags])} "
                    f"{' '.join([f'-I{directory}' for directory in directories])} "
                    f"-c {source} -o {object_file}"
                )
                compile_commands[source] = compile_command
        if buildMode == "win-gcc":
            for source in sources:
                object_file = f"{cacheDir}/{os.path.basename(source).replace('.cpp', '.o')}"
                object_files.append(object_file)
                compile_command = (
                    f"{compiler} -m64 -O3 -g -gcodeview -fno-use-linker-plugin "
                    "-pipe -w -std=c++20 -pthread -fexceptions "
                    f"{' '.join([f'-D{flag}' for flag in flags])} "
                    f"{' '.join([f'-I{directory}' for directory in directories])} "
                    f"-c {source} -o {object_file}"
                )
                compile_commands[source] = compile_command

        # Compile Source Code
        rgb_print("#367fff", "[ ^ ] Compiling Jenova Runtime Source Code...")
        with ThreadPoolExecutor(max_workers=len(sources)) as executor:
            futures = []
            for source, command in compile_commands.items():
                if should_compile(source, cache):
                    futures.append(executor.submit(run_compile_command, command, source, compiler))
                    update_cache_entry(source, cache)
                else:
                    rgb_print("#f59b42", f"[ ! ] Skipping Compilation for {os.path.basename(source)}, No Changes Detected.")
            for future in futures: future.result()

        # Save Cache
        rgb_print("#367fff", "[ ^ ] Saving Cache...")
        save_cache(cache, CacheDB)

        # Compile Resource File
        if not static_build:
            rgb_print("#367fff", "[ ^ ] Generating Resource Compiler Command...")
            compiled_resource_file = f"{cacheDir}/Jenova.Runtime.res"
            resource_compiler = "windres.exe"
            compile_resource_command = (
                f"{resource_compiler} -D_UNICODE -DUNICODE -c65001 "
                f"-o \"{compiled_resource_file}\" -i\"{resourceFileName}\""
            )
            run_compile_command(compile_resource_command, "Jenova.Runtime.rc", resource_compiler)

        # Generate Linker Command
        rgb_print("#367fff", "[ ^ ] Generating Linker Command...")
        if buildMode == "win-clang":
            link_command = (
                f"{linker} {'-static' if static_build else '-shared'} -static-libstdc++ -static-libgcc -fuse-ld=lld "
                f"{' '.join(object_files)} -o {outputDir}/{outputName} -m64 "
                f"{' '.join(libs)} -lws2_32 -lwinhttp -ldbghelp -lbcrypt -lcrypt32 -lwldap32" + " "
                "-delayimp -Wl,-Bstatic -lpthread" + " "
                "-Xlinker /DELAYLOAD:bcrypt.dll -Xlinker /DELAYLOAD:lcrypt32.dll "
                "-Xlinker /DELAYLOAD:wldap32.dll -Xlinker /DELAYLOAD:dbghelp.dll "
                f"-Wl,-Map=\"{outputDir}/{mapFileName}\" "
                f"-Wl,--pdb=\"{outputDir}/{symbolFileName}\" "
                f"\"{compiled_resource_file}\" "
                f"-Wl,--out-implib,\"{sdkDir}/Jenova.SDK.x64.a\" "
            )
        if buildMode == "win-gcc":
            link_command = (
                f"{linker} {'-static' if static_build else '-shared'} -static-libstdc++ -static-libgcc -fuse-ld=lld "
                f"{' '.join(object_files)} -o {outputDir}/{outputName} -m64 -flto -Os -s "
                f"{' '.join(libs)} -lws2_32 -lwinhttp -ldbghelp -lbcrypt -lcrypt32 -lwldap32" + " "
                "-delayimp -Wl,-Bstatic -lpthread" + " "
                "-Xlinker /DELAYLOAD:bcrypt.dll -Xlinker /DELAYLOAD:lcrypt32.dll "
                "-Xlinker /DELAYLOAD:wldap32.dll -Xlinker /DELAYLOAD:dbghelp.dll "
                f"-Wl,-Map=\"{outputDir}/{mapFileName}\" "
                f"-Wl,--pdb=\"{outputDir}/{symbolFileName}\" "
                f"\"{compiled_resource_file}\" "
                f"-Wl,--out-implib,\"{sdkDir}/Jenova.SDK.x64.a\" "
            )

        # Link Object Files
        rgb_print("#367fff", "[ ^ ] Linking Jenova Runtime Binary...")
        run_linker_command(link_command, linker)

    # Prepare Release
    open(f"{sdkDir}/.gitignore", "w").write("*")
    shutil.copy2("./Source/JenovaSDK.h", f"{sdkDir}/JenovaSDK.h")
    shutil.copy2("./Jenova.Runtime.gdextension", f"{outputDir}/Jenova.Runtime.gdextension")

    # Generate Godot SDK Package
    if args.generate_gdsdk: generate_gdsdk(outputDir)

    # Create Package
    if skip_packaging: return
    packageFiles = [
        {"src": f"{outputDir}/{outputName}", "dst": "./Jenova"},
        {"src": f"{outputDir}/{symbolFileName}", "dst": "./Jenova"},
        {"src": f"{outputDir}/Jenova.Runtime.gdextension", "dst": "./Jenova"},
        {"src": f"{sdkDir}/JenovaSDK.h", "dst": "./Jenova/JenovaSDK"},
        {"src": f"{sdkDir}/Jenova.SDK.x64.a", "dst": "./Jenova/JenovaSDK"},
        {"src": f"{sdkDir}/Jenova.SDK.x64.lib", "dst": "./Jenova/JenovaSDK"},
        {"src": f"{sdkDir}/.gitignore", "dst": "./Jenova/JenovaSDK"}
    ]
    os.makedirs(f"{outputDir}/Distribution", exist_ok=True)
    toolchainName = get_toolchain_name(buildMode)
    create_distribution_package(packageFiles, f"{outputDir}/Distribution/Jenova-Framework-Win64-{toolchainName}.7z")

# Entrypoint
if __name__ == "__main__":
    # Disable PyCache
    sys.dont_write_bytecode = True
    os.environ['PYTHONDONTWRITEBYTECODE'] = "1"

    # Create Arguments Parser
    parser = argparse.ArgumentParser(description=f"Jenova Runtime Build System {builder_version} Developed by Hamid.Memar")
    parser.add_argument('--compiler', type=str, help='Specify Compiler to Use.')
    parser.add_argument('--deploy-mode', action='store_true', help='Run As GitHub Action Deploy Mode')
    parser.add_argument('--deps-version', default="4.5", help='Specify Dependencies Version (default: 4.5)')
    parser.add_argument('--double-precision', action='store_true', help='Build Jenova Runtime using Double Precision')  
    parser.add_argument('--static-build', action='store_true', help='Build Jenova Runtime as Static Library') 
    parser.add_argument('--skip-banner', action='store_true', help='Skip Printing Banner')
    parser.add_argument('--skip-deps', action='store_true', help='Skip Building Dependencies')
    parser.add_argument('--skip-cache', action='store_true', help='Skip Source Caching')
    parser.add_argument('--skip-packaging', action='store_true', help='Skip Creating Distribution Package')
    parser.add_argument('--clean-up', action='store_true', help='Clean Up Build Files')
    parser.add_argument('--deep-clean-up', action='store_true', help='Clean Up Everything')
    parser.add_argument('--generate-gdsdk', action='store_true', help='Generate GodotSDK Package')
    parser.add_argument('--lithium-edition', action='store_true', help='Build Jenova Runtime foR Lithium IDE')

    # Parser Arguments
    args = parser.parse_args()

    # Print Banner
    if not args.skip_banner: print_banner()

    # Set Dependencies Version
    deps_version = args.deps_version

    # Enable Double Precision Mode
    if args.double_precision:
        double_precision = True
        flags.append("REAL_T_IS_DOUBLE")

    # Enable Static Build Mode
    if args.static_build:
        static_build = True
        flags.append("JENOVA_STATIC_BUILD")

    # Enable Deploy Mode
    if args.deploy_mode: deploy_mode = True

    # Skip Building Dependencies
    if args.skip_deps: skip_deps = True

    # Skip Source Caching
    if args.skip_cache: skip_cache = True

    # Skip Source Caching
    if args.skip_packaging: skip_packaging = True   

    # Clean Up
    if args.clean_up:
        rgb_print("#367fff", "[ ^ ] Cleaning Up Build...")
        clean_up_build(False)

    # Deep Clean Up
    if args.deep_clean_up:
        rgb_print("#367fff", "[ ^ ] Cleaning Up Everything...")
        clean_up_build(True)

    # Handle Lithium Edition
    if args.lithium_edition:
        lithium_edition = True
        flags.append("LITHIUM_EDITION")

    # Set Compiler And Start Build
    start_time = time.time()
    if args.compiler:
        if args.compiler == "win-msvc":
            build_windows("cl.exe", "link.exe", "win-msvc", "Microsoft Visual C++")
        elif args.compiler == "win-clangcl":
            build_windows("clang-cl.exe", "lld-link.exe", "win-clangcl", "Microsoft Visual C++ LLVM")      
        elif args.compiler == "win-clang":
            build_windows("clang++.exe", "clang++.exe", "win-clang", "LLVM Clang")
        elif args.compiler == "win-gcc":
            build_windows("g++.exe", "g++.exe", "win-gcc", "Minimalist GNU")
        elif args.compiler == "linux-clang":
            build_linux("clang++", "clang++", "linux-clang", "LLVM Clang")
        elif args.compiler == "linux-gcc":
            build_linux("g++", "g++", "linux-gcc", "GNU Compiler Collection")
        else:
            rgb_print("#e02626", "[ x ] Error : Invalid Input.")
            exit(-1)
    else:
        # Ask for Compiler
        get_compiler_choice()

    # Verbose Success
    build_time = time.time() - start_time
    rgb_print("#03fc6f", f"[ √ ] Jenova Runtime has been Successfully Compiled and Built in {format_duration(build_time)}.")
