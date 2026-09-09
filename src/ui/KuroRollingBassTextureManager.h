#pragma once
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace KuroUI {

    struct RollingBassTexturePack {
        GLuint tex_bg_chassis = 0;
        GLuint tex_btn_gen_pr = 0;
        GLuint tex_btn_gen_pl = 0;
        GLuint tex_btn_kbbb = 0;
        GLuint tex_btn_kbb = 0;
        GLuint tex_btn_kb = 0;
        GLuint tex_btn_octave = 0;
        GLuint tex_btn_wave_saw = 0;
        GLuint tex_btn_wave_square = 0;
        GLuint tex_btn_wave_subsine = 0;
        GLuint tex_dial_phase = 0;
        GLuint tex_knob_metallic = 0;
        GLuint tex_kuro_logo = 0;
        GLuint tex_eq_bar = 0;
        bool is_loaded = false;

        static GLuint UploadGLTexture(int w, int h, const unsigned char* data) {
            if (!data || w <= 0 || h <= 0) return 0;
            GLuint tex_id = 0;
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            return tex_id;
        }

        bool LoadFromBin(const std::string& filepath) {
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open()) return false;

            char magic[8];
            file.read(magic, 8);
            if (std::string(magic, 8) != "KUROTEX1") return false;

            uint32_t count = 0;
            file.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));

            struct TocEntry {
                char name[32];
                uint32_t w, h, size;
            };
            std::vector<TocEntry> toc(count);
            for (uint32_t i = 0; i < count; ++i) {
                file.read(reinterpret_cast<char*>(&toc[i]), sizeof(TocEntry));
            }

            for (uint32_t i = 0; i < count; ++i) {
                std::vector<unsigned char> buf(toc[i].size);
                file.read(reinterpret_cast<char*>(buf.data()), toc[i].size);

                std::string name(toc[i].name);
                GLuint tex = UploadGLTexture(toc[i].w, toc[i].h, buf.data());

                if (name == "bg_chassis") tex_bg_chassis = tex;
                else if (name == "btn_gen_pr") tex_btn_gen_pr = tex;
                else if (name == "btn_gen_pl") tex_btn_gen_pl = tex;
                else if (name == "btn_kbbb") tex_btn_kbbb = tex;
                else if (name == "btn_kbb") tex_btn_kbb = tex;
                else if (name == "btn_kb") tex_btn_kb = tex;
                else if (name == "btn_octave") tex_btn_octave = tex;
                else if (name == "btn_wave_saw") tex_btn_wave_saw = tex;
                else if (name == "btn_wave_square") tex_btn_wave_square = tex;
                else if (name == "btn_wave_subsine") tex_btn_wave_subsine = tex;
                else if (name == "dial_phase") tex_dial_phase = tex;
                else if (name == "knob_metallic") tex_knob_metallic = tex;
                else if (name == "kuro_logo") tex_kuro_logo = tex;
                else if (name == "eq_bar") tex_eq_bar = tex;
            }

            is_loaded = true;
            return true;
        }

        void Init() {
            if (is_loaded) return;

            const char* paths[] = {
                "assets/ui/rolling_bass/textures.bin",
                "../assets/ui/rolling_bass/textures.bin",
                "C:/Users/USUÁRIO/.gemini/antigravity-ide/scratch/abduction_studio_v2/assets/ui/rolling_bass/textures.bin",
                "../../assets/ui/rolling_bass/textures.bin"
            };

            for (const char* p : paths) {
                if (LoadFromBin(p)) {
                    std::cout << "[RollingBassTexturePack] Loaded textures from: " << p << std::endl;
                    return;
                }
            }
        }
    };

    inline RollingBassTexturePack g_rolling_bass_tex_pack;

    // ── ROTINAS DE DESENHO COM TEXTURAS FOTORREALISTAS ────────────────────────────

    // 1. Desenho de Knob com Rotação de Textura por Hardware (AddImageQuad com ângulo)
    inline bool DrawPhotorealisticRotatingKnob(const char* id, ImDrawList* dl, GLuint tex_id, ImVec2 center, float radius, float* val, float min_v, float max_v, const char* label, const char* format_str) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiID knob_id = window->GetID(id);
        float h_extra = 22.0f;
        ImRect bb(ImVec2(center.x - radius, center.y - radius), ImVec2(center.x + radius, center.y + radius + h_extra));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, knob_id)) return false;

        bool hovered, held;
        bool changed = false;
        ImGui::ButtonBehavior(bb, knob_id, &hovered, &held);

        if (held) {
            float delta = ImGui::GetIO().MouseDelta.y;
            float step = (max_v - min_v) / 120.0f;
            *val -= delta * step;
            if (*val < min_v) *val = min_v;
            if (*val > max_v) *val = max_v;
            changed = true;
        }

        float norm = (*val - min_v) / (max_v - min_v);
        norm = std::clamp(norm, 0.0f, 1.0f);

        // Ângulo de rotação (de -135° a +135° / -2.35 rad a +2.35 rad)
        float angle_rad = (-135.0f + norm * 270.0f) * (float)M_PI / 180.0f;

        // 1. Soquete circular rebaixado no chassi (fundo suave)
        dl->AddCircleFilled(center, radius + 1.0f, IM_COL32(12, 16, 22, 255), 32);
        dl->AddCircle(center, radius + 1.0f, IM_COL32(26, 40, 52, 200), 32, 1.0f);

        // 2. Trilho e arco neon de valor (externo ao knob)
        float track_angle_min = 0.75f * (float)M_PI;
        float track_angle_max = 2.25f * (float)M_PI;
        float cur_arc_end = track_angle_min + norm * (track_angle_max - track_angle_min);

        dl->PathArcTo(center, radius + 4.0f, track_angle_min, track_angle_max, 28);
        dl->PathStroke(IM_COL32(18, 28, 38, 255), 0, 2.5f);

        if (norm > 0.005f) {
            dl->PathArcTo(center, radius + 4.0f, track_angle_min, cur_arc_end, 28);
            dl->PathStroke(IM_COL32(0, 229, 255, 255), 0, 2.5f);

            // Brilho sutil ao redor do arco
            dl->PathArcTo(center, radius + 4.0f, track_angle_min, cur_arc_end, 28);
            dl->PathStroke(IM_COL32(0, 240, 255, 60), 0, 4.5f);
        }

        // 3. Desenhar o knob metálico fotorrealista com transparência 100% circular
        if (tex_id != 0) {
            float cos_a = std::cos(angle_rad);
            float sin_a = std::sin(angle_rad);
            float r = radius;

            ImVec2 p0(center.x + (-r * cos_a - (-r) * sin_a), center.y + (-r * sin_a + (-r) * cos_a));
            ImVec2 p1(center.x + ( r * cos_a - (-r) * sin_a), center.y + ( r * sin_a + (-r) * cos_a));
            ImVec2 p2(center.x + ( r * cos_a - ( r) * sin_a), center.y + ( r * sin_a + ( r) * cos_a));
            ImVec2 p3(center.x + (-r * cos_a - ( r) * sin_a), center.y + (-r * sin_a + ( r) * cos_a));

            dl->AddImageQuad((ImTextureID)(intptr_t)tex_id, p0, p1, p2, p3, ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32(255, 255, 255, 255));
        } else {
            // Fallback elegante
            dl->AddCircleFilled(center, radius, IM_COL32(20, 28, 36, 255), 32);
            dl->AddCircle(center, radius, IM_COL32(40, 60, 80, 255), 32, 1.5f);
        }

        // 4. Dot indicador de posição luminoso
        float ptr_r = radius * 0.70f;
        float ptr_angle = (-135.0f + norm * 270.0f - 90.0f) * (float)M_PI / 180.0f;
        ImVec2 ptr_pt(center.x + std::cos(ptr_angle) * ptr_r, center.y + std::sin(ptr_angle) * ptr_r);
        dl->AddCircleFilled(ptr_pt, 2.5f, IM_COL32(0, 240, 255, 255));

        if (label) {
            ImVec2 lbl_sz = ImGui::CalcTextSize(label);
            ImVec2 lbl_pos(center.x - lbl_sz.x * 0.5f, center.y + radius + 7.0f);
            dl->AddText(lbl_pos, IM_COL32(160, 195, 220, 255), label);
        }

        if (hovered) {
            char tip[32];
            snprintf(tip, sizeof(tip), format_str, *val);
            ImGui::SetTooltip("%s: %s", label ? label : "", tip);
        }

        return changed;
    }

    // 2. Desenho do Phase Retrigger Dial Fotorrealista com Rotação
    inline bool DrawPhotorealisticPhaseDial(const char* id, ImDrawList* dl, GLuint tex_id, ImVec2 center, float radius, float* val_deg) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiID dial_id = window->GetID(id);
        float total_r = radius + 24.0f;
        ImRect bb(ImVec2(center.x - total_r, center.y - total_r), ImVec2(center.x + total_r, center.y + total_r));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, dial_id)) return false;

        bool hovered, held;
        bool changed = false;
        ImGui::ButtonBehavior(bb, dial_id, &hovered, &held);

        if (held) {
            ImVec2 mouse = ImGui::GetIO().MousePos;
            float dx = mouse.x - center.x;
            float dy = mouse.y - center.y;
            float angle_rad = std::atan2(dy, dx) + (float)M_PI * 0.5f;
            if (angle_rad < 0.0f) angle_rad += (float)M_PI * 2.0f;
            float deg = angle_rad * 180.0f / (float)M_PI;
            if (deg < 0.0f) deg = 0.0f;
            if (deg > 360.0f) deg = 360.0f;
            *val_deg = deg;
            changed = true;
        }

        // Marcações radiais estáticas de graus
        static const int grad_degrees[] = { 0, 30, 90, 120, 180, 210, 270, 300, 360 };
        for (int g = 0; g < 9; ++g) {
            int deg = grad_degrees[g];
            float rad = (deg - 90) * (float)M_PI / 180.0f;
            float r_tick_inner = radius + 4.0f;
            float r_tick_outer = radius + 8.0f;
            float r_text = radius + 15.0f;

            ImVec2 pt_in(center.x + std::cos(rad) * r_tick_inner, center.y + std::sin(rad) * r_tick_inner);
            ImVec2 pt_out(center.x + std::cos(rad) * r_tick_outer, center.y + std::sin(rad) * r_tick_outer);
            dl->AddLine(pt_in, pt_out, IM_COL32(0, 180, 200, 160), 1.0f);

            char lbl[8];
            snprintf(lbl, sizeof(lbl), "%d", deg);
            ImVec2 text_sz = ImGui::CalcTextSize(lbl);
            ImVec2 pt_txt(center.x + std::cos(rad) * r_text - text_sz.x * 0.5f, center.y + std::sin(rad) * r_text - text_sz.y * 0.5f);
            dl->AddText(pt_txt, IM_COL32(110, 150, 175, 200), lbl);
        }

        // Soquete circular rebaixado no chassi
        dl->AddCircleFilled(center, radius, IM_COL32(12, 16, 22, 255), 32);
        dl->AddCircle(center, radius, IM_COL32(30, 48, 64, 255), 32, 1.2f);

        // Desenhar a textura fotorrealista do Dial rotacionada com transparência pura
        float angle_rot = (*val_deg) * (float)M_PI / 180.0f;
        if (tex_id != 0) {
            float cos_a = std::cos(angle_rot);
            float sin_a = std::sin(angle_rot);
            float r = radius - 2.0f;

            ImVec2 p0(center.x + (-r * cos_a - (-r) * sin_a), center.y + (-r * sin_a + (-r) * cos_a));
            ImVec2 p1(center.x + ( r * cos_a - (-r) * sin_a), center.y + ( r * sin_a + (-r) * cos_a));
            ImVec2 p2(center.x + ( r * cos_a - ( r) * sin_a), center.y + ( r * sin_a + ( r) * cos_a));
            ImVec2 p3(center.x + (-r * cos_a - ( r) * sin_a), center.y + (-r * sin_a + ( r) * cos_a));

            dl->AddImageQuad((ImTextureID)(intptr_t)tex_id, p0, p1, p2, p3, ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32(255, 255, 255, 255));
        }

        // Arco Neon Ciano do Valor Atual
        float cur_rad_end = (*val_deg - 90.0f) * (float)M_PI / 180.0f;
        dl->PathArcTo(center, radius + 2.0f, -0.5f * (float)M_PI, cur_rad_end, 32);
        dl->PathStroke(IM_COL32(0, 229, 255, 255), 0, 3.0f);

        // Ponteiro
        ImVec2 pt_pointer(center.x + std::cos(cur_rad_end) * (radius * 0.72f), center.y + std::sin(cur_rad_end) * (radius * 0.72f));
        dl->AddLine(center, pt_pointer, IM_COL32(0, 240, 255, 255), 2.5f);
        dl->AddCircleFilled(pt_pointer, 3.0f, IM_COL32(255, 255, 255, 255));

        return changed;
    }

} // namespace KuroUI
