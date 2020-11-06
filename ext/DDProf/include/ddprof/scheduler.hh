#ifndef DDPROF_SCHEDULER_HH
#define DDPROF_SCHEDULER_HH

#include <vector>

extern "C" {
#include <pthread.h>
};

#include "chrono.hh"
#include "exporter.hh"
#include "recorder.hh"
#include "service.hh"

namespace ddprof {

class scheduler : public periodic_service {
    using nanoseconds = std::chrono::nanoseconds;
    class recorder &recorder;
    std::vector<std::unique_ptr<exporter>> &exporters;
    std::chrono::nanoseconds configured_interval;
    system_clock::time_point last_export;

   public:
    scheduler(class recorder &r, std::vector<std::unique_ptr<exporter>> &exporters, nanoseconds cfg_interval) noexcept;
    ~scheduler() override;

    void on_start() noexcept override;
    void on_stop() noexcept override;

    void periodic() override;
};

inline scheduler::scheduler(class recorder &r, std::vector<std::unique_ptr<exporter>> &exporters,
                            nanoseconds cfg_interval) noexcept
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

namespace {
// todo: full license info. This technique comes from GDB.
template <typename R, typename A1, typename A2>
void set_thread_name(R (*set_name)(A1, A2), const char *name) {
    set_name(pthread_self(), name);
}

template <typename R, typename A1>
void set_thread_name(R (*set_name)(A1), const char *name) {
    set_name(name);
}
}  // namespace

inline void scheduler::on_start() noexcept {
    const char name[16] = "ddprof::sched";
    set_thread_name(pthread_setname_np, name);
    last_export = system_clock::now();
}

inline void scheduler::on_stop() noexcept { periodic(); }

inline scheduler::~scheduler() = default;

}  // namespace ddprof

#endif  // DDPROF_SCHEDULER_HH
