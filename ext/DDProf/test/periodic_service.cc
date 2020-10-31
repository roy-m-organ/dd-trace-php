#include <catch2/catch.hpp>

#include "ddprof/service.hh"

TEST_CASE("periodic_service basics", "[periodic_service]") {
    class test_service : public ddprof::periodic_service {
       public:
        bool &destroyed;
        int &called;

        explicit test_service(bool &d, int &i) noexcept : destroyed{d}, called{i} {}

        void periodic() override { ++called; }
        ~test_service() override { destroyed = true; }
    };

    bool destroyed = false;
    int called = 0;
    {
        ddprof::periodic_service *service = new test_service(destroyed, called);
        auto interval = std::chrono::milliseconds(10);
        service->interval = interval;

        service->start();
        while (!service->has_started()) {
            std::this_thread::sleep_for(interval);
        }

        service->stop();
        service->join();

        delete service;
    }

    REQUIRE(called);
    REQUIRE(destroyed);
}