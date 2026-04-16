#include "Scene.hpp"
#include "SFML/Graphics/Font.hpp"
#include "app/AppResources.hpp"
#include "core/Environment.hpp"
#include "core/Units.hpp"
#include "core/tools/Error.hpp"
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
    ImGui::InvisibleButton("##viewport_drag", ImGui::GetItemRectSize(),
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);

    if (ImGui::IsItemActive())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto mouse_delta = ImGui::GetIO().MouseDelta;
            auto delta = vec2f(mouse_delta.x, mouse_delta.y);

            auto delta_scene = 2.0 * vec2d(delta) / vec2d(vec2u(texture.getSize()));
            vec4f delta_world_vec;
            if (universe.camera->settings.is_render_perspective)
            {
                double aspect = texture.getSize().x / (double)texture.getSize().y;
                double tan_half_fov = std::tan(universe.camera->settings.fov * PI / 180.0 / 2.0);
                delta_world_vec.x = delta_scene.x * universe.camera->distance * aspect * tan_half_fov;
                delta_world_vec.y = delta_scene.y * universe.camera->distance * tan_half_fov;
            }
            else
            {
                delta_world_vec = this->renderer.transform2D.p_inverse * vec4f(delta_scene.x, delta_scene.y, 0.0, 1.0);
            }

            auto delta_crossX = vec3f(universe.camera->getCrossX()) * -delta_world_vec.x;
            auto delta_crossY = vec3f(universe.camera->getCrossY()) * delta_world_vec.y;
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
            if (universe.camera->x_angle > glm::pi<double>() - 0.01)
                universe.camera->x_angle = glm::pi<double>() - 0.01;
        }
    }

    if (ImGui::IsItemHovered())
    {
        // Scrolling
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            universe.camera->distance =
                universe.camera->settings.minimum_camera_distance +
                (universe.camera->distance - universe.camera->settings.minimum_camera_distance) * pow(1.1, -wheel);
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
                auto env = universe.env->getEnvironment_safe();
                selected_body_id = this->renderer.cordOnTargetToBodyInWorld(mouse_pos_item, *universe.camera,
                                                                            universe.env->getEnvironment_safe(),
                                                                            universe.properties, texture);
            }
        }

        // Right click popup
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            vec2f mouse_pos_global = ImGui::GetIO().MousePos;
            vec2f item_pos = ImGui::GetItemRectMin();
            auto mouse_pos_item = mouse_pos_global - item_pos;
            auto env = universe.env->getEnvironment_safe();
            if (auto body_id = this->renderer.cordOnTargetToBodyInWorld(mouse_pos_item, *universe.camera,
                                                                        universe.env->getEnvironment_safe(),
                                                                        universe.properties, texture))
            {
                selected_body_id = body_id;
                ImGui::OpenPopup("Body");
            }
            else
            {
                auto ray_start = this->renderer.cordOnTargetToWorldCord(mouse_pos_item, *universe.camera, 1.0, texture);
                auto ray_end = this->renderer.cordOnTargetToWorldCord(mouse_pos_item, *universe.camera, 0.5, texture);
                auto ray_dir = ray_end - ray_start;

                if (std::abs(ray_dir.z) > 1e-6)
                {
                    double t = -ray_start.z / ray_dir.z;
                    mouse_world = ray_start + ray_dir * t;
                }
                else
                {
                    mouse_world = ray_start;
                }
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
    // if (should_clear)
    //     this->texture.clear();
    updateInputs(cursor, universe, texture, this->selected_body_id, this->click_pos_world);

    auto &cam = *universe.camera;
    if (cam.settings.locked_body_id)
    {
        auto res = universe.getBody(cam.settings.locked_body_id);
        if (res)
        {
            auto &body = res->first;
            cam.center = body.pos;
        }
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

    const float FIELD_TRANSPARENCY = 0.5f;

    auto pair_selected = universe.getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
    auto body_selected = pair_selected.first;

    this->renderer.activate(this->texture);
    this->renderer.clear(BACKGROUND_COLOR);
    if (cam.settings.is_render_stars)
        this->renderer.renderSkyBox(SKYBOX, *universe.camera, SKYBOX_TRANSPARENCY);
    if (cam.settings.is_render_grid)
    {
        if (body_selected.id != 0)
        {
            this->renderer.renderGrids(GRID_SCALE, *universe.camera, GRID_TRANSPARENCY, body_selected.pos.z,
                                       GRID_COLOR_SMALL, GRID_COLOR_BIG);
        }
        else
        {
            this->renderer.renderGrids(GRID_SCALE, *universe.camera, GRID_TRANSPARENCY, 0, GRID_COLOR_SMALL,
                                       GRID_COLOR_BIG);
        }
    }
    auto env = universe.env->getEnvironment_safe();
    this->renderer.renderGravityField(universe.camera->distance * 8, body_selected.pos.z, FIELD_TRANSPARENCY, env,
                                      universe.properties, *universe.camera);
    this->renderer.renderBodies(env, universe.properties, *universe.camera);
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
        auto env = universe.env->getEnvironment_safe();

        ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("##BodiesTable", 1, flags))
        {
            ImGui::TableSetupColumn("BodyName", ImGuiTableColumnFlags_WidthStretch);

            for (auto &&[body, prop] : std::views::zip(env.bodies, universe.properties))
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
                    selected_body_id = body.id;
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();

        ImGui::Begin("Selection");
        {
            // Selected body
            auto pair_selected = universe.getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
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
                this->editing_pair = universe.getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
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

                    if (ImGui::BeginTable("PropertyTable", 4, ImGuiTableFlags_SizingStretchSame))
                    {
                        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Div", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("Mul", ImGuiTableColumnFlags_WidthFixed);

                        float btn_width = ImGui::GetFrameHeight() * 1.8f;

                        // Color
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Color");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::ColorEdit4("##color", reinterpret_cast<float *>(&editing_property.color));

                        // Size X
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Size X:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputDouble("##x_size", &editing_property.size.x, 0.0, 0.0, "%.2e");

                        // Size Y
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Size Y:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputDouble("##y_size", &editing_property.size.y, 0.0, 0.0, "%.2e");

                        // Size Z
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Size Z:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputDouble("##z_size", &editing_property.size.z, 0.0, 0.0, "%.2e");

                        // Tilt
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Tilt:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##tilt", &editing_property.tilt, 0.0, 0.0, "%.2e");

                        // Rotation
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rotation:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##rotation_start", &editing_property.rotation_start, 0.0, 0.0, "%.2e");

                        // Rotation Speed
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rotation Speed:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##rotation_speed", &editing_property.rotation_velocity, 0.0, 0.0, "%.2e");
                        ImGui::TableSetColumnIndex(2);
                        if (ImGui::Button("/1.5##rot_speed", ImVec2(btn_width, 0)))
                            editing_property.rotation_velocity /= 1.5f;
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("x1.5##rot_speed", ImVec2(btn_width, 0)))
                            editing_property.rotation_velocity *= 1.5f;

                        // Brightness
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Brightness:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##brightness", &editing_property.brightness, 0.0, 0.0, "%.2e");

                        ImGui::EndTable();
                    }
                }
                if (ImGui::Button("Configure"))
                {
                    auto res = universe.setBody(selected_body_id, this->editing_pair);
                    if (!res)
                    {
                        phys::showMessage(res.error().c_str());
                    }
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
                this->editing_pair = universe.getBody(1).value_or(std::pair<Body, Property>{});
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

                    if (ImGui::BeginTable("PropertyTable", 4, ImGuiTableFlags_SizingStretchSame))
                    {
                        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Div", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("Mul", ImGuiTableColumnFlags_WidthFixed);

                        float btn_width = ImGui::GetFrameHeight() * 1.8f;

                        // Color
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Color");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::ColorEdit4("##color", reinterpret_cast<float *>(&editing_property.color));

                        // Size X
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Size X:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputDouble("##x_size", &editing_property.size.x, 0.0, 0.0, "%.2e");

                        // Size Y
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Size Y:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputDouble("##y_size", &editing_property.size.y, 0.0, 0.0, "%.2e");

                        // Size Z
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Size Z:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputDouble("##z_size", &editing_property.size.z, 0.0, 0.0, "%.2e");

                        // Tilt
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Tilt:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##tilt", &editing_property.tilt, 0.0, 0.0, "%.2e");

                        // Rotation
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rotation:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##rotation_start", &editing_property.rotation_start, 0.0, 0.0, "%.2e");

                        // Rotation Speed
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rotation Speed:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##rotation_speed", &editing_property.rotation_velocity, 0.0, 0.0, "%.2e");
                        ImGui::TableSetColumnIndex(2);
                        if (ImGui::Button("/1.5##rot_speed", ImVec2(btn_width, 0)))
                            editing_property.rotation_velocity /= 1.5f;
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("x1.5##rot_speed", ImVec2(btn_width, 0)))
                            editing_property.rotation_velocity *= 1.5f;

                        // Brightness
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Brightness:");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputFloat("##brightness", &editing_property.brightness, 0.0, 0.0, "%.2e");

                        ImGui::EndTable();
                    }
                }

                if (ImGui::Button("Confirm"))
                {
                    universe.addBody(editing_pair.first, editing_pair.second);
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

    if (cam.settings.locked_body_id != 0)
    {
        auto pair_selected = universe.getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
        auto body_selected = pair_selected.first;
        auto property_selected = pair_selected.second;
        cam.settings.minimum_camera_distance = property_selected.size.z;
    }
    else
    {
        cam.settings.minimum_camera_distance = 0.0;
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

    auto cursor = ImGui::GetCursorPos();
    auto &resources_gl = this->context.resources_gl;
    auto &resources_app = this->context.resources_app;

    // update texture widget
    TextureWidget::update();
    this->texture.clear();

    // We only need basic inputs and floating window layout from the base update,
    // but SceneWidget::update does rendering. We have to bypass it and do our own.
    updateInputs(cursor, *this->universe, texture, this->selected_body_id, this->click_pos_world);

    auto &cam = *universes[0]->camera;
    if (cam.settings.locked_body_id)
    {
        auto res = universe->getBody(cam.settings.locked_body_id);
        if (res)
        {
            auto &body = res->first;
            cam.center = body.pos;
        }
    }

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
        this->renderer.renderSkyBox(SKYBOX, cam, SKYBOX_TRANSPARENCY);

    if (cam.settings.is_render_grid)
    {
        auto pair_selected = universe->getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
        auto body_selected = pair_selected.first;
        if (body_selected.id != 0)
        {
            this->renderer.renderGrids(GRID_SCALE, cam, GRID_TRANSPARENCY, body_selected.pos.z, GRID_COLOR_SMALL,
                                       GRID_COLOR_BIG);
        }
        else
        {
            this->renderer.renderGrids(GRID_SCALE, cam, GRID_TRANSPARENCY, 0, GRID_COLOR_SMALL, GRID_COLOR_BIG);
        }
    }

    this->renderer.renderBodiesAmalgamated(this->universes, this->properties, cam);

    this->renderer.deactivate();

    ///////////////////
    // ViewChild (FloatingWindow)
    //////////////////
    const float VIEWCHILD_WIDTH = 160;
    const float VIEWCHILD_HEIGHT = 350;
    const Color VIEWCHILD_BACKGROUND = Color(0.2f, 0.2f, 0.2f, 0.5f);
    ImFont *VIEWCHILD_FONT = resources_app.font_small;

    ImGui::SetCursorPos(cursor);
    ImGui::BeginChild("ViewChild", ImVec2(VIEWCHILD_WIDTH, VIEWCHILD_HEIGHT));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(VIEWCHILD_BACKGROUND.r, VIEWCHILD_BACKGROUND.g,
                                                   VIEWCHILD_BACKGROUND.b, VIEWCHILD_BACKGROUND.a));

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
        ImGui::InputDouble("##X", &cam.center.x, 0.0, 0.0, "%.2e");
        drawTableLabel("Y:");
        ImGui::InputDouble("##Y", &cam.center.y, 0.0, 0.0, "%.2e");
        drawTableLabel("Z:");
        ImGui::InputDouble("##Z", &cam.center.z, 0.0, 0.0, "%.2e");
        drawTableLabel("Zoom:");
        ImGui::InputDouble("##zoom", &cam.distance, 0.0, 0.0, "%.2e");
        drawTableLabel("Rot_X");
        ImGui::InputDouble("##rotX", &cam.x_angle, 0.0, 0.0);
        drawTableLabel("Rot_Z");
        ImGui::InputDouble("##rotZ", &cam.z_angle, 0.0, 0.0);

        ImGui::EndTable();
        ImGui::EndChild();
        ImGui::PopFont();
    }

    const float RENDERING_HEIGHT = 100.0f;
    if (ImGui::CollapsingHeader("Rendering"))
    {
        ImGui::PushFont(VIEWCHILD_FONT);
        ImGui::BeginChild("Rendering", ImVec2(VIEWCHILD_ITEM_WIDTH, RENDERING_HEIGHT), ImGuiChildFlags_Borders);

        ImGui::BeginTable("##RenderTable", 2);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        drawTableLabel("Scaled:");
        ImGui::Checkbox("##isScale", &cam.settings.is_scaled_body_size);
        if (cam.settings.is_scaled_body_size)
        {
            drawTableLabel("");
            ImGui::InputDouble("##scale_size", &cam.settings.body_scale);
        }

        drawTableLabel("Fixed:");
        ImGui::Checkbox("##isFixed", &cam.settings.is_fixed_body_size);
        if (cam.settings.is_fixed_body_size)
        {
            drawTableLabel("");
            ImGui::InputDouble("##fixed_size", &cam.settings.fixed_size, 0.1, 0.1, "%.2f");
        }

        drawTableLabel("Grid:");
        ImGui::Checkbox("##grid", &cam.settings.is_render_grid);
        drawTableLabel("Stars:");
        ImGui::Checkbox("##stars", &cam.settings.is_render_stars);
        drawTableLabel("Fancy:");
        ImGui::Checkbox("##fancy", &cam.settings.is_render_fancy);
        drawTableLabel("Persp:");
        ImGui::Checkbox("##perspective", &cam.settings.is_render_perspective);

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
        auto env = universe->env->getEnvironment_safe();
        auto &properties = universe->properties;
        ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("##BodiesTable", 1, flags))
        {
            ImGui::TableSetupColumn("BodyName", ImGuiTableColumnFlags_WidthStretch);
            for (auto &&[body, prop] : std::views::zip(env.bodies, properties))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                std::string button_text = std::format("{} ({})", prop.name, body.id);
                if (ImGui::Button(button_text.c_str(), ImVec2(-FLT_MIN, 0.0f)))
                {
                    cam.center = body.pos;
                    cam.distance = prop.size.x * 3.0;
                    cam.settings.locked_body_id = body.id;
                    this->selected_body_id = body.id;
                }
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();

    /////////////////
    /// Selection Window
    /////////////////
    ImGui::Begin("Selection");
    {
        auto pair_selected = universe->getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
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

                drawTableLabel("Lock: ");
                if (ImGui::Button("Lock Camera"))
                {
                    cam.settings.locked_body_id = this->selected_body_id;
                }
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();

    if (cam.settings.locked_body_id != 0)
    {
        auto pair_selected = universe->getBody(this->selected_body_id).value_or(std::pair<Body, Property>{});
        auto property_selected = pair_selected.second;
        cam.settings.minimum_camera_distance = property_selected.size.z;
    }
    else
    {
        cam.settings.minimum_camera_distance = 0.0;
    }
}
