#include "formats.h"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <ranges>
#include <string_view>
#include <unordered_map>

#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "re2/re2.h"
#include "re2/stringpiece.h"

#include "bpe.h"
#include "msg.h"
#include "utils.h"

constexpr auto db_path = "data/msg.sqlite";

namespace buddha::exports {

auto text(std::filesystem::path filename) -> void {
  auto f = std::ofstream(filename);

  auto cnt = 0;
  for (const auto &[time, content] : buddha::get_messages(db_path)) {
    f << reinterpret_cast<const char *>(content.c_str()) << "\n\n";
    cnt++;
  }
  f.close();

  std::println("Finished exporting {} messages to {}", cnt, filename.c_str());
}

re2::StringPiece to_piece(std::u8string_view u8sv) {
  return re2::StringPiece(reinterpret_cast<const char *>(u8sv.data()),
                          u8sv.size());
}

auto sft_jsonl(std::filesystem::path filename) -> void {
  auto fp = std::fopen(filename.c_str(), "wb");
  if (!fp) {
    std::println(stderr, "Failed to open file: {}", filename.string());
    return;
  }

  auto buffer = std::vector<char>(65536);
  auto os = rapidjson::FileWriteStream(fp, buffer.data(), buffer.size());
  auto writer = rapidjson::Writer<rapidjson::FileWriteStream>(os);

  auto current_content = std::u8string{};
  auto prev_author = std::string_view{};

  auto is_user_role = true;
  auto first_message = true;

  for (const auto &[author, content] : buddha::get_messages(db_path)) {
    if (!first_message && author != prev_author) {
      auto proxy = reinterpret_cast<std::string *>(&current_content);
      re2::RE2::GlobalReplace(proxy, regex::emoji_regex, "\\1");
      re2::RE2::GlobalReplace(proxy, regex::mention_regex, "@user");

      if (is_user_role) {
        writer.StartObject();
        writer.Key("messages");
        writer.StartArray();
      }

      writer.StartObject();
      writer.Key("role");
      if (is_user_role)
        writer.String("user", 4);
      else
        writer.String("assistant", 9);

      writer.Key("content");
      writer.String(reinterpret_cast<const char *>(current_content.c_str()),
                    static_cast<rapidjson::SizeType>(current_content.size()));
      writer.EndObject();

      if (!is_user_role) {
        writer.EndArray();
        writer.EndObject();
        os.Put('\n');

        is_user_role = true;
      } else
        is_user_role = false;

      current_content.clear();
    }

    if (!current_content.empty()) {
      if (current_content.ends_with(content)) continue;

      current_content += u8"\n";
    }
    current_content += content;

    prev_author = author;
    first_message = false;
  }

  if (!current_content.empty()) {
    if (is_user_role) {
      writer.StartObject();
      writer.Key("messages");
      writer.StartArray();
    }

    writer.StartObject();
    writer.Key("role");
    if (is_user_role)
      writer.String("user", 4);
    else
      writer.String("assistant", 9);

    writer.Key("content");
    writer.String(reinterpret_cast<const char *>(current_content.c_str()),
                  static_cast<rapidjson::SizeType>(current_content.size()));
    writer.EndObject();

    writer.EndArray();
    writer.EndObject();
    os.Put('\n');
  }

  os.Flush();
  std::fclose(fp);
}

auto bpe(std::filesystem::path filename) -> void {
  // clang-format off
  const auto msgs = buddha::get_messages(db_path)
    | std::views::transform([](const auto &pair) { return pair.second; })
    | std::views::join
    | std::ranges::to<std::u8string>();
  // clang-format on
  auto t = buddha::bpe::build(msgs);
  buddha::bpe::print_table(t);
  buddha::bpe::export_table(filename, t);
}

auto emojis(std::filesystem::path filename) -> void {
  assert(emoji_regex.ok());

  auto unique_emojis = std::unordered_map<std::string, int>{};

  std::string match;
  for (const auto &[_, content] : buddha::get_messages(db_path, false, false)) {
    re2::StringPiece input(reinterpret_cast<const char *>(content.c_str()),
                           static_cast<int>(content.size()));
    while (re2::RE2::FindAndConsume(&input, regex::emoji_regex, &match))
      unique_emojis[match]++;
  }

  // clang-format off
  const auto top_emojis = buddha::utils::flip_map(unique_emojis)
    | std::views::reverse
    | std::views::take(15)
    | std::views::transform([](const auto &pair) { return pair.second; });
  // clang-format on

  auto f = std::ofstream{filename};
  f << std::format("{:n}", top_emojis);
  f.close();
}

} // namespace buddha::exports
