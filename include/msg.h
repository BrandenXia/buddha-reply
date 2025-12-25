#ifndef BUDDHA_REPLY_MSG_H
#define BUDDHA_REPLY_MSG_H

#include <chrono>

#include "cppcoro/generator.hpp"

namespace buddha {

inline constexpr auto db_path = "data/msg.sqlite";

using time_point = std::chrono::system_clock::time_point;

auto get_messages() -> cppcoro::generator<std::pair<time_point, std::u8string>>;

} // namespace buddha

#endif
