import sys

def run():
    path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroPlaylistUI.h"
    with open(path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # 1. State variables
    content = "".join(lines)
    if "snap_to_zero_crossing" not in content:
        content = content.replace(
            "float playlist_scroll_x = 0.0f;\n        float playlist_scroll_y = 0.0f;",
            "float playlist_scroll_x = 0.0f;\n        float playlist_scroll_y = 0.0f;\n        bool snap_to_zero_crossing = false;\n        bool follow_playhead = true;"
        )

    # 2. Add DrawIconZeroCross and DrawIconFollow if missing
    if "DrawIconZeroCross" not in content:
        zc_icons = """        static void DrawIconZeroCross(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.55f, c.y - r * 0.70f), ImVec2(c.x - r * 0.55f, c.y + r * 0.70f), col, 2.0f);
            draw->AddLine(ImVec2(c.x - r * 0.55f, c.y), ImVec2(c.x + r * 0.35f, c.y), col, 1.8f);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.75f, c.y), ImVec2(c.x + r * 0.20f, c.y - r * 0.40f), ImVec2(c.x + r * 0.20f, c.y + r * 0.40f), col);
        }

        static void DrawIconFollow(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRectFilled(ImVec2(c.x - r * 0.65f, c.y - r * 0.70f), ImVec2(c.x - r * 0.18f, c.y + r * 0.70f), col, 1.0f);
            draw->AddRectFilled(ImVec2(c.x + r * 0.18f, c.y - r * 0.70f), ImVec2(c.x + r * 0.65f, c.y + r * 0.70f), col, 1.0f);
        }

"""
        content = content.replace("static void DrawIconFit(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {", zc_icons + "        static void DrawIconFit(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {")

    lines = content.splitlines(keepends=True)

    # 3. Locate renderModernToolbar
    tb_start = -1
    tb_end = -1
    for i, l in enumerate(lines):
        if "void renderModernToolbar(" in l:
            tb_start = i
            break

    for i in range(tb_start, len(lines)):
        if "void renderRuler(" in lines[i]:
            tb_end = i
            break

    print(f"Toolbar range: {tb_start} to {tb_end}")

    new_toolbar = """        void renderModernToolbar(float bpm, float bar_len_sec) {
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
                { TOOL_PAINT,  "Brush",  "Paint / Brush Tool (B): Pintar multiplos clipes consecutivos" },
                { TOOL_SLICE,  "Razor",  "Slice / Razor Tool (C): Cortar clipes na posicao do cursor" },
                { TOOL_SLIP,   "Slip",   "Slip Tool: Deslocar conteudo interno de audio/MIDI sem mover o clipe" },
                { TOOL_SELECT, "Select", "Select Tool (S): Selecao retangular de clipes em area" },
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

            // ── 2. SNAP PILL WITH HUD ICON (Matching Mockup) ──
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.13f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.22f, 0.32f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImVec2 snap_p0 = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemWidth(132);
            const char* snap_names[] = { "       1/4 Beat", "       1/2 Beat", "       1 Beat", "       1/2 Bar", "       1 Bar", "       Free" };
            if (ImGui::Combo("##PlaylistSnap", &current_snap, snap_names, IM_ARRAYSIZE(snap_names))) {
            }
            DrawIconSnap(draw, ImVec2(snap_p0.x + 12.0f, snap_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);

            ImGui::SameLine(0, 5);

            // ── 3. THREE MOCKUP TOOL BUTTONS: ZeroCross [|->], Fit [[]], Follow [||] ──
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.20f, 0.30f, 1.00f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            // 3A. Zero-Crossing Snap [ |-> ]
            ImVec2 zc_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##ZeroCrossBtn", ImVec2(26, 26))) {
                snap_to_zero_crossing = !snap_to_zero_crossing;
            }
            DrawIconZeroCross(draw, ImVec2(zc_p0.x + 13.0f, zc_p0.y + 13.0f), 13.0f, snap_to_zero_crossing ? IM_COL32(0, 240, 255, 255) : IM_COL32(130, 155, 185, 220));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zero-Crossing Snap: Alinhar cortes no cruzamento por zero");

            ImGui::SameLine(0, 4);

            // 3B. Fit Timeline to Screen [ [ ] ]
            ImVec2 fit_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##FitBtn", ImVec2(26, 26))) {
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
            DrawIconFit(draw, ImVec2(fit_p0.x + 13.0f, fit_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Fit to Screen: Enquadrar toda a musica na tela");

            ImGui::SameLine(0, 4);

            // 3C. Follow Playhead [ || ]
            ImVec2 fol_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##FollowBtn", ImVec2(26, 26))) {
                follow_playhead = !follow_playhead;
            }
            DrawIconFollow(draw, ImVec2(fol_p0.x + 13.0f, fol_p0.y + 13.0f), 13.0f, follow_playhead ? IM_COL32(0, 240, 255, 255) : IM_COL32(130, 155, 185, 220));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Follow Playhead: Seguir cursor durante reproducao");

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(1);

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 4. PATTERN PICKER WITH NOTE ICON ──
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.13f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.22f, 0.32f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImVec2 pat_p0 = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemWidth(142);
            std::vector<std::string> pat_names;
            for (const auto& p : g_clip_manager.global_patterns) {
                pat_names.push_back("       " + p.name);
            }
            if (pat_names.empty()) pat_names.push_back("       Pattern 1");
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
            DrawIconPattern(draw, ImVec2(pat_p0.x + 12.0f, pat_p0.y + 13.0f), 13.0f, IM_COL32(180, 130, 255, 240));

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 5. ARRANJADOR PSYTRANCE WITH SAUCER ICON ──
            ImVec2 psy_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.75f, 0.98f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.88f, 1.00f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.04f, 0.08f, 1.0f));
            if (ImGui::Button("       ARRANJADOR PSYTRANCE", ImVec2(0, 26))) {
                show_psy_arranger_modal = true;
            }
            ImGui::PopStyleColor(3);
            DrawIconPsytrance(draw, ImVec2(psy_p0.x + 13.0f, psy_p0.y + 13.0f), 14.0f, IM_COL32(0, 20, 35, 255));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Gerador de Estrutura Completa Psytrance (Intro, Build-ups, Drops, Breaks e Automacoes)");

            ImGui::SameLine(0, 5);

            // ── 6. + AUTO [v] WITH BEZIER ICON ──
            ImVec2 auto_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.16f, 0.24f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.22f, 0.32f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.88f, 1.0f, 1.0f));
            if (ImGui::Button("      + AUTO [v]", ImVec2(0, 26))) {
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

            // ── 7. IA TOOLS [v] WITH SPARKLE ICON ──
            ImVec2 ai_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.48f, 0.16f, 0.85f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.22f, 0.95f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("      IA TOOLS [v]", ImVec2(0, 26))) {
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

            // ── 8. LOOP TOGGLE WITH REPEAT ICON ──
            ImVec2 loop_p0 = ImGui::GetCursorScreenPos();
            bool is_loop = ::timeline.loop_enabled;
            if (is_loop) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.75f, 0.90f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.05f, 0.10f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 1.0f));
            }
            if (ImGui::Button(is_loop ? "      LOOP: ON" : "      LOOP: OFF", ImVec2(0, 26))) {
                ::timeline.loop_enabled = !::timeline.loop_enabled;
            }
            ImGui::PopStyleColor(2);
            DrawIconLoop(draw, ImVec2(loop_p0.x + 12.0f, loop_p0.y + 13.0f), 13.0f, is_loop ? IM_COL32(0, 20, 35, 255) : IM_COL32(0, 229, 255, 230));

            ImGui::SameLine(0, 5);

            // ── 9. ARRANJOS PRESETS WITH STACK ICON ──
            ImVec2 arr_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 1.0f));
            if (ImGui::Button("      Arranjos [v]", ImVec2(0, 26))) {
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
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                if (ImGui::MenuItem("Limpar Todos os Clipes da Playlist")) {
                    g_clip_manager.pushUndo();
                    g_clip_manager.reset();
                    ::timeline.clearSectionMarkers();
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 5);

            // ── 10. ZOOM CONTROLS [-] [+] ──
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

            ImGui::PopStyleColor(2);
        }

"""

    lines = lines[:tb_start] + [new_toolbar] + lines[tb_end:]

    # 4. Locate renderClipsForTrack
    rc_start = -1
    rc_end = -1
    for i, l in enumerate(lines):
        if "void renderClipsForTrack(" in l:
            # find template line right above
            rc_start = i - 1 if "template" in lines[i - 1] else i
            break

    for i in range(rc_start, len(lines)):
        if "void renderLaserPlayhead(" in lines[i]:
            rc_end = i
            break

    print(f"renderClipsForTrack range: {rc_start} to {rc_end}")

    new_render_clips = """        template<typename FSnap>
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

                // Container com cantos arredondados e borda neon suave
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(14, 20, 32, 245), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Header do clipe de áudio
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(10, 15, 24, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(220, 245, 255, 255), ac.name.c_str());

                // Configuração da Forma de Onda Estéreo (Dual Lane: L canal superior, R canal inferior)
                float wave_top = ty0 + 17.0f;
                float wave_bot = ty1 - 3.0f;
                float wave_h = wave_bot - wave_top;
                float mid_y = wave_top + wave_h * 0.5f;
                float cl_l = wave_top + wave_h * 0.25f; // Linha central Canal L
                float cl_r = wave_top + wave_h * 0.75f; // Linha central Canal R

                // Linha divisória de canais e baselines sutis
                draw->AddLine(ImVec2(draw_x0, mid_y), ImVec2(draw_x1, mid_y), IM_COL32(35, 55, 80, 160), 1.0f);
                draw->AddLine(ImVec2(draw_x0, cl_l), ImVec2(draw_x1, cl_l), IM_COL32(0, 200, 255, 45), 1.0f);
                draw->AddLine(ImVec2(draw_x0, cl_r), ImVec2(draw_x1, cl_r), IM_COL32(180, 110, 255, 45), 1.0f);

                const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->getStemBuffer(t) : nullptr;
                auto& ds = g_piano_synth.getDrumSample(t);

                float step_px = 2.0f;
                float sr = 44100.0f;
                float beat_sec = bar_len_sec * 0.25f;

                for (float px = draw_x0; px < draw_x1; px += step_px) {
                    float clip_t = ((px - cx0) / time_to_px) + ac.source_offset_sec;
                    float amp_l = 0.0f;
                    float amp_r = 0.0f;

                    if (ds.loaded && !ds.sample_data.empty() && ds.total_frames > 0) {
                        float sample_dur = (float)ds.total_frames / (float)ds.sample_rate;
                        float loop_clip_t = (sample_dur < beat_sec) ? fmodf(clip_t, beat_sec) : fmodf(clip_t, sample_dur);
                        size_t frame_idx = (size_t)(loop_clip_t * (float)ds.sample_rate);
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
                        // Modelagem de onda estéreo densa, contínua e orgânica (estilo WAV master real de DAW)
                        float beat_dur = bar_len_sec * 0.25f;
                        float sixteenth = beat_dur * 0.25f;
                        float t_in_beat = fmodf(clip_t, beat_dur);
                        float t_in_16 = fmodf(clip_t, sixteenth);

                        float transient = std::exp(-t_in_16 * 28.0f);
                        float beat_punch = std::exp(-t_in_beat * 10.0f);
                        float base_body = 0.42f + 0.26f * std::sin(clip_t * 3.14159f / bar_len_sec);
                        float osc = std::sin(clip_t * 640.0f) * 0.22f + std::sin(clip_t * 1380.0f) * 0.16f;

                        float raw_amp = base_body + beat_punch * 0.38f + transient * 0.22f + std::abs(osc);
                        amp_l = std::clamp(raw_amp, 0.08f, 0.94f);
                        amp_r = std::clamp(raw_amp * 0.92f + std::sin(clip_t * 320.0f) * 0.04f, 0.08f, 0.94f);
                    }

                    amp_l = std::clamp(amp_l, 0.06f, 0.95f);
                    amp_r = std::clamp(amp_r, 0.06f, 0.95f);

                    float max_h = wave_h * 0.22f;
                    float h_l = amp_l * max_h;
                    float h_r = amp_r * max_h;

                    // Cores estéreo: Gradiente Roxo Neon -> Ciano Elétrico (idêntico à referência do usuário)
                    float norm_pos = std::clamp((px - cx0) / (cx1 - cx0), 0.0f, 1.0f);
                    int r_val = (int)(150.0f * (1.0f - norm_pos * 0.85f));
                    int g_val = (int)(110.0f + norm_pos * 135.0f);
                    int b_val = 255;
                    ImU32 col_l = (t == 0) ? IM_COL32(r_val, g_val, b_val, 245) : IM_COL32(0, 235, 255, 245);
                    ImU32 col_r = (t == 0) ? IM_COL32(r_val, (int)(g_val * 0.92f), b_val, 245) : IM_COL32(0, 225, 250, 245);

                    // Desenha simetricamente em torno do baseline de cada canal
                    draw->AddLine(ImVec2(px, cl_l - h_l), ImVec2(px, cl_l + h_l), col_l, 1.8f);
                    draw->AddLine(ImVec2(px, cl_r - h_r), ImVec2(px, cl_r + h_r), col_r, 1.8f);
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

                // Container estilo Modern DAW: Fundo roxo profundo com contorno neon
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(26, 18, 44, 245), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Header do clipe MIDI
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(16, 12, 28, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(245, 230, 255, 255), mc.name.c_str());

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

                    // Garante uma extensao de afinacao confortavel (pelo menos 1 oitava)
                    int pitch_span = max_pitch - min_pitch;
                    if (pitch_span < 12) {
                        int mid_p = (min_pitch + max_pitch) / 2;
                        min_pitch = mid_p - 6;
                        max_pitch = mid_p + 6;
                    }

                    float grid_inner_y0 = ty0 + 17.0f;
                    float grid_inner_h = (trk_h - 22.0f);

                    for (float loop_start = 0.0f; loop_start < mc.length_sec; loop_start += pat_len) {
                        // Linha divisoria vertical suave de cada compasso dentro do clipe
                        for (float bar_div = loop_start; bar_div < loop_start + pat_len && bar_div < mc.length_sec; bar_div += bar_len_sec) {
                            if (bar_div > 0.0f) {
                                float div_x = cx0 + (bar_div * time_to_px);
                                if (div_x >= draw_x0 && div_x <= draw_x1) {
                                    draw->AddLine(ImVec2(div_x, ty0 + 17.0f), ImVec2(div_x, ty1 - 2.0f), IM_COL32(255, 255, 255, 22), 1.0f);
                                }
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
                                if (note_x1 - note_x0 < 4.0f) note_x1 = note_x0 + 4.0f;

                                if (note_x1 < draw_x0 || note_x0 > draw_x1) continue;

                                float n_draw_x0 = (std::max)(note_x0, draw_x0);
                                float n_draw_x1 = (std::min)(note_x1, draw_x1);

                                float pitch_norm = (float)(note.pitch - min_pitch) / (float)(max_pitch - min_pitch);
                                pitch_norm = std::clamp(pitch_norm, 0.06f, 0.94f);
                                float note_y = (ty1 - 6.0f) - pitch_norm * (grid_inner_h - 8.0f);

                                // NOTAS MIDI BRILHANTES EM CIANO NEON COM BRILHO AURA E NUCLEO BRANCO
                                // 1. Brilho Aura Ciano
                                draw->AddRectFilled(ImVec2(n_draw_x0 - 0.5f, note_y - 3.0f), ImVec2(n_draw_x1 + 0.5f, note_y + 3.0f), IM_COL32(0, 220, 255, 65), 2.5f);
                                // 2. Corpo Neon Ciano
                                draw->AddRectFilled(ImVec2(n_draw_x0, note_y - 2.0f), ImVec2(n_draw_x1, note_y + 2.0f), IM_COL32(0, 240, 255, 245), 2.0f);
                                // 3. Nucleo Branco Puro
                                draw->AddLine(ImVec2(n_draw_x0 + 1.0f, note_y), ImVec2(n_draw_x1 - 1.0f, note_y), IM_COL32(240, 255, 255, 255), 1.2f);
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

            // 3. CLIPS DE AUTOMACAO COM CURVAS BEZIER E GRADIENTE
            auto& auto_clips = g_clip_manager.track_auto_clips[t];
            for (int i = 0; i < (int)auto_clips.size(); ++i) {
                auto& ac = auto_clips[i];
                float cx0 = grid_start_x + (ac.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (ac.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(10, 24, 32, 235), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), ac.color, 5.0f);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(8, 18, 26, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(180, 240, 255, 255), ac.name.c_str());

                float auto_inner_y0 = ty0 + 18.0f;
                float auto_inner_h = (trk_h - 22.0f);

                if (ac.points.size() >= 2) {
                    for (size_t pt = 0; pt < ac.points.size() - 1; ++pt) {
                        auto& p0 = ac.points[pt];
                        auto& p1 = ac.points[pt + 1];

                        float px0 = cx0 + (p0.time_rel_sec * time_to_px);
                        float px1 = cx0 + (p1.time_rel_sec * time_to_px);
                        float py0 = (ty1 - 4.0f) - p0.value * auto_inner_h;
                        float py1 = (ty1 - 4.0f) - p1.value * auto_inner_h;

                        float cp_x = (px0 + px1) * 0.5f;
                        float cp_y = (py0 + py1) * 0.5f + p0.tension * 30.0f;

                        int segs = 16;
                        for (int s = 0; s < segs; ++s) {
                            float t_a = (float)s / (float)segs;
                            float t_b = (float)(s + 1) / (float)segs;

                            float xa = (1.0f - t_a) * (1.0f - t_a) * px0 + 2.0f * (1.0f - t_a) * t_a * cp_x + t_a * t_a * px1;
                            float ya = (1.0f - t_a) * (1.0f - t_a) * py0 + 2.0f * (1.0f - t_a) * t_a * cp_y + t_a * t_a * py1;
                            float xb = (1.0f - t_b) * (1.0f - t_b) * px0 + 2.0f * (1.0f - t_b) * t_b * cp_x + t_b * t_b * px1;
                            float yb = (1.0f - t_b) * (1.0f - t_b) * py0 + 2.0f * (1.0f - t_b) * t_b * cp_y + t_b * t_b * py1;

                            if (xb >= draw_x0 && xa <= draw_x1) {
                                draw->AddLine(ImVec2(xa, ya), ImVec2(xb, yb), ac.color, 2.0f);
                                draw->AddLine(ImVec2(xa, ya), ImVec2(xa, ty1 - 3.0f), IM_COL32(0, 229, 255, 20), 1.0f);
                            }
                        }

                        if (px0 >= draw_x0 && px0 <= draw_x1) {
                            draw->AddCircleFilled(ImVec2(px0, py0), 3.5f, IM_COL32(255, 255, 255, 255));
                            draw->AddCircle(ImVec2(px0, py0), 4.5f, ac.color, 12, 1.5f);
                        }
                    }
                    auto& plast = ac.points.back();
                    float plast_x = cx0 + (plast.time_rel_sec * time_to_px);
                    float plast_y = (ty1 - 4.0f) - plast.value * auto_inner_h;
                    if (plast_x >= draw_x0 && plast_x <= draw_x1) {
                        draw->AddCircleFilled(ImVec2(plast_x, plast_y), 3.5f, IM_COL32(255, 255, 255, 255));
                        draw->AddCircle(ImVec2(plast_x, plast_y), 4.5f, ac.color, 12, 1.5f);
                    }
                }
            }
        }
"""

    lines = lines[:rc_start] + [new_render_clips] + lines[rc_end:]

    with open(path, "w", encoding="utf-8") as f:
        f.writelines(lines)

    print("KuroPlaylistUI.h updated with complete DAW toolbar, waveforms, and glowing MIDI notes!")

if __name__ == "__main__":
    run()
