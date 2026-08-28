#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroHarmorAdditiveSynthUI {
    private:
        bool is_open = false;
        
        // Harmor / Harmless Synthesizer Parameters
        float harmonic_timbre = 0.5f;     // Saw -> Square -> Sine morph
        float harmonic_prism = 0.0f;      // Deslocamento espectral de harmônicos
        float harmonic_unison = 0.65f;    // 9-voice additive unison detune
        int unison_voices = 9;
        float phaser_mix = 0.4f;          // Classic FL Harmor Additive Phaser
        float phaser_speed = 0.8f;
        float pluck_decay = 0.45f;        // Harmonic Pluck Envelope
        float sub_osc_level = 0.5f;
        float distortion_drive = 0.3f;    // Rubba Distortion
        float delay_mix = 0.35f;
        float reverb_mix = 0.40f;
        int active_preset = 0;

    public:
        KuroHarmorAdditiveSynthUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.09f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.85f)); // Neon Cyan

            if (ImGui::Begin("🛸 HARMLESS & HARMOR ADDITIVE / RESYNTHESIS SYNTH (FL STUDIO STYLE)###HarmorSynthWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("👽 Psytrance Aggressive Harmor Lead")) {
                            harmonic_timbre = 0.85f; harmonic_prism = 0.3f; harmonic_unison = 0.8f; pluck_decay = 0.3f;
                        }
                        if (ImGui::MenuItem("🚀 Cyberpunk Pluck (Harmless Style)")) {
                            harmonic_timbre = 0.40f; harmonic_prism = 0.0f; harmonic_unison = 0.5f; pluck_decay = 0.2f;
                        }
                        if (ImGui::MenuItem("🌌 Cosmic Shimmer Pad (Additive Resynthesis)")) {
                            harmonic_timbre = 0.20f; harmonic_prism = 0.7f; harmonic_unison = 0.9f; pluck_decay = 1.8f;
                        }
                        if (ImGui::MenuItem("⚡ Resonant Reese Bass (9-Voice Unison)")) {
                            harmonic_timbre = 0.60f; harmonic_prism = 0.1f; harmonic_unison = 0.95f; sub_osc_level = 0.9f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.00f, 1.0f), "SÍNTESE ADITIVA & RESÍNTESE ESPECTRAL HARMONICA (512 PARÇAS)");
                ImGui::TextDisabled("Manipula diretamente cada harmônico individual do espectro, eliminando problemas de fase e aliasing.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY DO ESPECTRO DE HARMÔNICOS EM TEMPO REAL ────────
                ImVec2 spec_p0 = ImGui::GetCursorScreenPos();
                ImVec2 spec_sz = ImVec2(ImGui::GetContentRegionAvail().x, 140.0f);
                ImVec2 spec_p1 = ImVec2(spec_p0.x + spec_sz.x, spec_p0.y + spec_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(spec_p0, spec_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                dl->AddRect(spec_p0, spec_p1, IM_COL32(30, 45, 60, 255), 4.0f);

                // Desenhar 64 Barras Harmônicas Espectrais
                int num_harmonics = 64;
                float bar_w = (spec_sz.x - 20.0f) / (float)num_harmonics;
                for (int h = 0; h < num_harmonics; ++h) {
                    float h_idx = (float)(h + 1);
                    float amp = std::pow(1.0f / h_idx, 0.7f + (1.0f - harmonic_timbre) * 1.5f);
                    
                    // Efeito Prism & Phaser modulando os harmônicos
                    amp *= (1.0f + 0.35f * std::sin(h_idx * 0.4f + harmonic_prism * 5.0f));
                    amp = std::clamp(amp, 0.02f, 1.0f);

                    float bar_h = amp * (spec_sz.y - 20.0f);
                    float bx = spec_p0.x + 10.0f + h * bar_w;
                    float by = spec_p1.y - 10.0f - bar_h;

                    // Gradiente Neon Harmor
                    dl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + bar_w - 2.0f, spec_p1.y - 10.0f), IM_COL32(0, (int)(150 + amp * 105), 255, 230), 1.5f);
                }

                dl->AddText(ImVec2(spec_p0.x + 12.0f, spec_p0.y + 8.0f), IM_COL32(0, 230, 255, 220), "ESPECTRO ADITIVO HARMONICO (HARMONIC DISTRIBUTION)");
                ImGui::Dummy(spec_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÕES DE CONTROLE (TIMBRE | UNISON | PLUCK | FX) ─────
                ImGui::Columns(3, "HarmorCols", true);

                // Coluna 1: Timbre & Espectro
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "🔮 TIMBRE & PRISM");
                ImGui::Separator();
                ImGui::SliderFloat("Forma Espectral (Timbre)", &harmonic_timbre, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Harmonic Prism", &harmonic_prism, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Sub-Oscilador", &sub_osc_level, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Distorção Rubba", &distortion_drive, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Unison & Phaser Aditivo
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.7f, 1.0f), "👥 UNISON & PHASER");
                ImGui::Separator();
                ImGui::SliderInt("Vozes de Unison", &unison_voices, 1, 9);
                ImGui::SliderFloat("Unison Detune", &harmonic_unison, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Additive Phaser Mix", &phaser_mix, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Phaser Speed", &phaser_speed, 0.1f, 10.0f, "%.1f Hz");

                ImGui::NextColumn();

                // Coluna 3: Harmonic Pluck & Efeitos
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🎛️ PLUCK & ESPACIAL");
                ImGui::Separator();
                ImGui::SliderFloat("Harmonic Pluck Decay", &pluck_decay, 0.05f, 3.0f, "%.2f s");
                ImGui::SliderFloat("Stereo Ping-Pong Delay", &delay_mix, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Shimmer Space Reverb", &reverb_mix, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão de Teste e Disparo de Nota
                if (ImGui::Button("🎵 TESTAR NOTA (C4 - 261.6 Hz)", ImVec2(-1, 36))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(60, 1.0f, 0.9f, 5); // Aciona sintetizador lead
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
