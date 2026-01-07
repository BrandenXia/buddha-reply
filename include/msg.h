#ifndef BUDDHA_REPLY_MSG_H
#define BUDDHA_REPLY_MSG_H

#include <algorithm>
#include <chrono>
#include <filesystem>

#include "cppcoro/generator.hpp"

namespace buddha {

using time_point = std::chrono::system_clock::time_point;

template <std::ranges::input_range Range>
inline constexpr auto shannon_entropy(Range cnts, int total) -> double;

inline constexpr auto shannon_entropy(std::string_view content) -> double;

auto is_spam(std::string_view content) -> bool;

auto get_messages(std::filesystem::path db_path, bool skip_spam = true,
                  bool ordered = true)
    -> cppcoro::generator<std::pair<std::string, std::u8string>>;

} // namespace buddha

template <std::ranges::input_range Range>
inline constexpr auto buddha::shannon_entropy(Range cnts, int total) -> double {
  if (total == 0) return 0.0;
  return std::ranges::fold_left(cnts, 0.0, [total](double acc, int cnt) {
    if (cnt == 0) return acc;
    double p = static_cast<double>(cnt) / total;
    return acc - p * std::log2(p);
  });
}

inline constexpr auto buddha::shannon_entropy(std::string_view content)
    -> double {
  auto freq = std::array<int, sizeof(unsigned char)>{};
  for (unsigned char c : content)
    ++freq[c];
  return shannon_entropy(freq, content.size());
}

#endif
