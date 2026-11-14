#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct EdisonMarker {
        std::string name;
        float position_sec;
    };

    class KuroEdisonWaveEditorUI {
    private:
        bool is_open = false;
        std::string loaded_sample_name = "Kuro_Sample_Recorded.wav";
        float selection_start_sec = 0.0f;
        float selection_end_sec = 2.5f;
        float zoom_level = 1.0f;
        float playhead_sec = 0.0f;
        bool is_looping = false;
        bool is_recording = false;
        std::vector<EdisonMarker> markers;

    public:
        KuroEdisonWaveEditorUI() {
            markers.push_back({ "Hit 1 (Transient)", 0.00f });
            markers.push_back({ "Loop Start", 0.50f });
            markers.push_back({ "Transient Snare", 1.00f });
            markers.push_back({ "Loop End", 2.00f });
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.09f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.97f, 0.50f, 0.0f, 0.85f)); // Laranja FL

            if (ImGui::Begin("✂️ EDISON AUDIO WAVE EDITOR & RECORDER (FL STUDIO STYLE)###EdisonAudioEditor", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU SUPERIOR DO EDISON ─────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Arquivo")) {
                        if (ImGui::MenuItem("Novo Sample")) {}
                        if (ImGui::MenuItem("Carregar WAV...")) {}
                        if (ImGui::MenuItem("Salvar Sample Como...")) {}
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Ferramentas (Process)")) {
                        if (ImGui::MenuItem("Normalizar (0 dB)")) {}
                        if (ImGui::MenuItem("Remover DC Offset")) {}
                        if (ImGui::MenuItem("Inverter Polaridade (Phase Reverse)")) {}
                        if (ImGui::MenuItem("Reverso (Reverse Audio)")) {}
                        if (ImGui::MenuItem("Fade In")) {}
                        if (ImGui::MenuItem("Fade Out")) {}
                        ImGui::Separator();
                        if (ImGui::MenuItem("Detectar Transientes / Fatiar (Auto-Slice)")) {}
                        if (ImGui::MenuItem("De-Noiser / Redução de Ruído")) {}
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Regiões & Marcadores")) {
                        if (ImGui::MenuItem("Adicionar Marcador (Marker)")) {}
                        if (ImGui::MenuItem("Definir Pontos de Loop (Set Loop)")) {}
                        if (ImGui::MenuItem("Limpar Todos os Marcadores")) {}
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // ── BARRA DE FERRAMENTAS DE TRANSPORTE & EDIÇÃO ──────────────
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.14f, 1.0f));
                ImGui::BeginChild("EdisonToolbar", ImVec2(0, 42), true);
                
                // Botões de Play / Stop / Rec
                if (ImGui::Button("▶ Play", ImVec2(55, 26))) {}
                ImGui::SameLine();
                if (ImGui::Button("■ Stop", ImVec2(55, 26))) {}
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, is_recording ? ImVec4(0.8f, 0.1f, 0.1f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("● Record", ImVec2(70, 26))) { is_recording = !is_recording; }
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Checkbox("Loop", &is_looping);
                ImGui::SameLine(0, 16);

                // Quick Tools
                if (ImGui::Button("⚡ Normalizar")) {}
                ImGui::SameLine();
                if (ImGui::Button("🔄 Reverso")) {}
                ImGui::SameLine();
                if (ImGui::Button("✂️ Cortar Seleção")) {}
                ImGui::SameLine();
                if (ImGui::Button("📌 Auto-Slice (Chop)")) {}
                ImGui::SameLine(ImGui::GetWindowWidth() - 210);
                ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "44.1 kHz | 32-bit Float");

                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ── DISPLAY DA FORMA DE ONDA PCM COM ZOOM & SELEÇÃO ──────────
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 120.0f);
                if (canvas_sz.y < 150.0f) canvas_sz.y = 150.0f;
                ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();

                // Fundo do Visualizador Edison
                dl->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(15, 18, 24, 255), 4.0f);
                dl->AddRect(canvas_p0, canvas_p1, IM_COL32(40, 50, 65, 255), 4.0f);

                // Linha de centro 0 dB
                float center_y = canvas_p0.y + canvas_sz.y * 0.5f;
                dl->AddLine(ImVec2(canvas_p0.x, center_y), ImVec2(canvas_p1.x, center_y), IM_COL32(50, 65, 85, 200), 1.0f);

                // Grid de tempo e amplitude (+1.0, +0.5, 0, -0.5, -1.0)
                dl->AddText(ImVec2(canvas_p0.x + 8, canvas_p0.y + 4), IM_COL32(140, 160, 180, 180), "+1.0");
                dl->AddText(ImVec2(canvas_p0.x + 8, center_y - 8), IM_COL32(140, 160, 180, 180), " 0.0");
                dl->AddText(ImVec2(canvas_p0.x + 8, canvas_p1.y - 18), IM_COL32(140, 160, 180, 180), "-1.0");

                // Desenho da Onda PCM Estilizada do Edison
                int num_points = (int)canvas_sz.x;
                for (int x = 0; x < num_points; x += 2) {
                    float t = (float)x / (float)num_points;
                    float wave_val = std::sin(t * 40.0f) * std::exp(-t * 1.5f) * 0.75f + 0.15f * std::sin(t * 120.0f);
                    float py = center_y - wave_val * (canvas_sz.y * 0.42f);

                    // Gradiente Laranja / Neon FL Studio
                    dl->AddLine(ImVec2(canvas_p0.x + x, center_y), ImVec2(canvas_p0.x + x, py), IM_COL32(247, 140, 30, 220), 1.5f);
                }

                // Destaque de Região Selecionada (Azul Translúcido FL Studio)
                float sel_x0 = canvas_p0.x + canvas_sz.x * 0.20f;
                float sel_x1 = canvas_p0.x + canvas_sz.x * 0.65f;
                dl->AddRectFilled(ImVec2(sel_x0, canvas_p0.y), ImVec2(sel_x1, canvas_p1.y), IM_COL32(0, 150, 255, 60));
                dl->AddLine(ImVec2(sel_x0, canvas_p0.y), ImVec2(sel_x0, canvas_p1.y), IM_COL32(0, 200, 255, 255), 1.5f);
                dl->AddLine(ImVec2(sel_x1, canvas_p0.y), ImVec2(sel_x1, canvas_p1.y), IM_COL32(0, 200, 255, 255), 1.5f);

                // Desenho de Marcadores Verticais (Markers)
                for (const auto& m : markers) {
                    float mx = canvas_p0.x + (m.position_sec / 2.5f) * canvas_sz.x;
                    if (mx >= canvas_p0.x && mx <= canvas_p1.x) {
                        dl->AddLine(ImVec2(mx, canvas_p0.y), ImVec2(mx, canvas_p1.y), IM_COL32(255, 60, 60, 220), 1.5f);
                        dl->AddRectFilled(ImVec2(mx - 2, canvas_p0.y), ImVec2(mx + 80, canvas_p0.y + 16), IM_COL32(255, 60, 60, 200), 2.0f);
                        dl->AddText(ImVec2(mx + 2, canvas_p0.y + 1), IM_COL32(255, 255, 255, 255), m.name.c_str());
                    }
                }

                ImGui::Dummy(canvas_sz);

                // ── PAINEL INFERIOR: ARRASTAR PARA A PLAYLIST OU SAMPLER ─────
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.97f, 0.50f, 0.0f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.65f, 0.2f, 1.0f));
                if (ImGui::Button("🖐️ ARRASTAR AMOSTRA PROCESSADA PARA A PLAYLIST / CHANNEL RACK", ImVec2(-1, 36))) {
                    // Exporta / envia amostra do Edison diretamente para a Playlist
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
