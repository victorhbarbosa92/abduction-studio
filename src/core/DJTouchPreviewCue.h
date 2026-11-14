#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <fstream>
#include "DJAudioRouting.h"
#include "DJCommonUtils.h"
#include "../ai/dr_wav.h"
#include "../ai/dr_mp3.h"

namespace KuroAudio {

    // =========================================================================
    // DJTouchPreviewCue: Inspirado na Pioneer CDJ-3000
    // 1. Touch Preview: Tocar qualquer ponto de uma música da lista direto no fone
    //    sem precisar carregar em nenhum deck, com controle total (volume, scrub, pause).
    // 2. Touch Cue: Ouvir trechos futuros ou passados do deck tocando na pista
    //    sem alterar a agulha da pista (Master)!
    // =========================================================================
    class DJTouchPreviewCue {
    private:
        // 1. Estado do Touch Preview (Biblioteca)
        std::atomic<bool> preview_active{ false };
        std::atomic<bool> preview_paused{ false };
        std::atomic<bool> preview_route_master{ false };
        std::vector<float> preview_buf_l;
        std::vector<float> preview_buf_r;
        double preview_current_frame = 0.0;
        double preview_sample_rate = 44100.0;
        std::string preview_loaded_path;
        std::string preview_track_id;
        std::string preview_track_title;
        std::string preview_track_artist;
        std::atomic<float> preview_volume{ 0.85f };
        std::mutex preview_mutex;

        // 2. Estado do Touch Cue por Deck (Deck 0..3)
        std::atomic<bool> touch_cue_active[4]{ {false}, {false}, {false}, {false} };
        double touch_cue_virtual_frame[4]{ 0.0, 0.0, 0.0, 0.0 };
        std::atomic<float> touch_cue_volume{ 0.90f };

    public:
        DJTouchPreviewCue() = default;

        static DJTouchPreviewCue& getInstance() {
            static DJTouchPreviewCue instance;
            return instance;
        }

        std::string getPreviewTrackId() const {
            return preview_track_id;
        }

        std::string getPreviewTrackTitle() const {
            return preview_track_title;
        }

        std::string getPreviewTrackArtist() const {
            return preview_track_artist;
        }

        // =====================================================================
        // TOUCH PREVIEW (Biblioteca)
        // =====================================================================
        bool startTouchPreview(const std::string& audio_path, const std::string& track_id, const std::string& title = "", const std::string& artist = "", double progress_0_to_1 = 0.0) {
            preview_track_id = track_id;
            preview_track_title = title;
            preview_track_artist = artist;
            return startTouchPreview(audio_path, progress_0_to_1);
        }

        bool startTouchPreview(const std::string& audio_path, double progress_0_to_1) {
            std::lock_guard<std::mutex> lock(preview_mutex);

            std::string resolved = KuroAudio::resolveExistingAudioPath(audio_path);
            if (resolved.empty()) resolved = audio_path;

            if (resolved != preview_loaded_path || preview_buf_l.empty()) {
                preview_buf_l.clear();
                preview_buf_r.clear();
                preview_sample_rate = 44100.0;

                bool loaded = false;

                // 1. Tenta decodificar como WAV com suporte a wide-char no Windows
#ifdef _WIN32
                try {
                    drwav wav;
                    auto wp = std::filesystem::u8path(resolved);
                    if (drwav_init_file_w(&wav, wp.wstring().c_str(), NULL)) {
                        preview_sample_rate = (double)wav.sampleRate;
                        size_t total_frames = (size_t)wav.totalPCMFrameCount;
                        if (total_frames > 0) {
                            std::vector<float> temp(total_frames * wav.channels);
                            drwav_read_pcm_frames_f32(&wav, total_frames, temp.data());

                            preview_buf_l.resize(total_frames);
                            preview_buf_r.resize(total_frames);

                            if (wav.channels == 1) {
                                for (size_t i = 0; i < total_frames; i++) {
                                    preview_buf_l[i] = temp[i];
                                    preview_buf_r[i] = temp[i];
                                }
                            } else {
                                for (size_t i = 0; i < total_frames; i++) {
                                    preview_buf_l[i] = temp[i * wav.channels];
                                    preview_buf_r[i] = temp[i * wav.channels + 1];
                                }
                            }
                            loaded = true;
                        }
                        drwav_uninit(&wav);
                    }
                } catch (...) {}
#endif
                if (!loaded) {
                    drwav wav;
                    if (drwav_init_file(&wav, resolved.c_str(), NULL)) {
                        preview_sample_rate = (double)wav.sampleRate;
                        size_t total_frames = (size_t)wav.totalPCMFrameCount;
                        if (total_frames > 0) {
                            std::vector<float> temp(total_frames * wav.channels);
                            drwav_read_pcm_frames_f32(&wav, total_frames, temp.data());

                            preview_buf_l.resize(total_frames);
                            preview_buf_r.resize(total_frames);

                            if (wav.channels == 1) {
                                for (size_t i = 0; i < total_frames; i++) {
                                    preview_buf_l[i] = temp[i];
                                    preview_buf_r[i] = temp[i];
                                }
                            } else {
                                for (size_t i = 0; i < total_frames; i++) {
                                    preview_buf_l[i] = temp[i * wav.channels];
                                    preview_buf_r[i] = temp[i * wav.channels + 1];
                                }
                            }
                            loaded = true;
                        }
                        drwav_uninit(&wav);
                    }
                }

                // 2. Fallback para MP3 via dr_mp3 caso WAV falhe
                if (!loaded) {
                    try {
                        std::error_code ec;
                        auto p = std::filesystem::u8path(resolved);
                        if (std::filesystem::exists(p, ec)) {
                            std::ifstream f(p, std::ios::binary | std::ios::ate);
                            if (f.is_open()) {
                                std::streamsize sz = f.tellg();
                                if (sz > 1024) {
                                    f.seekg(0, std::ios::beg);
                                    std::vector<char> fbuf((size_t)sz);
                                    if (f.read(fbuf.data(), sz)) {
                                        drmp3_config mp3_cfg;
                                        drmp3_uint64 mp3_frames = 0;
                                        float* pSampleData = drmp3_open_memory_and_read_pcm_frames_f32(fbuf.data(), fbuf.size(), &mp3_cfg, &mp3_frames, NULL);
                                        if (pSampleData && mp3_frames > 0) {
                                            preview_sample_rate = (double)mp3_cfg.sampleRate;
                                            preview_buf_l.resize((size_t)mp3_frames);
                                            preview_buf_r.resize((size_t)mp3_frames);

                                            if (mp3_cfg.channels == 1) {
                                                for (size_t i = 0; i < (size_t)mp3_frames; i++) {
                                                    preview_buf_l[i] = pSampleData[i];
                                                    preview_buf_r[i] = pSampleData[i];
                                                }
                                            } else {
                                                for (size_t i = 0; i < (size_t)mp3_frames; i++) {
                                                    preview_buf_l[i] = pSampleData[i * mp3_cfg.channels];
                                                    preview_buf_r[i] = pSampleData[i * mp3_cfg.channels + 1];
                                                }
                                            }
                                            drmp3_free(pSampleData, NULL);
                                            loaded = true;
                                        }
                                    }
                                }
                            }
                        }
                    } catch (...) {}
                }

                if (loaded) {
                    preview_loaded_path = resolved;
                } else {
                    return false;
                }
            }

            if (!preview_buf_l.empty()) {
                double target_frame = progress_0_to_1 * (double)preview_buf_l.size();
                preview_current_frame = std::clamp(target_frame, 0.0, (double)preview_buf_l.size() - 1.0);
                preview_paused.store(false);
                preview_active.store(true);
                return true;
            }
            return false;
        }

        void stopTouchPreview() {
            preview_active.store(false);
            preview_paused.store(false);
        }

        void togglePlayPause() {
            if (!preview_active.load()) {
                if (!preview_buf_l.empty()) {
                    preview_active.store(true);
                    preview_paused.store(false);
                }
            } else {
                preview_paused.store(!preview_paused.load());
            }
        }

        bool isPaused() const {
            return preview_paused.load();
        }

        bool isTouchPreviewActive() const {
            return preview_active.load();
        }

        double getPreviewProgress() const {
            if (preview_active.load() && !preview_buf_l.empty()) {
                return std::clamp(preview_current_frame / (double)preview_buf_l.size(), 0.0, 1.0);
            }
            return 0.0;
        }

        double getPreviewDurationSec() const {
            if (!preview_buf_l.empty() && preview_sample_rate > 0.0) {
                return (double)preview_buf_l.size() / preview_sample_rate;
            }
            return 0.0;
        }

        double getPreviewCurrentSec() const {
            if (!preview_buf_l.empty() && preview_sample_rate > 0.0) {
                return preview_current_frame / preview_sample_rate;
            }
            return 0.0;
        }

        void seekPreview(double progress_0_to_1) {
            std::lock_guard<std::mutex> lock(preview_mutex);
            if (!preview_buf_l.empty()) {
                double target_frame = progress_0_to_1 * (double)preview_buf_l.size();
                preview_current_frame = std::clamp(target_frame, 0.0, (double)preview_buf_l.size() - 1.0);
                preview_active.store(true);
            }
        }

        void jumpSeconds(double delta_sec) {
            std::lock_guard<std::mutex> lock(preview_mutex);
            if (!preview_buf_l.empty() && preview_sample_rate > 0.0) {
                double delta_frames = delta_sec * preview_sample_rate;
                preview_current_frame = std::clamp(preview_current_frame + delta_frames, 0.0, (double)preview_buf_l.size() - 1.0);
                preview_active.store(true);
            }
        }

        float getPreviewVolume() const {
            return preview_volume.load();
        }

        void setPreviewVolume(float vol) {
            preview_volume.store(std::clamp(vol, 0.0f, 1.5f));
        }

        bool isRouteToMaster() const {
            return preview_route_master.load();
        }

        void setRouteToMaster(bool to_master) {
            preview_route_master.store(to_master);
        }

        // =====================================================================
        // TOUCH CUE (Pré-escuta do Deck no Ar)
        // =====================================================================
        void startTouchCue(int deck_idx, double target_frame) {
            if (deck_idx >= 0 && deck_idx < 4) {
                touch_cue_virtual_frame[deck_idx] = target_frame;
                touch_cue_active[deck_idx].store(true);
            }
        }

        void stopTouchCue(int deck_idx) {
            if (deck_idx >= 0 && deck_idx < 4) {
                touch_cue_active[deck_idx].store(false);
            }
        }

        void stopTouchCue() {
            for (int i = 0; i < 4; i++) {
                touch_cue_active[i].store(false);
            }
        }

        bool isTouchCueActive(int deck_idx) const {
            if (deck_idx >= 0 && deck_idx < 4) {
                return touch_cue_active[deck_idx].load();
            }
            return false;
        }

        bool isTouchCueActive() const {
            for (int i = 0; i < 4; i++) {
                if (touch_cue_active[i].load()) return true;
            }
            return false;
        }

        // =====================================================================
        // Processamento de Áudio no Barramento de Fone de Ouvido (CUE)
        // Injeta o Touch Preview e o Touch Cue exclusivamente no sinal de fone!
        // =====================================================================
        template <typename TDeck>
        void renderToHeadphones(float* cue_out_l, float* cue_out_r, unsigned int frames, double sr, const TDeck decks[4]) {
            const std::vector<float>* deck_bufs_l[4] = { &decks[0].buffer_l, &decks[1].buffer_l, &decks[2].buffer_l, &decks[3].buffer_l };
            const std::vector<float>* deck_bufs_r[4] = { &decks[0].buffer_r, &decks[1].buffer_r, &decks[2].buffer_r, &decks[3].buffer_r };
            double deck_speeds[4] = { 1.0, 1.0, 1.0, 1.0 };
            renderToHeadphones(cue_out_l, cue_out_r, frames, deck_bufs_l, deck_bufs_r, deck_speeds);
        }

        void renderToHeadphones(float* cue_out_l, float* cue_out_r, unsigned int frames,
                                const std::vector<float>* deck_buffers_l[4],
                                const std::vector<float>* deck_buffers_r[4],
                                double deck_playback_speeds[4]) {
            // 1. Injeta Touch Preview da Biblioteca
            if (preview_active.load() && !preview_buf_l.empty()) {
                float vol = preview_volume.load();
                size_t total_frames = preview_buf_l.size();
                bool paused = preview_paused.load();

                for (unsigned int i = 0; i < frames; i++) {
                    size_t idx = (size_t)preview_current_frame;
                    if (idx < total_frames) {
                        if (!paused) {
                            cue_out_l[i] += preview_buf_l[idx] * vol;
                            cue_out_r[i] += preview_buf_r[idx] * vol;
                            preview_current_frame += 1.0;
                        }
                    } else {
                        preview_active.store(false);
                        break;
                    }
                }
            }

            // 2. Injeta Touch Cue dos Decks
            for (int d = 0; d < 4; d++) {
                if (touch_cue_active[d].load() && deck_buffers_l[d] && deck_buffers_r[d]) {
                    const auto& d_l = *deck_buffers_l[d];
                    const auto& d_r = *deck_buffers_r[d];
                    size_t total_f = d_l.size();
                    float tc_vol = touch_cue_volume.load();
                    double speed = (deck_playback_speeds != nullptr) ? deck_playback_speeds[d] : 1.0;

                    for (unsigned int i = 0; i < frames; i++) {
                        size_t idx = (size_t)touch_cue_virtual_frame[d];
                        if (idx < total_f) {
                            cue_out_l[i] += d_l[idx] * tc_vol;
                            cue_out_r[i] += d_r[idx] * tc_vol;
                            touch_cue_virtual_frame[d] += speed;
                        } else {
                            touch_cue_active[d].store(false);
                            break;
                        }
                    }
                }
            }
        }
    };

} // namespace KuroAudio
