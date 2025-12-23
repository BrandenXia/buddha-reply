#include <fstream>
#include <print>

#include "msg.h"

int main() {
  auto f = std::ofstream("data/msg.txt");

  for (const auto &[time, content] : buddha::get_messages())
    f << content << "\n\n";
  f.close();

  std::print("Done!\n");
}
