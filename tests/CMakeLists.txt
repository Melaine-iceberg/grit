# 单元测试：vcpkg（vcpkg.json 的 gtest 依赖 + CMakePresets 的 vcpkg 预设）是主路径；
# 没有 vcpkg 的环境下用 FetchContent 拉同一版本，保证直接 configure 也能跑测试。
find_package(GTest CONFIG QUIET)

if (NOT GTest_FOUND)
    include(FetchContent)

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # 与 vcpkg 基线里的 gtest 版本保持一致，避免两条路径测到不同版本。
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.18.0
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL)

    FetchContent_MakeAvailable(googletest)
endif ()

add_executable(grit-tests test_macros.cpp)

target_link_libraries(grit-tests PRIVATE grit::grit GTest::gtest_main)

# 测试文件自己 import grit 与 macros（经由 <grit/macros.hpp>），CMake 会自动扫描模块依赖；
# 测试文件不 import std，因此无需 CXX_MODULE_STD（该属性只负责构建 std 模块本身）。

include(GoogleTest)
gtest_discover_tests(grit-tests)
