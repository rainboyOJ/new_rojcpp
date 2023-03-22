#pragma once

#include <string>
#include <chrono>

namespace LOGGER {
    

class TimeStamp {
    using microsecondsTimepoint = std::chrono::time_point<std::chrono::system_clock,std::chrono::microseconds>;

    public:
        // 1秒=1000*1000微妙
        static const int kMicroSecondsPerSecond = 1000 * 1000;
        TimeStamp() : m_time_point(std::chrono::microseconds(0))
        {}

        explicit TimeStamp(std::size_t microSecondSinceEpoch) 
            : m_time_point(std::chrono::microseconds(microSecondSinceEpoch))
        {}

        explicit TimeStamp(microsecondsTimepoint timePoint)
            :m_time_point(timePoint)
        {}


        //用std::string形式返回,格式[millisec].[microsec]
        std::string toString() const;

        //格式, "%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",时分秒.微秒
        std::string toFormattedString(bool showMicroseconds = false) const 
        {
            char buf[64];
            time_t seconds = static_cast<time_t>(m_time_point.time_since_epoch().count() / kMicroSecondsPerSecond);
            // 使用localtime函数将秒数格式化成日历时间
            tm *tm_time = localtime(&seconds);
            if (showMicroseconds)
            {
                int microseconds = static_cast<int>(m_time_point.time_since_epoch().count() % kMicroSecondsPerSecond);
                snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
                        tm_time->tm_year + 1900,
                        tm_time->tm_mon + 1,
                        tm_time->tm_mday,
                        tm_time->tm_hour,
                        tm_time->tm_min,
                        tm_time->tm_sec,
                        microseconds);
            }
            else
            {
                snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
                        tm_time->tm_year + 1900,
                        tm_time->tm_mon + 1,
                        tm_time->tm_mday,
                        tm_time->tm_hour,
                        tm_time->tm_min,
                        tm_time->tm_sec);
            }
            return buf;
        }

        static TimeStamp now() {

            auto now = std::chrono::system_clock::now();
            return TimeStamp( std::chrono::time_point_cast<std::chrono::microseconds>(now) );

        }

        //返回当前时间戳的微妙
        std::size_t microSecondsSinceEpoch() const {
            return  m_time_point.time_since_epoch().count();
        }

        //返回当前时间戳的秒数
        std::size_t secondsSinceEpoch() const {
            return  std::chrono::time_point_cast<std::chrono::seconds>(m_time_point).time_since_epoch().count();
        }

        // 失效的时间戳，返回一个值为0的Timestamp
        static TimeStamp invalid() {
            return TimeStamp();
        }

        bool operator<(const TimeStamp & other) {
            return m_time_point < other.m_time_point;
        }

        bool operator==(const TimeStamp & other) {
            return  m_time_point == other.m_time_point;
        }

        TimeStamp addTime(double seconds) {
            long long delta = static_cast<long long>(seconds * TimeStamp::kMicroSecondsPerSecond);
            return TimeStamp( m_time_point + std::chrono::microseconds(delta));
        }

    private:
        microsecondsTimepoint m_time_point;
};




} // end namespace LOGGER
