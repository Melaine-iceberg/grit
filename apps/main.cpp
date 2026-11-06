import std;

import grit;

/// 最小示例：打印库版本。断言宏的验证在 tests/ 下由 GoogleTest 覆盖。
int main() { std::print("grit {}\n", grit::VERSION); }
