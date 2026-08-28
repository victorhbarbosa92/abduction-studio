#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroGranulizerUI {
    private:
        bool is_open = false;

        // Parâmetros do Fruity Granulizer (Síntese Granular em Tempo Real)
        float grain_size_ms = 60.0f;     // Tamanho do grão (10ms a 500ms)
        float grain_spacing_ms = 45.0f;  // Intervalo / Densidade entre grãos
        float wave_position_norm = 0.35f;// Posição da agulha de varredura no sample
        float pitch_jitter = 0.15f;      // Variação estocástica de afinação dos grãos
        float pan_jitter = 0.50f;        // Espalhamento aleatório estéreo por grão
        float attack_shape = 0.30f;      // Envelope de cada grão (Suavidade Hanning / Tukey)
        float loop_speed = 1.0f;         // Velocidade de reprodução do loop
        bool loop_hold = false;          // Congelamento de tempo infinito (Freeze Pad)

    public:
        KuroGranulizerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void RenderEmbedded() {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.80f, 1.0f), "FRUITY GRANULIZER REALTIME CLOUD SYNTH");
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 210);
            if (ImGui::Button("↗ Abrir em Janela Flutuante", ImVec2(200, 20))) {
                is_open = true;
            }
            ImGui::Separator();
            ImGui::Columns(3, "GranulizerColsEmbedded", true);
            ImGui::TextColored(ImVec4(0.2f, 0.90f, 1.0f, 1.0f), "🔬 PARÂMETROS DO GRÃO");
            ImGui::SliderFloat("Tamanho##emb", &grain_size_ms, 10.0f, 500.0f, "%.0f ms");
            ImGui::SliderFloat("Espaçamento##emb", &grain_spacing_ms, 5.0f, 300.0f, "%.0f ms");
            ImGui::SliderFloat("Posição##emb", &wave_position_norm, 0.0f, 1.0f, "%.2f");

            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🎲 CAOS & JITTER");
            ImGui::SliderFloat("Pitch Jitter##emb", &pitch_jitter, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Pan Jitter##emb", &pan_jitter, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Suavidade##emb", &attack_shape, 0.05f, 0.5f, "%.2f");

            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "⏱️ VELOCIDADE & FREEZE");
            ImGui::SliderFloat("Velocidade##emb", &loop_speed, -2.0f, 2.0f, "%.2fx");
            ImGui::Checkbox("❄️ Congelar Posição##emb", &loop_hold);
            ImGui::Columns(1);
        }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.80f, 0.85f)); // Ciano / Turquesa Granulizer

            if (ImGui::Begin("🌀 FRUITY GRANULIZER (REALTIME CLOUD & TEXTURE SYNTH)###GranulizerWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS DO GRANULIZER ───────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("👽 Alien Ambient Cloud Texture (Freeze)")) {
                            grain_size_ms = 180.0f; grain_spacing_ms = 90.0f; pan_jitter = 0.9f; loop_hold = true;
                        }
                        if (ImGui::MenuItem("🚀 Cyberpunk Glitch Stutter (Fast Chop)")) {
                            grain_size_ms = 25.0f; grain_spacing_ms = 15.0f; pitch_jitter = 0.4f; loop_hold = false;
                        }
                        if (ImGui::MenuItem("🌌 Ethereal Shimmering Vocal Pad")) {
                            grain_size_ms = 120.0f; grain_spacing_ms = 40.0f; attack_shape = 0.5f; pan_jitter = 0.7f;
                        }
                        if (ImGui::MenuItem("⚡ Psytrance Granular Zap Sweep")) {
                            grain_size_ms = 45.0f; grain_spacing_ms = 20.0f; pitch_jitter = 0.8f; loop_speed = 2.0f;
                        }
                        ImGui::EndMenu();
                    }
                    
                    float menu_w = ImGui::GetWindowWidth();
                    ImGui::SameLine(menu_w - 90);
                    if (ImGui::Button("✖ Fechar", ImVec2(80, 20))) {
                        is_open = false;
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.80f, 1.0f), "FRUITY GRANULIZER REALTIME CLOUD & TIME-FREEZE TEXTURE SYNTHESIZER");
                ImGui::TextDisabled("Desconstrói qualquer áudio em micro-grãos sonoros dispersos no espaço para criar texturas infinitas e pads espaciais.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA NUVEM DE GRÃOS (GRAIN CLOUD) ───────
                ImVec2 cl_p0 = ImGui::GetCursorScreenPos();
                ImVec2 cl_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 cl_p1 = ImVec2(cl_p0.x + cl_sz.x, cl_p0.y + cl_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(cl_p0, cl_p1, IM_COL32(10, 16, 24, 255), 4.0f);
                dl->AddRect(cl_p0, cl_p1, IM_COL32(30, 50, 70, 255), 4.0f);

                // Desenhar Fundo de Forma de Onda Base do Sample
                int points = (int)cl_sz.x;
                float cy = cl_p0.y + cl_sz.y * 0.5f;
                for (int x = 0; x < points; x += 3) {
                    float t = (float)x / (float)points;
                    float wave = std::sin(t * 40.0f) * std::exp(-std::fmod(t * 4.0f, 1.0f) * 2.0f) * 0.6f;
                    float py = cy - wave * (cl_sz.y * 0.38f);
                    dl->AddLine(ImVec2(cl_p0.x + x, cy), ImVec2(cl_p0.x + x, py), IM_COL32(35, 65, 90, 140), 1.0f);
                }

                // Agulha Central de Leitura
                float needle_x = cl_p0.x + wave_position_norm * cl_sz.x;
                dl->AddLine(ImVec2(needle_x, cl_p0.y), ImVec2(needle_x, cl_p1.y), IM_COL32(255, 200, 0, 240), 2.0f);
                dl->AddText(ImVec2(needle_x + 4, cl_p0.y + 4), IM_COL32(255, 200, 0, 240), "Playhead");

                // Desenhar Nuvem de Grãos Estocásticos ao Redor da Agulha
                int active_grains = 24;
                for (int g = 0; g < active_grains; ++g) {
                    float gx_offset = ((std::rand() % 100) / 100.0f - 0.5f) * (grain_size_ms * 1.8f);
                    float gy_offset = ((std::rand() % 100) / 100.0f - 0.5f) * (cl_sz.y * 0.7f * pan_jitter);

                    float gx = std::clamp(needle_x + gx_offset, cl_p0.x + 5.0f, cl_p1.x - 5.0f);
                    float gy = std::clamp(cy + gy_offset, cl_p0.y + 10.0f, cl_p1.y - 10.0f);
                    float g_rad = 3.0f + ((grain_size_ms / 500.0f) * 6.0f);

                    // Ponto de Grão Brilhante
                    dl->AddCircleFilled(ImVec2(gx, gy), g_rad, IM_COL32(0, 240, 210, 190));
                    dl->AddCircle(ImVec2(gx, gy), g_rad + 2.0f, IM_COL32(0, 255, 255, 80));
                }

                char cl_info[128];
                snprintf(cl_info, sizeof(cl_info), "TAMANHO DO GRÃO: %.0f ms | INTERVALO: %.0f ms | ESTADO: %s", grain_size_ms, grain_spacing_ms, loop_hold ? "CONGELADO (FREEZE HOLD)" : "REPRODUÇÃO CONTÍNUA");
                dl->AddText(ImVec2(cl_p0.x + 12.0f, cl_p1.y - 20.0f), IM_COL32(0, 230, 255, 240), cl_info);

                ImGui::Dummy(cl_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES GRANULARES ────────────────────────
                ImGui::Columns(3, "GranulizerCols", true);

                // Coluna 1: Grão & Posição
                ImGui::TextColored(ImVec4(0.2f, 0.90f, 1.0f, 1.0f), "🔬 PARÂMETROS DO GRÃO");
                ImGui::Separator();

                ImGui::SliderFloat("Tamanho do Grão (Size)", &grain_size_ms, 10.0f, 500.0f, "%.0f ms");
                ImGui::SliderFloat("Espaçamento (Spacing)", &grain_spacing_ms, 5.0f, 300.0f, "%.0f ms");
                ImGui::SliderFloat("Posição no Sample", &wave_position_norm, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Caos & Espalhamento
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🎲 CAOS & ESTÉREO JITTER");
                ImGui::Separator();

                ImGui::SliderFloat("Pitch Jitter (Afinação)", &pitch_jitter, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Pan Jitter (3D Pan)", &pan_jitter, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Suavidade de Envelope", &attack_shape, 0.05f, 0.5f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Tempo & Congelamento
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "⏱️ VELOCIDADE & FREEZE");
                ImGui::Separator();

                ImGui::SliderFloat("Velocidade do Loop", &loop_speed, -2.0f, 2.0f, "%.2fx");
                ImGui::Checkbox("❄️ Congelar Posição (Hold Freeze)", &loop_hold);

                ImGui::Spacing();
                if (ImGui::Button("🎵 TESTAR NUVEM GRANULAR", ImVec2(-1, 30))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(60, 2.5f, 0.9f, 4);
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
