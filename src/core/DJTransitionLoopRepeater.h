#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <atomic>
#include "imgui.h"

namespace KuroAudio {

    struct GhostControlTarget {
        float crossfader = 0.5f;     // [0.0 .. 1.0]
        float deck_a_low = 1.0f;     // [0.0 .. 1.0]
        float deck_b_low = 0.0f;     // [0.0 .. 1.0]
        float deck_a_vol = 1.0f;     // [0.0 .. 1.0]
        float deck_b_vol = 1.0f;     // [0.0 .. 1.0]
    };

    class DJTransitionLoopRepeater {
    private:
        DJTransitionLoopRepeater() = default;

    public:
        static DJTransitionLoopRepeater& getInstance() {
            static DJTransitionLoopRepeater instance;
            return instance;
        }

        bool is_active = false;
        int loop_bars = 16;               // 8, 16 ou 32 compassos
        double transition_start_sec = 0.0;
        double transition_end_sec = 0.0;
        int repeat_count = 0;
        int target_repeats = 5;

        // Telemetria de Gamificação estilo Rocksmith
        float current_score = 0.0f;       // [0 .. 100%]
        int streak = 0;
        std::string feedback_message = "Pronto para a virada!";
        ImVec4 feedback_color = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);

        // Bass Clash Guard (Detector de Choque de Graves)
        bool bass_clash_detected = false;
        float bass_clash_penalty = 0.0f;

        void startTransitionPractice(double start_sec, double end_sec, int bars = 16) {
            is_active = true;
            transition_start_sec = start_sec;
            transition_end_sec = end_sec;
            loop_bars = bars;
            repeat_count = 0;
            current_score = 100.0f;
            streak = 0;
            feedback_message = "Ouça o compasso e prepare o Bass Swap!";
            feedback_color = ImVec4(0.2f, 1.0f, 0.5f, 1.0f);
        }

        void stopPractice() {
            is_active = false;
        }

        // Calcula a posição ideal dos controles "Fantasmas" no instante atual da virada
        GhostControlTarget getGhostTargetAtProgress(float progress /* 0.0 a 1.0 */) {
            GhostControlTarget target;
            progress = std::clamp(progress, 0.0f, 1.0f);

            // Transição típica de 16 compassos:
            // 0% a 50%: Faixa A tocando full; Faixa B entra com Low cortado a zero
            // 50% a 75%: BASS SWAP no Drop! Low de A corta a zero, Low de B sobe a 100%
            // 75% a 100%: Faixa A é retirada suavemente
            if (progress < 0.50f) {
                target.deck_a_vol = 1.0f;
                target.deck_b_vol = progress * 2.0f; // B sobe volume até 100%
                target.deck_a_low = 1.0f;
                target.deck_b_low = 0.0f;           // Grave de B zerado!
                target.crossfader = 0.25f + progress * 0.5f;
            } else if (progress < 0.75f) {
                // Momento do Bass Swap!
                float swap_t = (progress - 0.50f) / 0.25f;
                target.deck_a_vol = 1.0f;
                target.deck_b_vol = 1.0f;
                target.deck_a_low = 1.0f - swap_t;  // A perde grave
                target.deck_b_low = swap_t;         // B ganha grave com impacto
                target.crossfader = 0.50f + swap_t * 0.25f;
            } else {
                float out_t = (progress - 0.75f) / 0.25f;
                target.deck_a_vol = 1.0f - out_t;   // A sai
                target.deck_b_vol = 1.0f;
                target.deck_a_low = 0.0f;
                target.deck_b_low = 1.0f;
                target.crossfader = 0.75f + out_t * 0.25f;
            }

            return target;
        }

        // Avalia o desempenho do DJ e monitora Bass Clash a cada frame
        void evaluateFrame(float actual_a_low, float actual_b_low,
                           float actual_a_vol, float actual_b_vol,
                           float progress) {
            if (!is_active) return;

            // 1. Verificação do Choque de Graves (Bass Clash)
            // Se ambos os graves estiverem acima de 65% ao mesmo tempo
            if (actual_a_low > 0.65f && actual_b_low > 0.65f && actual_a_vol > 0.5f && actual_b_vol > 0.5f) {
                bass_clash_detected = true;
                bass_clash_penalty += 0.8f;
                current_score = std::max(0.0f, current_score - 0.8f);
                streak = 0;
                feedback_message = "ALERTA: CHOQUE DE GRAVES! Corte o LOW de uma das faixas!";
                feedback_color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
            } else {
                bass_clash_detected = false;
            }

            // 2. Pontuação baseada na precisão com o alvo fantasma
            GhostControlTarget ghost = getGhostTargetAtProgress(progress);
            float diff_low_a = std::abs(actual_a_low - ghost.deck_a_low);
            float diff_low_b = std::abs(actual_b_low - ghost.deck_b_low);

            if (!bass_clash_detected) {
                if (diff_low_a < 0.20f && diff_low_b < 0.20f) {
                    streak++;
                    current_score = std::min(100.0f, current_score + 0.15f);
                    if (streak > 60) {
                        feedback_message = "PERFEITO! Bass Swap cirúrgico estilo DJ Jayboo!";
                        feedback_color = ImVec4(0.2f, 1.0f, 0.4f, 1.0f);
                    }
                } else {
                    feedback_message = "Ajuste os knobs de grave para seguir os alvos fantasma";
                    feedback_color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
                }
            }
        }
    };

} // namespace KuroAudio

