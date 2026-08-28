#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct SlicexRegion {
        int id;
        std::string name;
        int mapped_key; // MIDI Key (ex: C5 = 60, C#5 = 61, etc.)
        float start_sec;
        float end_sec;
        float pitch_offset;
        float volume;
        bool is_reversed;
    };

    class KuroSlicexBeatSlicerUI {
    private:
        bool is_open = false;
        std::string loop_filename = "Amen_Break_174BPM_Loop.wav";
        std::vector<SlicexRegion> slices;
        int selected_slice_idx = 0;
        float sensitivity = 0.65f;
        bool auto_dump_to_piano_roll = true;

    public:
        KuroSlicexBeatSlicerUI() {
            // Inicializar fatias automáticas da clássica quebra de bateria (16 fatias MIDI mapeadas de C5 a D#6)
            float slice_len = 2.0f / 8.0f;
            for (int i = 0; i < 8; ++i) {
                char s_name[32];
                snprintf(s_name, sizeof(s_name), "Slice %02d (%s)", i + 1, (i % 2 == 0) ? "Kick/Hat" : "Snare/Ghost");
                slices.push_back({ i, s_name, 60 + i, i * slice_len, (i + 1) * slice_len, 0.0f, 1.0f, false });
            }
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.95f, 0.20f, 0.40f, 0.85f)); // Rosa / Magenta Slicex

            if (ImGui::Begin("🥁 SLICEX BEAT RE-ARRANGER & LOOP SLICER (FL STUDIO STYLE)###SlicexWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DO SLICEX ──────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Arquivo")) {
                        if (ImGui::MenuItem("Carregar Loop de Bateria (.WAV)...")) {}
                        if (ImGui::MenuItem("Exportar Fatias como Samples Individuais")) {}
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Fatiamento (Slicing)")) {
                        if (ImGui::MenuItem("Detectar Transientes (Grid Médio)")) {}
                        if (ImGui::MenuItem("Fatiar em Semi-Colcheias (1/16 Beat Slices)")) {}
                        if (ImGui::MenuItem("Fatiar em Tempos (1/4 Beat Slices)")) {}
                        if (ImGui::MenuItem("Limpar Todas as Fatias")) {}
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.30f, 0.55f, 1.0f), "SLICEX DRUM LOOP RE-ARRANGER & TRANSIENT SLICE SAMPLER");
                ImGui::TextDisabled("Fatia loops de áudio automaticamente por transientes e mapeia cada fatia para teclas individuais do Piano Roll.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY DE FORMA DE ONDA COM FATIAS COLORIDAS ────────
                ImVec2 wave_p0 = ImGui::GetCursorScreenPos();
                ImVec2 wave_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 wave_p1 = ImVec2(wave_p0.x + wave_sz.x, wave_p0.y + wave_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(wave_p0, wave_p1, IM_COL32(12, 16, 22, 255), 4.0f);
                dl->AddRect(wave_p0, wave_p1, IM_COL32(40, 50, 65, 255), 4.0f);

                // Desenhar Onda PCM do Loop
                int num_points = (int)wave_sz.x;
                float center_y = wave_p0.y + wave_sz.y * 0.5f;
                for (int x = 0; x < num_points; x += 2) {
                    float t = (float)x / (float)num_points;
                    float wave_val = std::sin(t * 30.0f) * std::exp(-std::fmod(t * 8.0f, 1.0f) * 3.0f) * 0.8f;
                    float py = center_y - wave_val * (wave_sz.y * 0.42f);
                    dl->AddLine(ImVec2(wave_p0.x + x, center_y), ImVec2(wave_p0.x + x, py), IM_COL32(255, 60, 120, 200), 1.5f);
                }

                // Desenhar Marcadores e Regiões de Fatias (Slices)
                static const ImU32 slice_colors[] = {
                    IM_COL32(255, 60, 100, 70), IM_COL32(0, 200, 255, 70),
                    IM_COL32(255, 180, 0, 70),  IM_COL32(100, 255, 100, 70)
                };

                for (size_t i = 0; i < slices.size(); ++i) {
                    const auto& s = slices[i];
                    float sx0 = wave_p0.x + (s.start_sec / 2.0f) * wave_sz.x;
                    float sx1 = wave_p0.x + (s.end_sec / 2.0f) * wave_sz.x;

                    ImU32 col = slice_colors[i % 4];
                    if (selected_slice_idx == (int)i) col = IM_COL32(255, 255, 255, 110);

                    dl->AddRectFilled(ImVec2(sx0, wave_p0.y), ImVec2(sx1, wave_p1.y), col);
                    dl->AddLine(ImVec2(sx0, wave_p0.y), ImVec2(sx0, wave_p1.y), IM_COL32(255, 255, 255, 220), 1.5f);

                    // Bandeira de Marcador com a Nota MIDI correspondente
                    char tag[16];
                    snprintf(tag, sizeof(tag), "S%d (C%d)", (int)i + 1, (s.mapped_key / 12) - 1);
                    dl->AddRectFilled(ImVec2(sx0, wave_p0.y), ImVec2(sx0 + 48, wave_p0.y + 16), IM_COL32(20, 25, 35, 220), 2.0f);
                    dl->AddText(ImVec2(sx0 + 4, wave_p0.y + 1), IM_COL32(255, 220, 100, 255), tag);
                }

                ImGui::Dummy(wave_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. TABELA DE FATIAS & CONTROLES INDIVIDUAIS ──────────────
                ImGui::Columns(2, "SlicexCols", true);

                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "📑 LISTA DE FATIAS MAPEADAS (MIDI PAD TRIGGER)");
                ImGui::Spacing();

                if (ImGui::BeginChild("SlicesList", ImVec2(0, 210), true)) {
                    for (size_t i = 0; i < slices.size(); ++i) {
                        auto& s = slices[i];
                        bool is_sel = (selected_slice_idx == (int)i);

                        char row_txt[64];
                        snprintf(row_txt, sizeof(row_txt), "Pad %02d | %s | MIDI: %d", (int)i + 1, s.name.c_str(), s.mapped_key);
                        if (ImGui::Selectable(row_txt, is_sel)) {
                            selected_slice_idx = (int)i;
                            // Toca a fatia instantaneamente ao clicar
                            extern KuroAudio::SynthEngine g_piano_synth;
                            g_piano_synth.triggerNote(s.mapped_key, 0.25f, 0.9f, 0);
                        }
                    }
                    ImGui::EndChild();
                }

                ImGui::NextColumn();

                // Controles da Fatia Selecionada
                if (selected_slice_idx >= 0 && selected_slice_idx < (int)slices.size()) {
                    auto& cur_s = slices[selected_slice_idx];
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "🎛️ CONTROLES DA FATIA: %s", cur_s.name.c_str());
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::SliderFloat("Volume da Fatia", &cur_s.volume, 0.0f, 2.0f, "%.2fx");
                    ImGui::SliderFloat("Pitch Offset (Semear)", &cur_s.pitch_offset, -24.0f, 24.0f, "%.1f st");
                    ImGui::Checkbox("Inverter / Tocar Reverso", &cur_s.is_reversed);

                    ImGui::Spacing();
                    if (ImGui::Button("🎵 TESTAR FATIA (PREVIEW)", ImVec2(-1, 30))) {
                        extern KuroAudio::SynthEngine g_piano_synth;
                        g_piano_synth.triggerNote(cur_s.mapped_key, 0.25f, 0.9f, 0);
                    }
                }

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. BOTÃO DE DUMP PARA O PIANO ROLL (FL STUDIO STYLE) ─────
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.20f, 0.40f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.35f, 0.55f, 1.0f));
                if (ImGui::Button("⚡ DUMP SCORE TO PIANO ROLL (ENVIAR NOTAS MIDI DO LOOP REARRANJADO)", ImVec2(-1, 36))) {
                    extern ClipManager g_clip_manager;
                    auto& notes = g_clip_manager.getCurrentPattern().getChannelNotes(0);
                    notes.clear();
                    float step_time = 2.0f / (float)slices.size();
                    for (size_t i = 0; i < slices.size(); ++i) {
                        KuroDSP::MidiNote n(slices[i].mapped_key, i * step_time, step_time * 0.9f, 0.9f, 1.0f, 0);
                        notes.push_back(n);
                    }
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
