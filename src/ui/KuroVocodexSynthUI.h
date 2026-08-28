#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroVocodexSynthUI {
    private:
        bool is_open = false;

        // Parâmetros do Vocodex (FL Studio Vocoder Clássico)
        int num_bands = 32;               // 16 a 100 bandas de filtro
        float carrier_synth_level = 0.85f;// Sintetizador interno Carrier (Saw/Pulse)
        float modulator_mic_gain = 1.20f; // Ganho da voz/modulador (Microfone / Vocal Track)
        float formant_shift = 0.0f;       // Deslocamento de formante (-12 a +12 semitons)
        float band_bandwidth = 1.0f;      // Largura de banda dos filtros
        float noise_generator_mix = 0.25f;// Injeção de ruído para consoantes sibilantes (S, T, K)
        float soundgoodizer_contour = 0.65f;
        int carrier_type = 0;             // 0=Sintetizador Interno, 1=Sidechain Track
        int internal_carrier_shape = 0;   // 0=Sawtooth Densa, 1=Pulse Oitavada, 2=Noise/Metallic

    public:
        KuroVocodexSynthUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.09f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.85f, 1.00f, 0.85f)); // Azul Ciano Vocodex

            if (ImGui::Begin("🤖 VOCODEX ADVANCED 100-BAND VOCODER (FL STUDIO STYLE)###VocodexWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS DO VOCODEX ──────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("👽 Alien Cyber Voice (100 Bands + Noise)")) {
                            num_bands = 64; formant_shift = 3.5f; noise_generator_mix = 0.45f;
                        }
                        if (ImGui::MenuItem("🤖 Daft Punk Classic Robotic Vocoder")) {
                            num_bands = 32; formant_shift = 0.0f; internal_carrier_shape = 0;
                        }
                        if (ImGui::MenuItem("🌌 Cosmic Choir Morph (Sidechain Carrier)")) {
                            num_bands = 48; formant_shift = -2.0f; soundgoodizer_contour = 0.8f;
                        }
                        if (ImGui::MenuItem("🚀 Neuro Bass Vocal Growl")) {
                            num_bands = 16; formant_shift = -5.0f; noise_generator_mix = 0.15f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "VOCODEX 100-BAND CARRIER & MODULATOR ROBOTIC SPEECH PROCESSOR");
                ImGui::TextDisabled("O lendário vocoder do FL Studio com modelagem de formantes, carrier embutido e matriz de envelope de voz.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. BANCO GRÁFICO DE FILTROS DO VOCODER (32 BANDAS) ──────
                ImVec2 voc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 voc_sz = ImVec2(ImGui::GetContentRegionAvail().x, 140.0f);
                ImVec2 voc_p1 = ImVec2(voc_p0.x + voc_sz.x, voc_p0.y + voc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(voc_p0, voc_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                dl->AddRect(voc_p0, voc_p1, IM_COL32(35, 50, 70, 255), 4.0f);

                // Desenhar Barras de Análise Espectral do Vocoder
                int display_bands = std::clamp(num_bands, 16, 64);
                float bw = (voc_sz.x - 20.0f) / (float)display_bands;

                for (int b = 0; b < display_bands; ++b) {
                    float t = (float)b / (float)display_bands;
                    // Simulação visual dinâmica de formantes vocais (picos em vogais A, E, O)
                    float v_peak1 = std::exp(-std::pow((t - 0.25f) * 6.0f, 2.0f));
                    float v_peak2 = std::exp(-std::pow((t - 0.65f) * 5.0f, 2.0f));
                    float band_amp = 0.2f + 0.7f * (v_peak1 + v_peak2);

                    float bh = band_amp * (voc_sz.y - 20.0f);
                    float bx = voc_p0.x + 10.0f + b * bw;
                    float by = voc_p1.y - 10.0f - bh;

                    dl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + bw - 2.0f, voc_p1.y - 10.0f), IM_COL32(0, (int)(180 + band_amp * 75), 255, 230), 1.5f);
                }

                char voc_info[64];
                snprintf(voc_info, sizeof(voc_info), "ANÁLISE DE FILTROS: %d BANDAS ATIVAS | FORMANTE: %+.1f ST", num_bands, formant_shift);
                dl->AddText(ImVec2(voc_p0.x + 12.0f, voc_p0.y + 6.0f), IM_COL32(0, 230, 255, 240), voc_info);

                ImGui::Dummy(voc_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CARRIER & MODULADOR ─────────────────────────
                ImGui::Columns(3, "VocodexCols", true);

                // Coluna 1: Carrier (Portadora)
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "⚡ CARRIER (SINAL BASE)");
                ImGui::Separator();
                
                const char* carrier_sources[] = { "Sintetizador Interno", "Sidechain Input (Faixa 2)" };
                ImGui::Combo("Fonte do Carrier", &carrier_type, carrier_sources, IM_ARRAYSIZE(carrier_sources));

                const char* carrier_waves[] = { "Sawtooth 80s (Rico)", "Pulse Oitavada", "Noise / Metallic" };
                ImGui::Combo("Forma de Onda", &internal_carrier_shape, carrier_waves, IM_ARRAYSIZE(carrier_waves));

                ImGui::SliderFloat("Carrier Volume", &carrier_synth_level, 0.0f, 1.5f, "%.2fx");

                ImGui::NextColumn();

                // Coluna 2: Modulador & Formantes (Voz)
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🎙️ MODULADOR & FORMANTE");
                ImGui::Separator();

                ImGui::SliderFloat("Ganho da Voz (Mic)", &modulator_mic_gain, 0.0f, 2.0f, "%.2fx");
                ImGui::SliderFloat("Deslocamento de Formante", &formant_shift, -12.0f, 12.0f, "%.1f st");
                ImGui::SliderFloat("Largura de Banda (BW)", &band_bandwidth, 0.2f, 3.0f, "%.2fx");
                ImGui::SliderInt("Contagem de Bandas", &num_bands, 16, 64);

                ImGui::NextColumn();

                // Coluna 3: Sibilância, Ruído & Contorno
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "🌊 CONTOURO & SIBILÂNCIA");
                ImGui::Separator();

                ImGui::SliderFloat("Ruído Sibilante (S/T)", &noise_generator_mix, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Soundgoodizer Vocoder", &soundgoodizer_contour, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão de Disparo / Teste do Vocoder
                if (ImGui::Button("🎤 DISPARAR VOCODER ROBÓTICO (TESTE COM VOZ SINTÉTICA + CARRIER)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(48, 1.5f, 0.9f, 4); // Dispara acorde de sintetizador do carrier
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
