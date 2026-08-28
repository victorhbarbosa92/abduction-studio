#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct PeakControllerState {
        bool active = true;
        int source_track = 0; // 0 = Master, 1 = Kick, etc.
        float base = 0.0f;    // 0.0 a 1.0
        float volume = 1.0f;  // Multiplicador -2.0 a +2.0
        float tension = 0.0f; // Curva de tensão (-1.0 a +1.0)
        float decay = 0.5f;   // 0.05s a 2.0s
        float peak_meter = 0.0f;
        float lfo_speed = 1.0f;
        float lfo_amount = 0.0f;
        int lfo_shape = 0;    // 0=Sine, 1=Triangle, 2=Square, 3=Saw
        int target_track = 3; // Bassline
        int target_param = 0; // Cutoff / Volume
        bool mute_source = false;
    };

    class KuroPeakControllerUI {
    private:
        bool is_open = false;
        PeakControllerState state;

    public:
        KuroPeakControllerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        PeakControllerState& getState() { return state; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(620, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.11f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.70f, 0.80f));

            if (ImGui::Begin("⚡ FRUITY PEAK CONTROLLER (Auto Sidechain & Envelope Follower)###PeakControllerWindow", &is_open, ImGuiWindowFlags_NoCollapse)) {
                ImDrawList* dl = ImGui::GetWindowDrawList();

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.75f, 1.0f), "🎛️ PEAK & LFO MODULATION CONTROLLER (ESTILO FL STUDIO)");
                ImGui::TextDisabled("Gera automação contínua a partir dos picos de áudio de qualquer faixa (Ex: Kick -> Sidechain Bass/Cutoff)");
                ImGui::Separator();
                ImGui::Spacing();

                // Colunas: PEAK FOLLOWER | LFO GENERATOR
                ImGui::Columns(2, "PeakCols", true);

                // --- 1. PEAK ENVELOPE FOLLOWER ---
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "📈 SEGUIDOR DE PICO (PEAK)");
                ImGui::Separator();
                ImGui::Spacing();

                const char* tracks[] = { "Master", "1: Kick", "2: Snare", "3: Hat", "4: Bass", "5: Chord", "6: Lead", "7: FX 01", "8: Extra" };
                ImGui::Combo("Faixa de Entrada", &state.source_track, tracks, IM_ARRAYSIZE(tracks));
                
                ImGui::SliderFloat("Base", &state.base, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Volume (Gain)", &state.volume, -2.0f, 2.0f, "%.2fx");
                ImGui::SliderFloat("Tensão (Curve)", &state.tension, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Decaimento (Decay)", &state.decay, 0.01f, 2.0f, "%.2f s");
                ImGui::Checkbox("Silenciar Entrada (Mute)", &state.mute_source);

                // Display Visual do Medidor de Pico Dinâmico
                ImGui::Spacing();
                ImGui::Text("Saída Peak Atual:");
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + 22.0f);
                dl->AddRectFilled(p0, p1, IM_COL32(20, 24, 30, 255), 4.0f);
                
                extern float track_vu_levels[20];
                extern float master_vu_level_l;
                float current_in_peak = (state.source_track == 0) ? master_vu_level_l : ((state.source_track - 1 < 8) ? track_vu_levels[state.source_track - 1] : 0.0f);
                float out_peak = std::clamp(state.base + (current_in_peak * state.volume), 0.0f, 1.0f);
                
                dl->AddRectFilled(p0, ImVec2(p0.x + (p1.x - p0.x) * out_peak, p1.y), IM_COL32(0, 230, 160, 220), 4.0f);
                dl->AddRect(p0, p1, IM_COL32(60, 80, 100, 255), 4.0f);

                char peak_txt[32];
                snprintf(peak_txt, sizeof(peak_txt), "Peak Out: %.0f%%", out_peak * 100.0f);
                dl->AddText(ImVec2(p0.x + 8.0f, p0.y + 3.0f), IM_COL32(255, 255, 255, 255), peak_txt);
                ImGui::Dummy(ImVec2(0, 26.0f));

                ImGui::NextColumn();

                // --- 2. GERADOR LFO ---
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "🌊 OSCILADOR LFO INTEGRADO");
                ImGui::Separator();
                ImGui::Spacing();

                const char* lfo_shapes[] = { "Senoide (Sine)", "Triângulo (Triangle)", "Quadrada (Square)", "Dente de Serra (Saw)" };
                ImGui::Combo("Forma de Onda##lfo", &state.lfo_shape, lfo_shapes, IM_ARRAYSIZE(lfo_shapes));
                ImGui::SliderFloat("Velocidade (Speed)", &state.lfo_speed, 0.1f, 20.0f, "%.2f Hz");
                ImGui::SliderFloat("Intensidade (Amount)", &state.lfo_amount, 0.0f, 1.0f, "%.2f");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🎯 ROTEAMENTO DE DESTINO (ROUTING)");
                ImGui::Combo("Faixa Alvo", &state.target_track, tracks, IM_ARRAYSIZE(tracks));
                
                const char* params[] = { "Volume da Faixa (Sidechain Ducking)", "Filtro Cutoff (Auto-Wah)", "Efeito Overdrive", "Pitch Offset" };
                ImGui::Combo("Parâmetro Alvo", &state.target_param, params, IM_ARRAYSIZE(params));

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão de Aplicação Rápida de Sidechain Pump
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.50f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.85f, 0.65f, 1.0f));
                if (ImGui::Button("⚡ ATIVAR SIDECHAIN DUCKING AUTOMÁTICO (KICK -> BASS)", ImVec2(-1, 38))) {
                    state.source_track = 1; // Kick
                    state.target_track = 4; // Bass
                    state.target_param = 0; // Volume Ducking
                    state.base = 1.0f;
                    state.volume = -1.2f;   // Inverte o pico para atenuar o baixo quando o bumbo bate
                    state.decay = 0.25f;
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
