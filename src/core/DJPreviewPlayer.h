#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "DJCommonUtils.h"
#include "../ai/dr_wav.h"
#include "../ai/dr_mp3.h"

namespace KuroAudio {

    class DJPreviewPlayer {
    private:
        std::vector<float> m_buffer_l;
        std::vector<float> m_buffer_r;
        std::atomic<size_t> m_read_pos{ 0 };
        double m_read_pos_frac{ 0.0 };
        std::atomic<bool> m_is_playing{ false };
        std::atomic<bool> m_is_loading{ false };
        std::atomic<float> m_volume{ 0.85f };
        double m_duration_sec = 30.0;
        double m_sample_rate = 44100.0;
        
        std::string m_current_id;
        std::string m_current_title;
        std::string m_current_artist;
        std::string m_current_url;
        
        std::vector<float> m_waveform_peaks;
        std::mutex m_mutex;

        static std::vector<float> computePeaks(const std::vector<float>& buf_l, const std::vector<float>& buf_r) {
            const int NUM_BARS = 64;
            std::vector<float> peaks(NUM_BARS, 0.2f);
            size_t total = buf_l.size();
            if (total < (size_t)NUM_BARS) return peaks;
            
            size_t step = total / NUM_BARS;
            float max_peak = 0.001f;
            
            for (int b = 0; b < NUM_BARS; b++) {
                size_t start = b * step;
                size_t end = std::min(start + step, total);
                float sum_sq = 0.0f;
                int count = 0;
                for (size_t i = start; i < end; i += 4) {
                    float s = (buf_l[i] + buf_r[i]) * 0.5f;
                    sum_sq += s * s;
                    count++;
                }
                float rms = (count > 0) ? std::sqrt(sum_sq / (float)count) : 0.0f;
                peaks[b] = rms;
                if (rms > max_peak) max_peak = rms;
            }
            
            for (int b = 0; b < NUM_BARS; b++) {
                float norm = peaks[b] / max_peak;
                peaks[b] = std::clamp(norm * 0.85f + 0.15f, 0.15f, 1.0f);
            }
            return peaks;
        }

    public:
        DJPreviewPlayer() = default;
        ~DJPreviewPlayer() {
            stop();
        }

        static DJPreviewPlayer& getInstance() {
            static DJPreviewPlayer instance;
            return instance;
        }

        bool isPlaying() const { return m_is_playing.load(); }
        bool isLoading() const { return m_is_loading.load(); }
        float getVolume() const { return m_volume.load(); }
        std::string getCurrentTrackId() const { return m_current_id; }
        std::string getCurrentTitle() const { return m_current_title; }
        std::string getCurrentArtist() const { return m_current_artist; }
        std::string getCurrentUrl() const { return m_current_url; }
        std::string getCurrentChannel() const { return m_current_artist; }
        
        std::vector<float> getWaveformPeaks() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_waveform_peaks;
        }

        void setVolume(float vol) {
            m_volume.store(std::clamp(vol, 0.0f, 1.0f));
        }

        void stop() {
            m_is_playing.store(false);
            m_is_loading.store(false);
            m_read_pos.store(0);
            std::lock_guard<std::mutex> lock(m_mutex);
            m_read_pos_frac = 0.0;
        }

        void togglePlayPause() {
            if (m_is_playing.load()) {
                m_is_playing.store(false);
                std::cout << "[DJ Preview Player] PAUSED (" << m_current_title << ")" << std::endl;
            } else {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_buffer_l.empty()) {
                    m_is_playing.store(true);
                    std::cout << "[DJ Preview Player] PLAYING (" << m_current_title << ")" << std::endl;
                }
            }
        }

        void seek(double target_sec) {
            std::lock_guard<std::mutex> lock(m_mutex);
            target_sec = std::clamp(target_sec, 0.0, m_duration_sec);
            double sr = (m_sample_rate > 1000.0 ? m_sample_rate : 44100.0);
            double frame = target_sec * sr;
            if (frame >= (double)m_buffer_l.size()) {
                frame = m_buffer_l.empty() ? 0.0 : (double)(m_buffer_l.size() - 1);
            }
            m_read_pos_frac = frame;
            m_read_pos.store((size_t)frame);
            std::cout << "[DJ Preview Player] SEEK to " << target_sec << "s (frame " << (size_t)frame << ")" << std::endl;
        }

        double getPosition() {
            size_t pos = m_read_pos.load();
            double sr = (m_sample_rate > 1000.0) ? m_sample_rate : 44100.0;
            return (double)pos / sr;
        }

        double getDuration() const {
            return (m_duration_sec > 0.0) ? m_duration_sec : 30.0;
        }

        // Mixa o áudio do preview diretamente no barramento Master com Resampling Linear Perfeito (Anti Pitch+)
        void processBlock(float* out_l, float* out_r, unsigned int frames, double host_sample_rate = 48000.0) {
            if (!m_is_playing.load()) return;
            
            std::unique_lock<std::mutex> lock(m_mutex, std::try_to_lock);
            if (!lock.owns_lock()) return;

            size_t buf_sz = m_buffer_l.size();
            if (buf_sz < 2) {
                m_is_playing.store(false);
                return;
            }

            double cur = m_read_pos_frac;
            float vol = m_volume.load();
            double h_sr = (host_sample_rate > 1000.0) ? host_sample_rate : 48000.0;
            double t_sr = (m_sample_rate > 1000.0) ? m_sample_rate : 44100.0;
            double step = t_sr / h_sr; // Resampling exato: elimina 100% de pitch+

            for (unsigned int i = 0; i < frames; i++) {
                if (cur >= (double)(buf_sz - 1)) {
                    m_is_playing.store(false);
                    cur = 0.0;
                    m_read_pos.store(0);
                    break;
                }

                size_t idx0 = (size_t)cur;
                size_t idx1 = std::min(idx0 + 1, buf_sz - 1);
                float frac = (float)(cur - (double)idx0);

                float s_l = m_buffer_l[idx0] * (1.0f - frac) + m_buffer_l[idx1] * frac;
                float s_r = m_buffer_r[idx0] * (1.0f - frac) + m_buffer_r[idx1] * frac;

                out_l[i] += s_l * vol;
                out_r[i] += s_r * vol;

                cur += step;
            }
            m_read_pos_frac = cur;
            m_read_pos.store((size_t)cur);
        }

        void play(const std::string& url_or_path, const std::string& track_id, const std::string& title, const std::string& artist) {
            if (url_or_path.empty() && track_id.empty()) return;

            // Se for a mesma faixa, alterna pause/play
            if (m_current_id == track_id && !m_buffer_l.empty()) {
                togglePlayPause();
                return;
            }

            stop();

            m_current_id = track_id;
            m_current_title = title;
            m_current_artist = artist;
            m_current_url = url_or_path;
            m_duration_sec = 30.0;
            m_is_loading.store(true);

            std::cout << "[DJ Preview Player] Solicitada reprodução: " << title << " (" << track_id << ")" << std::endl;

            std::thread([this, url_or_path, track_id, title]() {
                std::error_code ec;
                std::string wav_to_read;

                auto tracks_dir = getTracksDirectory();
                auto tools_dir = getToolsDirectory();
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto ffmpeg_path = getFfmpegPath();

                // 1. Checa se já existe arquivo WAV local baixado
                std::filesystem::path local_wav = tracks_dir / ("track_" + track_id + ".wav");
                std::filesystem::path alt_wav = tracks_dir / (track_id + ".wav");
                std::filesystem::path preview_wav = tracks_dir / ("preview_" + track_id + ".wav");
                std::filesystem::path local_file = std::filesystem::u8path(url_or_path);

                if (std::filesystem::exists(local_file, ec) && local_file.extension() == ".wav" && std::filesystem::file_size(local_file, ec) > 50000) {
                    wav_to_read = local_file.string();
                } else if (std::filesystem::exists(local_wav, ec) && std::filesystem::file_size(local_wav, ec) > 50000) {
                    wav_to_read = local_wav.string();
                } else if (std::filesystem::exists(alt_wav, ec) && std::filesystem::file_size(alt_wav, ec) > 50000) {
                    wav_to_read = alt_wav.string();
                } else if (std::filesystem::exists(preview_wav, ec) && std::filesystem::file_size(preview_wav, ec) > 20000) {
                    wav_to_read = preview_wav.string();
                } else if (url_or_path.rfind("http", 0) == 0) {
                    // É uma URL remota (YouTube, SoundCloud ou iTunes m4a)
                    std::filesystem::create_directories(tracks_dir, ec);
                    
                    if (url_or_path.find("youtube.com") != std::string::npos || url_or_path.find("youtu.be") != std::string::npos || url_or_path.find("soundcloud.com") != std::string::npos) {
                        // Snippet de áudio via worker
                        std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" preview_source --url \"" 
                            + std::filesystem::path(url_or_path).wstring() + L"\" --out_wav \"" + preview_wav.wstring() + L"\"";
                        RunHiddenProcess(wcmd);
                    } else {
                        // Download direto de m4a / mp3 via python urllib e conversão ffmpeg
                        std::filesystem::path tmp_m4a = tracks_dir / ("prev_tmp_" + track_id + ".m4a");
                        std::string py_cmd = "import urllib.request; req = urllib.request.Request('" + url_or_path + "', headers={'User-Agent': 'Mozilla/5.0'}); open('" + tmp_m4a.generic_string() + "', 'wb').write(urllib.request.urlopen(req, timeout=10).read())";
                        std::wstring wpy_cmd = L"\"" + py_path.wstring() + L"\" -c \"" + std::wstring(py_cmd.begin(), py_cmd.end()) + L"\"";
                        RunHiddenProcess(wpy_cmd);

                        if (std::filesystem::exists(tmp_m4a, ec) && std::filesystem::file_size(tmp_m4a, ec) > 5000) {
                            std::wstring wconv_cmd = L"\"" + ffmpeg_path.wstring() + L"\" -y -i \"" + tmp_m4a.wstring() + L"\" -ar 44100 -ac 2 \"" + preview_wav.wstring() + L"\"";
                            RunHiddenProcess(wconv_cmd);
                            std::filesystem::remove(tmp_m4a, ec);
                        }
                    }

                    if (std::filesystem::exists(preview_wav, ec) && std::filesystem::file_size(preview_wav, ec) > 20000) {
                        wav_to_read = preview_wav.string();
                    }
                }

                // Carrega PCM decodificado diretamente em memória (suporta WAV e MP3)
                if (!wav_to_read.empty()) {
                    unsigned int channels = 0;
                    unsigned int sr = 0;
                    drwav_uint64 total_pcm_frames = 0;
                    float* pSampleData = nullptr;
                    bool is_mp3_decoded = false;

#ifdef _WIN32
                    try {
                        std::filesystem::path p = std::filesystem::u8path(wav_to_read);
                        pSampleData = drwav_open_file_and_read_pcm_frames_f32_w(p.wstring().c_str(), &channels, &sr, &total_pcm_frames, NULL);
                    } catch (...) {}
#endif
                    if (!pSampleData) {
                        pSampleData = drwav_open_file_and_read_pcm_frames_f32(wav_to_read.c_str(), &channels, &sr, &total_pcm_frames, NULL);
                    }

                    // Se drwav falhar diretamente, lê em buffer de memória para suporte a MP3 e paths complexos
                    if (!pSampleData || total_pcm_frames == 0) {
                        try {
                            std::error_code ec;
                            std::filesystem::path p = std::filesystem::u8path(wav_to_read);
                            if (std::filesystem::exists(p, ec)) {
                                std::ifstream f(p, std::ios::binary | std::ios::ate);
                                if (f.is_open()) {
                                    std::streamsize sz = f.tellg();
                                    if (sz > 4096) {
                                        f.seekg(0, std::ios::beg);
                                        std::vector<char> fbuf((size_t)sz);
                                        if (f.read(fbuf.data(), sz)) {
                                            pSampleData = drwav_open_memory_and_read_pcm_frames_f32(fbuf.data(), fbuf.size(), &channels, &sr, &total_pcm_frames, NULL);
                                            if (!pSampleData || total_pcm_frames == 0) {
                                                drmp3_config mp3_cfg;
                                                drmp3_uint64 mp3_frames = 0;
                                                pSampleData = drmp3_open_memory_and_read_pcm_frames_f32(fbuf.data(), fbuf.size(), &mp3_cfg, &mp3_frames, NULL);
                                                if (pSampleData && mp3_frames > 0) {
                                                    channels = mp3_cfg.channels;
                                                    sr = mp3_cfg.sampleRate;
                                                    total_pcm_frames = mp3_frames;
                                                    is_mp3_decoded = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        } catch (...) {}
                    }

                    if (pSampleData && total_pcm_frames > 0) {
                        double srate = (double)sr;
                        double dur = (double)total_pcm_frames / srate;

                        std::vector<float> tmp_l(total_pcm_frames);
                        std::vector<float> tmp_r(total_pcm_frames);

                        if (channels == 1) {
                            for (size_t i = 0; i < total_pcm_frames; i++) {
                                tmp_l[i] = pSampleData[i];
                                tmp_r[i] = pSampleData[i];
                            }
                        } else {
                            for (size_t i = 0; i < total_pcm_frames; i++) {
                                tmp_l[i] = pSampleData[i * channels];
                                tmp_r[i] = pSampleData[i * channels + 1];
                            }
                        }

                        if (is_mp3_decoded) {
                            drmp3_free(pSampleData, NULL);
                        } else {
                            drwav_free(pSampleData, NULL);
                        }

                        auto tmp_peaks = computePeaks(tmp_l, tmp_r);

                        // Swap instantâneo em lock ultra-curto (sub-microsegundo)
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            m_buffer_l = std::move(tmp_l);
                            m_buffer_r = std::move(tmp_r);
                            m_waveform_peaks = std::move(tmp_peaks);
                            m_sample_rate = srate;
                            m_duration_sec = dur;
                            m_read_pos.store(0);
                            m_read_pos_frac = 0.0;
                            m_is_playing.store(true);
                        }

                        std::cout << "[DJ Preview Player] Reproduzindo PCM: " << title << " (" << dur << "s, " << total_pcm_frames << " frames, " << sr << "Hz)" << std::endl;
                    } else {
                        std::cout << "[DJ Preview Player] Falha ao decodificar PCM de: " << wav_to_read << std::endl;
                    }
                } else {
                    std::cout << "[DJ Preview Player] Nenhum áudio encontrado/baixado para: " << title << " (" << url_or_path << ")" << std::endl;
                }

                m_is_loading.store(false);
            }).detach();
        }

        void playDirectBuffer(std::vector<float> tmp_l, std::vector<float> tmp_r, double srate, const std::string& title, const std::string& artist = "Abduction Academy") {
            stop();
            m_current_id = "procedural_demo";
            m_current_title = title;
            m_current_artist = artist;
            m_current_url = "";
            double sr = (srate > 1000.0 ? srate : 44100.0);
            double dur = tmp_l.empty() ? 0.0 : ((double)tmp_l.size() / sr);
            auto tmp_peaks = computePeaks(tmp_l, tmp_r);
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_buffer_l = std::move(tmp_l);
                m_buffer_r = std::move(tmp_r);
                m_waveform_peaks = std::move(tmp_peaks);
                m_sample_rate = sr;
                m_duration_sec = dur;
                m_read_pos.store(0);
                m_read_pos_frac = 0.0;
                m_is_playing.store(true);
            }
            std::cout << "[DJ Preview Player] Procedural demo playing: " << title << " (" << dur << "s)" << std::endl;
        }
    };

} // namespace KuroAudio
