import sys
import re

# Read current file
with open("src/ui/KuroDJStudioUI.h", "r", encoding="utf-8") as f:
    code = f.read()

# Definitions of Rekordbox components to add before renderDeckPanel
rekordbox_functions = r'''
        // =========================================================================
        // REKORDBOX 7 OFICIAL 1:1 VISUAL ENGINE (PERFORMANCE MODE 2-DECKS)
        // =========================================================================

        void renderRekordboxTopBar(KuroAudio::DJEngine& engine, bool* p_open) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = KuroUI::UIScaleManager::Scale(30.0f);

            // Fundo Sleek Rekordbox #111317 com linha divisoria inferior #1d222b
            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(17, 19, 23, 255));
            dl->AddLine(ImVec2(pos.x, pos.y + h - 1.0f), ImVec2(pos.x + w, pos.y + h - 1.0f), IM_COL32(29, 34, 43, 255), 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(pos.x + 6.0f, pos.y + 4.0f));

            // 1. PERFORMANCE Dropdown
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.11f, 0.13f, 0.17f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
            if (ImGui::SmallButton("PERFORMANCE  v")) {
                rekordbox_app_mode = (rekordbox_app_mode == 0 ? 1 : 0);
                if (rekordbox_app_mode == 1) {
                    show_playlist_drawer = true;
                    show_export_modal = true;
                } else {
                    show_export_modal = false;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar modo PERFORMANCE / EXPORT");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 8.0f);

            // 2. Botoes Rapidos de Icones: [=] [FX] [Lck] [Pads] [Mixer] [Rec]
            auto quickIconBtn = [&](const char* id, const char* label, bool active, ImU32 act_col, const char* tooltip, auto on_click) {
                ImVec2 b_pos = ImGui::GetCursorScreenPos();
                float bw = 24.0f, bh = 22.0f;
                dl->AddRectFilled(b_pos, ImVec2(b_pos.x + bw, b_pos.y + bh), active ? act_col : IM_COL32(22, 26, 33, 255), 3.0f);
                dl->AddRect(b_pos, ImVec2(b_pos.x + bw, b_pos.y + bh), active ? IM_COL32(255, 255, 255, 180) : IM_COL32(38, 46, 58, 200), 3.0f, 0, 1.0f);
                ImVec2 t_sz = ImGui::CalcTextSize(label);
                dl->AddText(ImVec2(b_pos.x + (bw - t_sz.x) * 0.5f, b_pos.y + (bh - t_sz.y) * 0.5f), active ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 175, 195, 240), label);

                ImGui::InvisibleButton(id, ImVec2(bw, bh));
                if (ImGui::IsItemClicked(0)) on_click();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
                ImGui::SameLine(0, 3.0f);
            };

            // Layout
            quickIconBtn("##TopBtnLayout", "=", false, 0, "Alternar visualizacoes", [&]() {
                dj_layout_mode = (dj_layout_mode == 0 ? 1 : 0);
            });

            // FX Bar Toggle
            quickIconBtn("##TopBtnFX", "FX", show_rekordbox_fx_bar, IM_COL32(0, 132, 255, 255), "Exibir / Ocultar Barra de Beat FX e CFX", [&]() {
                show_rekordbox_fx_bar = !show_rekordbox_fx_bar;
            });

            // Lock / Quantize
            quickIconBtn("##TopBtnLock", "[*]", engine.decks[0].quantize_active, IM_COL32(0, 200, 100, 255), "Quantize Master Lock", [&]() {
                engine.decks[0].quantize_active = !engine.decks[0].quantize_active;
                engine.decks[1].quantize_active = engine.decks[0].quantize_active;
            });

            // Biblioteca / Gaveta
            quickIconBtn("##TopBtnPads", "#", show_playlist_drawer, IM_COL32(0, 132, 255, 255), "Alternar Gaveta da Biblioteca", [&]() {
                show_playlist_drawer = !show_playlist_drawer;
            });

            // Record Master
            bool rec_on = engine.isRecording();
            quickIconBtn("##TopBtnRec", "O", rec_on, IM_COL32(255, 45, 55, 255), rec_on ? "Gravando Master ao vivo! Clique para salvar" : "Gravar Master WAV Lossless", [&]() {
                engine.toggleRecording();
            });

            // 3. PRO DJ LINK (Centro)
            float center_btn_x = pos.x + w * 0.5f - 30.0f;
            if (ImGui::GetCursorPosX() < center_btn_x) {
                ImGui::SetCursorScreenPos(ImVec2(center_btn_x, pos.y + 4.0f));
                ImVec2 lk_pos = ImGui::GetCursorScreenPos();
                dl->AddRectFilled(lk_pos, ImVec2(lk_pos.x + 60.0f, lk_pos.y + 22.0f), IM_COL32(20, 25, 34, 255), 11.0f);
                dl->AddRect(lk_pos, ImVec2(lk_pos.x + 60.0f, lk_pos.y + 22.0f), IM_COL32(40, 52, 70, 200), 11.0f, 0, 1.0f);
                dl->AddCircleFilled(ImVec2(lk_pos.x + 12.0f, lk_pos.y + 11.0f), 3.5f, IM_COL32(0, 240, 120, 255));
                dl->AddText(ImVec2(lk_pos.x + 22.0f, lk_pos.y + 4.0f), IM_COL32(200, 220, 245, 240), "LINK");
                ImGui::InvisibleButton("##TopProLink", ImVec2(60.0f, 22.0f));
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("PRO DJ LINK: Conexao direta CDJ-3000 / DDJ / Kuro Engine");
            }

            // 4. Lado Direito: PAD, MIDI, (i), Free, Gear, Cloud, Bateria, Relogio
            float r_tools_w = 340.0f;
            ImGui::SetCursorScreenPos(ImVec2(pos.x + w - r_tools_w, pos.y + 4.0f));

            // PAD
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 1.0f));
            ImGui::SmallButton("PAD");
            ImGui::SameLine(0, 4.0f);

            // MIDI / DDJ-200
            bool hw_ok = ddj_hardware.isConnected();
            ImGui::PushStyleColor(ImGuiCol_Text, hw_ok ? ImVec4(0.0f, 1.0f, 0.5f, 1.0f) : ImVec4(0.6f, 0.7f, 0.8f, 1.0f));
            if (ImGui::SmallButton("MIDI")) {
                ddj_hardware.autoConnect(&engine);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pioneer Hardware: %s", hw_ok ? "Conectado" : "Desconectado (clique para escanear)");
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton(" i ")) {
                show_library_modal = true;
            }

            ImGui::SameLine(0, 4.0f);
            // Badge FREE dourada Rekordbox
            dl->AddRectFilled(ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + 36.0f, ImGui::GetCursorScreenPos().y + 20.0f), IM_COL32(212, 175, 55, 255), 3.0f);
            dl->AddText(ImVec2(ImGui::GetCursorScreenPos().x + 5.0f, ImGui::GetCursorScreenPos().y + 3.0f), IM_COL32(0, 0, 0, 255), "Free");
            ImGui::Dummy(ImVec2(36.0f, 20.0f));

            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton(" * ")) {
                show_camelot_radar_modal = !show_camelot_radar_modal;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Configuracoes & Camelot Wheel");

            ImGui::SameLine(0, 4.0f);
            if (ImGui::SmallButton("Cloud")) {
                show_search_modal = true;
            }

            ImGui::PopStyleColor();

            // Relogio Digital
            ImGui::SameLine(0, 8.0f);
            time_t rawtime;
            time(&rawtime);
            struct tm* timeinfo = localtime(&rawtime);
            char time_buf[16] = "18:20";
            if (timeinfo) strftime(time_buf, sizeof(time_buf), "%H:%M", timeinfo);
            ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f), "%s", time_buf);

            // Botao Fechar
            if (p_open) {
                ImGui::SameLine(0, 8.0f);
                if (ImGui::SmallButton("X")) {
                    *p_open = false;
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));
        }

        void renderRekordboxFXBar(KuroAudio::DJEngine& engine, float w) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float h = KuroUI::UIScaleManager::Scale(30.0f);

            // Fundo escuro do rack de FX #14171d
            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(20, 23, 29, 255));
            dl->AddLine(ImVec2(pos.x, pos.y + h - 1.0f), ImVec2(pos.x + w, pos.y + h - 1.0f), IM_COL32(32, 38, 48, 255), 1.0f);

            float col_fx_w = (w - 16.0f) * 0.44f;

            // --- FX 1 (Esquerda) ---
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 6.0f, pos.y + 4.0f));
            ImGui::TextColored(ImVec4(0.7f, 0.75f, 0.85f, 1.0f), "FX1");
            ImGui::SameLine(0, 6.0f);

            // Routing 1 2 S
            auto chBtn = [&](const char* lbl, bool* val) {
                if (*val) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.52f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.9f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.7f, 0.8f, 1));
                }
                if (ImGui::SmallButton(lbl)) *val = !*val;
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 2.0f);
            };

            chBtn("1##fx1_1", &rekordbox_fx1_ch[0]);
            chBtn("2##fx1_2", &rekordbox_fx1_ch[1]);
            chBtn("S##fx1_s", &rekordbox_fx1_ch[2]);

            // Seletor Beat FX
            const char* fx1_list[] = { "ENIGMA JET", "DELAY", "DOWN ECHO", "VINYL BRAKE" };
            ImGui::PushItemWidth(95.0f);
            ImGui::Combo("##FX1Combo", &rekordbox_fx1_type, fx1_list, 4);
            ImGui::PopItemWidth();
            ImGui::SameLine(0, 4.0f);

            // Knobs / Indicadores
            ImGui::TextColored(ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "< +4 >");
            ImGui::SameLine(0, 4.0f);
            ImGui::TextColored(ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "< 4 >");
            ImGui::SameLine(0, 4.0f);
            ImGui::TextColored(ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "< 1 >");
            ImGui::SameLine(0, 6.0f);

            // Botao AUTO e TAP
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.52f, 1.0f, 1.0f));
            ImGui::SmallButton("AUTO##FX1");
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 2.0f);
            ImGui::SmallButton("TAP##FX1");

            // --- CFX (Centro) ---
            float cfx_x = pos.x + w * 0.44f;
            ImGui::SetCursorScreenPos(ImVec2(cfx_x, pos.y + 4.0f));
            ImGui::TextColored(ImVec4(0.7f, 0.75f, 0.85f, 1.0f), "CFX");
            ImGui::SameLine(0, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.52f, 1.0f, 1.0f));
            ImGui::SmallButton("FILTER");
            ImGui::PopStyleColor();

            // --- FX 2 (Direita) ---
            float fx2_x = pos.x + w - col_fx_w;
            ImGui::SetCursorScreenPos(ImVec2(fx2_x, pos.y + 4.0f));
            ImGui::TextColored(ImVec4(0.7f, 0.75f, 0.85f, 1.0f), "FX2");
            ImGui::SameLine(0, 6.0f);

            chBtn("1##fx2_1", &rekordbox_fx2_ch[0]);
            chBtn("2##fx2_2", &rekordbox_fx2_ch[1]);
            chBtn("S##fx2_s", &rekordbox_fx2_ch[2]);

            const char* fx2_list[] = { "SPIRAL", "ECHO", "DOWN ECHO", "VINYL BRAKE" };
            ImGui::PushItemWidth(95.0f);
            ImGui::Combo("##FX2Combo", &rekordbox_fx2_type, fx2_list, 4);
            ImGui::PopItemWidth();
            ImGui::SameLine(0, 4.0f);

            ImGui::TextColored(ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "< 1 >");
            ImGui::SameLine(0, 4.0f);
            ImGui::TextColored(ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "< 3/4 >");
            ImGui::SameLine(0, 6.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.52f, 1.0f, 1.0f));
            ImGui::SmallButton("AUTO##FX2");
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 2.0f);
            ImGui::SmallButton("TAP##FX2");

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));
        }

        void renderRekordboxJogwheel(KuroAudio::DJDeck& deck, ImVec2 top_left, float size, ImVec4 theme_col) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float radius = size * 0.5f;
            ImVec2 center = ImVec2(top_left.x + radius, top_left.y + radius);

            // 1. Chassi externo / aro metalico escovado estilo Pioneer CDJ-3000 / DDJ
            dl->AddCircleFilled(center, radius, IM_COL32(14, 17, 24, 255), 64);
            dl->AddCircle(center, radius, IM_COL32(36, 44, 58, 255), 64, 1.5f);
            dl->AddCircle(center, radius - 2.0f, IM_COL32(22, 28, 38, 255), 64, 1.0f);

            // Chanfros / ranhuras radiais do rim tátil externo (grip de pitch bend)
            int num_ticks = 48;
            for (int t = 0; t < num_ticks; t++) {
                float ang = (float)t / (float)num_ticks * 2.0f * (float)M_PI;
                ImVec2 p_in = ImVec2(center.x + std::cos(ang) * (radius - 5.0f), center.y + std::sin(ang) * (radius - 5.0f));
                ImVec2 p_out = ImVec2(center.x + std::cos(ang) * (radius - 2.0f), center.y + std::sin(ang) * (radius - 2.0f));
                dl->AddLine(p_in, p_out, IM_COL32(50, 62, 80, 180), 1.0f);
            }

            // 2. Disco de Vinil / Acrilico Central (com textura de aneis concentricos)
            float r_platter = radius - 8.0f;
            dl->AddCircleFilled(center, r_platter, IM_COL32(10, 12, 16, 255), 64);
            dl->AddCircle(center, r_platter * 0.90f, IM_COL32(25, 30, 40, 140), 64, 1.0f);
            dl->AddCircle(center, r_platter * 0.78f, IM_COL32(20, 26, 36, 120), 64, 1.0f);
            dl->AddCircle(center, r_platter * 0.65f, IM_COL32(25, 32, 44, 140), 64, 1.0f);

            // 3. DISPLAY LCD CENTRAL ON-JOG (Exatamente como Rekordbox CDJ-3000 / DDJ-1000)
            float hub_r = r_platter * 0.54f;

            // Moldura do LCD (anel prateado/chumbo chanfrado)
            dl->AddCircleFilled(center, hub_r + 3.0f, IM_COL32(30, 38, 50, 255), 48);
            dl->AddCircle(center, hub_r + 3.0f, IM_COL32(65, 80, 105, 255), 48, 1.2f);
            dl->AddCircleFilled(center, hub_r, IM_COL32(5, 6, 9, 255), 48);

            // Anel externo de rotacao e posicao da agulha (Blue Arc Ring)
            float track_ring_r = hub_r - 4.0f;
            dl->AddCircle(center, track_ring_r, IM_COL32(18, 30, 52, 255), 48, 2.5f);

            // Arco azul de rotacao do prato (Playhead Rotation)
            float rot_ang = deck.jog_visual_angle;
            float arc_len = 1.35f * (float)M_PI;
            int arc_segs = 24;
            for (int s = 0; s < arc_segs; s++) {
                float a0 = rot_ang - arc_len + (float)s / (float)arc_segs * arc_len;
                float a1 = rot_ang - arc_len + (float)(s + 1) / (float)arc_segs * arc_len;
                float alpha = (float)(s + 1) / (float)arc_segs;
                ImU32 c_arc = IM_COL32(0, 160, 255, (int)(255.0f * alpha));
                ImVec2 p0 = ImVec2(center.x + std::cos(a0) * track_ring_r, center.y + std::sin(a0) * track_ring_r);
                ImVec2 p1 = ImVec2(center.x + std::cos(a1) * track_ring_r, center.y + std::sin(a1) * track_ring_r);
                dl->AddLine(p0, p1, c_arc, 2.8f);
            }

            // Indicador da agulha do Rekordbox: Marcador Vermelho/Laranja na cabeca do arco
            ImVec2 needle_head = ImVec2(center.x + std::cos(rot_ang) * track_ring_r, center.y + std::sin(rot_ang) * track_ring_r);
            dl->AddCircleFilled(needle_head, 3.5f, IM_COL32(255, 45, 55, 255), 16);
            ImVec2 needle_in = ImVec2(center.x + std::cos(rot_ang) * (track_ring_r - 5.0f), center.y + std::sin(rot_ang) * (track_ring_r - 5.0f));
            dl->AddLine(needle_head, needle_in, IM_COL32(255, 50, 60, 255), 2.0f);

            // Telemetria Digital no centro do LCD: BPM em destaque (ex: 142.00)
            double eff_bpm = (deck.bpm > 20.0) ? (deck.bpm * (1.0 + deck.pitch_percent * 0.01)) : 126.00;
            char bpm_main[16];
            char bpm_dec[8];
            int bpm_int_part = (int)eff_bpm;
            int bpm_frac_part = (int)std::round((eff_bpm - (double)bpm_int_part) * 100.0);
            if (bpm_frac_part >= 100) { bpm_int_part++; bpm_frac_part = 0; }
            snprintf(bpm_main, sizeof(bpm_main), "%d", bpm_int_part);
            snprintf(bpm_dec, sizeof(bpm_dec), ".%02d", bpm_frac_part);

            ImVec2 sz_m = ImGui::CalcTextSize(bpm_main);
            ImVec2 sz_d = ImGui::CalcTextSize(bpm_dec);
            float total_bpm_w = sz_m.x + sz_d.x;

            float bpm_y = center.y - 12.0f;
            dl->AddText(ImVec2(center.x - total_bpm_w * 0.5f, bpm_y), IM_COL32(255, 255, 255, 255), bpm_main);
            dl->AddText(ImVec2(center.x - total_bpm_w * 0.5f + sz_m.x, bpm_y + 1.0f), IM_COL32(220, 230, 245, 240), bpm_dec);

            // Linha inferior: Pitch % e Range (ex: "0.0% WIDE")
            char pitch_str[32];
            snprintf(pitch_str, sizeof(pitch_str), "%+.1f%%", deck.pitch_percent);
            ImVec2 sz_p = ImGui::CalcTextSize(pitch_str);
            ImVec2 sz_w = ImGui::CalcTextSize(" WIDE");
            float p_tot_w = sz_p.x + sz_w.x;
            float pitch_y = center.y + 4.0f;

            dl->AddText(ImVec2(center.x - p_tot_w * 0.5f, pitch_y), IM_COL32(220, 230, 245, 240), pitch_str);
            dl->AddText(ImVec2(center.x - p_tot_w * 0.5f + sz_p.x, pitch_y), IM_COL32(255, 45, 55, 255), " WIDE");

            // Indicador de Slip / Motor Stop
            if (deck.jog_touch || deck.motor_stopped) {
                dl->AddCircle(center, hub_r - 1.0f, IM_COL32(255, 30, 60, 240), 48, 2.0f);
            }

            // Interacao de Mouse do Jogwheel
            char j_id[32];
            snprintf(j_id, sizeof(j_id), "##RekordboxJog_%d", deck.deck_id);
            ImGui::SetCursorScreenPos(top_left);
            ImGui::InvisibleButton(j_id, ImVec2(size, size));

            bool hovered = ImGui::IsItemHovered();
            bool active  = ImGui::IsItemActive();

            if (active) {
                ImVec2 mouse = ImGui::GetMousePos();
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                float dist_sq = (mouse.x - center.x) * (mouse.x - center.x) + (mouse.y - center.y) * (mouse.y - center.y);
                float dist = std::sqrt(dist_sq);
                float ang_delta = (delta.x * (mouse.y - center.y) - delta.y * (mouse.x - center.x)) / (dist_sq + 120.0f);

                if (dist < r_platter * 0.85f) {
                    if (deck.vinyl_mode) {
                        deck.jog_touch = true;
                        deck.motor_stopped = true;
                    }
                    float rot_delta = ang_delta * 0.16f;
                    deck.target_scratch_velocity = std::clamp(rot_delta * 3.2f, -3.0f, 3.0f);
                    deck.scratch_active_ticks = 4;
                    deck.jog_visual_angle += rot_delta;

                    if (!deck.is_playing) {
                        double frames_delta = (double)rot_delta * (deck.sample_rate * 0.10);
                        if (ImGui::GetIO().KeyShift) frames_delta *= 4.0;
                        deck.current_frame = std::clamp(deck.current_frame + frames_delta, 0.0, (double)deck.buffer_l.size());
                    }
                } else {
                    float rot_delta = ang_delta * 0.035f;
                    deck.jog_pitch_bend = std::clamp(deck.jog_pitch_bend + rot_delta, -0.35f, 0.35f);
                    deck.jog_visual_angle += rot_delta;
                }
            } else if (hovered && ImGui::IsMouseReleased(0)) {
                deck.jog_touch = false;
                deck.motor_stopped = false;
            } else if (!active && !hovered && deck.jog_touch) {
                deck.jog_touch = false;
                deck.motor_stopped = false;
            }
        }

        void renderPioneerRoundButton(const char* label, ImVec2 center, float radius, ImU32 ring_col, ImU32 bg_col, bool is_active, std::function<void()> on_click) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 top_left = ImVec2(center.x - radius, center.y - radius);
            
            char btn_id[32];
            snprintf(btn_id, sizeof(btn_id), "##PioBtn_%s", label);
            ImGui::SetCursorScreenPos(top_left);
            ImGui::InvisibleButton(btn_id, ImVec2(radius * 2.0f, radius * 2.0f));
            
            bool clicked = ImGui::IsItemClicked(0);
            if (clicked && on_click) on_click();
            
            dl->AddCircleFilled(center, radius, bg_col, 36);
            dl->AddCircle(center, radius, ring_col, 36, is_active ? 2.5f : 1.8f);
            
            if (std::strcmp(label, "CUE") == 0) {
                ImVec2 sz = ImGui::CalcTextSize("CUE");
                dl->AddText(ImVec2(center.x - sz.x * 0.5f, center.y - sz.y * 0.5f), ring_col, "CUE");
            } else if (std::strcmp(label, "PLAY") == 0) {
                float tri_h = radius * 0.38f;
                ImVec2 p0 = ImVec2(center.x - tri_h * 0.5f, center.y - tri_h);
                ImVec2 p1 = ImVec2(center.x + tri_h * 0.8f, center.y);
                ImVec2 p2 = ImVec2(center.x - tri_h * 0.5f, center.y + tri_h);
                dl->AddTriangleFilled(p0, p1, p2, ring_col);
            }
        }

        void renderRekordboxPads(KuroAudio::DJDeck& deck, ImVec2 pos, float w, float h) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float pad_gap_x = 3.0f;
            float pad_gap_y = 3.0f;
            float pad_w = (w - pad_gap_x * 3.0f) * 0.25f;
            float pad_h = (h - 24.0f - pad_gap_y) * 0.5f;

            const char* letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };

            for (int p = 0; p < 8; p++) {
                int col = p % 4;
                int row = p / 4;
                ImVec2 p_min = ImVec2(pos.x + (float)col * (pad_w + pad_gap_x), pos.y + (float)row * (pad_h + pad_gap_y));
                ImVec2 p_max = ImVec2(p_min.x + pad_w, p_min.y + pad_h);

                bool active = deck.hot_cues[p].active;
                ImU32 base_col = active ? deck.hot_cues[p].color_rgba : IM_COL32(18, 22, 30, 240);
                ImU32 border_col = active ? IM_COL32(255, 255, 255, 200) : IM_COL32(36, 46, 62, 180);

                char p_id[32];
                snprintf(p_id, sizeof(p_id), "##RekPad_%d_%d", deck.deck_id, p);
                ImGui::SetCursorScreenPos(p_min);
                ImGui::InvisibleButton(p_id, ImVec2(pad_w, pad_h));

                if (ImGui::IsItemClicked(0)) {
                    deck.triggerHotCue(p);
                } else if (ImGui::IsItemClicked(1)) {
                    deck.clearHotCue(p);
                }

                dl->AddRectFilled(p_min, p_max, base_col, 3.0f);
                dl->AddRect(p_min, p_max, border_col, 3.0f, 0, 1.0f);

                dl->AddText(ImVec2(p_min.x + 4.0f, p_min.y + 2.0f), active ? IM_COL32(0, 0, 0, 255) : IM_COL32(160, 175, 195, 240), letters[p]);

                if (active) {
                    int t_sec = (int)deck.hot_cues[p].time_sec;
                    int m = t_sec / 60;
                    int s = t_sec % 60;
                    char t_str[16];
                    snprintf(t_str, sizeof(t_str), "%02d:%02d", m, s);
                    dl->AddText(ImVec2(p_min.x + 4.0f, p_min.y + pad_h - 13.0f), IM_COL32(0, 0, 0, 240), t_str);
                }
            }

            // Dropdown HOT CUE v no rodape dos pads
            ImVec2 drop_pos = ImVec2(pos.x, pos.y + pad_h * 2.0f + pad_gap_y + 2.0f);
            dl->AddRectFilled(drop_pos, ImVec2(drop_pos.x + w, drop_pos.y + 18.0f), IM_COL32(12, 16, 22, 255), 2.0f);
            dl->AddRect(drop_pos, ImVec2(drop_pos.x + w, drop_pos.y + 18.0f), IM_COL32(32, 42, 58, 220), 2.0f, 0, 1.0f);
            dl->AddText(ImVec2(drop_pos.x + 6.0f, drop_pos.y + 2.0f), IM_COL32(200, 215, 235, 240), "HOT CUE");
            dl->AddText(ImVec2(drop_pos.x + w - 14.0f, drop_pos.y + 2.0f), IM_COL32(150, 165, 185, 240), "v");
        }

        void renderRekordboxLoopBox(KuroAudio::DJDeck& deck, ImVec2 pos, float w) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float box_h = 24.0f;
            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + box_h), IM_COL32(6, 8, 12, 255), 3.0f);
            dl->AddRect(pos, ImVec2(pos.x + w, pos.y + box_h), deck.loop_active ? IM_COL32(255, 170, 0, 255) : IM_COL32(32, 42, 58, 220), 3.0f, 0, 1.0f);

            char loop_txt[16];
            if (deck.loop_beats >= 1.0) snprintf(loop_txt, sizeof(loop_txt), "%.0f", deck.loop_beats);
            else snprintf(loop_txt, sizeof(loop_txt), "1/%.0f", 1.0 / deck.loop_beats);
            ImVec2 sz = ImGui::CalcTextSize(loop_txt);
            dl->AddText(ImVec2(pos.x + (w - sz.x) * 0.5f, pos.y + 4.0f), deck.loop_active ? IM_COL32(255, 200, 40, 255) : IM_COL32(255, 255, 255, 255), loop_txt);

            float btn_y = pos.y + box_h + 3.0f;
            ImGui::SetCursorScreenPos(ImVec2(pos.x, btn_y));
            char b_halve[32]; snprintf(b_halve, sizeof(b_halve), "<##LpDec_%d", deck.deck_id);
            if (ImGui::Button(b_halve, ImVec2((w - 2.0f) * 0.5f, 18.0f))) {
                deck.loopHalve();
            }
            ImGui::SameLine(0, 2.0f);
            char b_doub[32]; snprintf(b_doub, sizeof(b_doub), ">##LpInc_%d", deck.deck_id);
            if (ImGui::Button(b_doub, ImVec2((w - 2.0f) * 0.5f, 18.0f))) {
                deck.loopDouble();
            }

            float btn_y2 = btn_y + 20.0f;
            ImGui::SetCursorScreenPos(ImVec2(pos.x, btn_y2));
            char b_au[32]; snprintf(b_au, sizeof(b_au), "AU##LpAu_%d", deck.deck_id);
            if (ImGui::Button(b_au, ImVec2((w - 2.0f) * 0.5f, 18.0f))) {
                deck.toggleLoop();
            }
            ImGui::SameLine(0, 2.0f);
            char b_ma[32]; snprintf(b_ma, sizeof(b_ma), "MA##LpMa_%d", deck.deck_id);
            if (ImGui::Button(b_ma, ImVec2((w - 2.0f) * 0.5f, 18.0f))) {
                deck.setLoopIn();
            }
        }

        void renderRekordboxTransportSection(KuroAudio::DJDeck& deck, ImVec2 pos, float w, float h) {
            renderRekordboxLoopBox(deck, pos, w);

            float mod_y = pos.y + 68.0f;
            ImGui::SetCursorScreenPos(ImVec2(pos.x, mod_y));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            float mb_w = (w - 6.0f) * 0.25f;

            ImGui::SmallButton("INT");
            ImGui::SameLine(0, 2.0f);

            if (deck.slip_mode) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
            }
            if (ImGui::Button("SLIP", ImVec2(mb_w, 18.0f))) deck.slip_mode = !deck.slip_mode;
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 2.0f);

            if (deck.quantize_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.85f, 0.35f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
            }
            if (ImGui::Button("Q", ImVec2(mb_w, 18.0f))) deck.quantize_active = !deck.quantize_active;
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 2.0f);

            if (deck.key_sync_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.52f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
            }
            if (ImGui::Button("MT", ImVec2(mb_w, 18.0f))) deck.key_sync_active = !deck.key_sync_active;
            ImGui::PopStyleColor(2);

            ImGui::PopStyleVar();

            float r_btn = std::clamp(w * 0.28f, 18.0f, 26.0f);
            float btn_cx = pos.x + w * 0.5f;
            float cue_cy = mod_y + 32.0f + r_btn;
            float play_cy = cue_cy + r_btn * 2.0f + 12.0f;

            renderPioneerRoundButton("CUE", ImVec2(btn_cx, cue_cy), r_btn, IM_COL32(255, 140, 0, 255), IM_COL32(18, 14, 8, 255), !deck.is_playing.load(), [&]() {
                deck.jumpToCue();
            });

            renderPioneerRoundButton("PLAY", ImVec2(btn_cx, play_cy), r_btn, IM_COL32(0, 240, 90, 255), IM_COL32(8, 18, 12, 255), deck.is_playing.load(), [&]() {
                deck.togglePlay();
            });
        }

        void renderRekordboxPitchFader(KuroAudio::DJDeck& deck, ImVec2 pos, float h) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float fader_w = 18.0f;
            dl->AddRectFilled(pos, ImVec2(pos.x + fader_w, pos.y + h), IM_COL32(8, 10, 14, 255), 2.0f);
            dl->AddRect(pos, ImVec2(pos.x + fader_w, pos.y + h), IM_COL32(26, 32, 42, 200), 2.0f, 0, 1.0f);

            float mid_x = pos.x + fader_w * 0.5f;
            dl->AddLine(ImVec2(mid_x, pos.y + 4.0f), ImVec2(mid_x, pos.y + h - 4.0f), IM_COL32(40, 50, 65, 255), 2.0f);

            float center_y = pos.y + h * 0.5f;
            dl->AddLine(ImVec2(pos.x + 2.0f, center_y), ImVec2(pos.x + fader_w - 2.0f, center_y), IM_COL32(255, 255, 255, 180), 1.5f);

            float norm = (deck.pitch_percent - (-deck.pitch_range)) / (deck.pitch_range * 2.0f);
            norm = std::clamp(norm, 0.0f, 1.0f);
            float handle_y = pos.y + norm * (h - 16.0f) + 8.0f;

            dl->AddRectFilled(ImVec2(pos.x + 1.0f, handle_y - 6.0f), ImVec2(pos.x + fader_w - 1.0f, handle_y + 6.0f), IM_COL32(50, 60, 75, 255), 2.0f);
            dl->AddLine(ImVec2(pos.x + 2.0f, handle_y), ImVec2(pos.x + fader_w - 2.0f, handle_y), IM_COL32(255, 255, 255, 255), 1.5f);

            char pf_id[32]; snprintf(pf_id, sizeof(pf_id), "##RekPF_%d", deck.deck_id);
            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton(pf_id, ImVec2(fader_w, h));
            if (ImGui::IsItemActive()) {
                float my = ImGui::GetMousePos().y;
                float new_norm = (my - pos.y) / h;
                new_norm = std::clamp(new_norm, 0.0f, 1.0f);
                deck.pitch_percent = -deck.pitch_range + new_norm * (deck.pitch_range * 2.0f);
                deck.sync_active = false;
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                deck.pitch_percent = 0.0f;
            }
        }

        void renderRekordboxDeckPanel(KuroAudio::DJDeck& deck, int deck_slot, ImVec4 theme_col, KuroAudio::DJEngine& engine) {
            ImGui::PushID(deck.deck_id);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float total_h = ImGui::GetContentRegionAvail().y;

            dl->AddRectFilled(p_min, ImVec2(p_min.x + w, p_min.y + total_h), IM_COL32(14, 16, 21, 255), 4.0f);
            dl->AddRect(p_min, ImVec2(p_min.x + w, p_min.y + total_h), IM_COL32(28, 34, 44, 200), 4.0f, 0, 1.0f);

            int d_num = deck_slot + 1;

            dl->AddRectFilled(ImVec2(p_min.x + 4.0f, p_min.y + 4.0f), ImVec2(p_min.x + 22.0f, p_min.y + 22.0f), IM_COL32(22, 28, 38, 255), 2.0f);
            char num_str[4]; snprintf(num_str, sizeof(num_str), "%d", d_num);
            dl->AddText(ImVec2(p_min.x + 9.0f, p_min.y + 5.0f), IM_COL32(255, 255, 255, 255), num_str);

            ImVec2 vinyl_c = ImVec2(p_min.x + 32.0f, p_min.y + 13.0f);
            dl->AddCircleFilled(vinyl_c, 8.0f, IM_COL32(10, 12, 16, 255));
            dl->AddCircle(vinyl_c, 8.0f, IM_COL32(80, 95, 115, 255), 16, 1.0f);
            dl->AddCircle(vinyl_c, 4.0f, IM_COL32(50, 60, 75, 255), 16, 1.0f);
            dl->AddCircleFilled(vinyl_c, 2.0f, IM_COL32(200, 220, 255, 255));

            std::string d_title = deck.track_title.empty() ? "Nenhuma Faixa Carregada" : deck.track_title;
            ImVec2 title_pos = ImVec2(p_min.x + 46.0f, p_min.y + 5.0f);
            dl->AddText(title_pos, IM_COL32(255, 255, 255, 255), d_title.c_str());

            float r_ctrl_w = 175.0f;
            float rx = p_min.x + w - r_ctrl_w;
            ImGui::SetCursorScreenPos(ImVec2(rx, p_min.y + 3.0f));

            if (deck.key_sync_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
            }
            if (ImGui::SmallButton("KEY SYNC")) engine.toggleKeySync(deck.deck_id);
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 2.0f);

            if (deck.sync_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.52f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
            }
            if (ImGui::SmallButton("BEAT SYNC")) engine.toggleSync(deck.deck_id, (deck_slot == 0 ? 1 : 0));
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 2.0f);

            bool is_m = engine.isMasterDeck(deck.deck_id);
            if (is_m) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.72f, 0.05f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
            }
            if (ImGui::SmallButton("MASTER")) engine.setMasterDeck(deck.deck_id);
            ImGui::PopStyleColor(2);

            float telem_y = p_min.y + 26.0f;
            double eff_b = (deck.bpm > 20.0) ? (deck.bpm * (1.0 + deck.pitch_percent * 0.01)) : 126.0;
            char telem_str[64];
            snprintf(telem_str, sizeof(telem_str), "%.2f  %s", eff_b, deck.getEffectiveKey().c_str());
            dl->AddText(ImVec2(p_min.x + 46.0f, telem_y), IM_COL32(180, 195, 215, 230), telem_str);

            double rem_sec = deck.getRemainingTimeSec();
            double elap_sec = deck.getElapsedTimeSec();
            int r_m = (int)rem_sec / 60, r_s = (int)rem_sec % 60, r_ms = (int)((rem_sec - std::floor(rem_sec)) * 10);
            int e_m = (int)elap_sec / 60, e_s = (int)elap_sec % 60, e_ms = (int)((elap_sec - std::floor(elap_sec)) * 10);
            char times_buf[64];
            snprintf(times_buf, sizeof(times_buf), "-%02d:%02d.%d  %02d:%02d.%d", r_m, r_s, r_ms, e_m, e_s, e_ms);
            dl->AddText(ImVec2(p_min.x + 130.0f, telem_y), IM_COL32(240, 245, 255, 240), times_buf);

            // 2. MINI OVERVIEW WAVEFORM STRIP
            float over_y = p_min.y + 44.0f;
            float over_h = 24.0f;
            ImGui::SetCursorScreenPos(ImVec2(p_min.x + 4.0f, over_y));
            renderMiniOverviewStrip(deck, w - 8.0f, over_h, theme_col);

            // 3. CORPO INFERIOR DO DECK
            float body_y = over_y + over_h + 8.0f;
            float body_h = total_h - (body_y - p_min.y) - 6.0f;

            float pads_w = w * 0.35f;
            float trans_w = w * 0.25f;
            float jog_w = w - pads_w - trans_w;

            if (deck_slot == 0) {
                // DECK 1: PADS (Esq) | TRANSPORT (Centro) | JOGWHEEL (Dir)
                ImVec2 pads_pos = ImVec2(p_min.x + 6.0f, body_y);
                renderRekordboxPads(deck, pads_pos, pads_w - 10.0f, body_h);

                ImVec2 trans_pos = ImVec2(p_min.x + pads_w + 4.0f, body_y);
                renderRekordboxTransportSection(deck, trans_pos, trans_w - 8.0f, body_h);

                ImVec2 jog_pos = ImVec2(p_min.x + pads_w + trans_w + 4.0f, body_y);
                float j_sz = std::min(jog_w - 24.0f, body_h - 10.0f);
                renderRekordboxJogwheel(deck, ImVec2(jog_pos.x + (jog_w - j_sz - 30.0f) * 0.5f, jog_pos.y + (body_h - j_sz) * 0.5f), j_sz, theme_col);

                float pf_x = p_min.x + w - 24.0f;
                renderRekordboxPitchFader(deck, ImVec2(pf_x, body_y + 10.0f), body_h - 20.0f);
            } else {
                // DECK 2: JOGWHEEL (Esq) | TRANSPORT (Centro) | PADS (Dir)
                float j_sz = std::min(jog_w - 24.0f, body_h - 10.0f);
                ImVec2 jog_pos = ImVec2(p_min.x + 28.0f, body_y);
                renderRekordboxJogwheel(deck, ImVec2(jog_pos.x + (jog_w - j_sz - 30.0f) * 0.5f, jog_pos.y + (body_h - j_sz) * 0.5f), j_sz, theme_col);

                float pf_x = p_min.x + 4.0f;
                renderRekordboxPitchFader(deck, ImVec2(pf_x, body_y + 10.0f), body_h - 20.0f);

                ImVec2 trans_pos = ImVec2(p_min.x + jog_w + 4.0f, body_y);
                renderRekordboxTransportSection(deck, trans_pos, trans_w - 8.0f, body_h);

                ImVec2 pads_pos = ImVec2(p_min.x + jog_w + trans_w + 4.0f, body_y);
                renderRekordboxPads(deck, pads_pos, pads_w - 10.0f, body_h);
            }

            ImGui::PopID();
        }

        void renderRekordboxCenterMixer(KuroAudio::DJEngine& engine) {
            float w = ImGui::GetContentRegionAvail().x;
            float h = ImGui::GetContentRegionAvail().y;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(14, 16, 21, 255), 4.0f);
            dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(28, 34, 44, 200), 4.0f, 0, 1.0f);

            int left_id = ddj_hardware.getActiveLeftDeck();
            int right_id = ddj_hardware.getActiveRightDeck();
            KuroAudio::DJDeck& deck_l = engine.decks[left_id];
            KuroAudio::DJDeck& deck_r = engine.decks[right_id];

            float ch_w = (w - 24.0f) * 0.5f;

            auto drawMixerStrip = [&](KuroAudio::DJDeck& deck, int id, float start_x) {
                float k_r = 10.0f;
                float cy = pos.y + 16.0f;

                // TRIM
                char t_lbl[32]; snprintf(t_lbl, sizeof(t_lbl), "TRIM##%d", id);
                KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(t_lbl, &deck.trim_gain, 0.0f, 2.0f, k_r, ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "%.1fx", 1.0f);

                // HIGH
                cy += 38.0f;
                char h_lbl[32]; snprintf(h_lbl, sizeof(h_lbl), "HIGH##%d", id);
                KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(h_lbl, &deck.eq_high, 0.0f, 2.0f, k_r, ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "%.1fx", 1.0f);

                // MID
                cy += 38.0f;
                char m_lbl[32]; snprintf(m_lbl, sizeof(m_lbl), "MID##%d", id);
                KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(m_lbl, &deck.eq_mid, 0.0f, 2.0f, k_r, ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "%.1fx", 1.0f);

                // LOW
                cy += 38.0f;
                char l_lbl[32]; snprintf(l_lbl, sizeof(l_lbl), "LOW##%d", id);
                KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(l_lbl, &deck.eq_low, 0.0f, 2.0f, k_r, ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "%.1fx", 1.0f);

                // CUE
                cy += 40.0f;
                float cue_btn_w = 34.0f;
                ImVec2 cue_p = ImVec2(start_x + (ch_w - cue_btn_w) * 0.5f, cy);
                ImGui::SetCursorScreenPos(cue_p);
                bool is_cue = (id == 0) ? engine.getCueDeckA() : engine.getCueDeckB();
                if (is_cue) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.65f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 0.9f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.65f, 0.0f, 0.9f));
                }
                char cb_id[32]; snprintf(cb_id, sizeof(cb_id), "CUE##ChCue_%d", id);
                if (ImGui::Button(cb_id, ImVec2(cue_btn_w, 18.0f))) {
                    if (id == 0) engine.setCueDeckA(!is_cue);
                    else engine.setCueDeckB(!is_cue);
                }
                ImGui::PopStyleColor(2);

                // Fader de Volume
                cy += 24.0f;
                float fader_x = start_x + (ch_w - 24.0f) * 0.5f;
                ImGui::SetCursorScreenPos(ImVec2(fader_x, cy));
                char vf_id[32]; snprintf(vf_id, sizeof(vf_id), "##RekVol_%d", id);
                KuroUI::KuroDJStudioSkinManager::DrawSkinVerticalFader(vf_id, &deck.volume, 0.0f, 1.0f, 24.0f, 75.0f, ImVec4(0.0f, 0.65f, 1.0f, 1.0f), "%.0f%%");

                // VU Meter Stereo LED
                float vu_x = fader_x + 26.0f;
                float vu_h = 70.0f;
                dl->AddRectFilled(ImVec2(vu_x, cy + 2.0f), ImVec2(vu_x + 4.0f, cy + 2.0f + vu_h), IM_COL32(5, 7, 10, 255));
                float vu_norm = std::clamp(deck.vu_meter, 0.0f, 1.0f);
                float lit_h = vu_norm * vu_h;
                if (lit_h > 1.0f) {
                    dl->AddRectFilled(ImVec2(vu_x, cy + 2.0f + vu_h - lit_h), ImVec2(vu_x + 4.0f, cy + 2.0f + vu_h), IM_COL32(0, 220, 255, 240));
                }
            };

            drawMixerStrip(deck_l, 0, pos.x + 8.0f);
            drawMixerStrip(deck_r, 1, pos.x + 8.0f + ch_w + 4.0f);

            dl->AddLine(ImVec2(pos.x + w * 0.5f, pos.y + 12.0f), ImVec2(pos.x + w * 0.5f, pos.y + h - 60.0f), IM_COL32(28, 34, 46, 180), 1.0f);

            float bot_y = pos.y + h - 50.0f;
            dl->AddLine(ImVec2(pos.x + 6.0f, bot_y), ImVec2(pos.x + w - 6.0f, bot_y), IM_COL32(26, 32, 44, 200), 1.0f);

            float xf_w = w - 36.0f;
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 18.0f, bot_y + 14.0f));
            ImGui::PushItemWidth(xf_w);
            ImGui::SliderFloat("##RekCrossfader", &engine.crossfader, 0.0f, 1.0f, "");
            ImGui::PopItemWidth();
        }

        void renderRekordboxBottomBar(KuroAudio::DJEngine& engine, float w) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float h = KuroUI::UIScaleManager::Scale(26.0f);

            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(12, 14, 18, 255));
            dl->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + w, pos.y), IM_COL32(25, 30, 40, 255), 1.0f);

            ImVec2 logo_pos = ImVec2(pos.x + 10.0f, pos.y + 6.0f);
            dl->AddCircle(ImVec2(logo_pos.x + 6.0f, logo_pos.y + 7.0f), 5.0f, IM_COL32(255, 255, 255, 255), 16, 1.5f);
            dl->AddText(ImVec2(logo_pos.x + 16.0f, logo_pos.y + 1.0f), IM_COL32(255, 255, 255, 255), "rekordbox");

            float tab1_x = pos.x + 120.0f;
            const char* pad_tabs[] = { "HOT CUE", "BEAT LOOP", "PAD FX 1", "SAMPLER" };
            for (int t = 0; t < 4; t++) {
                bool act = (rekordbox_pad_mode[0] == t);
                ImVec2 t_pos = ImVec2(tab1_x + (float)t * 68.0f, pos.y + 3.0f);
                dl->AddRectFilled(t_pos, ImVec2(t_pos.x + 64.0f, t_pos.y + 20.0f), act ? IM_COL32(0, 132, 255, 255) : IM_COL32(18, 22, 28, 255), 2.0f);
                dl->AddText(ImVec2(t_pos.x + 8.0f, t_pos.y + 3.0f), act ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 175, 195, 240), pad_tabs[t]);
                ImGui::SetCursorScreenPos(t_pos);
                char tb_id[32]; snprintf(tb_id, sizeof(tb_id), "##RekTab0_%d", t);
                if (ImGui::InvisibleButton(tb_id, ImVec2(64.0f, 20.0f))) {
                    rekordbox_pad_mode[0] = t;
                }
            }

            float center_x = pos.x + w * 0.5f - 28.0f;
            dl->AddText(ImVec2(center_x, pos.y + 6.0f), IM_COL32(120, 135, 155, 200), "PAD MODE");

            float tab2_x = pos.x + w - 380.0f;
            for (int t = 0; t < 4; t++) {
                bool act = (rekordbox_pad_mode[1] == t);
                ImVec2 t_pos = ImVec2(tab2_x + (float)t * 68.0f, pos.y + 3.0f);
                dl->AddRectFilled(t_pos, ImVec2(t_pos.x + 64.0f, t_pos.y + 20.0f), act ? IM_COL32(0, 132, 255, 255) : IM_COL32(18, 22, 28, 255), 2.0f);
                dl->AddText(ImVec2(t_pos.x + 8.0f, t_pos.y + 3.0f), act ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 175, 195, 240), pad_tabs[t]);
                ImGui::SetCursorScreenPos(t_pos);
                char tb_id[32]; snprintf(tb_id, sizeof(tb_id), "##RekTab1_%d", t);
                if (ImGui::InvisibleButton(tb_id, ImVec2(64.0f, 20.0f))) {
                    rekordbox_pad_mode[1] = t;
                }
            }
        }
'''

# Find insertion spot right before "void renderDeckPanel"
target_marker = "void renderDeckPanel(KuroAudio::DJDeck& deck"
if target_marker in code:
    code = code.replace(target_marker, rekordbox_functions + "\n        " + target_marker, 1)
    print("[2] Inserted Rekordbox methods before renderDeckPanel")
else:
    print("[2] ERROR: target_marker not found")

# Update renderStackedRekordboxWaveforms to add left zoom controls
target_start = "void renderStackedRekordboxWaveforms(KuroAudio::DJEngine& engine, float w, float h) {"
target_block = """        void renderStackedRekordboxWaveforms(KuroAudio::DJEngine& engine, float w, float h) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            
            float lane_gap = 2.0f; // Divisor ultrafino Rekordbox entre os Decks
            float min_lane = KuroUI::UIScaleManager::Scale(52.0f);
            float lane_h   = std::max(min_lane, (h - lane_gap) * 0.5f);
            float total_stack_h = lane_h * 2.0f + lane_gap;

            // Botoes de Zoom Verticais na lateral esquerda do Rekordbox (^, v, +, RST, -)
            float ctrl_w = 26.0f;
            float wave_x = pos.x + ctrl_w;
            float wave_w = w - ctrl_w;
            float center_x = wave_x + wave_w * 0.5f;

            dl->AddRectFilled(pos, ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(12, 15, 20, 255));
            dl->AddLine(ImVec2(pos.x + ctrl_w, pos.y), ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(24, 30, 42, 255), 1.0f);

            auto zoomSideBtn = [&](const char* id, const char* lbl, float y_off, auto on_click) {
                ImVec2 bp = ImVec2(pos.x + 2.0f, pos.y + y_off);
                dl->AddRectFilled(bp, ImVec2(bp.x + 22.0f, bp.y + 16.0f), IM_COL32(20, 25, 34, 255), 2.0f);
                dl->AddRect(bp, ImVec2(bp.x + 22.0f, bp.y + 16.0f), IM_COL32(38, 48, 65, 200), 2.0f, 0, 1.0f);
                ImVec2 sz = ImGui::CalcTextSize(lbl);
                dl->AddText(ImVec2(bp.x + (22.0f - sz.x) * 0.5f, bp.y + 1.0f), IM_COL32(190, 205, 225, 240), lbl);
                ImGui::SetCursorScreenPos(bp);
                if (ImGui::InvisibleButton(id, ImVec2(22.0f, 16.0f))) on_click();
            };

            zoomSideBtn("##WfSideUp", "^", 4.0f, [&]() {
                if (waveform_zoom_bars == 32) waveform_zoom_bars = 16;
                else if (waveform_zoom_bars == 16) waveform_zoom_bars = 8;
                else if (waveform_zoom_bars == 8) waveform_zoom_bars = 4;
            });
            zoomSideBtn("##WfSideDn", "v", 24.0f, [&]() {
                if (waveform_zoom_bars == 4) waveform_zoom_bars = 8;
                else if (waveform_zoom_bars == 8) waveform_zoom_bars = 16;
                else if (waveform_zoom_bars == 16) waveform_zoom_bars = 32;
            });
            zoomSideBtn("##WfSidePlus", "+", 46.0f, [&]() {
                waveform_height = std::clamp(waveform_height + 15.0f, 96.0f, 260.0f);
            });
            zoomSideBtn("##WfSideRst", "<", 66.0f, [&]() {
                waveform_zoom_bars = 4;
                waveform_height = 144.0f;
            });
            zoomSideBtn("##WfSideMinus", "-", 86.0f, [&]() {
                waveform_height = std::clamp(waveform_height - 15.0f, 96.0f, 260.0f);
            });

            ImVec2 l0_pos = ImVec2(wave_x, pos.y);
            ImVec2 l1_pos = ImVec2(wave_x, pos.y + lane_h + lane_gap);"""

# Replace in renderStackedRekordboxWaveforms:
pattern_wave = r"void renderStackedRekordboxWaveforms\(KuroAudio::DJEngine& engine, float w, float h\) \{\s+ImDrawList\* dl = ImGui::GetWindowDrawList\(\);\s+ImVec2 pos = ImGui::GetCursorScreenPos\(\);\s+float lane_gap = 2\.0f;[^\n]*\s+float min_lane = KuroUI::UIScaleManager::Scale\(52\.0f\);\s+float lane_h\s+= std::max\(min_lane, \(h - lane_gap\) \* 0\.5f\);\s+float total_stack_h = lane_h \* 2\.0f \+ lane_gap;\s+float center_x = pos\.x \+ w \* 0\.5f;\s+ImVec2 l0_pos = ImVec2\(pos\.x, pos\.y\);\s+ImVec2 l1_pos = ImVec2\(pos\.x, pos\.y \+ lane_h \+ lane_gap\);"
if re.search(pattern_wave, code):
    code = re.sub(pattern_wave, target_block, code, count=1)
    print("[3] Added lateral zoom controls to renderStackedRekordboxWaveforms")
else:
    print("[3] WARNING: pattern_wave not matched")

# Also in renderStackedRekordboxWaveforms, update width used in lanes from w to wave_w:
# Let's replace the line: dl->AddRectFilled(l_pos, ImVec2(l_pos.x + w, l_pos.y + lane_h), IM_COL32(0, 0, 0, 255));
code = code.replace("dl->AddRectFilled(l_pos, ImVec2(l_pos.x + w, l_pos.y + lane_h), IM_COL32(0, 0, 0, 255));",
                    "dl->AddRectFilled(l_pos, ImVec2(l_pos.x + wave_w, l_pos.y + lane_h), IM_COL32(0, 0, 0, 255));")
code = code.replace("float bx = l_pos.x + (float)((bf - start_f_d) / window_frames_d) * w;",
                    "float bx = l_pos.x + (float)((bf - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("int num_cols = std::max(1, (int)(w / col_step));",
                    "int num_cols = std::max(1, (int)(wave_w / col_step));")
code = code.replace("float cx_cue = l_pos.x + (float)((deck.cue_frame - start_f_d) / window_frames_d) * w;",
                    "float cx_cue = l_pos.x + (float)((deck.cue_frame - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("float cx = l_pos.x + (float)((cf - start_f_d) / window_frames_d) * w;",
                    "float cx = l_pos.x + (float)((cf - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("float mx = l_pos.x + (float)((mf - start_f_d) / window_frames_d) * w;",
                    "float mx = l_pos.x + (float)((mf - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("float dx = l_pos.x + (float)((deck.drop_point_frame - start_f_d) / window_frames_d) * w;",
                    "float dx = l_pos.x + (float)((deck.drop_point_frame - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("float x_in  = l_pos.x + (float)((deck.loop_start_frame - start_f_d) / window_frames_d) * w;",
                    "float x_in  = l_pos.x + (float)((deck.loop_start_frame - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("float x_out = l_pos.x + (float)((loop_end_frame - start_f_d) / window_frames_d) * w;",
                    "float x_out = l_pos.x + (float)((loop_end_frame - start_f_d) / window_frames_d) * wave_w;")
code = code.replace("ImGui::InvisibleButton(lane_btn_id, ImVec2(w, lane_h));",
                    "ImGui::SetCursorScreenPos(l_pos); ImGui::InvisibleButton(lane_btn_id, ImVec2(wave_w, lane_h));")

# Now update the main render() loop to use the Rekordbox visual when dj_layout_mode == 0
old_main_render = """                // --- 1. TOP HUD (1 linha compacta) ---
                renderTopHUD(engine, p_open);

                // --- CALCULA MODOS DE LAYOUT ANTES DE RENDERIZAR O TOPO ---
                ImGuiViewport* vp_curr = ImGui::GetMainViewport();
                if (dj_layout_mode == 0) {
                    // PRO CONSOLE TOUCH: Waveforms dinâmicas adaptativas para garantir 100% de visibilidade vertical para Decks e Mixer DJM sem cortes
                    float wave_ratio = (vp_curr->Size.y < 820.0f) ? 0.128f : 0.145f;
                    waveform_height = std::clamp(vp_curr->Size.y * wave_ratio, 96.0f, 150.0f);
                    show_playlist_drawer = false; // Gaveta fechada por padrão (Biblioteca aberta via Modal 1260x720)
                } else if (dj_layout_mode == 1) {
                    // WAVE MAX
                    waveform_height = std::clamp(vp_curr->Size.y * 0.22f, 180.0f, 260.0f);
                } else {
                    // SPLIT GAVETA
                    waveform_height = std::clamp(vp_curr->Size.y * 0.12f, 96.0f, 130.0f);
                    library_height  = 150.0f;
                    show_playlist_drawer = true;
                }

                // Calcula largura disponível para waveforms e decks
                float top_wave_w = ImGui::GetContentRegionAvail().x;

                // --- 2. WAVEFORM TOOLBAR (inclui Phase Meter integrado à direita) ---
                renderWaveformToolbar(engine, top_wave_w);

                // --- 3. WAVEFORMS FULL-WIDTH EMPILHADAS (Deck 1 + Régua Central + Deck 2) ---
                float wave_h_px = KuroUI::UIScaleManager::Scale(waveform_height);
                renderStackedRekordboxWaveforms(engine, top_wave_w, wave_h_px);

                // Separador fino entre waveforms e corpo central (LED Cyan Accent)
                {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 sep_pos = ImGui::GetCursorScreenPos();
                    dl->AddLine(sep_pos, ImVec2(sep_pos.x + top_wave_w, sep_pos.y),
                                IM_COL32(0, 180, 230, 180), 1.5f);
                    ImGui::Dummy(ImVec2(top_wave_w, 4.0f));
                }

                // --- 4. CORPO CENTRAL: 3 COLUNAS (DECK | MIXER | DECK) ---
                float total_w       = ImGui::GetContentRegionAvail().x;
                float total_avail_h = ImGui::GetContentRegionAvail().y;

                float bottom_drawer_h = 0.0f;
                if (show_playlist_drawer) bottom_drawer_h += KuroUI::UIScaleManager::Scale(library_height);
                if (show_export_modal)    bottom_drawer_h += KuroUI::UIScaleManager::Scale(136.0f);

                float central_h = total_avail_h - bottom_drawer_h
                                  - (show_playlist_drawer ? KuroUI::UIScaleManager::Scale(8.0f) : 0.0f);
                if (central_h < 320.0f) central_h = 320.0f;

                // Mixer proporção ergonômica de 24% (entre 240px e 380px), Decks dividem o restante
                float gap_px    = KuroUI::UIScaleManager::Scale(5.0f);
                float mixer_w   = std::clamp(total_w * 0.24f,
                                             KuroUI::UIScaleManager::Scale(240.0f),
                                             KuroUI::UIScaleManager::Scale(380.0f));
                float deck_w    = (total_w - mixer_w - gap_px * 2.0f) * 0.5f;

                // Rolagem vertical com roda do mouse e scrollbar caso a resolução seja compacta
                ImGuiWindowFlags child_flags = 0;

                ImVec4 col_left  = (active_l == 0 ? col_deck_1 : col_deck_3);
                ImVec4 col_right = (active_r == 1 ? col_deck_2 : col_deck_4);

                // DECK ESQUERDO
                ImGui::BeginChild("##DeckLeft_Child", ImVec2(deck_w, central_h), false, child_flags);
                renderDeckPanel(engine.decks[active_l], 0, col_left, engine);
                ImGui::EndChild();

                ImGui::SameLine(0, gap_px);

                // MIXER CENTRAL DJM-V10
                ImGui::BeginChild("##Mixer_Child", ImVec2(mixer_w, central_h), false, child_flags);
                renderCentralMixer(engine);
                ImGui::EndChild();

                ImGui::SameLine(0, gap_px);

                // DECK DIREITO
                ImGui::BeginChild("##DeckRight_Child", ImVec2(deck_w, central_h), false, child_flags);
                renderDeckPanel(engine.decks[active_r], 1, col_right, engine);
                ImGui::EndChild();"""

new_main_render = """                // --- 1. TOP HUD (Rekordbox Header) ---
                if (dj_layout_mode == 0) {
                    renderRekordboxTopBar(engine, p_open);
                } else {
                    renderTopHUD(engine, p_open);
                }

                // --- CALCULA MODOS DE LAYOUT ---
                ImGuiViewport* vp_curr = ImGui::GetMainViewport();
                if (dj_layout_mode == 0) {
                    float wave_ratio = (vp_curr->Size.y < 820.0f) ? 0.128f : 0.145f;
                    waveform_height = std::clamp(vp_curr->Size.y * wave_ratio, 96.0f, 140.0f);
                    show_playlist_drawer = false;
                } else if (dj_layout_mode == 1) {
                    waveform_height = std::clamp(vp_curr->Size.y * 0.22f, 180.0f, 260.0f);
                } else {
                    waveform_height = std::clamp(vp_curr->Size.y * 0.12f, 96.0f, 130.0f);
                    library_height  = 150.0f;
                    show_playlist_drawer = true;
                }

                float top_wave_w = ImGui::GetContentRegionAvail().x;

                // --- 2. BARRA DE FX / TOOLBAR ---
                if (dj_layout_mode == 0) {
                    if (show_rekordbox_fx_bar) {
                        renderRekordboxFXBar(engine, top_wave_w);
                    }
                } else {
                    renderWaveformToolbar(engine, top_wave_w);
                }

                // --- 3. WAVEFORMS FULL-WIDTH EMPILHADAS REKORDBOX ---
                float wave_h_px = KuroUI::UIScaleManager::Scale(waveform_height);
                renderStackedRekordboxWaveforms(engine, top_wave_w, wave_h_px);

                // Separador fino
                {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 sep_pos = ImGui::GetCursorScreenPos();
                    dl->AddLine(sep_pos, ImVec2(sep_pos.x + top_wave_w, sep_pos.y),
                                IM_COL32(24, 30, 42, 220), 1.0f);
                    ImGui::Dummy(ImVec2(top_wave_w, 2.0f));
                }

                // --- 4. CORPO CENTRAL: 3 COLUNAS (DECK 1 | MIXER | DECK 2) ---
                float total_w       = ImGui::GetContentRegionAvail().x;
                float total_avail_h = ImGui::GetContentRegionAvail().y;

                float bottom_bar_h = (dj_layout_mode == 0) ? KuroUI::UIScaleManager::Scale(26.0f) : 0.0f;
                float bottom_drawer_h = bottom_bar_h;
                if (show_playlist_drawer) bottom_drawer_h += KuroUI::UIScaleManager::Scale(library_height);
                if (show_export_modal)    bottom_drawer_h += KuroUI::UIScaleManager::Scale(136.0f);

                float central_h = total_avail_h - bottom_drawer_h - 4.0f;
                if (central_h < 300.0f) central_h = 300.0f;

                float gap_px    = KuroUI::UIScaleManager::Scale(4.0f);
                float mixer_w   = std::clamp(total_w * 0.20f,
                                             KuroUI::UIScaleManager::Scale(210.0f),
                                             KuroUI::UIScaleManager::Scale(300.0f));
                float deck_w    = (total_w - mixer_w - gap_px * 2.0f) * 0.5f;

                ImGuiWindowFlags child_flags = 0;
                ImVec4 col_left  = (active_l == 0 ? col_deck_1 : col_deck_3);
                ImVec4 col_right = (active_r == 1 ? col_deck_2 : col_deck_4);

                // DECK ESQUERDO
                ImGui::BeginChild("##DeckLeft_Child", ImVec2(deck_w, central_h), false, child_flags);
                if (dj_layout_mode == 0) {
                    renderRekordboxDeckPanel(engine.decks[active_l], 0, col_left, engine);
                } else {
                    renderDeckPanel(engine.decks[active_l], 0, col_left, engine);
                }
                ImGui::EndChild();

                ImGui::SameLine(0, gap_px);

                // MIXER CENTRAL
                ImGui::BeginChild("##Mixer_Child", ImVec2(mixer_w, central_h), false, child_flags);
                if (dj_layout_mode == 0) {
                    renderRekordboxCenterMixer(engine);
                } else {
                    renderCentralMixer(engine);
                }
                ImGui::EndChild();

                ImGui::SameLine(0, gap_px);

                // DECK DIREITO
                ImGui::BeginChild("##DeckRight_Child", ImVec2(deck_w, central_h), false, child_flags);
                if (dj_layout_mode == 0) {
                    renderRekordboxDeckPanel(engine.decks[active_r], 1, col_right, engine);
                } else {
                    renderDeckPanel(engine.decks[active_r], 1, col_right, engine);
                }
                ImGui::EndChild();

                // --- 4.5. BARRA DE STATUS INFERIOR REKORDBOX ---
                if (dj_layout_mode == 0) {
                    renderRekordboxBottomBar(engine, total_w);
                }"""

if old_main_render in code:
    code = code.replace(old_main_render, new_main_render, 1)
    print("[4] Replaced main render loop with Rekordbox layout switch")
else:
    print("[4] WARNING: old_main_render exact match failed, attempting line by line search")

with open("src/ui/KuroDJStudioUI.h", "w", encoding="utf-8") as f:
    f.write(code)

print("Finished applying Rekordbox layout modifications!")
