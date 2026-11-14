#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityStereoReverbPanUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity PanOMatic (Automated Panner, Tremolo & Surround LFO)
        float pan_position = 0.0f;       // -1.0 (Esquerda) a +1.0 (Direita)
        float volume_gain = 1.0f;        // 0.0 a 2.0x
        float lfo_pan_depth = 0.65f;     // Profundidade do auto-pan estéreo (0% a 100%)
        float lfo_vol_depth = 0.0f;      // Profundidade de Tremolo (modulação de volume)
        float lfo_rate_hz = 2.0f;        // Velocidade LFO (0.1Hz a 20Hz)
        int lfo_shape = 0;               // 0=Senoide, 1=Triângulo, 2=Quadrada (Gater), 3=Random Sample & Hold
        bool tempo_sync_beat = false;

    public:
        KuroFruityStereoReverbPanUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.70f, 0.85f)); // Verde Esmeralda PanOMatic

            if (ImGui::Begin("🔄 FRUITY PANOMATIC (AUTO-PANNER, TREMOLO & ROTARY LFO)###PanOMaticWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔄 3D Stereo Rotary Auto-Pan (Sine 1.5 Hz)")) {
                            lfo_pan_depth = 0.85f; lfo_vol_depth = 0.0f; lfo_rate_hz = 1.5f; lfo_shape = 0;
                        }
                        if (ImGui::MenuItem("⚡ Trance Gater Chopper (Square Tremolo 4.0 Hz)")) {
                            lfo_pan_depth = 0.0f; lfo_vol_depth = 0.95f; lfo_rate_hz = 4.0f; lfo_shape = 2;
                        }
                        if (ImGui::MenuItem("🎲 Random S&H Glitch Panner (Psytrance Lead)")) {
                            lfo_pan_depth = 0.75f; lfo_vol_depth = 0.20f; lfo_rate_hz = 6.0f; lfo_shape = 3;
                        }
                        if (ImGui::MenuItem("🎸 Vintage Fender Tube Amp Tremolo (Sine 3.2 Hz)")) {
                            lfo_pan_depth = 0.0f; lfo_vol_depth = 0.60f; lfo_rate_hz = 3.2f; lfo_shape = 0;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.75f, 1.0f), "FRUITY PANOMATIC: AUTOMATED STEREO PANNER, TREMOLO & ROTARY MODULATOR");
                ImGui::TextDisabled("Modulador clássico de panorama estéreo e volume com oscilador LFO multimodo para efeitos de Auto-Pan e Tremolo.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA ÓRBITA DE AUTO-PAN E VOLUME ────────
                ImVec2 po_p0 = ImGui::GetCursorScreenPos();
                ImVec2 po_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 po_p1 = ImVec2(po_p0.x + po_sz.x, po_p0.y + po_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(po_p0, po_p1, IM_COL32(10, 15, 20, 255), 4.0f);
                dl->AddRect(po_p0, po_p1, IM_COL32(25, 50, 45, 255), 4.0f);

                ImVec2 center = ImVec2(po_p0.x + po_sz.x * 0.5f, po_p0.y + po_sz.y * 0.5f);

                // Linha de Centro Estéreo
                dl->AddLine(ImVec2(center.x, po_p0.y + 10), ImVec2(center.x, po_p1.y - 10), IM_COL32(40, 70, 60, 160), 1.0f);

                // Desenhar Posição Dinâmica do Pan e Volume
                int steps = (int)po_sz.x;
                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps * 3.14159265f * 2.0f;
                    float pan_val = std::sin(t * (lfo_rate_hz * 0.5f)) * lfo_pan_depth + pan_position;
                    float vol_val = 1.0f - (std::cos(t * (lfo_rate_hz * 0.5f)) * 0.5f + 0.5f) * lfo_vol_depth;

                    pan_val = std::clamp(pan_val, -1.0f, 1.0f);
                    float px = center.x + pan_val * (po_sz.x * 0.42f);
                    float py = center.y + (1.0f - vol_val) * (po_sz.y * 0.35f);

                    dl->AddCircleFilled(ImVec2(px, py), 2.5f, IM_COL32(0, 255, 180, 180));
                }

                char po_info[128];
                const char* s_names[] = { "Senoide", "Triângulo", "Quadrada (Gate)", "Random S&H" };
                snprintf(po_info, sizeof(po_info), "PAN: %+.0f%% | LFO RATE: %.2f Hz | FORMA: %s | PAN DEPTH: %.0f%% | TREMOLO: %.0f%%", pan_position * 100.0f, lfo_rate_hz, s_names[lfo_shape], lfo_pan_depth * 100.0f, lfo_vol_depth * 100.0f);
                dl->AddText(ImVec2(po_p0.x + 12.0f, po_p0.y + 6.0f), IM_COL32(0, 255, 200, 240), po_info);

                ImGui::Dummy(po_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO PANOMATIC ──────────────────────
                ImGui::Columns(3, "PanOMaticCols", true);

                // Coluna 1: Pan & Volume Base
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.75f, 1.0f), "🎛️ POSIÇÃO & GANHO BASE");
                ImGui::Separator();

                ImGui::SliderFloat("Panorama Base (Pan)", &pan_position, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Ganho de Volume", &volume_gain, 0.0f, 2.0f, "%.2fx");

                ImGui::NextColumn();

                // Coluna 2: Modulação LFO (Auto-Pan & Tremolo)
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🌊 PROFUNDIDADE DE LFO");
                ImGui::Separator();

                ImGui::SliderFloat("Auto-Pan Depth", &lfo_pan_depth, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Tremolo Vol Depth", &lfo_vol_depth, 0.0f, 1.0f, "%.2f");
                ImGui::Combo("Forma de Onda LFO", &lfo_shape, s_names, IM_ARRAYSIZE(s_names));

                ImGui::NextColumn();

                // Coluna 3: Velocidade & Sync
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "⏱️ VELOCIDADE & TEMPO");
                ImGui::Separator();

                ImGui::SliderFloat("Velocidade LFO", &lfo_rate_hz, 0.05f, 20.0f, "%.2f Hz");
                ImGui::Checkbox("Sincronizar LFO ao BPM", &tempo_sync_beat);

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
