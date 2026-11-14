#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "DJLayoutMetrics.h"
#include "../core/DJPreviewPlayer.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroUI {

    enum class TourStepId {
        WELCOME_OVERVIEW = 0,
        LAYOUT_EDITOR,
        DECKS_AND_PLATTER,
        MIXER_AND_EQ,
        MULTI_RIBBON_WAVEFORMS,
        PERFORMANCE_PADS,
        LIVE_SYNTH_AND_MIDI,
        COUNT
    };

    enum class TourAnchorId {
        ANCHOR_TOP_BAR = 0,
        ANCHOR_EDIT_HUD,
        ANCHOR_DECK_LEFT,
        ANCHOR_MIXER,
        ANCHOR_DECK_RIGHT,
        ANCHOR_WAVEFORMS,
        ANCHOR_PADS,
        ANCHOR_TRANSPORT_LEFT,
        ANCHOR_COUNT
    };

    class DJInteractiveTourEngine {
    private:
        bool m_active = false;
        int m_current_step = 0;
        float m_anim_time = 0.0f;

        // Bounding boxes dos componentes da interface registrados neste frame
        ImVec2 m_anchor_min[(int)TourAnchorId::ANCHOR_COUNT];
        ImVec2 m_anchor_max[(int)TourAnchorId::ANCHOR_COUNT];
        bool m_anchor_valid[(int)TourAnchorId::ANCHOR_COUNT] = { false };

        // Posição suavizada do Spotlight (interpolação fluida entre passos)
        ImVec2 m_spot_curr_min = ImVec2(0, 0);
        ImVec2 m_spot_curr_max = ImVec2(0, 0);
        bool m_spot_initialized = false;

        DJInteractiveTourEngine() {
            for (int i = 0; i < (int)TourAnchorId::ANCHOR_COUNT; i++) {
                m_anchor_min[i] = ImVec2(0, 0);
                m_anchor_max[i] = ImVec2(0, 0);
                m_anchor_valid[i] = false;
            }
        }

    public:
        static DJInteractiveTourEngine& getInstance() {
            static DJInteractiveTourEngine instance;
            return instance;
        }

        bool isActive() const { return m_active; }
        void setActive(bool active) {
            m_active = active;
            if (active) {
                m_spot_initialized = false;
            }
        }
        void toggle() { setActive(!m_active); }

        int getCurrentStep() const { return m_current_step; }
        void setStep(int step) {
            int max_step = (int)TourStepId::COUNT - 1;
            m_current_step = std::clamp(step, 0, max_step);
        }
        void nextStep() {
            if (m_current_step < (int)TourStepId::COUNT - 1) {
                m_current_step++;
            } else {
                m_active = false; // Concluiu o tour!
            }
        }
        void prevStep() {
            if (m_current_step > 0) {
                m_current_step--;
            }
        }

        // Registro de âncoras na renderização da interface
        void registerAnchor(TourAnchorId id, ImVec2 p_min, ImVec2 p_max) {
            int idx = (int)id;
            if (idx >= 0 && idx < (int)TourAnchorId::ANCHOR_COUNT) {
                m_anchor_min[idx] = p_min;
                m_anchor_max[idx] = p_max;
                m_anchor_valid[idx] = true;
            }
        }

        // =========================================================================
        // SÍNTESE PROCEDURAL DE ÁUDIO DIDÁTICO (EXEMPLOS SONOROS DE 3 A 5 SEGUNDOS)
        // =========================================================================
        void playStepAudioDemo(int step_index) {
            double sr = 44100.0;
            double duration = 3.5;
            size_t total_samples = (size_t)(sr * duration);
            std::vector<float> buf_l(total_samples, 0.0f);
            std::vector<float> buf_r(total_samples, 0.0f);
            std::string title = "Demo Didática";

            switch (step_index) {
                case (int)TourStepId::WELCOME_OVERVIEW: {
                    title = "Abduction Chords Demo";
                    duration = 3.8;
                    total_samples = (size_t)(sr * duration);
                    buf_l.resize(total_samples, 0.0f);
                    buf_r.resize(total_samples, 0.0f);
                    // Acorde suave Cmaj9 (C3, G3, B3, D4, E4) com vibrato estéreo
                    double freqs[] = { 130.81, 196.00, 246.94, 293.66, 329.63 };
                    for (size_t i = 0; i < total_samples; i++) {
                        double t = (double)i / sr;
                        double env = std::min(t * 3.0, 1.0) * std::exp(-t * 0.7);
                        float s_l = 0.0f, s_r = 0.0f;
                        for (int f = 0; f < 5; f++) {
                            double ph_l = 2.0 * M_PI * freqs[f] * t + 0.1 * std::sin(t * 5.0);
                            double ph_r = 2.0 * M_PI * freqs[f] * t + 0.1 * std::cos(t * 5.0);
                            s_l += (float)std::sin(ph_l) * 0.15f;
                            s_r += (float)std::sin(ph_r) * 0.15f;
                        }
                        buf_l[i] = s_l * (float)env;
                        buf_r[i] = s_r * (float)env;
                    }
                    break;
                }
                case (int)TourStepId::LAYOUT_EDITOR: {
                    title = "Metrônomo Rítmico 4/4 (Beat 1 Destaque)";
                    // Metrônomo 128 BPM (0.46875s por beat)
                    double beat_dt = 60.0 / 128.0;
                    for (int b = 0; b < 8; b++) {
                        double beat_start = b * beat_dt;
                        size_t start_idx = (size_t)(beat_start * sr);
                        double freq = (b % 4 == 0) ? 880.0 : 440.0; // Beat 1 agudo de alta energia
                        float gain = (b % 4 == 0) ? 0.75f : 0.40f;
                        size_t beat_len = (size_t)(0.12 * sr);
                        for (size_t n = 0; n < beat_len && (start_idx + n) < total_samples; n++) {
                            double t = (double)n / sr;
                            double env = std::exp(-t * 35.0);
                            float s = (float)(std::sin(2.0 * M_PI * freq * t) * env * gain);
                            buf_l[start_idx + n] += s;
                            buf_r[start_idx + n] += s;
                        }
                    }
                    break;
                }
                case (int)TourStepId::DECKS_AND_PLATTER: {
                    title = "Efeito Vinil: Scratch & Motor Brake";
                    // Scratch modulado para frente e para trás, depois desaceleração de vinil
                    for (size_t i = 0; i < total_samples; i++) {
                        double t = (double)i / sr;
                        float s = 0.0f;
                        if (t < 1.6) {
                            // Scratch rhythm
                            double mod = std::sin(2.0 * M_PI * 3.5 * t);
                            double scratch_freq = 220.0 + mod * 180.0;
                            s = (float)(std::sin(2.0 * M_PI * scratch_freq * t) * 0.55f);
                            // Harmônicos do atrito do vinil
                            s += (float)(std::sin(2.0 * M_PI * scratch_freq * 2.0 * t) * 0.25f);
                        } else {
                            // Motor brake (queda contínua de pitch)
                            double brake_t = t - 1.6;
                            double brake_freq = std::max(20.0, 300.0 * std::exp(-brake_t * 2.2));
                            double env = std::exp(-brake_t * 1.5);
                            s = (float)(std::sin(2.0 * M_PI * brake_freq * t) * env * 0.6f);
                        }
                        buf_l[i] = s;
                        buf_r[i] = s;
                    }
                    break;
                }
                case (int)TourStepId::MIXER_AND_EQ: {
                    title = "A Arte do Bass Swap (Corte Cirúrgico de Grave)";
                    // Batida techno/house com graves nos compassos 1 e 2, corte de graves no compasso 3, e novo drop no 4
                    double beat_dt = 60.0 / 128.0;
                    for (int b = 0; b < 8; b++) {
                        double beat_start = b * beat_dt;
                        size_t start_idx = (size_t)(beat_start * sr);
                        bool is_bass_cut = (b >= 4 && b < 6); // Compassos sem grave (Bass Kill)
                        
                        // Kick / Sub
                        if (!is_bass_cut) {
                            size_t kick_len = (size_t)(0.25 * sr);
                            for (size_t n = 0; n < kick_len && (start_idx + n) < total_samples; n++) {
                                double t = (double)n / sr;
                                double f = 150.0 * std::exp(-t * 28.0) + 48.0;
                                double env = std::exp(-t * 12.0);
                                float kick = (float)(std::sin(2.0 * M_PI * f * t) * env * 0.8f);
                                buf_l[start_idx + n] += kick;
                                buf_r[start_idx + n] += kick;
                            }
                        }
                        // Hihat nos contra-tempos (off-beat)
                        size_t hat_start = start_idx + (size_t)(beat_dt * 0.5 * sr);
                        size_t hat_len = (size_t)(0.08 * sr);
                        for (size_t n = 0; n < hat_len && (hat_start + n) < total_samples; n++) {
                            double t = (double)n / sr;
                            double env = std::exp(-t * 40.0);
                            float noise = ((float)(rand() % 2000 - 1000) / 1000.0f) * (float)env * 0.35f;
                            buf_l[hat_start + n] += noise;
                            buf_r[hat_start + n] += noise;
                        }
                    }
                    break;
                }
                case (int)TourStepId::MULTI_RIBBON_WAVEFORMS: {
                    title = "Waveform Riser & Drop Explosivo";
                    // Build-up crescente com snare acelerando, 1 beat de silêncio (Break) e Drop estrondoso!
                    double riser_dur = 2.4;
                    size_t riser_samples = (size_t)(sr * riser_dur);
                    int snare_hits = 24;
                    for (int s = 0; s < snare_hits; s++) {
                        double t_hit = std::pow((double)s / (double)snare_hits, 1.8) * riser_dur;
                        size_t h_idx = (size_t)(t_hit * sr);
                        float vol = 0.2f + 0.6f * ((float)s / (float)snare_hits);
                        size_t hit_len = (size_t)(0.08 * sr);
                        for (size_t n = 0; n < hit_len && (h_idx + n) < total_samples; n++) {
                            double t = (double)n / sr;
                            float noise = ((float)(rand() % 2000 - 1000) / 1000.0f) * std::exp(-t * 30.0f) * vol;
                            buf_l[h_idx + n] += noise;
                            buf_r[h_idx + n] += noise;
                        }
                    }
                    // Silêncio dramático de 2.4s até 2.7s
                    // DROP em 2.7s com Kick poderoso + Bass sintetizado
                    size_t drop_idx = (size_t)(2.7 * sr);
                    for (size_t n = 0; (drop_idx + n) < total_samples; n++) {
                        double t = (double)n / sr;
                        double f = 160.0 * std::exp(-t * 18.0) + 50.0;
                        float kick = (float)(std::sin(2.0 * M_PI * f * t) * std::exp(-t * 8.0) * 0.85f);
                        float saw_bass = (float)(std::fmod(t * 110.0, 1.0) * 2.0 - 1.0) * 0.35f;
                        buf_l[drop_idx + n] += kick + saw_bass;
                        buf_r[drop_idx + n] += kick + saw_bass;
                    }
                    break;
                }
                case (int)TourStepId::PERFORMANCE_PADS: {
                    title = "Hot Cue & Beat Loop Stutter";
                    // Efeito de repetição rítmica rápida (1/4 -> 1/8 -> 1/16 roll)
                    double beat_dt = 60.0 / 128.0;
                    double rolls[] = { beat_dt, beat_dt, beat_dt * 0.5, beat_dt * 0.5, beat_dt * 0.25, beat_dt * 0.25, beat_dt * 0.125, beat_dt * 0.125 };
                    double cur_t = 0.0;
                    for (int r = 0; r < 8 && cur_t < duration; r++) {
                        size_t s_idx = (size_t)(cur_t * sr);
                        size_t hit_len = (size_t)(0.10 * sr);
                        for (size_t n = 0; n < hit_len && (s_idx + n) < total_samples; n++) {
                            double t = (double)n / sr;
                            float s = (float)(std::sin(2.0 * M_PI * 520.0 * t) * std::exp(-t * 22.0) * 0.65f);
                            buf_l[s_idx + n] += s;
                            buf_r[s_idx + n] += s;
                        }
                        cur_t += rolls[r];
                    }
                    break;
                }
                case (int)TourStepId::LIVE_SYNTH_AND_MIDI: {
                    title = "Sintetizador Analógico 303 ao Vivo";
                    // Linha melódica ácida clássica (Acid Bassline) com filtro ressonante abrindo
                    double note_dt = (60.0 / 128.0) * 0.25; // Semicolcheias
                    double notes[] = { 65.41, 65.41, 130.81, 65.41, 77.78, 65.41, 98.0, 116.54, 
                                       65.41, 65.41, 130.81, 98.0,  77.78, 65.41, 116.54, 130.81 };
                    for (int step = 0; step < 16; step++) {
                        double note_start = step * note_dt;
                        size_t s_idx = (size_t)(note_start * sr);
                        double freq = notes[step % 16];
                        float cutoff_sweep = 0.2f + 0.8f * ((float)step / 16.0f);
                        size_t note_len = (size_t)(note_dt * 0.85 * sr);
                        for (size_t n = 0; n < note_len && (s_idx + n) < total_samples; n++) {
                            double t = (double)n / sr;
                            // Onda dente de serra analógica
                            double saw = (std::fmod(t * freq, 1.0) * 2.0 - 1.0);
                            double env = std::exp(-t * 16.0);
                            float squelch = (float)(saw * env * cutoff_sweep * 0.6f);
                            buf_l[s_idx + n] += squelch;
                            buf_r[s_idx + n] += squelch;
                        }
                    }
                    break;
                }
                default: break;
            }

            // Normalização de segurança e envio para o DJPreviewPlayer
            float max_val = 0.001f;
            for (size_t i = 0; i < total_samples; i++) {
                max_val = std::max(max_val, std::abs(buf_l[i]));
                max_val = std::max(max_val, std::abs(buf_r[i]));
            }
            if (max_val > 0.85f) {
                float norm = 0.85f / max_val;
                for (size_t i = 0; i < total_samples; i++) {
                    buf_l[i] *= norm;
                    buf_r[i] *= norm;
                }
            }

            auto& player = KuroAudio::DJPreviewPlayer::getInstance();
            player.playDirectBuffer(std::move(buf_l), std::move(buf_r), sr, title, "Guia Didático Abduction");
        }

        // =========================================================================
        // RENDERIZAÇÃO PRINCIPAL DO TOUR GUIDE (SPOTLIGHT MASK + CARD DIDÁTICO)
        // =========================================================================
        void render() {
            if (!m_active) return;

            ImGuiIO& io = ImGui::GetIO();
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImDrawList* dl = ImGui::GetForegroundDrawList();

            // Atalhos de teclado no Tour: ESC ou F1 fecha, Setas navegam
            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
                m_active = false;
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false) || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
                nextStep();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
                prevStep();
            }

            m_anim_time += io.DeltaTime;

            // Determinar o retângulo alvo do Spotlight para a etapa atual
            ImVec2 target_min, target_max;
            calculateSpotlightTarget(m_current_step, vp, target_min, target_max);

            // Interpolação suave do Spotlight (Lerp dinâmico para transição macia)
            if (!m_spot_initialized) {
                m_spot_curr_min = target_min;
                m_spot_curr_max = target_max;
                m_spot_initialized = true;
            } else {
                float lerp_factor = std::clamp(io.DeltaTime * 14.0f, 0.05f, 1.0f);
                m_spot_curr_min.x += (target_min.x - m_spot_curr_min.x) * lerp_factor;
                m_spot_curr_min.y += (target_min.y - m_spot_curr_min.y) * lerp_factor;
                m_spot_curr_max.x += (target_max.x - m_spot_curr_max.x) * lerp_factor;
                m_spot_curr_max.y += (target_max.y - m_spot_curr_max.y) * lerp_factor;
            }

            // Margem de respiro ao redor do componente
            float pad = 6.0f;
            ImVec2 s_min = ImVec2(m_spot_curr_min.x - pad, m_spot_curr_min.y - pad);
            ImVec2 s_max = ImVec2(m_spot_curr_max.x + pad, m_spot_curr_max.y + pad);

            // Clamping para não ultrapassar a viewport
            s_min.x = std::max(vp->Pos.x, s_min.x);
            s_min.y = std::max(vp->Pos.y, s_min.y);
            s_max.x = std::min(vp->Pos.x + vp->Size.x, s_max.x);
            s_max.y = std::min(vp->Pos.y + vp->Size.y, s_max.y);

            // 1. DESENHO DA MÁSCARA ESCURA (Overlay Window com 78% Opacidade)
            ImGui::SetNextWindowPos(vp->Pos);
            ImGui::SetNextWindowSize(vp->Size);
            ImGuiWindowFlags mask_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs |
                                          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav;

            if (ImGui::Begin("##TourMaskOverlayWindow", nullptr, mask_flags)) {
                ImDrawList* wdl = ImGui::GetWindowDrawList();
                ImU32 mask_col = IM_COL32(6, 10, 16, 205);
                // Topo
                wdl->AddRectFilled(vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, s_min.y), mask_col);
                // Rodapé
                wdl->AddRectFilled(ImVec2(vp->Pos.x, s_max.y), ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y), mask_col);
                // Esquerda
                wdl->AddRectFilled(ImVec2(vp->Pos.x, s_min.y), ImVec2(s_min.x, s_max.y), mask_col);
                // Direita
                wdl->AddRectFilled(ImVec2(s_max.x, s_min.y), ImVec2(vp->Pos.x + vp->Size.x, s_max.y), mask_col);

                // 2. BORDA PULSANTE DO SPOTLIGHT (Neon Dourado / Ciano com pulso senoidal)
                float pulse = (std::sin(m_anim_time * 4.5f) + 1.0f) * 0.5f; // 0.0 a 1.0
                ImU32 border_col = IM_COL32(255, (int)(180 + pulse * 60), (int)(30 + pulse * 100), (int)(220 + pulse * 35));
                ImU32 glow_col   = IM_COL32(255, 190, 40, (int)(40 + pulse * 65));
                wdl->AddRect(s_min, s_max, border_col, 8.0f, 0, 2.5f);
                wdl->AddRect(ImVec2(s_min.x - 3.0f, s_min.y - 3.0f), ImVec2(s_max.x + 3.0f, s_max.y + 3.0f), glow_col, 10.0f, 0, 1.8f);
            }
            ImGui::End();

            // 3. CARD DIDÁTICO PARA LEIGOS (Renderizado em primeiro plano, sobre a máscara)
            renderTourCard(s_min, s_max, vp);
        }

    private:
        void calculateSpotlightTarget(int step, ImGuiViewport* vp, ImVec2& out_min, ImVec2& out_max) {
            auto& metrics = DJLayoutMetrics::getInstance();
            float topbar_h = 36.0f;

            switch (step) {
                case (int)TourStepId::WELCOME_OVERVIEW: {
                    // Visão Geral: Toda a metade superior (Top Bar + Decks principais)
                    out_min = ImVec2(vp->Pos.x + 20.0f, vp->Pos.y + 35.0f);
                    out_max = ImVec2(vp->Pos.x + vp->Size.x - 20.0f, vp->Pos.y + vp->Size.y * 0.48f);
                    break;
                }
                case (int)TourStepId::LAYOUT_EDITOR: {
                    // Modo Edição: A barra HUD superior ou topo
                    if (m_anchor_valid[(int)TourAnchorId::ANCHOR_EDIT_HUD]) {
                        out_min = m_anchor_min[(int)TourAnchorId::ANCHOR_EDIT_HUD];
                        out_max = m_anchor_max[(int)TourAnchorId::ANCHOR_EDIT_HUD];
                    } else {
                        // Fallback baseado na posição padrão da barra de edição
                        float bar_w = std::min(vp->Size.x - 40.0f, 1380.0f);
                        float bar_x = vp->Pos.x + (vp->Size.x - bar_w) * 0.5f;
                        out_min = ImVec2(bar_x, vp->Pos.y + 38.0f);
                        out_max = ImVec2(bar_x + bar_w, vp->Pos.y + 88.0f);
                    }
                    break;
                }
                case (int)TourStepId::DECKS_AND_PLATTER: {
                    // Deck Esquerdo (Deck 1 / 3)
                    if (m_anchor_valid[(int)TourAnchorId::ANCHOR_DECK_LEFT]) {
                        out_min = m_anchor_min[(int)TourAnchorId::ANCHOR_DECK_LEFT];
                        out_max = m_anchor_max[(int)TourAnchorId::ANCHOR_DECK_LEFT];
                    } else {
                        float left_w = (vp->Size.x - metrics.S(metrics.mixer_width)) * 0.5f;
                        out_min = ImVec2(vp->Pos.x + 20.0f, vp->Pos.y + 240.0f);
                        out_max = ImVec2(vp->Pos.x + left_w, vp->Pos.y + vp->Size.y - 70.0f);
                    }
                    break;
                }
                case (int)TourStepId::MIXER_AND_EQ: {
                    // Mixer Central
                    if (m_anchor_valid[(int)TourAnchorId::ANCHOR_MIXER]) {
                        out_min = m_anchor_min[(int)TourAnchorId::ANCHOR_MIXER];
                        out_max = m_anchor_max[(int)TourAnchorId::ANCHOR_MIXER];
                    } else {
                        float mixer_w = metrics.S(metrics.mixer_width);
                        float mixer_x = vp->Pos.x + (vp->Size.x - mixer_w) * 0.5f;
                        out_min = ImVec2(mixer_x, vp->Pos.y + 240.0f);
                        out_max = ImVec2(mixer_x + mixer_w, vp->Pos.y + vp->Size.y - 70.0f);
                    }
                    break;
                }
                case (int)TourStepId::MULTI_RIBBON_WAVEFORMS: {
                    // Área central de Waveforms
                    if (m_anchor_valid[(int)TourAnchorId::ANCHOR_WAVEFORMS]) {
                        out_min = m_anchor_min[(int)TourAnchorId::ANCHOR_WAVEFORMS];
                        out_max = m_anchor_max[(int)TourAnchorId::ANCHOR_WAVEFORMS];
                    } else {
                        out_min = ImVec2(vp->Pos.x + 30.0f, vp->Pos.y + 40.0f);
                        out_max = ImVec2(vp->Pos.x + vp->Size.x - 30.0f, vp->Pos.y + 235.0f);
                    }
                    break;
                }
                case (int)TourStepId::PERFORMANCE_PADS: {
                    // Pads no Deck Esquerdo
                    if (m_anchor_valid[(int)TourAnchorId::ANCHOR_PADS]) {
                        out_min = m_anchor_min[(int)TourAnchorId::ANCHOR_PADS];
                        out_max = m_anchor_max[(int)TourAnchorId::ANCHOR_PADS];
                    } else {
                        float left_w = (vp->Size.x - metrics.S(metrics.mixer_width)) * 0.5f;
                        out_min = ImVec2(vp->Pos.x + 40.0f, vp->Pos.y + vp->Size.y - 230.0f);
                        out_max = ImVec2(vp->Pos.x + left_w - 40.0f, vp->Pos.y + vp->Size.y - 75.0f);
                    }
                    break;
                }
                case (int)TourStepId::LIVE_SYNTH_AND_MIDI: {
                    // Barra Inferior / Deck Direito
                    if (m_anchor_valid[(int)TourAnchorId::ANCHOR_DECK_RIGHT]) {
                        out_min = m_anchor_min[(int)TourAnchorId::ANCHOR_DECK_RIGHT];
                        out_max = m_anchor_max[(int)TourAnchorId::ANCHOR_DECK_RIGHT];
                    } else {
                        float left_w = (vp->Size.x - metrics.S(metrics.mixer_width)) * 0.5f;
                        out_min = ImVec2(vp->Pos.x + vp->Size.x - left_w, vp->Pos.y + 240.0f);
                        out_max = ImVec2(vp->Pos.x + vp->Size.x - 20.0f, vp->Pos.y + vp->Size.y - 70.0f);
                    }
                    break;
                }
                default: {
                    out_min = ImVec2(vp->Pos.x + 50.0f, vp->Pos.y + 50.0f);
                    out_max = ImVec2(vp->Pos.x + vp->Size.x - 50.0f, vp->Pos.y + vp->Size.y - 50.0f);
                    break;
                }
            }
        }

        void renderTourCard(const ImVec2& s_min, const ImVec2& s_max, ImGuiViewport* vp) {
            float card_w = std::min(580.0f, vp->Size.x - 40.0f);
            float card_h = 320.0f;

            // Decidir se posiciona o card abaixo ou acima do Spotlight
            float space_below = (vp->Pos.y + vp->Size.y) - s_max.y;
            float space_above = s_min.y - vp->Pos.y;

            float card_x = vp->Pos.x + (vp->Size.x - card_w) * 0.5f;
            float card_y;

            if (space_below >= card_h + 30.0f) {
                card_y = s_max.y + 18.0f;
            } else if (space_above >= card_h + 30.0f) {
                card_y = s_min.y - card_h - 18.0f;
            } else {
                // Centralizado horizontalmente no rodapé
                card_y = vp->Pos.y + vp->Size.y - card_h - 25.0f;
            }

            ImVec2 c_pos = ImVec2(card_x, card_y);
            ImGui::SetNextWindowPos(c_pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(card_w, card_h), ImGuiCond_Always);

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.8f);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.12f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.75f, 0.15f, 0.90f));

            if (ImGui::Begin("##DJTourCardWindow", nullptr, flags)) {
                // Header com Badge e Botão Fechar
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_GRADUATION_CAP " GUIA DIDÁTICO ABDUCTION STUDIO");
                ImGui::SameLine();
                ImGui::TextDisabled("|");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Passo %d de %d", m_current_step + 1, (int)TourStepId::COUNT);

                ImGui::SameLine(ImGui::GetWindowWidth() - 32.0f);
                if (ImGui::Button("X##CloseTourBtn", ImVec2(24, 20))) {
                    m_active = false;
                }

                ImGui::Separator();
                ImGui::Spacing();

                // Textos Didáticos para Leigos
                renderStepContent(m_current_step);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão "Ouvir Demonstração Sonora"
                auto& player = KuroAudio::DJPreviewPlayer::getInstance();
                bool is_playing_demo = player.isPlaying();

                if (is_playing_demo) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.65f, 0.35f, 1.0f));
                    if (ImGui::Button(ICON_FA_STOP " Parar Exemplo Sonoro", ImVec2(230.0f, 30.0f))) {
                        player.stop();
                    }
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), ICON_FA_VOLUME_HIGH " Tocando no fone/master...");
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
                    if (ImGui::Button(ICON_FA_PLAY " Ouvir Exemplo Didático (3s)", ImVec2(230.0f, 30.0f))) {
                        playStepAudioDemo(m_current_step);
                    }
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Ouvir na prática a demonstração sonora deste conceito!");
                    }
                }

                ImGui::Spacing();

                // Rodapé de Navegação: [Anterior] [Pills de Passo] [Próximo]
                if (m_current_step > 0) {
                    if (ImGui::Button(ICON_FA_ARROW_LEFT " Anterior", ImVec2(90.0f, 26.0f))) {
                        prevStep();
                    }
                } else {
                    ImGui::InvisibleButton("##PrevDummy", ImVec2(90.0f, 26.0f));
                }

                ImGui::SameLine(0, 16.0f);
                // Indicador de Progresso (Pills interativas)
                for (int s = 0; s < (int)TourStepId::COUNT; s++) {
                    ImGui::PushID(s);
                    bool is_current = (s == m_current_step);
                    ImVec4 col = is_current ? ImVec4(1.0f, 0.75f, 0.1f, 1.0f) : ImVec4(0.25f, 0.35f, 0.5f, 0.8f);
                    ImGui::PushStyleColor(ImGuiCol_Button, col);
                    if (ImGui::Button(is_current ? "[o]" : " o ", ImVec2(24.0f, 22.0f))) {
                        setStep(s);
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                    ImGui::SameLine(0, 4.0f);
                }

                ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
                if (m_current_step < (int)TourStepId::COUNT - 1) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.55f, 0.05f, 1.0f));
                    if (ImGui::Button("Próximo " ICON_FA_ARROW_RIGHT, ImVec2(105.0f, 26.0f))) {
                        nextStep();
                    }
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.75f, 0.35f, 1.0f));
                    if (ImGui::Button(ICON_FA_CHECK " Concluir!", ImVec2(105.0f, 26.0f))) {
                        m_active = false;
                    }
                    ImGui::PopStyleColor();
                }
            }
            ImGui::End();

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
        }

        void renderStepContent(int step) {
            switch (step) {
                case (int)TourStepId::WELCOME_OVERVIEW: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "BEM-VINDO AO ABDUCTION STUDIO NEXT-GEN!");
                    ImGui::TextWrapped("Este é o seu estúdio completo de discotecagem profissional e produção musical ao vivo. "
                                       "Criado tanto para quem nunca tocou uma música na vida quanto para os maiores DJs do planeta.");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_LIGHTBULB " Como funciona a discotecagem?");
                    ImGui::TextWrapped("Você coloca uma música tocando no Deck Esquerdo e prepara a próxima música no Deck Direito. "
                                       "O seu objetivo é juntar as duas batidas com suavidade para que a pista de dança nunca pare!");
                    break;
                }
                case (int)TourStepId::LAYOUT_EDITOR: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "O MODO EDIÇÃO LIVE & CUSTOMIZAÇÃO TOTAL (Ctrl + E)");
                    ImGui::TextWrapped("Você tem liberdade absoluta para redimensionar qualquer componente da tela! "
                                       "Quer pratos de vinil gigantescos como os de Hip-Hop clássico? Aumente o slider 'Vinil/Jog'. "
                                       "Quer knobs de equalização maiores ou fontes gigantes de BPM para enxergar de longe no clube? Tudo é ajustável!");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), ICON_FA_WAND_MAGIC_SPARKLES " Atalho Secreto:");
                    ImGui::TextWrapped("Pressione 'Ctrl + E' ou clique no botão [EDIÇÃO] na barra superior para abrir/fechar a barra de redimensionamento a qualquer momento.");
                    break;
                }
                case (int)TourStepId::DECKS_AND_PLATTER: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "ANATOMIA DOS DECKS (ONDE A MÚSICA TOCA)");
                    ImGui::TextWrapped("Cada Deck é como um toca-discos de alta tecnologia. "
                                       "O grande prato redondo simula o vinil girando: você pode arrastar com o mouse para fazer scratch ou ajustar a batida.");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), ICON_FA_CIRCLE_PLAY " PLAY e CUE:");
                    ImGui::TextWrapped("• PLAY: Inicia ou pausa a reprodução da música.\n"
                                       "• CUE: É o seu trampolim secreto! Ele marca o ponto exato da primeira batida forte para você testar no fone de ouvido antes de soltar.");
                    break;
                }
                case (int)TourStepId::MIXER_AND_EQ: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "A MESA DE SOM (MIXER) & A REGRA DE OURO DO BASS SWAP");
                    ImGui::TextWrapped("O mixer central junta o som dos dois lados. Cada canal tem 3 botões redondos de equalização (EQ):\n"
                                       "• HI (Agudos): Controla o brilho, pratos e palmas.\n"
                                       "• MID (Médios): Controla as vozes dos cantores e instrumentos melódicos.\n"
                                       "• LOW (Graves): Controla o kick e a linha de baixo pesada.");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), ICON_FA_TRIANGLE_EXCLAMATION " Regra de Ouro dos DJs:");
                    ImGui::TextWrapped("Nunca deixe os graves (LOW) das duas músicas no máximo ao mesmo tempo! Isso 'embola' o som. "
                                       "Corte o grave da música antiga antes de entrar o grave da nova (Bass Swap).");
                    break;
                }
                case (int)TourStepId::MULTI_RIBBON_WAVEFORMS: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "LENDO A MÚSICA COM OS OLHOS (WAVEFORMS & METRÔNOMO)");
                    ImGui::TextWrapped("Você não precisa adivinhar o que vai acontecer na música! As ondas empilhadas mostram o futuro:\n"
                                       "• Fita Superior: Agudos (pratos e chimbais).\n"
                                       "• Fita do Meio: Médios (vocais e sintetizadores).\n"
                                       "• Fita Inferior: Graves pesados (sub-bass).");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), ICON_FA_MUSIC " As Cores das Fases da Música:");
                    ImGui::TextWrapped("Na régua de visão geral da pista, cada fase tem sua cor: a calmaria do Break, a subida do Build e a explosão do Drop! "
                                       "O Metrônomo visual pisca em dourado na primeira batida (Beat 1) para você nunca perder a contagem.");
                    break;
                }
                case (int)TourStepId::PERFORMANCE_PADS: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "PERFORMANCE PADS & LOOPS DE VIRADA");
                    ImGui::TextWrapped("Os 8 botões quadrados coloridos abaixo dos pratos são os seus Performance Pads:\n"
                                       "• Hot Cues: Marcam pontos favoritos da música para pular instantaneamente.\n"
                                       "• Beat Loop: Repete um pedaço de 1, 2, 4 ou 8 compassos em ciclo perfeito.");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), ICON_FA_REPEAT " Didática Rocksmith (Loop de Virada):");
                    ImGui::TextWrapped("O Abduction Studio isola automaticamente a virada entre duas músicas para você treinar repetidas vezes até ficar perfeito!");
                    break;
                }
                case (int)TourStepId::LIVE_SYNTH_AND_MIDI: {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "SINTETIZADORES AO VIVO & LIVE MIDI (VOCÊ É O PRODUTOR)");
                    ImGui::TextWrapped("O Abduction Studio não é só para tocar músicas já gravadas. "
                                       "Você tem um sintetizador analógico estilo TB-303 e uma bateria eletrônica de 16 pads para tocar ao vivo sincronizado com as músicas!");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), ICON_FA_MICROCHIP " Teclados & Controladores USB:");
                    ImGui::TextWrapped("Basta plugar qualquer teclado MIDI ou controladora USB: o som é quantizado no tempo perfeito com latência zero.");
                    break;
                }
                default: break;
            }
        }
    };

} // namespace KuroUI
