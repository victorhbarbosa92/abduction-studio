#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem>

// ONNX C++ API
#include <onnxruntime_cxx_api.h>

// dr_wav header-only library
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

// dr_mp3 header-only library
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
// KuroMIR (BPM & Campo Harmonico)
#include "KuroMIR.h"
#include "KuroDatabase.h"
#include "KuroSpectralExtractor.h"

// Log system
#include "../utils/Logger.h"
#include "../core/ClipManager.h"

#include "../core/KuroConfig.h"
extern std::string track_names[MAX_TRACKS];

// Filtro RC Simples para Crossover Estéreo (DSP)
struct SimpleRC {
    float prev_L = 0.0f;
    float prev_R = 0.0f;
    float alpha = 0.5f;
    void init(float cutoff, float sr) {
        float dt = 1.0f / sr;
        float rc = 1.0f / (6.2831853f * cutoff);
        alpha = dt / (rc + dt);
    }
    void processLP(float inL, float inR, float& outL, float& outR) {
        prev_L += alpha * (inL - prev_L);
        prev_R += alpha * (inR - prev_R);
        outL = prev_L; outR = prev_R;
    }
    void processHP(float inL, float inR, float& outL, float& outR) {
        processLP(inL, inR, outL, outR);
        outL = inL - outL; outR = inR - outR;
    }
};

struct AudioSlice {
    int source_track;
    std::string suggested_name;
    std::vector<float> data;
    std::vector<float> waveform_preview;
    bool selected;
};

class StemSeparationEngine {
private:
    std::atomic<bool> is_running{false};
    std::atomic<bool> has_finished{false};
    std::atomic<float> progress{0.0f};
    std::string current_status;
    std::thread worker_thread;
    
    // Metadados da Faixa (KuroMIR)
    float detected_bpm = 0.0f;
    std::string detected_key = "Unknown";
    
    // Áudio Real em Memória (KICK, LEADS, VOX, FX, SYNTHS, ZAPS, ATMOS, EXTRA)
    std::vector<float> stems_buffers[MAX_TRACKS];
    std::vector<float> waveform_overview[MAX_TRACKS];
    std::vector<float> master_waveform_overview;
    std::vector<float> original_audio_buffer;
    size_t original_channels = 2;
    size_t total_stem_frames = 0;
    unsigned int stem_sample_rate = 44100;

    // Configurações do ONNX
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "AbductionStudioV2"};
    
    void processLoop(const std::string& filepath, int mode) {
        is_running = true;
        has_finished = false;
        current_status = "Iniciando IA (ONNX) - Alocando modelos...";
        
        extern ClipManager g_clip_manager;
        g_clip_manager.reset();
        
        // 1. LER O ARQUIVO WAV/MP3 COM DR_WAV / DR_MP3
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 totalPCMFrameCount;
        float* pSampleData = nullptr;
        
        // Verifica a extensão do arquivo
        std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
        for(auto& c : ext) c = tolower(c);

        if (ext == "mp3") {
            current_status = "Decodificando MP3 para memoria RAM...";
            drmp3_config config;
            drmp3_uint64 totalFrames;
            pSampleData = drmp3_open_file_and_read_pcm_frames_f32(filepath.c_str(), &config, &totalFrames, NULL);
            if (pSampleData) {
                channels = config.channels;
                sampleRate = config.sampleRate;
                totalPCMFrameCount = totalFrames;
            }
        } else {
            current_status = "Decodificando WAV para memoria RAM...";
            pSampleData = drwav_open_file_and_read_pcm_frames_f32(filepath.c_str(), &channels, &sampleRate, &totalPCMFrameCount, NULL);
        }
        
        if (pSampleData == NULL) {
            current_status = "Erro: Falha ao decodificar o arquivo de audio (formato suportado: wav/mp3).";
            std::this_thread::sleep_for(std::chrono::seconds(3));
            is_running = false;
            return;
        }
        
        std::vector<float> safe_audio_data(pSampleData, pSampleData + (totalPCMFrameCount * channels));
        if (ext == "mp3") {
            drmp3_free(pSampleData, NULL);
        } else {
            drwav_free(pSampleData, NULL);
        }

        // Resampling Linear Rápido se a taxa de amostragem não for 44100Hz
        if (sampleRate != 44100) {
            current_status = "Convertendo Sample Rate (" + std::to_string(sampleRate) + "Hz -> 44100Hz)...";
            KuroUtils::Log(current_status);
            
            double ratio = 44100.0 / (double)sampleRate;
            drwav_uint64 newFrames = (drwav_uint64)(totalPCMFrameCount * ratio);
            std::vector<float> resampledData(newFrames * channels);
            
            for (drwav_uint64 i = 0; i < newFrames; i++) {
                double src_idx_exact = i / ratio;
                drwav_uint64 idx1 = (drwav_uint64)src_idx_exact;
                drwav_uint64 idx2 = idx1 + 1;
                if (idx2 >= totalPCMFrameCount) idx2 = totalPCMFrameCount - 1;
                double frac = src_idx_exact - idx1;
                
                for (unsigned int c = 0; c < channels; c++) {
                    float v1 = safe_audio_data[idx1 * channels + c];
                    float v2 = safe_audio_data[idx2 * channels + c];
                    resampledData[i * channels + c] = v1 + (float)((v2 - v1) * frac);
                }
            }
            safe_audio_data = std::move(resampledData);
            totalPCMFrameCount = newFrames;
            sampleRate = 44100;
        }
        
        // Substituir pSampleData pelo ponteiro gerenciado pelo vector
        pSampleData = safe_audio_data.data();

        original_channels = channels;
        original_audio_buffer = safe_audio_data;
        
        // --- 1.5. KURO DATABASE (Extração Rápida de BPM & KEY Sem IA) ---
        current_status = "Buscando BPM e Tom no banco de metadados...";
        
        bool db_hit = KuroAI::KuroDatabase::fetchMetadata(filepath, detected_bpm, detected_key);
        
        if (!db_hit) {
            std::vector<float> mono_downmix(totalPCMFrameCount);
            for(size_t i = 0; i < totalPCMFrameCount; i++) {
                mono_downmix[i] = pSampleData[i * channels];
                if(channels > 1) mono_downmix[i] = (mono_downmix[i] + pSampleData[i * channels + 1]) * 0.5f;
            }
            detected_bpm = KuroMIR::estimateBPM(mono_downmix, sampleRate);
            detected_key = KuroMIR::estimateKey(mono_downmix, sampleRate);
        }
        
        KuroUtils::Log("Metadados extraidos. BPM: " + std::to_string(detected_bpm) + " | Tom: " + detected_key);

        // 2. CONFIGURAR ONNX
        current_status = "Acelerando Tensores ONNX Runtime (AVX2)...";
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(4);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        
        std::unique_ptr<Ort::Session> session = nullptr;
        bool model_loaded = false;
        
        try {
            // Tenta carregar o modelo real
#ifdef _WIN32
            session = std::make_unique<Ort::Session>(env, L"demucs_kuro.onnx", session_options);
#else
            session = std::make_unique<Ort::Session>(env, "demucs_kuro.onnx", session_options);
#endif
            model_loaded = true;
            current_status = "Modelo neural 'demucs_kuro.onnx' carregado na memoria!";
        } catch(const std::exception& e) {
            current_status = "AVISO: demucs_kuro.onnx ausente! Rodando Fallback Mode...";
            model_loaded = false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        
        // Limpar buffers antigos
        for (int i = 0; i < MAX_TRACKS; i++) {
            stems_buffers[i].clear();
            stems_buffers[i].reserve(totalPCMFrameCount * 2); // Stereo
        }
        stem_sample_rate = sampleRate;
        total_stem_frames = totalPCMFrameCount;
        
        // 3. CHUNKING & INFERÊNCIA
        const size_t chunkSizeFrames = sampleRate * 10; // Janelas de 10 segundos para economizar RAM
        const size_t totalChunks = (totalPCMFrameCount / chunkSizeFrames) + 1;
        
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        
        for (size_t i = 0; i < totalChunks; ++i) {
            if (!is_running) break;
            
            current_status = "Inferência (ONNX): Processando Tensor " + std::to_string(i+1) + "/" + std::to_string(totalChunks);
            
            size_t framesToProcess = chunkSizeFrames;
            if ((i * chunkSizeFrames + framesToProcess) > totalPCMFrameCount) {
                framesToProcess = totalPCMFrameCount - (i * chunkSizeFrames);
            }
            
            if (model_loaded && session) {
                // ALOCAÇÃO REAL DO TENSOR DE ENTRADA (Batch=1, Channels=2, Time=framesToProcess)
                std::vector<int64_t> input_shape = {1, 2, static_cast<int64_t>(framesToProcess)};
                size_t input_tensor_size = 1 * 2 * framesToProcess;
                
                std::vector<float> input_tensor_values(input_tensor_size);
                
                // Preenchendo o buffer planar com os frames do dr_wav
                size_t offset = i * chunkSizeFrames * channels;
                for(size_t c = 0; c < 2; c++) {
                    for(size_t s = 0; s < framesToProcess; s++) {
                        input_tensor_values[c * framesToProcess + s] = pSampleData[offset + (s * channels) + c];
                    }
                }
                
                Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                    memory_info, input_tensor_values.data(), input_tensor_values.size(), input_shape.data(), input_shape.size()
                );
                
                const char* input_names[] = {"input"};
                const char* output_names[] = {"output"};
                
                try {
                    // CEREBRO EM AÇÃO: session->Run()
                    auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 1);
                    
                    // Extrai as STEMS separadas do Tensor (Batch=1, Stems=4, Channels=2, Time=framesToProcess)
                    float* output_arr = output_tensors.front().GetTensorMutableData<float>();
                    
                    // O Demucs HT tipicamente gera 4 fontes na ordem: Drums, Bass, Other, Vocals
                    // Nosso mixer tem: 0=KICK/BASS, 1=LEADS(Other/Drums), 2=VOX(Vocals), 3=FX
                    
                    for (size_t s = 0; s < framesToProcess; s++) {
                        // Tensor shape: [1, 4, 2, framesToProcess]
                        // Endereço = (source * 2 * framesToProcess) + (channel * framesToProcess) + s
                        
                        // KICK/BASS (Source 1 - Bass)
                        float bassL = output_arr[(1 * 2 * framesToProcess) + (0 * framesToProcess) + s];
                        float bassR = output_arr[(1 * 2 * framesToProcess) + (1 * framesToProcess) + s];
                        stems_buffers[0].push_back(bassL); stems_buffers[0].push_back(bassR);
                        
                        // LEADS (Source 0 - Drums + Source 2 - Other)
                        float leadsL = output_arr[(0 * 2 * framesToProcess) + (0 * framesToProcess) + s] + 
                                       output_arr[(2 * 2 * framesToProcess) + (0 * framesToProcess) + s];
                        float leadsR = output_arr[(0 * 2 * framesToProcess) + (1 * framesToProcess) + s] + 
                                       output_arr[(2 * 2 * framesToProcess) + (1 * framesToProcess) + s];
                        stems_buffers[1].push_back(leadsL); stems_buffers[1].push_back(leadsR);
                        
                        // VOX (Source 3 - Vocals)
                        float voxL = output_arr[(3 * 2 * framesToProcess) + (0 * framesToProcess) + s];
                        float voxR = output_arr[(3 * 2 * framesToProcess) + (1 * framesToProcess) + s];
                        stems_buffers[2].push_back(voxL); stems_buffers[2].push_back(voxR);
                        
                        // FX (Deixamos mudo inicialmente)
                        stems_buffers[3].push_back(0.0f); stems_buffers[3].push_back(0.0f);
                    }
                    
                } catch(const std::exception& e) {
                    current_status = "Erro Neural: " + std::string(e.what());
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    break;
                }
            } else {
                // Fallback de Reconstrução Perfeita com Crossover 4-Bandas (Simulação 4 Stems)
                size_t offset = i * chunkSizeFrames * channels;
                
                // Instanciar Filtros para o Chunk (8 bandas)
                SimpleRC lp_100; lp_100.init(100.0f, sampleRate);
                SimpleRC lp_250; lp_250.init(250.0f, sampleRate);
                SimpleRC lp_500; lp_500.init(500.0f, sampleRate);
                SimpleRC lp_1k; lp_1k.init(1000.0f, sampleRate);
                SimpleRC lp_2k; lp_2k.init(2000.0f, sampleRate);
                SimpleRC lp_4k; lp_4k.init(4000.0f, sampleRate);
                SimpleRC lp_8k; lp_8k.init(8000.0f, sampleRate);
                
                for (size_t s = 0; s < framesToProcess; s++) {
                    float inL = pSampleData[offset + (s * channels) + 0];
                    float inR = channels > 1 ? pSampleData[offset + (s * channels) + 1] : inL;
                    
                    float t0L, t0R, t1L, t1R, t2L, t2R, t3L, t3R, t4L, t4R, t5L, t5R, t6L, t6R, t7L, t7R;
                    float rL, rR;
                    
                    lp_100.processLP(inL, inR, t0L, t0R); // Track 0 (<100Hz)
                    rL = inL - t0L; rR = inR - t0R;
                    
                    lp_250.processLP(rL, rR, t1L, t1R); // Track 1 (100-250)
                    rL -= t1L; rR -= t1R;
                    
                    lp_500.processLP(rL, rR, t2L, t2R); // Track 2 (250-500)
                    rL -= t2L; rR -= t2R;
                    
                    lp_1k.processLP(rL, rR, t3L, t3R); // Track 3 (500-1k)
                    rL -= t3L; rR -= t3R;
                    
                    lp_2k.processLP(rL, rR, t4L, t4R); // Track 4 (1k-2k)
                    rL -= t4L; rR -= t4R;
                    
                    lp_4k.processLP(rL, rR, t5L, t5R); // Track 5 (2k-4k)
                    rL -= t5L; rR -= t5R;
                    
                    lp_8k.processLP(rL, rR, t6L, t6R); // Track 6 (4k-8k)
                    rL -= t6L; rR -= t6R;
                    
                    t7L = rL; t7R = rR; // Track 7 (>8k)
                    
                    stems_buffers[0].push_back(t0L); stems_buffers[0].push_back(t0R);
                    stems_buffers[1].push_back(t1L); stems_buffers[1].push_back(t1R);
                    stems_buffers[2].push_back(t2L); stems_buffers[2].push_back(t2R);
                    stems_buffers[3].push_back(t3L); stems_buffers[3].push_back(t3R);
                    stems_buffers[4].push_back(t4L); stems_buffers[4].push_back(t4R);
                    stems_buffers[5].push_back(t5L); stems_buffers[5].push_back(t5R);
                    stems_buffers[6].push_back(t6L); stems_buffers[6].push_back(t6R);
                    stems_buffers[7].push_back(t7L); stems_buffers[7].push_back(t7R);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(150)); 
            }
            
            progress = static_cast<float>(i + 1) / totalChunks;
        }
        // 4. DEEP PSYTRANCE EXTRACTION (Modo 8 Stems)
        if (mode == 8) {
            current_status = "KuroSpectralExtractor: Isolando FX, Synths e Zaps...";
            KuroUtils::Log("Ativando KuroSpectralExtractor em stems_buffers[3] (Outros)...");
            
            KuroAI::KuroSpectralExtractor::separateHarmonicPercussive(
                stems_buffers[3], stems_buffers[4], stems_buffers[5], sampleRate
            );
            
            // Opcional: Auto-Label nos logs (A Timeline pode ler isso depois)
            std::string label4 = KuroAI::KuroSpectralExtractor::autoLabel(stems_buffers[4], sampleRate);
            std::string label5 = KuroAI::KuroSpectralExtractor::autoLabel(stems_buffers[5], sampleRate);
            
            track_names[4] = label4;
            track_names[5] = label5;
            
            KuroUtils::Log("FX Separado! Faixa 4 (Harmonicos) Rotulada como: " + label4);
            KuroUtils::Log("FX Separado! Faixa 5 (Percussivos) Rotulada como: " + label5);
        }

        current_status = "Gerando visualizações (Waveforms CDJ)...";
        size_t visual_chunk_size = 882; // ~50 pixels por segundo
        
        master_waveform_overview.clear();
        size_t num_visual_samples = stems_buffers[0].size() / 2 / visual_chunk_size;
        master_waveform_overview.reserve(num_visual_samples);
        
        for (int track = 0; track < MAX_TRACKS; track++) {
            waveform_overview[track].clear();
            waveform_overview[track].reserve(num_visual_samples);
        }
        
        for (size_t v = 0; v < num_visual_samples; v++) {
            float master_max = 0.0f;
            float track_max[MAX_TRACKS] = {0.0f};
            
            for (size_t s = 0; s < visual_chunk_size; s++) {
                size_t audio_idx = (v * visual_chunk_size + s) * 2;
                float master_val = 0.0f;
                for (int track = 0; track < MAX_TRACKS; track++) {
                    if (audio_idx < stems_buffers[track].size()) {
                        float val = stems_buffers[track][audio_idx];
                        if (std::abs(val) > track_max[track]) track_max[track] = std::abs(val);
                        master_val += val;
                    }
                }
                if (std::abs(master_val) > master_max) master_max = std::abs(master_val);
            }
            
            master_waveform_overview.push_back(master_max);
            for (int track = 0; track < MAX_TRACKS; track++) {
                waveform_overview[track].push_back(track_max[track]);
            }
        }
        
        current_status = "Processamento Concluído. Tensors vetoriais em RAM!";
        KuroUtils::Log("Motor de Audio: Separacao de stems finalizada com sucesso.");
        
        // FASE 25 B: Adicionar Clips ao ClipManager automaticamente ao terminar o processamento
        extern ClipManager g_clip_manager;
        float duration_sec = (float)totalPCMFrameCount / (float)sampleRate;
        for (int i = 0; i < MAX_TRACKS; i++) {
            if (!stems_buffers[i].empty()) {
                g_clip_manager.initTrack(i, duration_sec);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(800)); 
        is_running = false;
        has_finished = true;
    }

public:
    std::vector<AudioSlice> pending_slices;
    
    StemSeparationEngine() = default;
    ~StemSeparationEngine() { stop(); }

    void startProcessing(const std::string& filepath, int mode = 4) {
        if (is_running) return;
        
        KuroUtils::Log("Motor de Audio: Iniciando carregamento de faixa: " + filepath);
        
        if (worker_thread.joinable()) worker_thread.join();
        worker_thread = std::thread(&StemSeparationEngine::processLoop, this, filepath, mode);
    }
    
    void stop() {
        is_running = false;
        if (worker_thread.joinable()) worker_thread.join();
    }
    
    void reset() {
        has_finished = false;
        progress = 0.0f;
        total_stem_frames = 0;
        detected_bpm = 0.0f;
        detected_key = "N/A";
        for(int i=0; i<MAX_TRACKS; i++) stems_buffers[i].clear();
    }
    
    // Acesso aos buffers
    const std::vector<float>& getStemBuffer(int track_index) const { return stems_buffers[track_index]; }
    const std::vector<float>& getWaveformOverview(int track_index) const { return waveform_overview[track_index]; }
    const std::vector<float>& getMasterWaveformOverview() const { return master_waveform_overview; }
    const std::vector<float>& getOriginalAudioBuffer() const { return original_audio_buffer; }
    
    // Recuperar Análise
    float getBPM() const { return detected_bpm; }
    std::string getKey() const { return detected_key; }

    // Exportar os canais isolados para WAV
    void exportStems(const std::string& output_folder, int num_tracks = 4) {
        if (!has_finished || total_stem_frames == 0) return;
        
        std::filesystem::create_directories(output_folder); // Use create_directories para paths profundos
        
        drwav_data_format format;
        format.container = drwav_container_riff;
        format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        format.channels = 2; // Stereo
        format.sampleRate = stem_sample_rate;
        format.bitsPerSample = 32;
        
        for (int i = 0; i < num_tracks; i++) {
            if (stems_buffers[i].empty()) continue;
            
            // Sanitiza o nome (Remove / ou \ que quebram o path)
            std::string safe_name = track_names[i];
            for(char& c : safe_name) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                    c = '_';
                }
            }
            
            std::string file_path = output_folder + "/" + std::to_string(i+1) + "_" + safe_name + ".wav";
            
            drwav wav;
            if (drwav_init_file_write(&wav, file_path.c_str(), &format, NULL)) {
                drwav_write_pcm_frames(&wav, stems_buffers[i].size() / 2, stems_buffers[i].data());
                drwav_uninit(&wav);
            }
        }
    }

    // Fatiador de Samples na Memória
    void generateOneShotsMemory(const std::string& song_name, int num_tracks = 8) {
        if (!has_finished || total_stem_frames == 0) return;
        
        pending_slices.clear();
        
        std::string safe_song = song_name;
        for(char& c : safe_song) if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') c = '_';
        
        float threshold = 0.05f; // -26dB aprox
        size_t min_silence_frames = stem_sample_rate * 0.15f; // 150ms of silence
        size_t padding_frames = stem_sample_rate * 0.05f; // 50ms padding
        
        for (int i = 0; i < num_tracks; i++) {
            if (stems_buffers[i].empty()) continue;
            
            std::string safe_track = track_names[i];
            for(char& c : safe_track) if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') c = '_';
            
            const auto& buffer = stems_buffers[i];
            size_t total_frames = buffer.size() / 2;
            
            bool in_onset = false;
            size_t onset_start = 0;
            size_t silence_counter = 0;
            int slice_index = 1;
            
            for (size_t f = 0; f < total_frames; f++) {
                float l = std::abs(buffer[f * 2]);
                float r = std::abs(buffer[f * 2 + 1]);
                float max_amp = std::max(l, r);
                
                if (max_amp > threshold) {
                    if (!in_onset) {
                        in_onset = true;
                        onset_start = (f > padding_frames) ? f - padding_frames : 0;
                    }
                    silence_counter = 0;
                } else if (in_onset) {
                    silence_counter++;
                    if (silence_counter > min_silence_frames || f == total_frames - 1) {
                        // Close slice
                        size_t onset_end = f + padding_frames;
                        if (onset_end > total_frames) onset_end = total_frames;
                        
                        size_t slice_length = onset_end - onset_start;
                        if (slice_length > stem_sample_rate * 0.1f) { // Ignore tiny clicks < 100ms
                            AudioSlice slice;
                            slice.source_track = i;
                            slice.suggested_name = safe_song + "_" + safe_track + "_" + std::to_string(slice_index++);
                            slice.selected = true;
                            
                            // Copy data
                            slice.data.assign(buffer.begin() + (onset_start * 2), buffer.begin() + (onset_end * 2));
                            
                            // Create mini waveform (100 points)
                            slice.waveform_preview.reserve(100);
                            size_t chunk_size = slice.data.size() / 2 / 100;
                            if (chunk_size == 0) chunk_size = 1;
                            
                            for (int w = 0; w < 100; w++) {
                                float pmax = 0.0f;
                                for (size_t k = 0; k < chunk_size; k++) {
                                    size_t idx = (w * chunk_size + k) * 2;
                                    if (idx < slice.data.size()) {
                                        if (std::abs(slice.data[idx]) > pmax) pmax = std::abs(slice.data[idx]);
                                    }
                                }
                                slice.waveform_preview.push_back(pmax);
                            }
                            
                            pending_slices.push_back(slice);
                        }
                        in_onset = false;
                        silence_counter = 0;
                    }
                }
            }
        }
        
        KuroUtils::Log("Auto-Slicer: Gerado " + std::to_string(pending_slices.size()) + " One-Shots na memoria.");
    }

    size_t getOriginalChannels() const { return original_channels; }
    size_t getTotalFrames() const { return total_stem_frames; }
    size_t getSampleRate() const { return stem_sample_rate; }
    
    float getProgress() const { return progress.load(); }
    bool isRunning() const { return is_running.load(); }
    bool hasFinished() const { return has_finished.load(); }
    std::string getStatus() const { return current_status; }
};
