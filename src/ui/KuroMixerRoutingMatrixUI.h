#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct KuroSidechainDucker {
        bool enabled = true;
        int trigger_track = 1;       // 1 = Track 1 (Kick)
        int target_track = 2;        // 2 = Track 2 (Bass)
        float threshold_db = -18.0f; // -40dB a 0dB
        float ratio = 4.0f;          // 1:1 a 20:1
        float attack_ms = 2.5f;      // 0.5ms a 50ms
        float release_ms = 85.0f;    // 10ms a 400ms
        float ducking_depth = 0.85f; // 0.0 a 1.0 (85%)
        float current_gr_db = -14.2f;

        // Waveform history buffers for dynamic oscilloscope rendering (128 samples)
        float trigger_history[128] = { 0 };
        float target_history[128] = { 0 };
        float gr_history[128] = { 0 };

        void update(float time_sec) {
            // Gera oscilograma dinâmico de Sidechain Ducking estilo Psytrance (Kick 4-on-the-floor com Bass pumping)
            float beat_phase = std::fmod(time_sec * 2.366f, 1.0f); // ~142 BPM
            for (int i = 0; i < 128; i++) {
                float sample_phase = std::fmod(beat_phase + (float)i / 128.0f, 1.0f);
                
                // Kick waveform: transient punch at start of beat
                float kick_env = (sample_phase < 0.22f) ? std::exp(-sample_phase * 18.0f) * std::sin(sample_phase * 120.0f) : (std::sin(sample_phase * 30.0f) * 0.05f);
                trigger_history[i] = kick_env * 0.95f;

                // Gain reduction envelope: drops aggressively on kick hit, releases smoothly
                float gr_dip = 0.0f;
                if (sample_phase < 0.12f) {
                    gr_dip = (sample_phase / 0.12f); // Attack
                } else if (sample_phase < 0.65f) {
                    float rel_t = (sample_phase - 0.12f) / 0.53f;
                    gr_dip = 1.0f - std::pow(rel_t, 0.7f); // Exponential release
                }
                float gr_val = gr_dip * ducking_depth;
                gr_history[i] = gr_val; // 0.0 (no reduction) to 1.0 (full reduction)

                // Bass waveform: rolling 16ths suppressed during kick transient, then pumps to full level
                float bass_raw = std::sin(sample_phase * 240.0f) * 0.85f + std::sin(sample_phase * 480.0f) * 0.35f;
                target_history[i] = bass_raw * (1.0f - gr_val * 0.90f);
            }
            float current_dip = gr_history[24];
            current_gr_db = -current_dip * (std::abs(threshold_db) * (ratio / (ratio + 1.0f)));
        }
    };

    class KuroMixerRoutingMatrixUI {
    private:
        bool is_open = false;
        // Matriz de Roteamento 17x17 (0 = Master, 1..16 = Tracks)
        // 14 = Reverb Bus (Send A), 15 = Delay Bus (Send B), 16 = Sidechain Bus
        float send_matrix[17][17] = { 0 };
        bool send_enable[17][17] = { false };
        bool is_sidechain_only[17][17] = { false };

    public:
        KuroSidechainDucker sidechain;
        int right_panel_tab = 0; // 0 = Effects Rack (10 Slots), 1 = Sidechain Ducking Visualizer

        KuroMixerRoutingMatrixUI() {
            resetToDefault();
        }

        void resetToDefault() {
            for (int s = 0; s < 17; ++s) {
                for (int d = 0; d < 17; ++d) {
                    send_matrix[s][d] = 0.0f;
                    send_enable[s][d] = false;
                    is_sidechain_only[s][d] = false;
                }
            }
            // Todas as faixas 1..16 enviam por padrão para a Master (0) com ganho 1.0f
            for (int t = 1; t <= 16; ++t) {
                send_enable[t][0] = true;
                send_matrix[t][0] = 1.0f;
            }
            // Roteamento padrão de Sidechain (Kick Faixa 1 -> Bass Faixa 2 e Sidechain Bus 16)
            send_enable[1][2] = true;
            send_matrix[1][2] = 0.85f;
            is_sidechain_only[1][2] = true;

            send_enable[1][16] = true;
            send_matrix[1][16] = 1.0f;
            is_sidechain_only[1][16] = true;

            // Roteamento de Send FX:
            // Faixa 3 (Snare) -> Reverb Send 14
            send_enable[3][14] = true;
            send_matrix[3][14] = 0.30f;

            // Faixa 6 (Lead) -> Reverb Send 14 & Delay Send 15
            send_enable[6][14] = true;
            send_matrix[6][14] = 0.40f;
            send_enable[6][15] = true;
            send_matrix[6][15] = 0.35f;

            // Faixa 7 (Chords/Pad) -> Reverb Send 14
            send_enable[7][14] = true;
            send_matrix[7][14] = 0.50f;
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        bool isConnected(int src, int dest) const {
            if (src < 0 || src >= 17 || dest < 0 || dest >= 17) return false;
            return send_enable[src][dest];
        }

        bool isSidechain(int src, int dest) const {
            if (src < 0 || src >= 17 || dest < 0 || dest >= 17) return false;
            return send_enable[src][dest] && is_sidechain_only[src][dest];
        }

        float getSendGain(int src, int dest) const {
            if (src < 0 || src >= 17 || dest < 0 || dest >= 17) return 0.0f;
            return send_enable[src][dest] ? send_matrix[src][dest] : 0.0f;
        }

        void setSendGain(int src, int dest, float gain) {
            if (src >= 0 && src < 17 && dest >= 0 && dest < 17) {
                send_matrix[src][dest] = std::clamp(gain, 0.0f, 1.25f);
            }
        }

        void toggleRoute(int src, int dest, bool sidechain_flag = false) {
            if (src < 0 || src >= 17 || dest < 0 || dest >= 17 || src == dest) return;
            send_enable[src][dest] = !send_enable[src][dest];
            if (send_enable[src][dest]) {
                if (send_matrix[src][dest] <= 0.01f) send_matrix[src][dest] = 0.8f;
                is_sidechain_only[src][dest] = sidechain_flag;
            }
        }

        // =========================================================================
        // ── RENDERIZAÇÃO DO RACK DE ROTEAMENTO INFERIOR COM CABOS VIRTUAIS ────────
        // =========================================================================
        void RenderBottomRoutingRack(ImDrawList* dl, ImVec2 pos, ImVec2 size, int selected_track) {
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(7, 10, 16, 250), 6.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 38, 56, 255), 6.0f, 0, 1.5f);

            // Parafusos sextavados nos 4 cantos para estética analógica cyberpunk
            auto drawScrew = [&](ImVec2 sp) {
                dl->AddCircleFilled(sp, 3.5f, IM_COL32(40, 52, 68, 255));
                dl->AddCircle(sp, 3.5f, IM_COL32(80, 100, 125, 200), 12, 1.0f);
                dl->AddLine(ImVec2(sp.x - 2.0f, sp.y), ImVec2(sp.x + 2.0f, sp.y), IM_COL32(20, 26, 34, 255), 1.2f);
            };
            drawScrew(ImVec2(pos.x + 8.0f, pos.y + 8.0f));
            drawScrew(ImVec2(pos.x + size.x - 8.0f, pos.y + 8.0f));
            drawScrew(ImVec2(pos.x + 8.0f, pos.y + size.y - 8.0f));
            drawScrew(ImVec2(pos.x + size.x - 8.0f, pos.y + size.y - 8.0f));

            // Título do Rack
            char title_buf[64];
            snprintf(title_buf, sizeof(title_buf), "⚡ ROUTING PATCH BAY & SENDS — TRACK %02d", selected_track);
            dl->AddText(ImVec2(pos.x + 24.0f, pos.y + 6.0f), IM_COL32(0, 229, 255, 240), title_buf);

            // Jack de Origem (Canal Selecionado)
            float jack_start_x = pos.x + 36.0f;
            float jack_spacing = std::clamp((size.x * 0.45f) / 4.0f, 65.0f, 92.0f);

            ImVec2 src_jack(jack_start_x, pos.y + 45.0f);
            dl->AddCircleFilled(src_jack, 12.0f, IM_COL32(14, 20, 30, 255));
            dl->AddCircle(src_jack, 12.0f, IM_COL32(0, 229, 255, 220), 16, 2.0f);
            dl->AddCircleFilled(src_jack, 5.0f, IM_COL32(0, 229, 255, 255));
            dl->AddText(ImVec2(jack_start_x - 18.0f, pos.y + 64.0f), IM_COL32(180, 210, 240, 220), "SOURCE");

            // Jacks de Destino
            struct DestJack {
                int id;
                const char* name;
                ImU32 color;
                ImVec2 pos;
            };

            std::vector<DestJack> dests = {
                { 0,  "Master",    IM_COL32(255, 195, 30, 255),  ImVec2(jack_start_x + jack_spacing * 1.0f, pos.y + 45.0f) },
                { 14, "Reverb",    IM_COL32(180, 75, 245, 255),  ImVec2(jack_start_x + jack_spacing * 2.0f, pos.y + 45.0f) },
                { 15, "Delay",     IM_COL32(0, 229, 255, 255),   ImVec2(jack_start_x + jack_spacing * 3.0f, pos.y + 45.0f) },
                { 16, "Sidechain", IM_COL32(255, 0, 128, 255),   ImVec2(jack_start_x + jack_spacing * 4.0f, pos.y + 45.0f) }
            };

            for (const auto& dj : dests) {
                bool conn = (selected_track > 0) && isConnected(selected_track, dj.id);
                bool sc = (selected_track > 0) && isSidechain(selected_track, dj.id);

                // Jack Socket
                dl->AddCircleFilled(dj.pos, 12.0f, IM_COL32(14, 20, 30, 255));
                dl->AddCircle(dj.pos, 12.0f, conn ? dj.color : IM_COL32(50, 65, 85, 180), 16, conn ? 2.0f : 1.2f);
                dl->AddCircleFilled(dj.pos, 5.0f, conn ? dj.color : IM_COL32(30, 42, 56, 255));
                
                // Rótulo
                dl->AddText(ImVec2(dj.pos.x - 20.0f, pos.y + 64.0f), conn ? dj.color : IM_COL32(110, 130, 150, 180), dj.name);

                // Se conectado, desenha o cabo Bézier com brilho neon
                if (conn && selected_track > 0) {
                    ImVec2 p0 = src_jack;
                    ImVec2 p3 = dj.pos;
                    float sag = 28.0f + std::abs(p3.x - p0.x) * 0.12f;
                    ImVec2 p1 = ImVec2(p0.x + (p3.x - p0.x) * 0.35f, p0.y + sag);
                    ImVec2 p2 = ImVec2(p0.x + (p3.x - p0.x) * 0.65f, p3.y + sag);

                    // Glow exterior do cabo
                    ImU32 glow_col = (dj.color & 0x00FFFFFF) | 0x40000000;
                    dl->AddBezierCubic(p0, p1, p2, p3, glow_col, 5.5f);
                    // Núcleo iluminado do cabo
                    dl->AddBezierCubic(p0, p1, p2, p3, dj.color, 2.2f);

                    // Indicador de tag (ex: 85% ou SC) no ponto mais baixo do cabo
                    ImVec2 mid_c(p0.x + (p3.x - p0.x) * 0.5f, p0.y + sag * 0.85f);
                    char tag_buf[16];
                    if (sc) snprintf(tag_buf, sizeof(tag_buf), "SC");
                    else snprintf(tag_buf, sizeof(tag_buf), "%.0f%%", send_matrix[selected_track][dj.id] * 100.0f);

                    dl->AddRectFilled(ImVec2(mid_c.x - 14.0f, mid_c.y - 7.0f), ImVec2(mid_c.x + 14.0f, mid_c.y + 7.0f), IM_COL32(8, 12, 18, 220), 3.0f);
                    dl->AddRect(ImVec2(mid_c.x - 14.0f, mid_c.y - 7.0f), ImVec2(mid_c.x + 14.0f, mid_c.y + 7.0f), dj.color, 3.0f, 0, 1.0f);
                    dl->AddText(ImVec2(mid_c.x - 10.0f, mid_c.y - 6.0f), dj.color, tag_buf);
                }
            }

            // Controles de envio no lado direito do rack
            float ctrl_x = jack_start_x + jack_spacing * 4.0f + 48.0f;
            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, pos.y + 10.0f));
            ImGui::PushID(selected_track + 7711);

            if (selected_track > 0) {
                // Botões de Ação Rápida
                if (ImGui::Button("⚡ SC Kick->Bass", ImVec2(118, 22))) {
                    toggleRoute(selected_track, 2, true);
                    sidechain.trigger_track = selected_track;
                    sidechain.target_track = 2;
                }
                ImGui::SameLine(0, 6);
                if (ImGui::Button("🔊 Reverb Bus", ImVec2(94, 22))) {
                    toggleRoute(selected_track, 14, false);
                }
                ImGui::SameLine(0, 6);
                if (ImGui::Button("🔊 Delay Bus", ImVec2(90, 22))) {
                    toggleRoute(selected_track, 15, false);
                }
                ImGui::SameLine(0, 6);
                if (ImGui::Button("✖ Limpar", ImVec2(60, 22))) {
                    for (int d = 1; d < 17; ++d) {
                        send_enable[selected_track][d] = false;
                    }
                }

                // Knobs/Sliders de Envio para o canal selecionado
                ImGui::SetCursorScreenPos(ImVec2(ctrl_x, pos.y + 40.0f));
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Envios: ");
                ImGui::SameLine(0, 4);
                ImGui::SetNextItemWidth(62.0f);
                float m_gain = send_matrix[selected_track][0] * 100.0f;
                if (ImGui::SliderFloat("##m_gain", &m_gain, 0.0f, 100.0f, "M:%.0f%%")) {
                    send_matrix[selected_track][0] = m_gain / 100.0f;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Envio para Master");

                ImGui::SameLine(0, 6);
                ImGui::SetNextItemWidth(62.0f);
                float rev_gain = send_matrix[selected_track][14] * 100.0f;
                if (ImGui::SliderFloat("##rev_gain", &rev_gain, 0.0f, 100.0f, "R:%.0f%%")) {
                    send_matrix[selected_track][14] = rev_gain / 100.0f;
                    send_enable[selected_track][14] = (rev_gain > 0.5f);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Envio para Reverb Bus");

                ImGui::SameLine(0, 6);
                ImGui::SetNextItemWidth(62.0f);
                float dly_gain = send_matrix[selected_track][15] * 100.0f;
                if (ImGui::SliderFloat("##dly_gain", &dly_gain, 0.0f, 100.0f, "D:%.0f%%")) {
                    send_matrix[selected_track][15] = dly_gain / 100.0f;
                    send_enable[selected_track][15] = (dly_gain > 0.5f);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Envio para Delay Bus");

                ImGui::SameLine(0, 6);
                ImGui::SetNextItemWidth(62.0f);
                float sc_gain = send_matrix[selected_track][16] * 100.0f;
                if (ImGui::SliderFloat("##sc_gain", &sc_gain, 0.0f, 100.0f, "SC:%.0f%%")) {
                    send_matrix[selected_track][16] = sc_gain / 100.0f;
                    send_enable[selected_track][16] = (sc_gain > 0.5f);
                    is_sidechain_only[selected_track][16] = true;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Nível de Envio Sidechain Ducking");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Canal Master Selecionado — Saída Principal para a Interface de Áudio");
            }
            ImGui::PopID();
        }

        // =========================================================================
        // ── RENDERIZAÇÃO DO VISUALIZADOR DE SIDECHAIN DUCKING (OSCILOGRAMA) ───────
        // =========================================================================
        void RenderSidechainVisualizer(ImDrawList* dl, ImVec2 pos, ImVec2 size, float time_sec) {
            sidechain.update(time_sec);

            // Moldura Exterior com Gradiente Cyberpunk
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(7, 10, 15, 255), 6.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 0, 128, 180), 6.0f, 0, 1.5f);

            // Cabeçalho & Legenda
            dl->AddText(ImVec2(pos.x + 12.0f, pos.y + 8.0f), IM_COL32(255, 255, 255, 255), "⚡ SIDECHAIN DUCKER");
            float leg_x = pos.x + 165.0f;
            dl->AddCircleFilled(ImVec2(leg_x, pos.y + 14.0f), 3.5f, IM_COL32(0, 229, 255, 255));
            dl->AddText(ImVec2(leg_x + 6.0f, pos.y + 7.0f), IM_COL32(0, 229, 255, 220), "Kick");

            dl->AddCircleFilled(ImVec2(leg_x + 50.0f, pos.y + 14.0f), 3.5f, IM_COL32(255, 0, 128, 255));
            dl->AddText(ImVec2(leg_x + 56.0f, pos.y + 7.0f), IM_COL32(255, 0, 128, 220), "Bass");

            dl->AddCircleFilled(ImVec2(leg_x + 100.0f, pos.y + 14.0f), 3.5f, IM_COL32(255, 195, 30, 255));
            dl->AddText(ImVec2(leg_x + 106.0f, pos.y + 7.0f), IM_COL32(255, 195, 30, 220), "GR");

            // Área do Osciloscópio
            ImVec2 osc_p0(pos.x + 12.0f, pos.y + 30.0f);
            ImVec2 osc_p1(pos.x + size.x - 45.0f, pos.y + size.y - 78.0f);
            float osc_w = osc_p1.x - osc_p0.x;
            float osc_h = osc_p1.y - osc_p0.y;

            dl->AddRectFilled(osc_p0, osc_p1, IM_COL32(4, 6, 10, 255), 4.0f);
            dl->AddRect(osc_p0, osc_p1, IM_COL32(18, 28, 42, 255), 4.0f);

            // Linhas de Grade e Níveis dB
            float mid_y = osc_p0.y + osc_h * 0.5f;
            dl->AddLine(ImVec2(osc_p0.x, mid_y), ImVec2(osc_p1.x, mid_y), IM_COL32(255, 255, 255, 25), 1.0f);
            dl->AddLine(ImVec2(osc_p0.x, osc_p0.y + osc_h * 0.25f), ImVec2(osc_p1.x, osc_p0.y + osc_h * 0.25f), IM_COL32(255, 255, 255, 12), 1.0f);
            dl->AddLine(ImVec2(osc_p0.x, osc_p0.y + osc_h * 0.75f), ImVec2(osc_p1.x, osc_p0.y + osc_h * 0.75f), IM_COL32(255, 255, 255, 12), 1.0f);

            // 1. Curva de Redução de Ganho (Gain Reduction Envelope) - Amber/Gold
            for (int i = 0; i < 127; i++) {
                float x_a = osc_p0.x + (float)i * (osc_w / 127.0f);
                float x_b = osc_p0.x + (float)(i + 1) * (osc_w / 127.0f);
                float gr_a = osc_p0.y + sidechain.gr_history[i] * (osc_h * 0.75f);
                float gr_b = osc_p0.y + sidechain.gr_history[i + 1] * (osc_h * 0.75f);

                // Preenchimento sombreado sob a curva de atenuação
                dl->AddRectFilled(ImVec2(x_a, osc_p0.y), ImVec2(x_b, gr_b), IM_COL32(255, 190, 20, 35));
                // Linha luminosa de atenuação
                dl->AddLine(ImVec2(x_a, gr_a), ImVec2(x_b, gr_b), IM_COL32(255, 200, 30, 240), 2.0f);
            }

            // 2. Onda do Bass (Target) - Magenta
            for (int i = 0; i < 127; i++) {
                float x_a = osc_p0.x + (float)i * (osc_w / 127.0f);
                float x_b = osc_p0.x + (float)(i + 1) * (osc_w / 127.0f);
                float y_a = mid_y - sidechain.target_history[i] * (osc_h * 0.42f);
                float y_b = mid_y - sidechain.target_history[i + 1] * (osc_h * 0.42f);
                dl->AddLine(ImVec2(x_a, y_a), ImVec2(x_b, y_b), IM_COL32(255, 0, 128, 220), 1.5f);
            }

            // 3. Onda do Kick (Trigger) - Ciano Neon
            for (int i = 0; i < 127; i++) {
                float x_a = osc_p0.x + (float)i * (osc_w / 127.0f);
                float x_b = osc_p0.x + (float)(i + 1) * (osc_w / 127.0f);
                float y_a = mid_y - sidechain.trigger_history[i] * (osc_h * 0.42f);
                float y_b = mid_y - sidechain.trigger_history[i + 1] * (osc_h * 0.42f);
                dl->AddLine(ImVec2(x_a, y_a), ImVec2(x_b, y_b), IM_COL32(0, 229, 255, 230), 1.6f);
            }

            // Medidor Vertical de Redução de Ganho (GR Meter) à direita
            ImVec2 m_p0(osc_p1.x + 8.0f, osc_p0.y);
            ImVec2 m_p1(pos.x + size.x - 12.0f, osc_p1.y);
            dl->AddRectFilled(m_p0, m_p1, IM_COL32(10, 14, 20, 255), 2.0f);
            dl->AddRect(m_p0, m_p1, IM_COL32(30, 42, 56, 255), 2.0f);

            float gr_norm = std::clamp(-sidechain.current_gr_db / 24.0f, 0.0f, 1.0f);
            float fill_y = m_p0.y + gr_norm * osc_h;
            dl->AddRectFilled(m_p0, ImVec2(m_p1.x, fill_y), IM_COL32(255, 185, 20, 240), 2.0f);
            dl->AddText(ImVec2(m_p0.x - 2.0f, osc_p1.y + 4.0f), IM_COL32(255, 195, 30, 220), "GR");

            // Painel Inferior de Parâmetros do Compressor Sidechain (2 Linhas Limpas)
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 12.0f, osc_p1.y + 6.0f));
            ImGui::PushID(9881);
            
            // Linha 1
            ImGui::SetNextItemWidth(62.0f);
            ImGui::SliderFloat("Thresh", &sidechain.threshold_db, -40.0f, 0.0f, "%.0fdB");
            ImGui::SameLine(0, 8);
            ImGui::SetNextItemWidth(55.0f);
            ImGui::SliderFloat("Ratio", &sidechain.ratio, 1.0f, 20.0f, "%.1f:1");
            ImGui::SameLine(0, 8);
            ImGui::SetNextItemWidth(55.0f);
            float depth_pct = sidechain.ducking_depth * 100.0f;
            if (ImGui::SliderFloat("Depth", &depth_pct, 0.0f, 100.0f, "%.0f%%")) {
                sidechain.ducking_depth = depth_pct / 100.0f;
            }

            // Linha 2
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 12.0f, osc_p1.y + 36.0f));
            ImGui::SetNextItemWidth(58.0f);
            ImGui::SliderFloat("Attack", &sidechain.attack_ms, 0.5f, 30.0f, "%.1fms");
            ImGui::SameLine(0, 8);
            ImGui::SetNextItemWidth(58.0f);
            ImGui::SliderFloat("Release", &sidechain.release_ms, 10.0f, 300.0f, "%.0fms");
            ImGui::SameLine(0, 10);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "GR: %.1fdB", sidechain.current_gr_db);

            ImGui::Dummy(ImVec2(size.x - 24.0f, 20.0f));
            ImGui::PopID();
        }

        // =========================================================================
        // ── JANELA MODAL: MATRIZ DE ROTEAMENTO GLOBAL (17x17) ─────────────────────
        // =========================================================================
        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(980, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.11f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.50f, 0.85f));

            if (ImGui::Begin("🎛️ FL STUDIO ADVANCED MIXER ROUTING & SIDECHAIN MATRIX###MixerRoutingMatrixWindow", &is_open)) {
                
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "MATRIZ DE ROTEAMENTO DE ÁUDIO, BUSES & SIDECHAIN MULTICANAL");
                ImGui::TextDisabled("Controle avançado de envio de áudio entre todas as faixas do mixer no estilo FL Studio / Ableton Live.");
                ImGui::Separator();
                ImGui::Spacing();

                const char* ch_names[17] = { 
                    "Master", "01. Kick", "02. Bass", "03. Snare", "04. HiHat", 
                    "05. Perc", "06. Lead", "07. Chords", "08. Arp", "09. Acid", 
                    "10. Vocal", "11. Synth2", "12. Sub", "13. Crash",
                    "14. RevBus", "15. DlyBus", "16. SC Bus" 
                };
                
                // Tabela da Matriz de Envio
                if (ImGui::BeginTable("RoutingGrid17", 18, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) {
                    
                    // Cabeçalho das Colunas (Destino)
                    ImGui::TableSetupColumn("De \\ Para", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    for (int dest = 0; dest < 17; ++dest) {
                        ImGui::TableSetupColumn(ch_names[dest], ImGuiTableColumnFlags_WidthFixed, 65.0f);
                    }
                    ImGui::TableHeadersRow();

                    // Linhas (Origem de Envio)
                    for (int src = 0; src < 17; ++src) {
                        ImGui::TableNextRow();
                        
                        // Nome da Faixa de Origem
                        ImGui::TableSetColumnIndex(0);
                        if (src == 0) ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", ch_names[src]);
                        else if (src >= 14) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.8f, 1.0f), "%s", ch_names[src]);
                        else ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "%s", ch_names[src]);

                        // Células de Cruzamento
                        for (int dest = 0; dest < 17; ++dest) {
                            ImGui::TableSetColumnIndex(dest + 1);
                            
                            if (src == dest || (src == 0 && dest > 0)) {
                                ImGui::TextDisabled(" -- ");
                            } else {
                                ImGui::PushID(src * 100 + dest);
                                
                                bool enabled = send_enable[src][dest];
                                bool is_sc = is_sidechain_only[src][dest];

                                if (enabled) {
                                    if (is_sc) {
                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.15f, 0.50f, 0.85f));
                                    } else {
                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.70f, 0.45f, 0.85f));
                                    }
                                } else {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.18f, 0.6f));
                                }

                                char btn_label[32];
                                if (enabled) {
                                    snprintf(btn_label, sizeof(btn_label), "%s %.0f%%", is_sc ? "SC" : "ON", send_matrix[src][dest] * 100.0f);
                                } else {
                                    snprintf(btn_label, sizeof(btn_label), "OFF");
                                }

                                if (ImGui::Button(btn_label, ImVec2(58, 20))) {
                                    toggleRoute(src, dest, dest == 16);
                                }

                                // Menu de contexto de clique direito
                                if (ImGui::BeginPopupContextItem()) {
                                    ImGui::Text("Roteamento: %s -> %s", ch_names[src], ch_names[dest]);
                                    ImGui::Separator();
                                    ImGui::Checkbox("Apenas Sidechain (Envelope Trigger)", &is_sidechain_only[src][dest]);
                                    ImGui::SliderFloat("Ganho de Envio (Send Gain)", &send_matrix[src][dest], 0.0f, 1.25f, "%.2f");
                                    ImGui::EndPopup();
                                }

                                ImGui::PopStyleColor();
                                ImGui::PopID();
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botões de Presets
                ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "⚡ PRESETS DE ROTEAMENTO RÁPIDO DO MIXER:");
                
                if (ImGui::Button("Resetar Roteamento Padrão (Todas as faixas -> Master)", ImVec2(340, 26))) {
                    resetToDefault();
                }
                ImGui::SameLine();
                if (ImGui::Button("Ativar Psytrance Sidechain (Kick -> Bass Ducking 85%)", ImVec2(350, 26))) {
                    send_enable[1][2] = true;
                    send_matrix[1][2] = 0.85f;
                    is_sidechain_only[1][2] = true;
                    sidechain.trigger_track = 1;
                    sidechain.target_track = 2;
                    sidechain.ducking_depth = 0.85f;
                }
                ImGui::SameLine();
                if (ImGui::Button("Roteamento de Sends (Lead & Chords -> Reverb/Delay)", ImVec2(320, 26))) {
                    send_enable[6][14] = true; send_matrix[6][14] = 0.40f;
                    send_enable[6][15] = true; send_matrix[6][15] = 0.35f;
                    send_enable[7][14] = true; send_matrix[7][14] = 0.50f;
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
