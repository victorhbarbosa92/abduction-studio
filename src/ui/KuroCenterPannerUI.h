#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroCenterPannerUI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio Fruity Center & DC Offset Remover
        bool remove_dc_offset = true;   // Filtro passa-altas de 5Hz para eliminar deslocamento DC
        float stereo_center_pan = 0.0f; // -1.0 (Esquerda) a +1.0 (Direita)
        float low_mono_crossover_hz = 120.0f; // Mono Bass Crossover (Todo grave abaixo deste corte vira Mono puro)
        bool enable_bass_mono = true;
        float phase_correction_deg = 0.0f;

    public:
        KuroCenterPannerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(840, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.70f, 1.00f, 0.85f)); // Azul Puro Fruity Center

            if (ImGui::Begin("🎯 FRUITY CENTER (DC OFFSET REMOVER & SUB MONO CROSSOVER)###CenterWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🎯 Club Master Sub Mono (120Hz Crossover + DC Kill)")) {
                            remove_dc_offset = true; enable_bass_mono = true; low_mono_crossover_hz = 120.0f;
                        }
                        if (ImGui::MenuItem("📻 Vinyl Cutting Safe Bass (180Hz Mono)")) {
                            enable_bass_mono = true; low_mono_crossover_hz = 180.0f;
                        }
                        if (ImGui::MenuItem("🧹 DC Offset Clean Only (Zero Pan)")) {
                            remove_dc_offset = true; enable_bass_mono = false;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "FRUITY CENTER: DC OFFSET ELIMINATION & MONO SUB-BASS CROSSOVER");
                ImGui::TextDisabled("Elimina o deslocamento DC prejudicial dos alto-falantes e converte sub-graves em Mono para mixagens sólidas de pista.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DO CORTE DE DC OFFSET & SUB MONO ──────
                ImVec2 fc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 fc_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 fc_p1 = ImVec2(fc_p0.x + fc_sz.x, fc_p0.y + fc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(fc_p0, fc_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                dl->AddRect(fc_p0, fc_p1, IM_COL32(30, 45, 65, 255), 4.0f);

                // Linha de Zero Central
                float cy = fc_p0.y + fc_sz.y * 0.5f;
                dl->AddLine(ImVec2(fc_p0.x, cy), ImVec2(fc_p1.x, cy), IM_COL32(50, 70, 90, 180), 1.0f);

                // Desenhar Onda Centralizada sem Offset DC
                int steps = (int)fc_sz.x;
                for (int x = 0; x < steps - 1; ++x) {
                    float t = (float)x / (float)steps;
                    float clean_wave = std::sin(t * 6.2831853f * 3.0f) * 0.8f;

                    float py0 = cy - clean_wave * (fc_sz.y * 0.40f);
                    float py1 = cy - std::sin(((float)(x + 1) / (float)steps) * 6.2831853f * 3.0f) * 0.8f * (fc_sz.y * 0.40f);

                    dl->AddLine(ImVec2(fc_p0.x + x, py0), ImVec2(fc_p0.x + x + 1, py1), IM_COL32(0, 220, 255, 220), 2.0f);
                }

                char fc_info[128];
                snprintf(fc_info, sizeof(fc_info), "DC KILL: %s | SUB MONO: %s (%.0f Hz) | PAN: %+.0f%%", remove_dc_offset ? "ATIVO (0.0 V)" : "OFF", enable_bass_mono ? "LIGADO" : "DESLIGADO", low_mono_crossover_hz, stereo_center_pan * 100.0f);
                dl->AddText(ImVec2(fc_p0.x + 12.0f, fc_p0.y + 6.0f), IM_COL32(0, 240, 255, 240), fc_info);

                ImGui::Dummy(fc_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO FRUITY CENTER ──────────────────
                ImGui::Columns(2, "CenterCols", true);

                // Coluna 1: DC Offset & Pan
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "🧹 LIMPEZA DC & PAN");
                ImGui::Separator();

                ImGui::Checkbox("Eliminar DC Offset (5Hz Highpass)", &remove_dc_offset);
                ImGui::SliderFloat("Center Pan", &stereo_center_pan, -1.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Mono Bass Crossover
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🔊 SUB-BASS MONO CROSSOVER");
                ImGui::Separator();

                ImGui::Checkbox("Converter Sub-Graves em Mono", &enable_bass_mono);
                ImGui::SliderFloat("Frequência de Crossover", &low_mono_crossover_hz, 40.0f, 300.0f, "%.0f Hz");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
