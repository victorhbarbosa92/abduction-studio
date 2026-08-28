#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityVocoderUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Vocoder (Classic 32-Band Robotic Filterbank)
        int filter_bands_count = 16;     // 8, 16 ou 32 Bandas
        float band_gain[16] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
        float carrier_synth_freq = 130.81f; // C3 (Frequência do oscilador portador interno)
        int carrier_shape = 0;           // 0=Sawtooth, 1=Pulse 25%, 2=White Noise, 3=External Input
        float attack_speed_ms = 4.0f;    // Resposta de ataque dos envelopes seguidores
        float release_speed_ms = 45.0f;  // Resposta de decaimento dos formantes
        float formant_shift = 0.0f;      // Deslocamento de formantes (-12 a +12 semitons)
        float sibilance_high_boost = 0.40f; // Passagem de consoantes/sibilantes (S, T, K)

    public:
        KuroFruityVocoderUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.90f, 0.60f, 0.85f)); // Verde Robótico Vocoder

            if (ImGui::Begin("🤖 FRUITY VOCODER (CLASSIC 32-BAND FORMANT FILTERBANK)###FruityVocoderWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🤖 Daft Punk Robot Rock Saw Lead")) {
                            carrier_shape = 0; formant_shift = 0.0f; attack_speed_ms = 3.0f;
                            release_speed_ms = 35.0f; sibilance_high_boost = 0.60f;
                        }
                        if (ImGui::MenuItem("🛸 Kraftwerk Trans-Europe Express (Whisper Noise)")) {
                            carrier_shape = 2; formant_shift = +3.0f; release_speed_ms = 80.0f;
                        }
                        if (ImGui::MenuItem("👽 Deep Cyberpunk Cyborg Pitch (+4st Shift)")) {
                            carrier_shape = 1; formant_shift = -5.0f;
                        }
                        if (ImGui::MenuItem("🎙️ Crisp Vocal Clarity (Enhanced Sibilance)")) {
                            sibilance_high_boost = 0.85f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.65f, 1.0f), "FRUITY VOCODER: CLASSIC REALTIME 32-BAND MODULATOR & CARRIER FILTERBANK");
                ImGui::TextDisabled("O clássico vocoder do FL Studio para criação de vozes robóticas, efeitos Daft Punk e sintetização vocal.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. EQUALIZADOR GRÁFICO DAS 16 BANDAS DE FORMANTES ───────
                ImVec2 vc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 vc_sz = ImVec2(ImGui::GetContentRegionAvail().x, 180.0f);
                ImVec2 vc_p1 = ImVec2(vc_p0.x + vc_sz.x, vc_p0.y + vc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(vc_p0, vc_p1, IM_COL32(10, 15, 20, 255), 4.0f);
                dl->AddRect(vc_p0, vc_p1, IM_COL32(30, 50, 45, 255), 4.0f);

                // Desenhar as 16 Barras de Espectro de Formantes
                float bar_w = (vc_sz.x - 40.0f) / 16.0f;
                for (int b = 0; b < 16; ++b) {
                    float bx = vc_p0.x + 20.0f + b * bar_w;
                    float bh = band_gain[b] * (vc_sz.y * 0.70f);
                    float by = vc_p1.y - 25.0f - bh;

                    // Barra Neon
                    dl->AddRectFilled(ImVec2(bx + 2, by), ImVec2(bx + bar_w - 4, vc_p1.y - 25.0f), IM_COL32(0, (int)(180 + band_gain[b] * 75), 120, 220), 2.0f);
                    dl->AddRect(ImVec2(bx + 2, by), ImVec2(bx + bar_w - 4, vc_p1.y - 25.0f), IM_COL32(100, 255, 180, 255), 1.0f);

                    char b_num[4];
                    snprintf(b_num, sizeof(b_num), "%d", b + 1);
                    dl->AddText(ImVec2(bx + (bar_w * 0.5f) - 4, vc_p1.y - 18.0f), IM_COL32(120, 160, 140, 200), b_num);
                }

                char vc_info[128];
                const char* c_names[] = { "Sawtooth Carrier", "Pulse 25% Carrier", "White Noise Carrier", "External Input" };
                snprintf(vc_info, sizeof(vc_info), "PORTADOR: %s | FORMANT SHIFT: %+.1f ST | SIBILÂNCIA: %.0f%%", c_names[carrier_shape], formant_shift, sibilance_high_boost * 100.0f);
                dl->AddText(ImVec2(vc_p0.x + 12.0f, vc_p0.y + 6.0f), IM_COL32(0, 255, 160, 240), vc_info);

                ImGui::Dummy(vc_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO VOCODER ────────────────────────
                ImGui::Columns(3, "VocoderCols", true);

                // Coluna 1: Portador (Carrier)
                ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.65f, 1.0f), "⚡ SINAL PORTADOR (CARRIER)");
                ImGui::Separator();

                ImGui::Combo("Forma Portadora", &carrier_shape, c_names, IM_ARRAYSIZE(c_names));
                ImGui::SliderFloat("Frequência Portadora", &carrier_synth_freq, 65.4f, 523.25f, "%.1f Hz (Nota)");
                ImGui::SliderFloat("Formant Shift", &formant_shift, -12.0f, 12.0f, "%+.1f Semitons");

                ImGui::NextColumn();

                // Coluna 2: Velocidade dos Envelopes
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "⏱️ RESPOSTA DE FORMANTES");
                ImGui::Separator();

                ImGui::SliderFloat("Attack Time", &attack_speed_ms, 0.5f, 50.0f, "%.1f ms");
                ImGui::SliderFloat("Release Time", &release_speed_ms, 5.0f, 250.0f, "%.0f ms");
                ImGui::SliderFloat("Sibilance Boost (S/T/K)", &sibilance_high_boost, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Teste e Modulação de Voz
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎤 TESTE ROBÓTICO");
                ImGui::Separator();

                if (ImGui::Button("🤖 DISPARAR VOZ DAFT PUNK (TESTE)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(48, 2.5f, 0.95f, 0); // Dispara sintetizador portador
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
