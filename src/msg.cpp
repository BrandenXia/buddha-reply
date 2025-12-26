#include "msg.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <unordered_set>

#include "SQLiteCpp/SQLiteCpp.h"

namespace buddha {

#define MAX_CHAR_LEN 600
#define MAX_NEWLINE 15
#define MIN_UNIQUE_RATIO 0.4

bool is_spam(std::string_view content) {
  using namespace std::literals::string_view_literals;
  auto newline_cnt = 0;
  auto word_cnt = 0;
  auto unique_words = std::unordered_set<std::string_view>{};

  auto start = 0;
  while (start < content.size()) {
    auto end = content.find_first_of(" \n"sv, start);
    if (end == std::string_view::npos)
      end = content.size();

    auto word = content.substr(start, end - start);
    if (!word.empty()) {
      ++word_cnt;
      if (word == "\n"sv)
        ++newline_cnt;
      else
        unique_words.insert(word);
    }

    if (newline_cnt > MAX_NEWLINE)
      return true;

    start = end + 1;
  }

  if (word_cnt == 0)
    return true;
  if (static_cast<double>(unique_words.size()) / word_cnt < MIN_UNIQUE_RATIO)
    return true;

  return false;
}

auto parse_time(std::string_view time_str) {
  auto t = std::tm{};
  auto ss = std::istringstream{std::string{time_str}};
  ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

#ifdef _WIN32
  std::time_t timet = _mkgmtime(&t);
#else
  auto timet = timegm(&t);
#endif

  if (timet == -1)
    throw std::runtime_error("Failed to parse time: " + std::string{time_str});

  return std::chrono::system_clock::from_time_t(timet);
}

auto get_messages(std::filesystem::path db_path)
    -> cppcoro::generator<std::pair<time_point, std::u8string>> {
  auto db = SQLite::Database{db_path};

  auto query = SQLite::Statement{db, "SELECT createdAt,content FROM "
                                     "messages WHERE length(content) <= ?"};
  query.bind(1, MAX_CHAR_LEN);

  while (query.executeStep()) {
    const char *createdAt = query.getColumn(0);
    const char *content = query.getColumn(1);

    if (is_spam(content))
      continue;

    auto sv = std::string_view{content};

    co_yield {
        parse_time(createdAt),
        std::u8string{reinterpret_cast<const char8_t *>(sv.data()), sv.size()}};
  }
}

} // namespace buddha
