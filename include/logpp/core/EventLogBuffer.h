#pragma once

#include "logpp/core/LogBuffer.h"
#include "logpp/core/LogWriter.h"

#include <cstddef>

namespace logpp
{
    class EventLogBuffer : public LogBuffer< 255 >
    {
    public:
        using LogFunc = void (*)(const LogBufferBase& buffer, uint16_t offsetsIndex, LogWriter& writer);
        static constexpr size_t HeaderOffset = 0;

        struct Header
        {
            LogFunc logFunc;
            uint16_t offsetsIndex;
        };

        EventLogBuffer()
        {
            advance(HeaderOffset + sizeof(Header));
        }

        template< typename Event >
        void writeEvent(const Event& event)
        {
            auto offsetsIndex = encode(event);
            encodeHeader< Event >(offsetsIndex);
        }

        void format(LogWriter& writer) const
        {
            const auto* header = decodeHeader();
            std::invoke(header->logFunc, *this, header->offsetsIndex, writer);
        }

    private:
        template< typename Event >
        void encodeHeader(size_t offsetsIndex)
        {
            auto* header         = overlayAt< Header >(HeaderOffset);
            header->offsetsIndex = static_cast< uint16_t >(offsetsIndex);
            header->logFunc      = [](const LogBufferBase& buffer, uint16_t offsetsIndex, LogWriter& writer)
            {
                LogBufferView view{buffer};
                const Event* event = view.overlayAs< Event >(offsetsIndex);
                event->format(view, writer);
            };
        }

        const Header* decodeHeader() const
        {
            return overlayAt< Header >(HeaderOffset);
        }
    };
}