#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct OscConfig {
        int shape;       // 0=Sine, 1=Triangle, 2=Saw, 3=Square, 4=Noise
        int octave;      // -3 a +3
        int semitone;    // -12 a +12
        int fine_cents;  // -50 a +50
        float volume;    // 0.0 a 1.0
        float pan;       // -1.0 a +1.0
        bool enabled;
    };

    class Kuro3xOscSynthUI {
    private:
        bool is_open = false;

        // Os 3 Osciladores Lendários do 3x Osc (FL Studio Classic Subtractive Synth)
        OscConfig oscs[3] = {
            { 2,  0, 0,  -8, 1.0f, -0.4f, true }, // OSC 1: Saw, 0 Oct, -8 cents (Left detune)
            { 2,  0, 0,  +8, 0.9f, +0.4f, true }, // OSC 2: Saw, 0 Oct, +8 cents (Right detune)
            { 0, -1, 0,   0, 0.8f,  0.0f, true }  // OSC 3: Sine Sub-Bass, -1 Oct
        };

        // Filtro & Envelope ADSR Integrado
        float filter_cutoff_hz = 4500.0f;
        float filter_res_q = 0.40f;
        float env_attack = 0.02f;
        float env_decay = 0.35f;
        float env_sustain = 0.60f;
        float env_release = 0.40f;
        bool stereo_phase_inversion = false;

    public:
        Kuro3xOscSynthUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.70f, 0.00f, 0.85f)); // Laranja Ouro 3x Osc

            if (ImGui::Begin("🎹 3x OSC (CLASSIC TRIPLE OSCILLATOR SUBTRACTIVE SYNTH)###ThreeOscWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🛸 Psytrance Super Saw Detuned Lead")) {
                            oscs[0].shape = 2; oscs[0].fine_cents = -12; oscs[0].pan = -0.6f;
                            oscs[1].shape = 2; oscs[1].fine_cents = +12; oscs[1].pan = +0.6f;
                            oscs[2].shape = 2; oscs[2].octave = -1; oscs[2].volume = 0.7f;
                            filter_cutoff_hz = 6500.0f; filter_res_q = 0.55f;
                        }
                        if (ImGui::MenuItem("💥 Reese Sub-Bass (Thick Low-End Detune)")) {
                            oscs[0].shape = 2; oscs[0].octave = -1; oscs[0].fine_cents = -5;
                            oscs[1].shape = 2; oscs[1].octave = -1; oscs[1].fine_cents = +5;
                            oscs[2].shape = 0; oscs[2].octave = -2; oscs[2].volume = 1.0f;
                            filter_cutoff_hz = 800.0f;
                        }
                        if (ImGui::MenuItem("🕹️ 8-Bit Chiptune Square Lead")) {
                            oscs[0].shape = 3; oscs[1].shape = 3; oscs[1].semitone = 7; // 5th Interval
                            oscs[2].enabled = false;
                        }
                        if (ImGui::MenuItem("🌌 Cosmic White Noise Sweep & Riser")) {
                            oscs[0].shape = 4; oscs[1].shape = 4; oscs[2].shape = 4;
                            filter_cutoff_hz = 2500.0f; filter_res_q = 0.85f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.1f, 1.0f), "3x OSC: ICONIC TRIPLE ANALOG OSCILLATOR SUBTRACTIVE SYNTHESIZER");
                ImGui::TextDisabled("O sintetizador lendário responsável pelos maiores clássicos de Lead, Bass e Plucks da música eletrônica mundial.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA FORMA DE ONDA RESULTANTE EM TEMPO REAL
                ImVec2 osc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 osc_sz = ImVec2(ImGui::GetContentRegionAvail().x, 140.0f);
                ImVec2 osc_p1 = ImVec2(osc_p0.x + osc_sz.x, osc_p0.y + osc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(osc_p0, osc_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                dl->AddRect(osc_p0, osc_p1, IM_COL32(40, 50, 65, 255), 4.0f);

                // Desenhar a Soma dos 3 Osciladores
                int steps = (int)osc_sz.x;
                float cy = osc_p0.y + osc_sz.y * 0.5f;

                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps * 6.2831853f * 2.0f;
                    float wave_sum = 0.0f;

                    for (int o = 0; o < 3; ++o) {
                        if (!oscs[o].enabled) continue;
                        float freq_mult = std::pow(2.0f, (float)oscs[o].octave + (oscs[o].semitone / 12.0f) + (oscs[o].fine_cents / 1200.0f));
                        float phase = t * freq_mult;

                        float osc_val = 0.0f;
                        if (oscs[o].shape == 0) osc_val = std::sin(phase);
                        else if (oscs[o].shape == 1) osc_val = (std::abs(std::fmod(phase / 3.14159265f, 2.0f) - 1.0f) - 0.5f) * 2.0f;
                        else if (oscs[o].shape == 2) osc_val = (std::fmod(phase / 6.2831853f, 1.0f) - 0.5f) * 2.0f;
                        else if (oscs[o].shape == 3) osc_val = (std::sin(phase) > 0.0f) ? 1.0f : -1.0f;
                        else osc_val = ((std::rand() % 100) / 100.0f - 0.5f) * 2.0f;

                        wave_sum += osc_val * oscs[o].volume;
                    }

                    wave_sum = std::clamp(wave_sum * 0.5f, -1.2f, 1.2f);
                    float py = cy - wave_sum * (osc_sz.y * 0.40f);

                    dl->AddLine(ImVec2(osc_p0.x + x, cy), ImVec2(osc_p0.x + x, py), IM_COL32(255, 180, 0, 180), 1.5f);
                }

                char osc_info[64];
                snprintf(osc_info, sizeof(osc_info), "SAÍDA: OSC 1 (%s) | OSC 2 (%s) | OSC 3 (%s)", oscs[0].enabled ? "ON" : "OFF", oscs[1].enabled ? "ON" : "OFF", oscs[2].enabled ? "ON" : "OFF");
                dl->AddText(ImVec2(osc_p0.x + 12.0f, osc_p0.y + 6.0f), IM_COL32(255, 200, 50, 240), osc_info);

                ImGui::Dummy(osc_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. OS 3 OSCILADORES LADO A LADO ─────────────────────────
                ImGui::Columns(3, "OscGridCols", true);

                const char* shape_names[] = { "Senoide (Sine)", "Triângulo (Tri)", "Dente de Serra (Saw)", "Onda Quadrada (Square)", "Ruído (Noise)" };

                for (int o = 0; o < 3; ++o) {
                    ImGui::PushID(o + 500);
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.1f, 1.0f), "⚡ OSCILADOR %d", o + 1);
                    ImGui::Checkbox("Ativar Oscilador", &oscs[o].enabled);

                    ImGui::Combo("Forma de Onda", &oscs[o].shape, shape_names, IM_ARRAYSIZE(shape_names));
                    ImGui::SliderInt("Oitava (Oct)", &oscs[o].octave, -3, 3);
                    ImGui::SliderInt("Semitom (Coarse)", &oscs[o].semitone, -12, 12);
                    ImGui::SliderInt("Detune Fino (Cents)", &oscs[o].fine_cents, -50, 50);
                    ImGui::SliderFloat("Volume", &oscs[o].volume, 0.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Pan", &oscs[o].pan, -1.0f, 1.0f, "%.2f");

                    ImGui::PopID();
                    if (o < 2) ImGui::NextColumn();
                }

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. FILTRO & BOTÃO DE TESTE DIRETO ───────────────────────
                ImGui::Columns(2, "OscMasterCols", true);

                ImGui::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "🎛️ FILTRO PASSABAIXAS & ENVELOPE");
                ImGui::SliderFloat("Cutoff (Frequência)", &filter_cutoff_hz, 20.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Ressonância (Q)", &filter_res_q, 0.0f, 0.95f, "%.2f");

                ImGui::NextColumn();

                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎹 DISPARO DE TESTE");
                if (ImGui::Button("🎵 TOCAR 3x OSC (C4 - 261.63 Hz)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(60, 2.0f, 0.9f, 0); // Dispara sintetizador polifônico
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
