/*
  编译器信息采集器（单文件，可直接编译运行）
  目标：
    1) 尽可能完整、细致地打印当前使用的 C++ 编译器及其环境的相关信息
    2) 覆盖主流编译器：Clang/Apple Clang、GCC、MSVC、Intel、NVHPC/PGI、Emscripten、MinGW 等
    3) 同时打印：
       - 编译器名称/厂商/版本（多种版本宏）
       - C++ 标准与特性宏（语言/库）
       - 目标平台（操作系统、CPU 架构、位宽、字节序）
       - 标准库与 C 库实现
       - 构建配置（调试/发布、优化级别、异常/RTTI、Sanitizer等）
       - 常见宏与构建时间
  说明：
    - 不同编译器/平台宏不完全一致，代码中大量使用条件编译进行探测。
    - 某些宏需要包含对应头文件或 <version> 头才会出现；本文件会尽可能有条件地包含它们。
    - 部分信息为“最佳努力”，可能无法 100% 精确识别（例如 VS 具体小版本、libc++ 版本精确号等）。

  使用示例（任选其一）：
    g++ -std=c++17 -O2 -Wall -Wextra print_compiler_info.cpp && ./a.out
    clang++ -std=c++20 -O2 print_compiler_info.cpp && ./a.out
    cl /std:c++20 /EHsc /O2 print_compiler_info.cpp && print_compiler_info.exe
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdint>
#include <climits>
#include <type_traits>
#include <cstdlib>

// —— 条件包含：C++20 的 <version> 与 <bit>（用于特性宏与字节序）——
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#    define HAS_HEADER_VERSION 1
#  endif
#  if __has_include(<bit>)
#    include <bit>
#    define HAS_HEADER_BIT 1
#  endif
#  if __has_include(<thread>)
#    include <thread>
#    define HAS_HEADER_THREAD 1
#  endif
#  if __has_include(<TargetConditionals.h>)
#    include <TargetConditionals.h>
#    define HAS_TARGET_CONDITIONALS 1
#  endif
#endif

// —— 为 Clang 专属的 __has_feature / __has_cpp_attribute 提供安全降级 ——
// 避免在非 Clang 编译器上报错
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#ifndef __has_cpp_attribute
#  define __has_cpp_attribute(x) 0
#endif

// —— 字符串化小工具宏 —— 
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// —— 小工具：布尔值 → “是/否” —— 
static const char* yesno(bool v) { return v ? "是" : "否"; }

// —— 推断 C++ 标准名称（基于 __cplusplus 或 _MSVC_LANG）——
static const char* cxx_standard_name(long v) {
    // 参考数值：199711L(C++98/03), 201103L(C++11), 201402L(C++14), 201703L(C++17),
    // 202002L(C++20), 202302L(C++23), 202600L/3?(C++26/草案，编译器各异)
    if      (v >= 202600L) return "C++26或更新/草案";
    else if (v >= 202302L) return "C++23";
    else if (v >= 202002L) return "C++20";
    else if (v >= 201703L) return "C++17";
    else if (v >= 201402L) return "C++14";
    else if (v >= 201103L) return "C++11";
    else if (v >= 199711L) return "C++98/03";
    else return "（未知/非常古老的标准）";
}

// —— MSVC 版本号近似映射 —— 
static std::string msvc_pretty_from_ver(int msc_ver) {
    // 仅粗略映射主版本线
    if      (msc_ver >= 1930) return "Visual Studio 2022 (v17)";
    else if (msc_ver >= 1920) return "Visual Studio 2019 (v16)";
    else if (msc_ver >= 1910) return "Visual Studio 2017 (v15)";
    else if (msc_ver >= 1900) return "Visual Studio 2015 (v14)";
    else if (msc_ver >= 1800) return "Visual Studio 2013 (v12)";
    else if (msc_ver >= 1700) return "Visual Studio 2012 (v11)";
    else if (msc_ver >= 1600) return "Visual Studio 2010 (v10)";
    else if (msc_ver >= 1500) return "Visual Studio 2008 (v9)";
    else if (msc_ver >= 1400) return "Visual Studio 2005 (v8)";
    else if (msc_ver >= 1310) return "Visual Studio .NET 2003 (v7.1)";
    else if (msc_ver >= 1300) return "Visual Studio .NET 2002 (v7.0)";
    else if (msc_ver >= 1200) return "Visual Studio 6.0";
    else return "非常古老的 MSVC";
}

// —— 编译器名称/厂商/版本探测 —— 
static std::string detect_compiler_name() {
    std::ostringstream oss;
#if defined(__clang__)
    // Clang 家族（包含 Apple Clang、clang-cl、Intel LLVM 前端等）
#  if defined(__apple_build_version__)
    oss << "Apple Clang";
#  elif defined(_MSC_VER)
    oss << "LLVM Clang (clang-cl, MSVC 前端)";
#  elif defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_CLANG_COMPILER)
    oss << "Intel oneAPI (LLVM/Clang 前端)";
#  else
    oss << "LLVM Clang";
#  endif
#elif defined(_MSC_VER)
    oss << "Microsoft Visual C++ (MSVC)";
#elif defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
    oss << "Intel C++ Compiler (ICC/Classic)";
#elif defined(__NVCOMPILER) || defined(__PGI)
    oss << "NVIDIA HPC SDK (NVHPC/PGI)";
#elif defined(__GNUC__)
    // 注意：Clang 也会定义 __GNUC__，因此应在上面先判断 __clang__
    oss << "GNU Compiler Collection (GCC)";
#else
    oss << "未知或非主流编译器";
#endif
    return oss.str();
}

static std::string detect_compiler_version() {
    std::ostringstream oss;
#if defined(__clang__)
    // Clang 提供 __clang_major__/__clang_minor__/__clang_patchlevel__ 和 __clang_version__
    oss << "Clang 版本: "
#  if defined(__clang_major__)
        << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#  else
        << "(未知)";
#  endif
    oss << "\n";
#  if defined(__clang_version__)
    oss << "完整版本串: " << __clang_version__ << "\n";
#  endif
#  if defined(__apple_build_version__)
    oss << "Apple Build Version: " << __apple_build_version__ << "\n";
#  endif
#  if defined(_MSC_VER)
    oss << "（clang-cl 兼容 MSVC，_MSC_VER=" << _MSC_VER;
#    if defined(_MSC_FULL_VER)
    oss << ", _MSC_FULL_VER=" << _MSC_FULL_VER;
#    endif
#    if defined(_MSC_BUILD)
    oss << ", _MSC_BUILD=" << _MSC_BUILD;
#    endif
    oss << "）\n";
#  endif
#  if defined(__INTEL_LLVM_COMPILER)
    oss << "Intel LLVM 前端版本宏 __INTEL_LLVM_COMPILER=" << __INTEL_LLVM_COMPILER << "\n";
#  endif
#elif defined(_MSC_VER)
    oss << "_MSC_VER=" << _MSC_VER;
#  if defined(_MSC_FULL_VER)
    oss << ", _MSC_FULL_VER=" << _MSC_FULL_VER;
#  endif
#  if defined(_MSC_BUILD)
    oss << ", _MSC_BUILD=" << _MSC_BUILD;
#  endif
    oss << " → " << msvc_pretty_from_ver(_MSC_VER) << "\n";
#  if defined(_MSVC_LANG)
    oss << "_MSVC_LANG=" << _MSVC_LANG << " (" << cxx_standard_name(_MSVC_LANG) << ")\n";
#  endif
#elif defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
    oss << "__INTEL_COMPILER=" << __INTEL_COMPILER;
#  if defined(__INTEL_COMPILER_UPDATE)
    oss << " update=" << __INTEL_COMPILER_UPDATE;
#  endif
    oss << "\n";
#elif defined(__NVCOMPILER) || defined(__PGI)
#  if defined(__NVCOMPILER)
    oss << "__NVCOMPILER=" << __NVCOMPILER;
#    if defined(__NVCOMPILER_MAJOR)
    oss << " (major=" << __NVCOMPILER_MAJOR << ")";
#    endif
#    if defined(__NVCOMPILER_MINOR)
    oss << " (minor=" << __NVCOMPILER_MINOR << ")";
#    endif
    oss << "\n";
#  endif
#  if defined(__PGI)
    oss << "__PGI=" << __PGI << "\n";
#  endif
#elif defined(__GNUC__)
    oss << "GCC 版本: ";
#  if defined(__GNUC__)
    oss << __GNUC__;
#    if defined(__GNUC_MINOR__)
    oss << "." << __GNUC_MINOR__;
#    else
    oss << ".?";
#    endif
#    if defined(__GNUC_PATCHLEVEL__)
    oss << "." << __GNUC_PATCHLEVEL__;
#    endif
#  else
    oss << "(未知)";
#  endif
    oss << "\n";
#else
    oss << "(未能识别版本宏)\n";
#endif

    // __VERSION__：GCC/Clang 常见的完整版本串
#if defined(__VERSION__)
    oss << "__VERSION__: " << __VERSION__ << "\n";
#endif
    return oss.str();
}

// —— 操作系统探测 —— 
static std::string detect_os() {
    std::ostringstream oss;
#if defined(__EMSCRIPTEN__)
    oss << "WebAssembly (Emscripten)";
#elif defined(_WIN32)
    oss << "Windows";
#  if defined(_WIN64)
    oss << " 64位";
#  else
    oss << " 32位";
#  endif
#elif defined(__APPLE__) && defined(__MACH__)
    oss << "Apple Darwin (macOS / iOS / tvOS / watchOS)";
#  if defined(HAS_TARGET_CONDITIONALS)
    // 需要 <TargetConditionals.h>
#    if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    oss << "（iOS 家族）";
#    elif defined(TARGET_OS_OSX) && TARGET_OS_OSX
    oss << "（macOS）";
#    endif
#  endif
#elif defined(__ANDROID__)
    oss << "Android (Linux 内核)";
#elif defined(__linux__)
    oss << "Linux";
#elif defined(__CYGWIN__)
    oss << "Cygwin (POSIX on Windows)";
#elif defined(__unix__) || defined(__unix)
    oss << "Unix (泛指, 具体未细分)";
#else
    oss << "未知/嵌入式/非主流操作系统";
#endif

    // BSD 系列补充
#if defined(__FreeBSD__)
    oss << " [FreeBSD]";
#endif
#if defined(__NetBSD__)
    oss << " [NetBSD]";
#endif
#if defined(__OpenBSD__)
    oss << " [OpenBSD]";
#endif
#if defined(__DragonFly__)
    oss << " [DragonFlyBSD]";
#endif
    return oss.str();
}

// —— CPU 架构探测 —— 
static std::string detect_arch() {
    std::ostringstream oss;
#if defined(__x86_64__) || defined(_M_X64)
    oss << "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    oss << "x86 (32位)";
#elif defined(__aarch64__) || defined(_M_ARM64)
    oss << "ARM64 (AArch64)";
#elif defined(__arm__) || defined(_M_ARM)
    oss << "ARM (32位)";
#elif defined(__riscv) || defined(__riscv__)
#  if defined(__riscv_xlen) && __riscv_xlen == 64
    oss << "RISC-V 64";
#  else
    oss << "RISC-V";
#  endif
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__)
    oss << "PowerPC 64";
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    oss << "PowerPC 32";
#elif defined(__s390x__)
    oss << "s390x";
#elif defined(__mips64)
    oss << "MIPS64";
#elif defined(__mips__)
    oss << "MIPS";
#elif defined(__wasm64__)
    oss << "WebAssembly 64";
#elif defined(__wasm32__)
    oss << "WebAssembly 32";
#else
    oss << "未知架构";
#endif
    return oss.str();
}

// —— 位宽（指针大小） —— 
static int detect_pointer_bits() {
    return static_cast<int>(sizeof(void*) * CHAR_BIT);
}

// —— 字节序探测 —— 
static const char* detect_endianness() {
#if defined(HAS_HEADER_BIT) && defined(__cpp_lib_endian)
    // C++20 提供 std::endian
    if (std::endian::native == std::endian::little) return "小端";
    if (std::endian::native == std::endian::big)    return "大端";
    if (std::endian::native == std::endian::mixed)  return "混合端序";
    return "未知";
#else
    // 常见编译器/平台宏
#  if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return "小端";
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return "大端";
#    else
    return "混合端序/未知";
#    endif
#  elif defined(_WIN32) || defined(__LITTLE_ENDIAN__) || defined(__x86_64__) || defined(__i386__) || defined(__aarch64__) || defined(__wasm__)
    return "小端";
#  else
    return "未知";
#  endif
#endif
}

// —— 标准库实现探测 —— 
static std::string detect_cpp_stdlib() {
    std::ostringstream oss;
#if defined(_LIBCPP_VERSION)
    oss << "libc++ (_LIBCPP_VERSION=" << _LIBCPP_VERSION << ")";
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
    // libstdc++ 使用 __GLIBCXX__（日期数值）或 __GLIBCPP__
#  if defined(__GLIBCXX__)
    oss << "libstdc++ (版本日期宏 __GLIBCXX__=" << __GLIBCXX__ << ")";
#  else
    oss << "libstdc++ (旧版宏 __GLIBCPP__ 定义)";
#  endif
#elif defined(_MSVC_STL_VERSION) || defined(_CPPLIB_VER)
    // MSVC STL（Dinkumware）
#  if defined(_MSVC_STL_VERSION)
    oss << "MSVC STL (_MSVC_STL_VERSION=" << _MSVC_STL_VERSION << ")";
#  elif defined(_CPPLIB_VER)
    oss << "Dinkumware/MSVC STL (_CPPLIB_VER=" << _CPPLIB_VER << ")";
#  endif
#else
    oss << "未知/定制 C++ 标准库实现";
#endif
    return oss.str();
}

// —— C 标准库实现探测（glibc/musl/bionic/newlib/uClibc等） —— 
static std::string detect_c_libc() {
    std::ostringstream oss;
#if defined(__GLIBC__)
    oss << "glibc " << __GLIBC__ << "." << __GLIBC_MINOR__;
    // _FORTIFY_SOURCE 是 glibc 加固宏
#  if defined(_FORTIFY_SOURCE)
    oss << " (FORTIFY_SOURCE=" << _FORTIFY_SOURCE << ")";
#  endif
#elif defined(__BIONIC__)
    oss << "Bionic (Android C 库)";
#elif defined(__UCLIBC__)
    oss << "uClibc";
#elif defined(__NEWLIB__)
    oss << "newlib";
#elif defined(__MSVCRT__) || defined(_UCRT)
    // Windows 上的 MSVCRT / UCRT
#  if defined(_UCRT)
    oss << "UCRT (Windows Universal C Runtime)";
#  else
    oss << "MSVCRT (Microsoft C Runtime)";
#  endif
#else
    oss << "未知/静态或嵌入式 C 运行时";
#endif
    return oss.str();
}

// —— 是否为 MinGW/Cygwin/Emscripten 等工具链层 —— 
static std::string detect_toolchain_flavor() {
    std::ostringstream oss;
    bool any = false;
#if defined(__MINGW64__)
    any = true; oss << "MinGW-w64";
#  if defined(__MINGW64_VERSION_MAJOR)
    oss << " " << __MINGW64_VERSION_MAJOR << "." << __MINGW64_VERSION_MINOR;
#  endif
    oss << "; ";
#elif defined(__MINGW32__)
    any = true; oss << "MinGW32; ";
#endif
#if defined(__CYGWIN__)
    any = true; oss << "Cygwin; ";
#endif
#if defined(__EMSCRIPTEN__)
    any = true; oss << "Emscripten";
#  if defined(__EMSCRIPTEN_major__)
    oss << " " << __EMSCRIPTEN_major__ << "." << __EMSCRIPTEN_minor__ << "." << __EMSCRIPTEN_tiny__;
#  endif
    oss << "; ";
#endif
    if (!any) oss << "（无特别工具链标识或不适用）";
    return oss.str();
}

// —— 构建配置（调试/优化/异常/RTTI/Sanitizer 等） —— 
static std::string detect_build_config() {
    std::ostringstream oss;

    // 调试/发布开关
#if defined(_DEBUG)
    oss << "调试宏 _DEBUG: 已定义\n";
#endif
#if defined(NDEBUG)
    oss << "发布宏 NDEBUG: 已定义（断言通常被禁用）\n";
#endif

    // 优化相关（GCC/Clang）
#if defined(__OPTIMIZE__)
    oss << "优化：__OPTIMIZE__=1（-O1/-O2/-O3/…）\n";
#endif
#if defined(__OPTIMIZE_SIZE__)
    oss << "优化：__OPTIMIZE_SIZE__=1（-Os 或 -Oz）\n";
#endif
#if defined(__NO_INLINE__)
    oss << "内联：__NO_INLINE__=1（禁止内联）\n";
#endif

    // 位置无关代码/可执行文件
#if defined(__PIC__)
    oss << "位置无关代码：__PIC__ 已定义\n";
#endif
#if defined(__PIE__)
    oss << "位置无关可执行文件：__PIE__ 已定义\n";
#endif

    // 异常开关
    bool exceptions_on =
#if __has_feature(cxx_exceptions)
        true;
#elif defined(__EXCEPTIONS)
        true;
#elif defined(_CPPUNWIND)
        true;
#elif defined(_HAS_EXCEPTIONS) && _HAS_EXCEPTIONS
        true;
#else
        false;
#endif
    oss << "C++ 异常支持： " << yesno(exceptions_on) << "\n";

    // RTTI 开关
    bool rtti_on =
#if __has_feature(cxx_rtti)
        true;
#elif defined(__GXX_RTTI)
        true;
#elif defined(_CPPRTTI)
        true;
#else
        false;
#endif
    oss << "RTTI（运行时类型信息）： " << yesno(rtti_on) << "\n";

    // Sanitizer 探测（Clang/GCC 部分支持）
    bool asan =
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
        true;
#else
        false;
#endif
    bool tsan =
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
        true;
#else
        false;
#endif
    bool msan =
#if __has_feature(memory_sanitizer)
        true;
#else
        false;
#endif
    bool ubsan =
#if __has_feature(undefined_behavior_sanitizer)
        true;
#else
        false;
#endif
    bool lsan =
#if __has_feature(leak_sanitizer)
        true;
#else
        false;
#endif
    oss << "Sanitizers：ASan=" << yesno(asan)
        << ", TSan=" << yesno(tsan)
        << ", MSan=" << yesno(msan)
        << ", UBSan=" << yesno(ubsan)
        << ", LSan=" << yesno(lsan) << "\n";

    // MSVC STL 迭代器调试级别
#if defined(_ITERATOR_DEBUG_LEVEL)
    oss << "MSVC _ITERATOR_DEBUG_LEVEL=" << _ITERATOR_DEBUG_LEVEL << "\n";
#endif

    // GNU libstdc++ 断言
#if defined(_GLIBCXX_ASSERTIONS)
    oss << "libstdc++ 运行时断言：已启用 (_GLIBCXX_ASSERTIONS)\n";
#endif

    return oss.str();
}

// —— C++ 标准与特性宏打印 —— 
static std::string detect_cxx_features() {
    std::ostringstream oss;

    // C++ 标准宏
#if defined(_MSVC_LANG)
    long std_macro = static_cast<long>(_MSVC_LANG); // MSVC 使用 _MSVC_LANG 表示活动标准
    oss << "活动标准（MSVC）：_MSVC_LANG=" << std_macro << " → " << cxx_standard_name(std_macro) << "\n";
#endif
    oss << "__cplusplus=" << static_cast<long>(__cplusplus) << " → " << cxx_standard_name(__cplusplus) << "\n";

    // 常见语言特性（__cpp_*）—— 仅在编译器/标准支持时会定义
    // 注：并非所有编译器都会定义完整的 feature-test 宏，以下为常见子集。
#if defined(__cpp_constexpr)
    oss << "__cpp_constexpr=" << __cpp_constexpr << "（constexpr 支持）\n";
#else
    oss << "__cpp_constexpr 未定义\n";
#endif

#if defined(__cpp_if_constexpr)
    oss << "__cpp_if_constexpr=" << __cpp_if_constexpr << "（if constexpr）\n";
#endif

#if defined(__cpp_concepts)
    oss << "__cpp_concepts=" << __cpp_concepts << "（概念）\n";
#endif

#if defined(__cpp_modules)
    oss << "__cpp_modules=" << __cpp_modules << "（模块）\n";
#endif

#if defined(__cpp_coroutines)
    oss << "__cpp_coroutines=" << __cpp_coroutines << "（协程语言支持）\n";
#endif

#if defined(__cpp_generic_lambdas)
    oss << "__cpp_generic_lambdas=" << __cpp_generic_lambdas << "（泛型 Lambda）\n";
#endif

#if defined(__cpp_fold_expressions)
    oss << "__cpp_fold_expressions=" << __cpp_fold_expressions << "（折叠表达式）\n";
#endif

#if defined(__cpp_structured_bindings)
    oss << "__cpp_structured_bindings=" << __cpp_structured_bindings << "（结构化绑定）\n";
#endif

#if defined(__cpp_consteval)
    oss << "__cpp_consteval=" << __cpp_consteval << "（consteval）\n";
#endif

#if defined(__cpp_nontype_template_args)
    oss << "__cpp_nontype_template_args=" << __cpp_nontype_template_args << "（非类型模板参数增强）\n";
#endif

#if defined(__cpp_designated_initializers)
    oss << "__cpp_designated_initializers=" << __cpp_designated_initializers << "（指定初始化器）\n";
#endif

    // C++ 标准库特性宏（需要 <version> 或相关头）
#if defined(HAS_HEADER_VERSION) || defined(__cpp_lib_ranges) || defined(__cpp_lib_format)
    oss << "—— C++ 标准库特性（__cpp_lib_*）——\n";
#  if defined(__cpp_lib_string_view)
    oss << "__cpp_lib_string_view=" << __cpp_lib_string_view << "\n";
#  endif
#  if defined(__cpp_lib_optional)
    oss << "__cpp_lib_optional=" << __cpp_lib_optional << "\n";
#  endif
#  if defined(__cpp_lib_variant)
    oss << "__cpp_lib_variant=" << __cpp_lib_variant << "\n";
#  endif
#  if defined(__cpp_lib_filesystem)
    oss << "__cpp_lib_filesystem=" << __cpp_lib_filesystem << "\n";
#  endif
#  if defined(__cpp_lib_ranges)
    oss << "__cpp_lib_ranges=" << __cpp_lib_ranges << "\n";
#  endif
#  if defined(__cpp_lib_format)
    oss << "__cpp_lib_format=" << __cpp_lib_format << "\n";
#  endif
#  if defined(__cpp_lib_span)
    oss << "__cpp_lib_span=" << __cpp_lib_span << "\n";
#  endif
#  if defined(__cpp_lib_expected)
    oss << "__cpp_lib_expected=" << __cpp_lib_expected << "\n";
#  endif
#  if defined(__cpp_lib_coroutine)
    oss << "__cpp_lib_coroutine=" << __cpp_lib_coroutine << "（库层协程支持）\n";
#  endif
#  if defined(__cpp_lib_parallel_algorithm)
    oss << "__cpp_lib_parallel_algorithm=" << __cpp_lib_parallel_algorithm << "\n";
#  endif
#  if defined(__cpp_lib_execution)
    oss << "__cpp_lib_execution=" << __cpp_lib_execution << "\n";
#  endif
#endif

    // 常见 C++ 属性可用性（使用 __has_cpp_attribute）
    oss << "—— C++ 属性可用性 ——\n";
    oss << "[[nodiscard]]: " << yesno(__has_cpp_attribute(nodiscard) != 0) << "\n";
    oss << "[[deprecated]]: " << yesno(__has_cpp_attribute(deprecated) != 0) << "\n";
    oss << "[[fallthrough]]: " << yesno(__has_cpp_attribute(fallthrough) != 0) << "\n";
    oss << "[[likely]]/[[unlikely]]: " << yesno(__has_cpp_attribute(likely) != 0 && __has_cpp_attribute(unlikely) != 0) << "\n";
    oss << "[[no_unique_address]]: " << yesno(__has_cpp_attribute(no_unique_address) != 0) << "\n";

    return oss.str();
}

int main() {
    std::cout << "================ 编译器与环境信息 ================\n";

    // 概览
    std::cout << "编译器： " << detect_compiler_name() << "\n";
    std::cout << detect_compiler_version();

    // 构建时间（以预处理宏为准）
    std::cout << "构建日期： " << __DATE__ << " " << __TIME__ << "\n";
#if defined(__TIMESTAMP__)
    std::cout << "源文件时间戳： " << __TIMESTAMP__ << "\n";
#endif
    std::cout << "\n";

    // 目标平台
    std::cout << "—— 目标平台 ——\n";
    std::cout << "操作系统： " << detect_os() << "\n";
    std::cout << "架构： " << detect_arch() << "\n";
    std::cout << "指针位宽： " << detect_pointer_bits() << " 位\n";
    std::cout << "字节序： " << detect_endianness() << "\n";
#if defined(HAS_HEADER_THREAD)
    std::cout << "硬件并发（提示）：";
    unsigned hc = std::thread::hardware_concurrency();
    if (hc == 0) std::cout << "未知\n";
    else std::cout << hc << " 线程（参考值，运行时查询）\n";
#endif
    std::cout << "\n";

    // 标准库与 C 库
    std::cout << "—— 标准库与 C 运行时 ——\n";
    std::cout << "C++ 标准库： " << detect_cpp_stdlib() << "\n";
    std::cout << "C 运行时库： " << detect_c_libc() << "\n";
    std::cout << "\n";

    // C++ 标准与特性
    std::cout << "—— C++ 标准与特性 ——\n";
    std::cout << detect_cxx_features() << "\n";

    // 构建配置
    std::cout << "—— 构建配置 ——\n";
    std::cout << detect_build_config() << "\n";

    // 常见工具链/环境层
    std::cout << "—— 工具链风味 ——\n";
    std::cout << detect_toolchain_flavor() << "\n\n";

    // 若使用 GCC/libstdc++，可以打印 ABI 版本
#if defined(__GXX_ABI_VERSION)
    std::cout << "libstdc++ ABI 版本：__GXX_ABI_VERSION=" << __GXX_ABI_VERSION << "\n";
#endif

    // 附加：一些基础类型信息（可选，帮助确认数据模型）
    std::cout << "\n—— 附加（数据模型） ——\n";
    std::cout << "sizeof(void*)=" << sizeof(void*) << ", sizeof(long)=" << sizeof(long)
              << ", sizeof(size_t)=" << sizeof(size_t) << "\n";
    std::cout << "char 是否有符号（实现定义）： " << yesno(static_cast<char>(-1) < 0) << "\n";

    std::cout << "==================================================\n";
    return 0;
}
