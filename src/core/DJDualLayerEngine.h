#pragma once
#include <cmath>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    struct DJOnJogTelemetry {
        float vinyl_rotation_rad = 0.0f;  // Rotação física de 33 1/3 RPM (0 a 2*PI)
        float cue_marker_rad = 0.0f;      // Posição angular do ponto de CUE principal
        float slip_needle_rad = 0.0f;     // Posição da agulha fantasma durante Slip/Scratch
        bool is_on_air = false;           // Deck está audível na pista (On Air Halo Vermelho)
        bool slip_active = false;
        int active_layer = 0;             // 0 = Layer A, 1 = Layer B
        uint32_t layer_color_rgba = 0xFF00E5FF; // Ciano para Layer A, Magenta para Layer B
    };

    // =========================================================================
    // DJDualLayerEngine: Arquitetura Dual-Layer estilo Denon SC6000 Prime
    // e Telemetria de Display Central de Jog Wheel estilo Pioneer CDJ-3000
    // =========================================================================
    class DJDualLayerEngine {
    private:
        int active_layer_player[2] = { 0, 0 }; // Player 1 (0 ou 2), Player 2 (1 ou 3)
        DJOnJogTelemetry telemetry[4];

    public:
        DJDualLayerEngine() {
            telemetry[0].layer_color_rgba = 0xFF00E5FF; // Azul Ciano (Deck 1 - Layer A)
            telemetry[1].layer_color_rgba = 0xFF00E5FF; // Azul Ciano (Deck 2 - Layer A)
            telemetry[2].layer_color_rgba = 0xFFFF00D4; // Magenta Neon (Deck 3 - Layer B)
            telemetry[3].layer_color_rgba = 0xFFFF00D4; // Magenta Neon (Deck 4 - Layer B)
        }

        static DJDualLayerEngine& getInstance() {
            static DJDualLayerEngine instance;
            return instance;
        }

        // Alterna entre Camada A e Camada B para o Player especificado (0 = Esquerdo, 1 = Direito)
        int toggleLayer(int player_idx) {
            if (player_idx == 0) {
                // Alterna entre Deck 0 e Deck 2
                active_layer_player[0] = (active_layer_player[0] == 0) ? 2 : 0;
                return active_layer_player[0];
            } else {
                // Alterna entre Deck 1 e Deck 3
                active_layer_player[1] = (active_layer_player[1] == 1) ? 3 : 1;
                return active_layer_player[1];
            }
        }

        int getActiveDeckForPlayer(int player_idx) const {
            if (player_idx >= 0 && player_idx < 2) return active_layer_player[player_idx];
            return 0;
        }

        // Atualiza a física de rotação angular de 33 1/3 RPM e On-Air Status a cada frame de UI
        void updateTelemetry(int deck_idx, double current_frame, double cue_frame, double slip_frame,
                             double sample_rate, bool is_playing, bool is_slipping,
                             float volume_fader, float crossfader, bool crossfader_side_a) {
            if (deck_idx < 0 || deck_idx >= 4) return;

            auto& t = telemetry[deck_idx];
            t.slip_active = is_slipping;

            // 1. Rotação analógica perfeita de 33 1/3 RPM de vinil (1 volta a cada 1.8 segundos):
            if (sample_rate > 1000.0) {
                double seconds_per_rev = 60.0 / 33.333333; // ~1.8s
                double frames_per_rev = seconds_per_rev * sample_rate;

                t.vinyl_rotation_rad = (float)std::fmod(current_frame * (2.0 * M_PI) / frames_per_rev, 2.0 * M_PI);
                t.cue_marker_rad = (float)std::fmod(cue_frame * (2.0 * M_PI) / frames_per_rev, 2.0 * M_PI);

                if (is_slipping) {
                    t.slip_needle_rad = (float)std::fmod(slip_frame * (2.0 * M_PI) / frames_per_rev, 2.0 * M_PI);
                } else {
                    t.slip_needle_rad = t.vinyl_rotation_rad;
                }
            }

            // 2. Cálculo do halo vermelho ON-AIR da Pioneer CDJ-3000:
            // O deck está no ar se estiver tocando E com o fader aberto E com o crossfader do seu lado
            bool cf_allows = crossfader_side_a ? (crossfader < 0.95f) : (crossfader > 0.05f);
            t.is_on_air = is_playing && (volume_fader > 0.05f) && cf_allows;
        }

        DJOnJogTelemetry getTelemetry(int deck_idx) const {
            if (deck_idx >= 0 && deck_idx < 4) return telemetry[deck_idx];
            return {};
        }
    };

} // namespace KuroAudio
