#include <cstdio>
#include <filesystem>
#include <fstream>
#include <print>
#include <ranges>
#include <string_view>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

#include "bpe.h"
#include "msg.h"

#define DB_PATH "data/msg.sqlite"

auto export_text(std::filesystem::path filename) {
  auto f = std::ofstream(filename);

  auto cnt = 0;
  for (const auto &[time, content] : buddha::get_messages(DB_PATH)) {
    f << reinterpret_cast<const char *>(content.c_str()) << "\n\n";
    cnt++;
  }
  f.close();

  std::println("Finished exporting {} messages to {}", cnt, filename.c_str());
}

auto export_jsonl(std::filesystem::path filename) {
  auto fp = std::fopen(filename.c_str(), "wb");
  if (!fp) {
    std::println(stderr, "Failed to open file: {}", filename.string());
    return;
  }

  auto buffer = std::vector<char>(65536);
  auto os = rapidjson::FileWriteStream(fp, buffer.data(), buffer.size());
  auto writer = rapidjson::Writer<rapidjson::FileWriteStream>(os);

  // Use u8string for the buffer to match the input content type
  auto current_content = std::u8string{};
  // Assuming author acts as a unique identifier key, we keep track of it
  auto prev_author =
      std::string_view{}; // Or u8string_view if author is also u8

  // Logic state
  auto is_user_role = true;
  auto first_message = true;

  for (const auto &[author, content] : buddha::get_messages(DB_PATH)) {
    // Check if author changed (skip check on very first message)
    if (!first_message && author != prev_author) {

      // 1. Start JSON structure if we are beginning a "User" block
      if (is_user_role) {
        writer.StartObject();
        writer.Key("messages");
        writer.StartArray();
      }

      // 2. Write the accumulated message
      writer.StartObject();
      writer.Key("role");
      if (is_user_role)
        writer.String("user", 4);
      else
        writer.String("assistant", 9);

      writer.Key("content");
      // CASTING: rapidjson expects const char*, but u8string gives const
      // char8_t*
      writer.String(reinterpret_cast<const char *>(current_content.c_str()),
                    static_cast<rapidjson::SizeType>(current_content.size()));
      writer.EndObject();

      // 3. Logic to close block or switch role
      if (!is_user_role) {
        // We just finished an Assistant block, so close the whole JSON line
        writer.EndArray();
        writer.EndObject();
        os.Put('\n');

        // Next block starts fresh as User
        is_user_role = true;
      } else {
        // We just finished a User block, next is Assistant (within same JSON
        // line)
        is_user_role = false;
      }

      // Clear buffer for the new author
      current_content.clear();
    }

    // Accumulate content
    if (!current_content.empty()) {
      // Append newline using u8 literal to match u8string
      current_content += u8"\n";
    }
    current_content += content;

    prev_author = author;
    first_message = false;
  }

  // Flush the final buffer after the loop
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

auto build_bpe() {
  // clang-format off
  const auto msgs = buddha::get_messages(DB_PATH)
    | std::views::transform([](const auto &pair) { return pair.second; })
    | std::views::join
    | std::ranges::to<std::u8string>();
  // clang-format on
  auto t = buddha::bpe::build(msgs);
  buddha::bpe::print_table(t);
  buddha::bpe::export_table("data/msg.bpe", t);
}

auto import_bpe() {
  auto t = buddha::bpe::import_table("data/msg.bpe");
  buddha::bpe::print_table(t);
}

auto main(int argc, char *argv[]) -> int {
  if (argc <= 2) {
    std::print("Usage: {} <mode>\n", argv[0]);
    return 1;
  }

  auto mode = std::string_view{argv[1]};
  if (mode == "export") {
    auto type = argc == 3 ? std::string_view{argv[2]} : "text";
    if (type == "text")
      export_text("data/msg.txt");
    else if (type == "jsonl")
      export_jsonl("data/msg.jsonl");
    else
      std::println("Unknown export type: {}", type);
  } else if (mode == "bpe")
    build_bpe();
  else if (mode == "import")
    import_bpe();
  else
    std::println("Unknown mode: {}", mode);

  return 0;
}
