#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <condition_variable>
#include "../ai/dr_wav.h"
#include "../ai/dr_mp3.h"
#include "../ai/KuroMIR.h"
#include "../utils/Logger.h"
#include "DJPreviewPlayer.h"
#include "DJHarmonicEngine.h"
#include "DJAudioRouting.h"
#include "DJLiveSafety.h"
#include "DJTouchPreviewCue.h"
#include "DJGridEngine.h"
#include "DJLibraryManager.h"
#include "DJRekordboxAnlz.h"
#include "QMBeatGridEngine.h"
#include "../audio/DJMasterIsolator.h"
#include "../audio/DJM10ChannelDSP.h"
#include "../audio/DJReleaseFX.h"
#include "DJHybridMixerDSP.h"
#include "DJLiveInstrumentEngine.h"
#include "DJGainStagingAnalyzer.h"
#include "DJTurntablismEngine.h"
#include "DJTransitionLoopRepeater.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    struct DJHotCue {
        int id = -1;
        double time_sec = 0.0;
        std::string label = "";
        uint32_t color_rgba = 0xFF00FFFF;
        bool active = false;
    };

    struct DJSamplerSlot {
        std::string name;
        std::vector<float> buffer_l;
        std::vector<float> buffer_r;
        double playback_pos = 0.0;
        bool is_playing = false;
        float volume = 0.9f;
    };

    // Filtro Biquad estéreo de precisão para EQ e Sound Color FX
    class DJBiquadFilter {
    private:
        float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float x1_l = 0.0f, x2_l = 0.0f, y1_l = 0.0f, y2_l = 0.0f;
        float x1_r = 0.0f, x2_r = 0.0f, y1_r = 0.0f, y2_r = 0.0f;

    public:
        void setLowPass(float cutoff_hz, float sample_rate, float q = 0.707f) {
            if (cutoff_hz >= sample_rate * 0.48f) {
                a0 = 1.0f; a1 = 0.0f; a2 = 0.0f; b1 = 0.0f; b2 = 0.0f;
                return;
            }
            float w0 = 2.0f * (float)M_PI * cutoff_hz / sample_rate;
            float cos_w = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0 = (1.0f - cos_w) * 0.5f;
            float b_1 = 1.0f - cos_w;
            float b_2 = (1.0f - cos_w) * 0.5f;
            float a_0 = 1.0f + alpha;
            float a_1 = -2.0f * cos_w;
            float a_2 = 1.0f - alpha;

            a0 = b0 / a_0; a1 = b_1 / a_0; a2 = b_2 / a_0;
            b1 = a_1 / a_0; b2 = a_2 / a_0;
        }

        void setHighPass(float cutoff_hz, float sample_rate, float q = 0.707f) {
            if (cutoff_hz <= 20.0f) {
                a0 = 1.0f; a1 = 0.0f; a2 = 0.0f; b1 = 0.0f; b2 = 0.0f;
                return;
            }
            float w0 = 2.0f * (float)M_PI * cutoff_hz / sample_rate;
            float cos_w = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0 = (1.0f + cos_w) * 0.5f;
            float b_1 = -(1.0f + cos_w);
            float b_2 = (1.0f + cos_w) * 0.5f;
            float a_0 = 1.0f + alpha;
            float a_1 = -2.0f * cos_w;
            float a_2 = 1.0f - alpha;

            a0 = b0 / a_0; a1 = b_1 / a_0; a2 = b_2 / a_0;
            b1 = a_1 / a_0; b2 = a_2 / a_0;
        }

        void setBandPass(float cutoff_hz, float sample_rate, float q = 1.4f) {
            float w0 = 2.0f * (float)M_PI * cutoff_hz / sample_rate;
            float cos_w = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0 = alpha;
            float b_1 = 0.0f;
            float b_2 = -alpha;
            float a_0 = 1.0f + alpha;
            float a_1 = -2.0f * cos_w;
            float a_2 = 1.0f - alpha;

            a0 = b0 / a_0; a1 = b_1 / a_0; a2 = b_2 / a_0;
            b1 = a_1 / a_0; b2 = a_2 / a_0;
        }

        void process(float& left, float& right) {
            float y_l = a0 * left + a1 * x1_l + a2 * x2_l - b1 * y1_l - b2 * y2_l;
            x2_l = x1_l; x1_l = left; y2_l = y1_l; y1_l = y_l;
            left = std::clamp(y_l, -3.0f, 3.0f);

            float y_r = a0 * right + a1 * x1_r + a2 * x2_r - b1 * y1_r - b2 * y2_r;
            x2_r = x1_r; x1_r = right; y2_r = y1_r; y1_r = y_r;
            right = std::clamp(y_r, -3.0f, 3.0f);
        }

        void reset() {
            x1_l = x2_l = y1_l = y2_l = 0.0f;
            x1_r = x2_r = y1_r = y2_r = 0.0f;
        }
    };

    // Unidade de Efeitos Pioneer Beat FX (Sincronizada ao BPM - Suíte Oficial Rekordbox 35 Efeitos)
    class DJBeatFXUnit {
    public:
        enum FXType {
            // Coluna 1 (18 Efeitos Oficiais)
            FX_DELAY = 0,
            FX_ECHO,
            FX_SPIRAL,
            FX_REVERB,
            FX_REV_DELAY,
            FX_MT_DELAY,
            FX_UP_ECHO,
            FX_DOWN_ECHO,
            FX_TRANS,
            FX_PAN,
            FX_FILTER,
            FX_FLANGER,
            FX_PHASER,
            FX_SLIP_ROLL,
            FX_ROLL,
            FX_REV_ROLL,
            FX_ROBOT,
            FX_PITCH,

            // Coluna 2 (17 Efeitos Oficiais)
            FX_ENIGMA_JET,
            FX_MOBIUS_SAW,
            FX_MOBIUS_TRI,
            FX_LOW_CUT_ECHO,
            FX_PING_PONG,      // 🔒 PING PONG
            FX_HELIX,          // 🔒 HELIX
            FX_VINYL_BRAKE,    // 🔒 VINYL BRAKE
            FX_STRETCH,        // 🔒 STRETCH
            
            // Coluna 2 - Grupo B
            FX_BPF_ECHO,
            FX_NOISE,
            FX_SPIRAL_UP,
            FX_REVERB_UP,
            FX_HPF_ECHO,
            FX_LPF_ECHO,
            FX_CRUSH_ECHO,
            FX_SPIRAL_DOWN,
            FX_REVERB_DOWN,

            FX_COUNT
        };

        struct FXSlot {
            int type = 0;
            bool enabled = false;          // Padrão OFF, acionado via botão ON dedicado
            float dry_wet = 0.5f;          // Nível de atuação do knob (0.0 a 1.0)
            float beat_fraction = 1.0f;
            int beat_idx = 5;              // Índice na tabela de frações
            
            // Novos controles profissionais de Cauda (Tail Spillover), Reverb e Transição Suave
            float tail_decay = 0.72f;      // Tempo/Decay da cauda (0.10f a 0.95f)
            float reverb_mix = 0.35f;      // Reverb / difusão espacial acoplada ao efeito
            float smooth_gain = 0.0f;      // Envelope de fade-in / fade-out (de-clicking / fade)
            float tail_energy = 0.0f;      // Medidor de sinal residual na cauda
            bool  spillover_active = true; // Mantém cauda pós-corte ligada
        };

        static const char* getFXName(int fx_type) {
            static const char* s_names[FX_COUNT] = {
                "DELAY", "ECHO", "SPIRAL", "REVERB", "REV DELAY", "MT DELAY",
                "UP ECHO", "DOWN ECHO", "TRANS", "PAN", "FILTER", "FLANGER",
                "PHASER", "SLIP ROLL", "ROLL", "REV ROLL", "ROBOT", "PITCH",
                "ENIGMA JET", "MOBIUS SAW", "MOBIUS TRI", "LOW CUT ECHO",
                "PING PONG", "HELIX", "VINYL BRAKE", "STRETCH",
                "BPF ECHO", "NOISE", "SPIRAL UP", "REVERB UP",
                "HPF ECHO", "LPF ECHO", "CRUSH ECHO", "SPIRAL DOWN", "REVERB DOWN"
            };
            if (fx_type >= 0 && fx_type < FX_COUNT) return s_names[fx_type];
            return "ECHO";
        }

        static const char* getShortFXName(int fx_type) {
            static const char* s_short_names[FX_COUNT] = {
                "DELAY", "ECHO", "SPIRAL", "REVERB", "REV DEL", "MT DEL",
                "UP ECHO", "DN ECHO", "TRANS", "PAN", "FILTER", "FLANGER",
                "PHASER", "SLIP ROL", "ROLL", "REV ROL", "ROBOT", "PITCH",
                "ENIGMA", "M-SAW", "M-TRI", "LC ECHO",
                "PINGPONG", "HELIX", "V-BRAKE", "STRETCH",
                "BPF ECHO", "NOISE", "SPIRAL^", "REV UP",
                "HPF ECHO", "LPF ECHO", "CRUSH EC", "SPIRALv", "REV DWN"
            };
            if (fx_type >= 0 && fx_type < FX_COUNT) return s_short_names[fx_type];
            return "ECHO";
        }

        static bool isLockedFX(int fx_type) {
            return false; // Todos os efeitos 100% liberados sem cadeado
        }

        int type = FX_DELAY;
        int channel_assign = 0; // 0=CH1, 1=CH2, 2=SAMPLER, 3=CH3, 4=CH4, 5=MASTER
        int stem_target = 0;    // 0 = MASTER (All), 1 = VOCALS, 2 = DRUMS, 3 = INSTRUMENTS
        bool enabled = false;
        float dry_wet = 0.0f; // 0.0 a 1.0
        float beat_fraction = 1.0f;

        // Modo Multi-FX vs Single-FX
        int fx_mode = 0; // 0 = MULTI-FX [...], 1 = SINGLE-FX [•]
        FXSlot multi_slots[4];
        bool auto_bpm = true;
        double bpm = 120.0;

        // Parâmetros estendidos para modo Single-FX
        float single_tail_decay = 0.72f;
        float single_reverb_mix = 0.35f;
        float single_smooth_gain = 0.0f;
        float single_tail_energy = 0.0f;

    private:
        static const int BUFFER_SIZE = 262144; // ~5.94 segundos a 44.1kHz
        std::vector<float> delay_buf_l[4];
        std::vector<float> delay_buf_r[4];
        int write_pos[4] = { 0, 0, 0, 0 };
        float lfo_phase[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        DJBiquadFilter echo_damp_filter[4];
        DJBiquadFilter fx_lp_filter[4];
        DJBiquadFilter fx_hp_filter[4];
        DJBiquadFilter fx_bp_filter[4];

        // Suporte a Roll (Slip Loop) independente por slot
        int roll_capture_pos[4] = { 0, 0, 0, 0 };
        int roll_length_frames[4] = { 0, 0, 0, 0 };
        bool roll_locked[4] = { false, false, false, false };

        // Suporte a Robot & Pitch
        float robot_phase[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float pitch_phase[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float helix_pitch[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Suporte a Vinyl Brake
        float vinyl_brake_speed[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        double vinyl_brake_pos[4] = { 0.0, 0.0, 0.0, 0.0 };

        // Gerador de Ruído
        uint32_t noise_seed = 0x58249a1c;

    public:
        DJBeatFXUnit() {
            for (int s = 0; s < 4; s++) {
                delay_buf_l[s].resize(BUFFER_SIZE, 0.0f);
                delay_buf_r[s].resize(BUFFER_SIZE, 0.0f);
                echo_damp_filter[s].setLowPass(7500.0f, 44100.0f);
                fx_lp_filter[s].setLowPass(1200.0f, 44100.0f);
                fx_hp_filter[s].setHighPass(600.0f, 44100.0f);
                fx_bp_filter[s].setBandPass(1600.0f, 2.5f, 44100.0f);
            }

            // Slots padrão idênticos ao Rekordbox
            multi_slots[0] = { FX_ENIGMA_JET, false, 0.0f, 4.0f, 7 };
            multi_slots[1] = { FX_DELAY,      false, 0.0f, 4.0f, 7 };
            multi_slots[2] = { FX_DOWN_ECHO,  false, 0.0f, 1.0f, 5 };
            multi_slots[3] = { FX_VINYL_BRAKE, false, 0.0f, 0.75f, 4 };
        }

        void processSingleEffect(int fx_type, float eff_dry_wet, float beat_frac, float* left, float* right, unsigned int frames, double bpm, double sample_rate, int slot_idx,
                                 bool is_slot_enabled = true, float tail_decay = 0.72f, float reverb_mix = 0.35f, float* p_smooth_gain = nullptr, float* p_tail_energy = nullptr) {
            int s = std::clamp(slot_idx, 0, 3);
            double beat_sec = 60.0 / std::clamp(bpm, 40.0, 220.0);
            int delay_frames = (int)(beat_frac * beat_sec * sample_rate);
            if (delay_frames < 64) delay_frames = 64;
            if (delay_frames >= BUFFER_SIZE - 2000) delay_frames = BUFFER_SIZE - 2000;

            float wet_mix = std::clamp(eff_dry_wet, 0.0f, 1.0f);
            float target_smooth = is_slot_enabled ? 1.0f : 0.0f;
            float smooth_gain = p_smooth_gain ? *p_smooth_gain : target_smooth;
            float fb = std::clamp(tail_decay, 0.15f, 0.95f);
            float block_energy = 0.0f;

            for (unsigned int i = 0; i < frames; i++) {
                // Envelope suave (~8ms) para transicao sem estalos (fade-in / fade-out)
                smooth_gain += (target_smooth - smooth_gain) * 0.005f;

                float in_l = left[i];
                float in_r = right[i];

                // Audio novo alimentado no efeito (faded out quando desligado)
                float fx_in_l = in_l * smooth_gain;
                float fx_in_r = in_r * smooth_gain;

                float fx_l = 0.0f;
                float fx_r = 0.0f;

                switch (fx_type) {
                    case FX_DELAY: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos];
                        fx_r = delay_buf_r[s][r_pos];
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;
                        break;
                    }
                    case FX_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos];
                        float del_r = delay_buf_r[s][r_pos];
                        echo_damp_filter[s].process(del_l, del_r);
                        fx_l = del_l;
                        fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_SPIRAL: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos] * 1.05f;
                        fx_r = delay_buf_r[s][r_pos] * 1.05f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(fx_l * fb, -1.5f, 1.5f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(fx_r * fb, -1.5f, 1.5f);
                        break;
                    }
                    case FX_REVERB: {
                        int t1 = delay_frames / 4; if (t1 < 8) t1 = 8;
                        int t2 = delay_frames / 3; if (t2 < 12) t2 = 12;
                        int t3 = delay_frames / 2; if (t3 < 16) t3 = 16;
                        int t4 = delay_frames;
                        int r1 = (write_pos[s] - t1 + BUFFER_SIZE) % BUFFER_SIZE;
                        int r2 = (write_pos[s] - t2 + BUFFER_SIZE) % BUFFER_SIZE;
                        int r3 = (write_pos[s] - t3 + BUFFER_SIZE) % BUFFER_SIZE;
                        int r4 = (write_pos[s] - t4 + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = (delay_buf_l[s][r1] * 0.35f + delay_buf_l[s][r2] * 0.30f + delay_buf_r[s][r3] * 0.20f + delay_buf_l[s][r4] * 0.15f);
                        fx_r = (delay_buf_r[s][r1] * 0.30f + delay_buf_r[s][r2] * 0.35f + delay_buf_l[s][r3] * 0.20f + delay_buf_r[s][r4] * 0.15f);
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;
                        break;
                    }
                    case FX_REV_DELAY: {
                        int mod_pos = write_pos[s] % delay_frames;
                        int r_pos = (write_pos[s] - mod_pos + (delay_frames - 1 - mod_pos) - delay_frames + BUFFER_SIZE * 2) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos];
                        fx_r = delay_buf_r[s][r_pos];
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.75f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.75f);
                        break;
                    }
                    case FX_MT_DELAY: {
                        int t1 = delay_frames / 3;
                        int t2 = (delay_frames * 2) / 3;
                        int t3 = delay_frames;
                        int r1 = (write_pos[s] - t1 + BUFFER_SIZE) % BUFFER_SIZE;
                        int r2 = (write_pos[s] - t2 + BUFFER_SIZE) % BUFFER_SIZE;
                        int r3 = (write_pos[s] - t3 + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r1] * 0.6f + delay_buf_l[s][r3] * 0.4f;
                        fx_r = delay_buf_r[s][r2] * 0.6f + delay_buf_r[s][r3] * 0.4f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + (fx_l + fx_r) * (fb * 0.40f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + (fx_l + fx_r) * (fb * 0.40f);
                        break;
                    }
                    case FX_UP_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos] * 1.08f;
                        float del_r = delay_buf_r[s][r_pos] * 1.08f;
                        fx_hp_filter[s].process(del_l, del_r);
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(del_l * fb, -1.2f, 1.2f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(del_r * fb, -1.2f, 1.2f);
                        break;
                    }
                    case FX_DOWN_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos] * 0.93f;
                        float del_r = delay_buf_r[s][r_pos] * 0.93f;
                        fx_lp_filter[s].process(del_l, del_r);
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_TRANS: {
                        float gate_phase = (float)(write_pos[s] % delay_frames) / (float)delay_frames;
                        float gate = (gate_phase < 0.5f) ? 1.0f : 0.0f;
                        fx_l = in_l * gate;
                        fx_r = in_r * gate;
                        break;
                    }
                    case FX_PAN: {
                        lfo_phase[s] += 1.0f / (float)delay_frames;
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float pan = std::sin(lfo_phase[s] * 2.0f * (float)M_PI);
                        fx_l = in_l * (1.0f - pan * 0.85f);
                        fx_r = in_r * (1.0f + pan * 0.85f);
                        break;
                    }
                    case FX_FILTER: {
                        lfo_phase[s] += 1.0f / (float)(delay_frames * 2);
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float mod = 0.5f + 0.5f * std::sin(lfo_phase[s] * 2.0f * (float)M_PI);
                        float cut = 350.0f + mod * 7500.0f;
                        fx_lp_filter[s].setLowPass(cut, (float)sample_rate);
                        float fl = in_l, fr = in_r;
                        fx_lp_filter[s].process(fl, fr);
                        fx_l = fl; fx_r = fr;
                        break;
                    }
                    case FX_FLANGER: {
                        lfo_phase[s] += 1.0f / (float)(delay_frames * 2);
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float mod = (std::sin(lfo_phase[s] * 2.0f * (float)M_PI) + 1.0f) * 0.5f;
                        int mod_delay = (int)(sample_rate * 0.001f + mod * sample_rate * 0.007f);
                        int r_pos = (write_pos[s] - mod_delay + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos];
                        fx_r = delay_buf_r[s][r_pos];
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.75f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.75f);
                        break;
                    }
                    case FX_PHASER: {
                        lfo_phase[s] += 1.0f / (float)(delay_frames * 4);
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float mod = (std::sin(lfo_phase[s] * 2.0f * (float)M_PI) + 1.0f) * 0.5f;
                        int mod_delay = (int)(sample_rate * 0.0005f + mod * sample_rate * 0.0025f);
                        int r_pos = (write_pos[s] - mod_delay + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = in_l - delay_buf_l[s][r_pos] * 0.7f;
                        fx_r = in_r - delay_buf_r[s][r_pos] * 0.7f;
                        delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.4f;
                        delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.4f;
                        break;
                    }
                    case FX_SLIP_ROLL:
                    case FX_ROLL: {
                        if (!roll_locked[s]) {
                            roll_capture_pos[s] = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                            roll_length_frames[s] = delay_frames;
                            roll_locked[s] = true;
                        }
                        int r_idx = (roll_capture_pos[s] + (write_pos[s] % roll_length_frames[s])) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_idx];
                        fx_r = delay_buf_r[s][r_idx];
                        delay_buf_l[s][write_pos[s]] = in_l;
                        delay_buf_r[s][write_pos[s]] = in_r;
                        break;
                    }
                    case FX_REV_ROLL: {
                        if (!roll_locked[s]) {
                            roll_capture_pos[s] = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                            roll_length_frames[s] = delay_frames;
                            roll_locked[s] = true;
                        }
                        int mod_pos = write_pos[s] % roll_length_frames[s];
                        int r_idx = (roll_capture_pos[s] + (roll_length_frames[s] - 1 - mod_pos)) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_idx];
                        fx_r = delay_buf_r[s][r_idx];
                        delay_buf_l[s][write_pos[s]] = in_l;
                        delay_buf_r[s][write_pos[s]] = in_r;
                        break;
                    }
                    case FX_ROBOT: {
                        robot_phase[s] += 110.0f / (float)sample_rate;
                        if (robot_phase[s] > 1.0f) robot_phase[s] -= 1.0f;
                        float carrier = std::sin(robot_phase[s] * 2.0f * (float)M_PI);
                        fx_l = in_l * carrier * 1.4f;
                        fx_r = in_r * carrier * 1.4f;
                        break;
                    }
                    case FX_PITCH: {
                        pitch_phase[s] += 1.0f / (float)(delay_frames * 0.25f);
                        if (pitch_phase[s] > 1.0f) pitch_phase[s] -= 1.0f;
                        float off = pitch_phase[s] * 1024.0f;
                        int r_pos = (write_pos[s] - (int)off + BUFFER_SIZE) % BUFFER_SIZE;
                        float win = 0.5f * (1.0f - std::cos(pitch_phase[s] * 2.0f * (float)M_PI));
                        fx_l = delay_buf_l[s][r_pos] * win * 1.5f;
                        fx_r = delay_buf_r[s][r_pos] * win * 1.5f;
                        delay_buf_l[s][write_pos[s]] = in_l;
                        delay_buf_r[s][write_pos[s]] = in_r;
                        break;
                    }
                    case FX_ENIGMA_JET: {
                        lfo_phase[s] += 1.0f / (float)(delay_frames * 3);
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float sweep = 0.5f + 0.5f * std::sin(lfo_phase[s] * 2.0f * (float)M_PI);
                        int jet_del = (int)(sample_rate * 0.0003f + sweep * sample_rate * 0.0055f);
                        int r_pos = (write_pos[s] - jet_del + BUFFER_SIZE) % BUFFER_SIZE;
                        noise_seed = noise_seed * 1664525u + 1013904223u;
                        float in_mag = std::max(std::abs(in_l), std::abs(in_r));
                        float hiss_gate = std::min(1.0f, (in_mag + std::abs(delay_buf_l[s][r_pos])) * 15.0f);
                        float hiss = (((float)(noise_seed & 0xFFFF) / 32768.0f) - 1.0f) * 0.04f * sweep * hiss_gate;
                        fx_l = delay_buf_l[s][r_pos] * 1.15f + hiss;
                        fx_r = delay_buf_r[s][r_pos] * 1.15f + hiss;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.95f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.95f);
                        break;
                    }
                    case FX_MOBIUS_SAW: {
                        lfo_phase[s] += 1.0f / (float)(delay_frames * 4);
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float saw1 = fmod(lfo_phase[s], 1.0f) * 2.0f - 1.0f;
                        float saw2 = fmod(lfo_phase[s] + 0.5f, 1.0f) * 2.0f - 1.0f;
                        int mod_del = (int)(sample_rate * 0.0008f + (saw1 * 0.5f + 0.5f) * sample_rate * 0.004f);
                        int r_pos = (write_pos[s] - mod_del + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos] * 0.9f + in_l * saw2 * 0.3f;
                        fx_r = delay_buf_r[s][r_pos] * 0.9f + in_r * saw1 * 0.3f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.70f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.70f);
                        break;
                    }
                    case FX_MOBIUS_TRI: {
                        lfo_phase[s] += 1.0f / (float)(delay_frames * 4);
                        if (lfo_phase[s] > 1.0f) lfo_phase[s] -= 1.0f;
                        float tri = 2.0f * std::abs(2.0f * (lfo_phase[s] - std::floor(lfo_phase[s] + 0.5f))) - 1.0f;
                        int mod_del = (int)(sample_rate * 0.001f + (tri * 0.5f + 0.5f) * sample_rate * 0.005f);
                        int r_pos = (write_pos[s] - mod_del + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos];
                        fx_r = delay_buf_r[s][r_pos];
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.75f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.75f);
                        break;
                    }
                    case FX_LOW_CUT_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos];
                        float del_r = delay_buf_r[s][r_pos];
                        fx_hp_filter[s].process(del_l, del_r);
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_PING_PONG: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_r[s][r_pos];
                        fx_r = delay_buf_l[s][r_pos];
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;
                        break;
                    }
                    case FX_HELIX: {
                        if (!roll_locked[s]) {
                            roll_capture_pos[s] = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                            roll_length_frames[s] = delay_frames;
                            roll_locked[s] = true;
                        }
                        helix_pitch[s] += 0.0001f;
                        if (helix_pitch[s] > 1.8f) helix_pitch[s] = 1.0f;
                        int r_idx = (roll_capture_pos[s] + (int)((write_pos[s] % roll_length_frames[s]) * helix_pitch[s])) % BUFFER_SIZE;
                        float h_l = delay_buf_l[s][r_idx];
                        float h_r = delay_buf_r[s][r_idx];
                        fx_hp_filter[s].process(h_l, h_r);
                        fx_l = h_l; fx_r = h_r;
                        delay_buf_l[s][write_pos[s]] = in_l;
                        delay_buf_r[s][write_pos[s]] = in_r;
                        break;
                    }
                    case FX_VINYL_BRAKE: {
                        if (vinyl_brake_speed[s] > 0.01f) {
                            vinyl_brake_speed[s] = std::max(0.0f, vinyl_brake_speed[s] - (float)(1.0 / (delay_frames * 1.5)));
                        }
                        vinyl_brake_pos[s] += (double)vinyl_brake_speed[s];
                        int r_pos = ((int)vinyl_brake_pos[s]) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos] * vinyl_brake_speed[s];
                        fx_r = delay_buf_r[s][r_pos] * vinyl_brake_speed[s];
                        delay_buf_l[s][write_pos[s]] = in_l;
                        delay_buf_r[s][write_pos[s]] = in_r;
                        break;
                    }
                    case FX_STRETCH: {
                        int grain_len = 1024;
                        int grain_pos = write_pos[s] % grain_len;
                        float win = 0.5f * (1.0f - std::cos((float)grain_pos / (float)grain_len * 2.0f * (float)M_PI));
                        int r_pos = (write_pos[s] - grain_pos + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos] * win * 1.8f;
                        fx_r = delay_buf_r[s][r_pos] * win * 1.8f;
                        delay_buf_l[s][write_pos[s]] = in_l;
                        delay_buf_r[s][write_pos[s]] = in_r;
                        break;
                    }
                    case FX_BPF_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos];
                        float del_r = delay_buf_r[s][r_pos];
                        fx_bp_filter[s].process(del_l, del_r);
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_NOISE: {
                        noise_seed = noise_seed * 1664525u + 1013904223u;
                        float raw_n = (((float)(noise_seed & 0xFFFF) / 32768.0f) - 1.0f) * 0.35f;
                        float n_l = raw_n, n_r = raw_n;
                        fx_bp_filter[s].process(n_l, n_r);
                        fx_l = in_l * 0.7f + n_l;
                        fx_r = in_r * 0.7f + n_r;
                        break;
                    }
                    case FX_SPIRAL_UP: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos] * 1.08f;
                        fx_r = delay_buf_r[s][r_pos] * 1.08f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(fx_l * fb, -1.5f, 1.5f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(fx_r * fb, -1.5f, 1.5f);
                        break;
                    }
                    case FX_REVERB_UP: {
                        int t1 = delay_frames / 3;
                        int r1 = (write_pos[s] - t1 + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r1] * 1.06f;
                        fx_r = delay_buf_r[s][r1] * 1.06f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(fx_l * fb, -1.5f, 1.5f);
                        delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(fx_r * fb, -1.5f, 1.5f);
                        break;
                    }
                    case FX_HPF_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos];
                        float del_r = delay_buf_r[s][r_pos];
                        fx_hp_filter[s].process(del_l, del_r);
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_LPF_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos];
                        float del_r = delay_buf_r[s][r_pos];
                        fx_lp_filter[s].process(del_l, del_r);
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_CRUSH_ECHO: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        float del_l = delay_buf_l[s][r_pos];
                        float del_r = delay_buf_r[s][r_pos];
                        del_l = std::floor(del_l * 32.0f) / 32.0f;
                        del_r = std::floor(del_r * 32.0f) / 32.0f;
                        fx_l = del_l; fx_r = del_r;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;
                        break;
                    }
                    case FX_SPIRAL_DOWN: {
                        int r_pos = (write_pos[s] - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r_pos] * 0.92f;
                        fx_r = delay_buf_r[s][r_pos] * 0.92f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;
                        break;
                    }
                    case FX_REVERB_DOWN: {
                        int t1 = delay_frames / 3;
                        int r1 = (write_pos[s] - t1 + BUFFER_SIZE) % BUFFER_SIZE;
                        fx_l = delay_buf_l[s][r1] * 0.94f;
                        fx_r = delay_buf_r[s][r1] * 0.94f;
                        delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;
                        delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;
                        break;
                    }
                    default: {
                        fx_l = in_l;
                        fx_r = in_r;
                        break;
                    }
                }

                // Injecao de Reverb Difuso / Espacial nos Efeitos ("reverbs nos efeitos")
                if (reverb_mix > 0.02f) {
                    int rt1 = (write_pos[s] - delay_frames / 5 + BUFFER_SIZE) % BUFFER_SIZE;
                    int rt2 = (write_pos[s] - delay_frames / 3 + BUFFER_SIZE) % BUFFER_SIZE;
                    int rt3 = (write_pos[s] - (delay_frames * 2) / 3 + BUFFER_SIZE) % BUFFER_SIZE;
                    float rev_d_l = (delay_buf_l[s][rt1] * 0.35f + delay_buf_r[s][rt2] * 0.35f + delay_buf_l[s][rt3] * 0.30f);
                    float rev_d_r = (delay_buf_r[s][rt1] * 0.35f + delay_buf_l[s][rt2] * 0.35f + delay_buf_l[s][rt3] * 0.30f);
                    fx_l = fx_l * (1.0f - reverb_mix * 0.35f) + rev_d_l * (reverb_mix * 0.65f);
                    fx_r = fx_r * (1.0f - reverb_mix * 0.35f) + rev_d_r * (reverb_mix * 0.65f);
                }

                // Quando o slot esta desligado mas a cauda continua, o som seco (dry) permanece 100% limpo
                float dry_mix = is_slot_enabled ? (1.0f - wet_mix * 0.45f) : 1.0f;
                if (is_slot_enabled && (fx_type == FX_TRANS || fx_type == FX_VINYL_BRAKE || fx_type == FX_ROLL || fx_type == FX_SLIP_ROLL || fx_type == FX_REV_ROLL)) {
                    dry_mix = 1.0f - wet_mix;
                }

                if (eff_dry_wet <= 0.001f && !is_slot_enabled) {
                    roll_locked[s] = false;
                    vinyl_brake_speed[s] = 1.0f;
                    vinyl_brake_pos[s] = (double)write_pos[s];
                }

                left[i] = in_l * dry_mix + fx_l * wet_mix;
                right[i] = in_r * dry_mix + fx_r * wet_mix;

                block_energy += std::abs(fx_l) + std::abs(fx_r);
                write_pos[s] = (write_pos[s] + 1) % BUFFER_SIZE;
            }

            if (p_smooth_gain) *p_smooth_gain = smooth_gain;
            if (p_tail_energy) *p_tail_energy = (*p_tail_energy) * 0.92f + (block_energy / (float)frames) * 0.08f;
        }

        void processBlock(float* left, float* right, unsigned int frames, double track_bpm, double sample_rate) {
            double eff_bpm = (auto_bpm || bpm <= 30.0) ? track_bpm : bpm;
            if (eff_bpm <= 10.0) eff_bpm = 120.0;
            if (fx_mode == 0) {
                // Multi-FX: Processa qualquer slot onde o botao ON esta ligado OU houver cauda residual tocando
                for (int s = 0; s < 4; s++) {
                    auto& slot = multi_slots[s];
                    bool has_tail = (slot.tail_energy > 0.0001f || slot.smooth_gain > 0.001f);
                    if (slot.enabled || has_tail) {
                        processSingleEffect(slot.type, slot.dry_wet, slot.beat_fraction, left, right, frames, eff_bpm, sample_rate, s,
                                            slot.enabled, slot.tail_decay, slot.reverb_mix, &slot.smooth_gain, &slot.tail_energy);
                    }
                }
            } else {
                // Single-FX: Processa se enabled ou cauda ativa
                bool has_tail = (single_tail_energy > 0.0001f || single_smooth_gain > 0.001f);
                if (enabled || has_tail) {
                    processSingleEffect(type, dry_wet, beat_fraction, left, right, frames, eff_bpm, sample_rate, 0,
                                        enabled, single_tail_decay, single_reverb_mix, &single_smooth_gain, &single_tail_energy);
                }
            }
        }
    };

    // Limiter Brickwall Transparente & Auto-Gain Estilo Pioneer DJM-V10
    // Evita qualquer ceifamento digital (clipping) no barramento Master durante transições de 4 decks
    class DJMasterLimiter {
    public:
        bool enabled = true;
        float ceiling_linear = 0.9885f; // -0.1 dBFS teto estrito contra inter-sample peaks
        bool auto_gain_enabled = true;
        
        // Master Bus Glue Compressor (Colagem analógica estilo CDJ-3000 / DJM-900NXS2 / DJM-V10)
        bool glue_comp_enabled = true;
        float glue_comp_amount = 0.35f; // 35% glue compressor (suave e encorpado)
        float glue_envelope = 0.0f;

        // Telemetria para UI / HUD
        float gain_reduction = 1.0f; // 1.0 = 0dB de redução, 0.5 = -6dB de atenuação
        float current_peak = 0.0f;
        float current_rms = 0.0f;
        bool is_limiting = false;

    private:
        static const int LOOKAHEAD_FRAMES = 48; // ~1.08ms a 44.1kHz
        std::vector<float> lookahead_buf_l;
        std::vector<float> lookahead_buf_r;
        int lookahead_write_pos = 0;

        float envelope = 0.0f;
        float auto_gain_factor = 1.0f;
        float smoothed_gr = 1.0f;

    public:
        DJMasterLimiter() {
            lookahead_buf_l.assign(LOOKAHEAD_FRAMES, 0.0f);
            lookahead_buf_r.assign(LOOKAHEAD_FRAMES, 0.0f);
        }

        void reset() {
            std::fill(lookahead_buf_l.begin(), lookahead_buf_l.end(), 0.0f);
            std::fill(lookahead_buf_r.begin(), lookahead_buf_r.end(), 0.0f);
            lookahead_write_pos = 0;
            envelope = 0.0f;
            glue_envelope = 0.0f;
            auto_gain_factor = 1.0f;
            smoothed_gr = 1.0f;
            gain_reduction = 1.0f;
            is_limiting = false;
        }

        void processBlock(float* left, float* right, unsigned int frames, double sample_rate) {
            if (!enabled) {
                gain_reduction = 1.0f;
                is_limiting = false;
                return;
            }

            // 0. Master Bus Glue Compressor Estilo Studio/DJM-V10
            if (glue_comp_enabled && glue_comp_amount > 0.01f) {
                float glue_att = (float)std::exp(-1.0 / (std::max(1000.0, sample_rate) * 0.025)); // 25ms attack preserva transientes
                float glue_rel = (float)std::exp(-1.0 / (std::max(1000.0, sample_rate) * 0.090)); // 90ms release
                float glue_thresh = std::pow(10.0f, (-14.0f * glue_comp_amount) / 20.0f);
                float glue_ratio = 1.0f + 1.25f * glue_comp_amount; // ratio suave 1:1 até 2.25:1
                float glue_makeup = std::pow(10.0f, (3.2f * glue_comp_amount) / 20.0f); // até +3.2dB de ganho encorpado

                for (unsigned int i = 0; i < frames; i++) {
                    float pk = std::max(std::abs(left[i]), std::abs(right[i]));
                    if (pk > glue_envelope) {
                        glue_envelope = glue_att * glue_envelope + (1.0f - glue_att) * pk;
                    } else {
                        glue_envelope = glue_rel * glue_envelope + (1.0f - glue_rel) * pk;
                    }

                    float g_gr = 1.0f;
                    if (glue_envelope > glue_thresh) {
                        float diff = glue_envelope / glue_thresh;
                        g_gr = std::pow(diff, (1.0f / glue_ratio) - 1.0f);
                    }
                    left[i] = left[i] * g_gr * glue_makeup;
                    right[i] = right[i] * g_gr * glue_makeup;
                }
            }

            // Coeficientes de Ataque e Release dependentes do Sample Rate
            // Ataque ultra-rápido analógico (0.1ms)
            float attack_coeff = (float)std::exp(-1.0 / (std::max(1000.0, sample_rate) * 0.0001));
            // Release duplo analógico: rápido (25ms) para transientes curtos, lento (100ms) para corpo de graves
            float release_fast = (float)std::exp(-1.0 / (std::max(1000.0, sample_rate) * 0.025));
            float release_slow = (float)std::exp(-1.0 / (std::max(1000.0, sample_rate) * 0.120));

            float max_peak_in_block = 0.0f;
            float sum_sq = 0.0f;

            // 1. Fase de Auto-Gain Dinâmico para Transições de 4 Decks
            if (auto_gain_enabled) {
                for (unsigned int i = 0; i < frames; i++) {
                    float s_max = std::max(std::abs(left[i]), std::abs(right[i]));
                    if (s_max > max_peak_in_block) max_peak_in_block = s_max;
                    sum_sq += (left[i] * left[i] + right[i] * right[i]) * 0.5f;
                }
                float block_rms = (frames > 0) ? std::sqrt(sum_sq / (float)frames) : 0.0f;
                current_rms = current_rms * 0.92f + block_rms * 0.08f;

                // Se a soma linear de decks exceder 1.25 (+2dB) ou RMS estiver muito alto,
                // reduz o ganho prévio suavemente para manter a música transparente sem esmagamento
                float target_ag = 1.0f;
                if (max_peak_in_block > 1.25f) {
                    target_ag = 1.25f / max_peak_in_block;
                }
                if (target_ag < auto_gain_factor) {
                    auto_gain_factor = auto_gain_factor * 0.85f + target_ag * 0.15f; // Rápida acomodação
                } else {
                    auto_gain_factor = auto_gain_factor * 0.995f + target_ag * 0.005f; // Retorno lento e imperceptível
                }
            } else {
                auto_gain_factor = 1.0f;
            }

            float min_gr_in_block = 1.0f;

            // 2. Loop de Limiting Brickwall com Lookahead & Saturação Suave Analógica estilo DJM-V10
            for (unsigned int i = 0; i < frames; i++) {
                float in_l = left[i] * auto_gain_factor;
                float in_r = right[i] * auto_gain_factor;

                // Detecção de pico na entrada antecipada
                float peak = std::max(std::abs(in_l), std::abs(in_r));
                if (peak > max_peak_in_block) max_peak_in_block = peak;

                // Envelope follower program-dependent
                if (peak > envelope) {
                    envelope = attack_coeff * envelope + (1.0f - attack_coeff) * peak;
                } else {
                    // Seleciona release dependente do nível relativo do envelope
                    float rel = (envelope > ceiling_linear * 1.2f) ? release_fast : release_slow;
                    envelope = rel * envelope + (1.0f - rel) * peak;
                }

                // Cálculo do fator de redução de ganho estrito
                float gr = 1.0f;
                if (envelope > ceiling_linear) {
                    gr = ceiling_linear / envelope;
                }

                if (gr < min_gr_in_block) min_gr_in_block = gr;

                // Armazena no lookahead buffer
                lookahead_buf_l[lookahead_write_pos] = in_l;
                lookahead_buf_r[lookahead_write_pos] = in_r;

                // Recupera a amostra atrasada pelo lookahead
                int read_pos = (lookahead_write_pos + 1) % LOOKAHEAD_FRAMES;
                float delayed_l = lookahead_buf_l[read_pos];
                float delayed_r = lookahead_buf_r[read_pos];

                // Aplica a redução de ganho na amostra atrasada
                float out_samp_l = delayed_l * gr;
                float out_samp_r = delayed_r * gr;

                // Curva de Saturação Soft-Knee Analógica Hermite quando se aproxima do teto absoluto (-0.1 dBFS)
                auto soft_knee_clip = [this](float x) -> float {
                    float sign = (x < 0.0f) ? -1.0f : 1.0f;
                    float ax = std::abs(x);
                    if (ax <= ceiling_linear * 0.90f) {
                        return x;
                    } else if (ax < ceiling_linear) {
                        float k = ceiling_linear * 0.90f;
                        float diff = (ax - k) / (ceiling_linear - k);
                        return sign * (k + (ceiling_linear - k) * (diff - 0.3333f * diff * diff * diff));
                    } else {
                        return sign * ceiling_linear;
                    }
                };

                left[i] = soft_knee_clip(out_samp_l);
                right[i] = soft_knee_clip(out_samp_r);

                lookahead_write_pos = (lookahead_write_pos + 1) % LOOKAHEAD_FRAMES;
            }

            smoothed_gr = smoothed_gr * 0.80f + min_gr_in_block * 0.20f;
            gain_reduction = smoothed_gr;
            current_peak = max_peak_in_block;
            is_limiting = (gain_reduction < 0.985f);
        }
    };

    // =========================================================================
    // PioneerChannelCompressor: Compressor de Canal Único Estilo Pioneer DJM-V10 / DJM-A9
    // Proporciona punch analógico, encorpa faixas antigas/fracas, preserva transientes
    // de kick/snare (ataque 8ms), e compensa automaticamente o ganho sem ceifamento.
    // =========================================================================
    class PioneerChannelCompressor {
    public:
        float amount = 0.0f;             // 0.0 = Bypass, 1.0 = Máxima presença/encorpamento
        float gain_reduction = 1.0f;     // 1.0 = 0dB GR, 0.5 = -6dB GR (Linear)
        float gain_reduction_db = 0.0f;  // Telemetria em dB para metering
        float current_envelope = 0.0f;

    private:
        float envelope = 0.0f;
        float smoothed_gr = 1.0f;
        float current_sr = 48000.0f;

    public:
        PioneerChannelCompressor() {
            reset();
        }

        void reset() {
            envelope = 0.0f;
            smoothed_gr = 1.0f;
            gain_reduction = 1.0f;
            gain_reduction_db = 0.0f;
            current_envelope = 0.0f;
        }

        void processBlock(float* left, float* right, unsigned int frames, double sample_rate) {
            if (amount < 0.005f) {
                gain_reduction = 1.0f;
                gain_reduction_db = 0.0f;
                smoothed_gr = 1.0f;
                envelope = 0.0f;
                current_envelope = 0.0f;
                return;
            }

            float sr = (float)std::max(1000.0, sample_rate);
            current_sr = sr;

            // Attack analógico: 8ms (Permite que o "punch" e transiente do kick passem limpos antes de comprimir)
            float attack_coeff = std::exp(-1.0f / (sr * 0.008f));
            // Release musical calibrado para música eletrônica/DJ: 70ms (evita bombeamento e dá sustentação ao grave)
            float release_coeff = std::exp(-1.0f / (sr * 0.070f));

            // Curva Pioneer DJM-V10 COMP Knob:
            // Threshold dinâmico: de 0 dBFS até -24 dBFS
            float threshold_db = -24.0f * amount;
            // Ratio dinâmico: de 1:1 até 4.5:1
            float ratio = 1.0f + 3.5f * amount;
            // Auto-Makeup Gain compensatório: até +8.0 dB (mantém faixas menos masterizadas no mesmo volume de bangers modernos)
            float makeup_db = (24.0f * amount) * (1.0f - 1.0f / ratio) * 0.85f;
            float makeup_lin = std::pow(10.0f, makeup_db / 20.0f);

            // Largura do Soft-Knee (4 dB) para transição hiper suave e sem artefatos
            const float knee_db = 4.0f;
            const float half_knee = knee_db * 0.5f;

            float min_gr = 1.0f;

            for (unsigned int i = 0; i < frames; i++) {
                float in_l = left[i];
                float in_r = right[i];

                float peak = std::max(std::abs(in_l), std::abs(in_r));

                if (peak > envelope) {
                    envelope = attack_coeff * envelope + (1.0f - attack_coeff) * peak;
                } else {
                    envelope = release_coeff * envelope + (1.0f - release_coeff) * peak;
                }

                float gr_lin = 1.0f;
                if (envelope > 1e-5f) {
                    float env_db = 20.0f * std::log10(envelope);
                    float delta_db = env_db - threshold_db;

                    if (delta_db <= -half_knee) {
                        gr_lin = 1.0f;
                    } else if (delta_db < half_knee) {
                        float x = delta_db + half_knee;
                        float comp_db = (1.0f - 1.0f / ratio) * (x * x / (2.0f * knee_db));
                        gr_lin = std::pow(10.0f, -comp_db / 20.0f);
                    } else {
                        float comp_db = delta_db * (1.0f - 1.0f / ratio);
                        gr_lin = std::pow(10.0f, -comp_db / 20.0f);
                    }
                }

                if (gr_lin < min_gr) min_gr = gr_lin;

                // Aplica redução de ganho e ganho compensatório
                float out_l = in_l * gr_lin * makeup_lin;
                float out_r = in_r * gr_lin * makeup_lin;

                // Saturação harmônica suave analógica Pioneer (emulação DAC Sabre / estágio analógico)
                auto warm_saturate = [](float x) -> float {
                    if (std::abs(x) < 0.85f) return x;
                    float sign = (x < 0.0f) ? -1.0f : 1.0f;
                    float ax = std::abs(x);
                    return sign * (0.85f + 0.15f * std::tanh((ax - 0.85f) * 2.2f));
                };

                left[i] = warm_saturate(out_l);
                right[i] = warm_saturate(out_r);
            }

            smoothed_gr = smoothed_gr * 0.70f + min_gr * 0.30f;
            gain_reduction = smoothed_gr;
            gain_reduction_db = (smoothed_gr > 0.001f) ? (20.0f * std::log10(smoothed_gr)) : -40.0f;
            current_envelope = envelope;
        }
    };

    class DJDeck {
    public:
        int deck_id = 0; // 0 = Deck A, 1 = Deck B
        std::string track_path = "";
        std::string track_title = "Nenhuma Faixa Carregada";
        std::string track_artist = "Arraste um WAV ou busque no Spotify";
        std::string cover_art_path = "";
        unsigned int gl_cover_texture = 0;
        bool has_cover_texture = false;
        
        std::vector<float> buffer_l;
        std::vector<float> buffer_r;
        std::vector<float> overview_peaks; // 800 pontos para mini-waveform
        
        double sample_rate = 44100.0;
        double host_sample_rate = 44100.0;
        double duration_sec = 0.0;
        double current_frame = 0.0;
        bool show_remaining_time = false; // Alternador ELAPSED / REMAIN (Padrão Pioneer CDJ-3000)

        double getElapsedTimeSec() const {
            return (sample_rate > 1000.0) ? (current_frame / sample_rate) : 0.0;
        }
        double getRemainingTimeSec() const {
            return std::max(0.0, duration_sec - getElapsedTimeSec());
        }
        void toggleTimeDisplayMode() {
            show_remaining_time = !show_remaining_time;
        }
        
        // Transporte e Velocidade
        std::atomic<bool> is_playing{ false };
        double playback_speed = 1.0;
        bool cue_active = false;
        double cue_frame = 0.0;
        
        // Pitch Fader 14-bit
        float pitch_percent = 0.0f; // Porcentagem de Pitch atual
        float pitch_range = 16.0f;   // Range selecionável: ±8%, ±16%, ±32%, ±50%
        int rate_msb = 64;
        int rate_lsb = 0;
        int vol_msb = 127;
        int vol_lsb = 0;

        float jog_pitch_bend = 0.0f; // Nudge temporário ao girar o anel do jog
        bool jog_touch = false; // Toque no prato (Scratch)
        bool vinyl_mode = true; // Modo Vinyl (parada instantânea de motor ao tocar no prato) vs Modo CDJ
        bool motor_stopped = false; // Estado de parada instantânea de motor
        float motor_speed = 1.0f; // Rampa de inércia do motor (0.0 a 1.0)
        float vinyl_stop_speed = 0.5f; // Inércia física de vinil ao pausar (0.0 = corte seco, 0.5 = vinil Pioneer CDJ, 1.0 = lento)
        bool is_stopping_vinyl = false; // Flag atômica de frenagem suave de vinil em andamento
        bool vinyl_stop_enabled = false; // Se true, dar Pause ativa parada lenta de vinil com queda de pitch
        int jog_feeling_mode = 0; // 0 = LIGHT (CDJ-3000 Free Flywheel), 1 = MID, 2 = HEAVY (Precision Nudge)
        float jog_momentum_velocity = 0.0f; // Inércia angular residual (simulação de volante de inércia para DDJ-200)
        int jog_display_mode = 0; // 0 = Holograma Alienígena, 1 = Capa do Álbum, 2 = Agulha Digital Vortex
        float scratch_velocity = 0.0f;
        float target_scratch_velocity = 0.0f;
        int scratch_active_ticks = 0;
        float jog_visual_angle = 0.0f;
        float touch_downlight_anim = 0.0f; // Animação de halo difuso sob as mãos do DJ (Platter Press Downlight)
        float needle_scrub_anim = 0.0f;    // Animação de pulso e glow da agulha laser interativa na waveform
        
        double bpm = 128.0;
        std::string key_signature = "8A / Am";
        double first_beat_frame = 0.0;
        std::vector<RekordboxBeat> rekordbox_beats;
        bool has_rekordbox_grid = false;
        bool sync_active = false;
        int sync_master_deck_id = -1;
        float sync_phase_diff = 0.0f; // Defasagem de fase em fração de beat (-0.5 a +0.5) para Phase Meter
        std::vector<std::chrono::steady_clock::time_point> tap_times;

        // Key Shift & Key Sync (Harmonic Transposition)
        int key_shift_semitones = 0; // -12 a +12 semitons
        bool key_sync_active = false;
        bool master_tempo = true; // Pioneer CDJ Master Tempo (Key Lock)
        std::vector<float> keyshift_ring_l;
        std::vector<float> keyshift_ring_r;
        int keyshift_ring_pos = 0;
        float keyshift_phase1 = 0.0f;
        float keyshift_phase2 = 0.5f;

        void shiftKey(int delta) {
            key_shift_semitones = std::clamp(key_shift_semitones + delta, -12, 12);
        }
        void resetKey() {
            key_shift_semitones = 0;
            key_sync_active = false;
        }

        void rebuildRekordboxBeats() {
            if (bpm < 20.0 || sample_rate < 1000.0 || buffer_l.empty()) return;
            rekordbox_beats.clear();
            double beat_len_frames = (60.0 / bpm) * sample_rate;
            int total_beats_est = (int)((buffer_l.size() - first_beat_frame) / beat_len_frames) + 1;
            if (total_beats_est > 0) {
                rekordbox_beats.reserve(total_beats_est);
                int beat_counter = 0;
                for (double f = first_beat_frame; f < (double)buffer_l.size(); f += beat_len_frames, beat_counter++) {
                    RekordboxBeat b;
                    b.bar_beat = (beat_counter % 4) + 1;
                    b.tempo_bpm = bpm;
                    b.frame = (int64_t)std::round(f);
                    b.time_sec = (double)b.frame / sample_rate;
                    b.time_ms = b.time_sec * 1000.0;
                    rekordbox_beats.push_back(b);
                }
            }
            has_rekordbox_grid = !rekordbox_beats.empty();
        }
        std::string getEffectiveKey() const {
            auto ck = DJHarmonicEngine::parseKey(key_signature);
            if (!ck.valid || key_shift_semitones == 0) return key_signature;
            auto shifted = DJHarmonicEngine::transposeKey(ck, key_shift_semitones);
            return shifted.getCamelotCode() + " / " + shifted.musical_key;
        }

        // Slip Mode (Padrão Pioneer CDJ-3000 / Traktor Flux)
        bool slip_mode = false;
        bool is_slipping = false;
        double slip_virtual_frame = 0.0;

        // Beat Loop Roll (Pads de Tensão)
        bool pad_roll_active = false;
        double pad_roll_start_frame = 0.0;
        double pad_roll_len_frames = 0.0;
        int active_roll_pad = -1;

        void triggerPadRoll(int pad_idx) {
            if (pad_idx < 0 || pad_idx >= 8) return;
            active_roll_pad = pad_idx;
            const double roll_fracs[8] = { 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/2.0, 1.0, 2.0, 4.0, 8.0 };
            double beat_sec = 60.0 / std::max(20.0, bpm);
            pad_roll_len_frames = roll_fracs[pad_idx] * beat_sec * sample_rate;
            pad_roll_start_frame = current_frame;
            if (!slip_mode) {
                is_slipping = true;
                slip_virtual_frame = current_frame;
            }
            pad_roll_active = true;
        }

        void releasePadRoll() {
            if (pad_roll_active) {
                pad_roll_active = false;
                active_roll_pad = -1;
                if (!slip_mode && is_slipping) {
                    current_frame = slip_virtual_frame;
                    is_slipping = false;
                }
            }
        }

        void triggerSlipRoll(double fraction_beats = 0.5) {
            if (pad_roll_active) {
                releasePadRoll();
            } else {
                double beat_sec = 60.0 / std::max(20.0, bpm);
                pad_roll_len_frames = fraction_beats * beat_sec * sample_rate;
                pad_roll_start_frame = current_frame;
                if (!slip_mode) {
                    is_slipping = true;
                    slip_virtual_frame = current_frame;
                }
                pad_roll_active = true;
            }
        }

        // Quantize Engine (Pioneer CDJ-3000 / Rekordbox Style)
        bool quantize_active = true;
        float quantize_value = 1.0f; // 1.0 = 1 beat, 0.5 = 1/2 beat, 0.25 = 1/4 beat

        double getQuantizedFrame(double frame) const {
            if (!quantize_active || bpm <= 10.0 || sample_rate < 1000.0) return frame;

            // Se o grid nativo do Rekordbox estiver ativo e o quantize for de 1 beat, quantiza diretamente no beat exato
            if (has_rekordbox_grid && !rekordbox_beats.empty() && std::abs(quantize_value - 1.0f) < 0.01f) {
                int64_t f_target = (int64_t)frame;
                auto it = std::lower_bound(rekordbox_beats.begin(), rekordbox_beats.end(), f_target,
                    [](const RekordboxBeat& b, int64_t val) { return b.frame < val; });
                if (it == rekordbox_beats.end()) {
                    return (double)rekordbox_beats.back().frame;
                }
                if (it == rekordbox_beats.begin()) {
                    return (double)it->frame;
                }
                auto prev = it - 1;
                if (std::abs((double)it->frame - frame) < std::abs((double)prev->frame - frame)) {
                    return (double)it->frame;
                } else {
                    return (double)prev->frame;
                }
            }

            double beat_len = (60.0 / std::max(20.0, bpm)) * sample_rate;
            double quant_step = beat_len * (double)quantize_value;
            if (quant_step < 1.0) return frame;
            double rel = frame - first_beat_frame;
            double num_steps = std::round(rel / quant_step);
            double q_frame = first_beat_frame + num_steps * quant_step;
            if (q_frame < 0.0) q_frame = 0.0;
            if (!buffer_l.empty() && q_frame > (double)buffer_l.size()) q_frame = (double)buffer_l.size();
            return q_frame;
        }

        // Pioneer Censor / Slip Reverse Mode
        bool censor_active = false;

        void triggerCensor() {
            if (censor_active) return;
            censor_active = true;
            if (!slip_mode && !is_slipping) {
                is_slipping = true;
                slip_virtual_frame = current_frame;
            }
        }

        void releaseCensor() {
            if (!censor_active) return;
            censor_active = false;
            if (is_slipping) {
                current_frame = slip_virtual_frame;
                if (!slip_mode && !pad_roll_active) {
                    is_slipping = false;
                }
            }
        }

        // Auto-Gain / ReplayGain
        bool auto_gain_enabled = true;
        float auto_gain_factor = 1.0f;
        float auto_gain_db = 0.0f;
        float track_lufs = -9.0f;

        // --- LIVE STEMS DSP ENGINE (Latência Zero & Padrão DDJ-FLX10 / Serato Stems) ---
        std::atomic<float> stem_vocal_gain{ 1.0f };
        std::atomic<float> stem_melody_gain{ 1.0f };
        std::atomic<float> stem_bass_gain{ 1.0f };
        std::atomic<float> stem_drum_gain{ 1.0f };
        std::atomic<bool> stem_vocal_active{ true };
        std::atomic<bool> stem_melody_active{ true };
        std::atomic<bool> stem_bass_active{ true };
        std::atomic<bool> stem_drum_active{ true };

        // Estados dos filtros de crossover para latência zero
        float stem_lp_l = 0.0f, stem_lp_r = 0.0f;
        float stem_hp_l = 0.0f, stem_hp_r = 0.0f;

        void toggleStemVocal() {
            stem_vocal_active = !stem_vocal_active.load();
            stem_vocal_gain.store(stem_vocal_active.load() ? 1.0f : 0.0f);
        }
        void toggleStemMelody() {
            stem_melody_active = !stem_melody_active.load();
            stem_melody_gain.store(stem_melody_active.load() ? 1.0f : 0.0f);
        }
        void toggleStemBass() {
            stem_bass_active = !stem_bass_active.load();
            stem_bass_gain.store(stem_bass_active.load() ? 1.0f : 0.0f);
            stem_mute_bass = !stem_bass_active.load();
        }
        void toggleStemDrums() {
            stem_drum_active = !stem_drum_active.load();
            stem_drum_gain.store(stem_drum_active.load() ? 1.0f : 0.0f);
        }
        void soloStemVocal() {
            stem_vocal_active = true;
            stem_vocal_gain = 1.0f;
            stem_melody_active = false;
            stem_melody_gain = 0.0f;
            stem_bass_active = false;
            stem_bass_gain = 0.0f;
            stem_drum_active = false;
            stem_drum_gain = 0.0f;
        }
        void soloStemMelody() {
            stem_vocal_active = false;
            stem_vocal_gain = 0.0f;
            stem_melody_active = true;
            stem_melody_gain = 1.0f;
            stem_bass_active = false;
            stem_bass_gain = 0.0f;
            stem_drum_active = false;
            stem_drum_gain = 0.0f;
        }
        void soloStemBass() {
            stem_vocal_active = false;
            stem_vocal_gain = 0.0f;
            stem_melody_active = false;
            stem_melody_gain = 0.0f;
            stem_bass_active = true;
            stem_bass_gain = 1.0f;
            stem_drum_active = false;
            stem_drum_gain = 0.0f;
        }
        void soloStemDrums() {
            stem_vocal_active = false;
            stem_vocal_gain = 0.0f;
            stem_melody_active = false;
            stem_melody_gain = 0.0f;
            stem_bass_active = false;
            stem_bass_gain = 0.0f;
            stem_drum_active = true;
            stem_drum_gain = 1.0f;
        }
        void resetStems() {
            stem_vocal_active = true;
            stem_vocal_gain = 1.0f;
            stem_melody_active = true;
            stem_melody_gain = 1.0f;
            stem_bass_active = true;
            stem_bass_gain = 1.0f;
            stem_drum_active = true;
            stem_drum_gain = 1.0f;
            stem_mute_drums = false;
            stem_mute_bass = false;
            stem_mute_inst = false;
            stem_mute_vocals = false;
        }
        void triggerVocalEchoOut() {
            stem_vocal_active = false;
            stem_vocal_gain = 0.0f;
        }

        void registerTap() {
            auto now = std::chrono::steady_clock::now();
            if (!tap_times.empty()) {
                auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - tap_times.back()).count();
                if (diff_ms > 2000) {
                    tap_times.clear();
                }
            }
            tap_times.push_back(now);
            if (tap_times.size() > 8) tap_times.erase(tap_times.begin());
            if (tap_times.size() >= 3) {
                double total_ms = 0.0;
                for (size_t i = 1; i < tap_times.size(); i++) {
                    total_ms += std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tap_times[i] - tap_times[i - 1]).count();
                }
                double avg_interval_sec = (total_ms / (tap_times.size() - 1)) / 1000.0;
                if (avg_interval_sec > 0.25 && avg_interval_sec < 1.5) {
                    bpm = std::round((60.0 / avg_interval_sec) * 10.0) / 10.0;
                }
            }
        }
        
        // 16 Hot Cues (A..P Pioneer Flagship)
        DJHotCue hot_cues[16];
        int pad_mode = 0; // 0 = Hot Cue, 1 = Sampler, 2 = Beat Loop, 3 = Pad FX
        
        // Loop
        bool loop_active = false;
        double loop_start_frame = 0.0;
        double loop_length_frames = 0.0;
        double loop_beats = 4.0;
        
        // Mixer por Deck
        float gain = 1.0f;
        float volume = 1.0f;

        // Compressor de Canal Pioneer DJM-V10 / DJM-A9
        float comp = 0.0f; // 0.0 (Bypass) a 1.0 (Máximo Encorpamento/Punch)
        PioneerChannelCompressor compressor;

        // Equalizador Isolator Pioneer 3-Bandas
        float eq_high = 1.0f; // 0.0 (Kill) a 1.0 (Flat) a 2.0 (+6dB)
        float eq_mid = 1.0f;
        float eq_low = 1.0f;
        // Stem Separation em Tempo Real nos 4 Canais (Isolamento de Drums, Bass, Instruments e Vocals)
        bool stem_mode = false;         // Se ligado, Low = Bateria/Grave, Mid = Instrumentos, High = Vocais
        float stem_drums = 1.0f;
        float stem_bass = 1.0f;
        float stem_instruments = 1.0f;
        float stem_vocals = 1.0f;
        bool stem_mute_drums = false;
        bool stem_mute_bass = false;
        bool stem_mute_inst = false;
        bool stem_mute_vocals = false;
        DJBiquadFilter eq_lp;           // Crossover Low EQ (320 Hz)
        DJBiquadFilter eq_hp;           // Crossover High EQ (2800 Hz)
        DJBiquadFilter vocal_lp;        // Crossover Low vocal (5500 Hz)
        DJBiquadFilter vocal_hp;        // Crossover High vocal (320 Hz)

        // Suporte a Stems de Alta Fidelidade (Pré-separados por IA / ONNX / Matriz Espectral HD)
        bool has_ai_stems = false;
        std::vector<float> stem_drums_l, stem_drums_r;
        std::vector<float> stem_bass_l, stem_bass_r;
        std::vector<float> stem_vocals_l, stem_vocals_r;
        std::vector<float> stem_inst_l, stem_inst_r;

        bool loadAiStems(const std::string& base_path) {
            try {
                std::filesystem::path p = std::filesystem::u8path(base_path);
                std::string stem_name = p.stem().string();
                std::filesystem::path parent = p.parent_path();
                std::vector<std::filesystem::path> search_dirs = {
                    parent / "stems",
                    parent,
                    "scratch/stems",
                    "scratch/tracks/stems"
                };

                std::filesystem::path p_drums, p_bass, p_vocals, p_inst;
                for (const auto& d : search_dirs) {
                    if (!std::filesystem::exists(d)) continue;
                    if (p_drums.empty() && std::filesystem::exists(d / (stem_name + "_drums.wav"))) p_drums = d / (stem_name + "_drums.wav");
                    if (p_bass.empty() && std::filesystem::exists(d / (stem_name + "_bass.wav"))) p_bass = d / (stem_name + "_bass.wav");
                    if (p_vocals.empty() && std::filesystem::exists(d / (stem_name + "_vocals.wav"))) p_vocals = d / (stem_name + "_vocals.wav");
                    if (p_inst.empty() && std::filesystem::exists(d / (stem_name + "_other.wav"))) p_inst = d / (stem_name + "_other.wav");
                    if (p_inst.empty() && std::filesystem::exists(d / (stem_name + "_inst.wav"))) p_inst = d / (stem_name + "_inst.wav");
                    if (p_inst.empty() && std::filesystem::exists(d / (stem_name + "_instruments.wav"))) p_inst = d / (stem_name + "_instruments.wav");
                }

                if (p_drums.empty() || p_vocals.empty()) {
                    has_ai_stems = false;
                    return false;
                }

                auto loadWavIntoVec = [](const std::filesystem::path& wp, std::vector<float>& out_l, std::vector<float>& out_r) -> bool {
                    unsigned int ch = 0, sr = 0;
                    drwav_uint64 total_pcm = 0;
                    float* data = drwav_open_file_and_read_pcm_frames_f32(wp.string().c_str(), &ch, &sr, &total_pcm, NULL);
                    if (!data || total_pcm == 0) return false;
                    out_l.resize(total_pcm);
                    out_r.resize(total_pcm);
                    if (ch == 1) {
                        for (size_t i = 0; i < total_pcm; i++) {
                            out_l[i] = data[i];
                            out_r[i] = data[i];
                        }
                    } else {
                        for (size_t i = 0; i < total_pcm; i++) {
                            out_l[i] = data[i * ch];
                            out_r[i] = data[i * ch + 1];
                        }
                    }
                    drwav_free(data, NULL);
                    return true;
                };

                bool ok1 = loadWavIntoVec(p_drums, stem_drums_l, stem_drums_r);
                bool ok2 = !p_bass.empty() ? loadWavIntoVec(p_bass, stem_bass_l, stem_bass_r) : true;
                bool ok3 = loadWavIntoVec(p_vocals, stem_vocals_l, stem_vocals_r);
                bool ok4 = !p_inst.empty() ? loadWavIntoVec(p_inst, stem_inst_l, stem_inst_r) : true;

                has_ai_stems = (ok1 && ok3);
                if (has_ai_stems) {
                    KuroUtils::Log("[DJ Engine Deck " + std::to_string(deck_id + 1) + "] AI 4-Stems HD carregados com sucesso: " + stem_name);
                }
                return has_ai_stems;
            } catch (...) {
                has_ai_stems = false;
                return false;
            }
        }

        // Métodos de Controle Rápido de Stems (Mute / Solo)
        void toggleStemMute(int stem_idx) {
            stem_mode = true;
            if (stem_idx == 0) { stem_mute_drums = !stem_mute_drums; stem_drum_gain.store(stem_mute_drums ? 0.0f : 1.0f); }
            else if (stem_idx == 1) { stem_mute_bass = !stem_mute_bass; stem_bass_gain.store(stem_mute_bass ? 0.0f : 1.0f); }
            else if (stem_idx == 2) { stem_mute_inst = !stem_mute_inst; stem_melody_gain.store(stem_mute_inst ? 0.0f : 1.0f); }
            else if (stem_idx == 3) { stem_mute_vocals = !stem_mute_vocals; stem_vocal_gain.store(stem_mute_vocals ? 0.0f : 1.0f); }
        }

        void setStemSolo(int stem_idx) {
            stem_mode = true;
            bool is_already_solo = false;
            if (stem_idx == 0 && !stem_mute_drums && stem_mute_bass && stem_mute_inst && stem_mute_vocals) is_already_solo = true;
            else if (stem_idx == 1 && stem_mute_drums && !stem_mute_bass && stem_mute_inst && stem_mute_vocals) is_already_solo = true;
            else if (stem_idx == 2 && stem_mute_drums && stem_mute_bass && !stem_mute_inst && stem_mute_vocals) is_already_solo = true;
            else if (stem_idx == 3 && stem_mute_drums && stem_mute_bass && stem_mute_inst && !stem_mute_vocals) is_already_solo = true;

            if (is_already_solo) {
                resetStems();
            } else {
                stem_mute_drums = (stem_idx != 0); stem_drum_gain.store(stem_mute_drums ? 0.0f : 1.0f);
                stem_mute_bass = (stem_idx != 1); stem_bass_gain.store(stem_mute_bass ? 0.0f : 1.0f);
                stem_mute_inst = (stem_idx != 2); stem_melody_gain.store(stem_mute_inst ? 0.0f : 1.0f);
                stem_mute_vocals = (stem_idx != 3); stem_vocal_gain.store(stem_mute_vocals ? 0.0f : 1.0f);
            }
        }

        void invertStems() {
            stem_mode = true;
            stem_mute_drums = !stem_mute_drums; stem_drum_gain.store(stem_mute_drums ? 0.0f : 1.0f);
            stem_mute_bass = !stem_mute_bass; stem_bass_gain.store(stem_mute_bass ? 0.0f : 1.0f);
            stem_mute_inst = !stem_mute_inst; stem_melody_gain.store(stem_mute_inst ? 0.0f : 1.0f);
            stem_mute_vocals = !stem_mute_vocals; stem_vocal_gain.store(stem_mute_vocals ? 0.0f : 1.0f);
        }

        // Calibração Fina do Beat Grid Absoluto (Padrão Pioneer Rekordbox)
        void nudgeBeatGrid(double ms) {
            double delta_frames = (ms / 1000.0) * sample_rate;
            first_beat_frame += delta_frames;
            for (auto& b : rekordbox_beats) {
                b.time_ms += ms;
                b.time_sec += (ms * 0.001);
                b.frame += (int64_t)delta_frames;
            }
        }

        void setFirstBeatHere() {
            double delta_frames = current_frame - first_beat_frame;
            first_beat_frame = current_frame;
            cue_frame = current_frame;
            hot_cues[0].active = true;
            hot_cues[0].time_sec = current_frame / sample_rate;
            hot_cues[0].label = "BEAT 1";
            for (auto& b : rekordbox_beats) {
                b.frame += (int64_t)delta_frames;
                b.time_sec = (double)b.frame / sample_rate;
                b.time_ms = b.time_sec * 1000.0;
            }
        }

        // =========================================================================
        // SISTEMA DUPLO DE CUES: MEMORY CUES VS HOT CUES (Pioneer Rekordbox / CDJ-3000)
        // =========================================================================
        struct DJMemoryCue {
            double time_sec = 0.0;
            std::string label = "Cue";
            uint32_t color = 0xFFFF3C50;
        };
        std::vector<DJMemoryCue> memory_cues;
        int selected_memory_cue = -1;

        void addMemoryCueHere(const std::string& name = "") {
            double t = getElapsedTimeSec();
            DJMemoryCue mc;
            mc.time_sec = t;
            mc.label = name.empty() ? ("Cue " + std::to_string(memory_cues.size() + 1)) : name;
            mc.color = (memory_cues.empty()) ? 0xFFFF3C50 : 0xFFFFB400;
            memory_cues.push_back(mc);
            std::sort(memory_cues.begin(), memory_cues.end(), [](const DJMemoryCue& a, const DJMemoryCue& b) {
                return a.time_sec < b.time_sec;
            });
        }

        void callPrevMemoryCue() {
            if (memory_cues.empty()) return;
            double cur_t = getElapsedTimeSec();
            int target = -1;
            for (int i = (int)memory_cues.size() - 1; i >= 0; i--) {
                if (memory_cues[i].time_sec < cur_t - 0.35) {
                    target = i;
                    break;
                }
            }
            if (target >= 0) {
                selected_memory_cue = target;
                current_frame = memory_cues[target].time_sec * sample_rate;
                cue_frame = current_frame;
            }
        }

        void callNextMemoryCue() {
            if (memory_cues.empty()) return;
            double cur_t = getElapsedTimeSec();
            int target = -1;
            for (size_t i = 0; i < memory_cues.size(); i++) {
                if (memory_cues[i].time_sec > cur_t + 0.35) {
                    target = (int)i;
                    break;
                }
            }
            if (target >= 0) {
                selected_memory_cue = target;
                current_frame = memory_cues[target].time_sec * sample_rate;
                cue_frame = current_frame;
            }
        }

        void deleteSelectedMemoryCue() {
            if (selected_memory_cue >= 0 && selected_memory_cue < (int)memory_cues.size()) {
                memory_cues.erase(memory_cues.begin() + selected_memory_cue);
                selected_memory_cue = -1;
            }
        }

        // =========================================================================
        // CONTAGEM REGRESSIVA PARA O DROP / BREAKDOWN (Pioneer CDJ-3000 Style)
        // =========================================================================
        double drop_point_frame = -1.0;
        void setDropPointHere() {
            drop_point_frame = current_frame;
        }
        void clearDropPoint() {
            drop_point_frame = -1.0;
        }
        double getBarsToDrop() const {
            if (drop_point_frame <= 0.0 || bpm < 20.0 || sample_rate < 1000.0) return 999.0;
            double eff_bpm = bpm * (1.0 + pitch_percent * 0.01);
            double beat_frames = (60.0 / std::max(20.0, eff_bpm)) * sample_rate;
            double frames_left = drop_point_frame - current_frame;
            return frames_left / (beat_frames * 4.0);
        }

        // Retorna a contagem de compassos (bars) e beats até a próxima Memory Cue à frente
        double getBarsToNextMemoryCue(int* out_cue_idx = nullptr) const {
            if (memory_cues.empty() || bpm < 20.0 || sample_rate < 1000.0) return 999.0;
            double eff_bpm = bpm * (1.0 + pitch_percent * 0.01);
            double beat_frames = (60.0 / std::max(20.0, eff_bpm)) * sample_rate;
            double bar_frames = beat_frames * 4.0;
            
            double min_frames_ahead = 1e12;
            int best_idx = -1;
            for (size_t i = 0; i < memory_cues.size(); i++) {
                double cue_frame_pos = memory_cues[i].time_sec * sample_rate;
                double diff = cue_frame_pos - current_frame;
                if (diff > (sample_rate * 0.1) && diff < min_frames_ahead) {
                    min_frames_ahead = diff;
                    best_idx = (int)i;
                }
            }
            if (best_idx >= 0) {
                if (out_cue_idx) *out_cue_idx = best_idx;
                return min_frames_ahead / bar_frames;
            }
            return 999.0;
        }

        // =========================================================================
        // ACTIVE LOOP (Pioneer CDJ-3000 / Rekordbox Standard)
        // =========================================================================
        bool has_active_loop = false;
        double active_loop_start_frame = -1.0;
        double active_loop_len_frames = 0.0;

        void toggleActiveLoopHere(double beats = 4.0) {
            if (has_active_loop) {
                has_active_loop = false;
                active_loop_start_frame = -1.0;
                active_loop_len_frames = 0.0;
            } else {
                double eff_bpm = bpm * (1.0 + pitch_percent * 0.01);
                double beat_frames = (60.0 / std::max(20.0, eff_bpm)) * sample_rate;
                active_loop_start_frame = current_frame;
                active_loop_len_frames = beats * beat_frames;
                has_active_loop = true;
                addMemoryCueHere("Active Loop");
            }
        }

        void multiplyBpm(double factor) {
            if (factor <= 0.0) return;
            bpm = std::clamp(bpm * factor, 30.0, 300.0);
        }

        // Informações métricas de Beat/Bar para Phase Meter Pioneer CDJ-3000
        void getBeatPhaseInfo(int& out_bar, int& out_beat, float& out_beat_progress, int* out_total_bar = nullptr) const {
            if (bpm < 20.0 || sample_rate < 1000.0) {
                out_bar = 1; out_beat = 1; out_beat_progress = 0.0f;
                if (out_total_bar) *out_total_bar = 1;
                return;
            }

            // Se o grid do Rekordbox estiver ativo, calcula diretamente a partir da tabela de batidas reais
            if (has_rekordbox_grid && !rekordbox_beats.empty()) {
                int64_t cf = (int64_t)current_frame;
                if (cf < rekordbox_beats.front().frame) {
                    double d_frames = (double)(rekordbox_beats.front().frame - cf);
                    double b_len = (sample_rate * 60.0) / std::max(20.0, bpm);
                    int b_before = (int)std::ceil(d_frames / b_len);
                    out_bar = 0;
                    out_beat = ((4 - (b_before % 4)) % 4) + 1;
                    out_beat_progress = 0.0f;
                    if (out_total_bar) *out_total_bar = 0;
                    return;
                }

                auto it = std::upper_bound(rekordbox_beats.begin(), rekordbox_beats.end(), cf,
                    [](int64_t val, const RekordboxBeat& b) { return val < b.frame; });
                size_t idx = (it == rekordbox_beats.begin()) ? 0 : std::distance(rekordbox_beats.begin(), it) - 1;
                if (idx < rekordbox_beats.size()) {
                    const auto& b = rekordbox_beats[idx];
                    out_beat = b.bar_beat; // 1, 2, 3, 4 exato do Rekordbox
                    out_bar = (int)((idx / 4) % 4) + 1;
                    if (out_total_bar) *out_total_bar = (int)(idx / 4) + 1;
                    if (idx + 1 < rekordbox_beats.size() && rekordbox_beats[idx + 1].frame > b.frame) {
                        out_beat_progress = (float)(current_frame - b.frame) / (float)(rekordbox_beats[idx + 1].frame - b.frame);
                    } else {
                        out_beat_progress = 0.0f;
                    }
                    return;
                }
            }

            double eff_bpm = bpm * (1.0 + pitch_percent * 0.01);
            double beat_sec = 60.0 / std::max(20.0, eff_bpm);
            double beat_frames = beat_sec * sample_rate;
            if (beat_frames < 1.0) {
                out_bar = 1; out_beat = 1; out_beat_progress = 0.0f;
                if (out_total_bar) *out_total_bar = 1;
                return;
            }
            double rel_frames = current_frame - first_beat_frame;
            if (rel_frames < 0.0) rel_frames = 0.0;
            double total_beats = rel_frames / beat_frames;
            long long int_beats = (long long)total_beats;
            out_beat_progress = (float)(total_beats - (double)int_beats);
            out_beat = (int)(int_beats % 4) + 1; // 1, 2, 3, 4
            out_bar = (int)((int_beats / 4) % 4) + 1; // 1, 2, 3, 4 no bloco de 4 compassos (frase)
            if (out_total_bar) *out_total_bar = (int)(int_beats / 4) + 1;
        }

        // Sound Color FX (Filter, Noise, Crush, Space, Dub Echo, Pitch)
        int sound_color_fx_mode = 0; // 0 = FILTER, 1 = NOISE, 2 = CRUSH, 3 = SPACE, 4 = DUB ECHO, 5 = PITCH
        float filter_bipolar = 0.5f; // 0.0 = Low, 0.5 = Bypass, 1.0 = High
        float sound_color_param = 0.5f; // Pioneer CFX PARAM (0.0 a 1.0 - ressonância / profundidade)
        DJBiquadFilter sound_color_filter;
        DJBiquadFilter noise_sweep_filter;
        float crush_counter = 0.0f;
        float crush_held_l = 0.0f, crush_held_r = 0.0f;

        // Dub Echo
        std::vector<float> dub_echo_buf_l;
        std::vector<float> dub_echo_buf_r;
        int dub_echo_pos = 0;
        DJBiquadFilter dub_echo_lp;

        // Pitch Shift Ring
        std::vector<float> pitch_ring_l;
        std::vector<float> pitch_ring_r;
        int pitch_ring_pos = 0;
        float pitch_phase1 = 0.0f;
        float pitch_phase2 = 0.5f;
        // 3-Band Spectral Overview Buffers (Estilo Rekordbox 3-Band)
        std::vector<float> overview_low;   // Graves / Kick / Sub (Branco/Ambar claro)
        std::vector<float> overview_mid;   // Medios / Vocais / Synths (Ambar/Laranja)
        std::vector<float> overview_high;  // Agudos / Hi-Hats / Cymbals (Azul/Ciano)

        // Waveform de Alta Resolução Pré-Computada (Flicker-Free / 120 FPS Buttery Smooth)
        struct DJDetailedWavePoint {
            float peak    = 0.0f;
            float low     = 0.0f;
            float mid     = 0.0f;
            float high    = 0.0f;
            float carrier = 0.0f; // Sinal senoidal assinado de sub-grave/kick filtrado Linkwitz-Riley LR4 24dB/oct
        };
        static constexpr int DETAILED_WAVE_STEP = 64; // ~1.45ms @ 44.1kHz (Alta definição Mixxx)
        std::vector<DJDetailedWavePoint> detailed_waveform;
        std::vector<DJDetailedWavePoint> raw_detailed_waveform; // Cache de bandas espectrais brutas antes do alongamento/decaimento
        float waveform_stretch = 1.00f; // Alongamento horizontal balístico dos transientes (sem acelerar scroll)
        float waveform_envelope_decay = 0.20f; // Retenção / Decaimento do envelope de transientes [0.0 = linhas puras]

        void generateDetailedWaveformIfNeeded() {
            if (!detailed_waveform.empty() || buffer_l.empty()) return;
            std::unique_lock<std::mutex> lock(deck_mutex, std::try_to_lock);
            if (!lock.owns_lock() || !detailed_waveform.empty() || buffer_l.empty()) return;

            size_t total_f = buffer_l.size();
            size_t num_bins = (total_f + DETAILED_WAVE_STEP - 1) / DETAILED_WAVE_STEP;
            if (num_bins == 0) return;

            // Se as bandas espectrais brutas ainda não foram calculadas para este arquivo de áudio:
            if (raw_detailed_waveform.size() != num_bins) {
                raw_detailed_waveform.resize(num_bins);

                DJBiquadFilter f_low1, f_low2, f_high1, f_high2;
                DJBiquadFilter f_mid_hp1, f_mid_hp2, f_mid_lp1, f_mid_lp2;
                float sr = (sample_rate > 1000.0) ? (float)sample_rate : 44100.0f;
                // Padrão Pioneer Rekordbox / CDJ-3000: Linkwitz-Riley 4th order (24 dB/oct)
                // Dois filtros Butterworth 2ª ordem cascateados em 250 Hz (Low) e 2500 Hz (High)
                f_low1.setLowPass(250.0f, sr, 0.7071f);
                f_low2.setLowPass(250.0f, sr, 0.7071f);
                f_high1.setHighPass(2500.0f, sr, 0.7071f);
                f_high2.setHighPass(2500.0f, sr, 0.7071f);
                // Mid Bandpass dedicado 24 dB/oct (HP 250Hz + LP 2500Hz) sem vazamento de graves
                f_mid_hp1.setHighPass(250.0f, sr, 0.7071f);
                f_mid_hp2.setHighPass(250.0f, sr, 0.7071f);
                f_mid_lp1.setLowPass(2500.0f, sr, 0.7071f);
                f_mid_lp2.setLowPass(2500.0f, sr, 0.7071f);

                for (size_t b = 0; b < num_bins; b++) {
                    size_t start = b * DETAILED_WAVE_STEP;
                    size_t end = std::min(start + (size_t)DETAILED_WAVE_STEP, total_f);
                    float max_p = 0.0f;
                    float max_l = 0.0f;
                    float max_m = 0.0f;
                    float max_h = 0.0f;
                    float last_yl = 0.0f;

                    for (size_t i = start; i < end; i++) {
                        float s_l = buffer_l[i];
                        float s_r = (i < buffer_r.size()) ? buffer_r[i] : s_l;
                        float mono = (std::abs(s_l) + std::abs(s_r)) * 0.5f;
                        if (mono > max_p) max_p = mono;

                        // Low pass (LR4 24 dB/oct - Subgrave e Corpo do Kick)
                        float yl_l = s_l, yl_r = s_r;
                        f_low1.process(yl_l, yl_r);
                        f_low2.process(yl_l, yl_r);
                        last_yl = (yl_l + yl_r) * 0.5f;
                        float v_l = std::abs(last_yl);
                        if (v_l > max_l) max_l = v_l;

                        // High pass (LR4 24 dB/oct)
                        float yh_l = s_l, yh_r = s_r;
                        f_high1.process(yh_l, yh_r);
                        f_high2.process(yh_l, yh_r);
                        float v_h = (std::abs(yh_l) + std::abs(yh_r)) * 0.5f;
                        if (v_h > max_h) max_h = v_h;

                        // Mid: Bandpass isolado 24 dB/oct (250 Hz - 2500 Hz) sem vazamento
                        float ym_l = s_l, ym_r = s_r;
                        f_mid_hp1.process(ym_l, ym_r);
                        f_mid_hp2.process(ym_l, ym_r);
                        f_mid_lp1.process(ym_l, ym_r);
                        f_mid_lp2.process(ym_l, ym_r);
                        float v_m = (std::abs(ym_l) + std::abs(ym_r)) * 0.5f;
                        if (v_m > max_m) max_m = v_m;
                    }

                    // Amplitudes físicas autênticas Rekordbox (calibradas para expansão perceptual)
                    raw_detailed_waveform[b].peak    = std::clamp(max_p, 0.0f, 1.0f);
                    raw_detailed_waveform[b].low     = std::clamp(max_l * 1.35f, 0.0f, 1.0f);
                    raw_detailed_waveform[b].mid     = std::clamp(max_m * 1.45f, 0.0f, 1.0f);
                    raw_detailed_waveform[b].high    = std::clamp(max_h * 2.10f, 0.0f, 1.0f);
                    raw_detailed_waveform[b].carrier = std::clamp(last_yl * 1.35f, -1.0f, 1.0f);
                }
            }

            // Passe instantâneo sobre as bandas pré-calculadas
            std::vector<DJDetailedWavePoint> temp_wave = raw_detailed_waveform;

            // =========================================================================
            // PASSE DE BALÍSTICA & ALONGAMENTO HORIZONTAL OFICIAL REKORDBOX (PIONEER PWV)
            // Transientes musicais (kicks, snares, hi-hats) são alargados na horizontal
            // com ataque instantâneo e cauda de decaimento orgânico (envelope release).
            // O andamento (BPM) e a velocidade de scroll permanecem 100% INALTERADOS!
            // =========================================================================
            float h_str = std::clamp(waveform_stretch, 0.8f, 3.5f);
            float decay_mult = std::clamp(waveform_envelope_decay, 0.0f, 2.0f);

            // Se o usuário quer linhas puras de transiente (decay_mult baixo) ou h_str <= 1.02:
            // os picos são mantidos 100% autênticos e cirúrgicos sem preencher vales artificialmente!
            if (decay_mult > 0.02f && h_str > 1.02f) {
                float decay_power = (h_str - 1.0f) * decay_mult;
                float decay_l = std::pow(0.965f, 1.0f / (1.0f + decay_power));
                float decay_m = std::pow(0.950f, 1.0f / (1.0f + decay_power));
                float decay_h = std::pow(0.920f, 1.0f / (1.0f + decay_power));
                float decay_p = std::pow(0.950f, 1.0f / (1.0f + decay_power));

                float env_p = 0.0f, env_l = 0.0f, env_m = 0.0f, env_h = 0.0f;
                for (size_t b = 0; b < num_bins; b++) {
                    env_l = std::max(temp_wave[b].low,  env_l * decay_l);
                    env_m = std::max(temp_wave[b].mid,  env_m * decay_m);
                    env_h = std::max(temp_wave[b].high, env_h * decay_h);
                    env_p = std::max(temp_wave[b].peak, env_p * decay_p);

                    temp_wave[b].low  = env_l;
                    temp_wave[b].mid  = env_m;
                    temp_wave[b].high = env_h;
                    temp_wave[b].peak = env_p;
                }
            }

            // Suavização Gaussiana de 3 pontos apenas se alongamento expressivo solicitado
            if (num_bins > 2 && h_str > 1.30f && decay_mult > 0.35f) {
                std::vector<DJDetailedWavePoint> smoothed = temp_wave;
                for (size_t b = 1; b + 1 < num_bins; b++) {
                    smoothed[b].low  = temp_wave[b-1].low  * 0.22f + temp_wave[b].low  * 0.56f + temp_wave[b+1].low  * 0.22f;
                    smoothed[b].mid  = temp_wave[b-1].mid  * 0.20f + temp_wave[b].mid  * 0.60f + temp_wave[b+1].mid  * 0.20f;
                    smoothed[b].high = temp_wave[b-1].high * 0.18f + temp_wave[b].high * 0.64f + temp_wave[b+1].high * 0.18f;
                    smoothed[b].peak = temp_wave[b-1].peak * 0.20f + temp_wave[b].peak * 0.60f + temp_wave[b+1].peak * 0.20f;
                }
                temp_wave = std::move(smoothed);
            }
            detailed_waveform = std::move(temp_wave);
        }

        void recomputeDetailedWaveform() {
            detailed_waveform.clear();
            generateDetailedWaveformIfNeeded();
        }

        // DJM-V10 Extended FX Buffers
        std::vector<float> ping_pong_buf_l;
        std::vector<float> ping_pong_buf_r;
        int ping_pong_pos = 0;

        std::vector<float> spiral_buf_l;
        std::vector<float> spiral_buf_r;
        int spiral_pos = 0;

        std::vector<float> roll_buf_l;
        std::vector<float> roll_buf_r;
        int roll_buf_pos = 0;
        int roll_len_frames = 0;
        bool roll_capturing = false;

        float vinyl_brake_factor = 1.0f;
        
        float vu_meter = 0.0f;
        std::string grid_status_msg;
        std::mutex deck_mutex;

        DJDeck(int id = 0) : deck_id(id) {
            uint32_t colors[16] = { 
                0xFF00E5FF, 0xFFFF0055, 0xFF00FF66, 0xFFFFCC00, 
                0xFFBF00FF, 0xFFFF6600, 0xFF00FFFF, 0xFFFFFFFF,
                0xFFFF3399, 0xFF33CCFF, 0xFFFF9933, 0xFF66FF33,
                0xFFCC33FF, 0xFFFFFF33, 0xFF33FFFF, 0xFFFF6666
            };
            for (int i = 0; i < 16; i++) {
                hot_cues[i].id = i;
                hot_cues[i].active = false;
                hot_cues[i].time_sec = 0.0;
                hot_cues[i].color_rgba = colors[i % 16];
            }
            eq_lp.setLowPass(320.0f, 44100.0f, 0.707f);
            eq_hp.setHighPass(2800.0f, 44100.0f, 0.707f);
            vocal_lp.setLowPass(5500.0f, 44100.0f, 0.707f);
            vocal_hp.setHighPass(320.0f, 44100.0f, 0.707f);
            sound_color_filter.setLowPass(22000.0f, 44100.0f);
            noise_sweep_filter.setBandPass(2000.0f, 44100.0f);

            comp = 0.0f;
            compressor.reset();

            dub_echo_buf_l.assign(96000, 0.0f);
            dub_echo_buf_r.assign(96000, 0.0f);
            dub_echo_lp.setLowPass(3200.0f, 44100.0f);

            pitch_ring_l.assign(4096, 0.0f);
            pitch_ring_r.assign(4096, 0.0f);

            ping_pong_buf_l.assign(96000, 0.0f);
            ping_pong_buf_r.assign(96000, 0.0f);

            spiral_buf_l.assign(96000, 0.0f);
            spiral_buf_r.assign(96000, 0.0f);

            roll_buf_l.assign(96000, 0.0f);
            roll_buf_r.assign(96000, 0.0f);

            keyshift_ring_l.assign(4096, 0.0f);
            keyshift_ring_r.assign(4096, 0.0f);
        }

                // Motor Profissional de Detecção de BPM Multi-Banda (Rekordbox / Traktor / Mixxx Industry Standard)
        // Coincidência de Transientes Sub-Bass (40-110Hz) + Click Percussivo (1.5k-4kHz) + Autocorrelação de Fase
        double analyzeBpm(const std::vector<float>& samples, int sr) {
            if (samples.empty() || sr <= 0) return 128.0;

            // Filtro 1: Sub-Bass Passa-Baixa Butterworth 110Hz (Energia mecânica do kick)
            DJBiquadFilter kick_sub;
            kick_sub.setLowPass(110.0f, (float)sr, 0.7071f);

            // Filtro 2: Click Passa-Faixa Butterworth 2500Hz Q=1.2 (Transiente percussivo de impacto do kick)
            DJBiquadFilter kick_click;
            kick_click.setBandPass(2500.0f, (float)sr, 1.2f);

            // Resolução temporal de 200 Hz (5ms por passo) - ideal para autocorrelação robusta sem ruído
            int downsample = sr / 200;
            if (downsample <= 0) downsample = 1;
            size_t max_src = std::min((size_t)(60 * sr), samples.size());
            size_t env_size = max_src / downsample;
            if (env_size < 400) return 128.0;

            std::vector<float> env_sub(env_size, 0.0f);
            std::vector<float> env_click(env_size, 0.0f);
            size_t sample_idx = 0;

            for (size_t i = 0; i < env_size; i++) {
                float sum_sub = 0.0f;
                float sum_click = 0.0f;
                for (int d = 0; d < downsample && sample_idx < samples.size(); d++, sample_idx++) {
                    float s_raw = samples[sample_idx];
                    
                    float s_sub = s_raw;
                    float s_sub_r = s_raw;
                    kick_sub.process(s_sub, s_sub_r);
                    sum_sub += s_sub * s_sub;

                    float s_clk = s_raw;
                    float s_clk_r = s_raw;
                    kick_click.process(s_clk, s_clk_r);
                    sum_click += s_clk * s_clk;
                }
                env_sub[i] = std::sqrt(sum_sub / (float)downsample);
                env_click[i] = std::sqrt(sum_click / (float)downsample);
            }

            // Spectral Flux Diferenciado (Apenas ataques positivos onde Sub e Click coincidem)
            std::vector<float> onset(env_size, 0.0f);
            float max_onset = 0.0f;
            for (size_t i = 1; i < env_size; i++) {
                float d_sub = env_sub[i] - env_sub[i - 1];
                float d_clk = env_click[i] - env_click[i - 1];
                float pos_sub = d_sub > 0.0005f ? d_sub : 0.0f;
                float pos_clk = d_clk > 0.0002f ? d_clk : 0.0f;

                float val = pos_sub * (1.0f + pos_clk * 15.0f);
                onset[i] = val;
                if (val > max_onset) max_onset = val;
            }

            if (max_onset < 1e-5f) return 128.0;

            // Autocorrelação Autêntica por Lags de Tempo (60 BPM a 180 BPM)
            // Lags a 200 Hz: 180 BPM = lag ~66 amostras | 65 BPM = lag ~184 amostras
            int min_lag = (int)(60.0 / 185.0 * 200.0);
            int max_lag = (int)(60.0 / 65.0 * 200.0);

            float best_corr = -1.0f;
            int best_lag = 100;
            size_t max_scan = env_size - max_lag * 2;

            for (int lag = min_lag; lag <= max_lag; lag++) {
                float corr = 0.0f;
                // Autocorrelação com reforço do 2º compasso (harmônico de beat 1 e beat 2)
                for (size_t i = 0; i < max_scan; i += 2) {
                    float o0 = onset[i];
                    if (o0 > 0.0001f) {
                        corr += o0 * (onset[i + lag] * 1.0f + onset[i + lag * 2] * 0.5f);
                    }
                }
                if (corr > best_corr) {
                    best_corr = corr;
                    best_lag = lag;
                }
            }

            // Refinamento Parabólico de Sub-Amostra para precisão de 0.01 BPM
            double refined_lag = (double)best_lag;
            if (best_lag > min_lag && best_lag < max_lag) {
                float c_prev = 0.0f, c_next = 0.0f;
                for (size_t i = 0; i < max_scan; i += 2) {
                    float o0 = onset[i];
                    if (o0 > 0.0001f) {
                        c_prev += o0 * (onset[i + best_lag - 1] * 1.0f + onset[i + (best_lag - 1) * 2] * 0.5f);
                        c_next += o0 * (onset[i + best_lag + 1] * 1.0f + onset[i + (best_lag + 1) * 2] * 0.5f);
                    }
                }
                double denom = 2.0 * (2.0 * best_corr - c_prev - c_next);
                if (std::abs(denom) > 1e-6) {
                    double delta = (c_next - c_prev) / denom;
                    refined_lag += std::clamp(delta, -0.5, 0.5);
                }
            }

            double detected_bpm = (60.0 * 200.0) / refined_lag;
            while (detected_bpm < 75.0) detected_bpm *= 2.0;
            while (detected_bpm > 185.0) detected_bpm *= 0.5;

            // Cravação inteligente em números inteiros (se dentro de tolerância de 0.08 BPM)
            double near_int = std::round(detected_bpm);
            if (std::abs(detected_bpm - near_int) <= 0.08) {
                detected_bpm = near_int;
            }

            return std::round(detected_bpm * 100.0) / 100.0;
        }

        // Detecção de Downbeat 1.1 com Alinhamento de Crista de Pico Real (Peak Crest Alignment)
        double analyzeDownbeatOffset(const std::vector<float>& samples, int sr, double track_bpm) {
            if (samples.empty() || track_bpm <= 10.0 || sr <= 0) return 0.0;

            DJBiquadFilter kick_lp;
            kick_lp.setLowPass(120.0f, (float)sr, 0.7071f);

            double spb = 60.0 / track_bpm;
            int downsample = sr / 400; // 400 Hz (2.5ms por amostra)
            if (downsample <= 0) downsample = 1;

            int max_search_samples = std::min((int)(45.0 * sr), (int)samples.size());
            int env_len = max_search_samples / downsample;
            if (env_len < 200) return 0.0;

            std::vector<float> env(env_len, 0.0f);
            size_t sample_idx = 0;
            float max_env_val = 0.0f;
            for (int i = 0; i < env_len; i++) {
                float sum = 0.0f;
                for (int d = 0; d < downsample && sample_idx < samples.size(); d++, sample_idx++) {
                    float s = samples[sample_idx];
                    float s_r = s;
                    kick_lp.process(s, s_r);
                    sum += s * s;
                }
                float val = std::sqrt(sum / (float)downsample);
                env[i] = val;
                if (val > max_env_val) max_env_val = val;
            }

            if (max_env_val < 0.005f) return 0.0;

            std::vector<float> attack_flux(env_len, 0.0f);
            float max_flux = 0.0f;
            for (int i = 1; i < env_len; i++) {
                float diff = env[i] - env[i - 1];
                if (diff > 0.0005f) {
                    attack_flux[i] = diff;
                    if (diff > max_flux) max_flux = diff;
                }
            }

            int beat_step_env = (int)(spb * 400.0);
            if (beat_step_env <= 1) return 0.0;

            // Detecta a região de kicks reais
            float kick_thresh = std::max(0.015f, max_env_val * 0.25f);
            float flux_thresh = std::max(0.002f, max_flux * 0.20f);
            int first_kick_region = 0;
            for (int i = 1; i < env_len - beat_step_env; i++) {
                if (env[i] >= kick_thresh && attack_flux[i] >= flux_thresh) {
                    int check_next = i + beat_step_env;
                    int tol = std::max(2, beat_step_env / 10);
                    bool has_rep = false;
                    for (int t = -tol; t <= tol; t++) {
                        int idx = check_next + t;
                        if (idx < env_len && (env[idx] >= kick_thresh * 0.7f || attack_flux[idx] >= flux_thresh * 0.5f)) {
                            has_rep = true;
                            break;
                        }
                    }
                    if (has_rep) {
                        first_kick_region = i;
                        break;
                    }
                }
            }

            int search_start = std::max(0, first_kick_region - beat_step_env);
            int search_limit = std::min(search_start + beat_step_env * 4, env_len - beat_step_env * 8);
            if (search_limit <= search_start) {
                search_start = 0;
                search_limit = std::min(beat_step_env * 4, env_len - beat_step_env * 8);
            }

            float max_score = -1.0f;
            int best_offset_env = 0;

            for (int o = search_start; o < search_limit; o++) {
                float score = 0.0f;
                for (int b = 0; b < 16; b++) {
                    int idx = o + b * beat_step_env;
                    if (idx < env_len) {
                        score += attack_flux[idx] * (1.0f + (b % 4 == 0 ? 0.5f : 0.0f));
                    }
                }
                if (score > max_score) {
                    max_score = score;
                    best_offset_env = o;
                }
            }

            // Alinhamento exato na Crista de Pico de Amplitude PCM real
            int64_t target_pcm_sample = (int64_t)best_offset_env * downsample;
            int pcm_window = (int)(spb * 0.15 * sr); // Janela de +-15% de beat
            int64_t scan_start = std::max((int64_t)0, target_pcm_sample - pcm_window);
            int64_t scan_end = std::min((int64_t)samples.size() - 1, target_pcm_sample + pcm_window);

            int64_t peak_pcm_sample = target_pcm_sample;
            float max_pcm_val = 0.0f;
            for (int64_t s = scan_start; s <= scan_end; s++) {
                float val = std::abs(samples[s]);
                if (val > max_pcm_val) {
                    max_pcm_val = val;
                    peak_pcm_sample = s;
                }
            }

            // Projeta a fase em módulo de beat para o tempo 0.0s da faixa
            double kick_sec = (double)peak_pcm_sample / (double)sr;
            double first_beat_sec = std::fmod(kick_sec, spb);
            if (first_beat_sec < 0.0) first_beat_sec += spb;

            return first_beat_sec;
        }

        // Análise Harmônica Automática: 12-Semitone Chromagram + Krumhansl-Schmuckler Camelot Detection
        std::string analyzeMusicalKey(const std::vector<float>& samples, int sr) {
            if (samples.empty() || sr <= 0) return "8A";

            // Perfis cognitivos Krumhansl-Schmuckler (Major e Minor)
            const float major_profile[12] = { 6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f, 2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f };
            const float minor_profile[12] = { 6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f, 2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f };

            // Tabela da Roda Camelot: Pitch 0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F, 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B
            const char* major_camelot[12] = { "8B", "3B", "10B", "5B", "12B", "7B", "2B", "9B", "4B", "11B", "6B", "1B" };
            const char* minor_camelot[12] = { "5A", "12A", "7A", "2A", "9A", "4A", "11A", "6A", "1A", "8A", "3A", "10A" };

            float chroma[12] = { 0.0f };
            size_t max_samples = std::min((size_t)(45 * sr), samples.size());
            size_t step = sr / 100; // janelas de ~10ms
            if (step <= 0) step = 1;

            // Análise nos semitons centrais (C3 ~130.81Hz até B5 ~987.77Hz)
            for (int pitch = 0; pitch < 12; pitch++) {
                float pitch_sum = 0.0f;
                for (int octave = 3; octave <= 5; octave++) {
                    float freq = 440.0f * std::pow(2.0f, ((float)pitch - 9.0f + (float)(octave - 4) * 12.0f) / 12.0f);
                    float w0 = 2.0f * (float)M_PI * freq / (float)sr;
                    
                    for (size_t s = 0; s + 512 < max_samples; s += step * 8) {
                        float cos_sum = 0.0f;
                        float sin_sum = 0.0f;
                        for (int k = 0; k < 256; k++) {
                            float smp = samples[s + k];
                            cos_sum += smp * std::cos(w0 * (float)k);
                            sin_sum += smp * std::sin(w0 * (float)k);
                        }
                        pitch_sum += (cos_sum * cos_sum + sin_sum * sin_sum);
                    }
                }
                chroma[pitch] = pitch_sum;
            }

            // Normalização do vetor chroma
            float c_mean = 0.0f;
            for (int i = 0; i < 12; i++) c_mean += chroma[i];
            c_mean /= 12.0f;

            float max_corr = -1e9f;
            std::string best_key = "8A";

            for (int shift = 0; shift < 12; shift++) {
                float corr_maj = 0.0f;
                float corr_min = 0.0f;
                for (int i = 0; i < 12; i++) {
                    float c_val = chroma[(i + shift) % 12] - c_mean;
                    corr_maj += c_val * major_profile[i];
                    corr_min += c_val * minor_profile[i];
                }
                if (corr_maj > max_corr) {
                    max_corr = corr_maj;
                    best_key = major_camelot[shift];
                }
                if (corr_min > max_corr) {
                    max_corr = corr_min;
                    best_key = minor_camelot[shift];
                }
            }
            return best_key;
        }

        void analyzeBPMAndGrid(const std::string& path, bool force_reanalyze = false) {
            if (buffer_l.empty() || sample_rate < 1000.0) return;

            // 1. CARREGAMENTO PRIORITÁRIO DIRETO DA ENGINE PIONEER REKORDBOX (ANLZ / PQTZ)
            RekordboxAnalysis rb_analysis;
            bool rb_loaded = false;
            if (!force_reanalyze) {
                rb_loaded = DJRekordboxEngine::getInstance().loadTrackRekordboxGrid(path, rb_analysis, sample_rate);
            }

            std::filesystem::path fpath = std::filesystem::u8path(path);
            std::filesystem::path meta_path = fpath;
            meta_path.replace_extension(".meta");

            if (rb_loaded && rb_analysis.is_valid && rb_analysis.main_bpm > 20.0) {
                bpm = rb_analysis.main_bpm;
                first_beat_frame = (double)rb_analysis.first_beat_frame;
                cue_frame = first_beat_frame;
                current_frame = first_beat_frame;
                rekordbox_beats = rb_analysis.beats;
                has_rekordbox_grid = true;

                KuroUtils::Log("[DJ Engine] Grid do Pioneer Rekordbox Ativado com Sucesso Deck " + 
                               std::to_string(deck_id + 1) + ": " + std::to_string(bpm) + " BPM, Downbeat 1.1 em " + 
                               std::to_string(first_beat_frame / sample_rate) + "s (" + 
                               std::to_string(rb_analysis.first_beat_ms) + " ms) - Total Beats: " + 
                               std::to_string(rb_analysis.total_beats));
            } else {
                has_rekordbox_grid = false;
                rekordbox_beats.clear();

                // 2. Se a música ainda não foi analisada no Rekordbox, verifica arquivo .meta com metadados prévios
                double parsed_bpm = 0.0;
                double parsed_first_beat_frame = -1.0;
                bool is_rekordbox_meta = false;

                if (!force_reanalyze && std::filesystem::exists(meta_path)) {
                    std::ifstream mf(meta_path);
                    if (mf.is_open()) {
                        std::string line;
                        while (std::getline(mf, line)) {
                            if (line.find("\"analyzer_version\":") != std::string::npos || line.find("\"analyzer\":") != std::string::npos) {
                                if (line.find("rekordbox") != std::string::npos || line.find("qm_dsp") != std::string::npos) {
                                    is_rekordbox_meta = true;
                                }
                            }
                            size_t bpos = line.find("\"bpm\":");
                            if (bpos != std::string::npos) {
                                std::string val = line.substr(bpos + 6);
                                for (char& c : val) { if (c == ',') c = '.'; }
                                try { parsed_bpm = std::stod(val); } catch (...) {}
                            }
                            size_t spos = line.find("\"first_beat_sec\":");
                            if (spos != std::string::npos) {
                                std::string val = line.substr(spos + 17);
                                for (char& c : val) { if (c == ',') c = '.'; }
                                try {
                                    double sec = std::stod(val);
                                    parsed_first_beat_frame = sec * sample_rate;
                                } catch (...) {}
                            }
                            size_t fpos = line.find("\"first_beat_frame\":");
                            if (fpos != std::string::npos && parsed_first_beat_frame < 0.0) {
                                std::string val = line.substr(fpos + 19);
                                std::string clean_val;
                                for (char c : val) {
                                    if (c >= '0' && c <= '9') clean_val += c;
                                    else if (c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n') break;
                                }
                                try { if (!clean_val.empty()) parsed_first_beat_frame = std::stod(clean_val); } catch (...) {}
                            }
                            size_t kpos = line.find("\"key\":");
                            if (kpos != std::string::npos) {
                                size_t q1 = line.find('"', kpos + 6);
                                if (q1 != std::string::npos) {
                                    size_t q2 = line.find('"', q1 + 1);
                                    if (q2 != std::string::npos) {
                                        key_signature = line.substr(q1 + 1, q2 - q1 - 1);
                                    }
                                }
                            }
                        }
                    }
                }

                if (!force_reanalyze && is_rekordbox_meta && parsed_bpm >= 20.0 && parsed_first_beat_frame >= 0.0) {
                    bpm = parsed_bpm;
                    first_beat_frame = parsed_first_beat_frame;
                    rebuildRekordboxBeats();
                    KuroUtils::Log("[DJ Engine] Meta Grid Carregado Instantaneamente Deck " + std::to_string(deck_id + 1) + 
                                   ": " + std::to_string(bpm) + " BPM | Downbeat 1.1: " + 
                                   std::to_string(first_beat_frame / sample_rate) + "s | Total Beats: " + 
                                   std::to_string(rekordbox_beats.size()));
                } else {
                    // Executa o novo motor Queen Mary (Mixxx / QM-DSP)
                    QMBeatGridResult qm = QMBeatGridEngine::analyze(buffer_l, buffer_r, sample_rate, 135.0);
                    if (qm.is_valid && qm.bpm >= 30.0) {
                        bpm = qm.bpm;
                        first_beat_frame = qm.first_beat_frame;
                        rekordbox_beats = qm.beats;
                        has_rekordbox_grid = true;
                        KuroUtils::Log("[DJ Engine] QM BeatGrid Engine Ativado Deck " + std::to_string(deck_id + 1) + 
                                       ": " + std::to_string(bpm) + " BPM (Raw: " + std::to_string(qm.raw_bpm) + 
                                       ") | Downbeat 1.1: " + std::to_string(first_beat_frame / sample_rate) + 
                                       "s | Total Beats: " + std::to_string(qm.total_beats));
                    } else if (parsed_bpm >= 20.0 && parsed_first_beat_frame >= 0.0) {
                        bpm = parsed_bpm;
                        first_beat_frame = parsed_first_beat_frame;
                    } else if (parsed_bpm >= 20.0) {
                        bpm = parsed_bpm;
                        double offset_sec = analyzeDownbeatOffset(buffer_l, (int)sample_rate, bpm);
                        first_beat_frame = offset_sec * sample_rate;
                    } else {
                        bpm = analyzeBpm(buffer_l, (int)sample_rate);
                        double offset_sec = analyzeDownbeatOffset(buffer_l, (int)sample_rate, bpm);
                        first_beat_frame = offset_sec * sample_rate;
                    }
                }

                // Análise de Tonalidade Musical Camelot nativa caso a faixa ainda não tenha tag
                if (key_signature.empty() || key_signature == "C" || key_signature == "Unknown") {
                    key_signature = analyzeMusicalKey(buffer_l, (int)sample_rate);
                    KuroUtils::Log("[DJ Engine] Tonalidade Musical Detectada Deck " + std::to_string(deck_id + 1) + ": " + key_signature + " (Camelot)");
                }

                cue_frame = first_beat_frame;
                current_frame = first_beat_frame;

                // Salva permanentemente o arquivo .meta com alta precisão
                if (has_rekordbox_grid && !rekordbox_beats.empty()) {
                    try {
                        std::ofstream mf(meta_path);
                        if (mf.is_open()) {
                            mf.imbue(std::locale::classic());
                            double offset_ms = (first_beat_frame / sample_rate) * 1000.0;
                            mf << std::fixed << std::setprecision(2);
                            mf << "{\n";
                            mf << "  \"analyzer\": \"qm_dsp_tempotracker\",\n";
                            mf << "  \"bpm\": " << bpm << ",\n";
                            mf << "  \"key\": \"" << key_signature << "\",\n";
                            mf << "  \"first_beat_offset_ms\": " << offset_ms << ",\n";
                            mf << "  \"first_beat_sec\": " << std::setprecision(5) << (first_beat_frame / sample_rate) << ",\n";
                            mf << "  \"first_beat_frame\": " << (int64_t)std::round(first_beat_frame) << ",\n";
                            mf << "  \"total_beats\": " << (int)rekordbox_beats.size() << ",\n";
                            mf << "  \"is_downbeat_exact\": true\n";
                            mf << "}\n";
                        }
                    } catch (...) {}
                }
            }

            // 3. Auto Cue Pioneer Rekordbox: Mapeamento dos Hot Cues
            cue_frame = first_beat_frame;
            current_frame = first_beat_frame;

            const uint32_t cue_colors[8] = {
                0xFF00E5FF, // 1: Cyan (Intro / Downbeat)
                0xFF00FF66, // 2: Green (Intro 16 / Build)
                0xFFFF0055, // 3: Vivid Red/Magenta (Drop 1)
                0xFFFFCC00, // 4: Amber/Yellow (Breakdown)
                0xFFBF00FF, // 5: Neon Purple (Drop 2)
                0xFFFF6600, // 6: Deep Orange (Outro)
                0xFF00BFFF, // 7: Deep Sky Blue (Fade)
                0xFFFFFFFF  // 8: Pure White (End)
            };

            for (int i = 0; i < 8; i++) {
                hot_cues[i].color_rgba = cue_colors[i];
            }

            double beat_frames = (bpm > 20.0 && sample_rate > 1000.0) ? ((60.0 / bpm) * sample_rate) : (sample_rate * 0.5);

            // Cue 1: Downbeat 1 (Intro)
            hot_cues[0].active = true;
            hot_cues[0].time_sec = first_beat_frame / sample_rate;
            hot_cues[0].label = "BEAT 1";

            // Cue 2: Intro Phrase 16 bars (64 beats)
            double b64 = first_beat_frame + 64.0 * beat_frames;
            if (b64 < (double)buffer_l.size() * 0.4) {
                hot_cues[1].active = true;
                hot_cues[1].time_sec = b64 / sample_rate;
                hot_cues[1].label = "BUILD 1";
            }

            // Análise de frases de 32 beats para detecção de Drop 1, Breakdown e Drop 2
            double phrase_beats = 32.0;
            double phrase_frames = phrase_beats * beat_frames;
            int num_phrases = (int)((buffer_l.size() - first_beat_frame) / phrase_frames);

            if (num_phrases >= 6) {
                std::vector<float> phrase_bass(num_phrases, 0.0f);
                for (int ph = 0; ph < num_phrases; ph++) {
                    size_t pf_start = (size_t)(first_beat_frame + ph * phrase_frames);
                    size_t pf_end = std::min(buffer_l.size(), (size_t)(pf_start + phrase_frames));
                    float sum_b = 0.0f;
                    int cnt = 0;
                    for (size_t f = pf_start; f < pf_end; f += 8) {
                        float s = (buffer_l[f] + buffer_r[f]) * 0.5f;
                        sum_b += s * s;
                        cnt++;
                    }
                    phrase_bass[ph] = (cnt > 0) ? std::sqrt(sum_b / (float)cnt) : 0.0f;
                }

                // 1. Detecta DROP 1
                int drop1_ph = -1;
                float max_jump = 0.0f;
                int max_ph_search = std::min(num_phrases * 45 / 100, num_phrases - 2);
                for (int ph = 2; ph <= max_ph_search; ph++) {
                    float jump = phrase_bass[ph] - phrase_bass[ph - 1];
                    if (jump > max_jump && phrase_bass[ph] > 0.15f) {
                        max_jump = jump;
                        drop1_ph = ph;
                    }
                }
                if (drop1_ph < 0 && num_phrases >= 4) drop1_ph = 4;

                if (drop1_ph > 0) {
                    double d1_frame = first_beat_frame + drop1_ph * phrase_frames;
                    hot_cues[2].active = true;
                    hot_cues[2].time_sec = d1_frame / sample_rate;
                    hot_cues[2].label = "DROP 1";
                }

                // 2. Detecta BREAKDOWN
                int break_ph = -1;
                float min_e = 1e9f;
                int start_break_search = (drop1_ph > 0) ? drop1_ph + 2 : 5;
                int end_break_search = std::min(num_phrases * 75 / 100, num_phrases - 2);
                for (int ph = start_break_search; ph <= end_break_search; ph++) {
                    if (phrase_bass[ph] < min_e) {
                        min_e = phrase_bass[ph];
                        break_ph = ph;
                    }
                }
                if (break_ph > 0) {
                    double br_frame = first_beat_frame + break_ph * phrase_frames;
                    hot_cues[3].active = true;
                    hot_cues[3].time_sec = br_frame / sample_rate;
                    hot_cues[3].label = "BREAK";

                    // 3. Detecta DROP 2
                    int drop2_ph = -1;
                    float max_d2_jump = 0.0f;
                    for (int ph = break_ph + 1; ph < std::min(num_phrases - 1, break_ph + 6); ph++) {
                        float jump = phrase_bass[ph] - phrase_bass[ph - 1];
                        if (jump > max_d2_jump && phrase_bass[ph] > 0.15f) {
                            max_d2_jump = jump;
                            drop2_ph = ph;
                        }
                    }
                    if (drop2_ph > 0) {
                        double d2_frame = first_beat_frame + drop2_ph * phrase_frames;
                        hot_cues[4].active = true;
                        hot_cues[4].time_sec = d2_frame / sample_rate;
                        hot_cues[4].label = "DROP 2";
                    }
                }

                // 4. OUTRO
                int outro_ph = std::max(num_phrases - 4, (int)(num_phrases * 0.82));
                if (outro_ph > 0 && outro_ph < num_phrases) {
                    double out_frame = first_beat_frame + outro_ph * phrase_frames;
                    hot_cues[5].active = true;
                    hot_cues[5].time_sec = out_frame / sample_rate;
                    hot_cues[5].label = "OUTRO";
                }

                // 5. FADE
                int fade_ph = std::max(0, num_phrases - 2);
                if (fade_ph > 0 && fade_ph < num_phrases) {
                    double fade_frame = first_beat_frame + fade_ph * phrase_frames;
                    hot_cues[6].active = true;
                    hot_cues[6].time_sec = fade_frame / sample_rate;
                    hot_cues[6].label = "FADE";
                }

                // 6. END
                double end_frame = std::max(0.0, (double)buffer_l.size() - 8.0 * beat_frames);
                hot_cues[7].active = true;
                hot_cues[7].time_sec = end_frame / sample_rate;
                hot_cues[7].label = "END";
            }

            // 4. Sincronização direta com o DJGridEngine (Metrônomo e alinhamento visual de alta fidelidade)
            auto& ge = DJGridEngine::getInstance();
            ge.setBpm(deck_id, bpm);
            ge.setFirstBeatMarker(deck_id, first_beat_frame, sample_rate);
            auto gd = ge.getGridData(deck_id);
            gd.bpm = bpm;
            gd.first_beat_frame = first_beat_frame;
            gd.is_analyzed = true;
            ge.setGridData(deck_id, gd);

            // 5. Salva a calibração do Rekordbox no arquivo .meta para persistência definitiva
            try {
                std::ofstream mf(meta_path);
                if (mf.is_open()) {
                    double offset_ms = (first_beat_frame / sample_rate) * 1000.0;
                    mf << "{\n";
                    mf << "  \"analyzer\": \"rekordbox_anlz\",\n";
                    mf << "  \"bpm\": " << bpm << ",\n";
                    mf << "  \"key\": \"" << key_signature << "\",\n";
                    mf << "  \"first_beat_offset_ms\": " << offset_ms << ",\n";
                    mf << "  \"first_beat_sec\": " << (first_beat_frame / sample_rate) << ",\n";
                    mf << "  \"first_beat_frame\": " << (int64_t)first_beat_frame << ",\n";
                    mf << "  \"title\": \"" << track_title << "\",\n";
                    mf << "  \"artist\": \"" << track_artist << "\",\n";
                    mf << "  \"is_downbeat_exact\": true\n";
                    mf << "}\n";
                }
            } catch (...) {}

            // 6. Atualiza no DJLibraryManager
            DJLibraryTrack lt;
            lt.local_wav_path = track_path;
            lt.title = track_title;
            lt.artist = track_artist;
            lt.bpm = bpm;
            lt.key = key_signature;
            DJLibraryManager::getInstance().addOrUpdateTrack(lt);

            char sbuf[96];
            snprintf(sbuf, sizeof(sbuf), "REKORDBOX: %.2f BPM (Beat 1.1)", bpm);
            grid_status_msg = sbuf;
        }

        bool loadTrack(const std::string& path) {
            std::lock_guard<std::mutex> lock(deck_mutex);
            is_playing = false;
            
            std::string resolved_path = KuroAudio::resolveExistingAudioPath(path);
            if (resolved_path.empty()) resolved_path = path;
            this->track_path = resolved_path;

            unsigned int channels = 0;
            unsigned int sr = 0;
            drwav_uint64 total_pcm_frames = 0;
            float* pSampleData = nullptr;
            bool is_mp3_decoded = false;

            // 1. Tenta decodificar nativamente como WAV
#ifdef _WIN32
            try {
                std::filesystem::path p = std::filesystem::u8path(resolved_path);
                pSampleData = drwav_open_file_and_read_pcm_frames_f32_w(p.wstring().c_str(), &channels, &sr, &total_pcm_frames, NULL);
            } catch (...) {}
#endif
            if (!pSampleData) {
                pSampleData = drwav_open_file_and_read_pcm_frames_f32(resolved_path.c_str(), &channels, &sr, &total_pcm_frames, NULL);
            }

            // 2. Se drwav em arquivo falhar, lê o arquivo em memória para suportar MP3 e caminhos com caracteres especiais
            if (!pSampleData || total_pcm_frames == 0) {
                try {
                    std::error_code ec;
                    std::filesystem::path p = std::filesystem::u8path(resolved_path);
                    if (std::filesystem::exists(p, ec)) {
                        std::ifstream f(p, std::ios::binary | std::ios::ate);
                        if (f.is_open()) {
                            std::streamsize sz = f.tellg();
                            if (sz > 4096) {
                                f.seekg(0, std::ios::beg);
                                std::vector<char> fbuf((size_t)sz);
                                if (f.read(fbuf.data(), sz)) {
                                    // Tenta drwav da memória
                                    pSampleData = drwav_open_memory_and_read_pcm_frames_f32(fbuf.data(), fbuf.size(), &channels, &sr, &total_pcm_frames, NULL);
                                    // Se ainda falhar, decodifica como MP3 via dr_mp3
                                    if (!pSampleData || total_pcm_frames == 0) {
                                        drmp3_config mp3_cfg;
                                        drmp3_uint64 mp3_frames = 0;
                                        pSampleData = drmp3_open_memory_and_read_pcm_frames_f32(fbuf.data(), fbuf.size(), &mp3_cfg, &mp3_frames, NULL);
                                        if (pSampleData && mp3_frames > 0) {
                                            channels = mp3_cfg.channels;
                                            sr = mp3_cfg.sampleRate;
                                            total_pcm_frames = mp3_frames;
                                            is_mp3_decoded = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } catch (...) {}
            }
            
            if (!pSampleData || total_pcm_frames == 0) {
                KuroUtils::Log("[DJ Engine] Erro ao decodificar arquivo de audio (WAV/MP3): " + path);
                return false;
            }

            sample_rate = (double)sr;
            duration_sec = (double)total_pcm_frames / sample_rate;
            current_frame = 0.0;
            cue_frame = 0.0;

            buffer_l.resize(total_pcm_frames);
            buffer_r.resize(total_pcm_frames);
            detailed_waveform.clear();
            raw_detailed_waveform.clear();

            if (channels == 1) {
                for (size_t i = 0; i < total_pcm_frames; i++) {
                    buffer_l[i] = pSampleData[i];
                    buffer_r[i] = pSampleData[i];
                }
            } else {
                for (size_t i = 0; i < total_pcm_frames; i++) {
                    buffer_l[i] = pSampleData[i * channels];
                    buffer_r[i] = pSampleData[i * channels + 1];
                }
            }

            if (is_mp3_decoded) {
                drmp3_free(pSampleData, NULL);
            } else {
                drwav_free(pSampleData, NULL);
            }

            // Gera waveform 3-Band normalizada de 1000 pontos (Pioneer Rekordbox Standard)
            size_t num_peaks = 1000;
            overview_peaks.resize(num_peaks, 0.0f);
            overview_low.resize(num_peaks, 0.0f);
            overview_mid.resize(num_peaks, 0.0f);
            overview_high.resize(num_peaks, 0.0f);

            DJBiquadFilter flt_l1, flt_l2, flt_h1, flt_h2;
            float f_sr = (sample_rate > 1000.0) ? (float)sample_rate : 44100.0f;
            flt_l1.setLowPass(250.0f, f_sr, 0.7071f);
            flt_l2.setLowPass(250.0f, f_sr, 0.7071f);
            flt_h1.setHighPass(2500.0f, f_sr, 0.7071f);
            flt_h2.setHighPass(2500.0f, f_sr, 0.7071f);

            size_t step = total_pcm_frames / num_peaks;
            if (step < 1) step = 1;
            for (size_t p = 0; p < num_peaks; p++) {
                size_t start = p * step;
                size_t end = std::min(start + step, (size_t)total_pcm_frames);
                float max_peak = 0.0f;
                float max_low = 0.0f;
                float max_mid = 0.0f;
                float max_high = 0.0f;

                for (size_t j = start; j < end; j += 2) {
                    float in_l = buffer_l[j];
                    float in_r = buffer_r[j];
                    float mono = (std::abs(in_l) + std::abs(in_r)) * 0.5f;
                    if (mono > max_peak) max_peak = mono;

                    // Low pass (LR4)
                    float y_low = in_l, y_low_r = in_r;
                    flt_l1.process(y_low, y_low_r);
                    flt_l2.process(y_low, y_low_r);
                    float low_v = (std::abs(y_low) + std::abs(y_low_r)) * 0.5f;
                    if (low_v > max_low) max_low = low_v;

                    // High pass (LR4)
                    float y_high = in_l, y_high_r = in_r;
                    flt_h1.process(y_high, y_high_r);
                    flt_h2.process(y_high, y_high_r);
                    float high_v = (std::abs(y_high) + std::abs(y_high_r)) * 0.5f;
                    if (high_v > max_high) max_high = high_v;

                    // Mid complementar (LR4)
                    float y_mid = in_l - y_low - y_high;
                    float y_mid_r = in_r - y_low_r - y_high_r;
                    float mid_v = (std::abs(y_mid) + std::abs(y_mid_r)) * 0.5f;
                    if (mid_v > max_mid) max_mid = mid_v;
                }

                float peak = std::clamp(max_peak, 0.001f, 1.0f);
                overview_peaks[p] = peak;
                overview_low[p]   = std::clamp(max_low * 1.35f, 0.0f, 1.0f);
                overview_mid[p]   = std::clamp(max_mid * 1.45f, 0.0f, 1.0f);
                overview_high[p]  = std::clamp(max_high * 2.10f, 0.0f, 1.0f);
            }
            generateDetailedWaveformIfNeeded();

            track_path = path;
            std::filesystem::path fpath(path);
            std::string stem = fpath.stem().string();

            auto* lib_trk = DJLibraryManager::getInstance().getTrackByPathOrStem(path);
            if (lib_trk && !DJLibraryManager::isDummyTitle(lib_trk->title)) {
                track_title = lib_trk->title;
                track_artist = lib_trk->artist;
                if (!lib_trk->key.empty()) key_signature = lib_trk->key;
                if (lib_trk->bpm > 20.0) bpm = lib_trk->bpm;
                if (!lib_trk->local_rgba_path.empty()) cover_art_path = lib_trk->local_rgba_path;
            } else {
                std::string meta_t, meta_a, meta_al, meta_g, meta_k;
                double meta_b = 0.0, meta_d = 0.0;
                auto meta_file = fpath;
                meta_file.replace_extension(".meta");
                DJLibraryManager::readMetaJson(meta_file, meta_t, meta_a, meta_al, meta_g, meta_b, meta_k, meta_d);
                DJLibraryManager::extractWavEmbeddedTags(fpath, meta_t, meta_a, meta_al, meta_g, meta_b, meta_k);

                if (!meta_t.empty() && !DJLibraryManager::isDummyTitle(meta_t)) {
                    track_title = meta_t;
                    track_artist = meta_a;
                } else {
                    size_t sep = stem.find(" - ");
                    if (sep != std::string::npos) {
                        track_artist = stem.substr(0, sep);
                        track_title = stem.substr(sep + 3);
                    } else {
                        track_artist = "";
                        track_title = stem;
                    }
                }
                if (!meta_k.empty()) key_signature = meta_k;
                if (meta_b > 20.0) bpm = meta_b;
                cover_art_path = DJLibraryManager::resolveRgbaCoverArt(path, stem);
            }
            if (cover_art_path.empty()) {
                cover_art_path = DJLibraryManager::resolveRgbaCoverArt(path, stem);
            }

            eq_lp.setLowPass(320.0f, (float)sample_rate, 0.707f);
            eq_hp.setHighPass(2800.0f, (float)sample_rate, 0.707f);

            // Calibra BPM e Beatgrid automaticamente
            analyzeBPMAndGrid(path);

            // Auto-Gain / ReplayGain:
            // 1. Prioriza metadados calibrados ITU-R BS.1770-4 (-9.0 LUFS Club Standard) salvos no .meta
            bool lufs_applied = false;
            try {
                auto meta_path = std::filesystem::path(path).replace_extension(".meta");
                std::error_code ec_meta;
                if (std::filesystem::exists(meta_path, ec_meta)) {
                    std::ifstream mf(meta_path);
                    if (mf.is_open()) {
                        std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                        auto g_pos = mstr.find("\"gain_offset_db\":");
                        if (g_pos != std::string::npos) {
                            double g_val = std::stod(mstr.substr(g_pos + 17));
                            auto_gain_db = (float)std::clamp(g_val, -12.0, 12.0);
                            auto_gain_factor = std::pow(10.0f, auto_gain_db / 20.0f);
                            lufs_applied = true;
                        }
                        auto l_pos = mstr.find("\"lufs\":");
                        if (l_pos != std::string::npos) {
                            track_lufs = (float)std::stod(mstr.substr(l_pos + 7));
                        }
                    }
                }
            } catch (...) {}

            // 2. Fallback: calcula energia RMS e normaliza para o padrão nominal club (-14 LUFS / RMS 0.18)
            if (!lufs_applied) {
                double sum_sq = 0.0;
                size_t step_sz = std::max((size_t)1, (size_t)total_pcm_frames / 12000);
                size_t sample_cnt = 0;
                for (size_t i = 0; i < (size_t)total_pcm_frames; i += step_sz) {
                    float s = (std::abs(buffer_l[i]) + std::abs(buffer_r[i])) * 0.5f;
                    sum_sq += s * s;
                    sample_cnt++;
                }
                if (sample_cnt > 0) {
                    double rms = std::sqrt(sum_sq / sample_cnt);
                    if (rms > 0.01) {
                        double target_rms = 0.18; // ~ -14.9 dBFS RMS
                        auto_gain_factor = (float)std::clamp(target_rms / rms, 0.45, 2.2);
                        auto_gain_db = (float)(20.0 * std::log10(auto_gain_factor));
                        track_lufs = (float)(-9.0 - auto_gain_db);
                    } else {
                        auto_gain_factor = 1.0f;
                        auto_gain_db = 0.0f;
                    }
                }
            }
            resetKey();
            loadAiStems(path);

            KuroUtils::Log("[DJ Engine] Faixa carregada no Deck " + std::string(deck_id == 0 ? "A: " : "B: ") + track_title + " (" + std::to_string((int)sample_rate) + " Hz) [LUFS: " + std::to_string((int)track_lufs) + " | Auto-Gain: " + std::to_string(auto_gain_db) + " dB]");
            return true;
        }

        void togglePlay(const DJDeck* master = nullptr) {
            if (buffer_l.empty() || duration_sec <= 0.0) {
                is_playing = false;
                cue_active = false;
                return;
            }
            if (cue_active) {
                // DJ pressionou PLAY enquanto segurava CUE -> trava reprodução contínua
                cue_active = false;
                is_playing = true;
            } else {
                bool will_play = !is_playing;
                if (will_play) {
                    // Quantized Play Start: Pioneer Rekordbox / CDJ-3000 Style
                    if (master && master->is_playing && master->deck_id != deck_id && bpm > 10.0 && master->bpm > 10.0) {
                        double master_eff_bpm = master->bpm * (1.0 + master->pitch_percent * 0.01);
                        if (master_eff_bpm > 10.0 && sample_rate > 1000.0 && master->sample_rate > 1000.0) {
                            if (sync_active) {
                                // Trava pitch inicial no BPM efetivo do Master
                                pitch_percent = (float)(((master_eff_bpm / bpm) - 1.0) * 100.0);
                            }

                            if (quantize_active || sync_active) {
                                double master_beat_len = (60.0 / master->bpm) * master->sample_rate;
                                double slave_beat_len  = (60.0 / bpm) * sample_rate;
                                double m_rel = master->current_frame - master->first_beat_frame;
                                double m_phase = std::fmod(m_rel, master_beat_len);
                                if (m_phase < 0.0) m_phase += master_beat_len;
                                double m_phase_norm = m_phase / master_beat_len; // 0.0 a 1.0

                                double s_rel = current_frame - first_beat_frame;
                                double s_beat_idx = std::round(s_rel / slave_beat_len);

                                // Alinhamento musical preservando 100% o ataque/kick (transiente)
                                // Sem pular áudio para o meio da batida
                                double target_beat = s_beat_idx;
                                if (m_phase_norm >= 0.50) {
                                    // Antecipação natural do DJ: o próximo downbeat está a milissegundos
                                    target_beat = std::max(0.0, s_beat_idx);
                                }
                                current_frame = std::clamp(
                                    first_beat_frame + target_beat * slave_beat_len,
                                    0.0, (double)buffer_l.size()
                                );
                            }
                        }
                    } else if (quantize_active && bpm > 10.0 && sample_rate > 1000.0) {
                        // Deck isolado: alinha ao grid exato da faixa
                        current_frame = getQuantizedFrame(current_frame);
                    }
                    cue_active = false;
                }
                if (will_play) {
                    is_stopping_vinyl = false;
                    motor_speed = 1.0f; // Arranque limpo e instantâneo
                    is_playing = true;
                } else {
                    if (vinyl_stop_enabled) {
                        // Modo Vinyl Stop: inicia rampa suave de desaceleração de motor com queda de pitch
                        is_stopping_vinyl = true;
                        // is_playing permanece true até motor_speed desacelerar até 0.0 no loop de áudio
                    } else {
                        // Parada imediata, atômica e confiável ao apertar Pause (Modo CDJ tradicional)
                        is_stopping_vinyl = false;
                        motor_speed = 0.0f;
                        is_playing = false;
                    }
                }
            }
        }

        void pressCue() {
            if (buffer_l.empty() || duration_sec <= 0.0) {
                is_playing = false;
                cue_active = false;
                return;
            }
            if (is_playing && !cue_active) {
                // Tocando normalmente: pausa e volta instantaneamente ao CUE point
                is_stopping_vinyl = false;
                motor_speed = 1.0f;
                is_playing = false;
                current_frame = cue_frame;
                cue_active = false;
            } else {
                // Deck pausado:
                // Se o usuário moveu a agulha para longe do CUE (ex: girou o jogwheel ou buscou):
                if (std::abs(current_frame - cue_frame) > sample_rate * 0.04) {
                    double target_f = (quantize_active && bpm > 10.0 && sample_rate > 1000.0)
                                      ? getQuantizedFrame(current_frame)
                                      : current_frame;
                    cue_frame = target_f;
                    current_frame = target_f;
                    cue_active = false;
                    is_playing = false;
                    return; // Padrão Pioneer CDJ / Rekordbox: marca o ponto e permanece pausado/mudo
                }
                // Agulha já está no CUE point:
                // Audition / Cue Stutter: inicia reprodução enquanto CUE estiver pressionado
                cue_active = true;
                current_frame = cue_frame;
                is_playing = true;
            }
        }

        void releaseCue() {
            if (buffer_l.empty() || duration_sec <= 0.0) {
                is_playing = false;
                cue_active = false;
                return;
            }
            // Ao soltar o botão CUE (se estiver em pré-escuta Cue Audition)
            if (cue_active) {
                is_playing = false;
                current_frame = cue_frame;
                cue_active = false;
            }
        }

        void setHotCue(int index) {
            if (index < 0 || index >= 16) return;
            double target_frame = quantize_active ? getQuantizedFrame(current_frame) : current_frame;
            hot_cues[index].active = true;
            hot_cues[index].time_sec = target_frame / sample_rate;
            hot_cues[index].label = "CUE " + std::to_string(index + 1);
        }

        void jumpToHotCue(int index) {
            if (index < 0 || index >= 16) return;
            if (hot_cues[index].active) {
                double target_frame = hot_cues[index].time_sec * sample_rate;
                if (quantize_active && is_playing && bpm > 20.0 && sample_rate > 1000.0) {
                    target_frame = getQuantizedFrame(target_frame);
                }
                current_frame = target_frame;
            } else {
                setHotCue(index);
            }
        }

        void deleteHotCue(int index) {
            if (index < 0 || index >= 16) return;
            hot_cues[index].active = false;
        }

        void toggleLoop(double beats) {
            if (loop_active && std::abs(loop_beats - beats) < 0.001) {
                loop_active = false;
            } else {
                loop_active = true;
                loop_beats = beats;
                double target_frame = quantize_active ? getQuantizedFrame(current_frame) : current_frame;
                loop_start_frame = target_frame;
                double seconds_per_beat = 60.0 / std::max(1.0, bpm);
                loop_length_frames = beats * seconds_per_beat * sample_rate;
            }
        }

        void setLoop(double beats) {
            loop_beats = beats;
            loop_active = true;
            if (quantize_active) {
                loop_start_frame = getQuantizedFrame(loop_start_frame);
            }
            double seconds_per_beat = 60.0 / std::max(1.0, bpm);
            loop_length_frames = beats * seconds_per_beat * sample_rate;
        }

        void loopHalve() {
            if (loop_active && loop_beats > 0.125) {
                setLoop(loop_beats * 0.5);
            }
        }

        void loopDouble() {
            if (loop_active && loop_beats < 64.0) {
                setLoop(loop_beats * 2.0);
            }
        }

        void setLoopIn() {
            loop_start_frame = current_frame;
            if (loop_active && loop_length_frames > 0) {
                // ajusta
            }
        }

        void setLoopOut() {
            if (current_frame > loop_start_frame) {
                loop_length_frames = current_frame - loop_start_frame;
                double seconds_per_beat = 60.0 / std::max(1.0, bpm);
                loop_beats = loop_length_frames / (seconds_per_beat * sample_rate);
                loop_active = true;
            }
        }

        void beatJump(double beats) {
            if (buffer_l.empty()) return;
            double eff_bpm = bpm * (1.0 + pitch_percent * 0.01);
            if (eff_bpm <= 0.0) eff_bpm = bpm;
            double beat_frames = (60.0 / std::max(1.0, eff_bpm)) * sample_rate;
            double frames_delta = beats * beat_frames;
            current_frame += frames_delta;
            if (quantize_active && beat_frames > 1.0) {
                double rel = current_frame - first_beat_frame;
                double nearest_beat_idx = std::round(rel / beat_frames);
                double snapped = first_beat_frame + nearest_beat_idx * beat_frames;
                current_frame = snapped;
            }
            if (current_frame < 0.0) current_frame = 0.0;
            if (current_frame > (double)buffer_l.size()) current_frame = (double)buffer_l.size();
        }

        void cyclePitchRange() {
            if (pitch_range < 12.0f) pitch_range = 16.0f;
            else if (pitch_range < 24.0f) pitch_range = 32.0f;
            else if (pitch_range < 40.0f) pitch_range = 50.0f;
            else pitch_range = 8.0f;
        }

        // Exporta o Loop ativo (ou trecho de 4 beats) diretamente para arquivo WAV em disco
        bool exportActiveLoopToWav(const std::string& out_path) {
            if (buffer_l.empty()) return false;
            uint64_t start_f = 0;
            uint64_t len_f = 0;
            if (loop_active && loop_length_frames > 0) {
                start_f = (uint64_t)std::clamp(loop_start_frame, 0.0, (double)buffer_l.size());
                len_f = (uint64_t)std::clamp(loop_length_frames, 1.0, (double)(buffer_l.size() - start_f));
            } else {
                double sec_per_beat = 60.0 / std::max(1.0, bpm);
                double f_len = 4.0 * sec_per_beat * sample_rate;
                start_f = (uint64_t)std::clamp(current_frame, 0.0, (double)buffer_l.size());
                len_f = (uint64_t)std::clamp(f_len, 1.0, (double)(buffer_l.size() - start_f));
            }
            if (len_f == 0) return false;

            drwav_data_format format;
            format.container = drwav_container_riff;
            format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
            format.channels = 2;
            format.sampleRate = (drwav_uint32)sample_rate;
            format.bitsPerSample = 32;

            drwav wav_out;
            if (!drwav_init_file_write(&wav_out, out_path.c_str(), &format, NULL)) {
                return false;
            }

            std::vector<float> interleaved(len_f * 2);
            for (uint64_t i = 0; i < len_f; i++) {
                uint64_t idx = start_f + i;
                float l = (idx < buffer_l.size()) ? buffer_l[idx] : 0.0f;
                float r = (idx < buffer_r.size()) ? buffer_r[idx] : l;
                interleaved[i * 2 + 0] = l;
                interleaved[i * 2 + 1] = r;
            }

            drwav_write_pcm_frames(&wav_out, len_f, interleaved.data());
            drwav_uninit(&wav_out);
            return true;
        }

        void setRate14Bit(int msb, int lsb, float range_percent = -1.0f) {
            rate_msb = msb;
            rate_lsb = lsb;
            int full_val = (msb << 7) | (lsb & 0x7F); // 0 a 16383, centro 8192

            // Pioneer DDJ center click deadband (±64 counts em torno de 8192)
            float norm = 0.0f;
            if (std::abs(full_val - 8192) > 64) {
                norm = ((float)full_val - 8192.0f) / 8192.0f;
            }
            float eff_range = (range_percent > 0.0f) ? range_percent : pitch_range;
            pitch_percent = norm * eff_range;

            // Manual Takeover (Pioneer / Rekordbox / Traktor rule):
            // Ao mexer no pitch fader manualmente, o deck escravo desliga o SYNC imediatamente
            sync_active = false;
            sync_phase_diff = 0.0f;
        }

        void setVolume14Bit(int msb, int lsb) {
            vol_msb = msb;
            vol_lsb = lsb;
            int full_val = (msb << 7) | (lsb & 0x7F);
            volume = (float)full_val / 16383.0f;
        }

        void seekDelta(int delta) {
            if (buffer_l.empty()) return;
            double step = (double)delta * (sample_rate * 0.04);
            current_frame += step;
            if (current_frame < 0.0) current_frame = 0.0;
            if (current_frame > (double)buffer_l.size()) current_frame = (double)buffer_l.size();
        }

        // =========================================================================
        // INSTANT DOUBLES (Pioneer Rekordbox / Serato DJ Pro Standard)
        // Clona instantaneamente o deck de origem (amostra, posição exata, pitch e estado de loop)
        // =========================================================================
        bool cloneFrom(const DJDeck& src) {
            if (src.buffer_l.empty()) return false;
            std::lock_guard<std::mutex> lock(deck_mutex);
            
            track_path = src.track_path;
            track_title = src.track_title;
            track_artist = src.track_artist;
            cover_art_path = src.cover_art_path;
            gl_cover_texture = src.gl_cover_texture;
            has_cover_texture = src.has_cover_texture;

            buffer_l = src.buffer_l;
            buffer_r = src.buffer_r;
            overview_peaks = src.overview_peaks;
            overview_low = src.overview_low;
            overview_mid = src.overview_mid;
            overview_high = src.overview_high;
            detailed_waveform = src.detailed_waveform;
            raw_detailed_waveform = src.raw_detailed_waveform;

            sample_rate = src.sample_rate;
            duration_sec = src.duration_sec;
            bpm = src.bpm;
            first_beat_frame = src.first_beat_frame;
            pitch_percent = src.pitch_percent;
            pitch_range = src.pitch_range;
            key_signature = src.key_signature;
            key_shift_semitones = src.key_shift_semitones;
            key_sync_active = src.key_sync_active;

            for (int i = 0; i < 8; i++) {
                hot_cues[i] = src.hot_cues[i];
            }
            memory_cues = src.memory_cues;
            rekordbox_beats = src.rekordbox_beats;

            cue_frame = src.cue_frame;
            current_frame = src.current_frame;
            loop_active = src.loop_active;
            loop_beats = src.loop_beats;
            loop_start_frame = src.loop_start_frame;
            loop_length_frames = src.loop_length_frames;

            // Transfere o estado de reprodução para manter o fluxo idêntico
            is_playing = src.is_playing.load();
            cue_active = false;

            KuroUtils::Log("[Instant Doubles] Deck " + std::to_string(deck_id + 1) + 
                           " clonou perfeitamente Deck " + std::to_string(src.deck_id + 1) + 
                           " (" + track_title + ")");
            return true;
        }

        void updateSoundColorFX() {
            float sr = (float)sample_rate;
            float q = 0.707f + std::clamp(sound_color_param, 0.0f, 1.0f) * 2.8f; // Resonância calibrada Pioneer CFX
            if (sound_color_fx_mode == 0) { // FILTER
                if (filter_bipolar < 0.48f) {
                    float norm = filter_bipolar / 0.48f;
                    float cutoff = 60.0f + norm * norm * 18000.0f;
                    sound_color_filter.setLowPass(cutoff, sr, q);
                } else if (filter_bipolar > 0.52f) {
                    float norm = (filter_bipolar - 0.52f) / 0.48f;
                    float cutoff = 30.0f + norm * norm * 14000.0f;
                    sound_color_filter.setHighPass(cutoff, sr, q);
                } else {
                    sound_color_filter.setLowPass(22000.0f, sr);
                }
            } else if (sound_color_fx_mode == 1) { // NOISE
                float sweep_freq = 400.0f + std::abs(filter_bipolar - 0.5f) * 2.0f * 6000.0f;
                float noise_q = 1.0f + std::clamp(sound_color_param, 0.0f, 1.0f) * 4.0f;
                noise_sweep_filter.setBandPass(sweep_freq, sr, noise_q);
            } else if (sound_color_fx_mode == 4 || sound_color_fx_mode == 6 || sound_color_fx_mode == 7 || sound_color_fx_mode == 8 || sound_color_fx_mode == 12) {
                float damp_cutoff = 1200.0f + (1.0f - std::abs(filter_bipolar - 0.5f) * 2.0f) * 3500.0f;
                dub_echo_lp.setLowPass(damp_cutoff, sr);
            }
        }
    };

    // Gravador Profissional do Barramento Master (WAV Estéreo 32-bit Float / 24-bit PCM)
    class DJSetRecorder {
    public:
        std::atomic<bool> is_recording{ false };
        std::string current_file_path = "";
        std::string last_recorded_path = "";
        std::atomic<uint64_t> frames_recorded{ 0 };
        double sample_rate = 44100.0;
        std::chrono::steady_clock::time_point start_time;

    private:
        drwav wav;
        std::mutex queue_mutex;
        std::vector<float> ring_buffer; // Interleaved L, R
        std::thread writer_thread;
        std::atomic<bool> thread_running{ false };
        std::condition_variable cv;

    public:
        DJSetRecorder() = default;
        ~DJSetRecorder() {
            stopRecording();
        }

        bool startRecording(const std::string& dir = "scratch/recordings", double srate = 44100.0) {
            if (is_recording) return false;
            sample_rate = srate;

            try {
                std::filesystem::create_directories(dir);
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                struct tm buf;
#ifdef _WIN32
                localtime_s(&buf, &in_time_t);
#else
                localtime_r(&in_time_t, &buf);
#endif
                char time_str[64];
                strftime(time_str, sizeof(time_str), "Live_Set_%Y%m%d_%H%M%S.wav", &buf);
                std::filesystem::path full_p = std::filesystem::u8path(dir) / time_str;
                current_file_path = full_p.string();

                drwav_data_format format;
                format.container = drwav_container_riff;
                format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
                format.channels = 2;
                format.sampleRate = (drwav_uint32)sample_rate;
                format.bitsPerSample = 32;

                if (!drwav_init_file_write(&wav, current_file_path.c_str(), &format, NULL)) {
                    KuroUtils::Log("[DJ Recorder] Falha ao inicializar gravação WAV: " + current_file_path);
                    return false;
                }

                ring_buffer.clear();
                ring_buffer.reserve((size_t)sample_rate * 2 * 4);
                frames_recorded = 0;
                start_time = std::chrono::steady_clock::now();
                thread_running = true;
                is_recording = true;

                writer_thread = std::thread([this]() {
                    std::vector<float> write_chunk;
                    while (thread_running || is_recording) {
                        {
                            std::unique_lock<std::mutex> lock(queue_mutex);
                            cv.wait_for(lock, std::chrono::milliseconds(30), [this]() {
                                return !ring_buffer.empty() || !thread_running;
                            });
                            if (!ring_buffer.empty()) {
                                write_chunk.swap(ring_buffer);
                            }
                        }

                        if (!write_chunk.empty()) {
                            drwav_uint64 frames_to_write = write_chunk.size() / 2;
                            drwav_write_pcm_frames(&wav, frames_to_write, write_chunk.data());
                            frames_recorded += frames_to_write;
                            write_chunk.clear();
                        }
                    }
                });

                KuroUtils::Log("[DJ Recorder] Gravação Master iniciada: " + current_file_path);
                return true;
            } catch (const std::exception& e) {
                KuroUtils::Log("[DJ Recorder] Erro ao iniciar gravação: " + std::string(e.what()));
                return false;
            }
        }

        void writeBlock(const float* l, const float* r, unsigned int frames, float master_vol) {
            if (!is_recording) return;
            std::lock_guard<std::mutex> lock(queue_mutex);
            for (unsigned int i = 0; i < frames; i++) {
                ring_buffer.push_back(l[i] * master_vol);
                ring_buffer.push_back(r[i] * master_vol);
            }
            cv.notify_one();
        }

        void stopRecording() {
            if (!is_recording) return;
            is_recording = false;
            thread_running = false;
            cv.notify_all();
            if (writer_thread.joinable()) {
                writer_thread.join();
            }
            drwav_uninit(&wav);
            last_recorded_path = current_file_path;
            KuroUtils::Log("[DJ Recorder] Gravação Master finalizada (" + std::to_string((int)getRecordSeconds()) + "s): " + last_recorded_path);
        }

        double getRecordSeconds() const {
            if (!is_recording) return 0.0;
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<double>(now - start_time).count();
        }
    };

    class DJEngine {
    private:
        float crossfader = 0.5f; // 0.0 = Deck A (1 e 3), 1.0 = Deck B (2 e 4)
        int crossfader_curve = 0; // 0 = SMOOTH (Equal Power), 1 = SHARP (Scratch Cut), 2 = LINEAR
        float master_volume = 1.0f;
        
        bool cue_deck_a = false;
        bool cue_deck_b = false;
        bool cue_master = true;

    public:
        double host_sample_rate = 44100.0;
        DJSetRecorder set_recorder;
        
        // 4 Decks Nativos
        DJDeck decks[4] = { DJDeck(0), DJDeck(1), DJDeck(2), DJDeck(3) };
        DJDeck& deckA = decks[0]; // Deck 1
        DJDeck& deckB = decks[1]; // Deck 2
        DJDeck& deckC = decks[2]; // Deck 3
        DJDeck& deckD = decks[3]; // Deck 4

        // Master Deck Único (Traktor / Rekordbox Standard)
        int master_deck_id = 0; // Deck 0 (Deck 1) é o Master inicial

        int getMasterDeckId() const { return master_deck_id; }
        bool isMasterDeck(int id) const { return master_deck_id == id; }

        void setMasterDeck(int id) {
            if (id < 0 || id >= 4) return;
            master_deck_id = id;
            decks[id].sync_active = false;
            decks[id].sync_master_deck_id = -1;
            decks[id].sync_phase_diff = 0.0f;
            KuroUtils::Log("[DJ Engine] Deck " + std::to_string(id + 1) + " definido como MASTER");

            // Atualiza todos os outros decks com SYNC ativo para seguirem o novo Master
            for (int i = 0; i < 4; i++) {
                if (i != id && decks[i].sync_active) {
                    decks[i].sync_master_deck_id = id;
                    syncBPM(id, i);
                }
            }
        }

        bool isRecording() const { return set_recorder.is_recording; }
        double getRecordSeconds() const { return set_recorder.getRecordSeconds(); }
        std::string getRecordingPath() const { return set_recorder.current_file_path; }
        std::string getLastRecordingPath() const { return set_recorder.last_recorded_path; }
        void toggleRecording(const std::string& dir = "scratch/recordings") {
            if (set_recorder.is_recording) {
                set_recorder.stopRecording();
            } else {
                set_recorder.startRecording(dir, host_sample_rate);
            }
        }

        void toggleKeySync(int slave_id, int master_id = -1) {
            if (slave_id < 0 || slave_id >= 4) return;
            if (master_id < 0 || master_id >= 4) master_id = master_deck_id;
            if (slave_id == master_id) return;

            auto& slave = decks[slave_id];
            auto& master = decks[master_id];
            if (slave.key_sync_active) {
                slave.resetKey();
                KuroUtils::Log("[DJ Engine] KEY SYNC OFF Deck " + std::to_string(slave_id + 1));
            } else {
                int shift = DJHarmonicEngine::getOptimalKeySyncShift(master.getEffectiveKey(), slave.key_signature);
                slave.key_shift_semitones = shift;
                slave.key_sync_active = true;
                KuroUtils::Log("[DJ Engine] KEY SYNC ON Deck " + std::to_string(slave_id + 1) + " -> Shift " + std::to_string(shift) + " semitons (" + slave.getEffectiveKey() + ")");
            }
        }

        DJSamplerSlot samplers[16];
        DJBeatFXUnit beat_fx;   // FX 1 Unit (Deck 1 / Master)
        DJBeatFXUnit beat_fx2;  // FX 2 Unit (Deck 2 / Master)
        DJMasterLimiter master_limiter; // Limiter Brickwall Transparente & Auto-Gain DJM-V10
        DJMasterIsolator master_isolator;
        DJM10ChannelDSP v10_channel_dsp[4];
        DJReleaseFX release_fx[4];

        DJEngine() {
            initBuiltinSamplers();
            setCueMaster(true);
            setCueDeckA(true);
            beat_fx.enabled = false;
            beat_fx2.enabled = false;
            beat_fx.channel_assign = 0;  // Padrão Rekordbox: FX1 roteado no Canal 1
            beat_fx2.channel_assign = 1; // Padrão Rekordbox: FX2 roteado no Canal 2
            beat_fx2.multi_slots[0] = { DJBeatFXUnit::FX_MOBIUS_SAW, false, 0.0f, 4.0f, 7 };
            beat_fx2.multi_slots[1] = { DJBeatFXUnit::FX_ENIGMA_JET, false, 0.0f, 4.0f, 7 };
            beat_fx2.multi_slots[2] = { DJBeatFXUnit::FX_DOWN_ECHO,  false, 0.0f, 1.0f, 5 };
            beat_fx2.multi_slots[3] = { DJBeatFXUnit::FX_VINYL_BRAKE, false, 0.0f, 0.75f, 4 };
        }

        ~DJEngine() = default;

        void initBuiltinSamplers() {
            std::string names[16] = { 
                "CLAP SHOT", "PSY LASER", "SUB DROP", "SIREN RISER", 
                "SNARE ROLL", "CRASH BOOM", "AIR HORN", "VOCAL DROP",
                "808 SUB", "SHAKER 16", "GLITCH HIT", "REV CYMBAL",
                "ACID STAB", "CHORD HIT", "SCRATCH FX", "WHITE NOISE"
            };
            
            for (int i = 0; i < 16; i++) {
                samplers[i].name = names[i];
                samplers[i].is_playing = false;
                samplers[i].playback_pos = 0.0;
                int len = 44100 * (i == 2 || i == 3 || i == 8 || i == 11 ? 2 : 1);
                samplers[i].buffer_l.resize(len);
                samplers[i].buffer_r.resize(len);
                
                for (int s = 0; s < len; s++) {
                    float t = (float)s / 44100.0f;
                    float env = std::exp(-t * (i == 0 || i == 9 ? 12.0f : (i == 1 || i == 10 ? 6.0f : 3.0f)));
                    float wave = 0.0f;
                    if (i == 0 || i == 9 || i == 15) {
                        wave = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * env;
                    } else if (i == 1 || i == 10) {
                        float f = 2500.0f * std::exp(-t * 15.0f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    } else if (i == 2 || i == 8) {
                        float f = (i == 8 ? 65.0f : 120.0f) * std::exp(-t * 2.0f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * std::exp(-t * 1.2f);
                    } else {
                        float f = 220.0f + (i % 8) * 110.0f;
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    }
                    samplers[i].buffer_l[s] = wave * 0.8f;
                    samplers[i].buffer_r[s] = wave * 0.8f;
                }
            }
        }

        void triggerSampler(int slot) {
            if (slot >= 0 && slot < 16) {
                samplers[slot].playback_pos = 0.0;
                samplers[slot].is_playing = true;
            }
        }

        void setCrossfader(float value) {
            crossfader = std::clamp(value, 0.0f, 1.0f);
        }
        
        float getCrossfader() const { return crossfader; }
        int getCrossfaderCurve() const { return crossfader_curve; }
        void setCrossfaderCurve(int curve) { crossfader_curve = std::clamp(curve, 0, 2); }

        void setMasterVolume(float vol) {
            master_volume = std::clamp(vol, 0.0f, 2.0f);
        }

        float getMasterVolume() const { return master_volume; }

        void setCueDeckA(bool active) { 
            cue_deck_a = active; 
            DJAudioRouting::getInstance().setDeckCue(0, active);
        }
        void setCueDeckB(bool active) { 
            cue_deck_b = active; 
            DJAudioRouting::getInstance().setDeckCue(1, active);
        }
        void setCueMaster(bool active) { 
            cue_master = active; 
            DJAudioRouting::getInstance().setMasterCue(active);
        }
        bool getCueDeckA() const { return cue_deck_a; }
        bool getCueDeckB() const { return cue_deck_b; }
        bool getCueMaster() const { return cue_master; }

        // Sincronização Profissional Pioneer / Rekordbox / Traktor Beat Sync & Phase Quantization
        void syncBPM(int master_id = 0, int slave_id = 1) {
            if (master_id < 0 || master_id >= 4 || slave_id < 0 || slave_id >= 4) return;
            if (master_id == slave_id) return;
            auto& master = decks[master_id];
            auto& slave = decks[slave_id];
            double natural_bpm = slave.bpm;
            if (natural_bpm <= 10.0 || master.bpm <= 10.0) return;

            double master_effective_bpm = master.bpm * (1.0 + master.pitch_percent * 0.01);
            if (master_effective_bpm <= 10.0) return;

            // Ajusta o pitch fader relativo para bater exatamente o BPM do Deck Master
            float target_pitch = (float)(((master_effective_bpm / natural_bpm) - 1.0) * 100.0);
            while (std::abs(target_pitch) > slave.pitch_range && slave.pitch_range < 50.0f) {
                slave.cyclePitchRange();
            }
            slave.pitch_percent = std::clamp(target_pitch, -slave.pitch_range, slave.pitch_range);
            
            // Beat phase quantization: alinha a fase apenas se o deck escravo já estiver TOCANDO.
            // Se estiver pausado no CUE, mantém a agulha travada na cabeça do CUE para não perder o transiente!
            if (slave.is_playing.load() && master.sample_rate > 1000.0 && slave.sample_rate > 1000.0 && !slave.buffer_l.empty()) {
                double master_beat_len = (60.0 / master.bpm) * master.sample_rate;
                double slave_beat_len = (60.0 / slave.bpm) * slave.sample_rate;
                if (master_beat_len > 1.0 && slave_beat_len > 1.0) {
                    double m_rel = master.current_frame - master.first_beat_frame;
                    double m_phase = std::fmod(m_rel, master_beat_len);
                    if (m_phase < 0.0) m_phase += master_beat_len;
                    double m_phase_norm = m_phase / master_beat_len;

                    double s_rel = slave.current_frame - slave.first_beat_frame;
                    double s_beat_idx = std::round(s_rel / slave_beat_len);
                    slave.current_frame = std::clamp(
                        slave.first_beat_frame + (s_beat_idx + m_phase_norm) * slave_beat_len,
                        0.0, (double)slave.buffer_l.size()
                    );
                }
            }
            KuroUtils::Log("[DJ Engine] SYNC Deck " + std::to_string(slave_id + 1) + " -> " + std::to_string(master_effective_bpm) + " BPM (Quantized)");
        }

        void toggleSync(int slave_id, int fallback_master_id = -1) {
            if (slave_id < 0 || slave_id >= 4) return;

            // Se este deck já é o Master atual:
            if (slave_id == master_deck_id) {
                // No Rekordbox e Traktor, apertar SYNC no Master transfere o Master para o outro deck ativo
                int new_master = -1;
                if (fallback_master_id >= 0 && fallback_master_id < 4 && fallback_master_id != slave_id) {
                    new_master = fallback_master_id;
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (i != slave_id && !decks[i].buffer_l.empty()) {
                            if (decks[i].is_playing) { new_master = i; break; }
                            if (new_master == -1) new_master = i;
                        }
                    }
                }

                if (new_master >= 0) {
                    setMasterDeck(new_master);
                    decks[slave_id].sync_active = true;
                    decks[slave_id].sync_master_deck_id = new_master;
                    syncBPM(new_master, slave_id);
                    KuroUtils::Log("[DJ Engine] Deck " + std::to_string(new_master + 1) + " agora é MASTER. Deck " + std::to_string(slave_id + 1) + " SYNC ON -> Master " + std::to_string(new_master + 1));
                }
                return;
            }

            // Se este deck NÃO é o Master:
            auto& slave = decks[slave_id];
            if (slave.sync_active) {
                slave.sync_active = false;
                slave.sync_master_deck_id = -1;
                slave.sync_phase_diff = 0.0f;
                KuroUtils::Log("[DJ Engine] SYNC OFF Deck " + std::to_string(slave_id + 1));
            } else {
                int m_id = master_deck_id;
                if (m_id == slave_id || m_id < 0 || m_id >= 4) {
                    m_id = (fallback_master_id >= 0 && fallback_master_id != slave_id) ? fallback_master_id : (slave_id == 0 ? 1 : 0);
                    setMasterDeck(m_id);
                }
                slave.sync_active = true;
                slave.sync_master_deck_id = m_id;
                syncBPM(m_id, slave_id);
                KuroUtils::Log("[DJ Engine] SYNC ON Deck " + std::to_string(slave_id + 1) + " -> Master Deck " + std::to_string(m_id + 1));
            }
        }

        void syncBPM() {
            int s_id = (master_deck_id == 0) ? 1 : 0;
            syncBPM(master_deck_id, s_id);
        }

        // Renderiza um bloco de áudio estéreo com DSP da DJM-V10 para qualquer um dos 4 decks
        void renderDeckBlock(DJDeck& deck, std::vector<float>& block_l, std::vector<float>& block_r, unsigned int frames) {
            std::unique_lock<std::mutex> lock(deck.deck_mutex, std::try_to_lock);
            if (!lock.owns_lock()) {
                deck.vu_meter *= 0.85f;
                return;
            }

            deck.updateSoundColorFX();

            // Decaimento de pitch bend
            deck.jog_pitch_bend *= 0.90f;
            if (std::abs(deck.jog_pitch_bend) < 0.0001f) deck.jog_pitch_bend = 0.0f;

            // Simulação física de Turntablism (Spinback e Vinyl Brake)
            float dt_block = (float)frames / (float)host_sample_rate;
            if (DJTurntablismEngine::getInstance().spinback[deck.deck_id].active) {
                deck.current_frame = DJTurntablismEngine::getInstance().updateSpinback(deck.deck_id, deck.current_frame, deck.sample_rate, dt_block);
            }
            if (DJTurntablismEngine::getInstance().vinyl_brake[deck.deck_id].active) {
                float brake_speed = DJTurntablismEngine::getInstance().updateVinylBrake(deck.deck_id, dt_block);
                deck.motor_speed = brake_speed;
                if (brake_speed <= 0.001f) deck.is_playing = false;
            }

            // Scratch física ultra-responsiva e detecção de toque com parada de motor
            // Rampa de desaceleração suave estilo Vinil / Turntable ao dar Pause
            if (deck.is_stopping_vinyl) {
                float decel_rate = 0.007f / std::clamp(deck.vinyl_stop_speed, 0.05f, 2.0f);
                deck.motor_speed = std::max(0.0f, deck.motor_speed - decel_rate);
                if (deck.motor_speed <= 0.002f) {
                    deck.motor_speed = 0.0f;
                    deck.is_stopping_vinyl = false;
                    deck.is_playing = false;
                }
            }

            if (deck.vinyl_mode && deck.jog_touch) {
                deck.motor_stopped = true;
                deck.motor_speed = 0.0f; // Motor stop instantâneo no toque
                deck.jog_momentum_velocity = 0.0f; // Toque cancela inércia imediatamente
                if (deck.scratch_active_ticks > 0) deck.scratch_active_ticks--;
                else deck.target_scratch_velocity = 0.0f;
                deck.scratch_velocity = deck.scratch_velocity * 0.30f + deck.target_scratch_velocity * 0.70f;
                if (std::abs(deck.scratch_velocity) < 0.0005f) deck.scratch_velocity = 0.0f;
            } else if (!deck.is_stopping_vinyl) {
                deck.motor_stopped = false;
                // Rampa suave de inércia do motor ao soltar o vinil (spin-up realista somente se tocando)
                if (deck.motor_speed < 1.0f && deck.is_playing) {
                    deck.motor_speed = std::min(1.0f, deck.motor_speed + 0.065f);
                }
                // Inércia de vinil ao soltar o prato após spinback/scratch
                if (std::abs(deck.scratch_velocity) > 0.015f) {
                    deck.scratch_velocity *= 0.93f; // Desaceleração suave por inércia
                } else {
                    deck.scratch_velocity = 0.0f;
                    deck.target_scratch_velocity = 0.0f;
                }
                // Momentum angular virtual (Simulação de Jogweight / Inércia de CDJ-3000 na DDJ-200)
                if (std::abs(deck.jog_momentum_velocity) > 0.0002f) {
                    deck.jog_visual_angle += deck.jog_momentum_velocity;
                    if (!deck.is_playing) {
                        deck.current_frame = std::clamp(deck.current_frame + (double)deck.jog_momentum_velocity * (deck.sample_rate * 0.08), 0.0, (double)deck.buffer_l.size());
                    } else {
                        deck.jog_pitch_bend = std::clamp(deck.jog_pitch_bend + deck.jog_momentum_velocity * 0.05f, -0.35f, 0.35f);
                    }
                    float friction = (deck.jog_feeling_mode == 0) ? 0.955f : ((deck.jog_feeling_mode == 2) ? 0.65f : 0.86f);
                    deck.jog_momentum_velocity *= friction;
                    if (std::abs(deck.jog_momentum_velocity) < 0.0002f) deck.jog_momentum_velocity = 0.0f;
                }
            }

            bool active = deck.is_playing || deck.jog_touch || (std::abs(deck.scratch_velocity) > 0.005f) || (std::abs(deck.jog_pitch_bend) > 0.0001f) || (std::abs(deck.jog_momentum_velocity) > 0.0002f) || deck.pad_roll_active || deck.censor_active;
            if (!active || deck.buffer_l.empty()) {
                deck.vu_meter *= 0.85f;
                return;
            }

            // Slip Mode: rastreamento de intervenções (Scratch, Loop, Roll, Censor)
            bool slip_intervention = (deck.jog_touch || std::abs(deck.scratch_velocity) > 0.005f || deck.loop_active || deck.pad_roll_active || deck.censor_active);
            if (deck.slip_mode) {
                if (slip_intervention) {
                    if (!deck.is_slipping) {
                        deck.is_slipping = true;
                        deck.slip_virtual_frame = deck.current_frame;
                    }
                } else {
                    if (deck.is_slipping) {
                        deck.is_slipping = false;
                        deck.current_frame = deck.slip_virtual_frame;
                    }
                }
            } else {
                if (!deck.pad_roll_active && !deck.censor_active) {
                    deck.is_slipping = false;
                }
            }

            // Trava contínua de Beat Sync Pioneer (PLL Phase-Locked Loop)
            double pll_pitch_trim = 0.0;
            if (deck.sync_active && !deck.jog_touch && deck.is_playing && deck.deck_id != master_deck_id) {
                int m_id = (master_deck_id >= 0 && master_deck_id < 4 && master_deck_id != deck.deck_id)
                           ? master_deck_id
                           : (deck.sync_master_deck_id >= 0 && deck.sync_master_deck_id < 4 && deck.sync_master_deck_id != deck.deck_id ? deck.sync_master_deck_id : -1);
                if (m_id >= 0) {
                    auto& master = decks[m_id];
                    if (master.bpm > 20.0 && deck.bpm > 20.0) {
                        double master_eff_bpm = master.bpm * (1.0 + master.pitch_percent * 0.01);
                        deck.pitch_percent = (float)(((master_eff_bpm / deck.bpm) - 1.0) * 100.0);

                        if (master.is_playing && master.sample_rate > 1000.0 && deck.sample_rate > 1000.0) {
                            double m_beat_len = (60.0 / master.bpm) * master.sample_rate;
                            double s_beat_len = (60.0 / deck.bpm) * deck.sample_rate;
                            if (m_beat_len > 10.0 && s_beat_len > 10.0) {
                                double m_rel = master.current_frame - master.first_beat_frame;
                                double m_phase = std::fmod(m_rel, m_beat_len);
                                if (m_phase < 0.0) m_phase += m_beat_len;
                                double m_phase_norm = m_phase / m_beat_len;

                                double s_rel = deck.current_frame - deck.first_beat_frame;
                                double s_phase = std::fmod(s_rel, s_beat_len);
                                if (s_phase < 0.0) s_phase += s_beat_len;
                                double s_phase_norm = s_phase / s_beat_len;

                                double phase_diff = s_phase_norm - m_phase_norm;
                                if (phase_diff > 0.5) phase_diff -= 1.0;
                                if (phase_diff < -0.5) phase_diff += 1.0;

                                deck.sync_phase_diff = (float)phase_diff;

                                // PLL (Phase-Locked Loop) suave contínuo Rekordbox/Traktor:
                                // Sem pulos de agulha bruscos durante reprodução
                                pll_pitch_trim = -std::clamp(phase_diff * 0.05, -0.04, 0.04);
                            }
                        }
                    }
                }
            } else {
                deck.sync_phase_diff = 0.0f;
            }

            double rate_ratio = deck.sample_rate / host_sample_rate;
            double speed = 0.0;
            if (deck.vinyl_mode && deck.jog_touch) {
                // Modo Vinyl: motor parado, somente rotação manual das mãos do DJ toca áudio
                speed = rate_ratio * deck.scratch_velocity;
            } else if (std::abs(deck.scratch_velocity) > 0.01f) {
                // Spinback ou inércia de soltura (scratch desacelerando + motor acelerando suavemente)
                speed = rate_ratio * (deck.is_playing ? ((1.0 + deck.pitch_percent * 0.01) * (double)deck.motor_speed) : 0.0) + rate_ratio * deck.scratch_velocity;
            } else if (deck.is_playing) {
                double pitch_fac = (1.0 + (deck.pitch_percent * 0.01)) * (double)deck.motor_speed * (1.0 + pll_pitch_trim);
                speed = (rate_ratio * pitch_fac) + (rate_ratio * deck.jog_pitch_bend);
            } else {
                speed = rate_ratio * deck.jog_pitch_bend * 3.0;
            }

            if (deck.censor_active && !deck.jog_touch) {
                speed = -std::abs(speed);
            }

            // Release FX (Vinyl Brake / Backspin motor stop):
            if (release_fx[deck.deck_id].isActive()) {
                speed *= release_fx[deck.deck_id].getSpeedMultiplier();
            }

            // Vinyl Brake effect
            if (deck.sound_color_fx_mode == 11) { // VINYL BRAKE
                float brake_amt = std::abs(deck.filter_bipolar - 0.5f) * 2.0f;
                if (brake_amt > 0.05f) {
                    deck.vinyl_brake_factor = std::max(0.0f, deck.vinyl_brake_factor - brake_amt * 0.00015f);
                } else {
                    deck.vinyl_brake_factor = std::min(1.0f, deck.vinyl_brake_factor + 0.001f);
                }
                speed *= (double)deck.vinyl_brake_factor;
            } else {
                deck.vinyl_brake_factor = 1.0f;
            }

            float peak_val = 0.0f;
            float cfx_amt = std::abs(deck.filter_bipolar - 0.5f) * 2.0f;

            size_t buf_sz = deck.buffer_l.size();
            for (unsigned int i = 0; i < frames; i++) {
                if (deck.current_frame + speed < 0.0) deck.current_frame = 0.0;
                size_t idx0 = (size_t)deck.current_frame;
                if (idx0 >= buf_sz) {
                    if (deck.is_playing) deck.is_playing = false;
                    break;
                }

                float raw_l = 0.0f, raw_r = 0.0f;
                float frac = (float)(deck.current_frame - (double)idx0);
                size_t idx1 = (idx0 + 1 < buf_sz) ? idx0 + 1 : idx0;

                if (deck.jog_touch || std::abs(deck.scratch_velocity) > 0.01f) {
                    // Interpolação cúbica Hermite de 4 pontos para reprodução contínua e sem artefatos em scratch
                    size_t idx_m1 = (idx0 > 0) ? idx0 - 1 : 0;
                    size_t idx2 = (idx0 + 2 < buf_sz) ? idx0 + 2 : idx1;

                    auto hermite4 = [](float xm1, float x0, float x1, float x2, float t) {
                        float c = (x1 - xm1) * 0.5f;
                        float v = x0 - x1;
                        float w = c + v;
                        float a = w + v + (x2 - x0) * 0.5f;
                        float b_neg = w + a;
                        return ((((a * t) - b_neg) * t + c) * t + x0);
                    };

                    raw_l = hermite4(deck.buffer_l[idx_m1], deck.buffer_l[idx0], deck.buffer_l[idx1], deck.buffer_l[idx2], frac);
                    raw_r = hermite4(deck.buffer_r[idx_m1], deck.buffer_r[idx0], deck.buffer_r[idx1], deck.buffer_r[idx2], frac);
                } else {
                    raw_l = (1.0f - frac) * deck.buffer_l[idx0] + frac * deck.buffer_l[idx1];
                    raw_r = (1.0f - frac) * deck.buffer_r[idx0] + frac * deck.buffer_r[idx1];
                }

                // 3-Band Isolator EQ & Stem Separation em Tempo Real nos 4 Canais
                float lp_l = raw_l, lp_r = raw_r;
                deck.eq_lp.process(lp_l, lp_r);
                float hp_l = raw_l, hp_r = raw_r;
                deck.eq_hp.process(hp_l, hp_r);
                float mid_l = raw_l - lp_l - hp_l;
                float mid_r = raw_r - lp_r - hp_r;

                float s_l = 0.0f, s_r = 0.0f;

                if (deck.stem_mode) {
                    float drums_l = 0.0f, drums_r = 0.0f;
                    float voc_l = 0.0f, voc_r = 0.0f;
                    float inst_l = 0.0f, inst_r = 0.0f;

                    size_t idx1 = (idx0 + 1 < buf_sz) ? idx0 + 1 : idx0;

                    if (deck.has_ai_stems && idx0 < deck.stem_drums_l.size() && idx1 < deck.stem_drums_l.size()) {
                        // Reprodução de Stems com Isolamento Cirúrgico por IA (Studio Grade)
                        drums_l = (1.0f - frac) * deck.stem_drums_l[idx0] + frac * deck.stem_drums_l[idx1];
                        drums_r = (1.0f - frac) * deck.stem_drums_r[idx0] + frac * deck.stem_drums_r[idx1];

                        voc_l = (1.0f - frac) * deck.stem_vocals_l[idx0] + frac * deck.stem_vocals_l[idx1];
                        voc_r = (1.0f - frac) * deck.stem_vocals_r[idx0] + frac * deck.stem_vocals_r[idx1];

                        if (idx0 < deck.stem_inst_l.size() && idx1 < deck.stem_inst_l.size()) {
                            inst_l = (1.0f - frac) * deck.stem_inst_l[idx0] + frac * deck.stem_inst_l[idx1];
                            inst_r = (1.0f - frac) * deck.stem_inst_r[idx0] + frac * deck.stem_inst_r[idx1];
                        } else {
                            inst_l = raw_l - drums_l - voc_l;
                            inst_r = raw_r - drums_r - voc_r;
                        }
                    } else {
                        // Crossover Espectral Mid-Side em Tempo Real (DSP)
                        drums_l = lp_l;
                        drums_r = lp_r;

                        float center_mid = (mid_l + mid_r) * 0.5f;
                        voc_l = center_mid; voc_r = center_mid;
                        deck.vocal_lp.process(voc_l, voc_r);
                        deck.vocal_hp.process(voc_l, voc_r);

                        inst_l = (raw_l - drums_l - voc_l);
                        inst_r = (raw_r - drums_r - voc_r);
                    }

                    float drums_gain = deck.stem_mute_drums ? 0.0f : deck.eq_low;
                    float vocals_gain = deck.stem_mute_vocals ? 0.0f : deck.eq_high;
                    float inst_gain = deck.stem_mute_inst ? 0.0f : deck.eq_mid;

                    s_l = drums_l * drums_gain + inst_l * inst_gain + voc_l * vocals_gain;
                    s_r = drums_r * drums_gain + inst_r * inst_gain + voc_r * vocals_gain;
                } else {
                    // Modo EQ Isolador Pioneer 3-Band Clássico
                    s_l = lp_l * deck.eq_low + mid_l * deck.eq_mid + hp_l * deck.eq_high;
                    s_r = lp_r * deck.eq_low + mid_r * deck.eq_mid + hp_r * deck.eq_high;
                }


                // --- EFEITOS DJM-V10 ---
                if (deck.sound_color_fx_mode == 0) { // FILTER
                    deck.sound_color_filter.process(s_l, s_r);
                } else if (deck.sound_color_fx_mode == 1) { // NOISE
                    if (cfx_amt > 0.05f) {
                        float white_l = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
                        float white_r = (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
                        deck.noise_sweep_filter.process(white_l, white_r);
                        s_l += white_l * cfx_amt * 0.4f;
                        s_r += white_r * cfx_amt * 0.4f;
                    }
                } else if (deck.sound_color_fx_mode == 2) { // CRUSH
                    if (cfx_amt > 0.05f) {
                        float period = 1.0f + cfx_amt * 28.0f;
                        deck.crush_counter += 1.0f;
                        if (deck.crush_counter >= period) {
                            deck.crush_counter = 0.0f;
                            deck.crush_held_l = s_l;
                            deck.crush_held_r = s_r;
                        }
                        s_l = deck.crush_held_l;
                        s_r = deck.crush_held_r;
                    }
                } else if (deck.sound_color_fx_mode == 3) { // SPACE
                    if (cfx_amt > 0.05f) {
                        float sp_l = s_l, sp_r = s_r;
                        deck.sound_color_filter.process(sp_l, sp_r);
                        s_l = s_l * (1.0f - cfx_amt * 0.4f) + sp_l * (cfx_amt * 0.5f);
                        s_r = s_r * (1.0f - cfx_amt * 0.4f) + sp_r * (cfx_amt * 0.5f);
                    }
                } else if (deck.sound_color_fx_mode == 4) { // DUB ECHO
                    if (cfx_amt > 0.05f) {
                        int del_len = (int)((60.0 / std::clamp(deck.bpm, 40.0, 220.0)) * 0.5 * deck.sample_rate);
                        if (del_len < 64) del_len = 64;
                        if (del_len > 95000) del_len = 95000;
                        int r_pos = (deck.dub_echo_pos - del_len + 96000) % 96000;
                        float echo_l = deck.dub_echo_buf_l[r_pos];
                        float echo_r = deck.dub_echo_buf_r[r_pos];
                        deck.dub_echo_lp.process(echo_l, echo_r);

                        float fb = 0.55f + cfx_amt * 0.35f;
                        deck.dub_echo_buf_l[deck.dub_echo_pos] = s_l + echo_l * fb;
                        deck.dub_echo_buf_r[deck.dub_echo_pos] = s_r + echo_r * fb;
                        deck.dub_echo_pos = (deck.dub_echo_pos + 1) % 96000;

                        s_l += echo_l * cfx_amt * 0.7f;
                        s_r += echo_r * cfx_amt * 0.7f;
                    }
                } else if (deck.sound_color_fx_mode == 5) { // PITCH
                    float p_amt = (deck.filter_bipolar - 0.5f) * 2.0f;
                    if (std::abs(p_amt) > 0.05f) {
                        float semitones = p_amt * 12.0f;
                        float rate = std::pow(2.0f, semitones / 12.0f);
                        int grain_sz = 2048;
                        deck.pitch_ring_l[deck.pitch_ring_pos] = s_l;
                        deck.pitch_ring_r[deck.pitch_ring_pos] = s_r;

                        float off1 = deck.pitch_phase1 * (float)grain_sz;
                        float off2 = deck.pitch_phase2 * (float)grain_sz;
                        int p1 = (deck.pitch_ring_pos - (int)off1 + 4096) % 4096;
                        int p2 = (deck.pitch_ring_pos - (int)off2 + 4096) % 4096;

                        float w1 = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * deck.pitch_phase1));
                        float w2 = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * deck.pitch_phase2));

                        float ps_l = deck.pitch_ring_l[p1] * w1 + deck.pitch_ring_l[p2] * w2;
                        float ps_r = deck.pitch_ring_r[p1] * w1 + deck.pitch_ring_r[p2] * w2;

                        deck.pitch_phase1 += (1.0f - rate) / (float)grain_sz;
                        if (deck.pitch_phase1 >= 1.0f) deck.pitch_phase1 -= 1.0f;
                        if (deck.pitch_phase1 < 0.0f) deck.pitch_phase1 += 1.0f;

                        deck.pitch_phase2 += (1.0f - rate) / (float)grain_sz;
                        if (deck.pitch_phase2 >= 1.0f) deck.pitch_phase2 -= 1.0f;
                        if (deck.pitch_phase2 < 0.0f) deck.pitch_phase2 += 1.0f;

                        deck.pitch_ring_pos = (deck.pitch_ring_pos + 1) % 4096;
                        float mix = std::min(1.0f, std::abs(p_amt) * 1.5f);
                        s_l = s_l * (1.0f - mix) + ps_l * mix;
                        s_r = s_r * (1.0f - mix) + ps_r * mix;
                    }
                } else if (deck.sound_color_fx_mode == 6) { // PING PONG
                    if (cfx_amt > 0.05f) {
                        int del_len = (int)((60.0 / std::clamp(deck.bpm, 40.0, 220.0)) * 0.75 * deck.sample_rate);
                        if (del_len < 64) del_len = 64;
                        if (del_len > 95000) del_len = 95000;
                        int r_pos = (deck.ping_pong_pos - del_len + 96000) % 96000;

                        float echo_l = deck.ping_pong_buf_r[r_pos];
                        float echo_r = deck.ping_pong_buf_l[r_pos];

                        float fb = 0.60f + cfx_amt * 0.30f;
                        deck.ping_pong_buf_l[deck.ping_pong_pos] = s_l + echo_l * fb;
                        deck.ping_pong_buf_r[deck.ping_pong_pos] = s_r + echo_r * fb;
                        deck.ping_pong_pos = (deck.ping_pong_pos + 1) % 96000;

                        s_l += echo_l * cfx_amt * 0.8f;
                        s_r += echo_r * cfx_amt * 0.8f;
                    }
                } else if (deck.sound_color_fx_mode == 7) { // SPIRAL
                    if (cfx_amt > 0.05f) {
                        int del_len = (int)((60.0 / std::clamp(deck.bpm, 40.0, 220.0)) * 0.5 * deck.sample_rate);
                        if (del_len < 64) del_len = 64;
                        if (del_len > 95000) del_len = 95000;
                        int r_pos = (deck.spiral_pos - del_len + 96000) % 96000;

                        float shift = (deck.filter_bipolar > 0.5f) ? 1.03f : 0.97f;
                        float echo_l = deck.spiral_buf_l[r_pos] * shift;
                        float echo_r = deck.spiral_buf_r[r_pos] * shift;

                        float fb = 0.65f;
                        deck.spiral_buf_l[deck.spiral_pos] = s_l + echo_l * fb;
                        deck.spiral_buf_r[deck.spiral_pos] = s_r + echo_r * fb;
                        deck.spiral_pos = (deck.spiral_pos + 1) % 96000;

                        s_l += echo_l * cfx_amt * 0.75f;
                        s_r += echo_r * cfx_amt * 0.75f;
                    }
                } else if (deck.sound_color_fx_mode == 8) { // REVERB & SHIMMER
                    if (cfx_amt > 0.05f) {
                        int del1 = (int)(deck.sample_rate * 0.045);
                        int del2 = (int)(deck.sample_rate * 0.075);
                        int r1 = (deck.dub_echo_pos - del1 + 96000) % 96000;
                        int r2 = (deck.dub_echo_pos - del2 + 96000) % 96000;

                        float rev_l = (deck.dub_echo_buf_l[r1] + deck.dub_echo_buf_r[r2]) * 0.5f;
                        float rev_r = (deck.dub_echo_buf_r[r1] + deck.dub_echo_buf_l[r2]) * 0.5f;

                        deck.dub_echo_buf_l[deck.dub_echo_pos] = s_l + rev_l * 0.65f;
                        deck.dub_echo_buf_r[deck.dub_echo_pos] = s_r + rev_r * 0.65f;
                        deck.dub_echo_pos = (deck.dub_echo_pos + 1) % 96000;

                        s_l += rev_l * cfx_amt * 0.8f;
                        s_r += rev_r * cfx_amt * 0.8f;
                    }
                } else if (deck.sound_color_fx_mode == 9 || deck.sound_color_fx_mode == 10) { // SLIP ROLL / ROLL
                    if (cfx_amt > 0.08f) {
                        if (!deck.roll_capturing) {
                            deck.roll_capturing = true;
                            deck.roll_len_frames = (int)((60.0 / std::max(20.0, deck.bpm)) * 0.5 * deck.sample_rate);
                            if (deck.roll_len_frames < 256) deck.roll_len_frames = 256;
                            if (deck.roll_len_frames > 48000) deck.roll_len_frames = 48000;
                            deck.roll_buf_pos = 0;
                            deck.slip_virtual_frame = deck.current_frame;
                        }
                        deck.roll_buf_l[deck.roll_buf_pos] = s_l;
                        deck.roll_buf_r[deck.roll_buf_pos] = s_r;
                        
                        int read_pos = deck.roll_buf_pos % deck.roll_len_frames;
                        s_l = deck.roll_buf_l[read_pos];
                        s_r = deck.roll_buf_r[read_pos];
                        deck.roll_buf_pos++;
                    } else if (deck.roll_capturing) {
                        deck.roll_capturing = false;
                        if (deck.sound_color_fx_mode == 9) {
                            deck.current_frame = deck.slip_virtual_frame;
                        }
                    }
                } else if (deck.sound_color_fx_mode == 12) { // HELIX
                    if (cfx_amt > 0.05f) {
                        float helix_mod = std::sin((float)deck.current_frame * 0.0008f) * 0.5f + 0.5f;
                        int del_len = (int)(deck.sample_rate * (0.005 + helix_mod * 0.025));
                        int r_pos = (deck.dub_echo_pos - del_len + 96000) % 96000;

                        float flg_l = deck.dub_echo_buf_l[r_pos];
                        float flg_r = deck.dub_echo_buf_r[r_pos];
                        deck.dub_echo_buf_l[deck.dub_echo_pos] = s_l + flg_l * 0.7f;
                        deck.dub_echo_buf_r[deck.dub_echo_pos] = s_r + flg_r * 0.7f;
                        deck.dub_echo_pos = (deck.dub_echo_pos + 1) % 96000;

                        s_l += flg_l * cfx_amt * 0.7f;
                        s_r += flg_r * cfx_amt * 0.7f;
                    }
                }

                // --- LIVE STEMS SEPARATION DSP & HD AI STEMS (VOCAL, MELODY, BASS, DRUMS) ---
                float g_voc = deck.stem_vocal_gain.load();
                float g_mel = deck.stem_melody_gain.load();
                float g_bas = deck.stem_bass_gain.load();
                float g_drm = deck.stem_drum_gain.load();

                if (deck.has_ai_stems && idx0 < deck.stem_drums_l.size() && idx1 < deck.stem_drums_l.size()) {
                    // Reprodução com isolamento cirúrgico de estúdio por IA (HD Studio 4-Stems)
                    float d_l = (1.0f - frac) * deck.stem_drums_l[idx0] + frac * deck.stem_drums_l[idx1];
                    float d_r = (1.0f - frac) * deck.stem_drums_r[idx0] + frac * deck.stem_drums_r[idx1];

                    float b_l = 0.0f, b_r = 0.0f;
                    if (idx0 < deck.stem_bass_l.size() && idx1 < deck.stem_bass_l.size()) {
                        b_l = (1.0f - frac) * deck.stem_bass_l[idx0] + frac * deck.stem_bass_l[idx1];
                        b_r = (1.0f - frac) * deck.stem_bass_r[idx0] + frac * deck.stem_bass_r[idx1];
                    }

                    float v_l = (1.0f - frac) * deck.stem_vocals_l[idx0] + frac * deck.stem_vocals_l[idx1];
                    float v_r = (1.0f - frac) * deck.stem_vocals_r[idx0] + frac * deck.stem_vocals_r[idx1];

                    float m_l = 0.0f, m_r = 0.0f;
                    if (idx0 < deck.stem_inst_l.size() && idx1 < deck.stem_inst_l.size()) {
                        m_l = (1.0f - frac) * deck.stem_inst_l[idx0] + frac * deck.stem_inst_l[idx1];
                        m_r = (1.0f - frac) * deck.stem_inst_r[idx0] + frac * deck.stem_inst_r[idx1];
                    } else {
                        m_l = s_l - d_l - b_l - v_l;
                        m_r = s_r - d_r - b_r - v_r;
                    }

                    s_l = d_l * g_drm + b_l * g_bas + m_l * g_mel + v_l * g_voc;
                    s_r = d_r * g_drm + b_r * g_bas + m_r * g_mel + v_r * g_voc;
                } else if (g_voc < 0.999f || g_mel < 0.999f || g_bas < 0.999f || g_drm < 0.999f) {
                    // Crossover Low (<220 Hz): Kick + Sub-Bass
                    float alpha_low = 0.031f; // ~220Hz
                    deck.stem_lp_l += alpha_low * (s_l - deck.stem_lp_l);
                    deck.stem_lp_r += alpha_low * (s_r - deck.stem_lp_r);
                    float low_l = deck.stem_lp_l;
                    float low_r = deck.stem_lp_r;

                    // Crossover High (>3800 Hz): Hihats + Cymbals
                    float alpha_high = 0.42f; // ~3800Hz
                    deck.stem_hp_l += alpha_high * (s_l - deck.stem_hp_l);
                    deck.stem_hp_r += alpha_high * (s_r - deck.stem_hp_r);
                    float high_l = s_l - deck.stem_hp_l;
                    float high_r = s_r - deck.stem_hp_r;

                    // Mid-Band: 220Hz a 3800Hz
                    float mid_l = s_l - low_l - high_l;
                    float mid_r = s_r - low_r - high_r;

                    // Center / Mid Extraction para Voz (Phantom Center)
                    float mid_center = (mid_l + mid_r) * 0.5f;
                    float mid_side_l = (mid_l - mid_r) * 0.5f;
                    float mid_side_r = (mid_r - mid_l) * 0.5f;

                    // Formante vocal
                    float voc_l = mid_center * 0.9f;
                    float voc_r = mid_center * 0.9f;

                    // Melodia / Sintetizadores: ambiência lateral + corpo restante
                    float mel_l = mid_side_l + mid_center * 0.1f;
                    float mel_r = mid_side_r + mid_center * 0.1f;

                    // Separação de Grave (Sub-Bass) vs Ataque/Punch de Bateria
                    float bas_l = low_l * 0.75f;
                    float bas_r = low_r * 0.75f;
                    float drm_l = (low_l * 0.25f) + high_l;
                    float drm_r = (low_r * 0.25f) + high_r;

                    // Reconstituição balanceada com ganhos de stems
                    s_l = drm_l * g_drm + bas_l * g_bas + voc_l * g_voc + mel_l * g_mel;
                    s_r = drm_r * g_drm + bas_r * g_bas + voc_r * g_voc + mel_r * g_mel;
                }

                // Key Shift (Harmonic Transposition) por Semitons em Tempo Real (Pioneer CDJ-3000 / Rekordbox)
                // Atua de forma cirúrgica sobre toda a saída do Deck (áudio estéreo normal ou AI Stems)
                if (deck.key_shift_semitones != 0) {
                    float p_rate = std::pow(2.0f, (float)deck.key_shift_semitones / 12.0f);
                    int grain_sz = 2048;
                    deck.keyshift_ring_l[deck.keyshift_ring_pos] = s_l;
                    deck.keyshift_ring_r[deck.keyshift_ring_pos] = s_r;

                    float off1 = deck.keyshift_phase1 * (float)grain_sz;
                    float off2 = deck.keyshift_phase2 * (float)grain_sz;
                    int p1 = (deck.keyshift_ring_pos - (int)off1 + 4096) % 4096;
                    int p2 = (deck.keyshift_ring_pos - (int)off2 + 4096) % 4096;

                    // Hann window crossfade (equal-power)
                    float w1 = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * deck.keyshift_phase1));
                    float w2 = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * deck.keyshift_phase2));

                    s_l = deck.keyshift_ring_l[p1] * w1 + deck.keyshift_ring_l[p2] * w2;
                    s_r = deck.keyshift_ring_r[p1] * w1 + deck.keyshift_ring_r[p2] * w2;

                    float phase_inc = (1.0f - p_rate) / (float)grain_sz;
                    deck.keyshift_phase1 += phase_inc;
                    if (deck.keyshift_phase1 >= 1.0f) deck.keyshift_phase1 -= 1.0f;
                    else if (deck.keyshift_phase1 < 0.0f) deck.keyshift_phase1 += 1.0f;

                    deck.keyshift_phase2 += phase_inc;
                    if (deck.keyshift_phase2 >= 1.0f) deck.keyshift_phase2 -= 1.0f;
                    else if (deck.keyshift_phase2 < 0.0f) deck.keyshift_phase2 += 1.0f;

                    deck.keyshift_ring_pos = (deck.keyshift_ring_pos + 1) % 4096;
                }

                float pre_fader_gain = deck.gain * (deck.auto_gain_enabled ? deck.auto_gain_factor : 1.0f);
                block_l[i] = s_l * pre_fader_gain;
                block_r[i] = s_r * pre_fader_gain;

                float p_samp = std::max(std::abs(block_l[i]), std::abs(block_r[i]));
                if (p_samp > peak_val) peak_val = p_samp;

                deck.current_frame += speed;
                // Calibração de rotação visual em 33 1/3 RPM natural de vinil (uma volta a cada 1.8s):
                deck.jog_visual_angle += (float)(speed * (2.0 * M_PI * 33.33333 / (std::max(1000.0, deck.sample_rate) * 60.0)));
                
                // Slip Mode: avanço contínuo da agulha fantasma em silêncio
                if (deck.is_slipping) {
                    double nat_fwd = rate_ratio * (deck.is_playing ? ((1.0 + (double)deck.pitch_percent * 0.01) * (double)deck.motor_speed) : 0.0);
                    deck.slip_virtual_frame += nat_fwd;
                    if (deck.slip_virtual_frame >= (double)buf_sz) {
                        deck.slip_virtual_frame = std::fmod(deck.slip_virtual_frame, (double)buf_sz);
                    }
                }

                // Beat Loop Roll: se ativo, faz loop no pedaço do roll
                if (deck.pad_roll_active && deck.pad_roll_len_frames > 0) {
                    if (deck.current_frame >= deck.pad_roll_start_frame + deck.pad_roll_len_frames) {
                        deck.current_frame = deck.pad_roll_start_frame;
                    }
                }

                if (deck.roll_capturing && deck.sound_color_fx_mode == 9) {
                    deck.slip_virtual_frame += speed;
                }

                // Active Loop Trigger Automático (Padrão Pioneer CDJ-3000 / Rekordbox)
                if (deck.has_active_loop && !deck.loop_active && deck.is_playing) {
                    if (deck.current_frame >= deck.active_loop_start_frame && 
                        deck.current_frame < deck.active_loop_start_frame + deck.active_loop_len_frames) {
                        deck.loop_start_frame = deck.active_loop_start_frame;
                        deck.loop_length_frames = deck.active_loop_len_frames;
                        deck.loop_active = true;
                    }
                }

                if (deck.loop_active && deck.loop_length_frames > 0) {
                    if (deck.current_frame >= deck.loop_start_frame + deck.loop_length_frames) {
                        deck.current_frame = deck.loop_start_frame;
                    }
                }

                // Emergency Loop Failsafe (Pioneer CDJ-3000): ativa loop de 4 batidas ao chegar no fim
                if (deck.is_playing && !deck.loop_active && buf_sz > 1000 && deck.current_frame >= (double)(buf_sz - 500)) {
                    KuroAudio::DJLiveSafety::getInstance().triggerEmergencyLoop(deck, 4.0);
                }
            }

            // Compressor de Canal Pioneer DJM-V10 / DJM-A9
            deck.compressor.amount = deck.comp;
            deck.compressor.processBlock(block_l.data(), block_r.data(), frames, host_sample_rate);

            // Recalcula o pico pós-compressor para o VU Meter do canal refletir a dinâmica real
            peak_val = 0.0f;
            for (unsigned int i = 0; i < frames; i++) {
                float p_samp = std::max(std::abs(block_l[i]), std::abs(block_r[i]));
                if (p_samp > peak_val) peak_val = p_samp;
            }

            deck.vu_meter = deck.vu_meter * 0.8f + peak_val * 0.2f;
        }

        // Processamento DSP em tempo real conectado ao audioCallback WASAPI (4 Canais / Roteamento Fone CUE)
        void process(float* out_l, float* out_r, float* out_cue_l, float* out_cue_r, unsigned int frames) {
            std::vector<float> blk0_l(frames, 0.0f), blk0_r(frames, 0.0f);
            std::vector<float> blk1_l(frames, 0.0f), blk1_r(frames, 0.0f);
            std::vector<float> blk2_l(frames, 0.0f), blk2_r(frames, 0.0f);
            std::vector<float> blk3_l(frames, 0.0f), blk3_r(frames, 0.0f);

            renderDeckBlock(decks[0], blk0_l, blk0_r, frames);
            renderDeckBlock(decks[1], blk1_l, blk1_r, frames);
            renderDeckBlock(decks[2], blk2_l, blk2_r, frames);
            renderDeckBlock(decks[3], blk3_l, blk3_r, frames);

            // Processamento Pioneer DJM-V10 Channel DSP (4-Band EQ + Comp) & Release FX por deck
            for (int d = 0; d < 4; d++) {
                auto* blk_l = (d == 0 ? blk0_l.data() : (d == 1 ? blk1_l.data() : (d == 2 ? blk2_l.data() : blk3_l.data())));
                auto* blk_r = (d == 0 ? blk0_r.data() : (d == 1 ? blk1_r.data() : (d == 2 ? blk2_r.data() : blk3_r.data())));
                if (v10_channel_dsp[d].enabled) {
                    v10_channel_dsp[d].process(blk_l, blk_r, frames, host_sample_rate);
                }
                if (release_fx[d].isActive()) {
                    release_fx[d].process(blk_l, blk_r, frames, host_sample_rate);
                }

                // Processamento Mixer Híbrido (Xone:96 4-Band EQ, VCF Resonance & DJM-V10 Comp)
                for (unsigned int f = 0; f < frames; f++) {
                    DJHybridMixerDSP::getInstance().processChannel(d, blk_l[f], blk_r[f], (float)host_sample_rate);
                }
            }

            // Pre-escala de Volume do Canal: Transforma Beat FX em 100% POST-FADER!
            // Quando o fader de volume e puxado para zero, o audio da musica cessa,
            // mas a cauda de reverb/eco dos efeitos continua soando naturalmente sem corte abrupto!
            float vol0 = decks[0].volume;
            float vol1 = decks[1].volume;
            float vol2 = decks[2].volume;
            float vol3 = decks[3].volume;

            for (unsigned int f = 0; f < frames; f++) {
                blk0_l[f] *= vol0; blk0_r[f] *= vol0;
                blk1_l[f] *= vol1; blk1_r[f] *= vol1;
                blk2_l[f] *= vol2; blk2_r[f] *= vol2;
                blk3_l[f] *= vol3; blk3_r[f] *= vol3;
            }

            // Channel assignment para Pioneer Beat FX (FX 1 e FX 2) - POST-FADER SPILLOVER
            auto applyBeatFXToChannel = [&](DJBeatFXUnit& fx, int ch_id, float* l, float* r, double ch_bpm) {
                if (fx.channel_assign == ch_id) {
                    fx.processBlock(l, r, frames, ch_bpm, host_sample_rate);
                }
            };

            applyBeatFXToChannel(beat_fx, 0, blk0_l.data(), blk0_r.data(), decks[0].bpm);
            applyBeatFXToChannel(beat_fx, 1, blk1_l.data(), blk1_r.data(), decks[1].bpm);
            applyBeatFXToChannel(beat_fx, 3, blk2_l.data(), blk2_r.data(), decks[2].bpm);
            applyBeatFXToChannel(beat_fx, 4, blk3_l.data(), blk3_r.data(), decks[3].bpm);

            applyBeatFXToChannel(beat_fx2, 0, blk0_l.data(), blk0_r.data(), decks[0].bpm);
            applyBeatFXToChannel(beat_fx2, 1, blk1_l.data(), blk1_r.data(), decks[1].bpm);
            applyBeatFXToChannel(beat_fx2, 3, blk2_l.data(), blk2_r.data(), decks[2].bpm);
            applyBeatFXToChannel(beat_fx2, 4, blk3_l.data(), blk3_r.data(), decks[3].bpm);

            // Crossfader e Soma de 4 Decks (Lado A = Deck 1 e 3; Lado B = Deck 2 e 4)
            float gain_a = 1.0f;
            float gain_b = 1.0f;
            if (crossfader_curve == 1) { // 1 = SHARP / SCRATCH
                gain_a = (crossfader >= 0.92f) ? (1.0f - crossfader) / 0.08f : 1.0f;
                gain_b = (crossfader <= 0.08f) ? crossfader / 0.08f : 1.0f;
            } else if (crossfader_curve == 2) { // 2 = LINEAR
                gain_a = 1.0f - crossfader;
                gain_b = crossfader;
            } else { // 0 = SMOOTH EQUAL POWER
                float cf_rad = crossfader * (float)(M_PI * 0.5);
                gain_a = std::cos(cf_rad);
                gain_b = std::sin(cf_rad);
            }

            std::vector<float> master_l(frames, 0.0f);
            std::vector<float> master_r(frames, 0.0f);

            for (unsigned int i = 0; i < frames; i++) {
                float side_a_l = blk0_l[i] + blk2_l[i];
                float side_a_r = blk0_r[i] + blk2_r[i];
                float side_b_l = blk1_l[i] + blk3_l[i];
                float side_b_r = blk1_r[i] + blk3_r[i];

                float smp_l = 0.0f, smp_r = 0.0f;
                for (int s = 0; s < 16; s++) {
                    if (samplers[s].is_playing) {
                        size_t pos = (size_t)samplers[s].playback_pos;
                        if (pos < samplers[s].buffer_l.size()) {
                            smp_l += samplers[s].buffer_l[pos] * samplers[s].volume;
                            smp_r += samplers[s].buffer_r[pos] * samplers[s].volume;
                            samplers[s].playback_pos += 1.0;
                        } else {
                            samplers[s].is_playing = false;
                        }
                    }
                }

                master_l[i] = (side_a_l * gain_a + side_b_l * gain_b + smp_l);
                master_r[i] = (side_a_r * gain_a + side_b_r * gain_b + smp_r);
            }

            // Injeção de Live Instruments (Sintetizador Analógico 303 + Drum Machine 16 Pads)
            DJLiveInstrumentEngine::getInstance().processLiveBlock(master_l.data(), master_r.data(), frames, (float)host_sample_rate);

            // Aplica Pioneer Beat FX no Master se selecionado (Canal 5 ou 2)
            double eff_bpm = decks[0].is_playing ? decks[0].bpm : (decks[1].is_playing ? decks[1].bpm : 126.0);
            if (beat_fx.channel_assign == 5 || beat_fx.channel_assign == 2) {
                beat_fx.processBlock(master_l.data(), master_r.data(), frames, eff_bpm, host_sample_rate);
            }
            if (beat_fx2.channel_assign == 5 || beat_fx2.channel_assign == 2) {
                beat_fx2.processBlock(master_l.data(), master_r.data(), frames, eff_bpm, host_sample_rate);
            }

            // --- MASTER ISOLATOR 3-BAND LINKWITZ-RILEY (-inf a +9dB com saturação analógica) ---
            master_isolator.process(master_l.data(), master_r.data(), frames, host_sample_rate);

            // --- MASTER LIMITER BRICKWALL & AUTO-GAIN ESTILO PIONEER DJM-V10 ---
            master_limiter.processBlock(master_l.data(), master_r.data(), frames, host_sample_rate);

            for (unsigned int i = 0; i < frames; i++) {
                master_l[i] *= master_volume;
                master_r[i] *= master_volume;
            }

            // Análise de Gain Staging (LUFS & True Peak) e Transições de Emergência (Echo Out)
            DJGainStagingAnalyzer::getInstance().processMaster(master_l.data(), master_r.data(), frames, (float)host_sample_rate);

            // Gravação Master ao vivo [● REC] (Lossless WAV 32-bit Float)
            if (set_recorder.is_recording) {
                set_recorder.writeBlock(master_l.data(), master_r.data(), frames, 1.0f);
            }

            // --- SISTEMA DE ROTEAMENTO DE ÁUDIO & MONITORAMENTO DE FONE (CUE / PFL) ---
            const float* const deck_blocks[4][2] = {
                { blk0_l.data(), blk0_r.data() },
                { blk1_l.data(), blk1_r.data() },
                { blk2_l.data(), blk2_r.data() },
                { blk3_l.data(), blk3_r.data() }
            };

            std::vector<float> routed_m_l(frames, 0.0f), routed_m_r(frames, 0.0f);
            std::vector<float> routed_cue_l(frames, 0.0f), routed_cue_r(frames, 0.0f);

            DJAudioRouting::getInstance().process(
                deck_blocks,
                master_l.data(), master_r.data(),
                routed_m_l.data(), routed_m_r.data(),
                routed_cue_l.data(), routed_cue_r.data(),
                frames
            );

            // Pré-escuta de Preview Player (Biblioteca / Abduction Hub) no FONE e MASTER
            if (DJPreviewPlayer::getInstance().isPlaying()) {
                std::vector<float> prev_blk_l(frames, 0.0f), prev_blk_r(frames, 0.0f);
                DJPreviewPlayer::getInstance().processBlock(prev_blk_l.data(), prev_blk_r.data(), frames, host_sample_rate);
                for (unsigned int i = 0; i < frames; i++) {
                    routed_cue_l[i] += prev_blk_l[i];
                    routed_cue_r[i] += prev_blk_r[i];
                    routed_m_l[i]   += prev_blk_l[i];
                    routed_m_r[i]   += prev_blk_r[i];
                }
            }
            if (DJTouchPreviewCue::getInstance().isTouchPreviewActive() || DJTouchPreviewCue::getInstance().isTouchCueActive()) {
                DJTouchPreviewCue::getInstance().renderToHeadphones(routed_cue_l.data(), routed_cue_r.data(), frames, host_sample_rate, decks);
                if (DJTouchPreviewCue::getInstance().isRouteToMaster()) {
                    for (unsigned int i = 0; i < frames; i++) {
                        routed_m_l[i] += routed_cue_l[i];
                        routed_m_r[i] += routed_cue_r[i];
                    }
                }
            }

            // Acoustic Metronome para alinhamento de Beatgrid
            for (int d = 0; d < 4; d++) {
                if (DJGridEngine::getInstance().isMetronomeEnabled(d)) {
                    DJGridEngine::getInstance().processMetronome(d, decks[d].current_frame, routed_cue_l.data(), routed_cue_r.data(), frames, host_sample_rate);
                }
            }

            // Injeção nos buffers de saída físicos:
            if (out_cue_l != nullptr && out_cue_r != nullptr) {
                // Modo 4-Canais Independente (Hardware com 4 saídas)
                for (unsigned int i = 0; i < frames; i++) {
                    out_l[i] += routed_m_l[i];
                    out_r[i] += routed_m_r[i];
                    out_cue_l[i] += routed_cue_l[i];
                    out_cue_r[i] += routed_cue_r[i];
                }
            } else {
                // Modo 2-Canais Físicos (Split Cue ou Stereo)
                auto mode = DJAudioRouting::getInstance().getChannelMode();
                if (mode == DJOutputChannelMode::SPLIT_CUE) {
                    for (unsigned int i = 0; i < frames; i++) {
                        out_l[i] += (routed_m_l[i] + routed_m_r[i]) * 0.5f;     // Esquerda: Master Mono
                        out_r[i] += (routed_cue_l[i] + routed_cue_r[i]) * 0.5f; // Direita: Fone Cue Mono
                    }
                } else if (mode == DJOutputChannelMode::STEREO_COMBINED) {
                    for (unsigned int i = 0; i < frames; i++) {
                        out_l[i] += routed_m_l[i] + routed_cue_l[i] * 0.5f;
                        out_r[i] += routed_m_r[i] + routed_cue_r[i] * 0.5f;
                    }
                } else {
                    for (unsigned int i = 0; i < frames; i++) {
                        out_l[i] += routed_m_l[i] + routed_cue_l[i] * 0.5f;
                        out_r[i] += routed_m_r[i] + routed_cue_r[i] * 0.5f;
                    }
                }
            }
        }

        void process(float* out_l, float* out_r, unsigned int frames) {
            process(out_l, out_r, nullptr, nullptr, frames);
        }

        bool saveHotCues(const std::string& filepath = "") {
            try {
                std::filesystem::path p = filepath.empty() ? (getTracksDirectory() / "hot_cues_db.json") : std::filesystem::u8path(filepath);
                std::filesystem::create_directories(p.parent_path());
                std::ofstream f(p);
                if (!f.is_open()) return false;
                f << "{\n  \"decks\": [\n";
                for (int d = 0; d < 4; d++) {
                    f << "    {\n";
                    f << "      \"deck_id\": " << d << ",\n";
                    f << "      \"track_path\": \"" << decks[d].track_path << "\",\n";
                    f << "      \"cues\": [\n";
                    for (int c = 0; c < 8; c++) {
                        f << "        { \"id\": " << c 
                          << ", \"active\": " << (decks[d].hot_cues[c].active ? "true" : "false")
                          << ", \"time_sec\": " << decks[d].hot_cues[c].time_sec 
                          << ", \"label\": \"" << decks[d].hot_cues[c].label << "\""
                          << ", \"color\": " << decks[d].hot_cues[c].color_rgba << " }"
                          << (c < 7 ? "," : "") << "\n";
                    }
                    f << "      ]\n    }" << (d < 3 ? "," : "") << "\n";
                }
                f << "  ]\n}\n";
                return true;
            } catch (...) { return false; }
        }

        bool loadHotCues(const std::string& filepath = "") {
            try {
                std::filesystem::path p = filepath.empty() ? (getTracksDirectory() / "hot_cues_db.json") : std::filesystem::u8path(filepath);
                if (!std::filesystem::exists(p)) return false;
                std::ifstream f(p);
                if (!f.is_open()) return false;
                std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                for (int d = 0; d < 4; d++) {
                    std::string d_tag = "\"deck_id\": " + std::to_string(d);
                    size_t d_pos = content.find(d_tag);
                    if (d_pos == std::string::npos) continue;
                    for (int c = 0; c < 8; c++) {
                        std::string c_tag = "\"id\": " + std::to_string(c);
                        size_t c_pos = content.find(c_tag, d_pos);
                        if (c_pos == std::string::npos) continue;
                        size_t act_pos = content.find("\"active\":", c_pos);
                        if (act_pos != std::string::npos && act_pos < c_pos + 60) {
                            bool act = (content.find("true", act_pos) != std::string::npos && content.find("true", act_pos) < act_pos + 12);
                            decks[d].hot_cues[c].active = act;
                        }
                        size_t t_pos = content.find("\"time_sec\":", c_pos);
                        if (t_pos != std::string::npos && t_pos < c_pos + 90) {
                            try {
                                decks[d].hot_cues[c].time_sec = std::stod(content.substr(t_pos + 11));
                            } catch (...) {}
                        }
                    }
                }
                return true;
            } catch (...) { return false; }
        }

        // =========================================================================
        // HISTÓRICO DE SESSÃO & EXPORTAÇÃO DE SETLIST (1001Tracklists / M3U)
        // =========================================================================
        struct DJSetlistEntry {
            std::string title;
            std::string artist;
            std::string start_time;
            double bpm = 0.0;
            std::string key;
        };
        std::vector<DJSetlistEntry> setlist_history;

        void logTrackPlayed(const std::string& title, const std::string& artist, double bpm, const std::string& key) {
            if (title.empty() || title == "Nenhuma Faixa Carregada") return;
            if (!setlist_history.empty() && setlist_history.back().title == title) return;
            
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            char time_buf[32];
            std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", std::localtime(&in_time_t));

            DJSetlistEntry entry;
            entry.title = title;
            entry.artist = artist;
            entry.start_time = time_buf;
            entry.bpm = bpm;
            entry.key = key;
            setlist_history.push_back(entry);
        }

        std::string exportSetlistTxt() const {
            std::stringstream ss;
            ss << "=== ABDUCTION STUDIO DJ - LIVE SETLIST ===\n";
            ss << "Total de Faixas Tocadas: " << setlist_history.size() << "\n\n";
            int idx = 1;
            for (const auto& t : setlist_history) {
                ss << "[" << t.start_time << "] #" << idx++ << " " << t.artist << " - " << t.title 
                   << " (" << std::fixed << std::setprecision(1) << t.bpm << " BPM | " << t.key << ")\n";
            }
            return ss.str();
        }

        std::string exportSetlistM3u() const {
            std::stringstream ss;
            ss << "#EXTM3U\n";
            ss << "#PLAYLIST: Abduction Studio Live DJ Set\n";
            for (const auto& t : setlist_history) {
                ss << "#EXTINF:-1," << t.artist << " - " << t.title << "\n";
                ss << t.artist << " - " << t.title << ".mp3\n";
            }
            return ss.str();
        }
    };

} // namespace KuroAudio
