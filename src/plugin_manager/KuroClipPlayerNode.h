#pragma once
#include "DAG.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"
#include <vector>
#include <memory>
#include <algorithm>

#include "../core/KuroConfig.h"
extern KuroDSP::Pedalboard track_pedalboards[MAX_TRACKS];

namespace KuroDSP {

    class KuroClipPlayerNode : public PluginNode {
    private:
        TimelineManager* timeline;
        ClipManager* clip_manager;
        int track_idx;
        const std::vector<float>* source_buffer;
        float sample_rate;

    public:
        KuroClipPlayerNode(const std::string& id, const std::string& name, TimelineManager* tl, ClipManager* cm, int track, const std::vector<float>* buffer, float sr = 44100.0f) 
            : PluginNode(id, name), timeline(tl), clip_manager(cm), track_idx(track), source_buffer(buffer), sample_rate(sr) {}
        
        void process(float* left, float* right, unsigned int frames) override {
            extern bool track_mutes[MAX_TRACKS];
            extern bool track_solos[MAX_TRACKS];
            
            bool any_solo = false;
            for (int i = 0; i < MAX_TRACKS; i++) {
                if (track_solos[i]) { any_solo = true; break; }
            }
            
            if (!timeline->getPlaying() || track_mutes[track_idx] || (any_solo && !track_solos[track_idx])) {
                // Preenche com zeros caso não esteja tocando, mutado ou solado por outra faixa
                std::fill(left, left + frames, 0.0f);
                std::fill(right, right + frames, 0.0f);
                return;
            }

            uint64_t start_frame = timeline->getMasterFrame(); // Posição atual da timeline
            
            // Pega clips (uma cópia rápida thread-safe, ou lock rápido. O ideal é lock-free depois)
            auto clips = clip_manager->getClips(track_idx);
            
            // Inicializa a saída local do player em 0
            std::fill(left, left + frames, 0.0f);
            std::fill(right, right + frames, 0.0f);

            if (!source_buffer || source_buffer->empty()) return;

            for (const auto& clip : clips) {
                uint64_t clip_start_frame = (uint64_t)(clip.start_time_sec * sample_rate);
                uint64_t clip_len_frames = (uint64_t)(clip.length_sec * sample_rate);
                uint64_t clip_end_frame = clip_start_frame + clip_len_frames;

                // Verifica intersecção entre [start_frame, start_frame + frames] e [clip_start_frame, clip_end_frame]
                if (start_frame + frames > clip_start_frame && start_frame < clip_end_frame) {
                    
                    unsigned int frame_offset = 0; // Onde escrever no bloco (0 a frames-1)
                    unsigned int frames_to_write = frames;
                    
                    uint64_t read_start = start_frame;

                    // Ajusta se o clipe começar NO MEIO do bloco
                    if (clip_start_frame > start_frame) {
                        frame_offset = (unsigned int)(clip_start_frame - start_frame);
                        frames_to_write -= frame_offset;
                        read_start = clip_start_frame;
                    }
                    
                    // Ajusta se o clipe terminar NO MEIO do bloco
                    if (start_frame + frames > clip_end_frame) {
                        unsigned int diff = (unsigned int)((start_frame + frames) - clip_end_frame);
                        if (diff < frames_to_write) frames_to_write -= diff;
                    }

                    // Calcula offset de onde ler no buffer original
                    // (read_start - clip_start_frame) dá o progresso atual dentro do clip (em frames)
                    uint64_t offset_in_clip = read_start - clip_start_frame;
                    uint64_t source_start_frame = (uint64_t)(clip.source_offset_sec * sample_rate) + offset_in_clip;
                    
                    // O buffer do StemSeparationEngine é Stereo Interleaved (L, R, L, R)
                    for (unsigned int i = 0; i < frames_to_write; ++i) {
                        size_t src_idx = (source_start_frame + i) * 2;
                        if (src_idx + 1 < source_buffer->size()) {
                            left[frame_offset + i] += (*source_buffer)[src_idx];
                            right[frame_offset + i] += (*source_buffer)[src_idx + 1];
                        }
                    }
                }
            }
            
            // FASE 28: Processar a cadeia de efeitos (Pedalboard) da faixa
            ::track_pedalboards[track_idx].process(left, right, frames);
        }
        
        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {}
    };
}
