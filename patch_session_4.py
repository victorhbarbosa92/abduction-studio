import sys

with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

# 1. Add waveform_view_mode member variable
old_mode = 'int waveform_color_mode = 0; // Padr'
new_mode = 'int waveform_color_mode = 0; // Padrão TRAKTOR PRO RGB em Alta Resolução (Vector Mesh)\n        int waveform_view_mode = 0;  // 0 = PISTA ÚNICA SOBREPOSTA (Revolucionária com Grade Única), 1 = DUAL STACKED (Pistas Empilhadas)'

if 'int waveform_view_mode = 0;' not in content:
    idx = content.find('int waveform_color_mode = 0;')
    eol = content.find('\n', idx)
    content = content[:eol+1] + '        int waveform_view_mode = 0; // 0 = PISTA UNICA SOBREPOSTA COM GRADE MESTRE, 1 = DUAL EMPILHADAS REKORDBOX\n' + content[eol+1:]
    print("Added waveform_view_mode member variable.")

# 2. Add [GRID] capsule to renderRekordboxTopBar
old_pads = 'capsuleBtn("##TopCapsulePads", "PADS", false, IM_COL32(180, 80, 240, 255), IM_COL32(240, 215, 255, 255),'
if '##TopCapsuleGrid' not in content:
    idx_pads = content.find(old_pads)
    idx_end_pads = content.find('});', idx_pads) + 3
    grid_capsule_code = """
            // [GRID]
            capsuleBtn("##TopCapsuleGrid", "GRID", show_grid_edit_drawer, IM_COL32(0, 225, 245, 255), IM_COL32(230, 255, 255, 255),
                       "GRID: Abrir / Ocultar Editor Cirurgico de Beatgrid", [&]() {
                show_grid_edit_drawer = !show_grid_edit_drawer;
            });"""
    content = content[:idx_end_pads] + grid_capsule_code + content[idx_end_pads:]
    print("Added [GRID] capsule to TopBar.")

# 3. Replace renderGridEditDrawer
new_grid_drawer = '''        void renderGridEditDrawer(KuroAudio::DJEngine& engine, float w) {
            if (!show_grid_edit_drawer) return;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float h = 36.0f;

            // Background: Obsidian Glass com gradiente e borda ciano neon
            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(8, 12, 18, 245), 4.0f);
            dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(0, 210, 245, 180), 4.0f, 0, 1.2f);
            dl->AddLine(ImVec2(pos.x + 4.0f, pos.y + 1.0f), ImVec2(pos.x + w - 4.0f, pos.y + 1.0f), IM_COL32(0, 240, 255, 120), 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(pos.x + 10.0f, pos.y + 6.0f));

            // Titulo
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.90f, 1.0f), ICON_FA_SLIDERS " BEATGRID EDITOR");
            ImGui::SameLine(0, 10.0f);

            // Seletor de Deck: DECK 1 (Ciano) e DECK 2 (Ambar)
            bool sel_d1 = (grid_edit_deck == 0);
            ImGui::PushStyleColor(ImGuiCol_Button, sel_d1 ? ImVec4(0.00f, 0.75f, 0.70f, 0.95f) : ImVec4(0.08f, 0.12f, 0.18f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Text, sel_d1 ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.00f, 0.90f, 0.85f, 1.0f));
            if (ImGui::SmallButton(sel_d1 ? "[1] DECK 1" : "DECK 1")) grid_edit_deck = 0;
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 4.0f);

            bool sel_d2 = (grid_edit_deck == 1);
            ImGui::PushStyleColor(ImGuiCol_Button, sel_d2 ? ImVec4(0.95f, 0.55f, 0.05f, 0.95f) : ImVec4(0.08f, 0.12f, 0.18f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Text, sel_d2 ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.00f, 0.65f, 0.10f, 1.0f));
            if (ImGui::SmallButton(sel_d2 ? "[2] DECK 2" : "DECK 2")) grid_edit_deck = 1;
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 10.0f);

            int d_idx = (grid_edit_deck == 0) ? ddj_hardware.getActiveLeftDeck() : ddj_hardware.getActiveRightDeck();
            auto& deck = engine.decks[d_idx];
            auto& grid_eng = KuroAudio::DJGridEngine::getInstance();

            // SET 1.1 Marker (Botao Vermelho Âncora)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.88f, 0.14f, 0.20f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::SmallButton(ICON_FA_LOCATION_DOT " SET 1.1")) {
                grid_eng.setFirstBeatMarker(d_idx, deck.current_frame);
                deck.first_beat_frame = deck.current_frame;
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Define a agulha central como o Marco Zero (Beat 1.1 do Compasso)");
            ImGui::SameLine(0, 8.0f);

            // Nudge Cápsula
            ImGui::TextDisabled("NUDGE:");
            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton("< -1ms")) { grid_eng.nudgeGrid(d_idx, -1.0, deck.sample_rate); deck.first_beat_frame -= (0.001 * deck.sample_rate); }
            ImGui::SameLine(0, 2.0f);
            if (ImGui::SmallButton("+1ms >")) { grid_eng.nudgeGrid(d_idx, 1.0, deck.sample_rate); deck.first_beat_frame += (0.001 * deck.sample_rate); }
            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton("<< -10ms")) { grid_eng.nudgeGrid(d_idx, -10.0, deck.sample_rate); deck.first_beat_frame -= (0.010 * deck.sample_rate); }
            ImGui::SameLine(0, 2.0f);
            if (ImGui::SmallButton("+10ms >>")) { grid_eng.nudgeGrid(d_idx, 10.0, deck.sample_rate); deck.first_beat_frame += (0.010 * deck.sample_rate); }
            ImGui::SameLine(0, 8.0f);

            // BPM Adjust Cápsula
            ImGui::TextDisabled("BPM:");
            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton("-0.01")) { grid_eng.adjustBpm(d_idx, -0.01); deck.bpm = grid_eng.getGridData(d_idx).bpm; }
            ImGui::SameLine(0, 2.0f);
            if (ImGui::SmallButton("+0.01")) { grid_eng.adjustBpm(d_idx, 0.01); deck.bpm = grid_eng.getGridData(d_idx).bpm; }
            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton("/2")) { grid_eng.halveBpm(d_idx); deck.bpm = grid_eng.getGridData(d_idx).bpm; }
            ImGui::SameLine(0, 2.0f);
            if (ImGui::SmallButton("2x")) { grid_eng.doubleBpm(d_idx); deck.bpm = grid_eng.getGridData(d_idx).bpm; }
            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton(ICON_FA_HAND_POINTER " TAP")) {
                grid_eng.tapTempo(d_idx);
                deck.bpm = grid_eng.getGridData(d_idx).bpm;
            }
            ImGui::SameLine(0, 8.0f);

            // Telemetria do Deck Ativo
            char tele_buf[48];
            double cur_bpm_val = (deck.bpm > 20.0) ? deck.bpm : 128.0;
            double offset_ms = (deck.sample_rate > 1000.0) ? ((deck.first_beat_frame - deck.current_frame) / deck.sample_rate * 1000.0) : 0.0;
            snprintf(tele_buf, sizeof(tele_buf), "%.2f BPM [%.1fms]", cur_bpm_val, offset_ms);
            ImGui::TextColored(sel_d1 ? ImVec4(0.0f, 0.95f, 0.85f, 1.0f) : ImVec4(1.0f, 0.70f, 0.15f, 1.0f), "%s", tele_buf);
            ImGui::SameLine(0, 10.0f);

            // Metronomo Sonoro
            bool metro_on = grid_eng.isMetronomeEnabled(d_idx);
            ImGui::PushStyleColor(ImGuiCol_Button, metro_on ? ImVec4(0.0f, 0.85f, 0.40f, 0.95f) : ImVec4(0.10f, 0.14f, 0.20f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Text, metro_on ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.70f, 0.85f, 0.75f, 1.0f));
            if (ImGui::SmallButton(metro_on ? ICON_FA_BELL " METRO: ON" : ICON_FA_BELL_SLASH " METRO: OFF")) {
                grid_eng.setMetronomeEnabled(d_idx, !metro_on);
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Emite cliques precisos de metronomo no CUE");
            ImGui::SameLine(0, 6.0f);

            // Grid Lock
            bool locked = grid_eng.getGridData(d_idx).is_locked;
            ImGui::PushStyleColor(ImGuiCol_Button, locked ? ImVec4(1.0f, 0.45f, 0.0f, 0.95f) : ImVec4(0.10f, 0.14f, 0.20f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Text, locked ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
            if (ImGui::SmallButton(locked ? ICON_FA_LOCK " TRAVADA" : ICON_FA_LOCK_OPEN " LIVRE")) {
                grid_eng.toggleGridLock(d_idx);
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 12.0f);
            if (ImGui::SmallButton(ICON_FA_XMARK " FECHAR")) {
                show_grid_edit_drawer = false;
            }

            ImGui::Dummy(ImVec2(w, h + 2.0f));
        }'''

idx_g1 = content.find('void renderGridEditDrawer(KuroAudio::DJEngine& engine, float w) {')
idx_g2 = content.find('void renderMasterIsolatorDrawer(KuroAudio::DJEngine& engine, float w) {')
content = content[:idx_g1] + new_grid_drawer + '\n\n        ' + content[idx_g2:]
print("Replaced renderGridEditDrawer successfully.")

with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
    f.write(content)
print("File written successfully.")
