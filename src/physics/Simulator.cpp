#include "Simulator.hpp"
#include "Kinematics.hpp"
#include "XLDocument.hpp"
#include "physics/PhysicFunctions.hpp"
#include "tools/Error.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"
#include <OpenXLSX.hpp>
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

const std::vector<phys::EnvironmentBase> &Recording::getKinematicFrames() const
{
    return this->frames_kinematic;
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

    if (universe->physicConfig.force_config.force_type == ForceType::Null)
    {
        phys::showMessage("Unvalid force type!");
        return;
    }
    if (universe->physicConfig.step_config.step_type == StepType::Null)
    {
        phys::showMessage("Unvalid step type!");
        return;
    }
    if (universe->physicConfig.step_config.delta_time <= 0.0)
    {
        phys::showMessage("Unvalid delta time!");
        return;
    }

    auto env = static_cast<Environment>(*universe->env);
    phys::prepareEnvironment(env, env.config, universe->physicConfig.step_config.delta_time);
    universe->env->setEnvironment_safe(env);

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

void addKinematicEnv(std::vector<EnvironmentBase> &frames_kinematic, Body body_kinematic)
{

    EnvironmentBase env_kinematic;
    env_kinematic.bodies.emplace_back(body_kinematic);
    frames_kinematic.emplace_back(env_kinematic);
}

void Simulator::startPreview(const Universe &universe, std::shared_ptr<Recording> recording)
{
    if (this->running_preview)
    {
        phys::showMessage("Simulator is already running!");
        return;
    }
    if (universe.physicConfig.force_config.force_type == ForceType::Null)
    {
        phys::showMessage("Unvalid force type!");
        return;
    }
    if (universe.physicConfig.step_config.step_type == StepType::Null)
    {
        phys::showMessage("Unvalid step type!");
        return;
    }
    if (universe.physicConfig.step_config.delta_time <= 0.0)
    {
        phys::showMessage("Unvalid delta time!");
        return;
    }
    if (universe.physicConfig.step_config.total_time <= 0.0)
    {
        phys::showMessage("Unvalid total time!");
        return;
    }

    assert(recording);

    recording->universe = std::make_unique<Universe>(universe.copy());

    Environment env = static_cast<Environment>(*universe.env);
    recording->frames.emplace_back(env);

    const auto physic_functions = PhysicFunctions(universe.physicConfig);
    const double delta_time = universe.physicConfig.step_config.delta_time;
    const double total_time = universe.physicConfig.step_config.total_time;
    recording->total_time = total_time;

    //// Kinematics
    const auto config_kinematic = env.config;
    const bool calculate_kinematic = config_kinematic.is_calculated;
    if (calculate_kinematic)
    {
        addKinematicEnv(recording->frames_kinematic, phys::calcBody(config_kinematic, env.passed_time));
    }
    ////

    phys::prepareEnvironment(recording->frames.back(), config_kinematic, delta_time);

    if (this->thread_preview.joinable())
        this->thread_preview.join();

    this->thread_preview = std::thread(
        [this, recording, physic_functions, delta_time, total_time, calculate_kinematic, config_kinematic]()
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

                /////Kinematics
                if (calculate_kinematic)
                {
                    addKinematicEnv(recording->frames_kinematic, phys::calcBody(config_kinematic, env_new.passed_time));
                }
                /////
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
