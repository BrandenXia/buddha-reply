#ifndef BUDDHA_REPLY_MSG_H
#define BUDDHA_REPLY_MSG_H

#include <chrono>
#include <filesystem>

#include "cppcoro/generator.hpp"

namespace buddha {

using time_point = std::chrono::system_clock::time_point;

auto get_messages(std::filesystem::path db_path)
    -> cppcoro::generator<std::pair<std::string, std::u8string>>;

} // namespace buddha

#endif
