#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <cmath>
#include "ChannelInstrumentManager.h"
#include "../plugin_manager/ThematicSynths.h"

extern KuroDSP::PsytranceRollingBassSynth g_psy_bass_synth;

namespace KuroUI {

    extern bool show_playlist;

    struct RecentProjectCardItem {
        std::string title;
        std::string file_path;
        int bpm;
        std::string modified_text;
        int wave_style; // 0: Pulsar (3-lobe), 1: Neon Chase (dense), 2: Cybernetic Drift (spiky)
        ImU32 badge_color;
    };

    struct QuickTemplateCardItem {
        std::string title;
        std::string subtitle;
        int bpm;
        int icon_type; // 0: Alien head, 1: Microchip, 2: Pulse wave
        ImU32 theme_color;
    };

    class HomeDashboardUI {
    public:
        bool is_open = false; // Desativado na inicialização
        bool show_on_startup = false; // Desativado na inicialização
    private:
        bool target_open = false; // Desativado na inicialização
        float transition_progress = 0.0f; // 0.0f a 1.0f (transição suave de entrada/saída)
        int selected_audio_driver = 0;
        float anim_timer = 0.0f;
        std::vector<RecentProjectCardItem> recent_projects;
        std::vector<QuickTemplateCardItem> quick_templates;

        const char* audio_drivers[4] = {
            "ASIO/WASAPI 44.1kHz, 128 samples buffer",
            "ASIO/WASAPI 48.0kHz, 256 samples buffer",
            "WASAPI Exclusive 96.0kHz, 64 samples buffer",
            "DirectSound / Generic (Safe Mode)"
        };

        // Renderiza as formas de onda mini idênticas ao mockup
        void RenderMockupWaveform(ImDrawList* dl, ImVec2 p_min, ImVec2 p_max, int style, ImU32 color) {
            float width = p_max.x - p_min.x;
            float height = p_max.y - p_min.y;
            if (width < 10.0f || height < 6.0f) return;

            float cy = p_min.y + height * 0.5f;
            float hh = height * 0.44f;

            // Linha do eixo central sutil
            dl->AddLine(ImVec2(p_min.x, cy), ImVec2(p_max.x, cy), IM_COL32(0, 229, 255, 45), 1.0f);

            constexpr int num_points = 120;
            ImVec2 top_pts[num_points];
            ImVec2 bot_pts[num_points];

            if (style == 0) {
                // Style 0: "Pulsar Rhythms" (3 Lobos suaves com envelope gaussiano / harmônicos)
                for (int i = 0; i < num_points; ++i) {
                    float t = (float)i / (float)(num_points - 1);
                    float px = p_min.x + t * width;

                    // 3 lobos principais centrados em 22%, 50% e 78%
                    float g1 = expf(-powf((t - 0.22f) / 0.11f, 2.0f)) * 0.82f;
                    float g2 = expf(-powf((t - 0.50f) / 0.13f, 2.0f)) * 0.98f;
                    float g3 = expf(-powf((t - 0.78f) / 0.10f, 2.0f)) * 0.78f;
                    float ripple = 0.04f * sinf(t * 80.0f + anim_timer * 1.5f);
                    float amp = std::clamp((g1 + g2 + g3 + 0.04f + ripple), 0.04f, 1.0f);

                    top_pts[i] = ImVec2(px, cy - amp * hh);
                    bot_pts[i] = ImVec2(px, cy + amp * hh);
                }

                // Desenha polígono preenchido com gradiente translúcido
                for (int i = 0; i < num_points - 1; ++i) {
                    ImVec2 quad[4] = { top_pts[i], top_pts[i + 1], bot_pts[i + 1], bot_pts[i] };
                    dl->AddConvexPolyFilled(quad, 4, IM_COL32(0, 195, 230, 48));
                }
                // Contornos externos em ciano brilhante
                dl->AddPolyline(top_pts, num_points, IM_COL32(0, 229, 255, 210), ImDrawFlags_None, 1.4f);
                dl->AddPolyline(bot_pts, num_points, IM_COL32(0, 229, 255, 210), ImDrawFlags_None, 1.4f);

            } else if (style == 1) {
                // Style 1: "Neon Chase" (Densidade sonora pesada com modulação contínua)
                int num_bars = 70;
                float bar_step = width / (float)num_bars;
                float bar_w = std::max(bar_step * 0.72f, 1.5f);

                for (int i = 0; i < num_bars; ++i) {
                    float t = (float)i / (float)num_bars;
                    float px = p_min.x + i * bar_step;

                    // Envelope contínuo com picos de energia
                    float env = (sinf(t * 3.14159f) + 0.35f) * 0.72f;
                    float density = 0.45f + 0.55f * fabsf(sinf(t * 45.0f + 0.3f) * cosf(t * 19.0f));
                    float amp = std::clamp(env * density, 0.08f, 0.95f);

                    float y0 = cy - amp * hh;
                    float y1 = cy + amp * hh;

                    dl->AddRectFilled(ImVec2(px, y0), ImVec2(px + bar_w, y1), IM_COL32(0, 210, 245, 175), 1.0f);
                }
                // Linha de playhead/energia central
                dl->AddLine(ImVec2(p_min.x, cy), ImVec2(p_max.x, cy), IM_COL32(255, 255, 255, 120), 1.0f);

            } else {
                // Style 2: "Cybernetic Drift" (Picos dinâmicos com transientes acentuados de Psytrance)
                int num_bars = 64;
                float bar_step = width / (float)num_bars;
                float bar_w = std::max(bar_step * 0.70f, 1.5f);

                for (int i = 0; i < num_bars; ++i) {
                    float t = (float)i / (float)num_bars;
                    float px = p_min.x + i * bar_step;

                    float step16 = fmodf(t * 16.0f, 1.0f);
                    float spike = (step16 < 0.22f) ? (1.0f - step16 * 3.5f) : (0.28f + 0.18f * sinf(t * 70.0f));
                    float macro = 0.35f + 0.65f * fabsf(sinf(t * 3.14159f * 2.2f));
                    float amp = std::clamp(spike * macro, 0.06f, 0.96f);

                    float y0 = cy - amp * hh;
                    float y1 = cy + amp * hh;

                    // Alterna sutilmente o brilho dos transientes
                    ImU32 bar_c = (spike > 0.7f) ? IM_COL32(0, 245, 255, 240) : IM_COL32(0, 180, 220, 150);
                    dl->AddRectFilled(ImVec2(px, y0), ImVec2(px + bar_w, y1), bar_c, 1.0f);
                }
            }
        }

        // Renderiza os ícones vetoriais exclusivos dos Templates
        void RenderTemplateIcon(ImDrawList* dl, ImVec2 p0, ImVec2 p1, int icon_type, ImU32 theme_color) {
            float w = p1.x - p0.x;
            float h = p1.y - p0.y;
            ImVec2 c = ImVec2(p0.x + w * 0.5f, p0.y + h * 0.5f);

            // Fundo escuro do ícone com borda de acento
            dl->AddRectFilled(p0, p1, IM_COL32(10, 18, 26, 255), 8.0f);
            dl->AddRect(p0, p1, theme_color, 8.0f, 0, 1.5f);

            if (icon_type == 0) {
                // Ícone Alien / Psytrance: Cabeça alienígena com ondas sonoras (( 👽 ))
                // Ondas sonoras na esquerda e direita
                dl->AddCircle(c, 18.0f, theme_color, 24, 1.5f);
                dl->AddCircle(c, 23.0f, (theme_color & 0x00FFFFFF) | 0x80000000, 24, 1.2f);
                
                // Mascara de fundo para a cabeça alien
                dl->AddCircleFilled(c, 14.0f, IM_COL32(10, 18, 26, 255), 20);

                // Contorno da cabeça alienígena (queixo afilado)
                ImVec2 head_top = ImVec2(c.x, c.y - 12.0f);
                ImVec2 head_l = ImVec2(c.x - 10.0f, c.y - 3.0f);
                ImVec2 head_r = ImVec2(c.x + 10.0f, c.y - 3.0f);
                ImVec2 head_chin = ImVec2(c.x, c.y + 11.0f);

                dl->AddBezierCubic(head_l, ImVec2(c.x - 11.0f, c.y - 12.0f), ImVec2(c.x + 11.0f, c.y - 12.0f), head_r, theme_color, 1.6f);
                dl->AddLine(head_l, head_chin, theme_color, 1.6f);
                dl->AddLine(head_r, head_chin, theme_color, 1.6f);

                // Olhos ovais inclinados para o centro
                ImVec2 eye_l1 = ImVec2(c.x - 6.5f, c.y - 3.0f);
                ImVec2 eye_l2 = ImVec2(c.x - 2.5f, c.y + 1.0f);
                dl->AddLine(eye_l1, eye_l2, theme_color, 2.8f);

                ImVec2 eye_r1 = ImVec2(c.x + 6.5f, c.y - 3.0f);
                ImVec2 eye_r2 = ImVec2(c.x + 2.5f, c.y + 1.0f);
                dl->AddLine(eye_r1, eye_r2, theme_color, 2.8f);

            } else if (icon_type == 1) {
                // Ícone Hi-Tech Cyber: Circuito integrado / Microchip com trilhas e pads
                float chip_sz = 14.0f;
                ImVec2 cp0 = ImVec2(c.x - chip_sz, c.y - chip_sz);
                ImVec2 cp1 = ImVec2(c.x + chip_sz, c.y + chip_sz);
                dl->AddRect(cp0, cp1, theme_color, 3.0f, 0, 1.6f);
                dl->AddRectFilled(ImVec2(c.x - 6.0f, c.y - 6.0f), ImVec2(c.x + 6.0f, c.y + 6.0f), theme_color, 1.5f);

                // Trilhas com pads circulares
                float pins[3] = { -8.0f, 0.0f, 8.0f };
                for (int p = 0; p < 3; ++p) {
                    // Cima e baixo
                    dl->AddLine(ImVec2(c.x + pins[p], cp0.y), ImVec2(c.x + pins[p], cp0.y - 7.0f), theme_color, 1.2f);
                    dl->AddCircleFilled(ImVec2(c.x + pins[p], cp0.y - 7.0f), 1.6f, theme_color);

                    dl->AddLine(ImVec2(c.x + pins[p], cp1.y), ImVec2(c.x + pins[p], cp1.y + 7.0f), theme_color, 1.2f);
                    dl->AddCircleFilled(ImVec2(c.x + pins[p], cp1.y + 7.0f), 1.6f, theme_color);

                    // Esquerda e direita
                    dl->AddLine(ImVec2(cp0.x, c.y + pins[p]), ImVec2(cp0.x - 7.0f, c.y + pins[p]), theme_color, 1.2f);
                    dl->AddCircleFilled(ImVec2(cp0.x - 7.0f, c.y + pins[p]), 1.6f, theme_color);

                    dl->AddLine(ImVec2(cp1.x, c.y + pins[p]), ImVec2(cp1.x + 7.0f, c.y + pins[p]), theme_color, 1.2f);
                    dl->AddCircleFilled(ImVec2(cp1.x + 7.0f, c.y + pins[p]), 1.6f, theme_color);
                }

            } else {
                // Ícone Peak-Time Techno: Pulso de onda / ECG estrobo minimalista
                dl->AddLine(ImVec2(c.x - 22.0f, c.y), ImVec2(c.x - 10.0f, c.y), theme_color, 2.0f);
                dl->AddLine(ImVec2(c.x - 10.0f, c.y), ImVec2(c.x - 5.0f, c.y - 15.0f), theme_color, 2.2f);
                dl->AddLine(ImVec2(c.x - 5.0f, c.y - 15.0f), ImVec2(c.x + 3.0f, c.y + 13.0f), theme_color, 2.2f);
                dl->AddLine(ImVec2(c.x + 3.0f, c.y + 13.0f), ImVec2(c.x + 8.0f, c.y - 4.0f), theme_color, 2.0f);
                dl->AddLine(ImVec2(c.x + 8.0f, c.y - 4.0f), ImVec2(c.x + 12.0f, c.y), theme_color, 2.0f);
                dl->AddLine(ImVec2(c.x + 12.0f, c.y), ImVec2(c.x + 22.0f, c.y), theme_color, 2.0f);
            }
        }

    public:
        // Aplica um template instantâneo no projeto com roteamento e síntese direta de áudio
        void LoadTemplate(int index, KuroDSP::TimelineManager& timeline, ClipManager& clip_manager) {
            std::lock_guard<std::mutex> lock(timeline.timeline_mutex);
            clip_manager.reset();
            timeline.setMasterFrame(0);
            timeline.setPlaying(false);

            // Reseta todos os clipes de todas as faixas
            for (int t = 0; t < MAX_TRACKS; ++t) {
                clip_manager.track_clips[t].clear();
                clip_manager.track_midi_clips[t].clear();
            }
            clip_manager.global_patterns.clear();

            if (index == 0) { // Full-On Psytrance (148 BPM)
                timeline.setBPM(148.0f);
                timeline.is_pattern_mode = false; // Comuta para SONG MODE para tocar o arranjo da Playlist

                // Nomes das trilhas
                ::track_names[0] = "01 Kick 4x4";
                ::track_names[1] = "02 Rolling Bass";
                ::track_names[2] = "03 Psy Clap";
                ::track_names[3] = "04 Open Hat";

                // Slots de Instrumento
                g_channel_slots[0].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[0].sample_name = "Astrix Punch Kick";
                g_channel_slots[0].custom_color = IM_COL32(46, 204, 113, 255);

                g_channel_slots[1].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[1].sample_name = "Rolling Bass";
                g_channel_slots[1].kuro_layer_tab = 2;
                g_channel_slots[1].custom_color = IM_COL32(0, 229, 255, 255);

                g_channel_slots[2].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[2].sample_name = "808 Crisp Clap";
                g_channel_slots[2].custom_color = IM_COL32(230, 70, 200, 255);

                g_channel_slots[3].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[3].sample_name = "Open Psy Hat";
                g_channel_slots[3].custom_color = IM_COL32(255, 171, 0, 255);

                // Carregar samples no motor com fallbacks seguros
                if (std::filesystem::exists("assets/samples/adhana_signature/Adhana_Astrix_Kick_Punch_01.wav")) {
                    g_piano_synth.loadTrackSample(0, "assets/samples/adhana_signature/Adhana_Astrix_Kick_Punch_01.wav");
                } else if (std::filesystem::exists("assets/samples/Psytrance_Kick_140BPM.wav")) {
                    g_piano_synth.loadTrackSample(0, "assets/samples/Psytrance_Kick_140BPM.wav");
                }

                if (std::filesystem::exists("assets/samples/808_Crisp_Clap.wav")) {
                    g_piano_synth.loadTrackSample(2, "assets/samples/808_Crisp_Clap.wav");
                }
                if (std::filesystem::exists("assets/samples/Open_Psy_Hat.wav")) {
                    g_piano_synth.loadTrackSample(3, "assets/samples/Open_Psy_Hat.wav");
                }

                // Calibrar motor de síntese de Rolling Bass para Full-On Psytrance
                ::g_psy_bass_synth.setSynthModel(0); // Kuro Moog 24dB
                ::g_psy_bass_synth.setOscType(0);   // Psy Sawtooth
                ::g_psy_bass_synth.setCutoff(480.0f);
                ::g_psy_bass_synth.setResonance(0.38f);
                ::g_psy_bass_synth.setEnvDecay(0.082f);
                ::g_psy_bass_synth.setEnvAmount(0.85f);
                ::g_psy_bass_synth.setSubLevel(0.42f);
                ::g_psy_bass_synth.setTransientClick(0.45f);
                ::g_psy_bass_synth.setDrive(0.30f);
                ::g_psy_bass_synth.setHpfCutoff(32.0f);

                Pattern p;
                p.id = 1;
                p.name = "01. Psytrance Kick & Rolling Bass";
                p.color = IM_COL32(46, 204, 113, 255);

                // 4-on-the-floor kick
                for (int step = 0; step < 16; step += 4) {
                    p.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, step * 0.125f, 0.10f, 1.0f, 1.0f, 0));
                }
                // Rolling Bass K-B-B-B (steps 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15)
                for (int step = 0; step < 16; ++step) {
                    if (step % 4 != 0) {
                        p.getChannelNotes(1).push_back(KuroDSP::MidiNote(36, step * 0.125f, 0.08f, 0.95f, 1.0f, 1));
                    }
                }
                // Clap nos tempos 2 e 4 (steps 4 e 12)
                p.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, 4 * 0.125f, 0.10f, 0.90f, 1.0f, 2));
                p.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, 12 * 0.125f, 0.10f, 0.90f, 1.0f, 2));

                // Open Hat no contratempo (offbeat: steps 2, 6, 10, 14)
                for (int step = 2; step < 16; step += 4) {
                    p.getChannelNotes(3).push_back(KuroDSP::MidiNote(42, step * 0.125f, 0.08f, 0.85f, 1.0f, 3));
                }

                clip_manager.global_patterns.push_back(p);
                clip_manager.current_pattern_idx = 0;

                // Povoa 8 compassos na playlist com faixas separadas (Arranjador completo)
                float bar_dur = 240.0f / 148.0f;
                for (int b = 0; b < 8; ++b) {
                    for (int trk = 0; trk < 4; ++trk) {
                        MidiClip mc;
                        mc.id = clip_manager.next_id++;
                        mc.pattern_id = 1;
                        mc.start_time_sec = b * bar_dur;
                        mc.length_sec = bar_dur;
                        mc.is_selected = false;
                        clip_manager.track_midi_clips[trk].push_back(mc);
                    }
                }
                timeline.loop_enabled = true;
                timeline.loop_start_sec = 0.0f;
                timeline.loop_end_sec = 8.0f * bar_dur;

            } else if (index == 1) { // Hi-Tech Cyber (160 BPM)
                timeline.setBPM(160.0f);
                timeline.is_pattern_mode = false;

                ::track_names[0] = "01 Cyber Kick";
                ::track_names[1] = "02 Hi-Tech Bass";
                ::track_names[2] = "03 Snare Click";
                ::track_names[3] = "04 Closed Hat";

                g_channel_slots[0].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[0].sample_name = "Cyber Sub Impact";
                g_channel_slots[0].custom_color = IM_COL32(32, 215, 175, 255);

                g_channel_slots[1].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[1].sample_name = "Hi-Tech Virus Bass";
                g_channel_slots[1].kuro_layer_tab = 2;
                g_channel_slots[1].custom_color = IM_COL32(0, 229, 255, 255);

                g_channel_slots[2].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[2].sample_name = "Ghost Snare Click";
                g_channel_slots[2].custom_color = IM_COL32(180, 100, 240, 255);

                g_channel_slots[3].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[3].sample_name = "Closed Metal Hat";
                g_channel_slots[3].custom_color = IM_COL32(0, 200, 255, 255);

                if (std::filesystem::exists("assets/samples/Cyber_Sub_Impact.wav")) {
                    g_piano_synth.loadTrackSample(0, "assets/samples/Cyber_Sub_Impact.wav");
                } else if (std::filesystem::exists("assets/samples/adhana_signature/Adhana_Astrix_Kick_Punch_01.wav")) {
                    g_piano_synth.loadTrackSample(0, "assets/samples/adhana_signature/Adhana_Astrix_Kick_Punch_01.wav");
                }

                if (std::filesystem::exists("assets/samples/Ghost_Snare_Click.wav")) {
                    g_piano_synth.loadTrackSample(2, "assets/samples/Ghost_Snare_Click.wav");
                }
                if (std::filesystem::exists("assets/samples/Closed_Metal_Hat.wav")) {
                    g_piano_synth.loadTrackSample(3, "assets/samples/Closed_Metal_Hat.wav");
                }

                ::g_psy_bass_synth.setSynthModel(1); // Virus TI Hyper-Saw
                ::g_psy_bass_synth.setOscType(0);
                ::g_psy_bass_synth.setCutoff(650.0f);
                ::g_psy_bass_synth.setResonance(0.42f);
                ::g_psy_bass_synth.setEnvDecay(0.060f);
                ::g_psy_bass_synth.setEnvAmount(0.90f);
                ::g_psy_bass_synth.setSubLevel(0.45f);
                ::g_psy_bass_synth.setTransientClick(0.55f);
                ::g_psy_bass_synth.setDrive(0.35f);
                ::g_psy_bass_synth.setHpfCutoff(36.0f);

                Pattern p;
                p.id = 1;
                p.name = "01. Hi-Tech Cyber Matrix";
                p.color = IM_COL32(32, 215, 175, 255);

                // Kick 4x4
                for (int step = 0; step < 16; step += 4) {
                    p.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, step * 0.125f, 0.08f, 1.0f, 1.0f, 0));
                }
                // Hi-Tech Galloping Bassline (160 BPM)
                int hi_pitches[16] = {36, 36, 36, 36, 36, 36, 36, 36, 38, 38, 38, 38, 36, 36, 38, 38};
                for (int step = 0; step < 16; ++step) {
                    if (step % 4 != 0) {
                        p.getChannelNotes(1).push_back(KuroDSP::MidiNote(hi_pitches[step], step * 0.125f, 0.06f, 0.95f, 1.0f, 1));
                    }
                }
                // Snare nos tempos 2 e 4
                p.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, 4 * 0.125f, 0.08f, 0.90f, 1.0f, 2));
                p.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, 12 * 0.125f, 0.08f, 0.90f, 1.0f, 2));

                // Closed Hat semicorcheias
                for (int step = 0; step < 16; step += 2) {
                    p.getChannelNotes(3).push_back(KuroDSP::MidiNote(42, step * 0.125f, 0.05f, 0.85f, 1.0f, 3));
                }

                clip_manager.global_patterns.push_back(p);
                clip_manager.current_pattern_idx = 0;

                float bar_dur = 240.0f / 160.0f;
                for (int b = 0; b < 8; ++b) {
                    for (int trk = 0; trk < 4; ++trk) {
                        MidiClip mc;
                        mc.id = clip_manager.next_id++;
                        mc.pattern_id = 1;
                        mc.start_time_sec = b * bar_dur;
                        mc.length_sec = bar_dur;
                        mc.is_selected = false;
                        clip_manager.track_midi_clips[trk].push_back(mc);
                    }
                }
                timeline.loop_enabled = true;
                timeline.loop_start_sec = 0.0f;
                timeline.loop_end_sec = 8.0f * bar_dur;

            } else if (index == 2) { // Peak-Time Techno (135 BPM)
                timeline.setBPM(135.0f);
                timeline.is_pattern_mode = false;

                ::track_names[0] = "01 909 Heavy Kick";
                ::track_names[1] = "02 Sub Rumble Bass";
                ::track_names[2] = "03 909 Tight Snare";
                ::track_names[3] = "04 909 Ride Hat";

                g_channel_slots[0].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[0].sample_name = "Punch 909 Kick";
                g_channel_slots[0].custom_color = IM_COL32(245, 166, 35, 255);

                g_channel_slots[1].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[1].sample_name = "Analog Sub Rumble";
                g_channel_slots[1].kuro_layer_tab = 2;
                g_channel_slots[1].custom_color = IM_COL32(255, 90, 0, 255);

                g_channel_slots[2].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[2].sample_name = "909 Tight Snare";
                g_channel_slots[2].custom_color = IM_COL32(255, 130, 70, 255);

                g_channel_slots[3].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[3].sample_name = "Closed Metal Hat";
                g_channel_slots[3].custom_color = IM_COL32(255, 200, 50, 255);

                if (std::filesystem::exists("assets/samples/Real_Drums/Punch_909_Kick.wav")) {
                    g_piano_synth.loadTrackSample(0, "assets/samples/Real_Drums/Punch_909_Kick.wav");
                } else if (std::filesystem::exists("assets/samples/Clean_808_Sub_Kick.wav")) {
                    g_piano_synth.loadTrackSample(0, "assets/samples/Clean_808_Sub_Kick.wav");
                }

                if (std::filesystem::exists("assets/samples/909_Tight_Snare.wav")) {
                    g_piano_synth.loadTrackSample(2, "assets/samples/909_Tight_Snare.wav");
                }
                if (std::filesystem::exists("assets/samples/Closed_Metal_Hat.wav")) {
                    g_piano_synth.loadTrackSample(3, "assets/samples/Closed_Metal_Hat.wav");
                }

                ::g_psy_bass_synth.setSynthModel(4); // Analog Monster Sub
                ::g_psy_bass_synth.setOscType(1);   // Square/Pulse
                ::g_psy_bass_synth.setCutoff(320.0f);
                ::g_psy_bass_synth.setResonance(0.25f);
                ::g_psy_bass_synth.setEnvDecay(0.120f);
                ::g_psy_bass_synth.setEnvAmount(0.70f);
                ::g_psy_bass_synth.setSubLevel(0.60f);
                ::g_psy_bass_synth.setTransientClick(0.35f);
                ::g_psy_bass_synth.setDrive(0.45f);
                ::g_psy_bass_synth.setHpfCutoff(28.0f);

                Pattern p;
                p.id = 1;
                p.name = "01. Peak-Time Heavy 909 Beat";
                p.color = IM_COL32(245, 166, 35, 255);

                // Heavy 909 Kick
                for (int step = 0; step < 16; step += 4) {
                    p.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, step * 0.125f, 0.12f, 1.0f, 1.0f, 0));
                }
                // Driving Offbeat Sub Rumble Bass
                for (int step = 2; step < 16; step += 4) {
                    p.getChannelNotes(1).push_back(KuroDSP::MidiNote(33, step * 0.125f, 0.12f, 0.95f, 1.0f, 1));
                }
                // Snare nos tempos 2 e 4
                p.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, 4 * 0.125f, 0.10f, 0.90f, 1.0f, 2));
                p.getChannelNotes(2).push_back(KuroDSP::MidiNote(38, 12 * 0.125f, 0.10f, 0.90f, 1.0f, 2));

                // 909 Open/Ride Hat
                for (int step = 2; step < 16; step += 4) {
                    p.getChannelNotes(3).push_back(KuroDSP::MidiNote(42, step * 0.125f, 0.10f, 0.90f, 1.0f, 3));
                }

                clip_manager.global_patterns.push_back(p);
                clip_manager.current_pattern_idx = 0;

                float bar_dur = 240.0f / 135.0f;
                for (int b = 0; b < 8; ++b) {
                    for (int trk = 0; trk < 4; ++trk) {
                        MidiClip mc;
                        mc.id = clip_manager.next_id++;
                        mc.pattern_id = 1;
                        mc.start_time_sec = b * bar_dur;
                        mc.length_sec = bar_dur;
                        mc.is_selected = false;
                        clip_manager.track_midi_clips[trk].push_back(mc);
                    }
                }
                timeline.loop_enabled = true;
                timeline.loop_start_sec = 0.0f;
                timeline.loop_end_sec = 8.0f * bar_dur;
            }
        }

    public:
        HomeDashboardUI() {
            recent_projects = {
                { "Pulsar Rhythms", "projects/Pulsar_Rhythms.kuro", 148, "Modified: 2h ago", 0, IM_COL32(0, 229, 255, 255) },
                { "Neon Chase", "projects/Neon_Chase.kuro", 142, "Modified: Yesterday", 1, IM_COL32(0, 229, 255, 255) },
                { "Cybernetic Drift", "projects/Cybernetic_Drift.kuro", 160, "Modified: 3 days ago", 2, IM_COL32(0, 229, 255, 255) }
            };

            quick_templates = {
                { "Full-On Psytrance", "Aggressive basslines, high-energy synths", 148, 0, IM_COL32(46, 204, 113, 255) },
                { "Hi-Tech Cyber", "Intricate sequences, modular textures", 160, 1, IM_COL32(32, 215, 175, 255) },
                { "Peak-Time Techno", "Minimal driving beat", 135, 2, IM_COL32(245, 166, 35, 255) }
            };
        }

        bool& getOpenState() { return is_open; }
        bool isTargetOpen() const { return target_open; }

        void open() {
            is_open = true;
            target_open = true;
            if (transition_progress <= 0.0f) transition_progress = 0.02f;
        }

        void close() {
            target_open = false;
            is_open = false;
            transition_progress = 0.0f;
            show_playlist = true;
        }

        void toggle() {
            if (target_open) close();
            else open();
        }

        void Render(KuroDSP::TimelineManager& timeline, ClipManager& clip_manager) {
            float dt = ImGui::GetIO().DeltaTime;
            if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;
            anim_timer += dt;

            // Animação suave com Fade-In exponencial e Cyber Glow
            const float open_speed = 5.2f;   // ~190ms fade-in fluido
            const float close_speed = 6.8f;  // ~145ms fade-out responsivo

            if (target_open) {
                transition_progress += dt * open_speed;
                if (transition_progress > 1.0f) transition_progress = 1.0f;
            } else {
                transition_progress -= dt * close_speed;
                if (transition_progress <= 0.0f) {
                    transition_progress = 0.0f;
                    is_open = false;
                    return;
                }
            }

            if (!is_open && transition_progress <= 0.0001f) return;

            // Tecla ESC fecha o Home Hub com animação suave
            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                close();
            }

            // Curva exponencial Quartic Out (1 - (1-t)^3.5) para movimento aerodinâmico
            float t = std::clamp(transition_progress, 0.0f, 1.0f);
            float eased = 1.0f - powf(1.0f - t, 3.5f);
            float scale = 0.94f + 0.06f * eased; // Escala sutil de 94% a 100%

            ImGuiViewport* vp = ImGui::GetMainViewport();

            // Backdrop escuro cobrindo toda a DAW atrás do Home Hub com fade suave
            ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(vp->Size, ImGuiCond_Always);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.015f, 0.025f, 0.040f, 0.82f * eased)); // Escurece apenas o fundo com fade
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGuiWindowFlags backdrop_flags = ImGuiWindowFlags_NoTitleBar | 
                                              ImGuiWindowFlags_NoResize | 
                                              ImGuiWindowFlags_NoMove | 
                                              ImGuiWindowFlags_NoScrollbar | 
                                              ImGuiWindowFlags_NoScrollWithMouse | 
                                              ImGuiWindowFlags_NoCollapse | 
                                              ImGuiWindowFlags_NoSavedSettings | 
                                              ImGuiWindowFlags_NoNav | 
                                              ImGuiWindowFlags_NoBringToFrontOnFocus | 
                                              ImGuiWindowFlags_NoFocusOnAppearing;

            // Dimensões da Janela do Home Hub com micro-escala
            float base_w = std::min(1060.0f, vp->Size.x - 30.0f);
            float base_h = std::min(620.0f, vp->Size.y - 30.0f);
            float win_w = base_w * scale;
            float win_h = base_h * scale;
            ImVec2 center_pos = ImVec2(vp->Pos.x + (vp->Size.x - win_w) * 0.5f, vp->Pos.y + (vp->Size.y - win_h) * 0.5f);

            // Halo e Cyber Glow multicamada com pulsação neon ciano
            float glow_pulse = 0.88f + 0.12f * sinf(anim_timer * 3.5f);
            float glow_mult = eased * glow_pulse;

            if (ImGui::Begin("##AbductionHubBackdrop", nullptr, backdrop_flags)) {
                ImDrawList* bdl = ImGui::GetWindowDrawList();
                ImVec2 m_min = center_pos;
                ImVec2 m_max = ImVec2(center_pos.x + win_w, center_pos.y + win_h);

                // 1. Sombra em 6 camadas profundas
                for (int s = 1; s <= 6; ++s) {
                    float expand = (float)s * 3.5f * scale;
                    int alpha = (int)((45.0f / (float)s) * eased);
                    if (alpha > 0) {
                        bdl->AddRect(ImVec2(m_min.x - expand, m_min.y - expand), 
                                     ImVec2(m_max.x + expand, m_max.y + expand), 
                                     IM_COL32(0, 0, 0, alpha), (12.0f + expand) * scale, 0, 2.0f);
                    }
                }

                // 2. Halo de Cyber Glow em 3 anéis neon ciano
                int g_alpha1 = (int)(80.0f * glow_mult);
                int g_alpha2 = (int)(45.0f * glow_mult);
                int g_alpha3 = (int)(22.0f * glow_mult);

                if (g_alpha1 > 0) {
                    bdl->AddRect(ImVec2(m_min.x - 2.0f, m_min.y - 2.0f), 
                                 ImVec2(m_max.x + 2.0f, m_max.y + 2.0f), 
                                 IM_COL32(0, 229, 255, g_alpha1), 13.0f * scale, 0, 2.0f);
                }
                if (g_alpha2 > 0) {
                    bdl->AddRect(ImVec2(m_min.x - 7.0f, m_min.y - 7.0f), 
                                 ImVec2(m_max.x + 7.0f, m_max.y + 7.0f), 
                                 IM_COL32(0, 180, 240, g_alpha2), 16.0f * scale, 0, 2.5f);
                }
                if (g_alpha3 > 0) {
                    bdl->AddRect(ImVec2(m_min.x - 14.0f, m_min.y - 14.0f), 
                                 ImVec2(m_max.x + 14.0f, m_max.y + 14.0f), 
                                 IM_COL32(0, 120, 200, g_alpha3), 20.0f * scale, 0, 3.0f);
                }

                // Clique no fundo fora do modal fecha o Home Hub suavemente
                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    close();
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(1);

            ImGui::SetNextWindowPos(center_pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(win_w, win_h), ImGuiCond_Always);
            ImGui::SetNextWindowFocus();

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(eased, 0.05f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.045f, 0.070f, 0.095f, 0.99f)); // Deep cyber dark
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.35f + 0.15f * glow_pulse)); // Cyan accent border com pulso
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 14.0f * scale));

            ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoDecoration | 
                                         ImGuiWindowFlags_NoMove | 
                                         ImGuiWindowFlags_NoSavedSettings | 
                                         ImGuiWindowFlags_NoScrollWithMouse;

            if (ImGui::Begin("##AbductionHomeHubWindow", &is_open, win_flags)) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 wp0 = ImGui::GetWindowPos();
                ImVec2 wp1 = ImVec2(wp0.x + win_w, wp0.y + win_h);

                // Grade cibernética sutil de fundo (36px pitch)
                for (float gx = wp0.x + 36.0f; gx < wp1.x; gx += 36.0f) {
                    dl->AddLine(ImVec2(gx, wp0.y), ImVec2(gx, wp1.y), IM_COL32(0, 229, 255, 8), 1.0f);
                }
                for (float gy = wp0.y + 36.0f; gy < wp1.y; gy += 36.0f) {
                    dl->AddLine(ImVec2(wp0.x, gy), ImVec2(wp1.x, gy), IM_COL32(0, 229, 255, 8), 1.0f);
                }

                // =========================================================================
                // 1. HEADER BAR SUPERIOR (LOGO, STATUS PILLS E BOTÃO DE SAÍDA)
                // =========================================================================
                float top_bar_h = 42.0f;
                ImVec2 header_p0 = wp0;
                ImVec2 header_p1 = ImVec2(wp1.x, wp0.y + top_bar_h);

                // Logo Abduction Studio (Ícone Pirâmide Alien estilizado + Texto)
                ImVec2 logo_c = ImVec2(wp0.x + 28.0f, wp0.y + 24.0f);
                // Triângulo 'A' em ciano luminoso
                dl->AddTriangleFilled(ImVec2(logo_c.x, logo_c.y - 12.0f),
                                      ImVec2(logo_c.x - 10.0f, logo_c.y + 8.0f),
                                      ImVec2(logo_c.x + 10.0f, logo_c.y + 8.0f),
                                      IM_COL32(0, 229, 255, 230));
                // Círculo central recortado
                dl->AddCircleFilled(ImVec2(logo_c.x, logo_c.y + 2.0f), 4.0f, IM_COL32(11, 18, 25, 255));
                // Feixe de abdução
                dl->AddLine(ImVec2(logo_c.x - 4.0f, logo_c.y + 9.0f), ImVec2(logo_c.x - 7.0f, logo_c.y + 14.0f), IM_COL32(0, 229, 255, 180), 1.4f);
                dl->AddLine(ImVec2(logo_c.x + 4.0f, logo_c.y + 9.0f), ImVec2(logo_c.x + 7.0f, logo_c.y + 14.0f), IM_COL32(0, 229, 255, 180), 1.4f);

                // Texto do Logo
                ImGui::SetCursorPos(ImVec2(48.0f, 13.0f));
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Abduction");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.60f, 0.75f, 0.85f, 1.0f), "Studio");

                // Badges Centrais de Status: CPU 3%, RAM 1.4GB, audio engine ACTIVE
                float pills_start_x = wp0.x + win_w * 0.42f;
                float py = wp0.y + 12.0f;

                // Pill 1: CPU 3%
                dl->AddRectFilled(ImVec2(pills_start_x, py), ImVec2(pills_start_x + 84.0f, py + 24.0f), IM_COL32(10, 26, 22, 220), 12.0f);
                dl->AddRect(ImVec2(pills_start_x, py), ImVec2(pills_start_x + 84.0f, py + 24.0f), IM_COL32(38, 166, 120, 220), 12.0f, 0, 1.2f);
                dl->AddText(ImVec2(pills_start_x + 10.0f, py + 4.0f), IM_COL32(180, 240, 200, 255), "CPU");
                dl->AddText(ImVec2(pills_start_x + 48.0f, py + 4.0f), IM_COL32(46, 204, 113, 255), "3%");

                // Pill 2: RAM 1.4GB
                float pill2_x = pills_start_x + 94.0f;
                dl->AddRectFilled(ImVec2(pill2_x, py), ImVec2(pill2_x + 104.0f, py + 24.0f), IM_COL32(10, 24, 34, 220), 12.0f);
                dl->AddRect(ImVec2(pill2_x, py), ImVec2(pill2_x + 104.0f, py + 24.0f), IM_COL32(0, 180, 216, 220), 12.0f, 0, 1.2f);
                dl->AddText(ImVec2(pill2_x + 10.0f, py + 4.0f), IM_COL32(170, 220, 245, 255), "RAM");
                dl->AddText(ImVec2(pill2_x + 48.0f, py + 4.0f), IM_COL32(0, 229, 255, 255), "1.4GB");

                // Pill 3: audio engine ACTIVE
                float pill3_x = wp0.x + win_w - 240.0f;
                dl->AddRectFilled(ImVec2(pill3_x, py), ImVec2(pill3_x + 172.0f, py + 24.0f), IM_COL32(10, 20, 28, 220), 12.0f);
                dl->AddRect(ImVec2(pill3_x, py), ImVec2(pill3_x + 172.0f, py + 24.0f), IM_COL32(30, 48, 64, 200), 12.0f);
                dl->AddText(ImVec2(pill3_x + 12.0f, py + 4.0f), IM_COL32(140, 160, 180, 255), "audio engine");
                dl->AddText(ImVec2(pill3_x + 110.0f, py + 4.0f), IM_COL32(0, 229, 255, 255), "ACTIVE");

                // Botão Fechar [X] no topo direito
                ImGui::SetCursorPos(ImVec2(win_w - 42.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.15f, 0.2f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1.0f));
                if (ImGui::Button("✕##hub_close_btn", ImVec2(28.0f, 26.0f))) {
                    show_playlist = true;
                    close();
                }
                ImGui::PopStyleColor(3);

                ImGui::SetCursorPosY(46.0f);

                // =========================================================================
                // 2. PAINEL PRINCIPAL EM 2 COLUNAS (RECENT PROJECTS & QUICK START TEMPLATES)
                // =========================================================================
                float avail_w = ImGui::GetContentRegionAvail().x;
                float col_w = (avail_w - 20.0f) * 0.5f;
                float panel_h = win_h - 118.0f; // Espaço reservado para o rodapé

                // -------------------------------------------------------------------------
                // COLUNA ESQUERDA: RECENT PROJECTS
                // -------------------------------------------------------------------------
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.065f, 0.09f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.22f, 0.30f, 0.80f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.2f);
                ImGui::BeginChild("##RecentProjectsContainer", ImVec2(col_w, panel_h), true);
                {
                    ImDrawList* cdl = ImGui::GetWindowDrawList();

                    ImGui::TextColored(ImVec4(0.55f, 0.70f, 0.82f, 1.0f), "RECENT PROJECTS");
                    ImGui::Spacing();

                    float card_h = 108.0f;
                    float card_w = ImGui::GetContentRegionAvail().x;

                    for (size_t i = 0; i < recent_projects.size(); ++i) {
                        const auto& item = recent_projects[i];
                        ImGui::PushID((int)i);

                        ImVec2 cp0 = ImGui::GetCursorScreenPos();
                        ImVec2 cp1 = ImVec2(cp0.x + card_w, cp0.y + card_h);

                        // Botão invisível interativo ocupando todo o card
                        ImGui::InvisibleButton("##card_btn", ImVec2(card_w, card_h));
                        bool hovered = ImGui::IsItemHovered();
                        bool clicked = ImGui::IsItemClicked();

                        if (clicked) {
                            // Abre o projeto e fecha a Home suavemente
                            timeline.setBPM((float)item.bpm);
                            clip_manager.reset();
                            timeline.setMasterFrame(0);
                            show_playlist = true;
                            close();
                        }

                        // Background do card com realce no hover
                        ImU32 card_bg = hovered ? IM_COL32(14, 24, 34, 255) : IM_COL32(9, 16, 23, 220);
                        ImU32 card_border = hovered ? IM_COL32(0, 229, 255, 220) : IM_COL32(24, 42, 56, 180);
                        cdl->AddRectFilled(cp0, cp1, card_bg, 8.0f);
                        cdl->AddRect(cp0, cp1, card_border, 8.0f, 0, hovered ? 1.6f : 1.0f);

                        // 1. Título do projeto
                        cdl->AddText(ImVec2(cp0.x + 14.0f, cp0.y + 10.0f), IM_COL32(255, 255, 255, 255), item.title.c_str());

                        // 2. Mini-Waveform renderizada no miolo do card
                        ImVec2 wave_p0 = ImVec2(cp0.x + 14.0f, cp0.y + 32.0f);
                        ImVec2 wave_p1 = ImVec2(cp1.x - 14.0f, cp0.y + 76.0f);
                        RenderMockupWaveform(cdl, wave_p0, wave_p1, item.wave_style, item.badge_color);

                        // 3. Badge do BPM (Pill ciano no canto inferior esquerdo)
                        ImVec2 bpm_p0 = ImVec2(cp0.x + 14.0f, cp1.y - 26.0f);
                        ImVec2 bpm_p1 = ImVec2(bpm_p0.x + 68.0f, cp1.y - 8.0f);
                        cdl->AddRectFilled(bpm_p0, bpm_p1, IM_COL32(0, 160, 200, 240), 10.0f);
                        char bpm_buf[32];
                        snprintf(bpm_buf, sizeof(bpm_buf), "%d BPM", item.bpm);
                        cdl->AddText(ImVec2(bpm_p0.x + 9.0f, bpm_p0.y + 2.0f), IM_COL32(5, 20, 28, 255), bpm_buf);

                        // 4. Data de modificação no canto inferior direito
                        ImVec2 mod_sz = ImGui::CalcTextSize(item.modified_text.c_str());
                        cdl->AddText(ImVec2(cp1.x - mod_sz.x - 14.0f, cp1.y - 24.0f), IM_COL32(130, 150, 170, 255), item.modified_text.c_str());

                        ImGui::Spacing();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);

                ImGui::SameLine(0, 18.0f);

                // -------------------------------------------------------------------------
                // COLUNA DIREITA: QUICK START TEMPLATES
                // -------------------------------------------------------------------------
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.065f, 0.09f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.22f, 0.30f, 0.80f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.2f);
                ImGui::BeginChild("##QuickTemplatesContainer", ImVec2(col_w, panel_h), true);
                {
                    ImDrawList* tdl = ImGui::GetWindowDrawList();

                    ImGui::TextColored(ImVec4(0.55f, 0.70f, 0.82f, 1.0f), "QUICK START TEMPLATES");
                    ImGui::Spacing();

                    float card_h = 108.0f;
                    float card_w = ImGui::GetContentRegionAvail().x;

                    for (size_t i = 0; i < quick_templates.size(); ++i) {
                        const auto& tmpl = quick_templates[i];
                        ImGui::PushID(100 + (int)i);

                        ImVec2 cp0 = ImGui::GetCursorScreenPos();
                        ImVec2 cp1 = ImVec2(cp0.x + card_w, cp0.y + card_h);

                        ImGui::InvisibleButton("##tmpl_btn", ImVec2(card_w, card_h));
                        bool hovered = ImGui::IsItemHovered();
                        bool clicked = ImGui::IsItemClicked();

                        if (clicked) {
                            LoadTemplate((int)i, timeline, clip_manager);
                            show_playlist = true;
                            close();
                        }

                        // Background e borda com acento temático por vertente
                        ImU32 card_bg = hovered ? IM_COL32(14, 24, 34, 255) : IM_COL32(9, 16, 23, 220);
                        tdl->AddRectFilled(cp0, cp1, card_bg, 8.0f);
                        tdl->AddRect(cp0, cp1, tmpl.theme_color, 8.0f, 0, hovered ? 2.0f : 1.3f);

                        // Ícone à esquerda
                        ImVec2 ic0 = ImVec2(cp0.x + 14.0f, cp0.y + 14.0f);
                        ImVec2 ic1 = ImVec2(cp0.x + 14.0f + 80.0f, cp1.y - 14.0f);
                        RenderTemplateIcon(tdl, ic0, ic1, tmpl.icon_type, tmpl.theme_color);

                        // Textos à direita do ícone
                        float text_x = ic1.x + 18.0f;
                        tdl->AddText(ImVec2(text_x, cp0.y + 26.0f), IM_COL32(255, 255, 255, 255), tmpl.title.c_str());
                        tdl->AddText(ImVec2(text_x, cp0.y + 54.0f), IM_COL32(150, 175, 195, 255), tmpl.subtitle.c_str());

                        ImGui::Spacing();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);

                // =========================================================================
                // 3. FOOTER BAR INFERIOR (DRIVER DE ÁUDIO, STATUS LED E NEW BLANK PROJECT)
                // =========================================================================
                ImGui::SetCursorPosY(win_h - 48.0f);
                ImGui::SetCursorPosX(18.0f);

                // Seletor de Áudio / Driver
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.10f, 0.14f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.30f, 0.40f, 0.80f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                ImGui::SetNextItemWidth(310.0f);

                if (ImGui::BeginCombo("##AudioDriverCombo", audio_drivers[selected_audio_driver])) {
                    for (int n = 0; n < 4; n++) {
                        const bool is_selected = (selected_audio_driver == n);
                        if (ImGui::Selectable(audio_drivers[n], is_selected)) {
                            selected_audio_driver = n;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(2);

                // LED indicador de hardware de áudio conectado (Âmbar/Verde suave pulsante)
                ImGui::SameLine(0, 12.0f);
                ImVec2 led_c = ImVec2(ImGui::GetCursorScreenPos().x + 8.0f, ImGui::GetCursorScreenPos().y + 12.0f);
                float pulse = 0.85f + 0.15f * sinf(anim_timer * 3.0f);
                dl->AddCircleFilled(led_c, 8.0f * pulse, IM_COL32(245, 166, 35, 55));
                dl->AddCircleFilled(led_c, 5.0f, IM_COL32(245, 166, 35, 255));
                dl->AddCircle(led_c, 5.0f, IM_COL32(255, 210, 100, 255), 12, 1.0f);

                // Botão de Destaque: NEW BLANK PROJECT +
                float new_btn_w = 210.0f;
                float new_btn_h = 32.0f;
                ImGui::SetCursorPosX(win_w - new_btn_w - 18.0f);
                ImGui::SetCursorPosY(win_h - 46.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.14f, 0.18f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.06f, 0.22f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.80f, 0.90f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.95f, 1.00f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.4f);
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.9f));

                if (ImGui::Button("NEW BLANK PROJECT  +", ImVec2(new_btn_w, new_btn_h))) {
                    timeline.setBPM(138.0f);
                    clip_manager.reset();
                    timeline.setMasterFrame(0);
                    timeline.setPlaying(false);
                    show_playlist = true;
                    close();
                }

                ImGui::PopStyleColor(5);
                ImGui::PopStyleVar(2);
            }
            ImGui::End();
            ImGui::PopStyleVar(4); // Alpha, WindowRounding, WindowBorderSize, WindowPadding
            ImGui::PopStyleColor(2); // WindowBg, Border
        }
    };

    inline HomeDashboardUI g_home_dashboard_ui;
}
