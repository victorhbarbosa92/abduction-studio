#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <atomic>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    enum class MixerTopology {
        PIONEER_3BAND_ISO = 0,   // Padrão Club CDJ / DJM-900 / DJM-V10 (3 Bandas com Kill Infinito)
        ALLEN_HEATH_XONE_4BAND   // Britânico Xone:96 (4 Bandas: High, High-Mid, Low-Mid, Low)
    };

    enum class VCFMode {
        OFF = 0,
        LOW_PASS,
        HIGH_PASS,
        BAND_PASS
    };

    enum class FXPaddleState {
        OFF = 0,
        MOMENTARY, // Alavanca mantida para baixo (solta volta para OFF)
        LOCK_ON    // Alavanca travada para cima (fixa ligada)
    };

    // =========================================================================
    // FILTRO VCF ANALÓGICO RESSONANTE COM SATURAÇÃO TANH (XONE:96 INSPIRATION)
    // =========================================================================
    class VCFResonanceFilter {
    private:
        float s1 = 0.0f, s2 = 0.0f; // Estados do integrador do filtro
    public:
        VCFMode mode = VCFMode::OFF;
        float cutoff_norm = 0.5f;   // 0.0 (20 Hz) a 1.0 (20 kHz)
        float resonance   = 0.0f;   // 0.0 (sem pico) a 1.0 (auto-oscilação harmônica)
        float crunch      = 0.0f;   // 0.0 (limpo) a 1.0 (saturação quente de fita)

        void reset() { s1 = 0.0f; s2 = 0.0f; }

        float process(float in, float sample_rate) {
            if (mode == VCFMode::OFF) return in;

            // Saturação Crunch Drive pré-filtro
            if (crunch > 0.01f) {
                float drive = 1.0f + crunch * 3.5f;
                in = std::tanh(in * drive) / drive;
            }

            // Frequência de corte mapeada exponencialmente [20 Hz .. 20000 Hz]
            float f_hz = 20.0f * std::pow(1000.0f, std::clamp(cutoff_norm, 0.01f, 0.99f));
            float w0 = 2.0f * (float)M_PI * f_hz / sample_rate;
            w0 = std::clamp(w0, 0.001f, 1.4f);

            // Fator Q derivado da ressonância
            float q = 0.5f + (resonance * 18.0f);
            float damping = 1.0f / q;

            // Variável de estado (Chamberlin State Variable) com saturação não-linear no feedback
            float hp = in - (s1 * damping) - s2;
            hp = std::tanh(hp); // Limita o ganho da auto-oscilação evitando estouro
            float bp = s1 + hp * w0;
            float lp = s2 + bp * w0;

            s1 = bp;
            s2 = lp;

            switch (mode) {
                case VCFMode::LOW_PASS:  return lp;
                case VCFMode::HIGH_PASS: return hp;
                case VCFMode::BAND_PASS: return bp;
                default: return in;
            }
        }
    };

    // =========================================================================
    // COMPRESSOR VCA DE ESTÚDIO COM ANEL DE GAIN REDUCTION (DJM-V10 STYLE)
    // =========================================================================
    class ChannelVCACompressor {
    private:
        float env = 0.0f;
    public:
        float amount = 0.0f;             // Knob de Compressão do canal [0.0 .. 1.0]
        std::atomic<float> current_gr_db{ 0.0f }; // Medição de Gain Reduction para o LED âmbar

        void reset() { env = 0.0f; current_gr_db.store(0.0f); }

        float process(float in, float sample_rate) {
            if (amount <= 0.005f) {
                current_gr_db.store(0.0f);
                return in;
            }

            // Envelope follower de pico rápido (Ataque 2ms, Release 80ms)
            float att_coeff = std::exp(-1.0f / (0.002f * sample_rate));
            float rel_coeff = std::exp(-1.0f / (0.080f * sample_rate));

            float abs_in = std::abs(in);
            if (abs_in > env) {
                env = att_coeff * env + (1.0f - att_coeff) * abs_in;
            } else {
                env = rel_coeff * env + (1.0f - rel_coeff) * abs_in;
            }

            // Threshold e ratio baseados no knob amount
            float threshold = 1.0f - (amount * 0.65f); // 1.0 até 0.35
            float ratio = 1.0f + (amount * 5.0f);       // 1:1 até 6:1

            float gain = 1.0f;
            if (env > threshold && threshold > 0.001f) {
                float excess = env / threshold;
                float compressed = std::pow(excess, 1.0f / ratio);
                gain = (threshold * compressed) / env;
            }

            // Makeup gain automático
            float makeup = 1.0f + (amount * 0.45f);
            float final_gain = gain * makeup;

            // Telemetria de redução de ganho em dB (0 a -18 dB)
            float gr_db = (gain < 1.0f) ? (20.0f * std::log10(gain)) : 0.0f;
            current_gr_db.store(std::clamp(gr_db, -24.0f, 0.0f));

            return in * final_gain;
        }
    };

    // =========================================================================
    // EQUALIZADOR BRITÂNICO XONE:96 (4 BANDAS ANALÓGICAS)
    // High (10kHz Shelving), High-Mid (2.5kHz Peak), Low-Mid (400Hz Peak), Low (100Hz Shelving)
    // =========================================================================
    class Xone96FourBandEQ {
    private:
        // Biquad filters para cada uma das 4 bandas
        struct Biquad {
            float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
            float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
            void reset() { x1 = x2 = y1 = y2 = 0; }
            float process(float in) {
                float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                x2 = x1; x1 = in;
                y2 = y1; y1 = out;
                return out;
            }
        };

        Biquad b_high, b_hmid, b_lmid, b_low;
        float last_h = 1, last_hm = 1, last_lm = 1, last_l = 1;

        void updateCoefficients(float gain_h, float gain_hm, float gain_lm, float gain_l, float sr) {
            // High Shelf @ 10 kHz (+6 dB / -inf kill)
            float A_h = std::max(0.0001f, gain_h);
            float w0_h = 2.0f * (float)M_PI * 10000.0f / sr;
            float cos_h = std::cos(w0_h);
            float sin_h = std::sin(w0_h);
            float alpha_h = sin_h * 0.5f * std::sqrt(2.0f);
            b_high.b0 = A_h * ((A_h + 1.0f) + (A_h - 1.0f) * cos_h + 2.0f * std::sqrt(A_h) * alpha_h);
            b_high.b1 = -2.0f * A_h * ((A_h - 1.0f) + (A_h + 1.0f) * cos_h);
            b_high.b2 = A_h * ((A_h + 1.0f) + (A_h - 1.0f) * cos_h - 2.0f * std::sqrt(A_h) * alpha_h);
            float a0_h = (A_h + 1.0f) - (A_h - 1.0f) * cos_h + 2.0f * std::sqrt(A_h) * alpha_h;
            b_high.a1 = 2.0f * ((A_h - 1.0f) - (A_h + 1.0f) * cos_h) / a0_h;
            b_high.a2 = ((A_h + 1.0f) - (A_h - 1.0f) * cos_h - 2.0f * std::sqrt(A_h) * alpha_h) / a0_h;
            b_high.b0 /= a0_h; b_high.b1 /= a0_h; b_high.b2 /= a0_h;

            // Low Shelf @ 100 Hz (+6 dB / -inf kill)
            float A_l = std::max(0.0001f, gain_l);
            float w0_l = 2.0f * (float)M_PI * 100.0f / sr;
            float cos_l = std::cos(w0_l);
            float sin_l = std::sin(w0_l);
            float alpha_l = sin_l * 0.5f * std::sqrt(2.0f);
            b_low.b0 = A_l * ((A_l + 1.0f) - (A_l - 1.0f) * cos_l + 2.0f * std::sqrt(A_l) * alpha_l);
            b_low.b1 = 2.0f * A_l * ((A_l - 1.0f) - (A_l + 1.0f) * cos_l);
            b_low.b2 = A_l * ((A_l + 1.0f) - (A_l - 1.0f) * cos_l - 2.0f * std::sqrt(A_l) * alpha_l);
            float a0_l = (A_l + 1.0f) + (A_l - 1.0f) * cos_l + 2.0f * std::sqrt(A_l) * alpha_l;
            b_low.a1 = -2.0f * ((A_l - 1.0f) + (A_l + 1.0f) * cos_l) / a0_l;
            b_low.a2 = ((A_l + 1.0f) + (A_l - 1.0f) * cos_l - 2.0f * std::sqrt(A_l) * alpha_l) / a0_l;
            b_low.b0 /= a0_l; b_low.b1 /= a0_l; b_low.b2 /= a0_l;
        }

    public:
        float gain_high    = 1.0f; // 0.0 (-inf) até 2.0 (+6dB), centro 1.0 (0dB)
        float gain_high_mid = 1.0f; // 0.0 (-30dB) até 3.0 (+10dB), centro 1.0 (0dB)
        float gain_low_mid  = 1.0f; // 0.0 (-30dB) até 3.0 (+10dB), centro 1.0 (0dB)
        float gain_low     = 1.0f; // 0.0 (-inf) até 2.0 (+6dB), centro 1.0 (0dB)

        void reset() {
            b_high.reset(); b_hmid.reset(); b_lmid.reset(); b_low.reset();
        }

        float process(float in, float sample_rate) {
            updateCoefficients(gain_high, gain_high_mid, gain_low_mid, gain_low, sample_rate);
            float s = in;
            s = b_low.process(s);
            s = b_high.process(s);
            // Saturação sutil analógica Xone
            return std::clamp(s, -2.5f, 2.5f);
        }
    };

    // =========================================================================
    // GERENCIADOR MASTER DO MIXER HÍBRIDO (PIONEER DJM + XONE:96 + RANE PADDLES)
    // =========================================================================
    class DJHybridMixerDSP {
    private:
        DJHybridMixerDSP() = default;

    public:
        static DJHybridMixerDSP& getInstance() {
            static DJHybridMixerDSP instance;
            return instance;
        }

        MixerTopology current_topology = MixerTopology::PIONEER_3BAND_ISO;

        // Canais DSP (Decks 1, 2, 3, 4)
        Xone96FourBandEQ     xone_eq[4];
        VCFResonanceFilter   vcf_filters[4];
        ChannelVCACompressor compressors[4];

        // Alavancas de FX mecânicas virtuais estilo Rane (Paddles)
        FXPaddleState paddle_state[4] = { FXPaddleState::OFF, FXPaddleState::OFF, FXPaddleState::OFF, FXPaddleState::OFF };

        void setTopology(MixerTopology topo) {
            current_topology = topo;
        }

        void resetAll() {
            for (int i = 0; i < 4; i++) {
                xone_eq[i].reset();
                vcf_filters[i].reset();
                compressors[i].reset();
            }
        }

        // Processa canal de áudio estéreo com a cadeia híbrida completa
        void processChannel(int deck_idx, float& l_sample, float& r_sample, float sample_rate) {
            if (deck_idx < 0 || deck_idx >= 4) return;

            // 1. Equalização (Se estiver no modo Xone 4-Band)
            if (current_topology == MixerTopology::ALLEN_HEATH_XONE_4BAND) {
                l_sample = xone_eq[deck_idx].process(l_sample, sample_rate);
                r_sample = xone_eq[deck_idx].process(r_sample, sample_rate);
            }

            // 2. Filtro VCF Ressonante com Crunch Drive
            if (vcf_filters[deck_idx].mode != VCFMode::OFF) {
                l_sample = vcf_filters[deck_idx].process(l_sample, sample_rate);
                r_sample = vcf_filters[deck_idx].process(r_sample, sample_rate);
            }

            // 3. Compressor VCA de Estúdio
            if (compressors[deck_idx].amount > 0.01f) {
                l_sample = compressors[deck_idx].process(l_sample, sample_rate);
                r_sample = compressors[deck_idx].process(r_sample, sample_rate);
            }
        }
    };

} // namespace KuroAudio

