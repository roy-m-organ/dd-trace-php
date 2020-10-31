#ifndef DDPROF_SERVICE_HH
#define DDPROF_SERVICE_HH

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace ddprof {

// a service is a thing that can be started, stopped, and joined
class service {
    std::atomic<bool> started{false};

   public:
    constexpr service() noexcept;
    virtual ~service();

    // services are not copyable
    service(const service &) = delete;
    service &operator=(const service &) = delete;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void join() = 0;
};

class periodic_service : public service {
    std::mutex m;
    std::thread thread;
    std::atomic<bool> should_continue{false};
    std::atomic<bool> actually_started{false};

   public:
    // positive numbers only
    std::chrono::nanoseconds interval{10};

    periodic_service();
    ~periodic_service() override;

    virtual void periodic() = 0;

    void start() override;
    void stop() override;
    void join() override;

    bool has_started() const noexcept;
};

inline constexpr service::service() noexcept = default;
inline service::~service() = default;

inline periodic_service::periodic_service() = default;
inline periodic_service::~periodic_service() = default;

inline void periodic_service::start() {
    std::lock_guard<std::mutex> lock{m};
    should_continue = true;
    thread = std::thread([this]() {
        actually_started = true;
        while (should_continue) {
            std::this_thread::sleep_for(interval);
            periodic();
        }
    });
}

inline void periodic_service::stop() { should_continue = false; }

bool periodic_service::has_started() const noexcept { return actually_started; }

inline void periodic_service::join() {
    // hmm... lock?
    if (thread.joinable()) {
        thread.join();
    }
}

}  // namespace ddprof

#endif  // DDPROF_SERVICE_HH
