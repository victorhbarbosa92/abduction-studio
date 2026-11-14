#pragma once
#include <cmath>
#include <algorithm>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    enum class DJEqMode {
        PIONEER_3BAND = 0,   // Padrão CDJ/DJM clássico (Low 320Hz, Mid, High 2800Hz)
        DJM_V10_4BAND = 1     // Padrão Cirúrgico DJM-V10 (Low, Low-Mid, High-Mid, High)
    };

    // Biquad estéreo de precisão interna
    struct DJV10Biquad {
        float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float x1_l = 0.0f, x2_l = 0.0f, y1_l = 0.0f, y2_l = 0.0f;
        float x1_r = 0.0f, x2_r = 0.0f, y1_r = 0.0f, y2_r = 0.0f;

        void reset() {
            x1_l = x2_l = y1_l = y2_l = 0.0f;
            x1_r = x2_r = y1_r = y2_r = 0.0f;
        }

        void setLowPass(float cutoff, float sr, float q = 0.707f) {
            float w0 = 2.0f * (float)M_PI * cutoff / sr;
            float cos_w = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0_raw = (1.0f - cos_w) * 0.5f;
            float a0_raw = 1.0f + alpha;
            a0 = b0_raw / a0_raw;
            a1 = (1.0f - cos_w) / a0_raw;
            a2 = b0_raw / a0_raw;
            b1 = (-2.0f * cos_w) / a0_raw;
            b2 = (1.0f - alpha) / a0_raw;
        }

        void setHighPass(float cutoff, float sr, float q = 0.707f) {
            float w0 = 2.0f * (float)M_PI * cutoff / sr;
            float cos_w = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0_raw = (1.0f + cos_w) * 0.5f;
            float a0_raw = 1.0f + alpha;
            a0 = b0_raw / a0_raw;
            a1 = -(1.0f + cos_w) / a0_raw;
            a2 = b0_raw / a0_raw;
            b1 = (-2.0f * cos_w) / a0_raw;
            b2 = (1.0f - alpha) / a0_raw;
        }

        void setBandPass(float cutoff, float sr, float q = 1.0f) {
            float w0 = 2.0f * (float)M_PI * cutoff / sr;
            float cos_w = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float a0_raw = 1.0f + alpha;
            a0 = alpha / a0_raw;
            a1 = 0.0f;
            a2 = -alpha / a0_raw;
            b1 = (-2.0f * cos_w) / a0_raw;
            b2 = (1.0f - alpha) / a0_raw;
        }

        inline void process(float& l, float& r) {
            float y_l = a0 * l + a1 * x1_l + a2 * x2_l - b1 * y1_l - b2 * y2_l;
            x2_l = x1_l; x1_l = l; y2_l = y1_l; y1_l = y_l;
            l = y_l;

            float y_r = a0 * r + a1 * x1_r + a2 * x2_r - b1 * y1_r - b2 * y2_r;
            x2_r = x1_r; x1_r = r; y2_r = y1_r; y1_r = y_r;
            r = y_r;
        }
    };

    // =========================================================================
    // DJM10ChannelDSP: Módulo de Canal Inspirado no Mixer Pioneer DJM-V10
    // 1. Equalizador de 4 Bandas (Low <120Hz, Low-Mid, Hi-Mid, High >4.5kHz)
    // 2. Knob de Compressor Analógico Único para encorpar faixas fracas/antigas
    // =========================================================================
    class DJM10ChannelDSP {
    public:
        bool enabled = false;
        DJEqMode eq_mode = DJEqMode::DJM_V10_4BAND;

        // Ganhos de EQ (0.0 = Kill total, 1.0 = Neutro 12h, 2.0 = +6dB Boost)
        float eq_low = 1.0f;      // Sub-Bass (< 120Hz)
        float eq_low_mid = 1.0f;  // Punch / Kick & Snare (120Hz - 800Hz)
        float eq_hi_mid = 1.0f;   // Vocais & Melodias (800Hz - 4.5kHz)
        float eq_high = 1.0f;     // Ar / Chimbais (> 4.5kHz)

        // Compressor de Canal DJM-V10 (0.0 = Bypass, 1.0 = Máxima presença/encorpamento)
        float comp_amount = 0.0f;
        float compressor_amount = 0.0f;

    private:
        // Crossover 4-Bandas DJM-V10
        DJV10Biquad filter_sub_lp;   // 120 Hz Low-Pass
        DJV10Biquad filter_lowmid_bp;// 400 Hz Band-Pass
        DJV10Biquad filter_himid_bp; // 2400 Hz Band-Pass
        DJV10Biquad filter_hi_hp;    // 4500 Hz High-Pass

        // Detector de Envelope do Compressor
        float comp_envelope = 0.0f;
        float current_sample_rate = 44100.0f;

    public:
        DJM10ChannelDSP() {
            setSampleRate(44100.0f);
        }

        void setSampleRate(float sr) {
            current_sample_rate = sr;
            filter_sub_lp.setLowPass(120.0f, sr, 0.707f);
            filter_lowmid_bp.setBandPass(420.0f, sr, 0.85f);
            filter_himid_bp.setBandPass(2400.0f, sr, 0.85f);
            filter_hi_hp.setHighPass(4500.0f, sr, 0.707f);
        }

        void reset() {
            filter_sub_lp.reset();
            filter_lowmid_bp.reset();
            filter_himid_bp.reset();
            filter_hi_hp.reset();
            comp_envelope = 0.0f;
        }

        // Processa bloco de áudio estéreo com EQ 4-Bandas e Compressor DJM-V10
        void process(float* in_l, float* in_r, unsigned int frames, float sr = 44100.0f) {
            if (sr > 1000.0f && std::abs(sr - current_sample_rate) > 10.0f) {
                setSampleRate(sr);
            }
            float attack_coeff = std::exp(-1.0f / (current_sample_rate * 0.005f));  // 5ms Attack rápido
            float release_coeff = std::exp(-1.0f / (current_sample_rate * 0.080f)); // 80ms Release suave

            // Parâmetros dinâmicos do compressor calibrados pelo knob único
            float eff_comp = std::max(comp_amount, compressor_amount);
            float thresh_db = -24.0f * eff_comp; // Abaixa o threshold até -24dB
            float thresh_lin = std::pow(10.0f, thresh_db / 20.0f);
            float ratio = 1.0f + 3.5f * eff_comp; // 1:1 até 4.5:1
            float makeup_gain = 1.0f + 0.85f * eff_comp; // Ganho compensatório automático

            for (unsigned int i = 0; i < frames; i++) {
                float l = in_l[i];
                float r = in_r[i];

                // 1. COMPRESSOR ANALÓGICO DJM-V10
                if (eff_comp > 0.01f) {
                    float peak = std::max(std::abs(l), std::abs(r));
                    if (peak > comp_envelope) {
                        comp_envelope = comp_envelope * attack_coeff + peak * (1.0f - attack_coeff);
                    } else {
                        comp_envelope = comp_envelope * release_coeff + peak * (1.0f - release_coeff);
                    }

                    float gain_red = 1.0f;
                    if (comp_envelope > thresh_lin) {
                        float env_db = 20.0f * std::log10(std::max(1e-5f, comp_envelope));
                        float over_db = env_db - thresh_db;
                        float compressed_over_db = over_db / ratio;
                        float target_gain_db = compressed_over_db - over_db;
                        gain_red = std::pow(10.0f, target_gain_db / 20.0f);
                    }

                    l = l * gain_red * makeup_gain;
                    r = r * gain_red * makeup_gain;
                }

                // 2. EQUALIZADOR 4-BANDAS DJM-V10
                if (eq_mode == DJEqMode::DJM_V10_4BAND) {
                    float sub_l = l, sub_r = r;
                    filter_sub_lp.process(sub_l, sub_r);

                    float lm_l = l, lm_r = r;
                    filter_lowmid_bp.process(lm_l, lm_r);

                    float hm_l = l, hm_r = r;
                    filter_himid_bp.process(hm_l, hm_r);

                    float hi_l = l, hi_r = r;
                    filter_hi_hp.process(hi_l, hi_r);

                    l = (sub_l * eq_low) + (lm_l * eq_low_mid) + (hm_l * eq_hi_mid) + (hi_l * eq_high);
                    r = (sub_r * eq_low) + (lm_r * eq_low_mid) + (hm_r * eq_hi_mid) + (hi_r * eq_high);
                }

                in_l[i] = l;
                in_r[i] = r;
            }
        }
    };

} // namespace KuroAudio
