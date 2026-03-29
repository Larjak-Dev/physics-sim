#include "Scene.hpp"
#include "SFML/Graphics/Font.hpp"
#include "app/AppResources.hpp"
#include "core/Units.hpp"
#include "core/universe/Environment.hpp"
#include "core/universe/Universe.hpp"
#include "extra.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ranges>

using namespace phys::app;

TextureWidget::TextureWidget()
{
}

void TextureWidget::update()
{
    auto size_imgui = ImGui::GetContentRegionAvail();

    ImGui::Image(this->texture);

    auto size_texure = sf::Vector2u(static_cast<unsigned int>(std::floor(size_imgui.x)),
                                    static_cast<unsigned int>(std::floor(size_imgui.y)));
    if (size_texure != this->texture_size)
    {
        bool isSuccesful = this->texture.resize(size_texure);
        if (isSuccesful)
        {
            this->texture_size = size_texure;
        }
    }
}

SceneWidget::SceneWidget(AppContext &context) : context(context)
{
}

void SceneWidget::updateInputs(ImVec2 cursor, phys::Universe &universe, sf::RenderTexture &texture,
                               unsigned int &selected_body_id, phys::vec3d &mouse_world)
{
    using namespace phys;
    ImGui::SetCursorPos(cursor);
    ImGui::InvisibleButton("##viewport_drag", ImGui::GetItemRectSize());

    if (ImGui::IsItemActive())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto mouse_delta = ImGui::GetIO().MouseDelta;
            auto delta = vec2f(mouse_delta.x, mouse_delta.y);

            auto delta_scene = 2.0 * vec2d(delta) / vec2d(vec2u(texture.getSize()));
            auto delta_world = this->renderer.transform2D.p_inverse * vec4f(delta_scene.x, delta_scene.y, 0.0, 1.0);

            auto delta_crossX = vec3f(universe.camera->getCrossX()) * -delta_world.x;
            auto delta_crossY = vec3f(universe.camera->getCrossY()) * delta_world.y;
            auto delta_cam = delta_crossX + delta_crossY;

            universe.camera->center += delta_cam;
            universe.camera->settings.locked_body_id = 0;
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            auto mouse_delta = ImGui::GetIO().MouseDelta;

            universe.camera->z_angle -= glm::quarter_pi<double>() * mouse_delta.x / 200.0;
            universe.camera->x_angle -= glm::quarter_pi<double>() * mouse_delta.y / 200.0;

            // Clamp pitch to prevent going upside down
            if (universe.camera->x_angle < 0.0)
                universe.camera->x_angle = 0.0;
            if (universe.camera->x_angle > glm::half_pi<double>() - 0.01)
                universe.camera->x_angle = glm::half_pi<double>() - 0.01;
        }
    }

    if (ImGui::IsItemHovered())
    {
        // Scrolling
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            universe.camera->distance *= std::pow(1.1, -wheel);
        }

        // Left Click
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            float threshold = 2.0f;

            if (std::abs(drag_delta.x) < threshold && std::abs(drag_delta.y) < threshold)
            {
                vec2f mouse_pos_global = ImGui::GetIO().MousePos;
                vec2f item_pos = ImGui::GetItemRectMin();

                auto mouse_pos_item = mouse_pos_global - item_pos;
                auto env = static_cast<Environment>(*universe.env);
                selected_body_id =
                    this->renderer.cordOnTargetToBodyInWorld(mouse_pos_item, *universe.camera, env, texture);
            }
        }

        // Right click popup
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            vec2f mouse_pos_global = ImGui::GetIO().MousePos;
            vec2f item_pos = ImGui::GetItemRectMin();
            auto mouse_pos_item = mouse_pos_global - item_pos;
            auto env = static_cast<Environment>(*universe.env);
            if (auto body_id = this->renderer.cordOnTargetToBodyInWorld(mouse_pos_item, *universe.camera, env, texture))
            {
                selected_body_id = body_id;
                ImGui::OpenPopup("Body");
            }
            else
            {
                mouse_world = this->renderer.cordOnTargetToWorldCord(mouse_pos_item, *universe.camera, 0, texture);
                ImGui::OpenPopup("World");
            }
        }
    }
}

void SceneWidget::update(phys::Universe &universe, bool should_clear)
{
    if (!universe.env)
    {
        ImGui::Text("Uninitialised Environment!");
        return;
    }
    if (!universe.camera)
    {
        ImGui::Text("Uninitialised Camera!");
        return;
    }

    auto cursor = ImGui::GetCursorPos();
    auto &resources_gl = this->context.resources_gl;
    auto &resources_app = this->context.resources_app;

    // update texture widget
    TextureWidget::update();
    if (should_clear)
        this->texture.clear();
    updateInputs(cursor, universe, texture, this->selected_body_id, this->click_pos_world);

    auto &cam = *universe.camera;
    if (cam.settings.locked_body_id)
    {
        auto body_pair = universe.env->getBody(cam.settings.locked_body_id);
        auto &body = body_pair.first;
        cam.center = body.pos;
    }

    /////////////////////
    /// Rendering Graphics
    /////////////////////

    const Color BACKGROUND_COLOR = Color::Black;
    const gl::Texture &SKYBOX = resources_gl.stars;
    const float SKYBOX_TRANSPARENCY = 0.5f;

    const Color GRID_COLOR_SMALL = Color(0.5f, 0.5, 0.5, 1.0f);
    const Color GRID_COLOR_BIG = Color(1.0f, 1.0, 1.0, 1.0f);
    const float GRID_TRANSPARENCY = 1.0f;
    const float GRID_SCALE = 1.0f;

    this->renderer.activate(this->texture);
    this->renderer.clear(BACKGROUND_COLOR);
    if (cam.settings.is_render_stars)
        this->renderer.renderSkyBox(SKYBOX, *universe.camera, SKYBOX_TRANSPARENCY);
    if (cam.settings.is_render_grid)
        this->renderer.renderGrids(GRID_SCALE, *universe.camera, GRID_TRANSPARENCY, GRID_COLOR_SMALL, GRID_COLOR_BIG);
    this->renderer.renderBodies(static_cast<Environment>(*universe.env), *universe.camera);
    this->renderer.deactivate();

    ///////////////////
    // ViewChild (FloatingWindow)
    //////////////////

    const float VIEWCHILD_WIDTH = 160;
    const float VIEWCHILD_HEIGHT = 350;
    const Color VIEWCHILD_BACKGROUND = Color(0.2f, 0.2f, 0.2f, 0.5f);
    ImFont *VIEWCHILD_FONT = resources_app.font_small;

    auto &camera = *universe.camera;
    ImGui::SetCursorPos(cursor);
    ImGui::BeginChild("ViewChild", ImVec2(VIEWCHILD_WIDTH, VIEWCHILD_HEIGHT));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(VIEWCHILD_BACKGROUND.r, VIEWCHILD_BACKGROUND.g,
                                                   VIEWCHILD_BACKGROUND.b, VIEWCHILD_BACKGROUND.a));
    // Viewport child

    const float VIEWCHILD_ITEM_WIDTH = 150;
    const float VIEWPORT_HEIGHT = 100;

    if (ImGui::CollapsingHeader("Viewport"))
    {
        ImGui::PushFont(VIEWCHILD_FONT);
        ImGui::BeginChild("Viewport", ImVec2(VIEWCHILD_ITEM_WIDTH, VIEWPORT_HEIGHT), ImGuiChildFlags_Borders);
        ImGui::BeginTable("##ViewTable", 2);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 30);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        drawTableLabel("X:");
        ImGui::InputDouble("##X", &universe.camera->center.x, 0.0, 0.0, "%.2e");
        drawTableLabel("Y:");
        ImGui::InputDouble("##Y", &universe.camera->center.y, 0.0, 0.0, "%.2e");
        drawTableLabel("Z:");
        ImGui::InputDouble("##Z", &universe.camera->center.z, 0.0, 0.0, "%.2e");
        drawTableLabel("Zoom:");
        ImGui::InputDouble("##zoom", &universe.camera->distance, 0.0, 0.0, "%.2e");
        drawTableLabel("Rotatio_X");
        ImGui::InputDouble("##rotX", &universe.camera->x_angle, 0.0, 0.0);
        drawTableLabel("Rotatio_Z");
        ImGui::InputDouble("##rotZ", &universe.camera->z_angle, 0.0, 0.0);

        ImGui::EndTable();
        ImGui::EndChild();
        ImGui::PopFont();
    }

    //////////////////
    /// Rendering Child
    //////////////////
    const float RENDERING_HEIGHT = 100.0f;

    if (ImGui::CollapsingHeader("Rendering"))
    {

        ImGui::PushFont(resources_app.font_small);
        ImGui::BeginChild("Rendering", ImVec2(VIEWCHILD_ITEM_WIDTH, RENDERING_HEIGHT), ImGuiChildFlags_Borders);

        ImGui::BeginTable("##ViewTable", 2);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        drawTableLabel("Scaled:");
        ImGui::Checkbox("##isScale", &camera.settings.is_scaled_body_size);

        if (camera.settings.is_scaled_body_size)
        {
            drawTableLabel("");
            ImGui::InputDouble("##scale_size", &camera.settings.body_scale);
        }

        drawTableLabel("Fixed Size:");
        ImGui::Checkbox("##isFixed", &camera.settings.is_fixed_body_size);

        if (camera.settings.is_fixed_body_size)
        {
            drawTableLabel("");
            ImGui::InputDouble("##fixed_size", &camera.settings.fixed_size, 0.1, 0.1, "%.2f");
        }

        drawTableLabel("Grid:");
        ImGui::Checkbox("##grid", &camera.settings.is_render_grid);

        drawTableLabel("Stars:");
        ImGui::Checkbox("##stars", &camera.settings.is_render_stars);

        drawTableLabel("Fancy:");
        ImGui::Checkbox("##fancy", &camera.settings.is_render_fancy);

        drawTableLabel("Perspective:");
        ImGui::Checkbox("##perspective", &camera.settings.is_render_perspective);

        ImGui::EndTable();

        ImGui::EndChild();
        ImGui::PopFont();
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();

    /////////////////
    /// Bodies Window
    /////////////////

    ImGui::Begin("Bodies");
    {
        auto env = static_cast<Environment>(*universe.env);

        ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("##BodiesTable", 1, flags))
        {
            ImGui::TableSetupColumn("BodyName", ImGuiTableColumnFlags_WidthStretch);

            for (auto &&[body, prop] : std::views::zip(env.bodies, env.properties))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Button fills the entire row width
                std::string button_text = std::format("{} ({})", prop.name, body.id);
                if (ImGui::Button(button_text.c_str(), ImVec2(-FLT_MIN, 0.0f)))
                {
                    camera.center = body.pos;
                    camera.distance = prop.size.x * 3.0;
                    camera.settings.locked_body_id = body.id;
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();

        ImGui::Begin("Selection");
        {
            // Selected body
            auto pair_selected = universe.env->getBody(this->selected_body_id);
            auto &body_selected = pair_selected.first;
            auto &body_property = pair_selected.second;
            if (body_selected.id != 0)
            {

                if (ImGui::BeginTable("##SelectionTable", 2, ImGuiTableFlags_RowBg))
                {
                    ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 65.0f);
                    ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

                    auto PropertyRow = [](const char *label, const char *id, double val)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("%s", label);
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputDouble(id, &val, 0.0, 0.0, "%.2e", ImGuiInputTextFlags_ReadOnly);
                    };
                    drawTableLabel("Name:");
                    ImGui::InputText("##name", &body_property.name, ImGuiInputTextFlags_ReadOnly);

                    drawTableLabel("ID:");
                    int id = static_cast<int>(body_selected.id);
                    ImGui::InputInt("##id", &id, 0.0, 0.0, ImGuiInputTextFlags_ReadOnly);

                    PropertyRow("Mass:", "##mass", body_selected.mass);
                    PropertyRow("Pos X:", "##px", body_selected.pos.x);
                    PropertyRow("Pos Y:", "##py", body_selected.pos.y);
                    PropertyRow("Pos Z:", "##pz", body_selected.pos.z);
                    PropertyRow("Vel X:", "##vx", body_selected.vel.x);
                    PropertyRow("Vel Y:", "##vy", body_selected.vel.y);
                    PropertyRow("Vel Z:", "##vz", body_selected.vel.z);

                    drawTableLabel("Lock camera: ");
                    if (ImGui::Button("Lock"))
                    {
                        cam.settings.locked_body_id = this->selected_body_id;
                    }

                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();

        // popup

        if (ImGui::BeginPopup("Body"))
        {
            if (ImGui::Button("Edit"))
            {
                this->editing_pair = universe.env->getBody(this->selected_body_id);
                ImGui::OpenPopup("Edit_Pop");
            };

            if (ImGui::Button("Delete"))
            {
            }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Edit_Pop", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                auto &editing_body = editing_pair.first;

                ImGui::Text("Mass:");
                ImGui::SameLine();
                ImGui::InputDouble("##mass", &editing_body.mass, 0.0, 0.0, "%.2e");
                ImGui::Text("Locked:");
                ImGui::SameLine();
                ImGui::Checkbox("##lock", &editing_body.is_locked);
                ImGui::Text("X:");
                ImGui::SameLine();
                ImGui::InputDouble("##x_body", &editing_body.pos.x, 0.0, 0.0, "%.2e");
                ImGui::Text("Y:");
                ImGui::SameLine();
                ImGui::InputDouble("##y_body", &editing_body.pos.y, 0.0, 0.0, "%.2e");
                ImGui::Text("Z:");
                ImGui::SameLine();
                ImGui::InputDouble("##z_body", &editing_body.pos.z, 0.0, 0.0, "%.2e");
                ImGui::Text("X_vel:");
                ImGui::SameLine();
                ImGui::InputDouble("##x_vel", &editing_body.vel.x, 0.0, 0.0, "%.2e");
                ImGui::Text("Y_vel");
                ImGui::SameLine();
                ImGui::InputDouble("##y_vel", &editing_body.vel.y, 0.0, 0.0, "%.2e");
                ImGui::Text("Z_vel");
                ImGui::SameLine();
                ImGui::InputDouble("##z_vel", &editing_body.vel.z, 0.0, 0.0, "%.2e");

                if (ImGui::CollapsingHeader("Property"))
                {
                    auto &editing_property = editing_pair.second;

                    ImGui::Text("Color");
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##color", reinterpret_cast<float *>(&editing_property.color));
                    ImGui::Text("Size X:");
                    ImGui::SameLine();
                    ImGui::InputDouble("##x_size", &editing_property.size.x, 0.0, 0.0, "%.2e");
                    ImGui::Text("Size Y:");
                    ImGui::SameLine();
                    ImGui::InputDouble("##y_size", &editing_property.size.y, 0.0, 0.0, "%.2e");
                    ImGui::Text("Size Z:");
                    ImGui::SameLine();
                    ImGui::InputDouble("##z_size", &editing_property.size.z, 0.0, 0.0, "%.2e");
                    ImGui::Text("Tilt:");
                    ImGui::SameLine();
                    ImGui::InputFloat("##tilt", &editing_property.tilt, 0.0, 0.0, "%.2e");
                    ImGui::Text("Rotation:");
                    ImGui::SameLine();
                    ImGui::InputFloat("##rotation_start", &editing_property.rotation_start, 0.0, 0.0, "%.2e");
                    ImGui::Text("Rotation Speed:");
                    ImGui::SameLine();
                    ImGui::InputFloat("##rotation_speed)", &editing_property.rotation_velocity, 0.0, 0.0, "%.2e");
                }
                if (ImGui::Button("Configure"))
                {
                    universe.env->setBody(selected_body_id, this->editing_pair);
                    ImGui::CloseCurrentPopup();
                };
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    ImGui::CloseCurrentPopup();
                };

                ImGui::EndPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("World"))
        {
            if (ImGui::Button("Summon"))
            {
                this->editing_pair = universe.env->getBody(1);
                this->editing_pair.first.pos = this->click_pos_world;
                this->editing_pair.first.pos.z = 0.0;
                ImGui::OpenPopup("Summon");
            };

            // SUMMON
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Summon", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {

                auto &editing_body = editing_pair.first;
                ImGui::Text("Mass:");
                ImGui::SameLine();
                ImGui::InputDouble("##mass", &editing_body.mass, 0.0, 0.0, "%.2e");
                ImGui::Text("Locked:");
                ImGui::SameLine();
                ImGui::Checkbox("##lock", &editing_body.is_locked);
                ImGui::Text("X:");
                ImGui::SameLine();
                ImGui::InputDouble("##x_body", &editing_body.pos.x, 0.0, 0.0, "%.2e");
                ImGui::Text("Y:");
                ImGui::SameLine();
                ImGui::InputDouble("##y_body", &editing_body.pos.y, 0.0, 0.0, "%.2e");
                ImGui::Text("Z:");
                ImGui::SameLine();
                ImGui::InputDouble("##z_body", &editing_body.pos.z, 0.0, 0.0, "%.2e");
                ImGui::Text("X_vel:");
                ImGui::SameLine();
                ImGui::InputDouble("##x_vel", &editing_body.vel.x, 0.0, 0.0, "%.2e");
                ImGui::Text("Y_vel");
                ImGui::SameLine();
                ImGui::InputDouble("##y_vel", &editing_body.vel.y, 0.0, 0.0, "%.2e");
                ImGui::Text("Z_vel");
                ImGui::SameLine();
                ImGui::InputDouble("##z_vel", &editing_body.vel.z, 0.0, 0.0, "%.2e");
                if (ImGui::CollapsingHeader("Property"))
                {
                    auto &editing_property = editing_pair.second;

                    ImGui::Text("Color");
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##color", reinterpret_cast<float *>(&editing_property.color));
                    ImGui::Text("Size X:");
                    ImGui::SameLine();
                    ImGui::InputDouble("##x_size", &editing_property.size.x, 0.0, 0.0, "%.2e");
                    ImGui::Text("Size Y:");
                    ImGui::SameLine();
                    ImGui::InputDouble("##y_size", &editing_property.size.y, 0.0, 0.0, "%.2e");
                    ImGui::Text("Size Z:");
                    ImGui::SameLine();
                    ImGui::InputDouble("##z_size", &editing_property.size.z, 0.0, 0.0, "%.2e");
                }

                if (ImGui::Button("Confirm"))
                {
                    universe.env->addBody(editing_pair);
                    ImGui::CloseCurrentPopup();
                };
                ImGui::SameLine();

                if (ImGui::Button("Cancel"))
                {
                    ImGui::CloseCurrentPopup();
                };
                ImGui::EndPopup();
            }
            ImGui::EndPopup();
        }
    }
}

UniverseWidget::UniverseWidget(AppContext &context) : SceneWidget(context)
{
    this->universe = std::make_shared<Universe>();
}
void UniverseWidget::update(bool should_clear)
{

    if (!universe)
    {
        ImGui::Text("Uninitialised Universe");
        return;
    }
    SceneWidget::update(*this->universe, should_clear);
}

AlmagationWidget::AlmagationWidget(AppContext &context) : UniverseWidget(context)
{
}

void AlmagationWidget::resize(int amount)
{
    this->universes.resize(amount);
    this->properties.resize(amount);

    for (auto &universe : this->universes)
    {
        if (!universe)
        {
            universe = std::make_shared<Universe>();
        }
    }
}
#include <glm/common.hpp>

phys::Color hueToRGB(float f)
{
    phys::vec3f rgb =
        glm::clamp(glm::abs(glm::mod(f * 6.0f + phys::vec3f(0.0f, 4.0f, 2.0), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
    return phys::Color(rgb.r, rgb.g, rgb.b, 1.0f);
}

void AlmagationWidget::resize_ColorSpectrum(int amount)
{
    resize(amount);
    int size = static_cast<int>(this->properties.size());
    int index = 0;
    for (auto &[transparency, color] : this->properties)
    {
        transparency = 1.0f;
        color = hueToRGB(static_cast<float>(index) / static_cast<float>(size));
        index++;
    }
}
void AlmagationWidget::resize_TransperancyFade(int amount)
{
    resize(amount);

    int size = static_cast<int>(this->properties.size());
    int index = 0;
    for (auto &[transparency, color] : this->properties)
    {
        transparency = (1 - static_cast<float>(index) / static_cast<float>(size));
        color = Color(0.0f, 0.0f, 0.0f, 0.0f);
        index++;
    }
}

void AlmagationWidget::update()
{
    if (universes.size() == 0 || !universes[0])
    {
        ImGui::Text("Uninitialised Universe");
        return;
    }
    if (!universes[0]->env)
    {
        ImGui::Text("Uninitialised Environment!");
        return;
    }
    if (!universes[0]->camera)
    {
        ImGui::Text("Uninitialised Camera!");
        return;
    }

    this->texture.clear();
    this->renderer.activate(this->texture);
    for (auto &&[i, universe] : std::views::enumerate(this->universes))
    {
        if (!universe || !universe->env)
            continue;
        this->renderer.renderBodies(static_cast<Environment>(*universe->env), *this->universes[0]->camera,
                                    this->properties[i].first, this->properties[i].second);
    }
    this->renderer.deactivate();
    UniverseWidget::update(false);
}
