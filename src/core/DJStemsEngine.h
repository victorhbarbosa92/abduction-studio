#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <atomic>
#include "../ai/dr_wav.h"
#include "../ai/dr_mp3.h"

namespace KuroAudio {

    enum DJStemType {
        STEM_VOCALS = 0,
        STEM_DRUMS = 1,
        STEM_BASS = 2,
        STEM_INST = 3,
        STEM_COUNT = 4
    };

    struct DJDeckStemSlot {
        bool is_loaded = false;
        std::vector<float> buffer_l;
        std::vector<float> buffer_r;
        float volume = 1.0f;
        bool is_muted = false;
        bool is_solo = false;
        float current_fade = 1.0f; // Interpolação suave para evitar estalos (cliques) ao mutar
    };

    class DJStemsEngine {
    private:
        DJDeckStemSlot deck_stems[4][STEM_COUNT];
        bool real_stems_available[4] = { false, false, false, false };

        static bool loadWavFile(const std::string& path, std::vector<float>& out_l, std::vector<float>& out_r) {
            drwav wav;
            if (!drwav_init_file(&wav, path.c_str(), NULL)) return false;

            size_t total_samples = (size_t)wav.totalPCMFrameCount * wav.channels;
            std::vector<float> temp(total_samples);
            drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, temp.data());

            out_l.resize((size_t)wav.totalPCMFrameCount);
            out_r.resize((size_t)wav.totalPCMFrameCount);

            if (wav.channels == 1) {
                for (size_t i = 0; i < (size_t)wav.totalPCMFrameCount; i++) {
                    out_l[i] = temp[i];
                    out_r[i] = temp[i];
                }
            } else {
                for (size_t i = 0; i < (size_t)wav.totalPCMFrameCount; i++) {
                    out_l[i] = temp[i * wav.channels];
                    out_r[i] = temp[i * wav.channels + 1];
                }
            }
            drwav_uninit(&wav);
            return true;
        }

    public:
        DJStemsEngine() = default;

        static DJStemsEngine& getInstance() {
            static DJStemsEngine instance;
            return instance;
        }

        bool hasRealStems(int deck_idx) const {
            if (deck_idx >= 0 && deck_idx < 4) return real_stems_available[deck_idx];
            return false;
        }

        // Tenta encontrar e carregar os 4 arquivos de Stems gerados por IA para a faixa
        bool loadStemsForTrack(int deck_idx, const std::string& track_path) {
            if (deck_idx < 0 || deck_idx >= 4 || track_path.empty()) return false;

            unloadDeck(deck_idx);

            std::filesystem::path p(track_path);
            std::string stem_stem = p.stem().string();
            std::filesystem::path dir = p.parent_path();

            // Padrões de busca por Stems pré-extraídos
            std::filesystem::path paths[STEM_COUNT] = {
                dir / (stem_stem + "_vocals.wav"),
                dir / (stem_stem + "_drums.wav"),
                dir / (stem_stem + "_bass.wav"),
                dir / (stem_stem + "_inst.wav")
            };

            // Também checa em pasta dedicada scratch/stems/
            std::filesystem::path cache_dir = "scratch/stems";
            if (!std::filesystem::exists(paths[0]) && std::filesystem::exists(cache_dir)) {
                paths[0] = cache_dir / (stem_stem + "_vocals.wav");
                paths[1] = cache_dir / (stem_stem + "_drums.wav");
                paths[2] = cache_dir / (stem_stem + "_bass.wav");
                paths[3] = cache_dir / (stem_stem + "_inst.wav");
            }

            int loaded_count = 0;
            for (int s = 0; s < STEM_COUNT; s++) {
                if (std::filesystem::exists(paths[s])) {
                    if (loadWavFile(paths[s].string(), deck_stems[deck_idx][s].buffer_l, deck_stems[deck_idx][s].buffer_r)) {
                        deck_stems[deck_idx][s].is_loaded = true;
                        deck_stems[deck_idx][s].volume = 1.0f;
                        deck_stems[deck_idx][s].is_muted = false;
                        deck_stems[deck_idx][s].is_solo = false;
                        deck_stems[deck_idx][s].current_fade = 1.0f;
                        loaded_count++;
                    }
                }
            }

            real_stems_available[deck_idx] = (loaded_count == STEM_COUNT);
            return real_stems_available[deck_idx];
        }

        void unloadDeck(int deck_idx) {
            if (deck_idx < 0 || deck_idx >= 4) return;
            for (int s = 0; s < STEM_COUNT; s++) {
                deck_stems[deck_idx][s].is_loaded = false;
                deck_stems[deck_idx][s].buffer_l.clear();
                deck_stems[deck_idx][s].buffer_r.clear();
            }
            real_stems_available[deck_idx] = false;
        }

        void setStemVolume(int deck_idx, DJStemType stem, float vol) {
            if (deck_idx >= 0 && deck_idx < 4 && stem >= 0 && stem < STEM_COUNT) {
                deck_stems[deck_idx][stem].volume = std::clamp(vol, 0.0f, 1.5f);
            }
        }

        void toggleStemMute(int deck_idx, DJStemType stem) {
            if (deck_idx >= 0 && deck_idx < 4 && stem >= 0 && stem < STEM_COUNT) {
                deck_stems[deck_idx][stem].is_muted = !deck_stems[deck_idx][stem].is_muted;
            }
        }

        void setStemSolo(int deck_idx, DJStemType stem) {
            if (deck_idx < 0 || deck_idx >= 4 || stem < 0 || stem >= STEM_COUNT) return;
            bool was_solo = deck_stems[deck_idx][stem].is_solo;
            for (int s = 0; s < STEM_COUNT; s++) {
                deck_stems[deck_idx][s].is_solo = false;
                deck_stems[deck_idx][s].is_muted = false;
            }
            if (!was_solo) {
                deck_stems[deck_idx][stem].is_solo = true;
                for (int s = 0; s < STEM_COUNT; s++) {
                    if (s != stem) deck_stems[deck_idx][s].is_muted = true;
                }
            }
        }

        // Processa e mistura os 4 stems sincronizados para o frame atual do deck
        bool processRealStemsSample(int deck_idx, size_t frame, float& out_l, float& out_r) {
            if (!real_stems_available[deck_idx]) return false;

            float sum_l = 0.0f;
            float sum_r = 0.0f;

            for (int s = 0; s < STEM_COUNT; s++) {
                auto& slot = deck_stems[deck_idx][s];
                if (!slot.is_loaded || frame >= slot.buffer_l.size()) continue;

                // Transição suave de fade (5ms) ao mutar/desmutar
                float target_fade = slot.is_muted ? 0.0f : 1.0f;
                slot.current_fade += (target_fade - slot.current_fade) * 0.05f;

                if (slot.current_fade > 0.001f) {
                    float g = slot.volume * slot.current_fade;
                    sum_l += slot.buffer_l[frame] * g;
                    sum_r += slot.buffer_r[frame] * g;
                }
            }

            out_l = sum_l;
            out_r = sum_r;
            return true;
        }
    };

} // namespace KuroAudio
