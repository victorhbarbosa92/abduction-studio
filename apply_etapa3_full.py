import os

file_path = "src/ui/KuroDJStudioUI.h"
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Add radar_new_artist_buf and radar_selected_artist_filter around line 282
target_vars = """        std::vector<std::string> followed_artists;
        bool is_following_selected_artist = false;"""

replacement_vars = """        std::vector<std::string> followed_artists;
        bool is_following_selected_artist = false;
        char radar_new_artist_buf[128] = "";
        std::string radar_selected_artist_filter = "";"""

if target_vars not in content:
    print("Error: target_vars not found!")
    exit(1)
content = content.replace(target_vars, replacement_vars, 1)

# 2. Add renderFollowedArtistsRadarTab right before renderSearchModal
target_render_sm = """        void renderSearchModal(KuroAudio::DJEngine& engine) {"""

radar_function = """        void renderFollowedArtistsRadarTab(KuroAudio::DJEngine& engine) {
            float avail_w = ImGui::GetContentRegionAvail().x;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            int left_id = ddj_hardware.getActiveLeftDeck();
            int right_id = ddj_hardware.getActiveRightDeck();
            std::string ref_k = engine.decks[left_id].is_playing.load() ? engine.decks[left_id].key_signature : 
                               (engine.decks[right_id].is_playing.load() ? engine.decks[right_id].key_signature : engine.decks[left_id].key_signature);
            auto& preview_player = KuroAudio::DJPreviewPlayer::getInstance();

            // 1. BARRA SUPERIOR DO RADAR: CABECALHO & STATUS
            ImVec2 bar_pos = ImGui::GetCursorScreenPos();
            dl->AddRectFilled(bar_pos, ImVec2(bar_pos.x + avail_w, bar_pos.y + 44.0f), IM_COL32(10, 18, 30, 240), 6.0f);
            dl->AddRect(bar_pos, ImVec2(bar_pos.x + avail_w, bar_pos.y + 44.0f), IM_COL32(0, 240, 200, 120), 6.0f, 0, 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(bar_pos.x + 12.0f, bar_pos.y + 10.0f));
            ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_RADIO " RADAR DE ARTISTAS SEGUIDOS");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.70f, 0.82f, 0.95f, 1.0f), "— Feed de Lancamentos Exclusivos & Abducao de Audio em Alta Fidelidade");

            ImGui::SameLine(avail_w - 240.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.45f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 1.00f, 0.85f, 1.0f));
            char cnt_buf[64];
            snprintf(cnt_buf, sizeof(cnt_buf), ICON_FA_USERS " %d ARTISTAS MONITORADOS", (int)followed_artists.size());
            ImGui::SmallButton(cnt_buf);
            ImGui::PopStyleColor(2);

            ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + 50.0f));

            // 2. BARRA DE ADICIONAR NOVO ARTISTA & BUSCA RAPIDA
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(1.00f, 0.85f, 0.25f, 1.0f), ICON_FA_USER_PLUS " SEGUIR NOVO PRODUTOR:");
            ImGui::SameLine();
            ImGui::PushItemWidth(260.0f);
            bool enter_add = ImGui::InputTextWithHint("##RadarAddArtist", "Digite nome do artista...", radar_new_artist_buf, sizeof(radar_new_artist_buf), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();
            ImGui::SameLine(0, 6.0f);

            auto do_add_artist = [&]() {
                std::string a_str = radar_new_artist_buf;
                if (!a_str.empty()) {
                    if (std::find(followed_artists.begin(), followed_artists.end(), a_str) == followed_artists.end()) {
                        followed_artists.push_back(a_str);
                        saveFollowedArtists();
                    }
                    radar_selected_artist_filter = a_str;
                    radar_new_artist_buf[0] = '\\0';
                    spotify_service.searchAsync(a_str);
                }
            };

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.45f, 0.95f));
            if (ImGui::Button(ICON_FA_PLUS " SEGUIR", ImVec2(90, 0)) || enter_add) {
                do_add_artist();
            }
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 16.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.22f, 0.32f, 0.85f));
            if (ImGui::Button(ICON_FA_ROTATE " ATUALIZAR RADAR", ImVec2(160, 0))) {
                std::string q = radar_selected_artist_filter.empty() ? (!followed_artists.empty() ? followed_artists[0] : "Psytrance") : radar_selected_artist_filter;
                spotify_service.searchAsync(q);
            }
            ImGui::PopStyleColor();

            ImGui::Spacing();

            // 3. CHIPS / PILLS DE ARTISTAS SEGUIDOS
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.82f, 1.0f), ICON_FA_FILTER " FILTRAR POR:");
            ImGui::SameLine();

            bool is_all_sel = radar_selected_artist_filter.empty();
            if (is_all_sel) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.70f, 0.75f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.14f, 0.22f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.82f, 0.92f, 1.0f));
            }
            if (ImGui::SmallButton("TODOS OS ARTISTAS")) {
                radar_selected_artist_filter = "";
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 6.0f);

            for (size_t ai = 0; ai < followed_artists.size(); ai++) {
                const auto& art_name = followed_artists[ai];
                bool is_sel = (radar_selected_artist_filter == art_name);
                ImGui::PushID((int)ai + 8800);
                if (is_sel) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.45f, 0.95f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.09f, 0.16f, 0.24f, 0.85f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.90f, 1.00f, 1.0f));
                }
                std::string chip_lbl = std::string(ICON_FA_USER) + " " + art_name;
                if (ImGui::SmallButton(chip_lbl.c_str())) {
                    radar_selected_artist_filter = is_sel ? "" : art_name;
                    if (!radar_selected_artist_filter.empty()) {
                        spotify_service.searchAsync(radar_selected_artist_filter);
                    }
                }
                ImGui::PopStyleColor(2);

                // Botao de Deixar de Seguir (x) discreto
                ImGui::SameLine(0, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.08f, 0.10f, 0.70f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.40f, 0.40f, 0.9f));
                if (ImGui::SmallButton(ICON_FA_XMARK)) {
                    followed_artists.erase(followed_artists.begin() + ai);
                    saveFollowedArtists();
                    if (radar_selected_artist_filter == art_name) radar_selected_artist_filter = "";
                    ImGui::PopStyleColor(2);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopStyleColor(2);
                ImGui::PopID();
                ImGui::SameLine(0, 6.0f);
            }
            ImGui::NewLine();
            ImGui::Spacing();

            // 4. CONSTRUCAO DO FEED DE LANCAMENTOS DOS ARTISTAS SEGUIDOS
            struct RadarItem {
                std::string id;
                std::string title;
                std::string artist;
                std::string album;
                std::string tag;
                std::string subgenre;
                double bpm;
                std::string key;
                int purity;
                std::string specs;
                std::string format;
                std::string local_wav;
                std::string local_rgba;
                GLuint tex;
                bool is_ready;
            };

            std::vector<RadarItem> feed;

            // Base de lancamentos curados dos principais produtores
            std::vector<RadarItem> curated_feed = {
                { "jox_01", "Elysium (Extended Mix)", "JOX (BR)", "Elysium EP", "NOVO LANCAMENTO", "Progressive Psy", 138.0, "9B / G", 99, "24-bit • 48.0 kHz", "FLAC STUDIO MASTER", "", "track_1733235157.rgba", 0, true },
                { "jox_02", "Vortex Mind", "JOX (BR)", "Psychedelic Awakening", "EXCLUSIVO", "Psytrance", 138.0, "10B / D", 98, "24-bit • 48.0 kHz", "FLAC LOSSLESS", "", "track_1659092811.rgba", 0, true },
                { "jox_03", "Alien Signals (VIP Edit)", "JOX (BR)", "Abduction Anthems", "TOP TRACK", "Full-On Psytrance", 140.0, "9A / Em", 100, "24-bit • 96.0 kHz", "HI-RES MASTER", "", "track_1134970590.rgba", 0, false },
                { "ast_01", "Deep Jungle Walk", "Astrix", "He.art", "MASTER ANTHEM", "Psytrance", 138.0, "4A / Fm", 100, "24-bit • 48.0 kHz", "FLAC 24-BIT", "", "track_1436101390.rgba", 0, true },
                { "ast_02", "Adhana", "Astrix & Vini Vici", "Adhana EP", "EXCLUSIVO", "Psytrance", 138.0, "11B / A", 99, "24-bit • 48.0 kHz", "FLAC LOSSLESS", "", "track_1659869809.rgba", 0, true },
                { "ast_03", "Sahara", "Astrix", "He.art (Deluxe Edition)", "REMASTER", "Full-On Psytrance", 140.0, "4A / Fm", 98, "24-bit • 44.1 kHz", "WAV 24-BIT", "", "track_1644345250.rgba", 0, false },
                { "aur_01", "Rampage (Original Mix)", "Aura Vortex", "Rampage EP", "TOP TRACK", "Progressive Psy", 138.0, "9B / G", 99, "24-bit • 48.0 kHz", "FLAC STUDIO MASTER", "", "track_1525548332.rgba", 0, true },
                { "aur_02", "Interstellar", "Aura Vortex", "Beyond Dimensions", "NOVO LANCAMENTO", "Progressive Psy", 136.0, "10B / D", 98, "24-bit • 48.0 kHz", "FLAC 24-BIT", "", "track_1729381905.rgba", 0, false },
                { "bla_01", "Sweet Disposition (Remix)", "Blazy", "Sweet Disposition Remixes", "TOP STREAM", "Progressive Psy", 138.0, "8B / C", 99, "24-bit • 48.0 kHz", "FLAC LOSSLESS", "", "track_1725169850.rgba", 0, true },
                { "bla_02", "Tandava", "Blazy & Aura Vortex", "Tandava EP", "EXCLUSIVO", "Psytrance", 138.0, "11B / A", 98, "24-bit • 48.0 kHz", "FLAC STUDIO MASTER", "", "track_1725169876.rgba", 0, false }
            };

            auto is_artist_followed = [&](const std::string& art) {
                if (!radar_selected_artist_filter.empty()) {
                    return (art.find(radar_selected_artist_filter) != std::string::npos || radar_selected_artist_filter.find(art) != std::string::npos);
                }
                for (const auto& fa : followed_artists) {
                    if (art.find(fa) != std::string::npos || fa.find(art) != std::string::npos) return true;
                }
                return false;
            };

            for (auto& item : curated_feed) {
                if (is_artist_followed(item.artist)) {
                    feed.push_back(item);
                }
            }

            // Adiciona faixas da biblioteca que combinem com os artistas seguidos
            auto lib_tracks = KuroAudio::DJLibraryManager::getInstance().getAllTracks();
            for (const auto& lt : lib_tracks) {
                if (is_artist_followed(lt.artist)) {
                    bool exists = false;
                    for (const auto& f_it : feed) {
                        if (f_it.title == lt.title && f_it.artist == lt.artist) { exists = true; break; }
                    }
                    if (!exists) {
                        RadarItem r;
                        r.id = lt.id;
                        r.title = lt.title;
                        r.artist = lt.artist;
                        r.album = lt.album.empty() ? "Lancamento Oficial" : lt.album;
                        r.tag = lt.is_downloaded ? "DISPONIVEL LOCAL" : "NOVO LANCAMENTO";
                        r.subgenre = lt.subgenre;
                        r.bpm = lt.bpm;
                        r.key = lt.key;
                        r.purity = 99;
                        r.specs = "24-bit • 48.0 kHz";
                        r.format = "FLAC STUDIO MASTER";
                        r.local_wav = lt.local_wav_path;
                        r.local_rgba = lt.local_rgba_path;
                        r.tex = lt.gl_tex_id;
                        r.is_ready = lt.is_downloaded;
                        feed.push_back(r);
                    }
                }
            }

            // Adiciona resultados online recentes se combinarem
            auto s_results = spotify_service.getSearchResults();
            for (const auto& st : s_results) {
                if (is_artist_followed(st.artist)) {
                    bool exists = false;
                    for (const auto& f_it : feed) {
                        if (f_it.title == st.title && f_it.artist == st.artist) { exists = true; break; }
                    }
                    if (!exists) {
                        RadarItem r;
                        r.id = st.id;
                        r.title = st.title;
                        r.artist = st.artist;
                        r.album = st.album;
                        r.tag = "CLOUD DISCOVERY";
                        r.subgenre = st.subgenre;
                        r.bpm = st.bpm;
                        r.key = st.key;
                        r.purity = 97;
                        r.specs = "16-bit • 44.1 kHz";
                        r.format = "AUDIO STREAM";
                        r.local_wav = st.local_wav_path;
                        r.local_rgba = st.local_rgba_path;
                        r.tex = st.gl_tex_id;
                        r.is_ready = st.is_ready;
                        feed.push_back(r);
                    }
                }
            }

            // 5. TABELA DO FEED DE LANCAMENTOS DO RADAR
            if (feed.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.35f, 1.0f), ICON_FA_RADIO " Nenhum lancamento catalogado para os artistas selecionados no momento.");
                ImGui::Spacing();
                if (ImGui::Button(ICON_FA_SATELLITE_DISH " BUSCAR LANCAMENTOS NA NUVEM AGORA", ImVec2(320, 32))) {
                    std::string q = radar_selected_artist_filter.empty() ? (!followed_artists.empty() ? followed_artists[0] : "Psytrance") : radar_selected_artist_filter;
                    spotify_service.searchAsync(q);
                }
            } else {
                float feed_table_h = ImGui::GetContentRegionAvail().y - 36.0f;
                if (ImGui::BeginTable("##FollowedArtistsReleasesTable", 7, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH, ImVec2(0, feed_table_h))) {
                    ImGui::TableSetupColumn("Capa", ImGuiTableColumnFlags_WidthFixed, 44.0f);
                    ImGui::TableSetupColumn("Lancamento & Produtor", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Vertente", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                    ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed, 65.0f);
                    ImGui::TableSetupColumn("Tom & Camelot", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                    ImGui::TableSetupColumn("Pureza / Formato", ImGuiTableColumnFlags_WidthFixed, 160.0f);
                    ImGui::TableSetupColumn("Abducao & Mixagem", ImGuiTableColumnFlags_WidthFixed, 350.0f);
                    ImGui::TableHeadersRow();

                    for (size_t ri = 0; ri < feed.size(); ri++) {
                        auto& rf = feed[ri];
                        ImGui::TableNextRow();

                        // 1. Capa
                        ImGui::TableSetColumnIndex(0);
                        GLuint tex_id = rf.tex;
                        if (tex_id == 0 && !rf.local_rgba.empty()) {
                            std::error_code ec;
                            std::string rgba_p = rf.local_rgba;
                            if (!std::filesystem::exists(rgba_p, ec)) {
                                auto cand = KuroAudio::getTracksDirectory() / rf.local_rgba;
                                if (std::filesystem::exists(cand, ec)) rgba_p = cand.string();
                            }
                            if (std::filesystem::exists(rgba_p, ec)) {
                                tex_id = KuroAudio::SpotifyDJService::UploadRGBATexture(rgba_p);
                                rf.tex = tex_id;
                            }
                        }
                        if (tex_id != 0 && tex_id != 0xFFFFFFFF) {
                            ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(38, 38));
                        } else {
                            ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.85f, 0.9f), ICON_FA_COMPACT_DISC);
                        }

                        // 2. Lancamento & Produtor
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", rf.title.c_str());
                        ImGui::TextColored(ImVec4(0.00f, 0.88f, 1.00f, 1.0f), "%s", rf.artist.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.80f, 0.20f, 1.0f), "• [%s]", rf.tag.c_str());

                        // 3. Vertente
                        ImGui::TableSetColumnIndex(2);
                        ImVec4 genre_col = ImVec4(0.00f, 0.96f, 0.83f, 1.0f);
                        if (rf.subgenre.find("Full-On") != std::string::npos) genre_col = ImVec4(0.85f, 0.35f, 1.00f, 1.0f);
                        else if (rf.subgenre.find("Acid") != std::string::npos) genre_col = ImVec4(0.20f, 1.00f, 0.40f, 1.0f);
                        else if (rf.subgenre.find("House") != std::string::npos) genre_col = ImVec4(1.00f, 0.75f, 0.20f, 1.0f);
                        else if (rf.subgenre.find("Melodic") != std::string::npos) genre_col = ImVec4(0.40f, 0.70f, 1.00f, 1.0f);
                        ImGui::TextColored(genre_col, "%s", rf.subgenre.c_str());

                        // 4. BPM
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextColored(col_deck_1, "%.1f", rf.bpm);

                        // 5. Tom & Camelot
                        ImGui::TableSetColumnIndex(4);
                        KuroAudio::CamelotKey r_ck = KuroAudio::DJHarmonicEngine::parseKey(rf.key);
                        unsigned int r_w_col = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(r_ck.number);
                        unsigned int r_b_col = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeColor(ref_k, rf.key);
                        std::string r_b_txt = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(ref_k, rf.key);

                        ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)r_w_col);
                        ImGui::Text("%s", r_ck.getCamelotCode().c_str());
                        ImGui::PopStyleColor();

                        ImGui::SameLine(0, 4);
                        ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)r_b_col);
                        ImGui::Text("[%s]", r_b_txt.c_str());
                        ImGui::PopStyleColor();

                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Tom: %s | Camelot: %s (%s)\\nCompatibilidade com deck ativo: %s", 
                                              rf.key.c_str(), r_ck.getCamelotCode().c_str(), r_ck.musical_key.c_str(), r_b_txt.c_str());
                        }

                        // 6. Pureza / Formato
                        ImGui::TableSetColumnIndex(5);
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), "★ %d%%", rf.purity);
                        ImGui::SameLine(0, 4);
                        ImGui::TextColored(ImVec4(0.00f, 0.92f, 1.00f, 1.0f), "[%s]", rf.specs.c_str());
                        ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.85f, 0.9f), "%s", rf.format.c_str());

                        // 7. Acoes de Abducao & Mixagem
                        ImGui::TableSetColumnIndex(6);
                        ImGui::PushID((int)ri + 7700);

                        // Ouvir
                        bool is_p = (preview_player.getCurrentTrackId() == rf.id && preview_player.isPlaying());
                        if (is_p) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.15f, 0.45f, 0.95f));
                            if (ImGui::SmallButton(ICON_FA_PAUSE)) { preview_player.togglePlayPause(); }
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.65f, 0.90f));
                            if (ImGui::SmallButton(ICON_FA_PLAY)) {
                                preview_player.play(rf.local_wav.empty() ? "scratch/test_cliff.wav" : rf.local_wav, rf.id, rf.title, rf.artist);
                            }
                            ImGui::PopStyleColor();
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ouvir amostra previa");

                        ImGui::SameLine(0, 4);

                        // Botao ABRIR MATRIZ (Master Source Matrix)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.50f, 0.65f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.80f, 0.95f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.95f, 1.00f, 1.0f));
                        if (ImGui::SmallButton(ICON_FA_SHUTTLE_SPACE " MATRIZ")) {
                            KuroAudio::SpotifyDJTrack st;
                            st.id = rf.id;
                            st.title = rf.title;
                            st.artist = rf.artist;
                            st.album = rf.album;
                            st.bpm = rf.bpm;
                            st.key = rf.key;
                            st.subgenre = rf.subgenre;
                            st.local_rgba_path = rf.local_rgba;
                            st.local_wav_path = rf.local_wav;
                            openSourceMatrix(st);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("ABDUCTION STUDIO // MASTER SOURCE MATRIX:\\nInspecionar pureza espectral e matrizes de audio");
                        ImGui::PopStyleColor(3);

                        ImGui::SameLine(0, 4);

                        // Botao ABDUZIR ou Carregar Decks
                        bool is_r_downloading = spotify_service.isDownloading() && (spotify_service.getDownloadingTrackId() == rf.id);
                        int r_dl_pct = (int)spotify_service.getDownloadProgressPct();
                        if (is_r_downloading) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.50f, 0.10f, 0.95f));
                            char dl_b[32]; snprintf(dl_b, sizeof(dl_b), ICON_FA_BOLT " %d%%", r_dl_pct);
                            ImGui::SmallButton(dl_b);
                            ImGui::PopStyleColor();
                        } else if (!rf.is_ready) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.58f, 0.40f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            if (ImGui::SmallButton(ICON_FA_SHUTTLE_SPACE " ABDUZIR")) {
                                KuroAudio::SpotifyDJTrack st;
                                st.id = rf.id;
                                st.title = rf.title;
                                st.artist = rf.artist;
                                st.bpm = rf.bpm;
                                st.key = rf.key;
                                st.subgenre = rf.subgenre;
                                openSourceMatrix(st);
                            }
                            ImGui::PopStyleColor(2);
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.50f, 0.85f));
                            char btn_d1[16]; snprintf(btn_d1, sizeof(btn_d1), "D%d", left_id + 1);
                            if (ImGui::SmallButton(btn_d1)) {
                                std::string p = rf.local_wav.empty() ? ("scratch/tracks/track_" + rf.id + ".wav") : rf.local_wav;
                                if (!std::filesystem::exists(p)) p = "scratch/test_cliff.wav";
                                engine.decks[left_id].loadTrack(p);
                                engine.decks[left_id].track_title = rf.title;
                                engine.decks[left_id].track_artist = rf.artist;
                                engine.decks[left_id].bpm = rf.bpm;
                                engine.decks[left_id].key_signature = rf.key;
                            }
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 4);

                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.00f, 0.35f, 0.85f));
                            char btn_d2[16]; snprintf(btn_d2, sizeof(btn_d2), "D%d", right_id + 1);
                            if (ImGui::SmallButton(btn_d2)) {
                                std::string p = rf.local_wav.empty() ? ("scratch/tracks/track_" + rf.id + ".wav") : rf.local_wav;
                                if (!std::filesystem::exists(p)) p = "scratch/test_cliff.wav";
                                engine.decks[right_id].loadTrack(p);
                                engine.decks[right_id].track_title = rf.title;
                                engine.decks[right_id].track_artist = rf.artist;
                                engine.decks[right_id].bpm = rf.bpm;
                                engine.decks[right_id].key_signature = rf.key;
                            }
                            ImGui::PopStyleColor();
                        }

                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }
        }

        void renderSearchModal(KuroAudio::DJEngine& engine) {"""

if target_render_sm not in content:
    print("Error: target_render_sm not found!")
    exit(1)
content = content.replace(target_render_sm, radar_function, 1)

# 3. Update hub_tabs to 4 tabs
target_hub_tabs = """                    HubTab hub_tabs[] = {
                        { 0, ICON_FA_MAGNIFYING_GLASS " BUSCA UNIVERSAL", "Busca instantânea por Artista, Música ou Vertente" },
                        { 1, ICON_FA_LINK " LINK DIRETO & SPOTIFY PLAYLIST", "Importe músicas individuais ou PLAYLISTS COMPLETAS do Spotify/SoundCloud" },
                        { 2, ICON_FA_LIST_CHECK " FILA & RECENTES", "Histórico de downloads, faixas baixadas e fila de espera" }
                    };

                    float tab_btn_w = (avail_w - 12.0f) / 3.0f;
                    for (int t = 0; t < 3; t++) {
                        bool is_sel = (cloud_hub_active_tab == hub_tabs[t].id);
                        if (is_sel) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.58f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.85f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 1.0f));
                        }
                        if (ImGui::Button(hub_tabs[t].label, ImVec2(tab_btn_w, 28))) {
                            cloud_hub_active_tab = hub_tabs[t].id;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", hub_tabs[t].desc);
                        ImGui::PopStyleColor(2);
                        if (t < 2) ImGui::SameLine(0, 6.0f);
                    }"""

replacement_hub_tabs = """                    HubTab hub_tabs[] = {
                        { 0, ICON_FA_MAGNIFYING_GLASS " BUSCA UNIVERSAL", "Busca instantânea por Artista, Música ou Vertente" },
                        { 1, ICON_FA_LINK " LINK DIRETO & SPOTIFY PLAYLIST", "Importe músicas individuais ou PLAYLISTS COMPLETAS do Spotify/SoundCloud" },
                        { 2, ICON_FA_LIST_CHECK " FILA & RECENTES", "Histórico de downloads, faixas baixadas e fila de espera" },
                        { 3, ICON_FA_RADIO " RADAR DE ARTISTAS", "Lancamentos e novidades exclusivas dos produtores que voce segue" }
                    };

                    float tab_btn_w = (avail_w - 18.0f) / 4.0f;
                    for (int t = 0; t < 4; t++) {
                        bool is_sel = (cloud_hub_active_tab == hub_tabs[t].id);
                        if (is_sel) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.58f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.85f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 1.0f));
                        }
                        if (ImGui::Button(hub_tabs[t].label, ImVec2(tab_btn_w, 28))) {
                            cloud_hub_active_tab = hub_tabs[t].id;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", hub_tabs[t].desc);
                        ImGui::PopStyleColor(2);
                        if (t < 3) ImGui::SameLine(0, 6.0f);
                    }"""

if target_hub_tabs not in content:
    print("Error: target_hub_tabs not found!")
    exit(1)
content = content.replace(target_hub_tabs, replacement_hub_tabs, 1)

# 4. Add ABA 3 check and render
target_tab2_end = """                        ImGui::Spacing();
                    }

                    // =========================================================================
                    // AUDITION BEAM / MINI PLAYER COMPACTO (48px)"""

replacement_tab2_end = """                        ImGui::Spacing();
                    }
                    // =========================================================================
                    // ABA 3: RADAR DE ARTISTAS SEGUIDOS
                    // =========================================================================
                    else if (cloud_hub_active_tab == 3) {
                        renderFollowedArtistsRadarTab(engine);
                    }

                    // =========================================================================
                    // AUDITION BEAM / MINI PLAYER COMPACTO (48px)"""

if target_tab2_end not in content:
    print("Error: target_tab2_end not found!")
    exit(1)
content = content.replace(target_tab2_end, replacement_tab2_end, 1)

# 5. Only render Search Results Table when cloud_hub_active_tab <= 1
target_search_table_start = """                    // =========================================================================
                    // BANNER DE PLAYLIST / ÁLBUM DETECTADO
                    // =========================================================================
                    auto results = spotify_service.getSearchResults();"""

replacement_search_table_start = """                    // =========================================================================
                    // BANNER DE PLAYLIST & RESULTADOS (APENAS NAS ABAS 0 E 1)
                    // =========================================================================
                    if (cloud_hub_active_tab <= 1) {
                    auto results = spotify_service.getSearchResults();"""

if target_search_table_start not in content:
    print("Error: target_search_table_start not found!")
    exit(1)
content = content.replace(target_search_table_start, replacement_search_table_start, 1)

target_search_table_end = """                            ImGui::EndTable();
                        }
                    }
                }

                // --- 4. RODAPÉ INFORMATIVO ---"""

replacement_search_table_end = """                            ImGui::EndTable();
                        }
                    }
                    } // fim de if (cloud_hub_active_tab <= 1)
                }

                // --- 4. RODAPÉ INFORMATIVO ---"""

if target_search_table_end not in content:
    print("Error: target_search_table_end not found!")
    exit(1)
content = content.replace(target_search_table_end, replacement_search_table_end, 1)

# Write to temp file and move
tmp_path = file_path + ".tmp"
with open(tmp_path, "w", encoding="utf-8") as f:
    f.write(content)

os.system(f'cmd /c move /y "{tmp_path.replace("/", chr(92))}" "{file_path.replace("/", chr(92))}"')
print("Successfully applied Etapa 3 Full!")
