#pragma once
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <map>
#include <iostream>
#include <complex>

#define PI 3.14159265358979323846

namespace KuroMIR {

    // Simples Radix-2 FFT In-Place (Tamanho da Janela deve ser potencia de 2, ex: 8192)
    inline void computeFFT(std::vector<std::complex<float>>& x) {
        int N = x.size();
        if (N <= 1) return;

        // Bit-reversal
        for (int i = 1, j = 0; i < N; i++) {
            int bit = N >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(x[i], x[j]);
        }

        // Cooley-Tukey
        for (int len = 2; len <= N; len <<= 1) {
            float angle = -2.0f * PI / len;
            std::complex<float> wlen(std::cos(angle), std::sin(angle));
            for (int i = 0; i < N; i += len) {
                std::complex<float> w(1.0f, 0.0f);
                for (int j = 0; j < len / 2; j++) {
                    std::complex<float> u = x[i + j];
                    std::complex<float> v = x[i + j + len / 2] * w;
                    x[i + j] = u + v;
                    x[i + j + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }
    }

    // Calcula o Tom (Campo Harmonico) aproximado usando Chromagram
    inline std::string estimateKey(const std::vector<float>& audio_mono, int sampleRate) {
        if (audio_mono.empty()) return "Unknown";

        // Usar primeiros 10 segundos para estimativa
        int frames = std::min((int)audio_mono.size(), sampleRate * 10);
        
        int N = 8192; // Tamanho da janela FFT
        if (frames < N) return "Unknown";

        std::vector<float> chroma(12, 0.0f);
        
        for (int offset = 0; offset < frames - N; offset += N) {
            std::vector<std::complex<float>> x(N);
            // Aplicar janela de Hann
            for (int i = 0; i < N; i++) {
                float multiplier = 0.5f * (1.0f - std::cos(2.0f * PI * i / (N - 1)));
                x[i] = std::complex<float>(audio_mono[offset + i] * multiplier, 0.0f);
            }
            
            computeFFT(x);
            
            // Converter FFT bins em Chromas (A4 = 440Hz)
            for (int i = 1; i < N / 2; i++) {
                float freq = (float)i * sampleRate / N;
                if (freq < 20.0f || freq > 4000.0f) continue;
                
                // Qual semitom do A4 (440Hz)
                float pitch = 12.0f * std::log2(freq / 440.0f);
                int pitch_class = ((int)std::round(pitch) % 12 + 12) % 12; // 0 = A, 1 = A#, 2 = B, 3 = C, etc.
                
                float magnitude = std::abs(x[i]);
                chroma[pitch_class] += magnitude;
            }
        }

        // Normalizar chroma
        float max_val = 0.0f;
        for (float v : chroma) if (v > max_val) max_val = v;
        if (max_val > 0.0f) {
            for (float& v : chroma) v /= max_val;
        }

        // Perfis de Krumhansl-Schmuckler (simplificados)
        // Indices: 0=A, 1=A#, 2=B, 3=C, 4=C#, 5=D, 6=D#, 7=E, 8=F, 9=F#, 10=G, 11=G#
        const char* noteNames[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
        
        // Perfil Maior (0=root, 2, 4, 5, 7, 9, 11)
        float majorProfile[12] = {6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f, 2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f};
        // Perfil Menor (0=root, 2, 3, 5, 7, 8, 10)
        float minorProfile[12] = {6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f, 2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f};

        float best_corr = -1.0f;
        std::string best_key = "Unknown";

        // Correlacionar com todas as 24 chaves
        for (int root = 0; root < 12; root++) {
            float corr_major = 0.0f;
            float corr_minor = 0.0f;
            for (int i = 0; i < 12; i++) {
                int ch_idx = (root + i) % 12;
                corr_major += chroma[ch_idx] * majorProfile[i];
                corr_minor += chroma[ch_idx] * minorProfile[i];
            }
            if (corr_major > best_corr) {
                best_corr = corr_major;
                best_key = std::string(noteNames[root]) + " Maior";
            }
            if (corr_minor > best_corr) {
                best_corr = corr_minor;
                best_key = std::string(noteNames[root]) + " Menor";
            }
        }

        return best_key;
    }

    // Calcula BPM Brutal (Procura picos graves usando envelope)
    inline float estimateBPM(const std::vector<float>& audio_mono, int sampleRate) {
        if (audio_mono.empty()) return 0.0f;

        int window_size = sampleRate / 100; // 10ms windows
        std::vector<float> energy;
        
        // Passa um filtro passa-baixa simples para isolar o Kick
        float prev = 0.0f;
        std::vector<float> kick_audio(audio_mono.size());
        float alpha = 0.05f; // Frequencia de corte beeeem baixa
        for (size_t i = 0; i < audio_mono.size(); i++) {
            prev += alpha * (audio_mono[i] - prev);
            kick_audio[i] = prev;
        }

        for (size_t i = 0; i + window_size < kick_audio.size(); i += window_size) {
            float sum = 0.0f;
            for (int j = 0; j < window_size; j++) {
                sum += kick_audio[i + j] * kick_audio[i + j];
            }
            energy.push_back(sum);
        }

        // Calcula derivada de energia
        std::vector<float> diff(energy.size(), 0.0f);
        for (size_t i = 1; i < energy.size(); i++) {
            diff[i] = std::max(0.0f, energy[i] - energy[i - 1]);
        }

        // Encontra picos
        std::vector<int> peaks;
        for (size_t i = 1; i < diff.size() - 1; i++) {
            if (diff[i] > diff[i - 1] && diff[i] > diff[i + 1] && diff[i] > 0.01f) {
                peaks.push_back(i);
            }
        }

        if (peaks.size() < 2) return 0.0f;

        // Histograma de distancias
        std::map<int, int> intervals;
        for (size_t i = 1; i < peaks.size(); i++) {
            int interval = peaks[i] - peaks[i - 1];
            // interval em janelas de 10ms. 
            // 60 / (interval * 0.01) = BPM
            // Ex: se interval = 50 (500ms), BPM = 120
            if (interval > 20 && interval < 150) { // BPM entre 40 e 300
                intervals[interval]++;
            }
        }

        int best_interval = -1;
        int max_count = 0;
        for (auto const& [interval, count] : intervals) {
            // Peso para intervalos mais "razoáveis" (BPM em torno de 100-140)
            float bpm_approx = 60.0f / (interval * 0.01f);
            float weight = 1.0f;
            if (bpm_approx > 90.0f && bpm_approx < 140.0f) weight = 1.5f;
            
            if (count * weight > max_count) {
                max_count = count * weight;
                best_interval = interval;
            }
        }

        if (best_interval <= 0) return 120.0f; // Default estendido
        
        float bpm = 60.0f / (best_interval * (window_size / (float)sampleRate));
        return std::round(bpm);
    }
}
