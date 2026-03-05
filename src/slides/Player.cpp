
#include "Player.hpp"
#include "imgui.h"
#include "tools/Debug.hpp"
#include <memory>
using namespace phys::slides;

Player::Player()
{
    this->recording = std::make_shared<Recording>();
}
void Player::tickContent()
{
    if (this->recording->getStatus() != 3)
    {
        ImGui::Begin("Preview", nullptr);
        this->reviewPanel.update(*this->universe);
        ImGui::End();
    }
    else
    {
        ImGui::Begin("Player", nullptr);
        const auto &frames = this->recording->getFrames();
        const float frame_amount = frames.size();
        int frame_index = std::floor((frame_amount - 1) * this->timeline_select);
        this->universe_sim->env->setEnvironment_safe(frames[frame_index]);
        this->simulator.update(*this->universe_sim);
        ImGui::End();
    }

    if (this->universe_sim)
    {
        auto uni = this->universe_sim->env->getEnvironment_safe();
        auto body = uni.bodies[0];
        phys::showDebugF("Body x:{},y:{},z:{}", body.pos.x, body.pos.y, body.pos.z);
    }
}

void Player::tickRightBar()
{
    ImGui::Begin("Control Panel");
    if (!this->physic_sim.isRunningPreview() && ImGui::Button("Start"))
    {
        auto universe_copy = this->universe->copy();
        this->universe_sim = std::make_shared<Universe>(universe_copy);
        this->physic_sim.startPreview(universe_copy, this->recording);
    }
    if (this->physic_sim.isRunningPreview() && ImGui::Button("Stop"))
    {
        this->physic_sim.stopPreview();
    }

    if (this->physic_sim.isRunningPreview())
    {
        if (!this->physic_sim.isPausedPreview() && ImGui::Button("Pause"))
        {
            this->physic_sim.pausePreview();
        }
        if (this->physic_sim.isPausedPreview() && ImGui::Button("Resume"))
        {
            this->physic_sim.resumePreview();
        }
    }
    auto status = recording->getStatusStr();
    auto completion = recording->getCompletion();

    ImGui::Text("%s", status.c_str());
    ImGui::Text("completion: %d", completion);
    // uint32_t amount_frames = recording->getFrames().size();
    //  ImGui::Text("Frame amount: %d", amount_frames);

    ImGui::SliderFloat("Timeline", &this->timeline_select, 0.0f, 1.0);

    ImGui::End();
}
