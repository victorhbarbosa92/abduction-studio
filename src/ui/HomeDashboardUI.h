#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <algorithm>

namespace KuroUI {

    struct RecentProjectItem {
        std::string name;
        std::string path;
        int bpm;
        std::string date_modified;
        unsigned int color_badge;
    };

    class HomeDashboardUI {
    private:
        bool is_open = true; // Aberto por padrão no início
        std::vector<RecentProjectItem> recent_projects;
        int selected_mode = 0; // 0: Studio, 1: DJ, 2: Stem Extractor

    public:
        HomeDashboardUI() {
            // Inicializa com projetos de demonstração/recentes
            recent_projects = {
                { "Psytrance_FullOn_Antigravity.kuro", "C:/Users/USUÁRIO/Documents/Psytrance_FullOn_Antigravity.kuro", 148, "08/08/2026", 0xFF00FF66 },
                { "Alien_Cyber_Acid_Groove.kuro", "C:/Users/USUÁRIO/Documents/Alien_Cyber_Acid_Groove.kuro", 142, "07/08/2026", 0xFF00E5FF },
                { "Darkpsy_HiTech_Experiment.kuro", "C:/Users/USUÁRIO/Documents/Darkpsy_HiTech_Experiment.kuro", 160, "05/08/2026", 0xFFAA50FF },
                { "Progressive_Deep_Journey.kuro", "C:/Users/USUÁRIO/Documents/Progressive_Deep_Journey.kuro", 138, "02/08/2026", 0xFFFF9900 }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(1060, 680), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.40f, 0.35f));

            if (ImGui::Begin(ICON_FA_HOUSE " ABDUCTION STUDIO V2 — PÁGINA INICIAL (HOME DASHBOARD)", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // 1. CABEÇALHO SUPERIOR (STATUS & BOAS-VINDAS)
                float window_width = ImGui::GetContentRegionAvail().x;

                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_FLOPPY_DISK " ABDUCTION STUDIO V2");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "|  Central de Produção e Performance Musical");
                
                ImGui::SameLine(window_width - 320);
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_MICROCHIP " CPU: 3%%  |  RAM: 1.4 GB  |  44.1 kHz");
                ImGui::EndGroup();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 2. HERO CARDS — SELEÇÃO DE MODOS DE TRABALHO (3 COLUNAS)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA_LAYER_GROUP " SELECIONE O MODO DE OPERAÇÃO:");
                ImGui::Spacing();

                float card_width = (window_width - 32.0f) / 3.0f;
                float card_height = 110.0f;

                // CARD 1: STUDIO MODE
                ImGui::PushStyleColor(ImGuiCol_ChildBg, (selected_mode == 0) ? ImVec4(0.08f, 0.18f, 0.14f, 0.9f) : ImVec4(0.07f, 0.09f, 0.12f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_Border, (selected_mode == 0) ? ImVec4(0.0f, 1.0f, 0.5f, 0.8f) : ImVec4(0.2f, 0.25f, 0.3f, 0.4f));
                ImGui::BeginChild("##ModeStudio", ImVec2(card_width, card_height), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_SLIDERS " STUDIO MODE (DAW)");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Arranjador completo, Mixer 20ch,\nPiano Roll e Sequenciador MIDI.");
                    ImGui::Spacing();
                    if (ImGui::Button(ICON_FA_PLAY " Iniciar Studio", ImVec2(-1, 24))) {
                        selected_mode = 0;
                        is_open = false; // Fecha dashboard e entra na DAW
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);

                ImGui::SameLine();

                // CARD 2: DJ PERFORMANCE
                ImGui::PushStyleColor(ImGuiCol_ChildBg, (selected_mode == 1) ? ImVec4(0.08f, 0.14f, 0.20f, 0.9f) : ImVec4(0.07f, 0.09f, 0.12f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_Border, (selected_mode == 1) ? ImVec4(0.0f, 0.9f, 1.0f, 0.8f) : ImVec4(0.2f, 0.25f, 0.3f, 0.4f));
                ImGui::BeginChild("##ModeDJ", ImVec2(card_width, card_height), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " DJ PERFORMANCE");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Interface de 2 Decks, Jog Wheels,\nBPM Sync e Crossfader rápido.");
                    ImGui::Spacing();
                    if (ImGui::Button(ICON_FA_HEADPHONES " Iniciar DJ Deck", ImVec2(-1, 24))) {
                        selected_mode = 1;
                        is_open = false;
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);

                ImGui::SameLine();

                // CARD 3: STEM EXTRACTOR
                ImGui::PushStyleColor(ImGuiCol_ChildBg, (selected_mode == 2) ? ImVec4(0.16f, 0.08f, 0.22f, 0.9f) : ImVec4(0.07f, 0.09f, 0.12f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_Border, (selected_mode == 2) ? ImVec4(0.7f, 0.3f, 1.0f, 0.8f) : ImVec4(0.2f, 0.25f, 0.3f, 0.4f));
                ImGui::BeginChild("##ModeStem", ImVec2(card_width, card_height), true);
                {
                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), ICON_FA_WAND_MAGIC_SPARKLES " AI STEM EXTRACTOR");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Separação inteligente de faixas\n(Vocais, Bateria, Baixo, Synths).");
                    ImGui::Spacing();
                    if (ImGui::Button(ICON_FA_GEARS " Iniciar Extrator IA", ImVec2(-1, 24))) {
                        selected_mode = 2;
                        is_open = false;
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 3. PAINEL DUPLO (PROJETOS RECENTES & TEMPLATES POR GÊNERO)
                float left_col_width = window_width * 0.45f;
                float right_col_width = window_width * 0.52f;
                float section_height = 240.0f;

                // COLUNA ESQUERDA: PROJETOS RECENTES
                ImGui::BeginChild("##RecentProjectsPanel", ImVec2(left_col_width, section_height), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_FOLDER_OPEN " PROJETOS RECENTES");
                    ImGui::SameLine(left_col_width - 120);
                    if (ImGui::Button(ICON_FA_PLUS " Novo", ImVec2(100, 22))) {
                        // Novo Projeto
                    }
                    ImGui::Separator();
                    ImGui::Spacing();

                    for (size_t i = 0; i < recent_projects.size(); ++i) {
                        const auto& proj = recent_projects[i];
                        ImGui::PushID((int)i);
                        
                        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", proj.name.c_str());
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "BPM: %d  |  Modificado: %s", proj.bpm, proj.date_modified.c_str());
                        
                        ImGui::SameLine(left_col_width - 80);
                        if (ImGui::Button(ICON_FA_FOLDER_OPEN " Abrir", ImVec2(70, 24))) {
                            is_open = false;
                        }
                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // COLUNA DIREITA: TEMPLATES DE GÊNEROS MUSICAIS
                ImGui::BeginChild("##GenreTemplatesPanel", ImVec2(right_col_width, section_height), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_RECORD_VINYL " TEMPLATES DE VERTENTES MUSICAIS");
                    ImGui::Separator();
                    ImGui::Spacing();

                    struct TemplateCard {
                        const char* name;
                        const char* desc;
                        int bpm;
                        ImVec4 color;
                    };

                    TemplateCard templates[] = {
                        { "Full-On Psytrance", "Bassline 1/16 KBB, Kick punchy, Arps de ácido", 148, ImVec4(0.0f, 1.0f, 0.5f, 1.0f) },
                        { "Darkpsy / Hi-Tech", "BPM elevado, FM zaps, atmosfera cibernética", 160, ImVec4(0.7f, 0.3f, 1.0f, 1.0f) },
                        { "Progressive Psy", "Groove profundo, atmosferas imersivas", 138, ImVec4(0.0f, 0.8f, 1.0f, 1.0f) },
                        { "Peak-Time Techno", "Kick gordo 909, synth stabs industriais", 135, ImVec4(1.0f, 0.6f, 0.0f, 1.0f) }
                    };

                    for (int t = 0; t < 4; ++t) {
                        ImGui::PushID(100 + t);
                        ImGui::TextColored(templates[t].color, ICON_FA_MUSIC " %s (%d BPM)", templates[t].name, templates[t].bpm);
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", templates[t].desc);
                        
                        ImGui::SameLine(right_col_width - 110);
                        if (ImGui::Button(ICON_FA_BOLT " Carregar", ImVec2(100, 24))) {
                            is_open = false;
                        }
                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 4. RODAPÉ DE LANÇAMENTO RÁPIDO (SINTETIZADORES & IA)
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), ICON_FA_ROCKET " LANÇADOR RÁPIDO DE SINTETIZADORES & FERRAMENTAS:");
                ImGui::Spacing();

                float btn_w = (window_width - 40.0f) / 5.0f;

                if (ImGui::Button(ICON_FA_WAVE_SQUARE " KuroWave Synth", ImVec2(btn_w, 32))) { is_open = false; }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_OM " Monk Synth", ImVec2(btn_w, 32))) { is_open = false; }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Alien Voice", ImVec2(btn_w, 32))) { is_open = false; }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CLOCK " GrossBeat FX", ImVec2(btn_w, 32))) { is_open = false; }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROBOT " Kuro AI Assist", ImVec2(btn_w, 32))) { is_open = false; }

            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
