#include <print>

#include "msg.h"

int main() {
  for (const auto &[time, content] : buddha::get_messages())
    std::println("{}: {}", time, content);
}
