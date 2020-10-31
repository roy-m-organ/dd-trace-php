#include "ddprof/scheduler.hh"

#include <catch2/catch.hpp>

TEST_CASE("scheduler basics", "[scheduler]") {
    class test_exporter : public ddprof::exporter {
       public:
        bool &exported;
        test_exporter(bool &b) : exported{b} {}

        void operator()(const ddprof::recorder::event_table_t &event_table, const ddprof::string_table &strings,
                        ddprof::system_clock::time_point start_time,
                        ddprof::system_clock::time_point stop_time) override {
            exported = true;
        }
        ~test_exporter() noexcept override = default;
    };

    bool exported = false;

    {
        test_exporter exporter{exported};
        auto interval = std::chrono::milliseconds(10);
        ddprof::recorder recorder;
        std::vector<ddprof::exporter *> exporters = {&exporter};

        ddprof::scheduler scheduler{recorder, exporters, interval};
        scheduler.start();
        while (!scheduler.has_started()) {
            std::this_thread::sleep_for(interval);
        }

        auto event = new ddprof::event(ddprof::basic_event{0, ddprof::system_clock::now()});
        recorder.push(std::unique_ptr<ddprof::event>(event));

        std::this_thread::sleep_for(interval);

        scheduler.stop();
        scheduler.join();
    }

    REQUIRE(exported);
}