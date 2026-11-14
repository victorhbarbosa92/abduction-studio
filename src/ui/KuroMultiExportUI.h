#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

#ifndef ICON_FA_DNA
#define ICON_FA_DNA "\xef\x81\xb1"
#endif

namespace KuroUI {

    enum class StemExportFormat {
        WAV_32_FLOAT,
        WAV_24_PCM,
        FLAC_LOSSLESS_24,
        MP3_320_CBR,
        OGG_VORBIS_Q10
    };

    struct StemTrackItem {
        std::string name;
        ImU32 theme_color;
        int thread_id;
        float progress;
        bool peak_norm_enabled;
        float peak_norm_db;
        int format_idx; // 0: WAV 32f, 1: WAV 24, 2: FLAC, 3: MP3 320
        bool enabled;
        std::string status_text;
    };

    class KuroMultiExportUI {
    private:
        bool is_open = false;
        bool is_rendering = false;
        float overall_progress = 0.0f;
        int worker_threads = 8;
        float render_speed_multiplier = 32.4f;
        int sample_rate_idx = 1; // 0: 44.1kHz, 1: 48kHz, 2: 96kHz

        // Stems track items
        std::vector<StemTrackItem> stems;

    public:
        KuroMultiExportUI() {
            initDefaults();
        }

        void initDefaults() {
            stems.clear();
            stems.push_back({ "Kick",       IM_COL32(0, 229, 255, 255),   1, 0.72f, true,  -0.3f, 0, true, "Multi-threaded parallel rendering" });
            stems.push_back({ "Bass",       IM_COL32(255, 215, 64, 255),  2, 0.65f, true,  -0.3f, 0, true, "Multi-threaded parallel rendering" });
            stems.push_back({ "Lead",       IM_COL32(186, 104, 200, 255), 3, 0.58f, true,  -0.3f, 0, true, "Multi-threaded parallel rendering" });
            stems.push_back({ "Percussion", IM_COL32(0, 240, 255, 255),   4, 0.60f, true,  -0.3f, 0, true, "Multi-threaded parallel rendering" });
            stems.push_back({ "FX",         IM_COL32(0, 229, 255, 255),   1, 0.70f, true,  -0.3f, 0, true, "Multi-threaded parallel rendering" });
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

    private:
        // Custom iOS/FL Style Toggle Switch matching the mockup
        bool DrawToggleSwitch(const char* id, bool* v, ImU32 on_col, float w = 38.0f, float h = 20.0f) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImGui::InvisibleButton(id, ImVec2(w, h));
            if (ImGui::IsItemClicked()) *v = !*v;
            bool is_hovered = ImGui::IsItemHovered();

            float radius = h * 0.5f;
            ImU32 bg_col = *v ? on_col : IM_COL32(36, 46, 60, 255);
            dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg_col, radius);
            if (is_hovered) {
                dl->AddRect(p, ImVec2(p.x + w, p.y + h), IM_COL32(255, 255, 255, 120), radius, 0, 1.2f);
            }

            float circle_x = *v ? (p.x + w - radius) : (p.x + radius);
            dl->AddCircleFilled(ImVec2(circle_x, p.y + radius), radius - 2.5f, IM_COL32(255, 255, 255, 255));
            return *v;
        }

    public:
        void Render() {
            if (!is_open) return;

            float cur_time = (float)ImGui::GetTime();

            // Animate progress simulation if active
            if (is_rendering) {
                overall_progress += ImGui::GetIO().DeltaTime * 0.15f;
                if (overall_progress > 1.0f) {
                    overall_progress = 1.0f;
                    is_rendering = false;
                }
                for (auto& s : stems) {
                    s.progress = std::min(1.0f, s.progress + ImGui::GetIO().DeltaTime * (0.08f + (float)(s.thread_id % 3) * 0.05f));
                }
            }

            ImGui::SetNextWindowPos(ImVec2(645, 68), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(625, 625), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.040f, 0.050f, 0.070f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.60f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

            if (ImGui::Begin("Stems Export and Multitrack Bounce###StemsExportWindow", &is_open, ImGuiWindowFlags_NoScrollbar)) {
                
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 avail = ImGui::GetContentRegionAvail();

                // ── CABEÇALHO COM TÍTULO E FECHAR ────────────────────────────
                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + 2.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.98f, 1.0f, 1.0f));
                ImGui::SetWindowFontScale(1.15f);
                ImGui::Text("Stems Export and Multitrack Bounce");
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();

                ImGui::SameLine(avail.x - 70.0f);
                if (ImGui::Button("✖ Fechar", ImVec2(65.0f, 22.0f))) {
                    is_open = false;
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── CABEÇALHOS DAS COLUNAS (Channel | Multi-threaded parallel rendering | Peaks | Format) ──
                ImVec2 col_header_pos = ImGui::GetCursorScreenPos();
                float x_channel = col_header_pos.x + 24.0f;
                float x_progress = col_header_pos.x + 105.0f;
                float x_peaks = col_header_pos.x + avail.x - 170.0f;
                float x_format = col_header_pos.x + avail.x - 105.0f;

                dl->AddText(ImVec2(x_channel, col_header_pos.y), IM_COL32(140, 160, 185, 220), "Channel");
                dl->AddText(ImVec2(x_progress, col_header_pos.y), IM_COL32(140, 160, 185, 220), "Multi-threaded parallel rendering");
                dl->AddText(ImVec2(x_peaks, col_header_pos.y), IM_COL32(140, 160, 185, 220), "Peaks");
                dl->AddText(ImVec2(x_format, col_header_pos.y), IM_COL32(140, 160, 185, 220), "Format");

                ImGui::Dummy(ImVec2(avail.x, 24.0f));

                // ── LISTA DAS FAIXAS DE STEMS COM RENDERIZAÇÃO PARALELA ──────
                float list_h = 390.0f;
                ImGui::BeginChild("StemsListContainer", ImVec2(avail.x, list_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    ImDrawList* dl_list = ImGui::GetWindowDrawList();
                    float row_h = 68.0f;
                    float row_spacing = 8.0f;

                    const char* format_names[] = {
                        "WAV\n32-bit float",
                        "WAV\n24-bit PCM",
                        "FLAC\n24-bit Lossless",
                        "MP3\n320 kbps"
                    };

                    for (size_t i = 0; i < stems.size(); ++i) {
                        auto& stem = stems[i];
                        ImGui::PushID((int)i + 7000);

                        ImVec2 row_pos = ImGui::GetCursorScreenPos();
                        float row_w = ImGui::GetContentRegionAvail().x;
                        ImVec2 row_p1 = ImVec2(row_pos.x + row_w, row_pos.y + row_h);

                        // Fundo escuro elegante com bordas arredondadas
                        dl_list->AddRectFilled(row_pos, row_p1, IM_COL32(12, 16, 24, 255), 6.0f);
                        dl_list->AddRect(row_pos, row_p1, IM_COL32(24, 34, 48, 255), 6.0f);

                        // Faixa de cor / Glow lateral esquerda específica do canal
                        dl_list->AddRectFilled(row_pos, ImVec2(row_pos.x + 4.0f, row_p1.y), stem.theme_color, 4.0f);

                        // Grip Dots ':::' para reordenação
                        float grip_x = row_pos.x + 12.0f;
                        float grip_y = row_pos.y + row_h * 0.35f;
                        for (int dy = 0; dy < 3; ++dy) {
                            dl_list->AddCircleFilled(ImVec2(grip_x, grip_y + dy * 6.0f), 1.5f, IM_COL32(100, 120, 145, 180));
                            dl_list->AddCircleFilled(ImVec2(grip_x + 5.0f, grip_y + dy * 6.0f), 1.5f, IM_COL32(100, 120, 145, 180));
                        }

                        // 1. Nome do Canal (Channel Name)
                        dl_list->AddText(ImVec2(row_pos.x + 22.0f, row_pos.y + 20.0f), IM_COL32(245, 250, 255, 255), stem.name.c_str());

                        // 2. Barra de Progresso com Thread Worker
                        float bar_x = row_pos.x + 95.0f;
                        float bar_w = (x_peaks - 15.0f) - bar_x;
                        float bar_y = row_pos.y + 18.0f;
                        float bar_h = 10.0f;

                        // Rótulo da Thread / Status acima da barra
                        dl_list->AddText(ImVec2(bar_x, row_pos.y + 5.0f), IM_COL32(160, 180, 205, 200), "Multi-threaded parallel rendering");

                        // Fundo da barra
                        dl_list->AddRectFilled(ImVec2(bar_x, bar_y + 4.0f), ImVec2(bar_x + bar_w, bar_y + 4.0f + bar_h), IM_COL32(20, 28, 40, 255), 5.0f);
                        
                        // Barra preenchida com a cor neon do instrumento
                        float fill_w = bar_w * stem.progress;
                        dl_list->AddRectFilled(ImVec2(bar_x, bar_y + 4.0f), ImVec2(bar_x + fill_w, bar_y + 4.0f + bar_h), stem.theme_color, 5.0f);
                        
                        // Efeito de brilho pulsar na ponta da barra
                        dl_list->AddCircleFilled(ImVec2(bar_x + fill_w, bar_y + 4.0f + bar_h * 0.5f), 4.5f, IM_COL32(255, 255, 255, 230));

                        // Subtítulo "Peak Normalization" logo abaixo da barra
                        dl_list->AddText(ImVec2(bar_x, bar_y + bar_h + 7.0f), IM_COL32(130, 150, 175, 200), "Peak Normalization");

                        // 3. Switch de Normalização de Pico (Peak Normalization Toggle)
                        float sw_x = x_peaks + 6.0f;
                        float sw_y = row_pos.y + 20.0f;
                        ImGui::SetCursorScreenPos(ImVec2(sw_x, sw_y));
                        char sw_id[32];
                        snprintf(sw_id, sizeof(sw_id), "##PeakSw_%d", (int)i);
                        DrawToggleSwitch(sw_id, &stem.peak_norm_enabled, stem.theme_color, 36.0f, 18.0f);

                        // Rótulo 'Peak' acima do switch
                        dl_list->AddText(ImVec2(sw_x - 2.0f, row_pos.y + 5.0f), IM_COL32(140, 160, 185, 200), "Peak");

                        // 4. Dropdown de Formato de Áudio
                        float fmt_x = x_format - 4.0f;
                        float fmt_y = row_pos.y + 12.0f;
                        ImGui::SetCursorScreenPos(ImVec2(fmt_x, fmt_y));
                        ImGui::SetNextItemWidth(90.0f);
                        const char* fmt_labels[] = { "WAV (32f)", "WAV (24b)", "FLAC (24b)", "MP3 (320)" };
                        ImGui::Combo("##FmtCombo", &stem.format_idx, fmt_labels, IM_ARRAYSIZE(fmt_labels));

                        ImGui::SetCursorScreenPos(row_pos);
                        ImGui::Dummy(ImVec2(row_w, row_h + row_spacing));
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // ── RODAPÉ COM STATUS E BOTÃO DE EXPORTAÇÃO FUTURISTA CYBERPUNK ──
                // Informações do Engine Paralelo
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_MICROCHIP " Parallel Offline Engine: 8 CPU Worker Threads Active | Speed: 32.4x Real-time");
                ImGui::TextDisabled("Destino: ./Rendered_Stems/ | Normalização True Peak: -0.3 dBFS Ceiling | 48.0 kHz 32-bit Float");

                ImGui::Spacing();

                // Botão "Export" Grande e Brilhante em Formato de Pílula (Pill Shape)
                float btn_w = 260.0f;
                float btn_h = 44.0f;
                float btn_x = (avail.x - btn_w) * 0.5f;

                ImGui::SetCursorPosX(btn_x);
                ImVec2 b_min = ImGui::GetCursorScreenPos();
                ImVec2 b_max = ImVec2(b_min.x + btn_w, b_min.y + btn_h);

                ImGui::InvisibleButton("##ExportStemsButton", ImVec2(btn_w, btn_h));
                bool btn_hovered = ImGui::IsItemHovered();
                bool btn_active = ImGui::IsItemActive();

                if (ImGui::IsItemClicked()) {
                    is_rendering = true;
                    overall_progress = 0.05f;
                    for (auto& s : stems) {
                        s.progress = 0.02f;
                    }
                }

                // Efeito visual neon do botão de exportação
                ImU32 b_bg = btn_hovered ? IM_COL32(0, 120, 160, 160) : IM_COL32(0, 60, 85, 120);
                ImU32 b_border = btn_hovered ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 229, 255, 240);
                float radius = btn_h * 0.5f;

                // Preenchimento e Borda Brilhante
                dl->AddRectFilled(b_min, b_max, b_bg, radius);
                dl->AddRect(b_min, b_max, b_border, radius, 0, 2.2f);

                // Brilho externo sutil (Outer Glow)
                dl->AddRect(ImVec2(b_min.x - 2.0f, b_min.y - 2.0f), ImVec2(b_max.x + 2.0f, b_max.y + 2.0f), IM_COL32(0, 229, 255, 60), radius + 2.0f, 0, 2.0f);

                // Texto "Export" Centralizado
                const char* export_txt = is_rendering ? "Rendering Stems..." : "Export";
                ImVec2 txt_sz = ImGui::CalcTextSize(export_txt);
                ImVec2 txt_pos = ImVec2(b_min.x + (btn_w - txt_sz.x) * 0.5f, b_min.y + (btn_h - txt_sz.y) * 0.5f);
                dl->AddText(txt_pos, IM_COL32(230, 250, 255, 255), export_txt);
            }
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
