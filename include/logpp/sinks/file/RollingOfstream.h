#pragma once

#include "logpp/utils/date.h"

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
        using Rep    = typename Duration::rep;
        using Period = typename Duration::period;

        Duration value;

        template <typename TimePoint>
        TimePoint next(TimePoint now) const
        {
            return now + value;
        }

        template <typename Clock>
        static TimePoint offsetUtc(TimePoint tp)
        {
            return tp;
        }

        template <typename Clock>
        static TimePoint local(TimePoint tp)
        {
            return tp;
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
        TimePoint next(TimePoint now) const
        {
            return date_utils::floor<std::chrono::duration<Rep, Period>>(now + value);
        }

        template <typename Clock>
        static TimePoint offsetUtc(TimePoint tp)
        {
            // Let's suppose that the time point returned by Clock::now() is
            // YYYY:MM:dd 14:39:00. If we ask for the next rolling time point,
            // rounded by day, we will get back YYYY:MM:(dd+1) 00:00:00, that
            // is midnight. However, here, we "lose" the time zone when rounding
            // the time point, since rounding a duration does not care about time
            // and offset at all.
            //
            // Now, let's suppose that our current time-zone is UTC +02:00, e.g (Europe/Paris).
            // YYYY:MM:(dd+1) 00:00:00 UTC is actually YYYY:MM:(dd+1) 02:00:00 local time, which
            // means that we will roll our file at 02:00 AM local time, and not midnight.
            // If we want to roll our file at midnight local-time, we actually need to roll it
            // at YYYY:MM:dd 22:00:00 UTC and not YYYY:MM:(dd+1) 00:00:00 UTC
            //
            // That is why we adjust the resulting timestamp back to UTC, as if it was
            // actually expressed as local-time.
            //
            // We only do that for periods > 1 day.
            static constexpr auto HasOffset
                = std::ratio_greater_equal_v<Period, date::days::period>;

            if constexpr (HasOffset)
            {
                auto tt = Clock::to_time_t(tp);
                std::tm tm;
                date_utils::gmtime(&tt, &tm);
                tm.tm_isdst = -1;

                auto utcTime = std::mktime(&tm);
                std::tm utcTm;
                date_utils::gmtime(&utcTime, &utcTm);
                return Clock::from_time_t(date_utils::timegm(&utcTm));
            }
            else
            {
                return tp;
            }
        }

        template <typename Clock>
        static TimePoint local(TimePoint tp)
        {
            static constexpr auto HasOffset
                = std::ratio_greater_equal_v<Period, date::days::period>;

            if constexpr (HasOffset)
            {
                auto tt = Clock::to_time_t(tp);
                std::tm tm;
                date_utils::localtime(&tt, &tm);
                return Clock::from_time_t(date_utils::timegm(&tm));
            }
            else
            {
                return tp;
            }
        }
    };

    template <typename Rep, typename Period>
    RoundInterval(std::chrono::duration<Rep, Period>) -> RoundInterval<std::chrono::duration<Rep, Period>>;

    template <typename Interval, typename Clock = std::chrono::system_clock>
    struct RollEvery
    {
        Interval interval;
        std::optional<std::chrono::system_clock::time_point> nextRollPoint;

        explicit RollEvery(Interval interval, Clock = Clock {})
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
                nextRollPoint = next(now);

            if (now >= *nextRollPoint)
            {
                nextRollPoint = next(now);
                return true;
            }

            return false;
        }

    private:
        TimePoint next(TimePoint tp)
        {
            return offsetUtc(interval.next(local(tp)));
        }

        static TimePoint offsetUtc(TimePoint tp)
        {
            return Interval::template offsetUtc<Clock>(tp);
        }

        static TimePoint local(TimePoint tp)
        {
            return Interval::template local<Clock>(tp);
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
        static void now(std::tm* out)
        {
            auto tp   = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(tp);

            date_utils::gmtime(&time, out);
        }
    };

    struct LocalTime
    {
        static void now(std::tm* out)
        {
            auto tp   = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(tp);

            date_utils::localtime(&time, out);
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

            std::tm tm;
            Time::now(&tm);
            char timeBuf[MAX_BUF];
            auto len = std::strftime(timeBuf, sizeof(timeBuf), pattern.c_str(), &tm);

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
