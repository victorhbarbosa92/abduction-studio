#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityMute2UI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio Fruity Mute 2 (Sample-Accurate Gate & Quick Kill Switch)
        bool channel_mute_state = false; // Mudo ligado/desligado
        float fade_time_ms = 4.0f;       // Fade in / Fade out anti-click (0.1ms a 50ms)
        bool invert_mute_behavior = false; // Modo Ducking / Invertido
        float ducking_attenuation_db = -96.0f; // Nível de atenuação
        bool solo_listen_mode = false;

    public:
        KuroFruityMute2UI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(800, 480), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.20f, 0.30f, 0.85f)); // Vermelho Mute 2

            if (ImGui::Begin("🔇 FRUITY MUTE 2 (SAMPLE-ACCURATE AUTOMATION KILL SWITCH)###Mute2Window", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("⚡ Instant Zero-Click Kill (2.0ms Fade)")) {
                            fade_time_ms = 2.0f; invert_mute_behavior = false; ducking_attenuation_db = -96.0f;
                        }
                        if (ImGui::MenuItem("🦆 Sidechain Ducking Attenuator (-18dB)")) {
                            fade_time_ms = 12.0f; ducking_attenuation_db = -18.0f;
                        }
                        if (ImGui::MenuItem("📻 Smooth Crossfade Transition (25ms)")) {
                            fade_time_ms = 25.0f; ducking_attenuation_db = -96.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.45f, 1.0f), "FRUITY MUTE 2: SAMPLE-ACCURATE CLICK-FREE AUTOMATION GATE");
                ImGui::TextDisabled("Chave de corte instantâneo e automação de silêncio com envelope de micro-fade anti-estalido.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DO ENVELOPE DE CORTE ANTI-CLICK ───────
                ImVec2 fm_p0 = ImGui::GetCursorScreenPos();
                ImVec2 fm_sz = ImVec2(ImGui::GetContentRegionAvail().x, 140.0f);
                ImVec2 fm_p1 = ImVec2(fm_p0.x + fm_sz.x, fm_p0.y + fm_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(fm_p0, fm_p1, IM_COL32(14, 10, 12, 255), 4.0f);
                dl->AddRect(fm_p0, fm_p1, IM_COL32(50, 25, 30, 255), 4.0f);

                // Desenhar Curva de Passagem do Sinal (Aberto vs Mudo)
                float cy = fm_p0.y + fm_sz.y * 0.5f;
                int steps = (int)fm_sz.x;

                for (int x = 0; x < steps - 1; ++x) {
                    float t = (float)x / (float)steps;
                    float gain_curve = channel_mute_state ? (1.0f - std::exp(-t * (50.0f / fade_time_ms))) : 1.0f;
                    if (channel_mute_state) gain_curve = 1.0f - gain_curve;

                    float py0 = cy - (std::sin(t * 6.2831853f * 4.0f) * gain_curve) * (fm_sz.y * 0.40f);
                    float py1 = cy - (std::sin(((float)(x + 1) / (float)steps) * 6.2831853f * 4.0f) * gain_curve) * (fm_sz.y * 0.40f);

                    ImU32 col = channel_mute_state ? IM_COL32(255, 60, 80, 220) : IM_COL32(0, 240, 140, 240);
                    dl->AddLine(ImVec2(fm_p0.x + x, py0), ImVec2(fm_p0.x + x + 1, py1), col, 2.0f);
                }

                char fm_info[128];
                snprintf(fm_info, sizeof(fm_info), "ESTADO: %s | FADE ANTI-CLICK: %.1f ms | ATENUAÇÃO: %.1f dB", channel_mute_state ? "MUTADO (SILÊNCIO)" : "ATIVO (ÁUDIO ON)", fade_time_ms, ducking_attenuation_db);
                dl->AddText(ImVec2(fm_p0.x + 12.0f, fm_p0.y + 6.0f), IM_COL32(255, 120, 140, 240), fm_info);

                ImGui::Dummy(fm_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO MUTE 2 ─────────────────────────
                ImGui::Columns(2, "Mute2Cols", true);

                // Botão de Chave Master
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "🔇 CHAVE DE MUDO");
                if (channel_mute_state) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.15f, 0.2f, 1.0f));
                    if (ImGui::Button("🔴 MUTADO (CLIQUE PARA ATIVAR)", ImVec2(-1, 38))) channel_mute_state = false;
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.75f, 0.3f, 1.0f));
                    if (ImGui::Button("🟢 ÁUDIO ON (CLIQUE PARA MUTAR)", ImVec2(-1, 38))) channel_mute_state = true;
                }
                ImGui::PopStyleColor();

                ImGui::NextColumn();

                // Fade Time & Atenuação
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "⏱️ ENVELOPE & ATTENUATION");
                ImGui::SliderFloat("Fade Time Anti-Click", &fade_time_ms, 0.1f, 50.0f, "%.1f ms");
                ImGui::SliderFloat("Nível de Atenuação", &ducking_attenuation_db, -96.0f, 0.0f, "%.1f dB");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
