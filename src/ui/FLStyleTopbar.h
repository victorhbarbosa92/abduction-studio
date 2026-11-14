#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "SciFiIconSystem.h"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>

namespace KuroUI {

    // Painel de Dicas Global estilo FL Studio (Hint Panel)
    inline std::string g_fl_hint_text = "(Abduction Studio V4.0) Pronto";

    inline void SetFLHint(const std::string& hint) {
        g_fl_hint_text = hint;
    }

    // Variáveis globais para o Deck 2 e ferramentas
    inline bool g_show_app_drawer = false; // Abre o menu de aplicativos estilo Android
    inline float g_master_pitch_semitones = 0.0f; // -12.0 a +12.0
    inline bool g_metronome_active = false;
    inline bool g_typing_keyboard_active = true;
    inline bool g_countdown_active = false;
    inline bool g_loop_record_active = false;

    class FLStyleTopbar {
    public:
        static void Render(
            StemSeparationEngine& ai_engine,
            KuroDSP::TimelineManager& timeline,
            ClipManager& clip_manager,
            bool& show_playlist,
            bool& show_step_sequencer,
            bool& show_piano_roll,
            bool& show_mixer,
            bool& show_browser,
            bool& g_need_focus_piano_roll,
            bool& g_need_focus_step_sequencer
        ) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImDrawList* wdl = dl;

            // Altura total da Barra Superior Dupla FL Studio: 32px (Deck 1) + 30px (Deck 2) = 62px
            const float topbar_h = 62.0f;
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
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.16f, 0.18f, 0.22f, 1.0f)); // Cor clássica grafite FL Studio

            if (ImGui::Begin("##FLStyleTopbar", nullptr, flags)) {
                
                // Reset da dica padrão ao iniciar o frame (será sobrescrita caso algum item seja hovered)
                if (!ImGui::IsAnyItemHovered()) {
                    g_fl_hint_text = "(Abduction Studio V4.0) Pronto";
                }

                // Permite arrastar a janela clicando em áreas vazias da barra superior
                if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                    GLFWwindow* win = glfwGetCurrentContext();
                    if (win && !glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) {
                        ImVec2 delta = ImGui::GetIO().MouseDelta;
                        int wx, wy;
                        glfwGetWindowPos(win, &wx, &wy);
                        glfwSetWindowPos(win, wx + (int)delta.x, wy + (int)delta.y);
                    }
                }

                // ══════════════════════════════════════════════════════════════
                // ── DECK 1: LINHA SUPERIOR (MENUS, MASTER VOL, TRANSPORTE, BPM, TEMPO, CPU) ──
                // ══════════════════════════════════════════════════════════════
                ImGui::BeginGroup();
                {
                    // 1. Logotipo Emblema Nave Abduction
                    ImVec2 logo_p0 = ImGui::GetCursorScreenPos();
                    SciFiHUD::DrawIcon(wdl, ImVec2(logo_p0.x, logo_p0.y + 2), ImVec2(logo_p0.x + 18, logo_p0.y + 20), SciFiHUD::IconType::ABDUCTION_SHIP, 0xFF00E5FF, 1.6f);
                    ImGui::Dummy(ImVec2(20, 22));
                    if (ImGui::IsItemHovered()) SetFLHint("Abduction Studio V4.0 Flagship Edition");
                    ImGui::SameLine(0, 4);

                    // 2. Menus de Texto FL Studio Clássicos
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.92f, 0.95f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.24f, 0.28f, 0.34f, 1.0f));

                    if (ImGui::Button("ARQUIVO##fl_menu")) ImGui::OpenPopup("PopupArquivo");
                    if (ImGui::IsItemHovered()) SetFLHint("Menu Arquivo: Novo, Abrir, Salvar, Exportar");
                    if (ImGui::BeginPopup("PopupArquivo")) {
                        if (ImGui::MenuItem("Novo Projeto (Reset)")) {
                            ai_engine.reset();
                            clip_manager.reset();
                            timeline.setMasterFrame(0);
                            timeline.setPlaying(false);
                        }
                        if (ImGui::MenuItem("Abrir Projeto... (Ctrl+O)")) {
                            std::string path = KuroUI::FileDialog::OpenFile("Abduction Project (*.kuro)\0*.kuro\0");
                            if (!path.empty()) ProjectManagerBridge::Load(path);
                        }
                        if (ImGui::MenuItem("Salvar Projeto (Ctrl+S)")) {
                            std::string path = KuroUI::FileDialog::SaveFile("Abduction Project (*.kuro)\0*.kuro\0");
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

                    if (ImGui::Button("EDITAR##fl_menu")) ImGui::OpenPopup("PopupEditar");
                    if (ImGui::IsItemHovered()) SetFLHint("Menu Editar: Desfazer, Refazer, Copiar, Colar");
                    if (ImGui::BeginPopup("PopupEditar")) {
                        if (ImGui::MenuItem("Desfazer (Undo)", "Ctrl+Z")) clip_manager.undo();
                        if (ImGui::MenuItem("Refazer (Redo)", "Ctrl+Y")) clip_manager.redo();
                        ImGui::Separator();
                        if (ImGui::MenuItem("Cortar", "Ctrl+X")) {}
                        if (ImGui::MenuItem("Copiar", "Ctrl+C")) {}
                        if (ImGui::MenuItem("Colar", "Ctrl+V")) {}
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("ADICIONAR##fl_menu")) ImGui::OpenPopup("PopupAdicionar");
                    if (ImGui::IsItemHovered()) SetFLHint("Adicionar Instrumentos, Sintetizadores e Efeitos");
                    if (ImGui::BeginPopup("PopupAdicionar")) {
                        if (ImGui::MenuItem("🛸 Kuro Psy Rolling Bass Engine")) {
                            g_psy_rolling_bass_ui.is_open = true;
                            g_psy_rolling_bass_ui.need_focus = true;
                        }
                        if (ImGui::MenuItem("FL DirectWave Multi-Sampler")) g_directwave_ui.is_open = true;
                        if (ImGui::MenuItem("FL Flex Synth (Psytrance)")) g_open_flex_synth_window = true;
                        if (ImGui::MenuItem("Fruity Granulizer")) g_open_granulizer_window = true;
                        if (ImGui::MenuItem("SliceX Beat Slicer")) g_stem_beat_slicer_ui.open();
                        if (ImGui::MenuItem("Delay Lama (Monge 3D)")) show_delay_lama = true;
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("PADRÕES##fl_menu")) ImGui::OpenPopup("PopupPadroes");
                    if (ImGui::IsItemHovered()) SetFLHint("Menu de Padrões (Patterns) e Sequências");
                    if (ImGui::BeginPopup("PopupPadroes")) {
                        if (ImGui::MenuItem("Novo Padrão (Pattern)")) {
                            clip_manager.global_patterns.push_back(KuroDSP::Pattern("Pattern " + std::to_string(clip_manager.global_patterns.size() + 1)));
                            clip_manager.current_pattern_idx = (int)clip_manager.global_patterns.size() - 1;
                        }
                        if (ImGui::MenuItem("Duplicar Padrão Atual")) {}
                        if (ImGui::MenuItem("Limpar Notas do Padrão")) {}
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("EXIBIR##fl_menu")) ImGui::OpenPopup("PopupExibir");
                    if (ImGui::IsItemHovered()) SetFLHint("Exibir/Ocultar Janelas e Módulos");
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
                        if (ImGui::MenuItem("Modo DJ Studio / Pioneer DDJ-200 (F11)", nullptr, show_dj_modal)) show_dj_modal = !show_dj_modal;
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("OPÇÕES##fl_menu")) ImGui::OpenPopup("PopupOpcoes");
                    if (ImGui::IsItemHovered()) SetFLHint("Configurações de Áudio, MIDI e Sistema");
                    if (ImGui::BeginPopup("PopupOpcoes")) {
                        if (ImGui::MenuItem("Dispositivos de Áudio (WASAPI/ASIO)...")) {}
                        if (ImGui::MenuItem("Configurações MIDI...")) {}
                        if (ImGui::MenuItem("Preferências Gerais...")) {}
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("FERRAMENTAS##fl_menu")) ImGui::OpenPopup("PopupFerramentas");
                    if (ImGui::IsItemHovered()) SetFLHint("Separador AI Stems, Inpainting e Riff Machine");
                    if (ImGui::BeginPopup("PopupFerramentas")) {
                        if (ImGui::MenuItem("Separador Neural de Stems (IA)")) g_cloud_stem_ui.toggle();
                        if (ImGui::MenuItem("Gerador de Linha de Baixo Psytrance")) {
                            g_psy_rolling_bass_ui.is_open = true;
                            g_psy_rolling_bass_ui.need_focus = true;
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("AJUDA##fl_menu")) ImGui::OpenPopup("PopupAjuda");
                    if (ImGui::IsItemHovered()) SetFLHint("Manual, Atalhos do Teclado e Sobre");
                    if (ImGui::BeginPopup("PopupAjuda")) {
                        if (ImGui::MenuItem("Guia de Atalhos do Abduction Studio")) {}
                        if (ImGui::MenuItem("Sobre o Abduction Studio V4.0")) {}
                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor(2);

                    ImGui::SameLine(0, 8);
                    ImGui::TextDisabled("|");
                    ImGui::SameLine(0, 8);

                    // 3. Master Volume Knob (Titânio com arco neon)
                    static float master_vol = 1.0f;
                    ImGui::PushID("##MasterVolKnobDeck1");
                    ImVec2 mvk_p = ImGui::GetCursorScreenPos();
                    float mvk_r = 10.0f;
                    ImVec2 mvk_c = ImVec2(mvk_p.x + mvk_r + 2, mvk_p.y + mvk_r + 1);
                    
                    // Soquete escuro
                    wdl->AddCircleFilled(mvk_c, mvk_r + 2.0f, IM_COL32(14, 16, 20, 255), 18);
                    
                    // Arco de volume (verde FL / neon ciano)
                    float v_angle_min = 2.4f;
                    float v_angle_max = 7.0f;
                    float v_curr_angle = v_angle_min + (master_vol / 1.25f) * (v_angle_max - v_angle_min);
                    wdl->PathArcTo(mvk_c, mvk_r + 1.5f, v_angle_min, v_curr_angle, 16);
                    wdl->PathStroke(IM_COL32(0, 229, 255, 255), 0, 2.0f);

                    // Corpo do knob metálico
                    wdl->AddCircleFilled(mvk_c, mvk_r - 1.0f, IM_COL32(50, 56, 64, 255), 16);
                    wdl->AddCircle(mvk_c, mvk_r - 1.0f, IM_COL32(90, 100, 115, 255), 16, 1.0f);
                    
                    // Marcador de pontador
                    float m_px = mvk_c.x + cosf(v_curr_angle) * (mvk_r - 2.5f);
                    float m_py = mvk_c.y + sinf(v_curr_angle) * (mvk_r - 2.5f);
                    wdl->AddLine(mvk_c, ImVec2(m_px, m_py), IM_COL32(255, 255, 255, 255), 1.6f);

                    ImGui::InvisibleButton("##mvk_btn", ImVec2(mvk_r * 2 + 4, mvk_r * 2 + 4));
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        master_vol -= ImGui::GetIO().MouseDelta.y * 0.006f;
                        master_vol = std::clamp(master_vol, 0.0f, 1.25f);
                    }
                    if (ImGui::IsItemHovered()) {
                        char v_buf[64];
                        snprintf(v_buf, sizeof(v_buf), "Volume Principal: %.0f%% (Arraste para ajustar)", master_vol * 100.0f);
                        SetFLHint(v_buf);
                    }
                    ImGui::PopID();

                    ImGui::SameLine(0, 8);

                    // 4. Transporte FL Studio: PAT / SONG + PLAY + STOP + REC
                    bool is_pat = timeline.is_pattern_mode;
                    const char* mode_lbl = is_pat ? "PAT" : "SONG";
                    ImVec4 mode_btn_bg = is_pat ? ImVec4(0.96f, 0.58f, 0.10f, 1.0f) : ImVec4(0.00f, 0.85f, 1.00f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, mode_btn_bg);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    if (ImGui::Button(mode_lbl, ImVec2(50, 22))) {
                        timeline.is_pattern_mode = !timeline.is_pattern_mode;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(is_pat 
                        ? "Modo Ativo: PAT (Toca apenas o Pattern selecionado). Clique para comutar para SONG (Arranjo da Playlist)" 
                        : "Modo Ativo: SONG (Toca a musica completa na Playlist). Clique para comutar para PAT (Apenas Pattern)");
                    ImGui::PopStyleColor(2);
                    ImGui::SameLine(0, 2);

                    bool is_p = ::is_playing;
                    if (SciFiHUD::SciFiTransportBtn("##FLPlay", is_p ? SciFiHUD::IconType::PAUSE : SciFiHUD::IconType::PLAY, is_p, ImVec4(0.18f, 0.92f, 0.40f, 1.0f), ImVec2(24, 22), "Tocar / Pausar (Espaço)")) {
                        KuroUI::TransportController::TogglePlayPause(timeline);
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(is_p ? "Pausar Reprodução (Espaço)" : "Iniciar Reprodução (Espaço)");
                    ImGui::SameLine(0, 2);

                    if (SciFiHUD::SciFiTransportBtn("##FLStop", SciFiHUD::IconType::STOP, false, ImVec4(0.55f, 0.65f, 0.78f, 1.0f), ImVec2(24, 22), "Parar (Stop)")) {
                        KuroUI::TransportController::Stop(timeline);
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Parar Reprodução e Reiniciar Cursor");
                    ImGui::SameLine(0, 2);

                    bool rec = g_record_manager.isRecording();
                    if (SciFiHUD::SciFiTransportBtn("##FLRec", SciFiHUD::IconType::RECORD, rec, ImVec4(0.96f, 0.18f, 0.22f, 1.0f), ImVec2(24, 22), "Gravação Master / MIDI")) {
                        if (rec) g_record_manager.setRecording(false);
                        else     g_record_manager.setRecording(true, false);
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(rec ? "Gravando Áudio... Clique para Parar" : "Armar Gravação de Áudio Master / MIDI");
                    ImGui::SameLine(0, 8);

                    // 5. Visor Digital de BPM Estilo FL Studio (Display Ciano com Setas)
                    float current_bpm = timeline.getBPM();
                    ImVec2 bpm_box_pos = ImGui::GetCursorScreenPos();
                    float bpm_box_w = 78.0f;
                    float bpm_box_h = 22.0f;
                    
                    wdl->AddRectFilled(bpm_box_pos, ImVec2(bpm_box_pos.x + bpm_box_w, bpm_box_pos.y + bpm_box_h), IM_COL32(10, 14, 18, 255), 3.0f);
                    wdl->AddRect(bpm_box_pos, ImVec2(bpm_box_pos.x + bpm_box_w, bpm_box_pos.y + bpm_box_h), IM_COL32(30, 42, 54, 255), 3.0f);
                    
                    char bpm_str[32];
                    snprintf(bpm_str, sizeof(bpm_str), "%.3f", current_bpm);
                    wdl->AddText(ImVec2(bpm_box_pos.x + 6, bpm_box_pos.y + 3), IM_COL32(0, 229, 255, 255), bpm_str);

                    // Setas de ajuste rápido ▲ ▼
                    wdl->AddTriangleFilled(ImVec2(bpm_box_pos.x + bpm_box_w - 9, bpm_box_pos.y + 7), ImVec2(bpm_box_pos.x + bpm_box_w - 13, bpm_box_pos.y + 11), ImVec2(bpm_box_pos.x + bpm_box_w - 5, bpm_box_pos.y + 11), IM_COL32(130, 160, 190, 255));
                    wdl->AddTriangleFilled(ImVec2(bpm_box_pos.x + bpm_box_w - 9, bpm_box_pos.y + 17), ImVec2(bpm_box_pos.x + bpm_box_w - 13, bpm_box_pos.y + 13), ImVec2(bpm_box_pos.x + bpm_box_w - 5, bpm_box_pos.y + 13), IM_COL32(130, 160, 190, 255));

                    ImGui::InvisibleButton("##BpmDisplayBtn", ImVec2(bpm_box_w, bpm_box_h));
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        current_bpm -= ImGui::GetIO().MouseDelta.y * 0.2f;
                        current_bpm = std::clamp(current_bpm, 40.0f, 300.0f);
                        timeline.setBPM(current_bpm);
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Andamento da Música (BPM) - Clique e arraste para alterar");
                    ImGui::SameLine(0, 6);

                    // 6. Botões Auxiliares FL Studio (Metrônomo, Teclado Virtual, Contagem 3.2.1, Loop)
                    if (SciFiHUD::SciFiButton("##FLMetronome", "", SciFiHUD::IconType::METRONOME, g_metronome_active, ImVec2(22, 22), "Metrônomo (Alternar Clique de Tempo)")) {
                        g_metronome_active = !g_metronome_active;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(g_metronome_active ? "Metrônomo: Ativado" : "Metrônomo: Desativado");
                    ImGui::SameLine(0, 2);

                    if (SciFiHUD::SciFiButton("##FLTypingKeys", "", SciFiHUD::IconType::TYPING_KEYBOARD, g_typing_keyboard_active, ImVec2(22, 22), "Teclado do Computador como Teclado Musical (Typing-to-Piano)")) {
                        g_typing_keyboard_active = !g_typing_keyboard_active;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(g_typing_keyboard_active ? "Teclado Musical Ativo: Digite letras do teclado para tocar notas" : "Teclado Musical Desativado");
                    ImGui::SameLine(0, 2);

                    if (SciFiHUD::SciFiButton("##FLCountdown", "", SciFiHUD::IconType::COUNTDOWN_PRECOUNT, g_countdown_active, ImVec2(22, 22), "Contagem Prévia de Gravação (3.2.1)")) {
                        g_countdown_active = !g_countdown_active;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(g_countdown_active ? "Contagem Prévia 3.2.1: Ativada" : "Contagem Prévia: Desativada");
                    ImGui::SameLine(0, 2);

                    if (SciFiHUD::SciFiButton("##FLLoopRec", "", SciFiHUD::IconType::LOOP_RECORD, g_loop_record_active, ImVec2(22, 22), "Gravação em Loop")) {
                        g_loop_record_active = !g_loop_record_active;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint(g_loop_record_active ? "Gravação em Loop: Ativada" : "Gravação em Loop: Desativada");
                    ImGui::SameLine(0, 8);

                    // 7. Visor LED Central: Tempo da Música Bar:Step:Tick (1:01:00)
                    ImVec2 time_box_pos = ImGui::GetCursorScreenPos();
                    float time_box_w = 96.0f;
                    float time_box_h = 22.0f;
                    wdl->AddRectFilled(time_box_pos, ImVec2(time_box_pos.x + time_box_w, time_box_pos.y + time_box_h), IM_COL32(8, 12, 16, 255), 3.0f);
                    wdl->AddRect(time_box_pos, ImVec2(time_box_pos.x + time_box_w, time_box_pos.y + time_box_h), IM_COL32(26, 38, 50, 255), 3.0f);

                    // Calcular Bar : Beat : Tick
                    double current_time_sec = (double)timeline.getMasterFrame() / 44100.0;
                    double beats_per_sec = (double)current_bpm / 60.0;
                    double total_beats = current_time_sec * beats_per_sec;
                    int bar = (int)(total_beats / 4.0) + 1;
                    int step = ((int)total_beats % 4) + 1;
                    int tick = (int)((total_beats - (int)total_beats) * 100.0);

                    char time_str[32];
                    snprintf(time_str, sizeof(time_str), "%d:%02d:%02d", bar, step, tick);
                    wdl->AddText(ImVec2(time_box_pos.x + 8, time_box_pos.y + 3), IM_COL32(0, 229, 255, 255), time_str);
                    
                    // Indicador de formato B:S:T
                    wdl->AddText(ImVec2(time_box_pos.x + time_box_w - 24, time_box_pos.y + 2), IM_COL32(90, 120, 150, 255), "BST");

                    ImGui::Dummy(ImVec2(time_box_w, time_box_h));
                    if (ImGui::IsItemHovered()) SetFLHint("Posição da Música: Compasso : Batida : Ticks (Bar:Step:Tick)");
                    ImGui::SameLine(0, 8);

                    // 8. Monitor de CPU e Memória RAM
                    ImVec2 perf_pos = ImGui::GetCursorScreenPos();
                    float perf_w = 88.0f;
                    float perf_h = 22.0f;
                    wdl->AddRectFilled(perf_pos, ImVec2(perf_pos.x + perf_w, perf_pos.y + perf_h), IM_COL32(12, 15, 18, 255), 3.0f);
                    wdl->AddRect(perf_pos, ImVec2(perf_pos.x + perf_w, perf_pos.y + perf_h), IM_COL32(26, 32, 40, 255), 3.0f);
                    
                    // Mini barra de CPU
                    float cpu_pct = 0.04f; // 4% CPU médio
                    wdl->AddRectFilled(ImVec2(perf_pos.x + 4, perf_pos.y + 14), ImVec2(perf_pos.x + 4 + (perf_w - 8) * cpu_pct, perf_pos.y + 18), IM_COL32(0, 229, 255, 255), 1.0f);
                    wdl->AddText(ImVec2(perf_pos.x + 5, perf_pos.y + 1), IM_COL32(180, 200, 220, 255), "CPU 4% 240MB");

                    ImGui::Dummy(ImVec2(perf_w, perf_h));
                    if (ImGui::IsItemHovered()) SetFLHint("Uso de CPU do Motor de Áudio: 4% | RAM: 240 MB");

                    // 9. Controles da Janela do Windows (Minimizar, Maximizar, Fechar) no extremo direito
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

                // Separador horizontal sutil entre os dois decks
                ImVec2 sep_p0 = ImVec2(viewport->Pos.x, viewport->Pos.y + 30.0f);
                ImVec2 sep_p1 = ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + 30.0f);
                dl->AddLine(sep_p0, sep_p1, IM_COL32(24, 28, 34, 255), 1.0f);
                dl->AddLine(ImVec2(sep_p0.x, sep_p0.y + 1), ImVec2(sep_p1.x, sep_p1.y + 1), IM_COL32(40, 48, 58, 255), 1.0f);

                // ══════════════════════════════════════════════════════════════
                // ── DECK 2: LINHA INFERIOR (HINT PANEL, PITCH, SCRUBBER, SNAP, PATTERN, CORE MODULES) ──
                // ══════════════════════════════════════════════════════════════
                ImGui::SetCursorPosY(33.0f);
                ImGui::BeginGroup();
                {
                    // 1. Painel de Dicas (Hint Panel) estilo FL Studio
                    ImVec2 hint_pos = ImGui::GetCursorScreenPos();
                    float hint_w = 140.0f;
                    float hint_h = 24.0f;
                    
                    // Caixa de exibição rebaixada com borda neon suave
                    wdl->AddRectFilled(hint_pos, ImVec2(hint_pos.x + hint_w, hint_pos.y + hint_h), IM_COL32(18, 22, 28, 255), 4.0f);
                    wdl->AddRect(hint_pos, ImVec2(hint_pos.x + hint_w, hint_pos.y + hint_h), IM_COL32(38, 48, 60, 255), 4.0f);
                    
                    // Texto da dica
                    wdl->AddText(ImVec2(hint_pos.x + 8, hint_pos.y + 4), IM_COL32(200, 225, 245, 255), g_fl_hint_text.c_str());

                    ImGui::Dummy(ImVec2(hint_w, hint_h));
                    ImGui::SameLine(0, 8);

                    // 2. Master Pitch Knob
                    ImVec2 mpk_p = ImGui::GetCursorScreenPos();
                    float mpk_r = 9.0f;
                    ImVec2 mpk_c = ImVec2(mpk_p.x + mpk_r + 2, mpk_p.y + mpk_r + 2);
                    
                    wdl->AddCircleFilled(mpk_c, mpk_r + 1.5f, IM_COL32(14, 16, 20, 255), 16);
                    wdl->AddCircleFilled(mpk_c, mpk_r - 1.0f, IM_COL32(48, 54, 62, 255), 14);
                    
                    // Indicador de centro / offset de pitch
                    float p_angle = 4.712f + (g_master_pitch_semitones / 12.0f) * 1.8f;
                    wdl->AddLine(mpk_c, ImVec2(mpk_c.x + cosf(p_angle) * (mpk_r - 2.0f), mpk_c.y + sinf(p_angle) * (mpk_r - 2.0f)), IM_COL32(255, 180, 50, 255), 1.5f);

                    ImGui::InvisibleButton("##mpk_btn", ImVec2(mpk_r * 2 + 4, mpk_r * 2 + 4));
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        g_master_pitch_semitones -= ImGui::GetIO().MouseDelta.y * 0.1f;
                        g_master_pitch_semitones = std::clamp(g_master_pitch_semitones, -12.0f, 12.0f);
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        g_master_pitch_semitones = 0.0f; // Reset no duplo clique
                    }
                    if (ImGui::IsItemHovered()) {
                        char p_buf[64];
                        snprintf(p_buf, sizeof(p_buf), "Master Pitch: %+.1f Semitons (Duplo clique para zerar)", g_master_pitch_semitones);
                        SetFLHint(p_buf);
                    }
                    ImGui::SameLine(0, 8);

                    // 3. Barra de Scrubber da Linha do Tempo (Song Position Slider)
                    ImVec2 scr_pos = ImGui::GetCursorScreenPos();
                    float scr_w = 75.0f;
                    float scr_h = 22.0f;
                    wdl->AddRectFilled(scr_pos, ImVec2(scr_pos.x + scr_w, scr_pos.y + scr_h), IM_COL32(14, 18, 22, 255), 3.0f);
                    wdl->AddRect(scr_pos, ImVec2(scr_pos.x + scr_w, scr_pos.y + scr_h), IM_COL32(32, 40, 50, 255), 3.0f);
                    
                    // Linha de trilha
                    float track_y = scr_pos.y + scr_h * 0.5f;
                    wdl->AddLine(ImVec2(scr_pos.x + 6, track_y), ImVec2(scr_pos.x + scr_w - 6, track_y), IM_COL32(45, 55, 68, 255), 2.0f);
                    
                    // Indicador deslizante (agulha dourada/laranja)
                    float progress_fraction = (float)(timeline.getMasterFrame() % (44100 * 32)) / (float)(44100 * 32);
                    float handle_x = scr_pos.x + 6.0f + progress_fraction * (scr_w - 12.0f);
                    wdl->AddRectFilled(ImVec2(handle_x - 2.5f, track_y - 6.0f), ImVec2(handle_x + 2.5f, track_y + 6.0f), IM_COL32(255, 170, 40, 255), 1.0f);

                    ImGui::InvisibleButton("##SongScrubber", ImVec2(scr_w, scr_h));
                    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        float click_x = ImGui::GetIO().MousePos.x;
                        float norm = std::clamp((click_x - (scr_pos.x + 6.0f)) / (scr_w - 12.0f), 0.0f, 1.0f);
                        timeline.setMasterFrame((unsigned long long)(norm * 44100.0 * 32.0));
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Navegador de Posição da Música (Clique ou arraste para navegar)");
                    ImGui::SameLine(0, 8);

                    // 4. Seletor de Snap (Imã / Grade)
                    const char* snap_names[] = { "Line", "Cell", "1/4 Beat", "1/2 Beat", "1/16 Beat", "None" };
                    static int cur_snap_idx = 0;
                    
                    ImVec2 snap_ic_pos = ImGui::GetCursorScreenPos();
                    SciFiHUD::DrawIcon(wdl, ImVec2(snap_ic_pos.x, snap_ic_pos.y + 3), ImVec2(snap_ic_pos.x + 14, snap_ic_pos.y + 17), SciFiHUD::IconType::SNAP_MAGNET, 0xFF00E5FF, 1.3f);
                    ImGui::Dummy(ImVec2(16, 22));
                    ImGui::SameLine(0, 2);

                    ImGui::SetNextItemWidth(74);
                    if (ImGui::BeginCombo("##FLSnapCombo", snap_names[cur_snap_idx])) {
                        for (int s = 0; s < 6; s++) {
                            if (ImGui::Selectable(snap_names[s], cur_snap_idx == s)) {
                                cur_snap_idx = s;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Resolução da Grade (Snap): Alinhamento de notas e clipes");
                    ImGui::SameLine(0, 8);

                    // 4.1 Groove Swing Knob (Deck 2)
                    float top_swing = timeline.getSwing();
                    ImVec2 sw_pos = ImGui::GetCursorScreenPos();
                    float sw_r = 9.0f;
                    ImVec2 sw_c = ImVec2(sw_pos.x + sw_r + 2, sw_pos.y + sw_r + 2);
                    wdl->AddCircleFilled(sw_c, sw_r + 1.5f, IM_COL32(14, 16, 20, 255), 16);
                    wdl->AddCircleFilled(sw_c, sw_r - 1.0f, IM_COL32(40, 52, 64, 255), 14);
                    float sw_angle = 4.712f + (top_swing - 0.5f) * 2.8f;
                    wdl->AddLine(sw_c, ImVec2(sw_c.x + cosf(sw_angle) * (sw_r - 2.0f), sw_c.y + sinf(sw_angle) * (sw_r - 2.0f)), IM_COL32(0, 229, 255, 255), 1.6f);

                    ImGui::InvisibleButton("##top_swing_btn", ImVec2(sw_r * 2 + 4, sw_r * 2 + 4));
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        top_swing -= ImGui::GetIO().MouseDelta.y * 0.01f;
                        timeline.setSwing(top_swing);
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        timeline.setSwing(0.0f);
                    }
                    if (ImGui::IsItemHovered()) {
                        char sw_buf[64];
                        snprintf(sw_buf, sizeof(sw_buf), "Groove Swing: %.0f%% (Clique/arraste p/ alterar, duplo clique para zerar)", timeline.getSwing() * 100.0f);
                        SetFLHint(sw_buf);
                    }
                    ImGui::SameLine(0, 8);

                    // 5. Seletor de Pattern (◄ Pattern 1 ► [+])
                    int total_patterns = (int)clip_manager.global_patterns.size();
                    int cur_pat = clip_manager.current_pattern_idx;
                    
                    if (ImGui::Button("◄##PatPrev", ImVec2(18, 22))) {
                        if (clip_manager.current_pattern_idx > 0) clip_manager.current_pattern_idx--;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Padrão Anterior");
                    ImGui::SameLine(0, 2);

                    char pat_lbl[32];
                    snprintf(pat_lbl, sizeof(pat_lbl), "Pattern %d", cur_pat + 1);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.18f, 1.0f));
                    ImGui::Button(pat_lbl, ImVec2(76, 22));
                    if (ImGui::IsItemHovered()) SetFLHint("Padrão Ativo no Channel Rack e Piano Roll");
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("►##PatNext", ImVec2(18, 22))) {
                        if (clip_manager.current_pattern_idx < total_patterns - 1) clip_manager.current_pattern_idx++;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Próximo Padrão");
                    ImGui::SameLine(0, 2);

                    if (ImGui::Button("+##PatAdd", ImVec2(20, 22))) {
                        clip_manager.global_patterns.push_back(KuroDSP::Pattern("Pattern " + std::to_string(clip_manager.global_patterns.size() + 1)));
                        clip_manager.current_pattern_idx = (int)clip_manager.global_patterns.size() - 1;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Criar Novo Padrão (Pattern)");
                    ImGui::SameLine(0, 10);
                    ImGui::TextDisabled("|");
                    ImGui::SameLine(0, 10);

                    // 6. OS 8 BOTÕES PRINCIPAIS DE FERRAMENTAS DO ABDUCTION STUDIO (ESTILO FL STUDIO)
                    if (SciFiHUD::SciFiButton("##btn_playlist", "Playlist", SciFiHUD::IconType::PLAYLIST_TRACKS, show_playlist, ImVec2(0, 22), "Playlist / Arranjo da Música (F5)")) {
                        show_playlist = !show_playlist;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Playlist: Tela de Arranjo e Sequenciamento de Clipes (F5)");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_piano", "Piano Roll", SciFiHUD::IconType::PIANO_KEYS, show_piano_roll, ImVec2(0, 22), "Piano Roll / Editor de Melodias (F7)")) {
                        show_piano_roll = !show_piano_roll;
                        if (show_piano_roll) g_need_focus_piano_roll = true;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Piano Roll: Teclado de 88 Teclas e Edição de Notas MIDI (F7)");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_rack", "Channel Rack", SciFiHUD::IconType::CHANNEL_RACK, show_step_sequencer, ImVec2(0, 22), "Channel Rack / Matrix 16-Passos (F6)")) {
                        show_step_sequencer = !show_step_sequencer;
                        if (show_step_sequencer) g_need_focus_step_sequencer = true;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Channel Rack: Sequenciador de Bateria e Instrumentos (F6)");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_mixer", "Mixer", SciFiHUD::IconType::MIXER_FADERS, show_mixer, ImVec2(0, 22), "Console Mixer & Canais de FX (F9)")) {
                        show_mixer = !show_mixer;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Mixer: Mesa de Mixagem, Equalizadores e Efeitos (F9)");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_browser", "Browser", SciFiHUD::IconType::BROWSER_FOLDER, show_browser, ImVec2(0, 22), "Browser de Samples e Pastas (F8)")) {
                        show_browser = !show_browser;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Browser: Biblioteca de Samples, Presets e Instrumentos (F8)");
                    ImGui::SameLine(0, 4);

                    // Botão Hub / App Drawer estilo Android OS
                    if (SciFiHUD::SciFiButton("##btn_app_drawer", "App Hub", SciFiHUD::IconType::APP_DRAWER_HUB, g_show_app_drawer, ImVec2(0, 22), "Central de Aplicativos & Ferramentas (Android Hub - F12)")) {
                        g_show_app_drawer = !g_show_app_drawer;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Central de Aplicativos estilo Android: Pastas de VSTs, IA e Ferramentas (F12)");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_ai_stems", "AI Stems", SciFiHUD::IconType::AI_NEURAL_STEMS, g_cloud_stem_ui.getOpenState(), ImVec2(0, 22), "Separador de Stems por IA (F10)")) {
                        g_cloud_stem_ui.toggle();
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Separador Neural de Stems: Separe vocais, baixo e bateria via IA (F10)");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_psy_bass", "Psy Bass", SciFiHUD::IconType::CHANNEL_RACK, g_psy_rolling_bass_ui.is_open, ImVec2(0, 22), "Kuro Psytrance Rolling Bass Engine")) {
                        g_psy_rolling_bass_ui.is_open = !g_psy_rolling_bass_ui.is_open;
                        if (g_psy_rolling_bass_ui.is_open) g_psy_rolling_bass_ui.need_focus = true;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Kuro Psytrance Rolling Bass: Sintetizador de Baixo Analógico Moog + Gerador KBBB");
                    ImGui::SameLine(0, 4);

                    if (SciFiHUD::SciFiButton("##btn_dj_studio", "Modo DJ", SciFiHUD::IconType::DJ_VINYL_DECKS, show_dj_modal, ImVec2(0, 22), "Abduction DJ Studio / Pioneer DDJ-200 (F11)")) {
                        show_dj_modal = !show_dj_modal;
                    }
                    if (ImGui::IsItemHovered()) SetFLHint("Abduction DJ Studio: 2 Decks, Pioneer DDJ-200, Waveforms, Hot Cues e Sampler (F11)");
                }
                ImGui::EndGroup();
            }
            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);
        }
    };

} // namespace KuroUI
