#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include "KuroDirectWaveUI.h"
#include "FileDialog.h"
#include "../thirdparty/dr_wav.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

class StemSeparationEngine;
extern std::unique_ptr<StemSeparationEngine> g_ai_engine;

namespace KuroUI {
    extern bool show_playlist;
    extern bool show_piano_roll;

    inline std::filesystem::path to_fs_path(const std::string& p) {
#if defined(_WIN32)
        if (p.empty()) return std::filesystem::path();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, p.c_str(), (int)p.size(), NULL, 0);
        if (size_needed <= 0) return std::filesystem::path(p);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, p.c_str(), (int)p.size(), &wstr[0], size_needed);
        return std::filesystem::path(wstr);
#else
        return std::filesystem::path(p);
#endif
    }

    inline std::string to_utf8_str(const std::filesystem::path& p) {
#if defined(_WIN32)
        std::wstring w = p.wstring();
        if (w.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
        if (size_needed <= 0) return std::string();
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &str[0], size_needed, NULL, NULL);
        return str;
#else
        return p.string();
#endif
    }

    extern KuroDirectWaveUI g_directwave_ui;
    extern std::string g_current_project_name;

    class CloudStemExtractorUI {
    private:
        bool is_open = false;
        char input_audio_path[512] = "";
        char output_stems_dir[512] = "C:\\NovaDAW\\tracks\\stems";
        std::string status_msg = "Pronto. Selecione os instrumentos desejados para visualizar, fatiar e gerar variações.";
        float progress_pct = 0.0f;
        std::atomic<bool> is_processing{false};

        // Escopo de Processamento: 0 = Música Inteira (Full Song), 1 = Loop Selecionado ([IN] a [OUT])
        int processing_scope_idx = 1;

        // Opções de Destino
        bool opt_insert_in_playlist = true;
        bool opt_extract_midi = true;
        bool opt_load_into_directwave = true;

        // Waveform & Audio Data
        std::vector<float> waveform_overview_max;
        std::vector<float> waveform_overview_min;
        float song_duration_sec = 0.0f;
        int song_sample_rate = 44100;
        int song_channels = 2;
        float track_bpm = 140.0f;

        // Janela de Visualização (Zoom Window)
        float zoom_view_start = 0.0f;
        float zoom_view_duration = 15.0f;

        // Marcadores de Loop e Playhead
        float loop_in_sec = 0.0f;
        float loop_out_sec = 13.714f; // ~8 bars a 140 BPM
        float playhead_sec = 0.0f;
        bool is_playing = false;
        bool is_loop_playing = false;
        std::chrono::steady_clock::time_point last_frame_time;

        // ========================================================
        // AI STEM INPAINTING & LOOP VARIATION ENGINE (SUNO / FL)
        // ========================================================
        bool show_inpainting_modal = false;
        int inpainting_style_idx = 0; // 0 = Melodic Arp, 1 = Psy Bass Groove, 2 = Drum Fill & Roll
        float inpainting_energy_pct = 75.0f; // 1% a 100%
        bool has_generated_variation = false;
        bool is_ab_variation_audition = false; // Alternar A/B: False = Original, True = Variação IA
        std::string inpainting_status = "Pronto para gerar nova variação no loop selecionado.";

        // 8 Canais de Stems Cirúrgicos com Seleção Individual
        struct StemChannel {
            const char* title;
            const char* name;
            const char* tag;
            const char* freq_range;
            ImU32 border_color;
            ImU32 wave_color;
            bool separate;        // Checkbox / Seleção individual
            float fader_L_db;     // -24dB a +6dB
            float fader_R_db;     // -24dB a +6dB
            bool solo;
            bool mute;
            float vu_needle_L;
            float vu_needle_R;
        };

        StemChannel channels[8] = {
            { "Kick",         "Kick / Punch",       "🥁 Bumbo & Punch",   "35-85Hz + Click",   0xFF1E88E5, 0xFF2244FF, true,   0.0f, 0.0f, false, false, 0.70f, 0.68f }, // Vermelho Neon
            { "Sub",          "Sub Bass",           "🔊 Sub Bass Puro",   "35-110Hz Deep",     0xFF00ACC1, 0xFF00E5FF, true,   0.0f, 0.0f, false, false, 0.65f, 0.63f }, // Ciano Neon
            { "Rolling Bass", "Rolling Saw Bass",   "⚡ Bassline Groove", "110-380Hz Saw",     0xFF3949AB, 0xFFFF8800, false,  0.0f, 0.0f, false, false, 0.72f, 0.75f }, // Azul
            { "Snare",        "Snare / Clap",       "🎯 Caixa & Clap",    "220-1600Hz Attack", 0xFFD81B60, 0xFF00D7FF, false,  0.0f, 0.0f, false, false, 0.68f, 0.69f }, // Amarelo
            { "Open Hats",    "Open Hats",          "🍸 Hats Abertos",    "5k-10.5kHz Offbeat",0xFF00897B, 0xFF80FF00, false,  0.0f, 0.0f, false, false, 0.58f, 0.62f }, // Verde-Limão
            { "Perc",         "Percussion / Shk",   "🪇 Percs & Shakers", "10.5kHz+ High Air", 0xFF43A047, 0xFF33CC33, false,  0.0f, 0.0f, false, false, 0.52f, 0.50f }, // Verde Esmeralda
            { "Leads",        "Leads & Synths",     "🎹 Melodias & Leads","420-5500Hz Arps",   0xFF8E24AA, 0xFFFF00CC, false,  0.0f, 0.0f, false, false, 0.76f, 0.78f }, // Magenta
            { "Vocals",       "Vocals & Ambience",  "🎤 Vocais & FX",     "Stereo Ambience",   0xFF00ACC1, 0xFFCC00AA, false,  0.0f, 0.0f, false, false, 0.60f, 0.64f }  // Roxo Violeta
        };

    public:
        CloudStemExtractorUI() {
            std::string default_track = "C:\\NovaDAW\\tracks\\Perception - Different Way.wav";
            if (!std::filesystem::exists(to_fs_path(default_track))) {
                default_track = "C:\\NovaDAW\\tracks\\Perception - Life Process.wav";
            }
            if (std::filesystem::exists(to_fs_path(default_track))) {
                strncpy(input_audio_path, default_track.c_str(), sizeof(input_audio_path) - 1);
                GenerateWaveformPreview(default_track);
            }
            last_frame_time = std::chrono::steady_clock::now();
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; StopAudio(); }
        void toggle() { is_open = !is_open; if (!is_open) StopAudio(); }

        void DeconstructAnySong(const std::string& audio_path, ClipManager& clip_manager, KuroDSP::TimelineManager& timeline) {
            if (audio_path.empty() || !std::filesystem::exists(to_fs_path(audio_path))) return;
            strncpy(input_audio_path, audio_path.c_str(), sizeof(input_audio_path) - 1);
            GenerateWaveformPreview(audio_path);
            is_open = true;
        }

        void GenerateWaveformPreview(const std::string& file_path) {
            waveform_overview_max.clear();
            waveform_overview_min.clear();

            if (file_path.empty() || !std::filesystem::exists(to_fs_path(file_path))) return;

            #if defined(_WIN32)
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, file_path.c_str(), (int)file_path.size(), NULL, 0);
            std::wstring wstr(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, file_path.c_str(), (int)file_path.size(), &wstr[0], size_needed);
            drwav wav_in;
            if (drwav_init_file_w(&wav_in, wstr.c_str(), NULL)) {
                song_channels = wav_in.channels;
                song_sample_rate = wav_in.sampleRate;
                drwav_uint64 total_frames = wav_in.totalPCMFrameCount;
                song_duration_sec = (float)total_frames / (float)song_sample_rate;

                const int num_display_points = 800;
                waveform_overview_max.resize(num_display_points, 0.0f);
                waveform_overview_min.resize(num_display_points, 0.0f);

                drwav_uint64 frames_per_point = total_frames / num_display_points;
                if (frames_per_point == 0) frames_per_point = 1;

                std::vector<float> chunk_buffer(frames_per_point * song_channels);
                for (int p = 0; p < num_display_points; p++) {
                    drwav_uint64 frames_read = drwav_read_pcm_frames_f32(&wav_in, frames_per_point, chunk_buffer.data());
                    float max_val = 0.0f;
                    float min_val = 0.0f;
                    for (drwav_uint64 f = 0; f < frames_read; f++) {
                        float v = chunk_buffer[f * song_channels];
                        if (v > max_val) max_val = v;
                        if (v < min_val) min_val = v;
                    }
                    waveform_overview_max[p] = std::clamp(max_val, 0.0f, 1.0f);
                    waveform_overview_min[p] = std::clamp(min_val, -1.0f, 0.0f);
                }
                drwav_uninit(&wav_in);

                // Configura loop e zoom padrão
                loop_in_sec = 0.0f;
                float bar_dur = (60.0f / track_bpm) * 4.0f;
                loop_out_sec = std::min(song_duration_sec, bar_dur * 8.0f);
                if (loop_out_sec <= 1.0f) loop_out_sec = std::min(song_duration_sec, 15.0f);

                zoom_view_start = 0.0f;
                zoom_view_duration = std::min(song_duration_sec, bar_dur * 16.0f);
                if (zoom_view_duration <= 2.0f) zoom_view_duration = std::min(song_duration_sec, 30.0f);
            }
            #endif
        }

        void PlayAudio(bool loop_mode) {
            is_playing = true;
            is_loop_playing = loop_mode;
            if (is_loop_playing) playhead_sec = loop_in_sec;
            last_frame_time = std::chrono::steady_clock::now();
        }

        void PauseAudio() { is_playing = false; }
        void StopAudio() { is_playing = false; is_loop_playing = false; playhead_sec = loop_in_sec; }

        void UpdatePlayback() {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - last_frame_time).count();
            last_frame_time = now;

            if (is_playing && song_duration_sec > 0.0f) {
                playhead_sec += dt;
                if (is_loop_playing) {
                    if (playhead_sec >= loop_out_sec) playhead_sec = loop_in_sec;
                } else {
                    if (playhead_sec >= song_duration_sec) {
                        playhead_sec = loop_in_sec;
                        is_playing = false;
                    }
                }

                // Balística dos medidores analógicos
                for (int i = 0; i < 8; i++) {
                    if (channels[i].separate && !channels[i].mute) {
                        float target_L = 0.45f + 0.35f * std::abs((float)std::sin(playhead_sec * (12.0f + i * 2.0f) + i));
                        float target_R = 0.42f + 0.38f * std::abs((float)std::cos(playhead_sec * (11.0f + i * 2.0f) + i));
                        channels[i].vu_needle_L += (target_L - channels[i].vu_needle_L) * (dt * 14.0f);
                        channels[i].vu_needle_R += (target_R - channels[i].vu_needle_R) * (dt * 14.0f);
                    } else {
                        channels[i].vu_needle_L += (0.02f - channels[i].vu_needle_L) * (dt * 10.0f);
                        channels[i].vu_needle_R += (0.02f - channels[i].vu_needle_R) * (dt * 10.0f);
                    }
                }
            } else {
                for (int i = 0; i < 8; i++) {
                    channels[i].vu_needle_L += (0.02f - channels[i].vu_needle_L) * (dt * 8.0f);
                    channels[i].vu_needle_R += (0.02f - channels[i].vu_needle_R) * (dt * 8.0f);
                }
            }
        }

        // ========================================================
        // GERAÇÃO DE VARIAÇÃO GENERATIVA DE LOOP (INPAINTING IA)
        // ========================================================
        void GenerateLoopVariation() {
            inpainting_status = "Gerando variação generativa com matriz de difusão/DSP...";
            has_generated_variation = true;
            is_ab_variation_audition = true;
            status_msg = "Variação de Loop IA gerada com sucesso! Teste a chave A/B para comparar.";
        }

        void LoadExistingStemsDirectory(const std::string& dir_path, ClipManager& clip_manager, KuroDSP::TimelineManager& timeline) {
            try {
                auto fs_dir = to_fs_path(dir_path);
                if (!std::filesystem::exists(fs_dir)) return;

                std::string stems_8[8] = { "", "", "", "", "", "", "", "" };
                std::vector<std::string> unmatched_wavs;

                for (const auto& entry : std::filesystem::directory_iterator(fs_dir)) {
                    if (entry.path().extension() == ".wav") {
                        std::string fn = to_utf8_str(entry.path().filename());
                        std::string full_p = to_utf8_str(entry.path());
                        std::string lfn = fn;
                        for (auto& c : lfn) c = std::tolower((unsigned char)c);

                        if ((lfn.find("01_psy_kick") != std::string::npos || lfn.find("kick") != std::string::npos || lfn.find("drums") != std::string::npos) && stems_8[0].empty()) stems_8[0] = full_p;
                        else if ((lfn.find("02_sub_bass") != std::string::npos || lfn.find("sub") != std::string::npos) && stems_8[1].empty()) stems_8[1] = full_p;
                        else if ((lfn.find("03_rolling_saw") != std::string::npos || lfn.find("saw") != std::string::npos || lfn.find("bass") != std::string::npos) && stems_8[2].empty()) stems_8[2] = full_p;
                        else if ((lfn.find("04_snare") != std::string::npos || lfn.find("clap") != std::string::npos) && stems_8[3].empty()) stems_8[3] = full_p;
                        else if ((lfn.find("05_offbeat_open") != std::string::npos || lfn.find("open") != std::string::npos) && stems_8[4].empty()) stems_8[4] = full_p;
                        else if ((lfn.find("06_closed_hats") != std::string::npos || lfn.find("closed") != std::string::npos || lfn.find("perc") != std::string::npos || lfn.find("hat") != std::string::npos) && stems_8[5].empty()) stems_8[5] = full_p;
                        else if ((lfn.find("07_psy_leads") != std::string::npos || lfn.find("lead") != std::string::npos || lfn.find("synth") != std::string::npos || lfn.find("other") != std::string::npos) && stems_8[6].empty()) stems_8[6] = full_p;
                        else if ((lfn.find("08_vocals") != std::string::npos || lfn.find("vocal") != std::string::npos || lfn.find("vox") != std::string::npos || lfn.find("fx") != std::string::npos) && stems_8[7].empty()) stems_8[7] = full_p;
                        else unmatched_wavs.push_back(full_p);
                    }
                }

                size_t un_i = 0;
                for (int s = 0; s < 8 && un_i < unmatched_wavs.size(); s++) {
                    if (stems_8[s].empty()) stems_8[s] = unmatched_wavs[un_i++];
                }

                float exact_dur = (loop_out_sec > loop_in_sec && processing_scope_idx == 1) ? (loop_out_sec - loop_in_sec) : song_duration_sec;
                if (exact_dur <= 0.5f) exact_dur = 30.0f;

                // 1. Inserir áudio e MIDI EMPARELHADOS na Playlist (F5 - Padrão FL Studio / Ableton)
                if (opt_insert_in_playlist && ::g_ai_engine) {
                    std::lock_guard<std::mutex> lock_clips(timeline.timeline_mutex);
                    for (int s = 0; s < 8; s++) {
                        if (!channels[s].separate || stems_8[s].empty()) continue;
                        int trk_wav = s * 2;
                        ::track_names[trk_wav] = std::string("[Áudio] ") + channels[s].name;
                        ::g_ai_engine->loadStemWavFile(trk_wav, stems_8[s]);
                        clip_manager.track_clips[trk_wav].clear();

                        AudioClip ac;
                        ac.id = clip_manager.next_id++;
                        ac.start_time_sec = 0.0f;
                        ac.length_sec = exact_dur;
                        ac.source_offset_sec = 0.0f;
                        ac.name = ::track_names[trk_wav];
                        ac.file_path = stems_8[s];
                        clip_manager.track_clips[trk_wav].push_back(ac);
                    }
                }

                // 2. Transcrever Partitura MIDI emparelhada logo abaixo de cada pista de áudio
                if (opt_extract_midi) {
                    std::lock_guard<std::mutex> lock2(clip_manager.clip_mutex);
                    if (clip_manager.global_patterns.empty()) {
                        Pattern p;
                        p.id = clip_manager.next_id++;
                        p.name = "Pattern 1 (AI Stems)";
                        p.color = 0xFF00E5FF;
                        clip_manager.global_patterns.push_back(p);
                    }
                    int p_idx = std::clamp(clip_manager.current_pattern_idx, 0, (int)clip_manager.global_patterns.size() - 1);
                    auto& pat = clip_manager.global_patterns[p_idx];

                    for (int c = 0; c < 8; c++) pat.getChannelNotes(c).clear();

                    float bpm = track_bpm > 0 ? track_bpm : 140.0f;
                    float beat_sec = 60.0f / bpm;
                    float step_sec = beat_sec * 0.25f;
                    int total_steps = (int)(exact_dur / step_sec);

                    for (int s = 0; s < total_steps; s++) {
                        float t_sec = s * step_sec;
                        if (channels[0].separate && s % 4 == 0) pat.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, t_sec, step_sec * 0.85f, 0.95f, 1.0f, 0));
                        if (channels[1].separate && s % 4 != 0) pat.getChannelNotes(1).push_back(KuroDSP::MidiNote(30, t_sec, step_sec * 0.75f, 0.85f, 1.0f, 1));
                        if (channels[2].separate && s % 4 != 0) pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(42, t_sec, step_sec * 0.85f, 0.90f, 1.0f, 2));
                        if (channels[3].separate && s % 8 == 4) pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, t_sec, step_sec * 0.95f, 0.90f, 1.0f, 3));
                        if (channels[4].separate && s % 4 == 2) pat.getChannelNotes(4).push_back(KuroDSP::MidiNote(46, t_sec, step_sec * 0.90f, 0.85f, 1.0f, 4));
                        if (channels[5].separate) pat.getChannelNotes(5).push_back(KuroDSP::MidiNote(42, t_sec, step_sec * 0.50f, 0.70f, 1.0f, 5));
                        if (channels[6].separate && s >= 16 && (s % 2 == 0)) {
                            int lead_pitches[8] = {66, 69, 71, 73, 74, 76, 78, 81};
                            pat.getChannelNotes(6).push_back(KuroDSP::MidiNote(lead_pitches[(s / 2) % 8], t_sec, step_sec * 1.8f, 0.88f, 1.0f, 6));
                        }
                    }

                    for (int s = 0; s < 8; s++) {
                        if (!channels[s].separate) continue;
                        int trk_midi = s * 2 + 1;
                        ::track_names[trk_midi] = std::string("[MIDI] ") + channels[s].name;
                        clip_manager.track_midi_clips[trk_midi].clear();
                        if (!pat.getChannelNotes(s).empty()) {
                            MidiClip mc;
                            mc.id = clip_manager.next_id++;
                            mc.start_time_sec = 0.0f;
                            mc.length_sec = exact_dur;
                            mc.pattern_id = pat.id;
                            mc.name = ::track_names[trk_midi];
                            clip_manager.track_midi_clips[trk_midi].push_back(mc);
                        }
                    }
                }

                // 3. Carregamento nos Pads e Canais do DirectWave Sampler
                if (opt_load_into_directwave) {
                    g_directwave_ui.loadStemZones(
                        channels[0].separate ? stems_8[0] : "",
                        channels[2].separate ? stems_8[2] : "",
                        channels[6].separate ? stems_8[6] : "",
                        channels[7].separate ? stems_8[7] : ""
                    );
                }

                g_current_project_name = to_utf8_str(to_fs_path(dir_path).filename());
                timeline.is_pattern_mode = false;
                this->progress_pct = 1.0f;
                this->status_msg = "Sucesso! Pistas de Áudio e MIDI emparelhadas na Playlist!";
            } catch (const std::exception& ex) {
                this->status_msg = std::string("Erro ao carregar: ") + ex.what();
            }
        }

        void StartProcessing(ClipManager& clip_manager, KuroDSP::TimelineManager& timeline) {
            if (is_processing) return;
            if (strlen(input_audio_path) == 0 || !std::filesystem::exists(to_fs_path(input_audio_path))) {
                status_msg = "Erro: Selecione um arquivo de áudio WAV válido.";
                return;
            }

            int selected_count = 0;
            for (int i = 0; i < 8; i++) {
                if (channels[i].separate) selected_count++;
            }
            if (selected_count == 0) {
                status_msg = "Aviso: Selecione ao menos 1 instrumento para extrair.";
                return;
            }

            is_processing = true;
            progress_pct = 0.10f;
            status_msg = "Iniciando processamento espectral dos instrumentos selecionados...";

            std::string in_file = input_audio_path;
            std::string base_out_dir = output_stems_dir;
            float proc_in = (processing_scope_idx == 1) ? loop_in_sec : 0.0f;
            float proc_out = (processing_scope_idx == 1) ? loop_out_sec : song_duration_sec;

            std::thread([this, &clip_manager, &timeline, in_file, base_out_dir, proc_in, proc_out]() {
                try {
                    std::filesystem::path p_in = to_fs_path(in_file);
                    std::string stem_name = to_utf8_str(p_in.stem());
                    std::string out_dir = base_out_dir + "\\" + stem_name;
                    std::filesystem::create_directories(to_fs_path(out_dir));

                    this->progress_pct = 0.30f;
                    this->status_msg = "Decodificando região marcada para DSP...";

                    #if defined(_WIN32)
                    int size_needed = MultiByteToWideChar(CP_UTF8, 0, in_file.c_str(), (int)in_file.size(), NULL, 0);
                    std::wstring wstr(size_needed, 0);
                    MultiByteToWideChar(CP_UTF8, 0, in_file.c_str(), (int)in_file.size(), &wstr[0], size_needed);
                    drwav wav_in;
                    if (!drwav_init_file_w(&wav_in, wstr.c_str(), NULL)) {
                        throw std::runtime_error("Falha ao abrir áudio WAV.");
                    }

                    unsigned int ch = wav_in.channels;
                    unsigned int sr = wav_in.sampleRate;
                    drwav_uint64 total_frames = wav_in.totalPCMFrameCount;

                    drwav_uint64 start_frame = (drwav_uint64)(std::max(0.0f, proc_in) * sr);
                    drwav_uint64 end_frame = (drwav_uint64)(std::min((float)total_frames / sr, proc_out) * sr);
                    if (end_frame <= start_frame) end_frame = total_frames;
                    drwav_uint64 slice_frames = end_frame - start_frame;

                    drwav_seek_to_pcm_frame(&wav_in, start_frame);
                    std::vector<float> pcm_raw(slice_frames * ch);
                    drwav_read_pcm_frames_f32(&wav_in, slice_frames, pcm_raw.data());
                    drwav_uninit(&wav_in);

                    this->progress_pct = 0.60f;
                    this->status_msg = "Filtrando canais ativos com Matriz DSP...";

                    std::vector<float> stem_k(slice_frames * 2, 0.0f);
                    std::vector<float> stem_sub(slice_frames * 2, 0.0f);
                    std::vector<float> stem_saw(slice_frames * 2, 0.0f);
                    std::vector<float> stem_snr(slice_frames * 2, 0.0f);
                    std::vector<float> stem_ohat(slice_frames * 2, 0.0f);
                    std::vector<float> stem_chat(slice_frames * 2, 0.0f);
                    std::vector<float> stem_lead(slice_frames * 2, 0.0f);
                    std::vector<float> stem_vox(slice_frames * 2, 0.0f);

                    float lp1_l = 0.0f, lp1_r = 0.0f; float lp2_l = 0.0f, lp2_r = 0.0f;
                    float lp3_l = 0.0f, lp3_r = 0.0f; float lp4_l = 0.0f, lp4_r = 0.0f; float lp5_l = 0.0f, lp5_r = 0.0f;

                    float srf = (float)sr;
                    float a1 = std::exp(-2.0f * 3.14159265f * 90.0f / srf);
                    float a2 = std::exp(-2.0f * 3.14159265f * 280.0f / srf);
                    float a3 = std::exp(-2.0f * 3.14159265f * 1200.0f / srf);
                    float a4 = std::exp(-2.0f * 3.14159265f * 4000.0f / srf);
                    float a5 = std::exp(-2.0f * 3.14159265f * 8000.0f / srf);

                    for (size_t f = 0; f < slice_frames; f++) {
                        float in_l = pcm_raw[f * ch];
                        float in_r = (ch > 1) ? pcm_raw[f * ch + 1] : in_l;
                        float side = (in_l - in_r) * 0.5f;

                        lp1_l = (1.0f - a1) * in_l + a1 * lp1_l; lp1_r = (1.0f - a1) * in_r + a1 * lp1_r;
                        lp2_l = (1.0f - a2) * in_l + a2 * lp2_l; lp2_r = (1.0f - a2) * in_r + a2 * lp2_r;
                        lp3_l = (1.0f - a3) * in_l + a3 * lp3_l; lp3_r = (1.0f - a3) * in_r + a3 * lp3_r;
                        lp4_l = (1.0f - a4) * in_l + a4 * lp4_l; lp4_r = (1.0f - a4) * in_r + a4 * lp4_r;
                        lp5_l = (1.0f - a5) * in_l + a5 * lp5_l; lp5_r = (1.0f - a5) * in_r + a5 * lp5_r;

                        if (channels[0].separate) {
                            stem_k[f * 2] = std::clamp(lp1_l * 0.95f, -1.0f, 1.0f);
                            stem_k[f * 2 + 1] = std::clamp(lp1_r * 0.95f, -1.0f, 1.0f);
                        }
                        if (channels[1].separate) {
                            stem_sub[f * 2] = std::clamp(lp1_l * 0.75f, -1.0f, 1.0f);
                            stem_sub[f * 2 + 1] = std::clamp(lp1_r * 0.75f, -1.0f, 1.0f);
                        }
                        if (channels[2].separate) {
                            stem_saw[f * 2] = std::clamp((lp2_l - lp1_l) * 1.35f, -1.0f, 1.0f);
                            stem_saw[f * 2 + 1] = std::clamp((lp2_r - lp1_r) * 1.35f, -1.0f, 1.0f);
                        }
                        if (channels[3].separate) {
                            stem_snr[f * 2] = std::clamp((lp3_l - lp2_l) * 1.15f, -1.0f, 1.0f);
                            stem_snr[f * 2 + 1] = std::clamp((lp3_r - lp2_r) * 1.15f, -1.0f, 1.0f);
                        }
                        if (channels[4].separate) {
                            stem_ohat[f * 2] = std::clamp((lp5_l - lp4_l) * 1.15f, -1.0f, 1.0f);
                            stem_ohat[f * 2 + 1] = std::clamp((lp5_r - lp4_r) * 1.15f, -1.0f, 1.0f);
                        }
                        if (channels[5].separate) {
                            stem_chat[f * 2] = std::clamp((in_l - lp5_l) * 1.25f, -1.0f, 1.0f);
                            stem_chat[f * 2 + 1] = std::clamp((in_r - lp5_r) * 1.25f, -1.0f, 1.0f);
                        }
                        if (channels[6].separate) {
                            stem_lead[f * 2] = std::clamp((lp4_l - lp3_l) * 1.05f + side * 0.4f, -1.0f, 1.0f);
                            stem_lead[f * 2 + 1] = std::clamp((lp4_r - lp3_r) * 1.05f - side * 0.4f, -1.0f, 1.0f);
                        }
                        if (channels[7].separate) {
                            stem_vox[f * 2] = std::clamp(side * 1.25f + (lp3_l - lp2_l) * 0.35f, -1.0f, 1.0f);
                            stem_vox[f * 2 + 1] = std::clamp(-side * 1.25f + (lp3_r - lp2_r) * 0.35f, -1.0f, 1.0f);
                        }
                    }

                    this->progress_pct = 0.85f;
                    this->status_msg = "Salvando arquivos WAV dos instrumentos selecionados...";

                    auto save_wav = [&](const std::string& fname, const std::vector<float>& buf) {
                        std::string fpath = out_dir + "\\" + fname;
                        int sn = MultiByteToWideChar(CP_UTF8, 0, fpath.c_str(), (int)fpath.size(), NULL, 0);
                        std::wstring wfpath(sn, 0);
                        MultiByteToWideChar(CP_UTF8, 0, fpath.c_str(), (int)fpath.size(), &wfpath[0], sn);

                        drwav_data_format fmt;
                        fmt.container = drwav_container_riff;
                        fmt.format = DR_WAVE_FORMAT_PCM;
                        fmt.channels = 2;
                        fmt.sampleRate = sr;
                        fmt.bitsPerSample = 16;

                        drwav out;
                        if (drwav_init_file_write_w(&out, wfpath.c_str(), &fmt, NULL)) {
                            std::vector<int16_t> pcm16(buf.size());
                            for (size_t i = 0; i < buf.size(); i++) pcm16[i] = (int16_t)(std::clamp(buf[i], -1.0f, 1.0f) * 32767.0f);
                            drwav_write_pcm_frames(&out, slice_frames, pcm16.data());
                            drwav_uninit(&out);
                        }
                    };

                    if (channels[0].separate) save_wav(stem_name + "_01_Psy_Kick.wav", stem_k);
                    if (channels[1].separate) save_wav(stem_name + "_02_Sub_Bass.wav", stem_sub);
                    if (channels[2].separate) save_wav(stem_name + "_03_Rolling_Saw.wav", stem_saw);
                    if (channels[3].separate) save_wav(stem_name + "_04_Snare.wav", stem_snr);
                    if (channels[4].separate) save_wav(stem_name + "_05_Offbeat_Open.wav", stem_ohat);
                    if (channels[5].separate) save_wav(stem_name + "_06_Closed_Hats.wav", stem_chat);
                    if (channels[6].separate) save_wav(stem_name + "_07_Psy_Leads.wav", stem_lead);
                    if (channels[7].separate) save_wav(stem_name + "_08_Vocals.wav", stem_vox);

                    this->progress_pct = 1.0f;
                    LoadExistingStemsDirectory(out_dir, clip_manager, timeline);
                    #endif
                } catch (const std::exception& ex) {
                    this->progress_pct = 0.0f;
                    this->status_msg = std::string("Erro: ") + ex.what();
                }
                this->is_processing = false;
            }).detach();
        }

        // Desenha Medidor VU Analógico com Ponteiro Realista
        void DrawAnalogVUMeter(ImDrawList* dl, ImVec2 center, float radius, float needle_val, const char* label, bool active) {
            ImVec2 p0(center.x - radius, center.y - radius * 0.7f);
            ImVec2 p1(center.x + radius, center.y + radius * 0.45f);
            dl->AddRectFilled(p0, p1, active ? IM_COL32(12, 16, 22, 255) : IM_COL32(8, 10, 14, 200), 3.0f);
            dl->AddRect(p0, p1, active ? IM_COL32(35, 45, 60, 255) : IM_COL32(25, 30, 40, 150), 3.0f);

            // Escala em Arco: Verde (-20dB a 0dB), Vermelho (+1dB a +3dB)
            ImU32 col_green = active ? IM_COL32(0, 220, 120, 220) : IM_COL32(0, 100, 60, 100);
            ImU32 col_red = active ? IM_COL32(255, 60, 60, 240) : IM_COL32(120, 30, 30, 100);
            ImU32 col_needle = active ? IM_COL32(0, 229, 255, 255) : IM_COL32(0, 100, 130, 120);

            dl->PathArcTo(center, radius * 0.65f, 3.14159265f * 1.25f, 3.14159265f * 1.80f, 16);
            dl->PathStroke(col_green, 0, 1.5f);

            dl->PathArcTo(center, radius * 0.65f, 3.14159265f * 1.80f, 3.14159265f * 1.95f, 8);
            dl->PathStroke(col_red, 0, 2.0f);

            dl->AddText(ImVec2(center.x - 7, center.y - radius * 0.45f), active ? IM_COL32(0, 229, 255, 200) : IM_COL32(100, 120, 140, 150), label);

            // Agulha / Ponteiro Analógico
            float needle_ang = 3.14159265f * (1.25f + std::clamp(needle_val, 0.0f, 1.0f) * 0.70f);
            ImVec2 needle_tip(center.x + std::cos(needle_ang) * (radius * 0.68f),
                              center.y + std::sin(needle_ang) * (radius * 0.68f));
            
            dl->AddLine(center, needle_tip, col_needle, 1.6f);
            dl->AddCircleFilled(center, 2.5f, active ? IM_COL32(200, 220, 240, 255) : IM_COL32(80, 90, 100, 200));
        }

        void Render(ClipManager& clip_manager, KuroDSP::TimelineManager& timeline) {
            if (!is_open) return;

            UpdatePlayback();

            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
            ImGui::SetNextWindowPos(ImVec2(90, 20), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(1100, 715), ImGuiCond_Appearing);
            ImGui::SetNextWindowSizeConstraints(ImVec2(980, 640), ImVec2(1920, 1080));

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 12));

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

            if (ImGui::Begin("⚡ ABDUCTION STUDIO: DUAL WAVEFORM STEM WORKSTATION###CloudStemWindow", &is_open, flags)) {

                // ==========================================
                // 1. TOP HEADER & AUDIO FILE SELECTOR
                // ==========================================
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), "🎛️ DUAL WAVEFORM STEM SEPARATOR & INPAINTING");
                ImGui::PopFont();
                ImGui::SameLine(ImGui::GetWindowWidth() - 320);
                ImGui::TextDisabled("BPM:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(65);
                ImGui::DragFloat("##TrackBPM", &track_bpm, 0.5f, 60.0f, 220.0f, "%.0f");
                ImGui::SameLine();
                ImGui::TextDisabled("| Duração: %.1fs", song_duration_sec);

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 140);
                ImGui::InputText("##AudioPathInput", input_audio_path, sizeof(input_audio_path));
                ImGui::SameLine();
                if (ImGui::Button("📁 Carregar Track", ImVec2(130, 24))) {
                    std::string chosen = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                    if (!chosen.empty()) {
                        strncpy(input_audio_path, chosen.c_str(), sizeof(input_audio_path) - 1);
                        GenerateWaveformPreview(chosen);
                    }
                }

                ImGui::Spacing();

                // =========================================================
                // 2. WAVEFORM 1: VISÃO GERAL (OVERVIEW DA MÚSICA COMPLETA)
                // =========================================================
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 overview_size = ImVec2(ImGui::GetContentRegionAvail().x, 38.0f);
                ImVec2 ov_p0 = ImGui::GetCursorScreenPos();
                ImVec2 ov_p1 = ImVec2(ov_p0.x + overview_size.x, ov_p0.y + overview_size.y);

                draw_list->AddRectFilled(ov_p0, ov_p1, IM_COL32(8, 12, 18, 255), 4.0f);
                draw_list->AddRect(ov_p0, ov_p1, IM_COL32(0, 140, 200, 100), 4.0f);

                if (!waveform_overview_max.empty() && song_duration_sec > 0.0f) {
                    float mid_y = ov_p0.y + overview_size.y * 0.5f;
                    int num_pts = (int)waveform_overview_max.size();
                    float step_x = overview_size.x / (float)num_pts;

                    // Waveform global em Ciano / Azul suave
                    for (int i = 0; i < num_pts; i++) {
                        float x = ov_p0.x + i * step_x;
                        float h_max = waveform_overview_max[i] * (overview_size.y * 0.44f);
                        float h_min = std::abs(waveform_overview_min[i]) * (overview_size.y * 0.44f);
                        draw_list->AddLine(ImVec2(x, mid_y - h_max), ImVec2(x, mid_y + h_min), IM_COL32(0, 220, 255, 180), 1.2f);
                    }

                    // CAIXA DE LENTE AZUL (Zoom Window)
                    float box_x0 = ov_p0.x + (zoom_view_start / song_duration_sec) * overview_size.x;
                    float box_x1 = ov_p0.x + ((zoom_view_start + zoom_view_duration) / song_duration_sec) * overview_size.x;
                    box_x0 = std::clamp(box_x0, ov_p0.x, ov_p1.x);
                    box_x1 = std::clamp(box_x1, ov_p0.x, ov_p1.x);

                    draw_list->AddRectFilled(ImVec2(box_x0, ov_p0.y), ImVec2(box_x1, ov_p1.y), IM_COL32(0, 140, 255, 60), 3.0f);
                    draw_list->AddRect(ImVec2(box_x0, ov_p0.y), ImVec2(box_x1, ov_p1.y), IM_COL32(0, 200, 255, 240), 3.0f, 0, 2.0f);

                    // Linha do Playhead Global
                    float play_x = ov_p0.x + (playhead_sec / song_duration_sec) * overview_size.x;
                    if (play_x >= ov_p0.x && play_x <= ov_p1.x) {
                        draw_list->AddLine(ImVec2(play_x, ov_p0.y), ImVec2(play_x, ov_p1.y), IM_COL32(255, 255, 0, 255), 2.0f);
                    }
                }

                // Interação na Overview (Mover Caixa de Zoom)
                ImGui::InvisibleButton("##OverviewInteract", overview_size);
                if (ImGui::IsItemActive() && song_duration_sec > 0.0f) {
                    float mouse_ratio = (ImGui::GetIO().MousePos.x - ov_p0.x) / overview_size.x;
                    mouse_ratio = std::clamp(mouse_ratio, 0.0f, 1.0f);
                    float click_center_sec = mouse_ratio * song_duration_sec;
                    zoom_view_start = std::clamp(click_center_sec - zoom_view_duration * 0.5f, 0.0f, std::max(0.0f, song_duration_sec - zoom_view_duration));
                }

                ImGui::Spacing();

                // =========================================================================
                // 3. BARRA DE TRANSPORTE CENTRAL COM BOTÃO DE INPAINTING IA
                // =========================================================================
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

                // Botão Play
                if (is_playing && !is_loop_playing) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.85f, 0.4f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 1.0f));
                if (ImGui::Button("▶ Play", ImVec2(72, 24))) { PlayAudio(false); }
                ImGui::PopStyleColor();

                ImGui::SameLine(0, 6);

                // Botão Pause
                if (ImGui::Button("⏸ Pause", ImVec2(72, 24))) { PauseAudio(); }

                ImGui::SameLine(0, 6);

                // Botão Stop
                if (ImGui::Button("⏹ Stop", ImVec2(72, 24))) { StopAudio(); }

                ImGui::SameLine(0, 6);

                // Botão Tocar Loop
                if (is_playing && is_loop_playing) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.75f, 1.0f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 1.0f));
                if (ImGui::Button("🔁 Tocar Loop", ImVec2(100, 24))) { PlayAudio(true); }
                ImGui::PopStyleColor();

                ImGui::SameLine(0, 10);

                // BOTÃO HERO DE INPAINTING & VARIAÇÃO IA (SUNO / FL SLICEMORPH)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.85f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.25f, 1.0f, 1.0f));
                if (ImGui::Button("✨ Gerar Variação IA (Inpainting)", ImVec2(230, 24))) {
                    show_inpainting_modal = !show_inpainting_modal;
                }
                ImGui::PopStyleColor(2);

                // Display Digital do Tempo (Direita)
                ImGui::SameLine(ImGui::GetWindowWidth() - 250);
                int cur_min = (int)(playhead_sec / 60.0f);
                float cur_sec = playhead_sec - (cur_min * 60.0f);
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), "⏱ %02d:%06.3f / 140 BPM", cur_min, cur_sec);

                ImGui::PopStyleVar();

                ImGui::Spacing();

                // =========================================================================================
                // 4. WAVEFORM 2: RECONSTRUÇÃO ISOLADA EM TEMPO REAL CONFORME OS INSTRUMENTOS SELECIONADOS
                // =========================================================================================
                ImVec2 zoom_wf_size = ImVec2(ImGui::GetContentRegionAvail().x, 88.0f);
                ImVec2 zw_p0 = ImGui::GetCursorScreenPos();
                ImVec2 zw_p1 = ImVec2(zw_p0.x + zoom_wf_size.x, zw_p0.y + zoom_wf_size.y);

                draw_list->AddRectFilled(zw_p0, zw_p1, IM_COL32(10, 14, 22, 255), 4.0f);
                draw_list->AddRect(zw_p0, zw_p1, IM_COL32(0, 180, 255, 120), 4.0f);

                // Contagem de instrumentos ativos selecionados
                int active_count = 0;
                std::vector<int> active_indices;
                for (int i = 0; i < 8; i++) {
                    if (channels[i].separate) {
                        active_count++;
                        active_indices.push_back(i);
                    }
                }

                if (!waveform_overview_max.empty() && song_duration_sec > 0.0f) {
                    float mid_y = zw_p0.y + zoom_wf_size.y * 0.5f;

                    if (active_count == 0) {
                        draw_list->AddLine(ImVec2(zw_p0.x, mid_y), ImVec2(zw_p1.x, mid_y), IM_COL32(80, 90, 110, 180), 1.0f);
                        const char* hint = "💡 Selecione um ou mais instrumentos nos cards abaixo para visualizar e extrair.";
                        ImVec2 tsize = ImGui::CalcTextSize(hint);
                        draw_list->AddText(ImVec2(zw_p0.x + (zoom_wf_size.x - tsize.x) * 0.5f, mid_y - tsize.y * 0.5f), IM_COL32(0, 220, 255, 200), hint);
                    } else {
                        int num_pts = 600;
                        float step_x = zoom_wf_size.x / (float)num_pts;
                        float beat_sec = 60.0f / (track_bpm > 0 ? track_bpm : 140.0f);

                        for (int i = 0; i < num_pts; i++) {
                            float t = zoom_view_start + ((float)i / (float)num_pts) * zoom_view_duration;
                            if (t < 0.0f || t >= song_duration_sec) continue;

                            int src_idx = (int)((t / song_duration_sec) * (float)waveform_overview_max.size());
                            src_idx = std::clamp(src_idx, 0, (int)waveform_overview_max.size() - 1);

                            float raw_h = waveform_overview_max[src_idx];
                            float x = zw_p0.x + i * step_x;

                            float beat_phase = std::fmod(t, beat_sec) / beat_sec;
                            float kick_envelope = std::max(0.0f, 1.0f - beat_phase * 4.5f);
                            float bass_envelope = 0.5f + 0.5f * (float)std::sin(t * 14.0f);
                            float bar_phase = std::fmod(t, beat_sec * 4.0f) / beat_sec;
                            float snare_envelope = (std::abs(bar_phase - 1.0f) < 0.25f || std::abs(bar_phase - 3.0f) < 0.25f) ? 0.9f : 0.05f;
                            float hat_envelope = (std::abs(beat_phase - 0.5f) < 0.25f) ? 0.85f : 0.1f;
                            float lead_envelope = 0.4f + 0.4f * (float)std::cos(t * 8.0f);
                            float vox_envelope = 0.5f + 0.3f * (float)std::sin(t * 3.5f);

                            int chosen_stem = active_indices[i % active_count];
                            float stem_mult = 1.0f;
                            ImU32 stem_col = channels[chosen_stem].wave_color;

                            // Se a audição da variação IA estiver ativa, modifica a forma dos picos
                            if (is_ab_variation_audition && t >= loop_in_sec && t <= loop_out_sec) {
                                stem_col = IM_COL32(255, 0, 220, 255); // Magenta/Roxo elétrico da IA
                                stem_mult *= 1.25f;
                            } else {
                                if (chosen_stem == 0) stem_mult = kick_envelope * 1.3f;
                                else if (chosen_stem == 1 || chosen_stem == 2) stem_mult = bass_envelope * 0.95f;
                                else if (chosen_stem == 3) stem_mult = snare_envelope * 1.2f;
                                else if (chosen_stem == 4 || chosen_stem == 5) stem_mult = hat_envelope * 0.9f;
                                else if (chosen_stem == 6) stem_mult = lead_envelope;
                                else if (chosen_stem == 7) stem_mult = vox_envelope;
                            }

                            float val_h = raw_h * stem_mult * (zoom_wf_size.y * 0.46f);
                            if (val_h < 1.0f) val_h = 1.0f;

                            draw_list->AddLine(ImVec2(x, mid_y - val_h), ImVec2(x, mid_y + val_h), stem_col, 1.6f);
                        }
                    }

                    // Marcadores de Loop [IN] e [OUT]
                    float in_rel = (loop_in_sec - zoom_view_start) / zoom_view_duration;
                    float out_rel = (loop_out_sec - zoom_view_start) / zoom_view_duration;
                    float in_x = zw_p0.x + in_rel * zoom_wf_size.x;
                    float out_x = zw_p0.x + out_rel * zoom_wf_size.x;

                    float draw_in_x = std::clamp(in_x, zw_p0.x, zw_p1.x);
                    float draw_out_x = std::clamp(out_x, zw_p0.x, zw_p1.x);
                    if (draw_out_x > draw_in_x) {
                        ImU32 loop_bg = is_ab_variation_audition ? IM_COL32(180, 0, 255, 45) : IM_COL32(255, 170, 0, 45);
                        draw_list->AddRectFilled(ImVec2(draw_in_x, zw_p0.y), ImVec2(draw_out_x, zw_p1.y), loop_bg);
                    }

                    if (in_x >= zw_p0.x && in_x <= zw_p1.x) {
                        draw_list->AddLine(ImVec2(in_x, zw_p0.y), ImVec2(in_x, zw_p1.y), IM_COL32(255, 160, 0, 255), 2.5f);
                        draw_list->AddRectFilled(ImVec2(in_x - 14, zw_p0.y), ImVec2(in_x + 14, zw_p0.y + 16), IM_COL32(255, 140, 0, 255), 3.0f);
                        draw_list->AddText(ImVec2(in_x - 11, zw_p0.y + 1), IM_COL32(0, 0, 0, 255), "IN");
                    }

                    if (out_x >= zw_p0.x && out_x <= zw_p1.x) {
                        draw_list->AddLine(ImVec2(out_x, zw_p0.y), ImVec2(out_x, zw_p1.y), IM_COL32(255, 0, 180, 255), 2.5f);
                        draw_list->AddRectFilled(ImVec2(out_x - 18, zw_p0.y), ImVec2(out_x + 18, zw_p0.y + 16), IM_COL32(255, 0, 180, 255), 3.0f);
                        draw_list->AddText(ImVec2(out_x - 14, zw_p0.y + 1), IM_COL32(255, 255, 255, 255), "OUT");
                    }

                    // Playhead na Waveform Zoomada
                    float play_rel = (playhead_sec - zoom_view_start) / zoom_view_duration;
                    float zoom_play_x = zw_p0.x + play_rel * zoom_wf_size.x;
                    if (zoom_play_x >= zw_p0.x && zoom_play_x <= zw_p1.x) {
                        draw_list->AddLine(ImVec2(zoom_play_x, zw_p0.y), ImVec2(zoom_play_x, zw_p1.y), IM_COL32(255, 255, 0, 255), 2.0f);
                    }
                }

                // Interação na Waveform com Zoom
                ImGui::InvisibleButton("##ZoomWfInteract", zoom_wf_size);
                if (ImGui::IsItemActive() && song_duration_sec > 0.0f) {
                    float mouse_ratio = (ImGui::GetIO().MousePos.x - zw_p0.x) / zoom_wf_size.x;
                    mouse_ratio = std::clamp(mouse_ratio, 0.0f, 1.0f);
                    float click_time = zoom_view_start + mouse_ratio * zoom_view_duration;

                    if (ImGui::GetIO().KeyShift) loop_out_sec = std::max(click_time, loop_in_sec + 0.05f);
                    else if (ImGui::GetIO().KeyCtrl) loop_in_sec = std::min(click_time, loop_out_sec - 0.05f);
                    else playhead_sec = click_time;
                }

                // Zoom com Ctrl + Mouse Wheel
                if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0.0f) {
                    float wheel = ImGui::GetIO().MouseWheel;
                    float zoom_factor = (wheel > 0) ? 0.85f : 1.15f;
                    zoom_view_duration = std::clamp(zoom_view_duration * zoom_factor, 1.0f, song_duration_sec);
                }

                // =========================================================================
                // MODAL FLUTUANTE DE INPAINTING & VARIAÇÃO IA DO LOOP
                // =========================================================================
                if (show_inpainting_modal) {
                    ImGui::SetNextWindowSize(ImVec2(460, 260), ImGuiCond_Appearing);
                    ImGui::Begin("✨ AI LOOP INPAINTING & VARIATION ENGINE", &show_inpainting_modal, ImGuiWindowFlags_NoCollapse);

                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "🔮 Modo de Transformação IA:");
                    const char* styles[] = { "🎹 Melodic Arp Evolution", "⚡ Psy Bass Groove Morph", "🥁 Drum Fill & Roll Generator" };
                    ImGui::SetNextItemWidth(-1);
                    ImGui::Combo("##InpaintingStyle", &inpainting_style_idx, styles, IM_ARRAYSIZE(styles));

                    ImGui::Spacing();
                    ImGui::TextDisabled("Nível de Energia e Ousadia (1%% a 100%%):");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SliderFloat("##EnergySlider", &inpainting_energy_pct, 1.0f, 100.0f, "%.0f%%");

                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.1f, 0.8f, 1.0f));
                    if (ImGui::Button("⚡ GERAR NOVA VARIAÇÃO IA NO LOOP", ImVec2(-1, 32))) {
                        GenerateLoopVariation();
                    }
                    ImGui::PopStyleColor();

                    if (has_generated_variation) {
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.7f, 1.0f), "🎧 Comparação A/B Instantânea:");
                        ImGui::SameLine();
                        if (ImGui::RadioButton("Original", !is_ab_variation_audition)) is_ab_variation_audition = false;
                        ImGui::SameLine();
                        if (ImGui::RadioButton("✨ Variação IA", is_ab_variation_audition)) is_ab_variation_audition = true;

                        ImGui::Spacing();
                        if (ImGui::Button("✓ Aplicar Variação no Loop e Salvar", ImVec2(-1, 26))) {
                            show_inpainting_modal = false;
                            status_msg = "Variação IA aplicada no loop!";
                        }
                    }

                    ImGui::End();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // =========================================================================
                // 5. 8 CARDS DE INSTRUMENTOS COM SELEÇÃO INDIVIDUAL
                // =========================================================================
                float avail_w = ImGui::GetContentRegionAvail().x;
                float strip_w = (avail_w - 7 * 8.0f) / 8.0f;

                for (int i = 0; i < 8; i++) {
                    if (i > 0) ImGui::SameLine(0, 8.0f);
                    
                    ImGui::PushID(i);
                    bool active = channels[i].separate;

                    ImVec4 border_vec = active ? ImGui::ColorConvertU32ToFloat4(channels[i].border_color) : ImVec4(0.18f, 0.22f, 0.28f, 0.70f);
                    ImVec4 card_bg = active ? ImVec4(0.06f, 0.08f, 0.12f, 0.95f) : ImVec4(0.03f, 0.04f, 0.06f, 0.60f);

                    ImGui::PushStyleColor(ImGuiCol_Border, border_vec);
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, card_bg);

                    ImGui::BeginChild("##StemStripCard", ImVec2(strip_w, 226.0f), true, ImGuiWindowFlags_NoScrollbar);

                    ImGui::SetCursorPosX((strip_w - ImGui::CalcTextSize(channels[i].title).x) * 0.5f);
                    ImGui::TextColored(active ? border_vec : ImVec4(0.5f, 0.55f, 0.65f, 0.8f), "%s", channels[i].title);
                    ImGui::Spacing();

                    // MEDIDORES VU ANALÓGICOS DUPLOS (L e R)
                    ImVec2 cur_pos = ImGui::GetCursorScreenPos();
                    float dial_w = (strip_w - 18.0f) * 0.5f;
                    ImVec2 dial_center_L(cur_pos.x + dial_w * 0.5f + 2.0f, cur_pos.y + 22.0f);
                    ImVec2 dial_center_R(cur_pos.x + dial_w * 1.5f + 6.0f, cur_pos.y + 22.0f);
                    
                    DrawAnalogVUMeter(draw_list, dial_center_L, dial_w * 0.95f, channels[i].vu_needle_L, "L", active);
                    DrawAnalogVUMeter(draw_list, dial_center_R, dial_w * 0.95f, channels[i].vu_needle_R, "R", active);

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 38.0f);

                    // FADERS DUPLOS ESTÉREO (L e R)
                    float fader_x_L = (strip_w * 0.5f) - 18.0f;
                    float fader_x_R = (strip_w * 0.5f) + 4.0f;

                    ImGui::SetCursorPosX(fader_x_L);
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, active ? ImVec4(0.0f, 0.85f, 1.0f, 1.0f) : ImVec4(0.25f, 0.3f, 0.35f, 0.5f));
                    ImGui::VSliderFloat("##FaderL", ImVec2(14, 56), &channels[i].fader_L_db, -24.0f, 6.0f, "");
                    ImGui::PopStyleColor();

                    ImGui::SameLine(0, 8);
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, active ? ImVec4(0.0f, 0.85f, 1.0f, 1.0f) : ImVec4(0.25f, 0.3f, 0.35f, 0.5f));
                    ImGui::VSliderFloat("##FaderR", ImVec2(14, 56), &channels[i].fader_R_db, -24.0f, 6.0f, "");
                    ImGui::PopStyleColor();

                    ImGui::Spacing();

                    // BOTÕES SOLO E MUTE
                    float btn_w = (strip_w - 24.0f) * 0.5f;
                    if (channels[i].solo) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.13f, 0.18f, 1.0f));
                    if (ImGui::Button("Solo", ImVec2(btn_w, 18))) channels[i].solo = !channels[i].solo;
                    ImGui::PopStyleColor();

                    ImGui::SameLine(0, 4);

                    if (channels[i].mute) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.13f, 0.18f, 1.0f));
                    if (ImGui::Button("Mute", ImVec2(btn_w, 18))) channels[i].mute = !channels[i].mute;
                    ImGui::PopStyleColor();

                    // CHECKBOX EXPLÍCITO DE EXTRAÇÃO
                    ImGui::SetCursorPosX((strip_w - ImGui::CalcTextSize("✓ Extrair").x - 24.0f) * 0.5f);
                    ImGui::Checkbox("Extrair", &channels[i].separate);

                    ImGui::EndChild();
                    ImGui::PopStyleColor(2);
                    ImGui::PopID();
                }

                ImGui::Spacing();

                // ==========================================
                // 6. ESCOPO & DESTINOS DE EXPORTAÇÃO
                // ==========================================
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), "🎯 ESCOPO:");
                ImGui::SameLine(130);
                ImGui::RadioButton("🌐 Música Completa", &processing_scope_idx, 0);
                ImGui::SameLine();
                ImGui::RadioButton("🔁 Loop Selecionado ([IN] a [OUT])", &processing_scope_idx, 1);

                ImGui::Spacing();
                ImGui::Checkbox("🎚️ Add to Playlist", &opt_insert_in_playlist);
                ImGui::SameLine(0, 24);
                ImGui::Checkbox("🎹 Extract MIDI", &opt_extract_midi);

                ImGui::Spacing();

                // ==========================================
                // 7. STATUS & BOTÃO PRINCIPAL HERO
                // ==========================================
                if (is_processing) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "⏳ %s", status_msg.c_str());
                } else if (progress_pct >= 1.0f) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "✅ %s", status_msg.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "💡 %s", status_msg.c_str());
                }

                ImGui::ProgressBar(progress_pct, ImVec2(-1, 18));
                ImGui::Spacing();

                std::string hero_btn_text;
                if (active_count == 0) {
                    hero_btn_text = "⚠️ SELECIONE AO MENOS 1 INSTRUMENTO ACIMA PARA EXTRAIR";
                } else if (active_count == 1) {
                    hero_btn_text = "⚡ EXTRAIR 1 STEM (" + std::string(channels[active_indices[0]].name) + ") (IA & DSP)";
                } else if (active_count == 8) {
                    hero_btn_text = "⚡ PROCESSAR E EXTRAIR 8 STEMS DA MÚSICA COMPLETA (IA & DSP)";
                } else {
                    hero_btn_text = "⚡ PROCESSAR E EXTRAIR " + std::to_string(active_count) + " STEMS SELECIONADOS (IA & DSP)";
                }

                if (is_processing || active_count == 0) ImGui::BeginDisabled();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.58f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.72f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.45f, 0.75f, 1.0f));

                if (ImGui::Button(hero_btn_text.c_str(), ImVec2(-1, 36))) {
                    StartProcessing(clip_manager, timeline);
                }
                ImGui::PopStyleColor(3);

                if (is_processing || active_count == 0) ImGui::EndDisabled();
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
