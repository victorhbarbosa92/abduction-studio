#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroLovePhilterUI {
    private:
        bool is_open = false;

        // 8 Bancos de Filtros Paralelos / Sequenciais
        int selected_bank = 0;
        float bank_cutoff[8] = { 2500.0f, 800.0f, 4500.0f, 1200.0f, 3000.0f, 1500.0f, 6000.0f, 1000.0f };
        float bank_resonance[8] = { 0.45f, 0.60f, 0.30f, 0.75f, 0.50f, 0.40f, 0.65f, 0.35f };
        float bank_drive[8] = { 1.2f, 2.0f, 1.0f, 2.5f, 1.0f, 1.5f, 1.0f, 1.0f };
        int bank_filter_type[8] = { 0, 1, 2, 0, 3, 0, 2, 1 }; // 0=LP, 1=BP, 2=HP, 3=Phaser/Notch
        float bank_pan[8] = { 0.0f, -0.6f, 0.6f, -0.3f, 0.3f, 0.0f, -0.8f, 0.8f };
        float bank_vol[8] = { 1.0f, 0.8f, 0.8f, 0.7f, 0.9f, 0.8f, 0.6f, 0.6f };
        bool bank_enable[8] = { true, false, false, false, false, false, false, false };

        // Envelope & LFO de Modulação por Padrão
        float lfo_speed = 1.0f; // Hz ou Sync
        float lfo_depth = 0.65f;
        int lfo_shape = 0; // 0=Sine, 1=Triangle, 2=Saw, 3=Stepped Pattern (Gater)
        float master_dry_wet = 1.0f;

    public:
        KuroLovePhilterUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.20f, 0.60f, 0.85f)); // Rosa / Magenta Love Philter

            if (ImGui::Begin("💖 FRUITY LOVE PHILTER (8-BANK MODULAR RESONANT FILTER)###LovePhilterWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS DO LOVE PHILTER ─────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("👽 Psytrance Sweeping Comb Resonance")) {
                            bank_cutoff[0] = 3200.0f; bank_resonance[0] = 0.85f; lfo_speed = 2.0f; lfo_shape = 2;
                        }
                        if (ImGui::MenuItem("🌊 French House Lowpass Auto-Wah Pump")) {
                            bank_cutoff[0] = 1200.0f; bank_resonance[0] = 0.60f; lfo_speed = 0.5f; lfo_shape = 0;
                        }
                        if (ImGui::MenuItem("🔥 Trance Gate / Stutter Gater (Bank Multi-Step)")) {
                            lfo_shape = 3; lfo_speed = 4.0f; lfo_depth = 1.0f;
                        }
                        if (ImGui::MenuItem("🚀 Cyberpunk Dual Notch Stereophonic Sweep")) {
                            bank_enable[0] = true; bank_enable[1] = true;
                            bank_pan[0] = -0.8f; bank_pan[1] = 0.8f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.65f, 1.0f), "FRUITY LOVE PHILTER (8-BANK MODULATION & RESONANCE FX)");
                ImGui::TextDisabled("8 bancos de filtros com envelopes de curva XY, saturação analógica, LFO de varredura e gerador de Trance Gate.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. BARRA DE SELEÇÃO DOS 8 BANCOS DE FILTRO ──────────────
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.00f, 1.0f), "BANCO DE FILTRO ATIVO:");
                ImGui::SameLine();

                for (int b = 0; b < 8; ++b) {
                    char btn_label[16];
                    snprintf(btn_label, sizeof(btn_label), "Bank %d%s", b + 1, bank_enable[b] ? " ●" : "");
                    
                    bool is_sel = (selected_bank == b);
                    if (is_sel) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.20f, 0.60f, 0.9f));
                    } else if (bank_enable[b]) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.35f, 0.45f, 0.9f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.18f, 0.6f));
                    }

                    if (ImGui::Button(btn_label, ImVec2(85, 26))) {
                        selected_bank = b;
                    }
                    ImGui::PopStyleColor();
                    if (b < 7) ImGui::SameLine();
                }

                ImGui::Spacing();

                // ── 2. DISPLAY GRÁFICO DA CURVA DE RESSONÂNCIA DO FILTRO ────
                ImVec2 crv_p0 = ImGui::GetCursorScreenPos();
                ImVec2 crv_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 crv_p1 = ImVec2(crv_p0.x + crv_sz.x, crv_p0.y + crv_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(crv_p0, crv_p1, IM_COL32(12, 15, 22, 255), 4.0f);
                dl->AddRect(crv_p0, crv_p1, IM_COL32(40, 50, 65, 255), 4.0f);

                // Desenhar Curva de Resposta de Frequência do Banco Ativo
                float cur_cut = bank_cutoff[selected_bank];
                float cur_res = bank_resonance[selected_bank];
                float cutoff_norm = (std::log10(cur_cut / 20.0f) / 3.0f); // 20Hz a 20kHz

                int points = (int)crv_sz.x;
                for (int x = 0; x < points; x += 2) {
                    float fx_norm = (float)x / (float)points;
                    float resp = 0.0f;
                    
                    // Cálculo da resposta de filtro estilizada (LP / BP / HP)
                    int type = bank_filter_type[selected_bank];
                    if (type == 0) { // Lowpass
                        float diff = (fx_norm - cutoff_norm);
                        resp = (diff < 0.0f) ? 1.0f : std::exp(-diff * 8.0f);
                        if (std::abs(diff) < 0.08f) resp += cur_res * 0.8f;
                    } else if (type == 1) { // Bandpass
                        float dist = std::abs(fx_norm - cutoff_norm);
                        resp = std::exp(-dist * 12.0f) * (1.0f + cur_res * 1.2f);
                    } else if (type == 2) { // Highpass
                        float diff = (cutoff_norm - fx_norm);
                        resp = (diff < 0.0f) ? 1.0f : std::exp(-diff * 8.0f);
                        if (std::abs(diff) < 0.08f) resp += cur_res * 0.8f;
                    } else { // Notch / Phaser
                        float dist = std::abs(fx_norm - cutoff_norm);
                        resp = 1.0f - std::exp(-dist * 10.0f);
                    }

                    resp = std::clamp(resp, 0.0f, 1.8f);
                    float py = crv_p1.y - 15.0f - resp * (crv_sz.y * 0.45f);

                    dl->AddLine(ImVec2(crv_p0.x + x, crv_p1.y - 15.0f), ImVec2(crv_p0.x + x, py), IM_COL32(255, 40, 140, 180), 1.5f);
                }

                // Marcador do Ponto de Cutoff
                float marker_x = crv_p0.x + cutoff_norm * crv_sz.x;
                dl->AddLine(ImVec2(marker_x, crv_p0.y), ImVec2(marker_x, crv_p1.y), IM_COL32(0, 230, 255, 220), 1.5f);
                dl->AddCircleFilled(ImVec2(marker_x, crv_p1.y - 15.0f - (1.0f + cur_res * 0.8f) * (crv_sz.y * 0.45f)), 5.0f, IM_COL32(0, 255, 255, 255));

                char graph_txt[64];
                snprintf(graph_txt, sizeof(graph_txt), "BANCO %d: %.0f HZ | RES: %.0f%% | DRIVE: %.1fx", selected_bank + 1, cur_cut, cur_res * 100.0f, bank_drive[selected_bank]);
                dl->AddText(ImVec2(crv_p0.x + 12.0f, crv_p0.y + 6.0f), IM_COL32(255, 120, 200, 240), graph_txt);

                ImGui::Dummy(crv_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. CONTROLES DO BANCO SELECIONADO & LFO ──────────────────
                ImGui::Columns(2, "LovePhilterCols", true);

                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.7f, 1.0f), "🎛️ PARÂMETROS DO BANCO %d", selected_bank + 1);
                ImGui::Separator();
                
                ImGui::Checkbox("Ativar Este Banco", &bank_enable[selected_bank]);
                
                const char* types[] = { "Lowpass 24dB Moog", "Bandpass 12dB", "Highpass 24dB", "Notch / Phaser Comb" };
                ImGui::Combo("Tipo de Filtro##bf", &bank_filter_type[selected_bank], types, IM_ARRAYSIZE(types));

                ImGui::SliderFloat("Cutoff (Frequência)", &bank_cutoff[selected_bank], 20.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Ressonância (Q)", &bank_resonance[selected_bank], 0.0f, 0.98f, "%.2f");
                ImGui::SliderFloat("Drive / Saturação", &bank_drive[selected_bank], 1.0f, 4.0f, "%.2fx");
                ImGui::SliderFloat("Panorâmica Estéreo", &bank_pan[selected_bank], -1.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🌊 LFO DE MODULAÇÃO & TRANCE GATE");
                ImGui::Separator();

                const char* lfo_shapes[] = { "Senoide Suave (Sine)", "Triângulo (Triangle)", "Dente de Serra (Saw)", "Padrão Rítmico (Trance Gate)" };
                ImGui::Combo("Forma de Modulação", &lfo_shape, lfo_shapes, IM_ARRAYSIZE(lfo_shapes));
                ImGui::SliderFloat("Velocidade do LFO", &lfo_speed, 0.1f, 16.0f, "%.2f Hz");
                ImGui::SliderFloat("Profundidade (Depth)", &lfo_depth, 0.0f, 1.0f, "%.2f");

                ImGui::Spacing();
                ImGui::SliderFloat("Master Dry / Wet Mix", &master_dry_wet, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
