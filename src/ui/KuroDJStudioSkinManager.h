#pragma once
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "../core/SpotifyDJService.h"
#include "UIScaleManager.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroUI {

    class KuroDJStudioSkinManager {
    public:
        GLuint tex_btn_play = 0;
        GLuint tex_btn_cue = 0;
        GLuint tex_btn_sync = 0;
        GLuint tex_knob_eq = 0;
        GLuint tex_knob_cfx = 0;
        GLuint tex_fader_cap = 0;
        GLuint tex_crossfader_cap = 0;
        GLuint tex_pad_emerald = 0;
        GLuint tex_pad_magenta = 0;
        GLuint tex_pad_cyan = 0;
        GLuint tex_pad_amber = 0;
        GLuint tex_vu_meter = 0;
        GLuint tex_jog_platter = 0;
        GLuint tex_jog_wheel_l = 0;
        GLuint tex_jog_wheel_r = 0;
        GLuint tex_jog_vinyl = 0;
        GLuint tex_jog_center_alien = 0;
        GLuint tex_jog_center_vortex = 0;
        bool is_loaded = false;

        static KuroDJStudioSkinManager& getInstance() {
            static KuroDJStudioSkinManager instance;
            return instance;
        }

        void loadTextures() {
            if (is_loaded) return;

            std::string base = "assets/dj_skin/";
            if (!std::filesystem::exists(base + "btn_play.rgba")) {
                if (std::filesystem::exists("../assets/dj_skin/btn_play.rgba")) {
                    base = "../assets/dj_skin/";
                }
            }

            tex_btn_play        = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "btn_play.rgba", 64, 64);
            tex_btn_cue         = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "btn_cue.rgba", 64, 64);
            tex_btn_sync        = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "btn_sync.rgba", 64, 64);
            tex_knob_eq         = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "knob_aluminum_eq.rgba", 64, 64);
            tex_knob_cfx        = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "knob_color_fx.rgba", 80, 80);
            tex_fader_cap       = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "fader_cap_100mm.rgba", 32, 54);
            tex_crossfader_cap  = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "crossfader_cap.rgba", 52, 32);
            tex_pad_emerald     = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "pad_rubber_emerald.rgba", 64, 64);
            tex_pad_magenta     = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "pad_rubber_magenta.rgba", 64, 64);
            tex_pad_cyan        = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "pad_rubber_cyan.rgba", 64, 64);
            tex_pad_amber       = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "pad_rubber_amber.rgba", 64, 64);
            tex_vu_meter        = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "vu_meter_leds.rgba", 16, 128);
            tex_jog_platter     = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "jog_platter_512.rgba", 512, 512);

            tex_jog_wheel_l       = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "jog_wheel_left.rgba", 256, 256);
            tex_jog_wheel_r       = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "jog_wheel_right.rgba", 256, 256);
            tex_jog_vinyl         = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "jog_rotating_vinyl.rgba", 256, 256);
            tex_jog_center_alien  = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "jog_center_alien.rgba", 128, 128);
            tex_jog_center_vortex = KuroAudio::SpotifyDJService::UploadRGBATexture(base + "jog_center_vortex.rgba", 128, 128);

            is_loaded = true;
        }

        // 1. KNOB ROTATIVO DE ALUMÍNIO ESCOVADO COM ROTAÇÃO ATÉ 300 GRAUS E ACTIVE GLOW (SOLO / MUTE)
        static bool DrawSkinRotaryKnob(const char* label, float* p_value, float v_min, float v_max, float radius, ImVec4 theme_col, const char* unit_fmt = "%.1fx", float default_val = 1.0f, bool is_bipolar = false, bool is_large_cfx = false, bool is_muted = false, bool is_solo = false, bool show_value_text = true) {
            auto& skin = getInstance();
            if (!skin.is_loaded) skin.loadTextures();

            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            float total_w = radius * 2.0f + 12.0f;
            float total_h = show_value_text ? (radius * 2.0f + 14.0f) : (radius * 2.0f + 4.0f);

            ImGui::InvisibleButton(label, ImVec2(total_w, total_h));
            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();

            // Interação contínua via arrasto e scroll
            if (is_active && io.MouseDelta.y != 0.0f) {
                float step = (v_max - v_min) / 200.0f;
                *p_value -= io.MouseDelta.y * step;
                *p_value = std::clamp(*p_value, v_min, v_max);
            }
            if (is_hovered && io.MouseWheel != 0.0f) {
                float step = (v_max - v_min) / 40.0f;
                *p_value += io.MouseWheel * step;
                *p_value = std::clamp(*p_value, v_min, v_max);
            }
            if (is_hovered && ImGui::IsMouseDoubleClicked(0)) {
                *p_value = default_val;
            }

            ImVec2 center = ImVec2(pos.x + total_w * 0.5f, pos.y + radius + (show_value_text ? 3.0f : 2.0f));

            // ACTIVE GLOW DINÂMICO (Interação / Solo / Mute)
            if (is_muted) {
                float pulse = (std::sin((float)ImGui::GetTime() * 6.0f) + 1.0f) * 0.5f;
                ImU32 mute_aura = IM_COL32(255, 35, 65, (int)(110 + 110 * pulse));
                dl->AddCircleFilled(center, radius + 5.0f, IM_COL32(255, 30, 60, (int)(30 + 40 * pulse)), 32);
                dl->AddCircle(center, radius + 2.5f, mute_aura, 32, 2.2f);
            } else if (is_solo) {
                float pulse = (std::sin((float)ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
                ImU32 solo_aura = IM_COL32(255, 215, 0, (int)(150 + 95 * pulse));
                dl->AddCircleFilled(center, radius + 6.0f, IM_COL32(255, 210, 0, (int)(45 + 40 * pulse)), 32);
                dl->AddCircle(center, radius + 2.8f, solo_aura, 32, 2.5f);
            } else if (is_active) {
                // Glow ativo ultra-vibrante de manipulação física
                dl->AddCircleFilled(center, radius + 6.5f, ImGui::GetColorU32(ImVec4(theme_col.x, theme_col.y, theme_col.z, 0.25f)), 32);
                dl->AddCircle(center, radius + 4.0f, ImGui::GetColorU32(ImVec4(theme_col.x, theme_col.y, theme_col.z, 0.65f)), 32, 1.8f);
                dl->AddCircle(center, radius + 1.8f, ImGui::GetColorU32(theme_col), 32, 2.5f);
            } else if (is_hovered) {
                dl->AddCircle(center, radius + 2.0f, ImGui::GetColorU32(ImVec4(theme_col.x, theme_col.y, theme_col.z, 0.50f)), 32, 1.5f);
            }

            // Cálculo do ângulo em radianos: 300 graus totais (-150° a +150°)
            float norm = (*p_value - v_min) / std::max(0.0001f, (v_max - v_min));
            norm = std::clamp(norm, 0.0f, 1.0f);
            float angle_rad = (-150.0f + norm * 300.0f) * ((float)M_PI / 180.0f);

            // Sombra e anel exterior
            dl->AddCircleFilled(center, radius + 2.0f, IM_COL32(5, 7, 10, 220), 32);
            dl->AddCircle(center, radius + 1.0f, IM_COL32(25, 35, 48, 180), 32, 1.0f);

            // Anel de LED indicador ao redor do knob
            float arc_r = radius + 3.5f;
            float a_start = -150.0f * ((float)M_PI / 180.0f) - (float)M_PI * 0.5f;
            float a_curr = a_start + norm * (300.0f * ((float)M_PI / 180.0f));

            ImU32 led_color = is_muted ? IM_COL32(255, 40, 60, 220) : (is_solo ? IM_COL32(255, 215, 0, 240) : ImGui::GetColorU32(theme_col));

            if (is_bipolar) {
                float a_mid = - (float)M_PI * 0.5f;
                int segs = std::max(3, (int)(std::abs(norm - 0.5f) * 32.0f));
                for (int s = 0; s < segs; s++) {
                    float t0 = (float)s / (float)segs;
                    float t1 = (float)(s + 1) / (float)segs;
                    float a0 = a_mid + t0 * (a_curr - a_mid);
                    float a1 = a_mid + t1 * (a_curr - a_mid);
                    ImVec2 p0 = ImVec2(center.x + std::cos(a0) * arc_r, center.y + std::sin(a0) * arc_r);
                    ImVec2 p1 = ImVec2(center.x + std::cos(a1) * arc_r, center.y + std::sin(a1) * arc_r);
                    dl->AddLine(p0, p1, led_color, is_active ? 2.8f : 1.8f);
                }
            } else {
                int segs = std::max(3, (int)(norm * 32.0f));
                for (int s = 0; s < segs; s++) {
                    float t0 = (float)s / (float)segs;
                    float t1 = (float)(s + 1) / (float)segs;
                    float a0 = a_start + t0 * (a_curr - a_start);
                    float a1 = a_start + t1 * (a_curr - a_start);
                    ImVec2 p0 = ImVec2(center.x + std::cos(a0) * arc_r, center.y + std::sin(a0) * arc_r);
                    ImVec2 p1 = ImVec2(center.x + std::cos(a1) * arc_r, center.y + std::sin(a1) * arc_r);
                    dl->AddLine(p0, p1, led_color, is_active ? 2.8f : 1.8f);
                }
            }

            // Renderização da Textura do Knob Giratório com AddImageQuad
            GLuint knob_tex = is_large_cfx ? (skin.tex_knob_cfx ? skin.tex_knob_cfx : skin.tex_knob_eq) : skin.tex_knob_eq;
            if (knob_tex != 0) {
                float r_knob = radius;
                float rot = angle_rad;
                float c = std::cos(rot);
                float s = std::sin(rot);

                ImVec2 p0 = ImVec2(center.x + (-r_knob * c - -r_knob * s), center.y + (-r_knob * s + -r_knob * c));
                ImVec2 p1 = ImVec2(center.x + ( r_knob * c - -r_knob * s), center.y + ( r_knob * s + -r_knob * c));
                ImVec2 p2 = ImVec2(center.x + ( r_knob * c -  r_knob * s), center.y + ( r_knob * s +  r_knob * c));
                ImVec2 p3 = ImVec2(center.x + (-r_knob * c -  r_knob * s), center.y + (-r_knob * s +  r_knob * c));

                dl->AddImageQuad((ImTextureID)(intptr_t)knob_tex,
                                 p0, p1, p2, p3,
                                 ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1));
            } else {
                // Procedural fallback
                dl->AddCircleFilled(center, radius, IM_COL32(24, 28, 36, 255), 32);
                dl->AddCircle(center, radius, IM_COL32(70, 85, 105, 255), 32, 1.5f);
            }

            // Indicador Neon na Ponta do Knob com active_glow
            float ind_ang = angle_rad - (float)M_PI * 0.5f;
            ImVec2 ind_pos = ImVec2(center.x + std::cos(ind_ang) * (radius * 0.76f), center.y + std::sin(ind_ang) * (radius * 0.76f));
            if (is_active) {
                dl->AddCircleFilled(ind_pos, 4.0f, ImGui::GetColorU32(ImVec4(theme_col.x, theme_col.y, theme_col.z, 0.40f)));
                dl->AddCircleFilled(ind_pos, 2.4f, led_color);
            } else {
                dl->AddCircleFilled(ind_pos, 1.8f, led_color);
            }

            // Rótulo de Valor (Opcional - omitido quando compactado em racks de console)
            if (show_value_text) {
                char val_buf[32];
                snprintf(val_buf, sizeof(val_buf), unit_fmt, *p_value);
                ImVec2 sz_v = ImGui::CalcTextSize(val_buf);
                dl->AddText(ImVec2(center.x - sz_v.x * 0.5f, pos.y + radius * 2.0f + 2.0f),
                            is_active ? IM_COL32(255, 255, 255, 255) : (is_muted ? IM_COL32(255, 100, 120, 200) : (is_solo ? IM_COL32(255, 225, 60, 220) : IM_COL32(180, 195, 215, 220))),
                            val_buf);
            }

            return is_active;
        }

        // 2. FADER METÁLICO VERTICAL 100MM ESTILO CLUB STANDARD
        static bool DrawSkinVerticalFader(const char* label, float* p_value, float v_min, float v_max, float width, float height, ImVec4 theme_col, const char* unit_fmt = "%.1fdB") {
            auto& skin = getInstance();
            if (!skin.is_loaded) skin.loadTextures();

            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton(label, ImVec2(width, height));
            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();

            float norm = (*p_value - v_min) / std::max(0.0001f, (v_max - v_min));
            norm = std::clamp(norm, 0.0f, 1.0f);

            // Interação de arraste e scroll no fader
            if (is_active) {
                float my = io.MousePos.y - pos.y;
                float n = 1.0f - (my / height);
                *p_value = v_min + std::clamp(n, 0.0f, 1.0f) * (v_max - v_min);
            }
            if (is_hovered && io.MouseWheel != 0.0f) {
                float step = (v_max - v_min) / 30.0f;
                *p_value += io.MouseWheel * step;
                *p_value = std::clamp(*p_value, v_min, v_max);
            }
            if (is_hovered && ImGui::IsMouseDoubleClicked(0)) {
                *p_value = (v_max + v_min) * 0.5f;
            }

            norm = (*p_value - v_min) / std::max(0.0001f, (v_max - v_min));
            norm = std::clamp(norm, 0.0f, 1.0f);

            // 1. Fundo da Trilha e Escala Milimétrica Decibel
            float track_x = pos.x + width * 0.5f;
            dl->AddRectFilled(ImVec2(track_x - 3.0f, pos.y + 4.0f), ImVec2(track_x + 3.0f, pos.y + height - 4.0f), IM_COL32(6, 8, 12, 255), 2.0f);
            dl->AddRect(ImVec2(track_x - 3.0f, pos.y + 4.0f), ImVec2(track_x + 3.0f, pos.y + height - 4.0f), IM_COL32(28, 38, 52, 180), 2.0f);

            // Trilha de LED iluminada por trás do fader com active_glow
            float fill_y = pos.y + height - 4.0f - norm * (height - 8.0f);
            if (is_active) {
                dl->AddLine(ImVec2(track_x, pos.y + height - 4.0f), ImVec2(track_x, fill_y), ImGui::GetColorU32(ImVec4(theme_col.x, theme_col.y, theme_col.z, 0.35f)), 5.0f);
            }
            dl->AddLine(ImVec2(track_x, pos.y + height - 4.0f), ImVec2(track_x, fill_y), ImGui::GetColorU32(theme_col), is_active ? 2.5f : 2.0f);

            // Ranhuras de calibração na lateral (+10, +6, 0, -5, -10, -inf)
            float ticks[] = { 1.0f, 0.85f, 0.70f, 0.50f, 0.30f, 0.0f };
            for (float tk : ticks) {
                float ty = pos.y + height - 4.0f - tk * (height - 8.0f);
                dl->AddLine(ImVec2(track_x - 10.0f, ty), ImVec2(track_x - 5.0f, ty), IM_COL32(90, 110, 135, 160), 1.0f);
                dl->AddLine(ImVec2(track_x + 5.0f, ty), ImVec2(track_x + 10.0f, ty), IM_COL32(90, 110, 135, 160), 1.0f);
            }

            // 2. Cap Metálico do Fader Recortado
            float cap_w = width * 0.75f;
            float cap_h = std::clamp(height * 0.35f, 16.0f, 24.0f);
            float cap_y = pos.y + (1.0f - norm) * (height - cap_h);
            ImVec2 cap_min = ImVec2(track_x - cap_w * 0.5f, cap_y);
            ImVec2 cap_max = ImVec2(track_x + cap_w * 0.5f, cap_y + cap_h);

            if (skin.tex_fader_cap != 0) {
                dl->AddImage((ImTextureID)(intptr_t)skin.tex_fader_cap, cap_min, cap_max);
            } else {
                dl->AddRectFilled(cap_min, cap_max, IM_COL32(45, 52, 65, 255), 3.0f);
                dl->AddRect(cap_min, cap_max, IM_COL32(180, 195, 215, 255), 3.0f, 0, 1.5f);
                dl->AddLine(ImVec2(cap_min.x + 3.0f, cap_y + cap_h * 0.5f), ImVec2(cap_max.x - 3.0f, cap_y + cap_h * 0.5f), IM_COL32(255, 255, 255, 255), 2.0f);
            }

            // Indicador de valor no Hover
            if (is_hovered || is_active) {
                char db_buf[32];
                snprintf(db_buf, sizeof(db_buf), unit_fmt, *p_value);
                ImGui::SetTooltip("%s: %s", label, db_buf);
            }

            return is_active;
        }

        // 3. CROSSFADER HORIZONTAL DE ALTA PRECISÃO
        static bool DrawSkinCrossfader(const char* label, float* p_value, float width, float height, ImVec4 theme_col) {
            auto& skin = getInstance();
            if (!skin.is_loaded) skin.loadTextures();

            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton(label, ImVec2(width, height));
            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();

            if (is_active) {
                float mx = io.MousePos.x - pos.x;
                *p_value = std::clamp(mx / width, 0.0f, 1.0f);
            }
            if (is_hovered && ImGui::IsMouseDoubleClicked(0)) {
                *p_value = 0.5f; // Centro instantâneo
            }

            // Trilha horizontal
            float mid_y = pos.y + height * 0.5f;
            dl->AddRectFilled(ImVec2(pos.x + 4.0f, mid_y - 3.0f), ImVec2(pos.x + width - 4.0f, mid_y + 3.0f), IM_COL32(6, 8, 12, 255), 2.0f);
            dl->AddRect(ImVec2(pos.x + 4.0f, mid_y - 3.0f), ImVec2(pos.x + width - 4.0f, mid_y + 3.0f), IM_COL32(28, 38, 52, 180), 2.0f);

            // Marcação de Centro
            dl->AddLine(ImVec2(pos.x + width * 0.5f, mid_y - 8.0f), ImVec2(pos.x + width * 0.5f, mid_y + 8.0f), IM_COL32(180, 205, 230, 200), 1.5f);

            // Cap do Crossfader
            float cap_w = 40.0f;
            float cap_h = height * 0.85f;
            float cap_x = pos.x + 4.0f + (*p_value) * (width - 8.0f - cap_w);
            ImVec2 c_min = ImVec2(cap_x, mid_y - cap_h * 0.5f);
            ImVec2 c_max = ImVec2(cap_x + cap_w, mid_y + cap_h * 0.5f);

            if (skin.tex_crossfader_cap != 0) {
                dl->AddImage((ImTextureID)(intptr_t)skin.tex_crossfader_cap, c_min, c_max);
            } else {
                dl->AddRectFilled(c_min, c_max, IM_COL32(50, 58, 70, 255), 3.0f);
                dl->AddRect(c_min, c_max, IM_COL32(200, 215, 235, 255), 3.0f, 0, 1.5f);
                dl->AddLine(ImVec2(cap_x + cap_w * 0.5f, c_min.y + 2.0f), ImVec2(cap_x + cap_w * 0.5f, c_max.y - 2.0f), IM_COL32(255, 255, 255, 255), 2.0f);
            }

            return is_active;
        }

        // 4. BOTÃO CIRCULAR ILUMINADO COM DYNAMIC ACTIVE GLOW (PLAY, CUE, SYNC)
        static bool DrawSkinCircularButton(const char* label, bool is_active, GLuint tex_btn, float diameter, ImVec4 glow_col, const char* text_caption = nullptr) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton(label, ImVec2(diameter, diameter));
            bool is_hovered = ImGui::IsItemHovered();
            bool is_clicked = ImGui::IsItemClicked(0);
            bool is_held = ImGui::IsItemActive();

            ImVec2 center = ImVec2(pos.x + diameter * 0.5f, pos.y + diameter * 0.5f);
            float radius = diameter * 0.5f;

            // ACTIVE GLOW MULTICAMADA (Pressionado / Tocado / Ativo)
            if (is_held) {
                // Brilho intenso de pressão física no botão
                dl->AddCircleFilled(center, radius + 8.0f, ImGui::GetColorU32(ImVec4(glow_col.x, glow_col.y, glow_col.z, 0.28f)), 36);
                dl->AddCircleFilled(center, radius + 4.5f, ImGui::GetColorU32(ImVec4(glow_col.x, glow_col.y, glow_col.z, 0.60f)), 36);
                dl->AddCircle(center, radius + 1.5f, ImGui::GetColorU32(glow_col), 36, 3.2f);
            } else if (is_active) {
                // Pulsação rítmica fluida de playback / cueing ativo
                float pulse = (std::sin((float)ImGui::GetTime() * 7.5f) + 1.0f) * 0.5f;
                dl->AddCircleFilled(center, radius + 5.0f, ImGui::GetColorU32(ImVec4(glow_col.x, glow_col.y, glow_col.z, 0.22f + 0.16f * pulse)), 36);
                dl->AddCircle(center, radius + 2.5f, ImGui::GetColorU32(ImVec4(glow_col.x, glow_col.y, glow_col.z, 0.85f + 0.15f * pulse)), 36, 2.5f);
            } else if (is_hovered) {
                dl->AddCircleFilled(center, radius + 3.0f, ImGui::GetColorU32(ImVec4(glow_col.x, glow_col.y, glow_col.z, 0.18f)), 32);
                dl->AddCircle(center, radius + 1.0f, ImGui::GetColorU32(glow_col), 32, 1.8f);
            } else {
                dl->AddCircleFilled(center, radius, IM_COL32(10, 14, 18, 255), 32);
                dl->AddCircle(center, radius, IM_COL32(35, 45, 60, 200), 32, 1.2f);
            }

            // Textura PNG do botão com depressão tátil ao pressionar
            if (tex_btn != 0) {
                float press_shrink = is_held ? 2.0f : 0.0f;
                dl->AddImage((ImTextureID)(intptr_t)tex_btn,
                             ImVec2(pos.x + press_shrink, pos.y + press_shrink),
                             ImVec2(pos.x + diameter - press_shrink, pos.y + diameter - press_shrink));
            } else {
                dl->AddCircleFilled(center, radius * 0.85f, is_active ? ImGui::GetColorU32(glow_col) : IM_COL32(20, 26, 36, 255), 32);
            }

            // Texto se houver
            if (text_caption && text_caption[0] != '\0') {
                ImVec2 sz_t = ImGui::CalcTextSize(text_caption);
                dl->AddText(ImVec2(center.x - sz_t.x * 0.5f, center.y - sz_t.y * 0.5f),
                            is_active ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255),
                            text_caption);
            }

            return is_clicked;
        }

        // 5. PAD EMBORRACHADO QUADRADO 1:1 COM DYNAMIC ACTIVE GLOW RGB E SILICONE CHANFRADO
        static bool DrawSkinPad(const char* label, bool is_active, GLuint tex_pad, ImVec2 size, const char* main_txt, const char* sub_txt, ImU32 pad_glow_col) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton(label, size);
            bool is_hovered = ImGui::IsItemHovered();
            bool is_clicked = ImGui::IsItemClicked(0);
            bool is_held = ImGui::IsItemActive();

            // ACTIVE GLOW DO PAD DE SILICONE
            if (is_held) {
                // Efeito de choque luminoso tátil no impacto do dedo
                dl->AddRectFilled(ImVec2(pos.x - 5.0f, pos.y - 5.0f), ImVec2(pos.x + size.x + 5.0f, pos.y + size.y + 5.0f), (pad_glow_col & 0x00FFFFFF) | 0x40000000, 8.0f);
                dl->AddRectFilled(ImVec2(pos.x - 2.0f, pos.y - 2.0f), ImVec2(pos.x + size.x + 2.0f, pos.y + size.y + 2.0f), (pad_glow_col & 0x00FFFFFF) | 0x90000000, 6.0f);
                dl->AddRect(ImVec2(pos.x - 1.0f, pos.y - 1.0f), ImVec2(pos.x + size.x + 1.0f, pos.y + size.y + 1.0f), pad_glow_col, 5.0f, 0, 2.5f);
            } else if (is_active) {
                // Pad ativo com anel neon vivo e difusão perimétrica
                dl->AddRectFilled(ImVec2(pos.x - 3.0f, pos.y - 3.0f), ImVec2(pos.x + size.x + 3.0f, pos.y + size.y + 3.0f), (pad_glow_col & 0x00FFFFFF) | 0x45000000, 6.0f);
                dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), pad_glow_col, 5.0f, 0, 2.0f);
            } else if (is_hovered) {
                dl->AddRectFilled(ImVec2(pos.x - 2.0f, pos.y - 2.0f), ImVec2(pos.x + size.x + 2.0f, pos.y + size.y + 2.0f), (pad_glow_col & 0x00FFFFFF) | 0x22000000, 5.0f);
                dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), (pad_glow_col & 0x00FFFFFF) | 0xCC000000, 4.0f, 0, 1.5f);
            } else {
                dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(12, 16, 22, 255), 4.0f);
                dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(35, 45, 60, 200), 4.0f, 0, 1.2f);
            }

            // Textura PNG de silicone do pad (perfeita proporção 1:1)
            if (tex_pad != 0) {
                float sh = is_held ? 1.5f : 0.0f;
                dl->AddImage((ImTextureID)(intptr_t)tex_pad, ImVec2(pos.x + sh, pos.y + sh), ImVec2(pos.x + size.x - sh, pos.y + size.y - sh));
            }

            // Rótulos de texto elegantes no centro do pad
            if (main_txt && main_txt[0] != '\0') {
                ImVec2 sz_m = ImGui::CalcTextSize(main_txt);
                float y_offset = (sub_txt && sub_txt[0] != '\0') ? -5.0f : 0.0f;
                dl->AddText(ImVec2(pos.x + (size.x - sz_m.x) * 0.5f + 1, pos.y + (size.y - sz_m.y) * 0.5f + y_offset + 1), IM_COL32(0, 0, 0, 220), main_txt);
                dl->AddText(ImVec2(pos.x + (size.x - sz_m.x) * 0.5f, pos.y + (size.y - sz_m.y) * 0.5f + y_offset), IM_COL32(255, 255, 255, 255), main_txt);
            }
            if (sub_txt && sub_txt[0] != '\0') {
                ImVec2 sz_s = ImGui::CalcTextSize(sub_txt);
                dl->AddText(ImVec2(pos.x + (size.x - sz_s.x) * 0.5f + 1, pos.y + (size.y - sz_s.y) * 0.5f + 8.0f + 1), IM_COL32(0, 0, 0, 220), sub_txt);
                dl->AddText(ImVec2(pos.x + (size.x - sz_s.x) * 0.5f, pos.y + (size.y - sz_s.y) * 0.5f + 8.0f),
                            (is_active || is_held) ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 205, 230, 210), sub_txt);
            }

            return is_clicked;
        }

        // 6. VU METER ESTÉREO DE LEDS EM ESCALA REAL
        static void DrawSkinVUMeter(ImDrawList* dl, ImVec2 pos, ImVec2 size, float peak_l, float peak_r) {
            auto& skin = getInstance();
            if (!skin.is_loaded) skin.loadTextures();

            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(6, 8, 12, 255), 2.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(25, 35, 48, 180), 2.0f);

            int num_segs = 20;
            float seg_h = (size.y - 6.0f) / (float)num_segs;
            float bar_w = (size.x - 5.0f) * 0.5f;

            float norm_l = std::clamp(peak_l, 0.0f, 1.0f);
            float norm_r = std::clamp(peak_r, 0.0f, 1.0f);
            int active_segs_l = (int)(norm_l * (float)num_segs);
            int active_segs_r = (int)(norm_r * (float)num_segs);

            for (int i = 0; i < num_segs; i++) {
                float y = pos.y + size.y - 3.0f - (i + 1) * seg_h;
                ImU32 col_lit;
                ImU32 col_dim;
                if (i < 12) {
                    col_lit = IM_COL32(0, 255, 120, 255);
                    col_dim = IM_COL32(0, 60, 25, 150);
                } else if (i < 17) {
                    col_lit = IM_COL32(255, 180, 0, 255);
                    col_dim = IM_COL32(70, 45, 0, 150);
                } else {
                    col_lit = IM_COL32(255, 30, 60, 255);
                    col_dim = IM_COL32(75, 10, 20, 150);
                }

                // Canal L
                dl->AddRectFilled(ImVec2(pos.x + 2.0f, y), ImVec2(pos.x + 2.0f + bar_w, y + seg_h - 1.0f),
                                  (i <= active_segs_l) ? col_lit : col_dim, 1.0f);
                // Canal R
                dl->AddRectFilled(ImVec2(pos.x + 3.0f + bar_w, y), ImVec2(pos.x + size.x - 2.0f, y + seg_h - 1.0f),
                                  (i <= active_segs_r) ? col_lit : col_dim, 1.0f);
            }
        }
    };
}
