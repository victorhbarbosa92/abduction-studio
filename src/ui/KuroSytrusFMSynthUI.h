#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroSytrusFMSynthUI {
    private:
        bool is_open = false;
        
        // 6 Operadores: Ratios (Multiplicadores de frequência)
        float op_ratio[6] = { 1.0f, 2.0f, 0.5f, 3.0f, 1.0f, 4.0f };
        float op_volume[6] = { 0.8f, 0.5f, 0.4f, 0.3f, 0.0f, 0.0f };
        int op_shape[6] = { 0, 0, 0, 0, 0, 0 }; // 0=Sine, 1=Triangle, 2=Saw, 3=Square
        
        // Matriz de Modulação FM 6x6 (op_from -> op_to)
        float fm_matrix[6][6] = { 0 };
        
        // Seção Global & Filtro
        float main_filter_cutoff = 3500.0f;
        float main_filter_res = 0.35f;
        int filter_type = 0; // 0=Lowpass 24dB, 1=Bandpass, 2=Highpass, 3=Formant
        float chorus_amount = 0.45f;
        float unison_pan = 0.70f;
        int active_unison = 5;

    public:
        KuroSytrusFMSynthUI() {
            // Preset Padrão: Pluck / FM Brass Dinâmico (Op 2 modula Op 1, Op 3 modula Op 2)
            fm_matrix[1][0] = 0.65f; // Op2 -> Op1 FM
            fm_matrix[2][1] = 0.40f; // Op3 -> Op2 FM
            fm_matrix[0][0] = 0.15f; // Feedback Op1
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(920, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.09f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.65f, 0.00f, 0.85f)); // Laranja / Ouro Sytrus

            if (ImGui::Begin("⚡ SYTRUS 6-OPERATOR HYBRID FM/RM SYNTH (FL STUDIO STYLE)###SytrusSynthWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("👽 Psytrance FM Metallic Zap")) {
                            op_ratio[0] = 1.0f; op_ratio[1] = 3.5f; op_ratio[2] = 7.0f;
                            fm_matrix[1][0] = 0.85f; fm_matrix[2][0] = 0.60f;
                        }
                        if (ImGui::MenuItem("🎹 Deep FM Electric Piano (DX7 / Sytrus)")) {
                            op_ratio[0] = 1.0f; op_ratio[1] = 1.0f; op_ratio[2] = 14.0f;
                            fm_matrix[1][0] = 0.45f; fm_matrix[2][1] = 0.20f;
                        }
                        if (ImGui::MenuItem("🚀 Cyberpunk Neuro Bass (FM Modulated)")) {
                            op_ratio[0] = 0.5f; op_ratio[1] = 1.0f; op_ratio[2] = 2.0f;
                            fm_matrix[1][0] = 0.90f; fm_matrix[0][0] = 0.40f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.0f, 1.0f), "SYTRUS 6-OPERATOR MATRIX FM / RING MODULATION SYNTHESIZER");
                ImGui::TextDisabled("O sintetizador FM clássico e lendário do FL Studio com matriz livre de 6 operadores e filtros multimodo.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. MATRIZ DE MODULAÇÃO FM 6x6 ───────────────────────────
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.00f, 1.0f), "🎛️ MATRIZ DE MODULAÇÃO FM (6 OPERADORES)");
                ImGui::Spacing();

                if (ImGui::BeginTable("SytrusFMGrid", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("MOD \\ CARRIER", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                    for (int c = 0; c < 6; ++c) {
                        char col_name[16];
                        snprintf(col_name, sizeof(col_name), "Op %d Out", c + 1);
                        ImGui::TableSetupColumn(col_name, ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    }
                    ImGui::TableHeadersRow();

                    for (int from = 0; from < 6; ++from) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Op %d (%.2fx)", from + 1, op_ratio[from]);

                        for (int to = 0; to < 6; ++to) {
                            ImGui::TableSetColumnIndex(to + 1);
                            ImGui::PushID(from * 100 + to);

                            float val = fm_matrix[from][to];
                            ImGui::PushItemWidth(70.0f);
                            if (ImGui::SliderFloat("##fm_knob", &val, 0.0f, 1.0f, "%.2f")) {
                                fm_matrix[from][to] = val;
                            }
                            ImGui::PopItemWidth();
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. CONTROLES DOS OPERADORES (RATIOS & VOLUMES) ──────────
                ImGui::Columns(2, "SytrusCols", true);

                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "🎚️ CONFIGURAÇÃO DOS OPERADORES");
                ImGui::Separator();

                const char* shapes[] = { "Senoide (Sine)", "Triângulo (Triangle)", "Dente de Serra (Saw)", "Quadrada (Square)" };
                for (int op = 0; op < 4; ++op) {
                    ImGui::PushID(op + 500);
                    ImGui::Text("Operador %d:", op + 1);
                    ImGui::SameLine(100);
                    ImGui::PushItemWidth(70.0f);
                    ImGui::SliderFloat("Ratio", &op_ratio[op], 0.25f, 16.0f, "%.2fx");
                    ImGui::SameLine(240);
                    ImGui::SliderFloat("Vol", &op_volume[op], 0.0f, 1.0f, "%.2f");
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                }

                ImGui::NextColumn();

                // ── 3. FILTRO MULTIMODO & UNISON GLOBAL ──────────────────────
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "🌊 FILTRO MULTIMODO & UNISON");
                ImGui::Separator();

                const char* flt_names[] = { "Lowpass 24dB (Moog Style)", "Bandpass 12dB", "Highpass 24dB", "Formant Vowel Morph" };
                ImGui::Combo("Tipo de Filtro", &filter_type, flt_names, IM_ARRAYSIZE(flt_names));
                ImGui::SliderFloat("Cutoff (Frequência)", &main_filter_cutoff, 50.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Ressonância", &main_filter_res, 0.0f, 0.95f, "%.2f");

                ImGui::Spacing();
                ImGui::SliderInt("Vozes de Unison", &active_unison, 1, 9);
                ImGui::SliderFloat("Unison Stereo Spread", &unison_pan, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Warm Chorus FX", &chorus_amount, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão de Disparo / Teste de Som
                if (ImGui::Button("🎵 TOCAR SYTRUS (C4 - LEAD / BASS MOTIF)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(60, 1.0f, 0.9f, 6); // Aciona o motor FM 4-Op / Sytrus
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
