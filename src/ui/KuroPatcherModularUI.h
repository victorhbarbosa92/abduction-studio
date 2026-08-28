#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct PatcherNode {
        int id;
        std::string name;
        ImVec2 pos;
        ImVec2 size;
        ImU32 color;
        int input_pins;
        int output_pins;
        bool is_instrument;
    };

    struct PatcherCable {
        int from_node;
        int from_pin;
        int to_node;
        int to_pin;
        ImU32 color;
    };

    class KuroPatcherModularUI {
    private:
        bool is_open = false;
        std::vector<PatcherNode> nodes;
        std::vector<PatcherCable> cables;
        int selected_node_id = -1;
        bool is_dragging_cable = false;
        int drag_from_node = -1;
        int drag_from_pin = 0;

    public:
        KuroPatcherModularUI() {
            // Setup inicial de nós Patcher FL Studio
            nodes.push_back({ 0, "FL From MIDI In", ImVec2(50, 200), ImVec2(130, 60), IM_COL32(0, 180, 220, 255), 0, 1, false });
            nodes.push_back({ 1, "Abduction FM Alien Synth", ImVec2(240, 120), ImVec2(180, 75), IM_COL32(0, 230, 140, 255), 1, 1, true });
            nodes.push_back({ 2, "Analog Monster 80s", ImVec2(240, 260), ImVec2(180, 75), IM_COL32(230, 120, 0, 255), 1, 1, true });
            nodes.push_back({ 3, "Gross Beat Glitch FX", ImVec2(480, 140), ImVec2(160, 70), IM_COL32(180, 50, 220, 255), 1, 1, false });
            nodes.push_back({ 4, "Parametric EQ 2", ImVec2(480, 260), ImVec2(160, 70), IM_COL32(0, 150, 255, 255), 1, 1, false });
            nodes.push_back({ 5, "Soundgoodizer (Max A)", ImVec2(690, 190), ImVec2(170, 70), IM_COL32(255, 180, 0, 255), 2, 1, false });
            nodes.push_back({ 6, "FL To Master Out", ImVec2(920, 200), ImVec2(140, 60), IM_COL32(220, 50, 50, 255), 1, 0, false });

            // Cabos de roteamento modular
            cables.push_back({ 0, 0, 1, 0, IM_COL32(0, 200, 255, 255) }); // MIDI -> Synth 1
            cables.push_back({ 0, 0, 2, 0, IM_COL32(0, 200, 255, 255) }); // MIDI -> Synth 2
            cables.push_back({ 1, 0, 3, 0, IM_COL32(0, 230, 140, 255) }); // Synth 1 -> Gross Beat
            cables.push_back({ 2, 0, 4, 0, IM_COL32(230, 120, 0, 255) }); // Synth 2 -> EQ 2
            cables.push_back({ 3, 0, 5, 0, IM_COL32(180, 50, 220, 255) }); // Gross Beat -> Soundgoodizer
            cables.push_back({ 4, 0, 5, 1, IM_COL32(0, 150, 255, 255) }); // EQ 2 -> Soundgoodizer
            cables.push_back({ 5, 0, 6, 0, IM_COL32(255, 180, 0, 255) }); // Soundgoodizer -> Master Out
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(1050, 640), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.85f));

            if (ImGui::Begin("🧩 FL STUDIO PATCHER (MODULAR DSP & FX CHAIN BUILDER)###PatcherWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DO PATCHER ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Predefinições (Presets)")) {
                        if (ImGui::MenuItem("🛸 Alien Cyber Lead (Dual FM + Gross Beat)")) {}
                        if (ImGui::MenuItem("🔥 Huge Psytrance Bassline (Layer + EQ)")) {}
                        if (ImGui::MenuItem("🌌 Ambient Shimmer Space Reverb")) {}
                        if (ImGui::MenuItem("⚡ Mid/Side Multiband Splitter")) {}
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Adicionar Módulo")) {
                        if (ImGui::MenuItem("Sintetizador VST3 / MPE")) {}
                        if (ImGui::MenuItem("Efeito DSP (Gross Beat, Delay, Chorus)")) {}
                        if (ImGui::MenuItem("Controle de Ganho / Splitter")) {}
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // ── BARRA DE STATUS / MAP / SURFACE ─────────────────────────
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.09f, 0.12f, 1.0f));
                ImGui::BeginChild("PatcherHeader", ImVec2(0, 36), true);
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "MAP (Cadeia Modular)");
                ImGui::SameLine(180);
                ImGui::TextDisabled("|  SURFACE (Painel Customizado)");
                ImGui::SameLine(ImGui::GetWindowWidth() - 250);
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "⚡ Latência de Roteamento: 0 ms");
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                if (canvas_sz.x < 200.0f) canvas_sz.x = 800.0f;
                if (canvas_sz.y < 200.0f) canvas_sz.y = 400.0f;
                ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();

                // Grade Modular Dark do Patcher FL Studio
                dl->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(14, 17, 23, 255), 4.0f);
                
                // Desenhar Grid de Pontos
                float grid_step = 25.0f;
                for (float gx = canvas_p0.x; gx < canvas_p1.x; gx += grid_step) {
                    for (float gy = canvas_p0.y; gy < canvas_p1.y; gy += grid_step) {
                        dl->AddCircleFilled(ImVec2(gx, gy), 1.0f, IM_COL32(40, 50, 65, 120));
                    }
                }

                // ── 1. DESENHAR CABOS CURVOS BEZIER DO PATCHER ──────────────
                for (const auto& cable : cables) {
                    if (cable.from_node < (int)nodes.size() && cable.to_node < (int)nodes.size()) {
                        const auto& n_from = nodes[cable.from_node];
                        const auto& n_to = nodes[cable.to_node];

                        ImVec2 p_from = ImVec2(canvas_p0.x + n_from.pos.x + n_from.size.x, canvas_p0.y + n_from.pos.y + 30.0f);
                        ImVec2 p_to = ImVec2(canvas_p0.x + n_to.pos.x, canvas_p0.y + n_to.pos.y + 30.0f + cable.to_pin * 20.0f);

                        ImVec2 cp1 = ImVec2(p_from.x + 60.0f, p_from.y);
                        ImVec2 cp2 = ImVec2(p_to.x - 60.0f, p_to.y);

                        // Sombra do cabo
                        dl->AddBezierCubic(p_from, cp1, cp2, p_to, IM_COL32(0, 0, 0, 180), 4.5f);
                        // Cabo Brilhante
                        dl->AddBezierCubic(p_from, cp1, cp2, p_to, cable.color, 2.5f);
                    }
                }

                // ── 2. DESENHAR BLOCOS / MÓDULOS DO PATCHER ─────────────────
                for (size_t i = 0; i < nodes.size(); ++i) {
                    auto& node = nodes[i];
                    ImVec2 node_p0 = ImVec2(canvas_p0.x + node.pos.x, canvas_p0.y + node.pos.y);
                    ImVec2 node_p1 = ImVec2(node_p0.x + node.size.x, node_p0.y + node.size.y);

                    // Sombra e Fundo
                    dl->AddRectFilled(ImVec2(node_p0.x + 3, node_p0.y + 3), ImVec2(node_p1.x + 3, node_p1.y + 3), IM_COL32(0, 0, 0, 140), 6.0f);
                    dl->AddRectFilled(node_p0, node_p1, IM_COL32(25, 30, 40, 255), 6.0f);
                    dl->AddRectFilled(node_p0, ImVec2(node_p1.x, node_p0.y + 24.0f), node.color & 0x60FFFFFF, 6.0f);
                    dl->AddRect(node_p0, node_p1, node.color, 6.0f, 0, 1.8f);

                    // Nome do Módulo
                    dl->AddText(ImVec2(node_p0.x + 8.0f, node_p0.y + 4.0f), IM_COL32(240, 240, 240, 255), node.name.c_str());

                    // Conectores (Pins) de Entrada e Saída
                    if (node.input_pins > 0) {
                        for (int pin = 0; pin < node.input_pins; ++pin) {
                            ImVec2 pin_pos = ImVec2(node_p0.x, node_p0.y + 30.0f + pin * 20.0f);
                            dl->AddCircleFilled(pin_pos, 5.0f, IM_COL32(0, 220, 255, 255));
                            dl->AddCircle(pin_pos, 5.0f, IM_COL32(10, 15, 20, 255), 0, 1.5f);
                        }
                    }

                    if (node.output_pins > 0) {
                        ImVec2 pin_pos = ImVec2(node_p1.x, node_p0.y + 30.0f);
                        dl->AddCircleFilled(pin_pos, 5.0f, IM_COL32(255, 180, 0, 255));
                        dl->AddCircle(pin_pos, 5.0f, IM_COL32(10, 15, 20, 255), 0, 1.5f);
                    }
                }

                ImGui::Dummy(canvas_sz);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
