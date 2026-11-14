#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "imgui.h"

namespace KuroAudio {

    enum class ScratchTechnique {
        BABY_SCRATCH = 0,
        CHIRP_SCRATCH,
        FLARE_SCRATCH,
        CRAB_SCRATCH
    };

    struct ScratchHighwayNote {
        double time_sec = 0.0;
        float vinyl_direction = 1.0f; // +1.0 = empurrar para frente, -1.0 = puxar para trás
        bool fader_cut = false;       // se o crossfader deve estar cortado neste instante
        bool hit = false;
    };

    class DJTurntablismEngine {
    private:
        DJTurntablismEngine() = default;

    public:
        static DJTurntablismEngine& getInstance() {
            static DJTurntablismEngine instance;
            return instance;
        }

        // =========================================================================
        // 1. SIMULADOR FÍSICO DE SPINBACK (RETROCESSO RÁPIDO COM INÉRCIA ANGULAR)
        // =========================================================================
        struct SpinbackState {
            bool active = false;
            float angular_velocity = 0.0f; // rad/s
            float friction = 2.8f;
        };

        SpinbackState spinback[4];

        void triggerSpinback(int deck_idx, float initial_speed = -35.0f) {
            if (deck_idx < 0 || deck_idx >= 4) return;
            spinback[deck_idx].active = true;
            spinback[deck_idx].angular_velocity = initial_speed;
        }

        double updateSpinback(int deck_idx, double current_frame, double sample_rate, float dt) {
            if (deck_idx < 0 || deck_idx >= 4 || !spinback[deck_idx].active) return current_frame;

            auto& sb = spinback[deck_idx];
            // Decaimento por atrito mecânico
            sb.angular_velocity *= std::exp(-sb.friction * dt);

            // Atualiza posição do áudio
            double delta_frames = (double)sb.angular_velocity * (sample_rate * 0.08) * (double)dt;
            double new_frame = std::max(0.0, current_frame + delta_frames);

            if (std::abs(sb.angular_velocity) < 0.2f) {
                sb.active = false;
                sb.angular_velocity = 0.0f;
            }

            return new_frame;
        }

        // =========================================================================
        // 2. VINYL BRAKE (PARADA MECÂNICA DE MOTOR OFF)
        // =========================================================================
        struct VinylBrakeState {
            bool active = false;
            float speed_factor = 1.0f;
            float brake_time_sec = 1.5f;
            float elapsed_sec = 0.0f;
        };

        VinylBrakeState vinyl_brake[4];

        void triggerVinylBrake(int deck_idx, float duration_sec = 1.5f) {
            if (deck_idx < 0 || deck_idx >= 4) return;
            vinyl_brake[deck_idx].active = true;
            vinyl_brake[deck_idx].speed_factor = 1.0f;
            vinyl_brake[deck_idx].brake_time_sec = duration_sec;
            vinyl_brake[deck_idx].elapsed_sec = 0.0f;
        }

        float updateVinylBrake(int deck_idx, float dt) {
            if (deck_idx < 0 || deck_idx >= 4 || !vinyl_brake[deck_idx].active) return 1.0f;

            auto& vb = vinyl_brake[deck_idx];
            vb.elapsed_sec += dt;
            float progress = std::clamp(vb.elapsed_sec / vb.brake_time_sec, 0.0f, 1.0f);
            // Queda exponencial natural da inércia do motor
            vb.speed_factor = std::max(0.0f, (1.0f - progress) * (1.0f - progress));

            if (progress >= 1.0f) {
                vb.active = false;
                vb.speed_factor = 0.0f;
            }

            return vb.speed_factor;
        }

        bool isBraking(int deck_idx = 0) const {
            if (deck_idx >= 0 && deck_idx < 4) return vinyl_brake[deck_idx].active;
            return false;
        }

        bool isBackspinning(int deck_idx = 0) const {
            if (deck_idx >= 0 && deck_idx < 4) return spinback[deck_idx].active;
            return false;
        }

        void reset() {
            for (int i = 0; i < 4; i++) {
                spinback[i].active = false;
                spinback[i].angular_velocity = 0.0f;
                vinyl_brake[i].active = false;
                vinyl_brake[i].speed_factor = 1.0f;
            }
        }

        // =========================================================================
        // 3. SCRATCH HIGHWAY (TRILHA GUIA DE CORTES ESTILO GUITAR HERO)
        // =========================================================================
        std::vector<ScratchHighwayNote> highway_notes;
        bool highway_active = false;
        ScratchTechnique current_technique = ScratchTechnique::BABY_SCRATCH;

        void loadScratchDrill(ScratchTechnique tech) {
            current_technique = tech;
            highway_notes.clear();
            highway_active = true;

            // Gera sequência de notas guia para o treino selecionado
            switch (tech) {
                case ScratchTechnique::BABY_SCRATCH: {
                    for (int i = 0; i < 8; i++) {
                        ScratchHighwayNote n_fwd;
                        n_fwd.time_sec = (double)i * 0.5;
                        n_fwd.vinyl_direction = 1.0f;
                        n_fwd.fader_cut = false;
                        highway_notes.push_back(n_fwd);

                        ScratchHighwayNote n_bwd;
                        n_bwd.time_sec = (double)i * 0.5 + 0.25;
                        n_bwd.vinyl_direction = -1.0f;
                        n_bwd.fader_cut = false;
                        highway_notes.push_back(n_bwd);
                    }
                    break;
                }
                case ScratchTechnique::CHIRP_SCRATCH: {
                    for (int i = 0; i < 8; i++) {
                        ScratchHighwayNote n1; // Empurra com fader aberto
                        n1.time_sec = (double)i * 0.6;
                        n1.vinyl_direction = 1.0f;
                        n1.fader_cut = false;
                        highway_notes.push_back(n1);

                        ScratchHighwayNote n2; // Puxa com corte seco no fader
                        n2.time_sec = (double)i * 0.6 + 0.3;
                        n2.vinyl_direction = -1.0f;
                        n2.fader_cut = true;
                        highway_notes.push_back(n2);
                    }
                    break;
                }
                default: break;
            }
        }
    };

} // namespace KuroAudio

