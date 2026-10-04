#include "aegis/kv/crc32.h"
#include <array>

namespace aegis::kv {

namespace {
std::array<uint32_t, 256> make_table() {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}
}  // namespace

uint32_t crc32(const std::string& data) {
    static const auto table = make_table();
    uint32_t c = 0xFFFFFFFFu;
    for (unsigned char ch : data) {
        c = table[(c ^ ch) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFu;
}

}  // namespace aegis::kv