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
#include "../ai/dr_wav.h"
#include "../utils/Logger.h"

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

    // Unidade de Efeitos Pioneer Beat FX (Sincronizada ao BPM)
    // Unidade de Efeitos Pioneer Beat FX (Sincronizada ao BPM)
    class DJBeatFXUnit {
    public:
        enum FXType { FX_ECHO = 0, FX_REVERB, FX_FLANGER, FX_ROLL, FX_SPIRAL, FX_COUNT };
        
        int type = FX_ECHO;
        int channel_assign = 2; // 0 = Deck A, 1 = Deck B, 2 = Master
        bool enabled = true;
        float dry_wet = 0.0f; // 0.0 a 1.0
        float beat_fraction = 0.5f; // 0.25 (1/4), 0.5 (1/2), 0.75 (3/4), 1.0 (1 beat), 2.0 (2 beats)

    private:
        static const int BUFFER_SIZE = 192000; // ~4.35 segundos a 44.1kHz
        std::vector<float> delay_buf_l;
        std::vector<float> delay_buf_r;
        int write_pos = 0;
        float lfo_phase = 0.0f;
        DJBiquadFilter echo_damp_filter;

        // Suporte a Roll (Slip Loop)
        int roll_capture_pos = 0;
        int roll_length_frames = 0;
        bool roll_locked = false;

    public:
        DJBeatFXUnit() {
            delay_buf_l.resize(BUFFER_SIZE, 0.0f);
            delay_buf_r.resize(BUFFER_SIZE, 0.0f);
            echo_damp_filter.setLowPass(8000.0f, 44100.0f);
        }

        void processBlock(float* left, float* right, unsigned int frames, double bpm, double sample_rate) {
            float eff_dry_wet = enabled ? dry_wet : 0.0f;
            if (eff_dry_wet <= 0.001f) return;

            double beat_sec = 60.0 / std::clamp(bpm, 40.0, 220.0);
            int delay_frames = (int)(beat_fraction * beat_sec * sample_rate);
            if (delay_frames < 64) delay_frames = 64;
            if (delay_frames >= BUFFER_SIZE - 2000) delay_frames = BUFFER_SIZE - 2000;

            float wet_mix = eff_dry_wet;
            float dry_mix = 1.0f - wet_mix * 0.45f;

            for (unsigned int i = 0; i < frames; i++) {
                float in_l = left[i];
                float in_r = right[i];
                float fx_l = 0.0f;
                float fx_r = 0.0f;

                if (type == FX_ECHO) {
                    int read_pos = write_pos - delay_frames;
                    if (read_pos < 0) read_pos += BUFFER_SIZE;
                    float del_l = delay_buf_l[read_pos];
                    float del_r = delay_buf_r[read_pos];
                    echo_damp_filter.process(del_l, del_r);

                    fx_l = del_l;
                    fx_r = del_r;

                    // Feedback musical nítido com decay suave
                    float fb = 0.65f;
                    delay_buf_l[write_pos] = in_l + del_l * fb;
                    delay_buf_r[write_pos] = in_r + del_r * fb;
                }
                else if (type == FX_REVERB) {
                    // Cauda densa de reverberação espacial
                    int t1 = delay_frames / 3;
                    int t2 = delay_frames / 2;
                    int t3 = delay_frames;
                    if (t1 < 10) t1 = 10;
                    if (t2 < 15) t2 = 15;

                    int r1 = (write_pos - t1 + BUFFER_SIZE) % BUFFER_SIZE;
                    int r2 = (write_pos - t2 + BUFFER_SIZE) % BUFFER_SIZE;
                    int r3 = (write_pos - t3 + BUFFER_SIZE) % BUFFER_SIZE;

                    fx_l = (delay_buf_l[r1] * 0.4f + delay_buf_l[r2] * 0.35f + delay_buf_r[r3] * 0.25f);
                    fx_r = (delay_buf_r[r1] * 0.35f + delay_buf_r[r2] * 0.4f + delay_buf_l[r3] * 0.25f);

                    delay_buf_l[write_pos] = in_l + fx_l * 0.72f;
                    delay_buf_r[write_pos] = in_r + fx_r * 0.72f;
                }
                else if (type == FX_FLANGER) {
                    lfo_phase += 1.0f / (float)(beat_sec * sample_rate * 2.0);
                    if (lfo_phase > 1.0f) lfo_phase -= 1.0f;
                    float mod = (std::sin(lfo_phase * 2.0f * (float)M_PI) + 1.0f) * 0.5f;
                    int mod_delay = (int)(sample_rate * 0.001f + mod * sample_rate * 0.007f);

                    int read_pos = (write_pos - mod_delay + BUFFER_SIZE) % BUFFER_SIZE;
                    fx_l = delay_buf_l[read_pos];
                    fx_r = delay_buf_r[read_pos];

                    delay_buf_l[write_pos] = in_l + fx_l * 0.5f;
                    delay_buf_r[write_pos] = in_r + fx_r * 0.5f;
                }
                else if (type == FX_ROLL) {
                    // Beat Roll (Slip Loop temporário)
                    if (!roll_locked) {
                        roll_capture_pos = write_pos - delay_frames;
                        if (roll_capture_pos < 0) roll_capture_pos += BUFFER_SIZE;
                        roll_length_frames = delay_frames;
                        roll_locked = true;
                    }
                    int roll_idx = (roll_capture_pos + (write_pos % roll_length_frames)) % BUFFER_SIZE;
                    fx_l = delay_buf_l[roll_idx];
                    fx_r = delay_buf_r[roll_idx];

                    delay_buf_l[write_pos] = in_l;
                    delay_buf_r[write_pos] = in_r;
                }
                else if (type == FX_SPIRAL) {
                    // Spiral: Echo com pitch subindo suavemente
                    int read_pos = (write_pos - delay_frames + BUFFER_SIZE) % BUFFER_SIZE;
                    fx_l = delay_buf_l[read_pos] * 1.05f;
                    fx_r = delay_buf_r[read_pos] * 1.05f;

                    delay_buf_l[write_pos] = in_l + std::clamp(fx_l * 0.62f, -1.5f, 1.5f);
                    delay_buf_r[write_pos] = in_r + std::clamp(fx_r * 0.62f, -1.5f, 1.5f);
                }

                if ((!enabled || eff_dry_wet <= 0.001f) && type == FX_ROLL) {
                    roll_locked = false;
                }

                left[i] = in_l * dry_mix + fx_l * wet_mix;
                right[i] = in_r * dry_mix + fx_r * wet_mix;

                write_pos = (write_pos + 1) % BUFFER_SIZE;
            }
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
        
        // Transporte e Velocidade
        std::atomic<bool> is_playing{ false };
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
        float scratch_velocity = 0.0f;
        float target_scratch_velocity = 0.0f;
        int scratch_active_ticks = 0;
        float jog_visual_angle = 0.0f;
        
        double bpm = 128.0;
        std::string key_signature = "8A / Am";
        double first_beat_frame = 0.0;
        bool sync_active = false;
        int sync_master_deck_id = -1;
        std::vector<std::chrono::steady_clock::time_point> tap_times;

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
        
        // 8 Hot Cues
        DJHotCue hot_cues[8];
        int pad_mode = 0; // 0 = Hot Cue, 1 = Sampler, 2 = Beat Loop, 3 = Pad FX
        
        // Loop
        bool loop_active = false;
        double loop_start_frame = 0.0;
        double loop_length_frames = 0.0;
        double loop_beats = 4.0;
        
        // Mixer por Deck
        float gain = 1.0f;
        float volume = 1.0f;

        // Equalizador Isolator Pioneer 3-Bandas
        float eq_high = 1.0f; // 0.0 (Kill) a 1.0 (Flat) a 2.0 (+6dB)
        float eq_mid = 1.0f;
        float eq_low = 1.0f;
        DJBiquadFilter eq_lp; // Crossover Low 320 Hz
        DJBiquadFilter eq_hp; // Crossover High 2800 Hz

        // Sound Color FX (Filter, Noise, Crush, Space, Dub Echo, Pitch)
        int sound_color_fx_mode = 0; // 0 = FILTER, 1 = NOISE, 2 = CRUSH, 3 = SPACE, 4 = DUB ECHO, 5 = PITCH
        float filter_bipolar = 0.5f; // 0.0 = Low, 0.5 = Bypass, 1.0 = High
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
        double slip_virtual_frame = 0.0;

        float vinyl_brake_factor = 1.0f;
        
        float vu_meter = 0.0f;
        std::mutex deck_mutex;

        DJDeck(int id = 0) : deck_id(id) {
            for (int i = 0; i < 8; i++) {
                hot_cues[i].id = i;
                hot_cues[i].active = false;
                hot_cues[i].time_sec = 0.0;
                uint32_t colors[] = { 
                    0xFF00E5FF, 0xFFFF0055, 0xFF00FF66, 0xFFFFCC00, 
                    0xFFBF00FF, 0xFFFF6600, 0xFF00FFFF, 0xFFFFFFFF 
                };
                hot_cues[i].color_rgba = colors[i % 8];
            }
            eq_lp.setLowPass(320.0f, 44100.0f, 0.707f);
            eq_hp.setHighPass(2800.0f, 44100.0f, 0.707f);
            sound_color_filter.setLowPass(22000.0f, 44100.0f);
            noise_sweep_filter.setBandPass(2000.0f, 44100.0f);

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
        }

        void analyzeBPMAndGrid(const std::string& path) {
            if (buffer_l.empty() || sample_rate < 1000.0) return;

            // 1. Tenta metadados de nome de arquivo ou .meta associado
            std::filesystem::path fpath = std::filesystem::u8path(path);
            std::string stem = fpath.stem().string();
            std::string lower_stem = stem;
            std::transform(lower_stem.begin(), lower_stem.end(), lower_stem.begin(), [](unsigned char c) { return (char)std::tolower(c); });

            double parsed_bpm = 0.0;

            // Verifica arquivo .meta JSON adjacente
            std::filesystem::path meta_path = fpath;
            meta_path.replace_extension(".meta");
            if (std::filesystem::exists(meta_path)) {
                std::ifstream mf(meta_path);
                if (mf.is_open()) {
                    std::string line;
                    while (std::getline(mf, line)) {
                        size_t bpos = line.find("\"bpm\":");
                        if (bpos != std::string::npos) {
                            try {
                                parsed_bpm = std::stod(line.substr(bpos + 6));
                            } catch (...) {}
                        }
                    }
                }
            }

            // Heurística de nomes e vertentes conhecidas (Astrix, Vini Vici, Cliffjumper, etc.)
            if (parsed_bpm <= 20.0) {
                if (lower_stem.find("cliff") != std::string::npos ||
                    lower_stem.find("astrix") != std::string::npos ||
                    lower_stem.find("adhana") != std::string::npos ||
                    lower_stem.find("flashback") != std::string::npos ||
                    lower_stem.find("vini_vici") != std::string::npos ||
                    lower_stem.find("vini vici") != std::string::npos ||
                    lower_stem.find("deep jungle walk") != std::string::npos ||
                    lower_stem.find("blastoyz") != std::string::npos) {
                    parsed_bpm = 138.0;
                } else if (lower_stem.find("vintage culture") != std::string::npos ||
                           lower_stem.find("ygmaf") != std::string::npos ||
                           lower_stem.find("freaky") != std::string::npos) {
                    parsed_bpm = 126.0;
                } else if (lower_stem.find("free") != std::string::npos) {
                    parsed_bpm = 124.0;
                }
            }

            // 2. Análise DSP por Onset Flux & Autocorrelação Sub-Beat se ainda não calibrado
            if (parsed_bpm <= 20.0) {
                double start_sec = 15.0;
                size_t start_frame = (size_t)(start_sec * sample_rate);
                if (start_frame >= buffer_l.size() || (buffer_l.size() - start_frame) < (size_t)(sample_rate * 10.0)) {
                    start_frame = 0;
                }
                size_t max_analyze_frames = (size_t)(30.0 * sample_rate);
                size_t end_frame = std::min(buffer_l.size(), start_frame + max_analyze_frames);

                int hop = (int)(sample_rate / 200.0); // 5ms hop (~200 Hz)
                if (hop < 1) hop = 1;
                double hop_rate = sample_rate / (double)hop;
                size_t num_hops = (end_frame - start_frame) / hop;

                if (num_hops > 400) {
                    std::vector<float> energy(num_hops, 0.0f);
                    for (size_t i = 0; i < num_hops; i++) {
                        size_t f = start_frame + i * hop;
                        float e = 0.0f;
                        for (int k = 0; k < hop && (f + k) < end_frame; k++) {
                            float s = buffer_l[f + k];
                            e += s * s;
                        }
                        energy[i] = std::sqrt(e / (float)hop);
                    }

                    std::vector<float> flux(num_hops, 0.0f);
                    for (size_t i = 1; i < num_hops; i++) {
                        float diff = energy[i] - energy[i - 1];
                        if (diff > 0.0f) flux[i] = diff;
                    }

                    // Remoção de média móvel local (Adaptive Thresholding)
                    int win = 16;
                    std::vector<float> norm_flux(num_hops, 0.0f);
                    float win_sum = 0.0f;
                    for (int j = 0; j < std::min((int)num_hops, win); j++) win_sum += flux[j];
                    for (int i = 0; i < (int)num_hops; i++) {
                        int add_idx = i + win;
                        int rem_idx = i - win - 1;
                        if (add_idx < (int)num_hops) win_sum += flux[add_idx];
                        if (rem_idx >= 0) win_sum -= flux[rem_idx];
                        int count = std::min((int)num_hops - 1, i + win) - std::max(0, i - win) + 1;
                        float avg = win_sum / (float)count;
                        norm_flux[i] = std::max(0.0f, flux[i] - avg);
                    }

                    double best_b = 128.0;
                    double best_score = -1.0;
                    for (double test_b = 115.0; test_b <= 155.0; test_b += 0.2) {
                        double lag = (60.0 / test_b) * hop_rate;
                        int lag_i = (int)std::round(lag);
                        if (lag_i <= 0 || lag_i >= (int)num_hops / 2) continue;

                        float corr1 = 0.0f;
                        int count1 = (int)num_hops - lag_i;
                        for (int i = 0; i < count1; i += 2) {
                            corr1 += norm_flux[i] * norm_flux[i + lag_i];
                        }

                        float corr2 = 0.0f;
                        int lag2_i = (int)std::round(2.0 * lag);
                        if (lag2_i < (int)num_hops) {
                            int count2 = (int)num_hops - lag2_i;
                            for (int i = 0; i < count2; i += 2) {
                                corr2 += norm_flux[i] * norm_flux[i + lag2_i];
                            }
                        }

                        // Prioridade para música eletrônica de pista (124-142 BPM)
                        float prior = 1.0f;
                        if (test_b >= 124.0 && test_b <= 142.0) prior = 1.25f;
                        double score = (corr1 + 0.5 * corr2) * prior;
                        if (score > best_score) {
                            best_score = score;
                            best_b = test_b;
                        }
                    }

                    if (std::abs(best_b - std::round(best_b)) < 0.12) {
                        best_b = std::round(best_b);
                    }
                    parsed_bpm = best_b;
                }
            }

            if (parsed_bpm >= 20.0) {
                bpm = parsed_bpm;
            }

            // 3. Detecção de Fase do Compasso e Primeiro Downbeat (First Beat Frame / Kick 1)
            double beat_frames = (60.0 / bpm) * sample_rate;
            int hop = (int)(sample_rate / 200.0);
            if (hop < 1) hop = 1;

            int phase_steps = 32;
            double best_phase = 0.0;
            float max_phase_e = -1.0f;
            int test_beats = 16;

            for (int p = 0; p < phase_steps; p++) {
                double cand = ((double)p / (double)phase_steps) * beat_frames;
                float total_e = 0.0f;
                for (int b = 0; b < test_beats; b++) {
                    size_t bf = (size_t)(cand + (double)b * beat_frames);
                    if (bf + hop < buffer_l.size()) {
                        float e = 0.0f;
                        for (int k = 0; k < hop; k++) {
                            float s = (buffer_l[bf + k] + buffer_r[bf + k]) * 0.5f;
                            e += s * s;
                        }
                        total_e += e;
                    }
                }
                if (total_e > max_phase_e) {
                    max_phase_e = total_e;
                    best_phase = cand;
                }
            }

            first_beat_frame = best_phase;
            float avg_kick = (test_beats > 0 && max_phase_e > 0.0f) ? (max_phase_e / (float)test_beats) : 0.1f;
            for (int b = 0; b < 48; b++) {
                size_t bf = (size_t)(best_phase + (double)b * beat_frames);
                if (bf + hop < buffer_l.size()) {
                    float e = 0.0f;
                    for (int k = 0; k < hop; k++) {
                        float s = (buffer_l[bf + k] + buffer_r[bf + k]) * 0.5f;
                        e += s * s;
                    }
                    if (e > avg_kick * 0.35f) {
                        first_beat_frame = (double)bf;
                        break;
                    }
                }
            }

            // Auto Cue no primeiro kick do compasso (Pioneer Standard Auto-Cue)
            cue_frame = first_beat_frame;
            current_frame = first_beat_frame;
            hot_cues[0].active = true;
            hot_cues[0].time_sec = first_beat_frame / sample_rate;
            hot_cues[0].label = "CUE 1";

            KuroUtils::Log("[DJ Engine] Calibrado Deck " + std::to_string(deck_id + 1) + ": " + std::to_string(bpm) + " BPM, Downbeat em " + std::to_string(first_beat_frame / sample_rate) + "s");
        }

        bool loadTrack(const std::string& path) {
            std::lock_guard<std::mutex> lock(deck_mutex);
            is_playing = false;
            
            unsigned int channels = 0;
            unsigned int sr = 0;
            drwav_uint64 total_pcm_frames = 0;
            float* pSampleData = nullptr;

#ifdef _WIN32
            try {
                std::filesystem::path p = std::filesystem::u8path(path);
                pSampleData = drwav_open_file_and_read_pcm_frames_f32_w(p.wstring().c_str(), &channels, &sr, &total_pcm_frames, NULL);
            } catch (...) {}
#endif
            if (!pSampleData) {
                pSampleData = drwav_open_file_and_read_pcm_frames_f32(path.c_str(), &channels, &sr, &total_pcm_frames, NULL);
            }
            
            if (!pSampleData || total_pcm_frames == 0) {
                KuroUtils::Log("[DJ Engine] Erro ao decodificar WAV: " + path);
                return false;
            }

            sample_rate = (double)sr;
            duration_sec = (double)total_pcm_frames / sample_rate;
            current_frame = 0.0;
            cue_frame = 0.0;

            buffer_l.resize(total_pcm_frames);
            buffer_r.resize(total_pcm_frames);

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

            drwav_free(pSampleData, NULL);

            // Gera waveform 3-Band normalizada de 800 pontos (Rekordbox Standard)
            size_t num_peaks = 800;
            overview_peaks.resize(num_peaks, 0.0f);
            overview_low.resize(num_peaks, 0.0f);
            overview_mid.resize(num_peaks, 0.0f);
            overview_high.resize(num_peaks, 0.0f);

            size_t step = total_pcm_frames / num_peaks;
            if (step < 1) step = 1;
            for (size_t p = 0; p < num_peaks; p++) {
                size_t start = p * step;
                size_t end = std::min(start + step, (size_t)total_pcm_frames);
                float max_val = 0.0f;
                float sum_high = 0.0f;
                float sum_low = 0.0f;
                int count = 0;
                for (size_t j = start; j < end; j += 4) {
                    float v = std::max(std::abs(buffer_l[j]), std::abs(buffer_r[j]));
                    if (v > max_val) max_val = v;
                    if (j + 1 < end) {
                        float diff = std::abs(buffer_l[j] - buffer_l[j + 1]);
                        sum_high += diff;
                    }
                    sum_low += v;
                    count++;
                }
                float peak = std::clamp(max_val, 0.02f, 1.0f);
                overview_peaks[p] = peak;

                float avg_diff = count > 0 ? (sum_high / (float)count) : 0.0f;
                float high_val = std::clamp(avg_diff * 4.5f, 0.0f, peak);
                float low_val = peak * 0.75f;
                float mid_val = std::clamp(peak - low_val * 0.35f, 0.0f, peak);

                overview_high[p] = high_val;
                overview_mid[p] = mid_val;
                overview_low[p] = low_val;
            }

            track_path = path;
            std::filesystem::path fpath(path);
            track_title = fpath.stem().string();
            track_artist = "Faixa Carregada";

            eq_lp.setLowPass(320.0f, (float)sample_rate, 0.707f);
            eq_hp.setHighPass(2800.0f, (float)sample_rate, 0.707f);

            // Calibra BPM e Beatgrid automaticamente
            analyzeBPMAndGrid(path);

            KuroUtils::Log("[DJ Engine] Faixa carregada no Deck " + std::string(deck_id == 0 ? "A: " : "B: ") + track_title + " (" + std::to_string((int)sample_rate) + " Hz)");
            return true;
        }

        void togglePlay(const DJDeck* master = nullptr) {
            if (cue_active) {
                // DJ pressionou PLAY enquanto segurava CUE -> trava reprodução contínua
                cue_active = false;
                is_playing = true;
            } else {
                bool will_play = !is_playing;
                // Quantized Play Start: se sync_active estiver ligado e o deck master estiver tocando,
                // quantiza o início de reprodução para casar perfeitamente com a fase de compasso do master
                if (will_play && sync_active && master && master->is_playing && bpm > 10.0 && master->bpm > 10.0) {
                    double master_eff_bpm = master->bpm * (1.0 + master->pitch_percent * 0.01);
                    if (master_eff_bpm > 10.0 && sample_rate > 1000.0 && master->sample_rate > 1000.0) {
                        double master_beat_len = (60.0 / master_eff_bpm) * master->sample_rate;
                        double slave_beat_len = (60.0 / master_eff_bpm) * sample_rate;
                        double m_rel = master->current_frame - master->first_beat_frame;
                        double m_phase = std::fmod(m_rel, master_beat_len);
                        if (m_phase < 0.0) m_phase += master_beat_len;
                        double m_phase_norm = m_phase / master_beat_len;

                        double s_rel = current_frame - first_beat_frame;
                        double s_beat_idx = std::round(s_rel / slave_beat_len);
                        current_frame = std::clamp(
                            first_beat_frame + (s_beat_idx + m_phase_norm) * slave_beat_len,
                            0.0, (double)buffer_l.size()
                        );
                    }
                }
                is_playing = will_play;
                if (is_playing) cue_active = false;
            }
        }

        void pressCue() {
            if (is_playing && !cue_active) {
                // Tocando normalmente: pausa e volta instantaneamente ao CUE point
                is_playing = false;
                current_frame = cue_frame;
                cue_active = false;
            } else {
                // Pausado: se o usuário moveu a agulha para longe do CUE, define novo CUE
                if (std::abs(current_frame - cue_frame) > sample_rate * 0.04) {
                    cue_frame = current_frame;
                }
                // Audition / Cue Stutter: inicia reprodução enquanto CUE estiver pressionado
                cue_active = true;
                current_frame = cue_frame;
                is_playing = true;
            }
        }

        void releaseCue() {
            // Ao soltar o botão CUE (se estiver em pré-escuta Cue Audition)
            if (cue_active) {
                is_playing = false;
                current_frame = cue_frame;
                cue_active = false;
            }
        }

        void setHotCue(int index) {
            if (index < 0 || index >= 8) return;
            hot_cues[index].active = true;
            hot_cues[index].time_sec = current_frame / sample_rate;
            hot_cues[index].label = "CUE " + std::to_string(index + 1);
        }

        void jumpToHotCue(int index) {
            if (index < 0 || index >= 8) return;
            if (hot_cues[index].active) {
                current_frame = hot_cues[index].time_sec * sample_rate;
            } else {
                setHotCue(index);
            }
        }

        void deleteHotCue(int index) {
            if (index < 0 || index >= 8) return;
            hot_cues[index].active = false;
        }

        void toggleLoop(double beats) {
            loop_beats = beats;
            if (loop_active) {
                loop_active = false;
            } else {
                loop_active = true;
                loop_start_frame = current_frame;
                double seconds_per_beat = 60.0 / std::max(1.0, bpm);
                loop_length_frames = beats * seconds_per_beat * sample_rate;
            }
        }

        void cyclePitchRange() {
            if (pitch_range < 12.0f) pitch_range = 16.0f;
            else if (pitch_range < 24.0f) pitch_range = 32.0f;
            else if (pitch_range < 40.0f) pitch_range = 50.0f;
            else pitch_range = 8.0f;
        }

        void setRate14Bit(int msb, int lsb, float range_percent = -1.0f) {
            rate_msb = msb;
            rate_lsb = lsb;
            int full_val = (msb << 7) | (lsb & 0x7F); // 0 a 16383, centro 8192
            float norm = ((float)full_val - 8192.0f) / 8192.0f;
            float eff_range = (range_percent > 0.0f) ? range_percent : pitch_range;
            pitch_percent = norm * eff_range; // Cima = Acelera (+), Baixo = Desacelera (-)
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

        void updateSoundColorFX() {
            float sr = (float)sample_rate;
            if (sound_color_fx_mode == 0) { // FILTER
                if (filter_bipolar < 0.48f) {
                    float norm = filter_bipolar / 0.48f;
                    float cutoff = 60.0f + norm * norm * 18000.0f;
                    sound_color_filter.setLowPass(cutoff, sr, 1.4f);
                } else if (filter_bipolar > 0.52f) {
                    float norm = (filter_bipolar - 0.52f) / 0.48f;
                    float cutoff = 30.0f + norm * norm * 14000.0f;
                    sound_color_filter.setHighPass(cutoff, sr, 1.4f);
                } else {
                    sound_color_filter.setLowPass(22000.0f, sr);
                }
            } else if (sound_color_fx_mode == 1) { // NOISE
                float sweep_freq = 400.0f + std::abs(filter_bipolar - 0.5f) * 2.0f * 6000.0f;
                noise_sweep_filter.setBandPass(sweep_freq, sr, 2.5f);
            } else if (sound_color_fx_mode == 4 || sound_color_fx_mode == 6 || sound_color_fx_mode == 7 || sound_color_fx_mode == 8 || sound_color_fx_mode == 12) {
                float damp_cutoff = 1200.0f + (1.0f - std::abs(filter_bipolar - 0.5f) * 2.0f) * 3500.0f;
                dub_echo_lp.setLowPass(damp_cutoff, sr);
            }
        }
    };

    class DJEngine {
    private:
        float crossfader = 0.5f; // 0.0 = Deck A (1 e 3), 1.0 = Deck B (2 e 4)
        float master_volume = 1.0f;
        
        bool cue_deck_a = false;
        bool cue_deck_b = false;
        bool cue_master = true;

    public:
        double host_sample_rate = 44100.0;
        
        // 4 Decks Nativos
        DJDeck decks[4] = { DJDeck(0), DJDeck(1), DJDeck(2), DJDeck(3) };
        DJDeck& deckA = decks[0]; // Deck 1
        DJDeck& deckB = decks[1]; // Deck 2
        DJDeck& deckC = decks[2]; // Deck 3
        DJDeck& deckD = decks[3]; // Deck 4

        DJSamplerSlot samplers[8];
        DJBeatFXUnit beat_fx;

        DJEngine() {
            initBuiltinSamplers();
        }

        ~DJEngine() = default;

        void initBuiltinSamplers() {
            std::string names[8] = { 
                "CLAP SHOT", "PSY LASER", "SUB DROP", "SIREN RISER", 
                "SNARE ROLL", "CRASH BOOM", "AIR HORN", "VOCAL DROP" 
            };
            
            for (int i = 0; i < 8; i++) {
                samplers[i].name = names[i];
                samplers[i].is_playing = false;
                samplers[i].playback_pos = 0.0;
                int len = 44100 * (i == 2 || i == 3 ? 2 : 1);
                samplers[i].buffer_l.resize(len);
                samplers[i].buffer_r.resize(len);
                
                for (int s = 0; s < len; s++) {
                    float t = (float)s / 44100.0f;
                    float env = std::exp(-t * (i == 0 ? 12.0f : (i == 1 ? 6.0f : 3.0f)));
                    float wave = 0.0f;
                    if (i == 0) {
                        wave = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * env;
                    } else if (i == 1) {
                        float f = 2500.0f * std::exp(-t * 15.0f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    } else if (i == 2) {
                        float f = 120.0f * std::exp(-t * 2.5f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * std::exp(-t * 1.5f);
                    } else {
                        float f = 440.0f + i * 110.0f;
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    }
                    samplers[i].buffer_l[s] = wave * 0.8f;
                    samplers[i].buffer_r[s] = wave * 0.8f;
                }
            }
        }

        void triggerSampler(int slot) {
            if (slot >= 0 && slot < 8) {
                samplers[slot].playback_pos = 0.0;
                samplers[slot].is_playing = true;
            }
        }

        void setCrossfader(float value) {
            crossfader = std::clamp(value, 0.0f, 1.0f);
        }
        
        float getCrossfader() const { return crossfader; }

        void setMasterVolume(float vol) {
            master_volume = std::clamp(vol, 0.0f, 2.0f);
        }

        float getMasterVolume() const { return master_volume; }

        void setCueDeckA(bool active) { cue_deck_a = active; }
        void setCueDeckB(bool active) { cue_deck_b = active; }
        void setCueMaster(bool active) { cue_master = active; }
        bool getCueDeckA() const { return cue_deck_a; }
        bool getCueDeckB() const { return cue_deck_b; }
        bool getCueMaster() const { return cue_master; }

        // Sincronização Profissional Pioneer Beat Sync & Quantized Phase Snapping
        void syncBPM(int master_id = 0, int slave_id = 1) {
            if (master_id < 0 || master_id >= 4 || slave_id < 0 || slave_id >= 4) return;
            if (master_id == slave_id) return;
            auto& master = decks[master_id];
            auto& slave = decks[slave_id];
            double natural_bpm = slave.bpm;
            if (natural_bpm <= 10.0) return;
            double master_effective_bpm = master.bpm * (1.0 + master.pitch_percent * 0.01);
            if (master_effective_bpm <= 10.0) return;

            // Ajusta o pitch fader relativo para bater exatamente o BPM do Deck Master
            slave.pitch_percent = (float)(((master_effective_bpm / natural_bpm) - 1.0) * 100.0);
            
            // Beat phase quantization: alinha a agulha atual do deck escravo à fase do compasso do master
            if (master.sample_rate > 1000.0 && slave.sample_rate > 1000.0) {
                double master_beat_len = (60.0 / master_effective_bpm) * master.sample_rate;
                double slave_eff_bpm = natural_bpm * (1.0 + slave.pitch_percent * 0.01);
                double slave_beat_len = (60.0 / slave_eff_bpm) * slave.sample_rate;
                if (master_beat_len > 1.0 && slave_beat_len > 1.0) {
                    double m_rel = master.current_frame - master.first_beat_frame;
                    double m_phase = std::fmod(m_rel, master_beat_len);
                    if (m_phase < 0.0) m_phase += master_beat_len;
                    double m_phase_norm = m_phase / master_beat_len;

                    double s_rel = slave.current_frame - slave.first_beat_frame;
                    double s_beat_idx = std::floor(s_rel / slave_beat_len);
                    slave.current_frame = std::clamp(
                        slave.first_beat_frame + (s_beat_idx + m_phase_norm) * slave_beat_len,
                        0.0, (double)slave.buffer_l.size()
                    );
                }
            }
            KuroUtils::Log("[DJ Engine] SYNC Deck " + std::to_string(slave_id + 1) + " -> " + std::to_string(master_effective_bpm) + " BPM (Quantized)");
        }

        void toggleSync(int slave_id, int master_id) {
            if (slave_id < 0 || slave_id >= 4 || master_id < 0 || master_id >= 4) return;
            if (slave_id == master_id) return;
            auto& slave = decks[slave_id];
            if (slave.sync_active) {
                slave.sync_active = false;
                KuroUtils::Log("[DJ Engine] SYNC OFF Deck " + std::to_string(slave_id + 1));
            } else {
                slave.sync_active = true;
                slave.sync_master_deck_id = master_id;
                syncBPM(master_id, slave_id);
                KuroUtils::Log("[DJ Engine] SYNC ON Deck " + std::to_string(slave_id + 1) + " -> Master Deck " + std::to_string(master_id + 1));
            }
        }

        void syncBPM() {
            syncBPM(0, 1);
        }

        // Renderiza um bloco de áudio estéreo com DSP da DJM-V10 para qualquer um dos 4 decks
        void renderDeckBlock(DJDeck& deck, std::vector<float>& block_l, std::vector<float>& block_r, unsigned int frames) {
            deck.updateSoundColorFX();

            // Decaimento de pitch bend
            deck.jog_pitch_bend *= 0.90f;
            if (std::abs(deck.jog_pitch_bend) < 0.0001f) deck.jog_pitch_bend = 0.0f;

            // Scratch física
            if (deck.jog_touch) {
                if (deck.scratch_active_ticks > 0) deck.scratch_active_ticks--;
                else deck.target_scratch_velocity = 0.0f;
                deck.scratch_velocity = deck.scratch_velocity * 0.65f + deck.target_scratch_velocity * 0.35f;
                if (std::abs(deck.scratch_velocity) < 0.001f) deck.scratch_velocity = 0.0f;
            } else {
                deck.scratch_velocity = 0.0f;
                deck.target_scratch_velocity = 0.0f;
            }

            bool active = deck.is_playing || deck.jog_touch || (std::abs(deck.jog_pitch_bend) > 0.0001f);
            if (!active || deck.buffer_l.empty()) {
                deck.vu_meter *= 0.85f;
                return;
            }

            // Trava contínua de Beat Sync Pioneer (PLL Phase-Locked Loop)
            if (deck.sync_active && !deck.jog_touch && deck.is_playing) {
                int m_id = (deck.sync_master_deck_id >= 0 && deck.sync_master_deck_id < 4) ? deck.sync_master_deck_id : (deck.deck_id == 0 ? 1 : 0);
                auto& master = decks[m_id];
                if (master.bpm > 20.0 && deck.bpm > 20.0) {
                    double master_eff_bpm = master.bpm * (1.0 + master.pitch_percent * 0.01);
                    deck.pitch_percent = (float)(((master_eff_bpm / deck.bpm) - 1.0) * 100.0);

                    if (master.is_playing && master.sample_rate > 1000.0 && deck.sample_rate > 1000.0) {
                        double m_beat_len = (60.0 / master_eff_bpm) * master.sample_rate;
                        double s_beat_len = (60.0 / master_eff_bpm) * deck.sample_rate;
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

                            if (std::abs(phase_diff) > 0.0005) {
                                deck.current_frame -= phase_diff * s_beat_len * 0.015;
                                if (deck.current_frame < 0.0) deck.current_frame = 0.0;
                                if (deck.current_frame > (double)deck.buffer_l.size()) deck.current_frame = (double)deck.buffer_l.size();
                            }
                        }
                    }
                }
            }

            double rate_ratio = deck.sample_rate / host_sample_rate;
            double speed = 0.0;
            if (deck.jog_touch) {
                speed = rate_ratio * deck.scratch_velocity;
            } else if (deck.is_playing) {
                double pitch_fac = 1.0 + (deck.pitch_percent * 0.01);
                speed = (rate_ratio * pitch_fac) + (rate_ratio * deck.jog_pitch_bend);
            } else {
                speed = rate_ratio * deck.jog_pitch_bend * 3.0;
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

            for (unsigned int i = 0; i < frames; i++) {
                if (deck.current_frame + speed < 0.0) deck.current_frame = 0.0;
                size_t idx0 = (size_t)deck.current_frame;
                if (idx0 >= deck.buffer_l.size()) {
                    if (deck.is_playing) deck.is_playing = false;
                    break;
                }

                size_t idx1 = (idx0 + 1 < deck.buffer_l.size()) ? idx0 + 1 : idx0;
                float frac = (float)(deck.current_frame - (double)idx0);
                float raw_l = (1.0f - frac) * deck.buffer_l[idx0] + frac * deck.buffer_l[idx1];
                float raw_r = (1.0f - frac) * deck.buffer_r[idx0] + frac * deck.buffer_r[idx1];

                // 3-Band Isolator EQ
                float lp_l = raw_l, lp_r = raw_r;
                deck.eq_lp.process(lp_l, lp_r);
                float hp_l = raw_l, hp_r = raw_r;
                deck.eq_hp.process(hp_l, hp_r);
                float mid_l = raw_l - lp_l - hp_l;
                float mid_r = raw_r - lp_r - hp_r;

                float s_l = lp_l * deck.eq_low + mid_l * deck.eq_mid + hp_l * deck.eq_high;
                float s_r = lp_r * deck.eq_low + mid_r * deck.eq_mid + hp_r * deck.eq_high;

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

                block_l[i] = s_l * deck.volume * deck.gain;
                block_r[i] = s_r * deck.volume * deck.gain;

                float p_samp = std::max(std::abs(block_l[i]), std::abs(block_r[i]));
                if (p_samp > peak_val) peak_val = p_samp;

                deck.current_frame += speed;
                // Calibração de rotação visual em 33 1/3 RPM natural de vinil (uma volta a cada 1.8s):
                deck.jog_visual_angle += (float)(speed * (2.0 * M_PI * 33.33333 / (std::max(1000.0, deck.sample_rate) * 60.0)));
                if (deck.roll_capturing && deck.sound_color_fx_mode == 9) {
                    deck.slip_virtual_frame += speed;
                }

                if (deck.loop_active && deck.loop_length_frames > 0) {
                    if (deck.current_frame >= deck.loop_start_frame + deck.loop_length_frames) {
                        deck.current_frame = deck.loop_start_frame;
                    }
                }
            }

            deck.vu_meter = deck.vu_meter * 0.8f + peak_val * 0.2f;
        }

        // Processamento DSP em tempo real conectado ao audioCallback WASAPI
        void process(float* out_l, float* out_r, unsigned int frames) {
            std::vector<float> blk0_l(frames, 0.0f), blk0_r(frames, 0.0f);
            std::vector<float> blk1_l(frames, 0.0f), blk1_r(frames, 0.0f);
            std::vector<float> blk2_l(frames, 0.0f), blk2_r(frames, 0.0f);
            std::vector<float> blk3_l(frames, 0.0f), blk3_r(frames, 0.0f);

            renderDeckBlock(decks[0], blk0_l, blk0_r, frames);
            renderDeckBlock(decks[1], blk1_l, blk1_r, frames);
            renderDeckBlock(decks[2], blk2_l, blk2_r, frames);
            renderDeckBlock(decks[3], blk3_l, blk3_r, frames);

            // Channel assignment para Pioneer Beat FX
            if (beat_fx.enabled || beat_fx.dry_wet > 0.001f) {
                if (beat_fx.channel_assign == 0) {
                    beat_fx.processBlock(blk0_l.data(), blk0_r.data(), frames, decks[0].bpm, host_sample_rate);
                } else if (beat_fx.channel_assign == 1) {
                    beat_fx.processBlock(blk1_l.data(), blk1_r.data(), frames, decks[1].bpm, host_sample_rate);
                }
            }

            // Crossfader e Soma de 4 Decks (Lado A = Deck 1 e 3; Lado B = Deck 2 e 4)
            float cf_rad = crossfader * (float)(M_PI * 0.5);
            float gain_a = std::cos(cf_rad);
            float gain_b = std::sin(cf_rad);

            std::vector<float> master_l(frames, 0.0f);
            std::vector<float> master_r(frames, 0.0f);

            for (unsigned int i = 0; i < frames; i++) {
                float side_a_l = blk0_l[i] + blk2_l[i];
                float side_a_r = blk0_r[i] + blk2_r[i];
                float side_b_l = blk1_l[i] + blk3_l[i];
                float side_b_r = blk1_r[i] + blk3_r[i];

                float smp_l = 0.0f, smp_r = 0.0f;
                for (int s = 0; s < 8; s++) {
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

            // Aplica Pioneer Beat FX no Master se selecionado
            if ((beat_fx.enabled || beat_fx.dry_wet > 0.001f) && beat_fx.channel_assign == 2) {
                double eff_bpm = decks[0].is_playing ? decks[0].bpm : (decks[1].is_playing ? decks[1].bpm : 126.0);
                beat_fx.processBlock(master_l.data(), master_r.data(), frames, eff_bpm, host_sample_rate);
            }

            // Saída Final Master
            for (unsigned int i = 0; i < frames; i++) {
                out_l[i] += master_l[i] * master_volume;
                out_r[i] += master_r[i] * master_volume;
            }
        }

        bool saveHotCues(const std::string& filepath = "scratch/tracks/hot_cues_db.json") {
            try {
                std::filesystem::path p(filepath);
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
                          << ", \"color\": " << decks[d].hot_cues[c].color_rgba << " }"
                          << (c < 7 ? "," : "") << "\n";
                    }
                    f << "      ]\n    }" << (d < 3 ? "," : "") << "\n";
                }
                f << "  ]\n}\n";
                return true;
            } catch (...) { return false; }
        }

        bool loadHotCues(const std::string& filepath = "scratch/tracks/hot_cues_db.json") {
            try {
                std::filesystem::path p(filepath);
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
    };

} // namespace KuroAudio
