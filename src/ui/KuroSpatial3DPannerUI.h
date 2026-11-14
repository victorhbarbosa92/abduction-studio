#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct SoundSource3D {
        int track_id;
        std::string name;
        float pos_x = 0.0f; // -1.0 (Esquerda) a +1.0 (Direita)
        float pos_y = 0.0f; // -1.0 (Atrás) a +1.0 (Frente)
        float pos_z = 0.0f; // -1.0 (Abaixo) a +1.0 (Acima)
        float distance = 1.0f;
        bool is_orbiting = false;
        float orbit_speed = 1.0f;
    };

    class KuroSpatial3DPannerUI {
    private:
        bool is_open = false;
        std::vector<SoundSource3D> sources;
        int active_source_idx = 0;
        bool enable_binaural_hrtf = true;
        float room_reverb_decay = 0.4f;

    public:
        KuroSpatial3DPannerUI() {
            // Fontes 3D padrão associadas aos canais do mixer
            sources = {
                { 0, "Faixa 1 (Kick)", 0.0f, 0.2f, 0.0f, 1.0f, false, 0.0f },
                { 1, "Faixa 2 (Snare)", 0.0f, 0.1f, 0.0f, 1.0f, false, 0.0f },
                { 2, "Faixa 3 (Percussion)", -0.6f, 0.5f, 0.2f, 1.2f, true, 1.5f },
                { 3, "Faixa 4 (Bassline)", 0.0f, -0.1f, -0.2f, 0.9f, false, 0.0f },
                { 4, "Faixa 5 (Chords/Pads)", 0.0f, 0.8f, 0.5f, 1.5f, false, 0.0f },
                { 5, "Faixa 6 (Lead Synth)", 0.7f, 0.4f, 0.3f, 1.1f, true, 2.0f },
                { 6, "Faixa 7 (Acid Arp)", -0.8f, -0.3f, 0.4f, 1.4f, true, 3.0f },
                { 7, "Faixa 8 (Sub FX)", 0.0f, -0.5f, -0.5f, 1.0f, false, 0.0f }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.45f));

            if (ImGui::Begin(ICON_FA_EAR_LISTEN " KURO SPATIAL 3D AUDIO PANNER & BINAURAL HRTF ENGINE", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE MODOS DE ESPACIALIZAÇÃO
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_HEADPHONES " Processador Binaural HRTF:");
                ImGui::SameLine();
                ImGui::Checkbox("Ativar HRTF 3D", &enable_binaural_hrtf);

                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                ImGui::SliderFloat("Decaimento Sala", &room_reverb_decay, 0.1f, 1.0f, "%.2f s");

                ImGui::SameLine(ImGui::GetWindowWidth() - 200);
                if (ImGui::Button(ICON_FA_GLOBE " Resetar Posições 3D", ImVec2(180, 24))) {
                    // Resetar fontes 3D
                }

                ImGui::Separator();
                ImGui::Spacing();

                // RADAR 3D DE ESPACIALIZAÇÃO (VISUALIZAÇÃO TETO/VISTA SUPERIOR)
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 240.0f);

                // Fundo Escuro do Radar
                draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(10, 14, 22, 255), 4.0f);
                draw_list->AddRect(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(0, 229, 255, 120), 4.0f);

                // Centro do Ouvvinte (Head / Listener position)
                ImVec2 center(canvas_p0.x + canvas_sz.x * 0.5f, canvas_p0.y + canvas_sz.y * 0.5f);
                float radius = std::min(canvas_sz.x, canvas_sz.y) * 0.42f;

                // Círculos de Distância do Radar (25%, 50%, 75%, 100%)
                for (int r = 1; r <= 4; ++r) {
                    draw_list->AddCircle(center, radius * (r / 4.0f), IM_COL32(0, 229, 255, 30 + r * 10), 32, 1.0f);
                }
                draw_list->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(0, 229, 255, 40));
                draw_list->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(0, 229, 255, 40));

                // Ícone da Cabeça do Ouvvinte no Centro
                draw_list->AddCircleFilled(center, 12.0f, IM_COL32(0, 255, 102, 255));
                draw_list->AddCircle(center, 14.0f, IM_COL32(255, 255, 255, 200), 16, 1.5f);
                draw_list->AddText(ImVec2(center.x - 16.0f, center.y - 6.0f), IM_COL32(0, 0, 0, 255), "OUVINTE");

                // Desenhar Posições das Fontes Sonoras no Radar
                for (size_t i = 0; i < sources.size(); ++i) {
                    auto& src = sources[i];

                    // Atualizar órbita 3D se ativada
                    if (src.is_orbiting) {
                        float angle = (float)ImGui::GetTime() * src.orbit_speed + (i * 0.8f);
                        src.pos_x = std::cos(angle) * 0.7f;
                        src.pos_y = std::sin(angle) * 0.7f;
                    }

                    float px = center.x + src.pos_x * radius;
                    float py = center.y - src.pos_y * radius;

                    bool is_sel = (active_source_idx == (int)i);
                    ImU32 src_col = is_sel ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 229, 255, 220);

                    draw_list->AddCircleFilled(ImVec2(px, py), is_sel ? 8.0f : 6.0f, src_col);
                    draw_list->AddCircle(ImVec2(px, py), is_sel ? 10.0f : 7.5f, IM_COL32(255, 255, 255, 200), 12, 1.5f);

                    char label[16];
                    snprintf(label, sizeof(label), "F%d", src.track_id + 1);
                    draw_list->AddText(ImVec2(px + 10.0f, py - 6.0f), src_col, label);
                }

                ImGui::Dummy(canvas_sz);

                ImGui::Spacing();

                // CONTROLES DA FONTE 3D SELECIONADA
                ImGui::BeginChild("##SourceControlPanel", ImVec2(0, 180), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_SLIDERS " CONTROLE DE POSICIONAMENTO DA FONTE SELECIONADA:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(200);
                    
                    std::vector<const char*> src_names;
                    for (const auto& s : sources) src_names.push_back(s.name.c_str());
                    ImGui::Combo("##SelectSrc", &active_source_idx, src_names.data(), (int)src_names.size());

                    ImGui::Separator();
                    ImGui::Spacing();

                    auto& curr_src = sources[active_source_idx];

                    ImGui::Columns(2, "Src3DControls", true);

                    // Coluna 1: Coordenadas X, Y, Z
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Coordenadas Tridimensionais (X, Y, Z)");
                    ImGui::SliderFloat("Posição X (Esquerda/Direita)", &curr_src.pos_x, -1.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Posição Y (Frente/Atrás)", &curr_src.pos_y, -1.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Posição Z (Altura)", &curr_src.pos_z, -1.0f, 1.0f, "%.2f");
                    ImGui::NextColumn();

                    // Coluna 2: Órbita Automática 360°
                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "Órbita Automática 360° (3D LFO Path)");
                    ImGui::Checkbox("Ativar Rotação em Órbita", &curr_src.is_orbiting);
                    ImGui::SliderFloat("Velocidade da Órbita", &curr_src.orbit_speed, 0.1f, 5.0f, "%.1f rad/s");
                    ImGui::NextColumn();
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: Arraste a posição X, Y e Z de qualquer faixa para criar efeitos de imersão 3D e rotação Binaural HRTF no fone de ouvido.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
