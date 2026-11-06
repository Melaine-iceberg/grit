add_executable(grit-demo main.cpp)

target_link_libraries(grit-demo PRIVATE grit::grit)

# stdc++exp（std::print / std::stacktrace）由 grit 的 PUBLIC 链接项传递，示例无需重复声明。

# 示例程序自身也 import std，需要显式开启。
set_target_properties(grit-demo PROPERTIES CXX_MODULE_STD ON)
