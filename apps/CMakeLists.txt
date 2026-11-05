add_executable(grit-demo main.cpp)

target_link_libraries(grit-demo PRIVATE grit::grit)

# libstdc++ 把 std::print 的终端实现放在独立的 libstdc++exp 中，import std 不会自动带入该链接项。
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(grit-demo PRIVATE stdc++exp)
endif()

# 示例程序自身也 import std，需要显式开启。
set_target_properties(grit-demo PROPERTIES CXX_MODULE_STD ON)
