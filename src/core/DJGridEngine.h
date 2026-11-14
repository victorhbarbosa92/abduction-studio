#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <mutex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    struct DJBeatGridData {
        double first_beat_frame = 0.0;
        double bpm = 128.0;
        double grid_offset_ms = 0.0;
        bool is_locked = false;
        bool is_analyzed = false;
    };

    class DJGridEngine {
    private:
        DJBeatGridData deck_grids[4];
        std::vector<std::chrono::steady_clock::time_point> tap_history[4];
        std::mutex grid_mutex;

        // Metrônomo Sonoro de Referência
        bool metronome_enabled[4] = { false, false, false, false };
        float metronome_volume = 0.6f;

    public:
        DJGridEngine() = default;

        static DJGridEngine& getInstance() {
            static DJGridEngine instance;
            return instance;
        }

        DJBeatGridData getGridData(int deck_idx) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4) return deck_grids[deck_idx];
            return {};
        }

        void setGridData(int deck_idx, const DJBeatGridData& data) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4) {
                deck_grids[deck_idx] = data;
            }
        }

        // Define a posição da agulha atual como o Marco Zero (Beat 1.1 do Compasso) Absoluto (Padrão Pioneer Rekordbox)
        void setFirstBeatMarker(int deck_idx, double current_frame, double sample_rate = 44100.0) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                deck_grids[deck_idx].first_beat_frame = current_frame;
            }
        }

        // Micro-deslocamento temporal da grade com preservação temporal absoluta (Padrão Pioneer Rekordbox)
        void nudgeGrid(int deck_idx, double offset_ms, double sample_rate) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                double frames_delta = (offset_ms * 0.001) * sample_rate;
                deck_grids[deck_idx].first_beat_frame += frames_delta;
                deck_grids[deck_idx].grid_offset_ms += offset_ms;
            }
        }

        // Ajuste de BPM ancorado na posição da agulha atual (Padrão Pioneer Rekordbox)
        void adjustBpmAnchored(int deck_idx, double delta_bpm, double current_frame, double sample_rate) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                double old_bpm = deck_grids[deck_idx].bpm;
                double new_bpm = std::clamp(old_bpm + delta_bpm, 40.0, 260.0);
                new_bpm = std::round(new_bpm * 100.0) / 100.0;
                if (std::abs(new_bpm - old_bpm) > 0.0001 && sample_rate > 1000.0 && old_bpm > 20.0) {
                    double old_beat_len = 60.0 * sample_rate / old_bpm;
                    double new_beat_len = 60.0 * sample_rate / new_bpm;
                    double beat_idx = std::round((current_frame - deck_grids[deck_idx].first_beat_frame) / old_beat_len);
                    deck_grids[deck_idx].first_beat_frame = current_frame - (beat_idx * new_beat_len);
                }
                deck_grids[deck_idx].bpm = new_bpm;
            }
        }

        // Ajuste fino de BPM (ex: +0.01 ou -0.01)
        void adjustBpm(int deck_idx, double delta_bpm) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                double new_bpm = std::clamp(deck_grids[deck_idx].bpm + delta_bpm, 40.0, 260.0);
                deck_grids[deck_idx].bpm = std::round(new_bpm * 100.0) / 100.0;
            }
        }

        void setBpm(int deck_idx, double new_bpm) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                deck_grids[deck_idx].bpm = new_bpm;
            }
        }

        void halveBpm(int deck_idx) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                deck_grids[deck_idx].bpm = std::max(30.0, deck_grids[deck_idx].bpm * 0.5);
            }
        }

        void doubleBpm(int deck_idx) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4 && !deck_grids[deck_idx].is_locked) {
                deck_grids[deck_idx].bpm = std::min(300.0, deck_grids[deck_idx].bpm * 2.0);
            }
        }

        void toggleGridLock(int deck_idx) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx >= 0 && deck_idx < 4) {
                deck_grids[deck_idx].is_locked = !deck_grids[deck_idx].is_locked;
            }
        }

        // TAP Tempo inteligente
        void tapTempo(int deck_idx) {
            if (deck_idx < 0 || deck_idx >= 4) return;
            auto now = std::chrono::steady_clock::now();
            auto& hist = tap_history[deck_idx];

            if (!hist.empty()) {
                auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - hist.back()).count();
                if (diff > 2000) hist.clear();
            }

            hist.push_back(now);
            if (hist.size() > 8) hist.erase(hist.begin());

            if (hist.size() >= 3) {
                double total_ms = 0.0;
                for (size_t i = 1; i < hist.size(); i++) {
                    total_ms += std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(hist[i] - hist[i - 1]).count();
                }
                double avg_interval_sec = (total_ms / (hist.size() - 1)) / 1000.0;
                if (avg_interval_sec > 0.20 && avg_interval_sec < 2.0) {
                    double calculated_bpm = std::round((60.0 / avg_interval_sec) * 100.0) / 100.0;
                    std::lock_guard<std::mutex> lock(grid_mutex);
                    if (!deck_grids[deck_idx].is_locked) {
                        deck_grids[deck_idx].bpm = calculated_bpm;
                    }
                }
            }
        }

        // Metrônomo Sonoro
        void setMetronomeEnabled(int deck_idx, bool enabled) {
            if (deck_idx >= 0 && deck_idx < 4) metronome_enabled[deck_idx] = enabled;
        }

        bool isMetronomeEnabled(int deck_idx) const {
            if (deck_idx >= 0 && deck_idx < 4) return metronome_enabled[deck_idx];
            return false;
        }

        // Renderiza o clique do metrônomo no bloco de áudio se ativo (Pioneer Rekordbox / Ableton Standard)
        void renderMetronome(int deck_idx, double current_frame, double sample_rate, float* cue_l, float* cue_r, unsigned int frames) {
            if (deck_idx < 0 || deck_idx >= 4 || !metronome_enabled[deck_idx]) return;

            std::lock_guard<std::mutex> lock(grid_mutex);
            double bpm = deck_grids[deck_idx].bpm;
            if (bpm < 20.0 || sample_rate < 1000.0) return;

            double beat_len = (60.0 / bpm) * sample_rate;
            double first_f = deck_grids[deck_idx].first_beat_frame;
            int click_len_frames = (int)(sample_rate * 0.015); // 15ms de clique cirúrgico

            for (unsigned int i = 0; i < frames; i++) {
                double f = current_frame + i;
                if (f < first_f) continue;

                double rel_f = f - first_f;
                int64_t beat_num = (int64_t)std::floor(rel_f / beat_len);
                double beat_start_f = first_f + (double)beat_num * beat_len;
                double delta_f = f - beat_start_f;

                // O clique dispara estritamente NO frame do beat (delta_f >= 0) e decai exponencialmente
                if (delta_f >= 0.0 && delta_f < (double)click_len_frames) {
                    bool is_downbeat = (((beat_num % 4) + 4) % 4 == 0);
                    // Downbeat 1.1: 1800Hz com harmônico de 3600Hz para clareza em fones; Beats 2, 3, 4: 950Hz
                    float freq = is_downbeat ? 1800.0f : 950.0f;
                    float t = (float)delta_f / (float)sample_rate;
                    float env = std::exp(-t * 260.0f); // Decaimento analógico rápido e encorpado
                    float wave = std::sin(2.0f * (float)M_PI * freq * t);
                    if (is_downbeat) {
                        wave += 0.35f * std::sin(4.0f * (float)M_PI * freq * t);
                    }
                    float click_samp = wave * env * metronome_volume;

                    cue_l[i] += click_samp;
                    cue_r[i] += click_samp;
                }
            }
        }

        void processMetronome(int deck_idx, double current_frame, float* out_l, float* out_r, unsigned int frames, double sample_rate) {
            renderMetronome(deck_idx, current_frame, sample_rate, out_l, out_r, frames);
        }

        // Serialização de Grade em JSON
        std::string exportGridJson(int deck_idx) {
            std::lock_guard<std::mutex> lock(grid_mutex);
            if (deck_idx < 0 || deck_idx >= 4) return "{}";
            const auto& g = deck_grids[deck_idx];
            std::stringstream ss;
            ss << "{\n"
               << "  \"first_beat_frame\": " << g.first_beat_frame << ",\n"
               << "  \"bpm\": " << g.bpm << ",\n"
               << "  \"grid_offset_ms\": " << g.grid_offset_ms << ",\n"
               << "  \"is_locked\": " << (g.is_locked ? "true" : "false") << "\n"
               << "}";
            return ss.str();
        }
    };

} // namespace KuroAudio
