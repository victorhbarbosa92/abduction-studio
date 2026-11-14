#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include <cmath>

namespace SciFiHUD {

    enum class IconType {
        ABDUCTION_SHIP,     // Alien Saucer with Abduction Tractor Beam
        PLAYLIST_TRACKS,    // Multitrack Timeline & Clips
        CHANNEL_RACK,       // 16-Step Quantum Drum Sequencer Matrix
        PIANO_KEYS,         // Cyber Synth Piano Keys
        MIXER_FADERS,       // Equalizer Console Fader Sliders
        BROWSER_FOLDER,     // Cyber Data Vault / Folder
        AI_NEURAL_STEMS,    // AI Neural Stem Core & Satellites
        PLAY,               // Vector Laser Play Triangle
        PAUSE,              // Dual Energy Pause Bars
        STOP,               // Solid Stop Matrix Box
        RECORD,             // Pulsing Atomic Recording Reticle
        PROJECT_VAULT,      // Holographic Project Disk / Node
        WORKSPACE_TARGET,   // HUD Reticle Target
        METRONOME,          // Metronome pendulum
        TYPING_KEYBOARD,    // Typing-to-Piano virtual keyboard
        COUNTDOWN_PRECOUNT, // 3.2.1 Recording Countdown
        LOOP_RECORD,        // Loop Recording Cycle
        APP_DRAWER_HUB,     // Android 3x3 App Launcher Matrix
        SNAP_MAGNET,        // Magnetic Snap Tool
        PLUGIN_PICKER,      // VST / Audio Plugin Connector
        DJ_VINYL_DECKS      // Pioneer DJ Turntable & Vinyl
    };

    // Vector drawing routine for crisp spaceship console HUD glyphs
    inline void DrawIcon(ImDrawList* draw, ImVec2 p_min, ImVec2 p_max, IconType type, ImU32 col, float thickness = 1.5f) {
        if (!draw) return;
        
        float w = p_max.x - p_min.x;
        float h = p_max.y - p_min.y;
        float cx = p_min.x + w * 0.5f;
        float cy = p_min.y + h * 0.5f;

        switch (type) {
            case IconType::ABDUCTION_SHIP: {
                // Flying Saucer Hull
                float hull_w = w * 0.44f;
                float hull_h = h * 0.18f;
                draw->AddEllipse(ImVec2(cx, cy - h * 0.08f), ImVec2(hull_w, hull_h), col, 0.0f, 16, thickness);
                
                // Saucer Cockpit Dome (Top)
                draw->AddCircle(ImVec2(cx, cy - h * 0.22f), hull_w * 0.42f, col, 12, thickness);
                
                // Abduction Tractor Beam Lines (Radiating downwards)
                ImU32 beam_col = (col & 0x00FFFFFF) | 0x88000000;
                draw->AddLine(ImVec2(cx - hull_w * 0.5f, cy + hull_h * 0.5f), ImVec2(cx - hull_w * 0.9f, p_max.y), beam_col, 1.2f);
                draw->AddLine(ImVec2(cx, cy + hull_h * 0.5f), ImVec2(cx, p_max.y), col, 1.5f);
                draw->AddLine(ImVec2(cx + hull_w * 0.5f, cy + hull_h * 0.5f), ImVec2(cx + hull_w * 0.9f, p_max.y), beam_col, 1.2f);
                break;
            }

            case IconType::PLAYLIST_TRACKS: {
                // 3 Horizontal Track Lanes with Audio Blocks
                float lane_h = h / 4.0f;
                float y1 = p_min.y + lane_h * 0.8f;
                float y2 = p_min.y + lane_h * 2.0f;
                float y3 = p_min.y + lane_h * 3.2f;
                
                // Track 1
                draw->AddLine(ImVec2(p_min.x, y1), ImVec2(p_max.x, y1), (col & 0x00FFFFFF) | 0x44000000, 1.0f);
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.1f, y1 - 2.5f), ImVec2(p_min.x + w * 0.5f, y1 + 2.5f), col, 1.0f);
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.6f, y1 - 2.5f), ImVec2(p_min.x + w * 0.95f, y1 + 2.5f), col, 1.0f);

                // Track 2
                draw->AddLine(ImVec2(p_min.x, y2), ImVec2(p_max.x, y2), (col & 0x00FFFFFF) | 0x44000000, 1.0f);
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.25f, y2 - 2.5f), ImVec2(p_min.x + w * 0.85f, y2 + 2.5f), col, 1.0f);

                // Track 3
                draw->AddLine(ImVec2(p_min.x, y3), ImVec2(p_max.x, y3), (col & 0x00FFFFFF) | 0x44000000, 1.0f);
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.05f, y3 - 2.5f), ImVec2(p_min.x + w * 0.35f, y3 + 2.5f), col, 1.0f);
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.45f, y3 - 2.5f), ImVec2(p_min.x + w * 0.75f, y3 + 2.5f), col, 1.0f);
                break;
            }

            case IconType::CHANNEL_RACK: {
                // 2 Rows of 4 Step Sequencer Quantum Pads (8 pads total)
                float pad_w = (w - 6.0f) / 4.0f;
                float pad_h = (h - 4.0f) / 2.0f;
                
                for (int r = 0; r < 2; r++) {
                    for (int c = 0; c < 4; c++) {
                        float px = p_min.x + c * (pad_w + 2.0f);
                        float py = p_min.y + r * (pad_h + 3.0f);
                        bool is_lit = (r == 0 && (c == 0 || c == 2)) || (r == 1 && (c == 1 || c == 3));
                        if (is_lit) {
                            draw->AddRectFilled(ImVec2(px, py), ImVec2(px + pad_w, py + pad_h), col, 1.0f);
                        } else {
                            draw->AddRect(ImVec2(px, py), ImVec2(px + pad_w, py + pad_h), (col & 0x00FFFFFF) | 0x66000000, 1.0f, 0, 1.0f);
                        }
                    }
                }
                break;
            }

            case IconType::PIANO_KEYS: {
                // Outer Keyboard Frame
                draw->AddRect(p_min, p_max, col, 1.5f, 0, thickness);
                
                // 3 White Key Dividers
                float kw = w / 4.0f;
                draw->AddLine(ImVec2(p_min.x + kw * 1.0f, p_min.y), ImVec2(p_min.x + kw * 1.0f, p_max.y), (col & 0x00FFFFFF) | 0x66000000, 1.0f);
                draw->AddLine(ImVec2(p_min.x + kw * 2.0f, p_min.y), ImVec2(p_min.x + kw * 2.0f, p_max.y), (col & 0x00FFFFFF) | 0x66000000, 1.0f);
                draw->AddLine(ImVec2(p_min.x + kw * 3.0f, p_min.y), ImVec2(p_min.x + kw * 3.0f, p_max.y), (col & 0x00FFFFFF) | 0x66000000, 1.0f);

                // 2 Black Keys (Extending 55% down)
                float bk_w = kw * 0.55f;
                float bk_h = h * 0.55f;
                draw->AddRectFilled(ImVec2(p_min.x + kw * 1.0f - bk_w * 0.5f, p_min.y), ImVec2(p_min.x + kw * 1.0f + bk_w * 0.5f, p_min.y + bk_h), col, 0.5f);
                draw->AddRectFilled(ImVec2(p_min.x + kw * 2.5f - bk_w * 0.5f, p_min.y), ImVec2(p_min.x + kw * 2.5f + bk_w * 0.5f, p_min.y + bk_h), col, 0.5f);
                break;
            }

            case IconType::MIXER_FADERS: {
                // 3 Vertical Mixer Stem Lines & Sliders
                float fx1 = p_min.x + w * 0.2f;
                float fx2 = p_min.x + w * 0.5f;
                float fx3 = p_min.x + w * 0.8f;

                draw->AddLine(ImVec2(fx1, p_min.y), ImVec2(fx1, p_max.y), (col & 0x00FFFFFF) | 0x66000000, 1.2f);
                draw->AddLine(ImVec2(fx2, p_min.y), ImVec2(fx2, p_max.y), (col & 0x00FFFFFF) | 0x66000000, 1.2f);
                draw->AddLine(ImVec2(fx3, p_min.y), ImVec2(fx3, p_max.y), (col & 0x00FFFFFF) | 0x66000000, 1.2f);

                // Fader Knobs (at varying heights)
                float k_w = w * 0.28f;
                float k_h = 3.5f;
                float ky1 = p_min.y + h * 0.30f;
                float ky2 = p_min.y + h * 0.65f;
                float ky3 = p_min.y + h * 0.40f;

                draw->AddRectFilled(ImVec2(fx1 - k_w * 0.5f, ky1 - k_h * 0.5f), ImVec2(fx1 + k_w * 0.5f, ky1 + k_h * 0.5f), col, 1.0f);
                draw->AddRectFilled(ImVec2(fx2 - k_w * 0.5f, ky2 - k_h * 0.5f), ImVec2(fx2 + k_w * 0.5f, ky2 + k_h * 0.5f), col, 1.0f);
                draw->AddRectFilled(ImVec2(fx3 - k_w * 0.5f, ky3 - k_h * 0.5f), ImVec2(fx3 + k_w * 0.5f, ky3 + k_h * 0.5f), col, 1.0f);
                break;
            }

            case IconType::BROWSER_FOLDER: {
                // Cyber Vault Folder
                float tab_w = w * 0.42f;
                float tab_h = h * 0.28f;
                
                // Folder Tab & Body Outline
                ImVec2 pts[6] = {
                    p_min,
                    ImVec2(p_min.x + tab_w, p_min.y),
                    ImVec2(p_min.x + tab_w + 3.0f, p_min.y + tab_h),
                    ImVec2(p_max.x, p_min.y + tab_h),
                    p_max,
                    ImVec2(p_min.x, p_max.y)
                };
                draw->AddPolyline(pts, 6, col, ImDrawFlags_Closed, thickness);
                
                // Internal Cyber Data Line
                draw->AddLine(ImVec2(p_min.x + w * 0.22f, cy + 1.0f), ImVec2(p_max.x - w * 0.22f, cy + 1.0f), col, 1.2f);
                draw->AddLine(ImVec2(p_min.x + w * 0.22f, cy + 4.5f), ImVec2(p_max.x - w * 0.40f, cy + 4.5f), (col & 0x00FFFFFF) | 0x88000000, 1.0f);
                break;
            }

            case IconType::AI_NEURAL_STEMS: {
                // Central Glowing AI Core
                draw->AddCircleFilled(ImVec2(cx, cy), 2.5f, col, 10);
                
                // 4 Satellite Neural Nodes (Top-Left, Top-Right, Bottom-Left, Bottom-Right)
                float d = w * 0.36f;
                ImVec2 n1(cx - d, cy - d);
                ImVec2 n2(cx + d, cy - d);
                ImVec2 n3(cx - d, cy + d);
                ImVec2 n4(cx + d, cy + d);

                draw->AddLine(ImVec2(cx, cy), n1, (col & 0x00FFFFFF) | 0x88000000, 1.0f);
                draw->AddLine(ImVec2(cx, cy), n2, (col & 0x00FFFFFF) | 0x88000000, 1.0f);
                draw->AddLine(ImVec2(cx, cy), n3, (col & 0x00FFFFFF) | 0x88000000, 1.0f);
                draw->AddLine(ImVec2(cx, cy), n4, (col & 0x00FFFFFF) | 0x88000000, 1.0f);

                draw->AddCircle(n1, 2.0f, col, 8, 1.2f);
                draw->AddCircle(n2, 2.0f, col, 8, 1.2f);
                draw->AddCircle(n3, 2.0f, col, 8, 1.2f);
                draw->AddCircle(n4, 2.0f, col, 8, 1.2f);
                break;
            }

            case IconType::PLAY: {
                // Laser Play Triangle
                ImVec2 p1(p_min.x + w * 0.15f, p_min.y);
                ImVec2 p2(p_min.x + w * 0.15f, p_max.y);
                ImVec2 p3(p_max.x - w * 0.10f, cy);
                draw->AddTriangleFilled(p1, p2, p3, col);
                break;
            }

            case IconType::PAUSE: {
                // Dual Energy Pause Bars
                float bar_w = w * 0.28f;
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.12f, p_min.y), ImVec2(p_min.x + w * 0.12f + bar_w, p_max.y), col, 1.0f);
                draw->AddRectFilled(ImVec2(p_max.x - w * 0.12f - bar_w, p_min.y), ImVec2(p_max.x - w * 0.12f, p_max.y), col, 1.0f);
                break;
            }

            case IconType::STOP: {
                // Solid Stop Matrix Square
                draw->AddRectFilled(ImVec2(p_min.x + w * 0.15f, p_min.y + h * 0.15f), ImVec2(p_max.x - w * 0.15f, p_max.y - h * 0.15f), col, 1.0f);
                break;
            }

            case IconType::RECORD: {
                // Pulsing Atomic Recording Reticle
                draw->AddCircleFilled(ImVec2(cx, cy), w * 0.32f, col, 16);
                draw->AddCircle(ImVec2(cx, cy), w * 0.46f, (col & 0x00FFFFFF) | 0x88000000, 16, 1.0f);
                break;
            }

            case IconType::PROJECT_VAULT: {
                // Holographic Project Node / Chip
                draw->AddRect(p_min, p_max, col, 2.0f, 0, thickness);
                draw->AddCircleFilled(ImVec2(cx, cy), 2.2f, col, 8);
                break;
            }

            case IconType::WORKSPACE_TARGET: {
                // HUD Reticle Target
                draw->AddCircle(ImVec2(cx, cy), w * 0.40f, col, 16, thickness);
                draw->AddLine(ImVec2(cx - w * 0.48f, cy), ImVec2(cx - w * 0.18f, cy), col, 1.2f);
                draw->AddLine(ImVec2(cx + w * 0.18f, cy), ImVec2(cx + w * 0.48f, cy), col, 1.2f);
                draw->AddLine(ImVec2(cx, cy - h * 0.48f), ImVec2(cx, cy - h * 0.18f), col, 1.2f);
                draw->AddLine(ImVec2(cx, cy + h * 0.18f), ImVec2(cx, cy + h * 0.48f), col, 1.2f);
                break;
            }

            case IconType::METRONOME: {
                // Inverted V Metronome Body & Pendulum
                draw->AddTriangle(ImVec2(cx, p_min.y + 1.0f), ImVec2(p_min.x + 2.0f, p_max.y - 1.0f), ImVec2(p_max.x - 2.0f, p_max.y - 1.0f), col, thickness);
                draw->AddLine(ImVec2(cx, p_max.y - 3.0f), ImVec2(cx + w * 0.22f, p_min.y + h * 0.35f), col, 1.3f);
                draw->AddCircleFilled(ImVec2(cx + w * 0.18f, p_min.y + h * 0.42f), 1.8f, col, 8);
                break;
            }

            case IconType::TYPING_KEYBOARD: {
                // Keyboard frame and keys
                draw->AddRect(ImVec2(p_min.x, p_min.y + 2.0f), ImVec2(p_max.x, p_max.y - 2.0f), col, 2.0f, 0, thickness);
                float kw = (w - 6.0f) / 3.0f;
                float kh = (h - 8.0f) / 2.0f;
                for (int r = 0; r < 2; r++) {
                    for (int c = 0; c < 3; c++) {
                        float kx = p_min.x + 2.0f + c * (kw + 1.0f);
                        float ky = p_min.y + 4.0f + r * (kh + 1.0f);
                        draw->AddRectFilled(ImVec2(kx, ky), ImVec2(kx + kw, ky + kh), (col & 0x00FFFFFF) | 0x99000000);
                    }
                }
                break;
            }

            case IconType::COUNTDOWN_PRECOUNT: {
                // Clock countdown indicator
                draw->AddCircle(ImVec2(cx, cy), w * 0.40f, col, 16, thickness);
                draw->AddLine(ImVec2(cx, cy), ImVec2(cx, cy - h * 0.28f), col, 1.2f);
                draw->AddLine(ImVec2(cx, cy), ImVec2(cx + w * 0.24f, cy), col, 1.2f);
                break;
            }

            case IconType::LOOP_RECORD: {
                // Circular arrows / repeat loop
                draw->AddCircle(ImVec2(cx, cy), w * 0.38f, col, 16, thickness);
                draw->AddTriangleFilled(ImVec2(cx + w * 0.38f, cy - 2.0f), ImVec2(cx + w * 0.38f + 3.0f, cy + 3.0f), ImVec2(cx + w * 0.38f - 3.0f, cy + 3.0f), col);
                break;
            }

            case IconType::APP_DRAWER_HUB: {
                // Android 3x3 App Launcher Matrix
                float dot_r = 1.3f;
                float sp = w / 3.4f;
                for (int r = -1; r <= 1; r++) {
                    for (int c = -1; c <= 1; c++) {
                        draw->AddCircleFilled(ImVec2(cx + c * sp, cy + r * sp), dot_r, col, 8);
                    }
                }
                break;
            }

            case IconType::SNAP_MAGNET: {
                // Horseshoe magnet
                float mw = w * 0.32f;
                draw->AddLine(ImVec2(cx - mw, p_min.y + 2.0f), ImVec2(cx - mw, cy + 1.0f), col, 1.5f);
                draw->AddLine(ImVec2(cx + mw, p_min.y + 2.0f), ImVec2(cx + mw, cy + 1.0f), col, 1.5f);
                draw->AddBezierCubic(ImVec2(cx - mw, cy + 1.0f), ImVec2(cx - mw, p_max.y), ImVec2(cx + mw, p_max.y), ImVec2(cx + mw, cy + 1.0f), col, 1.5f);
                break;
            }

            case IconType::PLUGIN_PICKER: {
                // Plug connector
                draw->AddRect(ImVec2(cx - w * 0.25f, cy - h * 0.25f), ImVec2(cx + w * 0.25f, cy + h * 0.35f), col, 1.0f, 0, thickness);
                draw->AddLine(ImVec2(cx - w * 0.15f, p_min.y + 1.0f), ImVec2(cx - w * 0.15f, cy - h * 0.25f), col, 1.5f);
                draw->AddLine(ImVec2(cx + w * 0.15f, p_min.y + 1.0f), ImVec2(cx + w * 0.15f, cy - h * 0.25f), col, 1.5f);
                draw->AddLine(ImVec2(cx, cy + h * 0.35f), ImVec2(cx, p_max.y), col, 1.2f);
                break;
            }

            case IconType::DJ_VINYL_DECKS: {
                // Vinyl disc outer ring
                float r = h * 0.42f;
                draw->AddCircle(ImVec2(cx - 1.0f, cy), r, col, 16, thickness);
                // Center spindle hole
                draw->AddCircleFilled(ImVec2(cx - 1.0f, cy), r * 0.30f, col, 12);
                // Tonearm needle
                draw->AddLine(ImVec2(p_max.x - 1.0f, p_min.y + 1.0f), ImVec2(cx + r * 0.15f, cy - r * 0.15f), col, 1.2f);
                break;
            }
        }
    }

    // High-Tech Spaceship Cockpit Navigation Button with Built-In Vector Icon
    inline bool SciFiButton(const char* id, const char* label, IconType icon, bool is_active, ImVec2 size = ImVec2(0, 22), const char* tooltip = nullptr) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID btn_id = window->GetID(id);

        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        const float icon_box_size = 13.0f;
        const float spacing = 6.0f;
        float btn_w = size.x;
        if (btn_w <= 0.0f) {
            btn_w = label_size.x + icon_box_size + spacing + style.FramePadding.x * 2.0f + 4.0f;
        }
        float btn_h = size.y > 0.0f ? size.y : 22.0f;

        ImVec2 pos = window->DC.CursorPos;
        const ImRect bb(pos, ImVec2(pos.x + btn_w, pos.y + btn_h));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, btn_id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, btn_id, &hovered, &held);

        // Cockpit HUD Color Scheme
        ImU32 bg_col;
        ImU32 border_col;
        ImU32 text_col;
        ImU32 icon_col;

        if (is_active) {
            bg_col = ImGui::GetColorU32(ImVec4(0.00f, 0.85f, 1.00f, 0.92f));     // Glowing Cyan Active
            border_col = ImGui::GetColorU32(ImVec4(0.50f, 1.00f, 1.00f, 1.00f));
            text_col = ImGui::GetColorU32(ImVec4(0.02f, 0.05f, 0.08f, 1.00f));   // Crisp Black Text
            icon_col = text_col;
        } else if (hovered) {
            bg_col = ImGui::GetColorU32(ImVec4(0.12f, 0.20f, 0.30f, 0.95f));     // Sci-Fi Cockpit Hover
            border_col = ImGui::GetColorU32(ImVec4(0.00f, 0.85f, 1.00f, 0.80f));
            text_col = ImGui::GetColorU32(ImVec4(0.95f, 0.98f, 1.00f, 1.00f));
            icon_col = ImGui::GetColorU32(ImVec4(0.00f, 0.85f, 1.00f, 1.00f));   // Neon Cyan Icon
        } else {
            bg_col = ImGui::GetColorU32(ImVec4(0.07f, 0.10f, 0.15f, 0.85f));     // Dark Ship Panel
            border_col = ImGui::GetColorU32(ImVec4(0.16f, 0.24f, 0.35f, 0.70f));
            text_col = ImGui::GetColorU32(ImVec4(0.70f, 0.82f, 0.92f, 1.00f));   // Crisp Pale Blue Text
            icon_col = ImGui::GetColorU32(ImVec4(0.00f, 0.75f, 0.90f, 0.85f));   // Subtle Cyan Icon
        }

        // Draw Cockpit Button Background with Cyber Rounded Corners & Tech Border
        ImDrawList* draw = window->DrawList;
        draw->AddRectFilled(bb.Min, bb.Max, bg_col, 4.0f);
        draw->AddRect(bb.Min, bb.Max, border_col, 4.0f, 0, 1.0f);

        // If Active, add high-tech bottom accent glow line
        if (is_active) {
            draw->AddLine(ImVec2(bb.Min.x + 2.0f, bb.Max.y - 1.5f), ImVec2(bb.Max.x - 2.0f, bb.Max.y - 1.5f), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.9f)), 2.0f);
        }

        // Render Vector Icon
        float icon_x0 = bb.Min.x + style.FramePadding.x + 2.0f;
        float icon_y0 = bb.Min.y + (btn_h - icon_box_size) * 0.5f;
        DrawIcon(draw, ImVec2(icon_x0, icon_y0), ImVec2(icon_x0 + icon_box_size, icon_y0 + icon_box_size), icon, icon_col, 1.4f);

        // Render Text Label
        float text_x = icon_x0 + icon_box_size + spacing;
        float text_y = bb.Min.y + (btn_h - label_size.y) * 0.5f;
        draw->AddText(ImVec2(text_x, text_y), text_col, label);

        if (tooltip && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tooltip);
        }

        return pressed;
    }

    // High-Tech Spaceship Transport Icon Button (Play, Pause, Stop, Rec)
    inline bool SciFiTransportBtn(const char* id, IconType icon, bool is_active, ImVec4 active_col = ImVec4(0.00f, 0.85f, 1.00f, 1.0f), ImVec2 size = ImVec2(24, 22), const char* tooltip = nullptr) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID btn_id = window->GetID(id);

        ImVec2 pos = window->DC.CursorPos;
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, g.Style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, btn_id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, btn_id, &hovered, &held);

        ImU32 bg_col;
        ImU32 border_col;
        ImU32 icon_col;

        if (is_active) {
            bg_col = ImGui::GetColorU32(active_col);
            border_col = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.9f));
            icon_col = ImGui::GetColorU32(ImVec4(0.02f, 0.05f, 0.08f, 1.0f));
        } else if (hovered) {
            bg_col = ImGui::GetColorU32(ImVec4(0.12f, 0.18f, 0.26f, 1.0f));
            border_col = ImGui::GetColorU32(active_col);
            icon_col = ImGui::GetColorU32(active_col);
        } else {
            bg_col = ImGui::GetColorU32(ImVec4(0.07f, 0.10f, 0.15f, 1.0f));
            border_col = ImGui::GetColorU32(ImVec4(0.16f, 0.24f, 0.35f, 0.8f));
            icon_col = ImGui::GetColorU32(active_col);
        }

        ImDrawList* draw = window->DrawList;
        draw->AddRectFilled(bb.Min, bb.Max, bg_col, 3.0f);
        draw->AddRect(bb.Min, bb.Max, border_col, 3.0f, 0, 1.0f);

        float icon_size = 11.0f;
        float ix = bb.Min.x + (size.x - icon_size) * 0.5f;
        float iy = bb.Min.y + (size.y - icon_size) * 0.5f;
        DrawIcon(draw, ImVec2(ix, iy), ImVec2(ix + icon_size, iy + icon_size), icon, icon_col, 1.5f);

        if (tooltip && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tooltip);
        }

        return pressed;
    }

} // namespace SciFiHUD
