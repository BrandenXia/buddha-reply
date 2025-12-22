#ifndef BUDDHA_REPLY_MSG_H
#define BUDDHA_REPLY_MSG_H

#include <chrono>

#include "cppcoro/generator.hpp"

namespace buddha {

inline constexpr std::string_view db_path = "data/msg.sqlite";

using time_point = std::chrono::system_clock::time_point;

cppcoro::generator<std::pair<time_point, std::string>> get_messages();

} // namespace buddha

#endif
