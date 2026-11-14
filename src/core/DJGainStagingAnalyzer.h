#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <atomic>
#include "imgui.h"

namespace KuroAudio {

    class DJGainStagingAnalyzer {
    private:
        DJGainStagingAnalyzer() = default;

        // Ring buffer para cálculo de RMS / LUFS de curto prazo (3 segundos)
        std::vector<float> m_history_l;
        std::vector<float> m_history_r;
        size_t m_write_idx = 0;
        size_t m_window_size = 44100 * 3;

    public:
        static DJGainStagingAnalyzer& getInstance() {
            static DJGainStagingAnalyzer instance;
            return instance;
        }

        std::atomic<float> current_lufs{ -14.0f };
        std::atomic<float> current_true_peak_db{ -6.0f };
        std::atomic<bool> is_redlining{ false };

        // Transições de Emergência (Echo Out e Reverb Freeze)
        std::atomic<bool> echo_out_active{ false };
        float echo_out_feedback = 0.72f;       // [0.10 .. 0.95] Cauda do eco
        float echo_out_damping = 0.65f;        // [0.0 .. 1.0] Filtro macio LP
        int   echo_beat_division = 1;          // 0=1/4, 1=1/2, 2=3/4, 3=1 beat, 4=2 beats
        float echo_bpm = 138.0f;
        float echo_fade_mult = 1.0f;           // Fade out progressivo da música original
        size_t current_delay_samples = 24000;
        std::vector<float> delay_buf_l;
        std::vector<float> delay_buf_r;
        size_t delay_write_pos = 0;
        size_t echo_capture_remaining = 0;
        float echo_crossfade_pos = 0.0f;
        float echo_crossfade_step = 0.001f;
        float current_sr = 48000.0f;

        // Filtros analógicos e DC blocker para cauda sedosa livre de estalos/distorção
        float lp_filter_l = 0.0f;
        float lp_filter_r = 0.0f;
        float prev_dc_in_l = 0.0f;
        float prev_dc_in_r = 0.0f;
        float prev_dc_out_l = 0.0f;
        float prev_dc_out_r = 0.0f;

        bool reverb_freeze_active = false;
        std::vector<float> freeze_buf;

        void init(float sample_rate) {
            current_sr = sample_rate > 8000.0f ? sample_rate : 48000.0f;
            m_window_size = (size_t)(sample_rate * 3.0f);
            m_history_l.assign(m_window_size, 0.0f);
            m_history_r.assign(m_window_size, 0.0f);
            m_write_idx = 0;

            // Buffer de delay amplo (até 4 beats a 40 BPM ~ 6.0s)
            size_t max_delay = (size_t)(sample_rate * 6.0f);
            delay_buf_l.assign(max_delay, 0.0f);
            delay_buf_r.assign(max_delay, 0.0f);
            delay_write_pos = 0;
            current_delay_samples = (size_t)(sample_rate * 0.25f);
            stopEchoOut();
        }

        void triggerEchoOut(double bpm = 138.0) {
            if (bpm > 40.0) echo_bpm = (float)bpm;
            float beat_mult = 0.50f;
            if (echo_beat_division == 0) beat_mult = 0.25f;       // 1/4 Beat
            else if (echo_beat_division == 1) beat_mult = 0.50f;  // 1/2 Beat
            else if (echo_beat_division == 2) beat_mult = 0.75f;  // 3/4 Beat
            else if (echo_beat_division == 3) beat_mult = 1.00f;  // 1 Beat
            else if (echo_beat_division == 4) beat_mult = 2.00f;  // 2 Beats

            float sr = current_sr > 8000.0f ? current_sr : 48000.0f;
            size_t max_d = delay_buf_l.empty() ? 96000 : delay_buf_l.size();
            current_delay_samples = (size_t)std::clamp((sr * 60.0f / echo_bpm) * beat_mult, 200.0f, (float)max_d - 100.0f);

            // Crossfade rápido musical de 25ms para corte macio sem pop/estalo
            size_t xfade_frames = (size_t)std::max(64.0f, sr * 0.025f);
            echo_crossfade_pos = 0.0f;
            echo_crossfade_step = 1.0f / (float)xfade_frames;

            lp_filter_l = 0.0f;
            lp_filter_r = 0.0f;
            prev_dc_in_l = prev_dc_in_r = 0.0f;
            prev_dc_out_l = prev_dc_out_r = 0.0f;
            echo_out_active.store(true);
        }

        // Parada de Emergência Imediata (Limpa todos os buffers para NUNCA continuar soando)
        void stopEchoOut() {
            echo_out_active.store(false);
            echo_crossfade_pos = 1.0f;
            echo_fade_mult = 1.0f;
            echo_capture_remaining = 0;
            if (!delay_buf_l.empty()) {
                std::fill(delay_buf_l.begin(), delay_buf_l.end(), 0.0f);
                std::fill(delay_buf_r.begin(), delay_buf_r.end(), 0.0f);
            }
            delay_write_pos = 0;
            lp_filter_l = lp_filter_r = 0.0f;
            prev_dc_in_l = prev_dc_in_r = 0.0f;
            prev_dc_out_l = prev_dc_out_r = 0.0f;
        }

        void triggerReverbFreeze() {
            reverb_freeze_active = true;
        }

        void stopReverbFreeze() {
            reverb_freeze_active = false;
        }

        // Análise de Gain Staging do Master Out e processamento de emergência
        void processMaster(float* buffer_l, float* buffer_r, size_t frames, float sample_rate) {
            if (m_history_l.empty() || delay_buf_l.empty()) init(sample_rate);

            float peak_sq = 0.0f;
            float sum_sq = 0.0f;
            const size_t buf_sz = delay_buf_l.size();

            for (size_t i = 0; i < frames; i++) {
                float l = buffer_l[i];
                float r = buffer_r[i];

                // Anti-denormal nos inputs
                if (std::abs(l) < 1e-15f) l = 0.0f;
                if (std::abs(r) < 1e-15f) r = 0.0f;

                // Efeito Echo Out Analógico Pioneer Style (com Tape Saturation e Zero Clipping)
                if (echo_out_active.load() && current_delay_samples > 0 && current_delay_samples < buf_sz) {
                    size_t read_pos = (delay_write_pos + buf_sz - current_delay_samples) % buf_sz;
                    float echo_l = delay_buf_l[read_pos];
                    float echo_r = delay_buf_r[read_pos];

                    // 1. Saturação Suave Analógica estilo Tape Delay (evita estouros e distorção digital)
                    float sat_l = std::tanh(echo_l);
                    float sat_r = std::tanh(echo_r);

                    // 2. Filtro Damping LP Analógico de 1-Pólo (Escurece as repetições gradualmente)
                    float damp_alpha = 1.0f - std::clamp(echo_out_damping, 0.0f, 0.95f) * 0.70f;
                    lp_filter_l += damp_alpha * (sat_l - lp_filter_l);
                    lp_filter_r += damp_alpha * (sat_r - lp_filter_r);

                    // 3. DC Blocker em 80 Hz para eliminar embolamento e sub-grave desnecessário no delay
                    float dc_l = lp_filter_l - prev_dc_in_l + 0.995f * prev_dc_out_l;
                    float dc_r = lp_filter_r - prev_dc_in_r + 0.995f * prev_dc_out_r;
                    prev_dc_in_l = lp_filter_l;
                    prev_dc_in_r = lp_filter_r;
                    prev_dc_out_l = dc_l;
                    prev_dc_out_r = dc_r;

                    // 4. Feedback musical seguro (clamp em 0.88f impede oscilação infinita/buzz estridente)
                    float eff_fb = std::clamp(echo_out_feedback, 0.10f, 0.88f);
                    float fed_l = dc_l * eff_fb;
                    float fed_r = dc_r * eff_fb;

                    // IMPORTANTE: Alimenta o buffer APENAS com o feedback congelado da cauda!
                    // Sem injetar áudio novo da pista ao vivo para não embolar nem distorcer
                    delay_buf_l[delay_write_pos] = fed_l;
                    delay_buf_r[delay_write_pos] = fed_r;
                    delay_write_pos = (delay_write_pos + 1) % buf_sz;

                    // 5. Crossfade suave com curva Equal-Power (25ms): zero clique, zero distorção
                    if (echo_crossfade_pos < 1.0f) {
                        echo_crossfade_pos = std::min(1.0f, echo_crossfade_pos + echo_crossfade_step);
                    }
                    float dry_gain = std::cos(echo_crossfade_pos * 1.5707963f);
                    float wet_gain = std::sin(echo_crossfade_pos * 1.5707963f);

                    float out_l = l * dry_gain + dc_l * (wet_gain * 0.85f);
                    float out_r = r * dry_gain + dc_r * (wet_gain * 0.85f);

                    buffer_l[i] = std::clamp(out_l, -1.0f, 1.0f);
                    buffer_r[i] = std::clamp(out_r, -1.0f, 1.0f);
                } else if (!echo_out_active.load() && buf_sz > 0) {
                    // Buffer circular contínuo durante reprodução normal (mantém buffer cheio pronto para o disparo)
                    delay_buf_l[delay_write_pos] = l;
                    delay_buf_r[delay_write_pos] = r;
                    delay_write_pos = (delay_write_pos + 1) % buf_sz;
                }

                // Cálculo de medição
                float max_sample = std::max(std::abs(l), std::abs(r));
                peak_sq = std::max(peak_sq, max_sample * max_sample);

                float mono_sample = (l + r) * 0.5f;
                sum_sq += mono_sample * mono_sample;

                m_history_l[m_write_idx] = l;
                m_history_r[m_write_idx] = r;
                m_write_idx = (m_write_idx + 1) % m_window_size;
            }

            // Estimativa rápida de LUFS integrado e Peak dB
            float rms = std::sqrt(sum_sq / (float)frames);
            float lufs_est = (rms > 1e-5f) ? (20.0f * std::log10(rms) - 0.691f) : -70.0f;
            float peak_val = std::sqrt(peak_sq);
            float peak_db = (peak_val > 1e-5f) ? (20.0f * std::log10(peak_val)) : -70.0f;

            current_lufs.store(lufs_est);
            current_true_peak_db.store(peak_db);

            // Alerta de Redline (Distorção perigosa para os sistemas de som do clube)
            is_redlining.store(peak_db > -0.1f || lufs_est > -6.0f);
        }
    };

} // namespace KuroAudio

