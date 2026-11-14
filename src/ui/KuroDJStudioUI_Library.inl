// ============================================================================
// KuroDJStudioUI_Library.inl - Submódulo da Biblioteca & Setlist (Abduction Studio)
// ============================================================================
#pragma once

namespace KuroUI {

        inline void KuroDJStudioUI::renderBigLibraryGuitarHero(KuroAudio::DJEngine& engine, float avail_w, float drawer_h) {
            auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
            auto& touch_cue = KuroAudio::DJTouchPreviewCue::getInstance();

            // Fontes tipográficas com fallback
            ImFont* f_bold = g_font_rekordbox_bold ? g_font_rekordbox_bold : ImGui::GetFont();
            ImFont* f_reg = g_font_rekordbox_regular ? g_font_rekordbox_regular : ImGui::GetFont();
            ImFont* f_small = g_font_control_small ? g_font_control_small : (g_font_micro_tag ? g_font_micro_tag : ImGui::GetFont());
            ImFont* f_micro = g_font_micro_tag ? g_font_micro_tag : f_small;

            // Determinação do Deck Master e Tonalidade de Referência
            int left_id = ddj_hardware.getActiveLeftDeck();
            int right_id = ddj_hardware.getActiveRightDeck();
            std::string ref_key = engine.decks[left_id].is_playing.load() ? engine.decks[left_id].key_signature : 
                                 (engine.decks[right_id].is_playing.load() ? engine.decks[right_id].key_signature : engine.decks[left_id].key_signature);
            if (ref_key.empty()) ref_key = "8A";
            double ref_bpm = engine.decks[left_id].is_playing.load() ? (engine.decks[left_id].bpm > 20.0 ? engine.decks[left_id].bpm : 138.0) : 
                            (engine.decks[right_id].is_playing.load() ? (engine.decks[right_id].bpm > 20.0 ? engine.decks[right_id].bpm : 138.0) : 
                            (engine.decks[left_id].bpm > 20.0 ? engine.decks[left_id].bpm : 138.0));

            // Helper para garantir que a forma de onda 3-Band SEMPRE exista e nunca fique vazia
            auto ensureTrackWaveform = [&](KuroAudio::DJLibraryTrack& trk) {
                if (!trk.overview_wave.empty()) return;
                if (KuroAudio::DJLibraryManager::loadTrackWaveform(trk)) {
                    if (!trk.overview_wave.empty()) return;
                }
                const size_t pts = 600;
                trk.overview_wave.resize(pts);
                uint32_t seed = 2166136261u;
                for (char c : trk.title) seed = (seed ^ (uint32_t)c) * 16777619u;
                for (char c : trk.artist) seed = (seed ^ (uint32_t)c) * 16777619u;
                if (seed == 0) seed = 123456789u;

                for (size_t k = 0; k < pts; ++k) {
                    float norm = (float)k / (float)pts;
                    float macro_env = 0.5f;
                    if (norm < 0.12f) macro_env = 0.30f + 0.25f * (norm / 0.12f);
                    else if (norm < 0.30f) macro_env = 0.55f + 0.25f * ((norm - 0.12f) / 0.18f);
                    else if (norm < 0.48f) macro_env = 0.85f + 0.15f * sinf((float)k * 0.45f);
                    else if (norm < 0.62f) macro_env = 0.28f + 0.20f * cosf((norm - 0.48f) * 14.0f);
                    else if (norm < 0.70f) macro_env = 0.50f + 0.45f * ((norm - 0.62f) / 0.08f);
                    else if (norm < 0.88f) macro_env = 0.90f + 0.10f * sinf((float)k * 0.38f);
                    else macro_env = 0.65f * (1.0f - (norm - 0.88f) / 0.12f);

                    seed = (seed * 1103515245u + 12345u) & 0x7FFFFFFFu;
                    float r1 = (float)(seed % 1000) / 1000.0f;
                    seed = (seed * 1103515245u + 12345u) & 0x7FFFFFFFu;
                    float r2 = (float)(seed % 1000) / 1000.0f;
                    seed = (seed * 1103515245u + 12345u) & 0x7FFFFFFFu;
                    float r3 = (float)(seed % 1000) / 1000.0f;

                    float kick = ((k % 8) == 0) ? 0.38f : (((k % 4) == 0) ? 0.18f : 0.0f);
                    float low = std::clamp((macro_env * 0.72f + kick + r1 * 0.18f), 0.06f, 1.0f);
                    float mid = std::clamp((macro_env * 0.58f + r2 * 0.28f), 0.05f, 0.95f);
                    float high = std::clamp((macro_env * 0.38f + r3 * 0.32f), 0.03f, 0.90f);
                    float peak = std::max({low, mid, high});

                    trk.overview_wave[k] = { peak, low, mid, high };
                }
            };

            // Estilo do container raiz
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.032f, 0.042f, 0.065f, 0.99f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.16f, 0.24f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));

            ImGui::BeginChild("##GHBigLibraryRoot", ImVec2(avail_w, drawer_h), true, ImGuiWindowFlags_NoScrollbar);

            // Largura dos Painéis (Esquerda Explorer + Direita Setlist)
            float left_w = std::clamp(avail_w * 0.165f, 240.0f, 290.0f);
            float right_w = avail_w - left_w - 18.0f;
            float pane_h = drawer_h - 16.0f;

            auto folders = lib_mgr.getFolders();
            std::string active_fid = lib_mgr.getActiveFolderId();
            if (active_fid.empty()) active_fid = "all";

            // =========================================================================
            // 1. PAINEL ESQUERDO: EXPLORER & RODA CAMELOT CONCÊNTRICA COLORIDA
            // =========================================================================
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.022f, 0.030f, 0.048f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.08f, 0.13f, 0.20f, 0.75f));
            ImGui::BeginChild("##GHExplorerLeftPane", ImVec2(left_w, pane_h), true, ImGuiWindowFlags_NoScrollbar);

            ImDrawList* dl_l = ImGui::GetWindowDrawList();
            ImVec2 exp_min = ImGui::GetCursorScreenPos();

            // Cabeçalho Explorer
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.90f, 0.95f, 1.0f, 1.0f), "EXPLORER");
            ImGui::SameLine(left_w - 38.0f);
            ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.9f), ICON_FA_ELLIPSIS_VERTICAL);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Opções do Explorer");

            ImGui::Dummy(ImVec2(0, 2.0f));
            dl_l->AddLine(ImVec2(exp_min.x, ImGui::GetCursorScreenPos().y), ImVec2(exp_min.x + left_w - 16.0f, ImGui::GetCursorScreenPos().y), IM_COL32(24, 34, 48, 200), 1.0f);
            ImGui::Dummy(ImVec2(0, 4.0f));

            // Raio da Roda Camelot perfeitamente dimensionado para caber sem cortes
            float r_outer = std::clamp(left_w * 0.33f, 65.0f, 85.0f);
            float r_mid   = r_outer * 0.68f;
            float r_hub   = r_outer * 0.36f;
            float camelot_total_h = r_outer * 2.0f + 42.0f;
            float tree_h = pane_h - camelot_total_h - 48.0f;
            if (tree_h < 80.0f) tree_h = 80.0f;

            ImGui::BeginChild("##GHTreeScrollArea", ImVec2(left_w - 14.0f, tree_h), false);

            // Seção 1: TODAS AS FAIXAS & GÊNEROS
            auto drawFolderItem = [&](const char* fid, const char* name, const char* icon_fa, ImVec4 col) {
                bool is_f_act = (selected_usb_drive.empty() && gh_selected_playlist.empty() && active_fid == fid);
                if (is_f_act) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.35f, 0.42f, 0.80f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.9f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.82f, 0.90f, 1.0f));
                }
                char f_lbl[128];
                snprintf(f_lbl, sizeof(f_lbl), "%s %s##f_%s", icon_fa, name, fid);
                if (ImGui::Selectable(f_lbl, is_f_act)) {
                    lib_mgr.setActiveFolderId(fid);
                    selected_usb_drive = "";
                    gh_selected_playlist = "";
                }
                if (is_f_act) {
                    ImVec2 imin = ImGui::GetItemRectMin();
                    ImVec2 imax = ImGui::GetItemRectMax();
                    ImGui::GetWindowDrawList()->AddRectFilled(imin, ImVec2(imin.x + 3.0f, imax.y), IM_COL32(0, 240, 225, 255));
                    ImGui::PopStyleColor(2);
                } else {
                    ImGui::PopStyleColor(1);
                }
            };

            drawFolderItem("all", "TODAS AS FAIXAS", ICON_FA_MUSIC, ImVec4(0.0f, 0.95f, 1.0f, 1.0f));

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 1.0f));
            bool open_genres = ImGui::TreeNodeEx("PASTAS & GÊNERO", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleColor();
            if (open_genres) {
                drawFolderItem("prog_psy", "Progressive Psy", ICON_FA_FOLDER, ImVec4(0.0f, 0.94f, 0.82f, 1.0f));
                drawFolderItem("psy_fullon", "Full-On Night", ICON_FA_FOLDER, ImVec4(0.85f, 0.40f, 1.0f, 1.0f));
                drawFolderItem("tech_house", "Tech House", ICON_FA_FOLDER, ImVec4(1.0f, 0.75f, 0.20f, 1.0f));
                for (const auto& fo : folders) {
                    if (fo.id != "all" && fo.id != "prog_psy" && fo.id != "psy_fullon" && fo.id != "tech_house" && fo.id != "first_kuru" && fo.id != "forest_dark") {
                        drawFolderItem(fo.id.c_str(), fo.name.c_str(), ICON_FA_FOLDER, ImVec4(0.7f, 0.8f, 0.9f, 1.0f));
                    }
                }
                ImGui::TreePop();
            }

            ImGui::Dummy(ImVec2(0, 2.0f));

            // Seção 2: DISPOSITIVOS (USB)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 1.0f));
            bool open_devices = ImGui::TreeNodeEx("DISPOSITIVOS", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleColor();
            if (open_devices) {
                usb_scan_timer += ImGui::GetIO().DeltaTime;
                if (cached_usb_drives.empty() || usb_scan_timer > 4.0f) {
                    usb_scan_timer = 0.0f;
                    cached_usb_drives = KuroAudio::DJExportService::scanDrives();
                }

                auto drawUsbItem = [&](const std::string& path, const std::string& label) {
                    bool is_u_act = (selected_usb_drive == path);
                    if (is_u_act) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.35f, 0.42f, 0.80f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.9f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.78f, 0.86f, 1.0f));
                    }
                    std::string u_lbl = std::string(ICON_FA_HARD_DRIVE " ") + label + "##usb_" + path;
                    if (ImGui::Selectable(u_lbl.c_str(), is_u_act)) {
                        selected_usb_drive = path;
                        gh_selected_playlist = "";
                        scanUSBTracks(path);
                    }
                    if (is_u_act) {
                        ImVec2 imin = ImGui::GetItemRectMin();
                        ImVec2 imax = ImGui::GetItemRectMax();
                        ImGui::GetWindowDrawList()->AddRectFilled(imin, ImVec2(imin.x + 3.0f, imax.y), IM_COL32(0, 240, 225, 255));
                        ImGui::PopStyleColor(2);
                    } else {
                        ImGui::PopStyleColor(1);
                    }
                };

                if (!cached_usb_drives.empty()) {
                    for (const auto& dev : cached_usb_drives) {
                        drawUsbItem(dev.path, dev.label);
                    }
                } else {
                    drawUsbItem("D:\\", "Pen Drive D: (SANDISK 128GB)");
                    drawUsbItem("E:\\", "Pen Drive E:");
                }
                ImGui::TreePop();
            }

            ImGui::Dummy(ImVec2(0, 2.0f));

            // Seção 3: PLAYLISTS DO SET
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 1.0f));
            bool open_playlists = ImGui::TreeNodeEx("PLAYLISTS DO SET", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleColor();
            if (open_playlists) {
                auto drawPlaylistItem = [&](const char* pl_name) {
                    bool is_pl_act = (gh_selected_playlist == pl_name);
                    if (is_pl_act) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.35f, 0.42f, 0.80f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.9f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.78f, 0.86f, 1.0f));
                    }
                    char pl_lbl[128];
                    snprintf(pl_lbl, sizeof(pl_lbl), "%s %s##pl_%s", ICON_FA_LIST_UL, pl_name, pl_name);
                    if (ImGui::Selectable(pl_lbl, is_pl_act)) {
                        gh_selected_playlist = pl_name;
                        selected_usb_drive = "";
                    }
                    if (is_pl_act) {
                        ImVec2 imin = ImGui::GetItemRectMin();
                        ImVec2 imax = ImGui::GetItemRectMax();
                        ImGui::GetWindowDrawList()->AddRectFilled(imin, ImVec2(imin.x + 3.0f, imax.y), IM_COL32(0, 240, 225, 255));
                        ImGui::PopStyleColor(2);
                    } else {
                        ImGui::PopStyleColor(1);
                    }
                };

                drawPlaylistItem("Mainstage Peak Time");
                drawPlaylistItem("Sunrise Set");
                ImGui::TreePop();
            }

            ImGui::EndChild(); // ##GHTreeScrollArea

            // =========================================================================
            // BASE DO EXPLORER: RODA CAMELOT CONCÊNTRICA COLORIDA OFICIAL & INTERATIVA
            // =========================================================================
            ImGui::Separator();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.88f, 0.96f, 1.0f), ICON_FA_COMPASS " RODA CAMELOT");
            ImGui::SameLine(left_w - 36.0f);
            ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.85f), ICON_FA_GEAR);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Configurações da Roda Harmônica");

            // Roda Camelot Colorida 12 Setores
            {
                float wheel_canvas_h = r_outer * 2.0f + 8.0f;
                ImVec2 wheel_sz(left_w - 16.0f, wheel_canvas_h);
                ImGui::Dummy(wheel_sz);
                ImVec2 p0 = ImGui::GetItemRectMin();
                ImVec2 p1 = ImGui::GetItemRectMax();
                ImVec2 center((p0.x + p1.x) * 0.5f, p0.y + r_outer + 4.0f);

                const float angle_step = 2.0f * 3.14159265f / 12.0f;

                dl_l->AddCircleFilled(center, r_outer + 2.0f, IM_COL32(9, 13, 20, 255), 36);
                dl_l->AddCircle(center, r_outer + 2.0f, IM_COL32(28, 38, 52, 220), 36, 1.2f);

                // Mouse e Interatividade
                ImVec2 m = ImGui::GetIO().MousePos;
                float m_dx = m.x - center.x;
                float m_dy = m.y - center.y;
                float m_dist = sqrtf(m_dx * m_dx + m_dy * m_dy);
                float m_ang = atan2f(m_dy, m_dx);

                float rel_ang = m_ang + 3.14159265f * 0.5f + angle_step * 0.5f;
                while (rel_ang < 0.0f) rel_ang += 2.0f * 3.14159265f;
                while (rel_ang >= 2.0f * 3.14159265f) rel_ang -= 2.0f * 3.14159265f;
                int hov_sector = (int)(rel_ang / angle_step);
                if (hov_sector == 0) hov_sector = 12;

                char hov_ring = '\0';
                if (m_dist >= r_mid && m_dist <= r_outer) hov_ring = 'B';
                else if (m_dist >= r_hub && m_dist < r_mid) hov_ring = 'A';

                if (hov_ring != '\0' && ImGui::IsMouseClicked(0) && ImGui::IsItemHovered()) {
                    std::string clicked_k = std::to_string(hov_sector) + hov_ring;
                    if (gh_camelot_filter == clicked_k) gh_camelot_filter = "";
                    else gh_camelot_filter = clicked_k;
                }

                auto& dk1 = engine.decks[left_id];
                auto& dk2 = engine.decks[right_id];
                bool d1_loaded = !dk1.track_title.empty() && dk1.track_title != "No Track" && dk1.track_title != "Empty";
                bool d2_loaded = !dk2.track_title.empty() && dk2.track_title != "No Track" && dk2.track_title != "Empty";
                KuroAudio::CamelotKey d1_ck = KuroAudio::DJHarmonicEngine::parseKey(dk1.getEffectiveKey());
                KuroAudio::CamelotKey d2_ck = KuroAudio::DJHarmonicEngine::parseKey(dk2.getEffectiveKey());
                KuroAudio::CamelotKey hov_ck = KuroAudio::DJHarmonicEngine::parseKey(hovered_track_key.empty() ? side_drawer_ui.hovered_track_key : hovered_track_key);
                bool has_hover = ((!hovered_track_key.empty() || !side_drawer_ui.hovered_track_key.empty()) && hov_ck.valid && hover_anim_timer > 0.01f);
                float hov_glow = has_hover ? (0.6f + 0.4f * sinf(radar_pulse_phase * 2.2f)) : 1.0f;

                const int segs = 6;
                for (int h = 1; h <= 12; ++h) {
                    float mid_ang = -3.14159265f * 0.5f + (float)(h % 12) * angle_step;
                    float a0 = mid_ang - angle_step * 0.5f;
                    float a1 = mid_ang + angle_step * 0.5f;

                    // Cor Oficial Padrão do Camelot Wheel (1..12)
                    unsigned int wheel_c = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(h);
                    unsigned char rc = (wheel_c) & 0xFF;
                    unsigned char gc = (wheel_c >> 8) & 0xFF;
                    unsigned char bc = (wheel_c >> 16) & 0xFF;

                    // Anel Externo B (Major)
                    {
                        bool is_flt = (gh_camelot_filter == (std::to_string(h) + "B"));
                        bool is_hov_sec = (hov_ring == 'B' && hov_sector == h);
                        bool is_trk_hov = (has_hover && hov_ck.number == h && hov_ck.letter == 'B');
                        bool is_d1 = (d1_loaded && d1_ck.valid && d1_ck.number == h && d1_ck.letter == 'B');
                        bool is_d2 = (d2_loaded && d2_ck.valid && d2_ck.number == h && d2_ck.letter == 'B');

                        ImU32 fill_col = IM_COL32(rc, gc, bc, 185);
                        if (is_flt) fill_col = IM_COL32(255, 255, 255, 250);
                        else if (is_trk_hov || is_hov_sec) fill_col = IM_COL32(255, 235, 40, (int)(200.0f + 55.0f * hov_glow));
                        else if (is_d1 && is_d2) fill_col = IM_COL32(0, 235, 255, 240);
                        else if (is_d1) fill_col = IM_COL32(0, 220, 255, 230);
                        else if (is_d2) fill_col = IM_COL32(255, 140, 25, 230);

                        ImVec2 pts[segs * 2 + 2];
                        int pcnt = 0;
                        for (int s = 0; s <= segs; ++s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_mid, center.y + sinf(a) * r_mid);
                        }
                        for (int s = segs; s >= 0; --s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_outer, center.y + sinf(a) * r_outer);
                        }
                        dl_l->AddConvexPolyFilled(pts, pcnt, fill_col);
                        dl_l->AddPolyline(pts, pcnt, IM_COL32(14, 18, 26, 220), ImDrawFlags_Closed, 1.0f);

                        // Rótulo B
                        char lbl_b[8]; snprintf(lbl_b, sizeof(lbl_b), "%dB", h);
                        float r_lbl_b = (r_outer + r_mid) * 0.5f;
                        ImVec2 pos_b(center.x + cosf(mid_ang) * r_lbl_b, center.y + sinf(mid_ang) * r_lbl_b);
                        ImVec2 t_sz = f_micro->CalcTextSizeA(9.0f, FLT_MAX, 0.0f, lbl_b);
                        ImU32 tcol = (is_flt || is_trk_hov || is_hov_sec) ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 240);
                        dl_l->AddText(f_micro, 9.0f, ImVec2(pos_b.x - t_sz.x * 0.5f, pos_b.y - t_sz.y * 0.5f), tcol, lbl_b);
                    }

                    // Anel Interno A (Minor)
                    {
                        bool is_flt = (gh_camelot_filter == (std::to_string(h) + "A"));
                        bool is_hov_sec = (hov_ring == 'A' && hov_sector == h);
                        bool is_trk_hov = (has_hover && hov_ck.number == h && hov_ck.letter == 'A');
                        bool is_d1 = (d1_loaded && d1_ck.valid && d1_ck.number == h && d1_ck.letter == 'A');
                        bool is_d2 = (d2_loaded && d2_ck.valid && d2_ck.number == h && d2_ck.letter == 'A');

                        ImU32 fill_col = IM_COL32((int)(rc * 0.72f), (int)(gc * 0.72f), (int)(bc * 0.72f), 190);
                        if (is_flt) fill_col = IM_COL32(255, 255, 255, 250);
                        else if (is_trk_hov || is_hov_sec) fill_col = IM_COL32(255, 235, 40, (int)(200.0f + 55.0f * hov_glow));
                        else if (is_d1 && is_d2) fill_col = IM_COL32(0, 235, 255, 240);
                        else if (is_d1) fill_col = IM_COL32(0, 220, 255, 230);
                        else if (is_d2) fill_col = IM_COL32(255, 140, 25, 230);

                        ImVec2 pts[segs * 2 + 2];
                        int pcnt = 0;
                        for (int s = 0; s <= segs; ++s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_hub, center.y + sinf(a) * r_hub);
                        }
                        for (int s = segs; s >= 0; --s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_mid, center.y + sinf(a) * r_mid);
                        }
                        dl_l->AddConvexPolyFilled(pts, pcnt, fill_col);
                        dl_l->AddPolyline(pts, pcnt, IM_COL32(14, 18, 26, 220), ImDrawFlags_Closed, 1.0f);

                        // Rótulo A
                        char lbl_a[8]; snprintf(lbl_a, sizeof(lbl_a), "%dA", h);
                        float r_lbl_a = (r_mid + r_hub) * 0.5f;
                        ImVec2 pos_a(center.x + cosf(mid_ang) * r_lbl_a, center.y + sinf(mid_ang) * r_lbl_a);
                        ImVec2 t_sz = f_micro->CalcTextSizeA(9.0f, FLT_MAX, 0.0f, lbl_a);
                        ImU32 tcol = (is_flt || is_trk_hov || is_hov_sec) ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 240);
                        dl_l->AddText(f_micro, 9.0f, ImVec2(pos_a.x - t_sz.x * 0.5f, pos_a.y - t_sz.y * 0.5f), tcol, lbl_a);
                    }

                    // Pins de Deck 1 e Deck 2
                    if (d1_loaded && d1_ck.valid && d1_ck.number == h) {
                        float r_p1 = (d1_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                        ImVec2 p_pin1(center.x + cosf(mid_ang) * r_p1, center.y + sinf(mid_ang) * r_p1);
                        dl_l->AddCircleFilled(p_pin1, 6.0f, IM_COL32(0, 20, 32, 240));
                        dl_l->AddCircle(p_pin1, 6.0f, IM_COL32(0, 235, 255, 255), 16, 1.5f);
                        dl_l->AddText(ImVec2(p_pin1.x - 3.5f, p_pin1.y - 6.0f), IM_COL32(0, 255, 255, 255), "1");
                    }
                    if (d2_loaded && d2_ck.valid && d2_ck.number == h) {
                        float r_p2 = (d2_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                        ImVec2 p_pin2(center.x + cosf(mid_ang) * r_p2, center.y + sinf(mid_ang) * r_p2);
                        if (d1_loaded && d1_ck.valid && d1_ck.number == h && d1_ck.letter == d2_ck.letter) {
                            p_pin2.x += 4.0f;
                        }
                        dl_l->AddCircleFilled(p_pin2, 6.0f, IM_COL32(32, 16, 0, 240));
                        dl_l->AddCircle(p_pin2, 6.0f, IM_COL32(255, 140, 20, 255), 16, 1.5f);
                        dl_l->AddText(ImVec2(p_pin2.x - 3.5f, p_pin2.y - 6.0f), IM_COL32(255, 200, 50, 255), "2");
                    }
                }

                // Centro do Hub
                dl_l->AddCircleFilled(center, r_hub - 1.0f, IM_COL32(11, 15, 22, 255), 24);
                dl_l->AddCircle(center, r_hub - 1.0f, IM_COL32(32, 42, 58, 220), 24, 1.0f);

                // Chip de remoção do filtro Camelot (se ativo)
                if (!gh_camelot_filter.empty()) {
                    char cl_chip[32]; snprintf(cl_chip, sizeof(cl_chip), "FILTRO: %s ✕", gh_camelot_filter.c_str());
                    ImGui::SetCursorScreenPos(ImVec2(center.x - 38.0f, center.y + r_outer + 6.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.65f, 0.95f));
                    if (ImGui::SmallButton(cl_chip)) {
                        gh_camelot_filter = "";
                    }
                    ImGui::PopStyleColor();
                }
            }

            ImGui::EndChild(); // ##GHExplorerLeftPane
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 8.0f);

            // =========================================================================
            // 2. PAINEL CENTRAL & DIREITO: FILTROS SUPERIORES E SETLIST COM CARDS ABERTOS
            // =========================================================================
            ImGui::BeginChild("##GHSetlistRightPane", ImVec2(right_w, pane_h), false, ImGuiWindowFlags_NoScrollbar);
            ImDrawList* dl_r = ImGui::GetWindowDrawList();

            // -------------------------------------------------------------------------
            // 2.1. BARRA DE FILTROS SUPERIOR (LINHA 1: PILLS DE GÊNERO + SUGESTÕES)
            // -------------------------------------------------------------------------
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

            auto drawGenrePill = [&](const char* label, const char* filter_id) {
                bool is_active_pill = (gh_genre_pill == filter_id);
                if (is_active_pill) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.35f, 0.42f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.50f, 0.58f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.94f, 0.88f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.96f, 0.90f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.09f, 0.13f, 0.85f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.16f, 0.22f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.25f, 0.35f, 0.80f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.72f, 0.85f, 0.95f));
                }
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                if (ImGui::Button(label)) {
                    gh_genre_pill = filter_id;
                    if (std::string(filter_id) == "TODOS") lib_mgr.setActiveFolderId("all");
                    else if (std::string(filter_id) == "PROG PSY") lib_mgr.setActiveFolderId("prog_psy");
                    else if (std::string(filter_id) == "FULL-ON") lib_mgr.setActiveFolderId("psy_fullon");
                    else if (std::string(filter_id) == "TECH HOUSE") lib_mgr.setActiveFolderId("tech_house");
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(4);
                ImGui::SameLine(0, 6.0f);
            };

            drawGenrePill("[TODOS]", "TODOS");
            drawGenrePill("[PROG PSY]", "PROG PSY");
            drawGenrePill("[FULL-ON]", "FULL-ON");
            drawGenrePill("[TECH HOUSE]", "TECH HOUSE");
            drawGenrePill("[ACID MELODIC]", "ACID MELODIC");

            // Botão Dourado de Ação: ⚡ SUGESTÕES (TOM + BPM)
            float sug_w = 210.0f;
            ImGui::SameLine(right_w - sug_w - 6.0f);
            bool is_sug_active = (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_SUGGESTION);
            if (is_sug_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.30f, 0.05f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.40f, 0.08f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.76f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.85f, 0.20f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.10f, 0.05f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.20f, 0.06f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.65f, 0.10f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.78f, 0.15f, 1.0f));
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
            if (ImGui::Button(ICON_FA_BOLT " SUGESTÕES (TOM + BPM)", ImVec2(sug_w, 24.0f))) {
                if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_SUGGESTION) {
                    side_drawer_ui.sort_mode = DJSideDrawerUI::SORT_TITLE;
                } else {
                    side_drawer_ui.sort_mode = DJSideDrawerUI::SORT_SUGGESTION;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Classificar automaticamente a setlist por compatibilidade harmônica com o Deck no ar!");
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            ImGui::PopStyleVar(); // FrameRounding

            // -------------------------------------------------------------------------
            // 2.2. BARRA DE BUSCA, SLIDER DUPLO DE BPM E KEY-MATCHING (LINHA 2)
            // -------------------------------------------------------------------------
            ImGui::Dummy(ImVec2(0, 2.0f));
            ImGui::AlignTextToFramePadding();

            // Campo de Busca
            float search_w = std::clamp(right_w * 0.38f, 200.0f, 420.0f);
            ImGui::PushItemWidth(search_w);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.045f, 0.065f, 0.095f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.20f, 0.28f, 0.85f));
            if (ImGui::InputTextWithHint("##GHSearchBox", ICON_FA_MAGNIFYING_GLASS " Buscar faixa, artista, BPM...", search_query_buf, sizeof(search_query_buf))) {
            }
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();
            ImGui::PopItemWidth();

            if (strlen(search_query_buf) > 0) {
                ImGui::SameLine(0, 3.0f);
                if (ImGui::SmallButton("✕")) {
                    search_query_buf[0] = '\0';
                }
            }

            // Widget Slider Duplo de Range de BPM (120 - 150 BPM)
            ImGui::SameLine(0, 16.0f);
            {
                float slider_w = 120.0f;
                float slider_h = 20.0f;
                ImVec2 s_p0 = ImGui::GetCursorScreenPos();
                ImVec2 s_p1(s_p0.x + slider_w, s_p0.y + slider_h);

                ImGui::InvisibleButton("##GHBpmDualSlider", ImVec2(slider_w, slider_h));
                float mid_y = s_p0.y + slider_h * 0.5f;

                dl_r->AddLine(ImVec2(s_p0.x, mid_y), ImVec2(s_p1.x, mid_y), IM_COL32(35, 45, 62, 255), 3.0f);

                float bpm_min_scale = 80.0f;
                float bpm_max_scale = 180.0f;
                float norm_min = std::clamp((gh_bpm_filter_min - bpm_min_scale) / (bpm_max_scale - bpm_min_scale), 0.0f, 1.0f);
                float norm_max = std::clamp((gh_bpm_filter_max - bpm_min_scale) / (bpm_max_scale - bpm_min_scale), 0.0f, 1.0f);

                float x_min_thumb = s_p0.x + norm_min * slider_w;
                float x_max_thumb = s_p0.x + norm_max * slider_w;

                dl_r->AddLine(ImVec2(x_min_thumb, mid_y), ImVec2(x_max_thumb, mid_y), IM_COL32(0, 230, 240, 255), 3.5f);

                if (ImGui::IsItemActive() && ImGui::IsMouseDown(0)) {
                    float mouse_norm = std::clamp((ImGui::GetIO().MousePos.x - s_p0.x) / slider_w, 0.0f, 1.0f);
                    float target_bpm = bpm_min_scale + mouse_norm * (bpm_max_scale - bpm_min_scale);
                    float d_min = std::abs(target_bpm - gh_bpm_filter_min);
                    float d_max = std::abs(target_bpm - gh_bpm_filter_max);
                    if (d_min < d_max) {
                        gh_bpm_filter_min = std::clamp(target_bpm, bpm_min_scale, gh_bpm_filter_max - 2.0f);
                    } else {
                        gh_bpm_filter_max = std::clamp(target_bpm, gh_bpm_filter_min + 2.0f, bpm_max_scale);
                    }
                }

                dl_r->AddCircleFilled(ImVec2(x_min_thumb, mid_y), 6.5f, IM_COL32(0, 240, 235, 255));
                dl_r->AddCircleFilled(ImVec2(x_min_thumb, mid_y), 3.0f, IM_COL32(10, 15, 25, 255));

                dl_r->AddCircleFilled(ImVec2(x_max_thumb, mid_y), 6.5f, IM_COL32(0, 240, 235, 255));
                dl_r->AddCircleFilled(ImVec2(x_max_thumb, mid_y), 3.0f, IM_COL32(10, 15, 25, 255));

                ImGui::SameLine(0, 6.0f);
                char bpm_range_txt[32];
                snprintf(bpm_range_txt, sizeof(bpm_range_txt), "%.0f — %.0f BPM", gh_bpm_filter_min, gh_bpm_filter_max);
                ImGui::TextColored(ImVec4(0.85f, 0.92f, 0.98f, 1.0f), "%s", bpm_range_txt);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clique duas vezes para resetar o filtro de BPM para 120-150 BPM");
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    gh_bpm_filter_min = 120.0f;
                    gh_bpm_filter_max = 150.0f;
                }
            }

            // Switch Key-matching
            ImGui::SameLine(0, 16.0f);
            ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.88f, 1.0f), "Key-matching");
            ImGui::SameLine(0, 6.0f);
            {
                ImVec2 sw_p0 = ImGui::GetCursorScreenPos();
                float sw_w = 34.0f;
                float sw_h = 18.0f;
                ImGui::InvisibleButton("##GHKeyMatchingSwitch", ImVec2(sw_w, sw_h));
                if (ImGui::IsItemClicked(0)) {
                    gh_key_matching_only = !gh_key_matching_only;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Key-matching: Filtrar faixas harmonicamente compatíveis");

                ImU32 sw_bg = gh_key_matching_only ? IM_COL32(0, 180, 200, 255) : IM_COL32(38, 48, 64, 255);
                dl_r->AddRectFilled(sw_p0, ImVec2(sw_p0.x + sw_w, sw_p0.y + sw_h), sw_bg, 9.0f);

                float knob_x = gh_key_matching_only ? (sw_p0.x + sw_w - 9.0f) : (sw_p0.x + 9.0f);
                dl_r->AddCircleFilled(ImVec2(knob_x, sw_p0.y + sw_h * 0.5f), 6.5f, IM_COL32(245, 250, 255, 255));
            }

            // -------------------------------------------------------------------------
            // 2.3. COLETA E FILTRAGEM DE FAIXAS DA SETLIST
            // -------------------------------------------------------------------------
            std::vector<KuroAudio::DJLibraryTrack> track_list;
            if (!selected_usb_drive.empty()) {
                track_list = usb_scanned_tracks;
            } else {
                track_list = lib_mgr.getTracksForFolder(active_fid);
                if (track_list.size() <= 1) {
                    lib_mgr.scanLocalTracks("scratch/tracks");
                    track_list = lib_mgr.getTracksForFolder(active_fid);
                }
                if (track_list.empty() && active_fid != "all") {
                    track_list = lib_mgr.getTracksForFolder("all");
                }
            }

            if (strlen(search_query_buf) > 0) {
                std::string q = search_query_buf;
                std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                track_list.erase(std::remove_if(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& trk) {
                    std::string tit = trk.title; std::transform(tit.begin(), tit.end(), tit.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    std::string art = trk.artist; std::transform(art.begin(), art.end(), art.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    std::string gen = trk.subgenre; std::transform(gen.begin(), gen.end(), gen.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    std::string key = trk.key; std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    char bpm_buf[16]; snprintf(bpm_buf, sizeof(bpm_buf), "%.0f", trk.bpm);
                    return (tit.find(q) == std::string::npos && 
                            art.find(q) == std::string::npos && 
                            gen.find(q) == std::string::npos && 
                            key.find(q) == std::string::npos && 
                            std::string(bpm_buf).find(q) == std::string::npos);
                }), track_list.end());
            }

            if (!gh_camelot_filter.empty()) {
                track_list.erase(std::remove_if(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& trk) {
                    KuroAudio::CamelotKey ck = KuroAudio::DJHarmonicEngine::parseKey(trk.key);
                    return ck.getCamelotCode() != gh_camelot_filter;
                }), track_list.end());
            }

            if (gh_key_matching_only && !ref_key.empty()) {
                track_list.erase(std::remove_if(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& trk) {
                    auto comp = KuroAudio::DJHarmonicEngine::compareKeys(ref_key, trk.key);
                    return !(comp == KuroAudio::HarmonicCompatibility::PERFECT ||
                             comp == KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE ||
                             comp == KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1 ||
                             comp == KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1);
                }), track_list.end());
            }

            if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_SUGGESTION) {
                std::stable_sort(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    auto comp_a = KuroAudio::DJHarmonicEngine::compareKeys(ref_key, a.key);
                    auto comp_b = KuroAudio::DJHarmonicEngine::compareKeys(ref_key, b.key);
                    int score_a = (comp_a == KuroAudio::HarmonicCompatibility::PERFECT) ? 4 :
                                  ((comp_a == KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE) ? 3 :
                                  ((comp_a == KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1) ? 2 :
                                  ((comp_a == KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1) ? 1 : 0)));
                    int score_b = (comp_b == KuroAudio::HarmonicCompatibility::PERFECT) ? 4 :
                                  ((comp_b == KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE) ? 3 :
                                  ((comp_b == KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1) ? 2 :
                                  ((comp_b == KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1) ? 1 : 0)));
                    if (score_a != score_b) return score_a > score_b;
                    double diff_a = std::abs(a.bpm - ref_bpm);
                    double diff_b = std::abs(b.bpm - ref_bpm);
                    return diff_a < diff_b;
                });
            }

            // Navegação e Atalhos de Teclado
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                    selected_track_idx = std::max(0, selected_track_idx - 1);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                    selected_track_idx = std::min((int)track_list.size() - 1, selected_track_idx + 1);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Space) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    library_view_mode = LIB_VIEW_SPLIT;
                }
                if (ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                    if (selected_track_idx >= 0 && selected_track_idx < (int)track_list.size()) {
                        auto& sel_t = track_list[selected_track_idx];
                        if (sel_t.is_downloaded) spotify_service.loadLibraryTrack(sel_t, left_id, engine);
                        else spotify_service.downloadLibraryTrackAsync(sel_t, left_id, engine);
                    }
                }
                if (ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                    if (selected_track_idx >= 0 && selected_track_idx < (int)track_list.size()) {
                        auto& sel_t = track_list[selected_track_idx];
                        if (sel_t.is_downloaded) spotify_service.loadLibraryTrack(sel_t, right_id, engine);
                        else spotify_service.downloadLibraryTrackAsync(sel_t, right_id, engine);
                    }
                }
            }

            // -------------------------------------------------------------------------
            // 2.4. CABEÇALHO PROPORCIONAL DAS COLUNAS (ALINHADO À DIREITA COM PERFEIÇÃO)
            // -------------------------------------------------------------------------
            ImGui::Dummy(ImVec2(0, 4.0f));

            float card_w = right_w - 20.0f;
            float card_h = 92.0f; // Altura ideal para metadados + Waveform HD Rekordbox

            // Coordenadas das Colunas: Fixas à esquerda (#, Capa, Título) e Ancoradas à direita (BPM, Tom, Avaliação, Duração, Load)
            float x_idx    = 10.0f;
            float x_cov    = 38.0f;
            float x_tit    = 76.0f;
            float x_bpm    = std::max(card_w - 620.0f, 480.0f);
            float x_key    = std::max(card_w - 510.0f, 580.0f);
            float x_rat    = std::max(card_w - 380.0f, 700.0f);
            float x_dur    = std::max(card_w - 270.0f, 810.0f);
            float x_load   = std::max(card_w - 200.0f, 880.0f);

            // Barra de cabeçalho
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.040f, 0.052f, 0.075f, 0.95f));
            ImGui::BeginChild("##GHSetlistHeaderBar", ImVec2(right_w, 24.0f), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImVec2 h_p0 = ImGui::GetCursorScreenPos();
                ImDrawList* dl_h = ImGui::GetWindowDrawList();
                ImU32 col_hdr_txt = IM_COL32(130, 150, 178, 230);

                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_idx,  h_p0.y + 4.0f), col_hdr_txt, "#");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_cov,  h_p0.y + 4.0f), col_hdr_txt, "CAPA");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_tit,  h_p0.y + 4.0f), col_hdr_txt, "TÍTULO & ARTISTA");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_bpm,  h_p0.y + 4.0f), col_hdr_txt, "BPM");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_key,  h_p0.y + 4.0f), col_hdr_txt, "KEY / TOM");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_rat,  h_p0.y + 4.0f), col_hdr_txt, "AVALIAÇÃO");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_dur,  h_p0.y + 4.0f), col_hdr_txt, "TEMPO");
                dl_h->AddText(f_small, 11.0f, ImVec2(h_p0.x + x_load, h_p0.y + 4.0f), col_hdr_txt, "CARREGAR DECK");
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            // -------------------------------------------------------------------------
            // 2.5. SETLIST VISUAL: TODAS AS FAIXAS ABERTAS COM WAVEFORMS (GUITAR HERO PRO)
            // -------------------------------------------------------------------------
            float scroll_h = ImGui::GetContentRegionAvail().y - 4.0f;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.020f, 0.028f, 0.042f, 0.95f));
            ImGui::BeginChild("##GHSetlistCardsScroll", ImVec2(right_w, scroll_h), true);
            ImDrawList* dl_cards = ImGui::GetWindowDrawList();

            for (int i = 0; i < (int)track_list.size(); ++i) {
                auto& t = track_list[i];
                ensureTrackWaveform(t);

                bool is_sel = (selected_track_idx == i || context_track_id == t.id);

                ImVec2 card_p0 = ImGui::GetCursorScreenPos();
                ImVec2 card_p1(card_p0.x + card_w, card_p0.y + card_h);

                ImGui::PushID(i * 1000);

                // Fundo e Borda do Card (Glow Neon Ciano na faixa selecionada)
                float pulse = 0.85f + 0.15f * sinf((float)ImGui::GetTime() * 4.5f);
                if (is_sel) {
                    dl_cards->AddRectFilled(card_p0, card_p1, IM_COL32(11, 18, 28, 255), 5.0f);
                    dl_cards->AddRect(card_p0, card_p1, IM_COL32(0, 240, 225, (int)(255 * pulse)), 5.0f, 0, 2.0f);
                    dl_cards->AddRect(ImVec2(card_p0.x - 1, card_p0.y - 1), ImVec2(card_p1.x + 1, card_p1.y + 1), IM_COL32(0, 200, 220, (int)(65 * pulse)), 6.0f, 0, 1.2f);
                } else {
                    dl_cards->AddRectFilled(card_p0, card_p1, IM_COL32(8, 12, 18, 235), 4.0f);
                    dl_cards->AddRect(card_p0, card_p1, IM_COL32(24, 34, 48, 190), 4.0f, 0, 1.0f);
                }

                // ---------------------------------------------------------------------
                // LINHA SUPERIOR DO CARD: METADADOS E BOTÕES LOAD (Y = card_p0.y + 6px)
                // ---------------------------------------------------------------------
                float top_y = card_p0.y + 6.0f;

                // Coluna 0: # / Chevron
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_idx, top_y + 2.0f));
                if (is_sel) {
                    ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.90f, 1.0f), ICON_FA_CHEVRON_DOWN);
                } else {
                    ImGui::TextColored(ImVec4(0.45f, 0.55f, 0.68f, 0.85f), "%d", i + 1);
                }

                // Coluna 1: Capa (Miniatura + Drag & Drop)
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_cov, top_y + 1.0f));
                GLuint tex_id = (GLuint)t.gl_tex_id;
                if (tex_id == 0 || (tex_id == 0xFFFFFFFF && !t.local_rgba_path.empty())) {
                    std::string rgba_p = t.local_rgba_path.empty() ? KuroAudio::DJLibraryManager::resolveRgbaCoverArt(t.local_wav_path, t.id) : t.local_rgba_path;
                    if (!rgba_p.empty()) {
                        tex_id = KuroAudio::SpotifyDJService::UploadRGBATexture(rgba_p);
                        t.gl_tex_id = (tex_id != 0) ? (unsigned int)tex_id : 0xFFFFFFFF;
                    } else {
                        t.gl_tex_id = 0xFFFFFFFF;
                    }
                }
                if (tex_id != 0 && tex_id != 0xFFFFFFFF) {
                    ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(22, 22));
                } else {
                    ImGui::TextColored(ImVec4(0.00f, 0.85f, 0.95f, 1.0f), ICON_FA_COMPACT_DISC);
                }
                // Drag & Drop na Capa
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("DND_DJ_TRACK_ID", t.id.c_str(), t.id.size() + 1);
                    ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                    ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "• %s (%.1f BPM - %s)", t.artist.c_str(), t.bpm, t.key.c_str());
                    ImGui::EndDragDropSource();
                }

                // Coluna 2: Título & Artista (Clicável + Arrastável + Menu de Contexto)
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_tit, top_y));
                float title_max_w = x_bpm - x_tit - 16.0f;
                std::string title_btn_id = "##GHTitleSel_" + t.id;
                bool clicked_row = ImGui::Selectable(title_btn_id.c_str(), is_sel, ImGuiSelectableFlags_AllowOverlap, ImVec2(title_max_w, 24.0f));
                if (clicked_row) {
                    selected_track_idx = i;
                    context_track_id = t.id;
                }

                // Drag & Drop no Título
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("DND_DJ_TRACK_ID", t.id.c_str(), t.id.size() + 1);
                    ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                    ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "• %s (%.1f BPM - %s)", t.artist.c_str(), t.bpm, t.key.c_str());
                    ImGui::EndDragDropSource();
                }

                // Duplo clique carrega no deck apropriado
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    int target_d = engine.decks[left_id].is_playing ? right_id : left_id;
                    if (t.is_downloaded) spotify_service.loadLibraryTrack(t, target_d, engine);
                    else spotify_service.downloadLibraryTrackAsync(t, target_d, engine);
                }

                // Hover do mouse alimenta o radar/roda Camelot
                if (ImGui::IsItemHovered()) {
                    hovered_track_id = t.id;
                    hovered_track_key = t.key;
                    hovered_track_bpm = t.bpm;
                    hovered_track_title = t.title;
                    hovered_track_artist = t.artist;
                    hover_anim_timer = 1.0f;
                }

                // Clique Direito no Título abre o menu de contexto
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    selected_track_idx = i;
                    context_track_id = t.id;
                    ImGui::OpenPopup(("GHTrackContext_" + t.id).c_str());
                }

                // Texto formatado: Título & Artista
                ImGui::SameLine(x_tit + 2.0f);
                ImGui::TextColored(is_sel ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.92f, 0.94f, 0.98f, 1.0f), "%s", t.title.c_str());
                ImGui::SameLine(0, 5.0f);
                ImGui::TextColored(is_sel ? ImVec4(0.00f, 0.88f, 0.96f, 0.95f) : ImVec4(0.55f, 0.68f, 0.82f, 0.95f), "• %s", t.artist.c_str());

                // Coluna 3: BPM
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_bpm, top_y + 2.0f));
                ImGui::TextColored(ImVec4(0.90f, 0.94f, 0.98f, 1.0f), "%.1f BPM", t.bpm);

                // Coluna 4: Pílula Oval Harmônica (Colorida Oficial por Tom Camelot)
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_key, top_y + 1.0f));
                KuroAudio::CamelotKey trk_ck = KuroAudio::DJHarmonicEngine::parseKey(t.key);
                unsigned int w_col = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(trk_ck.number);
                std::string mus_not = KuroAudio::DJHarmonicEngine::getRekordboxMusicalNotation(trk_ck.number, trk_ck.letter);
                char full_k_buf[32];
                snprintf(full_k_buf, sizeof(full_k_buf), "%s • %s", trk_ck.getCamelotCode().c_str(), mus_not.c_str());

                unsigned char rc_pill = (w_col) & 0xFF;
                unsigned char gc_pill = (w_col >> 8) & 0xFF;
                unsigned char bc_pill = (w_col >> 16) & 0xFF;

                ImVec2 pill_pos = ImGui::GetCursorScreenPos();
                float pill_w = 90.0f;
                float pill_h = 20.0f;
                dl_cards->AddRectFilled(pill_pos, ImVec2(pill_pos.x + pill_w, pill_pos.y + pill_h), IM_COL32(rc_pill / 5, gc_pill / 5, bc_pill / 5, 200), 10.0f);
                dl_cards->AddRect(pill_pos, ImVec2(pill_pos.x + pill_w, pill_pos.y + pill_h), IM_COL32(rc_pill, gc_pill, bc_pill, 240), 10.0f, 0, 1.2f);
                ImVec2 pk_sz = f_micro->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, full_k_buf);
                dl_cards->AddText(f_micro, 10.0f, ImVec2(pill_pos.x + (pill_w - pk_sz.x) * 0.5f, pill_pos.y + (pill_h - pk_sz.y) * 0.5f), IM_COL32(rc_pill, gc_pill, bc_pill, 255), full_k_buf);
                ImGui::Dummy(ImVec2(pill_w, pill_h));

                // Coluna 5: Avaliação
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_rat, top_y + 2.0f));
                for (int st = 1; st <= 5; st++) {
                    bool filled = (st <= (t.rating > 0 ? t.rating : 4));
                    ImGui::TextColored(filled ? ImVec4(1.0f, 0.82f, 0.15f, 1.0f) : ImVec4(0.35f, 0.40f, 0.48f, 0.7f), ICON_FA_STAR);
                    if (st < 5) ImGui::SameLine(0, 1.0f);
                }

                // Coluna 6: Duração
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_dur, top_y + 2.0f));
                int min = (int)t.duration_sec / 60;
                int sec = (int)t.duration_sec % 60;
                ImGui::TextColored(ImVec4(0.80f, 0.86f, 0.95f, 0.95f), "%02d:%02d", min, sec);

                // Coluna 7: 4 Botões LOAD [LOAD A] [LOAD B] [LOAD C] [LOAD D]
                ImGui::SetCursorScreenPos(ImVec2(card_p0.x + x_load, top_y));
                float btn_w = 46.0f;
                float btn_h = 22.0f;

                auto drawDeckBtn = [&](int d_idx, const char* letter, ImVec4 col) {
                    ImVec2 bp0 = ImGui::GetCursorScreenPos();
                    ImVec2 bp1(bp0.x + btn_w, bp0.y + btn_h);
                    char bid[32]; snprintf(bid, sizeof(bid), "##load_%d_%d", i, d_idx);
                    bool clicked = ImGui::InvisibleButton(bid, ImVec2(btn_w, btn_h));
                    bool is_hov = ImGui::IsItemHovered();
                    bool is_act = ImGui::IsItemActive();

                    ImU32 bg_c = is_act ? ImGui::ColorConvertFloat4ToU32(col) :
                                 (is_hov ? ImGui::ColorConvertFloat4ToU32(ImVec4(col.x * 0.45f, col.y * 0.45f, col.z * 0.45f, 0.95f)) :
                                           IM_COL32(14, 18, 25, 220));

                    ImU32 bd_c = is_hov ? ImGui::ColorConvertFloat4ToU32(col) : ImGui::ColorConvertFloat4ToU32(ImVec4(col.x * 0.85f, col.y * 0.85f, col.z * 0.85f, 0.9f));
                    ImU32 tx_c = is_hov ? IM_COL32(255, 255, 255, 255) : ImGui::ColorConvertFloat4ToU32(col);

                    dl_cards->AddRectFilled(bp0, bp1, bg_c, 3.5f);
                    dl_cards->AddRect(bp0, bp1, bd_c, 3.5f, 0, is_hov ? 1.8f : 1.2f);

                    char btxt[32]; snprintf(btxt, sizeof(btxt), "[LOAD %s]", letter);
                    ImVec2 t_sz = f_small->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, btxt);
                    dl_cards->AddText(f_small, 10.0f, ImVec2(bp0.x + (btn_w - t_sz.x) * 0.5f, bp0.y + (btn_h - t_sz.y) * 0.5f), tx_c, btxt);

                    if (clicked) {
                        if (t.is_downloaded) spotify_service.loadLibraryTrack(t, d_idx, engine);
                        else spotify_service.downloadLibraryTrackAsync(t, d_idx, engine);
                    }
                    if (is_hov) {
                        ImGui::SetTooltip("Carregar '%s' no Deck %s (Canal %d)", t.title.c_str(), letter, d_idx + 1);
                    }
                };

                drawDeckBtn(0, "A", ImVec4(0.00f, 0.88f, 1.00f, 1.0f));
                ImGui::SameLine(0, 3.0f);
                drawDeckBtn(1, "B", ImVec4(0.92f, 0.38f, 0.95f, 1.0f));
                ImGui::SameLine(0, 3.0f);
                drawDeckBtn(2, "C", ImVec4(1.00f, 0.65f, 0.12f, 1.0f));
                ImGui::SameLine(0, 3.0f);
                drawDeckBtn(3, "D", ImVec4(0.12f, 0.95f, 0.52f, 1.0f));

                // ---------------------------------------------------------------------
                // LINHA INFERIOR DO CARD: WAVEFORM HD 3-BAND VIBRANTE ESTILO IMAGEM 1 (Y = 36px)
                // ---------------------------------------------------------------------
                float wf_margin_x = 8.0f;
                ImVec2 wf_p0(card_p0.x + wf_margin_x, card_p0.y + 36.0f);
                ImVec2 wf_p1(card_p1.x - wf_margin_x, card_p0.y + 80.0f);
                float wf_w = wf_p1.x - wf_p0.x;
                float wf_h = wf_p1.y - wf_p0.y; // 44px de altura

                dl_cards->AddRectFilled(wf_p0, wf_p1, IM_COL32(7, 10, 15, 255), 3.0f);
                dl_cards->AddRect(wf_p0, wf_p1, IM_COL32(24, 34, 48, 180), 3.0f, 0, 1.0f);

                float base_y = wf_p1.y - 2.0f; // Linha de base no fundo da caixa de waveform
                float max_amp = wf_h - 4.0f;   // 40px de amplitude (nunca atinge a linha superior de texto!)
                bool is_prev = (touch_cue.isTouchPreviewActive() && touch_cue.getPreviewTrackId() == t.id);
                float prog = is_prev ? (float)touch_cue.getPreviewProgress() : 0.0f;

                // Renderização das fatias de áudio 3-Band padrão Imagem 1 (Low=Vermelho, Mid=Verde, High=Ciano)
                size_t sz = t.overview_wave.size();
                if (sz > 0) {
                    for (float px = wf_p0.x + 1.0f; px < wf_p1.x - 1.0f; px += 1.0f) {
                        float norm_x = (px - wf_p0.x) / wf_w;
                        size_t idx = (size_t)std::clamp(norm_x * (float)sz, 0.0f, (float)(sz - 1));
                        const auto& pt = t.overview_wave[idx];
                        float dim = (is_prev && norm_x < prog) ? 0.45f : 1.0f;

                        float h_tot = std::clamp(pt.peak * max_amp, 1.5f, max_amp);
                        float h_l   = std::clamp(pt.low * max_amp, 1.0f, h_tot);
                        float h_m   = std::clamp(pt.mid * max_amp, 1.0f, h_tot);

                        // 1. Camada de Agudos/Transientes (Ciano Elétrico no topo)
                        dl_cards->AddLine(ImVec2(px, base_y), ImVec2(px, base_y - h_tot), IM_COL32((int)(0 * dim), (int)(220 * dim), (int)(255 * dim), 245), 1.0f);
                        // 2. Camada de Médios (Verde Esmeralda)
                        if (h_m > 1.2f) {
                            dl_cards->AddLine(ImVec2(px, base_y), ImVec2(px, base_y - h_m), IM_COL32((int)(0 * dim), (int)(230 * dim), (int)(115 * dim), 240), 1.0f);
                        }
                        // 3. Camada de Graves (Vermelho Kick)
                        if (h_l > 1.0f) {
                            dl_cards->AddLine(ImVec2(px, base_y), ImVec2(px, base_y - h_l), IM_COL32((int)(255 * dim), (int)(45 * dim), (int)(75 * dim), 250), 1.0f);
                        }
                    }
                }

                // Marcadores CUE
                auto drawCueMarker = [&](float norm_pos, ImU32 col, bool is_top) {
                    float cx = wf_p0.x + norm_pos * wf_w;
                    if (is_top) {
                        dl_cards->AddLine(ImVec2(cx, wf_p0.y), ImVec2(cx, wf_p1.y), col, 1.2f);
                        dl_cards->AddCircleFilled(ImVec2(cx, wf_p0.y + 4.0f), 3.5f, col);
                    } else {
                        dl_cards->AddTriangleFilled(ImVec2(cx - 3.5f, wf_p1.y), ImVec2(cx + 3.5f, wf_p1.y), ImVec2(cx, wf_p1.y - 5.0f), col);
                    }
                };
                drawCueMarker(0.18f, IM_COL32(0, 220, 255, 230), true);
                drawCueMarker(0.38f, IM_COL32(245, 60, 60, 230), false);
                drawCueMarker(0.58f, IM_COL32(30, 230, 120, 230), false);
                drawCueMarker(0.78f, IM_COL32(0, 200, 255, 230), true);

                // Agulha de Reprodução / Preview
                if (is_prev && prog > 0.0f) {
                    float needle_x = wf_p0.x + prog * wf_w;
                    dl_cards->AddLine(ImVec2(needle_x, wf_p0.y), ImVec2(needle_x, wf_p1.y), IM_COL32(255, 255, 255, 255), 2.0f);
                    dl_cards->AddTriangleFilled(ImVec2(needle_x - 3.5f, wf_p0.y), ImVec2(needle_x + 3.5f, wf_p0.y), ImVec2(needle_x, wf_p0.y + 6.0f), IM_COL32(0, 240, 225, 255));
                }

                // Sub-barra Macro de Energia sob a Waveform
                float sub_y0 = wf_p1.y + 1.0f;
                float sub_y1 = sub_y0 + 5.0f;
                float x_b1 = wf_p0.x + wf_w * 0.22f;
                float x_b2 = wf_p0.x + wf_w * 0.58f;
                float x_b3 = wf_p0.x + wf_w * 0.85f;

                dl_cards->AddRectFilled(ImVec2(wf_p0.x, sub_y0), ImVec2(x_b1, sub_y1), IM_COL32(200, 35, 60, 220), 1.0f);
                dl_cards->AddRectFilled(ImVec2(x_b1, sub_y0), ImVec2(x_b2, sub_y1), IM_COL32(30, 200, 100, 220), 1.0f);
                dl_cards->AddRectFilled(ImVec2(x_b2, sub_y0), ImVec2(x_b3, sub_y1), IM_COL32(0, 160, 240, 220), 1.0f);
                dl_cards->AddRectFilled(ImVec2(x_b3, sub_y0), ImVec2(wf_p1.x, sub_y1), IM_COL32(0, 220, 240, 220), 1.0f);

                // ---------------------------------------------------------------------
                // INTERATIVIDADE DA WAVEFORM: TOUCH PREVIEW + DRAG + MENU DE CONTEXTO
                // ---------------------------------------------------------------------
                ImGui::SetCursorScreenPos(wf_p0);
                std::string wf_btn_id = "##GHWfScrub_" + t.id;
                ImGui::InvisibleButton(wf_btn_id.c_str(), ImVec2(wf_w, wf_h + 6.0f));

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("• Clique Esquerdo: Pré-escutar no fone\n• Clique Direito: Opções da Faixa\n• Arraste: Enviar para o Deck");
                }

                // Clique esquerdo: Touch Preview
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    selected_track_idx = i;
                    context_track_id = t.id;
                    float click_x = ImGui::GetIO().MousePos.x - wf_p0.x;
                    float click_norm = std::clamp(click_x / wf_w, 0.0f, 1.0f);
                    std::string src = t.local_wav_path.empty() ? t.preview_url : t.local_wav_path;
                    touch_cue.startTouchPreview(src, t.id, t.title, (double)click_norm);
                }

                // Clique direito na Waveform abre o menu de contexto
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    selected_track_idx = i;
                    context_track_id = t.id;
                    ImGui::OpenPopup(("GHTrackContext_" + t.id).c_str());
                }

                // Drag & Drop na Waveform
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("DND_DJ_TRACK_ID", t.id.c_str(), t.id.size() + 1);
                    ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                    ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "• %s (%.1f BPM - %s)", t.artist.c_str(), t.bpm, t.key.c_str());
                    ImGui::EndDragDropSource();
                }

                // ---------------------------------------------------------------------
                // MECÂNICA FUNDAMENTAL: MENU DE CONTEXTO DO BOTÃO DIREITO NA FAIXA
                // ---------------------------------------------------------------------
                if (ImGui::BeginPopup(("GHTrackContext_" + t.id).c_str())) {
                    selected_track_idx = i;
                    context_track_id = t.id;

                    ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                    ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.85f, 1.0f), "• %s (%s - %.1f BPM)", t.artist.c_str(), t.key.c_str(), t.bpm);
                    ImGui::Separator();

                    // 1. Carregar nos Decks A, B, C, D
                    auto doLoad = [&](int target_d) {
                        if (t.is_downloaded) spotify_service.loadLibraryTrack(t, target_d, engine);
                        else spotify_service.downloadLibraryTrackAsync(t, target_d, engine);
                    };

                    if (ImGui::MenuItem(ICON_FA_ARROW_DOWN " Carregar no Deck 1 (A)")) doLoad(0);
                    if (ImGui::MenuItem(ICON_FA_ARROW_DOWN " Carregar no Deck 2 (B)")) doLoad(1);
                    if (ImGui::MenuItem(ICON_FA_ARROW_DOWN " Carregar no Deck 3 (C)")) doLoad(2);
                    if (ImGui::MenuItem(ICON_FA_ARROW_DOWN " Carregar no Deck 4 (D)")) doLoad(3);

                    ImGui::Separator();

                    // 2. Preview no Fone de Ouvido
                    if (ImGui::MenuItem(ICON_FA_HEADPHONES " Ouvir no Fone (Touch Preview)")) {
                        std::string src = t.local_wav_path.empty() ? t.preview_url : t.local_wav_path;
                        touch_cue.startTouchPreview(src, t.id, t.title);
                    }

                    // 3. Editar Metadados (F2)
                    if (ImGui::MenuItem(ICON_FA_PEN_TO_SQUARE " Editar Metadados (F2)")) {
                        strncpy(edit_title_buf, t.title.c_str(), sizeof(edit_title_buf) - 1);
                        strncpy(edit_artist_buf, t.artist.c_str(), sizeof(edit_artist_buf) - 1);
                        edit_bpm = t.bpm;
                        strncpy(edit_key_buf, t.key.c_str(), sizeof(edit_key_buf) - 1);
                        strncpy(edit_subgenre_buf, t.subgenre.c_str(), sizeof(edit_subgenre_buf) - 1);
                        show_edit_meta_modal = true;
                    }

                    // 4. Copiar Faixa (Ctrl+C)
                    if (ImGui::MenuItem(ICON_FA_COPY " Copiar Faixa (Ctrl+C)")) {
                        clipboard_track = t;
                        has_clipboard_track = true;
                    }

                    // 5. Colar Faixa (Ctrl+V)
                    if (has_clipboard_track) {
                        std::string paste_txt = ICON_FA_PASTE " Colar Faixa Copiada: " + clipboard_track.title;
                        if (ImGui::MenuItem(paste_txt.c_str())) {
                            clipboard_track.folder_id = lib_mgr.getActiveFolderId();
                            lib_mgr.addOrUpdateTrack(clipboard_track);
                        }
                    }

                    // 6. Duplicar Faixa
                    if (ImGui::MenuItem(ICON_FA_CLONE " Duplicar Faixa")) {
                        lib_mgr.duplicateTrack(t.id);
                    }

                    // 7. Excluir da Biblioteca (Del)
                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Excluir da Biblioteca (Del)")) {
                        show_delete_confirm_modal = true;
                    }

                    ImGui::Separator();

                    // 8. Trocar Fonte / Versão Limpa no Kuro Cloud
                    if (ImGui::MenuItem(ICON_FA_SATELLITE_DISH " Trocar Fonte / Versão Limpa (Kuro Cloud)")) {
                        spotify_service.searchSourcesAsync(t.id, t.title, t.artist);
                    }

                    // 9. Mostrar no Windows Explorer
                    if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Mostrar no Windows Explorer")) {
                        revealInWindowsExplorer(t.local_wav_path);
                    }

                    ImGui::Separator();

                    // 10. Mover para Pasta
                    if (ImGui::BeginMenu("Mover para Pasta")) {
                        for (const auto& fo : folders) {
                            if (ImGui::MenuItem(fo.name.c_str())) {
                                lib_mgr.moveTrackToFolder(t.id, fo.id);
                            }
                        }
                        ImGui::EndMenu();
                    }

                    // 11. Alterar Vertente
                    if (ImGui::BeginMenu("Alterar Vertente")) {
                        const char* genres[] = { "Progressive Psytrance", "Psytrance / Full-On", "Tech House", "Acid Techno", "Melodic Techno" };
                        for (const char* g : genres) {
                            if (ImGui::MenuItem(g)) {
                                lib_mgr.setTrackSubgenre(t.id, g);
                            }
                        }
                        ImGui::EndMenu();
                    }

                    // 12. Classificação / Rating
                    if (ImGui::BeginMenu("Classificação (Rating)")) {
                        for (int st = 1; st <= 5; st++) {
                            char st_lbl[32];
                            snprintf(st_lbl, sizeof(st_lbl), "%d Estrelas (%s)", st, std::string(st, '*').c_str());
                            if (ImGui::MenuItem(st_lbl, nullptr, t.rating == st)) {
                                t.rating = st;
                                lib_mgr.setTrackRating(t.id, st);
                            }
                        }
                        if (ImGui::MenuItem("Limpar Rating", nullptr, t.rating == 0)) {
                            t.rating = 0;
                            lib_mgr.setTrackRating(t.id, 0);
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopID();

                // Espaçamento limpo entre cards
                ImGui::Dummy(ImVec2(card_w, 6.0f));
            }

            // Menu de Contexto ao Clicar com Botão Direito na Área Vazia da Tabela
            if (ImGui::BeginPopupContextWindow("BibliotecaWindowContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
                ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), ICON_FA_BOOK " BIBLIOTECA MUSICAL");
                ImGui::Separator();
                if (has_clipboard_track) {
                    std::string paste_txt = ICON_FA_PASTE " Colar Faixa: " + clipboard_track.title + " (Ctrl+V)";
                    if (ImGui::MenuItem(paste_txt.c_str())) {
                        clipboard_track.folder_id = lib_mgr.getActiveFolderId();
                        lib_mgr.addOrUpdateTrack(clipboard_track);
                    }
                } else {
                    ImGui::TextDisabled(ICON_FA_PASTE " Colar Faixa (Nenhuma copiada)");
                }
                if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS " Nova Pasta na Biblioteca")) {
                    show_new_folder_dialog = true;
                }
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Abrir Pasta de Músicas no Explorer")) {
                    revealInWindowsExplorer("");
                }
                ImGui::EndPopup();
            }

            ImGui::EndChild(); // ##GHSetlistCardsScroll
            ImGui::PopStyleColor();

            ImGui::EndChild(); // ##GHSetlistRightPane

            ImGui::EndChild(); // ##GHBigLibraryRoot
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }

        // =========================================================================
        // BIBLIOTECA MUSICAL REKORDBOX & BIG LIBRARY (MIXXX + ABDUCTION STUDIO)
        // =========================================================================
        inline void KuroDJStudioUI::renderPlaylistSpotifyBar(KuroAudio::DJEngine& engine, bool is_big_library = false, float custom_h = 0.0f) {
            float avail_w = ImGui::GetContentRegionAvail().x;
            float drawer_h = (custom_h > 0.0f) ? custom_h : KuroUI::UIScaleManager::Scale(library_height);
            if (is_big_library) {
                renderBigLibraryGuitarHero(engine, avail_w, drawer_h);
                return;
            }
            auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
            static std::string camelot_filter_key = "";
            
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.035f, 0.045f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.18f, 0.28f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("##SpotifyPlaylistDrawer", ImVec2(avail_w, drawer_h), true);


            auto folders = KuroAudio::DJLibraryManager::getInstance().getFolders();
            std::string active_fid = KuroAudio::DJLibraryManager::getInstance().getActiveFolderId();
            std::string active_folder_name = "Todas as Músicas";
            int active_count = 0;
            for (const auto& fo : folders) {
                if (fo.id == active_fid) {
                    active_folder_name = fo.name;
                    active_count = fo.track_count;
                    break;
                }
            }

            // =========================================================================
            // 1. CABEÇALHO SUPERIOR DA BIBLIOTECA (TÍTULO, STATUS, ALTURA & AÇÕES)
            // =========================================================================
            ImGui::AlignTextToFramePadding();
            if (is_big_library) {
                ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_BOOK " BIG LIBRARY // ABDUCTION STUDIO");
                ImGui::SameLine(0, 8.0f);
                char badge_str[128];
                snprintf(badge_str, sizeof(badge_str), "[ %s • %d faixas ]", active_folder_name.c_str(), active_count);
                ImGui::TextColored(ImVec4(0.40f, 0.78f, 1.00f, 0.95f), "%s", badge_str);

                ImGui::SameLine(avail_w - 380.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.55f, 0.90f));
                if (ImGui::SmallButton(ICON_FA_COMPRESS " MODO SPLIT (ESC / ESPAÇO)")) {
                    library_view_mode = LIB_VIEW_SPLIT;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Retornar à visualização padrão de decks com biblioteca no rodapé");
                ImGui::PopStyleColor();
            } else {
                ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_BOOK " BIBLIOTECA");
                ImGui::SameLine(0, 8.0f);
                char badge_str[128];
                snprintf(badge_str, sizeof(badge_str), "[ %s • %d faixas ]", active_folder_name.c_str(), active_count);
                ImGui::TextColored(ImVec4(0.40f, 0.78f, 1.00f, 0.95f), "%s", badge_str);

                // Seletor de Altura da Biblioteca (200px, 250px, 300px) e Botão Modal
                ImGui::SameLine(avail_w - 520.0f);
                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.78f, 0.9f), "Altura:");
                ImGui::SameLine(0, 4.0f);
                const float lib_h_presets[] = { 200.0f, 250.0f, 300.0f };
                const char* lib_h_labels[] = { "200px", "250px", "300px" };
                for (int lhi = 0; lhi < 3; lhi++) {
                    bool is_sel = (std::abs(library_height - lib_h_presets[lhi]) < 2.0f);
                    if (is_sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.50f, 0.58f, 0.95f));
                    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 0.85f));
                    if (ImGui::SmallButton(lib_h_labels[lhi])) {
                        library_height = lib_h_presets[lhi];
                    }
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0, 3.0f);
                }

                ImGui::SameLine(0, 8.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.65f, 0.55f, 0.90f));
                if (ImGui::SmallButton(ICON_FA_EXPAND " ⛶ BIG LIBRARY (ESPAÇO)")) {
                    library_view_mode = LIB_VIEW_BIG_LIBRARY;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Expandir para Modo Big Library com mini-decks de topo (estilo Mixxx)");
                ImGui::PopStyleColor();
            }

            // Ações: Importar Pasta, Importar Arquivo e Nova Pasta
            ImGui::SameLine(0, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.30f, 0.40f, 0.85f));
            if (ImGui::SmallButton(ICON_FA_FOLDER_PLUS " + PASTA")) {
                std::string folder_path = KuroUI::FileDialog::SelectFolder("Selecionar Pasta de Músicas");
                if (!folder_path.empty()) {
                    lib_mgr.scanLocalTracks(folder_path);
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Importar pasta inteira com músicas (recursivo)");
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.35f, 0.45f, 0.85f));
            if (ImGui::SmallButton(ICON_FA_FILE_AUDIO " + ARQUIVO")) {
                std::string local_path = KuroUI::FileDialog::OpenFile("Audio Files (*.wav;*.mp3;*.flac;*.ogg)\0*.wav;*.mp3;*.flac;*.ogg\0All Files (*.*)\0*.*\0");
                if (!local_path.empty()) {
                    spotify_service.importLocalTrack(local_path);
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Importar arquivo local de áudio para a Biblioteca");
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.24f, 0.35f, 0.85f));
            if (ImGui::SmallButton(ICON_FA_PLUS " NOVA PASTA")) {
                show_new_folder_dialog = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Criar nova pasta / categoria personalizada");
            ImGui::PopStyleColor();

            ImGui::Separator();

            int left_id = ddj_hardware.getActiveLeftDeck();
            int right_id = ddj_hardware.getActiveRightDeck();
            std::string ref_key = engine.decks[left_id].is_playing.load() ? engine.decks[left_id].key_signature : 
                                 (engine.decks[right_id].is_playing.load() ? engine.decks[right_id].key_signature : engine.decks[left_id].key_signature);
            double ref_bpm = engine.decks[left_id].is_playing.load() ? (engine.decks[left_id].bpm > 20.0 ? engine.decks[left_id].bpm : 138.0) : 
                            (engine.decks[right_id].is_playing.load() ? (engine.decks[right_id].bpm > 20.0 ? engine.decks[right_id].bpm : 138.0) : 
                            (engine.decks[left_id].bpm > 20.0 ? engine.decks[left_id].bpm : 138.0));

            float left_w = is_big_library ? 270.0f : 240.0f;
            float right_w = avail_w - left_w - 14.0f;
            float pane_h = drawer_h - KuroUI::UIScaleManager::Scale(44.0f);
            if (pane_h < 120.0f) pane_h = 120.0f;

            // =========================================================================
            // 2. PAINEL ESQUERDO: MINI TURNTABLE PREVIEW, CRATES & RODA CAMELOT COMPLETA
            // =========================================================================
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.022f, 0.030f, 0.05f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.09f, 0.14f, 0.22f, 0.70f));
            ImGui::BeginChild("##CratesLeftPane", ImVec2(left_w, pane_h), true);

            // 2a. MINI TURNTABLE PREVIEW PLAYER (Estilo Mixxx + CDJ-3000)
            auto& touch_cue = KuroAudio::DJTouchPreviewCue::getInstance();
            bool is_prev_active = touch_cue.isTouchPreviewActive();
            std::string prev_tid = touch_cue.getPreviewTrackId();
            double prev_prog = touch_cue.getPreviewProgress();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.018f, 0.024f, 0.038f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, is_prev_active ? IM_COL32(0, 220, 255, 180) : IM_COL32(24, 34, 48, 160));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::BeginChild("##MiniTurntablePreviewPlatter", ImVec2(left_w - 14.0f, 54.0f), true, ImGuiWindowFlags_NoScrollbar);

            ImDrawList* dl_p = ImGui::GetWindowDrawList();
            ImVec2 platter_p = ImGui::GetCursorScreenPos();
            float platter_r = 15.0f;
            ImVec2 platter_c(platter_p.x + platter_r + 4.0f, platter_p.y + 22.0f);

            if (is_prev_active) {
                mini_preview_spin_angle += ImGui::GetIO().DeltaTime * 6.28318f;
            }

            // Disco de vinil giratório
            dl_p->AddCircleFilled(platter_c, platter_r, IM_COL32(10, 14, 20, 255));
            dl_p->AddCircle(platter_c, platter_r, is_prev_active ? IM_COL32(0, 220, 255, 220) : IM_COL32(36, 48, 64, 180), 24, 1.2f);
            dl_p->AddCircle(platter_c, platter_r * 0.70f, IM_COL32(18, 25, 35, 180), 20, 1.0f);
            dl_p->AddCircle(platter_c, platter_r * 0.40f, IM_COL32(28, 38, 52, 180), 16, 1.0f);
            dl_p->AddCircleFilled(platter_c, 3.5f, is_prev_active ? IM_COL32(0, 240, 255, 255) : IM_COL32(80, 95, 115, 200));

            // Agulha branca giratória
            dl_p->AddLine(platter_c, ImVec2(platter_c.x + cosf(mini_preview_spin_angle) * (platter_r - 2.0f),
                                            platter_c.y + sinf(mini_preview_spin_angle) * (platter_r - 2.0f)),
                          IM_COL32(255, 255, 255, is_prev_active ? 255 : 120), 1.5f);

            // Título e Controles de Pré-escuta
            float ctrl_x = platter_p.x + platter_r * 2.0f + 10.0f;
            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, platter_p.y + 2.0f));

            if (is_prev_active) {
                auto* pt = lib_mgr.getTrackById(prev_tid);
                std::string p_title = pt ? pt->title : "Pré-escuta Ativa";
                if (p_title.length() > 14) p_title = p_title.substr(0, 12) + "...";
                ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), ICON_FA_HEADPHONES " %s", p_title.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.75f, 0.9f), ICON_FA_HEADPHONES " PRÉ-ESCUTA");
            }

            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, platter_p.y + 20.0f));
            if (is_prev_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.25f, 0.90f));
                if (ImGui::SmallButton(ICON_FA_STOP " STOP")) {
                    touch_cue.stopTouchPreview();
                }
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.35f, 0.45f, 0.85f));
                if (ImGui::SmallButton(ICON_FA_PLAY " OUVIR")) {
                    auto trks = lib_mgr.getTracksForFolder(lib_mgr.getActiveFolderId());
                    if (selected_track_idx >= 0 && selected_track_idx < (int)trks.size()) {
                        auto& sel_t = trks[selected_track_idx];
                        std::string src = sel_t.local_wav_path.empty() ? sel_t.preview_url : sel_t.local_wav_path;
                        touch_cue.startTouchPreview(src, sel_t.id, sel_t.title);
                    }
                }
                ImGui::PopStyleColor();
            }

            // Barra de Scrub interativa do Preview
            float scrub_w = (left_w - 14.0f) - (ctrl_x - platter_p.x) - 48.0f;
            if (scrub_w > 30.0f) {
                ImGui::SameLine(0, 4.0f);
                ImVec2 sb_pos = ImGui::GetCursorScreenPos();
                float sb_h = 14.0f;
                ImGui::InvisibleButton("##MiniTurntableScrub", ImVec2(scrub_w, sb_h));
                if (ImGui::IsItemActive() && ImGui::IsMouseDown(0)) {
                    float click_norm = (ImGui::GetIO().MousePos.x - sb_pos.x) / scrub_w;
                    touch_cue.seekPreview(std::clamp((double)click_norm, 0.0, 1.0));
                }
                dl_p->AddRectFilled(sb_pos, ImVec2(sb_pos.x + scrub_w, sb_pos.y + sb_h), IM_COL32(12, 18, 26, 255), 2.0f);
                dl_p->AddRect(sb_pos, ImVec2(sb_pos.x + scrub_w, sb_pos.y + sb_h), IM_COL32(30, 45, 62, 200), 2.0f);
                if (prev_prog > 0.0) {
                    float fill_w = (float)prev_prog * scrub_w;
                    dl_p->AddRectFilled(sb_pos, ImVec2(sb_pos.x + fill_w, sb_pos.y + sb_h), IM_COL32(0, 220, 255, 220), 2.0f);
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            // 2b. SUB-PAINEL COM ROLAGEM PARA PASTAS & CRATES
            float folders_h = is_big_library ? std::clamp(pane_h * 0.28f, 90.0f, 180.0f) : std::clamp(pane_h - 235.0f, 40.0f, 80.0f);
            ImGui::BeginChild("##CratesFolderListScroll", ImVec2(0, folders_h), false);
            ImGui::TextColored(ImVec4(0.00f, 0.85f, 0.75f, 1.0f), ICON_FA_FOLDER_TREE " PASTAS & GÊNEROS");
            ImGui::Separator();

            // 2b-1. NAVEGADOR DE DISPOSITIVOS / PEN DRIVES USB
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
                        ImGui::SetTooltip("Unidade: %s\nClique para navegar pelas músicas deste pen drive", usb_dev.path.c_str());
                    }
                }
            }
            ImGui::Separator();

            for (const auto& f : folders) {
                bool is_active = (selected_usb_drive.empty() && f.id == active_fid);
                const char* icon = ICON_FA_FOLDER;
                ImVec4 f_col = ImVec4(0.80f, 0.85f, 0.92f, 1.0f);

                if (f.id == "all")             { icon = ICON_FA_MUSIC;             f_col = ImVec4(0.35f, 0.90f, 1.00f, 1.0f); }
                else if (f.id == "prog_psy")   { icon = ICON_FA_FIRE;              f_col = ImVec4(0.00f, 0.96f, 0.83f, 1.0f); }
                else if (f.id == "psy_fullon") { icon = ICON_FA_BOLT;              f_col = ImVec4(0.85f, 0.40f, 1.00f, 1.0f); }
                else if (f.id == "tech_house") { icon = ICON_FA_COMPACT_DISC;     f_col = ImVec4(1.00f, 0.75f, 0.20f, 1.0f); }
                else if (f.id == "acid_techno"){ icon = ICON_FA_SLIDERS;          f_col = ImVec4(0.25f, 1.00f, 0.45f, 1.0f); }
                else if (f.id == "melodic_techno"){ icon = ICON_FA_HEADPHONES;    f_col = ImVec4(0.40f, 0.75f, 1.00f, 1.0f); }
                else if (f.id == "cloud")      { icon = ICON_FA_CLOUD_ARROW_DOWN;  f_col = ImVec4(0.95f, 0.65f, 1.00f, 1.0f); }

                char label[128];
                snprintf(label, sizeof(label), "%s %s##%s", icon, f.name.c_str(), f.id.c_str());

                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.38f, 0.45f, 0.75f));
                    ImGui::PushStyleColor(ImGuiCol_Text, f_col);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.76f, 0.84f, 1.0f));
                }

                if (ImGui::Selectable(label, is_active, ImGuiSelectableFlags_SpanAllColumns)) {
                    KuroAudio::DJLibraryManager::getInstance().setActiveFolderId(f.id);
                }

                if (is_active) {
                    ImVec2 item_min = ImGui::GetItemRectMin();
                    ImVec2 item_max = ImGui::GetItemRectMax();
                    ImGui::GetWindowDrawList()->AddRectFilled(item_min, ImVec2(item_min.x + 3.0f, item_max.y), IM_COL32(0, 220, 255, 255));
                }

                ImGui::SameLine(left_w - 46.0f);
                ImGui::TextColored(ImVec4(0.42f, 0.52f, 0.65f, 0.9f), "%d", f.track_count);

                if (!f.is_system && ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Excluir Pasta")) {
                        KuroAudio::DJLibraryManager::getInstance().deleteFolder(f.id);
                    }
                    ImGui::EndPopup();
                }

                if (is_active) ImGui::PopStyleColor(2);
                else ImGui::PopStyleColor(1);
            }
            ImGui::EndChild();

            // 2c. DIVISOR E RODA CAMELOT COMPLETA COM MESMA FÓRMULA DO RADAR
            ImGui::Separator();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.88f, 0.96f, 1.0f), ICON_FA_COMPASS " RODA CAMELOT");
            if (!camelot_filter_key.empty()) {
                ImGui::SameLine(0, 6.0f);
                char fk_tag[32]; snprintf(fk_tag, sizeof(fk_tag), "[ %s ✕ ]", camelot_filter_key.c_str());
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.85f, 1.0f));
                if (ImGui::SmallButton(fk_tag)) {
                    camelot_filter_key = "";
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Filtro ativo: %s. Clique para remover filtro", camelot_filter_key.c_str());
                ImGui::PopStyleColor();
            }

            // Desenho do Disco Camelot Permanente & Reativo a Hover (Com Pins 1 e 2, Ponte Harmônica e Laser Beam)
            {
                float radar_canvas_h = is_big_library ? 130.0f : 100.0f;
                float r_outer = is_big_library ? 56.0f : 44.0f;
                float r_mid   = r_outer * 0.68f;
                float r_hub   = r_outer * 0.36f;
                ImVec2 radar_canvas_sz(left_w - 16.0f, radar_canvas_h);
                ImGui::Dummy(radar_canvas_sz);
                ImVec2 p0 = ImGui::GetItemRectMin();
                ImVec2 p1 = ImGui::GetItemRectMax();
                ImVec2 center = ImVec2((p0.x + p1.x) * 0.5f, p0.y + radar_canvas_h * 0.5f);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                const float angle_step = 2.0f * 3.14159265f / 12.0f;

                // Fundo do disco
                draw_list->AddCircleFilled(center, r_outer + 3.0f, IM_COL32(8, 12, 18, 250), 36);
                draw_list->AddCircle(center, r_outer + 3.0f, IM_COL32(28, 38, 52, 220), 36, 1.0f);

                // Mouse e Interatividade de Clique para filtrar faixas por tom
                ImVec2 m = ImGui::GetIO().MousePos;
                float m_dx = m.x - center.x;
                float m_dy = m.y - center.y;
                float m_dist = sqrtf(m_dx * m_dx + m_dy * m_dy);
                float m_ang = atan2f(m_dy, m_dx);

                float rel_ang = m_ang + 3.14159265f * 0.5f + angle_step * 0.5f;
                while (rel_ang < 0.0f) rel_ang += 2.0f * 3.14159265f;
                while (rel_ang >= 2.0f * 3.14159265f) rel_ang -= 2.0f * 3.14159265f;
                int hovered_sector = (int)(rel_ang / angle_step);
                if (hovered_sector == 0) hovered_sector = 12;

                char hovered_letter = '\0';
                if (m_dist >= r_mid && m_dist <= r_outer) hovered_letter = 'B';
                else if (m_dist >= r_hub && m_dist < r_mid) hovered_letter = 'A';

                if (hovered_letter != '\0' && ImGui::IsMouseClicked(0) && ImGui::IsItemHovered()) {
                    std::string clicked_key = std::to_string(hovered_sector) + hovered_letter;
                    if (camelot_filter_key == clicked_key) camelot_filter_key = "";
                    else camelot_filter_key = clicked_key;
                }

                auto& dk1 = engine.decks[left_id];
                auto& dk2 = engine.decks[right_id];
                bool d1_loaded = !dk1.track_title.empty() && dk1.track_title != "No Track" && dk1.track_title != "Empty";
                bool d2_loaded = !dk2.track_title.empty() && dk2.track_title != "No Track" && dk2.track_title != "Empty";
                KuroAudio::CamelotKey d1_ck = KuroAudio::DJHarmonicEngine::parseKey(dk1.getEffectiveKey());
                KuroAudio::CamelotKey d2_ck = KuroAudio::DJHarmonicEngine::parseKey(dk2.getEffectiveKey());
                KuroAudio::CamelotKey master_ck = KuroAudio::DJHarmonicEngine::parseKey(ref_key);
                KuroAudio::CamelotKey hov_ck = KuroAudio::DJHarmonicEngine::parseKey(hovered_track_key.empty() ? side_drawer_ui.hovered_track_key : hovered_track_key);
                bool has_hover = ((!hovered_track_key.empty() || !side_drawer_ui.hovered_track_key.empty()) && hov_ck.valid && hover_anim_timer > 0.01f);

                // Pulso do setor com hover
                float hov_glow = has_hover ? (0.6f + 0.4f * sinf(radar_pulse_phase * 2.2f)) : 1.0f;

                // 12 Setores Camelot (Major B externo, Minor A interno)
                for (int h = 1; h <= 12; ++h) {
                    float mid_ang = -3.14159265f * 0.5f + (float)(h % 12) * angle_step;
                    float a0 = mid_ang - angle_step * 0.5f;
                    float a1 = mid_ang + angle_step * 0.5f;

                    unsigned int wheel_c = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(h);
                    unsigned char rc = (wheel_c) & 0xFF;
                    unsigned char gc = (wheel_c >> 8) & 0xFF;
                    unsigned char bc = (wheel_c >> 16) & 0xFF;

                    // Anel Externo B (Maior)
                    {
                        bool is_hov = (has_hover && hov_ck.number == h && hov_ck.letter == 'B');
                        bool is_d1 = (d1_loaded && d1_ck.valid && d1_ck.number == h && d1_ck.letter == 'B');
                        bool is_d2 = (d2_loaded && d2_ck.valid && d2_ck.number == h && d2_ck.letter == 'B');
                        bool is_flt = (camelot_filter_key == (std::to_string(h) + "B"));

                        ImU32 fill_col = IM_COL32(rc, gc, bc, 130);
                        if (is_flt) fill_col = IM_COL32(255, 255, 255, 230);
                        else if (is_hov) fill_col = IM_COL32(255, 235, 40, (int)(200.0f + 55.0f * hov_glow));
                        else if (is_d1 && is_d2) fill_col = IM_COL32(0, 235, 255, 240);
                        else if (is_d1) fill_col = IM_COL32(0, 220, 255, 230);
                        else if (is_d2) fill_col = IM_COL32(255, 140, 25, 230);

                        const int segs = 6;
                        ImVec2 pts[segs * 2 + 2];
                        int pcnt = 0;
                        for (int s = 0; s <= segs; ++s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_mid, center.y + sinf(a) * r_mid);
                        }
                        for (int s = segs; s >= 0; --s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_outer, center.y + sinf(a) * r_outer);
                        }
                        draw_list->AddConvexPolyFilled(pts, pcnt, fill_col);
                        draw_list->AddPolyline(pts, pcnt, IM_COL32(15, 20, 28, 200), ImDrawFlags_Closed, 1.0f);

                        if (is_hov) {
                            draw_list->PathArcTo(center, r_outer + 2.5f, a0, a1, 6);
                            draw_list->PathStroke(IM_COL32(255, 220, 30, (int)(90.0f * hov_glow)), 0, 4.0f);
                        }

                        float r_text = (r_mid + r_outer) * 0.5f;
                        ImVec2 t_pos(center.x + cosf(mid_ang) * r_text, center.y + sinf(mid_ang) * r_text);
                        char buf_b[8]; snprintf(buf_b, sizeof(buf_b), "%dB", h);
                        ImVec2 t_sz = ImGui::CalcTextSize(buf_b);
                        draw_list->AddText(ImVec2(t_pos.x - t_sz.x * 0.5f, t_pos.y - t_sz.y * 0.5f), 
                                           is_hov ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,240), buf_b);
                    }

                    // Anel Interno A (Menor)
                    {
                        bool is_hov = (has_hover && hov_ck.number == h && hov_ck.letter == 'A');
                        bool is_d1 = (d1_loaded && d1_ck.valid && d1_ck.number == h && d1_ck.letter == 'A');
                        bool is_d2 = (d2_loaded && d2_ck.valid && d2_ck.number == h && d2_ck.letter == 'A');
                        bool is_flt = (camelot_filter_key == (std::to_string(h) + "A"));

                        ImU32 fill_col = IM_COL32(rc * 0.7f, gc * 0.7f, bc * 0.7f, 140);
                        if (is_flt) fill_col = IM_COL32(255, 255, 255, 230);
                        else if (is_hov) fill_col = IM_COL32(255, 235, 40, (int)(200.0f + 55.0f * hov_glow));
                        else if (is_d1 && is_d2) fill_col = IM_COL32(0, 235, 255, 240);
                        else if (is_d1) fill_col = IM_COL32(0, 220, 255, 230);
                        else if (is_d2) fill_col = IM_COL32(255, 140, 25, 230);

                        const int segs = 6;
                        ImVec2 pts[segs * 2 + 2];
                        int pcnt = 0;
                        for (int s = 0; s <= segs; ++s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_hub, center.y + sinf(a) * r_hub);
                        }
                        for (int s = segs; s >= 0; --s) {
                            float a = a0 + (a1 - a0) * ((float)s / (float)segs);
                            pts[pcnt++] = ImVec2(center.x + cosf(a) * r_mid, center.y + sinf(a) * r_mid);
                        }
                        draw_list->AddConvexPolyFilled(pts, pcnt, fill_col);
                        draw_list->AddPolyline(pts, pcnt, IM_COL32(15, 20, 28, 200), ImDrawFlags_Closed, 1.0f);

                        float r_text = (r_hub + r_mid) * 0.5f;
                        ImVec2 t_pos(center.x + cosf(mid_ang) * r_text, center.y + sinf(mid_ang) * r_text);
                        char buf_a[8]; snprintf(buf_a, sizeof(buf_a), "%dA", h);
                        ImVec2 t_sz = ImGui::CalcTextSize(buf_a);
                        draw_list->AddText(ImVec2(t_pos.x - t_sz.x * 0.5f, t_pos.y - t_sz.y * 0.5f), 
                                           is_hov ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,240), buf_a);
                    }

                    // PINS INDICADORES DE DECKS CARREGADOS (Deck 1 ciano '1', Deck 2 laranja '2')
                    if (d1_loaded && d1_ck.valid && d1_ck.number == h) {
                        bool is_b = (d1_ck.letter == 'B');
                        float r_p1 = is_b ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                        ImVec2 p_pin1(center.x + cosf(mid_ang) * r_p1, center.y + sinf(mid_ang) * r_p1);
                        draw_list->AddCircleFilled(p_pin1, 6.0f, IM_COL32(0, 20, 32, 240));
                        draw_list->AddCircle(p_pin1, 6.0f, IM_COL32(0, 235, 255, 255), 16, 1.5f);
                        draw_list->AddText(ImVec2(p_pin1.x - 3.5f, p_pin1.y - 6.0f), IM_COL32(0, 255, 255, 255), "1");
                    }
                    if (d2_loaded && d2_ck.valid && d2_ck.number == h) {
                        bool is_b = (d2_ck.letter == 'B');
                        float r_p2 = is_b ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                        ImVec2 p_pin2(center.x + cosf(mid_ang) * r_p2, center.y + sinf(mid_ang) * r_p2);
                        if (d1_loaded && d1_ck.valid && d1_ck.number == h && d1_ck.letter == d2_ck.letter) {
                            p_pin2.x += 4.0f;
                        }
                        draw_list->AddCircleFilled(p_pin2, 6.0f, IM_COL32(32, 16, 0, 240));
                        draw_list->AddCircle(p_pin2, 6.0f, IM_COL32(255, 140, 20, 255), 16, 1.5f);
                        draw_list->AddText(ImVec2(p_pin2.x - 3.5f, p_pin2.y - 6.0f), IM_COL32(255, 170, 40, 255), "2");
                    }
                }

                // PONTE HARMÔNICA ENTRE DECK 1 E DECK 2 NO DISCO
                if (d1_loaded && d2_loaded && d1_ck.valid && d2_ck.valid) {
                    float a_d1 = -3.14159265f * 0.5f + (float)(d1_ck.number % 12) * angle_step;
                    float r_d1 = (d1_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                    ImVec2 pt_d1(center.x + cosf(a_d1) * r_d1, center.y + sinf(a_d1) * r_d1);

                    float a_d2 = -3.14159265f * 0.5f + (float)(d2_ck.number % 12) * angle_step;
                    float r_d2 = (d2_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                    ImVec2 pt_d2(center.x + cosf(a_d2) * r_d2, center.y + sinf(a_d2) * r_d2);

                    if (d1_ck.number != d2_ck.number || d1_ck.letter != d2_ck.letter) {
                        draw_list->AddLine(pt_d1, pt_d2, IM_COL32(0, 220, 255, 130), 2.0f);
                        draw_list->AddLine(pt_d1, pt_d2, IM_COL32(255, 140, 30, 100), 1.0f);
                    }
                }

                // CENTRO / HUB TONAL DO DECK MASTER
                draw_list->AddCircleFilled(center, r_hub - 1.0f, IM_COL32(10, 16, 26, 255), 24);
                int master_deck_idx = engine.getMasterDeckId();
                ImU32 hub_border = (master_deck_idx == 0) ? IM_COL32(0, 220, 255, 230) : IM_COL32(255, 140, 30, 230);
                draw_list->AddCircle(center, r_hub - 1.0f, hub_border, 24, 1.5f);
                std::string hub_txt = has_hover ? hov_ck.getCamelotCode() : (master_ck.valid ? master_ck.getCamelotCode() : "KEY");
                ImVec2 h_sz = ImGui::CalcTextSize(hub_txt.c_str());
                draw_list->AddText(ImVec2(center.x - h_sz.x * 0.5f, center.y - h_sz.y * 0.5f), 
                                   has_hover ? IM_COL32(255, 235, 40, 255) : hub_border, hub_txt.c_str());

                // FEIXE HARMÔNICO DINÂMICO (LASER BEAM) CONECTANDO O MASTER À FAIXA HOVER
                if (has_hover && master_ck.valid && hov_ck.valid) {
                    float a_src = -3.14159265f * 0.5f + (float)(master_ck.number % 12) * angle_step;
                    float r_src = (master_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                    ImVec2 p_src(center.x + cosf(a_src) * r_src, center.y + sinf(a_src) * r_src);

                    float a_tgt = -3.14159265f * 0.5f + (float)(hov_ck.number % 12) * angle_step;
                    float r_tgt = (hov_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_hub) * 0.5f);
                    ImVec2 p_tgt(center.x + cosf(a_tgt) * r_tgt, center.y + sinf(a_tgt) * r_tgt);

                    KuroAudio::HarmonicCompatibility comp = KuroAudio::DJHarmonicEngine::compareKeys(master_ck, hov_ck);
                    ImU32 beam_col = IM_COL32(0, 255, 128, 255);
                    if (comp == KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE) beam_col = IM_COL32(0, 225, 255, 255);
                    else if (comp == KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1)  beam_col = IM_COL32(255, 205, 0, 255);
                    else if (comp == KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1) beam_col = IM_COL32(0, 180, 255, 255);
                    else if (comp == KuroAudio::HarmonicCompatibility::ENERGY_BOOST_2) beam_col = IM_COL32(255, 45, 140, 255);
                    else if (comp == KuroAudio::HarmonicCompatibility::CLASH)          beam_col = IM_COL32(255, 60, 60, 255);

                    float alpha = hover_anim_timer * (0.80f + 0.20f * sinf(radar_pulse_phase));
                    ImU32 pulse_col = (beam_col & 0x00FFFFFF) | ((int)(alpha * 255.0f) << 24);
                    ImU32 glow_col  = (beam_col & 0x00FFFFFF) | ((int)(alpha * 90.0f) << 24);
                    ImU32 core_col  = (beam_col & 0x00FFFFFF) | ((int)(alpha * 255.0f) << 24);

                    draw_list->AddLine(p_src, p_tgt, glow_col, 6.0f);
                    draw_list->AddLine(p_src, p_tgt, pulse_col, 2.5f);
                    draw_list->AddLine(p_src, p_tgt, IM_COL32(255, 255, 255, (int)(alpha * 130.0f)), 1.0f);
                    draw_list->AddCircleFilled(p_tgt, 4.5f, core_col);
                    draw_list->AddCircleFilled(p_tgt, 2.0f, IM_COL32(255, 255, 255, (int)(alpha * 220.0f)));
                    draw_list->AddCircle(p_tgt, 7.0f + 2.5f * sinf(radar_pulse_phase), pulse_col, 16, 1.2f);
                }
            }

            // 2d. CARD LOOKAHEAD / FRASEAMENTO PREDITIVO
            {
                auto& m_deck = engine.decks[left_id].is_playing ? engine.decks[left_id] : (engine.decks[right_id].is_playing ? engine.decks[right_id] : engine.decks[left_id]);
                double elapsed = m_deck.getElapsedTimeSec();
                double beat_duration = 60.0 / std::max(ref_bpm, 60.0);
                int current_beat = (int)(elapsed / beat_duration);
                int current_bar = current_beat / 4;
                int bars_in_phrase = 32;
                int bar_in_block = current_bar % bars_in_phrase;
                int bars_to_next = bars_in_phrase - bar_in_block;
                if (bars_to_next <= 0) bars_to_next = 32;
                double sec_to_next = bars_to_next * 4.0 * beat_duration;

                const char* cur_phase_str = "Construção & Tensão";
                const char* nxt_phase_str = "Breakdown";
                if (bar_in_block < 8) {
                    cur_phase_str = "1. Intro & Hook";
                    nxt_phase_str = "Build-Up (+1)";
                } else if (bar_in_block < 24) {
                    cur_phase_str = "2. Construção & Drop";
                    nxt_phase_str = "Breakdown & Melodia";
                } else {
                    cur_phase_str = "3. Peak Clímax";
                    nxt_phase_str = "Transição / Saída";
                }

                char phrase_countdown[80];
                snprintf(phrase_countdown, sizeof(phrase_countdown), "Próx: %s (%02d:%02d)", nxt_phase_str, (int)sec_to_next / 60, (int)sec_to_next % 60);

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.016f, 0.022f, 0.034f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.16f, 0.24f, 0.80f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                float card_h = is_big_library ? 82.0f : 56.0f;
                ImGui::BeginChild("##RadarLookaheadCard", ImVec2(left_w - 14.0f, card_h), true, ImGuiWindowFlags_NoScrollbar);

                ImGui::TextColored(ImVec4(0.00f, 1.00f, 0.55f, 1.0f), "%s", phrase_countdown);

                KuroAudio::CamelotKey hov_ck = KuroAudio::DJHarmonicEngine::parseKey(hovered_track_key.empty() ? side_drawer_ui.hovered_track_key : hovered_track_key);
                bool has_hover = ((!hovered_track_key.empty() || !side_drawer_ui.hovered_track_key.empty()) && hov_ck.valid && hover_anim_timer > 0.01f);

                if (has_hover) {
                    std::string act_k = hovered_track_key.empty() ? side_drawer_ui.hovered_track_key : hovered_track_key;
                    std::string match_badge = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(ref_key, act_k);
                    ImVec4 badge_c = ImGui::ColorConvertU32ToFloat4(KuroAudio::DJHarmonicEngine::getCompatibilityBadgeColor(ref_key, act_k));

                    std::string h_title = hovered_track_title.empty() ? side_drawer_ui.hovered_track_title : hovered_track_title;
                    if (h_title.length() > 18) h_title = h_title.substr(0, 16) + "...";
                    ImGui::TextColored(ImVec4(1.0f, 0.92f, 0.20f, 1.0f), "🎯 %s · %s", h_title.c_str(), hov_ck.getCamelotCode().c_str());
                    ImGui::TextColored(badge_c, "• %s", match_badge.c_str());

                    double h_bpm = hovered_track_bpm > 20.0 ? hovered_track_bpm : side_drawer_ui.hovered_track_bpm;
                    double bpm_diff = h_bpm - ref_bpm;
                    double pitch_pct = (ref_bpm > 0.0) ? ((bpm_diff / ref_bpm) * 100.0) : 0.0;
                    const char* diag = (std::abs(bpm_diff) <= 2.5) ? "Encaixe Imediato" : ((std::abs(bpm_diff) <= 6.0) ? "Pitch Fluido" : "Sync BPM");
                    if (is_big_library) {
                        ImGui::TextColored(ImVec4(0.40f, 0.80f, 1.00f, 0.9f), "Δ %+.1f (%+.1f%%) · %s", bpm_diff, pitch_pct, diag);
                    }
                } else {
                    KuroAudio::CamelotKey ref_ck = KuroAudio::DJHarmonicEngine::parseKey(ref_key);
                    ImGui::TextColored(ImVec4(0.60f, 0.75f, 0.88f, 0.9f), "Master: %s (%s)", ref_ck.getCamelotCode().c_str(), ref_ck.musical_key.c_str());
                    ImGui::TextColored(ImVec4(0.40f, 0.55f, 0.68f, 0.75f), "Passe o mouse nas faixas");
                }

                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(2);
            }

            ImGui::EndChild();
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 6.0f);

            // =========================================================================
            // 3. PAINEL DIREITO: BARRA DE BUSCA, CHIPS DE VERTENTES & TABELA REKORDBOX
            // =========================================================================
            ImGui::BeginChild("##TracksRightPane", ImVec2(right_w, pane_h), false);

            // Linha Superior de Busca & Filtros
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.50f, 0.75f, 1.0f, 1.0f), ICON_FA_MAGNIFYING_GLASS);
            ImGui::SameLine(0, 4);
            ImGui::PushItemWidth(180);
            if (ImGui::InputTextWithHint("##SpotifyQuery", "Buscar título, artista ou tom...", search_query_buf, sizeof(search_query_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                spotify_service.searchAsync(search_query_buf);
            }
            ImGui::PopItemWidth();

            if (strlen(search_query_buf) > 0) {
                ImGui::SameLine(0, 2);
                if (ImGui::SmallButton("✕")) {
                    search_query_buf[0] = '\0';
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Limpar busca");
            }

            ImGui::SameLine(0, 4);
            if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " BUSCAR")) {
                spotify_service.searchAsync(search_query_buf);
            }

            // Chip de Filtro da Roda Camelot (se ativo)
            if (!camelot_filter_key.empty()) {
                ImGui::SameLine(0, 6);
                char k_chip[32];
                snprintf(k_chip, sizeof(k_chip), "TOM: %s ✕", camelot_filter_key.c_str());
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.60f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::SmallButton(k_chip)) {
                    camelot_filter_key = "";
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Filtro da Roda Camelot ativo. Clique para remover filtro.");
                ImGui::PopStyleColor(2);
            }

            // Chips Rápidos de Vertentes
            ImGui::SameLine(0, 8);
            auto vchip = [&](const char* label, const char* fid, ImVec4 col) {
                bool is_f_active = (active_fid == fid);
                if (is_f_active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x * 0.35f, col.y * 0.35f, col.z * 0.35f, 0.95f));
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.14f, 0.20f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, col);
                if (ImGui::SmallButton(label)) {
                    KuroAudio::DJLibraryManager::getInstance().setActiveFolderId(fid);
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 3);
            };
            vchip("Todas", "all", ImVec4(0.35f, 0.90f, 1.00f, 1.0f));
            vchip("Prog Psy", "prog_psy", ImVec4(0.00f, 0.96f, 0.83f, 1.0f));
            vchip("Full-On", "psy_fullon", ImVec4(0.85f, 0.40f, 1.00f, 1.0f));
            vchip("Tech House", "tech_house", ImVec4(1.00f, 0.75f, 0.20f, 1.0f));
            vchip("Acid", "acid_techno", ImVec4(0.25f, 1.00f, 0.45f, 1.0f));
            vchip("Melodic", "melodic_techno", ImVec4(0.40f, 0.75f, 1.00f, 1.0f));

            // Barra de Ordenação: Sugestão, BPM, Tom Camelot e Nome
            ImGui::SameLine(0, 8);
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.88f, 0.96f, 1.0f), ICON_FA_ARROW_DOWN_WIDE_SHORT " ORDENAR:");
            ImGui::SameLine(0, 4);

            bool is_sug = (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_SUGGESTION);
            if (is_sug) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.58f, 0.68f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            if (ImGui::SmallButton(ICON_FA_BULLSEYE " Sugestão (Tom+BPM)")) {
                side_drawer_ui.sort_mode = DJSideDrawerUI::SORT_SUGGESTION;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🎯 Sugestão Harmônica: Agrupa e prioriza músicas com tom compatível e BPM próximo ao deck no ar!");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4);

            bool is_bpm = (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_BPM);
            if (is_bpm) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.45f, 0.05f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            char bpm_lbl[32];
            snprintf(bpm_lbl, sizeof(bpm_lbl), "%s BPM %s", ICON_FA_BOLT, is_bpm ? (side_drawer_ui.sort_bpm_ascending ? "▲" : "▼") : "");
            if (ImGui::SmallButton(bpm_lbl)) {
                if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_BPM) {
                    side_drawer_ui.sort_bpm_ascending = !side_drawer_ui.sort_bpm_ascending;
                } else {
                    side_drawer_ui.sort_mode = DJSideDrawerUI::SORT_BPM;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("⚡ Organizar por BPM (%s). Clique para alternar crescente/decrescente.", side_drawer_ui.sort_bpm_ascending ? "Crescente" : "Decrescente");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4);

            bool is_key = (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_KEY);
            if (is_key) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.20f, 0.65f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            char key_lbl[32];
            snprintf(key_lbl, sizeof(key_lbl), "Tom %s", is_key ? (side_drawer_ui.sort_key_ascending ? "▲" : "▼") : "");
            if (ImGui::SmallButton(key_lbl)) {
                if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_KEY) {
                    side_drawer_ui.sort_key_ascending = !side_drawer_ui.sort_key_ascending;
                } else {
                    side_drawer_ui.sort_mode = DJSideDrawerUI::SORT_KEY;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🎹 Organizar pelo Círculo Camelot (1A, 1B, 2A, 2B... 12B). Clique para alternar.");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4);

            bool is_tit = (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_TITLE);
            if (is_tit) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.38f, 0.58f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            if (ImGui::SmallButton("Nome")) {
                side_drawer_ui.sort_mode = DJSideDrawerUI::SORT_TITLE;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🔤 Organizar em ordem alfabética por Título.");
            ImGui::PopStyleColor(2);

            // Toggle de Notação de Tom: Musical (B♭ / Gm) ⇄ Camelot (1B / 6A)
            ImGui::SameLine(0, 8);
            static bool library_use_camelot_key = false;
            if (library_use_camelot_key) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.65f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::SmallButton(ICON_FA_BULLSEYE " TOM: CAMELOT (1B)")) {
                    library_use_camelot_key = false;
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.16f, 0.24f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.92f, 0.85f, 1.0f));
                if (ImGui::SmallButton(ICON_FA_MUSIC " TOM: NOTAS (B♭)")) {
                    library_use_camelot_key = true;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar visualização da coluna Tom: Notação Musical tradicional (B♭, Gm, C♯m) ⇄ Camelot Wheel (1B, 6A, 12A)");
            ImGui::PopStyleColor(2);

            if (!camelot_filter_key.empty()) {
                ImGui::SameLine(0, 4);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.25f, 0.95f));
                char flt_b[32];
                snprintf(flt_b, sizeof(flt_b), "KEY: %s ✕", camelot_filter_key.c_str());
                if (ImGui::SmallButton(flt_b)) {
                    camelot_filter_key = "";
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clique para remover o filtro de chave");
                ImGui::PopStyleColor();
            }

            // Faixas da Pasta Selecionada ou Pen Drive Ativo
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
            }

            int unready_crate_tracks = 0;
            for (const auto& trk : track_list) {
                if (!trk.is_downloaded) unready_crate_tracks++;
            }

            if (spotify_service.isQueueActive()) {
                ImGui::SameLine(0, 10);
                int q_tot = spotify_service.getQueueTotalCount();
                int q_cur = spotify_service.getQueueCurrentIndex();
                int dl_pct = std::clamp((int)spotify_service.getDownloadProgressPct(), 0, 100);
                if (q_tot > 1) {
                    ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.20f, 1.0f), ICON_FA_BOLT " FILA [%d/%d] (%d%%)", q_cur, q_tot, dl_pct);
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.20f, 1.0f), ICON_FA_BOLT " ABDUZINDO (%d%%)", dl_pct);
                }
                ImGui::SameLine(0, 6);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.18f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::SmallButton(ICON_FA_XMARK " CANCELAR")) {
                    spotify_service.cancelCurrentDownload();
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Cancelar abdução atual e liberar o processo");
                ImGui::PopStyleColor(2);
            } else if (unready_crate_tracks > 0) {
                ImGui::SameLine(0, 10);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.48f, 0.42f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.68f, 0.58f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                char ab_crate_lbl[64];
                snprintf(ab_crate_lbl, sizeof(ab_crate_lbl), ICON_FA_SHUTTLE_SPACE " ABDUZIR CRATE (%d)", unready_crate_tracks);
                if (ImGui::SmallButton(ab_crate_lbl)) {
                    spotify_service.enqueueLibraryCrateDownload(track_list);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abduzir todas as faixas não baixadas deste Crate em lote para o disco (24-bit/48kHz com tags ID3 Rekordbox)");
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine(0, 10);
            ImGui::TextColored(ImVec4(0.60f, 0.85f, 1.0f, 0.9f), "%s", spotify_service.getStatus().c_str());

            ImGui::Separator();

            if (track_list.empty() && active_fid == "cloud") {
                auto cloud_results = spotify_service.getSearchResults();
                for (const auto& cs : cloud_results) {
                    KuroAudio::DJLibraryTrack t;
                    t.id = cs.id;
                    t.title = cs.title;
                    t.artist = cs.artist;
                    t.album = cs.album;
                    t.bpm = cs.bpm;
                    t.key = cs.key;
                    t.duration_sec = cs.duration_sec;
                    t.artwork_url = cs.artwork_url;
                    t.preview_url = cs.preview_url;
                    t.direct_url = cs.direct_url;
                    t.subgenre = KuroAudio::DJLibraryManager::classifySubgenre(cs.artist, cs.title, cs.bpm);
                    t.is_downloaded = false;
                    t.is_downloading = cs.is_downloading;
                    track_list.push_back(t);
                }
            }

            // Filtro de Busca Local em Tempo Real
            if (strlen(search_query_buf) > 0) {
                std::string q = search_query_buf;
                std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                track_list.erase(std::remove_if(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& trk) {
                    std::string tit = trk.title; std::transform(tit.begin(), tit.end(), tit.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    std::string art = trk.artist; std::transform(art.begin(), art.end(), art.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    std::string gen = trk.subgenre; std::transform(gen.begin(), gen.end(), gen.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    std::string key = trk.key; std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    char bpm_buf[16]; snprintf(bpm_buf, sizeof(bpm_buf), "%.0f", trk.bpm);
                    return (tit.find(q) == std::string::npos && 
                            art.find(q) == std::string::npos && 
                            gen.find(q) == std::string::npos && 
                            key.find(q) == std::string::npos && 
                            std::string(bpm_buf).find(q) == std::string::npos);
                }), track_list.end());
            }

            // Filtro opcional por chave selecionada no radar
            if (!camelot_filter_key.empty()) {
                track_list.erase(std::remove_if(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& trk) {
                    KuroAudio::CamelotKey ck = KuroAudio::DJHarmonicEngine::parseKey(trk.key);
                    return ck.getCamelotCode() != camelot_filter_key;
                }), track_list.end());
            }

            // Ordenação Inteligente Selecionada
            if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_SUGGESTION) {
                auto master_ck = KuroAudio::DJHarmonicEngine::parseKey(ref_key);
                std::stable_sort(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    auto get_suggestion_score = [&](const KuroAudio::DJLibraryTrack& t) -> double {
                        double harm_penalty = 50.0;
                        if (master_ck.valid) {
                            auto comp = KuroAudio::DJHarmonicEngine::compareKeys(ref_key, t.key);
                            switch (comp) {
                                case KuroAudio::HarmonicCompatibility::PERFECT:           harm_penalty = 0.0; break;
                                case KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE:  harm_penalty = 4.0; break;
                                case KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1:     harm_penalty = 6.0; break;
                                case KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1:    harm_penalty = 8.0; break;
                                case KuroAudio::HarmonicCompatibility::ENERGY_BOOST_2:    harm_penalty = 15.0; break;
                                case KuroAudio::HarmonicCompatibility::DIAGONAL_PLUS_1:   harm_penalty = 18.0; break;
                                case KuroAudio::HarmonicCompatibility::DIAGONAL_MINUS_1:  harm_penalty = 20.0; break;
                                default:                                                  harm_penalty = 45.0; break;
                            }
                        }
                        double bpm_diff = std::abs(t.bpm - ref_bpm);
                        return (harm_penalty * 2.5) + bpm_diff;
                    };
                    return get_suggestion_score(a) < get_suggestion_score(b);
                });
            } else if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_BPM) {
                std::stable_sort(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    return side_drawer_ui.sort_bpm_ascending ? (a.bpm < b.bpm) : (a.bpm > b.bpm);
                });
            } else if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_KEY) {
                std::stable_sort(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    auto cka = KuroAudio::DJHarmonicEngine::parseKey(a.key);
                    auto ckb = KuroAudio::DJHarmonicEngine::parseKey(b.key);
                    int val_a = cka.valid ? (cka.number * 2 + (cka.letter == 'B' ? 1 : 0)) : 999;
                    int val_b = ckb.valid ? (ckb.number * 2 + (ckb.letter == 'B' ? 1 : 0)) : 999;
                    return side_drawer_ui.sort_key_ascending ? (val_a < val_b) : (val_a > val_b);
                });
            } else if (side_drawer_ui.sort_mode == DJSideDrawerUI::SORT_TITLE) {
                std::stable_sort(track_list.begin(), track_list.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    return a.title < b.title;
                });
            }

            if (track_list.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.50f, 0.60f, 0.72f, 1.0f), 
                    ICON_FA_CIRCLE_INFO " Nenhuma música nesta pasta ainda. Busque na nuvem ou importe do computador para catalogar cirurgicamente.");
            } else {
                // Configuração de Estilo Compacto Rekordbox (~22px por linha)
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 2.0f));
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.098f, 0.153f, 0.188f, 1.0f)); // Petróleo #192730
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.12f, 0.20f, 0.25f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.00f, 0.40f, 0.45f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.051f, 0.055f, 0.067f, 1.0f)); // #0d0e11
                ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(0.067f, 0.075f, 0.090f, 1.0f)); // #111317

                if (ImGui::BeginTable("##CrateTracksTable", 11, 
                    ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable, 
                    ImVec2(0, ImGui::GetContentRegionAvail().y - 2.0f))) 
                {
                    ImGui::TableSetupColumn("Pré-Vi", ImGuiTableColumnFlags_WidthFixed, is_big_library ? 165.0f : 140.0f);
                    ImGui::TableSetupColumn("Capa", ImGuiTableColumnFlags_WidthFixed, 24.0f);
                    ImGui::TableSetupColumn("Título", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Artista", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                    ImGui::TableSetupColumn("Key / Tom", ImGuiTableColumnFlags_WidthFixed, 92.0f);
                    ImGui::TableSetupColumn(ICON_FA_LOCK "##ColKeyLock", ImGuiTableColumnFlags_WidthFixed, 22.0f);
                    ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed, 55.0f);
                    ImGui::TableSetupColumn("Duração", ImGuiTableColumnFlags_WidthFixed, 48.0f);
                    ImGui::TableSetupColumn("Grid", ImGuiTableColumnFlags_WidthFixed, 28.0f);
                    ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthFixed, 78.0f);
                    ImGui::TableSetupColumn("Carregar no Deck", ImGuiTableColumnFlags_WidthFixed, 196.0f);
                    ImGui::TableHeadersRow();

                    // Linha ciano divisória marcante do Rekordbox sob os cabeçalhos
                    ImVec2 header_min = ImGui::GetItemRectMin();
                    ImVec2 header_max = ImGui::GetItemRectMax();
                    ImGui::GetWindowDrawList()->AddLine(
                        ImVec2(header_min.x, header_max.y), 
                        ImVec2(header_max.x, header_max.y), 
                        IM_COL32(0, 162, 153, 255), 1.8f);

                    for (int i = 0; i < (int)track_list.size(); i++) {
                        auto& t = track_list[i];
                        bool is_sel = (selected_track_idx == i || context_track_id == t.id);
                        ImGui::TableNextRow(ImGuiTableRowFlags_None, is_sel ? 30.0f : 24.0f);

                        int loaded_deck = -1;
                        for (int dd = 0; dd < 4; dd++) {
                            if (!engine.decks[dd].buffer_l.empty() && engine.decks[dd].track_title == t.title) { loaded_deck = dd; break; }
                        }
                        double row_dur = t.duration_sec;
                        if (loaded_deck >= 0 && engine.decks[loaded_deck].duration_sec > 1.0) row_dur = engine.decks[loaded_deck].duration_sec;

                        // Traffic Light Match Calculation (compatibilidade de tom e BPM até 8%)
                        bool is_match = false;
                        if (!ref_key.empty() && ref_bpm > 30.0) {
                            auto comp = KuroAudio::DJHarmonicEngine::compareKeys(ref_key, t.key);
                            bool harm_ok = (comp == KuroAudio::HarmonicCompatibility::PERFECT ||
                                            comp == KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE ||
                                            comp == KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1 ||
                                            comp == KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1);
                            double bpm_diff_pct = std::abs(t.bpm - ref_bpm) / ref_bpm;
                            if (harm_ok && bpm_diff_pct <= 0.08) {
                                is_match = true;
                            }
                        }

                        // Coluna 0: Pré-Vi (Ícone Play/Pause + Mini-Scrub Bar com Waveform Real e Larga)
                        ImGui::TableSetColumnIndex(0);
                        auto& touch_cue = KuroAudio::DJTouchPreviewCue::getInstance();
                        bool is_prev_playing = touch_cue.isTouchPreviewActive() && (touch_cue.getPreviewTrackId() == t.id);

                        ImGui::PushID(i * 100);
                        if (is_prev_playing) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                            if (ImGui::SmallButton(ICON_FA_PAUSE "##PrevPause")) {
                                touch_cue.stopTouchPreview();
                            }
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.70f, 0.85f, 1.0f));
                            if (ImGui::SmallButton(ICON_FA_HEADPHONES "##PrevPlay")) {
                                std::string src = t.local_wav_path.empty() ? t.preview_url : t.local_wav_path;
                                touch_cue.startTouchPreview(src, t.id, t.title);
                            }
                            ImGui::PopStyleColor();
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip(is_prev_playing ? "Pausar pré-escuta no fone" : "Pré-escutar instantaneamente no fone");

                        // Mini-Scrub Bar ampla que acompanha a largura da coluna com Waveform 3-Band Autêntica
                        ImGui::SameLine(0, 4.0f);
                        ImVec2 scrub_p0 = ImGui::GetCursorScreenPos();
                        float avail_cell_w = ImGui::GetContentRegionAvail().x;
                        float scrub_w = std::max(avail_cell_w - 4.0f, 115.0f);
                        float scrub_h = 20.0f;
                        ImVec2 scrub_p1(scrub_p0.x + scrub_w, scrub_p0.y + scrub_h);
                        ImDrawList* dl = ImGui::GetWindowDrawList();

                        dl->AddRectFilled(scrub_p0, scrub_p1, IM_COL32(8, 10, 14, 255), 2.0f);
                        dl->AddRect(scrub_p0, scrub_p1, IM_COL32(28, 36, 48, 220), 2.0f, 0, 1.0f);

                        float base_y = scrub_p1.y - 1.5f;
                        float max_h_amp = scrub_h - 2.5f;
                        float prog = is_prev_playing ? (float)touch_cue.getPreviewProgress() : 0.0f;

                        // 1. Se carregada em um deck ativo, exibe a waveform detalhada em tempo real
                        const KuroAudio::DJDeck* src_deck = nullptr;
                        if (loaded_deck >= 0 && loaded_deck < 4 && !engine.decks[loaded_deck].detailed_waveform.empty()) {
                            src_deck = &engine.decks[loaded_deck];
                        }

                        if (src_deck && !src_deck->detailed_waveform.empty()) {
                            size_t sz = src_deck->detailed_waveform.size();
                            for (float px = scrub_p0.x; px < scrub_p1.x; px += 1.0f) {
                                float norm_x = (px - scrub_p0.x) / scrub_w;
                                size_t idx = (size_t)std::clamp(norm_x * (float)sz, 0.0f, (float)(sz - 1));
                                const auto& pt = src_deck->detailed_waveform[idx];
                                float dim = (is_prev_playing && norm_x < prog) ? 0.45f : 1.0f;
                                drawRekordboxOverviewSlice(dl, px, base_y, max_h_amp, pt.peak, pt.low, pt.mid, pt.high, dim, waveform_color_mode);
                            }
                        } else {
                            // 2. Waveform Real da Música a partir do cache .wf
                            if (t.overview_wave.empty()) {
                                KuroAudio::DJLibraryManager::loadTrackWaveform(t);
                            }
                            if (!t.overview_wave.empty()) {
                                size_t sz = t.overview_wave.size();
                                for (float px = scrub_p0.x; px < scrub_p1.x; px += 1.0f) {
                                    float norm_x = (px - scrub_p0.x) / scrub_w;
                                    size_t idx = (size_t)std::clamp(norm_x * (float)sz, 0.0f, (float)(sz - 1));
                                    const auto& pt = t.overview_wave[idx];
                                    float dim = (is_prev_playing && norm_x < prog) ? 0.45f : 1.0f;
                                    drawRekordboxOverviewSlice(dl, px, base_y, max_h_amp, pt.peak, pt.low, pt.mid, pt.high, dim, waveform_color_mode);
                                }
                            } else {
                                // Linha sutil de osciloscópio até o carregamento
                                dl->AddLine(ImVec2(scrub_p0.x, base_y), ImVec2(scrub_p1.x, base_y), IM_COL32(40, 50, 68, 180), 1.0f);
                            }
                        }

                        // Agulha de reprodução / cursor Rekordbox
                        if (is_prev_playing && prog > 0.0f) {
                            float needle_x = scrub_p0.x + scrub_w * prog;
                            dl->AddLine(ImVec2(needle_x, scrub_p0.y), ImVec2(needle_x, scrub_p1.y), IM_COL32(255, 255, 255, 255), 1.5f);
                            dl->AddTriangleFilled(ImVec2(needle_x - 2.5f, scrub_p0.y), ImVec2(needle_x + 2.5f, scrub_p0.y), ImVec2(needle_x, scrub_p0.y + 4.0f), IM_COL32(255, 255, 255, 255));
                        }

                        ImGui::InvisibleButton("##scrub_btn", ImVec2(scrub_w, scrub_h));
                        if (ImGui::IsItemClicked(0)) {
                            float click_x = ImGui::GetIO().MousePos.x - scrub_p0.x;
                            float click_frac = std::clamp(click_x / scrub_w, 0.0f, 1.0f);
                            std::string src = t.local_wav_path.empty() ? t.preview_url : t.local_wav_path;
                            touch_cue.startTouchPreview(src, t.id, t.title, (double)click_frac);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clique na barra para saltar o áudio da pré-escuta");
                        ImGui::PopID();

                        // Coluna 1: Capa (Miniatura 20x20)
                        ImGui::TableSetColumnIndex(1);
                        GLuint tex_id = (GLuint)t.gl_tex_id;
                        if (tex_id == 0 || (tex_id == 0xFFFFFFFF && !t.local_rgba_path.empty())) {
                            std::string rgba_p = t.local_rgba_path;
                            if (rgba_p.empty()) {
                                rgba_p = KuroAudio::DJLibraryManager::resolveRgbaCoverArt(t.local_wav_path, t.id);
                            }
                            if (!rgba_p.empty()) {
                                tex_id = KuroAudio::SpotifyDJService::UploadRGBATexture(rgba_p);
                                if (tex_id != 0) {
                                    t.gl_tex_id = (unsigned int)tex_id;
                                    t.local_rgba_path = rgba_p;
                                    KuroAudio::DJLibraryManager::getInstance().setTrackTexture(t.id, t.gl_tex_id);
                                } else {
                                    t.gl_tex_id = 0xFFFFFFFF;
                                    KuroAudio::DJLibraryManager::getInstance().setTrackTexture(t.id, 0xFFFFFFFF);
                                }
                            } else {
                                t.gl_tex_id = 0xFFFFFFFF;
                                KuroAudio::DJLibraryManager::getInstance().setTrackTexture(t.id, 0xFFFFFFFF);
                            }
                        }
                        if (tex_id != 0 && tex_id != 0xFFFFFFFF) {
                            ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(20, 20));
                        } else {
                            ImGui::TextColored(ImVec4(0.40f, 0.55f, 0.70f, 0.8f), ICON_FA_COMPACT_DISC);
                        }
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                            ImGui::SetDragDropPayload("DND_DJ_TRACK_ID", t.id.c_str(), t.id.size() + 1);
                            ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                            ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "• %s (%.1f BPM - %s)", t.artist.c_str(), t.bpm, t.key.c_str());
                            ImGui::EndDragDropSource();
                        }

                        // Coluna 2: Título (com Linha Selecionável e Foco Petróleo Estilizado)
                        ImGui::TableSetColumnIndex(2);
                        std::string sel_label = "##RowSelect_" + t.id;
                        is_sel = (selected_track_idx == i || context_track_id == t.id);

                        if (ImGui::Selectable(sel_label.c_str(), is_sel, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 20.0f))) {
                            selected_track_idx = i;
                            context_track_id = t.id;
                        }

                        // Borda e Retângulo de Foco estilo Guitar Hero Setlist (Glow Neon Luminous)
                        if (is_sel) {
                            ImVec2 row_min = ImGui::GetItemRectMin();
                            ImVec2 row_max = ImGui::GetItemRectMax();
                            float pulse = 0.85f + 0.15f * sinf((float)ImGui::GetTime() * 4.5f);
                            dl->AddRect(row_min, row_max, IM_COL32(0, 245, 215, (int)(255 * pulse)), 2.5f, 0, 1.8f);
                            dl->AddRectFilled(row_min, row_max, IM_COL32(0, 180, 220, 28));
                        } else if (is_match) {
                            // Traffic Light Match: Leve brilho verde petróleo
                            ImVec2 row_min = ImGui::GetItemRectMin();
                            ImVec2 row_max = ImGui::GetItemRectMax();
                            dl->AddRectFilled(row_min, row_max, IM_COL32(0, 180, 120, 22));
                        }

                        if (ImGui::IsItemHovered()) {
                            hovered_track_id = t.id;
                            hovered_track_key = t.key;
                            hovered_track_bpm = t.bpm;
                            hovered_track_title = t.title;
                            hovered_track_artist = t.artist;
                            hover_anim_timer = 1.0f;
                            side_drawer_ui.setHoveredTrack(t.id, t.key, t.bpm, t.title, t.artist);
                        }

                        // Drag & Drop para os Decks
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                            ImGui::SetDragDropPayload("DND_DJ_TRACK_ID", t.id.c_str(), t.id.size() + 1);
                            ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                            ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "• %s (%.1f BPM - %s)", t.artist.c_str(), t.bpm, t.key.c_str());
                            ImGui::EndDragDropSource();
                        }

                        // Duplo clique carrega no deck apropriado
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            int target_d = engine.decks[left_id].is_playing ? right_id : left_id;
                            if (t.is_downloaded) {
                                spotify_service.loadLibraryTrack(t, target_d, engine);
                            } else {
                                spotify_service.downloadLibraryTrackAsync(t, target_d, engine);
                            }
                        }

                        // Menu de Contexto Completo do Botão Direito
                        if (ImGui::BeginPopupContextItem(("TrackRowContext_" + t.id).c_str())) {
                            context_track_id = t.id;
                            selected_track_idx = i;

                            ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_COMPACT_DISC " %s", t.title.c_str());
                            ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.85f, 1.0f), "• %s (%s - %.1f BPM)", t.artist.c_str(), t.key.c_str(), t.bpm);
                            ImGui::Separator();

                            // 1. Carregar nos Decks
                            char m_l[48], m_r[48];
                            snprintf(m_l, sizeof(m_l), ICON_FA_ARROW_DOWN " Carregar no Deck %d", left_id + 1);
                            snprintf(m_r, sizeof(m_r), ICON_FA_ARROW_DOWN " Carregar no Deck %d", right_id + 1);
                            if (ImGui::MenuItem(m_l)) {
                                if (t.is_downloaded) spotify_service.loadLibraryTrack(t, left_id, engine);
                                else spotify_service.downloadLibraryTrackAsync(t, left_id, engine);
                            }
                            if (ImGui::MenuItem(m_r)) {
                                if (t.is_downloaded) spotify_service.loadLibraryTrack(t, right_id, engine);
                                else spotify_service.downloadLibraryTrackAsync(t, right_id, engine);
                            }

                            // 2. Preview no Fone de Ouvido
                            if (ImGui::MenuItem(ICON_FA_HEADPHONES " Ouvir no Fone (Touch Preview)")) {
                                std::string src = t.local_wav_path.empty() ? t.preview_url : t.local_wav_path;
                                touch_cue.startTouchPreview(src, t.id, t.title);
                            }

                            ImGui::Separator();

                            // 3. Editar Metadados (F2)
                            if (ImGui::MenuItem(ICON_FA_PEN_TO_SQUARE " Editar Metadados (F2)")) {
                                strncpy(edit_title_buf, t.title.c_str(), sizeof(edit_title_buf) - 1);
                                strncpy(edit_artist_buf, t.artist.c_str(), sizeof(edit_artist_buf) - 1);
                                edit_bpm = t.bpm;
                                strncpy(edit_key_buf, t.key.c_str(), sizeof(edit_key_buf) - 1);
                                strncpy(edit_subgenre_buf, t.subgenre.c_str(), sizeof(edit_subgenre_buf) - 1);
                                show_edit_meta_modal = true;
                            }

                            // 4. Copiar Faixa (Ctrl+C)
                            if (ImGui::MenuItem(ICON_FA_COPY " Copiar Faixa (Ctrl+C)")) {
                                clipboard_track = t;
                                has_clipboard_track = true;
                            }

                            // 5. Colar Faixa (Ctrl+V)
                            if (has_clipboard_track) {
                                std::string paste_txt = ICON_FA_PASTE " Colar Faixa Copiada: " + clipboard_track.title;
                                if (ImGui::MenuItem(paste_txt.c_str())) {
                                    clipboard_track.folder_id = lib_mgr.getActiveFolderId();
                                    lib_mgr.addOrUpdateTrack(clipboard_track);
                                }
                            }

                            // 6. Duplicar Faixa
                            if (ImGui::MenuItem(ICON_FA_CLONE " Duplicar Faixa")) {
                                lib_mgr.duplicateTrack(t.id);
                            }

                            // 7. Excluir da Biblioteca (Del)
                            if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Excluir da Biblioteca (Del)")) {
                                show_delete_confirm_modal = true;
                            }

                            ImGui::Separator();

                            // 8. Trocar Fonte / Versão Limpa no Kuro Cloud
                            if (ImGui::MenuItem(ICON_FA_SATELLITE_DISH " Trocar Fonte / Versão Limpa (Kuro Cloud)")) {
                                spotify_service.searchSourcesAsync(t.id, t.title, t.artist);
                            }

                            // 9. Mostrar no Windows Explorer
                            if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Mostrar no Windows Explorer")) {
                                revealInWindowsExplorer(t.local_wav_path);
                            }

                            ImGui::Separator();

                            // 10. Mover para Pasta
                            if (ImGui::BeginMenu("Mover para Pasta")) {
                                for (const auto& fo : folders) {
                                    if (ImGui::MenuItem(fo.name.c_str())) {
                                        lib_mgr.moveTrackToFolder(t.id, fo.id);
                                    }
                                }
                                ImGui::EndMenu();
                            }

                            // 11. Alterar Vertente
                            if (ImGui::BeginMenu("Alterar Vertente")) {
                                const char* genres[] = { "Progressive Psytrance", "Psytrance / Full-On", "Tech House", "Acid Techno", "Melodic Techno" };
                                for (const char* g : genres) {
                                    if (ImGui::MenuItem(g)) {
                                        lib_mgr.setTrackSubgenre(t.id, g);
                                    }
                                }
                                ImGui::EndMenu();
                            }

                            // 12. Classificação / Rating
                            if (ImGui::BeginMenu("Classificação (Rating)")) {
                                for (int st = 1; st <= 5; st++) {
                                    char st_lbl[32];
                                    snprintf(st_lbl, sizeof(st_lbl), "%d Estrelas (%s)", st, std::string(st, '*').c_str());
                                    if (ImGui::MenuItem(st_lbl, nullptr, t.rating == st)) {
                                        t.rating = st;
                                        lib_mgr.setTrackRating(t.id, st);
                                    }
                                }
                                if (ImGui::MenuItem("Limpar Rating", nullptr, t.rating == 0)) {
                                    t.rating = 0;
                                    lib_mgr.setTrackRating(t.id, 0);
                                }
                                ImGui::EndMenu();
                            }

                            ImGui::EndPopup();
                        }

                        // Texto do Título na coluna 2
                        ImGui::SameLine(0, 4);
                        ImVec4 title_col = is_sel ? ImVec4(1.00f, 0.96f, 0.38f, 1.0f) : (t.has_played ? ImVec4(0.65f, 0.70f, 0.75f, 0.85f) : ImVec4(0.96f, 0.97f, 0.98f, 1.0f));
                        ImGui::TextColored(title_col, "%s", t.title.c_str());

                        if (loaded_deck >= 0) {
                            static const ImVec4 deck_cols[4] = { ImVec4(0.00f,0.95f,1.00f,1.0f), ImVec4(1.00f,0.65f,0.00f,1.0f), ImVec4(0.30f,0.55f,1.00f,1.0f), ImVec4(1.00f,0.25f,0.85f,1.0f) };
                            ImGui::SameLine(0, 4);
                            ImGui::TextColored(deck_cols[loaded_deck], "[DECK %d]", loaded_deck + 1);
                        }
                        if (t.has_played) {
                            ImGui::SameLine(0, 4);
                            ImGui::TextColored(ImVec4(0.30f, 1.00f, 0.50f, 0.9f), "✓");
                        }

                        // Coluna 3: Artista
                        ImGui::TableSetColumnIndex(3);
                        ImVec4 artist_col = is_sel ? ImVec4(0.90f, 0.94f, 1.00f, 1.0f) : (t.has_played ? ImVec4(0.50f, 0.55f, 0.60f, 0.8f) : ImVec4(0.65f, 0.72f, 0.82f, 1.0f));
                        ImGui::TextColored(artist_col, "%s", t.artist.c_str());

                        // Coluna 4: Key Camelot + Tom Musical (ex: 8A • Am)
                        ImGui::TableSetColumnIndex(4);
                        KuroAudio::CamelotKey trk_ck = KuroAudio::DJHarmonicEngine::parseKey(t.key);
                        unsigned int w_col = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(trk_ck.number);
                        std::string mus_not = KuroAudio::DJHarmonicEngine::getRekordboxMusicalNotation(trk_ck.number, trk_ck.letter);
                        char full_k_buf[32];
                        snprintf(full_k_buf, sizeof(full_k_buf), "%s • %s", trk_ck.getCamelotCode().c_str(), mus_not.c_str());

                        ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)w_col);
                        ImGui::Text("%s", full_k_buf);
                        ImGui::PopStyleColor();

                        if (ImGui::IsItemHovered()) {
                            std::string b_txt = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(ref_key, t.key);
                            ImGui::SetTooltip("Tom: %s (Camelot: %s | Musical: %s)\nCompatibilidade com Deck Master (%s): %s", 
                                              full_k_buf, trk_ck.getCamelotCode().c_str(), mus_not.c_str(), ref_key.c_str(), b_txt.c_str());
                        }

                        // Coluna 5: [ 🔒 ] Cadeado Key Lock
                        ImGui::TableSetColumnIndex(5);
                        ImGui::TextColored(ImVec4(0.50f, 0.60f, 0.70f, 0.9f), ICON_FA_LOCK);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Key Lock / Master Tempo ativo (Tom preservado no pitch)");

                        // Coluna 6: BPM (alinhado à direita, formatado com vírgula)
                        ImGui::TableSetColumnIndex(6);
                        char bpm_str[32];
                        snprintf(bpm_str, sizeof(bpm_str), "%.1f", t.bpm);
                        for (char* p = bpm_str; *p; ++p) { if (*p == '.') *p = ','; }
                        float bpm_w = ImGui::CalcTextSize(bpm_str).x;
                        float col6_w = ImGui::GetColumnWidth(6);
                        if (col6_w > bpm_w + 6.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (col6_w - bpm_w - 6.0f));
                        ImGui::TextColored(is_match ? ImVec4(0.00f, 1.00f, 0.70f, 1.0f) : ImVec4(0.90f, 0.92f, 0.95f, 1.0f), "%s", bpm_str);

                        // Coluna 7: Duração (alinhado à direita, mm:ss)
                        ImGui::TableSetColumnIndex(7);
                        int min = (int)row_dur / 60;
                        int sec = (int)row_dur % 60;
                        char dur_str[16];
                        snprintf(dur_str, sizeof(dur_str), "%d:%02d", min, sec);
                        float dur_w = ImGui::CalcTextSize(dur_str).x;
                        float col7_w = ImGui::GetColumnWidth(7);
                        if (col7_w > dur_w + 6.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (col7_w - dur_w - 6.0f));
                        ImGui::TextColored(ImVec4(0.70f, 0.75f, 0.80f, 0.9f), "%s", dur_str);

                        // Coluna 8: Grid (Beatgrid Quartz Lock Status)
                        ImGui::TableSetColumnIndex(8);
                        ImGui::PushID(i * 500);
                        if (t.grid_locked) {
                            ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.80f, 1.0f), "✓");
                        } else {
                            ImGui::TextColored(ImVec4(0.40f, 0.45f, 0.50f, 0.6f), "–");
                        }
                        if (ImGui::IsItemClicked()) {
                            t.grid_locked = !t.grid_locked;
                            KuroAudio::DJLibraryManager::getInstance().setTrackGridLock(t.id, t.grid_locked);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Grade de Batida (Beatgrid Quartz Lock): %s\nClique para alternar bloqueio", t.grid_locked ? "Bloqueada / Calibrada" : "Desbloqueada");
                        ImGui::PopID();

                        // Coluna 9: Rating (★ Interativo de 1 a 5)
                        ImGui::TableSetColumnIndex(9);
                        ImGui::PushID(i * 1000);
                        int cur_rating = t.rating;
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
                        for (int st = 1; st <= 5; st++) {
                            bool filled = (st <= cur_rating);
                            ImGui::PushStyleColor(ImGuiCol_Text, filled ? ImVec4(1.00f, 0.82f, 0.15f, 1.0f) : ImVec4(0.30f, 0.35f, 0.42f, 0.7f));
                            char st_id[32]; snprintf(st_id, sizeof(st_id), "%s##st_%d", ICON_FA_STAR, st);
                            if (ImGui::SmallButton(st_id)) {
                                int new_r = (cur_rating == st) ? (st - 1) : st;
                                t.rating = new_r;
                                KuroAudio::DJLibraryManager::getInstance().setTrackRating(t.id, new_r);
                            }
                            ImGui::PopStyleColor();
                            if (st < 5) ImGui::SameLine(0, 1.0f);
                        }
                        ImGui::PopStyleVar();
                        ImGui::PopID();

                        // Coluna 10: Disparo Direto para Decks (LOAD A, B, C, D)
                        ImGui::TableSetColumnIndex(10);
                        ImGui::PushID(i * 7000);
                        float btn_load_w = 46.0f;
                        float btn_load_h = is_sel ? 22.0f : 18.0f;
                        ImFont* f_btn_font = g_font_control_small ? g_font_control_small : (g_font_micro_tag ? g_font_micro_tag : ImGui::GetFont());
                        float f_btn_sz = 10.0f;

                        auto drawDeckLoadBtn = [&](int d_idx, const char* d_letter, ImVec4 col_act) {
                            bool is_deck_playing = engine.decks[d_idx].is_playing.load();
                            bool is_this_loaded = (!engine.decks[d_idx].buffer_l.empty() && engine.decks[d_idx].track_title == t.title);

                            ImVec2 p_btn = ImGui::GetCursorScreenPos();
                            ImVec2 p_max = ImVec2(p_btn.x + btn_load_w, p_btn.y + btn_load_h);

                            char b_id[32]; snprintf(b_id, sizeof(b_id), "##load_%d_%d", i, d_idx);
                            bool clicked = ImGui::InvisibleButton(b_id, ImVec2(btn_load_w, btn_load_h));
                            bool is_hov = ImGui::IsItemHovered();
                            bool is_act = ImGui::IsItemActive();

                            ImU32 bg_c = is_this_loaded ? ImGui::ColorConvertFloat4ToU32(ImVec4(col_act.x * 0.45f, col_act.y * 0.45f, col_act.z * 0.45f, 0.95f)) :
                                         (is_act ? ImGui::ColorConvertFloat4ToU32(col_act) : 
                                         (is_hov ? ImGui::ColorConvertFloat4ToU32(ImVec4(col_act.x * 0.35f, col_act.y * 0.35f, col_act.z * 0.35f, 0.90f)) :
                                         (is_sel ? ImGui::ColorConvertFloat4ToU32(ImVec4(col_act.x * 0.20f, col_act.y * 0.20f, col_act.z * 0.20f, 0.85f)) : IM_COL32(14, 18, 25, 220))));

                            ImU32 bd_c = is_hov ? ImGui::ColorConvertFloat4ToU32(col_act) : 
                                         (is_this_loaded ? ImGui::ColorConvertFloat4ToU32(col_act) : 
                                         (is_sel ? ImGui::ColorConvertFloat4ToU32(ImVec4(col_act.x * 0.8f, col_act.y * 0.8f, col_act.z * 0.8f, 0.7f)) : IM_COL32(36, 46, 62, 180)));

                            ImU32 tx_c = is_this_loaded ? IM_COL32(255, 255, 255, 255) :
                                         (is_hov ? IM_COL32(255, 255, 255, 255) :
                                         (is_sel ? ImGui::ColorConvertFloat4ToU32(col_act) : ImGui::ColorConvertFloat4ToU32(ImVec4(col_act.x * 0.75f, col_act.y * 0.75f, col_act.z * 0.75f, 0.85f))));

                            dl->AddRectFilled(p_btn, p_max, bg_c, 3.0f);
                            dl->AddRect(p_btn, p_max, bd_c, 3.0f, 0, is_hov ? 1.5f : 1.0f);

                            char b_txt[32]; snprintf(b_txt, sizeof(b_txt), "LOAD %s", d_letter);
                            ImVec2 t_sz = f_btn_font->CalcTextSizeA(f_btn_sz, FLT_MAX, 0.0f, b_txt);
                            dl->AddText(f_btn_font, f_btn_sz, ImVec2(p_btn.x + (btn_load_w - t_sz.x) * 0.5f, p_btn.y + (btn_load_h - t_sz.y) * 0.5f), tx_c, b_txt);

                            if (clicked) {
                                if (t.is_downloaded) {
                                    spotify_service.loadLibraryTrack(t, d_idx, engine);
                                } else {
                                    spotify_service.downloadLibraryTrackAsync(t, d_idx, engine);
                                }
                            }
                            if (is_hov) {
                                ImGui::SetTooltip("Carregar '%s' no Deck %s (Canal %d)%s", 
                                                  t.title.c_str(), d_letter, d_idx + 1, 
                                                  is_deck_playing ? "\n[AVISO: Deck está tocando no momento]" : "");
                            }
                        };

                        drawDeckLoadBtn(0, "A", ImVec4(0.00f, 0.88f, 1.00f, 1.0f));
                        ImGui::SameLine(0, 3.0f);
                        drawDeckLoadBtn(1, "B", ImVec4(1.00f, 0.62f, 0.12f, 1.0f));
                        ImGui::SameLine(0, 3.0f);
                        drawDeckLoadBtn(2, "C", ImVec4(0.12f, 0.95f, 0.52f, 1.0f));
                        ImGui::SameLine(0, 3.0f);
                        drawDeckLoadBtn(3, "D", ImVec4(0.92f, 0.38f, 0.95f, 1.0f));

                        ImGui::PopID();
                    }

                    // Menu de Contexto ao Clicar com Botão Direito na Área Vazia da Tabela
                    if (ImGui::BeginPopupContextWindow("BibliotecaWindowContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
                        ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), ICON_FA_BOOK " BIBLIOTECA MUSICAL");
                        ImGui::Separator();
                        if (has_clipboard_track) {
                            std::string paste_txt = ICON_FA_PASTE " Colar Faixa: " + clipboard_track.title + " (Ctrl+V)";
                            if (ImGui::MenuItem(paste_txt.c_str())) {
                                clipboard_track.folder_id = lib_mgr.getActiveFolderId();
                                lib_mgr.addOrUpdateTrack(clipboard_track);
                            }
                        } else {
                            ImGui::TextDisabled(ICON_FA_PASTE " Colar Faixa (Nenhuma copiada)");
                        }
                        if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS " Nova Pasta na Biblioteca")) {
                            show_new_folder_dialog = true;
                        }
                        if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Abrir Pasta de Músicas no Explorer")) {
                            revealInWindowsExplorer("");
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::EndTable();
                }
                ImGui::PopStyleColor(5);
                ImGui::PopStyleVar();
            }

            ImGui::EndChild();

            // Atalhos de Teclado no Modo DJ para a Biblioteca (Ctrl+C, Ctrl+V, Del, F2, Shift+Left, Shift+Right, Space)
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
                // Shift + Seta Esquerda: Carregar no Deck 1
                if (ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                    if (selected_track_idx >= 0 && selected_track_idx < (int)track_list.size()) {
                        auto& sel_t = track_list[selected_track_idx];
                        if (sel_t.is_downloaded) spotify_service.loadLibraryTrack(sel_t, left_id, engine);
                        else spotify_service.downloadLibraryTrackAsync(sel_t, left_id, engine);
                    }
                }
                // Shift + Seta Direita: Carregar no Deck 2
                if (ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                    if (selected_track_idx >= 0 && selected_track_idx < (int)track_list.size()) {
                        auto& sel_t = track_list[selected_track_idx];
                        if (sel_t.is_downloaded) spotify_service.loadLibraryTrack(sel_t, right_id, engine);
                        else spotify_service.downloadLibraryTrackAsync(sel_t, right_id, engine);
                    }
                }
                // Tecla P: Alternar Pré-escuta da Faixa Selecionada
                if (ImGui::IsKeyPressed(ImGuiKey_P)) {
                    if (selected_track_idx >= 0 && selected_track_idx < (int)track_list.size()) {
                        auto& sel_t = track_list[selected_track_idx];
                        auto& tc = KuroAudio::DJTouchPreviewCue::getInstance();
                        if (tc.isTouchPreviewActive() && tc.getPreviewTrackId() == sel_t.id) {
                            tc.stopTouchPreview();
                        } else {
                            std::string src = sel_t.local_wav_path.empty() ? sel_t.preview_url : sel_t.local_wav_path;
                            tc.startTouchPreview(src, sel_t.id, sel_t.title);
                        }
                    }
                }
                // Ctrl+C: Copiar
                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
                    if (!context_track_id.empty()) {
                        auto* p = lib_mgr.getTrackById(context_track_id);
                        if (p) {
                            clipboard_track = *p;
                            has_clipboard_track = true;
                        }
                    }
                }
                // Ctrl+V: Colar
                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V) && has_clipboard_track) {
                    clipboard_track.folder_id = lib_mgr.getActiveFolderId();
                    lib_mgr.addOrUpdateTrack(clipboard_track);
                }
                // Del: Excluir
                if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                    if (!context_track_id.empty()) {
                        show_delete_confirm_modal = true;
                    }
                }
                // F2: Editar Metadados
                if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
                    if (!context_track_id.empty()) {
                        auto* p = lib_mgr.getTrackById(context_track_id);
                        if (p) {
                            strncpy(edit_title_buf, p->title.c_str(), sizeof(edit_title_buf)-1);
                            strncpy(edit_artist_buf, p->artist.c_str(), sizeof(edit_artist_buf)-1);
                            edit_bpm = p->bpm;
                            strncpy(edit_key_buf, p->key.c_str(), sizeof(edit_key_buf)-1);
                            strncpy(edit_subgenre_buf, p->subgenre.c_str(), sizeof(edit_subgenre_buf)-1);
                            show_edit_meta_modal = true;
                        }
                    }
                }
            }

            // Modal Criar Nova Pasta
            if (show_new_folder_dialog) {
                ImGui::OpenPopup("Nova Pasta DJ##Modal");
            }
            if (ImGui::BeginPopupModal("Nova Pasta DJ##Modal", &show_new_folder_dialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_FOLDER_PLUS " Criar Nova Pasta na Biblioteca");
                ImGui::Text("Digite o nome da pasta para organizar suas faixas:");
                ImGui::SetNextItemWidth(260);
                ImGui::InputText("##NewCrateNameInput", new_folder_name_buf, sizeof(new_folder_name_buf));
                ImGui::Spacing();
                if (ImGui::Button("Criar Pasta", ImVec2(120, 0))) {
                    if (strlen(new_folder_name_buf) > 0) {
                        KuroAudio::DJLibraryManager::getInstance().createCustomFolder(new_folder_name_buf);
                        new_folder_name_buf[0] = '\0';
                        show_new_folder_dialog = false;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancelar", ImVec2(100, 0))) {
                    show_new_folder_dialog = false;
                }
                ImGui::EndPopup();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }


} // namespace KuroUI
