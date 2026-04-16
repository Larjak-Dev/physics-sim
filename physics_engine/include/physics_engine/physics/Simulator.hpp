#pragma once
#include "../core/Environment.hpp"
#include "../core/PhysicConfig.hpp"
#include <atomic>
#include <condition_variable>
#include <expected>
#include <functional>
#include <mutex>
#include <sys/types.h>
#include <thread>

namespace phys
{

class Simulator;

class Recording
{
  public:
    std::string getStatusStr() const;
    uint32_t getStatus() const;
    uint32_t getCompletion() const;

    const std::vector<phys::EnvironmentBase> &getFrames() const;
    void saveAsExcel();

    double total_time;
    std::function<void(const EnvironmentBase &)> on_step_callback;

  private:
    std::atomic_uint status{0};
    std::atomic_uint completion{0};
    std::vector<phys::EnvironmentBase> frames;

    friend Simulator;
};

class Simulator
{
  public:
    Simulator();
    ~Simulator();

    std::expected<void, std::string> startSim(std::shared_ptr<EnvironmentActive> env, const PhysicConfig &physicConfig,
                                              std::atomic<double> *speed_ref = nullptr);
    std::expected<void, std::string> startPreview(const Environment &env, const PhysicConfig &physicConfig,
                                                  std::shared_ptr<Recording> recording);

    void stopSim();
    void stopPreview();

    void pauseSim();
    void pausePreview();

    void resumeSim();
    void resumePreview();

    bool isRunningSim();
    bool isPausedSim();
    bool isStoppedSim();

    bool isRunningPreview();
    bool isPausedPreview();
    bool isStoppedPreview();

    std::atomic<double> speed{1.0};

  private:
    std::thread thread_sim;
    std::thread thread_preview;

    std::atomic_bool running_sim{false};
    std::atomic_bool running_preview{false};

    std::atomic_bool stop_sim{false};
    std::atomic_bool stop_preview{false};

    std::atomic_bool paused_sim{false};
    std::atomic_bool paused_preview{false};

    std::mutex mtx_sim;
    std::mutex mtx_preview;

    std::condition_variable cv_sim;
    std::condition_variable cv_preview;
};
} // namespace phys
