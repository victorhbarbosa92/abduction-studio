import os

HEADER_PATH = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h"

with open(HEADER_PATH, "r", encoding="utf-8") as f:
    c = f.read()

# 1. Hotkeys in keyboard shortcut section
target_hotkeys = """                // 3. MIXER & CROSSFADER CORTES RÁPIDOS
                // -------------------------------------------------------------"""

replacement_hotkeys = """                // -------------------------------------------------------------
                // 4. BEAT FX & EFFECT HOTKEYS (CONTROLE DIRETO VIA TECLADO)
                // -------------------------------------------------------------
                // Beat FX Toggle / Hold: Tecla B
                if (ImGui::IsKeyPressed(ImGuiKey_B, false)) {
                    if (ImGui::GetIO().KeyShift) {
                        rekordbox_fx1_hold = true;
                        engine.beat_fx[0].enabled = true;
                    } else {
                        engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                    }
                }
                if (ImGui::IsKeyReleased(ImGuiKey_B) && rekordbox_fx1_hold) {
                    rekordbox_fx1_hold = false;
                    engine.beat_fx[0].enabled = false;
                }

                // TAP TEMPO: Tecla T
                if (ImGui::IsKeyPressed(ImGuiKey_T, false)) {
                    double now_s = ImGui::GetTime();
                    double tap_bpm = tap_trackers[0].registerTap(now_s);
                    if (tap_bpm > 30.0) {
                        engine.beat_fx[0].bpm = tap_bpm;
                        engine.beat_fx[0].auto_bpm = false;
                    }
                }

                // Fração de Batida (Beat < e Beat >): Teclas - e = (ou +)
                if (ImGui::IsKeyPressed(ImGuiKey_Minus, false)) {
                    rekordbox_fx1_beat_idx = std::max(0, rekordbox_fx1_beat_idx - 1);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Equal, false)) {
                    rekordbox_fx1_beat_idx = std::min(5, rekordbox_fx1_beat_idx + 1);
                }

                // FX Depth / Dry-Wet: Ctrl + Seta Cima / Seta Baixo
                if (ImGui::GetIO().KeyCtrl) {
                    if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
                        rekordbox_fx1_param = std::min(1.0f, rekordbox_fx1_param + 0.015f);
                    }
                    if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
                        rekordbox_fx1_param = std::max(0.0f, rekordbox_fx1_param - 0.015f);
                    }
                }

                // Disparadores rápidos de Efeitos (F2 a F6):
                if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
                    // F2: Vinyl Brake
                    int d = engine.getMasterDeckId();
                    engine.decks[d].is_stopping_vinyl = true;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
                    // F3: Slip Roll
                    int d = engine.getMasterDeckId();
                    engine.decks[d].triggerSlipRoll(0.5);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F4, false)) {
                    // F4: Echo
                    engine.beat_fx[0].selected_fx = 0;
                    engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
                    // F5: Reverb
                    engine.beat_fx[0].selected_fx = 1;
                    engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F6, false)) {
                    // F6: Spiral
                    engine.beat_fx[0].selected_fx = 4;
                    engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                }

                // -------------------------------------------------------------
                // 3. MIXER & CROSSFADER CORTES RÁPIDOS
                // -------------------------------------------------------------"""

if target_hotkeys in c:
    c = c.replace(target_hotkeys, replacement_hotkeys, 1)
    print("1. Added Beat FX hotkeys.")
else:
    print("Error: target_hotkeys not found!")

# 2. Functional TAP button in Beat FX bar
target_tap_btn = """                ImGui::SetCursorScreenPos(b_pos);
                char tbid[32]; snprintf(tbid, sizeof(tbid), "##TapBtn_%s", unit_name);
                if (ImGui::InvisibleButton(tbid, ImVec2(auto_tap_w, btn_h))) {
                    fx_unit.auto_bpm = false;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tap Tempo: Ajuste manual de tempo no ritmo da música");"""

replacement_tap_btn = """                int u_idx = (strcmp(unit_name, "FX 1") == 0) ? 0 : 1;
                if (tap_trackers[u_idx].flash_timer > 0.0f) {
                    tap_trackers[u_idx].flash_timer -= ImGui::GetIO().DeltaTime;
                    bg_c = IM_COL32(255, 170, 0, 255);
                    txt_c = IM_COL32(0, 0, 0, 255);
                }

                ImGui::SetCursorScreenPos(b_pos);
                char tbid[32]; snprintf(tbid, sizeof(tbid), "##TapBtn_%s", unit_name);
                if (ImGui::InvisibleButton(tbid, ImVec2(auto_tap_w, btn_h))) {
                    double now_s = ImGui::GetTime();
                    double tap_bpm = tap_trackers[u_idx].registerTap(now_s);
                    if (tap_bpm > 30.0) {
                        fx_unit.bpm = tap_bpm;
                        fx_unit.auto_bpm = false;
                    } else {
                        fx_unit.auto_bpm = false;
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tap Tempo: Clique no ritmo para calibrar o BPM manualmente (BPM Atual: %.1f)", fx_unit.bpm);"""

if target_tap_btn in c:
    c = c.replace(target_tap_btn, replacement_tap_btn, 1)
    print("2. Replaced TAP button with real tempo calculation and flash.")
else:
    print("Error: target_tap_btn not found!")

# 3. Sidebar USB devices section
target_sidebar_folders = """            for (const auto& f : folders) {
                bool is_active = (f.id == active_fid);"""

replacement_sidebar_usb = """            // 2b-1. NAVEGADOR DE DISPOSITIVOS / PEN DRIVES USB
            usb_scan_timer += ImGui::GetIO().DeltaTime;
            if (cached_usb_drives.empty() || usb_scan_timer > 3.5f) {
                usb_scan_timer = 0.0f;
                cached_usb_drives = KuroAudio::DJExportService::scanDrives();
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.0f), ICON_FA_USB " DISPOSITIVOS");

            if (cached_usb_drives.empty()) {
                ImGui::TextDisabled(" Nenhum pen drive detectado");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Dica: Conecte um pen drive formatado em FAT32 ou exFAT.");
                }
            } else {
                for (const auto& usb_dev : cached_usb_drives) {
                    bool is_usb_selected = (selected_usb_drive == usb_dev.path);
                    std::string usb_lbl = std::string(ICON_FA_USB " ") + usb_dev.label;
                    if (is_usb_selected) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.45f, 0.65f, 0.85f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.9f, 1.0f));
                    }
                    if (ImGui::Selectable(usb_lbl.c_str(), is_usb_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                        selected_usb_drive = usb_dev.path;
                        scanUSBTracks(usb_dev.path);
                    }
                    if (is_usb_selected) {
                        ImGui::PopStyleColor(2);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Unidade: %s\\nClique para navegar pelas músicas deste pen drive", usb_dev.path.c_str());
                    }
                }
            }
            ImGui::Separator();

            for (const auto& f : folders) {
                bool is_active = (selected_usb_drive.empty() && f.id == active_fid);"""

if target_sidebar_folders in c:
    c = c.replace(target_sidebar_folders, replacement_sidebar_usb, 1)
    print("3. Added USB Devices section to sidebar.")
else:
    print("Error: target_sidebar_folders not found!")

# 4. Library table track list substitution for USB
target_track_list = """            // Faixas da Pasta Selecionada
            auto track_list = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder(active_fid);"""

replacement_track_list = """            // Faixas da Pasta Selecionada ou Pen Drive Ativo
            std::vector<KuroAudio::DJLibraryTrack> track_list;
            if (!selected_usb_drive.empty()) {
                track_list = usb_scanned_tracks;
            } else {
                track_list = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder(active_fid);
            }

            if (!selected_usb_drive.empty()) {
                ImGui::SameLine(0, 15);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.45f, 0.65f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::SmallButton(ICON_FA_ARROW_LEFT " VOLTAR PARA COLEÇÃO LOCAL")) {
                    selected_usb_drive = "";
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 8);
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), ICON_FA_USB " PEN DRIVE: %s (%d faixas)", selected_usb_drive.c_str(), (int)usb_scanned_tracks.size());
            }"""

if target_track_list in c:
    c = c.replace(target_track_list, replacement_track_list, 1)
    print("4. Replaced track_list with USB selection support.")
else:
    print("Error: target_track_list not found!")

with open(HEADER_PATH, "w", encoding="utf-8") as f:
    f.write(c)

print("Patch part 2 completed.")
