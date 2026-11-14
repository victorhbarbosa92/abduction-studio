#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroStereoEnhancerUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Stereo Enhancer (Pan, Stereo Separation & Haas Delay)
        float stereo_separation = 0.45f; // -1.0 (100% Mono) a +1.0 (100% Extra Wide Estéreo)
        float stereo_pan = 0.0f;         // -1.0 (Esquerda) a +1.0 (Direita)
        float haas_delay_ms = 18.0f;     // Efeito Haas de alargamento psicoacústico (0ms a 50ms)
        int haas_channel = 1;            // 0=Desligado, 1=Atrasar Canal Direito, 2=Atrasar Canal Esquerdo
        float phase_inversion = 0.0f;    // 0=Normal, 1=Inverter L, 2=Inverter R
        float volume_trim = 1.0f;

    public:
        KuroStereoEnhancerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.80f, 1.00f, 0.85f)); // Azul Ciano Estéreo

            if (ImGui::Begin("🎧 FRUITY STEREO ENHANCER (HAAS EFFECT & STEREO SEPARATION)###StereoEnhancerWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🌌 3D Super Wide Haas Widener (22ms Right Delay)")) {
                            stereo_separation = 0.80f; haas_channel = 1; haas_delay_ms = 22.0f;
                        }
                        if (ImGui::MenuItem("📻 Mono Safe Radio Compatibility (0% Separation)")) {
                            stereo_separation = -1.0f; haas_channel = 0;
                        }
                        if (ImGui::MenuItem("🎸 Acoustic Guitar Left Haas Spreader (15ms Left)")) {
                            stereo_separation = 0.60f; haas_channel = 2; haas_delay_ms = 15.0f;
                        }
                        if (ImGui::MenuItem("⚡ Synth Lead Stereo Detune Spreader")) {
                            stereo_separation = 0.50f; haas_channel = 1; haas_delay_ms = 8.5f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "FRUITY STEREO ENHANCER: PSYCHOACOUSTIC HAAS DELAY & M/S SEPARATION");
                ImGui::TextDisabled("Processador clássico para expansão de imagem estéreo, efeito de precedência Haas e controle de fase.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DO VETORSCÓPIO DE IMAGEM ESTÉREO ──────
                ImVec2 se_p0 = ImGui::GetCursorScreenPos();
                ImVec2 se_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 se_p1 = ImVec2(se_p0.x + se_sz.x, se_p0.y + se_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(se_p0, se_p1, IM_COL32(10, 14, 22, 255), 4.0f);
                dl->AddRect(se_p0, se_p1, IM_COL32(30, 50, 70, 255), 4.0f);

                ImVec2 center = ImVec2(se_p0.x + se_sz.x * 0.5f + (stereo_pan * 80.0f), se_p0.y + se_sz.y * 0.5f);

                // Linha Central (Mono Axis) e Eixo Estéreo (Side Axis)
                dl->AddLine(ImVec2(center.x, se_p0.y + 10.0f), ImVec2(center.x, se_p1.y - 10.0f), IM_COL32(50, 70, 90, 180), 1.0f);
                dl->AddLine(ImVec2(se_p0.x + 30.0f, center.y), ImVec2(se_p1.x - 30.0f, center.y), IM_COL32(50, 70, 90, 180), 1.0f);

                // Desenhar Elipse / Nuvem de Dispersão Estéreo
                float width_factor = (stereo_separation + 1.0f) * 0.5f; // 0.0 (Mono) a 1.0 (Wide)
                float rad_x = 20.0f + width_factor * 110.0f;
                float rad_y = 55.0f;

                // Forma da Imagem Estéreo
                dl->AddEllipse(center, ImVec2(rad_x, rad_y), IM_COL32(0, (int)(160 + width_factor * 95), 255, 220), 0.0f, 64, 2.5f);
                dl->AddCircleFilled(center, 4.0f, IM_COL32(255, 255, 255, 255));

                char se_info[128];
                const char* h_ch[] = { "Desligado", "Atrasar Canal Direito (R)", "Atrasar Canal Esquerdo (L)" };
                snprintf(se_info, sizeof(se_info), "SEPARAÇÃO: %+.0f%% | PAN: %+.0f%% | EFEITO HAAS: %s (%.1f ms)", stereo_separation * 100.0f, stereo_pan * 100.0f, h_ch[haas_channel], haas_delay_ms);
                dl->AddText(ImVec2(se_p0.x + 12.0f, se_p0.y + 6.0f), IM_COL32(0, 230, 255, 240), se_info);

                ImGui::Dummy(se_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO ENHANCER ────────────────────────
                ImGui::Columns(3, "StereoEnhancerCols", true);

                // Coluna 1: Separação Estéreo & Pan
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🌐 IMAGEM & PANORAMA");
                ImGui::Separator();

                ImGui::SliderFloat("Separação Estéreo", &stereo_separation, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Panorama (Pan)", &stereo_pan, -1.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Efeito Haas (Atraso Psicoacústico)
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "⏱️ EFEITO HAAS (WIDENER)");
                ImGui::Separator();

                ImGui::Combo("Canal de Atraso Haas", &haas_channel, h_ch, IM_ARRAYSIZE(h_ch));
                ImGui::SliderFloat("Tempo de Atraso Haas", &haas_delay_ms, 0.0f, 50.0f, "%.1f ms");

                ImGui::NextColumn();

                // Coluna 3: Inversão de Fase & Trim
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎚️ FASE & TRIM");
                ImGui::Separator();

                const char* ph_modes[] = { "Fase Normal", "Inverter Canal L", "Inverter Canal R" };
                int ph_sel = (int)phase_inversion;
                if (ImGui::Combo("Inversor de Fase", &ph_sel, ph_modes, IM_ARRAYSIZE(ph_modes))) {
                    phase_inversion = (float)ph_sel;
                }
                ImGui::SliderFloat("Volume Trim", &volume_trim, 0.0f, 1.5f, "%.2fx");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
