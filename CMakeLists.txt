cmake_minimum_required(VERSION 4.4)

# import std 是实验特性，激活值随 CMake 版本变化，必须在 project() 之前设置。
# 本值对应 CMake 4.4.5；升级 CMake 后按报错提示更新，否则 CXX_MODULE_STD 会配置失败。
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

project(grit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 作为子目录（add_subdirectory / FetchContent）引入时，默认不构建示例与测试、不生成安装规则。
option(GRIT_BUILD_APPS "构建 apps/ 下的示例程序" ${PROJECT_IS_TOP_LEVEL})
option(GRIT_BUILD_TESTS "构建 tests/ 下的 GoogleTest 单元测试" ${PROJECT_IS_TOP_LEVEL})
option(GRIT_INSTALL "生成 grit 的安装与 CMake 包规则" ${PROJECT_IS_TOP_LEVEL})

# ---------------------------------------------------------------------------
# 库目标
# ---------------------------------------------------------------------------
add_library(grit)
add_library(grit::grit ALIAS grit)

# src/*.cppm 作为模块接口单元，src/*.cpp 作为普通实现单元；
# 新增文件由 CONFIGURE_DEPENDS 自动重新扫描。
file(GLOB_RECURSE GRIT_MODULE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm")
file(GLOB_RECURSE GRIT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if (NOT GRIT_MODULE_SOURCES AND NOT GRIT_SOURCES)
    message(FATAL_ERROR "grit 库没有源文件：请在 src/ 下添加 .cppm 模块接口单元或 .cpp 实现单元")
endif ()

if (GRIT_MODULE_SOURCES)
    target_sources(grit PUBLIC FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
            FILES ${GRIT_MODULE_SOURCES})
endif ()

if (GRIT_SOURCES)
    target_sources(grit PRIVATE ${GRIT_SOURCES})
endif ()

# include/ 下的普通头文件（宏无法从模块导出，只能走头文件）；HEADERS 文件集自动把
# BASE_DIRS 加入目标的包含目录，并参与下面的 install(TARGETS ...)。
target_sources(grit PUBLIC FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/grit/macros.hpp")

# 版本头由 project(VERSION) 生成，C++ 与 CMake 的版本号只保留一个来源。
configure_file(cmake/version.hpp.in
        "${CMAKE_CURRENT_BINARY_DIR}/generated/include/grit/version.hpp"
        @ONLY)

target_include_directories(grit PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# 导出的目标携带 cxx_std_26，消费者无需重复设置标准。
target_compile_features(grit PUBLIC cxx_std_26)

# 允许目标内部 import std；CMake 会自行构建 libstdc++ 的 std/std.compat BMI（需 Ninja 生成器）。
set_target_properties(grit PROPERTIES CXX_MODULE_STD ON)

# GCC / Clang（Linux 与 MinGW）：关闭 RTTI 与 C++ 异常。
# PUBLIC：消费者的代码生成选项必须与 grit 模块 BMI 保持一致。
target_compile_options(grit PUBLIC
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-fno-rtti -fno-exceptions>)

# libstdc++ 把 std::print 和 std::stacktrace 的实现放在独立的 libstdc++exp 中，
# import std 不会自动带入该链接项；grit 的诊断输出依赖这两者，消费者需要一并链接。
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(grit PUBLIC stdc++exp)
endif ()

# ---------------------------------------------------------------------------
# 示例程序
# ---------------------------------------------------------------------------
if (GRIT_BUILD_APPS)
    add_subdirectory(apps)
endif ()

# ---------------------------------------------------------------------------
# 单元测试
# ---------------------------------------------------------------------------
if (GRIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

# ---------------------------------------------------------------------------
# 安装与导出
# ---------------------------------------------------------------------------
if (GRIT_INSTALL)
    install(TARGETS grit
            EXPORT gritTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/grit
            FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/include/grit/version.hpp"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/grit)

    install(EXPORT gritTargets
            FILE gritTargets.cmake
            NAMESPACE grit::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grit)

    configure_package_config_file(cmake/gritConfig.cmake.in
            "${CMAKE_CURRENT_BINARY_DIR}/gritConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grit)

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/gritConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion)

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/gritConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/gritConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grit)
endif ()
