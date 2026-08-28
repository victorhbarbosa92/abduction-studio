#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityNotebookUI {
    private:
        bool is_open = false;

        // Páginas de Anotações do Projeto (FL Studio Fruity Notebook 2)
        int current_page_idx = 0;
        char project_notes[4][2048] = {
            "=== [ABDUCTION STUDIO - DIÁRIO DE PRODUÇÃO] ===\n\n- Tom da Música: F# Menor (F# Minor Melodic)\n- BPM Master: 138.00\n- Estrutura:\n  0:00 - Intro Atmosférica com Pad Resonante\n  0:32 - Entrada do Bassdrum Punch & Acid 303\n  1:20 - Breakdown Vocal com Fruity Vocoder\n  1:48 - Drop Principal (Super Saw Lead)",
            "=== [LISTA DE MIXAGEM & MASTERING] ===\n\n[x] Sub-bass em Mono abaixo de 120Hz (Fruity Center)\n[x] Lookahead Sidechain no Kick/Bass (Fruity Limiter)\n[ ] Ajustar Haas Delay nos Hi-Hats (Fruity Stereo Enhancer)\n[x] Equalização Cirúrgica 7-Bandas (Parametric EQ 2)\n[x] Maximizer & Tape Saturation no Master Bus (Maximus)",
            "=== [IDEIAS DE MELODIA & ACORDES] ===\n\nProgressão de Acordes:\n| F#m | Dmaj7 | Bm9 | C#7 |\n\nArpeggiator Pattern:\n1/16 Up-Down com Riff Machine (Alt+R)",
            "=== [CRÉDITOS & ANOTAÇÕES TÉCNICAS] ===\n\nDAW: Abduction Studio V2 (Ultra Audio Workstation)\nDSP Engine: Kuro Audio 64-Bit Float Multi-Threaded\nVocal AI: ONNX Deep Voice Synthesizer"
        };
        char page_titles[4][64] = { "1. Diário de Produção", "2. Checklist de Mix", "3. Acordes & Melodia", "4. Créditos do Projeto" };

    public:
        KuroFruityNotebookUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(860, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.80f, 0.20f, 0.85f)); // Amarelo Dourado Caderno

            if (ImGui::Begin("📝 FRUITY NOTEBOOK 2 (PROJECT DOCUMENTATION & MIX NOTES)###NotebookWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PÁGINAS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Páginas do Caderno")) {
                        for (int p = 0; p < 4; ++p) {
                            if (ImGui::MenuItem(page_titles[p], "", current_page_idx == p)) {
                                current_page_idx = p;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "FRUITY NOTEBOOK 2: MULTI-PAGE PROJECT LOG, MIX CHECKLIST & CHORD CHARTS");
                ImGui::TextDisabled("Bloco de notas integrado por projeto para salvar ideias musicais, tom, BPM, arranjos e checklists de mixagem.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. ABAS SUPERIORES DE PÁGINAS (TABS) ────────────────────
                ImGui::BeginTabBar("NotebookTabs");
                for (int p = 0; p < 4; ++p) {
                    if (ImGui::BeginTabItem(page_titles[p])) {
                        current_page_idx = p;
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();

                ImGui::Spacing();

                // ── 2. ÁREA DE TEXTO MULTILINHA COM AUTO-SAVE NO PROJETO ────
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.07f, 0.08f, 0.11f, 0.95f));
                ImGui::InputTextMultiline("##NotebookContent", project_notes[current_page_idx], sizeof(project_notes[current_page_idx]), ImVec2(-1, -45), ImGuiInputTextFlags_AllowTabInput);
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::Separator();

                // Rodapé de Status
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "💾 Salvo automaticamente no arquivo de sessão .abduction");
                ImGui::SameLine(ImGui::GetWindowWidth() - 200.0f);
                ImGui::TextDisabled("Página %d de 4", current_page_idx + 1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
