#pragma once
#include "../universe/Universe.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace phys
{

class Simulator
{
  public:
    Simulator();
    ~Simulator();

    void startSim(std::shared_ptr<Universe> universe);
    void startPreview(const Universe &universe);

    void stopSim();
    void stopPreview();

    void pauseSim();
    void pausePreview();

    void resumeSim();
    void resumePreview();

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
