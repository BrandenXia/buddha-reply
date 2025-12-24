#include <fstream>
#include <print>
#include <string_view>

#include "msg.h"

void export_clean(std::string_view filename) {
  auto f = std::ofstream(filename.data());

  auto cnt = 0;
  for (const auto &[time, content] : buddha::get_messages()) {
    f << content << "\n\n";
    cnt++;
  }
  f.close();

  std::println("Finished exporting {} messages to {}", cnt, filename);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::print("Usage: {} <mode>\n", argv[0]);
    return 1;
  }

  auto mode = std::string_view{argv[1]};
  if (mode == "export")
    export_clean("data/msg.txt");
  else
    std::println("Unknown mode: {}", mode);
}
