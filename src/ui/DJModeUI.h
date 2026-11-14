#pragma once
#include "imgui.h"
#include "../ai/StemSeparationEngine.h"

namespace KuroUI {

    inline void DrawKuroKnob(const char* label, float* p_value, float v_min, float v_max) {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float radius = 35.0f;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2 + 20));
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
            float delta = ImGui::GetIO().MouseDelta.y;
            *p_value -= delta * 0.01f * (v_max - v_min);
            if (*p_value < v_min) *p_value = v_min;
            if (*p_value > v_max) *p_value = v_max;
        }
        
        float center_x = p.x + radius;
        float center_y = p.y + radius;
        
        float angle_min = 3.14159f * 0.75f;
        float angle_max = 3.14159f * 2.25f;
        float t = (*p_value - v_min) / (v_max - v_min);
        float angle_val = angle_min + (angle_max - angle_min) * t;
        
        // Background rust/dark
        draw_list->AddCircleFilled(ImVec2(center_x, center_y), radius, IM_COL32(20, 20, 25, 255), 32);
        // Outer ring
        draw_list->AddCircle(ImVec2(center_x, center_y), radius, IM_COL32(10, 10, 10, 255), 32, 4.0f);
        
        // Value arc (Green)
        draw_list->PathArcTo(ImVec2(center_x, center_y), radius - 6.0f, angle_min, angle_val, 32);
        draw_list->PathStroke(IM_COL32(0, 255, 0, 255), 0, 4.0f);
        
        // Inner indicator (Purple)
        draw_list->AddCircle(ImVec2(center_x, center_y), radius * 0.4f, IM_COL32(150, 0, 255, 255), 16, 2.0f);
        draw_list->AddLine(ImVec2(center_x, center_y), ImVec2(center_x + cosf(angle_val) * (radius - 10.0f), center_y + sinf(angle_val) * (radius - 10.0f)), IM_COL32(0, 255, 0, 255), 3.0f);
        
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + radius * 2 + 5));
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "%.2f", *p_value);
    }

    inline void RenderDJMode(StemSeparationEngine* deckA, StemSeparationEngine* deckB, float* eqA, float* eqB, float* tempoA, float* tempoB, float* crossfader) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.08f, 1.0f));
        
        // 3 Colunas: DECK A | MIXER | DECK B
        ImGui::Columns(3, "dj_cols");
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.35f);
        ImGui::SetColumnWidth(1, ImGui::GetWindowWidth() * 0.30f);
        
        // --- DECK A ---
        ImGui::BeginChild("DeckA", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.5f, 1.0f), "DECK A - %s", deckA->hasFinished() ? "Track Loaded" : "No Track Loaded");
        ImGui::Separator();
        if (!deckA->hasFinished()) {
            if (ImGui::Button("Load Track A", ImVec2(-1, 40))) {
                deckA->startProcessing("trackA.wav"); // Simulando drag&drop
            }
        } else {
            ImGui::Text("BPM: %.1f | Key: %s", deckA->getBPM(), deckA->getKey().c_str());
            ImGui::ProgressBar(deckA->getProgress(), ImVec2(-1, 0));
        }
        ImGui::Spacing();
        ImGui::Text("TEMPO");
        ImGui::VSliderFloat("##tempoA", ImVec2(30, 200), tempoA, 0.5f, 2.0f, "%.2f");
        ImGui::EndChild();
        
        ImGui::NextColumn();
        
        // --- MIXER ---
        ImGui::BeginChild("MixerDJ", ImVec2(0, 0), true);
        ImGui::Columns(2, "eq_cols", false);
        
        // EQ DECK A
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "   HIGH"); DrawKuroKnob("##hA", &eqA[0], -1.0f, 1.0f);
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "   MID"); DrawKuroKnob("##mA", &eqA[1], -1.0f, 1.0f);
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "   LOW"); DrawKuroKnob("##lA", &eqA[2], -1.0f, 1.0f);
        
        ImGui::NextColumn();
        
        // EQ DECK B
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "   HIGH"); DrawKuroKnob("##hB", &eqB[0], -1.0f, 1.0f);
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "   MID"); DrawKuroKnob("##mB", &eqB[1], -1.0f, 1.0f);
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "   LOW"); DrawKuroKnob("##lB", &eqB[2], -1.0f, 1.0f);
        
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "A <---- CROSSFADER ----> B");
        ImGui::SliderFloat("##crossfader", crossfader, 0.0f, 1.0f, "%.2f");
        
        ImGui::EndChild();
        
        ImGui::NextColumn();
        
        // --- DECK B ---
        ImGui::BeginChild("DeckB", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.5f, 1.0f), "DECK B - %s", deckB->hasFinished() ? "Track Loaded" : "No Track Loaded");
        ImGui::Separator();
        if (!deckB->hasFinished()) {
            if (ImGui::Button("Load Track B", ImVec2(-1, 40))) {
                deckB->startProcessing("trackB.wav");
            }
        } else {
            ImGui::Text("BPM: %.1f | Key: %s", deckB->getBPM(), deckB->getKey().c_str());
            ImGui::ProgressBar(deckB->getProgress(), ImVec2(-1, 0));
        }
        ImGui::Spacing();
        ImGui::Text("TEMPO");
        ImGui::VSliderFloat("##tempoB", ImVec2(30, 200), tempoB, 0.5f, 2.0f, "%.2f");
        ImGui::EndChild();
        
        ImGui::Columns(1);
        ImGui::PopStyleColor();
    }
}
