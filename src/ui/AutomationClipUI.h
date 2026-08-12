#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "../core/TimelineManager.h"
#include "FileDialog.h"

extern KuroDSP::TimelineManager timeline;

namespace KuroUI {

    class AutomationClipUI {
    private:
        bool is_open = false;
        int selected_target_type = 0; // 0: Volume, 1: Pan, 2: Cutoff, 3: Overdrive, 4: Pitch Shift, 5: GrossBeat Scratch
        int selected_track_target = 0; // Track 0..7

        int active_drag_point_idx = -1;
        float default_duration = 16.0f; // 4 bars a 120bpm = 8s, 16s max

    public:
        AutomationClipUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(900, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.09f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.40f, 0.4f));

            if (ImGui::Begin(ICON_FA_SLIDERS " ABDUCTION STUDIO V2 - EDITOR DE PISTAS DE AUTOMAÇÃO", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // TOP CONTROLS: TARGET SELECTION & PRESETS
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Alvo da Automação:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                const char* target_types[] = { "Volume", "Pan", "Filtro Cutoff", "Overdrive Drive", "Pitch Shift", "GrossBeat Scratch" };
                ImGui::Combo("##TargetType", &selected_target_type, target_types, IM_ARRAYSIZE(target_types));

                ImGui::SameLine();
                ImGui::SetNextItemWidth(120);
                const char* track_names_combo[] = { "Faixa 1 (Kick)", "Faixa 2 (Snare)", "Faixa 3 (Perc)", "Faixa 4 (Bass)", "Faixa 5 (Chords)", "Faixa 6 (Lead)", "Faixa 7 (Acid)", "Faixa 8 (Sub)" };
                ImGui::Combo("##TrackTarget", &selected_track_target, track_names_combo, IM_ARRAYSIZE(track_names_combo));

                ImGui::SameLine(ImGui::GetWindowWidth() - 360);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Presets de Curva:");
                
                std::string target_node_id = "Track" + std::to_string(selected_track_target);
                int target_param = selected_target_type;

                // Buttons for Curve Presets using FontAwesome 6 icons
                if (ImGui::Button(ICON_FA_CHART_LINE " Subida")) {
                    applyPreset(target_node_id, target_param, 0);
                }
                ImGui::SameLine();
                if (ImGui::Button(" ICON_FA_A "RROW_TREND_DOWN " Queda")) {
                    applyPreset(target_node_id, target_param, 1);
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_WAVE_SQUARE " Senoidal")) {
                    applyPreset(target_node_id, target_param, 2);
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_BOLT " Stutter")) {
                    applyPreset(target_node_id, target_param, 3);
                }

                ImGui::Separator();
                ImGui::Spacing();

                // ENVELOPE CANVAS GRID
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 360.0f);

                // Background Obsidian Box
                draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(10, 14, 18, 255), 4.0f);
                draw_list->AddRect(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(0, 229, 255, 120), 4.0f);

                // Grid Bar Divisions (4 Compassos = 16 Beats)
                float max_time = 8.0f; // 8 segundos de visualização
                for (int b = 0; b <= 16; ++b) {
                    float norm_x = (float)b / 16.0f;
                    float gx = canvas_p0.x + norm_x * canvas_sz.x;
                    ImU32 col = (b % 4 == 0) ? IM_COL32(0, 255, 128, 140) : IM_COL32(0, 200, 240, 35);
                    draw_list->AddLine(ImVec2(gx, canvas_p0.y), ImVec2(gx, canvas_p0.y + canvas_sz.y), col, (b % 4 == 0) ? 1.5f : 1.0f);
                    
                    if (b % 4 == 0) {
                        char bar_txt[16];
                        snprintf(bar_txt, sizeof(bar_txt), "%d.1", (b / 4) + 1);
                        draw_list->AddText(ImVec2(gx + 4.0f, canvas_p0.y + 4.0f), IM_COL32(0, 255, 128, 220), bar_txt);
                    }
                }

                // Horizontal Value Lines (0%, 25%, 50%, 75%, 100%)
                for (int v = 0; v <= 4; ++v) {
                    float norm_y = (float)v / 4.0f;
                    float gy = canvas_p0.y + (1.0f - norm_y) * canvas_sz.y;
                    draw_list->AddLine(ImVec2(canvas_p0.x, gy), ImVec2(canvas_p0.x + canvas_sz.x, gy), IM_COL32(255, 255, 255, 20));
                    char val_txt[16];
                    snprintf(val_txt, sizeof(val_txt), "%.0f%%", norm_y * 100.0f);
                    draw_list->AddText(ImVec2(canvas_p0.x + canvas_sz.x - 36.0f, gy - 12.0f), IM_COL32(180, 180, 180, 150), val_txt);
                }

                // Obter Pontos do TimelineManager
                std::vector<KuroDSP::TimelineManager::AutoPoint> points;
                {
                    std::lock_guard<std::mutex> lock(timeline.timeline_mutex);
                    for (const auto& lane : timeline.automation_lanes) {
                        if (lane.target_node_id == target_node_id && lane.param_index == target_param) {
                            points = lane.points;
                            break;
                        }
                    }
                }

                // Garantir pontos de borda padrão se estiver vazio
                if (points.empty()) {
                    timeline.addAutomationPoint(target_node_id, target_param, 0.0f, 0.5f);
                    timeline.addAutomationPoint(target_node_id, target_param, max_time, 0.5f);
                    points = { {0.0f, 0.5f}, {max_time, 0.5f} };
                }

                // Desenhar Curva Neon Green com Suavização Bézier
                for (size_t i = 0; i < points.size() - 1; ++i) {
                    float px0 = canvas_p0.x + (points[i].time_sec / max_time) * canvas_sz.x;
                    float py0 = canvas_p0.y + (1.0f - points[i].value) * canvas_sz.y;
                    float px1 = canvas_p0.x + (points[i+1].time_sec / max_time) * canvas_sz.x;
                    float py1 = canvas_p0.y + (1.0f - points[i+1].value) * canvas_sz.y;

                    // Pontos de controle da curva Bézier cúbica para transição suave
                    float dx = (px1 - px0) * 0.4f;
                    ImVec2 cp0(px0 + dx, py0);
                    ImVec2 cp1(px1 - dx, py1);

                    draw_list->AddBezierCubic(ImVec2(px0, py0), cp0, cp1, ImVec2(px1, py1), IM_COL32(0, 255, 102, 255), 2.5f);
                }

                // Interação do Mouse no Canvas
                ImGui::SetCursorScreenPos(canvas_p0);
                ImGui::InvisibleButton("##AutomationCanvas", canvas_sz);
                bool is_canvas_hovered = ImGui::IsItemHovered();
                ImVec2 mouse_pos = ImGui::GetMousePos();

                float mouse_time = std::clamp((mouse_pos.x - canvas_p0.x) / canvas_sz.x * max_time, 0.0f, max_time);
                float mouse_val = std::clamp(1.0f - (mouse_pos.y - canvas_p0.y) / canvas_sz.y, 0.0f, 1.0f);

                // Desenhar Nódulos de Controle Cyan e Tratar Arraste
                int hovered_point_idx = -1;
                for (size_t i = 0; i < points.size(); ++i) {
                    float px = canvas_p0.x + (points[i].time_sec / max_time) * canvas_sz.x;
                    float py = canvas_p0.y + (1.0f - points[i].value) * canvas_sz.y;

                    float dist = std::sqrt((mouse_pos.x - px) * (mouse_pos.x - px) + (mouse_pos.y - py) * (mouse_pos.y - py));
                    bool is_point_hovered = (dist < 10.0f);
                    if (is_point_hovered) hovered_point_idx = (int)i;

                    ImU32 node_col = is_point_hovered ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 229, 255, 255);
                    draw_list->AddCircleFilled(ImVec2(px, py), is_point_hovered ? 7.0f : 5.0f, node_col);
                    draw_list->AddCircle(ImVec2(px, py), is_point_hovered ? 8.5f : 6.0f, IM_COL32(255, 255, 255, 200), 12, 1.5f);
                }

                // Clique Direito: Adicionar ou Remover Ponto
                if (is_canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    if (hovered_point_idx != -1 && points.size() > 2) {
                        timeline.removeAutomationPoint(target_node_id, target_param, points[hovered_point_idx].time_sec);
                    } else {
                        timeline.addAutomationPoint(target_node_id, target_param, mouse_time, mouse_val);
                    }
                }

                // Clique Esquerdo: Arrastar Ponto
                if (is_canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered_point_idx != -1) {
                    active_drag_point_idx = hovered_point_idx;
                }

                if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && active_drag_point_idx != -1 && active_drag_point_idx < (int)points.size()) {
                    timeline.addAutomationPoint(target_node_id, target_param, mouse_time, mouse_val);
                    ImGui::SetTooltip("Tempo: %.2fs | Valor: %.0f%%", mouse_time, mouse_val * 100.0f);
                }

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    active_drag_point_idx = -1;
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: Clique esquerdo e arraste nódulos para ajustar valor/tempo. Clique direito sobre a grade para adicionar pontos e sobre um nódulo para removê-lo.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }

    private:
        void applyPreset(const std::string& node_id, int param_idx, int preset_type) {
            float max_time = 8.0f;
            {
                std::lock_guard<std::mutex> lock(timeline.timeline_mutex);
                for (auto& lane : timeline.automation_lanes) {
                    if (lane.target_node_id == node_id && lane.param_index == param_idx) {
                        lane.points.clear();
                        break;
                    }
                }
            }

            if (preset_type == 0) { // Subida Ramp Up
                timeline.addAutomationPoint(node_id, param_idx, 0.0f, 0.0f);
                timeline.addAutomationPoint(node_id, param_idx, max_time, 1.0f);
            } else if (preset_type == 1) { // Queda Ramp Down
                timeline.addAutomationPoint(node_id, param_idx, 0.0f, 1.0f);
                timeline.addAutomationPoint(node_id, param_idx, max_time, 0.0f);
            } else if (preset_type == 2) { // Onda Senoidal
                for (int i = 0; i <= 16; ++i) {
                    float t = (float)i / 16.0f * max_time;
                    float val = 0.5f + 0.5f * std::sin((float)i / 16.0f * 4.0f * 3.14159265f);
                    timeline.addAutomationPoint(node_id, param_idx, t, val);
                }
            } else if (preset_type == 3) { // Stutter Gate
                for (int i = 0; i < 8; ++i) {
                    float t0 = (float)i / 8.0f * max_time;
                    float t1 = t0 + (max_time / 16.0f);
                    timeline.addAutomationPoint(node_id, param_idx, t0, 1.0f);
                    timeline.addAutomationPoint(node_id, param_idx, t1, 0.0f);
                }
            }
        }
    };
}
