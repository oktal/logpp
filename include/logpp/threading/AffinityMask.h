#pragma once

#include <bitset>

namespace logpp::threading
{
    template<size_t N>
    using AffinityMaskN = std::bitset<N>;

#if defined(LOGPP_CORES)
    using AffinityMask = AffinityMaskN<LOGPP_CORES>;
#else
    using AffinityMask = AffinityMaskN<64>;
#endif
}