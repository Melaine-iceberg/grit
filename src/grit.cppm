module;

#include <grit/version.hpp>

export module grit;

import std;

export namespace grit {

/// 库版本，由 CMake project(VERSION) 注入（见 cmake/version.hpp.in）。
inline constexpr std::string_view VERSION = GRIT_VERSION_STRING;

} // namespace grit
