#pragma once
#include <cmath>
#include <algorithm>
#include "DJM10ChannelDSP.h"

namespace KuroAudio {

    // =========================================================================
    // DJMasterIsolator: Isolador Master de 3 Bandas Estilo Pioneer DJM-V10 / Rotary
    // - Faixa: -infinito (Kill Total) até +9dB de Boost
    // - Crossover Linkwitz-Riley alinhado em fase (Low 280Hz, High 3500Hz)
    // - Saturação suave analógica ao impulsionar para calor vintage de pista
    // =========================================================================
    class DJMasterIsolator {
    public:
        bool enabled = true;

        // Ganhos: 0.0f = Kill total (-inf), 1.0f = Flat (0dB), 2.818f = +9dB Boost
        float iso_low = 1.0f;
        float iso_mid = 1.0f;
        float iso_high = 1.0f;

    private:
        // Crossover 4ª ordem (Linkwitz-Riley cascata de 2 biquads Butterworth)
        DJV10Biquad lp1_low, lp2_low;
        DJV10Biquad bp1_mid, bp2_mid;
        DJV10Biquad hp1_high, hp2_high;

        float sample_rate = 44100.0f;

        // Saturação suave analógica tanh para boost acima de 0dB
        static inline float softSaturate(float x) {
            if (x > 1.2f) return 1.2f + std::tanh(x - 1.2f) * 0.4f;
            if (x < -1.2f) return -1.2f + std::tanh(x + 1.2f) * 0.4f;
            return x;
        }

    public:
        DJMasterIsolator() {
            setSampleRate(44100.0f);
        }

        void setSampleRate(float sr) {
            sample_rate = sr;
            // Crossover Low em 280 Hz
            lp1_low.setLowPass(280.0f, sr, 0.707f);
            lp2_low.setLowPass(280.0f, sr, 0.707f);

            // Crossover Mid Band-Pass (280Hz - 3500Hz)
            bp1_mid.setBandPass(1200.0f, sr, 0.60f);
            bp2_mid.setBandPass(1200.0f, sr, 0.60f);

            // Crossover High em 3500 Hz
            hp1_high.setHighPass(3500.0f, sr, 0.707f);
            hp2_high.setHighPass(3500.0f, sr, 0.707f);
        }

        void reset() {
            lp1_low.reset(); lp2_low.reset();
            bp1_mid.reset(); bp2_mid.reset();
            hp1_high.reset(); hp2_high.reset();
        }

        // Processa o barramento Master de áudio
        void process(float* master_l, float* master_r, unsigned int frames, float sr = 44100.0f) {
            if (!enabled) return;
            if (sr > 1000.0f && std::abs(sr - sample_rate) > 10.0f) {
                setSampleRate(sr);
            }

            // Se todas as bandas estiverem em 1.0 (neutro), não consome CPU
            if (std::abs(iso_low - 1.0f) < 0.001f && 
                std::abs(iso_mid - 1.0f) < 0.001f && 
                std::abs(iso_high - 1.0f) < 0.001f) {
                return;
            }

            for (unsigned int i = 0; i < frames; i++) {
                float in_l = master_l[i];
                float in_r = master_r[i];

                // 1. Banda Low (Graves)
                float l_low = in_l, r_low = in_r;
                lp1_low.process(l_low, r_low);
                lp2_low.process(l_low, r_low);

                // 2. Banda High (Agudos)
                float l_hi = in_l, r_hi = in_r;
                hp1_high.process(l_hi, r_hi);
                hp2_high.process(l_hi, r_hi);

                // 3. Banda Mid (Médios)
                float l_mid = in_l, r_mid = in_r;
                bp1_mid.process(l_mid, r_mid);
                bp2_mid.process(l_mid, r_mid);

                // 4. Soma com ganhos do Isolador
                float out_l = (l_low * iso_low) + (l_mid * iso_mid) + (l_hi * iso_high);
                float out_r = (r_low * iso_low) + (r_mid * iso_mid) + (r_hi * iso_high);

                // 5. Calor analógico se houver Boost
                if (iso_low > 1.0f || iso_mid > 1.0f || iso_high > 1.0f) {
                    out_l = softSaturate(out_l);
                    out_r = softSaturate(out_r);
                }

                master_l[i] = out_l;
                master_r[i] = out_r;
            }
        }
    };

} // namespace KuroAudio
