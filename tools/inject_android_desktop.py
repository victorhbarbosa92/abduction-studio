import sys

desktop_func = '''    // =========================================================================
    // FASE 2: DESKTOP CENTRAL ESTILO ANDROID HUB (WALLPAPER & APP LAUNCHER)
    // =========================================================================
    inline int g_open_folder_idx = -1; // -1: fechado, 0: Plugins, 1: Sequenciadores, 2: Mixer, 3: IA, 4: Arquivos
    inline char g_desktop_search_query[128] = "";

    static void RenderAndroidDesktopHub(StemSeparationEngine& ai_engine) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.10f, 0.14f, 1.0f)); // #141A24 Deep Dark Slate
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        if (ImGui::Begin("DESKTOP", nullptr, flags)) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 wpos = ImGui::GetWindowPos();
            ImVec2 wsz = ImGui::GetWindowSize();
            float cx = wpos.x + wsz.x * 0.5f;
            float cy = wpos.y + wsz.y * 0.5f;

            // 1. Grade sutil de fundo cibernetico
            float grid_spacing = 48.0f;
            for (float gx = wpos.x; gx < wpos.x + wsz.x; gx += grid_spacing) {
                dl->AddLine(ImVec2(gx, wpos.y), ImVec2(gx, wpos.y + wsz.y), IM_COL32(28, 36, 48, 35), 1.0f);
            }
            for (float gy = wpos.y; gy < wpos.y + wsz.y; gy += grid_spacing) {
                dl->AddLine(ImVec2(wpos.x, gy), ImVec2(wpos.x + wsz.x, gy), IM_COL32(28, 36, 48, 35), 1.0f);
            }

            // 2. EMBLEMA CENTRAL DA NAVE ALIENIGENA EM RELEVO METALICO 3D (WALLPAPER)
            float s_rx = 135.0f;
            float s_ry = 48.0f;
            ImVec2 s_c = ImVec2(cx, cy - 20.0f);

            // Halo de brilho suave ciano atras da nave
            dl->AddCircleFilled(s_c, s_rx * 0.90f, IM_COL32(0, 229, 255, 14), 48);

            // Anel externo em baixo-relevo (sombra e chanfro de luz)
            dl->AddEllipseFilled(ImVec2(s_c.x, s_c.y + 3.0f), s_rx + 2.0f, s_ry + 2.0f, IM_COL32(10, 13, 18, 255), 48);
            dl->AddEllipseFilled(s_c, s_rx, s_ry, IM_COL32(26, 32, 42, 255), 48);
            dl->AddEllipse(s_c, s_rx, s_ry, IM_COL32(55, 68, 88, 255), 48, 1.5f);

            // Disco intermediario da nave
            dl->AddEllipseFilled(s_c, s_rx * 0.72f, s_ry * 0.65f, IM_COL32(34, 42, 54, 255), 40);
            dl->AddEllipse(s_c, s_rx * 0.72f, s_ry * 0.65f, IM_COL32(70, 88, 115, 255), 40, 1.0f);

            // Cupula superior da nave alienigena
            ImVec2 dome_c = ImVec2(s_c.x, s_c.y - s_ry * 0.28f);
            float dome_r = s_rx * 0.35f;
            dl->AddEllipseFilled(dome_c, dome_r, dome_r * 0.55f, IM_COL32(42, 54, 70, 255), 32);
            dl->AddEllipse(dome_c, dome_r, dome_r * 0.55f, IM_COL32(85, 110, 145, 255), 32, 1.2f);
            // Reflexo de luz na cupula
            dl->AddLine(ImVec2(dome_c.x - dome_r * 0.6f, dome_c.y - dome_r * 0.2f),
                        ImVec2(dome_c.x + dome_r * 0.2f, dome_c.y - dome_r * 0.35f),
                        IM_COL32(180, 225, 255, 160), 1.5f);

            // 3. BARRA DE PESQUISA RAPIDA ESTILO ANDROID
            float search_w = 400.0f;
            float search_h = 36.0f;
            ImGui::SetCursorPos(ImVec2((wsz.x - search_w) * 0.5f, 22.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.13f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.30f, 0.42f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 18.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::SetNextItemWidth(search_w);
            ImGui::InputTextWithHint("##DesktopSearch", "Buscar instrumentos, plugins, samples e stems... 🔍", g_desktop_search_query, sizeof(g_desktop_search_query));
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            // 4. AS 5 PASTAS DE APLICATIVOS ESTILO ANDROID (APP FOLDERS)
            struct AppFolderDef {
                const char* title;
                const char* subtitle;
                SciFiHUD::IconType icon;
                ImU32 glow_color;
                int app_count;
            };

            AppFolderDef folders[5] = {
                { "Sintetizadores", "11 Plugins VST", SciFiHUD::IconType::CHANNEL_RACK, IM_COL32(0, 229, 255, 255), 11 },
                { "Sequenciadores", "Piano Roll, Rack, Playlist", SciFiHUD::IconType::PIANO_KEYS, IM_COL32(255, 170, 40, 255), 4 },
                { "Mixagem & FX", "Mixer, Inserts, FFT, LUFS", SciFiHUD::IconType::MIXER_FADERS, IM_COL32(180, 80, 255, 255), 5 },
                { "IA & Stems", "Separador Neural & Inpainting", SciFiHUD::IconType::AI_NEURAL_STEMS, IM_COL32(0, 255, 180, 255), 4 },
                { "Arquivos & Samples", "Browser, Presets, Export", SciFiHUD::IconType::BROWSER_FOLDER, IM_COL32(70, 190, 255, 255), 6 }
            };

            float folder_w = 118.0f;
            float folder_h = 104.0f;
            float total_folders_w = 5 * folder_w + 4 * 20.0f;
            float start_fx = (wsz.x - total_folders_w) * 0.5f;
            if (start_fx < 20.0f) start_fx = 20.0f;
            float start_fy = wsz.y - 150.0f;

            for (int f = 0; f < 5; f++) {
                ImGui::SetCursorPos(ImVec2(start_fx + f * (folder_w + 20.0f), start_fy));
                ImVec2 fp = ImGui::GetCursorScreenPos();
                ImVec2 fp_max = ImVec2(fp.x + folder_w, fp.y + folder_h);

                char fid[32];
                snprintf(fid, sizeof(fid), "##DesktopFolderBtn_%d", f);
                ImGui::InvisibleButton(fid, ImVec2(folder_w, folder_h));
                bool hovered = ImGui::IsItemHovered();
                if (ImGui::IsItemClicked()) {
                    g_open_folder_idx = f;
                }

                // Corpo da pasta translucida estilo Android
                ImU32 f_bg = hovered ? IM_COL32(30, 42, 58, 230) : IM_COL32(18, 25, 36, 190);
                dl->AddRectFilled(fp, fp_max, f_bg, 14.0f);
                ImU32 f_border = hovered ? folders[f].glow_color : IM_COL32(45, 60, 80, 180);
                dl->AddRect(fp, fp_max, f_border, 14.0f, 0, hovered ? 2.0f : 1.0f);

                // Icone da pasta em destaque
                float ic_box_w = 34.0f;
                ImVec2 ic_p0 = ImVec2(fp.x + (folder_w - ic_box_w) * 0.5f, fp.y + 14.0f);
                ImVec2 ic_p1 = ImVec2(ic_p0.x + ic_box_w, ic_p0.y + ic_box_w);
                SciFiHUD::DrawIcon(dl, ic_p0, ic_p1, folders[f].icon, folders[f].glow_color, 1.8f);

                // Titulo da Pasta
                ImVec2 tsz = ImGui::CalcTextSize(folders[f].title);
                dl->AddText(ImVec2(fp.x + (folder_w - tsz.x) * 0.5f, fp.y + 54.0f), IM_COL32(230, 240, 255, 255), folders[f].title);

                // Subtitulo / Badge de contagem
                char badge[32];
                snprintf(badge, sizeof(badge), "%d Apps", folders[f].app_count);
                ImVec2 bsz = ImGui::CalcTextSize(badge);
                dl->AddText(ImVec2(fp.x + (folder_w - bsz.x) * 0.5f, fp.y + 72.0f), IM_COL32(130, 165, 200, 255), badge);
            }

            // 5. RODAPE DE VERSAO DISCRETO
            const char* ver_str = "Abduction Studio V4.0 [Build 4526] - 64-Bit Windows";
            ImVec2 vsz = ImGui::CalcTextSize(ver_str);
            dl->AddText(ImVec2(cx - vsz.x * 0.5f, wpos.y + wsz.y - 24.0f), IM_COL32(80, 100, 130, 200), ver_str);

            // =====================================================================
            // 6. POP-UP DE GAVETA DE APLICATIVOS (AO CLICAR NA PASTA)
            // =====================================================================
            if (g_open_folder_idx >= 0 && g_open_folder_idx < 5) {
                ImGui::OpenPopup("##AndroidAppDrawerPopup");
            }

            ImGui::SetNextWindowPos(ImVec2(cx - 275.0f, cy - 170.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(550.0f, 340.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.12f, 0.17f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.80f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

            if (ImGui::BeginPopup("##AndroidAppDrawerPopup")) {
                int f = g_open_folder_idx;
                if (f >= 0 && f < 5) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "FOLDER: %s", folders[f].title);
                    ImGui::SameLine(0, 10);
                    ImGui::TextDisabled("| %s", folders[f].subtitle);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65);
                    if (ImGui::Button("X Fechar", ImVec2(60, 20))) {
                        g_open_folder_idx = -1;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Renderizar itens da pasta selecionada
                    if (f == 0) { // PASTA 1: SINTETIZADORES & PLUGINS
                        if (ImGui::Button("Kuro Psy Rolling Bass Engine", ImVec2(255, 42))) {
                            g_psy_rolling_bass_ui.is_open = true;
                            g_psy_rolling_bass_ui.need_focus = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("FL DirectWave Multi-Sampler", ImVec2(255, 42))) {
                            g_directwave_ui.open();
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        if (ImGui::Button("Fruity Granulizer (Grain Cloud)", ImVec2(255, 42))) {
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("SliceX Beat Slicer", ImVec2(255, 42))) {
                            g_stem_beat_slicer_ui.open();
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        if (ImGui::Button("FL Flex Synth (Psytrance)", ImVec2(255, 42))) {
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Delay Lama (Monge 3D VST)", ImVec2(255, 42))) {
                            show_delay_lama = true;
                            focus_delay_lama = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                    }
                    else if (f == 1) { // PASTA 2: SEQUENCIADORES & COMPOSICAO
                        if (ImGui::Button("Piano Roll (88 Teclas - F7)", ImVec2(255, 46))) {
                            show_piano_roll = true;
                            g_need_focus_piano_roll = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Channel Rack (16 Passos - F6)", ImVec2(255, 46))) {
                            show_step_sequencer = true;
                            g_need_focus_step_sequencer = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        if (ImGui::Button("Playlist / Arranjador (F5)", ImVec2(255, 46))) {
                            show_playlist = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Novo Padrao (Pattern)", ImVec2(255, 46))) {
                            Pattern p;
                            p.id = (int)g_clip_manager.global_patterns.size() + 1;
                            p.name = "Pattern " + std::to_string(p.id);
                            p.color = 0xFF00E5FF;
                            g_clip_manager.global_patterns.push_back(p);
                            g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                    }
                    else if (f == 2) { // PASTA 3: MIXER & FX
                        if (ImGui::Button("Mixer Console (F9)", ImVec2(255, 46))) {
                            show_mixer = true;
                            set_mixer_focus = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Rack de Efeitos FX", ImVec2(255, 46))) {
                            show_mixer = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                    }
                    else if (f == 3) { // PASTA 4: IA & STEMS
                        if (ImGui::Button("Separador Neural de Stems (IA)", ImVec2(255, 46))) {
                            g_cloud_stem_ui.open();
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Gerador Rolling Bass Psytrance", ImVec2(255, 46))) {
                            g_psy_rolling_bass_ui.is_open = true;
                            g_psy_rolling_bass_ui.need_focus = true;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                    }
                    else if (f == 4) { // PASTA 5: ARQUIVOS & BROWSER
                        if (ImGui::Button("Browser de Amostras (F8)", ImVec2(255, 46))) {
                            show_browser = !show_browser;
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Salvar Projeto (.kuro)", ImVec2(255, 46))) {
                            std::string path = FileDialog::SaveFile("Abduction Project (*.kuro)\\0*.kuro\\0");
                            if (!path.empty()) {
                                if (path.find(".kuro") == std::string::npos) path += ".kuro";
                                ProjectManagerBridge::Save(path);
                            }
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }
'''

path = 'src/ui/StudioUI.h'
with open(path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

marker = '    static void RenderStudioMode(StemSeparationEngine& ai_engine) {'
idx = content.find(marker)
if idx != -1:
    # Check if RenderAndroidDesktopHub is already there
    if 'RenderAndroidDesktopHub' not in content:
        call_insertion = '        // 1. ÁREA DE TRABALHO LIMPA ESTILO ANDROID HUB (DESKTOP)\n        RenderAndroidDesktopHub(ai_engine);\n'
        content = content[:idx] + desktop_func + '\n\n' + content[idx:idx+len(marker)] + '\n' + call_insertion + content[idx+len(marker):]
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print("Injected RenderAndroidDesktopHub and call into StudioUI.h successfully!")
    else:
        print("RenderAndroidDesktopHub already present!")
else:
    print("Marker RenderStudioMode not found!")
