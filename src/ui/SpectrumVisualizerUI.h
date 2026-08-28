#pragma once
#include "imgui.h"
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <chrono>

namespace KuroUI {

    class SpectrumVisualizerUI {
    public:
        static constexpr size_t FFT_SIZE = 512;
        static constexpr size_t NUM_BANDS = 64;
        
        float audio_ring_l[FFT_SIZE] = {0.0f};
        float audio_ring_r[FFT_SIZE] = {0.0f};
        std::atomic<size_t> ring_write_pos{0};
        
        std::vector<float> spectrum_mags;
        std::vector<float> spectrum_decay;
        
        // Vectorscope / Goniometer points
        std::vector<ImVec2> goniometer_pts;
        
        bool is_visible = false;
        
        // Fast LUT tables for zero runtime sin/cos calculations
        float hann_window[FFT_SIZE];
        float dft_cos_lut[NUM_BANDS][FFT_SIZE / 2];
        float dft_sin_lut[NUM_BANDS][FFT_SIZE / 2];
        std::chrono::steady_clock::time_point last_fft_time;

        SpectrumVisualizerUI() {
            spectrum_mags.assign(NUM_BANDS, 0.0f);
            spectrum_decay.assign(NUM_BANDS, 0.0f);
            goniometer_pts.resize(FFT_SIZE);
            last_fft_time = std::chrono::steady_clock::now();

            // Precompute Hann window
            for (size_t i = 0; i < FFT_SIZE; i++) {
                hann_window[i] = 0.5f * (1.0f - std::cos(2.0f * 3.14159265f * i / (FFT_SIZE - 1)));
            }

            // Precompute DFT trigonometric tables for 64 log-spaced bins
            for (size_t b = 0; b < NUM_BANDS; b++) {
                float freq_ratio = (float)b / (float)NUM_BANDS;
                int k = (int)(1 + freq_ratio * freq_ratio * (FFT_SIZE / 2 - 1));
                k = std::clamp(k, 1, (int)(FFT_SIZE / 2 - 1));

                for (size_t step = 0; step < FFT_SIZE / 2; step++) {
                    size_t n = step * 2;
                    float angle = 2.0f * 3.14159265f * k * n / FFT_SIZE;
                    dft_cos_lut[b][step] = std::cos(angle);
                    dft_sin_lut[b][step] = std::sin(angle);
                }
            }
        }

        // Push real-time audio samples from audio callback (100% Lock-Free)
        inline void pushAudioSamples(float l, float r) {
            size_t pos = ring_write_pos.load(std::memory_order_relaxed);
            audio_ring_l[pos] = l;
            audio_ring_r[pos] = r;
            ring_write_pos.store((pos + 1) % FFT_SIZE, std::memory_order_relaxed);
        }

        // Compute FFT magnitudes and vectorscope points (Throttled & LUT-accelerated)
        void updateFFT() {
            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fft_time).count();
            if (elapsed_ms < 30) {
                return; // Throttle to ~33 FPS max to save CPU
            }
            last_fft_time = now;

            size_t current_pos = ring_write_pos.load(std::memory_order_relaxed);
            float windowed[FFT_SIZE];
            
            for (size_t i = 0; i < FFT_SIZE; i++) {
                size_t idx = (current_pos + i) % FFT_SIZE;
                float mono = 0.5f * (audio_ring_l[idx] + audio_ring_r[idx]);
                windowed[i] = mono * hann_window[i];

                // Vectorscope point (M-S mapping)
                float side = 0.5f * (audio_ring_l[idx] - audio_ring_r[idx]);
                float mid = mono;
                goniometer_pts[i] = ImVec2(side, mid);
            }

            // Ultra-fast DFT calculation via precomputed LUT
            for (size_t b = 0; b < NUM_BANDS; b++) {
                float re = 0.0f, im = 0.0f;
                const float* cos_row = dft_cos_lut[b];
                const float* sin_row = dft_sin_lut[b];

                for (size_t step = 0; step < FFT_SIZE / 2; step++) {
                    float w = windowed[step * 2];
                    re += w * cos_row[step];
                    im -= w * sin_row[step];
                }

                float mag = std::sqrt(re * re + im * im) / (FFT_SIZE * 0.15f);
                mag = std::clamp(mag, 0.0f, 1.0f);

                // Smooth decay
                if (mag > spectrum_decay[b]) {
                    spectrum_decay[b] = mag;
                } else {
                    spectrum_decay[b] = spectrum_decay[b] * 0.88f + mag * 0.12f;
                }
                spectrum_mags[b] = spectrum_decay[b];
            }
        }

        // Renderizador Embutido dentro do Mixer Inspector Dock
        void RenderEmbedded(float width = 0.0f, float height = 0.0f) {
            updateFFT();

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 canvas_size = ImVec2(width > 0.0f ? width : avail.x, height > 0.0f ? height : 110.0f);

            if (canvas_size.x < 60 || canvas_size.y < 40) {
                return;
            }

            // Fundo Neon
            draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(10, 14, 20, 255), 4.0f);
            draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(0, 212, 255, 100), 4.0f, 0, 1.0f);

            // Partição: 65% Espectro FFT, 35% Vectorscope
            float scope_width = std::min(canvas_size.x * 0.32f, canvas_size.y * 1.1f);
            float fft_width = canvas_size.x - scope_width - 12.0f;
            float bar_w = fft_width / (float)NUM_BANDS;

            // Barras de Espectro
            for (size_t b = 0; b < NUM_BANDS; b++) {
                float val = spectrum_mags[b];
                float bar_h = val * (canvas_size.y - 12.0f);
                
                float x0 = canvas_pos.x + 6.0f + b * bar_w;
                float y0 = canvas_pos.y + canvas_size.y - 4.0f;
                float x1 = x0 + bar_w - 1.0f;
                float y1 = y0 - bar_h;

                ImU32 col;
                if (val < 0.5f) {
                    float t = val * 2.0f;
                    col = IM_COL32((int)(0 + t * 255), (int)(220 - t * 100), 255, 220);
                } else {
                    float t = (val - 0.5f) * 2.0f;
                    col = IM_COL32(255, (int)(120 * (1.0f - t)), (int)(255 * (1.0f - t)), 240);
                }

                draw_list->AddRectFilled(ImVec2(x0, y1), ImVec2(x1, y0), col, 1.0f);
            }

            // Goniômetro / Vectorscope
            ImVec2 scope_center(canvas_pos.x + canvas_size.x - scope_width * 0.5f - 4.0f, canvas_pos.y + canvas_size.y * 0.5f);
            float scope_radius = std::min(scope_width * 0.44f, canvas_size.y * 0.42f);

            draw_list->AddCircle(scope_center, scope_radius, IM_COL32(0, 255, 200, 70), 28, 1.0f);
            draw_list->AddLine(ImVec2(scope_center.x - scope_radius, scope_center.y), ImVec2(scope_center.x + scope_radius, scope_center.y), IM_COL32(0, 255, 200, 45));
            draw_list->AddLine(ImVec2(scope_center.x, scope_center.y - scope_radius), ImVec2(scope_center.x, scope_center.y + scope_radius), IM_COL32(0, 255, 200, 45));

            for (size_t i = 0; i < FFT_SIZE; i += 4) {
                ImVec2 pt = goniometer_pts[i];
                float px = scope_center.x + pt.x * scope_radius * 1.3f;
                float py = scope_center.y - pt.y * scope_radius * 1.3f;
                draw_list->AddCircleFilled(ImVec2(px, py), 1.2f, IM_COL32(0, 255, 200, 190));
            }

            ImGui::Dummy(canvas_size);
        }

        // Render visualizer UI using ImGui ImDrawList (Janela Flutuante Sob Demanda)
        void renderUI() {
            if (!is_visible) return;

            updateFFT();

            ImGui::SetNextWindowSize(ImVec2(540, 280), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("🛸 Master Spectrum & Vectorscope", &is_visible, ImGuiWindowFlags_NoCollapse)) {
                RenderEmbedded(0.0f, 0.0f);
            }
            ImGui::End();
        }
    };
}
