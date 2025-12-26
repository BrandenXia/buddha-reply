#include <fstream>
#include <print>
#include <ranges>
#include <string_view>

#include "bpe.h"
#include "msg.h"

#define DB_PATH "data/msg.sqlite"

auto export_clean(std::string_view filename) {
  auto f = std::ofstream(filename.data());

  auto cnt = 0;
  for (const auto &[time, content] : buddha::get_messages(DB_PATH)) {
    f << reinterpret_cast<const char *>(content.c_str()) << "\n\n";
    cnt++;
  }
  f.close();

  std::println("Finished exporting {} messages to {}", cnt, filename);
}

auto bpe() {
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
  if (argc != 2) {
    std::print("Usage: {} <mode>\n", argv[0]);
    return 1;
  }

  auto mode = std::string_view{argv[1]};
  if (mode == "export")
    export_clean("data/msg.txt");
  else if (mode == "bpe")
    bpe();
  else if (mode == "import")
    import_bpe();
  else
    std::println("Unknown mode: {}", mode);
}
