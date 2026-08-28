#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroHardcorePedalboardUI {
    private:
        bool is_open = false;

        // 5 Pedais Clássicos do FL Studio Hardcore Guitar Pedalboard:
        // 1. Distortion Pedal (Tube Screamer / Big Muff)
        bool pedal_dist_on = true;
        float dist_drive = 0.75f;
        float dist_tone = 0.60f;

        // 2. Chorus Pedal (Warm Analog Chorus)
        bool pedal_chorus_on = true;
        float chorus_rate = 0.8f;
        float chorus_depth = 0.65f;

        // 3. Delay Pedal (Analog Bucket Brigade)
        bool pedal_delay_on = false;
        float delay_time_ms = 320.0f;
        float delay_feedback = 0.50f;

        // 4. Reverb Pedal (Spring & Plate Reverb)
        bool pedal_reverb_on = true;
        float reverb_size = 0.70f;
        float reverb_decay = 0.60f;

        // 5. Cabinet Simulator (Gabinete de Alto-Falante Marshall 4x12 / Mesa Boogie)
        bool cab_sim_on = true;
        int cab_model = 0; // 0=Marshall 4x12 Vintage, 1=Mesa Boogie 4x12 Recto, 2=Fender Twin Reverb 2x12
        float master_output_gain = 1.0f;

    public:
        KuroHardcorePedalboardUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(980, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.20f, 0.20f, 0.85f)); // Vermelho Hardcore

            if (ImGui::Begin("🎸 HARDCORE GUITAR PEDALBOARD & CAB SIMULATOR (FL STUDIO STYLE)###HardcoreWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔥 Cyberpunk Heavy Metal Distortion Lead")) {
                            pedal_dist_on = true; dist_drive = 0.95f; dist_tone = 0.75f;
                            pedal_chorus_on = false; pedal_delay_on = true; delay_time_ms = 350.0f;
                            cab_model = 1;
                        }
                        if (ImGui::MenuItem("🌌 Shoegaze Dreamy Chorus & Long Reverb")) {
                            pedal_dist_on = false; pedal_chorus_on = true; chorus_depth = 0.85f;
                            pedal_reverb_on = true; reverb_size = 0.95f; cab_model = 2;
                        }
                        if (ImGui::MenuItem("🎸 Warm Vintage Blues Tube Drive")) {
                            pedal_dist_on = true; dist_drive = 0.40f; dist_tone = 0.50f;
                            pedal_reverb_on = true; cab_model = 0;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "HARDCORE: MULTI-STOMPBOX GUITAR PEDALBOARD & SPEAKER CABINET SIMULATOR");
                ImGui::TextDisabled("Pedalboard com pedais analógicos de Distorção, Chorus, Delay, Reverb e simulação de gabinetes valvulados.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. OS 4 PEDAIS DE CHÃO (STOMPBOXES) ──────────────────────
                ImGui::Columns(4, "PedalboardStomps", true);

                // --- PEDAL 1: DISTORTION ---
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.05f, 0.05f, 0.9f));
                ImGui::BeginChild("DistPedal", ImVec2(0, 240), true);
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "🔥 DISTORTION");
                ImGui::Separator();
                ImGui::Checkbox("Ligar Pedal##dist", &pedal_dist_on);
                ImGui::SliderFloat("Drive", &dist_drive, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Tone", &dist_tone, 0.0f, 1.0f, "%.2f");
                ImGui::Spacing();
                ImGui::RadioButton(pedal_dist_on ? "LED: [ATIVO]" : "LED: [OFF]", pedal_dist_on);
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::NextColumn();

                // --- PEDAL 2: CHORUS ---
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.12f, 0.18f, 0.9f));
                ImGui::BeginChild("ChorusPedal", ImVec2(0, 240), true);
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "🌊 CHORUS");
                ImGui::Separator();
                ImGui::Checkbox("Ligar Pedal##cho", &pedal_chorus_on);
                ImGui::SliderFloat("Rate", &chorus_rate, 0.1f, 5.0f, "%.1f Hz");
                ImGui::SliderFloat("Depth", &chorus_depth, 0.0f, 1.0f, "%.2f");
                ImGui::Spacing();
                ImGui::RadioButton(pedal_chorus_on ? "LED: [ATIVO]" : "LED: [OFF]", pedal_chorus_on);
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::NextColumn();

                // --- PEDAL 3: DELAY ---
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.16f, 0.10f, 0.9f));
                ImGui::BeginChild("DelayPedal", ImVec2(0, 240), true);
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "⏱️ DELAY");
                ImGui::Separator();
                ImGui::Checkbox("Ligar Pedal##del", &pedal_delay_on);
                ImGui::SliderFloat("Time", &delay_time_ms, 20.0f, 800.0f, "%.0f ms");
                ImGui::SliderFloat("Feedback", &delay_feedback, 0.0f, 0.95f, "%.2f");
                ImGui::Spacing();
                ImGui::RadioButton(pedal_delay_on ? "LED: [ATIVO]" : "LED: [OFF]", pedal_delay_on);
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::NextColumn();

                // --- PEDAL 4: REVERB ---
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.14f, 0.08f, 0.18f, 0.9f));
                ImGui::BeginChild("ReverbPedal", ImVec2(0, 240), true);
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 1.0f, 1.0f), "🌌 REVERB");
                ImGui::Separator();
                ImGui::Checkbox("Ligar Pedal##rev", &pedal_reverb_on);
                ImGui::SliderFloat("Size", &reverb_size, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Decay", &reverb_decay, 0.0f, 1.0f, "%.2f");
                ImGui::Spacing();
                ImGui::RadioButton(pedal_reverb_on ? "LED: [ATIVO]" : "LED: [OFF]", pedal_reverb_on);
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE SIMULAÇÃO DE GABINETE (CABINET SIMULATOR) ────
                ImGui::Columns(2, "CabCols", true);

                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🪵 SIMULAÇÃO DE GABINETE DE GUITARRA (CAB SIM)");
                ImGui::Checkbox("Ativar Gabinete Valvulado", &cab_sim_on);
                
                const char* cabs[] = { "Marshall 1960A 4x12 (Vintage Celestion)", "Mesa Boogie Dual Rectifier 4x12", "Fender Twin Reverb 2x12 Open Back" };
                ImGui::Combo("Modelo de Gabinete", &cab_model, cabs, IM_ARRAYSIZE(cabs));

                ImGui::NextColumn();

                ImGui::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "🎚️ MASTER OUTPUT & TESTE");
                ImGui::SliderFloat("Master Output Gain", &master_output_gain, 0.0f, 1.5f, "%.2fx");

                ImGui::Spacing();
                if (ImGui::Button("🎸 DISPARAR POWER CHORD DE GUITARRA (TESTE)", ImVec2(-1, 32))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(40, 2.0f, 0.9f, 2);
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
