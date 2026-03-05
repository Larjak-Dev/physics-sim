#include "Simulator.hpp"
#include "tools/Error.hpp"
#include <mutex>

using namespace phys;

Simulator::Simulator()
{
}
Simulator::~Simulator()
{
}

void Simulator::startSim(std::shared_ptr<Universe> universe)
{
    if (this->running_sim)
    {
        phys::showMessage("Simulator is already running!");
        return;
    }

    this->thread_sim = std::thread(
        [this, universe]()
        {
            this->running_sim = true;

            while (!this->stop_sim)
            {
                {
                    std::unique_lock<std::mutex> lock(mtx_sim);
                    cv_sim.wait(lock, [this] { return !this->paused_sim; });
                }

                // TODO
            }

            this->stop_sim = false;
            this->running_sim = false;
        });
}
void Simulator::startPreview(const Universe &universe)
{
    if (this->running_preview)
    {
        phys::showMessage("Simulator is already running!");
        return;
    }

    Environment env = static_cast<Environment>(*universe.env);

    this->thread_preview = std::thread(
        [this, env]()
        {
            this->running_preview = true;

            while (!this->stop_preview)
            {
                {
                    std::unique_lock<std::mutex> lock(mtx_preview);
                    cv_preview.wait(lock, [this] { return !this->paused_preview; });
                }

                // TODO
            }

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
