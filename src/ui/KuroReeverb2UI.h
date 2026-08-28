#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroReeverb2UI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Reeverb 2
        float decay_time_s = 2.8f;       // Tempo de decaimento (0.2s a 20.0s)
        float room_size_norm = 0.65f;    // Tamanho da sala 3D (Small Room a Huge Cathedral)
        float diffusion_percent = 0.80f; // Densidade de reflexões difusas
        float damping_hicut_hz = 6000.0f;// Amortecimento de frequências agudas
        float lowcut_locut_hz = 120.0f;  // Corte de frequências graves
        float pre_delay_ms = 18.0f;      // Atraso de reflexão inicial
        float stereo_separation = 0.35f; // -1.0 (Mono) a +1.0 (Extra Wide 3D)
        float dry_level = 1.0f;
        float wet_level = 0.45f;
        float early_reflections = 0.50f; // Nível das primeiras reflexões (Early Refl)

    public:
        KuroReeverb2UI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.60f, 1.00f, 0.85f)); // Azul Espacial Reeverb 2

            if (ImGui::Begin("🌌 FRUITY REEVERB 2 (ALGORITHMIC 3D ROOM & DIFFUSION REVERB)###Reeverb2Window", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🛸 Alien Cathedral Space (6.5s Decay + Wide)")) {
                            decay_time_s = 6.5f; room_size_norm = 0.90f; stereo_separation = 0.80f;
                            damping_hicut_hz = 8500.0f; lowcut_locut_hz = 150.0f; wet_level = 0.55f;
                        }
                        if (ImGui::MenuItem("🎙️ Vocal Plate Reverb (Smooth Diffusion)")) {
                            decay_time_s = 2.2f; room_size_norm = 0.55f; diffusion_percent = 0.95f;
                            damping_hicut_hz = 5500.0f; lowcut_locut_hz = 200.0f; wet_level = 0.35f;
                        }
                        if (ImGui::MenuItem("🥁 Tight Drum Studio Room (0.8s)")) {
                            decay_time_s = 0.8f; room_size_norm = 0.30f; early_reflections = 0.80f;
                            lowcut_locut_hz = 100.0f; wet_level = 0.25f;
                        }
                        if (ImGui::MenuItem("✨ Cosmic Shimmer Tail (12.0s Infinite Pad)")) {
                            decay_time_s = 12.0f; room_size_norm = 1.0f; stereo_separation = 1.0f;
                            damping_hicut_hz = 12000.0f; wet_level = 0.70f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.2f, 0.75f, 1.0f, 1.0f), "FRUITY REEVERB 2: ALGORITHMIC STEREO SPACE & DIFFUSION PROCESSOR");
                ImGui::TextDisabled("O reverbero algorítmico mais famoso do mundo com controle de tamanho de sala 3D, difusão e amortecimento.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA SALA 3D E REFLEXÕES ESTOCÁSTICAS ────
                ImVec2 rv_p0 = ImGui::GetCursorScreenPos();
                ImVec2 rv_sz = ImVec2(ImGui::GetContentRegionAvail().x, 180.0f);
                ImVec2 rv_p1 = ImVec2(rv_p0.x + rv_sz.x, rv_p0.y + rv_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(rv_p0, rv_p1, IM_COL32(10, 14, 22, 255), 4.0f);
                dl->AddRect(rv_p0, rv_p1, IM_COL32(35, 50, 70, 255), 4.0f);

                // Desenhar Cubo de Sala 3D Isométrica
                float cx = rv_p0.x + 120.0f;
                float cy = rv_p0.y + rv_sz.y * 0.5f;
                float box_w = 40.0f + room_size_norm * 60.0f;
                float box_h = 30.0f + room_size_norm * 45.0f;

                // Wireframe da Sala 3D
                dl->AddRect(ImVec2(cx - box_w, cy - box_h), ImVec2(cx + box_w, cy + box_h), IM_COL32(0, 180, 255, 180), 2.0f);
                dl->AddRect(ImVec2(cx - box_w * 0.6f, cy - box_h * 0.6f), ImVec2(cx + box_w * 0.6f, cy + box_h * 0.6f), IM_COL32(0, 140, 200, 120), 1.0f);
                dl->AddLine(ImVec2(cx - box_w, cy - box_h), ImVec2(cx - box_w * 0.6f, cy - box_h * 0.6f), IM_COL32(0, 140, 200, 120));
                dl->AddLine(ImVec2(cx + box_w, cy - box_h), ImVec2(cx + box_w * 0.6f, cy - box_h * 0.6f), IM_COL32(0, 140, 200, 120));
                dl->AddLine(ImVec2(cx - box_w, cy + box_h), ImVec2(cx - box_w * 0.6f, cy + box_h * 0.6f), IM_COL32(0, 140, 200, 120));
                dl->AddLine(ImVec2(cx + box_w, cy + box_h), ImVec2(cx + box_w * 0.6f, cy + box_h * 0.6f), IM_COL32(0, 140, 200, 120));

                // Desenhar Gráfico de Decaimento Exponencial do Reverb ao Lado da Sala
                float graph_x0 = cx + box_w + 30.0f;
                float graph_w = rv_p1.x - graph_x0 - 20.0f;

                int steps = (int)graph_w;
                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps;
                    float decay_exp = std::exp(-t * (5.0f / (decay_time_s > 0.2f ? decay_time_s : 0.2f)));
                    float noise_refl = ((std::rand() % 100) / 100.0f - 0.5f) * (diffusion_percent * 0.6f);
                    float amp = decay_exp * (0.5f + noise_refl);

                    float py = cy - amp * (rv_sz.y * 0.40f);
                    dl->AddLine(ImVec2(graph_x0 + x, cy), ImVec2(graph_x0 + x, py), IM_COL32(0, (int)(160 + decay_exp * 95), 255, 200), 1.5f);
                }

                char rv_info[128];
                snprintf(rv_info, sizeof(rv_info), "SALA: %.0f%% | DECAY: %.1fs | DIFUSÃO: %.0f%% | CORTE: %.0f Hz - %.0f Hz", room_size_norm * 100.0f, decay_time_s, diffusion_percent * 100.0f, lowcut_locut_hz, damping_hicut_hz);
                dl->AddText(ImVec2(rv_p0.x + 12.0f, rv_p1.y - 20.0f), IM_COL32(0, 230, 255, 240), rv_info);

                ImGui::Dummy(rv_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO REEVERB 2 ───────────────────────
                ImGui::Columns(3, "Reeverb2Cols", true);

                // Coluna 1: Dimensão da Sala & Decaimento
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🏛️ SALA 3D & DECAIMENTO");
                ImGui::Separator();

                ImGui::SliderFloat("Tamanho da Sala (Size)", &room_size_norm, 0.1f, 1.0f, "%.2f");
                ImGui::SliderFloat("Tempo de Decaimento", &decay_time_s, 0.2f, 20.0f, "%.1f s");
                ImGui::SliderFloat("Difusão (Reflexões)", &diffusion_percent, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Pre-Delay", &pre_delay_ms, 0.0f, 200.0f, "%.0f ms");

                ImGui::NextColumn();

                // Coluna 2: Amortecimento & Filtros de Sala
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🌊 FILTROS DE ABSORÇÃO");
                ImGui::Separator();

                ImGui::SliderFloat("Low Cut (Graves)", &lowcut_locut_hz, 20.0f, 1000.0f, "%.0f Hz");
                ImGui::SliderFloat("High Damp (Amortecimento)", &damping_hicut_hz, 1000.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Early Reflections", &early_reflections, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Separação Estéreo & Mix
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "🎧 IMAGEM ESTÉREO & MIX");
                ImGui::Separator();

                ImGui::SliderFloat("Separação Estéreo (3D)", &stereo_separation, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Dry Level", &dry_level, 0.0f, 1.5f, "%.2fx");
                ImGui::SliderFloat("Wet Level", &wet_level, 0.0f, 1.5f, "%.2fx");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
