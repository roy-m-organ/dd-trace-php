#ifndef DDPROF_SCHEDULER_HH
#define DDPROF_SCHEDULER_HH

#include <vector>

#include "chrono.hh"
#include "exporter.hh"
#include "recorder.hh"
#include "service.hh"

namespace ddprof {

class scheduler : public periodic_service {
    using nanoseconds = std::chrono::nanoseconds;
    class recorder &recorder;
    std::vector<exporter *> exporters;
    std::chrono::nanoseconds configured_interval;
    system_clock::time_point last_export;

   public:
    scheduler(class recorder &r, std::vector<exporter *> &exporters, nanoseconds cfg_interval) noexcept;
    virtual ~scheduler();

    virtual void start() noexcept;
    virtual void stop() noexcept;

    virtual void periodic();
};

inline scheduler::scheduler(class recorder &r, std::vector<exporter *> &exporters, nanoseconds cfg_interval) noexcept
    : recorder{r}, exporters{exporters}, configured_interval{cfg_interval} {}

inline void scheduler::periodic() {
    auto start_time = steady_clock::now();
    if (!exporters.empty()) {
        auto prev_last_export = last_export;
        last_export = system_clock::now();
        auto [event_table, strings] = recorder.release();
        for (auto &exporter : exporters) {
            (*exporter)(event_table, strings, prev_last_export, last_export);
        }
    }
    auto stop_time = steady_clock::now();
    interval = std::max(nanoseconds(0), configured_interval - (stop_time - start_time));
}

inline void scheduler::start() noexcept {
    // todo: log scheduler start-up
    periodic_service::start();
    last_export = system_clock::now();
}

inline void scheduler::stop() noexcept {
    // todo: log scheduler stop
    periodic_service::stop();
    last_export = system_clock::now();
}

inline scheduler::~scheduler() = default;

}  // namespace ddprof

#endif  // DDPROF_SCHEDULER_HH
