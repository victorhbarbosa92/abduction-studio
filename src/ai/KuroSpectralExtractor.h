#pragma once
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include "../core/MidiNote.h"

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

    // Separação Espacial (Mid/Side) para isolar Vocais (Mid) de Reverbs e Backing Vocals (Side)
    static void separateMidSide(
        const std::vector<float>& input,
        std::vector<float>& out_mid,
        std::vector<float>& out_side)
    {
        out_mid.clear();
        out_mid.reserve(input.size());
        out_side.clear();
        out_side.reserve(input.size());

        for (size_t i = 0; i < input.size(); i += 2) {
            float in_l = input[i];
            float in_r = i + 1 < input.size() ? input[i+1] : in_l;
            
            // Mid = (L + R) / 2
            float mid = (in_l + in_r) * 0.5f;
            // Side = (L - R) / 2
            float side = (in_l - in_r) * 0.5f;
            
            // Reconstruct L/R for Mid (Mono in Center)
            out_mid.push_back(mid);
            out_mid.push_back(mid);
            
            // Reconstruct L/R for Side (Anti-phase)
            out_side.push_back(side);
            out_side.push_back(-side);
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

    // Algoritmo Audio-to-MIDI (Pitch Tracking via Autocorrelação Espectral)
    static std::vector<KuroDSP::MidiNote> convertAudioToMidi(
        const std::vector<float>& audio_samples,
        float sampleRate = 44100.0f,
        float sensitivity = 0.05f) 
    {
        std::vector<KuroDSP::MidiNote> notes;
        if (audio_samples.empty()) return notes;

        size_t window_size = 2048;
        size_t hop_size = 1024;
        
        int current_pitch = -1;
        float note_start_sec = 0.0f;
        float note_duration_sec = 0.0f;
        float note_velocity = 0.8f;

        for (size_t pos = 0; pos + window_size < audio_samples.size(); pos += hop_size) {
            float time_sec = (float)pos / sampleRate;

            // Compute RMS energy
            float rms = 0.0f;
            for (size_t i = 0; i < window_size; i++) {
                float s = audio_samples[pos + i];
                rms += s * s;
            }
            rms = std::sqrt(rms / window_size);

            if (rms < sensitivity) {
                if (current_pitch != -1) {
                    // Close active note
                    notes.push_back(KuroDSP::MidiNote(current_pitch, note_start_sec, std::max(0.1f, note_duration_sec), note_velocity));
                    current_pitch = -1;
                }
                continue;
            }

            // Autocorrelation Pitch Estimation
            int best_lag = 0;
            float max_autocorr = -1.0f;

            int min_lag = (int)(sampleRate / 1000.0f); // 1000 Hz limit (~ C6)
            int max_lag = (int)(sampleRate / 50.0f);   // 50 Hz limit (~ G1)

            for (int lag = min_lag; lag < max_lag && lag < (int)window_size / 2; lag++) {
                float autocorr = 0.0f;
                for (size_t i = 0; i < window_size / 2; i++) {
                    autocorr += audio_samples[pos + i] * audio_samples[pos + i + lag];
                }

                if (autocorr > max_autocorr) {
                    max_autocorr = autocorr;
                    best_lag = lag;
                }
            }

            int detected_pitch = -1;
            if (best_lag > 0) {
                float fundamental_freq = sampleRate / (float)best_lag;
                if (fundamental_freq >= 50.0f && fundamental_freq <= 1200.0f) {
                    float midi_val = 69.0f + 12.0f * std::log2(fundamental_freq / 440.0f);
                    detected_pitch = (int)std::round(midi_val);
                }
            }

            if (detected_pitch != -1) {
                if (current_pitch == -1) {
                    current_pitch = detected_pitch;
                    note_start_sec = time_sec;
                    note_duration_sec = (float)hop_size / sampleRate;
                    note_velocity = std::clamp(rms * 2.5f, 0.4f, 1.0f);
                } else if (std::abs(detected_pitch - current_pitch) <= 1) {
                    note_duration_sec += (float)hop_size / sampleRate;
                } else {
                    // Pitch shifted -> Emit note and start new note
                    notes.push_back(KuroDSP::MidiNote(current_pitch, note_start_sec, std::max(0.1f, note_duration_sec), note_velocity));
                    current_pitch = detected_pitch;
                    note_start_sec = time_sec;
                    note_duration_sec = (float)hop_size / sampleRate;
                }
            }
        }

        if (current_pitch != -1) {
            notes.push_back(KuroDSP::MidiNote(current_pitch, note_start_sec, std::max(0.1f, note_duration_sec), note_velocity));
        }

        return notes;
    }
};

}
