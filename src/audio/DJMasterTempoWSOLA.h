#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    // =========================================================================
    // DJMasterTempoWSOLA: Algoritmo de Time-Stretching de Alta Fidelidade (WSOLA)
    // Preserva a afinação tonal e o punch de transientes (Kicks, Snares, Baixo)
    // sem o efeito metálico de flanger/fase gerado por pitch shifters granulares simples.
    // =========================================================================
    class DJMasterTempoWSOLA {
    private:
        static const int WINDOW_SIZE = 1024;    // ~21ms a 48kHz
        static const int HOP_SIZE = 256;        // 75% de sobreposição (Overlap)
        static const int SEARCH_RANGE = 256;    // Janela de busca de similaridade de fase

        std::vector<float> window_func;
        std::vector<float> in_buf_l, in_buf_r;
        std::vector<float> out_accum_l, out_accum_r;
        std::vector<float> win_accum;

        int in_write_pos = 0;
        double in_read_pos = 0.0;
        int out_read_pos = 0;
        int out_write_pos = 0;

        double tempo_ratio = 1.0; // 1.0 = normal, 1.08 = +8%, 0.92 = -8%
        bool is_active = false;

        void initHanningWindow() {
            window_func.assign(WINDOW_SIZE, 0.0f);
            for (int i = 0; i < WINDOW_SIZE; i++) {
                // Janela de Hanning periódica suave
                window_func[i] = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * (float)i / (float)WINDOW_SIZE));
            }
        }

        // Busca o deslocamento de máxima similaridade de onda (Cross-Correlation de pico)
        int findBestOverlapOffset(int target_pos) {
            float max_corr = -1e9f;
            int best_offset = 0;

            int buf_sz = (int)in_buf_l.size();
            int base_ref = (target_pos + buf_sz) % buf_sz;

            // Busca na vizinhança de ±SEARCH_RANGE
            for (int delta = -SEARCH_RANGE; delta <= SEARCH_RANGE; delta += 2) {
                int cand_pos = (target_pos + delta + buf_sz) % buf_sz;
                float corr = 0.0f;

                // Amostragem rápida em intervalos de 4 samples para alta performance no loop
                for (int i = 0; i < HOP_SIZE; i += 4) {
                    int r_idx = (base_ref + i) % buf_sz;
                    int c_idx = (cand_pos + i) % buf_sz;
                    float s_ref = in_buf_l[r_idx] + in_buf_r[r_idx];
                    float s_cand = in_buf_l[c_idx] + in_buf_r[c_idx];
                    corr += s_ref * s_cand;
                }

                if (corr > max_corr) {
                    max_corr = corr;
                    best_offset = delta;
                }
            }
            return best_offset;
        }

    public:
        DJMasterTempoWSOLA() {
            initHanningWindow();
            int max_storage = 65536;
            in_buf_l.assign(max_storage, 0.0f);
            in_buf_r.assign(max_storage, 0.0f);
            out_accum_l.assign(max_storage, 0.0f);
            out_accum_r.assign(max_storage, 0.0f);
            win_accum.assign(max_storage, 0.0f);
            reset();
        }

        void reset() {
            std::fill(in_buf_l.begin(), in_buf_l.end(), 0.0f);
            std::fill(in_buf_r.begin(), in_buf_r.end(), 0.0f);
            std::fill(out_accum_l.begin(), out_accum_l.end(), 0.0f);
            std::fill(out_accum_r.begin(), out_accum_r.end(), 0.0f);
            std::fill(win_accum.begin(), win_accum.end(), 0.0f);
            in_write_pos = WINDOW_SIZE;
            in_read_pos = 0.0;
            out_read_pos = 0;
            out_write_pos = WINDOW_SIZE;
        }

        void setTempoRatio(double ratio) {
            tempo_ratio = std::clamp(ratio, 0.5, 2.0);
            is_active = (std::abs(tempo_ratio - 1.0) > 0.001);
        }

        double getTempoRatio() const {
            return tempo_ratio;
        }

        bool isActive() const {
            return is_active;
        }

        // Processa bloco de áudio estéreo com preservação de tom (Keylock)
        void process(const float* in_l, const float* in_r, float* out_l, float* out_r, unsigned int frames) {
            if (!is_active || std::abs(tempo_ratio - 1.0) <= 0.0005) {
                // Pass-through direto bit-perfect se a velocidade for 1.0
                std::memcpy(out_l, in_l, frames * sizeof(float));
                std::memcpy(out_r, in_r, frames * sizeof(float));
                return;
            }

            int buf_sz = (int)in_buf_l.size();

            // 1. Armazena novos samples de entrada
            for (unsigned int i = 0; i < frames; i++) {
                in_buf_l[in_write_pos] = in_l[i];
                in_buf_r[in_write_pos] = in_r[i];
                in_write_pos = (in_write_pos + 1) % buf_sz;
            }

            // 2. Loop de síntese WSOLA Overlap-Add
            while (out_write_pos - out_read_pos < (int)frames + WINDOW_SIZE) {
                int target_pos = (int)in_read_pos;
                int best_offset = findBestOverlapOffset(target_pos);
                int synthesis_in_pos = (target_pos + best_offset + buf_sz) % buf_sz;

                // Sobrepõe e soma o grão com a janela de Hanning
                for (int w = 0; w < WINDOW_SIZE; w++) {
                    int in_idx = (synthesis_in_pos + w) % buf_sz;
                    int out_idx = (out_write_pos + w) % buf_sz;
                    float win = window_func[w];

                    out_accum_l[out_idx] += in_buf_l[in_idx] * win;
                    out_accum_r[out_idx] += in_buf_r[in_idx] * win;
                    win_accum[out_idx] += win;
                }

                in_read_pos += (double)HOP_SIZE * tempo_ratio;
                if (in_read_pos >= (double)buf_sz) in_read_pos -= (double)buf_sz;
                out_write_pos += HOP_SIZE;
            }

            // 3. Normalização e extração dos samples de saída
            for (unsigned int i = 0; i < frames; i++) {
                int idx = out_read_pos % buf_sz;
                float norm = win_accum[idx];
                if (norm > 0.001f) {
                    out_l[i] = out_accum_l[idx] / norm;
                    out_r[i] = out_accum_r[idx] / norm;
                } else {
                    out_l[i] = out_accum_l[idx];
                    out_r[i] = out_accum_r[idx];
                }

                // Limpa acumulador para os próximos ciclos
                out_accum_l[idx] = 0.0f;
                out_accum_r[idx] = 0.0f;
                win_accum[idx] = 0.0f;
                out_read_pos++;
            }
        }
    };

} // namespace KuroAudio
