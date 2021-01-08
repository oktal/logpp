#pragma once

#include "logpp/sinks/Sink.h"

#include <ostream>

namespace logpp::sink
{
    class LogFmt : public Sink
    {
    public:
        explicit LogFmt(std::ostream &os);
        void format(std::string_view name, LogLevel level, EventLogBuffer buffer) override;

    private:
        std::ostream& m_os;
    };
}