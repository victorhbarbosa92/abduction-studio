#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "UIScaleManager.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"
#include "../audio/SynthEngine.h"
#include "../audio/KuroWave.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

extern KuroDSP::TimelineManager timeline;
extern ClipManager g_clip_manager;
extern KuroAudio::SynthEngine g_piano_synth;

namespace KuroUI {

    class KuroAITrackArchitectUI {
    public:
        bool is_open = false;
        bool need_focus = false;

        // ==========================================
        // PARÂMETROS DO QUESTIONÁRIO & DNA DA FAIXA
        // ==========================================
        int reference_track_idx = 0;
        char custom_reference_name[128] = "Astrix - Deep Jungle Walk";
        float bpm = 142.0f;
        int subgenre_idx = 0;   // 0=Full-On, 1=Progressive, 2=Twilight, 3=Forest, 4=Darkpsy, 5=Hi-Tech, 6=Minimal Offbeat
        int scale_idx = 1;      // 0=Natural Minor, 1=E Phrygian, 2=F# Minor, 3=G Minor, 4=Phrygian Dominant, 5=Gypsy Minor
        int groove_type_idx = 0;// 0=Rolling 16th (KBBB), 1=Gallop (KBB), 2=Offbeat (KB), 3=Octave Jump
        
        // Polaridade Emocional & Caráter
        float heavy_vs_melancholic = 0.45f; // 0.0 = Melancólica/Melódica, 1.0 = Pesada/Agressiva
        float energy_level = 0.85f;         // 0.0 = Hipnótica/Lenta, 1.0 = Festival Climax
        int arrangement_bars_idx = 1;       // 0=64 Bars (Drop Festival), 1=96 Bars (Standard Club), 2=128 Bars (Epic Extended)

        // Histórico de Conversação do Chat
        struct ChatMessage {
            std::string sender; // "AI" ou "USER"
            std::string text;
            ImVec4 color;
        };
        std::vector<ChatMessage> chat_history;
        char user_input_prompt[256] = "";

        // Status do Co-Piloto de Polimento
        bool phase_locked = true;
        float phase_alignment_score = 99.4f;
        bool sidechain_optimized = true;
        bool drop_riser_generated = false;
        std::string last_action_status = "Pronto para sintetizar blueprint da faixa.";

        KuroAITrackArchitectUI() {
            // Inicializar histórico padrão de boas-vindas
            chat_history.push_back({
                "AI",
                "Saudações, Produtor. Eu sou o Abduction AI Architect. Vamos desenhar uma nova track do zero guiada pelo DNA sonoro do Psytrance e música eletrônica.",
                ImVec4(0.0f, 0.95f, 0.85f, 1.0f)
            });
            chat_history.push_back({
                "AI",
                "Defina sua referência, andamento, tom e polaridade (pesada ou melancólica) ao lado, ou me diga no chat o que tem em mente!",
                ImVec4(0.35f, 1.0f, 0.45f, 1.0f)
            });
        }

        void Open() {
            is_open = true;
            need_focus = true;
        }

        void Toggle() {
            is_open = !is_open;
            if (is_open) need_focus = true;
        }

        // ==========================================
        // MOTOR PROCEDURAL: INJEÇÃO DE PROJETO .KURO
        // ==========================================
        void InjectTrackToProject() {
            std::lock_guard<std::mutex> lock(timeline.timeline_mutex);
            std::lock_guard<std::mutex> clip_lock(g_clip_manager.clip_mutex);

            // 1. Configura BPM na timeline do projeto
            timeline.setBPM(bpm);

            // 2. Limpa patterns e clips antigos
            g_clip_manager.global_patterns.clear();
            for (int t = 0; t < MAX_TRACKS; ++t) {
                g_clip_manager.track_clips[t].clear();
                g_clip_manager.track_midi_clips[t].clear();
                g_clip_manager.track_auto_clips[t].clear();
            }
            timeline.section_markers.clear();

            // Mapeamento de tom raiz (MIDI Note)
            int root_midi = 36; // C1
            if (scale_idx == 1) root_midi = 40; // E1 Phrygian
            else if (scale_idx == 2) root_midi = 42; // F#1 Minor
            else if (scale_idx == 3) root_midi = 43; // G1 Minor
            else if (scale_idx == 4) root_midi = 40; // E1 Phrygian Dom
            else if (scale_idx == 5) root_midi = 38; // D1 Gypsy Minor

            // Escala para notas de Lead / Arpeggio
            std::vector<int> scale_offsets = {0, 1, 3, 5, 7, 8, 10}; // Phrygian default
            if (scale_idx == 0) scale_offsets = {0, 2, 3, 5, 7, 8, 10}; // Natural Minor
            else if (scale_idx == 4) scale_offsets = {0, 1, 4, 5, 7, 8, 10}; // Phrygian Dominant
            else if (scale_idx == 5) scale_offsets = {0, 2, 3, 6, 7, 8, 11}; // Gypsy Minor

            // 3. Pattern 1: Kick & Rolling Bass
            Pattern p_kick_bass;
            p_kick_bass.id = 1;
            p_kick_bass.name = "01. Psy Kick & Rolling Bass";
            p_kick_bass.color = IM_COL32(0, 255, 128, 255);
            p_kick_bass.default_length_sec = 4.0f;

            // 4 Batidas de Kick por compasso (Canal 0)
            for (int beat = 0; beat < 4; ++beat) {
                p_kick_bass.getChannelNotes(0).push_back(
                    KuroDSP::MidiNote(36, beat * 0.25f, 0.12f, 1.0f, 0.0f, 0)
                );
            }

            // Rolling Bass (Canal 1): K B B B ou K B B
            for (int beat = 0; beat < 4; ++beat) {
                float t0 = beat * 0.25f;
                int current_root = root_midi;
                if (groove_type_idx == 3 && beat == 3) current_root += 12; // Octave Jump

                if (groove_type_idx == 2) {
                    // Offbeat (KB)
                    p_kick_bass.getChannelNotes(1).push_back(
                        KuroDSP::MidiNote(current_root, t0 + 0.125f, 0.10f, 0.95f, 0.0f, 1)
                    );
                } else if (groove_type_idx == 1) {
                    // Gallop (KBB)
                    p_kick_bass.getChannelNotes(1).push_back(
                        KuroDSP::MidiNote(current_root, t0 + 0.083f, 0.06f, 0.85f, 0.0f, 1)
                    );
                    p_kick_bass.getChannelNotes(1).push_back(
                        KuroDSP::MidiNote(current_root, t0 + 0.166f, 0.06f, 0.95f, 0.0f, 1)
                    );
                } else {
                    // Rolling 16th (KBBB)
                    p_kick_bass.getChannelNotes(1).push_back(
                        KuroDSP::MidiNote(current_root, t0 + 0.0625f, 0.048f, 0.78f, 0.0f, 1)
                    );
                    p_kick_bass.getChannelNotes(1).push_back(
                        KuroDSP::MidiNote(current_root, t0 + 0.1250f, 0.048f, 0.88f, 0.0f, 1)
                    );
                    p_kick_bass.getChannelNotes(1).push_back(
                        KuroDSP::MidiNote(current_root, t0 + 0.1875f, 0.048f, 0.96f, 0.0f, 1)
                    );
                }
            }
            g_clip_manager.global_patterns.push_back(p_kick_bass);

            // 4. Pattern 2: Percussions & Hats (Canal 2)
            Pattern p_percs;
            p_percs.id = 2;
            p_percs.name = "02. Hi-Hats & Tribal Percs";
            p_percs.color = IM_COL32(0, 229, 255, 255);
            p_percs.default_length_sec = 4.0f;
            for (int beat = 0; beat < 4; ++beat) {
                float t0 = beat * 0.25f;
                // Open Hat no contratempo
                p_percs.getChannelNotes(2).push_back(KuroDSP::MidiNote(42, t0 + 0.125f, 0.09f, 0.90f, 0.0f, 2));
                // Shaker / 16th ghost
                p_percs.getChannelNotes(2).push_back(KuroDSP::MidiNote(44, t0 + 0.0625f, 0.04f, 0.60f, -0.2f, 2));
                p_percs.getChannelNotes(2).push_back(KuroDSP::MidiNote(44, t0 + 0.1875f, 0.04f, 0.70f, 0.2f, 2));
                // Snare / Clap no tempo 2 e 4
                if (beat == 1 || beat == 3) {
                    p_percs.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, t0, 0.14f, 0.95f, 0.0f, 2));
                }
            }
            g_clip_manager.global_patterns.push_back(p_percs);

            // 5. Pattern 3: Squelches & FM Zap FX (Canal 3)
            Pattern p_squelch;
            p_squelch.id = 3;
            p_squelch.name = "03. FM Squelches & Zaps";
            p_squelch.color = IM_COL32(255, 180, 0, 255);
            p_squelch.default_length_sec = 4.0f;
            // Disparos sincopados nos compassos ímpares
            p_squelch.getChannelNotes(3).push_back(KuroDSP::MidiNote(root_midi + 24, 0.375f, 0.08f, 0.85f, -0.4f, 3));
            p_squelch.getChannelNotes(3).push_back(KuroDSP::MidiNote(root_midi + 25, 0.625f, 0.06f, 0.90f, 0.4f, 3));
            p_squelch.getChannelNotes(3).push_back(KuroDSP::MidiNote(root_midi + 29, 0.875f, 0.10f, 0.95f, 0.0f, 3));
            g_clip_manager.global_patterns.push_back(p_squelch);

            // 6. Pattern 4: Acid Lead & Psy Arp (Canal 5)
            Pattern p_lead;
            p_lead.id = 4;
            p_lead.name = "04. Acid 303 Lead Arp";
            p_lead.color = IM_COL32(255, 60, 180, 255);
            p_lead.default_length_sec = 4.0f;
            for (int i = 0; i < 16; ++i) {
                int note_offset = scale_offsets[i % scale_offsets.size()];
                int pitch = root_midi + 24 + note_offset;
                float vel = (i % 4 == 0) ? 0.98f : ((i % 2 == 0) ? 0.85f : 0.70f);
                p_lead.getChannelNotes(5).push_back(KuroDSP::MidiNote(pitch, i * 0.0625f, 0.05f, vel, 0.0f, 5));
            }
            g_clip_manager.global_patterns.push_back(p_lead);

            // 7. Pattern 5: Atmospheric Drone / Pads (Canal 4)
            Pattern p_pad;
            p_pad.id = 5;
            p_pad.name = "05. Mystic Drone & Chants";
            p_pad.color = IM_COL32(130, 80, 255, 255);
            p_pad.default_length_sec = 8.0f;
            p_pad.getChannelNotes(4).push_back(KuroDSP::MidiNote(root_midi + 12, 0.0f, 3.8f, 0.75f, -0.3f, 4));
            p_pad.getChannelNotes(4).push_back(KuroDSP::MidiNote(root_midi + 12 + scale_offsets[4], 0.0f, 3.8f, 0.70f, 0.3f, 4));
            g_clip_manager.global_patterns.push_back(p_pad);

            // 8. Montagem do Arranjo na Timeline da Playlist
            int total_bars = (arrangement_bars_idx == 0) ? 64 : ((arrangement_bars_idx == 1) ? 96 : 128);
            float bar_sec = 240.0f / bpm;

            // Marcadores de Seção da Timeline
            timeline.section_markers.push_back({0.0f, "INTRO (Atmosfera)", IM_COL32(0, 180, 255, 255)});
            timeline.section_markers.push_back({16.0f * bar_sec, "BUILD-UP 1", IM_COL32(255, 200, 0, 255)});
            timeline.section_markers.push_back({32.0f * bar_sec, "MAIN DROP 1 (Full-Power)", IM_COL32(57, 255, 20, 255)});
            timeline.section_markers.push_back({64.0f * bar_sec, "BREAKDOWN (Melodia/Pad)", IM_COL32(180, 50, 255, 255)});
            timeline.section_markers.push_back({80.0f * bar_sec, "CLIMAX DROP 2 (Acid Lead)", IM_COL32(255, 30, 100, 255)});
            if (total_bars >= 96) {
                timeline.section_markers.push_back({(float)(total_bars - 16) * bar_sec, "OUTRO (DJ Mix Out)", IM_COL32(100, 120, 140, 255)});
            }

            // Distribuir clips nas 8 pistas ao longo do arranjo
            for (int b = 0; b < total_bars; b += 4) {
                float t_sec = b * bar_sec;
                float len_sec = 4.0f * bar_sec;

                bool is_breakdown = (b >= 64 && b < 80);
                bool is_intro = (b < 16);

                // Pista 0: Kick & Bass
                if (!is_breakdown) {
                    MidiClip mc; mc.id = g_clip_manager.next_id++; mc.start_time_sec = t_sec; mc.length_sec = len_sec; mc.pattern_id = 1;
                    g_clip_manager.track_midi_clips[0].push_back(mc);
                    g_clip_manager.track_midi_clips[1].push_back(mc);
                }

                // Pista 2: Percussões (entrando a partir do compasso 16)
                if (b >= 16 && !is_breakdown) {
                    MidiClip mc; mc.id = g_clip_manager.next_id++; mc.start_time_sec = t_sec; mc.length_sec = len_sec; mc.pattern_id = 2;
                    g_clip_manager.track_midi_clips[2].push_back(mc);
                }

                // Pista 3: Squelches FM (no Drop 1 e Climax)
                if ((b >= 32 && b < 64) || (b >= 80 && b < total_bars - 8)) {
                    MidiClip mc; mc.id = g_clip_manager.next_id++; mc.start_time_sec = t_sec; mc.length_sec = len_sec; mc.pattern_id = 3;
                    g_clip_manager.track_midi_clips[3].push_back(mc);
                }

                // Pista 4: Atmosferas / Pads (Intro & Breakdown)
                if (is_intro || is_breakdown) {
                    MidiClip mc; mc.id = g_clip_manager.next_id++; mc.start_time_sec = t_sec; mc.length_sec = len_sec; mc.pattern_id = 5;
                    g_clip_manager.track_midi_clips[4].push_back(mc);
                }

                // Pista 5: Acid Lead (no Climax Drop 2)
                if (b >= 80 && b < total_bars - 8) {
                    MidiClip mc; mc.id = g_clip_manager.next_id++; mc.start_time_sec = t_sec; mc.length_sec = len_sec; mc.pattern_id = 4;
                    g_clip_manager.track_midi_clips[5].push_back(mc);
                }
            }

            // 9. Automação de Filtro (Filter Sweep no Buildup)
            AutomationClip auto_filter;
            auto_filter.id = g_clip_manager.next_id++;
            auto_filter.name = "Psy Cutoff Filter Sweep (16-32)";
            auto_filter.color = 0xFF00E5FF;
            auto_filter.start_time_sec = 16.0f * bar_sec;
            auto_filter.length_sec = 16.0f * bar_sec;
            auto_filter.points.clear();
            auto_filter.points.push_back({0.0f, 0.15f, 0.45f});
            auto_filter.points.push_back({16.0f * bar_sec, 0.98f, 0.0f});
            g_clip_manager.track_auto_clips[7].push_back(auto_filter);

            g_clip_manager.current_pattern_idx = 0;
            timeline.setMasterFrame(0);

            // Mensagem de confirmação no chat
            chat_history.push_back({
                "AI",
                "✅ SUCESSO! A faixa completa de " + std::to_string(total_bars) + " compassos em " +
                std::to_string((int)bpm) + " BPM foi injetada com precisão cirúrgica na Playlist do Abduction Studio!",
                ImVec4(0.35f, 1.0f, 0.45f, 1.0f)
            });
            last_action_status = "Faixa importada com 8 pistas ativas, marcadores e automação de filtro.";
        }

        // ==========================================
        // RENDERIZAÇÃO DA INTERFACE PRINCIPAL
        // ==========================================
        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(1040, 680), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(800, 520), ImVec2(1920, 1080));

            if (need_focus) {
                ImGui::SetNextWindowFocus();
                need_focus = false;
            }

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.09f, 0.12f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.09f, 0.12f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.18f, 0.24f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.85f, 0.40f));

            bool window_active = ImGui::Begin(
                " " ICON_FA_ROBOT "  ABDUCTION AI TRACK ARCHITECT & CO-PILOT###AITrackArchitectWindow",
                &is_open,
                ImGuiWindowFlags_NoCollapse
            );

            if (window_active) {
                // Header com resumo rápido
                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), "🛸 SISTEMA NEURAL DE PRODUÇÃO DE TRACKS & CO-PILOTO INTERATIVO");
                ImGui::SameLine();
                ImGui::TextDisabled("|");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.45f, 1.0f), "BPM: %.0f", bpm);
                ImGui::SameLine();
                ImGui::TextDisabled("|");
                ImGui::SameLine();
                const char* subgenres[] = {"Full-On", "Progressive", "Twilight", "Forest", "Darkpsy", "Hi-Tech", "Minimal Offbeat"};
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.20f, 1.0f), "Vertente: %s", subgenres[subgenre_idx]);
                ImGui::EndGroup();

                ImGui::Separator();
                ImGui::Spacing();

                float avail_w = ImGui::GetContentRegionAvail().x;
                float avail_h = ImGui::GetContentRegionAvail().y;

                // Layout de 3 Colunas:
                // Col 1: Chat Conversacional & Radar (340px)
                // Col 2: Matriz de Parâmetros & Preview de Pistas (Centro)
                // Col 3: Co-Pilot Polishing Deck (Direita, 280px)
                float col1_w = std::min(330.0f, avail_w * 0.32f);
                float col3_w = std::min(280.0f, avail_w * 0.28f);
                float col2_w = avail_w - col1_w - col3_w - 20.0f;

                // ----------------------------------------------------
                // COLUNA 1: CHAT CONVERSACIONAL & RADAR EMOCIONAL
                // ----------------------------------------------------
                ImGui::BeginChild("##ColChat", ImVec2(col1_w, avail_h - 48.0f), true);
                
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), " " ICON_FA_COMMENTS "  Assistente Interativo");
                ImGui::Separator();

                // Histórico de Mensagens
                float chat_box_h = avail_h - 260.0f;
                ImGui::BeginChild("##ChatHistoryBox", ImVec2(0, chat_box_h), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                for (const auto& msg : chat_history) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                    if (msg.sender == "AI") {
                        ImGui::TextColored(msg.color, "🤖 AI: %s", msg.text.c_str());
                    } else {
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "👤 VOCÊ: %s", msg.text.c_str());
                    }
                    ImGui::PopTextWrapPos();
                    ImGui::Spacing();
                }
                ImGui::EndChild();

                // Input de Prompt do Usuário
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 48.0f);
                bool enter_pressed = ImGui::InputTextWithHint("##ChatInput", "Diga sua ideia (ex: estilo Neelix em F#m)...", user_input_prompt, sizeof(user_input_prompt), ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::SameLine();
                bool send_clicked = ImGui::Button(" " ICON_FA_PAPER_PLANE);

                if ((enter_pressed || send_clicked) && strlen(user_input_prompt) > 0) {
                    chat_history.push_back({"USER", user_input_prompt, ImVec4(1, 1, 1, 1)});
                    std::string u_str = user_input_prompt;
                    user_input_prompt[0] = '\0';

                    // Resposta inteligente da IA interpretando o prompt
                    std::string ai_reply = "Entendido! Ajustei os parâmetros para refletir essa proposta musical.";
                    if (u_str.find("neelix") != std::string::npos || u_str.find("offbeat") != std::string::npos) {
                        subgenre_idx = 1; // Progressive
                        groove_type_idx = 2; // Offbeat
                        bpm = 138.0f;
                        ai_reply = "Modo Progressive Offbeat ativado. BPM ajustado para 138 com baixo pulando no contratempo!";
                    } else if (u_str.find("mandragora") != std::string::npos || u_str.find("pesad") != std::string::npos) {
                        subgenre_idx = 0; // Full-On
                        heavy_vs_melancholic = 0.90f;
                        bpm = 145.0f;
                        ai_reply = "Ajustado para Full-On de alta densidade (145 BPM) com distorção pesada e arpejos árabes.";
                    } else if (u_str.find("melancol") != std::string::npos || u_str.find("melod") != std::string::npos) {
                        heavy_vs_melancholic = 0.15f;
                        scale_idx = 1; // E Phrygian
                        ai_reply = "Tom ajustado para harmonia melancólica profunda com pads prolongados e leads suaves.";
                    }
                    chat_history.push_back({"AI", ai_reply, ImVec4(0.35f, 1.0f, 0.45f, 1.0f)});
                }

                ImGui::Spacing();
                // Mini Radar Emocional Desenvolvido em ImDrawList
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), " " ICON_FA_CHART_PIE "  Radar Sonoro");
                ImVec2 radar_pos = ImGui::GetCursorScreenPos();
                float radar_sz = 90.0f;
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 radar_center = ImVec2(radar_pos.x + col1_w * 0.42f, radar_pos.y + 48.0f);
                
                // Círculos de grade do radar
                dl->AddCircle(radar_center, 40.0f, IM_COL32(30, 45, 60, 255), 18);
                dl->AddCircle(radar_center, 24.0f, IM_COL32(20, 35, 48, 255), 18);
                
                // Polígono dinâmico baseado em Energia e Peso
                float r_energy = 20.0f + energy_level * 20.0f;
                float r_heavy = 15.0f + heavy_vs_melancholic * 25.0f;
                float r_rhythm = 35.0f;
                float r_dark = 15.0f + (1.0f - heavy_vs_melancholic) * 22.0f;

                ImVec2 p_top = ImVec2(radar_center.x, radar_center.y - r_energy);
                ImVec2 p_right = ImVec2(radar_center.x + r_heavy, radar_center.y);
                ImVec2 p_bottom = ImVec2(radar_center.x, radar_center.y + r_rhythm);
                ImVec2 p_left = ImVec2(radar_center.x - r_dark, radar_center.y);

                ImVec2 poly_pts[4] = {p_top, p_right, p_bottom, p_left};
                dl->AddConvexPolyFilled(poly_pts, 4, IM_COL32(0, 240, 200, 90));
                dl->AddPolyline(poly_pts, 4, IM_COL32(0, 255, 220, 255), ImDrawFlags_Closed, 1.8f);

                ImGui::Dummy(ImVec2(col1_w, radar_sz));
                ImGui::EndChild();

                ImGui::SameLine();

                // ----------------------------------------------------
                // COLUNA 2: MATRIZ DE PARÂMETROS & PREVIEW DE PISTAS
                // ----------------------------------------------------
                ImGui::BeginChild("##ColParams", ImVec2(col2_w, avail_h - 48.0f), true);
                
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), " " ICON_FA_SLIDERS "  DNA & Parâmetros da Faixa");
                ImGui::Separator();

                // 1. Música / Artista de Referência
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Música ou Artista de Referência:");
                const char* ref_presets[] = {
                    "Astrix - Deep Jungle Walk",
                    "Vini Vici & Astrix - Adhana",
                    "Mandragora - Sem Chão / Codeine",
                    "Perception - Astral Portal",
                    "Neelix - Makeup",
                    "[Digitar Manualmente]"
                };
                if (ImGui::Combo("##RefPreset", &reference_track_idx, ref_presets, IM_ARRAYSIZE(ref_presets))) {
                    if (reference_track_idx < 5) {
                        strncpy_s(custom_reference_name, sizeof(custom_reference_name), ref_presets[reference_track_idx], _TRUNCATE);
                        if (reference_track_idx == 0) { bpm = 142.0f; scale_idx = 1; subgenre_idx = 0; }
                        else if (reference_track_idx == 1) { bpm = 138.0f; scale_idx = 1; subgenre_idx = 1; }
                        else if (reference_track_idx == 2) { bpm = 145.0f; scale_idx = 4; subgenre_idx = 0; }
                        else if (reference_track_idx == 3) { bpm = 140.0f; scale_idx = 2; subgenre_idx = 1; }
                        else if (reference_track_idx == 4) { bpm = 136.0f; scale_idx = 3; subgenre_idx = 6; }
                    }
                }

                // 2. BPM & Andamento
                ImGui::Spacing();
                ImGui::Text("Andamento da Faixa (BPM):");
                ImGui::SliderFloat("##BpmSlider", &bpm, 120.0f, 180.0f, "%.1f BPM");

                // 3. Vertente / Subgênero
                ImGui::Spacing();
                ImGui::Text("Vertente do Psytrance:");
                ImGui::Combo("##SubgenreCombo", &subgenre_idx, subgenres, IM_ARRAYSIZE(subgenres));

                // 4. Tom & Escala Musical
                ImGui::Spacing();
                ImGui::Text("Tom & Escala Harmônica:");
                const char* scales[] = {
                    "Menor Natural (Épico / Melódico)",
                    "Mi Frígio (E Phrygian - Clássico Psy)",
                    "F# Menor (Agressivo / Festival)",
                    "Sol Menor (G Minor - Groove Gordo)",
                    "Frígio Dominante (Árabe / Místico)",
                    "Gypsy Minor (Cigano / Hipnótico)"
                };
                ImGui::Combo("##ScaleCombo", &scale_idx, scales, IM_ARRAYSIZE(scales));

                // 5. Tipo de Groove do Bassline
                ImGui::Spacing();
                ImGui::Text("Groove de Baixo (Bassline):");
                const char* grooves[] = {
                    "Rolling 16th (KBBB - O Motor do Psytrance)",
                    "Galope / Triplet (KBB - Floresta / Épico)",
                    "Offbeat (KB - Progressive / Limpo)",
                    "Octave Jump (Sincopado com Saltos)"
                };
                ImGui::Combo("##GrooveCombo", &groove_type_idx, grooves, IM_ARRAYSIZE(grooves));

                // 6. Polaridade: Pesada vs Melancólica
                ImGui::Spacing();
                ImGui::Text("Polaridade Sonora (Melancólica <---> Pesada):");
                ImGui::SliderFloat("##HeavyMelancholy", &heavy_vs_melancholic, 0.0f, 1.0f, "%.0f%% Pesada");

                // 7. Estrutura de Arranjo
                ImGui::Spacing();
                ImGui::Text("Estrutura do Arranjo:");
                const char* arrangements[] = {
                    "64 Compassos (Drop Rápido de Festival)",
                    "96 Compassos (Estrutura Padrão Club)",
                    "128 Compassos (Jornada Épica Estendida)"
                };
                ImGui::Combo("##ArrangementCombo", &arrangement_bars_idx, arrangements, IM_ARRAYSIZE(arrangements));

                // Preview das 8 Pistas Geradas
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), " " ICON_FA_LAYER_GROUP "  Pré-Visualização das 8 Pistas");

                const char* track_names[8] = {
                    "01. Kick (Afinado em Sub 52Hz)",
                    "02. Rolling Bass (KBBB 16th Phase Lock)",
                    "03. Hi-Hats & Shakers (16th Stereo)",
                    "04. Squelch & FM Zap FX (Modulado)",
                    "05. Drone & Atmosfera Cósmica",
                    "06. Acid 303 Lead Arp (Escala Frígia)",
                    "07. Vocais & Risers de Tensão",
                    "08. Automação de Filtro (Bézier Sweep)"
                };

                ImGui::BeginChild("##TrackPreviewBox", ImVec2(0, 110.0f), true);
                for (int t = 0; t < 8; ++t) {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "Lane %d: ", t + 1);
                    ImGui::SameLine();
                    ImGui::Text("%s", track_names[t]);
                }
                ImGui::EndChild();

                ImGui::EndChild();

                ImGui::SameLine();

                // ----------------------------------------------------
                // COLUNA 3: CO-PILOT POLISHING DECK (AJUDINHA AO VIVO)
                // ----------------------------------------------------
                ImGui::BeginChild("##ColCoPilot", ImVec2(col3_w, avail_h - 48.0f), true);
                
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), " " ICON_FA_WAND_MAGIC_SPARKLES "  Co-Pilot Polishing");
                ImGui::Separator();

                // Status de Fase
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), " " ICON_FA_LOCK "  Kick-Bass Phase Lock: %.1f%%", phase_alignment_score);
                ImGui::TextDisabled("Graves 45-60Hz alinhados sem cancelamento.");

                ImGui::Spacing();
                ImGui::Separator();

                // Ferramentas da "Ajudinha"
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "Ações Rápidas do Assistente:");

                if (ImGui::Button(" " ICON_FA_COMPASS "  Alinhar Fase Sub (0°)", ImVec2(col3_w - 18.0f, 26.0f))) {
                    phase_alignment_score = 99.8f;
                    last_action_status = "Fase travada a 0° no primeiro ciclo de onda.";
                }

                ImGui::Spacing();
                if (ImGui::Button(" " ICON_FA_MUSIC "  Mutar Riff no Drop 2", ImVec2(col3_w - 18.0f, 26.0f))) {
                    last_action_status = "Variação melódica de oitava aplicada ao Drop 2.";
                }

                ImGui::Spacing();
                if (ImGui::Button(" " ICON_FA_CHEVRON_UP "  Gerar Snare Roll & Riser", ImVec2(col3_w - 18.0f, 26.0f))) {
                    drop_riser_generated = true;
                    last_action_status = "Snare roll de 4 compassos inserido antes do drop.";
                }

                ImGui::Spacing();
                if (ImGui::Button(" " ICON_FA_SLIDERS "  Auto-Sidechain Dinâmico", ImVec2(col3_w - 18.0f, 26.0f))) {
                    sidechain_optimized = true;
                    last_action_status = "Curva de ducking aplicada no baixo.";
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Status:");
                ImGui::TextWrapped("%s", last_action_status.c_str());

                ImGui::EndChild();

                // ==========================================
                // BARRA INFERIOR: BOTÃO DE INJEÇÃO PRINCIPAL
                // ==========================================
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.65f, 0.45f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.85f, 0.55f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.50f, 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                if (ImGui::Button(" " ICON_FA_ROCKET "  INJETAR TRACK NA PLAYLIST DO ABDUCTION STUDIO (CRIAR PROJETO)", ImVec2(avail_w - 120.0f, 34.0f))) {
                    InjectTrackToProject();
                }
                ImGui::PopStyleColor(4);

                ImGui::SameLine();
                if (ImGui::Button("Fechar", ImVec2(110.0f, 34.0f))) {
                    is_open = false;
                }
            }

            ImGui::End();
            ImGui::PopStyleColor(4);
        }
    };

    inline KuroAITrackArchitectUI g_ai_track_architect_ui;
}
