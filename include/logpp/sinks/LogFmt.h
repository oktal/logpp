#pragma once

#include "logpp/sinks/Sink.h"

#include <ostream>

namespace logpp::sink
{
    class LogFmt : public Sink
    {
    public:
        static constexpr std::string_view Name = "LogFmt";

        LogFmt();
        explicit LogFmt(std::ostream &os);

        bool setOption(std::string key, std::string value) override;

        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override;
    private:
        std::ostream& m_os;
    };
}