#pragma once
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include "../ai/dr_wav.h"
#include "../plugin_manager/NativePlugins.h"

#include "../core/KuroConfig.h"
#include "../plugin_manager/DAG.h"

// Referências Globais
extern KuroDSP::AudioGraph master_graph;
extern float track_volumes[MAX_TRACKS];
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
extern bool play_test_tone;
extern float track_offsets[MAX_TRACKS];
extern std::unique_ptr<StemSeparationEngine> g_ai_engine;

class AudioExporter {
public:
    static void ExportMixdown(const std::string& filepath) {
        std::cout << "[Mixdown] Iniciando exportacao para: " << filepath << std::endl;
        
        drwav_data_format format;
        format.container = drwav_container_riff;
        format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        format.channels = 2;
        format.sampleRate = 44100;
        format.bitsPerSample = 32;
        
        drwav wav;
        if (!drwav_init_file_write(&wav, filepath.c_str(), &format, NULL)) {
            std::cerr << "Falha ao abrir arquivo para exportacao!" << std::endl;
            return;
        }

        if (!g_ai_engine || !g_ai_engine->hasFinished()) {
            std::cerr << "Erro: Nenhuma musica carregada ou IA nao finalizou o processamento!" << std::endl;
            drwav_uninit(&wav);
            return;
        }

        // Tamanho real da música
        unsigned int baseFrames = g_ai_engine->getTotalFrames();
        
        // Descobrir qual trilha foi empurrada mais longe na Timeline
        float max_offset = 0.0f;
        for (int i = 0; i < MAX_TRACKS; i++) {
            if (track_offsets[i] > max_offset) max_offset = track_offsets[i];
        }
        
        float pixels_per_second = 15.0f;
        long long max_offset_samples = (long long)((max_offset / pixels_per_second) * 44100.0f);
        
        unsigned int nFrames = baseFrames + max_offset_samples; 
        
        std::cout << "[Mixdown] Renderizando " << nFrames << " samples de áudio..." << std::endl;
        
        std::vector<float> pcmData(nFrames * 2); 
        
        bool any_solo = false;
        for(int i=0; i<MAX_TRACKS; i++) if (track_solos[i]) any_solo = true;
        
        for (unsigned int global_s = 0; global_s < nFrames; global_s++) {
            float mix_l = 0.0f;
            float mix_r = 0.0f;
            
            for (int t = 0; t < MAX_TRACKS; t++) {
                if (any_solo && !track_solos[t]) continue;
                if (track_mutes[t] && !track_solos[t]) continue; 
                
                float sample = 0.0f;
                
                // Matemática Temporal da Timeline (Offsetting offline)
                long long offset_in_samples = (long long)((track_offsets[t] / pixels_per_second) * 44100.0f);
                long long track_local_sample = (long long)global_s - offset_in_samples;
                
                if (track_local_sample >= 0) {
                    const auto& stem_buf = g_ai_engine->getStemBuffer(t);
                    size_t sample_index = track_local_sample * 2; // Stereo interleaved
                    if (sample_index < stem_buf.size()) {
                        sample = stem_buf[sample_index];
                    }
                }
                
                // Volume Master da Trilha
                float linear_vol = std::pow(10.0f, track_volumes[t] / 20.0f);
                sample *= linear_vol;
                
                mix_l += sample;
                mix_r += sample; // Copia L pra R já que extraímos apenas L como simplificação
            }
            
            if (mix_l > 1.0f) mix_l = 1.0f; if (mix_l < -1.0f) mix_l = -1.0f;
            if (mix_r > 1.0f) mix_r = 1.0f; if (mix_r < -1.0f) mix_r = -1.0f;
            
            pcmData[global_s * 2] = mix_l;
            pcmData[global_s * 2 + 1] = mix_r;
        }
        
        drwav_write_pcm_frames(&wav, nFrames, pcmData.data());
        drwav_uninit(&wav);
        
        std::cout << "[Mixdown] Concluido. Escritos " << nFrames << " frames estéreo." << std::endl;
    }

    static void ExportStemTrack(const std::string& filepath, int track_idx) {
        if (track_idx < 0 || track_idx >= 8) return;
        std::cout << "[Stem Export] Exportando Faixa " << (track_idx + 1) << " para: " << filepath << std::endl;

        drwav_data_format format;
        format.container = drwav_container_riff;
        format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        format.channels = 2;
        format.sampleRate = 44100;
        format.bitsPerSample = 32;

        drwav wav;
        if (!drwav_init_file_write(&wav, filepath.c_str(), &format, NULL)) {
            std::cerr << "Falha ao abrir arquivo para exportar Stem!" << std::endl;
            return;
        }

        if (!g_ai_engine || !g_ai_engine->hasFinished()) {
            std::cerr << "Erro: Nenhuma musica carregada ou IA nao finalizou o processamento!" << std::endl;
            drwav_uninit(&wav);
            return;
        }

        unsigned int baseFrames = g_ai_engine->getTotalFrames();
        float pixels_per_second = 15.0f;
        long long offset_in_samples = (long long)((track_offsets[track_idx] / pixels_per_second) * 44100.0f);
        unsigned int nFrames = baseFrames + (unsigned int)std::max(0LL, offset_in_samples);

        std::vector<float> pcmData(nFrames * 2, 0.0f);
        const auto& stem_buf = g_ai_engine->getStemBuffer(track_idx);
        float linear_vol = std::pow(10.0f, track_volumes[track_idx] / 20.0f);

        for (unsigned int global_s = 0; global_s < nFrames; global_s++) {
            long long track_local_sample = (long long)global_s - offset_in_samples;
            float sample = 0.0f;

            if (track_local_sample >= 0) {
                size_t sample_index = track_local_sample * 2;
                if (sample_index < stem_buf.size()) {
                    sample = stem_buf[sample_index] * linear_vol;
                }
            }

            if (sample > 1.0f) sample = 1.0f; if (sample < -1.0f) sample = -1.0f;
            pcmData[global_s * 2] = sample;
            pcmData[global_s * 2 + 1] = sample;
        }

        drwav_write_pcm_frames(&wav, nFrames, pcmData.data());
        drwav_uninit(&wav);
        std::cout << "[Stem Export] Stem Faixa " << (track_idx + 1) << " exportada com sucesso." << std::endl;
    }

    static void ExportAllStems(const std::string& directory_path) {
        std::cout << "[Stem Export Batch] Iniciando exportacao de todas as faixas em: " << directory_path << std::endl;
        const char* stem_names[8] = {
            "Kick_Drum.wav", "Snare_Clap.wav", "Percussion.wav", "Bassline.wav",
            "Chords_Keys.wav", "Lead_Synth.wav", "Acid_Arp.wav", "Sub_FX.wav"
        };
        for (int i = 0; i < 8; ++i) {
            std::string full_path = directory_path + "/" + stem_names[i];
            ExportStemTrack(full_path, i);
        }
        std::cout << "[Stem Export Batch] Concluido exportacao de 8 Stems." << std::endl;
    }
};
