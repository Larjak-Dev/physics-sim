#pragma once
#include "../universe/Universe.hpp"
#include "universe/Environment.hpp"
#include <atomic>
#include <condition_variable>
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
    u_int32_t getStatus() const;
    u_int32_t getCompletion() const;

    const std::vector<phys::EnvironmentBase> &getFrames() const;
    const std::vector<phys::EnvironmentBase> &getKinematicFrames() const;
    void saveAsExcel();

    std::unique_ptr<Universe> universe;
    double total_time;

  private:
    std::atomic_uint status{0};
    std::atomic_uint completion{0};
    std::vector<phys::EnvironmentBase> frames;
    std::vector<phys::EnvironmentBase> frames_kinematic;

    friend Simulator;
};

class Simulator
{
  public:
    Simulator();
    ~Simulator();

    void startSim(std::shared_ptr<Universe> universe);
    void startPreview(const Universe &universe, std::shared_ptr<Recording> recording);

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
