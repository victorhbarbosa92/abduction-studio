#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct EnvBreakpoint {
        float x; // 0.0 a 1.0 (Tempo / Duração do Step)
        float y; // 0.0 a 1.0 (Valor de Saída da Modulação)
        float tension; // -1.0 a +1.0 (Curvatura Bezier)
    };

    class KuroEnvelopeControllerUI {
    private:
        bool is_open = false;

        // 8 Articuladores de Envelope Independentes (Articulator 1-8)
        int selected_articulator = 0;

        // Pontos de Envelope com Curvas Bezier Multi-Segmento
        std::vector<EnvBreakpoint> env_points = {
            { 0.0f, 0.0f, 0.0f },
            { 0.15f, 1.0f, 0.5f },  // Ataque rápido
            { 0.35f, 0.5f, -0.3f }, // Decay para Sustain
            { 0.70f, 0.5f, 0.0f },  // Sustain
            { 1.0f, 0.0f, -0.6f }   // Release suave
        };

        int active_lfo_shape = 0; // 0=Sine, 1=Triangle, 2=Square, 3=Random S&H
        float lfo_speed = 2.0f;   // Hz
        float lfo_depth = 0.40f;
        float base_level = 0.0f;
        float attack_scale = 1.0f;
        float release_scale = 1.0f;
        bool loop_envelope = true;

    public:
        KuroEnvelopeControllerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.60f, 0.00f, 0.85f)); // Laranja / Ouro Envelope Controller

            if (ImGui::Begin("📈 FRUITY ENVELOPE CONTROLLER (MULTI-SEGMENT BREAKPOINT LFO)###EnvelopeControllerWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("⚡ Pluck Envelope (Fast Snap Attack)")) {
                            env_points = { { 0.0f, 0.0f, 0.0f }, { 0.05f, 1.0f, 0.8f }, { 0.30f, 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } };
                        }
                        if (ImGui::MenuItem("🌊 Slow Ambient Pad Swell (Long Attack & Release)")) {
                            env_points = { { 0.0f, 0.0f, 0.0f }, { 0.45f, 1.0f, -0.4f }, { 0.75f, 0.8f, 0.0f }, { 1.0f, 0.0f, -0.7f } };
                        }
                        if (ImGui::MenuItem("🛸 Stepped Acid 1/16 Gater Pattern")) {
                            env_points = { { 0.0f, 1.0f, 0.0f }, { 0.25f, 0.0f, 0.0f }, { 0.50f, 1.0f, 0.0f }, { 0.75f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "FRUITY ENVELOPE CONTROLLER: 8-ARTICULATOR MULTI-POINT MODULATOR");
                ImGui::TextDisabled("Gera curvas de automação complexas de envelope e LFO para rotear para qualquer fader, cutoff ou parâmetro da DAW.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. SELEÇÃO DE ARTICULADORES (ARTICULATOR 1 A 8) ─────────
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.00f, 1.0f), "ARTICULADOR ATIVO:");
                ImGui::SameLine();

                for (int a = 0; a < 8; ++a) {
                    char art_tag[16];
                    snprintf(art_tag, sizeof(art_tag), "Art %d", a + 1);
                    if (selected_articulator == a) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.18f, 0.22f, 0.8f));
                    }

                    if (ImGui::Button(art_tag, ImVec2(65, 26))) {
                        selected_articulator = a;
                    }
                    ImGui::PopStyleColor();
                    if (a < 7) ImGui::SameLine();
                }

                ImGui::Spacing();

                // ── 2. GRÁFICO VISUAL DO ENVELOPE DE CURVA BEZIER ────────────
                ImVec2 env_p0 = ImGui::GetCursorScreenPos();
                ImVec2 env_sz = ImVec2(ImGui::GetContentRegionAvail().x, 200.0f);
                ImVec2 env_p1 = ImVec2(env_p0.x + env_sz.x, env_p0.y + env_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(env_p0, env_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                dl->AddRect(env_p0, env_p1, IM_COL32(40, 50, 65, 255), 4.0f);

                // Linha Base Inferior (0.0) e Superior (1.0)
                dl->AddLine(ImVec2(env_p0.x, env_p1.y - 10), ImVec2(env_p1.x, env_p1.y - 10), IM_COL32(40, 50, 65, 180), 1.0f);
                dl->AddLine(ImVec2(env_p0.x, env_p0.y + 10), ImVec2(env_p1.x, env_p0.y + 10), IM_COL32(40, 50, 65, 180), 1.0f);

                // Desenhar Curva do Envelope Interligando os Pontos
                int steps = (int)env_sz.x;
                for (size_t p = 0; p < env_points.size() - 1; ++p) {
                    const auto& pA = env_points[p];
                    const auto& pB = env_points[p + 1];

                    float px0 = env_p0.x + pA.x * env_sz.x;
                    float py0 = env_p1.y - 10.0f - pA.y * (env_sz.y - 20.0f);
                    float px1 = env_p0.x + pB.x * env_sz.x;
                    float py1 = env_p1.y - 10.0f - pB.y * (env_sz.y - 20.0f);

                    // Desenhar Linha de Curva
                    dl->AddLine(ImVec2(px0, py0), ImVec2(px1, py1), IM_COL32(255, 140, 0, 255), 2.5f);
                }

                // Desenhar Nós / Breakpoints Interativos
                for (size_t p = 0; p < env_points.size(); ++p) {
                    float bx = env_p0.x + env_points[p].x * env_sz.x;
                    float by = env_p1.y - 10.0f - env_points[p].y * (env_sz.y - 20.0f);

                    dl->AddCircleFilled(ImVec2(bx, by), 6.0f, IM_COL32(255, 200, 50, 255));
                    dl->AddCircle(ImVec2(bx, by), 9.0f, IM_COL32(255, 255, 255, 220), 1.5f);
                }

                char env_info[64];
                snprintf(env_info, sizeof(env_info), "ARTICULADOR %d: %zu PONTOS ATIVOS | LOOP: %s", selected_articulator + 1, env_points.size(), loop_envelope ? "LIGADO" : "DESLIGADO");
                dl->AddText(ImVec2(env_p0.x + 12.0f, env_p0.y + 8.0f), IM_COL32(255, 160, 50, 240), env_info);

                ImGui::Dummy(env_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. CONTROLES DE ESCALA DO ENVELOPE & LFO ────────────────
                ImGui::Columns(2, "EnvCols", true);

                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.0f, 1.0f), "🎛️ ESCALA & VELOCIDADE DO ENVELOPE");
                ImGui::Separator();

                ImGui::SliderFloat("Escala de Ataque (Time)", &attack_scale, 0.1f, 4.0f, "%.2fx");
                ImGui::SliderFloat("Escala de Release (Time)", &release_scale, 0.1f, 4.0f, "%.2fx");
                ImGui::SliderFloat("Nível Base (Offset)", &base_level, 0.0f, 1.0f, "%.2f");
                ImGui::Checkbox("Envelope em Loop Contínuo", &loop_envelope);

                ImGui::NextColumn();

                // Coluna 2: LFO Modulador Secundário
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🌊 LFO DE MODULAÇÃO INTEGRADO");
                ImGui::Separator();

                const char* shapes[] = { "Senoide Suave (Sine)", "Triângulo (Triangle)", "Onda Quadrada (Square)", "Sample & Hold Aleatório" };
                ImGui::Combo("Forma de LFO", &active_lfo_shape, shapes, IM_ARRAYSIZE(shapes));
                ImGui::SliderFloat("Velocidade (Speed)", &lfo_speed, 0.1f, 20.0f, "%.2f Hz");
                ImGui::SliderFloat("Profundidade (Depth)", &lfo_depth, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
