#pragma once
#include <cstdint>

namespace cobb {
   constexpr uint32_t fourcc(const char (&value)[5]) noexcept {
      return
         (static_cast<uint32_t>(static_cast<uint8_t>(value[0])) << 24) |
         (static_cast<uint32_t>(static_cast<uint8_t>(value[1])) << 16) |
         (static_cast<uint32_t>(static_cast<uint8_t>(value[2])) <<  8) |
         (static_cast<uint32_t>(static_cast<uint8_t>(value[3])) <<  0);
   }
}
