#pragma once
#include "imgui.h"
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <mutex>

namespace KuroUI {

    class SpectrumVisualizerUI {
    public:
        static constexpr size_t FFT_SIZE = 512;
        static constexpr size_t NUM_BANDS = 128;
        
        std::vector<float> audio_ring_l;
        std::vector<float> audio_ring_r;
        size_t ring_write_pos = 0;
        
        std::vector<float> spectrum_mags;
        std::vector<float> spectrum_decay;
        
        // Vectorscope / Goniometer points
        std::vector<ImVec2> goniometer_pts;
        
        std::mutex data_mutex;
        bool is_visible = true;

        SpectrumVisualizerUI() {
            audio_ring_l.assign(FFT_SIZE, 0.0f);
            audio_ring_r.assign(FFT_SIZE, 0.0f);
            spectrum_mags.assign(NUM_BANDS, 0.0f);
            spectrum_decay.assign(NUM_BANDS, 0.0f);
            goniometer_pts.resize(FFT_SIZE);
        }

        // Push real-time audio samples from main audio callback
        void pushAudioSamples(float l, float r) {
            std::lock_guard<std::mutex> lock(data_mutex);
            audio_ring_l[ring_write_pos] = l;
            audio_ring_r[ring_write_pos] = r;
            ring_write_pos = (ring_write_pos + 1) % FFT_SIZE;
        }

        // Compute FFT magnitudes and vectorscope points
        void updateFFT() {
            std::lock_guard<std::mutex> lock(data_mutex);
            
            std::vector<float> windowed(FFT_SIZE);
            for (size_t i = 0; i < FFT_SIZE; i++) {
                size_t idx = (ring_write_pos + i) % FFT_SIZE;
                float mono = 0.5f * (audio_ring_l[idx] + audio_ring_r[idx]);
                float hann = 0.5f * (1.0f - std::cos(2.0f * 3.14159265f * i / (FFT_SIZE - 1)));
                windowed[i] = mono * hann;

                // Vectorscope point (M-S mapping)
                float side = 0.5f * (audio_ring_l[idx] - audio_ring_r[idx]);
                float mid = mono;
                goniometer_pts[i] = ImVec2(side, mid);
            }

            // Simple DFT/FFT spectrum estimation for 128 log-spaced bins
            for (size_t b = 0; b < NUM_BANDS; b++) {
                float freq_ratio = (float)b / (float)NUM_BANDS;
                int k = (int)(1 + freq_ratio * freq_ratio * (FFT_SIZE / 2 - 1));
                k = std::clamp(k, 1, (int)(FFT_SIZE / 2 - 1));

                float re = 0.0f, im = 0.0f;
                for (size_t n = 0; n < FFT_SIZE; n += 2) {
                    float angle = 2.0f * 3.14159265f * k * n / FFT_SIZE;
                    re += windowed[n] * std::cos(angle);
                    im -= windowed[n] * std::sin(angle);
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

        // Render visualizer UI using ImGui ImDrawList
        void renderUI() {
            if (!is_visible) return;

            updateFFT();

            ImGui::SetNextWindowSize(ImVec4(540, 280, 0, 0).x ? ImVec2(540, 280) : ImVec2(540, 280), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("🛸 Master Spectrum & Vectorscope", &is_visible, ImGuiWindowFlags_NoCollapse)) {
                
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                ImVec2 canvas_size = ImGui::GetContentRegionAvail();

                if (canvas_size.x < 100 || canvas_size.y < 80) {
                    ImGui::End();
                    return;
                }

                // Background box
                draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(15, 18, 26, 240), 8.0f);
                draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(0, 230, 255, 100), 8.0f, 0, 1.5f);

                // Partition width: 70% FFT Spectrum, 30% Vectorscope
                float fft_width = canvas_size.x * 0.68f;
                float scope_width = canvas_size.x * 0.28f;
                float bar_w = fft_width / (float)NUM_BANDS;

                // Render Spectrum Bars
                for (size_t b = 0; b < NUM_BANDS; b++) {
                    float val = spectrum_mags[b];
                    float bar_h = val * (canvas_size.y - 30.0f);
                    
                    float x0 = canvas_pos.x + 10.0f + b * bar_w;
                    float y0 = canvas_pos.y + canvas_size.y - 15.0f;
                    float x1 = x0 + bar_w - 1.0f;
                    float y1 = y0 - bar_h;

                    // Neon Gradient color (Cyan -> Magenta -> Yellow)
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

                // Render Vectorscope / Goniometer
                ImVec2 scope_center(canvas_pos.x + canvas_size.x - scope_width * 0.5f - 10.0f, canvas_pos.y + canvas_size.y * 0.5f);
                float scope_radius = std::min(scope_width, canvas_size.y) * 0.42f;

                // Grid circle
                draw_list->AddCircle(scope_center, scope_radius, IM_COL32(0, 255, 200, 60), 32, 1.0f);
                draw_list->AddLine(ImVec2(scope_center.x - scope_radius, scope_center.y), ImVec2(scope_center.x + scope_radius, scope_center.y), IM_COL32(0, 255, 200, 40));
                draw_list->AddLine(ImVec2(scope_center.x, scope_center.y - scope_radius), ImVec2(scope_center.x, scope_center.y + scope_radius), IM_COL32(0, 255, 200, 40));

                for (size_t i = 0; i < FFT_SIZE; i += 2) {
                    ImVec2 pt = goniometer_pts[i];
                    float px = scope_center.x + pt.x * scope_radius * 1.4f;
                    float py = scope_center.y - pt.y * scope_radius * 1.4f;
                    draw_list->AddCircleFilled(ImVec2(px, py), 1.5f, IM_COL32(0, 255, 180, 180));
                }
            }
            ImGui::End();
        }
    };
}
