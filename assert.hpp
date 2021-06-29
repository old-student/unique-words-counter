#pragma once

#include <sstream>

namespace details
{

void __assert__(bool v, const char* file, int line, const char* expr);

template <typename... Args>
void print_variadic_args(std::ostringstream& out, Args&&... args)
{
    (void)std::initializer_list<int>{(out << " " << std::forward<Args>(args), 0)...};
}

template <typename... Args>
void __assert__(bool expr, const std::string& file, int line, const std::string& exprCode, Args&&... args)
{
  if (expr) { return; }

  std::ostringstream stream;
  stream << "\n -- " << file << ":" << line << ": The expression \"" <<  exprCode << "\" is not correct.";
  print_variadic_args(stream, std::forward<Args>(args)...);

  throw std::runtime_error(stream.str());
}

} // namespace details

#define rt_assert(expr, ...) \
  ::details::__assert__((expr), __FILE__, __LINE__, #expr, __VA_ARGS__);
