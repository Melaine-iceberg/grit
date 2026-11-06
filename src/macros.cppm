export module grit:macros;

import std.compat;

export namespace girt::macros {

constexpr uint64_t INVALID64 = std::numeric_limits<int64_t>::max();
constexpr uint32_t INVALID32 = std::numeric_limits<uint32_t>::max();
constexpr uint16_t INVALID16 = std::numeric_limits<uint16_t>::max();
constexpr uint8_t INVALID8 = std::numeric_limits<uint8_t>::max();

using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using usize = std::size_t;
using isize = std::ptrdiff_t;

/// 条件类型：能显式转换为 bool，可作为断言条件。
template <typename T>
concept Assertable = requires(const T &value) {
  { static_cast<bool>(value) } -> std::same_as<bool>;
};

/// 消息类型：能用 std::format("{}", value) 输出，可作为断言消息。
template <typename T>
concept Formattable = std::formattable<std::remove_cvref_t<T>, char>;

namespace detail {

/// 默认构造出来的空字符串视为"没有消息"。
template <Formattable W>
[[nodiscard]] constexpr bool hasMessage(const W &message) {
  if constexpr (requires { message.empty(); }) {
    return !message.empty();
  } else {
    return true;
  }
}

template <Formattable W>
[[nodiscard]] std::string formatMessage(const W &message) {
  if constexpr (std::same_as<std::remove_cvref_t<W>, std::string>) {
    return message;
  } else if constexpr (std::same_as<std::remove_cvref_t<W>, std::string_view>) {
    return std::string{message};
  } else {
    return std::format("{}", message);
  }
}

/// 唯一的诊断出口：源码位置、条件表达式、消息、调用栈。不终止进程。
void printDiagnosis(std::string_view expression, std::string_view message,
                    const std::source_location &location);

} // namespace detail

/// 输出诊断信息，不终止进程。
/// expression 为空时省略"条件"一行；message 为空时省略"消息"一行。
template <Formattable W = std::string_view>
void report(
    std::string_view expression, const W &message = {},
    const std::source_location &location = std::source_location::current()) {
  std::string text;
  if (detail::hasMessage(message)) {
    text = detail::formatMessage(message);
  }
  detail::printDiagnosis(expression, text, location);
}

/// 输出诊断信息后终止进程（致命断言）。
template <Formattable W = std::string_view>
[[noreturn]] void
fail(std::string_view expression, const W &message = {},
     const std::source_location &location = std::source_location::current()) {
  report(expression, message, location);
  std::abort();
}

/// 软断言：条件成立返回 true；否则输出诊断并返回
/// false，由调用方决定后续控制流。
template <Assertable T, Formattable W = std::string_view>
[[nodiscard]] bool
check(std::string_view expression, const T &value, const W &message = {},
      const std::source_location &location = std::source_location::current()) {
  if (static_cast<bool>(value)) [[likely]] {
    return true;
  }
  report(expression, message, location);
  return false;
}

/// 致命断言：条件不成立时输出诊断并终止进程。
/// 需要"失败后 return / continue / break"时用 <grit/macros.hpp> 中的
/// GRIT_ASSERT_* 宏。
template <Assertable T, Formattable W = std::string_view>
void assert(
    const T &value, const W &message = {},
    const std::source_location &location = std::source_location::current()) {
  if (static_cast<bool>(value)) [[likely]] {
    return;
  }
  fail("", message, location);
}

} // namespace girt::macros

namespace girt::macros::detail {

void printDiagnosis(std::string_view expression, std::string_view message,
                    const std::source_location &location) {
  // 注意：这里刻意不使用 std::format 与 std::print。
  // 该项目最初在 Windows + MinGW GCC 上开发：GCC 会把模块单元里实例化的格式化
  // 内建符号放进普通 .rdata（非 COMDAT），与其它编译单元中的同名实例冲突导致
  // 链接失败。迁移到 Linux 后保留字符串拼接实现，避免重新引入该问题。
  std::string text;

  text += std::string_view{location.file_name()};
  text += ':';
  text += std::to_string(location.line());
  text += ':';
  text += std::to_string(location.column());
  text += ": assertion failed";
  const std::string_view function{location.function_name()};
  if (!function.empty()) {
    text += " in ";
    text += function;
  }
  text += '\n';
  if (!expression.empty()) {
    text += "  condition: ";
    text += expression;
    text += '\n';
  }
  if (!message.empty()) {
    text += "  message: ";
    text += message;
    text += '\n';
  }

  // 跳过 printDiagnosis 与 report 两帧，从真正的断言点开始。
  const std::stacktrace trace = std::stacktrace::current(2);
  text += "stacktrace (";
  text += std::to_string(trace.size());
  text += " frame(s)):\n";
  for (std::size_t i = 0; i < trace.size(); ++i) {
    const std::stacktrace_entry &frame = trace[i];
    text += "  #";
    text += std::to_string(i);
    text += ' ';
    text += frame.description();
    text += '\n';
    if (!frame.source_file().empty()) {
      text += "      at ";
      text += frame.source_file();
      text += ':';
      text += std::to_string(frame.source_line());
      text += '\n';
    }
  }

  std::cerr << text;
}

} // namespace girt::macros::detail
