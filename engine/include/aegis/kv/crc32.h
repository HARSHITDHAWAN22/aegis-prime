#pragma once
#include <cstdint>
#include <string>

namespace aegis::kv {

// Standard CRC32 (IEEE 802.3 polynomial), used to detect corrupt or
// torn WAL records.
uint32_t crc32(const std::string& data);

}  // namespace aegis::kv