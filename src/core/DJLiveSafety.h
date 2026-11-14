#pragma once
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace KuroAudio {

    class DJLiveSafety {
    private:
        bool load_lock_enabled = false;
        bool emergency_loop_enabled = true;
        bool fader_start_enabled = false;

    public:
        DJLiveSafety() = default;

        static DJLiveSafety& getInstance() {
            static DJLiveSafety instance;
            return instance;
        }

        void setLoadLock(bool enabled) {
            load_lock_enabled = enabled;
        }

        bool isLoadLockEnabled() const {
            return load_lock_enabled;
        }

        void setEmergencyLoopEnabled(bool enabled) {
            emergency_loop_enabled = enabled;
        }

        bool isEmergencyLoopEnabled() const {
            return emergency_loop_enabled;
        }

        void toggleLoadLock() {
            load_lock_enabled = !load_lock_enabled;
        }

        // =====================================================================
        // 1. LOAD LOCK: Impede carregar acidentalmente uma música no deck que está
        // tocando ao vivo na pista com volume aberto.
        // =====================================================================
        bool canLoadTrack(int deck_idx, bool is_playing, float volume, std::string& out_error_msg) const {
            if (!load_lock_enabled) return true;

            if (is_playing && volume > 0.05f) {
                out_error_msg = "TRAVA DE SEGURANÇA (LOAD LOCK): Deck " + std::to_string(deck_idx + 1) + 
                                " está tocando na pista. Pause o deck ou feche o fader antes de carregar.";
                return false;
            }
            return true;
        }

        template <typename TDeck>
        bool canLoadTrack(const TDeck& deck) const {
            if (!load_lock_enabled) return true;
            // Deck sem buffer ou duração nula NUNCA está tocando na pista de fato
            if (deck.buffer_l.empty() || deck.duration_sec <= 0.0 || deck.track_title.empty() || deck.track_title == "Nenhuma Faixa Carregada") {
                return true;
            }
            if (deck.is_playing && deck.volume > 0.05f) {
                return false;
            }
            return true;
        }

        template <typename TDeck>
        bool canLoadTrack(const TDeck& deck, std::string& out_error_msg) const {
            if (!load_lock_enabled) return true;
            // Deck sem buffer ou duração nula NUNCA está tocando na pista de fato
            if (deck.buffer_l.empty() || deck.duration_sec <= 0.0 || deck.track_title.empty() || deck.track_title == "Nenhuma Faixa Carregada") {
                return true;
            }
            return canLoadTrack(deck.deck_id, deck.is_playing, deck.volume, out_error_msg);
        }

        // =====================================================================
        // 2. EMERGENCY LOOP: Se o arquivo terminar ou houver erro de leitura,
        // ativa automaticamente um loop de 4 tempos para a pista não parar.
        // =====================================================================
        bool checkEmergencyLoopTrigger(double current_frame, double total_frames, double bpm, double sample_rate,
                                       bool& loop_active, double& loop_start_frame, double& loop_length_frames) const {
            if (!emergency_loop_enabled || total_frames <= 0 || bpm < 20.0) return false;

            double frames_per_beat = (60.0 / bpm) * sample_rate;
            double four_bars_frames = frames_per_beat * 16.0; // 4 compassos

            // Se estiver a menos de 2 compassos do final da música e o loop não estiver ativo
            if (!loop_active && current_frame >= (total_frames - frames_per_beat * 8.0)) {
                loop_start_frame = std::max(0.0, current_frame - four_bars_frames);
                loop_length_frames = four_bars_frames;
                loop_active = true;
                return true; // Disparou loop de emergência
            }
            return false;
        }

        template <typename TDeck>
        bool triggerEmergencyLoop(TDeck& deck, double beats = 4.0) const {
            if (!emergency_loop_enabled || deck.buffer_l.empty() || deck.bpm < 20.0) return false;
            double frames_per_beat = (60.0 / deck.bpm) * deck.sample_rate;
            double loop_len = frames_per_beat * beats;
            deck.loop_start_frame = std::max(0.0, deck.current_frame - loop_len);
            deck.loop_length_frames = loop_len;
            deck.loop_active = true;
            return true;
        }

        // =====================================================================
        // 3. BEAT JUMP: Salto temporal quantizado para frente (+) ou para trás (-)
        // Suporta 1, 2, 4, 8, 16 ou 32 batidas mantendo a fase perfeitamente alinhada.
        // =====================================================================
        double executeBeatJump(int beats, double current_frame, double bpm, double sample_rate, double total_frames) const {
            if (bpm < 20.0 || sample_rate < 1000.0) return current_frame;

            double beat_len_frames = (60.0 / bpm) * sample_rate;
            double jump_frames = (double)beats * beat_len_frames;
            double target_frame = current_frame + jump_frames;

            if (target_frame < 0.0) target_frame = 0.0;
            if (target_frame > total_frames) target_frame = total_frames;

            return target_frame;
        }

        template <typename TDeck>
        void quantizedBeatJump(TDeck& deck, double beats, double sample_rate) const {
            double total_f = (double)deck.buffer_l.size();
            deck.current_frame = executeBeatJump((int)beats, deck.current_frame, deck.bpm, sample_rate, total_f);
        }
    };

} // namespace KuroAudio
