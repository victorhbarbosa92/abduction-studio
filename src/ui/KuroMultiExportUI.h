#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    enum class ExportAudioFormat {
        WAV_32_FLOAT,
        WAV_24_PCM,
        WAV_16_PCM,
        MP3_320_CBR,
        FLAC_LOSSLESS_24,
        OGG_VORBIS_Q10
    };

    class KuroMultiExportUI {
    private:
        bool is_open = false;
        int selected_format_idx = 0; // 0: WAV 32f, 1: WAV 24b, 2: WAV 16b, 3: MP3 320, 4: FLAC 24b, 5: OGG
        int selected_samplerate_idx = 0; // 0: 44.1kHz, 1: 48kHz, 2: 88.2kHz, 3: 96kHz
        bool enable_dithering = true;
        bool enable_normalize = false;
        float normalize_peak_db = -0.3f;
        bool is_exporting = false;
        float export_progress = 0.0f;

        // Metadados da Faixa
        char meta_title[128] = "Abduction Cyber Psytrance Track";
        char meta_artist[128] = "Produtor Abduction";
        char meta_genre[64] = "Psytrance / Full-On";
        char meta_bpm[16] = "148";

    public:
        KuroMultiExportUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(800, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.50f, 0.45f));

            if (ImGui::Begin(ICON_FA_DOWNLOAD " EXPORTAÇÃO DE ÁUDIO MULTI-FORMATO & PAINEL DE RENDERIZAÇÃO", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE FORMATO & QUALIDADE
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_SLIDERS " Formato do Arquivo de Áudio:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(220);
                const char* formats[] = {
                    "WAV (32-bit Float High-Res)",
                    "WAV (24-bit Studio PCM)",
                    "WAV (16-bit CD Quality)",
                    "MP3 (320 kbps CBR High Bitrate)",
                    "FLAC (24-bit Lossless)",
                    "OGG Vorbis (Quality 10)"
                };
                ImGui::Combo("##FormatCombo", &selected_format_idx, formats, IM_ARRAYSIZE(formats));

                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                const char* sample_rates[] = { "44.1 kHz", "48.0 kHz", "88.2 kHz", "96.0 kHz" };
                ImGui::Combo("##SampleRateCombo", &selected_samplerate_idx, sample_rates, IM_ARRAYSIZE(sample_rates));

                ImGui::Separator();
                ImGui::Spacing();

                // DITHERING & REGRAMENTO DE PICO
                ImGui::BeginChild("##AudioConfigSection", ImVec2(0, 120), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_GEARS " CONFIGURAÇÕES DE PROCESSAMENTO DE RENDERIZAÇÃO:");
                    ImGui::Spacing();

                    ImGui::Checkbox("Ativar Dithering Triangular (Noise Shaping 16-bit)", &enable_dithering);
                    ImGui::SameLine(400);
                    ImGui::Checkbox("Normalizar Pico de Áudio", &enable_normalize);

                    if (enable_normalize) {
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(120);
                        ImGui::SliderFloat("Pico Máx", &normalize_peak_db, -3.0f, 0.0f, "%.1f dB");
                    }

                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Modo de Renderização: Offline Bounce Ultra-Fast (Renderização direta em arquivo)");
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // METADADOS ID3 / ARQUIVO
                ImGui::BeginChild("##MetadataSection", ImVec2(0, 200), true);
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), ICON_FA_TAGS " METADADOS DO ARQUIVO (TAGS ID3):");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Columns(2, "MetaCols", true);

                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Título da Faixa:");
                    ImGui::InputText("##MetaTitle", meta_title, sizeof(meta_title));

                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Artista / Projeto:");
                    ImGui::InputText("##MetaArtist", meta_artist, sizeof(meta_artist));
                    ImGui::NextColumn();

                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Gênero Musical:");
                    ImGui::InputText("##MetaGenre", meta_genre, sizeof(meta_genre));

                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Andamento (BPM):");
                    ImGui::InputText("##MetaBpm", meta_bpm, sizeof(meta_bpm));
                    ImGui::NextColumn();
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // BARRA DE PROGRESSO & BOTÃO DE EXPORTAÇÃO
                if (is_exporting) {
                    export_progress += 0.02f;
                    if (export_progress >= 1.0f) {
                        export_progress = 1.0f;
                        is_exporting = false;
                    }
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
                    ImGui::ProgressBar(export_progress, ImVec2(-1, 32), "Renderizando arquivo de áudio...");
                    ImGui::PopStyleColor();
                } else {
                    if (ImGui::Button(ICON_FA_DOWNLOAD " INICIAR RENDERIZAÇÃO & EXPORTAR", ImVec2(-1, 36))) {
                        is_exporting = true;
                        export_progress = 0.1f;
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
