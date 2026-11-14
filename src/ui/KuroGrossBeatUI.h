#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroGrossBeatUI {
    private:
        bool is_open = false;

        // 36 Slots de Padrões de Tempo (Time Patterns) e Volume (Volume Gates)
        int selected_time_slot = 0;
        int selected_vol_slot = 0;

        // 4 Pontos da Curva de Tempo do Padrão Ativo (X = 0, 1, 2, 3 Beats)
        // Y representa o deslocamento de leitura da fita/tempo (ex: Y=0.5x é Half-Speed)
        float time_curve_points[4] = { 0.0f, 0.5f, 1.0f, 1.5f };
        float vol_curve_points[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        float scratch_needle_pos = 0.45f; // Posição atual da agulha de scratch
        float dry_wet_mix = 1.0f;
        int active_bank = 0; // 0=Time / Half-Speed, 1=Volume Gating, 2=Scratch & Reverse

    public:
        KuroGrossBeatUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.50f, 0.00f, 0.85f)); // Laranja / Ouro Gross Beat

            if (ImGui::Begin("⏳ GROSS BEAT (TIME MANIPULATION, HALF-SPEED & REPEAT GATER)###GrossBeatWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("⏳ 1/2 Speed (Half-Speed Trap / Hip-Hop Classic)")) {
                            time_curve_points[0] = 0.0f; time_curve_points[1] = 0.25f;
                            time_curve_points[2] = 0.5f; time_curve_points[3] = 0.75f;
                        }
                        if (ImGui::MenuItem("🔄 Turntable Vinyl Scratch & Reverse (1-Bar)")) {
                            time_curve_points[0] = 1.0f; time_curve_points[1] = 0.75f;
                            time_curve_points[2] = 0.5f; time_curve_points[3] = 0.0f;
                        }
                        if (ImGui::MenuItem("⚡ 1/8 Trance Gater (Volume Multi-Step)")) {
                            vol_curve_points[0] = 1.0f; vol_curve_points[1] = 0.0f;
                            vol_curve_points[2] = 1.0f; vol_curve_points[3] = 0.0f;
                        }
                        if (ImGui::MenuItem("👽 Complex 4-Beat Triplet Stutter")) {
                            time_curve_points[0] = 0.0f; time_curve_points[1] = 0.66f;
                            time_curve_points[2] = 0.33f; time_curve_points[3] = 1.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.0f, 1.0f), "GROSS BEAT: REALTIME BUFFER TIME STRETCH, HALF-SPEED & VOLUME GATER");
                ImGui::TextDisabled("O lendário plugin de manipulação temporal, reverse instantâneo, scratching de vinil e efeito Half-Speed do Trap mundial.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRÁFICO INTERATIVO DE TEMPO / ENVELOPE (4 COMPASSOS) ──
                ImVec2 gb_p0 = ImGui::GetCursorScreenPos();
                ImVec2 gb_sz = ImVec2(ImGui::GetContentRegionAvail().x, 200.0f);
                ImVec2 gb_p1 = ImVec2(gb_p0.x + gb_sz.x, gb_p0.y + gb_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(gb_p0, gb_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                dl->AddRect(gb_p0, gb_p1, IM_COL32(35, 50, 65, 255), 4.0f);

                // Grade de Compassos Verticais (Beat 1, 2, 3, 4)
                for (int b = 1; b < 4; ++b) {
                    float bx = gb_p0.x + (b / 4.0f) * gb_sz.x;
                    dl->AddLine(ImVec2(bx, gb_p0.y), ImVec2(bx, gb_p1.y), IM_COL32(30, 45, 60, 180), 1.0f);
                }

                // Linha Diagonal de Tempo Normal (1:1)
                dl->AddLine(ImVec2(gb_p0.x, gb_p1.y), ImVec2(gb_p1.x, gb_p0.y), IM_COL32(50, 65, 80, 140), 1.0f);

                // Desenhar a Curva de Manipulação de Tempo
                int steps = (int)gb_sz.x;
                for (int x = 0; x < steps - 1; ++x) {
                    float t0 = (float)x / (float)steps;
                    float t1 = (float)(x + 1) / (float)steps;

                    float idx0 = t0 * 3.0f;
                    int i0 = (int)idx0;
                    float f0 = idx0 - i0;
                    float y0_val = (i0 < 3) ? (time_curve_points[i0] * (1.0f - f0) + time_curve_points[i0 + 1] * f0) : 1.0f;

                    float idx1 = t1 * 3.0f;
                    int i1 = (int)idx1;
                    float f1 = idx1 - i1;
                    float y1_val = (i1 < 3) ? (time_curve_points[i1] * (1.0f - f1) + time_curve_points[i1 + 1] * f1) : 1.0f;

                    float px0 = gb_p0.x + t0 * gb_sz.x;
                    float py0 = gb_p1.y - (y0_val / 2.0f) * gb_sz.y;
                    float px1 = gb_p0.x + t1 * gb_sz.x;
                    float py1 = gb_p1.y - (y1_val / 2.0f) * gb_sz.y;

                    dl->AddLine(ImVec2(px0, py0), ImVec2(px1, py1), IM_COL32(255, 140, 0, 255), 2.5f);
                }

                // Agulha de Leitura em Tempo Real (Gross Beat Playhead)
                float needle_x = gb_p0.x + scratch_needle_pos * gb_sz.x;
                dl->AddLine(ImVec2(needle_x, gb_p0.y), ImVec2(needle_x, gb_p1.y), IM_COL32(0, 240, 255, 230), 2.0f);
                dl->AddCircleFilled(ImVec2(needle_x, gb_p0.y + 6), 4.0f, IM_COL32(0, 255, 255, 255));

                dl->AddText(ImVec2(gb_p0.x + 12.0f, gb_p0.y + 8.0f), IM_COL32(255, 160, 0, 240), "TIME BUFFER ENVELOPE (HALF-SPEED & REVERSE MATRIX)");
                ImGui::Dummy(gb_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SELEÇÃO DE SLOTS DE PADRÃO (TIME / VOLUME) ───────────
                ImGui::Columns(2, "GrossBeatCols", true);

                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.0f, 1.0f), "🎛️ SLOTS DE PADRÃO DE TEMPO (TIME SLOTS)");
                ImGui::Separator();

                const char* time_slot_names[] = {
                    "1: 1/2 Speed", "2: Slow Triplet", "3: 1-Bar Reverse", "4: 1/2 Reverse",
                    "5: Pitch Down Scratch", "6: Stutter Jump", "7: Push & Hold", "8: Flake Gater"
                };

                for (int s = 0; s < 8; ++s) {
                    bool is_sel = (selected_time_slot == s);
                    if (is_sel) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.50f, 0.0f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.18f, 0.22f, 0.8f));
                    }

                    if (ImGui::Button(time_slot_names[s], ImVec2(100, 28))) {
                        selected_time_slot = s;
                        if (s == 0) { // 1/2 Speed
                            time_curve_points[0] = 0.0f; time_curve_points[1] = 0.25f;
                            time_curve_points[2] = 0.5f; time_curve_points[3] = 0.75f;
                        } else if (s == 2) { // 1-Bar Reverse
                            time_curve_points[0] = 1.0f; time_curve_points[1] = 0.75f;
                            time_curve_points[2] = 0.5f; time_curve_points[3] = 0.0f;
                        }
                    }
                    ImGui::PopStyleColor();
                    if ((s % 4) != 3) ImGui::SameLine();
                }

                ImGui::NextColumn();

                // Coluna 2: Controles de Mix & Scratch Interativo
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "💿 VINYL SCRATCH & DRY/WET");
                ImGui::Separator();

                ImGui::SliderFloat("Dry / Wet Mix", &dry_wet_mix, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Agulha de Leitura (Playhead)", &scratch_needle_pos, 0.0f, 1.0f, "%.2f");

                ImGui::Spacing();
                if (ImGui::Button("⚡ DISPARAR HALF-SPEED (PREVIEW 1-BAR)", ImVec2(-1, 32))) {
                    selected_time_slot = 0;
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
