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
    std::println(stderr, "Failed to open file: {}", filename.c_str());
    return;
  }

  auto buffer = std::vector<char>(65536);
  auto os = rapidjson::FileWriteStream(fp, buffer.data(), buffer.size());

  auto writer = rapidjson::Writer<rapidjson::FileWriteStream>(os);
  writer.StartObject();
  writer.Key("messages");
  writer.StartArray();

  auto user = true;
  for (const auto &[_, content] : buddha::get_messages(DB_PATH)) {
    writer.StartObject();
    writer.Key("role");
    if (user)
      writer.String("user", 4);
    else
      writer.String("assistant", 9);
    writer.Key("content");
    writer.String(reinterpret_cast<const char *>(content.c_str()),
                  static_cast<rapidjson::SizeType>(content.size()));
    writer.EndObject();

    user = !user;
  }

  writer.EndArray();
  writer.EndObject();

  os.Put('\n');
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
