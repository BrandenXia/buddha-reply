#ifndef BUDDHA_REPLY_BPE_H
#define BUDDHA_REPLY_BPE_H

#include <string_view>
#include <vector>

namespace buddha::bpe {

using Token = std::uint32_t;
using Pair = std::pair<Token, Token>;
using Table = std::vector<Pair>;

auto build(std::u8string_view raw) -> Table;

auto print_table(const Table &table) -> void;

auto export_table(const Table &table, std::string_view filename) -> void;

auto import_table(std::string_view filename) -> Table;

} // namespace buddha::bpe

#endif
