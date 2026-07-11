#pragma once
#include <vector>
#include <cmath>
#include <string>

namespace KuroAI {

class KuroSpectralExtractor {
public:
    // Separação no Domínio do Tempo (Transient vs Sustain) para separar FX Percussivos de Harmônicos Contínuos
    static void separateHarmonicPercussive(
        const std::vector<float>& input, 
        std::vector<float>& out_harmonic, 
        std::vector<float>& out_percussive,
        float sampleRate = 44100.0f) 
    {
        out_harmonic.clear();
        out_harmonic.reserve(input.size());
        out_percussive.clear();
        out_percussive.reserve(input.size());

        float env = 0.0f;
        float attack_coef = std::exp(-1.0f / (0.005f * sampleRate)); // 5ms attack
        float release_coef = std::exp(-1.0f / (0.100f * sampleRate)); // 100ms release

        float prev_env = 0.0f;
        float smooth_perc = 0.0f;

        for (size_t i = 0; i < input.size(); i += 2) {
            float in_l = input[i];
            float in_r = i + 1 < input.size() ? input[i+1] : in_l;
            
            float mono = (std::abs(in_l) + std::abs(in_r)) * 0.5f;

            if (mono > env) {
                env = attack_coef * env + (1.0f - attack_coef) * mono;
            } else {
                env = release_coef * env + (1.0f - release_coef) * mono;
            }

            float diff = env - prev_env;
            prev_env = env;

            float perc_gain = 0.0f;
            
            // Se o envelope sobe repentinamente (transiente = Zaps, Squelches, Kicks)
            if (diff > 0.001f) { 
                perc_gain = 1.0f;
            }

            // Suavização do ganho para não pipocar (click) o áudio
            smooth_perc = 0.99f * smooth_perc + 0.01f * perc_gain;

            out_percussive.push_back(in_l * smooth_perc);
            out_percussive.push_back(in_r * smooth_perc);

            out_harmonic.push_back(in_l * (1.0f - smooth_perc));
            out_harmonic.push_back(in_r * (1.0f - smooth_perc));
        }
    }

    // Auto-Labeller (Inteligência de Nomenclatura para Psytrance)
    static std::string autoLabel(const std::vector<float>& buffer, float sampleRate) {
        if (buffer.empty()) return "VAZIO";
        
        int zcr = 0; // Zero Crossing Rate
        float energy = 0.0f;
        
        // Avalia apenas uma amostra da track para velocidade (os primeiros 5 segundos)
        size_t samples_to_check = std::min(buffer.size(), (size_t)(sampleRate * 5.0f * 2.0f));
        
        if (samples_to_check < 100) return "SILENCIO";
        
        for (size_t i = 1; i < samples_to_check; i++) {
            if (buffer[i] * buffer[i-1] < 0.0f) zcr++;
            energy += buffer[i] * buffer[i];
        }
        
        float zcr_rate = (float)zcr / (float)samples_to_check;
        energy /= (float)samples_to_check;

        if (energy < 0.000001f) return "SILENCIO";

        if (zcr_rate > 0.12f) return "ZAPS / SQUELCHES (FX)"; // Som de alta frequencia com muitos transientes
        if (zcr_rate > 0.05f) return "SYNTH / FM LEAD"; // Frequencias medias
        if (zcr_rate > 0.01f) return "ATMOS / DRONE / PAD"; // Frequencias mais baixas
        
        return "BASS / SUB"; 
    }
};

}
