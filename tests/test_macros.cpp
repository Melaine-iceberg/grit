// grit 断言宏与诊断出口的单元测试。
//
// 致命断言 GRIT_ASSERT 的失败路径会 std::abort()，用 EXPECT_DEATH
// 在子进程里验证； 软断言的失败路径全部在本进程内覆盖。
//
// 注意：GCC 只接受"先包含标准库头文件、后 import 模块"的顺序；反过来会得到
// std / std.compat 实体重复声明的编译错误，所以 gtest 与标准库头文件必须排在
// <grit/macros.hpp> 与 import 之前。

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

#include <grit/macros.hpp>

import grit;

namespace {

/// 把 std::cerr 临时接到内存缓冲区，用于断言诊断文本。
class StderrCapture {
public:
  StderrCapture() : previous_{std::cerr.rdbuf(buffer_.rdbuf())} {}

  ~StderrCapture() { std::cerr.rdbuf(previous_); }

  StderrCapture(const StderrCapture &) = delete;
  StderrCapture &operator=(const StderrCapture &) = delete;

  [[nodiscard]] std::string text() const { return buffer_.str(); }

private:
  std::ostringstream buffer_;
  std::streambuf *previous_;
};

/// 软断言：分母为 0 时返回 0。
int divide(int numerator, int denominator) {
  GRIT_ASSERT_RET_VAL(denominator != 0, 0, "denominator 不能为 0");
  return numerator / denominator;
}

/// 软断言：条件不成立时直接返回，out 保持原值。
void storePositive(int value, int &out) {
  GRIT_ASSERT_RET_NONE(value > 0, "value 必须为正数");
  out = value;
}

} // namespace

TEST(Version, NotEmpty) {
  EXPECT_FALSE(grit::VERSION.empty());
  EXPECT_TRUE(grit::VERSION.starts_with("0."));
}

TEST(Check, PassesTruthyValueThroughWithoutDiagnosis) {
  StderrCapture capture;
  EXPECT_TRUE(girt::macros::check("true", true));
  EXPECT_TRUE(girt::macros::check("value > 0", 3 > 0, "条件成立时不输出"));
  EXPECT_TRUE(capture.text().empty());
}

TEST(Check, ReportsAndReturnsFalseOnFailure) {
  StderrCapture capture;
  EXPECT_FALSE(girt::macros::check("value > 0", 0 > 1, "必须为正数"));

  const std::string text = capture.text();
  EXPECT_NE(text.find("assertion failed"), std::string::npos);
  EXPECT_NE(text.find("condition: value > 0"), std::string::npos);
  EXPECT_NE(text.find("message: 必须为正数"), std::string::npos);
  EXPECT_NE(text.find("stacktrace"), std::string::npos);
}

TEST(Report, OmitsEmptyMessage) {
  StderrCapture capture;
  girt::macros::report("ptr != nullptr");

  const std::string text = capture.text();
  EXPECT_NE(text.find("condition: ptr != nullptr"), std::string::npos);
  EXPECT_EQ(text.find("message:"), std::string::npos);
}

TEST(Report, PrintsFormattedMessage) {
  StderrCapture capture;
  girt::macros::report("size == capacity", 42);

  EXPECT_NE(capture.text().find("message: 42"), std::string::npos);
}

TEST(Assert, PassingConditionHasNoEffect) {
  StderrCapture capture;
  GRIT_ASSERT(grit::VERSION.size() > 1);
  // <cassert> 把 assert 定义成宏，加括号阻止宏展开，才能调用同名函数。
  (girt::macros::assert)(grit::VERSION.starts_with("0."));
  EXPECT_TRUE(capture.text().empty());
}

TEST(Assert, AbortsAndReportsOnFailure) {
  EXPECT_DEATH(GRIT_ASSERT(false, "致命断言不能继续"),
               "message: 致命断言不能继续");
}

TEST(AssertNoEffect, FailingConditionKeepsExecuting) {
  StderrCapture capture;
  int visited = 0;
  for (int i = 0; i < 2; ++i) {
    GRIT_ASSERT_NO_EFFECT(i < 0);
    ++visited;
  }

  EXPECT_EQ(visited, 2);
  EXPECT_NE(capture.text().find("condition: i < 0"), std::string::npos);
}

TEST(AssertNoEffect, PassingConditionIsSilent) {
  StderrCapture capture;
  GRIT_ASSERT_NO_EFFECT(grit::VERSION.size() > 1);
  EXPECT_TRUE(capture.text().empty());
}

TEST(AssertContinue, SkipsRestOfIterationOnFailure) {
  StderrCapture capture;
  int visited = 0;
  for (int i = 0; i < 4; ++i) {
    GRIT_ASSERT_CONTINUE(i < 2);
    ++visited;
  }

  EXPECT_EQ(visited, 2);
  EXPECT_NE(capture.text().find("condition: i < 2"), std::string::npos);
}

TEST(AssertBreak, LeavesLoopOnFailure) {
  StderrCapture capture;
  int stopped = 0;
  while (true) {
    GRIT_ASSERT_BREAK(stopped < 2);
    ++stopped;
  }

  EXPECT_EQ(stopped, 2);
  EXPECT_NE(capture.text().find("condition: stopped < 2"), std::string::npos);
}

TEST(AssertRetVal, ReturnsComputedValue) { EXPECT_EQ(divide(10, 2), 5); }

TEST(AssertRetVal, ReturnsFallbackAndReportsOnFailure) {
  StderrCapture capture;
  EXPECT_EQ(divide(10, 0), 0);
  EXPECT_NE(capture.text().find("message: denominator 不能为 0"),
            std::string::npos);
}

TEST(AssertRetNone, LeavesOutputUntouchedOnFailure) {
  StderrCapture capture;
  int stored = -1;
  storePositive(-1, stored);
  EXPECT_EQ(stored, -1);

  storePositive(3, stored);
  EXPECT_EQ(stored, 3);
  EXPECT_NE(capture.text().find("condition: value > 0"), std::string::npos);
}
