#include "Simulator.hpp"
#include "Kinematics.hpp"
#include "physics/PhysicFunctions.hpp"
#include "tools/Error.hpp"
#include <mutex>
#include <thread>

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
u_int32_t Recording::getStatus() const
{
    return this->status;
}
u_int32_t Recording::getCompletion() const
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

void Simulator::startSim(std::shared_ptr<Universe> universe)
{
    if (this->running_sim)
    {
        phys::showMessage("Simulator is already running!");
        return;
    }

    assert(universe);
    assert(universe->env);

    if (this->thread_sim.joinable())
        this->thread_sim.join();

    this->thread_sim = std::thread(
        [this, universe]()
        {
            this->running_sim = true;

            const auto physic_functions = PhysicFunctions(universe->physicConfig);
            const double delta_time = universe->physicConfig.step_config.delta_time;
            StepBuffer step_buffer;

            while (!this->stop_sim)
            {
                {
                    std::unique_lock<std::mutex> lock(mtx_sim);
                    cv_sim.wait(lock, [this] { return !this->paused_sim; });
                }

                const auto env_copy = static_cast<EnvironmentBase>(*universe->env);
                const auto env_new = physic_functions.step(env_copy, delta_time, step_buffer);
                universe->env->setEnvironment_safe(env_new);

                std::this_thread::sleep_for(std::chrono::duration<double>(delta_time));
            }

            this->stop_sim = false;
            this->running_sim = false;
        });
}
void Simulator::startPreview(const Universe &universe, std::shared_ptr<Recording> recording)
{
    if (this->running_preview)
    {
        phys::showMessage("Simulator is already running!");
        return;
    }

    assert(recording);

    recording->universe = std::make_unique<Universe>(universe.copy());

    Environment env = static_cast<Environment>(*universe.env);
    recording->frames.emplace_back(env);

    const auto physic_functions = PhysicFunctions(universe.physicConfig);
    const double delta_time = universe.physicConfig.step_config.delta_time;
    const double total_time = universe.physicConfig.step_config.total_time;

    // Kinematics
    const bool calculate_kinematic = env.config.is_calculated;
    const auto config_kinemtic = env.config;
    if (calculate_kinematic)
    {
        recording->frames_kinematic.emplace_back(phys::calcBody(config_kinemtic, env.passed_time));
    }

    if (this->thread_preview.joinable())
        this->thread_preview.join();

    this->thread_preview = std::thread(
        [this, recording, physic_functions, delta_time, total_time, calculate_kinematic, config_kinemtic]()
        {
            recording->status = 2;
            this->running_preview = true;
            StepBuffer step_buffer;

            while (!this->stop_preview && recording->frames.back().passed_time + delta_time < total_time)
            {
                {
                    std::unique_lock<std::mutex> lock(mtx_preview);
                    cv_preview.wait(lock, [this] { return !this->paused_preview; });
                }

                recording->completion = static_cast<u_int16_t>(100 * recording->frames.back().passed_time / total_time);

                const auto env_prev = recording->frames.back();
                const auto env_new = physic_functions.step(env_prev, delta_time, step_buffer);
                recording->frames.emplace_back(env_new);

                if (calculate_kinematic)
                {
                    recording->frames_kinematic.emplace_back(
                        phys::calcBody(config_kinemtic, recording->frames.back().passed_time));
                }
            }

            recording->status = 3;
            this->stop_preview = false;
            this->running_preview = false;
        });
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
    return this->stop_sim;
}
