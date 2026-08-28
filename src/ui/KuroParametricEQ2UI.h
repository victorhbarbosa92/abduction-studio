#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct ParametricBand {
        int id;
        float freq;      // 20Hz a 20000Hz
        float gain_db;   // -18dB a +18dB
        float q_width;   // 0.1 a 10.0
        int type;        // 0=Low Cut, 1=Low Shelf, 2=Peaking/Bell, 3=Notch, 4=High Shelf, 5=High Cut
        bool enabled;
    };

    class KuroParametricEQ2UI {
    private:
        bool is_open = false;

        // 7 Bandas Paramétricas Clássicas do FL Studio Parametric EQ 2
        ParametricBand bands[7] = {
            { 1, 40.0f,    0.0f,  1.4f, 0, true }, // Band 1: Low Cut (Sub cleaner)
            { 2, 100.0f,   2.5f,  1.2f, 1, true }, // Band 2: Low Shelf (Kick/Bass)
            { 3, 350.0f,  -1.5f,  2.0f, 2, true }, // Band 3: Peaking (Mud cut)
            { 4, 1000.0f,  0.0f,  1.5f, 2, true }, // Band 4: Peaking (Mid body)
            { 5, 3200.0f,  1.8f,  1.8f, 2, true }, // Band 5: Peaking (Presence / Snap)
            { 6, 8000.0f,  2.0f,  1.2f, 4, true }, // Band 6: High Shelf (Brightness)
            { 7, 18000.0f, 0.0f,  1.4f, 5, true }  // Band 7: High Cut (Air rolloff)
        };

        int selected_band_idx = 4;
        bool show_spectrogram = true;
        float master_gain_db = 0.0f;

    public:
        KuroParametricEQ2UI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.80f, 0.85f)); // Verde-Água / Ciano EQ 2

            if (ImGui::Begin("🎛️ FRUITY PARAMETRIC EQ 2 (7-BAND PRECISION EQUALIZER)###ParametricEQ2Window", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔥 Master Clarity Clean (Sub Cut + 350Hz Scoop + 10k Air)")) {
                            bands[0].freq = 30.0f; bands[0].type = 0;
                            bands[2].freq = 320.0f; bands[2].gain_db = -2.5f;
                            bands[5].freq = 11000.0f; bands[5].gain_db = 3.0f; bands[5].type = 4;
                        }
                        if (ImGui::MenuItem("👽 Psytrance Punchy Kick & Bass Tightener")) {
                            bands[0].freq = 35.0f; bands[1].freq = 85.0f; bands[1].gain_db = 3.5f;
                            bands[2].freq = 250.0f; bands[2].gain_db = -3.0f;
                        }
                        if (ImGui::MenuItem("🎙️ Vocal Presence & Radio Warmth")) {
                            bands[0].freq = 90.0f; bands[4].freq = 4200.0f; bands[4].gain_db = 3.5f;
                        }
                        if (ImGui::MenuItem("📻 Telephone / Megaphone Bandpass Band")) {
                            bands[0].freq = 400.0f; bands[0].type = 0;
                            bands[6].freq = 3500.0f; bands[6].type = 5;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.80f, 1.0f), "FRUITY PARAMETRIC EQ 2: 7-BAND SURGICAL & MASTERING EQUALIZER");
                ImGui::TextDisabled("Equalizador gráfico paramétrico de alta precisão com espectrograma em tempo real e controle de 7 bandas.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRÁFICO ESPECTRAL DE FREQUÊNCIA (20 HZ - 20 KHZ) ─────
                ImVec2 eq_p0 = ImGui::GetCursorScreenPos();
                ImVec2 eq_sz = ImVec2(ImGui::GetContentRegionAvail().x, 220.0f);
                ImVec2 eq_p1 = ImVec2(eq_p0.x + eq_sz.x, eq_p0.y + eq_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(eq_p0, eq_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                dl->AddRect(eq_p0, eq_p1, IM_COL32(35, 50, 65, 255), 4.0f);

                // Linha de 0 dB Central
                float cy = eq_p0.y + eq_sz.y * 0.5f;
                dl->AddLine(ImVec2(eq_p0.x, cy), ImVec2(eq_p1.x, cy), IM_COL32(60, 75, 90, 180), 1.0f);
                dl->AddText(ImVec2(eq_p0.x + 8, cy - 14), IM_COL32(100, 130, 150, 180), "0 dB");

                // Grade de Frequências (100Hz, 1kHz, 10kHz)
                float f_lines[] = { 100.0f, 1000.0f, 10000.0f };
                const char* f_lbls[] = { "100 Hz", "1 kHz", "10 kHz" };
                for (int i = 0; i < 3; ++i) {
                    float fx = eq_p0.x + (std::log10(f_lines[i] / 20.0f) / 3.0f) * eq_sz.x;
                    dl->AddLine(ImVec2(fx, eq_p0.y), ImVec2(fx, eq_p1.y), IM_COL32(25, 35, 48, 180), 1.0f);
                    dl->AddText(ImVec2(fx + 4, eq_p1.y - 18), IM_COL32(80, 110, 130, 180), f_lbls[i]);
                }

                // Desenhar Curva Combinada das 7 Bandas
                int steps = (int)eq_sz.x;
                for (int x = 0; x < steps - 1; x += 2) {
                    float f_norm = (float)x / (float)steps;
                    float freq_at_x = 20.0f * std::pow(1000.0f, f_norm); // 20 a 20000

                    float total_gain = master_gain_db;
                    for (int b = 0; b < 7; ++b) {
                        if (!bands[b].enabled) continue;
                        float diff = std::log10(freq_at_x / bands[b].freq);
                        float bw = 0.35f / (bands[b].q_width > 0.1f ? bands[b].q_width : 0.1f);
                        total_gain += bands[b].gain_db * std::exp(-std::pow(diff / bw, 2.0f));
                    }

                    total_gain = std::clamp(total_gain, -18.0f, 18.0f);
                    float py = cy - (total_gain / 18.0f) * (eq_sz.y * 0.44f);

                    dl->AddLine(ImVec2(eq_p0.x + x, cy), ImVec2(eq_p0.x + x, py), IM_COL32(0, 220, 180, 110), 1.5f);
                    dl->AddLine(ImVec2(eq_p0.x + x, py), ImVec2(eq_p0.x + x + 2, py), IM_COL32(0, 255, 200, 255), 2.0f);
                }

                // Cores dos nós das 7 bandas do FL Studio
                static const ImU32 band_node_colors[7] = {
                    IM_COL32(255, 60, 60, 255),   // 1: Vermelho
                    IM_COL32(255, 140, 0, 255),  // 2: Laranja
                    IM_COL32(255, 220, 0, 255),  // 3: Amarelo
                    IM_COL32(60, 220, 60, 255),   // 4: Verde
                    IM_COL32(0, 200, 255, 255),  // 5: Ciano
                    IM_COL32(100, 120, 255, 255),// 6: Azul
                    IM_COL32(200, 80, 255, 255)  // 7: Roxo
                };

                // Desenhar os Nós das 7 Bandas
                for (int b = 0; b < 7; ++b) {
                    float bx = eq_p0.x + (std::log10(bands[b].freq / 20.0f) / 3.0f) * eq_sz.x;
                    float by = cy - (bands[b].gain_db / 18.0f) * (eq_sz.y * 0.44f);

                    bool is_sel = (selected_band_idx == b);
                    dl->AddCircleFilled(ImVec2(bx, by), is_sel ? 8.0f : 6.0f, band_node_colors[b]);
                    dl->AddCircle(ImVec2(bx, by), is_sel ? 11.0f : 8.0f, IM_COL32(255, 255, 255, is_sel ? 255 : 160), 1.5f);

                    char b_num[4];
                    snprintf(b_num, sizeof(b_num), "%d", b + 1);
                    dl->AddText(ImVec2(bx - 3, by - 6), IM_COL32(0, 0, 0, 255), b_num);
                }

                ImGui::Dummy(eq_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SELETOR E CONTROLES DA BANDA ATIVA ───────────────────
                ImGui::Columns(2, "ParametricCols", true);

                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "🎛️ SELEÇÃO DE BANDA:");
                for (int b = 0; b < 7; ++b) {
                    char btn_tag[16];
                    snprintf(btn_tag, sizeof(btn_tag), "Banda %d", b + 1);
                    if (selected_band_idx == b) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.75f, 0.65f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.18f, 0.22f, 0.8f));
                    }

                    if (ImGui::Button(btn_tag, ImVec2(55, 26))) {
                        selected_band_idx = b;
                    }
                    ImGui::PopStyleColor();
                    if (b < 6) ImGui::SameLine();
                }

                ImGui::Spacing();
                auto& cur_b = bands[selected_band_idx];
                ImGui::Checkbox("Ativar Banda", &cur_b.enabled);
                
                const char* filter_types[] = { "Low Cut (Highpass)", "Low Shelf", "Peaking (Bell)", "Notch", "High Shelf", "High Cut (Lowpass)" };
                ImGui::Combo("Tipo de Filtro##eqtype", &cur_b.type, filter_types, IM_ARRAYSIZE(filter_types));

                ImGui::NextColumn();

                // Controles de Frequência, Ganho e Largura Q
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "🔬 PARÂMETROS DA BANDA %d", selected_band_idx + 1);
                ImGui::Separator();

                ImGui::SliderFloat("Frequência", &cur_b.freq, 20.0f, 20000.0f, "%.1f Hz");
                ImGui::SliderFloat("Ganho (Gain)", &cur_b.gain_db, -18.0f, 18.0f, "%+.1f dB");
                ImGui::SliderFloat("Largura Q (Bandwidth)", &cur_b.q_width, 0.1f, 10.0f, "%.2f");

                ImGui::Spacing();
                ImGui::SliderFloat("Master Output Gain", &master_gain_db, -12.0f, 12.0f, "%+.1f dB");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
