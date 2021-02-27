#include "logpp/sinks/file/RollingFileSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/sinks/file/RollingOfstream.h"

#include "logpp/utils/env.h"
#include "logpp/utils/string.h"

#include <cstring>

namespace logpp::sink
{

    namespace
    {
        template <typename Func>
        bool parseRolling(const Options::Value& options, Func&& onParsed)
        {
            if (auto opts = options.asDict())
            {
                auto typeIt = opts->find("type");
                if (typeIt == std::end(*opts))
                    return false;

                auto type = typeIt->second;
                if (string_utils::iequals(type, "size"))
                {
                    auto sizeIt = opts->find("size");
                    if (sizeIt == std::end(*opts))
                        return false;

                    auto size = string_utils::parseSize(sizeIt->second);
                    if (!size)
                        return false;

                    onParsed(RollBySize { *size });
                    return true;
                }
                else if (string_utils::iequals(type, "date"))
                {
                    auto intervalIt = opts->find("interval");
                    if (intervalIt == std::end(*opts))
                        return false;

                    auto kindIt = opts->find("kind");
                    if (kindIt == std::end(*opts))
                        return false;

                    bool ok = false;

                    ok = string_utils::parseDuration(intervalIt->second, [&](auto duration) {
                        if (string_utils::iequals(kindIt->second, "precise"))
                        {
                            onParsed(RollEvery { PreciseInterval { duration } });
                            ok = true;
                        }
                        else if (string_utils::iequals(kindIt->second, "round"))
                        {
                            onParsed(RollEvery { RoundInterval { duration } });
                            ok = true;
                        }
                    });

                    return ok;
                }
            }
            return false;
        }

        template <typename Func>
        bool parseArchive(const Options::Value& options, Func&& onParsed)
        {
            if (auto opts = options.asString())
            {
                if (string_utils::iequals(*opts, "incremental"))
                {
                    onParsed(ArchiveIncremental {});
                    return true;
                }
                else if (string_utils::iequals(*opts, "timestamp"))
                {
                    onParsed(ArchiveTimestamp<UTCTime> {});
                    return true;
                }

                return false;
            }
            else if (auto opts = options.asDict())
            {
                auto typeIt = opts->find("type");
                if (typeIt == std::end(*opts))
                    return false;

                auto type = typeIt->second;

                if (string_utils::iequals(type, "incremental"))
                {
                    onParsed(ArchiveIncremental {});
                    return true;
                }
                else if (string_utils::iequals(type, "timestamp"))
                {
                    static constexpr auto DefaultPattern = std::string_view("%Y%m%d");

                    auto patternIt = opts->find("pattern");
                    auto pattern   = patternIt == std::end(*opts) ? std::string(DefaultPattern) : patternIt->second;

                    auto tzIt = opts->find("tz");
                    if (tzIt == std::end(*opts))
                    {
                        onParsed(ArchiveTimestamp<UTCTime> { std::move(pattern) });
                        return true;
                    }

                    auto tz = tzIt->second;
                    if (string_utils::iequals(tz, "utc"))
                    {
                        onParsed(ArchiveTimestamp<UTCTime> { std::move(pattern) });
                        return true;
                    }
                    else if (string_utils::iequals(tz, "local"))
                    {
                        onParsed(ArchiveTimestamp<LocalTime> { std::move(pattern) });
                        return true;
                    }

                    return false;
                }
            }

            return false;
        }

        template <typename Func>
        bool parseRollingAndArchive(const Options& options, Func&& onParsed)
        {
            auto rollingOpts = options.tryGet("strategy");
            auto archiveOpts = options.tryGet("archive");

            if (!rollingOpts || !archiveOpts)
                return false;

            auto parsed = false;
            parseRolling(*rollingOpts, [&](auto rollingStrategy) {
                parseArchive(*archiveOpts, [&](auto archiveStrategy) {
                    onParsed(rollingStrategy, archiveStrategy);
                    parsed = true;
                });
            });

            return parsed;
        }
    }

    class FileImpl : public File
    {
    public:
        template <typename RollingStrategy, typename ArchiveStrategy>
        FileImpl(std::string_view filePath, std::ios_base::openmode openMode, RollingStrategy rollingStrategy, ArchiveStrategy archiveStrategy)
        {
            open(filePath, openMode, rollingStrategy, archiveStrategy);
        }

        template <typename RollingStrategy, typename ArchiveStrategy>
        bool open(std::string_view filePath, std::ios_base::openmode openMode, RollingStrategy rollingStrategy, ArchiveStrategy archiveStrategy)
        {
            if (m_rofs)
                return false;

            auto rofs = std::make_unique<rolling_ofstream>(filePath, openMode, rollingStrategy, archiveStrategy, roll_mode::manual);
            if (rofs->bad() || !rofs->is_open())
                return false;

            std::swap(m_rofs, rofs);
            m_path = filePath;
            return true;
        }

        bool isOpen() const override
        {
            return m_rofs && m_rofs->is_open();
        }

        bool close() override
        {
            if (!m_rofs)
                return false;

            m_rofs->close();
            m_rofs.reset();

            return true;
        }

        size_t write(const char* data, size_t size) override
        {
            if (!m_rofs)
                return 0ULL;

            m_rofs->write(data, size);
            return size;
        }

        size_t write(const char c) override
        {
            if (!m_rofs)
                return 0ULL;

            m_rofs->put(c);
            return 1ULL;
        }

        size_t size() const override
        {
            if (!m_rofs || !m_rofs->rdbuf())
                return 0;

            return m_rofs->rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::out);
        }

        void flush() override
        {
            if (!m_rofs)
                return;

            m_rofs->flush();
        }

        bool canRoll() const
        {
            return m_rofs->can_roll();
        }

        void roll()
        {
            m_rofs->roll();
        }

    private:
        std::unique_ptr<rolling_ofstream> m_rofs;
    };

    RollingFileSink::RollingFileSink()
        : FileSink()
    { }

    RollingFileSink::RollingFileSink(std::string_view baseFilePath, std::shared_ptr<Formatter> formatter)
        : FileSink(baseFilePath, std::move(formatter))
        , m_baseFilePath(baseFilePath)
    { }

    bool RollingFileSink::activateOptions(const Options& options)
    {
        if (!FormatSink::activateOptions(options))
            return false;

        auto fileOption = options.tryGet("file");
        if (!fileOption)
            return false;

        auto file = fileOption->asString();
        if (!file)
            return false;

        bool isOpen = false;

        parseRollingAndArchive(options, [&](auto rollingStrategy, auto archiveStrategy) {
            m_file.reset(new FileImpl(env_utils::expandEnvironmentVariables(*file), std::ios_base::out | std::ios_base::app, rollingStrategy, archiveStrategy));
            isOpen = m_file->isOpen();
        });

        return isOpen;
    }

    void RollingFileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        if (!m_file)
            return;

        auto* file = static_cast<FileImpl*>(m_file.get());
        if (file->canRoll())
        {
            onBeforeClosing(m_file);
            file->roll();
            onAfterOpened(m_file);
        }

        FileSink::sink(name, level, buffer);
    }
}
