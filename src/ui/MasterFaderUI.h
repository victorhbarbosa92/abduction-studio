#pragma once
#include "imgui.h"
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class MasterFaderUI {
    public:
        static void Render(bool* open, float& master_volume, float vu_l, float vu_r) {
            if (open && !*open) return;

            ImGui::SetNextWindowSize(ImVec2(180, 240), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Master Fader", open, ImGuiWindowFlags_NoCollapse)) {
                
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

                if (canvas_sz.x < 60 || canvas_sz.y < 120) {
                    ImGui::End();
                    return;
                }

                // Layout matching mockup screenshot:
                // Left: Fader Slider | Middle: Dual Stereo VU Meters | Right: dB Scale Numbers
                float fader_x = canvas_p0.x + 12.0f;
                float meter_y0 = canvas_p0.y + 25.0f;
                float meter_h = canvas_sz.y - 55.0f;
                float meter_y1 = meter_y0 + meter_h;

                // 1. Fader Slider Slot on the Left
                ImGui::SetCursorScreenPos(ImVec2(fader_x, meter_y0));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.10f, 0.13f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.85f, 0.88f, 0.92f, 1.0f));
                ImGui::VSliderFloat("##MasterVolFader", ImVec2(22, meter_h), &master_volume, 0.0f, 1.25f,);
                ImGui::PopStyleColor(2);

                float meter_w = 16.0f;
                float meter_l_x0 = fader_x + 36.0f;
                float meter_l_x1 = meter_l_x0 + meter_w;

                float meter_r_x0 = meter_l_x1 + 5.0f;
                float meter_r_x1 = meter_r_x0 + meter_w;

                // 2. Meter Background Slots (Middle)
                draw_list->AddRectFilled(ImVec2(meter_l_x0, meter_y0), ImVec2(meter_l_x1, meter_y1), IM_COL32(18, 22, 28, 255), 2.0f);
                draw_list->AddRectFilled(ImVec2(meter_r_x0, meter_y0), ImVec2(meter_r_x1, meter_y1), IM_COL32(18, 22, 28, 255), 2.0f);

                // Meter Signal Level (Neon Mint Green #65e28b)
                float h_l = std::clamp(vu_l, 0.0f, 1.0f) * meter_h;
                float h_r = std::clamp(vu_r, 0.0f, 1.0f) * meter_h;

                if (h_l > 0.0f) {
                    draw_list->AddRectFilled(ImVec2(meter_l_x0 + 1.0f, meter_y1 - h_l), ImVec2(meter_l_x1 - 1.0f, meter_y1), IM_COL32(101, 226, 139, 245), 2.0f);
                }
                if (h_r > 0.0f) {
                    draw_list->AddRectFilled(ImVec2(meter_r_x0 + 1.0f, meter_y1 - h_r), ImVec2(meter_r_x1 - 1.0f, meter_y1), IM_COL32(101, 226, 139, 245), 2.0f);
                }

                // Peak Caps (White)
                static float cap_l = 0.0f, cap_r = 0.0f;
                cap_l = std::max(cap_l - 0.015f, h_l);
                cap_r = std::max(cap_r - 0.015f, h_r);

                if (cap_l > 2.0f) {
                    float cap_y = meter_y1 - cap_l;
                    draw_list->AddLine(ImVec2(meter_l_x0 + 1.0f, cap_y), ImVec2(meter_l_x1 - 1.0f, cap_y), IM_COL32(255, 255, 255, 255), 2.0f);
                }
                if (cap_r > 2.0f) {
                    float cap_y = meter_y1 - cap_r;
                    draw_list->AddLine(ImVec2(meter_r_x0 + 1.0f, cap_y), ImVec2(meter_r_x1 - 1.0f, cap_y), IM_COL32(255, 255, 255, 255), 2.0f);
                }

                // Grid ticks across peak meters
                for (int db = -40; db <= 0; db += 10) {
                    float norm_y = 1.0f - (db + 40.0f) / 40.0f;
                    float y = meter_y0 + norm_y * meter_h;
                    draw_list->AddLine(ImVec2(meter_l_x0, y), ImVec2(meter_r_x1, y), IM_COL32(0, 0, 0, 100), 1.0f);
                }

                // 3. Render dB Scale markers on the Right
                float db_x = meter_r_x1 + 10.0f;
                const int db_levels[] = { 0, -10, -20, -30, -40 };
                for (int db : db_levels) {
                    float norm_y = 1.0f - (db + 40.0f) / 40.0f;
                    float y = meter_y0 + norm_y * meter_h;

                    char db_buf[16];
                    snprintf(db_buf, sizeof(db_buf), "%d", db);
                    draw_list->AddText(ImVec2(db_x + 8.0f, y - 6.0f), IM_COL32(160, 175, 190, 255), db_buf);
                    draw_list->AddLine(ImVec2(db_x, y), ImVec2(db_x + 5.0f, y), IM_COL32(90, 105, 120, 255));
                }

                // Labels at bottom
                draw_list->AddText(ImVec2(fader_x + 22.0f, meter_y1 + 8.0f), IM_COL32(210, 220, 230, 255), "Master");
                draw_list->AddText(ImVec2(db_x + 10.0f, meter_y1 + 8.0f), IM_COL32(160, 170, 185, 255), "dB");
            }
            ImGui::End();
        }
    };
}
