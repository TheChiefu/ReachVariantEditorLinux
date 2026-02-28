#pragma once
#include <cstddef>
#include <cstdint>

namespace cobb {
   void memswap(void* a, void* b, std::size_t size) noexcept;
}
