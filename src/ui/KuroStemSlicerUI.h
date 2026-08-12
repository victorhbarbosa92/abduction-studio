#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct AudioSliceMarker {
        int id;
        float start_time_sec;
        float end_time_sec;
        int mapped_midi_note;
        std::string label;
    };

    class KuroStemSlicerUI {
    private:
        bool is_open = false;
        int slice_count = 16;
        int selected_slice_mode = 0; // 0: Transientes (Picos), 1: 16º Beats, 2: 8º Beats, 3: Bar Slices
        std::vector<AudioSliceMarker> slices;
        int active_selected_slice = 0;

    public:
        KuroStemSlicerUI() {
            // Slices padrão demonstrativos
            for (int i = 0; i < 16; ++i) {
                AudioSliceMarker slice;
                slice.id = i + 1;
                slice.start_time_sec = i * 0.25f;
                slice.end_time_sec = (i + 1) * 0.25f;
                slice.mapped_midi_note = 48 + i; // Começando na nota C3 (MIDI 48)
                slice.label = "Slice " + std::to_string(i + 1) + " (C" + std::to_string(3 + (i / 12)) + ")";
                slices.push_back(slice);
            }
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.45f));

            if (ImGui::Begin(ICON_FA_SCISSORS " KURO STEM AI SLICER & SAMPLE LOOP ENGINE", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE MODOS DE CORTES (SLICING)
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_SLIDERS " Modo de Fatiamento (Slicing):");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(180);
                const char* slice_modes[] = { "Transientes IA (Picos)", "Grade 1/16 (Semínimas)", "Grade 1/8 (Colcheias)", "Grade 1/4 (Compassos)" };
                ImGui::Combo("##SliceMode", &selected_slice_mode, slice_modes, IM_ARRAYSIZE(slice_modes));

                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                ImGui::SliderInt("Qtde", &slice_count, 4, 32);

                ImGui::SameLine(ImGui::GetWindowWidth() - 230);
                if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Auto-Detectar Slices", ImVec2(210, 26))) {
                    // Recalcular Slices
                }

                ImGui::Separator();
                ImGui::Spacing();

                // VISUALIZADOR DE ONDA COM MARCADORES DE SLICE (CANVAS)
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 180.0f);

                // Background Fundo Escuro
                draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(10, 14, 20, 255), 4.0f);
                draw_list->AddRect(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(0, 229, 255, 100), 4.0f);

                // Desenhar Onda Sintética Cyan
                float total_time = 4.0f; // 4 segundos
                float mid_y = canvas_p0.y + canvas_sz.y * 0.5f;

                for (int x = 0; x < (int)canvas_sz.x; x += 2) {
                    float t = (float)x / canvas_sz.x * total_time;
                    float wave_val = std::sin(t * 20.0f) * std::cos(t * 3.0f) * 0.8f;
                    float py = mid_y - wave_val * (canvas_sz.y * 0.4f);
                    draw_list->AddLine(ImVec2(canvas_p0.x + x, mid_y), ImVec2(canvas_p0.x + x, py), IM_COL32(0, 229, 255, 140));
                }

                // Desenhar Linhas Verticais e Marcas de Slice
                for (size_t i = 0; i < slices.size(); ++i) {
                    float norm_x = slices[i].start_time_sec / total_time;
                    float sx = canvas_p0.x + norm_x * canvas_sz.x;

                    ImU32 slice_col = (active_selected_slice == (int)i) ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 255, 102, 220);
                    draw_list->AddLine(ImVec2(sx, canvas_p0.y), ImVec2(sx, canvas_p0.y + canvas_sz.y), slice_col, (active_selected_slice == (int)i) ? 2.5f : 1.5f);

                    char num_lbl[8];
                    snprintf(num_lbl, sizeof(num_lbl), "%d", (int)i + 1);
                    draw_list->AddText(ImVec2(sx + 3.0f, canvas_p0.y + 4.0f), slice_col, num_lbl);
                }

                ImGui::Dummy(canvas_sz);

                ImGui::Spacing();

                // TABELA DE SLICES MAPEADAS PARA TECLAS MIDI
                ImGui::BeginChild("##SliceTableGrid", ImVec2(0, 200), true);
                {
                    ImGui::Columns(4, "SliceTable", true);
                    ImGui::SetColumnWidth(0, 80);
                    ImGui::SetColumnWidth(1, 140);
                    ImGui::SetColumnWidth(2, 220);

                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "SLICE #"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "INTERVALO (s)"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "NOTA MIDI ATRIBUÍDA"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "AÇÃO"); ImGui::NextColumn();
                    ImGui::Separator();

                    for (size_t i = 0; i < slices.size(); ++i) {
                        ImGui::PushID((int)i);
                        
                        bool is_sel = (active_selected_slice == (int)i);
                        if (ImGui::Selectable(std::to_string(i + 1).c_str(), is_sel, ImGuiSelectableFlags_SpanAllColumns)) {
                            active_selected_slice = (int)i;
                        }
                        ImGui::NextColumn();

                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.2fs - %.2fs", slices[i].start_time_sec, slices[i].end_time_sec); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "%s", slices[i].label.c_str()); ImGui::NextColumn();

                        if (ImGui::Button(ICON_FA_PLAY " Testar Slice", ImVec2(110, 20))) {
                            active_selected_slice = (int)i;
                        }
                        ImGui::NextColumn();

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: As fatias cortadas pela IA são mapeadas automaticamente para as teclas do seu teclado MIDI/Piano Roll, permitindo tocar qualquer loop como instrumento.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
