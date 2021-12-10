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

    enum TimedEvents {
        TimedEvent_none = 0,
        TimedEvent_load_from_csv,
        TimedEvent_kmeans_fit,
        TimedEvent_save_to_csv,
        TimedEvent_Count
    };
    class Timer {
    public:
        using clock_t = std::chrono::high_resolution_clock;
        using time_point_t = decltype(clock_t::now());

        explicit Timer(TimedEvents event_to_time = TimedEvent_none)
                : m_start(clock_t::now()), m_currently_timing(event_to_time) {
        }

        void stop_and_log() {
            m_timed_events[static_cast<int>(m_currently_timing)] = elapsed();
        }

        void start(TimedEvents event_to_time) {
            assert(static_cast<int>(event_to_time) < static_cast<int>(TimedEvent_Count));
            m_currently_timing = event_to_time;
            m_start = clock_t::now();
        }

        double elapsed() const {
            std::chrono::duration<double> difference = clock_t::now() - m_start;
            return difference.count();
        }

        double elapsed(TimedEvents event) const {
            assert(static_cast<int>(event) < static_cast<int>(TimedEvent_Count));
            return m_timed_events[static_cast<int>(event)];
        }

    private:
        time_point_t m_start;
        time_point_t m_end;
        std::array<double, TimedEvent_Count> m_timed_events;
        TimedEvents m_currently_timing;
    };

}
#endif //MARKO_SPASIC_NI2020_NI_TIMER_H
