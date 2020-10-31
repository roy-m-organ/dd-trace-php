#ifndef DDPROF_PROFILER_HH
#define DDPROF_PROFILER_HH

#include <vector>

#include "collector.hh"
#include "exporter.hh"
#include "scheduler.hh"

namespace ddprof {
class profiler {
   public:
    enum class status : unsigned {
        stopped = 0,
        running = 1,
    };

    void start() {
        for (auto &collector : collectors) {
            collector.start();
        }
        scheduler.start();

        status = status::running;
    }

    void stop(bool flush = true) {
        scheduler.stop();

        for (auto it = collectors.rbegin(); it != collectors.rend(); ++it) {
            it->stop();
        }

        if (flush) {
            scheduler.join();
        }

        status = status::stopped;
    }

   private:
    std::vector<collector> collectors;
    std::vector<exporter *> exporters;
    class scheduler scheduler;
    enum status status;
};
}  // namespace ddprof

#endif  // DDPROF_PROFILER_HH
