#include <stddef.h>
#include <stdint.h>

#include "FuzzStream.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    logpp::EventLogBuffer buffer;
    // TODO: Fuzz fields
    logpp::fuzz::FuzzStream stream { data, size };
    stream.fill(buffer);
    return 0;
}
