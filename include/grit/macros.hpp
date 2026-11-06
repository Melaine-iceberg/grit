#ifndef GRIT_MACROS_HPP
#define GRIT_MACROS_HPP

// 预处理宏不会被模块导出：import 只导入声明，不导入宏定义。
// 需要在失败时改变调用方控制流（return / continue / break）的断言只能写成宏，
// 因此它们放在这个头文件里；诊断与格式化逻辑都在模块 macros 中。
//
// 这些宏只作为语句使用，不要直接放在不带花括号的 if/else 分支里（悬挂 else）。

import grit;

/// 致命断言：条件不成立时输出诊断并终止进程。
/// 用法：GRIT_ASSERT(ptr != nullptr, "ptr 为空");
#define GRIT_ASSERT(condition, ...)                                            \
  do {                                                                         \
    static_assert(::girt::macros::Assertable<decltype(condition)>,             \
                  "断言条件必须能转换为 bool");                                \
    if (!static_cast<bool>(condition)) [[unlikely]] {                          \
      ::girt::macros::fail(#condition __VA_OPT__(, ) __VA_ARGS__);             \
    }                                                                          \
  } while (false)

/// 软断言，失败后不做任何事（只输出诊断）。
#define GRIT_ASSERT_NO_EFFECT(condition, ...)                                  \
  do {                                                                         \
    static_assert(::girt::macros::Assertable<decltype(condition)>,             \
                  "断言条件必须能转换为 bool");                                \
    if (!static_cast<bool>(condition)) [[unlikely]] {                          \
      ::girt::macros::report(#condition __VA_OPT__(, ) __VA_ARGS__);           \
    }                                                                          \
  } while (false)

/// 软断言，失败后 return（要求所在函数的返回类型为 void）。
#define GRIT_ASSERT_RET_NONE(condition, ...)                                   \
  if (!::girt::macros::check(#condition,                                       \
                             (condition)__VA_OPT__(, ) __VA_ARGS__))           \
  return

/// 软断言，失败后 return value。
#define GRIT_ASSERT_RET_VAL(condition, value, ...)                             \
  if (!::girt::macros::check(#condition,                                       \
                             (condition)__VA_OPT__(, ) __VA_ARGS__))           \
  return value

/// 软断言，失败后 continue（只能在循环体内使用）。
#define GRIT_ASSERT_CONTINUE(condition, ...)                                   \
  if (!::girt::macros::check(#condition,                                       \
                             (condition)__VA_OPT__(, ) __VA_ARGS__))           \
  continue

/// 软断言，失败后 break（只能在循环或 switch 内使用）。
#define GRIT_ASSERT_BREAK(condition, ...)                                      \
  if (!::girt::macros::check(#condition,                                       \
                             (condition)__VA_OPT__(, ) __VA_ARGS__))           \
  break

#endif // GRIT_MACROS_HPP
