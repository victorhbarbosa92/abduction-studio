import os
import re

print("Reading src/ui/KuroPlaylistUI.h...")
with open("src/ui/KuroPlaylistUI.h", "r", encoding="utf-8") as f:
    orig_code = f.read()

# Load icon methods
with open("scratch/icon_methods.py", "r", encoding="utf-8") as f:
    py_txt = f.read()
    # extract icon_drawing_methods
    start_idx = py_txt.find("icon_drawing_methods = r'''") + len("icon_drawing_methods = r'''")
    end_idx = py_txt.rfind("'''")
    icon_methods = py_txt[start_idx:end_idx].strip()

# Insert icon methods right before renderModernToolbar
marker = "        void renderModernToolbar(float bpm, float bar_len_sec) {"

new_toolbar_and_clips = r'''
''' + icon_methods + r'''

        void renderModernToolbar(float bpm, float bar_len_sec) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 start_pos = ImGui::GetCursorScreenPos();
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
            int prev_snap = current_snap;
            if (ImGui::Combo("##PlaylistSnap", &current_snap, snap_names, IM_ARRAYSIZE(snap_names))) {
                // Snap changed
            }
            // Overlay Snap Magnet Icon
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
                    std::string chosen = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3;*.flac)\0*.wav;*.mp3;*.flac\0");
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
'''

print("New toolbar code ready.")
