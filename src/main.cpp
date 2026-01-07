#include <filesystem>
#include <print>
#include <string_view>

#include "bpe.h"
#include "formats.h"

using namespace buddha;

auto import_bpe() {
  auto t = bpe::import_table("data/msg.bpe");
  bpe::print_table(t);
}

auto main(int argc, char *argv[]) -> int {
  if (argc < 2) {
    std::print("Usage: {} <mode>\n", argv[0]);
    return 1;
  }

  auto mode = std::string_view{argv[1]};
  if (mode == "export") {
    auto type = argc == 3 ? std::string_view{argv[2]} : "text";
    if (type == "text")
      exports::text("data/msg.txt");
    else if (type == "sft-jsonl")
      exports::sft_jsonl("data/msg.jsonl");
    else if (type == "bpe")
      exports::bpe("data/msg.bpe");
    else
      std::println("Unknown export type: {}", type);
  } else if (mode == "import")
    import_bpe();
  else
    std::println("Unknown mode: {}", mode);

  return 0;
}
