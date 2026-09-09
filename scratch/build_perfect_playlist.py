import os
import sys

print("Building pure C++ KuroPlaylistUI.h...")

with open("scratch/generate_playlist_ui.py", "r", encoding="utf-8") as f:
    text = f.read()

s = text.find("#pragma once")
e = text.find("inline KuroPlaylistUI g_playlist_ui;") + len("inline KuroPlaylistUI g_playlist_ui;\n}")
orig = text[s:e]
assert orig.startswith("#pragma once")
assert orig.endswith("}")

# 1. Icons to insert before renderModernToolbar
icons_code = """
        // ── VECTOR HUD ICONS FOR PLAYLIST TOOLBAR ────────────────────────────────
        static void DrawIconPencil(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p_tip(c.x - r * 0.65f, c.y + r * 0.65f);
            ImVec2 p_top(c.x + r * 0.65f, c.y - r * 0.65f);
            float w = 2.4f;
            draw->AddLine(ImVec2(p_tip.x - w, p_tip.y - w), ImVec2(p_top.x - w, p_top.y - w), col, 1.4f);
            draw->AddLine(ImVec2(p_tip.x + w, p_tip.y + w), ImVec2(p_top.x + w, p_top.y + w), col, 1.4f);
            draw->AddTriangleFilled(ImVec2(p_tip.x - w, p_tip.y - w), ImVec2(p_tip.x + w, p_tip.y + w), ImVec2(c.x - r * 0.95f, c.y + r * 0.95f), col);
            draw->AddLine(ImVec2(p_top.x - w, p_top.y - w), ImVec2(p_top.x + w, p_top.y + w), col, 1.8f);
        }

        static void DrawIconBrush(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x + r * 0.1f, c.y - r * 0.1f), ImVec2(c.x + r * 0.85f, c.y - r * 0.85f), col, 2.0f);
            draw->AddLine(ImVec2(c.x - r * 0.15f, c.y - r * 0.15f), ImVec2(c.x + r * 0.2f, c.y + r * 0.2f), col, 2.5f);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.15f, c.y - r * 0.15f), ImVec2(c.x + r * 0.2f, c.y + r * 0.2f), ImVec2(c.x - r * 0.80f, c.y + r * 0.80f), col);
        }

        static void DrawIconRazor(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 pts[4] = {
                ImVec2(c.x - r * 0.75f, c.y - r * 0.35f),
                ImVec2(c.x + r * 0.45f, c.y - r * 0.85f),
                ImVec2(c.x + r * 0.75f, c.y + r * 0.35f),
                ImVec2(c.x - r * 0.45f, c.y + r * 0.85f)
            };
            draw->AddPolyline(pts, 4, col, ImDrawFlags_Closed, 1.4f);
            draw->AddLine(ImVec2(c.x - r * 0.35f, c.y + r * 0.12f), ImVec2(c.x + r * 0.35f, c.y - r * 0.12f), col, 1.4f);
            draw->AddCircleFilled(c, 1.5f, col);
        }

        static void DrawIconSlip(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - 3.5f, c.y - r * 0.75f), ImVec2(c.x - 3.5f, c.y + r * 0.75f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + 3.5f, c.y - r * 0.75f), ImVec2(c.x + 3.5f, c.y + r * 0.75f), col, 1.5f);
            draw->AddTriangleFilled(ImVec2(c.x - 5.5f, c.y), ImVec2(c.x - 1.5f, c.y - 3.0f), ImVec2(c.x - 1.5f, c.y + 3.0f), col);
            draw->AddTriangleFilled(ImVec2(c.x + 5.5f, c.y), ImVec2(c.x + 1.5f, c.y - 3.0f), ImVec2(c.x + 1.5f, c.y + 3.0f), col);
        }

        static void DrawIconSelect(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p0(c.x - r * 0.65f, c.y - r * 0.80f);
            ImVec2 p1(c.x + r * 0.65f, c.y + r * 0.10f);
            ImVec2 p2(c.x - r * 0.05f, c.y + r * 0.18f);
            ImVec2 p3(c.x - r * 0.35f, c.y + r * 0.85f);
            draw->AddTriangleFilled(p0, p1, p2, col);
            draw->AddLine(p2, p3, col, 2.0f);
        }

        static void DrawIconDelete(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRect(ImVec2(c.x - r * 0.50f, c.y - r * 0.20f), ImVec2(c.x + r * 0.50f, c.y + r * 0.75f), col, 1.5f, 0, 1.2f);
            draw->AddLine(ImVec2(c.x - r * 0.70f, c.y - r * 0.35f), ImVec2(c.x + r * 0.70f, c.y - r * 0.35f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - r * 0.25f, c.y - r * 0.65f), ImVec2(c.x + r * 0.25f, c.y - r * 0.65f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - 2.0f, c.y), ImVec2(c.x - 2.0f, c.y + r * 0.55f), col, 1.0f);
            draw->AddLine(ImVec2(c.x + 2.0f, c.y), ImVec2(c.x + 2.0f, c.y + r * 0.55f), col, 1.0f);
        }

        static void DrawIconMute(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRectFilled(ImVec2(c.x - r * 0.75f, c.y - r * 0.35f), ImVec2(c.x - r * 0.30f, c.y + r * 0.35f), col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.30f, c.y - r * 0.35f), ImVec2(c.x + r * 0.05f, c.y - r * 0.75f), ImVec2(c.x + r * 0.05f, c.y + r * 0.75f), col);
            draw->AddLine(ImVec2(c.x + r * 0.30f, c.y - r * 0.50f), ImVec2(c.x + r * 0.80f, c.y + r * 0.50f), col, 1.6f);
            draw->AddLine(ImVec2(c.x + r * 0.80f, c.y - r * 0.50f), ImVec2(c.x + r * 0.30f, c.y + r * 0.50f), col, 1.6f);
        }

        static void DrawIconSnap(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.75f, c.y), ImVec2(c.x + r * 0.75f, c.y), col, 1.5f);
            draw->AddLine(ImVec2(c.x, c.y - r * 0.75f), ImVec2(c.x, c.y + r * 0.75f), col, 1.5f);
            draw->AddCircleFilled(c, 1.5f, col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.85f, c.y), ImVec2(c.x - r * 0.50f, c.y - 2.5f), ImVec2(c.x - r * 0.50f, c.y + 2.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.85f, c.y), ImVec2(c.x + r * 0.50f, c.y - 2.5f), ImVec2(c.x + r * 0.50f, c.y + 2.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x, c.y - r * 0.85f), ImVec2(c.x - 2.5f, c.y - r * 0.50f), ImVec2(c.x + 2.5f, c.y - r * 0.50f), col);
            draw->AddTriangleFilled(ImVec2(c.x, c.y + r * 0.85f), ImVec2(c.x - 2.5f, c.y + r * 0.50f), ImVec2(c.x + 2.5f, c.y + r * 0.50f), col);
        }

        static void DrawIconPattern(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddCircleFilled(ImVec2(c.x - r * 0.40f, c.y + r * 0.45f), 2.2f, col);
            draw->AddCircleFilled(ImVec2(c.x + r * 0.40f, c.y + r * 0.25f), 2.2f, col);
            draw->AddLine(ImVec2(c.x - r * 0.22f, c.y + r * 0.45f), ImVec2(c.x - r * 0.22f, c.y - r * 0.55f), col, 1.4f);
            draw->AddLine(ImVec2(c.x + r * 0.58f, c.y + r * 0.25f), ImVec2(c.x + r * 0.58f, c.y - r * 0.75f), col, 1.4f);
            draw->AddLine(ImVec2(c.x - r * 0.22f, c.y - r * 0.55f), ImVec2(c.x + r * 0.58f, c.y - r * 0.75f), col, 2.2f);
        }

        static void DrawIconPsytrance(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddEllipse(c, ImVec2(r * 0.85f, r * 0.35f), col, 0.0f, 16, 1.5f);
            draw->AddCircle(ImVec2(c.x, c.y - r * 0.25f), r * 0.40f, col, 12, 1.4f);
            draw->AddCircleFilled(ImVec2(c.x - r * 0.45f, c.y), 1.2f, col);
            draw->AddCircleFilled(ImVec2(c.x, c.y + 1.0f), 1.4f, col);
            draw->AddCircleFilled(ImVec2(c.x + r * 0.45f, c.y), 1.2f, col);
        }

        static void DrawIconBezier(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p0(c.x - r * 0.75f, c.y + r * 0.55f);
            ImVec2 cp1(c.x - r * 0.25f, c.y - r * 0.75f);
            ImVec2 cp2(c.x + r * 0.25f, c.y + r * 0.75f);
            ImVec2 p1(c.x + r * 0.75f, c.y - r * 0.55f);
            draw->AddBezierCubic(p0, cp1, cp2, p1, col, 1.8f);
            draw->AddCircleFilled(p0, 2.0f, col);
            draw->AddCircleFilled(p1, 2.0f, col);
            draw->AddCircle(cp1, 1.5f, col);
            draw->AddCircle(cp2, 1.5f, col);
        }

        static void DrawIconAiSparkle(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddQuadFilled(
                ImVec2(c.x, c.y - r * 0.85f),
                ImVec2(c.x + r * 0.35f, c.y),
                ImVec2(c.x, c.y + r * 0.85f),
                ImVec2(c.x - r * 0.35f, c.y),
                col
            );
            draw->AddCircleFilled(ImVec2(c.x + r * 0.65f, c.y - r * 0.55f), 1.4f, col);
            draw->AddCircleFilled(ImVec2(c.x - r * 0.65f, c.y + r * 0.55f), 1.2f, col);
        }

        static void DrawIconLoop(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddCircle(c, r * 0.65f, col, 16, 1.5f);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.65f, c.y - 1.0f), ImVec2(c.x + r * 0.95f, c.y - 4.5f), ImVec2(c.x + r * 0.35f, c.y - 4.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.65f, c.y + 1.0f), ImVec2(c.x - r * 0.95f, c.y + 4.5f), ImVec2(c.x - r * 0.35f, c.y + 4.5f), col);
        }

        static void DrawIconPresets(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y - r * 0.5f), ImVec2(c.x + r * 0.65f, c.y - r * 0.5f), col, 1.8f);
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y), ImVec2(c.x + r * 0.35f, c.y), col, 1.8f);
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y + r * 0.5f), ImVec2(c.x + r * 0.65f, c.y + r * 0.5f), col, 1.8f);
        }

        static void DrawIconFit(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            float d = r * 0.70f;
            draw->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x - d + 3.5f, c.y - d), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x - d, c.y - d + 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y - d), ImVec2(c.x + d - 3.5f, c.y - d), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y - d), ImVec2(c.x + d, c.y - d + 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y + d), ImVec2(c.x - d + 3.5f, c.y + d), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y + d), ImVec2(c.x - d, c.y + d - 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y + d), ImVec2(c.x + d - 3.5f, c.y + d), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y + d), ImVec2(c.x + d, c.y + d - 3.5f), col, 1.5f);
        }
"""

# 2. Enhanced renderModernToolbar
toolbar_code = """        void renderModernToolbar(float bpm, float bar_len_sec) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImGuiIO& io = ImGui::GetIO();

            // ── 1. TOOLBAR CAPSULE CONTAINER (Pencil, Brush, Razor, Slip, Select, Del, Mute) ──
            struct ToolDef {
                PlaylistTool tool;
                const char* label;
                const char* tooltip;
            };
            ToolDef tools[] = {
                { TOOL_DRAW,   "Pencil", "Draw / Pencil Tool (P): Inserir ou mover clipes na grade" },
                { TOOL_PAINT,  "Brush",  "Paint / Brush Tool (B): Pintar múltiplos clipes consecutivos" },
                { TOOL_SLICE,  "Razor",  "Slice / Razor Tool (C): Cortar clipes na posição do cursor" },
                { TOOL_SLIP,   "Slip",   "Slip Tool: Deslocar conteúdo interno de áudio/MIDI sem mover o clipe" },
                { TOOL_SELECT, "Select", "Select Tool (S): Seleção retangular de clipes em área" },
                { TOOL_DELETE, "Del",    "Delete / Eraser Tool (D): Excluir clipes com clique" },
                { TOOL_MUTE,   "Mute",   "Mute Tool (T): Mutar / desmutar clipes individualmente" }
            };

            float tool_w = 40.0f;
            float capsule_h = 36.0f;
            float capsule_w = tool_w * 7.0f + 6.0f;
            ImVec2 cap_p0 = ImGui::GetCursorScreenPos();
            ImVec2 cap_p1(cap_p0.x + capsule_w, cap_p0.y + capsule_h);

            // Background & Border of the Tool Capsule
            draw->AddRectFilled(cap_p0, cap_p1, IM_COL32(12, 18, 26, 250), 7.0f);
            draw->AddRect(cap_p0, cap_p1, IM_COL32(30, 44, 60, 255), 7.0f, 0, 1.0f);

            for (int i = 0; i < 7; ++i) {
                ImVec2 b_p0(cap_p0.x + 3.0f + i * tool_w, cap_p0.y + 2.0f);
                ImVec2 b_p1(b_p0.x + tool_w - 1.0f, cap_p0.y + capsule_h - 2.0f);

                bool is_act = (current_tool == tools[i].tool);
                bool is_hov = (io.MousePos.x >= b_p0.x && io.MousePos.x <= b_p1.x && io.MousePos.y >= b_p0.y && io.MousePos.y <= b_p1.y);

                if (is_hov && io.MouseClicked[0]) {
                    current_tool = tools[i].tool;
                    is_act = true;
                }

                if (is_act) {
                    draw->AddRectFilled(b_p0, b_p1, IM_COL32(0, 180, 220, 50), 5.0f);
                    draw->AddRect(b_p0, b_p1, IM_COL32(0, 229, 255, 230), 5.0f, 0, 1.5f);
                    draw->AddLine(ImVec2(b_p0.x + 4.0f, b_p1.y - 1.0f), ImVec2(b_p1.x - 4.0f, b_p1.y - 1.0f), IM_COL32(0, 255, 255, 255), 2.0f);
                } else if (is_hov) {
                    draw->AddRectFilled(b_p0, b_p1, IM_COL32(28, 42, 58, 200), 5.0f);
                }

                ImU32 icon_col = is_act ? IM_COL32(0, 240, 255, 255) : (is_hov ? IM_COL32(230, 245, 255, 255) : IM_COL32(135, 155, 180, 255));
                ImU32 text_col = is_act ? IM_COL32(0, 240, 255, 255) : (is_hov ? IM_COL32(220, 240, 255, 255) : IM_COL32(110, 135, 160, 255));

                ImVec2 icon_center(b_p0.x + (tool_w - 1.0f) * 0.5f, b_p0.y + 11.0f);

                switch (tools[i].tool) {
                    case TOOL_DRAW:   DrawIconPencil(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_PAINT:  DrawIconBrush(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_SLICE:  DrawIconRazor(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_SLIP:   DrawIconSlip(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_SELECT: DrawIconSelect(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_DELETE: DrawIconDelete(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_MUTE:   DrawIconMute(draw, icon_center, 15.0f, icon_col); break;
                    default: break;
                }

                ImVec2 t_sz = ImGui::CalcTextSize(tools[i].label);
                draw->AddText(ImVec2(icon_center.x - t_sz.x * 0.5f, b_p0.y + 20.0f), text_col, tools[i].label);

                if (is_hov) {
                    ImGui::SetTooltip("%s", tools[i].tooltip);
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(cap_p1.x + 8.0f, cap_p0.y + 4.0f));

            // ── 2. SNAP PILL WITH HUD ICON ──
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.13f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.22f, 0.32f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImVec2 snap_p0 = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemWidth(126);
            const char* snap_names[] = { "  1/4 Beat", "  1/2 Beat", "  1 Beat", "  1/2 Bar", "  1 Bar", "  Free" };
            if (ImGui::Combo("##PlaylistSnap", &current_snap, snap_names, IM_ARRAYSIZE(snap_names))) {
            }
            DrawIconSnap(draw, ImVec2(snap_p0.x + 12.0f, snap_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));

            ImGui::SameLine(0, 6);

            // ── 3. PATTERN PICKER WITH NOTE ICON ──
            ImVec2 pat_p0 = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemWidth(130);
            std::vector<std::string> pat_names;
            for (const auto& p : g_clip_manager.global_patterns) {
                pat_names.push_back("  " + p.name);
            }
            if (pat_names.empty()) pat_names.push_back("  Pattern 1");
            int pat_idx = (std::clamp)(g_clip_manager.current_pattern_idx, 0, (int)pat_names.size() - 1);
            if (ImGui::BeginCombo("##PatPicker", pat_names[pat_idx].c_str())) {
                for (size_t p = 0; p < pat_names.size(); p++) {
                    bool is_sel = (p == (size_t)pat_idx);
                    if (ImGui::Selectable(pat_names[p].c_str(), is_sel)) {
                        g_clip_manager.current_pattern_idx = (int)p;
                    }
                    if (is_sel) ImGui::SetItemDefaultFocus();
                }
                if (ImGui::Selectable("  + Novo Padrao (Pattern)...")) {
                    Pattern np;
                    np.id = g_clip_manager.next_id++;
                    np.name = "Pattern " + std::to_string(g_clip_manager.global_patterns.size() + 1);
                    np.color = 0xFF00E5FF;
                    g_clip_manager.global_patterns.push_back(np);
                    g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                }
                ImGui::EndCombo();
            }
            DrawIconPattern(draw, ImVec2(pat_p0.x + 11.0f, pat_p0.y + 13.0f), 13.0f, IM_COL32(180, 130, 255, 240));

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 4. ARRANJADOR PSYTRANCE WITH SAUCER ICON ──
            ImVec2 psy_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.75f, 0.98f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.88f, 1.00f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.04f, 0.08f, 1.0f));
            if (ImGui::Button("    ARRANJADOR PSYTRANCE", ImVec2(0, 26))) {
                show_psy_arranger_modal = true;
            }
            ImGui::PopStyleColor(3);
            DrawIconPsytrance(draw, ImVec2(psy_p0.x + 13.0f, psy_p0.y + 13.0f), 14.0f, IM_COL32(0, 20, 35, 255));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Gerador de Estrutura Completa Psytrance (Intro, Build-ups, Drops, Breaks e Automacoes)");

            ImGui::SameLine(0, 5);

            // ── 5. + AUTO [v] WITH BEZIER ICON ──
            ImVec2 auto_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.16f, 0.24f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.22f, 0.32f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.88f, 1.0f, 1.0f));
            if (ImGui::Button("    + AUTO [v]", ImVec2(0, 26))) {
                ImGui::OpenPopup("AddAutoClipPopup");
            }
            ImGui::PopStyleColor(3);
            DrawIconBezier(draw, ImVec2(auto_p0.x + 12.0f, auto_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 240));

            if (ImGui::BeginPopup("AddAutoClipPopup")) {
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Criar Clipe de Automacao:");
                ImGui::Separator();
                if (ImGui::MenuItem("Master Volume Auto (Bezier)")) {
                    g_clip_manager.addAutomationClip(0, 0.0f, bar_len_sec * 8.0f, "Master Volume", "Master", 0);
                }
                if (ImGui::MenuItem("Filter Cutoff Sweep Auto")) {
                    g_clip_manager.addAutomationClip(1, 0.0f, bar_len_sec * 8.0f, "Cutoff Sweep", "Filter", 1);
                }
                if (ImGui::MenuItem("Reverb Wet Mix Auto")) {
                    g_clip_manager.addAutomationClip(2, 0.0f, bar_len_sec * 8.0f, "Reverb Wet", "Reverb", 2);
                }
                if (ImGui::MenuItem("Pitch Riser Auto (+12 st)")) {
                    g_clip_manager.addAutomationClip(3, 0.0f, bar_len_sec * 4.0f, "Pitch Riser", "Pitch", 3);
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 5);

            // ── 6. IA TOOLS [v] WITH SPARKLE ICON ──
            ImVec2 ai_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.48f, 0.16f, 0.85f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.22f, 0.95f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("    IA TOOLS [v]", ImVec2(0, 26))) {
                ImGui::OpenPopup("PlaylistAiToolsPopup");
            }
            ImGui::PopStyleColor(3);
            DrawIconAiSparkle(draw, ImVec2(ai_p0.x + 12.0f, ai_p0.y + 13.0f), 13.0f, IM_COL32(255, 230, 255, 255));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Suite de Inteligencia Artificial: Desconstrucao de Stems, Inpainting Generativo e Alinhamento Kick/Bass");

            if (ImGui::BeginPopup("PlaylistAiToolsPopup")) {
                ImGui::TextColored(ImVec4(0.85f, 0.45f, 1.0f, 1.0f), "SUITE DE INTELIGENCIA ARTIFICIAL");
                ImGui::Separator();
                if (ImGui::MenuItem("Desconstruir Musica (IA) - Separar Stems para Arranjo")) {
                    std::string chosen = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3;*.flac)\\0*.wav;*.mp3;*.flac\\0");
                    if (!chosen.empty()) {
                        g_cloud_stem_ui.DeconstructAnySong(chosen, g_clip_manager, ::timeline);
                    }
                }
                if (ImGui::MenuItem("AI Inpaint Arranjo (Variacoes de Build-up / Drops)")) {
                    show_timeline_inpainting_modal = true;
                }
                ImGui::Separator();
                std::string ghost_lbl = show_ghost_waveform ? "Desativar Ghost Align (Kick/Bass)" : "Ativar Ghost Align (Kick/Bass)";
                if (ImGui::MenuItem(ghost_lbl.c_str())) {
                    show_ghost_waveform = !show_ghost_waveform;
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 7. LOOP TOGGLE WITH REPEAT ICON ──
            ImVec2 loop_p0 = ImGui::GetCursorScreenPos();
            bool is_loop = ::timeline.loop_enabled;
            if (is_loop) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.75f, 0.90f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.05f, 0.10f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 1.0f));
            }
            if (ImGui::Button(is_loop ? "    LOOP: ON" : "    LOOP: OFF", ImVec2(0, 26))) {
                ::timeline.loop_enabled = !::timeline.loop_enabled;
            }
            ImGui::PopStyleColor(2);
            DrawIconLoop(draw, ImVec2(loop_p0.x + 12.0f, loop_p0.y + 13.0f), 13.0f, is_loop ? IM_COL32(0, 20, 35, 255) : IM_COL32(0, 229, 255, 230));

            ImGui::SameLine(0, 5);

            // ── 8. ARRANJOS PRESETS WITH STACK ICON ──
            ImVec2 arr_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 1.0f));
            if (ImGui::Button("    Arranjos [v]", ImVec2(0, 26))) {
                ImGui::OpenPopup("ArrangementPresetsPopup");
            }
            ImGui::PopStyleColor(2);
            DrawIconPresets(draw, ImVec2(arr_p0.x + 12.0f, arr_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 220));

            if (ImGui::BeginPopup("ArrangementPresetsPopup")) {
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "Presets de Estrutura de Musica:");
                ImGui::Separator();
                if (ImGui::MenuItem("Full Psytrance 138 (Padrao Comercial - 7min)")) {
                    KuroArranger::PsyArrangerConfig cfg;
                    cfg.bpm = 138.0f;
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                if (ImGui::MenuItem("Full-On Power Psy (142 BPM - Astrix/Adhana Style)")) {
                    KuroArranger::PsyArrangerConfig cfg;
                    cfg.bpm = 142.0f;
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                if (ImGui::MenuItem("Dark / Twilight Psy (148 BPM - Fast Peak)")) {
                    KuroArranger::PsyArrangerConfig cfg;
                    cfg.bpm = 148.0f;
                    cfg.subgenre = 2;
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Limpar Todos os Clipes da Playlist")) {
                    g_clip_manager.pushUndo();
                    g_clip_manager.reset();
                    ::timeline.clearSectionMarkers();
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 5);

            // ── 9. ZOOM CONTROLS WITH VECTOR ICONS ──
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.90f, 1.00f, 1.0f));

            if (ImGui::Button("-##ZoomOut", ImVec2(26, 26))) {
                playlist_zoom_x = (std::max)(12.0f, playlist_zoom_x * 0.80f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom Out (-)");

            ImGui::SameLine(0, 3);
            if (ImGui::Button("+##ZoomIn", ImVec2(26, 26))) {
                playlist_zoom_x = (std::min)(240.0f, playlist_zoom_x * 1.25f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom In (+)");

            ImGui::SameLine(0, 3);
            ImVec2 fit_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("    FIT", ImVec2(48, 26))) {
                float song_end = ::timeline.getSongEndSec();
                if (song_end > 1.0f) {
                    float avail_w = ImGui::GetContentRegionAvail().x - track_card_w - 40.0f;
                    if (avail_w > 100.0f) {
                        playlist_zoom_x = (avail_w / song_end) * bar_len_sec;
                        playlist_zoom_x = (std::clamp)(playlist_zoom_x, 12.0f, 240.0f);
                        playlist_scroll_x = 0.0f;
                    }
                }
            }
            DrawIconFit(draw, ImVec2(fit_p0.x + 11.0f, fit_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ajustar Zoom para Caber Todo o Arranjo (Fit)");

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
        }
"""

# 3. Enhanced renderClipsForTrack
clips_code = """        template<typename FSnap>
        void renderClipsForTrack(ImDrawList* draw, int t, float ty0, float ty1, float grid_start_x, float grid_view_w, float time_to_px, float bar_len_sec, ImU32 trk_color, float snap_sec, FSnap snap_time, bool is_hovered) {
            ImGuiIO& io = ImGui::GetIO();
            float trk_h = ty1 - ty0;

            // ── 1. CLIPS DE AUDIO COM FORMA DE ONDA ESTEREO REAL (WAVEFORMS) ──
            auto& a_clips = g_clip_manager.track_clips[t];
            for (int i = 0; i < (int)a_clips.size(); ++i) {
                auto& ac = a_clips[i];
                float cx0 = grid_start_x + (ac.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (ac.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(14, 22, 34, 235), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Barra de título do clipe de áudio
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(10, 16, 26, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(220, 245, 255, 255), ac.name.c_str());

                // Linha zero central da forma de onda estéreo
                float wave_top = ty0 + 17.0f;
                float wave_bot = ty1 - 3.0f;
                float wave_h = wave_bot - wave_top;
                float mid_y = wave_top + wave_h * 0.5f;

                draw->AddLine(ImVec2(draw_x0, mid_y), ImVec2(draw_x1, mid_y), IM_COL32(40, 75, 110, 180), 1.0f);

                const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->getStemBuffer(t) : nullptr;
                auto& ds = g_piano_synth.getDrumSample(t);

                float step_px = 2.0f;
                float sr = 44100.0f;

                for (float px = draw_x0; px < draw_x1; px += step_px) {
                    float clip_t = ((px - cx0) / time_to_px) + ac.source_offset_sec;
                    float amp_l = 0.0f;
                    float amp_r = 0.0f;

                    if (ds.loaded && !ds.sample_data.empty()) {
                        size_t frame_idx = (size_t)(clip_t * ds.sample_rate);
                        if (frame_idx < (size_t)ds.total_frames) {
                            if (ds.channels >= 2) {
                                amp_l = std::abs(ds.sample_data[frame_idx * ds.channels]);
                                amp_r = std::abs(ds.sample_data[frame_idx * ds.channels + 1]);
                            } else {
                                amp_l = amp_r = std::abs(ds.sample_data[frame_idx]);
                            }
                        }
                    } else if (p_stem_buf && !p_stem_buf->empty()) {
                        size_t s_idx = (size_t)(clip_t * sr);
                        if (s_idx < p_stem_buf->size()) {
                            amp_l = amp_r = std::abs((*p_stem_buf)[s_idx]);
                        }
                    } else {
                        // Forma de onda estéreo dinâmica com harmônicos e transiente
                        float rel_bar_t = fmodf(clip_t, bar_len_sec * 0.25f);
                        float env = std::exp(-rel_bar_t * 9.0f);
                        float osc = std::sin(rel_bar_t * 620.0f) * 0.6f + std::sin(rel_bar_t * 1280.0f) * 0.4f;
                        amp_l = std::clamp(std::abs(osc) * env * 0.95f + 0.04f, 0.03f, 0.95f);
                        amp_r = std::clamp(amp_l * 0.88f + std::sin(rel_bar_t * 400.0f) * 0.05f, 0.03f, 0.95f);
                    }

                    amp_l = std::clamp(amp_l, 0.02f, 0.98f);
                    amp_r = std::clamp(amp_r, 0.02f, 0.98f);

                    float h_l = amp_l * (wave_h * 0.46f);
                    float h_r = amp_r * (wave_h * 0.46f);

                    // Canal L em Ciano Elétrico
                    draw->AddLine(ImVec2(px, mid_y - h_l), ImVec2(px, mid_y - 0.5f), IM_COL32(0, 230, 255, 235), 1.5f);
                    // Canal R em Roxo Neon
                    draw->AddLine(ImVec2(px, mid_y + 0.5f), ImVec2(px, mid_y + h_r), IM_COL32(190, 110, 255, 235), 1.5f);
                }

                ImVec2 hit_p0(draw_x0, ty0 + 2.0f);
                ImVec2 hit_p1(draw_x1, ty1 - 2.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(hit_p0, hit_p1)) {
                    if (current_tool == TOOL_DELETE && ImGui::IsMouseClicked(0)) {
                        a_clips.erase(a_clips.begin() + i);
                        --i;
                        continue;
                    }
                    if (current_tool == TOOL_SLICE && ImGui::IsMouseClicked(0)) {
                        float cut_time = snap_time((io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px);
                        if (cut_time > ac.start_time_sec + 0.05f && cut_time < ac.start_time_sec + ac.length_sec - 0.05f) {
                            AudioClip ac2 = ac;
                            float first_len = cut_time - ac.start_time_sec;
                            ac2.start_time_sec = cut_time;
                            ac2.length_sec = ac.length_sec - first_len;
                            ac2.source_offset_sec = ac.source_offset_sec + first_len;
                            ac.length_sec = first_len;
                            a_clips.insert(a_clips.begin() + i + 1, ac2);
                            break;
                        }
                    }
                }
            }

            // ── 2. CLIPS DE PADRAO MIDI COM NOTAS VISIVEIS EM NEON CIANO ──
            auto& m_clips = g_clip_manager.track_midi_clips[t];
            for (int i = 0; i < (int)m_clips.size(); ++i) {
                auto& mc = m_clips[i];
                float cx0 = grid_start_x + (mc.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (mc.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(20, 16, 32, 240), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(14, 10, 24, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(245, 225, 255, 255), mc.name.c_str());

                const Pattern* p = nullptr;
                for (const auto& pat : g_clip_manager.global_patterns) {
                    if (pat.id == mc.pattern_id) { p = &pat; break; }
                }
                if (!p && mc.pattern_id >= 0 && mc.pattern_id < (int)g_clip_manager.global_patterns.size()) {
                    p = &g_clip_manager.global_patterns[mc.pattern_id];
                }

                if (p) {
                    std::vector<int> target_channels;
                    if (t >= 0 && t < MAX_TRACKS && !p->getChannelNotes(t).empty()) {
                        target_channels.push_back(t);
                    } else {
                        for (int ch = 0; ch < MAX_TRACKS; ch++) {
                            if (!p->getChannelNotes(ch).empty()) target_channels.push_back(ch);
                        }
                    }

                    float pat_len = 0.0f;
                    int min_pitch = 127;
                    int max_pitch = 0;
                    for (int ch : target_channels) {
                        for (const auto& n : p->getChannelNotes(ch)) {
                            if (n.start_time + n.duration > pat_len) pat_len = n.start_time + n.duration;
                            if (n.pitch < min_pitch) min_pitch = n.pitch;
                            if (n.pitch > max_pitch) max_pitch = n.pitch;
                        }
                    }
                    if (pat_len <= 0.001f) pat_len = bar_len_sec * 4.0f;
                    float bars = std::ceil(pat_len / bar_len_sec);
                    if (bars < 1.0f) bars = 1.0f;
                    pat_len = bars * bar_len_sec;
                    if (min_pitch > max_pitch) { min_pitch = 36; max_pitch = 60; }

                    float grid_inner_y0 = ty0 + 17.0f;
                    float grid_inner_h = (trk_h - 22.0f);

                    for (float loop_start = 0.0f; loop_start < mc.length_sec; loop_start += pat_len) {
                        if (loop_start > 0.0f) {
                            float div_x = cx0 + (loop_start * time_to_px);
                            if (div_x >= draw_x0 && div_x <= draw_x1) {
                                draw->AddLine(ImVec2(div_x, ty0 + 16.0f), ImVec2(div_x, ty1 - 2.0f), IM_COL32(255, 255, 255, 35), 1.0f);
                            }
                        }

                        for (int ch : target_channels) {
                            for (const auto& note : p->getChannelNotes(ch)) {
                                float note_start_rel = loop_start + note.start_time;
                                if (note_start_rel >= mc.length_sec) continue;
                                float note_dur_rel = note.duration;
                                if (note_start_rel + note_dur_rel > mc.length_sec) {
                                    note_dur_rel = mc.length_sec - note_start_rel;
                                }

                                float note_x0 = cx0 + (note_start_rel * time_to_px);
                                float note_x1 = note_x0 + (note_dur_rel * time_to_px);
                                if (note_x1 - note_x0 < 3.0f) note_x1 = note_x0 + 3.0f;

                                if (note_x1 < draw_x0 || note_x0 > draw_x1) continue;

                                float n_draw_x0 = (std::max)(note_x0, draw_x0);
                                float n_draw_x1 = (std::min)(note_x1, draw_x1);

                                float note_y = 0.0f;
                                if (max_pitch > min_pitch) {
                                    float pitch_norm = (float)(note.pitch - min_pitch) / (float)(max_pitch - min_pitch);
                                    note_y = (ty1 - 6.0f) - pitch_norm * (grid_inner_h - 8.0f);
                                } else {
                                    note_y = grid_inner_y0 + grid_inner_h * 0.5f;
                                }

                                // NOTAS MIDI BRILHANTES EM CIANO NEON COM NÚCLEO BRANCO
                                draw->AddRectFilled(ImVec2(n_draw_x0, note_y - 2.5f), ImVec2(n_draw_x1, note_y + 2.5f), IM_COL32(0, 235, 255, 245), 2.0f);
                                draw->AddRectFilled(ImVec2(n_draw_x0, note_y - 1.0f), ImVec2(n_draw_x1, note_y + 1.0f), IM_COL32(230, 255, 255, 255), 1.0f);
                            }
                        }
                    }
                }

                ImVec2 hit_p0(draw_x0, ty0 + 2.0f);
                ImVec2 hit_p1(draw_x1, ty1 - 2.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(hit_p0, hit_p1)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        g_clip_manager.current_pattern_idx = mc.pattern_id;
                        show_piano_roll = true;
                    }
                    if (current_tool == TOOL_DELETE && ImGui::IsMouseClicked(0)) {
                        m_clips.erase(m_clips.begin() + i);
                        --i;
                        continue;
                    }
                }
            }
"""

idx_tb_start = orig.find("        void renderModernToolbar(float bpm, float bar_len_sec) {")
assert idx_tb_start != -1, "tb_start not found"

idx_ruler = orig.find("        void renderRuler(ImDrawList* draw, ImVec2 grid_p0, float view_w, float ruler_h, float time_to_px, float bar_len_sec, float total_song_sec, bool is_hovered) {")
assert idx_ruler != -1, "ruler not found"

idx_clips_start = orig.find("        template<typename FSnap>\n        void renderClipsForTrack(ImDrawList* draw, int t, float ty0, float ty1, float grid_start_x, float grid_view_w, float time_to_px, float bar_len_sec, ImU32 trk_color, float snap_sec, FSnap snap_time, bool is_hovered) {")
assert idx_clips_start != -1, "clips_start not found"

idx_auto_clips = orig.find("            // 3. CLIPS DE AUTOMACAO COM CURVAS BEZIER E GRADIENTE", idx_clips_start)
assert idx_auto_clips != -1, "auto_clips not found"

part1 = orig[:idx_tb_start]
part2 = icons_code + "\n" + toolbar_code + "\n"
part3 = orig[idx_ruler:idx_clips_start]
part4 = clips_code + "\n"
part5 = orig[idx_auto_clips:]

final_code = part1 + part2 + part3 + part4 + part5

with open("src/ui/KuroPlaylistUI.h", "w", encoding="utf-8") as f:
    f.write(final_code)

print("Generated clean src/ui/KuroPlaylistUI.h successfully!")
print("Starts with:", repr(final_code[:40]))
print("Ends with:", repr(final_code[-40:]))
print("Total lines:", len(final_code.splitlines()))
