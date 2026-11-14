#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityLimiterUI {
    private:
        bool is_open = false;

        // Seção Limiter (Lookahead & Ceiling)
        float limit_ceiling_db = -0.2f;    // Teto de pico verdadeiro (dBFS)
        float limit_gain_db = 2.5f;        // Ganho de entrada (Push)
        float limit_attack_ms = 4.5f;      // Ataque de Lookahead
        float limit_release_ms = 120.0f;   // Release da atenuação
        float limit_ahead_ms = 5.0f;       // Janela de antecipação (Lookahead)

        // Seção Compressor & Sidechain Ducking
        float comp_threshold_db = -14.0f;
        float comp_ratio = 4.0f;           // 1.0:1 a 20.0:1
        float comp_knee = 0.5f;            // Soft Knee
        int sidechain_source_track = 1;    // 0=Nenhum, 1=Kick Track, 2=Snare Track

        // Seção Noise Gate
        float gate_threshold_db = -45.0f;
        float gate_gain_reduction = -30.0f;
        bool enable_noise_gate = false;

        int active_tab = 0; // 0=Limiter, 1=Compressor, 2=Noise Gate

    public:
        KuroFruityLimiterUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.85f)); // Azul Ciano Limiter

            if (ImGui::Begin("🛡️ FRUITY LIMITER (LOOKAHEAD LIMITER, SIDECHAIN COMP & NOISE GATE)###FruityLimiterWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔥 Transparent Master Lookahead Ceiling (-0.1 dB)")) {
                            limit_ceiling_db = -0.1f; limit_gain_db = 1.8f; limit_ahead_ms = 5.0f;
                        }
                        if (ImGui::MenuItem("🥊 Heavy Sidechain Ducking (Kick Trigger)")) {
                            active_tab = 1; comp_threshold_db = -18.0f; comp_ratio = 8.0f; sidechain_source_track = 1;
                        }
                        if (ImGui::MenuItem("⚡ Aggressive Drum Bus Glue Compressor")) {
                            active_tab = 1; comp_threshold_db = -12.0f; comp_ratio = 3.5f; comp_knee = 0.8f;
                        }
                        if (ImGui::MenuItem("🎙️ Clean Vocal Noise Gate (-45 dB)")) {
                            active_tab = 2; enable_noise_gate = true; gate_threshold_db = -42.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "FRUITY LIMITER: PRECISION LOOKAHEAD PEAK LIMITER, SIDECHAIN & GATE");
                ImGui::TextDisabled("O limitador de referência do FL Studio com visualizador contínuo de curva de atenuação e compressão.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA FORMA DE ONDA E CURVA DE ATENUAÇÃO (GAIN REDUCTION)
                ImVec2 lm_p0 = ImGui::GetCursorScreenPos();
                ImVec2 lm_sz = ImVec2(ImGui::GetContentRegionAvail().x, 170.0f);
                ImVec2 lm_p1 = ImVec2(lm_p0.x + lm_sz.x, lm_p0.y + lm_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(lm_p0, lm_p1, IM_COL32(10, 14, 22, 255), 4.0f);
                dl->AddRect(lm_p0, lm_p1, IM_COL32(35, 50, 70, 255), 4.0f);

                // Linha de Ceiling (Teto Limiter)
                float ceil_norm = (limit_ceiling_db + 18.0f) / 18.0f; // -18dB a 0dB
                float ceil_y = lm_p1.y - ceil_norm * (lm_sz.y * 0.85f);
                dl->AddLine(ImVec2(lm_p0.x, ceil_y), ImVec2(lm_p1.x, ceil_y), IM_COL32(255, 60, 60, 240), 2.0f);

                char ceil_lbl[32];
                snprintf(ceil_lbl, sizeof(ceil_lbl), "Ceiling: %.1f dBFS", limit_ceiling_db);
                dl->AddText(ImVec2(lm_p0.x + 8.0f, ceil_y - 14.0f), IM_COL32(255, 100, 100, 240), ceil_lbl);

                // Linha de Threshold de Compressão
                float thresh_norm = (comp_threshold_db + 30.0f) / 30.0f;
                float thresh_y = lm_p1.y - thresh_norm * (lm_sz.y * 0.65f);
                dl->AddLine(ImVec2(lm_p0.x, thresh_y), ImVec2(lm_p1.x, thresh_y), IM_COL32(0, 220, 255, 180), 1.0f);

                // Desenhar Curva da Onda de Áudio e Curva de Atenuação (Gain Reduction Envelope)
                int steps = (int)lm_sz.x;
                float cy = lm_p0.y + lm_sz.y * 0.6f;

                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps;
                    float raw_signal = std::sin(t * 35.0f) * std::exp(-std::fmod(t * 6.0f, 1.0f) * 2.5f) * (1.0f + limit_gain_db * 0.2f);
                    
                    // Onda Direta
                    float py = cy - raw_signal * (lm_sz.y * 0.35f);
                    dl->AddLine(ImVec2(lm_p0.x + x, cy), ImVec2(lm_p0.x + x, py), IM_COL32(40, 150, 255, 160), 1.5f);

                    // Curva de Redução de Ganho (Laranja/Lilás no topo)
                    float gr_env = (raw_signal > 0.7f) ? (raw_signal - 0.7f) * 1.5f : 0.0f;
                    float gr_y = lm_p0.y + 10.0f + gr_env * 35.0f;
                    dl->AddLine(ImVec2(lm_p0.x + x, lm_p0.y + 10.0f), ImVec2(lm_p0.x + x, gr_y), IM_COL32(255, 140, 0, 200), 1.5f);
                }

                dl->AddText(ImVec2(lm_p0.x + 12.0f, lm_p0.y + 6.0f), IM_COL32(255, 160, 0, 240), "ENVELOPE DE REDUÇÃO DE GANHO (GAIN REDUCTION)");
                ImGui::Dummy(lm_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SELEÇÃO DE ABAS (LIMITER | COMPRESSOR | NOISE GATE) ──
                if (ImGui::Button(active_tab == 0 ? "🛡️ [ABA LIMITER ATIVA]" : "🛡️ Limiter", ImVec2(180, 28))) active_tab = 0;
                ImGui::SameLine();
                if (ImGui::Button(active_tab == 1 ? "🥊 [ABA COMPRESSOR ATIVA]" : "🥊 Compressor / Sidechain", ImVec2(220, 28))) active_tab = 1;
                ImGui::SameLine();
                if (ImGui::Button(active_tab == 2 ? "🚪 [ABA NOISE GATE ATIVA]" : "🚪 Noise Gate", ImVec2(180, 28))) active_tab = 2;

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. PAINEL DA ABA SELECIONADA ────────────────────────────
                if (active_tab == 0) {
                    // ABA LIMITER
                    ImGui::Columns(3, "LimiterCols", true);

                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "🛡️ CEILING & GANHO");
                    ImGui::SliderFloat("Ceiling (Teto Máximo)", &limit_ceiling_db, -12.0f, 0.0f, "%.1f dBFS");
                    ImGui::SliderFloat("Gain (Push In)", &limit_gain_db, 0.0f, 12.0f, "%+.1f dB");

                    ImGui::NextColumn();

                    ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "⏱️ TEMPOS DE ATUAÇÃO");
                    ImGui::SliderFloat("Attack Time", &limit_attack_ms, 0.1f, 50.0f, "%.1f ms");
                    ImGui::SliderFloat("Release Time", &limit_release_ms, 10.0f, 800.0f, "%.0f ms");

                    ImGui::NextColumn();

                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "👁️ LOOKAHEAD (ANTECIPAÇÃO)");
                    ImGui::SliderFloat("Ahead Window", &limit_ahead_ms, 0.0f, 20.0f, "%.1f ms");
                    ImGui::ProgressBar(0.85f, ImVec2(-1, 20), "True Peak Safe: -0.1 dBFS");

                    ImGui::Columns(1);
                } else if (active_tab == 1) {
                    // ABA COMPRESSOR
                    ImGui::Columns(3, "CompCols", true);

                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🥊 THRESHOLD & RATIO");
                    ImGui::SliderFloat("Threshold", &comp_threshold_db, -40.0f, 0.0f, "%.1f dB");
                    ImGui::SliderFloat("Ratio", &comp_ratio, 1.0f, 20.0f, "%.1f:1");
                    ImGui::SliderFloat("Knee (Curvatura)", &comp_knee, 0.0f, 1.0f, "%.2f");

                    ImGui::NextColumn();

                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🎧 ENTRADA DE SIDECHAIN");
                    const char* sc_sources[] = { "Nenhum (Sinal Próprio)", "Canal 1 (Bateria / Kick)", "Canal 2 (Caixa / Snare)" };
                    ImGui::Combo("Sidechain Source", &sidechain_source_track, sc_sources, IM_ARRAYSIZE(sc_sources));

                    ImGui::NextColumn();

                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "📊 ATENUAÇÃO ATIVA");
                    ImGui::ProgressBar(0.45f, ImVec2(-1, 24), "Sidechain Ducking: -6.2 dB");

                    ImGui::Columns(1);
                } else {
                    // ABA NOISE GATE
                    ImGui::Columns(2, "GateCols", true);

                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "🚪 PARÂMETROS DO GATE");
                    ImGui::Checkbox("Ativar Noise Gate", &enable_noise_gate);
                    ImGui::SliderFloat("Gate Threshold", &gate_threshold_db, -80.0f, -10.0f, "%.1f dB");
                    ImGui::SliderFloat("Redução de Ganho", &gate_gain_reduction, -60.0f, 0.0f, "%.1f dB");

                    ImGui::NextColumn();

                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🔇 SILÊNCIO LIMPO");
                    ImGui::TextDisabled("Elimina ruídos de fundo e vazamento de microfones quando o sinal cai abaixo do limiar.");

                    ImGui::Columns(1);
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
