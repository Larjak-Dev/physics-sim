#include "physics/Simulator.hpp"
#include "core/Environment.hpp"
#include "core/PhysicConfig.hpp"
#include "physics/physics_functions/PhysicFunctions.hpp"
#include <cmath>
#include <mutex>
#include <thread>
#include <vector>

using namespace phys;

std::string Recording::getStatusStr() const
{
    switch (this->status)
    {
    case 0:
        return "Empty Recording!";
        break;
    case 1:
        return "...!";
        break;
    case 2:
        return "Recording!";
        break;
    case 3:
        return "Finished!";
        break;
    default:
        return "Error!";
    }
}
uint32_t Recording::getStatus() const
{
    return this->status;
}
uint32_t Recording::getCompletion() const
{
    return this->completion;
}

const std::vector<phys::EnvironmentBase> &Recording::getFrames() const
{
    return this->frames;
}

Simulator::Simulator()
{
}
Simulator::~Simulator()
{
    if (this->thread_sim.joinable())
    {
        stopSim();
        this->thread_sim.join();
    }
    if (this->thread_preview.joinable())
    {
        stopPreview();
        this->thread_preview.join();
    }
}

std::expected<void, std::string> Simulator::startSim(std::shared_ptr<EnvironmentActive> env_active,
                                                     const PhysicConfig &physicConfig, std::atomic<double> *speed_ref)
{
    if (this->running_sim)
    {
        return std::unexpected("Simulator is already running!");
    }

    if (physicConfig.force_config.force_type == ForceType::Null)
    {
        return std::unexpected("Unvalid force type!");
    }
    if (physicConfig.step_config.step_type == StepType::Null)
    {
        return std::unexpected("Unvalid step type!");
    }
    if (physicConfig.step_config.delta_time <= 0.0)
    {
        return std::unexpected("Unvalid delta time!");
    }
    if (physicConfig.step_config.delta_time / physicConfig.step_config.speed > 5.0)
    {
        return std::unexpected("Too long time steps! Increase Speed!");
    }

    if (this->thread_sim.joinable())
        this->thread_sim.join();

    this->thread_sim = std::thread(
        [this, env_active, physicConfig, speed_ref]()
        {
            this->running_sim = true;

            const auto physic_functions = PhysicFunctions(physicConfig);
            const double delta_time = physicConfig.step_config.delta_time;
            const double speed_config = physicConfig.step_config.speed;
            StepBuffer step_buffer;

            while (!this->stop_sim)
            {
                {
                    std::unique_lock<std::mutex> lock(mtx_sim);
                    cv_sim.wait(lock, [this] { return !this->paused_sim; });
                }

                const auto env_copy = env_active->getEnvironment_safe();
                const auto env_new = physic_functions.step(env_copy, delta_time, step_buffer);
                env_active->setEnvironment_safe(env_new);

                double current_speed = speed_ref ? speed_ref->load() : this->speed.load();
                std::this_thread::sleep_for(std::chrono::duration<double>(delta_time / (speed_config * current_speed)));
            }

            this->stop_sim = false;
            this->running_sim = false;
        });
    return {};
}

std::expected<void, std::string> Simulator::startPreview(const Environment &env_initial,
                                                         const PhysicConfig &physicConfig,
                                                         std::shared_ptr<Recording> recording)
{
    if (this->running_preview)
    {
        return std::unexpected("Simulator is already running!");
    }
    if (physicConfig.force_config.force_type == ForceType::Null)
    {
        return std::unexpected("Unvalid force type!");
    }
    if (physicConfig.step_config.step_type == StepType::Null)
    {
        return std::unexpected("Unvalid step type!");
    }
    if (physicConfig.step_config.delta_time <= 0.0)
    {
        return std::unexpected("Unvalid delta time!");
    }
    if (physicConfig.step_config.total_time <= 0.0)
    {
        return std::unexpected("Unvalid total time!");
    }

    assert(recording);

    recording->frames.emplace_back(static_cast<EnvironmentBase>(env_initial));

    const auto physic_functions = PhysicFunctions(physicConfig);
    const double delta_time = physicConfig.step_config.delta_time;
    const double total_time = physicConfig.step_config.total_time;
    recording->total_time = total_time;

    if (this->thread_preview.joinable())
        this->thread_preview.join();

    this->thread_preview = std::thread(
        [this, recording, physic_functions, delta_time, total_time]()
        {
            recording->status = 2;
            this->running_preview = true;
            StepBuffer step_buffer;

            int amount_frames = std::round(total_time / delta_time);

            for (int i = 0; i < amount_frames; i++)
            {
                if (this->stop_preview)
                    break;
                {
                    std::unique_lock<std::mutex> lock(mtx_preview);
                    cv_preview.wait(lock, [this] { return !this->paused_preview; });
                }

                recording->completion = static_cast<uint16_t>(100 * recording->frames.back().passed_time / total_time);

                const auto env_prev = recording->frames.back();
                const auto env_new = physic_functions.step(env_prev, delta_time, step_buffer);
                recording->frames.emplace_back(env_new);

                if (recording->on_step_callback)
                {
                    recording->on_step_callback(env_new);
                }
            }

            recording->status = 3;
            this->stop_preview = false;
            this->running_preview = false;
        });
    return {};
}

void Simulator::stopSim()
{
    this->stop_sim = true;
    resumeSim();
}
void Simulator::stopPreview()
{
    this->stop_preview = true;
    resumePreview();
}

void Simulator::pauseSim()
{
    std::lock_guard<std::mutex> lock(mtx_sim);
    this->paused_sim = true;
}
void Simulator::pausePreview()
{
    std::lock_guard<std::mutex> lock(mtx_preview);
    this->paused_preview = true;
}
void Simulator::resumeSim()
{
    {
        std::lock_guard<std::mutex> lock(mtx_sim);
        this->paused_sim = false;
    }
    cv_sim.notify_one();
}
void Simulator::resumePreview()
{
    {
        std::lock_guard<std::mutex> lock(mtx_preview);
        this->paused_preview = false;
    }
    cv_preview.notify_one();
}

bool Simulator::isRunningSim()
{
    return this->running_sim;
}
bool Simulator::isPausedSim()
{
    return this->paused_sim;
}
bool Simulator::isStoppedSim()
{
    return this->stop_sim;
}

bool Simulator::isRunningPreview()
{
    return this->running_preview;
}
bool Simulator::isPausedPreview()
{
    return this->paused_preview;
}
bool Simulator::isStoppedPreview()
{
    return this->stop_preview;
}
