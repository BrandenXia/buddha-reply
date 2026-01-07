#ifndef BUDDHA_REPLY_FORMATS_H
#define BUDDHA_REPLY_FORMATS_H

#include <filesystem>

#include "re2/re2.h"

namespace buddha::exports {

auto text(std::filesystem::path filename) -> void;

auto sft_jsonl(std::filesystem::path filename) -> void;

auto dpo_jsonl(std::filesystem::path filename) -> void;

auto bpe(std::filesystem::path filename) -> void;

auto emojis(std::filesystem::path filename) -> void;

namespace regex {

inline const auto emoji_regex = re2::RE2(R"(<a?(:.+?:)\d{19}>)");

inline const auto mention_regex = re2::RE2(R"((<@\d{19}>))");

} // namespace regex

} // namespace buddha::exports

#endif
