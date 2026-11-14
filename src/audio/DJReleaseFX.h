#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroAudio {

    enum class DJReleaseFXType {
        NONE = 0,
        VINYL_BRAKE = 1,
        BACKSPIN = 2,
        ECHO_OUT = 3
    };

    // =========================================================================
    // DJReleaseFX: Efeitos de Transição de Saída Inspirados na Pioneer CDJ-3000
    // - Vinyl Brake: Desaceleração física e pitch bend realista de motor de vinil
    // - Backspin: Giro reverso rápido com amortecimento inercial
    // - Echo Out: Corte quantizado com cauda de eco/reverb flutuante
    // =========================================================================
    class DJReleaseFX {
    private:
        DJReleaseFXType active_type = DJReleaseFXType::NONE;
        bool is_active = false;

        // 1. Estado do Vinyl Brake
        double brake_speed_factor = 1.0;
        double brake_deceleration = 0.00008; // Taxa de atrito por sample

        // 2. Estado do Backspin
        double backspin_speed = -5.0;
        double backspin_friction = 0.9997;

        // 3. Estado do Echo Out
        static const int ECHO_BUF_SZ = 192000;
        std::vector<float> echo_buf_l, echo_buf_r;
        int echo_write_pos = 0;
        int echo_delay_samples = 24000;
        float echo_feedback = 0.65f;
        float echo_mix = 0.0f;
        bool echo_capturing = false;
        int echo_capture_remaining_samples = 0;

    public:
        DJReleaseFX() {
            echo_buf_l.assign(ECHO_BUF_SZ, 0.0f);
            echo_buf_r.assign(ECHO_BUF_SZ, 0.0f);
        }

        void reset() {
            active_type = DJReleaseFXType::NONE;
            is_active = false;
            brake_speed_factor = 1.0;
            backspin_speed = 0.0;
            echo_mix = 0.0f;
            echo_capturing = false;
            std::fill(echo_buf_l.begin(), echo_buf_l.end(), 0.0f);
            std::fill(echo_buf_r.begin(), echo_buf_r.end(), 0.0f);
        }

        void triggerVinylBrake(float duration_seconds = 0.8f, float sample_rate = 44100.0f) {
            active_type = DJReleaseFXType::VINYL_BRAKE;
            is_active = true;
            brake_speed_factor = 1.0;
            double total_samples = std::max(100.0f, duration_seconds * sample_rate);
            brake_deceleration = 1.0 / total_samples;
        }

        void triggerBackspin(float intensity = 5.0f) {
            active_type = DJReleaseFXType::BACKSPIN;
            is_active = true;
            backspin_speed = -(double)intensity;
        }

        void triggerEchoOut(double bpm, float sample_rate, float beats = 1.0f) {
            active_type = DJReleaseFXType::ECHO_OUT;
            is_active = true;
            echo_capturing = true;
            double beat_sec = 60.0 / std::clamp(bpm, 40.0, 240.0);
            echo_delay_samples = (int)(beat_sec * beats * sample_rate);
            if (echo_delay_samples < 64) echo_delay_samples = 64;
            if (echo_delay_samples > ECHO_BUF_SZ - 1) echo_delay_samples = ECHO_BUF_SZ - 1;
            echo_capture_remaining_samples = echo_delay_samples;
            echo_mix = 1.0f;
        }

        void trigger(DJReleaseFXType type, float duration_or_beats = 1.0f, float sr = 44100.0f, double bpm = 126.0) {
            if (type == DJReleaseFXType::VINYL_BRAKE) {
                triggerVinylBrake(0.8f, sr);
            } else if (type == DJReleaseFXType::BACKSPIN) {
                triggerBackspin(5.0f);
            } else if (type == DJReleaseFXType::ECHO_OUT) {
                triggerEchoOut(bpm, sr, duration_or_beats);
            }
        }

        bool isActive() const {
            return is_active;
        }

        DJReleaseFXType getType() const {
            return active_type;
        }

        // Retorna o multiplicador de velocidade de reprodução atual (pitch/motor)
        double getSpeedMultiplier() {
            if (!is_active) return 1.0;

            if (active_type == DJReleaseFXType::VINYL_BRAKE) {
                brake_speed_factor -= brake_deceleration;
                if (brake_speed_factor <= 0.001) {
                    brake_speed_factor = 0.0;
                    is_active = false; // Parou completamente o motor
                }
                return brake_speed_factor;
            }
            else if (active_type == DJReleaseFXType::BACKSPIN) {
                backspin_speed *= backspin_friction;
                if (std::abs(backspin_speed) <= 0.01) {
                    backspin_speed = 0.0;
                    is_active = false; // Parou o giro reverso
                }
                return backspin_speed;
            }
            return 1.0;
        }

        // Processa áudio para efeitos de cauda (Echo Out) com saturação analógica suave
        void processAudio(float* l, float* r, unsigned int frames) {
            if (!is_active || active_type != DJReleaseFXType::ECHO_OUT) return;

            float eff_fb = std::clamp(echo_feedback, 0.10f, 0.88f);

            for (unsigned int i = 0; i < frames; i++) {
                int read_pos = (echo_write_pos - echo_delay_samples + ECHO_BUF_SZ) % ECHO_BUF_SZ;
                float echo_l = echo_buf_l[read_pos];
                float echo_r = echo_buf_r[read_pos];

                // Saturação suave tipo fita impede estouro digital
                float sat_l = std::tanh(echo_l);
                float sat_r = std::tanh(echo_r);

                if (echo_capture_remaining_samples > 0) {
                    echo_capture_remaining_samples--;
                    // Captura inicial da batida
                    echo_buf_l[echo_write_pos] = l[i] + sat_l * eff_fb;
                    echo_buf_r[echo_write_pos] = r[i] + sat_r * eff_fb;
                    l[i] = std::clamp(l[i] + sat_l * 0.70f, -1.0f, 1.0f);
                    r[i] = std::clamp(r[i] + sat_r * 0.70f, -1.0f, 1.0f);
                } else {
                    echo_capturing = false;
                    // Cauda se dissipando no ar com o deck original cortado
                    echo_buf_l[echo_write_pos] = sat_l * eff_fb;
                    echo_buf_r[echo_write_pos] = sat_r * eff_fb;
                    l[i] = sat_l * echo_mix;
                    r[i] = sat_r * echo_mix;
                    echo_mix *= 0.99992f;
                    if (echo_mix < 0.001f) {
                        is_active = false;
                        echo_mix = 0.0f;
                    }
                }

                echo_write_pos = (echo_write_pos + 1) % ECHO_BUF_SZ;
            }
        }

        void process(float* l, float* r, unsigned int frames, float sr = 44100.0f) {
            processAudio(l, r, frames);
        }
    };

} // namespace KuroAudio
