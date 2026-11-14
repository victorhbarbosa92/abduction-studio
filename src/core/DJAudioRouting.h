#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <cstring>

namespace KuroAudio {

    enum class DJOutputChannelMode {
        FOUR_CHANNEL = 0,      // Canais 1-2: Master Out (Pista) | Canais 3-4: Headphone Cue (Fone)
        SPLIT_CUE = 1,         // Canal 1 (Esq): Master Mono | Canal 2 (Dir): Cue Mono (Uso com cabo Split P2)
        STEREO_COMBINED = 2    // 2 Canais: Master e Cue somados com balanço configurável
    };

    class DJAudioRouting {
    private:
        std::atomic<DJOutputChannelMode> channel_mode{ DJOutputChannelMode::FOUR_CHANNEL };
        std::atomic<bool> cue_deck_active[4]{ {true}, {false}, {false}, {false} };
        std::atomic<bool> cue_master_active{ true };
        std::atomic<float> cue_mix{ 0.5f };            // 0.5f = Balanço equilibrado Cue Decks + Master no fone
        std::atomic<float> headphone_volume{ 1.0f };  // Volume dos fones de ouvido (0.0 a 1.5)
        std::atomic<float> booth_volume{ 0.9f };        // Volume do retorno de cabine (Booth Monitor)
        std::atomic<bool> split_cue_enabled{ false };
        std::atomic<bool> booth_mute{ false };

    public:
        DJAudioRouting() = default;

        static DJAudioRouting& getInstance() {
            static DJAudioRouting instance;
            return instance;
        }

        void setChannelMode(DJOutputChannelMode mode) {
            channel_mode.store(mode);
            split_cue_enabled.store(mode == DJOutputChannelMode::SPLIT_CUE);
        }

        DJOutputChannelMode getChannelMode() const {
            return channel_mode.load();
        }

        void setDeckCue(int deck_idx, bool active) {
            if (deck_idx >= 0 && deck_idx < 4) {
                cue_deck_active[deck_idx].store(active);
            }
        }

        bool isDeckCueActive(int deck_idx) const {
            if (deck_idx >= 0 && deck_idx < 4) {
                return cue_deck_active[deck_idx].load();
            }
            return false;
        }

        void toggleDeckCue(int deck_idx) {
            if (deck_idx >= 0 && deck_idx < 4) {
                bool curr = cue_deck_active[deck_idx].load();
                cue_deck_active[deck_idx].store(!curr);
            }
        }

        void setMasterCue(bool active) {
            cue_master_active.store(active);
        }

        bool isMasterCueActive() const {
            return cue_master_active.load();
        }

        void setCueMix(float mix) {
            cue_mix.store(std::clamp(mix, 0.0f, 1.0f));
        }

        float getCueMix() const {
            return cue_mix.load();
        }

        void setHeadphoneVolume(float vol) {
            headphone_volume.store(std::clamp(vol, 0.0f, 2.0f));
        }

        float getHeadphoneVolume() const {
            return headphone_volume.load();
        }

        float getHeadphoneGain() const {
            return headphone_volume.load();
        }

        void setHeadphoneGain(float gain) {
            setHeadphoneVolume(gain);
        }

        bool isSplitCue() const {
            return split_cue_enabled.load() || (channel_mode.load() == DJOutputChannelMode::SPLIT_CUE);
        }

        void setSplitCue(bool enabled) {
            split_cue_enabled.store(enabled);
            channel_mode.store(enabled ? DJOutputChannelMode::SPLIT_CUE : DJOutputChannelMode::FOUR_CHANNEL);
        }

        void setBoothVolume(float vol) {
            booth_volume.store(std::clamp(vol, 0.0f, 2.0f));
        }

        float getBoothVolume() const {
            return booth_volume.load();
        }

        void setBoothMute(bool mute) {
            booth_mute.store(mute);
        }

        bool isBoothMuted() const {
            return booth_mute.load();
        }

        // =====================================================================
        // Processamento Central de Roteamento de Barramentos de Áudio
        // =====================================================================
        // deck_blocks: array com 4 decks, cada um com canais [0=Left, 1=Right]
        // master_l, master_r: barramento estéreo Master (pós fader e limitador)
        // out_master_l, out_master_r: saída física 1-2 (Master)
        // out_cue_l, out_cue_r: saída física 3-4 (Headphones)
        void process(const float* const deck_blocks[4][2],
                     const float* master_l, const float* master_r,
                     float* out_master_l, float* out_master_r,
                     float* out_cue_l, float* out_cue_r,
                     unsigned int frames) {

            DJOutputChannelMode mode = channel_mode.load();
            float hp_vol = headphone_volume.load();
            float mix_val = cue_mix.load(); // 0.0 = Pure Cue, 1.0 = Pure Master
            bool cue_m = cue_master_active.load();

            // 1. Somatória dos Decks selecionados para CUE (Pré-Fader Listen - PFL)
            std::vector<float> cue_pfl_l(frames, 0.0f);
            std::vector<float> cue_pfl_r(frames, 0.0f);

            bool any_deck_cue = false;
            for (int d = 0; d < 4; d++) {
                if (cue_deck_active[d].load()) {
                    any_deck_cue = true;
                    const float* d_l = deck_blocks[d][0];
                    const float* d_r = deck_blocks[d][1];
                    if (d_l && d_r) {
                        for (unsigned int i = 0; i < frames; i++) {
                            cue_pfl_l[i] += d_l[i];
                            cue_pfl_r[i] += d_r[i];
                        }
                    }
                }
            }

            // 2. Mixagem de Fone (Cue PFL balanceado com o Master)
            float cue_weight = std::cos(mix_val * 1.5707963f);   // 1.0 -> 0.0
            float master_weight = std::sin(mix_val * 1.5707963f); // 0.0 -> 1.0

            if (cue_m) {
                // Se o botão MASTER CUE estiver explicitamente aceso
                master_weight = std::max(master_weight, 0.85f);
            }

            std::vector<float> final_hp_l(frames, 0.0f);
            std::vector<float> final_hp_r(frames, 0.0f);

            for (unsigned int i = 0; i < frames; i++) {
                float pfl_l = cue_pfl_l[i] * cue_weight;
                float pfl_r = cue_pfl_r[i] * cue_weight;
                float mst_l = master_l[i] * master_weight;
                float mst_r = master_r[i] * master_weight;

                final_hp_l[i] = (pfl_l + mst_l) * hp_vol;
                final_hp_r[i] = (pfl_r + mst_r) * hp_vol;
            }

            // 3. Distribuição para os Canais Físicos de Saída
            if (mode == DJOutputChannelMode::FOUR_CHANNEL) {
                // Modo 4 Canais: Master nos canais 1-2, Headphone nos canais 3-4
                for (unsigned int i = 0; i < frames; i++) {
                    out_master_l[i] = master_l[i];
                    out_master_r[i] = master_r[i];
                    if (out_cue_l && out_cue_r) {
                        out_cue_l[i] = final_hp_l[i];
                        out_cue_r[i] = final_hp_r[i];
                    }
                }
            }
            else if (mode == DJOutputChannelMode::SPLIT_CUE) {
                // Modo Split CUE (Cabo Split P2 com saída de 2 canais):
                // Canal Esquerdo (1) = Master Mono | Canal Direito (2) = Fone Cue Mono
                for (unsigned int i = 0; i < frames; i++) {
                    float master_mono = (master_l[i] + master_r[i]) * 0.5f;
                    float cue_mono    = (final_hp_l[i] + final_hp_r[i]) * 0.5f;

                    out_master_l[i] = master_mono;
                    out_master_r[i] = cue_mono;

                    if (out_cue_l && out_cue_r) {
                        out_cue_l[i] = cue_mono;
                        out_cue_r[i] = cue_mono;
                    }
                }
            }
            else { // STEREO_COMBINED
                for (unsigned int i = 0; i < frames; i++) {
                    out_master_l[i] = master_l[i];
                    out_master_r[i] = master_r[i];
                    if (out_cue_l && out_cue_r) {
                        out_cue_l[i] = final_hp_l[i];
                        out_cue_r[i] = final_hp_r[i];
                    }
                }
            }
        }
    };

} // namespace KuroAudio
