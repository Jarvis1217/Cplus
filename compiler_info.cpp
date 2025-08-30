/*
  ��������Ϣ�ɼ��������ļ�����ֱ�ӱ������У�
  Ŀ�꣺
    1) ������������ϸ�µش�ӡ��ǰʹ�õ� C++ ���������价���������Ϣ
    2) ����������������Clang/Apple Clang��GCC��MSVC��Intel��NVHPC/PGI��Emscripten��MinGW ��
    3) ͬʱ��ӡ��
       - ����������/����/�汾�����ְ汾�꣩
       - C++ ��׼�����Ժ꣨����/�⣩
       - Ŀ��ƽ̨������ϵͳ��CPU �ܹ���λ���ֽ���
       - ��׼���� C ��ʵ��
       - �������ã�����/�������Ż������쳣/RTTI��Sanitizer�ȣ�
       - �������빹��ʱ��
  ˵����
    - ��ͬ������/ƽ̨�겻��ȫһ�£������д���ʹ�������������̽�⡣
    - ĳЩ����Ҫ������Ӧͷ�ļ��� <version> ͷ�Ż���֣����ļ��ᾡ�����������ذ������ǡ�
    - ������ϢΪ�����Ŭ�����������޷� 100% ��ȷʶ������ VS ����С�汾��libc++ �汾��ȷ�ŵȣ���

  ʹ��ʾ������ѡ��һ����
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

// ���� ����������C++20 �� <version> �� <bit>���������Ժ����ֽ��򣩡���
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

// ���� Ϊ Clang ר���� __has_feature / __has_cpp_attribute �ṩ��ȫ���� ����
// �����ڷ� Clang �������ϱ���
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#ifndef __has_cpp_attribute
#  define __has_cpp_attribute(x) 0
#endif

// ���� �ַ�����С���ߺ� ���� 
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// ���� С���ߣ�����ֵ �� ����/�� ���� 
static const char* yesno(bool v) { return v ? "��" : "��"; }

// ���� �ƶ� C++ ��׼���ƣ����� __cplusplus �� _MSVC_LANG������
static const char* cxx_standard_name(long v) {
    // �ο���ֵ��199711L(C++98/03), 201103L(C++11), 201402L(C++14), 201703L(C++17),
    // 202002L(C++20), 202302L(C++23), 202600L/3?(C++26/�ݰ�������������)
    if      (v >= 202600L) return "C++26�����/�ݰ�";
    else if (v >= 202302L) return "C++23";
    else if (v >= 202002L) return "C++20";
    else if (v >= 201703L) return "C++17";
    else if (v >= 201402L) return "C++14";
    else if (v >= 201103L) return "C++11";
    else if (v >= 199711L) return "C++98/03";
    else return "��δ֪/�ǳ����ϵı�׼��";
}

// ���� MSVC �汾�Ž���ӳ�� ���� 
static std::string msvc_pretty_from_ver(int msc_ver) {
    // ������ӳ�����汾��
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
    else return "�ǳ����ϵ� MSVC";
}

// ���� ����������/����/�汾̽�� ���� 
static std::string detect_compiler_name() {
    std::ostringstream oss;
#if defined(__clang__)
    // Clang ���壨���� Apple Clang��clang-cl��Intel LLVM ǰ�˵ȣ�
#  if defined(__apple_build_version__)
    oss << "Apple Clang";
#  elif defined(_MSC_VER)
    oss << "LLVM Clang (clang-cl, MSVC ǰ��)";
#  elif defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_CLANG_COMPILER)
    oss << "Intel oneAPI (LLVM/Clang ǰ��)";
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
    // ע�⣺Clang Ҳ�ᶨ�� __GNUC__�����Ӧ���������ж� __clang__
    oss << "GNU Compiler Collection (GCC)";
#else
    oss << "δ֪�������������";
#endif
    return oss.str();
}

static std::string detect_compiler_version() {
    std::ostringstream oss;
#if defined(__clang__)
    // Clang �ṩ __clang_major__/__clang_minor__/__clang_patchlevel__ �� __clang_version__
    oss << "Clang �汾: "
#  if defined(__clang_major__)
        << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#  else
        << "(δ֪)";
#  endif
    oss << "\n";
#  if defined(__clang_version__)
    oss << "�����汾��: " << __clang_version__ << "\n";
#  endif
#  if defined(__apple_build_version__)
    oss << "Apple Build Version: " << __apple_build_version__ << "\n";
#  endif
#  if defined(_MSC_VER)
    oss << "��clang-cl ���� MSVC��_MSC_VER=" << _MSC_VER;
#    if defined(_MSC_FULL_VER)
    oss << ", _MSC_FULL_VER=" << _MSC_FULL_VER;
#    endif
#    if defined(_MSC_BUILD)
    oss << ", _MSC_BUILD=" << _MSC_BUILD;
#    endif
    oss << "��\n";
#  endif
#  if defined(__INTEL_LLVM_COMPILER)
    oss << "Intel LLVM ǰ�˰汾�� __INTEL_LLVM_COMPILER=" << __INTEL_LLVM_COMPILER << "\n";
#  endif
#elif defined(_MSC_VER)
    oss << "_MSC_VER=" << _MSC_VER;
#  if defined(_MSC_FULL_VER)
    oss << ", _MSC_FULL_VER=" << _MSC_FULL_VER;
#  endif
#  if defined(_MSC_BUILD)
    oss << ", _MSC_BUILD=" << _MSC_BUILD;
#  endif
    oss << " �� " << msvc_pretty_from_ver(_MSC_VER) << "\n";
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
    oss << "GCC �汾: ";
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
    oss << "(δ֪)";
#  endif
    oss << "\n";
#else
    oss << "(δ��ʶ��汾��)\n";
#endif

    // __VERSION__��GCC/Clang �����������汾��
#if defined(__VERSION__)
    oss << "__VERSION__: " << __VERSION__ << "\n";
#endif
    return oss.str();
}

// ���� ����ϵͳ̽�� ���� 
static std::string detect_os() {
    std::ostringstream oss;
#if defined(__EMSCRIPTEN__)
    oss << "WebAssembly (Emscripten)";
#elif defined(_WIN32)
    oss << "Windows";
#  if defined(_WIN64)
    oss << " 64λ";
#  else
    oss << " 32λ";
#  endif
#elif defined(__APPLE__) && defined(__MACH__)
    oss << "Apple Darwin (macOS / iOS / tvOS / watchOS)";
#  if defined(HAS_TARGET_CONDITIONALS)
    // ��Ҫ <TargetConditionals.h>
#    if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    oss << "��iOS ���壩";
#    elif defined(TARGET_OS_OSX) && TARGET_OS_OSX
    oss << "��macOS��";
#    endif
#  endif
#elif defined(__ANDROID__)
    oss << "Android (Linux �ں�)";
#elif defined(__linux__)
    oss << "Linux";
#elif defined(__CYGWIN__)
    oss << "Cygwin (POSIX on Windows)";
#elif defined(__unix__) || defined(__unix)
    oss << "Unix (��ָ, ����δϸ��)";
#else
    oss << "δ֪/Ƕ��ʽ/����������ϵͳ";
#endif

    // BSD ϵ�в���
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

// ���� CPU �ܹ�̽�� ���� 
static std::string detect_arch() {
    std::ostringstream oss;
#if defined(__x86_64__) || defined(_M_X64)
    oss << "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    oss << "x86 (32λ)";
#elif defined(__aarch64__) || defined(_M_ARM64)
    oss << "ARM64 (AArch64)";
#elif defined(__arm__) || defined(_M_ARM)
    oss << "ARM (32λ)";
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
    oss << "δ֪�ܹ�";
#endif
    return oss.str();
}

// ���� λ��ָ���С�� ���� 
static int detect_pointer_bits() {
    return static_cast<int>(sizeof(void*) * CHAR_BIT);
}

// ���� �ֽ���̽�� ���� 
static const char* detect_endianness() {
#if defined(HAS_HEADER_BIT) && defined(__cpp_lib_endian)
    // C++20 �ṩ std::endian
    if (std::endian::native == std::endian::little) return "С��";
    if (std::endian::native == std::endian::big)    return "���";
    if (std::endian::native == std::endian::mixed)  return "��϶���";
    return "δ֪";
#else
    // ����������/ƽ̨��
#  if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return "С��";
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return "���";
#    else
    return "��϶���/δ֪";
#    endif
#  elif defined(_WIN32) || defined(__LITTLE_ENDIAN__) || defined(__x86_64__) || defined(__i386__) || defined(__aarch64__) || defined(__wasm__)
    return "С��";
#  else
    return "δ֪";
#  endif
#endif
}

// ���� ��׼��ʵ��̽�� ���� 
static std::string detect_cpp_stdlib() {
    std::ostringstream oss;
#if defined(_LIBCPP_VERSION)
    oss << "libc++ (_LIBCPP_VERSION=" << _LIBCPP_VERSION << ")";
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
    // libstdc++ ʹ�� __GLIBCXX__��������ֵ���� __GLIBCPP__
#  if defined(__GLIBCXX__)
    oss << "libstdc++ (�汾���ں� __GLIBCXX__=" << __GLIBCXX__ << ")";
#  else
    oss << "libstdc++ (�ɰ�� __GLIBCPP__ ����)";
#  endif
#elif defined(_MSVC_STL_VERSION) || defined(_CPPLIB_VER)
    // MSVC STL��Dinkumware��
#  if defined(_MSVC_STL_VERSION)
    oss << "MSVC STL (_MSVC_STL_VERSION=" << _MSVC_STL_VERSION << ")";
#  elif defined(_CPPLIB_VER)
    oss << "Dinkumware/MSVC STL (_CPPLIB_VER=" << _CPPLIB_VER << ")";
#  endif
#else
    oss << "δ֪/���� C++ ��׼��ʵ��";
#endif
    return oss.str();
}

// ���� C ��׼��ʵ��̽�⣨glibc/musl/bionic/newlib/uClibc�ȣ� ���� 
static std::string detect_c_libc() {
    std::ostringstream oss;
#if defined(__GLIBC__)
    oss << "glibc " << __GLIBC__ << "." << __GLIBC_MINOR__;
    // _FORTIFY_SOURCE �� glibc �ӹ̺�
#  if defined(_FORTIFY_SOURCE)
    oss << " (FORTIFY_SOURCE=" << _FORTIFY_SOURCE << ")";
#  endif
#elif defined(__BIONIC__)
    oss << "Bionic (Android C ��)";
#elif defined(__UCLIBC__)
    oss << "uClibc";
#elif defined(__NEWLIB__)
    oss << "newlib";
#elif defined(__MSVCRT__) || defined(_UCRT)
    // Windows �ϵ� MSVCRT / UCRT
#  if defined(_UCRT)
    oss << "UCRT (Windows Universal C Runtime)";
#  else
    oss << "MSVCRT (Microsoft C Runtime)";
#  endif
#else
    oss << "δ֪/��̬��Ƕ��ʽ C ����ʱ";
#endif
    return oss.str();
}

// ���� �Ƿ�Ϊ MinGW/Cygwin/Emscripten �ȹ������� ���� 
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
    if (!any) oss << "�����ر𹤾�����ʶ�����ã�";
    return oss.str();
}

// ���� �������ã�����/�Ż�/�쳣/RTTI/Sanitizer �ȣ� ���� 
static std::string detect_build_config() {
    std::ostringstream oss;

    // ����/��������
#if defined(_DEBUG)
    oss << "���Ժ� _DEBUG: �Ѷ���\n";
#endif
#if defined(NDEBUG)
    oss << "������ NDEBUG: �Ѷ��壨����ͨ�������ã�\n";
#endif

    // �Ż���أ�GCC/Clang��
#if defined(__OPTIMIZE__)
    oss << "�Ż���__OPTIMIZE__=1��-O1/-O2/-O3/����\n";
#endif
#if defined(__OPTIMIZE_SIZE__)
    oss << "�Ż���__OPTIMIZE_SIZE__=1��-Os �� -Oz��\n";
#endif
#if defined(__NO_INLINE__)
    oss << "������__NO_INLINE__=1����ֹ������\n";
#endif

    // λ���޹ش���/��ִ���ļ�
#if defined(__PIC__)
    oss << "λ���޹ش��룺__PIC__ �Ѷ���\n";
#endif
#if defined(__PIE__)
    oss << "λ���޹ؿ�ִ���ļ���__PIE__ �Ѷ���\n";
#endif

    // �쳣����
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
    oss << "C++ �쳣֧�֣� " << yesno(exceptions_on) << "\n";

    // RTTI ����
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
    oss << "RTTI������ʱ������Ϣ���� " << yesno(rtti_on) << "\n";

    // Sanitizer ̽�⣨Clang/GCC ����֧�֣�
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
    oss << "Sanitizers��ASan=" << yesno(asan)
        << ", TSan=" << yesno(tsan)
        << ", MSan=" << yesno(msan)
        << ", UBSan=" << yesno(ubsan)
        << ", LSan=" << yesno(lsan) << "\n";

    // MSVC STL ���������Լ���
#if defined(_ITERATOR_DEBUG_LEVEL)
    oss << "MSVC _ITERATOR_DEBUG_LEVEL=" << _ITERATOR_DEBUG_LEVEL << "\n";
#endif

    // GNU libstdc++ ����
#if defined(_GLIBCXX_ASSERTIONS)
    oss << "libstdc++ ����ʱ���ԣ������� (_GLIBCXX_ASSERTIONS)\n";
#endif

    return oss.str();
}

// ���� C++ ��׼�����Ժ��ӡ ���� 
static std::string detect_cxx_features() {
    std::ostringstream oss;

    // C++ ��׼��
#if defined(_MSVC_LANG)
    long std_macro = static_cast<long>(_MSVC_LANG); // MSVC ʹ�� _MSVC_LANG ��ʾ���׼
    oss << "���׼��MSVC����_MSVC_LANG=" << std_macro << " �� " << cxx_standard_name(std_macro) << "\n";
#endif
    oss << "__cplusplus=" << static_cast<long>(__cplusplus) << " �� " << cxx_standard_name(__cplusplus) << "\n";

    // �����������ԣ�__cpp_*������ ���ڱ�����/��׼֧��ʱ�ᶨ��
    // ע���������б��������ᶨ�������� feature-test �꣬����Ϊ�����Ӽ���
#if defined(__cpp_constexpr)
    oss << "__cpp_constexpr=" << __cpp_constexpr << "��constexpr ֧�֣�\n";
#else
    oss << "__cpp_constexpr δ����\n";
#endif

#if defined(__cpp_if_constexpr)
    oss << "__cpp_if_constexpr=" << __cpp_if_constexpr << "��if constexpr��\n";
#endif

#if defined(__cpp_concepts)
    oss << "__cpp_concepts=" << __cpp_concepts << "�����\n";
#endif

#if defined(__cpp_modules)
    oss << "__cpp_modules=" << __cpp_modules << "��ģ�飩\n";
#endif

#if defined(__cpp_coroutines)
    oss << "__cpp_coroutines=" << __cpp_coroutines << "��Э������֧�֣�\n";
#endif

#if defined(__cpp_generic_lambdas)
    oss << "__cpp_generic_lambdas=" << __cpp_generic_lambdas << "������ Lambda��\n";
#endif

#if defined(__cpp_fold_expressions)
    oss << "__cpp_fold_expressions=" << __cpp_fold_expressions << "���۵����ʽ��\n";
#endif

#if defined(__cpp_structured_bindings)
    oss << "__cpp_structured_bindings=" << __cpp_structured_bindings << "���ṹ���󶨣�\n";
#endif

#if defined(__cpp_consteval)
    oss << "__cpp_consteval=" << __cpp_consteval << "��consteval��\n";
#endif

#if defined(__cpp_nontype_template_args)
    oss << "__cpp_nontype_template_args=" << __cpp_nontype_template_args << "��������ģ�������ǿ��\n";
#endif

#if defined(__cpp_designated_initializers)
    oss << "__cpp_designated_initializers=" << __cpp_designated_initializers << "��ָ����ʼ������\n";
#endif

    // C++ ��׼�����Ժ꣨��Ҫ <version> �����ͷ��
#if defined(HAS_HEADER_VERSION) || defined(__cpp_lib_ranges) || defined(__cpp_lib_format)
    oss << "���� C++ ��׼�����ԣ�__cpp_lib_*������\n";
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
    oss << "__cpp_lib_coroutine=" << __cpp_lib_coroutine << "�����Э��֧�֣�\n";
#  endif
#  if defined(__cpp_lib_parallel_algorithm)
    oss << "__cpp_lib_parallel_algorithm=" << __cpp_lib_parallel_algorithm << "\n";
#  endif
#  if defined(__cpp_lib_execution)
    oss << "__cpp_lib_execution=" << __cpp_lib_execution << "\n";
#  endif
#endif

    // ���� C++ ���Կ����ԣ�ʹ�� __has_cpp_attribute��
    oss << "���� C++ ���Կ����� ����\n";
    oss << "[[nodiscard]]: " << yesno(__has_cpp_attribute(nodiscard) != 0) << "\n";
    oss << "[[deprecated]]: " << yesno(__has_cpp_attribute(deprecated) != 0) << "\n";
    oss << "[[fallthrough]]: " << yesno(__has_cpp_attribute(fallthrough) != 0) << "\n";
    oss << "[[likely]]/[[unlikely]]: " << yesno(__has_cpp_attribute(likely) != 0 && __has_cpp_attribute(unlikely) != 0) << "\n";
    oss << "[[no_unique_address]]: " << yesno(__has_cpp_attribute(no_unique_address) != 0) << "\n";

    return oss.str();
}

int main() {
    std::cout << "================ �������뻷����Ϣ ================\n";

    // ����
    std::cout << "�������� " << detect_compiler_name() << "\n";
    std::cout << detect_compiler_version();

    // ����ʱ�䣨��Ԥ�����Ϊ׼��
    std::cout << "�������ڣ� " << __DATE__ << " " << __TIME__ << "\n";
#if defined(__TIMESTAMP__)
    std::cout << "Դ�ļ�ʱ����� " << __TIMESTAMP__ << "\n";
#endif
    std::cout << "\n";

    // Ŀ��ƽ̨
    std::cout << "���� Ŀ��ƽ̨ ����\n";
    std::cout << "����ϵͳ�� " << detect_os() << "\n";
    std::cout << "�ܹ��� " << detect_arch() << "\n";
    std::cout << "ָ��λ�� " << detect_pointer_bits() << " λ\n";
    std::cout << "�ֽ��� " << detect_endianness() << "\n";
#if defined(HAS_HEADER_THREAD)
    std::cout << "Ӳ����������ʾ����";
    unsigned hc = std::thread::hardware_concurrency();
    if (hc == 0) std::cout << "δ֪\n";
    else std::cout << hc << " �̣߳��ο�ֵ������ʱ��ѯ��\n";
#endif
    std::cout << "\n";

    // ��׼���� C ��
    std::cout << "���� ��׼���� C ����ʱ ����\n";
    std::cout << "C++ ��׼�⣺ " << detect_cpp_stdlib() << "\n";
    std::cout << "C ����ʱ�⣺ " << detect_c_libc() << "\n";
    std::cout << "\n";

    // C++ ��׼������
    std::cout << "���� C++ ��׼������ ����\n";
    std::cout << detect_cxx_features() << "\n";

    // ��������
    std::cout << "���� �������� ����\n";
    std::cout << detect_build_config() << "\n";

    // ����������/������
    std::cout << "���� ��������ζ ����\n";
    std::cout << detect_toolchain_flavor() << "\n\n";

    // ��ʹ�� GCC/libstdc++�����Դ�ӡ ABI �汾
#if defined(__GXX_ABI_VERSION)
    std::cout << "libstdc++ ABI �汾��__GXX_ABI_VERSION=" << __GXX_ABI_VERSION << "\n";
#endif

    // ���ӣ�һЩ����������Ϣ����ѡ������ȷ������ģ�ͣ�
    std::cout << "\n���� ���ӣ�����ģ�ͣ� ����\n";
    std::cout << "sizeof(void*)=" << sizeof(void*) << ", sizeof(long)=" << sizeof(long)
              << ", sizeof(size_t)=" << sizeof(size_t) << "\n";
    std::cout << "char �Ƿ��з��ţ�ʵ�ֶ��壩�� " << yesno(static_cast<char>(-1) < 0) << "\n";

    std::cout << "==================================================\n";
    return 0;
}
