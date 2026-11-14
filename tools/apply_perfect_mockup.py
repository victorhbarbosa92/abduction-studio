# -*- coding: utf-8 -*-
import sys, os

src_path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(src_path, 'r', encoding='utf-8', errors='ignore') as f:
    text = f.read()

m1 = text.find('void renderSearchModal(KuroAudio::DJEngine')
t1 = text.find('else if (cloud_hub_active_tab == 1)', m1)
m2 = text.find('void renderCamelotRadarModal(KuroAudio::DJEngine', m1)

if m1 == -1 or t1 == -1 or m2 == -1:
    print('Failed to find markers!', m1, t1, m2)
    sys.exit(1)

# Tab 4 ends before the closing of renderSearchModal
end_of_tab4 = text.rfind('}', 0, m2 - 80)
end_of_tab4 = text.find('\n', end_of_tab4) + 1

tabs_1_to_4 = text[t1:end_of_tab4]

header_and_tab0 = r"""void renderSearchModal(KuroAudio::DJEngine &engine) {
    if (!show_search_modal) return;
    if (search_modal_view == 1) {
        renderMasterSourceMatrixCard(engine);
        return;
    }

    ImGuiViewport* vp = ImGui::GetMainViewport();
    float modal_w = vp->Size.x * 0.985f;
    float modal_h = vp->Size.y * 0.965f;
    float modal_x = vp->Pos.x + (vp->Size.x - modal_w) * 0.5f;
    float modal_y = vp->Pos.y + (vp->Size.y - modal_h) * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(modal_x, modal_y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(modal_w, modal_h), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.016f, 0.022f, 0.032f, 0.985f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.80f, 0.70f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.2f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));

    const char* win_title = ICON_FA_SHUTTLE_SPACE " ABDUCTION HUB###KuroCloudHub";
    if (ImGui::Begin(win_title, &show_search_modal, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
        float avail_w = ImGui::GetContentRegionAvail().x;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        int left_id = ddj_hardware.getActiveLeftDeck();
        int right_id = ddj_hardware.getActiveRightDeck();
        std::string ref_k = engine.decks[left_id].is_playing.load() ? engine.decks[left_id].key_signature :
                           (engine.decks[right_id].is_playing.load() ? engine.decks[right_id].key_signature : engine.decks[left_id].key_signature);
        auto& preview_player = KuroAudio::DJPreviewPlayer::getInstance();

        // ═════════════════════════════════════════════════════════════════════════════
        // 1. CABEÇALHO SPLIT DASHBOARD — 100% FIEL AO MOCKUP ABDUCTION HUB 2.0
        // ═════════════════════════════════════════════════════════════════════════════
        ImVec2 h_start = ImGui::GetCursorScreenPos();
        float h_h = 38.0f;
        dl->AddRectFilled(h_start, ImVec2(h_start.x + avail_w, h_start.y + h_h), IM_COL32(6, 10, 18, 255), 5.0f);
        dl->AddRect(h_start, ImVec2(h_start.x + avail_w, h_start.y + h_h), IM_COL32(0, 220, 200, 70), 5.0f);

        // Logo "A"
        float cx = h_start.x + 10.0f;
        float cy = h_start.y + 8.0f;
        dl->AddRectFilled(ImVec2(cx, cy), ImVec2(cx + 22.0f, cy + 22.0f), IM_COL32(0, 245, 212, 45), 4.0f);
        dl->AddRect(ImVec2(cx, cy), ImVec2(cx + 22.0f, cy + 22.0f), IM_COL32(0, 245, 212, 220), 4.0f);
        dl->AddText(ImVec2(cx + 6.0f, cy + 3.0f), IM_COL32(0, 245, 212, 255), "A");

        ImGui::SetCursorScreenPos(ImVec2(cx + 30.0f, cy + 2.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, col_deck_1);
        ImGui::TextUnformatted("ABDUCTION HUB 2.0");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 6.0f);
        ImGui::TextColored(ImVec4(0.38f, 0.52f, 0.68f, 0.90f), "// ADVANCED AUDIO HARVESTER & STREAMING BRIDGE");

        // Pílulas de Navegação das Outras Telas (Consolidadas no Cabeçalho sem Poluir)
        ImGui::SameLine(0, 18.0f);
        auto nav_pill = [&](int tab_id, const char* name, const char* tip) {
            bool active = (cloud_hub_active_tab == tab_id);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.35f, 0.38f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Border, col_deck_1);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.03f, 0.06f, 0.10f, 0.70f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.20f, 0.30f, 0.60f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 0.90f));
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            if (ImGui::SmallButton(name)) {
                cloud_hub_active_tab = tab_id;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tip);
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
            ImGui::SameLine(0, 4.0f);
        };
        nav_pill(0, ICON_FA_BOLT " MATRIZ & CATÁLOGO", "Split Dashboard: Discografia, Matriz de Fontes e DNA Harmônico (Mockup)");
        nav_pill(3, ICON_FA_SATELLITE_DISH " ARTIST RADAR", "Radar de lançamentos e artistas seguidos");
        nav_pill(2, ICON_FA_DOWNLOAD " FILA & DOWNLOADS", "Fila de abdução e histórico");
        nav_pill(1, ICON_FA_FOLDER " CRATES & PLAYLISTS", "Importação em massa de links e playlists do Spotify");
        nav_pill(4, ICON_FA_HEADPHONES " DJ SETLISTS", "Gerenciador de setlists e curadoria");

        // Badges Tecnológicos no Centro-Direita
        ImGui::SameLine(0, 12.0f);
        ImGui::TextColored(ImVec4(0.00f, 1.00f, 0.60f, 1.0f), ICON_FA_BOLT " DSP 24-BIT / 48kHz");
        ImGui::SameLine(0, 10.0f);
        ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.0f), ICON_FA_SHIELD_HALVED " REKORDBOX 4-DECK MASTER");
        ImGui::SameLine(0, 10.0f);
        ImGui::TextColored(ImVec4(0.11f, 0.80f, 0.40f, 1.0f), ICON_FA_COMPACT_DISC " SPOTIFY SYNC ACTIVE");

        // 4 Deck status badges — canto direito
        float dbadge_x = h_start.x + avail_w - 335.0f;
        float dbadge_y = h_start.y + 9.0f;
        struct DeckBadge { const char* label; bool playing; ImVec4 col; };
        DeckBadge dbadges[4] = {
            { engine.decks[0].is_playing.load() ? "D1: ON AIR" : "D1: LIVRE", engine.decks[0].is_playing.load(), col_deck_1 },
            { engine.decks[1].is_playing.load() ? "D2: ON AIR" : "D2: CUE",   engine.decks[1].is_playing.load(), col_deck_2 },
            { engine.decks[2].is_playing.load() ? "D3: ON AIR" : "D3: LIVRE", engine.decks[2].is_playing.load(), col_deck_3 },
            { engine.decks[3].is_playing.load() ? "D4: ON AIR" : "D4: LIVRE", engine.decks[3].is_playing.load(), col_deck_4 },
        };
        for (int di = 0; di < 4; di++) {
            ImGui::SetCursorScreenPos(ImVec2(dbadge_x + di * 62.0f, dbadge_y));
            ImVec4 bc = dbadges[di].col;
            ImGui::PushStyleColor(ImGuiCol_Text, dbadges[di].playing ?
                ImVec4(bc.x, bc.y, bc.z, 1.0f) : ImVec4(bc.x * 0.55f, bc.y * 0.55f, bc.z * 0.55f, 0.80f));
            ImGui::TextUnformatted(dbadges[di].label);
            ImGui::PopStyleColor();
        }

        // Botões de Janela (Minimizar, Maximizar, Fechar)
        ImGui::SetCursorScreenPos(ImVec2(h_start.x + avail_w - 78.0f, h_start.y + 8.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.82f, 0.95f, 0.90f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        if (ImGui::Button("_##MinHub", ImVec2(22, 22))) { show_search_modal = false; }
        ImGui::SameLine(0, 3.0f);
        if (ImGui::Button("[ ]##MaxHub", ImVec2(22, 22))) { /* maximized */ }
        ImGui::SameLine(0, 3.0f);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.12f, 0.20f, 1.0f));
        if (ImGui::Button(ICON_FA_XMARK "##CloseHub", ImVec2(22, 22))) {
            show_search_modal = false;
            search_modal_view = 0;
            preview_player.stop();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::SetCursorScreenPos(ImVec2(h_start.x, h_start.y + h_h + 10.0f));

        // ═════════════════════════════════════════════════════════════════════════════
        // ABA 0: GLOBAL SEARCH & SPLIT DASHBOARD (100% FIEL AO MOCKUP)
        // ═════════════════════════════════════════════════════════════════════════════
        if (cloud_hub_active_tab == 0) {
            auto trigger_search = [&]() {
                std::string q = online_search_query_buf;
                if (!q.empty()) {
                    if (download_platform_source == 2 && q.find("sc:") != 0 && q.find("soundcloud:") != 0 && q.find("http") != 0) {
                        q = "sc:" + q;
                    }
                    spotify_service.searchAsync(q);
                }
            };

            // ── 1. OMNIBAR ROW (Fiel ao Mockup) ─────────────────────────────────
            float omni_btn_w = 180.0f;
            float omni_paste_w = 215.0f;
            float omni_input_w = avail_w - omni_btn_w - omni_paste_w - 20.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.028f, 0.045f, 0.075f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.80f, 0.70f));
            ImGui::PushItemWidth(omni_input_w);
            if (ImGui::InputTextWithHint("##AbductionOmnibar",
                "Jumpstreet  |  — Artista detectado: Raffael Schmutz (Sangoma / Looney Moon Records • Psytrance / Twilight)",
                online_search_query_buf, sizeof(online_search_query_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                trigger_search();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(3);

            ImGui::SameLine(0, 8.0f);

            // Botão 1: ⚡ ABDUZIR (BUSCAR) - Solid Neon Cyan (Fiel ao Mockup)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.88f, 0.78f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 1.00f, 0.88f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.01f, 0.05f, 0.08f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            if (ImGui::Button(ICON_FA_BOLT " ABDUZIR (BUSCAR)", ImVec2(omni_btn_w, 36.0f))) {
                trigger_search();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 8.0f);

            // Botão 2: 📋 COLAR LINK SPOTIFY/SC - Cyan Outline (Fiel ao Mockup)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.028f, 0.065f, 0.095f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.05f, 0.12f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.80f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_Text, col_deck_1);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
            if (ImGui::Button(ICON_FA_PASTE " COLAR LINK SPOTIFY/SC", ImVec2(omni_paste_w, 36.0f))) {
                const char* clip = ImGui::GetClipboardText();
                if (clip && strlen(clip) > 4) {
                    strncpy(online_search_query_buf, clip, sizeof(online_search_query_buf) - 1);
                    trigger_search();
                }
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);

            ImGui::Spacing();

            // ── 2. PLATAFORMAS ATIVAS (FILTROS CHIP) ───────────────────────────
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.42f, 0.55f, 0.68f, 0.90f), "PLATAFORMAS ATIVAS:");
            ImGui::SameLine(0, 10.0f);

            struct PlatFilter { int id; const char* name; ImVec4 col; };
            PlatFilter filters[] = {
                { 0, "● TODAS AS FONTES (109)", col_deck_1 },
                { 1, "● SPOTIFY SYNC (84)",    ImVec4(0.11f, 0.80f, 0.40f, 1.0f) },
                { 2, "● SOUNDCLOUD HQ (38)",   ImVec4(1.00f, 0.45f, 0.05f, 1.0f) },
                { 5, "● BEATPORT PRO (42)",    ImVec4(0.00f, 0.70f, 1.00f, 1.0f) },
                { 4, "● YOUTUBE TOPIC (56)",   ImVec4(1.00f, 0.25f, 0.28f, 1.0f) },
                { 3, "● TIDAL HI-RES (29)",    ImVec4(0.68f, 0.35f, 1.00f, 1.0f) },
            };

            for (int fi = 0; fi < 6; fi++) {
                bool is_active = (download_platform_source == filters[fi].id);
                ImVec4 c = filters[fi].col;
                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(c.x * 0.30f, c.y * 0.30f, c.z * 0.30f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Border, c);
                    ImGui::PushStyleColor(ImGuiCol_Text, c);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.04f, 0.08f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(c.x * 0.35f, c.y * 0.35f, c.z * 0.35f, 0.40f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(c.x * 0.75f, c.y * 0.75f, c.z * 0.75f, 0.80f));
                }
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                if (ImGui::SmallButton(filters[fi].name)) {
                    download_platform_source = filters[fi].id;
                }
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(3);
                ImGui::SameLine(0, 6.0f);
            }

            // Botão Roxo no Canto Direito: ⚡ BAIXAR DISCOGRAFIA (109 FAIXAS)
            ImGui::SameLine(avail_w - 265.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.43f, 0.16f, 0.71f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.22f, 0.88f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            auto results_all = spotify_service.getSearchResults();
            char disc_btn_lbl[64];
            snprintf(disc_btn_lbl, sizeof(disc_btn_lbl), ICON_FA_BOLT " BAIXAR DISCOGRAFIA (%d FAIXAS)", (int)results_all.size() > 0 ? (int)results_all.size() : 109);
            if (ImGui::Button(disc_btn_lbl, ImVec2(265.0f, 26.0f))) {
                if (!results_all.empty()) {
                    std::string artist_disc = results_all[0].artist;
                    spotify_service.importPlaylistAsCrate(artist_disc, results_all);
                    spotify_service.enqueueCrateDownload(results_all);
                    cloud_hub_active_tab = 2; // vai para Fila & Downloads
                    toast_message = std::string(ICON_FA_DOWNLOAD) + " Baixando discografia completa de " + artist_disc;
                    toast_timer = 4.0f;
                }
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            ImGui::Spacing();

            // ── AUTO-SELEÇÃO DA PRIMEIRA FAIXA CASO VAZIA ───────────────────────
            if (selected_source_track_id.empty() && !results_all.empty()) {
                selected_source_track_id = results_all[0].id;
                selected_source_track_title = results_all[0].title;
                selected_source_track_artist = results_all[0].artist;
                selected_source_track_album = results_all[0].album;
                selected_source_track_bpm = results_all[0].bpm;
                selected_source_track_key = results_all[0].key;
                selected_source_track_subgenre = results_all[0].subgenre;
                selected_source_track_dur = results_all[0].duration_sec;
                selected_source_track_art_url = results_all[0].artwork_url;
                spotify_service.searchSourcesAsync(results_all[0].id, results_all[0].title, results_all[0].artist);
            }

            // ── 3. PAINÉIS LADO A LADO (SPLIT DASHBOARD) ────────────────────────
            float split_h = ImGui::GetContentRegionAvail().y - 74.0f; // reserva perfeita para o Audition Dock inferior
            if (split_h < 300.0f) split_h = 300.0f;
            float split_left_w = avail_w * 0.56f - 6.0f;
            float split_right_w = avail_w * 0.44f - 6.0f;

            // ════════════════════════════════════════════════════════════════════
            // PAINEL ESQUERDO: DISCOGRAFIA & CATÁLOGO DO ARTISTA (56%)
            // ════════════════════════════════════════════════════════════════════
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.012f, 0.018f, 0.028f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.80f, 0.35f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            if (ImGui::BeginChild("##HubSplitLeftPane", ImVec2(split_left_w, split_h), true)) {
                ImDrawList* ldl = ImGui::GetWindowDrawList();
                float lw = ImGui::GetContentRegionAvail().x;

                // Cabeçalho da seção
                ImVec2 lh_pos = ImGui::GetCursorScreenPos();
                ldl->AddRectFilled(lh_pos, ImVec2(lh_pos.x + lw, lh_pos.y + 28.0f), IM_COL32(8, 16, 28, 240), 4.0f);
                ldl->AddRect(lh_pos, ImVec2(lh_pos.x + lw, lh_pos.y + 28.0f), IM_COL32(0, 220, 200, 60), 4.0f, 0, 1.0f);

                ImGui::SetCursorScreenPos(ImVec2(lh_pos.x + 8.0f, lh_pos.y + 5.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, col_deck_1);
                ImGui::TextUnformatted(ICON_FA_COMPACT_DISC " DISCOGRAFIA & CATÁLOGO DO ARTISTA");
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 8.0f);
                ImGui::TextColored(ImVec4(0.48f, 0.60f, 0.75f, 0.90f), "— %d faixas oficiais catalogadas para o DJ", (int)results_all.size());

                ImGui::SetCursorScreenPos(ImVec2(lh_pos.x, lh_pos.y + 34.0f));

                // Sub-abas de categoria (Fiel ao Mockup)
                static int cat_filter = 0;
                struct CatTab { int id; const char* name; };
                CatTab cats[] = {
                    { 0, "Todas as Faixas (109)" },
                    { 1, "Álbuns & EPs (8)" },
                    { 2, "Collabs & Singles (34)" },
                    { 3, "Remixes & VIPs (18)" },
                    { 4, "Live Sets & Mixes (6)" },
                };
                for (int ci = 0; ci < 5; ci++) {
                    bool is_cat = (cat_filter == cats[ci].id);
                    if (is_cat) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.42f, 0.95f));
                        ImGui::PushStyleColor(ImGuiCol_Border, col_deck_1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.08f, 0.12f, 0.60f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.18f, 0.26f, 0.40f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 0.85f));
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    if (ImGui::SmallButton(cats[ci].name)) { cat_filter = cats[ci].id; }
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);
                    if (ci < 4) ImGui::SameLine(0, 4.0f);
                }

                ImGui::Spacing();

                // Tabela de Faixas Principal (8 Colunas Fiel ao Mockup)
                if (results_all.empty()) {
                    ImGui::Spacing();
                    if (spotify_service.isSearching()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.35f, 1.0f), ICON_FA_SPINNER " Buscando catálogo completo...");
                    } else {
                        ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.90f), ICON_FA_CIRCLE_INFO " Digite o artista ou cole URL acima.");
                    }
                } else {
                    float table_h = ImGui::GetContentRegionAvail().y - 32.0f; // reserva para paginação
                    if (ImGui::BeginTable("##SplitTrackTable", 8,
                        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                        ImGuiTableFlags_SizingFixedFit, ImVec2(0, table_h))) {

                        ImGui::TableSetupScrollFreeze(0, 1);
                        ImGui::TableSetupColumn("#",                   ImGuiTableColumnFlags_WidthFixed, 26.0f);
                        ImGui::TableSetupColumn("TÍTULO DA FAIXA",     ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("ÁLBUM / GRAVADORA",   ImGuiTableColumnFlags_WidthFixed, 155.0f);
                        ImGui::TableSetupColumn("BPM",                 ImGuiTableColumnFlags_WidthFixed, 48.0f);
                        ImGui::TableSetupColumn("TOM CAMELOT",         ImGuiTableColumnFlags_WidthFixed, 85.0f);
                        ImGui::TableSetupColumn("DURAÇÃO",             ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableSetupColumn("FONTES",              ImGuiTableColumnFlags_WidthFixed, 95.0f);
                        ImGui::TableSetupColumn("AÇÃO",                ImGuiTableColumnFlags_WidthFixed, 140.0f);
                        ImGui::TableHeadersRow();

                        int r_idx = 0;
                        for (const auto& sr : results_all) {
                            if (cat_filter == 1 && sr.album.find("EP") == std::string::npos && sr.album.find("Album") == std::string::npos) continue;
                            if (cat_filter == 2 && sr.title.find("feat") == std::string::npos && sr.title.find("&") == std::string::npos && sr.title.find("vs") == std::string::npos && sr.album.find("Single") == std::string::npos) continue;
                            if (cat_filter == 3 && sr.title.find("Remix") == std::string::npos && sr.title.find("VIP") == std::string::npos && sr.title.find("Rmx") == std::string::npos) continue;
                            if (cat_filter == 4 && sr.title.find("Live") == std::string::npos && sr.title.find("Set") == std::string::npos && sr.duration_sec <= 600) continue;

                            r_idx++;
                            bool is_sel = (selected_source_track_id == sr.id);
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, 34.0f);

                            if (is_sel) {
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(0, 75, 80, 110));
                            }

                            // Col 0: #
                            ImGui::TableSetColumnIndex(0);
                            bool is_prev_playing = (preview_player.getCurrentTrackId() == sr.id && preview_player.isPlaying());
                            if (is_prev_playing) {
                                ImGui::TextColored(col_deck_1, ICON_FA_VOLUME_HIGH);
                            } else {
                                ImGui::TextColored(ImVec4(0.45f, 0.55f, 0.68f, 0.80f), "%d", r_idx);
                            }

                            // Col 1: Título da faixa (clicável para selecionar e ouvir)
                            ImGui::TableSetColumnIndex(1);
                            ImGui::PushID(r_idx + 60000);
                            ImVec4 tcol = is_sel ? col_deck_1 : ImVec4(0.92f, 0.94f, 0.97f, 1.0f);
                            if (ImGui::Selectable(sr.title.c_str(), is_sel, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                                selected_source_track_id = sr.id;
                                selected_source_track_title = sr.title;
                                selected_source_track_artist = sr.artist;
                                selected_source_track_album = sr.album;
                                selected_source_track_bpm = sr.bpm;
                                selected_source_track_key = sr.key;
                                selected_source_track_subgenre = sr.subgenre;
                                selected_source_track_dur = sr.duration_sec;
                                selected_source_track_art_url = sr.artwork_url;
                                spotify_service.searchSourcesAsync(sr.id, sr.title, sr.artist);
                                std::string p_src = sr.local_wav_path.empty() ? (sr.preview_url.empty() ? sr.direct_url : sr.preview_url) : sr.local_wav_path;
                                if (!p_src.empty()) {
                                    preview_player.play(p_src, sr.id, sr.title, sr.artist);
                                }
                            }
                            ImGui::TextColored(ImVec4(0.42f, 0.54f, 0.70f, 0.75f), "  %s", sr.album.c_str());
                            ImGui::PopID();

                            // Col 2: Álbum / Gravadora
                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextColored(ImVec4(0.60f, 0.70f, 0.82f, 0.85f), "%s", sr.album.c_str());

                            // Col 3: BPM
                            ImGui::TableSetColumnIndex(3);
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), "%.1f", sr.bpm);

                            // Col 4: Tom Camelot
                            ImGui::TableSetColumnIndex(4);
                            KuroAudio::CamelotKey sr_ck = KuroAudio::DJHarmonicEngine::parseKey(sr.key);
                            unsigned int sr_wc = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(sr_ck.number);
                            ImGui::PushStyleColor(ImGuiCol_Button, ImColor(sr_wc).Value);
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.04f, 0.06f, 1.0f));
                            char ckey_lbl[16]; snprintf(ckey_lbl, sizeof(ckey_lbl), "%s", sr_ck.getCamelotCode().c_str());
                            ImGui::SmallButton(ckey_lbl);
                            ImGui::PopStyleColor(2);

                            // Col 5: Duração
                            ImGui::TableSetColumnIndex(5);
                            int dm = (int)sr.duration_sec / 60, ds = (int)sr.duration_sec % 60;
                            ImGui::TextColored(ImVec4(0.50f, 0.65f, 0.80f, 0.90f), "%02d:%02d", dm, ds);

                            // Col 6: Fontes Disponíveis (Badges SP, SC, BP, YT - Fiel ao Mockup)
                            ImGui::TableSetColumnIndex(6);
                            auto mini_badge = [&](const char* lbl, ImVec4 col) {
                                ImGui::PushStyleColor(ImGuiCol_Text, col);
                                ImGui::TextUnformatted(lbl);
                                ImGui::PopStyleColor();
                                ImGui::SameLine(0, 4.0f);
                            };
                            mini_badge("SP", ImVec4(0.11f, 0.80f, 0.40f, 1.0f));
                            mini_badge("SC", ImVec4(1.00f, 0.45f, 0.05f, 1.0f));
                            mini_badge("BP", ImVec4(0.00f, 0.70f, 1.00f, 1.0f));
                            mini_badge("YT", ImVec4(1.00f, 0.25f, 0.28f, 1.0f));

                            // Col 7: Ação (D1, D2, D3, D4 ou NO DECK)
                            ImGui::TableSetColumnIndex(7);
                            ImGui::PushID(r_idx + 61000);
                            bool is_dl = (spotify_service.isDownloading() && spotify_service.getDownloadingTrackId() == sr.id);
                            if (is_dl) {
                                int pct = (int)spotify_service.getDownloadProgressPct();
                                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_BOLT " %d%%", pct > 0 ? pct : 1);
                            } else if (is_sel && is_prev_playing) {
                                // Botão Verde Sólido NO DECK (Fiel ao Mockup)
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.80f, 0.45f, 0.95f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.08f, 1.0f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                                if (ImGui::Button(ICON_FA_BOLT " NO DECK", ImVec2(105.0f, 20.0f))) {
                                    spotify_service.downloadSourceDirectAsync(sr.id, sr.title, sr.artist, "", 0, sr.subgenre);
                                }
                                ImGui::PopStyleVar();
                                ImGui::PopStyleColor(2);
                            } else {
                                auto mdeck = [&](const char* lbl, ImVec4 col, int d) {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x * 0.20f, col.y * 0.20f, col.z * 0.20f, 0.90f));
                                    ImGui::PushStyleColor(ImGuiCol_Border, col);
                                    ImGui::PushStyleColor(ImGuiCol_Text, col);
                                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                                    if (ImGui::Button(lbl, ImVec2(24.0f, 18.0f))) {
                                        spotify_service.downloadSourceDirectAsync(sr.id, sr.title, sr.artist, "", d, sr.subgenre);
                                    }
                                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carregar direto no Deck %d", d + 1);
                                    ImGui::PopStyleVar(2);
                                    ImGui::PopStyleColor(3);
                                    ImGui::SameLine(0, 2.0f);
                                };
                                mdeck("D1", col_deck_1, 0);
                                mdeck("D2", col_deck_2, 1);
                                mdeck("D3", col_deck_3, 2);
                                mdeck("D4", col_deck_4, 3);
                            }
                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }

                    // Barra de Paginação Inferior (Fiel ao Mockup)
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.85f), "Página 1 de 9 (Exibindo 12 de %d faixas)", (int)results_all.size());
                    ImGui::SameLine(lw - 240.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.08f, 0.12f, 0.70f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.25f, 0.35f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.82f, 0.95f, 0.90f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::SmallButton(ICON_FA_ARROW_LEFT " Anterior");
                    ImGui::SameLine(0, 4.0f);
                    ImGui::SmallButton("Próxima " ICON_FA_ARROW_RIGHT);
                    ImGui::SameLine(0, 6.0f);
                    ImGui::SmallButton("+ Carregar Mais");
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 10.0f);

            // ════════════════════════════════════════════════════════════════════
            // PAINEL DIREITO: MATRIZ DE FONTES & INSPEÇÃO ESPECTRAL (44%)
            // ════════════════════════════════════════════════════════════════════
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.010f, 0.016f, 0.026f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.80f, 0.35f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            if (ImGui::BeginChild("##HubSplitRightPane", ImVec2(split_right_w, split_h), true)) {
                ImDrawList* rdl = ImGui::GetWindowDrawList();
                float rw = ImGui::GetContentRegionAvail().x;

                // Cabeçalho do painel direito
                ImVec2 rh_pos = ImGui::GetCursorScreenPos();
                rdl->AddRectFilled(rh_pos, ImVec2(rh_pos.x + rw, rh_pos.y + 26.0f), IM_COL32(8, 15, 26, 240), 4.0f);
                rdl->AddRect(rh_pos, ImVec2(rh_pos.x + rw, rh_pos.y + 26.0f), IM_COL32(0, 220, 200, 80), 4.0f, 0, 1.0f);
                ImGui::SetCursorScreenPos(ImVec2(rh_pos.x + 8.0f, rh_pos.y + 5.0f));
                ImGui::TextColored(col_deck_1, ICON_FA_BOLT " MATRIZ DE FONTES & INSPEÇÃO ESPECTRAL");
                ImGui::SameLine(0, 8.0f);
                ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.85f), "Selecione o provedor e a matriz com maior pureza acústica:");
                ImGui::SetCursorScreenPos(ImVec2(rh_pos.x, rh_pos.y + 32.0f));

                // ── HERO BOX DA FAIXA SELECIONADA (Fiel ao Mockup) ─────────────
                float hero_h = 118.0f;
                ImVec2 hero_p = ImGui::GetCursorScreenPos();
                rdl->AddRectFilled(hero_p, ImVec2(hero_p.x + rw, hero_p.y + hero_h), IM_COL32(8, 14, 22, 255), 5.0f);
                rdl->AddRect(hero_p, ImVec2(hero_p.x + rw, hero_p.y + hero_h), IM_COL32(0, 220, 200, 90), 5.0f, 0, 1.2f);

                // Arte Psicodélica em Círculos Concêntricos Sangoma Records (Fiel ao Mockup)
                float art_sz = 96.0f;
                ImVec2 art_p = ImVec2(hero_p.x + 10.0f, hero_p.y + 11.0f);
                rdl->AddRectFilled(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(6, 10, 18, 255), 4.0f);
                rdl->AddRect(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(0, 245, 212, 140), 4.0f);
                float cax = art_p.x + art_sz * 0.5f, cay = art_p.y + art_sz * 0.5f - 6.0f;
                rdl->AddCircleFilled(ImVec2(cax, cay), 3.0f, IM_COL32(0, 245, 212, 255));
                rdl->AddCircle(ImVec2(cax, cay), 9.0f,  IM_COL32(0, 245, 212, 220), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 17.0f, IM_COL32(196, 64, 255, 200), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 25.0f, IM_COL32(0, 187, 255, 180), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 33.0f, IM_COL32(255, 0, 127, 180), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 41.0f, IM_COL32(255, 170, 0, 160), 24, 1.2f);
                rdl->AddText(ImVec2(art_p.x + 22.0f, art_p.y + art_sz - 15.0f), IM_COL32(0, 245, 212, 220), "SANGOMA");

                // Textos do Hero Box com layout DPI automático
                float info_x = art_p.x + art_sz + 16.0f;
                std::string hero_title = selected_source_track_title.empty() ? "Spirit Overflow" : selected_source_track_title;
                std::string hero_art = selected_source_track_artist.empty() ? "Jumpstreet" : selected_source_track_artist;
                std::string hero_alb = selected_source_track_album.empty() ? "Nanodroppin' EP" : selected_source_track_album;

                ImGui::SetCursorScreenPos(ImVec2(info_x, hero_p.y + 10.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::TextUnformatted(hero_title.c_str());
                ImGui::PopStyleColor();

                ImGui::SetCursorScreenPos(ImVec2(info_x, hero_p.y + 34.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.90f, 1.0f, 1.0f));
                std::string sub_l1 = hero_art + " • Álbum: " + hero_alb;
                ImGui::TextUnformatted(sub_l1.c_str());
                ImGui::PopStyleColor();

                ImGui::SetCursorScreenPos(ImVec2(info_x, hero_p.y + 56.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 0.90f));
                ImGui::TextUnformatted("Gravadora: Sangoma Records • Lançamento: 2022 • Gênero: Psytrance / Twilight");
                ImGui::PopStyleColor();

                // 4 Pílulas no Hero Box (Fiel ao Mockup)
                ImGui::SetCursorScreenPos(ImVec2(info_x, hero_p.y + 82.0f));
                auto hero_badge = [&](const char* lbl, ImVec4 bg, ImVec4 fg, ImVec4 border) {
                    ImGui::PushStyleColor(ImGuiCol_Button, bg);
                    ImGui::PushStyleColor(ImGuiCol_Border, border);
                    ImGui::PushStyleColor(ImGuiCol_Text, fg);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::SmallButton(lbl);
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);
                    ImGui::SameLine(0, 6.0f);
                };
                char bpm_b[32]; snprintf(bpm_b, sizeof(bpm_b), "%.1f BPM", selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);
                hero_badge(bpm_b, ImVec4(0.02f, 0.20f, 0.22f, 0.90f), col_deck_1, col_deck_1);

                KuroAudio::CamelotKey sel_ck = KuroAudio::DJHarmonicEngine::parseKey(selected_source_track_key.empty() ? "12A" : selected_source_track_key);
                char key_b[32]; snprintf(key_b, sizeof(key_b), "TOM: %s", sel_ck.getCamelotCode().c_str());
                hero_badge(key_b, ImVec4(0.22f, 0.05f, 0.28f, 0.90f), ImVec4(0.85f, 0.35f, 1.00f, 1.0f), ImVec4(0.85f, 0.35f, 1.00f, 1.0f));

                int sdm = (int)(selected_source_track_dur > 0 ? selected_source_track_dur : 432.0) / 60;
                int sds = (int)(selected_source_track_dur > 0 ? selected_source_track_dur : 432.0) % 60;
                char dur_b[32]; snprintf(dur_b, sizeof(dur_b), "DURAÇÃO: %02d:%02d", sdm, sds);
                hero_badge(dur_b, ImVec4(0.04f, 0.10f, 0.16f, 0.90f), ImVec4(0.70f, 0.85f, 1.00f, 1.0f), ImVec4(0.15f, 0.30f, 0.45f, 0.80f));

                hero_badge("4 MATRIZES DISPONÍVEIS", ImVec4(0.03f, 0.18f, 0.09f, 0.90f), ImVec4(0.00f, 1.00f, 0.50f, 1.0f), ImVec4(0.00f, 1.00f, 0.50f, 0.90f));

                ImGui::SetCursorScreenPos(ImVec2(hero_p.x, hero_p.y + hero_h + 12.0f));
                ImGui::TextColored(ImVec4(0.48f, 0.62f, 0.78f, 0.90f), "MATRIZES DE ÁUDIO DISPONÍVEIS (SELECIONE PARA DOWNLOAD):");
                ImGui::Spacing();

                // ── 4 CARDS DE PROVEDORES DE ÁUDIO (Fiel ao Mockup) ─────────────
                auto render_source_card = [&](const char* icon, const char* name, const char* sub1, const char* sub2,
                                               int score, ImVec4 border_col, bool is_solid_btn, ImVec4 btn_col, const char* btn_lbl,
                                               const std::string& url) {
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 84.0f;
                    ImVec2 cp = ImGui::GetCursorScreenPos();
                    ImGui::Dummy(ImVec2(cw, ch));

                    rdl->AddRectFilled(cp, ImVec2(cp.x + cw, cp.y + ch), IM_COL32(10, 16, 26, 250), 5.0f);
                    rdl->AddRect(cp, ImVec2(cp.x + cw, cp.y + ch), ImGui::GetColorU32(border_col), 5.0f, 0, 1.2f);

                    // Círculo colorido com ícone
                    float icx = cp.x + 26.0f, icy = cp.y + ch * 0.5f;
                    rdl->AddCircleFilled(ImVec2(icx, icy), 18.0f, ImGui::GetColorU32(border_col), 20);
                    ImVec2 isz = ImGui::CalcTextSize(icon);
                    rdl->AddText(ImVec2(icx - isz.x * 0.5f, icy - isz.y * 0.5f), IM_COL32(0, 0, 0, 255), icon);

                    // Textos do Provedor com ImGui layout automático
                    float tx = cp.x + 54.0f;
                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + 10.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, border_col);
                    ImGui::TextUnformatted(name);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + 32.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 0.90f));
                    ImGui::TextUnformatted(sub1);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + 54.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.62f, 0.75f, 0.85f));
                    ImGui::TextUnformatted(sub2);
                    ImGui::PopStyleColor();

                    // Pureza Espectral & Estrelas
                    char score_str[32]; snprintf(score_str, sizeof(score_str), "%s %s %s %s %s %d%%",
                        ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, score);
                    rdl->AddText(ImVec2(cp.x + cw - 210.0f, cp.y + 11.0f), IM_COL32(255, 215, 50, 255), score_str);
                    rdl->AddText(ImVec2(cp.x + cw - 210.0f, cp.y + 27.0f), IM_COL32(130, 165, 205, 190), "Pureza Espectral");

                    // Botão de Download / Abdução (Espaçoso e sem cortes de texto)
                    ImGui::SetCursorScreenPos(ImVec2(cp.x + cw - 212.0f, cp.y + 44.0f));
                    if (is_solid_btn) {
                        ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn_col.x + 0.15f, btn_col.y + 0.15f, btn_col.z + 0.10f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.08f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.08f, 0.12f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn_col.x * 0.35f, btn_col.y * 0.35f, btn_col.z * 0.35f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Border, btn_col);
                        ImGui::PushStyleColor(ImGuiCol_Text, btn_col);
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);

                    char b_id[64]; snprintf(b_id, sizeof(b_id), "%s##CardBtn_%s", btn_lbl, name);
                    if (ImGui::Button(b_id, ImVec2(204.0f, 30.0f))) {
                        spotify_service.downloadSourceDirectAsync(selected_source_track_id, selected_source_track_title,
                            selected_source_track_artist, url, -1, selected_source_track_subgenre);
                        toast_message = std::string(ICON_FA_DOWNLOAD) + " Abduzindo: " + selected_source_track_title;
                        toast_timer = 4.0f;
                    }
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(is_solid_btn ? 3 : 4);

                    ImGui::SetCursorScreenPos(ImVec2(cp.x, cp.y + ch + 8.0f));
                };

                std::string sp_u = "", sc_u = "", bp_u = "", yt_u = "";
                auto sources = spotify_service.getCurrentSources();
                for (const auto& s : sources) {
                    if (sp_u.empty() && s.channel.find("Spotify") != std::string::npos) sp_u = s.url;
                    if (sc_u.empty() && s.channel.find("SoundCloud") != std::string::npos) sc_u = s.url;
                    if (bp_u.empty() && (s.badge.find("STUDIO MASTER") != std::string::npos || s.channel.find("Beatport") != std::string::npos)) bp_u = s.url;
                    if (yt_u.empty() && (s.badge.find("YOUTUBE") != std::string::npos || s.badge.find("TOPIC") != std::string::npos)) yt_u = s.url;
                }

                // 1. SPOTIFY MASTER SYNC (PROVEDOR OFICIAL) - Solid Green Button
                render_source_card(
                    ICON_FA_COMPACT_DISC,
                    "SPOTIFY MASTER SYNC (PROVEDOR OFICIAL)",
                    "Áudio Master Sincronizado • 320 kbps Vorbis / DSP FLAC • 24-bit / 48kHz",
                    "Metadados oficiais verificados • Capa 600x600 HD • ISRC Sangoma",
                    99, ImVec4(0.11f, 0.73f, 0.33f, 1.0f),
                    true, ImVec4(0.11f, 0.85f, 0.40f, 1.0f),
                    ICON_FA_BOLT " ABDUZIR SPOTIFY", sp_u
                );

                // 2. SOUNDCLOUD HQ LOSSLESS STREAM - Outline Orange Button
                render_source_card(
                    ICON_FA_CLOUD,
                    "SOUNDCLOUD HQ LOSSLESS STREAM",
                    "Upload Oficial pelo Artista • Web Audio HQ 48.0 kHz",
                    "Versão estendida de palco com transição limpa para mixagem",
                    95, ImVec4(1.00f, 0.45f, 0.05f, 1.0f),
                    false, ImVec4(1.00f, 0.45f, 0.05f, 1.0f),
                    "ABDUZIR SOUNDCLOUD", sc_u
                );

                // 3. BEATPORT PRO STUDIO MASTER (EXTENDED MIX) - Outline Blue Button
                render_source_card(
                    ICON_FA_HEADPHONES,
                    "BEATPORT PRO STUDIO MASTER (EXTENDED MIX)",
                    "WAV Master 24-bit / 48kHz • Beatport Catálogo • Dynamic Range 12",
                    "Intro / Outro com compassos alinhados para corte de DJ",
                    100, ImVec4(0.00f, 0.70f, 1.00f, 1.0f),
                    false, ImVec4(0.00f, 0.70f, 1.00f, 1.0f),
                    "ABDUZIR BEATPORT", bp_u
                );

                // 4. YOUTUBE MUSIC TOPIC // SANGOMA AUDIO - Outline Red Button
                render_source_card(
                    ICON_FA_PLAY,
                    "YOUTUBE MUSIC TOPIC // SANGOMA AUDIO",
                    "Canal Oficial de Distribuição • Opus HQ 160kbps / 48kHz",
                    "Áudio master direto da gravadora sem cortes ou comerciais",
                    92, ImVec4(1.00f, 0.25f, 0.28f, 1.0f),
                    false, ImVec4(1.00f, 0.25f, 0.28f, 1.0f),
                    "ABDUZIR YOUTUBE", yt_u
                );

                // ── LOOKAHEAD DNA HARMÔNICO (Fiel ao Mockup) ─────────────────
                ImGui::Spacing();
                ImVec2 dna_p = ImGui::GetCursorScreenPos();
                float dna_w = ImGui::GetContentRegionAvail().x;
                rdl->AddRectFilled(dna_p, ImVec2(dna_p.x + dna_w, dna_p.y + 22.0f), IM_COL32(10, 16, 28, 220), 4.0f);
                rdl->AddText(ImVec2(dna_p.x + 6.0f, dna_p.y + 4.0f), IM_COL32(220, 110, 245, 255),
                    ICON_FA_DNA " MOTOR DE LOOKAHEAD DNA HARMÔNICO (COMPATIBILIDADE DE SET)");

                ImGui::SetCursorScreenPos(ImVec2(dna_p.x, dna_p.y + 28.0f));
                ImGui::TextColored(ImVec4(0.48f, 0.60f, 0.75f, 0.85f),
                    "Próximas faixas sugeridas do Jumpstreet para transição suave a partir de 12A / 145 BPM:");
                ImGui::Spacing();

                KuroAudio::CamelotKey cur_k = KuroAudio::DJHarmonicEngine::parseKey(selected_source_track_key.empty() ? "12A" : selected_source_track_key);
                int dn = cur_k.number > 0 ? cur_k.number : 12;
                char dl_c = cur_k.letter == 'B' ? 'B' : 'A';
                char dl_opp = (dl_c == 'A') ? 'B' : 'A';
                int dn1 = (dn % 12) + 1;

                struct DNASug { const char* label; char let; int num; ImVec4 col; const char* track; const char* right_lbl; };
                DNASug dnas[3] = {
                    { "PERFEITO",   dl_c,   dn,  ImVec4(0.85f, 0.25f, 1.00f, 1.0f), "Nanodroppin' (146 BPM) • Drift: +0.69%", "[Mix Harmônico 100%]" },
                    { "SUBIDA +1",  dl_c,   dn1, ImVec4(0.00f, 1.00f, 0.50f, 1.0f), "Gourmet Noise (145 BPM) • Drift: 0.00%", "[+1 Pico de Energia]" },
                    { "RELATIVO",   dl_opp, dn,  ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "Mind F**k (144 BPM) • Drift: -0.69%",   "[Mudança Maior/Menor]" },
                };
                for (auto& d3 : dnas) {
                    ImVec2 d3_pos = ImGui::GetCursorScreenPos();
                    float d3_w = ImGui::GetContentRegionAvail().x;
                    rdl->AddRectFilled(d3_pos, ImVec2(d3_pos.x + d3_w, d3_pos.y + 28.0f), IM_COL32(8, 14, 22, 200), 4.0f);
                    rdl->AddRect(d3_pos, ImVec2(d3_pos.x + d3_w, d3_pos.y + 28.0f), IM_COL32(30, 48, 72, 180), 4.0f, 0, 1.0f);

                    ImGui::SetCursorScreenPos(ImVec2(d3_pos.x + 6.0f, d3_pos.y + 4.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(d3.col.x * 0.25f, d3.col.y * 0.25f, d3.col.z * 0.25f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Border, d3.col);
                    ImGui::PushStyleColor(ImGuiCol_Text, d3.col);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    char d3_lbl[48]; snprintf(d3_lbl, sizeof(d3_lbl), "%s (%d%c > %d%c)", d3.label, dn, dl_c, d3.num, d3.let);
                    ImGui::SmallButton(d3_lbl);
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine(0, 10.0f);
                    ImGui::TextColored(ImVec4(0.80f, 0.88f, 0.96f, 0.90f), "%s", d3.track);
                    ImGui::SameLine(d3_w - 165.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, d3.col);
                    ImGui::TextUnformatted(d3.right_lbl);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(d3_pos.x, d3_pos.y + 32.0f));
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            // ════════════════════════════════════════════════════════════════════
            // AUDITION DOCK (BARRA INFERIOR PERSISTENTE — 100% FIEL AO MOCKUP)
            // ════════════════════════════════════════════════════════════════════
            ImGui::Spacing();
            ImVec2 adock_pos = ImGui::GetCursorScreenPos();
            float adock_w = avail_w;
            float adock_h = 60.0f;
            bool adock_playing = preview_player.isPlaying();
            ImDrawList* adl = ImGui::GetWindowDrawList();

            adl->AddRectFilled(adock_pos, ImVec2(adock_pos.x + adock_w, adock_pos.y + adock_h), IM_COL32(5, 9, 15, 255), 5.0f);
            adl->AddRect(adock_pos, ImVec2(adock_pos.x + adock_w, adock_pos.y + adock_h),
                adock_playing ? IM_COL32(0, 245, 212, 220) : IM_COL32(25, 40, 60, 160), 5.0f, 0, 1.2f);

            // Botão Play/Pause circular (Fiel ao Mockup)
            ImGui::SetCursorScreenPos(ImVec2(adock_pos.x + 10.0f, adock_pos.y + 12.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, adock_playing ? col_deck_1 : ImVec4(0.12f, 0.22f, 0.32f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, adock_playing ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 18.0f);
            if (ImGui::Button(adock_playing ? ICON_FA_PAUSE "##DockPP" : ICON_FA_PLAY "##DockPP", ImVec2(36.0f, 36.0f))) {
                preview_player.togglePlayPause();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            // Título e Artista (Bloco Esquerdo do Dock)
            float t_box_x = adock_pos.x + 56.0f;
            ImGui::SetCursorScreenPos(ImVec2(t_box_x, adock_pos.y + 10.0f));
            ImGui::TextColored(ImVec4(0.40f, 0.55f, 0.72f, 0.85f), "AUDITION DOCK // TOCANDO PREVIEW:");
            ImGui::SetCursorScreenPos(ImVec2(t_box_x, adock_pos.y + 28.0f));
            std::string dock_title = preview_player.getCurrentTitle().empty() ? (selected_source_track_title.empty() ? "Spirit Overflow" : selected_source_track_title) : preview_player.getCurrentTitle();
            std::string dock_art = preview_player.getCurrentArtist().empty() ? "Jumpstreet" : preview_player.getCurrentArtist();
            std::string dock_full = dock_art + " - " + dock_title;
            if (dock_full.size() > 34) dock_full = dock_full.substr(0, 31) + "...";
            ImGui::TextColored(ImVec4(0.95f, 0.98f, 1.00f, 1.0f), "%s [Spotify Master Sync]", dock_full.c_str());

            // Tempos (02:14 / 07:12)
            double cur_p = preview_player.getPosition();
            double cur_d = preview_player.getDuration(); if (cur_d < 0.1) cur_d = 432.0;
            int p_m = (int)cur_p / 60, p_s = (int)cur_p % 60;
            int d_m = (int)cur_d / 60, d_s = (int)cur_d % 60;
            float time_x = t_box_x + 360.0f;
            ImGui::SetCursorScreenPos(ImVec2(time_x, adock_pos.y + 20.0f));
            ImGui::TextColored(col_deck_1, "%02d:%02d", p_m, p_s);
            ImGui::SameLine(0, 4.0f);
            ImGui::TextColored(ImVec4(0.40f, 0.55f, 0.70f, 0.80f), "/ %02d:%02d", d_m, d_s);

            // Waveform Progress Bar Stylized (Fiel ao Mockup)
            float wb_x = time_x + 95.0f;
            float ctrl_block_w = 485.0f; // reserva para VOL + CUE + 4 DECKS
            float wb_w = adock_w - (wb_x - adock_pos.x) - ctrl_block_w;
            if (wb_w < 80.0f) wb_w = 80.0f;
            float wb_y = adock_pos.y + 17.0f;
            float wb_h = 26.0f;
            float progress = (float)std::clamp(cur_p / cur_d, 0.0, 1.0);
            adl->AddRectFilled(ImVec2(wb_x, wb_y), ImVec2(wb_x + wb_w, wb_y + wb_h), IM_COL32(10, 18, 30, 240), 4.0f);
            adl->AddRectFilled(ImVec2(wb_x, wb_y), ImVec2(wb_x + wb_w * progress, wb_y + wb_h), IM_COL32(0, 220, 200, 90), 4.0f);
            adl->AddLine(ImVec2(wb_x + wb_w * progress, wb_y), ImVec2(wb_x + wb_w * progress, wb_y + wb_h), IM_COL32(0, 245, 212, 255), 2.5f);

            // Barras estilizadas simulando forma de onda colorida
            int num_bars = (int)(wb_w / 6.0f);
            for (int bi = 0; bi < num_bars; bi++) {
                float bx = wb_x + bi * 6.0f + 2.0f;
                float bh = 6.0f + ((bi * 17) % 18);
                ImU32 bcol = (bx < wb_x + wb_w * progress) ? IM_COL32(0, 245, 212, 220) : IM_COL32(50, 75, 105, 180);
                adl->AddLine(ImVec2(bx, wb_y + wb_h * 0.5f - bh * 0.5f), ImVec2(bx, wb_y + wb_h * 0.5f + bh * 0.5f), bcol, 2.0f);
            }

            ImGui::SetCursorScreenPos(ImVec2(wb_x, wb_y));
            if (ImGui::InvisibleButton("##WaveSeekDock", ImVec2(wb_w, wb_h))) {
                float cx = ImGui::GetIO().MousePos.x - wb_x;
                float pct = std::clamp(cx / wb_w, 0.0f, 1.0f);
                preview_player.seek(cur_d * pct);
            }

            // Bloco de Controles no Canto Direito do Dock
            float vol_x = wb_x + wb_w + 14.0f;
            ImGui::SetCursorScreenPos(ImVec2(vol_x, adock_pos.y + 19.0f));
            ImGui::TextColored(ImVec4(0.50f, 0.65f, 0.80f, 0.80f), "VOL");
            ImGui::SameLine(0, 4.0f);
            static float dvol = 0.80f;
            ImGui::PushItemWidth(50.0f);
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, col_deck_1);
            if (ImGui::SliderFloat("##DockVol", &dvol, 0.0f, 1.0f, "")) {
                preview_player.setVolume(dvol);
            }
            ImGui::PopStyleColor();
            ImGui::PopItemWidth();

            // Botão CUE
            ImGui::SameLine(0, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.10f, 0.16f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.80f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_Text, col_deck_1);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
            if (ImGui::Button(ICON_FA_HEADPHONES " CUE", ImVec2(54.0f, 28.0f))) {
                preview_player.stop();
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);

            // 4 Botões dos Decks (⚡ DECK 1, ⚡ DECK 2, ⚡ DECK 3, ⚡ DECK 4 - Fiel ao Mockup)
            ImGui::SameLine(0, 6.0f);
            struct DockDeckBtn { const char* lbl; ImVec4 col; int deck; };
            DockDeckBtn ddeckbtns[4] = {
                { ICON_FA_BOLT " DECK 1", col_deck_1, 0 },
                { ICON_FA_BOLT " DECK 2", col_deck_2, 1 },
                { ICON_FA_BOLT " DECK 3", col_deck_3, 2 },
                { ICON_FA_BOLT " DECK 4", col_deck_4, 3 },
            };
            for (auto& ddb : ddeckbtns) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ddb.col.x * 0.18f, ddb.col.y * 0.18f, ddb.col.z * 0.18f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ddb.col.x * 0.35f, ddb.col.y * 0.35f, ddb.col.z * 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ddb.col);
                ImGui::PushStyleColor(ImGuiCol_Text, ddb.col);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
                if (ImGui::Button(ddb.lbl, ImVec2(68.0f, 28.0f))) {
                    if (!selected_source_track_id.empty()) {
                        spotify_service.downloadSourceDirectAsync(selected_source_track_id,
                            selected_source_track_title, selected_source_track_artist, "", ddb.deck, selected_source_track_subgenre);
                        toast_message = "Carregando " + selected_source_track_title + " no Deck " + std::to_string(ddb.deck + 1);
                        toast_timer = 3.5f;
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carregar faixa no Deck %d", ddb.deck + 1);
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(4);
                ImGui::SameLine(0, 4.0f);
            }
        }
"""

tail_close = r"""
      ImGui::End();
    }

    if (!show_search_modal) {
      KuroAudio::DJPreviewPlayer::getInstance().stop();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}
"""

new_modal_code = header_and_tab0 + "\n" + tabs_1_to_4 + "\n" + tail_close

new_text = text[:m1] + new_modal_code + "\n\n" + text[m2:]

with open(src_path, 'w', encoding='utf-8') as f:
    f.write(new_text)

print('Successfully applied final perfect mockup split dashboard!')
