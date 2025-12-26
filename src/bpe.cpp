#include "bpe.h"

#include <algorithm>
#include <fstream>
#include <print>
#include <ranges>
#include <tuple>
#include <unordered_map>

namespace buddha::bpe {

template <typename T1, typename T2> struct pair_hash {
  auto operator()(const std::pair<T1, T2> &p) const -> std::size_t {
    auto h1 = std::hash<T1>{}(p.first);
    auto h2 = std::hash<T2>{}(p.second);
    return h1 ^ (h2 << 1);
  }
};

using Freqs = std::unordered_map<Pair, std::size_t, pair_hash<Token, Token>>;

inline auto fmt_c(Token t) {
  if (t >= 32 && t <= 126)
    return std::format("'{}'", static_cast<char>(t));
  else
    return std::format("\\{:02x}", t);
}

inline auto print_freqs(const Freqs &freqs) {
  for (const auto &[p, count] : freqs)
    std::println("({}, {}) : {}", fmt_c(p.first), fmt_c(p.second), count);
}

inline auto find_max_freq(const Freqs &freqs) -> std::pair<Pair, std::size_t> {
  return *std::ranges::max_element(
      freqs, [](const auto &a, const auto &b) { return a.second < b.second; });
}

inline auto init_table(Table &t) {
  for (auto i : std::views::iota(0, 256))
    t.emplace_back(Pair{static_cast<Token>(i), 0});
}

auto build(std::u8string_view raw) -> Table {
  auto t = Table{};
  auto freqs = Freqs{};
  auto data = std::vector<Token>{};

  t.reserve(256);
  init_table(t);

  auto len = raw.size();
  data.reserve(len);
  for (auto i = 0; i < len - 1; ++i) {
    auto c = static_cast<Token>(raw[i]);
    auto next_c = static_cast<Token>(raw[i + 1]);
    data.emplace_back(c);
    freqs[Pair{c, next_c}]++;
  }
  data.emplace_back(static_cast<Token>(raw[len - 1]));

  auto [max_p, max_freq] = find_max_freq(freqs);
  while (max_freq > 1) {
    auto new_token = static_cast<Token>(t.size());
    t.emplace_back(max_p);

    auto idx = 0;
    while (idx < data.size() - 1) {
      auto p = Pair{data[idx], data[idx + 1]};
      if (p == max_p) {
        freqs[p]--;
        if (freqs[p] == 0)
          freqs.erase(p);

        if (idx > 0) {
          auto left_p = Pair{data[idx - 1], data[idx]};
          freqs[left_p]--;
          if (freqs[left_p] == 0)
            freqs.erase(left_p);
          auto new_left_p = Pair{data[idx - 1], new_token};
          freqs[new_left_p]++;
        }

        if (idx + 2 < data.size()) {
          auto right_p = Pair{data[idx + 1], data[idx + 2]};
          freqs[right_p]--;
          if (freqs[right_p] == 0)
            freqs.erase(right_p);
          auto new_right_p = Pair{new_token, data[idx + 2]};
          freqs[new_right_p]++;
        }

        data[idx] = new_token;
        data.erase(data.begin() + idx + 1);
      }

      idx++;
    }

    std::tie(max_p, max_freq) = find_max_freq(freqs);
  }

  return t;
}

auto print_table(const Table &table) -> void {
  std::println("BPE Table of size {}", table.size());
  for (auto i = 256; i < table.size(); ++i) {
    const auto &[first, second] = table[i];
    std::println("{}: ({}, {})", fmt_c(i), fmt_c(first), fmt_c(second));
  }
}

auto export_table(std::filesystem::path path, const Table &table) -> void {
  auto f = std::ofstream{path, std::ios::binary};
  // header
  f.write("BPE1", 4);
  // body
  for (auto i = 256; i < table.size(); ++i) {
    const auto &[first, second] = table[i];
    f.write(reinterpret_cast<const char *>(&first), sizeof(Token));
    f.write(reinterpret_cast<const char *>(&second), sizeof(Token));
  }
  f.close();
}

auto import_table(std::filesystem::path path) -> Table {
  auto f = std::ifstream{path, std::ios::binary};
  // check header
  char header[4];
  f.read(header, 4);
  if (std::string_view{header, 3} != "BPE")
    throw std::runtime_error("Invalid BPE table file");

  auto t = Table{};
  // reserve space
  auto fsize = std::filesystem::file_size(path);
  t.reserve(256 + (fsize - 4) / (2 * sizeof(Token)));
  init_table(t);

  switch (header[3]) {
  case '1':
    Token first, second;
    while (f.read(reinterpret_cast<char *>(&first), sizeof(Token))) {
      f.read(reinterpret_cast<char *>(&second), sizeof(Token));
      t.emplace_back(Pair{first, second});
    }
    return t;
  default:
    throw std::runtime_error("Unsupported BPE table version");
  }
}

} // namespace buddha::bpe
