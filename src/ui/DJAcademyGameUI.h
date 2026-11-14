#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "DJLayoutMetrics.h"
#include "DJLayoutEditorUI.h"
#include "UIScaleManager.h"
#include "../core/DJTransitionLoopRepeater.h"
#include "../core/DJTurntablismEngine.h"
#include "../core/DJGainStagingAnalyzer.h"
#include "../core/DJLiveInstrumentEngine.h"
#include "../core/DJEngine.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <filesystem>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroUI {

    // =========================================================================
    // ESTRUTURA DIDÁTICA DE UMA FASE DO TUTORIAL EM TEMPO REAL
    // Baseada no estudo minucioso da faixa: Aura Vortex & Blazy - Nuke (136.0 BPM)
    // =========================================================================
    struct TutorialPhaseInfo {
        int id;                     // 0 a 4 (Fases 1 a 5)
        const char* step_number;    // "1/5", "2/5", etc.
        const char* title;          // "Beatgrid & Alinhamento (Intro)"
        const char* bars_str;       // "Compassos 1-16"
        const char* time_str;       // "00:00 - 00:28"
        double start_sec;
        double end_sec;
        const char* icon;
        ImVec4 theme_color;
        const char* section_type;   // "INTRO", "BUILD-UP", "DROP", etc.
        const char* mission_goal;   // Objetivo da Missão
        const char* why_for_human;  // A Lógica de Uso para o Ser Humano
        const char* action_step;    // Como executar passo a passo
        const char* golden_rule;    // Regra de ouro profissional
    };

    class DJAcademyGameUI {
    private:
        bool m_active = false;
        bool m_show_modal_guide = false; // Guia detalhado de 3 cards em janela modal
        int m_current_phase = 0;           // 0 = Fase 1, 1 = Fase 2, etc.
        bool m_auto_follow_playback = true;// Segue a música conforme ela toca
        bool m_phase_loop_active = false;  // Mantém a fase em loop para treino contínuo
        float m_anim_time = 0.0f;
        
        // Gamificação e telemetria
        float m_tutorial_score = 100.0f;
        int m_tutorial_streak = 1;
        std::string m_last_feedback = "Selecione uma fase ou dê Play para iniciar o tutorial interativo!";
        ImVec4 m_feedback_col = ImVec4(0.0f, 0.90f, 1.0f, 1.0f);
        bool m_tracks_loaded = false;

        // Controle de conclusão dinâmica de tarefas
        float m_completion_banner_timer = 0.0f;
        std::string m_completion_banner_txt = "";
        int m_phase_step_progress = 0;

        // Controle interno do Acid 303 no card do tutorial
        float m_synth_cutoff_val = 0.30f;
        float m_synth_reso_val = 0.60f;

        DJAcademyGameUI() = default;

    public:
        // =========================================================================
        // COMPONENTE: KNOB ROTATIVO ANALÓGICO COM VALOR DIGITAL E ARCO DE ILUMINAÇÃO
        // =========================================================================
        static bool DrawRotaryKnob(const char* label, float* p_value, float v_min, float v_max, float radius, ImVec4 theme_col, const char* unit_fmt = "%.0f%%", float default_val = 0.5f, float display_mult = 100.0f) {
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            std::string disp_label = label;
            size_t hash_pos = disp_label.find("##");
            if (hash_pos != std::string::npos) disp_label = disp_label.substr(0, hash_pos);

            ImVec2 text_sz = ImGui::CalcTextSize(disp_label.c_str());

            float gap_label_knob = 5.0f;
            float arc_extra = 3.5f;
            float knob_diam = (radius + arc_extra) * 2.0f;
            float gap_knob_val = 5.0f;
            float val_h = ImGui::GetTextLineHeight();

            float total_w = std::max(knob_diam + 16.0f, text_sz.x + 8.0f);
            float total_h = text_sz.y + gap_label_knob + knob_diam + gap_knob_val + val_h;

            ImGui::InvisibleButton(label, ImVec2(total_w, total_h));
            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();

            if (is_active) {
                float delta = -io.MouseDelta.y + io.MouseDelta.x * 0.4f;
                if (delta != 0.0f) {
                    float step = (v_max - v_min) * 0.005f;
                    *p_value += delta * step;
                    *p_value = std::clamp(*p_value, v_min, v_max);
                }
            }
            if (is_hovered && io.MouseWheel != 0.0f) {
                float step = (v_max - v_min) * 0.025f;
                *p_value += io.MouseWheel * step;
                *p_value = std::clamp(*p_value, v_min, v_max);
            }
            if (is_hovered && ImGui::IsMouseDoubleClicked(0)) {
                *p_value = default_val;
            }

            ImVec2 label_pos = ImVec2(pos.x + (total_w - text_sz.x) * 0.5f, pos.y);
            dl->AddText(label_pos, is_hovered ? ImGui::GetColorU32(theme_col) : IM_COL32(185, 200, 225, 230), disp_label.c_str());

            ImVec2 center = ImVec2(pos.x + total_w * 0.5f, pos.y + text_sz.y + gap_label_knob + radius + arc_extra);

            float angle_min = 0.75f * (float)M_PI;
            float angle_max = 2.25f * (float)M_PI;
            float norm = (*p_value - v_min) / (v_max - v_min);
            norm = std::clamp(norm, 0.0f, 1.0f);
            float angle_val = angle_min + norm * (angle_max - angle_min);

            dl->PathArcTo(center, radius + 2.5f, angle_min, angle_max, 24);
            dl->PathStroke(IM_COL32(26, 35, 50, 255), 0, 3.0f);

            dl->PathArcTo(center, radius + 2.5f, angle_min, angle_val, 24);
            dl->PathStroke(ImGui::GetColorU32(theme_col), 0, 3.0f);

            dl->AddCircleFilled(center, radius, IM_COL32(18, 22, 32, 255), 24);
            dl->AddCircle(center, radius, is_hovered ? ImGui::GetColorU32(theme_col) : IM_COL32(50, 65, 88, 255), 24, 1.2f);
            dl->AddCircleFilled(center, radius * 0.80f, IM_COL32(24, 30, 42, 255), 20);

            ImVec2 pointer_p = ImVec2(center.x + std::cos(angle_val) * (radius * 0.72f), center.y + std::sin(angle_val) * (radius * 0.72f));
            dl->AddLine(center, pointer_p, ImGui::GetColorU32(theme_col), 2.2f);
            dl->AddCircleFilled(pointer_p, 1.8f, IM_COL32(255, 255, 255, 255));

            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), unit_fmt, *p_value * display_mult);
            ImVec2 val_sz = ImGui::CalcTextSize(val_buf);
            ImVec2 val_pos = ImVec2(pos.x + (total_w - val_sz.x) * 0.5f, center.y + radius + arc_extra + gap_knob_val);
            dl->AddText(val_pos, IM_COL32(145, 170, 200, 230), val_buf);

            if (is_hovered || is_active) {
                ImGui::SetTooltip("%s: %s (Arraste ou MouseWheel | Duplo clique reseta)", disp_label.c_str(), val_buf);
            }

            return is_active;
        }

    public:
        static DJAcademyGameUI& getInstance() {
            static DJAcademyGameUI instance;
            return instance;
        }

        bool isActive() const { return m_active; }
        void setActive(bool act) { m_active = act; }
        void toggle() { m_active = !m_active; }

        int getCurrentPhase() const { return m_current_phase; }
        void setPhase(int phase_idx) {
            m_current_phase = std::clamp(phase_idx, 0, 4);
        }

        // =========================================================================
        // CARREGAMENTO DA FAIXA REAL ESTUDADA NOS DECKS (NUKE & SWEET DISPOSITION)
        // =========================================================================
        void loadStudiedTracks(KuroAudio::DJEngine& engine) {
            std::string pA = "scratch/tracks/Aura Vortex & Blazy - Nuke/Aura Vortex & Blazy - Nuke.wav";
            if (!std::filesystem::exists(pA)) pA = "scratch/tracks/Aura Vortex & Blazy - Nuke.wav";
            if (!std::filesystem::exists(pA)) pA = "scratch/tracks/Render & Gigi - Dite The Bust/Render & Gigi - Dite The Bust.wav";
            if (!std::filesystem::exists(pA)) pA = "scratch/test_cliff.wav";

            std::string pB = "scratch/tracks/The Temper Trap - Sweet Disposition (Blazy & Sighter Remix)/The Temper Trap - Sweet Disposition (Blazy & Sighter Remix).wav";
            if (!std::filesystem::exists(pB)) pB = "scratch/tracks/The Temper Trap - Sweet Disposition (Blazy & Sighter Remix).wav";
            if (!std::filesystem::exists(pB)) pB = "scratch/tracks/Groundbass & Zanon - Psychedelic Experience/Groundbass & Zanon - Psychedelic Experience.wav";
            if (!std::filesystem::exists(pB)) pB = "scratch/test_kvsh.wav";

            bool ok_a = false;
            bool ok_b = false;

            if (std::filesystem::exists(pA)) {
                if (engine.deckA.loadTrack(pA)) {
                    engine.deckA.track_title = "Nuke";
                    engine.deckA.track_artist = "Aura Vortex & Blazy";
                    engine.deckA.bpm = 136.0;
                    engine.deckA.key_signature = "10B / D";
                    engine.deckA.rebuildRekordboxBeats();
                    ok_a = true;
                }
            }

            if (std::filesystem::exists(pB)) {
                if (engine.deckB.loadTrack(pB)) {
                    engine.deckB.track_title = "Sweet Disposition (Remix)";
                    engine.deckB.track_artist = "The Temper Trap";
                    engine.deckB.bpm = 136.0;
                    engine.deckB.key_signature = "10A / Bm";
                    engine.deckB.rebuildRekordboxBeats();
                    ok_b = true;
                }
            }

            m_tracks_loaded = (ok_a || ok_b);
            if (m_tracks_loaded) {
                m_last_feedback = "Músicas reais carregadas com sucesso! Faixa de Estudo: Aura Vortex & Blazy - Nuke (136 BPM)";
                m_feedback_col = ImVec4(0.2f, 1.0f, 0.5f, 1.0f);
            } else {
                m_last_feedback = "Aviso: Verifique os arquivos WAV no diretório scratch/tracks/";
                m_feedback_col = ImVec4(1.0f, 0.5f, 0.2f, 1.0f);
            }
        }

        // =========================================================================
        // BANCO DE DADOS DAS 5 FASES ESTUDADAS NA MÚSICA "NUKE" (136.0 BPM)
        // =========================================================================
        static const std::vector<TutorialPhaseInfo>& getPhases() {
            static const std::vector<TutorialPhaseInfo> s_phases = {
                {
                    0, "1/5", "1. Beatgrid & Alinhamento de Intro",
                    "Compassos 1-16", "00:00 - 00:28",
                    0.0, 28.24,
                    ICON_FA_HEADPHONES,
                    ImVec4(0.0f, 0.85f, 1.0f, 1.0f),
                    "INTRO",
                    "Dê PLAY no Deck 1. Observe o pulso dos 4 tempos (1, 2, 3, 4) no metrônomo visual e verifique o primeiro downbeat.",
                    "Na música eletrônica, tudo é construído em múltiplos de 4 tempos (compasso 4/4) e blocos de 16 ou 32 compassos. Um DJ profissional nunca solta uma música no meio do compasso: o 'Tempo 1' é sagrado para manter a pista no ritmo.",
                    "1. Clique em PLAY no Deck 1 | 2. Acompanhe as luzes 1-2-3-4 piscando | 3. Mantenha a velocidade travada em 136 BPM.",
                    "Sempre conte mentalmente: 1-2-3-4, 2-2-3-4, 3-2-3-4, 4-2-3-4. Cada virada acontece a cada 16 ou 32 compassos."
                },
                {
                    1, "2/5", "2. Bass Staging Seguro & Equalização",
                    "Compassos 17-32", "00:28 - 00:56",
                    28.24, 56.47,
                    ICON_FA_SHIELD_HALVED,
                    ImVec4(0.2f, 1.0f, 0.5f, 1.0f),
                    "GROOVE / KICK",
                    "O kick pesado e o rolling bass entraram na música! O Deck 2 está armado com Sweet Disposition. Mantenha o LOW do Deck 2 em 0% e o Deck 1 em 100%!",
                    "Dois kicks pesados tocando juntos em fase somam até +6dB de pico no PA, destruindo os alto-falantes e embolando os graves. A regra de ouro é: enquanto uma música tem grave dominante, a outra deve ter seu LOW cortado a zero.",
                    "1. Observe o medidor de graves no Card 1 | 2. Se o Deck 2 subir o LOW, veja o alerta de BASS CLASH disparar | 3. Use o botão AUTO BASS SWAP no momento da virada.",
                    "NUNCA deixe dois graves abertos em 100% simultaneamente no clube. O som perde definição e vira uma pasta sonora."
                },
                {
                    2, "3/5", "3. Live Performance: Acid 303 no Build-up",
                    "Compassos 33-48", "00:56 - 01:24",
                    56.47, 84.71,
                    ICON_FA_MICROCHIP,
                    ImVec4(1.0f, 0.40f, 0.85f, 1.0f),
                    "BUILD-UP (SUBIDA)",
                    "A faixa Nuke entrou na subida de tensão! Ligue o Sintetizador Acid 303, abra o knob CUTOFF devagar de 20% a 90% e CORTE NO DROP exato aos 01:24!",
                    "O sintetizador Roland TB-303 analógico adiciona eletricidade ao vivo ao set. Durante a subida, abrir a frequência de corte (Cutoff) gera tensão na pista. No segundo exato em que o Drop explode, cortar o 303 entrega todo o impacto para o bumbo original.",
                    "1. Clique em [⏻ LIGAR 303] | 2. Gire o knob CUTOFF suavemente da esquerda para a direita | 3. Ao atingir o Drop (01:24), CLIQUE EM DESLIGAR instantaneamente!",
                    "O silêncio cirúrgico no momento do Drop faz a explosão de som parecer o dobro mais potente. Zero som residual!"
                },
                {
                    3, "4/5", "4. Transição de Emergência: Echo Out & Flush",
                    "Compassos 49-64", "01:24 - 01:52",
                    84.71, 112.94,
                    ICON_FA_TOWER_BROADCAST,
                    ImVec4(1.0f, 0.75f, 0.20f, 1.0f),
                    "PEAK / SAÍDA FX",
                    "No final da frase (compasso 64 tempo 4), dispare o Echo Out para congelar a faixa no espaço estéreo. Quando a próxima música entrar, aperte FLUSH BUFFER para matar o eco!",
                    "O Echo Out é o salva-vidas do DJ: permite sair de qualquer música mesmo sem encaixe harmônico. Porém, o erro de iniciante é deixar o eco soando por baixo da nova faixa. O FLUSH BUFFER limpa a memória do delay e zera qualquer ruído residual.",
                    "1. Selecione [1 Beat] ou [1/2 Beat] | 2. No tempo 4 antes da virada, clique em [DISPARAR ECHO OUT] | 3. Solte o Deck 2 e aperte [PARAR ECHO OUT & FLUSH BUFFER].",
                    "Delay sem flush vira poluição sonora. Limpar o buffer restaura 100% do headroom para o punch da nova música."
                },
                {
                    4, "5/5", "5. Turntablism Físico: Spinback & Vinyl Brake",
                    "Compassos 65+", "01:52+",
                    112.94, 180.0,
                    ICON_FA_COMPACT_DISC,
                    ImVec4(0.85f, 0.35f, 1.0f, 1.0f),
                    "BREAKDOWN / MANOBRA",
                    "Execute uma manobra clássica de vinil analógico: o Spinback (giro reverso veloz) para troca radical de gênero ou o Vinyl Brake (queda de tom do motor Technics SL-1200).",
                    "As manobras físicas de discotecagem quebram a sensação fria do digital. Elas avisam a pista que um momento especial está acontecendo e criam euforia. Devem ser disparadas 1 ou 2 tempos antes da virada, com o Play imediato na nova faixa.",
                    "1. Clique em [Spinback Deck 1] para o som característico de rebobinar fita | 2. Ou clique em [Vinyl Brake Deck 1] para a desaceleração de vitrola analógica.",
                    "O Spinback é a transição de choque mais famosa da música eletrônica mundial (usada por James Hype, Carl Cox e Tiësto)."
                }
            };
            return s_phases;
        }

        // =========================================================================
        // RENDERIZAÇÃO PRINCIPAL DO MODO JOGO & ABDUCTION DJ ACADEMY
        // COM SUPORTE TOTAL A MÚSICA REAL E FEEDBACK EM TEMPO REAL
        // =========================================================================
        void render(KuroAudio::DJEngine& engine, float screen_w, float screen_h) {
            render(&engine, screen_w, screen_h);
        }

        void render(float screen_w, float screen_h) {
            render((KuroAudio::DJEngine*)nullptr, screen_w, screen_h);
        }

        bool isModalGuideOpen() const { return m_show_modal_guide; }
        void setModalGuideOpen(bool open) { m_show_modal_guide = open; }
        void toggleModalGuide() { m_show_modal_guide = !m_show_modal_guide; }

        void render(KuroAudio::DJEngine* p_engine, float screen_w, float screen_h) {
            if (!m_active || !m_show_modal_guide) return;

            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                m_show_modal_guide = false;
                return;
            }

            auto& repeater = KuroAudio::DJTransitionLoopRepeater::getInstance();
            auto& turntable = KuroAudio::DJTurntablismEngine::getInstance();
            auto& gain_staging = KuroAudio::DJGainStagingAnalyzer::getInstance();
            auto& live_inst = KuroAudio::DJLiveInstrumentEngine::getInstance();

            ImGuiViewport* vp = ImGui::GetMainViewport();
            m_anim_time += ImGui::GetIO().DeltaTime;

            // 1. BACKDROP ELIMINADO: Nada de pintar tela preta/escurecida que bloqueie os decks!
            // O modo jogo agora roda perfeitamente transparente e integrado ao software.

            // 2. Janela Centralizada Adaptativa com proteção contra overflow em qualquer tela
            float max_w = vp ? (vp->Size.x - 24.0f) : 1900.0f;
            float max_h = vp ? (vp->Size.y - 24.0f) : 1060.0f;
            float min_w = std::min(700.0f, max_w);
            float min_h = std::min(420.0f, max_h);
            float hud_w = std::clamp((vp ? vp->Size.x : 1920.0f) * 0.92f, min_w, max_w);
            float hud_h = std::clamp((vp ? vp->Size.y : 1080.0f) * 0.86f, min_h, max_h);
            float hud_x = vp ? (vp->Pos.x + (vp->Size.x - hud_w) * 0.5f) : 20.0f;
            float hud_y = vp ? (vp->Pos.y + (vp->Size.y - hud_h) * 0.5f) : 20.0f;

            ImGui::SetNextWindowPos(ImVec2(hud_x, hud_y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(hud_w, hud_h), ImGuiCond_Always);

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 18.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 8.0f));

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.10f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.72f, 0.14f, 0.95f)); // Gold Cyberpunk

            if (ImGui::Begin("##DJAcademyGameHUD", nullptr, flags)) {
                const auto& phases = getPhases();

                // Telemetria em tempo real do Deck A
                double elapsed_s = 0.0;
                double total_dur_s = 344.35;
                double bpm = 136.0;
                int current_bar = 1;
                int current_beat = 1;
                bool is_playing = false;
                std::string track_title = "Nuke";
                std::string track_artist = "Aura Vortex & Blazy";

                if (p_engine) {
                    elapsed_s = p_engine->deckA.getElapsedTimeSec();
                    total_dur_s = (p_engine->deckA.duration_sec > 1.0) ? p_engine->deckA.duration_sec : 344.35;
                    bpm = (p_engine->deckA.bpm > 20.0) ? p_engine->deckA.bpm : 136.0;
                    is_playing = p_engine->deckA.is_playing;
                    if (!p_engine->deckA.track_title.empty()) track_title = p_engine->deckA.track_title;
                    if (!p_engine->deckA.track_artist.empty()) track_artist = p_engine->deckA.track_artist;

                    // Cálculo matemático do compasso e beat
                    double beat_len_s = 60.0 / bpm;
                    double bar_len_s = beat_len_s * 4.0;
                    current_bar = (int)(elapsed_s / bar_len_s) + 1;
                    current_beat = ((int)(elapsed_s / beat_len_s) % 4) + 1;

                    // Lógica de avanço automático da fase (Auto-Follow)
                    if (m_auto_follow_playback && is_playing) {
                        for (int i = 0; i < (int)phases.size(); i++) {
                            if (elapsed_s >= phases[i].start_sec && elapsed_s < phases[i].end_sec) {
                                m_current_phase = i;
                                break;
                            }
                        }
                    }

                    // Lógica de loop contínuo da fase
                    if (m_phase_loop_active && is_playing) {
                        const auto& cur_ph = phases[m_current_phase];
                        if (elapsed_s >= cur_ph.end_sec) {
                            p_engine->deckA.current_frame = cur_ph.start_sec * p_engine->deckA.sample_rate;
                        }
                    }
                }

                const auto& active_phase = phases[m_current_phase];

                // =========================================================================
                // 1. CABEÇALHO HERO: TÍTULO, FAIXA DE ESTUDO, STATUS & BOTÃO FECHAR
                // =========================================================================
                float btn_close_w = UIScaleManager::Scale(110.0f);
                float cur_win_w = ImGui::GetWindowWidth();
                float right_close_x = std::max(200.0f, cur_win_w - btn_close_w - UIScaleManager::Scale(24.0f));

                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), ICON_FA_GAMEPAD " ABDUCTION DJ ACADEMY");
                ImGui::SameLine();
                ImGui::TextDisabled("| TUTORIAL INTERATIVO EM TEMPO REAL [F2]");

                // Botão de Fechar fixo no canto superior direito para colisão zero
                ImGui::SameLine(right_close_x);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.10f, 0.12f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.20f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.70f, 0.75f, 1.0f));
                if (ImGui::Button(ICON_FA_XMARK " FECHAR", ImVec2(btn_close_w, UIScaleManager::Scale(28.0f)))) {
                    m_show_modal_guide = false;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Fechar Console da Academy e continuar praticando nos Decks [ESC]");
                ImGui::PopStyleColor(3);

                // Sub-linha estruturada com Botão de Carga + Telemetria da Faixa + Score/Combo
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.32f, 0.50f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.50f, 0.75f, 1.0f));
                if (ImGui::Button(ICON_FA_MUSIC " CARREGAR FAIXAS DE ESTUDO (NUKE 136 BPM)")) {
                    if (p_engine) loadStudiedTracks(*p_engine);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carrega Aura Vortex & Blazy - Nuke no Deck 1 e Sweet Disposition no Deck 2");
                ImGui::PopStyleColor(2);

                ImGui::SameLine(0, 12.0f);
                ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " %s - %s", track_artist.c_str(), track_title.c_str());
                ImGui::SameLine(0, 8.0f);
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[%.1f BPM | 10B]", bpm);
                ImGui::SameLine(0, 14.0f);
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), ICON_FA_STAR " SCORE: %.0f PTS", m_tutorial_score);
                ImGui::SameLine(0, 10.0f);
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ICON_FA_FIRE " COMBO: %dx", m_tutorial_streak);

                ImGui::Spacing();

                // =========================================================================
                // 2. RÉGUA DE NAVEGAÇÃO DE FASES (5 ETAPAS DO TUTORIAL + CONTROLE DE TEMPO)
                // =========================================================================
                float avail_w = ImGui::GetContentRegionAvail().x;
                float phase_btn_w = (avail_w - 140.0f - 16.0f) / 5.0f;

                for (int i = 0; i < (int)phases.size(); i++) {
                    const auto& ph = phases[i];
                    bool is_sel = (m_current_phase == i);

                    if (is_sel) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ph.theme_color);
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(10, 14, 22, 255));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.14f, 0.22f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ph.theme_color);
                    }
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ph.theme_color.x * 0.8f, ph.theme_color.y * 0.8f, ph.theme_color.z * 0.8f, 1.0f));

                    char btn_label[128];
                    snprintf(btn_label, sizeof(btn_label), "%s %s##PhaseBtn_%d", ph.icon, ph.title, i);
                    if (ImGui::Button(btn_label, ImVec2(phase_btn_w, 34.0f))) {
                        m_current_phase = i;
                        m_auto_follow_playback = false; // Usuário selecionou manualmente para estudar
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Fase %s: %s (%s | %s)\nClique para estudar os detalhes desta etapa!", ph.step_number, ph.title, ph.bars_str, ph.time_str);
                    }
                    ImGui::PopStyleColor(3);

                    if (i < 4) ImGui::SameLine(0, 4.0f);
                }

                // Botão de Seguir a Música Automaticamente
                ImGui::SameLine(0, 10.0f);
                if (m_auto_follow_playback) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.50f, 0.30f, 1.0f));
                    if (ImGui::Button(ICON_FA_COMPASS " AUTO-FASE: ON", ImVec2(130.0f, 34.0f))) {
                        m_auto_follow_playback = false;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Avanço automático ativado: a fase acompanha o segundo exato da música!");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.28f, 0.35f, 1.0f));
                    if (ImGui::Button(ICON_FA_COMPASS " AUTO-FASE: OFF", ImVec2(130.0f, 34.0f))) {
                        m_auto_follow_playback = true;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clique para religar o avanço automático da fase conforme a música toca!");
                    ImGui::PopStyleColor();
                }

                // Barra de Progresso Geral da Faixa de Estudo
                float song_progress = (total_dur_s > 0.0) ? (float)(elapsed_s / total_dur_s) : 0.0f;
                song_progress = std::clamp(song_progress, 0.0f, 1.0f);

                char prog_str[128];
                int el_min = (int)(elapsed_s / 60.0);
                int el_sec = (int)(elapsed_s) % 60;
                int tot_min = (int)(total_dur_s / 60.0);
                int tot_sec = (int)(total_dur_s) % 60;
                snprintf(prog_str, sizeof(prog_str), "Tempo: %02d:%02d / %02d:%02d | Compasso: %d | Beat: %d/4 | Seção: %s",
                         el_min, el_sec, tot_min, tot_sec, current_bar, current_beat, active_phase.section_type);

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, active_phase.theme_color);
                ImGui::ProgressBar(song_progress, ImVec2(-1.0f, 18.0f), prog_str);
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // =========================================================================
                // 3. GRID PRINCIPAL DE 3 CARDS INTEGRADOS COM LÓGICA DE USO HUMANA
                // =========================================================================
                float card_gap = 18.0f;
                float card_w = (avail_w - card_gap * 2.0f) / 3.0f;
                float card_h = ImGui::GetContentRegionAvail().y - UIScaleManager::Scale(56.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.28f, 0.42f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.08f, 0.13f, 0.95f));

                // -------------------------------------------------------------------------
                // CARD 1: 🎧 TELEMETRIA DA MÚSICA & DETECTOR DE BASS CLASH AO VIVO
                // -------------------------------------------------------------------------
                ImGui::BeginChild("##AcademyCard1", ImVec2(card_w, card_h), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), ICON_FA_WAVE_SQUARE " TELEMETRIA & BASS CLASH GUARD");
                    ImGui::TextDisabled("Monitoramento em tempo real da música nos Decks");
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Transporte Rápido do Deck 1
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f), ICON_FA_COMPACT_DISC " CONTROLE DO DECK 1:");
                    if (is_playing) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.25f, 0.25f, 0.90f));
                        if (ImGui::Button(ICON_FA_PAUSE " PAUSAR DECK 1", ImVec2(-1.0f, 32.0f))) {
                            if (p_engine) p_engine->deckA.is_playing = false;
                        }
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.65f, 0.35f, 0.90f));
                        if (ImGui::Button(ICON_FA_PLAY " TOCAR DECK 1 (ESPAÇO)", ImVec2(-1.0f, 32.0f))) {
                            if (p_engine) p_engine->deckA.is_playing = true;
                        }
                        ImGui::PopStyleColor();
                    }

                    ImGui::Spacing();

                    // Metrônomo visual de 4 tempos (Pulsante)
                    ImGui::Text("Metrônomo da Frase (Tempo 1-2-3-4):");
                    float beat_box_w = (ImGui::GetContentRegionAvail().x - 18.0f) / 4.0f;
                    for (int b = 1; b <= 4; b++) {
                        bool is_cur_beat = is_playing && (current_beat == b);
                        ImVec4 b_col = is_cur_beat ?
                            ((b == 1) ? ImVec4(0.0f, 1.0f, 0.8f, 1.0f) : ImVec4(1.0f, 0.75f, 0.2f, 1.0f)) :
                            ImVec4(0.12f, 0.16f, 0.24f, 0.8f);

                        ImGui::PushStyleColor(ImGuiCol_Button, b_col);
                        ImGui::PushStyleColor(ImGuiCol_Text, is_cur_beat ? IM_COL32(10, 14, 22, 255) : IM_COL32(160, 180, 210, 255));
                        char b_lbl[32]; snprintf(b_lbl, sizeof(b_lbl), "%d", b);
                        ImGui::Button(b_lbl, ImVec2(beat_box_w, 28.0f));
                        ImGui::PopStyleColor(2);

                        if (b < 4) ImGui::SameLine(0, 6.0f);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Medidor de Graves & Bass Clash
                    float low_a = p_engine ? p_engine->deckA.eq_low : 1.0f;
                    float low_b = p_engine ? p_engine->deckB.eq_low : 0.0f;
                    float sum_low = low_a + low_b;
                    bool clash = (sum_low > 1.30f);

                    ImGui::TextColored(ImVec4(0.40f, 0.85f, 1.0f, 1.0f), ICON_FA_CHART_BAR " SOMA DE SUBGRAVES (LOW EQ):");
                    float clash_norm = std::clamp(sum_low / 2.0f, 0.0f, 1.0f);

                    if (clash) {
                        float pulse = (std::sin(m_anim_time * 12.0f) + 1.0f) * 0.5f;
                        ImVec4 red_pulse(1.0f, 0.20f * pulse, 0.20f * pulse, 1.0f);
                        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, red_pulse);
                        ImGui::ProgressBar(clash_norm, ImVec2(-1.0f, 24.0f), "PERIGO: CHOQUE DE GRAVES!");
                        ImGui::PopStyleColor();

                        ImGui::Spacing();
                        ImGui::TextColored(red_pulse, ICON_FA_TRIANGLE_EXCLAMATION " GRAVES EMBOLANDO!");
                        ImGui::TextWrapped("Ambos os decks estão com grave aberto. Corte o knob LOW de um dos decks para abrir espaço no som!");
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.85f, 0.4f, 1.0f));
                        ImGui::ProgressBar(clash_norm, ImVec2(-1.0f, 24.0f), "BASS STAGING SEGURO & LIMPO");
                        ImGui::PopStyleColor();

                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), ICON_FA_CIRCLE_CHECK " TRANSIÇÃO TRANSPARENTE");
                        ImGui::TextWrapped("Subgraves equilibrados. O PA do clube não distorcerá nesta passagem.");
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Telemetria Master LUFS
                    float master_lufs = gain_staging.current_lufs.load();
                    float true_peak = gain_staging.current_true_peak_db.load();
                    bool is_red = gain_staging.is_redlining.load();
                    ImGui::Text("• Master: %+.1f LUFS | Peak: %+.1f dB", master_lufs, true_peak);
                    float hr_norm = std::clamp((master_lufs + 24.0f) / 18.0f, 0.0f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, is_red ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 0.85f, 0.4f, 1.0f));
                    ImGui::ProgressBar(hr_norm, ImVec2(-1.0f, 18.0f), is_red ? "REDLINING!" : "HEADROOM OK");
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();

                ImGui::SameLine(0, card_gap);

                // -------------------------------------------------------------------------
                // CARD 2: 🎯 ÁREA DE TREINO DA FASE ATIVA (CONTROLES DIRETOS DA FASE)
                // -------------------------------------------------------------------------
                ImGui::BeginChild("##AcademyCard2", ImVec2(card_w, card_h), true);
                {
                    ImGui::TextColored(active_phase.theme_color, "%s %s", active_phase.icon, active_phase.title);
                    ImGui::TextDisabled("%s • %s", active_phase.bars_str, active_phase.time_str);
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Botões de Ação de Transporte da Fase (PULAR e LOOP)
                    float act_half = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.35f, 0.55f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.50f, 0.80f, 1.0f));
                    if (ImGui::Button(ICON_FA_FORWARD_FAST " PULAR P/ ESTA FASE", ImVec2(act_half, 34.0f))) {
                        if (p_engine) {
                            p_engine->deckA.current_frame = active_phase.start_sec * p_engine->deckA.sample_rate;
                            p_engine->deckA.is_playing = true;
                            m_last_feedback = "Música posicionada no início da Fase!";
                            m_feedback_col = active_phase.theme_color;
                        }
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Salta a agulha da música direto para o início deste trecho (%.1fs)!", active_phase.start_sec);

                    ImGui::SameLine(0, 10.0f);

                    if (m_phase_loop_active) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.60f, 0.35f, 1.0f));
                        if (ImGui::Button(ICON_FA_REPEAT " LOOP: ATIVO", ImVec2(act_half, 34.0f))) {
                            m_phase_loop_active = false;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Loop ativado! A música repetirá este trecho continuamente para treino.");
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.25f, 0.32f, 1.0f));
                        if (ImGui::Button(ICON_FA_REPEAT " LOOPAR FASE", ImVec2(act_half, 34.0f))) {
                            m_phase_loop_active = true;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ativa loop de 16 compassos nesta fase para praticar repetidas vezes.");
                        ImGui::PopStyleColor();
                    }
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // OBJETIVO DA FASE
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.20f, 1.0f), ICON_FA_BULLSEYE " OBJETIVO PRÁTICO:");
                    ImGui::TextWrapped("%s", active_phase.mission_goal);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // CONTROLES ESPECÍFICOS DE CADA FASE
                    if (m_current_phase == 0) {
                        // FASE 1: BEATGRID & INTRO
                        ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_SLIDERS " CONTROLES DE ALINHAMENTO:");
                        ImGui::Text("• BPM Detectado: %.1f BPM", bpm);
                        ImGui::Text("• Primeiro Downbeat: Compasso 1 Beat 1");
                        if (ImGui::Button(ICON_FA_COMPACT_DISC " CUE NO TEMPO 1 (REINICIAR INTRO)", ImVec2(-1.0f, 36.0f))) {
                            if (p_engine) {
                                p_engine->deckA.current_frame = 0.0;
                                p_engine->deckA.is_playing = true;
                            }
                        }
                    } else if (m_current_phase == 1) {
                        // FASE 2: BASS STAGING SEGURO
                        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), ICON_FA_SLIDERS " ALVOS DE GRAVE (EQ LOW):");
                        if (p_engine) {
                            ImGui::SliderFloat("LOW Deck 1 (Nuke)", &p_engine->deckA.eq_low, 0.0f, 1.0f, "%.0f%%");
                            ImGui::SliderFloat("LOW Deck 2 (Sweet)", &p_engine->deckB.eq_low, 0.0f, 1.0f, "%.0f%%");
                        }
                        ImGui::Spacing();
                        if (ImGui::Button(ICON_FA_BOLT " ⚡ EXECUTAR BASS SWAP (SWAP DE GRAVES)", ImVec2(-1.0f, 36.0f))) {
                            if (p_engine) {
                                float tmp = p_engine->deckA.eq_low;
                                p_engine->deckA.eq_low = p_engine->deckB.eq_low;
                                p_engine->deckB.eq_low = tmp;
                                m_last_feedback = "Bass Swap realizado com sucesso no compasso!";
                                m_feedback_col = ImVec4(0.2f, 1.0f, 0.5f, 1.0f);
                                m_tutorial_score = std::min(100.0f, m_tutorial_score + 5.0f);
                                m_tutorial_streak++;
                            }
                        }
                    } else if (m_current_phase == 2) {
                        // FASE 3: LIVE PERFORMANCE ACID 303 NO BUILD-UP
                        ImGui::TextColored(ImVec4(1.0f, 0.40f, 0.85f, 1.0f), ICON_FA_MICROCHIP " SINTETIZADOR ACID 303 AO VIVO:");
                        bool synth_on = live_inst.synth303.is_running.load();
                        if (synth_on) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.25f, 1.0f));
                            if (ImGui::Button(ICON_FA_POWER_OFF " ⏻ SINTETIZADOR ACID 303: LIGADO (CLIQUE P/ CORTAR NO DROP!)", ImVec2(-1.0f, 36.0f))) {
                                live_inst.synth303.stop();
                                m_last_feedback = "Corte perfeito no Drop! Zero ruído residual.";
                                m_feedback_col = ImVec4(0.2f, 1.0f, 0.5f, 1.0f);
                                m_tutorial_score = std::min(100.0f, m_tutorial_score + 10.0f);
                                m_tutorial_streak++;
                            }
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.45f, 0.75f, 1.0f));
                            if (ImGui::Button(ICON_FA_POWER_OFF " ⏻ LIGAR ACID 303 NA SUBIDA", ImVec2(-1.0f, 36.0f))) {
                                live_inst.synth303.start();
                                m_last_feedback = "Acid 303 Ligado! Abra o Cutoff suavemente.";
                                m_feedback_col = ImVec4(1.0f, 0.40f, 0.85f, 1.0f);
                            }
                            ImGui::PopStyleColor();
                        }

                        ImGui::Spacing();
                        // Knobs Rotativos do 303
                        ImGui::BeginGroup();
                        DrawRotaryKnob("CUTOFF##Acad303", &live_inst.synth303.cutoff, 0.05f, 0.98f, 18.0f, ImVec4(1.0f, 0.40f, 0.85f, 1.0f), "%.0f%%");
                        ImGui::SameLine(0, 16.0f);
                        DrawRotaryKnob("RESO##Acad303", &live_inst.synth303.resonance, 0.10f, 0.95f, 18.0f, ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%.0f%%");
                        ImGui::EndGroup();
                    } else if (m_current_phase == 3) {
                        // FASE 4: TRANSIÇÃO DE EMERGÊNCIA ECHO OUT & FLUSH
                        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.20f, 1.0f), ICON_FA_TOWER_BROADCAST " CONTROLE DO ECHO OUT:");
                        bool echo_on = gain_staging.echo_out_active.load();

                        if (echo_on) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.25f, 1.0f));
                            if (ImGui::Button(ICON_FA_SQUARE " ⬛ PARAR ECHO OUT & FLUSH BUFFER", ImVec2(-1.0f, 38.0f))) {
                                gain_staging.stopEchoOut();
                                m_last_feedback = "Buffer esvaziado! Zero som residual na transição.";
                                m_feedback_col = ImVec4(0.2f, 1.0f, 0.5f, 1.0f);
                                m_tutorial_score = std::min(100.0f, m_tutorial_score + 10.0f);
                                m_tutorial_streak++;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Corta imediatamente o eco e zera a memória em milissegundos!");
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.45f, 0.10f, 1.0f));
                            if (ImGui::Button(ICON_FA_SIGNAL " 📡 DISPARAR ECHO OUT (NO TEMPO 4)", ImVec2(-1.0f, 38.0f))) {
                                gain_staging.triggerEchoOut(bpm);
                                m_last_feedback = "Echo Out disparado! A faixa foi congelada no reverb.";
                                m_feedback_col = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Dispare no 4º tempo antes da virada para uma saída épica!");
                            ImGui::PopStyleColor();
                        }

                        ImGui::Spacing();
                        ImGui::Text("Fração de Sincronia de Tempo:");
                        float fr_w = (ImGui::GetContentRegionAvail().x - 12.0f) / 3.0f;
                        if (ImGui::Button("1/2 Beat", ImVec2(fr_w, 26.0f))) {
                            gain_staging.echo_beat_division = 1;
                            if (gain_staging.echo_out_active.load()) gain_staging.triggerEchoOut(bpm);
                        }
                        ImGui::SameLine(0, 6.0f);
                        if (ImGui::Button("1 Beat", ImVec2(fr_w, 26.0f))) {
                            gain_staging.echo_beat_division = 3;
                            if (gain_staging.echo_out_active.load()) gain_staging.triggerEchoOut(bpm);
                        }
                        ImGui::SameLine(0, 6.0f);
                        if (ImGui::Button("2 Beats", ImVec2(fr_w, 26.0f))) {
                            gain_staging.echo_beat_division = 4;
                            if (gain_staging.echo_out_active.load()) gain_staging.triggerEchoOut(bpm);
                        }
                    } else if (m_current_phase == 4) {
                        // FASE 5: TURNTABLISM FÍSICO
                        ImGui::TextColored(ImVec4(0.85f, 0.35f, 1.0f, 1.0f), ICON_FA_COMPACT_DISC " DISPARO DE MANOBRAS FÍSICAS:");
                        float btn_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.22f, 0.35f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.35f, 0.55f, 1.0f));
                        if (ImGui::Button(ICON_FA_ARROW_ROTATE_LEFT " Spinback Deck 1", ImVec2(btn_w, 38.0f))) {
                            turntable.triggerSpinback(0, -42.0f);
                            m_last_feedback = "Spinback executado com inércia angular Pioneer CDJ!";
                            m_feedback_col = ImVec4(0.0f, 0.85f, 1.0f, 1.0f);
                        }
                        ImGui::SameLine(0, 10.0f);
                        if (ImGui::Button(ICON_FA_ARROW_ROTATE_LEFT " Spinback Deck 2", ImVec2(btn_w, 38.0f))) {
                            turntable.triggerSpinback(1, -42.0f);
                        }
                        ImGui::PopStyleColor(2);

                        ImGui::Spacing();

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.18f, 0.10f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.28f, 0.15f, 1.0f));
                        if (ImGui::Button(ICON_FA_POWER_OFF " Vinyl Brake Deck 1", ImVec2(btn_w, 38.0f))) {
                            turntable.triggerVinylBrake(0, 1.6f);
                            m_last_feedback = "Vinyl Brake executado! Desaceleração de motor Technics 1200.";
                            m_feedback_col = ImVec4(1.0f, 0.65f, 0.2f, 1.0f);
                        }
                        ImGui::SameLine(0, 10.0f);
                        if (ImGui::Button(ICON_FA_POWER_OFF " Vinyl Brake Deck 2", ImVec2(btn_w, 38.0f))) {
                            turntable.triggerVinylBrake(1, 1.6f);
                        }
                        ImGui::PopStyleColor(2);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // MENSAGEM DINÂMICA DE FEEDBACK
                    ImGui::TextColored(m_feedback_col, ICON_FA_INFO " %s", m_last_feedback.c_str());
                }
                ImGui::EndChild();

                ImGui::SameLine(0, card_gap);

                // -------------------------------------------------------------------------
                // CARD 3: 📖 GUIA DIDÁTICO: A LÓGICA DE USO PARA O SER HUMANO
                // -------------------------------------------------------------------------
                ImGui::BeginChild("##AcademyCard3", ImVec2(card_w, card_h), true);
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.80f, 0.20f, 1.0f), ICON_FA_BOOK_OPEN " A LÓGICA DE USO PARA O HUMANO");
                    ImGui::TextDisabled("Por que cada regra existe e como o cérebro processa");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "1. POR QUE ESSA TÉCNICA EXISTE?");
                    ImGui::TextWrapped("%s", active_phase.why_for_human);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "2. PASSO A PASSO NA PRÁTICA:");
                    ImGui::TextWrapped("%s", active_phase.action_step);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "3. REGRA DE OURO PROFISSIONAL:");
                    ImGui::TextWrapped("%s", active_phase.golden_rule);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Tabela de Consulta Rápida de Fraseamento
                    ImGui::TextColored(ImVec4(0.70f, 0.80f, 0.95f, 1.0f), ICON_FA_TABLE " CONSULTA RÁPIDA DE COMPASSOS:");
                    ImGui::Text("• 1 Frase = 16 Compassos = 64 Beats (~28.2s a 136 BPM)");
                    ImGui::Text("• 1 Bloco = 32 Compassos = 128 Beats (~56.5s a 136 BPM)");
                    ImGui::Text("• Trocas Harmônicas perfeitas: 10B (D) <-> 10A (Bm)");
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(3);

                // =========================================================================
                // 4. BARRA INFERIOR DE NAVEGAÇÃO & BOTÃO DE SEGURANÇA MESTRE (PANIC MUTE)
                // =========================================================================
                ImGui::Spacing();
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(0.50f, 0.60f, 0.75f, 1.0f),
                    ICON_FA_KEYBOARD " ATALHOS: [F2] Fechar Central | [F3] Rack de Live FX | [ESC] Fechar Janela | [Espaço] Play/Pause Deck 1");

                float panic_w = 230.0f;
                ImGui::SameLine(ImGui::GetWindowWidth() - panic_w - 24.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.12f, 0.16f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.20f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.85f, 1.0f));
                if (ImGui::Button(ICON_FA_TRIANGLE_EXCLAMATION " SILENCIAR TUDO", ImVec2(panic_w, 32.0f))) {
                    live_inst.panicMute();
                    gain_staging.stopEchoOut();
                    m_last_feedback = "Silêncio Mestre Ativado! Todos os buffers limpos.";
                    m_feedback_col = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Corta imediatamente sintetizador, eco e resíduos de áudio!");
                ImGui::PopStyleColor(3);
            }
            ImGui::End();

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(4);
        }

        // =========================================================================
        // VISOR EM TEMPO REAL DO MODO JOGO (IN-GAME HUD VISOR DOCK)
        // Exibe em tempo real o que fazer a cada passo, com mudanças dinâmicas de abordagem
        // quando cada tarefa for concluída! (Ativo EXCLUSIVAMENTE quando m_active == true)
        // =========================================================================
        void renderInGameVisorHUD(KuroAudio::DJEngine& engine, float area_w) {
            if (!m_active) return; // SOMENTE ativo quando o modo game estiver ligado pelo usuário!

            if (!m_tracks_loaded && engine.deckA.buffer_l.empty()) {
                loadStudiedTracks(engine);
            }

            float dt = ImGui::GetIO().DeltaTime;
            if (dt <= 0.0001f || dt > 0.1f) dt = 1.0f / 60.0f;
            if (m_completion_banner_timer > 0.0f) {
                m_completion_banner_timer -= dt;
            }

            const auto& phases = getPhases();
            m_current_phase = std::clamp(m_current_phase, 0, (int)phases.size() - 1);
            const auto& cur_ph = phases[m_current_phase];

            auto& gain_staging = KuroAudio::DJGainStagingAnalyzer::getInstance();
            auto& live_inst = KuroAudio::DJLiveInstrumentEngine::getInstance();
            auto& turntable = KuroAudio::DJTurntablismEngine::getInstance();

            // Telemetria do Deck 1
            auto& d1 = engine.deckA;
            auto& d2 = engine.deckB;
            double elapsed_s = d1.getElapsedTimeSec();
            double bpm = (d1.bpm > 20.0) ? d1.bpm : 136.0;
            double beat_len_s = 60.0 / bpm;
            int current_bar = (int)(elapsed_s / (beat_len_s * 4.0)) + 1;
            int current_beat = ((int)(elapsed_s / beat_len_s) % 4) + 1;
            bool is_playing = d1.is_playing;

            // ---------------------------------------------------------------------
            // VERIFICAÇÃO AUTOMÁTICA DE CONCLUSÃO DE TAREFA & TRANSIÇÃO DE DINÂMICA
            // ---------------------------------------------------------------------
            if (m_completion_banner_timer <= 0.0f) {
                if (m_current_phase == 0) { // FASE 1: BEATGRID & INTRO
                    if (is_playing && current_bar >= 4) {
                        m_completion_banner_timer = 4.0f;
                        m_completion_banner_txt = "🎉 TAREFA 1 CONCLUÍDA! (+100 PTS) DINÂMICA DE BASS STAGING ATIVADA!";
                        m_tutorial_score += 100.0f;
                        m_tutorial_streak++;
                        m_current_phase = 1;
                    }
                } else if (m_current_phase == 1) { // FASE 2: BASS STAGING & EQ
                    // Conclui se o usuário manteve o grave do Deck 2 cortado e tocou por 4 compassos
                    if (d2.eq_low < 0.20f && is_playing && current_bar >= 20) {
                        m_completion_banner_timer = 4.0f;
                        m_completion_banner_txt = "🎉 BASS STAGING PERFEITO! (+150 PTS) DINÂMICA DE LIVE ACID 303 ATIVADA!";
                        m_tutorial_score += 150.0f;
                        m_tutorial_streak++;
                        m_current_phase = 2;
                    }
                } else if (m_current_phase == 2) { // FASE 3: LIVE ACID 303 NO BUILD-UP
                    if (live_inst.synth303.is_running.load() && live_inst.synth303.cutoff > 0.60f) {
                        m_phase_step_progress = 1;
                    }
                    if (m_phase_step_progress == 1 && (!live_inst.synth303.is_running.load() || elapsed_s >= 84.0)) {
                        m_completion_banner_timer = 4.0f;
                        m_completion_banner_txt = "🎉 CORTE NO DROP CIRÚRGICO! (+200 PTS) DINÂMICA DE ECHO OUT ATIVADA!";
                        m_tutorial_score += 200.0f;
                        m_tutorial_streak++;
                        m_current_phase = 3;
                        m_phase_step_progress = 0;
                    }
                } else if (m_current_phase == 3) { // FASE 4: ECHO OUT TRANSITION
                    if (gain_staging.echo_out_active.load()) {
                        m_phase_step_progress = 1;
                    }
                    if (m_phase_step_progress == 1 && !gain_staging.echo_out_active.load()) {
                        m_completion_banner_timer = 4.0f;
                        m_completion_banner_txt = "🎉 TRANSIÇÃO ECHO OUT COM FLUSH! (+250 PTS) DINÂMICA TURNTABLISM ATIVADA!";
                        m_tutorial_score += 250.0f;
                        m_tutorial_streak++;
                        m_current_phase = 4;
                        m_phase_step_progress = 0;
                    }
                } else if (m_current_phase == 4) { // FASE 5: TURNTABLISM
                    if (turntable.isBraking() || turntable.isBackspinning()) {
                        m_completion_banner_timer = 5.0f;
                        m_completion_banner_txt = "🏆 PARABÉNS! VOCÊ DOMINOU AS 5 DINÂMICAS DO ABDUCTION DJ ACADEMY!";
                        m_tutorial_score += 300.0f;
                        m_tutorial_streak++;
                    }
                }
            }

            // ---------------------------------------------------------------------
            // RENDERIZAÇÃO DO VISOR HUD DOURADO/CIANO COM ZERO COLISÃO
            // ---------------------------------------------------------------------
            float actual_w = area_w;
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 100.0f && avail.x < actual_w) {
                actual_w = avail.x;
            }

            float scale = UIScaleManager::getScale();
            float visor_h = std::round(UIScaleManager::Scale(68.0f));
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + actual_w, p_min.y + visor_h);

            // Fundo escuro com gradiente e borda dourada Cyberpunk
            dl->AddRectFilled(p_min, p_max, IM_COL32(10, 14, 22, 252), UIScaleManager::Scale(6.0f));
            ImU32 border_col = (m_completion_banner_timer > 0.0f) ? IM_COL32(0, 255, 140, 255) : IM_COL32(0, 220, 255, 220);
            dl->AddRect(p_min, p_max, border_col, UIScaleManager::Scale(6.0f), 0, UIScaleManager::Scale(1.8f));

            // Indicador LED de Modo Jogo Ativo na ponta esquerda
            dl->AddCircleFilled(ImVec2(p_min.x + UIScaleManager::Scale(14.0f), p_min.y + visor_h * 0.5f), 4.0f, IM_COL32(0, 255, 140, 255));

            // ZONA DIREITA DINÂMICA PARA BOTÕES (Auto-calculada com base no tamanho real do texto e escala)
            float pad_x = ImGui::GetStyle().FramePadding.x * 2.0f;
            float spacing_x = ImGui::GetStyle().ItemSpacing.x;
            float btn_h = UIScaleManager::Scale(28.0f);
            float btn_prev_w = UIScaleManager::Scale(28.0f);
            float btn_next_w = UIScaleManager::Scale(28.0f);
            bool is_compact_w = (actual_w < 1280.0f);
            const char* acad_lbl = is_compact_w ? (ICON_FA_BOOK_OPEN " GUIA") : (ICON_FA_BOOK_OPEN " GUIA COMPLETO [F2]");
            const char* exit_lbl = is_compact_w ? (ICON_FA_POWER_OFF " DESLIGAR") : (ICON_FA_POWER_OFF " SAIR DO MODO JOGO");
            float btn_acad_w = ImGui::CalcTextSize(acad_lbl).x + pad_x + UIScaleManager::Scale(10.0f);
            float btn_exit_w = ImGui::CalcTextSize(exit_lbl).x + pad_x + UIScaleManager::Scale(10.0f);
            float right_zone_w = btn_prev_w + spacing_x + btn_next_w + spacing_x + btn_acad_w + spacing_x + btn_exit_w;
            float right_zone_x = p_max.x - right_zone_w - UIScaleManager::Scale(14.0f);
            float btns_y = p_min.y + (visor_h - btn_h) * 0.5f;

            // ZONA ESQUERDA PARA TEXTOS E TELEMETRIA (Com proteção estrita contra colisão)
            float left_zone_w = std::max(UIScaleManager::Scale(100.0f), right_zone_x - p_min.x - UIScaleManager::Scale(14.0f));

            // Aplica corte estrito (ClipRect) na zona esquerda para garantir colisão zero
            dl->PushClipRect(ImVec2(p_min.x + UIScaleManager::Scale(6.0f), p_min.y + UIScaleManager::Scale(2.0f)),
                             ImVec2(p_min.x + left_zone_w, p_min.y + visor_h - UIScaleManager::Scale(2.0f)), true);

            if (m_completion_banner_timer > 0.0f) {
                // Banner de Conclusão e Mudança de Dinâmica
                float pulse = (std::sin((float)ImGui::GetTime() * 12.0f) + 1.0f) * 0.5f;
                ImVec4 b_col(0.2f + 0.8f * pulse, 1.0f, 0.4f, 1.0f);
                char b_txt[256];
                snprintf(b_txt, sizeof(b_txt), "%s %s  •  SCORE: %.0f PTS | COMBO: %dx", ICON_FA_TROPHY, m_completion_banner_txt.c_str(), m_tutorial_score, m_tutorial_streak);
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), ImGui::GetFontSize() * 0.95f,
                                                 ImVec2(p_min.x + UIScaleManager::Scale(12.0f), p_min.y + UIScaleManager::Scale(18.0f)),
                                                 ImVec2(p_min.x + left_zone_w - UIScaleManager::Scale(6.0f), p_min.y + UIScaleManager::Scale(50.0f)),
                                                 ImGui::ColorConvertFloat4ToU32(b_col), b_txt, 2.0f, 1.0f);
            } else {
                // LINHA 1: Cápsula da Fase Atual + Como Proceder (Alinhado à Esquerda)
                char phase_tag[64];
                snprintf(phase_tag, sizeof(phase_tag), "%s FASE %s: %s", cur_ph.icon, cur_ph.step_number, cur_ph.title);
                ImVec2 p_sz = ImGui::CalcTextSize(phase_tag);
                float badge_w = p_sz.x + UIScaleManager::Scale(18.0f);
                ImVec2 b_min(p_min.x + UIScaleManager::Scale(10.0f), p_min.y + UIScaleManager::Scale(8.0f));
                ImVec2 b_max(b_min.x + badge_w, p_min.y + UIScaleManager::Scale(32.0f));

                dl->AddRectFilled(b_min, b_max, ImGui::ColorConvertFloat4ToU32(cur_ph.theme_color), UIScaleManager::Scale(4.0f));
                dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.90f, ImVec2(b_min.x + UIScaleManager::Scale(9.0f), b_min.y + UIScaleManager::Scale(4.0f)), IM_COL32(10, 14, 22, 255), phase_tag);

                // Instrução dinâmica exata da etapa atual
                const char* dynamic_instruction = "";
                if (m_current_phase == 0) {
                    dynamic_instruction = is_playing ?
                        "Deck 1 tocando! Sinta o ritmo e observe o pulso no metrônomo. Mantenha até o compasso 4!" :
                        "Dê PLAY no Deck 1 no Tempo 1! Observe o pulso dos 4 tempos no metrônomo visual.";
                } else if (m_current_phase == 1) {
                    dynamic_instruction = (d2.eq_low > 0.20f) ?
                        "ALERTA BASS CLASH: Corte o LOW do Deck 2 para zero antes da virada!" :
                        "Bass Staging Seguro! Deck 1 tem o grave dominante. Use o Auto Bass Swap no drop.";
                } else if (m_current_phase == 2) {
                    dynamic_instruction = (!live_inst.synth303.is_running.load()) ?
                        "Abra o Rack Live FX [F3] e ligue o Acid 303 para eletrificar a subida!" :
                        "Gire o CUTOFF no sentido horário para criar tensão! CORTE no segundo exato do drop aos 01:24!";
                } else if (m_current_phase == 3) {
                    dynamic_instruction = (!gain_staging.echo_out_active.load()) ?
                        "No tempo 4 do último compasso da frase, clique em DISPARAR ECHO OUT!" :
                        "Cauda congelada! Solte a próxima música e clique em LIMPAR BUFFER para zerar ruídos.";
                } else {
                    dynamic_instruction = "Dispare um Spinback veloz ou Vinyl Brake no Deck 1 para choque de pista!";
                }

                float text_start_x = b_max.x + UIScaleManager::Scale(12.0f);
                char instr_full[512];
                snprintf(instr_full, sizeof(instr_full), "%s COMO PROCEDER: %s", ICON_FA_LIGHTBULB, dynamic_instruction);
                float instr_box_w = (p_min.x + left_zone_w) - text_start_x;
                if (instr_box_w > UIScaleManager::Scale(40.0f)) {
                    DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), ImGui::GetFontSize() * 0.95f,
                                                     ImVec2(text_start_x, p_min.y + UIScaleManager::Scale(8.0f)),
                                                     ImVec2(p_min.x + left_zone_w, p_min.y + UIScaleManager::Scale(32.0f)),
                                                     IM_COL32(240, 245, 255, 250), instr_full, 2.0f, 1.0f);
                }

                // LINHA 2: Telemetria em Tempo Real & Abordagem Dinâmica (Alinhado à Esquerda com Ajuste Automático)
                const char* approach_name = "";
                switch (m_current_phase) {
                    case 0: approach_name = "Sincronia Rítmica & Respiração Tessaiga"; break;
                    case 1: approach_name = "Isolamento de Graves & Anti-Bass Clash"; break;
                    case 2: approach_name = "Riser Sintético Acid 303 & Drop Impact"; break;
                    case 3: approach_name = "Congelamento de Cauda Echo Out & Transição"; break;
                    default: approach_name = "Atrito de Vinil / Backspin & Finalização"; break;
                }

                char line2_buf[384];
                if (is_compact_w) {
                    snprintf(line2_buf, sizeof(line2_buf), "%s C:%d | B:%d/4 | %.1f BPM  •  DINÂMICA: [%s]  •  %s",
                             ICON_FA_COMPACT_DISC, current_bar, current_beat, bpm, approach_name, cur_ph.section_type);
                } else {
                    snprintf(line2_buf, sizeof(line2_buf), "%s Compasso: %d | Beat: %d/4 | %.1f BPM   •   DINÂMICA: [%s]   •   Seção MIR: %s",
                             ICON_FA_COMPACT_DISC, current_bar, current_beat, bpm, approach_name, cur_ph.section_type);
                }
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), ImGui::GetFontSize() * 0.90f,
                                                 ImVec2(p_min.x + UIScaleManager::Scale(12.0f), p_min.y + UIScaleManager::Scale(36.0f)),
                                                 ImVec2(p_min.x + left_zone_w, p_min.y + UIScaleManager::Scale(60.0f)),
                                                 IM_COL32(0, 220, 255, 255), line2_buf, 2.0f, 1.0f);
            }

            dl->PopClipRect();

            // -----------------------------------------------------------------
            // BOTÕES DE NAVEGAÇÃO E ACADEMY NA ZONA DIREITA (LARGURA ADAPTATIVA DINÂMICA)
            // -----------------------------------------------------------------
            ImGui::SetCursorScreenPos(ImVec2(right_zone_x, btns_y));
            if (ImGui::Button(ICON_FA_CHEVRON_LEFT "##PrevDyn", ImVec2(btn_prev_w, btn_h))) {
                m_current_phase = (m_current_phase + 4) % 5;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Voltar para a dinâmica anterior");

            ImGui::SameLine(0, spacing_x);
            if (ImGui::Button(ICON_FA_CHEVRON_RIGHT "##NextDyn", ImVec2(btn_next_w, btn_h))) {
                m_current_phase = (m_current_phase + 1) % 5;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Avançar para a próxima dinâmica");

            ImGui::SameLine(0, spacing_x);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.42f, 0.65f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.55f, 0.85f, 1.0f));
            if (ImGui::Button(acad_lbl, ImVec2(btn_acad_w, btn_h))) {
                m_show_modal_guide = !m_show_modal_guide;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abrir/Fechar Console Completo da DJ Academy (3 Cards didáticos) [F2]");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, spacing_x);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.48f, 0.12f, 0.16f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.18f, 0.22f, 1.0f));
            if (ImGui::Button(exit_lbl, ImVec2(btn_exit_w, btn_h))) {
                m_active = false;
                m_show_modal_guide = false;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Desativar o Modo Jogo e voltar ao modo DJ normal");
            ImGui::PopStyleColor(2);

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + visor_h + UIScaleManager::Scale(4.0f)));
            ImGui::Dummy(ImVec2(actual_w, UIScaleManager::Scale(4.0f)));
        }
    };

} // namespace KuroUI
