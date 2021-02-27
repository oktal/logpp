#pragma once

#include <chrono>

#include <filesystem>
#include <fstream>

#include <memory>
#include <optional>

// `basic_rolling_ofstream` is an ofstream that extends the `std::basic_ofstream` by adding
// rolling capabilities
//
// `basic_rolling_ofstream` is based on two policies:
//   - `RollingPolicy` will determine _when_ to roll the file
//   - `ArchivePolicy` will determine _how_ to roll the file
//
// Currently supported rolling policies are:
//   - `RollBySize` will roll the file after reaching a threshold in bytes
//   - `RollEvery` will roll the file every n (minutes, hours, ...)
// The `RollEvery` policy supports two kind of rolling intervals:
//   - `PreciseInterval` will roll the file with an exact interval.
//     For example, rolling a file every minute, starting at 10:01:25 will yield the following rolling times:
//        10:02:25
//        10:03:25
//        10:04:25
//        ...
//   - `RoundInterval` will round down the interval.
//     For example, rolling a file every minute, starting at 10:01:25 will yield the following rolling times:
//       10:03:00
//       10:04:00
//       10:05:00
//       ...
//
// Currently supported archive policies are:
//   - `ArchiveIncremental` will add an incrementing suffix.
//     For example, archiving a file named "test.log" will yield the following files:
//       test.log.0
//       test.log.1
//       test.log.2
//     with an ascending time order (0 being the most recent)
//   - `ArchiveTimestamp` will add a time suffix, provided through a strftime-compatible time format.
//     The `ArchiveTimestamp` will fallback to the `ArchiveIncremental` if the target file already exists

// In `roll_mode::automatic` mode, the file will automatically roll without any action from the user.
// In `roll_mode::manual`, the user can check if the file can roll with `rolling_ofstream::can_roll()` and must
// manually roll the file with `rolling_ofstream::roll()`
//
// Usage examples
//
//  ```
//      rolling_ofstream ofs(
//          "test.log",
//          std::ios_base::out,
//          RollBySize{100 * 1024 * 1024},
//          ArchiveIncremental{},
//          roll_mode::automatic
//      );
//  ```
//
// Will roll after reaching 100MiB and archive files with an incrementing suffix
//
//
//  ```
//      rolling_ofstream ofs(
//          "test.log",
//          std::ios_base::out,
//          RollEvery{RoundInterval{std::chrono::hours(1)}},
//          ArchiveTimestamp<LocalTime>{"%Y%m%d%%H%M%S},
//          roll_mode::automatic
//      );
//  ```
// Will roll the file every hour (rounded), adding a time-formatted suffix expressed as local time

namespace logpp
{

    template <typename CharT>
    class basic_rolling_ofstream;

    // basic_rolling_filebuf that is used as a std::streambuf by basic_rolling_ofstream

    template <typename CharT>
    class rolling_filebuf_base : public std::basic_filebuf<CharT>
    {
    public:
        virtual ~rolling_filebuf_base() = default;

        void path(std::string_view path)
        {
            m_path = std::string(path);
        }

        void mode(std::ios_base::openmode mode)
        {
            m_mode = mode;
        }

        std::string_view path() const
        {
            return m_path;
        }

        std::ios_base::openmode mode() const
        {
            return m_mode;
        }

        virtual bool can_roll() = 0;
        virtual void roll()     = 0;

    private:
        std::string m_path;
        std::ios_base::openmode m_mode;
    };

    enum class roll_mode {
        automatic,
        manual
    };

    template <typename CharT, typename RollingPolicy, typename ArchivePolicy>
    class basic_rolling_filebuf : public rolling_filebuf_base<CharT>
    {
    public:
        using Base      = rolling_filebuf_base<CharT>;
        using char_type = typename Base::char_type;

        basic_rolling_filebuf(RollingPolicy rollingPolicy, ArchivePolicy archivePolicy, roll_mode mode)
            : m_rollingPolicy(rollingPolicy)
            , m_archivePolicy(archivePolicy)
            , m_mode(mode)
        {
        }

        bool can_roll() override
        {
            return m_rollingPolicy.can_roll(this);
        }

        void roll() override
        {
            m_rollingPolicy.apply(this);
            m_archivePolicy.apply(this);
        }

    protected:
        std::streamsize xsputn(const char_type* s, std::streamsize count) override
        {
            if (m_rollingPolicy.apply(this) && m_mode == roll_mode::automatic)
            {
                Base::pubsync();
                m_archivePolicy.apply(this);
            }

            return Base::xsputn(s, count);
        }

    private:
        RollingPolicy m_rollingPolicy;
        ArchivePolicy m_archivePolicy;
        roll_mode m_mode;
    };

    // Collection of utilities

    // File utilities

    namespace file_utils
    {
        inline bool rename(std::string_view from, std::string_view to)
        {
            try
            {
                std::filesystem::rename(from, to);
                return true;
            }
            catch (const std::filesystem::filesystem_error&)
            {
                return false;
            }
        }
    }

    // streambuf utilities

    namespace buf_utils
    {
        template <typename CharT>
        void set_rdbuf(std::basic_ios<CharT>* ios, std::basic_streambuf<CharT>* buf)
        {
            ios->rdbuf(buf);
        }
    }

    // basic_rolling_ofstream

    template <typename CharT>
    class basic_rolling_ofstream : public std::basic_ostream<CharT>
    {
    public:
        using Base = std::basic_ostream<CharT>;

        template <typename RollingPolicy, typename ArchivePolicy>
        basic_rolling_ofstream(RollingPolicy rollingPolicy, ArchivePolicy archivePolicy, roll_mode mode)
            : Base(new basic_rolling_filebuf<CharT, RollingPolicy, ArchivePolicy>(rollingPolicy, archivePolicy, mode))
        {
            m_buf.reset(static_cast<basic_rolling_filebuf<CharT, RollingPolicy, ArchivePolicy>*>(Base::rdbuf()));
        }

        template <typename RollingPolicy, typename ArchivePolicy>
        basic_rolling_ofstream(
            const char* fileName,
            std::ios_base::openmode openMode,
            RollingPolicy rollingPolicy,
            ArchivePolicy archivePolicy,
            roll_mode mode)
            : basic_rolling_ofstream(rollingPolicy, archivePolicy, mode)
        {
            open(fileName, openMode);
        }

        template <typename RollingPolicy, typename ArchivePolicy>
        basic_rolling_ofstream(
            const std::string& fileName,
            std::ios_base::openmode openMode,
            RollingPolicy rollingPolicy,
            ArchivePolicy archivePolicy,
            roll_mode rollMode)
            : basic_rolling_ofstream(fileName.c_str(), openMode, rollingPolicy, archivePolicy, rollMode)
        {
        }

        template <typename RollingPolicy, typename ArchivePolicy>
        basic_rolling_ofstream(
            std::string_view fileName,
            std::ios_base::openmode openMode,
            RollingPolicy rollingPolicy,
            ArchivePolicy archivePolicy,
            roll_mode rollMode)
            : basic_rolling_ofstream(fileName.data(), openMode, rollingPolicy, archivePolicy, rollMode)
        {
        }

        bool is_open() const
        {
            return m_buf->is_open();
        }

        void open(const char* fileName, std::ios_base::openmode mode = std::ios_base::out)
        {
            m_buf->open(fileName, mode);
            m_buf->path(fileName);
            m_buf->mode(mode);
        }

        void open(const std::string& fileName, std::ios_base::openmode mode = std::ios_base::out)
        {
            open(fileName.c_str(), mode);
        }

        void close()
        {
            m_buf->close();
        }

        bool can_roll()
        {
            return m_buf->can_roll();
        }

        void roll()
        {
            m_buf->roll();
        }

    private:
        std::unique_ptr<rolling_filebuf_base<CharT>> m_buf;
    };

    // -----------------
    // Rolling policies
    // -----------------

    struct RollBySize
    {
        size_t bytesThreshold;

        template <typename CharT>
        bool can_roll(std::basic_filebuf<CharT>* filebuf)
        {
            auto pos = filebuf->pubseekoff(0, std::ios_base::cur, std::ios_base::out);
            return pos < 0 ? false : static_cast<size_t>(pos) >= bytesThreshold;
        }

        template <typename CharT>
        bool apply(std::basic_filebuf<CharT>* filebuf)
        {
            return can_roll(filebuf);
        }
    };

    template <typename Duration>
    struct PreciseInterval
    {
        Duration value;

        template <typename TimePoint>
        TimePoint next(TimePoint tp) const
        {
            return tp + value;
        }
    };

    template <typename Rep, typename Period>
    PreciseInterval(std::chrono::duration<Rep, Period>) -> PreciseInterval<std::chrono::duration<Rep, Period>>;

    template <typename Duration>
    struct RoundInterval
    {
        using Rep    = typename Duration::rep;
        using Period = typename Duration::period;

        Duration value;

        explicit RoundInterval(Duration value)
            : value { value }
        {
        }

        template <typename TimePoint>
        TimePoint next(TimePoint tp) const
        {
            return std::chrono::floor<std::chrono::duration<Rep, Period>>(tp + value);
        }
    };

    template <typename Rep, typename Period>
    RoundInterval(std::chrono::duration<Rep, Period>) -> RoundInterval<std::chrono::duration<Rep, Period>>;

    template <typename RollInterval, typename Clock = std::chrono::system_clock>
    struct RollEvery
    {
        RollInterval interval;
        std::optional<std::chrono::system_clock::time_point> nextRollPoint;

        explicit RollEvery(const RollInterval& interval, Clock = Clock {})
            : interval(interval)
        {
        }

        template <typename CharT>
        bool can_roll(const std::basic_filebuf<CharT>*)
        {
            if (!nextRollPoint)
                return false;

            auto now = Clock::now();
            return now >= *nextRollPoint;
        }

        template <typename CharT>
        bool apply(std::basic_filebuf<CharT>*)
        {
            auto now = Clock::now();

            if (!nextRollPoint)
                nextRollPoint = interval.next(now);

            if (now >= *nextRollPoint)
            {
                nextRollPoint = interval.next(now);
                return true;
            }

            return false;
        }
    };

    // -----------------
    // Archive policies
    // -----------------

    struct ArchiveIncremental
    {
        template <typename CharT>
        bool apply(rolling_filebuf_base<CharT>* buf) const
        {
            auto basePath = buf->path();
            auto mode     = buf->mode();

            buf->close();

            archive(basePath);
            return buf->open(basePath.data(), mode);
        }

        static int archive(std::string_view basePath)
        {
            auto formatPath = [&](std::string_view name, int n) {
                static constexpr size_t MAX_PATH_SIZE = 255;
                char pathBuf[MAX_PATH_SIZE];
                auto len = std::snprintf(pathBuf, sizeof(pathBuf), "%s.%d", name.data(), n);
                return std::string(pathBuf, len);
            };

            std::string path;
            int n = -1;
            do
            {
                path = formatPath(basePath, ++n);
            } while (std::filesystem::exists(path));

            auto index = n;

            while (n >= 0)
            {
                auto oldPath = n > 0 ? formatPath(basePath, n - 1) : std::string(basePath);
                auto newPath = formatPath(basePath, n);
                file_utils::rename(oldPath, newPath);
                --n;
            }

            return index;
        }
    };

    struct UTCTime
    {
        static std::tm* now()
        {
            auto tp   = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(tp);

            return std::gmtime(&time);
        }
    };

    struct LocalTime
    {
        static std::tm* now()
        {
            auto tp   = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(tp);

            return std::localtime(&time);
        }
    };

    template <typename Time>
    struct ArchiveTimestamp
    {
        static constexpr auto DefaultPattern = std::string_view("%Y%m%d");

        std::string pattern { DefaultPattern };

        template <typename CharT>
        bool apply(rolling_filebuf_base<CharT>* buf) const
        {
            if (pattern.empty())
                return false;

            auto basePath = buf->path();
            auto mode     = buf->mode();

            static constexpr size_t MAX_BUF = 255;

            buf->close();

            auto tm = Time::now();
            char timeBuf[MAX_BUF];
            auto len = std::strftime(timeBuf, sizeof(timeBuf), pattern.c_str(), tm);

            auto newPath = std::string(basePath);
            newPath.push_back('.');
            newPath.append(timeBuf, len);

            if (std::filesystem::exists(newPath))
                ArchiveIncremental::archive(newPath);

            if (!file_utils::rename(basePath, newPath))
                return false;

            return buf->open(basePath.data(), mode);
        }
    };

    using rolling_ofstream = basic_rolling_ofstream<char>;
}
