#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroConvolverReverbUI {
    private:
        bool is_open = false;

        // Impulso de Convolução (IR - Impulse Response)
        std::string loaded_ir_name = "Alien_Spacecraft_Cathedral_4.2s.wav";
        float ir_decay_time = 3.5f;       // Segundos de cauda
        float pre_delay_ms = 25.0f;       // Atraso inicial
        float dry_level = 1.0f;
        float wet_level = 0.45f;
        float stereo_spread = 1.30f;      // Expansão estéreo M/S
        float eq_low_cut = 120.0f;        // Corte de graves para evitar embolamento
        float eq_high_damp = 7500.0f;     // Amortecimento de agudos do espaço
        int active_ir_preset = 0;

    public:
        KuroConvolverReverbUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.70f, 1.00f, 0.85f)); // Azul Espacial Convolver

            if (ImGui::Begin("🌌 FRUITY CONVOLVER (IMPULSE RESPONSE CONVOLUTION REVERB)###ConvolverWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS DE IMPULSO (IRs) ────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Impulsos de Sala (IR Presets)")) {
                        if (ImGui::MenuItem("🛸 Alien Deep Spacecraft Hangar (4.8s)")) {
                            loaded_ir_name = "Alien_Deep_Spacecraft_Hangar.wav";
                            ir_decay_time = 4.8f; stereo_spread = 1.5f; wet_level = 0.55f;
                        }
                        if (ImGui::MenuItem("⛪ Gothic Cathedral Sanctuary (3.2s)")) {
                            loaded_ir_name = "Gothic_Cathedral_Sanctuary.wav";
                            ir_decay_time = 3.2f; eq_low_cut = 150.0f; wet_level = 0.40f;
                        }
                        if (ImGui::MenuItem("🏟️ Stadium Arena Echo (2.0s)")) {
                            loaded_ir_name = "Stadium_Arena_Echo.wav";
                            ir_decay_time = 2.0f; pre_delay_ms = 40.0f;
                        }
                        if (ImGui::MenuItem("📼 Vintage Lexicon 480L Plate Reverb")) {
                            loaded_ir_name = "Vintage_Plate_480L.wav";
                            ir_decay_time = 1.8f; eq_high_damp = 9000.0f; wet_level = 0.35f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.2f, 0.80f, 1.0f, 1.0f), "FRUITY CONVOLVER ZERO-LATENCY IMPULSE RESPONSE CONVOLUTION PROCESSOR");
                ImGui::TextDisabled("Processador acústico de convolução matemática por resposta de impulso de espaços reais e emuladores vintage.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY GRÁFICO DA RESPOSTA DE IMPULSO (IR ENVELOPE) ──
                ImVec2 ir_p0 = ImGui::GetCursorScreenPos();
                ImVec2 ir_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 ir_p1 = ImVec2(ir_p0.x + ir_sz.x, ir_p0.y + ir_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(ir_p0, ir_p1, IM_COL32(10, 15, 24, 255), 4.0f);
                dl->AddRect(ir_p0, ir_p1, IM_COL32(35, 50, 75, 255), 4.0f);

                // Desenhar Curva de Decaimento Exponencial Real da Resposta de Impulso
                int num_points = (int)ir_sz.x;
                float cy = ir_p0.y + ir_sz.y * 0.5f;

                for (int x = 0; x < num_points; x += 2) {
                    float t = (float)x / (float)num_points;
                    // Envelope com cauda difusa de reflexões iniciais + densidade estocástica
                    float decay_curve = std::exp(-t * (4.5f / (ir_decay_time > 0.5f ? ir_decay_time : 0.5f)));
                    float noise_reflection = ((std::rand() % 100) / 100.0f - 0.5f) * 2.0f;
                    float wave = decay_curve * (0.4f * std::sin(t * 120.0f) + 0.6f * noise_reflection);

                    float py = cy - wave * (ir_sz.y * 0.44f);

                    // Gradiente Azul Ciano Convolver
                    dl->AddLine(ImVec2(ir_p0.x + x, cy), ImVec2(ir_p0.x + x, py), IM_COL32(0, (int)(160 + decay_curve * 95), 255, 220), 1.5f);
                }

                // Linha de Pré-Atraso (Pre-Delay Marker)
                float pdel_x = ir_p0.x + (pre_delay_ms / 100.0f) * (ir_sz.x * 0.25f);
                dl->AddLine(ImVec2(pdel_x, ir_p0.y), ImVec2(pdel_x, ir_p1.y), IM_COL32(255, 120, 0, 220), 2.0f);
                dl->AddText(ImVec2(pdel_x + 4, ir_p0.y + 4), IM_COL32(255, 180, 50, 240), "Pre-Delay");

                char ir_info[128];
                snprintf(ir_info, sizeof(ir_info), "IR: %s | DECAY: %.1fs | PRE-DELAY: %.0fms", loaded_ir_name.c_str(), ir_decay_time, pre_delay_ms);
                dl->AddText(ImVec2(ir_p0.x + 12.0f, ir_p1.y - 20.0f), IM_COL32(0, 230, 255, 240), ir_info);

                ImGui::Dummy(ir_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES ESPACIAIS & EQ DE CONTORNO ────────
                ImGui::Columns(3, "ConvolverCols", true);

                // Coluna 1: Níveis & Mix
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🎚️ NÍVEIS & BALANÇO");
                ImGui::Separator();

                ImGui::SliderFloat("Dry Level (Direto)", &dry_level, 0.0f, 1.5f, "%.2fx");
                ImGui::SliderFloat("Wet Level (Reverb)", &wet_level, 0.0f, 1.5f, "%.2fx");
                ImGui::SliderFloat("Pre-Delay", &pre_delay_ms, 0.0f, 150.0f, "%.0f ms");

                ImGui::NextColumn();

                // Coluna 2: Amortecimento & Filtros Acústicos
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🌊 FILTROS & AMORTECIMENTO");
                ImGui::Separator();

                ImGui::SliderFloat("Low Cut (Corte de Sub)", &eq_low_cut, 20.0f, 500.0f, "%.0f Hz");
                ImGui::SliderFloat("High Damp (Amortecimento)", &eq_high_damp, 2000.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Tempo de Decaimento", &ir_decay_time, 0.5f, 8.0f, "%.1f s");

                ImGui::NextColumn();

                // Coluna 3: Espalhamento Estéreo
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "🎧 IMAGEM ESTÉREO 3D");
                ImGui::Separator();

                ImGui::SliderFloat("Stereo Width Expander", &stereo_spread, 0.5f, 2.0f, "%.2fx");

                ImGui::Spacing();
                if (ImGui::Button("📂 CARREGAR NOVO ARQUIVO .WAV / IR...", ImVec2(-1, 30))) {
                    // Abre seletor de arquivo de impulso
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
