import sys

path = 'src/ui/StudioUI.h'
with open(path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

old_drawer_code = '''            // =====================================================================
            // 6. GAVETA DE APLICATIVOS FLUTUANTE ESTILO ANDROID (AO CLICAR NA PASTA)
            // =====================================================================
            if (g_open_folder_idx >= 0 && g_open_folder_idx < 5) {
                int f = g_open_folder_idx;
                ImGui::SetNextWindowPos(ImVec2(cx - 280.0f, cy - 180.0f), ImGuiCond_Appearing);
                ImGui::SetNextWindowSize(ImVec2(560.0f, 360.0f));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.12f, 0.17f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, folders[f].glow_color);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

                bool drawer_open = true;
                char win_id[64];
                snprintf(win_id, sizeof(win_id), "PASTA: %s (Abduction Hub)###AndroidFolderDrawer", folders[f].title);
                if (ImGui::Begin(win_id, &drawer_open, ImGuiWindowFlags_NoCollapse)) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "PASTA: %s", folders[f].title);
                    ImGui::SameLine(0, 10);
                    ImGui::TextDisabled("| %s", folders[f].subtitle);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65);
                    if (ImGui::Button("X Fechar", ImVec2(60, 20))) {
                        drawer_open = false;
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
                            std::string path = FileDialog::SaveFile("Abduction Project (*.kuro)\\\\0*.kuro\\\\0");
                            if (!path.empty()) {
                                if (path.find(".kuro") == std::string::npos) path += ".kuro";
                                ProjectManagerBridge::Save(path);
                            }
                            ImGui::CloseCurrentPopup();
                            g_open_folder_idx = -1;
                        }
                    }
                }
                ImGui::End();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                if (!drawer_open) {
                    g_open_folder_idx = -1;
                }
            }'''

new_drawer_code = '''            // =====================================================================
            // 6. GAVETA DE APLICATIVOS FLUTUANTE ESTILO ANDROID (AO CLICAR NA PASTA OU HUB)
            // =====================================================================
            if (g_show_app_drawer && g_open_folder_idx < 0) {
                g_open_folder_idx = 0;
            }

            if (g_open_folder_idx >= 0 && g_open_folder_idx < 5) {
                int f = g_open_folder_idx;
                ImGui::SetNextWindowPos(ImVec2(cx - 300.0f, cy - 200.0f), ImGuiCond_Appearing);
                ImGui::SetNextWindowSize(ImVec2(600.0f, 400.0f));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.12f, 0.17f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, folders[f].glow_color);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

                bool drawer_open = true;
                if (ImGui::Begin("📁 ABDUCTION APP HUB & GAVETA DE FERRAMENTAS###AndroidFolderDrawer", &drawer_open, ImGuiWindowFlags_NoCollapse)) {
                    // Header com fechar
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "ABDUCTION APP HUB");
                    ImGui::SameLine(0, 10);
                    ImGui::TextDisabled("| Central de Ferramentas Categorizadas");
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65);
                    if (ImGui::Button("X Fechar", ImVec2(60, 20))) {
                        drawer_open = false;
                    }
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Abas das 5 Categorias
                    if (ImGui::BeginTabBar("##AppDrawerTabs")) {
                        for (int t = 0; t < 5; t++) {
                            ImGuiTabItemFlags t_flags = (t == f) ? ImGuiTabItemFlags_SetSelected : 0;
                            if (ImGui::BeginTabItem(folders[t].title, nullptr, t_flags)) {
                                f = t;
                                g_open_folder_idx = t;

                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "%s", folders[t].subtitle);
                                ImGui::Separator();
                                ImGui::Spacing();

                                if (t == 0) { // PASTA 1: SINTETIZADORES & PLUGINS
                                    if (ImGui::Button("🛸 Kuro Psy Rolling Bass Engine", ImVec2(275, 46))) {
                                        g_psy_rolling_bass_ui.is_open = true;
                                        g_psy_rolling_bass_ui.need_focus = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("💽 FL DirectWave Multi-Sampler", ImVec2(275, 46))) {
                                        g_directwave_ui.open();
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    if (ImGui::Button("🌊 Fruity Granulizer (Grain Cloud)", ImVec2(275, 46))) {
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("✂ SliceX Beat Slicer", ImVec2(275, 46))) {
                                        g_stem_beat_slicer_ui.open();
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    if (ImGui::Button("🎹 FL Flex Synth (Psytrance)", ImVec2(275, 46))) {
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("🧘 Delay Lama (Monge 3D VST)", ImVec2(275, 46))) {
                                        show_delay_lama = true;
                                        focus_delay_lama = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                }
                                else if (t == 1) { // PASTA 2: SEQUENCIADORES & COMPOSICAO
                                    if (ImGui::Button("🎹 Piano Roll (88 Teclas - F7)", ImVec2(275, 46))) {
                                        show_piano_roll = true;
                                        g_need_focus_piano_roll = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("🎛 Channel Rack (16 Passos - F6)", ImVec2(275, 46))) {
                                        show_step_sequencer = true;
                                        g_need_focus_step_sequencer = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    if (ImGui::Button("📊 Playlist / Arranjador (F5)", ImVec2(275, 46))) {
                                        show_playlist = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("➕ Novo Padrao (Pattern)", ImVec2(275, 46))) {
                                        Pattern p;
                                        p.id = (int)g_clip_manager.global_patterns.size() + 1;
                                        p.name = "Pattern " + std::to_string(p.id);
                                        p.color = 0xFF00E5FF;
                                        g_clip_manager.global_patterns.push_back(p);
                                        g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                }
                                else if (t == 2) { // PASTA 3: MIXER & FX
                                    if (ImGui::Button("🎚 Mixer Console (F9)", ImVec2(275, 46))) {
                                        show_mixer = true;
                                        set_mixer_focus = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("🎛 Rack de Efeitos FX", ImVec2(275, 46))) {
                                        show_mixer = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                }
                                else if (t == 3) { // PASTA 4: IA & STEMS
                                    if (ImGui::Button("🧠 Separador Neural de Stems (IA)", ImVec2(275, 46))) {
                                        g_cloud_stem_ui.open();
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("🛸 Gerador Rolling Bass Psytrance", ImVec2(275, 46))) {
                                        g_psy_rolling_bass_ui.is_open = true;
                                        g_psy_rolling_bass_ui.need_focus = true;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                }
                                else if (t == 4) { // PASTA 5: ARQUIVOS & BROWSER
                                    if (ImGui::Button("📁 Browser de Amostras (F8)", ImVec2(275, 46))) {
                                        show_browser = !show_browser;
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button("💾 Salvar Projeto (.kuro)", ImVec2(275, 46))) {
                                        std::string path = FileDialog::SaveFile("Abduction Project (*.kuro)\\\\0*.kuro\\\\0");
                                        if (!path.empty()) {
                                            if (path.find(".kuro") == std::string::npos) path += ".kuro";
                                            ProjectManagerBridge::Save(path);
                                        }
                                        g_open_folder_idx = -1;
                                        g_show_app_drawer = false;
                                    }
                                }

                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                }
                ImGui::End();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                if (!drawer_open) {
                    g_open_folder_idx = -1;
                    g_show_app_drawer = false;
                }
            }'''

if old_drawer_code in content:
    content = content.replace(old_drawer_code, new_drawer_code)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print("Enhanced tabbed app hub applied successfully!")
else:
    print("Could not match old_drawer_code exactly!")
