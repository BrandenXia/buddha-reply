#ifndef BUDDHA_REPLY_FORMATS_H
#define BUDDHA_REPLY_FORMATS_H

#include <filesystem>

namespace buddha::exports {

auto text(std::filesystem::path filename) -> void;

auto sft_jsonl(std::filesystem::path filename) -> void;

auto dpo_jsonl(std::filesystem::path filename) -> void;

auto bpe(std::filesystem::path filename) -> void;

auto emojis(std::filesystem::path filename) -> void;

} // namespace buddha::exports

#endif
