//
// Created by spaske on 28.11.21..
//

#ifndef MARKO_SPASIC_NI2020_NI_TIMER_H
#define MARKO_SPASIC_NI2020_NI_TIMER_H

#include<string>
namespace ni::logging {

    template<typename ChronoDuration>
    constexpr std::string_view get_duration_postfix() {
        if constexpr (std::is_same_v<ChronoDuration, std::chrono::nanoseconds>) {
            return "ns";
        } else if constexpr(std::is_same_v<ChronoDuration, std::chrono::microseconds>) {
            return "us";
        } else if constexpr (std::is_same_v<ChronoDuration, std::chrono::milliseconds>) {
            return "ms";
        } else if constexpr (std::is_same_v<ChronoDuration, std::chrono::seconds>) {
            return "s";
        } else if constexpr (std::is_same_v<ChronoDuration, std::chrono::minutes>) {
            return "m";
        }
    }

    class Timer {
    public:
        using duration_t = std::chrono::milliseconds;
        using clock_t = std::chrono::high_resolution_clock;
        using time_point_t = decltype(clock_t::now());

        explicit Timer(std::string_view name = "")
                : m_start(clock_t::now()) {
            m_log.reserve(4096);
        }

        void stop_and_log() {
            m_end = clock_t::now();
            m_log.append(m_timed_block_name);
            m_log.push_back(':');
            m_log.append(std::to_string(std::chrono::duration_cast<duration_t>(m_end - m_start).count()));
            m_log.push_back('\n');
        }

        void start(std::string_view timed_block_name) {
            m_timed_block_name = timed_block_name;
            m_start = clock_t::now();
        }

        auto elapsed() const {
            return std::chrono::duration_cast<duration_t>(clock_t::now() - m_start).count();
        }

        const std::string& get_log() const { return m_log; }
    private:
        std::string m_log;
        time_point_t m_start;
        time_point_t m_end;
        std::string_view m_timed_block_name;
    };

}
#endif //MARKO_SPASIC_NI2020_NI_TIMER_H
