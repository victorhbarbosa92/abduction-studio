#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroControlSurfaceUI {
    private:
        bool is_open = false;

        // Controles de Performance ao Vivo do Control Surface (FL Studio Style)
        // 1. Grande XY Pad de Filtro & Espaço
        float xy_pad_x = 0.5f; // Eixo X: Cutoff de Filtro (0% a 100%)
        float xy_pad_y = 0.3f; // Eixo Y: Reverb / Space Depth (0% a 100%)

        // 2. Quatro Knobs Makro Masters (Macro 1-4)
        float macro_knobs[4] = { 0.75f, 0.40f, 0.60f, 0.85f };
        const char* macro_names[4] = { "🛸 Master Drop Filter", "💥 Sub Bass Drive", "🌌 Space Atmosphere", "⚡ Glitch Stutter" };

        // 3. Quatro Faders Verticais de Performance
        float perf_faders[4] = { 0.9f, 0.7f, 0.8f, 0.5f };
        const char* fader_names[4] = { "Drums Stem", "Bass Stem", "Synths Stem", "Vocals Stem" };

        // 4. Quatro Botões de Efeito Instantâneo (Momentary Stutter / Kill Switches)
        bool kill_switches[4] = { false, false, false, false };
        const char* kill_names[4] = { "🛑 Mute Sub", "⚡ Tape Stop", "📻 High Cut 1k", "🚀 Infinite Reverb" };

    public:
        KuroControlSurfaceUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.90f, 0.40f, 1.00f, 0.85f)); // Roxo / Magenta Control Surface

            if (ImGui::Begin("🎛️ FL STUDIO CONTROL SURFACE (CUSTOM MACRO DASHBOARD & XY PAD)###ControlSurfaceWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Layouts")) {
                        if (ImGui::MenuItem("🛸 Live Performance Stage Dashboard")) {}
                        if (ImGui::MenuItem("🎛️ Mixing Master Macro Hub")) {}
                        if (ImGui::MenuItem("👽 DJ Transition & Drop Controller")) {}
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.9f, 0.45f, 1.0f, 1.0f), "CONTROL SURFACE: LIVE PERFORMANCE MACROS & CUSTOM TOUCH DASHBOARD");
                ImGui::TextDisabled("Superfície modular customizável para controle em tempo real de múltiplos parâmetros da DAW, XY Pad e Stems.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRANDE XY PAD INTERATIVO (TOUCH PAD 2D) ──────────────
                ImGui::Columns(2, "ControlSurfaceGrid", true);

                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "🕹️ PAD TOUCH BIDIMENSIONAL (XY CONTROLLER)");
                ImGui::TextDisabled("Eixo X: Cutoff Filtro | Eixo Y: Reverb / Space");
                ImGui::Spacing();

                ImVec2 xy_p0 = ImGui::GetCursorScreenPos();
                ImVec2 xy_sz = ImVec2(ImGui::GetContentRegionAvail().x, 260.0f);
                ImVec2 xy_p1 = ImVec2(xy_p0.x + xy_sz.x, xy_p0.y + xy_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(xy_p0, xy_p1, IM_COL32(10, 14, 22, 255), 6.0f);
                dl->AddRect(xy_p0, xy_p1, IM_COL32(50, 40, 70, 255), 6.0f);

                // Linhas de Grade Central
                float cx = xy_p0.x + xy_sz.x * 0.5f;
                float cy = xy_p0.y + xy_sz.y * 0.5f;
                dl->AddLine(ImVec2(cx, xy_p0.y), ImVec2(cx, xy_p1.y), IM_COL32(35, 45, 60, 180), 1.0f);
                dl->AddLine(ImVec2(xy_p0.x, cy), ImVec2(xy_p1.x, cy), IM_COL32(35, 45, 60, 180), 1.0f);

                // Ponto Interativo XY do Cursor
                float dot_x = xy_p0.x + xy_pad_x * xy_sz.x;
                float dot_y = xy_p1.y - xy_pad_y * xy_sz.y;

                // Linhas guias até as bordas
                dl->AddLine(ImVec2(dot_x, xy_p0.y), ImVec2(dot_x, xy_p1.y), IM_COL32(180, 80, 255, 120), 1.0f);
                dl->AddLine(ImVec2(xy_p0.x, dot_y), ImVec2(xy_p1.x, dot_y), IM_COL32(180, 80, 255, 120), 1.0f);

                // Cursor Brilhante de Controle
                dl->AddCircleFilled(ImVec2(dot_x, dot_y), 10.0f, IM_COL32(220, 60, 255, 240));
                dl->AddCircle(ImVec2(dot_x, dot_y), 14.0f, IM_COL32(255, 255, 255, 255), 2.0f);

                // Interação de Clique / Arraste no XY Pad
                ImGui::InvisibleButton("XYPadHitbox", xy_sz);
                if (ImGui::IsItemActive()) {
                    ImVec2 mpos = ImGui::GetIO().MousePos;
                    xy_pad_x = std::clamp((mpos.x - xy_p0.x) / xy_sz.x, 0.0f, 1.0f);
                    xy_pad_y = std::clamp((xy_p1.y - mpos.y) / xy_sz.y, 0.0f, 1.0f);
                }

                char pad_info[64];
                snprintf(pad_info, sizeof(pad_info), "X: %.0f%% (CUTOFF) | Y: %.0f%% (SPACE)", xy_pad_x * 100.0f, xy_pad_y * 100.0f);
                dl->AddText(ImVec2(xy_p0.x + 12.0f, xy_p0.y + 8.0f), IM_COL32(220, 150, 255, 240), pad_info);

                ImGui::NextColumn();

                // ── 2. SEÇÃO DE MACRO KNOBS (SUPER-KNOBS 1 A 4) ─────────────
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🎚️ SUPER MACRO CONTROLLERS (1-4)");
                ImGui::Separator();
                ImGui::Spacing();

                for (int k = 0; k < 4; ++k) {
                    ImGui::PushID(k + 100);
                    ImGui::Text("%s", macro_names[k]);
                    ImGui::SliderFloat("##macro_knob", &macro_knobs[k], 0.0f, 1.0f, "%.2f");
                    ImGui::PopID();
                    ImGui::Spacing();
                }

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. FADERS DE STEMS & BOTOES DE KILL SWITCH AO VIVO ──────
                ImGui::Columns(2, "StemCols", true);

                ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.5f, 1.0f), "📊 STEMS MIXER (FADERS DE PERFORMANCE)");
                ImGui::Spacing();

                for (int f = 0; f < 4; ++f) {
                    ImGui::PushID(f + 200);
                    ImGui::Text("%s:", fader_names[f]);
                    ImGui::SameLine(120);
                    ImGui::SliderFloat("##fader", &perf_faders[f], 0.0f, 1.2f, "%.2fx");
                    ImGui::PopID();
                }

                ImGui::NextColumn();

                // Botões de Efeito Instantâneo / Kill Switches
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.4f, 1.0f), "⚡ KILL SWITCHES & DROP FX");
                ImGui::Spacing();

                for (int s = 0; s < 4; ++s) {
                    ImGui::PushID(s + 300);
                    if (kill_switches[s]) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.28f, 0.8f));
                    }

                    if (ImGui::Button(kill_names[s], ImVec2(150, 32))) {
                        kill_switches[s] = !kill_switches[s];
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                    if (s % 2 == 0) ImGui::SameLine();
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
