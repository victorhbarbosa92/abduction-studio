import sys

def apply_patch():
    target_file = 'src/ui/KuroDJStudioUI.h'
    with open(target_file, 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Update openSourceMatrix to ensure show_search_modal = true;
    m1_target = '''        void openSourceMatrix(const KuroAudio::SpotifyDJTrack& trk) {
            search_modal_view = 1;'''
    m1_repl = '''        void openSourceMatrix(const KuroAudio::SpotifyDJTrack& trk) {
            show_search_modal = true;
            search_modal_view = 1;'''
    if m1_target in content:
        content = content.replace(m1_target, m1_repl, 1)
        print('[+] Patched openSourceMatrix(SpotifyDJTrack)')
    else:
        print('[!] m1_target not found (might already be patched)')

    m2_target = '''        void openSourceMatrix(const KuroAudio::DJLibraryTrack& trk) {
            search_modal_view = 1;'''
    m2_repl = '''        void openSourceMatrix(const KuroAudio::DJLibraryTrack& trk) {
            show_search_modal = true;
            search_modal_view = 1;'''
    if m2_target in content:
        content = content.replace(m2_target, m2_repl, 1)
        print('[+] Patched openSourceMatrix(DJLibraryTrack)')
    else:
        print('[!] m2_target not found (might already be patched)')

    # 2. Update Rekordbox Top Bar button in renderRekordboxTopBar
    topbar_target = '''            // Botão Cyberpunk / Rekordbox: CENTRAL DE FONTES & DOWNLOADS
            {
                ImVec2 s_pos = ImGui::GetCursorScreenPos();
                bool is_dl = spotify_service.isDownloading();
                float pulse = (std::sin((float)ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
                
                std::string btn_label = is_dl ? (std::string(ICON_FA_BOLT) + " BAIXANDO...")
                                              : (show_search_modal ? (std::string(ICON_FA_CLOUD_ARROW_DOWN) + " FONTES // CLOUD") 
                                                                   : (std::string(ICON_FA_CLOUD_ARROW_DOWN) + " FONTES"));
                ImVec2 t_sz = ImGui::CalcTextSize(btn_label.c_str());
                float s_w = std::max(104.0f, t_sz.x + 24.0f);
                float s_h = 20.0f;'''

    topbar_repl = '''            // Botão Cyberpunk / Rekordbox: ABDUCTION STUDIO // MASTER SOURCE MATRIX
            {
                ImVec2 s_pos = ImGui::GetCursorScreenPos();
                bool is_dl = spotify_service.isDownloading();
                float pulse = (std::sin((float)ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
                int dl_pct = (int)spotify_service.getDownloadProgressPct();
                
                std::string btn_label;
                if (is_dl) {
                    if (dl_pct > 0) {
                        btn_label = std::string(ICON_FA_BOLT) + " ABDUZINDO " + std::to_string(dl_pct) + "%";
                    } else {
                        btn_label = std::string(ICON_FA_BOLT) + " ABDUZINDO...";
                    }
                } else {
                    btn_label = show_search_modal ? (std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUCTION HUB") 
                                                  : (std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUZIR");
                }
                ImVec2 t_sz = ImGui::CalcTextSize(btn_label.c_str());
                float s_w = std::max(110.0f, t_sz.x + 24.0f);
                float s_h = 20.0f;'''

    if topbar_target in content:
        content = content.replace(topbar_target, topbar_repl, 1)
        print('[+] Patched Rekordbox Top Bar button')
    else:
        print('[!] topbar_target not found')

    topbar_tt_target = '''                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("CENTRAL DE FONTES & DOWNLOADS (KURO CLOUD)\\n"
                                      "• Clique para abrir o Streaming Hub e buscador multi-plataformas\\n"
                                      "• Fontes integradas: Spotify, SoundCloud HQ, Tidal Hi-Fi, YouTube Music, Beatport\\n"
                                      "• Status: %s", is_dl ? "Baixando faixas ativamente..." : "Online & Pronto");
                }'''
    topbar_tt_repl = '''                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("ABDUCTION STUDIO // MASTER SOURCE MATRIX\\n"
                                      "• Clique para abrir a matriz de fontes e abduzir faixas em pureza de estúdio\\n"
                                      "• Fontes integradas: Beatport Extended, Tidal Master, SoundCloud HQ, YouTube Topic, Spotify\\n"
                                      "• Status: %s", is_dl ? ("Abduzindo faixa de estúdio (" + std::to_string(dl_pct > 0 ? dl_pct : 1) + "%)...").c_str() : "Pronto para Abdução");
                }'''
    if topbar_tt_target in content:
        content = content.replace(topbar_tt_target, topbar_tt_repl, 1)
        print('[+] Patched Rekordbox Top Bar tooltip')
    else:
        print('[!] topbar_tt_target not found')

    # 3. Add renderMasterSourceMatrixCard method and hook into renderSearchModal
    search_modal_entry = '''        void renderSearchModal(KuroAudio::DJEngine& engine) {
            if (!show_search_modal) return;'''

    search_modal_hook = '''        void renderMasterSourceMatrixCard(KuroAudio::DJEngine& engine) {
            ImGuiViewport* vp = ImGui::GetMainViewport();

            // 1. Escurecimento Total do Fundo (Backdrop Overlay)
            ImDrawList* fg_dl = ImGui::GetForegroundDrawList();
            fg_dl->AddRectFilled(vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y), IM_COL32(2, 5, 10, 230));

            // 2. Card Centralizado Ocupando ~85% da Tela
            float card_w = std::clamp(vp->Size.x * 0.84f, 840.0f, 1180.0f);
            float card_h = std::clamp(vp->Size.y * 0.86f, 580.0f, 890.0f);
            float card_x = vp->Pos.x + (vp->Size.x - card_w) * 0.5f;
            float card_y = vp->Pos.y + (vp->Size.y - card_h) * 0.5f;

            ImGui::SetNextWindowPos(ImVec2(card_x, card_y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(card_w, card_h), ImGuiCond_Always);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.012f, 0.020f, 0.032f, 0.99f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.88f, 0.82f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.8f);

            if (ImGui::Begin("##AbductionMasterSourceMatrixCard", &show_search_modal, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
                float avail_w = ImGui::GetContentRegionAvail().x;
                ImDrawList* dl = ImGui::GetWindowDrawList();
                int left_id = ddj_hardware.getActiveLeftDeck();
                int right_id = ddj_hardware.getActiveRightDeck();
                std::string ref_k = engine.decks[left_id].is_playing.load() ? engine.decks[left_id].key_signature : 
                                   (engine.decks[right_id].is_playing.load() ? engine.decks[right_id].key_signature : engine.decks[left_id].key_signature);

                // --- 1. CABEÇALHO ELEGANTE DE ESTÚDIO ---
                ImVec2 head_pos = ImGui::GetCursorScreenPos();
                dl->AddRectFilled(head_pos, ImVec2(head_pos.x + avail_w, head_pos.y + 44.0f), IM_COL32(10, 18, 28, 240), 6.0f);
                dl->AddRect(head_pos, ImVec2(head_pos.x + avail_w, head_pos.y + 44.0f), IM_COL32(0, 220, 210, 140), 6.0f, 0, 1.0f);

                ImGui::SetCursorScreenPos(ImVec2(head_pos.x + 12.0f, head_pos.y + 6.0f));
                ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_SHUTTLE_SPACE " ABDUCTION STUDIO // MASTER SOURCE MATRIX");
                
                ImGui::SetCursorScreenPos(ImVec2(head_pos.x + 12.0f, head_pos.y + 24.0f));
                ImGui::TextColored(ImVec4(0.55f, 0.72f, 0.88f, 1.0f), "Inspeção de Pureza Espectral & Seleção de Matriz de Áudio");

                // Badges no centro do cabeçalho
                ImGui::SetCursorScreenPos(ImVec2(head_pos.x + 480.0f, head_pos.y + 12.0f));
                ImGui::TextColored(ImVec4(0.00f, 1.00f, 0.60f, 1.0f), ICON_FA_BOLT " [DSP 24-BIT / 48kHz]");
                ImGui::SameLine(0, 10.0f);
                ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.0f), ICON_FA_SHIELD_HALVED " [REKORDBOX MASTER]");

                // Botões Voltar e Fechar
                ImGui::SetCursorScreenPos(ImVec2(head_pos.x + avail_w - 235.0f, head_pos.y + 9.0f));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.22f, 0.32f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.35f, 0.50f, 1.0f));
                if (ImGui::Button(ICON_FA_ARROW_LEFT " VOLTAR À BUSCA", ImVec2(140.0f, 26.0f))) {
                    search_modal_view = 0;
                }
                ImGui::PopStyleColor(2);

                ImGui::SameLine(0, 6.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.12f, 0.18f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.16f, 0.22f, 1.0f));
                if (ImGui::Button(ICON_FA_XMARK " FECHAR", ImVec2(80.0f, 26.0f))) {
                    show_search_modal = false;
                    search_modal_view = 0;
                    KuroAudio::DJPreviewPlayer::getInstance().stop();
                }
                ImGui::PopStyleColor(2);

                ImGui::SetCursorScreenPos(ImVec2(head_pos.x, head_pos.y + 52.0f));

                // --- 2. HERO BANNER DA FAIXA SELECIONADA ---
                ImVec2 hero_pos = ImGui::GetCursorScreenPos();
                float hero_h = 125.0f;
                dl->AddRectFilled(hero_pos, ImVec2(hero_pos.x + avail_w, hero_pos.y + hero_h), IM_COL32(7, 12, 20, 240), 6.0f);
                dl->AddRect(hero_pos, ImVec2(hero_pos.x + avail_w, hero_pos.y + hero_h), IM_COL32(20, 40, 65, 200), 6.0f, 0, 1.0f);

                // Artwork à esquerda (105x105)
                float art_sz = 105.0f;
                ImVec2 art_p(hero_pos.x + 10.0f, hero_pos.y + 10.0f);
                if (selected_source_track_tex > 0) {
                    dl->AddImage((ImTextureID)(intptr_t)selected_source_track_tex, art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz));
                    dl->AddRect(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(0, 220, 220, 180), 3.0f, 0, 1.0f);
                } else {
                    dl->AddRectFilled(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(12, 18, 30, 255), 4.0f);
                    float cx = art_p.x + art_sz * 0.5f;
                    float cy = art_p.y + art_sz * 0.5f;
                    float rad = art_sz * 0.42f;
                    dl->AddCircleFilled(ImVec2(cx, cy), rad, IM_COL32(18, 28, 45, 255), 32);
                    dl->AddCircle(ImVec2(cx, cy), rad, IM_COL32(0, 200, 220, 220), 32, 1.5f);
                    dl->AddCircle(ImVec2(cx, cy), rad * 0.65f, IM_COL32(0, 140, 180, 120), 24, 1.0f);
                    dl->AddCircleFilled(ImVec2(cx, cy), rad * 0.22f, IM_COL32(0, 220, 240, 240), 16);
                    dl->AddText(ImVec2(cx - 6.0f, cy - 7.0f), IM_COL32(10, 20, 30, 255), ICON_FA_MUSIC);
                }

                // Metadados da Faixa
                float meta_x = art_p.x + art_sz + 16.0f;
                ImGui::SetCursorScreenPos(ImVec2(meta_x, hero_pos.y + 10.0f));
                
                // Título em destaque
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::Text("%s", selected_source_track_title.c_str());
                ImGui::PopStyleColor();

                // Artista + Botão Seguir
                ImGui::SetCursorScreenPos(ImVec2(meta_x, hero_pos.y + 32.0f));
                ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.98f, 1.0f), ICON_FA_USER " %s", selected_source_track_artist.c_str());
                ImGui::SameLine(0, 14.0f);

                if (is_following_selected_artist) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.38f, 0.22f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.08f, 0.50f, 0.28f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 1.00f, 0.60f, 1.0f));
                    if (ImGui::SmallButton(ICON_FA_CHECK " SEGUINDO ARTISTA")) {
                        is_following_selected_artist = false;
                        followed_artists.erase(std::remove(followed_artists.begin(), followed_artists.end(), selected_source_track_artist), followed_artists.end());
                    }
                    ImGui::PopStyleColor(3);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.18f, 0.28f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.65f, 0.85f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.92f, 1.00f, 1.0f));
                    if (ImGui::SmallButton(ICON_FA_PLUS " SEGUIR ARTISTA")) {
                        is_following_selected_artist = true;
                        followed_artists.push_back(selected_source_track_artist);
                    }
                    ImGui::PopStyleColor(3);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Radar de Artistas: Marque este produtor para priorizar seus lançamentos e remixes");

                // Linha de Pílulas de Metadados: Camelot Key, Compatibilidade Harmônica, BPM, Subgênero, Duração
                ImGui::SetCursorScreenPos(ImVec2(meta_x, hero_pos.y + 58.0f));
                
                // Tom Camelot
                KuroAudio::CamelotKey trk_ck = KuroAudio::DJHarmonicEngine::parseCamelotKey(selected_source_track_key);
                unsigned int wheel_c = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(trk_ck.number);
                ImVec4 wheel_v4 = ImColor(wheel_c).Value;
                
                ImGui::PushStyleColor(ImGuiCol_Button, wheel_v4);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.04f, 0.06f, 1.0f));
                char key_btn_str[32];
                snprintf(key_btn_str, sizeof(key_btn_str), "TOM %s (%s)", trk_ck.getCamelotCode().c_str(), trk_ck.musical_key.c_str());
                ImGui::SmallButton(key_btn_str);
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 6.0f);

                // Compatibilidade Harmônica
                KuroAudio::CamelotKey base_ck = KuroAudio::DJHarmonicEngine::parseCamelotKey(ref_k);
                std::string comp_txt = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(base_ck.getCamelotCode(), trk_ck.getCamelotCode());
                ImU32 comp_c = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeColor(base_ck.getCamelotCode(), trk_ck.getCamelotCode());
                ImVec4 comp_v4 = ImColor(comp_c).Value;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(comp_v4.x * 0.25f, comp_v4.y * 0.25f, comp_v4.z * 0.25f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, comp_v4);
                ImGui::SmallButton(comp_txt.c_str());
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Compatibilidade harmônica com o Deck Master (%s)", base_ck.getCamelotCode().c_str());
                ImGui::SameLine(0, 6.0f);

                // BPM
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.14f, 0.22f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
                char bpm_str[32]; snprintf(bpm_str, sizeof(bpm_str), "⚡ %.1f BPM", selected_source_track_bpm);
                ImGui::SmallButton(bpm_str);
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 6.0f);

                // Subgênero
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.08f, 0.24f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.60f, 1.00f, 1.0f));
                std::string gen_lbl = selected_source_track_subgenre.empty() ? "Psytrance" : selected_source_track_subgenre;
                ImGui::SmallButton((std::string(ICON_FA_TAGS) + " " + gen_lbl).c_str());
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 6.0f);

                // Duração
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.14f, 0.20f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.90f, 0.95f, 1.0f));
                int d_min = (int)selected_source_track_dur / 60;
                int d_sec = (int)selected_source_track_dur % 60;
                char dur_str[32]; snprintf(dur_str, sizeof(dur_str), "%02d:%02d", d_min, d_sec);
                ImGui::SmallButton(dur_str);
                ImGui::PopStyleColor(2);

                // Status de Abdução e Progresso
                ImGui::SetCursorScreenPos(ImVec2(meta_x, hero_pos.y + 86.0f));
                bool is_this_dl = spotify_service.isDownloading() && 
                                 (spotify_service.getDownloadingTrackId() == selected_source_track_id || spotify_service.getDownloadingTrackId().empty());
                int dl_pct = (int)spotify_service.getDownloadProgressPct();
                if (is_this_dl) {
                    float pulse = (std::sin((float)ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
                    ImGui::TextColored(ImVec4(1.0f, 0.60f + 0.40f * pulse, 0.10f, 1.0f), 
                                      ICON_FA_BOLT " ABDUZINDO FAIXA DE ESTÚDIO (%d%%)", dl_pct > 0 ? dl_pct : 1);
                    ImGui::SameLine(0, 12.0f);
                    float prog_val = std::clamp((float)dl_pct / 100.0f, 0.02f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.00f, 0.92f, 0.85f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.10f, 0.16f, 1.0f));
                    ImGui::ProgressBar(prog_val, ImVec2(240.0f, 16.0f), "");
                    ImGui::PopStyleColor(2);
                } else {
                    auto* trk_ptr = KuroAudio::DJLibraryManager::getInstance().getTrackById(selected_source_track_id);
                    if (trk_ptr && trk_ptr->is_downloaded) {
                        ImGui::TextColored(ImVec4(0.20f, 1.00f, 0.60f, 1.0f), ICON_FA_CIRCLE_CHECK " FAIXA JÁ ABDUZIDA NA BIBLIOTECA (ÁUDIO MASTER NO DISCO)");
                    } else {
                        ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.98f, 1.0f), ICON_FA_SHUTTLE_SPACE " PRONTO PARA ABDUÇÃO: Selecione a matriz mais pura abaixo");
                    }
                }

                ImGui::SetCursorScreenPos(ImVec2(hero_pos.x, hero_pos.y + hero_h + 10.0f));

                // --- 3. BARRA DE ABDUÇÃO VIA LINK DIRETO ---
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.20f, 1.0f), ICON_FA_LINK " ABDUÇÃO DE LINK ESPECÍFICO:");
                ImGui::SameLine();
                ImGui::PushItemWidth(avail_w - 380.0f);
                ImGui::InputTextWithHint("##DirectSourceUrlModal", "Cole link do Beatport, SoundCloud, Tidal, YouTube ou áudio direto (.flac / .wav)...", custom_direct_url_buf, sizeof(custom_direct_url_buf));
                ImGui::PopItemWidth();
                ImGui::SameLine(0, 6.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.55f, 0.35f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.45f, 1.0f));
                if (ImGui::Button(ICON_FA_SHUTTLE_SPACE " ABDUZIR LINK DIRETO", ImVec2(200.0f, 0.0f))) {
                    if (strlen(custom_direct_url_buf) > 5) {
                        spotify_service.downloadSourceDirectAsync(selected_source_track_id, selected_source_track_title, selected_source_track_artist, custom_direct_url_buf, -1);
                    }
                }
                ImGui::PopStyleColor(2);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // --- 4. TABS DE FILTRO DE MATRIZ ---
                const char* stab_names[] = { 
                    ICON_FA_MUSIC " TODAS AS FONTES", 
                    ICON_FA_STAR " ★★★★★ STUDIO MASTER", 
                    ICON_FA_MUSIC " ★★★★☆ TIDAL MASTER", 
                    ICON_FA_CLOUD " SOUNDCLOUD HQ", 
                    ICON_FA_STAR " YOUTUBE TOPIC" 
                };
                for (int t = 0; t < 5; t++) {
                    bool sel = (sources_tab_filter == t);
                    if (sel) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.85f, 0.80f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.85f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 1.0f));
                    }
                    if (ImGui::SmallButton(stab_names[t])) {
                        sources_tab_filter = t;
                    }
                    ImGui::PopStyleColor(2);
                    if (t < 4) ImGui::SameLine(0, 4.0f);
                }

                ImGui::Spacing();

                if (spotify_service.isSearchingSources()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.35f, 1.0f), ICON_FA_SPINNER " Inspecionando pureza espectral e buscando matrizes no Beatport, Tidal Master, SoundCloud e YouTube Topic...");
                }

                auto sources = spotify_service.getCurrentSources();
                if (sources.empty() && !spotify_service.isSearchingSources()) {
                    ImGui::TextColored(ImVec4(0.85f, 0.50f, 0.40f, 1.0f), "Nenhuma matriz automática encontrada. Cole um link direto no campo acima para abduzir.");
                } else if (!sources.empty()) {
                    if (ImGui::BeginTable("##AbductionMasterSourcesTable", 5, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH, ImVec2(0, ImGui::GetContentRegionAvail().y - 32.0f))) {
                        ImGui::TableSetupColumn("Pureza Espectral", ImGuiTableColumnFlags_WidthFixed, 185.0f);
                        ImGui::TableSetupColumn("Provedor / Plataforma", ImGuiTableColumnFlags_WidthFixed, 155.0f);
                        ImGui::TableSetupColumn("Título da Versão", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Duração", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                        ImGui::TableSetupColumn("Ações de Abdução", ImGuiTableColumnFlags_WidthFixed, 360.0f);
                        ImGui::TableHeadersRow();

                        for (size_t s_idx = 0; s_idx < sources.size(); s_idx++) {
                            const auto& src = sources[s_idx];
                            if (sources_tab_filter == 1 && src.badge.find("STUDIO MASTER") == std::string::npos && src.badge.find("24-bit") == std::string::npos) continue;
                            if (sources_tab_filter == 2 && src.badge.find("TIDAL") == std::string::npos && src.channel.find("Tidal") == std::string::npos) continue;
                            if (sources_tab_filter == 3 && src.badge.find("SOUNDCLOUD") == std::string::npos && src.channel.find("SoundCloud") == std::string::npos) continue;
                            if (sources_tab_filter == 4 && src.badge.find("TOPIC") == std::string::npos && !src.is_official) continue;

                            ImGui::TableNextRow();

                            // 1. Pureza Espectral & Badge
                            ImGui::TableSetColumnIndex(0);
                            if (src.badge.find("STUDIO MASTER") != std::string::npos || src.badge.find("24-bit") != std::string::npos) {
                                ImGui::TextColored(ImVec4(1.00f, 0.85f, 0.20f, 1.0f), "★★★★★ STUDIO MASTER");
                            } else if (src.badge.find("TIDAL") != std::string::npos || src.badge.find("LOSSLESS STREAM") != std::string::npos) {
                                ImGui::TextColored(ImVec4(0.00f, 0.90f, 1.00f, 1.0f), "★★★★☆ LOSSLESS STREAM");
                            } else if (src.badge.find("SOUNDCLOUD") != std::string::npos || src.badge.find("LOSSLESS HQ") != std::string::npos) {
                                ImGui::TextColored(ImVec4(1.00f, 0.55f, 0.15f, 1.0f), "★★★★☆ LOSSLESS HQ");
                            } else if (src.badge.find("TOPIC") != std::string::npos) {
                                ImGui::TextColored(ImVec4(0.00f, 1.00f, 0.55f, 1.0f), "★★★☆☆ TOPIC AUDIO");
                            } else {
                                ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.85f, 1.0f), "★★★☆☆ STANDARD HQ");
                            }

                            // 2. Provedor / Plataforma
                            ImGui::TableSetColumnIndex(1);
                            if (src.channel.find("SoundCloud") != std::string::npos || src.badge.find("SOUNDCLOUD") != std::string::npos) {
                                ImGui::TextColored(ImVec4(1.00f, 0.55f, 0.15f, 1.0f), ICON_FA_CLOUD " %s", src.channel.c_str());
                            } else if (src.channel.find("Tidal") != std::string::npos || src.badge.find("TIDAL") != std::string::npos) {
                                ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), ICON_FA_MUSIC " %s", src.channel.c_str());
                            } else if (src.channel.find("Beatport") != std::string::npos || src.badge.find("STUDIO MASTER") != std::string::npos) {
                                ImGui::TextColored(ImVec4(1.00f, 0.85f, 0.20f, 1.0f), ICON_FA_COMPACT_DISC " %s", src.channel.c_str());
                            } else if (src.channel.find("Spotify") != std::string::npos || src.badge.find("SPOTIFY") != std::string::npos) {
                                ImGui::TextColored(ImVec4(0.12f, 0.95f, 0.40f, 1.0f), ICON_FA_COMPACT_DISC " %s", src.channel.c_str());
                            } else {
                                ImGui::TextColored(ImVec4(0.40f, 0.85f, 1.00f, 1.0f), ICON_FA_STAR " %s", src.channel.c_str());
                            }

                            // 3. Título
                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", src.title.c_str());

                            // 4. Duração
                            ImGui::TableSetColumnIndex(3);
                            ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), "[%s]", src.duration_formatted.c_str());

                            // 5. Ações de Abdução
                            ImGui::TableSetColumnIndex(4);
                            ImGui::PushID((int)s_idx + 9100);

                            // Preview
                            bool is_previewing = KuroAudio::DJPreviewPlayer::getInstance().isPlaying() && 
                                                 KuroAudio::DJPreviewPlayer::getInstance().getCurrentTitle() == selected_source_track_title;
                            if (is_previewing) {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.20f, 0.90f));
                                if (ImGui::SmallButton(ICON_FA_STOP " PARAR")) {
                                    KuroAudio::DJPreviewPlayer::getInstance().stop();
                                }
                                ImGui::PopStyleColor();
                            } else {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.50f, 0.65f, 0.90f));
                                if (ImGui::SmallButton(ICON_FA_PLAY " PREVIEW")) {
                                    spotify_service.previewSourceAsync(src.url, selected_source_track_title, src.channel);
                                }
                                ImGui::PopStyleColor();
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ouvir amostra prévia em tempo real");

                            ImGui::SameLine(0, 4.0f);

                            // Botão Abduzir esta Fonte
                            int cur_pct = (int)spotify_service.getDownloadProgressPct();
                            bool is_down = spotify_service.isDownloading();
                            if (is_down) {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.18f, 0.08f, 0.90f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.65f, 0.20f, 1.0f));
                                char dl_lbl[48];
                                snprintf(dl_lbl, sizeof(dl_lbl), "%s %d%%", ICON_FA_BOLT, cur_pct > 0 ? cur_pct : 1);
                                ImGui::SmallButton(dl_lbl);
                                ImGui::PopStyleColor(2);
                            } else {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.52f, 0.32f, 0.95f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.68f, 0.42f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 1.00f, 0.95f, 1.0f));
                                if (ImGui::SmallButton(ICON_FA_SHUTTLE_SPACE " ABDUZIR ESTA FONTE")) {
                                    spotify_service.downloadSourceDirectAsync(selected_source_track_id, selected_source_track_title, selected_source_track_artist, src.url, -1);
                                }
                                ImGui::PopStyleColor(3);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abduzir áudio diretamente desta fonte com DSP 24-bit");

                            ImGui::SameLine(0, 4.0f);

                            // D1
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.50f, 0.85f));
                            char b1[24]; snprintf(b1, sizeof(b1), "D%d", left_id + 1);
                            if (ImGui::SmallButton(b1)) {
                                spotify_service.downloadSourceDirectAsync(selected_source_track_id, selected_source_track_title, selected_source_track_artist, src.url, left_id);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abduzir e carregar no Deck %d (Esquerdo)", left_id + 1);
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 4.0f);

                            // D2
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.00f, 0.35f, 0.85f));
                            char b2[24]; snprintf(b2, sizeof(b2), "D%d", right_id + 1);
                            if (ImGui::SmallButton(b2)) {
                                spotify_service.downloadSourceDirectAsync(selected_source_track_id, selected_source_track_title, selected_source_track_artist, src.url, right_id);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abduzir e carregar no Deck %d (Direito)", right_id + 1);
                            ImGui::PopStyleColor();

                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
                }

                // Rodapé do Card
                ImGui::SetCursorScreenPos(ImVec2(head_pos.x, head_pos.y + card_h - 28.0f));
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.00f, 0.88f, 0.96f, 1.0f), ICON_FA_COMPACT_DISC " DESTINO:");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.70f, 0.85f, 0.95f, 1.0f), "CRATES // %s • Flac / WAV 24-bit 48kHz • Metadados ID3 & Camelot Gravados", selected_source_track_artist.c_str());

                ImGui::End();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                return;
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }

        void renderSearchModal(KuroAudio::DJEngine& engine) {
            if (!show_search_modal) return;

            // =========================================================================
            // CARD IMERSIVO CENTRAL: ABDUCTION STUDIO // MASTER SOURCE MATRIX
            // =========================================================================
            if (search_modal_view == 1) {
                renderMasterSourceMatrixCard(engine);
                return;
            }'''

    if search_modal_entry in content:
        content = content.replace(search_modal_entry, search_modal_hook, 1)
        print('[+] Injected renderMasterSourceMatrixCard and hooked into renderSearchModal')
    else:
        print('[!] search_modal_entry not found')

    # 4. In search results table: update FONTES and BAIXAR buttons to ABDUZIR calling openSourceMatrix(r)
    tbl_btn_target = '''                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.60f, 0.90f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.80f, 0.95f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.95f, 1.00f, 1.0f));
                                if (ImGui::SmallButton(ICON_FA_SATELLITE_DISH " FONTES")) {
                                    search_modal_view = 1;
                                    selected_source_track_id = r.id;
                                    selected_source_track_title = r.title;
                                    selected_source_track_artist = r.artist;
                                    spotify_service.searchSourcesAsync(r.id, r.title, r.artist);
                                }
                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("SELETOR DE FONTES & VERSÕES:\\nExplorar e escolher masters oficiais no SoundCloud, Tidal Hi-Fi, YouTube Music e Beatport");
                                ImGui::PopStyleColor(3);

                                ImGui::SameLine(0, 4);

                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.50f, 0.30f, 0.90f));
                                if (ImGui::SmallButton(ICON_FA_DOWNLOAD " BAIXAR")) {
                                    spotify_service.importSearchResultTrack(r);
                                    auto* ptr = KuroAudio::DJLibraryManager::getInstance().getTrackById(r.id);
                                    if (ptr) {
                                        spotify_service.downloadLibraryTrackAsync(*ptr, -1, engine);
                                    }
                                }
                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Baixar áudio e arte para a biblioteca");
                                ImGui::PopStyleColor();'''

    tbl_btn_repl = '''                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.50f, 0.65f, 0.90f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.80f, 0.95f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.95f, 1.00f, 1.0f));
                                bool is_r_downloading = spotify_service.isDownloading() && 
                                                        (spotify_service.getDownloadingTrackId() == r.id || spotify_service.getDownloadingTrackId().empty());
                                int r_dl_pct = (int)spotify_service.getDownloadProgressPct();
                                std::string abduzir_lbl = is_r_downloading
                                    ? (r_dl_pct > 0 ? (std::string(ICON_FA_BOLT) + " " + std::to_string(r_dl_pct) + "%") : (std::string(ICON_FA_BOLT) + " ..."))
                                    : (std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUZIR");
                                if (ImGui::SmallButton(abduzir_lbl.c_str())) {
                                    openSourceMatrix(r);
                                }
                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("ABDUCTION STUDIO // MASTER SOURCE MATRIX:\\nInspecionar pureza espectral e abduzir matriz oficial");
                                ImGui::PopStyleColor(3);'''

    if tbl_btn_target in content:
        content = content.replace(tbl_btn_target, tbl_btn_repl, 1)
        print('[+] Replaced FONTES/BAIXAR in search table with [ ABDUZIR ] calling openSourceMatrix')
    else:
        print('[!] tbl_btn_target not found')

    temp_file = 'src\\ui\\KuroDJStudioUI.h.tmp'
    real_target = 'src\\ui\\KuroDJStudioUI.h'
    with open(temp_file, 'w', encoding='utf-8') as f:
        f.write(content)
    import subprocess
    res = subprocess.run(['cmd', '/c', 'move', '/y', temp_file, real_target], capture_output=True, text=True)
    if res.returncode == 0:
        print('[+] Successfully moved and updated src/ui/KuroDJStudioUI.h!')
    else:
        print('[!] Move failed:', res.stderr)

if __name__ == '__main__':
    apply_patch()
