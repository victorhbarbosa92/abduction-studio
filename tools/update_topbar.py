import sys

new_code = '''    // =========================================================================
    // FASE 1: CONCEITO 2 - QUANTUM CYBER-DECK (MODULAR HARDWARE WORKSTATION)
    // =========================================================================
    inline std::string g_fl_hint_text = "Abduction Quantum Workstation";
    inline void SetFLHint(const std::string& hint) { g_fl_hint_text = hint; }
    inline bool g_show_app_drawer = false;
    inline float g_master_pitch_semitones = 0.0f;
    inline bool g_metronome_active = false;
    inline bool g_typing_keyboard_active = true;
    inline bool g_countdown_active = false;
    inline bool g_loop_record_active = false;

    // Helper para desenhar os Pads de Módulo do Conceito 2 (Backlit Touch-Pads de Luxo)
    inline bool RenderQuantumPad(ImDrawList* dl, const char* id, const char* label, SciFiHUD::IconType icon, bool is_active, const char* tooltip) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float pad_w = 47.0f;
        float pad_h = 27.0f;
        ImVec2 p_min = pos;
        ImVec2 p_max = ImVec2(pos.x + pad_w, pos.y + pad_h);

        ImGui::InvisibleButton(id, ImVec2(pad_w, pad_h));
        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        if (hovered && tooltip) {
            SetFLHint(tooltip);
        }

        // 1. Corpo do Pad em silicone escuro
        ImU32 bg_col = is_active ? IM_COL32(14, 38, 48, 255) : (hovered ? IM_COL32(28, 36, 46, 255) : IM_COL32(18, 22, 28, 255));
        dl->AddRectFilled(p_min, p_max, bg_col, 4.0f);

        // 2. Borda iluminada (Halo âmbar sutil em repouso, ciano vivo quando ativo)
        ImU32 border_col = is_active ? IM_COL32(0, 229, 255, 255) : (hovered ? IM_COL32(255, 180, 50, 240) : IM_COL32(255, 160, 40, 110));
        dl->AddRect(p_min, p_max, border_col, 4.0f, 0, is_active ? 1.6f : 1.0f);

        // 3. Ícone vetorial ciano/branco
        ImU32 icon_col = is_active ? IM_COL32(0, 229, 255, 255) : (hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 215, 240, 255));
        float icon_sz = 12.0f;
        ImVec2 ic_min = ImVec2(pos.x + (pad_w - icon_sz) * 0.5f, pos.y + 2.5f);
        ImVec2 ic_max = ImVec2(ic_min.x + icon_sz, ic_min.y + icon_sz);
        SciFiHUD::DrawIcon(dl, ic_min, ic_max, icon, icon_col, 1.3f);

        // 4. Texto micro tipografia no rodapé do pad
        ImVec2 text_sz = ImGui::CalcTextSize(label);
        ImU32 text_col = is_active ? IM_COL32(0, 229, 255, 255) : (hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 185, 210, 255));
        dl->AddText(ImVec2(pos.x + (pad_w - text_sz.x) * 0.5f, pos.y + 15.0f), text_col, label);

        return clicked;
    }

    static void RenderFLStyleTopbar(StemSeparationEngine& ai_engine) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        const float topbar_h = 68.0f;
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, topbar_h));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoScrollWithMouse |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 3.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.14f, 0.18f, 1.0f)); // Chassi grafite usinado escuro

        if (ImGui::Begin("##FLStyleTopbar", nullptr, flags)) {
            ImDrawList* wdl = ImGui::GetWindowDrawList();
            ImDrawList* dl = wdl;

            // Reset da dica padrao ao iniciar o frame (sera sobrescrita caso algum item seja hovered)
            if (!ImGui::IsAnyItemHovered()) {
                g_fl_hint_text = "Abduction Quantum Workstation";
            }

            // Permite arrastar a janela clicando em areas vazias da barra superior
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win && !glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    int wx, wy;
                    glfwGetWindowPos(win, &wx, &wy);
                    glfwSetWindowPos(win, wx + (int)delta.x, wy + (int)delta.y);
                }
            }

            // Linha chanfrada de reforço do chassi no fundo
            wdl->AddLine(ImVec2(viewport->Pos.x, viewport->Pos.y + topbar_h - 1), 
                         ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + topbar_h - 1), 
                         IM_COL32(0, 229, 255, 180), 1.2f);

            // =========================================================================
            // 1. MASTER VOLUME CONSOLE BAY (MÓDULO DE LUXO EM TITÂNIO ANODIZADO NA ESQUERDA)
            // =========================================================================
            ImGui::SetCursorPos(ImVec2(6.0f, 3.0f));
            float bay_w = 84.0f;
            float bay_h = 62.0f;
            ImVec2 bay_p = ImGui::GetCursorScreenPos();

            // Moldura rebaixada metálica do Master Volume Bay
            wdl->AddRectFilled(bay_p, ImVec2(bay_p.x + bay_w, bay_p.y + bay_h), IM_COL32(18, 22, 28, 255), 5.0f);
            wdl->AddRect(bay_p, ImVec2(bay_p.x + bay_w, bay_p.y + bay_h), IM_COL32(40, 52, 68, 255), 5.0f);

            // Parafusos sextavados nos 4 cantos
            wdl->AddCircleFilled(ImVec2(bay_p.x + 5, bay_p.y + 5), 1.4f, IM_COL32(75, 90, 110, 255));
            wdl->AddCircleFilled(ImVec2(bay_p.x + bay_w - 5, bay_p.y + 5), 1.4f, IM_COL32(75, 90, 110, 255));
            wdl->AddCircleFilled(ImVec2(bay_p.x + 5, bay_p.y + bay_h - 5), 1.4f, IM_COL32(75, 90, 110, 255));
            wdl->AddCircleFilled(ImVec2(bay_p.x + bay_w - 5, bay_p.y + bay_h - 5), 1.4f, IM_COL32(75, 90, 110, 255));

            // Centro do Knob Master
            static float master_vol = 0.85f;
            ImVec2 knob_c = ImVec2(bay_p.x + bay_w * 0.5f, bay_p.y + 25.0f);
            float knob_r = 16.5f;

            // Soquete escuro
            wdl->AddCircleFilled(knob_c, knob_r + 4.0f, IM_COL32(10, 12, 16, 255), 24);
            wdl->AddCircle(knob_c, knob_r + 4.0f, IM_COL32(32, 42, 54, 255), 24, 1.0f);

            // Arco de LEDs bicolor: âmbar (à esquerda) até ciano (à direita)
            float a_start = 2.4f;
            float a_end = 7.0f;
            float a_curr = a_start + (master_vol / 1.25f) * (a_end - a_start);

            for (float a = a_start; a <= a_curr; a += 0.16f) {
                float t = (a - a_start) / (a_end - a_start);
                ImU32 led_c = (t < 0.45f) ? IM_COL32(255, (int)(150 + t * 100), 20, 255) : IM_COL32((int)((1.0f - t) * 100), 225, 255, 255);
                float px = knob_c.x + cosf(a) * (knob_r + 2.8f);
                float py = knob_c.y + sinf(a) * (knob_r + 2.8f);
                wdl->AddCircleFilled(ImVec2(px, py), 1.4f, led_c, 6);
            }

            // Corpo usinado do Knob em titânio
            wdl->AddCircleFilled(knob_c, knob_r, IM_COL32(44, 52, 64, 255), 24);
            wdl->AddCircle(knob_c, knob_r, IM_COL32(90, 105, 125, 255), 24, 1.2f);
            wdl->AddCircleFilled(knob_c, knob_r - 4.5f, IM_COL32(30, 36, 44, 255), 20);

            // Marcador indicador branco no knob
            float ind_x = knob_c.x + cosf(a_curr) * (knob_r - 2.0f);
            float ind_y = knob_c.y + sinf(a_curr) * (knob_r - 2.0f);
            wdl->AddLine(knob_c, ImVec2(ind_x, ind_y), IM_COL32(255, 255, 255, 255), 2.0f);

            // Display numérico em dB / % abaixo do knob
            float vol_db = (master_vol > 0.001f) ? (20.0f * log10f(master_vol)) : -60.0f;
            char db_str[32];
            snprintf(db_str, sizeof(db_str), "%+.1fdB  %.0f%%", vol_db, master_vol * 100.0f);
            ImVec2 db_sz = ImGui::CalcTextSize(db_str);
            wdl->AddText(ImVec2(knob_c.x - db_sz.x * 0.5f, bay_p.y + 46.0f), IM_COL32(255, 180, 50, 255), db_str);

            // Botão invisível interativo para o Knob
            ImGui::InvisibleButton("##QuantumMasterVol", ImVec2(bay_w, bay_h));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                master_vol -= ImGui::GetIO().MouseDelta.y * 0.006f;
                master_vol = std::clamp(master_vol, 0.0f, 1.25f);
            }
            if (ImGui::IsItemHovered()) {
                char hbuf[64];
                snprintf(hbuf, sizeof(hbuf), "Master Volume Console: %+.1f dB (Arraste para ajustar)", vol_db);
                SetFLHint(hbuf);
            }

            // =========================================================================
            // 2. DECK 1: LINHA SUPERIOR (MENUS, TRANSPORTE, TELAS OLED, FFT SPECTRUM, CPU)
            // =========================================================================
            ImGui::SetCursorPos(ImVec2(96.0f, 4.0f));
            ImGui::BeginGroup();
            {
                // Menus de Texto FL Studio Modernizados
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.94f, 0.98f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.24f, 0.28f, 0.34f, 1.0f));

                if (ImGui::Button("FILE##fl_menu")) ImGui::OpenPopup("PopupArquivo");
                if (ImGui::IsItemHovered()) SetFLHint("Menu File: New, Open, Save, Export Audio");
                if (ImGui::BeginPopup("PopupArquivo")) {
                    if (ImGui::MenuItem("Novo Projeto (Reset)")) {
                        ai_engine.reset();
                        g_clip_manager.reset();
                        ::timeline.setMasterFrame(0);
                        ::timeline.setPlaying(false);
                    }
                    if (ImGui::MenuItem("Abrir Projeto... (Ctrl+O)")) {
                        std::string path = FileDialog::OpenFile("Abduction Project (*.kuro)\\0*.kuro\\0");
                        if (!path.empty()) ProjectManagerBridge::Load(path);
                    }
                    if (ImGui::MenuItem("Salvar Projeto (Ctrl+S)")) {
                        std::string path = FileDialog::SaveFile("Abduction Project (*.kuro)\\0*.kuro\\0");
                        if (!path.empty()) {
                            if (path.find(".kuro") == std::string::npos) path += ".kuro";
                            ProjectManagerBridge::Save(path);
                        }
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exportar Master (.wav)...")) {
                        g_record_manager.saveToWav("Kuro_Export_Master.wav");
                    }
                    if (ImGui::MenuItem("Exportar Stems Separadas...")) {
                        KuroAudio::OfflineRenderer::RenderStems(master_graph, 10.0f, ".");
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Sair do Abduction Studio")) {
                        GLFWwindow* win = glfwGetCurrentContext();
                        if (win) glfwSetWindowShouldClose(win, GLFW_TRUE);
                    }
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("EDIT##fl_menu")) ImGui::OpenPopup("PopupEditar");
                if (ImGui::IsItemHovered()) SetFLHint("Menu Edit: Undo, Redo, Cut, Copy, Paste");
                if (ImGui::BeginPopup("PopupEditar")) {
                    if (ImGui::MenuItem("Desfazer (Undo)", "Ctrl+Z")) g_clip_manager.undo();
                    if (ImGui::MenuItem("Refazer (Redo)", "Ctrl+Y")) g_clip_manager.redo();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cortar", "Ctrl+X")) {}
                    if (ImGui::MenuItem("Copiar", "Ctrl+C")) {}
                    if (ImGui::MenuItem("Colar", "Ctrl+V")) {}
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("ADD##fl_menu")) ImGui::OpenPopup("PopupAdicionar");
                if (ImGui::IsItemHovered()) SetFLHint("Menu Add: Adicionar Instrumentos e Sintetizadores");
                if (ImGui::BeginPopup("PopupAdicionar")) {
                    if (ImGui::MenuItem("🛸 Kuro Psy Rolling Bass Engine")) {
                        g_psy_rolling_bass_ui.is_open = true;
                        g_psy_rolling_bass_ui.need_focus = true;
                    }
                    if (ImGui::MenuItem("FL DirectWave Multi-Sampler")) g_directwave_ui.open();
                    if (ImGui::MenuItem("SliceX Beat Slicer")) g_stem_beat_slicer_ui.open();
                    if (ImGui::MenuItem("Delay Lama (Monge 3D)")) show_delay_lama = true;
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("PATTERNS##fl_menu")) ImGui::OpenPopup("PopupPadroes");
                if (ImGui::IsItemHovered()) SetFLHint("Menu Patterns: Gerenciamento de Padroes e Sequencias");
                if (ImGui::BeginPopup("PopupPadroes")) {
                    if (ImGui::MenuItem("Novo Padrao (Pattern)")) {
                        Pattern p;
                        p.id = (int)g_clip_manager.global_patterns.size() + 1;
                        p.name = "Pattern " + std::to_string(p.id);
                        p.color = 0xFF00E5FF;
                        g_clip_manager.global_patterns.push_back(p);
                        g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                    }
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("VIEW##fl_menu")) ImGui::OpenPopup("PopupExibir");
                if (ImGui::IsItemHovered()) SetFLHint("Menu View: Alternar Janelas Principais (F5, F6, F7, F9)");
                if (ImGui::BeginPopup("PopupExibir")) {
                    if (ImGui::MenuItem("Playlist / Arranjo (F5)", nullptr, show_playlist)) show_playlist = !show_playlist;
                    if (ImGui::MenuItem("Channel Rack (F6)", nullptr, show_step_sequencer)) {
                        show_step_sequencer = !show_step_sequencer;
                        if (show_step_sequencer) g_need_focus_step_sequencer = true;
                    }
                    if (ImGui::MenuItem("Piano Roll (F7)", nullptr, show_piano_roll)) {
                        show_piano_roll = !show_piano_roll;
                        if (show_piano_roll) g_need_focus_piano_roll = true;
                    }
                    if (ImGui::MenuItem("Mixer Console (F9)", nullptr, show_mixer)) show_mixer = !show_mixer;
                    if (ImGui::MenuItem("Browser de Amostras (F8)", nullptr, show_browser)) show_browser = !show_browser;
                    ImGui::Separator();
                    if (ImGui::MenuItem("Gaveta de Apps / Android Hub (F12)", nullptr, g_show_app_drawer)) g_show_app_drawer = !g_show_app_drawer;
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("OPTIONS##fl_menu")) ImGui::OpenPopup("PopupOpcoes");
                if (ImGui::IsItemHovered()) SetFLHint("Menu Options: Audio, MIDI e Preferencias");
                if (ImGui::BeginPopup("PopupOpcoes")) {
                    if (ImGui::MenuItem("Dispositivos de Audio (WASAPI/ASIO)...")) {}
                    if (ImGui::MenuItem("Configuracoes MIDI...")) {}
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("TOOLS##fl_menu")) ImGui::OpenPopup("PopupFerramentas");
                if (ImGui::IsItemHovered()) SetFLHint("Menu Tools: Separador Neural de Stems e Sintese");
                if (ImGui::BeginPopup("PopupFerramentas")) {
                    if (ImGui::MenuItem("Separador Neural de Stems (IA)")) g_cloud_stem_ui.toggle();
                    if (ImGui::MenuItem("Gerador de Linha de Baixo Psytrance")) {
                        g_psy_rolling_bass_ui.is_open = true;
                        g_psy_rolling_bass_ui.need_focus = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::SameLine(0, 2);

                if (ImGui::Button("HELP##fl_menu")) ImGui::OpenPopup("PopupAjuda");
                if (ImGui::IsItemHovered()) SetFLHint("Menu Help: Manual e Sobre");
                if (ImGui::BeginPopup("PopupAjuda")) {
                    if (ImGui::MenuItem("Guia do Abduction Studio V4.0")) {}
                    ImGui::EndPopup();
                }
                ImGui::PopStyleColor(2);

                ImGui::SameLine(0, 6);
                ImGui::TextDisabled("|");
                ImGui::SameLine(0, 6);

                // Botões de Transporte Backlit Silicone
                bool is_pat = ::timeline.is_pattern_mode;
                ImGui::PushStyleColor(ImGuiCol_Button, is_pat ? ImVec4(0.96f, 0.58f, 0.10f, 1.0f) : ImVec4(0.12f, 0.15f, 0.19f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, is_pat ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.85f, 0.60f, 0.20f, 1.0f));
                if (ImGui::Button("PAT / SONG", ImVec2(68, 22))) {
                    ::timeline.is_pattern_mode = !::timeline.is_pattern_mode;
                }
                if (ImGui::IsItemHovered()) SetFLHint("Modo de Reproducao: Pattern (PAT) ou Musica Inteira (SONG)");
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 3);

                bool is_p = ::is_playing;
                if (SciFiHUD::SciFiTransportBtn("##FLPlay", is_p ? SciFiHUD::IconType::PAUSE : SciFiHUD::IconType::PLAY, is_p, ImVec4(0.00f, 0.90f, 1.00f, 1.0f), ImVec2(26, 22), "Play / Pause (Espaco)")) {
                    TransportController::TogglePlayPause(::timeline);
                }
                if (ImGui::IsItemHovered()) SetFLHint(is_p ? "Pausar Reproducao (Espaco)" : "Iniciar Reproducao (Espaco)");
                ImGui::SameLine(0, 3);

                if (SciFiHUD::SciFiTransportBtn("##FLStop", SciFiHUD::IconType::STOP, false, ImVec4(0.00f, 0.80f, 0.95f, 1.0f), ImVec2(26, 22), "Stop (Parar)")) {
                    TransportController::Stop(::timeline);
                }
                if (ImGui::IsItemHovered()) SetFLHint("Parar Reproducao e Resetar Cursor");
                ImGui::SameLine(0, 3);

                bool rec = g_record_manager.isRecording();
                if (SciFiHUD::SciFiTransportBtn("##FLRec", SciFiHUD::IconType::RECORD, rec, ImVec4(0.98f, 0.15f, 0.25f, 1.0f), ImVec2(26, 22), "Armar Gravacao")) {
                    if (rec) g_record_manager.setRecording(false);
                    else     g_record_manager.setRecording(true, false);
                }
                if (ImGui::IsItemHovered()) SetFLHint(rec ? "Gravando Audio... Clique para Parar" : "Armar Gravacao Master / MIDI");
                ImGui::SameLine(0, 8);

                // Telas Duplas OLED (Display 1: BPM / TEMPO 4/4)
                float current_bpm = ::timeline.getBPM();
                ImVec2 bpm_p = ImGui::GetCursorScreenPos();
                float bpm_w = 84.0f;
                float bpm_h = 24.0f;
                wdl->AddRectFilled(bpm_p, ImVec2(bpm_p.x + bpm_w, bpm_p.y + bpm_h), IM_COL32(10, 14, 20, 255), 3.0f);
                wdl->AddRect(bpm_p, ImVec2(bpm_p.x + bpm_w, bpm_p.y + bpm_h), IM_COL32(30, 44, 60, 255), 3.0f);
                wdl->AddText(ImVec2(bpm_p.x + 5, bpm_p.y + 1), IM_COL32(90, 130, 170, 255), "BPM");
                wdl->AddText(ImVec2(bpm_p.x + bpm_w - 24, bpm_p.y + 1), IM_COL32(80, 110, 140, 255), "4/4");
                char bpm_buf[32];
                snprintf(bpm_buf, sizeof(bpm_buf), "%.3f", current_bpm);
                wdl->AddText(ImVec2(bpm_p.x + 5, bpm_p.y + 10), IM_COL32(0, 229, 255, 255), bpm_buf);
                // Setas ▲ ▼
                wdl->AddTriangleFilled(ImVec2(bpm_p.x + bpm_w - 7, bpm_p.y + 13), ImVec2(bpm_p.x + bpm_w - 11, bpm_p.y + 16), ImVec2(bpm_p.x + bpm_w - 3, bpm_p.y + 16), IM_COL32(120, 160, 200, 255));
                wdl->AddTriangleFilled(ImVec2(bpm_p.x + bpm_w - 7, bpm_p.y + 21), ImVec2(bpm_p.x + bpm_w - 11, bpm_p.y + 18), ImVec2(bpm_p.x + bpm_w - 3, bpm_p.y + 18), IM_COL32(120, 160, 200, 255));

                ImGui::InvisibleButton("##BpmDisplayBtn", ImVec2(bpm_w, bpm_h));
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    current_bpm -= ImGui::GetIO().MouseDelta.y * 0.2f;
                    current_bpm = std::clamp(current_bpm, 40.0f, 300.0f);
                    ::timeline.setBPM(current_bpm);
                }
                if (ImGui::IsItemHovered()) SetFLHint("Andamento da Musica (BPM) - Clique e arraste para alterar");
                ImGui::SameLine(0, 6);

                // Telas Duplas OLED (Display 2: TIME / BAR 1)
                ImVec2 time_p = ImGui::GetCursorScreenPos();
                float time_w = 90.0f;
                float time_h = 24.0f;
                wdl->AddRectFilled(time_p, ImVec2(time_p.x + time_w, time_p.y + time_h), IM_COL32(10, 14, 20, 255), 3.0f);
                wdl->AddRect(time_p, ImVec2(time_p.x + time_w, time_p.y + time_h), IM_COL32(30, 44, 60, 255), 3.0f);

                double current_time_sec = (double)::timeline.getMasterFrame() / 44100.0;
                double beats_per_sec = (double)current_bpm / 60.0;
                double total_beats = current_time_sec * beats_per_sec;
                int bar = (int)(total_beats / 4.0) + 1;
                int step = ((int)total_beats % 4) + 1;
                int tick = (int)((total_beats - (int)total_beats) * 100.0);

                char time_str[32];
                snprintf(time_str, sizeof(time_str), "%d:%02d:%02d", bar, step, tick);
                wdl->AddText(ImVec2(time_p.x + 5, time_p.y + 1), IM_COL32(90, 130, 170, 255), "TIME");
                wdl->AddText(ImVec2(time_p.x + time_w - 36, time_p.y + 1), IM_COL32(80, 110, 140, 255), "BAR 1");
                wdl->AddText(ImVec2(time_p.x + 5, time_p.y + 10), IM_COL32(0, 229, 255, 255), time_str);

                ImGui::Dummy(ImVec2(time_w, time_h));
                if (ImGui::IsItemHovered()) SetFLHint("Tempo da Musica: Bar : Beat : Tick");
                ImGui::SameLine(0, 6);

                // Mini Analisador de Espectro FFT em Tempo Real (Conceito 2)
                ImVec2 spec_p = ImGui::GetCursorScreenPos();
                float spec_w = 72.0f;
                float spec_h = 24.0f;
                wdl->AddRectFilled(spec_p, ImVec2(spec_p.x + spec_w, spec_p.y + spec_h), IM_COL32(10, 13, 18, 255), 3.0f);
                wdl->AddRect(spec_p, ImVec2(spec_p.x + spec_w, spec_p.y + spec_h), IM_COL32(28, 38, 52, 255), 3.0f);

                const int num_bars = 14;
                float bar_w = (spec_w - 8.0f) / num_bars - 1.0f;
                static float bar_heights[14] = { 0.2f, 0.4f, 0.6f, 0.5f, 0.7f, 0.8f, 0.6f, 0.7f, 0.5f, 0.4f, 0.3f, 0.5f, 0.3f, 0.2f };

                for (int b = 0; b < num_bars; b++) {
                    float target = is_p ? (0.20f + 0.75f * fabsf(sinf((float)ImGui::GetTime() * 9.0f + b * 0.7f))) : 0.08f;
                    bar_heights[b] += (target - bar_heights[b]) * 0.30f;
                    float bh = bar_heights[b] * (spec_h - 6.0f);
                    float bx = spec_p.x + 4.0f + b * (bar_w + 1.0f);
                    float by = spec_p.y + spec_h - 3.0f - bh;
                    
                    ImU32 bar_col = (b < 7) ? IM_COL32(160 + b * 10, 60 + b * 20, 255, 240) : IM_COL32(0, 210 + b * 3, 255, 240);
                    wdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + bar_w, spec_p.y + spec_h - 3.0f), bar_col, 1.0f);
                    wdl->AddLine(ImVec2(bx, by - 1), ImVec2(bx + bar_w, by - 1), IM_COL32(255, 255, 255, 220));
                }

                ImGui::Dummy(ImVec2(spec_w, spec_h));
                if (ImGui::IsItemHovered()) SetFLHint("Mini Analisador de Espectro FFT de Audio em Tempo Real");
                ImGui::SameLine(0, 6);

                // Monitor de CPU e Memoria RAM
                ImVec2 perf_p = ImGui::GetCursorScreenPos();
                float perf_w = 80.0f;
                float perf_h = 24.0f;
                wdl->AddRectFilled(perf_p, ImVec2(perf_p.x + perf_w, perf_p.y + perf_h), IM_COL32(10, 13, 18, 255), 3.0f);
                wdl->AddRect(perf_p, ImVec2(perf_p.x + perf_w, perf_p.y + perf_h), IM_COL32(28, 38, 52, 255), 3.0f);
                
                wdl->AddText(ImVec2(perf_p.x + 4, perf_p.y + 1), IM_COL32(170, 195, 220, 255), "CPU: 4%");
                wdl->AddText(ImVec2(perf_p.x + 4, perf_p.y + 11), IM_COL32(170, 195, 220, 255), "MEM: 240MB");

                ImGui::Dummy(ImVec2(perf_w, perf_h));
                if (ImGui::IsItemHovered()) SetFLHint("Monitor de Hardware: CPU 4% | RAM 240 MB");

                // Controles de Janela do Windows no extremo direito
                float right_margin = viewport->Size.x - ImGui::GetCursorPosX() - 95.0f;
                if (right_margin > 0) ImGui::SameLine(0, right_margin);
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                if (ImGui::Button("_##FLWinMin", ImVec2(26, 20))) {
                    GLFWwindow* win = glfwGetCurrentContext();
                    if (win) glfwIconifyWindow(win);
                }
                if (ImGui::IsItemHovered()) SetFLHint("Minimizar Abduction Studio");
                ImGui::SameLine(0, 2);

                if (ImGui::Button("[ ]##FLWinMax", ImVec2(26, 20))) {
                    GLFWwindow* win = glfwGetCurrentContext();
                    if (win) {
                        if (glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) glfwRestoreWindow(win);
                        else glfwMaximizeWindow(win);
                    }
                }
                if (ImGui::IsItemHovered()) SetFLHint("Maximizar / Restaurar Janela");
                ImGui::SameLine(0, 2);

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
                if (ImGui::Button("X##FLWinClose", ImVec2(26, 20))) {
                    GLFWwindow* win = glfwGetCurrentContext();
                    if (win) glfwSetWindowShouldClose(win, GLFW_TRUE);
                }
                if (ImGui::IsItemHovered()) SetFLHint("Fechar Abduction Studio");
                ImGui::PopStyleColor(2);
            }
            ImGui::EndGroup();

            // =========================================================================
            // 3. DECK 2: LINHA INFERIOR (OLED HINT DISPLAY, PITCH, LASER SCRUBBER, SNAP, PATTERN, TOUCH-PADS)
            // =========================================================================
            ImGui::SetCursorPos(ImVec2(96.0f, 35.0f));
            ImGui::BeginGroup();
            {
                // 1. OLED Hint Display (Caixa rebaixada escura com texto ciano)
                ImVec2 hint_pos = ImGui::GetCursorScreenPos();
                float hint_w = 175.0f;
                float hint_h = 27.0f;
                
                wdl->AddRectFilled(hint_pos, ImVec2(hint_pos.x + hint_w, hint_pos.y + hint_h), IM_COL32(12, 16, 22, 255), 4.0f);
                wdl->AddRect(hint_pos, ImVec2(hint_pos.x + hint_w, hint_pos.y + hint_h), IM_COL32(32, 44, 58, 255), 4.0f);
                wdl->AddText(ImVec2(hint_pos.x + 8, hint_pos.y + 6), IM_COL32(0, 229, 255, 255), g_fl_hint_text.c_str());

                ImGui::Dummy(ImVec2(hint_w, hint_h));
                ImGui::SameLine(0, 8);

                // 2. Master Pitch Knob
                ImVec2 mpk_p = ImGui::GetCursorScreenPos();
                float mpk_r = 10.0f;
                ImVec2 mpk_c = ImVec2(mpk_p.x + mpk_r + 2, mpk_p.y + mpk_r + 3);
                
                wdl->AddCircleFilled(mpk_c, mpk_r + 2.0f, IM_COL32(14, 16, 20, 255), 18);
                wdl->AddCircleFilled(mpk_c, mpk_r, IM_COL32(48, 56, 68, 255), 16);
                
                float p_angle = 4.712f + (g_master_pitch_semitones / 12.0f) * 1.8f;
                wdl->AddLine(mpk_c, ImVec2(mpk_c.x + cosf(p_angle) * (mpk_r - 2.0f), mpk_c.y + sinf(p_angle) * (mpk_r - 2.0f)), IM_COL32(255, 170, 40, 255), 1.6f);

                ImGui::InvisibleButton("##mpk_btn", ImVec2(mpk_r * 2 + 4, mpk_r * 2 + 4));
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    g_master_pitch_semitones -= ImGui::GetIO().MouseDelta.y * 0.1f;
                    g_master_pitch_semitones = std::clamp(g_master_pitch_semitones, -12.0f, 12.0f);
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    g_master_pitch_semitones = 0.0f;
                }
                if (ImGui::IsItemHovered()) {
                    char p_buf[64];
                    snprintf(p_buf, sizeof(p_buf), "Master Pitch: %+.2f Semitons (Duplo-clique para zerar)", g_master_pitch_semitones);
                    SetFLHint(p_buf);
                }
                ImGui::SameLine(0, 8);

                // 3. Laser Scrubber (Trilho Duplo Ciano com Indicador de Posição)
                ImVec2 scr_pos = ImGui::GetCursorScreenPos();
                float scr_w = 115.0f;
                float scr_h = 27.0f;
                wdl->AddRectFilled(scr_pos, ImVec2(scr_pos.x + scr_w, scr_pos.y + scr_h), IM_COL32(12, 16, 22, 255), 3.0f);
                wdl->AddRect(scr_pos, ImVec2(scr_pos.x + scr_w, scr_pos.y + scr_h), IM_COL32(30, 40, 52, 255), 3.0f);
                
                // Trilho laser ciano
                float track_y = scr_pos.y + scr_h * 0.5f;
                wdl->AddLine(ImVec2(scr_pos.x + 6, track_y - 2), ImVec2(scr_pos.x + scr_w - 6, track_y - 2), IM_COL32(0, 180, 220, 180), 1.5f);
                wdl->AddLine(ImVec2(scr_pos.x + 6, track_y + 2), ImVec2(scr_pos.x + scr_w - 6, track_y + 2), IM_COL32(0, 180, 220, 80), 1.0f);
                
                float progress_fraction = (float)(::timeline.getMasterFrame() % (44100 * 32)) / (float)(44100 * 32);
                float handle_x = scr_pos.x + 6.0f + progress_fraction * (scr_w - 12.0f);
                // Cursor laser vertical com brilho
                wdl->AddRectFilled(ImVec2(handle_x - 1.5f, track_y - 7.0f), ImVec2(handle_x + 1.5f, track_y + 7.0f), IM_COL32(0, 229, 255, 255), 1.0f);
                wdl->AddCircleFilled(ImVec2(handle_x, track_y), 2.5f, IM_COL32(255, 255, 255, 255));

                ImGui::InvisibleButton("##SongScrubber", ImVec2(scr_w, scr_h));
                if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    float click_x = ImGui::GetIO().MousePos.x;
                    float norm = std::clamp((click_x - (scr_pos.x + 6.0f)) / (scr_w - 12.0f), 0.0f, 1.0f);
                    ::timeline.setMasterFrame((unsigned long long)(norm * 44100.0 * 32.0));
                }
                if (ImGui::IsItemHovered()) SetFLHint("Laser Scrubber: Navegacao da Timeline (Clique ou arraste)");
                ImGui::SameLine(0, 8);

                // 4. Seletor de Snap (SNAP: 1/16)
                const char* snap_names[] = { "Line", "Cell", "1/4 Beat", "1/2 Beat", "1/16 Beat", "None" };
                static int cur_snap_idx = 4; // Padrão 1/16 Beat
                
                ImGui::SetNextItemWidth(78);
                if (ImGui::BeginCombo("##QuantumSnapCombo", snap_names[cur_snap_idx])) {
                    for (int s = 0; s < 6; s++) {
                        if (ImGui::Selectable(snap_names[s], cur_snap_idx == s)) {
                            cur_snap_idx = s;
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::IsItemHovered()) SetFLHint("Resolucao do Snap / Grade de Alinhamento");
                ImGui::SameLine(0, 6);

                // 5. Seletor de Pattern (◄ Pattern 1 ► [+])
                int total_patterns = (int)g_clip_manager.global_patterns.size();
                int cur_pat = g_clip_manager.current_pattern_idx;
                
                if (ImGui::Button("◄##PatPrev", ImVec2(18, 25))) {
                    if (g_clip_manager.current_pattern_idx > 0) g_clip_manager.current_pattern_idx--;
                }
                if (ImGui::IsItemHovered()) SetFLHint("Padrao Anterior");
                ImGui::SameLine(0, 2);

                char pat_lbl[32];
                snprintf(pat_lbl, sizeof(pat_lbl), "Pattern %d", cur_pat + 1);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.22f, 1.0f));
                ImGui::Button(pat_lbl, ImVec2(74, 25));
                if (ImGui::IsItemHovered()) SetFLHint("Padrao Ativo");
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 2);

                if (ImGui::Button("►##PatNext", ImVec2(18, 25))) {
                    if (g_clip_manager.current_pattern_idx < total_patterns - 1) g_clip_manager.current_pattern_idx++;
                }
                if (ImGui::IsItemHovered()) SetFLHint("Proximo Padrao");
                ImGui::SameLine(0, 2);

                if (ImGui::Button("+##PatAdd", ImVec2(20, 25))) {
                    Pattern p;
                    p.id = (int)g_clip_manager.global_patterns.size() + 1;
                    p.name = "Pattern " + std::to_string(p.id);
                    p.color = 0xFF00E5FF;
                    g_clip_manager.global_patterns.push_back(p);
                    g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                }
                if (ImGui::IsItemHovered()) SetFLHint("Criar Novo Padrao (Pattern)");
                ImGui::SameLine(0, 8);
                ImGui::TextDisabled("|");
                ImGui::SameLine(0, 8);

                // 6. OS 8 PADS DE MÓDULO DO CONCEITO 2 (BACKLIT TOUCH-PADS)
                if (RenderQuantumPad(wdl, "##qpad_playlist", "Playlist", SciFiHUD::IconType::PLAYLIST_TRACKS, show_playlist, "Playlist / Arranjo da Musica (F5)")) {
                    show_playlist = !show_playlist;
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_piano", "Piano", SciFiHUD::IconType::PIANO_KEYS, show_piano_roll, "Piano Roll / Editor de Melodias (F7)")) {
                    show_piano_roll = !show_piano_roll;
                    if (show_piano_roll) g_need_focus_piano_roll = true;
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_rack", "Rack", SciFiHUD::IconType::CHANNEL_RACK, show_step_sequencer, "Channel Rack / Matrix 16-Passos (F6)")) {
                    show_step_sequencer = !show_step_sequencer;
                    if (show_step_sequencer) g_need_focus_step_sequencer = true;
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_mixer", "Mixer", SciFiHUD::IconType::MIXER_FADERS, show_mixer, "Console Mixer & Canais de FX (F9)")) {
                    show_mixer = !show_mixer;
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_browser", "Browser", SciFiHUD::IconType::BROWSER_FOLDER, show_browser, "Browser de Samples e Pastas (F8)")) {
                    show_browser = !show_browser;
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_app_hub", "Hub", SciFiHUD::IconType::APP_DRAWER_HUB, g_show_app_drawer, "Central de Aplicativos estilo Android Hub (F12)")) {
                    g_show_app_drawer = !g_show_app_drawer;
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_ai_stems", "AI Stems", SciFiHUD::IconType::AI_NEURAL_STEMS, g_cloud_stem_ui.getOpenState(), "Separador Neural de Stems por IA (F10)")) {
                    g_cloud_stem_ui.toggle();
                }
                ImGui::SameLine(0, 4);

                if (RenderQuantumPad(wdl, "##qpad_psy_bass", "Psy Bass", SciFiHUD::IconType::CHANNEL_RACK, g_psy_rolling_bass_ui.is_open, "Kuro Psytrance Rolling Bass Engine")) {
                    g_psy_rolling_bass_ui.is_open = !g_psy_rolling_bass_ui.is_open;
                    if (g_psy_rolling_bass_ui.is_open) g_psy_rolling_bass_ui.need_focus = true;
                }
            }
            ImGui::EndGroup();
        }
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }
'''

path = 'src/ui/StudioUI.h'
with open(path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

start_marker = '    // =========================================================================\n    // FASE 1: CONCEITO 2'
if content.find(start_marker) == -1:
    start_marker = '    static void RenderFLStyleTopbar(StemSeparationEngine& ai_engine) {'
end_marker = '    inline void RenderModulationOverlay(float active_val, float min_val, float max_val) {'

start_idx = content.find(start_marker)
end_idx = content.find(end_marker)

if start_idx != -1 and end_idx != -1:
    content = content[:start_idx] + new_code + '\n\n' + content[end_idx:]
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Concept 2 polished successfully!')
else:
    print(f'Markers not found! start: {start_idx}, end: {end_idx}')
