#include "logpp/core/LogBufferView.h"
#include "logpp/core/LogBuffer.h"

namespace logpp
{
    const char* LogBufferView::read(size_t index) const
    {
        return buffer.dataAt(index);
    }
}