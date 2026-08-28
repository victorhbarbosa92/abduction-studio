#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityScratcherUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Scratcher (Virtual Vinyl Turntable & Scratch Engine)
        float turntable_angle_deg = 0.0f; // Ângulo atual do prato de vinil (0° a 360°)
        float scratch_speed = 1.0f;       // Velocidade de reprodução (-3.0x reverso a +3.0x direto)
        float vinyl_friction = 0.85f;     // Atrito e inércia do vinil analógico
        float hold_brake_speed = 0.50f;   // Força da parada manual (Motor Stop)
        bool vinyl_motor_on = true;       // Motor do toca-discos ligado/desligado
        int active_vinyl_preset = 0;      // 0=Classic Hip-Hop Scratch, 1=Baby Scratch, 2=Transformer / Flare

    public:
        KuroFruityScratcherUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.50f, 0.00f, 0.85f)); // Laranja / Dourado Vinil Scratcher

            if (ImGui::Begin("🎧 FRUITY SCRATCHER (VIRTUAL VINYL TURNTABLE & MOTOR BRAKE)###ScratcherWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🎧 Classic Hip-Hop Record Scratch (Ahhh/Fresh)")) {
                            active_vinyl_preset = 0; vinyl_friction = 0.85f;
                        }
                        if (ImGui::MenuItem("⚡ Fast Baby Scratch Combo (Rhythmic)")) {
                            active_vinyl_preset = 1; vinyl_friction = 0.60f;
                        }
                        if (ImGui::MenuItem("🛑 Heavy Motor Brake (Turntable Power Down)")) {
                            active_vinyl_preset = 2; hold_brake_speed = 0.90f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.1f, 1.0f), "FRUITY SCRATCHER: TWO-DIRECTIONAL REALTIME VINYL SCRATCH & BRAKE");
                ImGui::TextDisabled("Emulador de toca-discos analógico e física de vinil para scratches manuais e efeitos de parada de motor.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. PRATO DE VINIL INTERATIVO 2D ─────────────────────────
                ImGui::Columns(2, "ScratcherCols", true);

                ImGui::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "💽 PRATO DE VINIL INTERATIVO (ARRASTE PARA SCRATCH)");
                ImGui::Spacing();

                ImVec2 sc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 sc_sz = ImVec2(280.0f, 280.0f);
                ImVec2 sc_p1 = ImVec2(sc_p0.x + sc_sz.x, sc_p0.y + sc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(sc_p0, sc_p1, IM_COL32(12, 14, 18, 255), 6.0f);
                dl->AddRect(sc_p0, sc_p1, IM_COL32(40, 45, 55, 255), 6.0f);

                ImVec2 center = ImVec2(sc_p0.x + sc_sz.x * 0.5f, sc_p0.y + sc_sz.y * 0.5f);
                float radius = 120.0f;

                // Desenhar Prato e Ranhuras do Vinil
                dl->AddCircleFilled(center, radius, IM_COL32(18, 18, 20, 255));
                dl->AddCircle(center, radius, IM_COL32(60, 60, 70, 255), 2.0f);

                for (int r = 1; r <= 4; ++r) {
                    dl->AddCircle(center, radius * (0.3f + r * 0.16f), IM_COL32(30, 30, 35, 180), 1.0f);
                }

                // Rótulo Central do Vinil (Selo Abduction)
                dl->AddCircleFilled(center, 40.0f, IM_COL32(200, 80, 20, 255));
                dl->AddCircle(center, 40.0f, IM_COL32(255, 180, 50, 255), 2.0f);
                dl->AddCircleFilled(center, 8.0f, IM_COL32(20, 20, 20, 255)); // Furo central

                // Linha Marcadora do Vinil (Agulha / Posição Angular)
                if (vinyl_motor_on) {
                    turntable_angle_deg += scratch_speed * 1.8f;
                    if (turntable_angle_deg >= 360.0f) turntable_angle_deg -= 360.0f;
                    if (turntable_angle_deg < 0.0f) turntable_angle_deg += 360.0f;
                }

                float rad = turntable_angle_deg * 0.0174532925f;
                ImVec2 marker_end = ImVec2(center.x + std::cos(rad) * radius, center.y + std::sin(rad) * radius);
                dl->AddLine(center, marker_end, IM_COL32(255, 255, 255, 255), 3.0f);
                dl->AddCircleFilled(marker_end, 5.0f, IM_COL32(255, 60, 60, 255));

                // Botão Invisível de Interação no Vinil
                ImGui::InvisibleButton("VinylHitbox", sc_sz);
                if (ImGui::IsItemActive()) {
                    ImVec2 mpos = ImGui::GetIO().MousePos;
                    float dx = mpos.x - center.x;
                    float dy = mpos.y - center.y;
                    float new_angle = std::atan2(dy, dx) * 57.2957795f;
                    float delta = new_angle - turntable_angle_deg;
                    if (delta > 180.0f) delta -= 360.0f;
                    if (delta < -180.0f) delta += 360.0f;

                    scratch_speed = std::clamp(delta * 0.2f, -3.0f, 3.0f);
                    turntable_angle_deg = new_angle;
                } else {
                    if (vinyl_motor_on) scratch_speed += (1.0f - scratch_speed) * 0.08f;
                    else scratch_speed += (0.0f - scratch_speed) * (0.02f + hold_brake_speed * 0.08f);
                }

                ImGui::NextColumn();

                // ── 2. SEÇÃO DE CONTROLES DO MOTOR & FÍSICA DE VINIL ────────
                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.2f, 1.0f), "🎛️ CONTROLE DE MOTOR & VELOCIDADE");
                ImGui::Separator();
                ImGui::Spacing();

                if (vinyl_motor_on) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
                    if (ImGui::Button("⚡ MOTOR ON (REPRODUZINDO 33 RPM)", ImVec2(-1, 36))) vinyl_motor_on = false;
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Button("🛑 MOTOR STOP (PARADA DE VINIL)", ImVec2(-1, 36))) vinyl_motor_on = true;
                }
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::SliderFloat("Velocidade Scratch", &scratch_speed, -3.0f, 3.0f, "%.2fx");
                ImGui::SliderFloat("Inércia / Atrito", &vinyl_friction, 0.1f, 1.0f, "%.2f");
                ImGui::SliderFloat("Força do Freio (Brake)", &hold_brake_speed, 0.0f, 1.0f, "%.2f");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botões Rápidos de Scratch
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "⚡ DISPAROS RÁPIDOS DE SCRATCH");
                if (ImGui::Button("🔄 Baby Scratch Rápido", ImVec2(160, 30))) {
                    scratch_speed = -2.5f;
                }
                ImGui::SameLine();
                if (ImGui::Button("🛑 Tape Stop / Vinyl Brake", ImVec2(160, 30))) {
                    vinyl_motor_on = false;
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
