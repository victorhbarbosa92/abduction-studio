#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "../ai/StemSeparationEngine.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "dr_wav.h"

namespace KuroUI {

    class PrecisionSampleEditorUI {
    public:
        static void Render(bool* open, StemSeparationEngine& engine) {
            if (!*open) return;

            ImGui::SetNextWindowSize(ImVec2(1050, 680), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.6f));

            if (ImGui::Begin(ICON_FA_WAVE_SQUARE " ABDUCTION EDISON - EDITOR CIRÚRGICO DE SAMPLES", open, ImGuiWindowFlags_MenuBar)) {
                
                // BARRA DE FERRAMENTAS CIRÚRGICAS (EDISON STYLE)
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Ferramentas DSP")) {
                        if (ImGui::MenuItem(ICON_FA_ARROWS_UP_TO_LINE " Normalizar 0dB (Peak Normalize)")) {
                            NormalizeSelectedSlices(engine);
                        }
                        if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT_ARROW_LEFT " Inverter Áudio (Reverse)")) {
                            ReverseSelectedSlices(engine);
                        }
                        if (ImGui::MenuItem(ICON_FA_SCISSORS " Snap to Zero-Crossing")) {
                            SnapZeroCrossing(engine);
                        }
                        if (ImGui::MenuItem(ICON_FA_WATER " Remover Offset DC")) {
                            RemoveDCOffset(engine);
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // HEADER DE CONTROLES RÁPIDOS
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 1.0f));
                if (ImGui::Button(ICON_FA_ARROWS_UP_TO_LINE " Normalizar")) { NormalizeSelectedSlices(engine); }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ARROW_RIGHT_ARROW_LEFT " Reverse")) { ReverseSelectedSlices(engine); }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_SCISSORS " Zero-Crossing")) { SnapZeroCrossing(engine); }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_WATER " DC Offset")) { RemoveDCOffset(engine); }
                ImGui::SameLine(ImGui::GetWindowWidth() - 180);
                if (ImGui::Button(ICON_FA_XMARK " Fechar Editor", ImVec2(160, 26))) { *open = false; }
                ImGui::PopStyleColor();

                ImGui::Separator();

                if (engine.pending_slices.empty()) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_CIRCLE_INFO " Nenhum recorte ou áudio carregado. Utilize o Separador de Stems ou carregue um áudio WAV.");
                    ImGui::End();
                    ImGui::PopStyleColor(2);
                    return;
                }

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_LIST " Fatias / Amostras Identificadas: %d", (int)engine.pending_slices.size());
                ImGui::Spacing();

                ImGui::BeginChild("SlicesMasterList", ImVec2(0, -60), true);
                
                for (size_t i = 0; i < engine.pending_slices.size(); i++) {
                    auto& slice = engine.pending_slices[i];
                    ImGui::PushID((int)i);

                    // Seleção e Ações Rápidas
                    ImGui::Checkbox("##sel", &slice.selected);
                    ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.70f, 0.40f, 1.0f));
                    if (ImGui::Button(ICON_FA_PLAY " PLAY", ImVec2(65, 30))) {
                        extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;
                        if (g_global_sampler) {
                            g_global_sampler->loadFromMemory(slice.data, 2, engine.getSampleRate());
                            g_global_sampler->noteOn(0);
                        }
                    }
                    ImGui::PopStyleColor();
                    ImGui::SameLine();

                    // Nome da Amostra
                    ImGui::SetNextItemWidth(180);
                    char name_buf[128];
                    strncpy(name_buf, slice.suggested_name.c_str(), sizeof(name_buf));
                    if (ImGui::InputText("##name", name_buf, sizeof(name_buf))) {
                        slice.suggested_name = name_buf;
                    }
                    ImGui::SameLine();

                    // Waveform Interativa de Alta Precisão (Estilo Edison)
                    ImVec2 p0 = ImGui::GetCursorScreenPos();
                    ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x - 10.0f, p0.y + 42.0f);
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();

                    // Fundo Obsidian & Grade
                    draw_list->AddRectFilled(p0, p1, IM_COL32(10, 14, 18, 255), 4.0f);
                    draw_list->AddRect(p0, p1, IM_COL32(0, 200, 255, 70), 4.0f);

                    float w = p1.x - p0.x;
                    float h = 21.0f; // Centro
                    draw_list->AddLine(ImVec2(p0.x, p0.y + h), ImVec2(p1.x, p0.y + h), IM_COL32(255, 255, 255, 30));

                    // Desenho da onda espectral neon
                    if (!slice.waveform_preview.empty()) {
                        for (size_t w_idx = 0; w_idx < slice.waveform_preview.size() - 1; w_idx++) {
                            float v0 = slice.waveform_preview[w_idx] * h;
                            float v1 = slice.waveform_preview[w_idx+1] * h;
                            if (v0 > h) v0 = h;
                            if (v1 > h) v1 = h;

                            float x0 = p0.x + ((float)w_idx / (float)slice.waveform_preview.size()) * w;
                            float x1 = p0.x + ((float)(w_idx+1) / (float)slice.waveform_preview.size()) * w;

                            draw_list->AddLine(ImVec2(x0, p0.y + h - v0), ImVec2(x1, p0.y + h - v1), IM_COL32(0, 255, 170, 255), 1.5f);
                            draw_list->AddLine(ImVec2(x0, p0.y + h + v0), ImVec2(x1, p0.y + h + v1), IM_COL32(0, 200, 255, 200), 1.5f);
                        }
                    }

                    ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y + 48.0f));
                    ImGui::PopID();
                    ImGui::Separator();
                }

                ImGui::EndChild();

                // BOTÃO DE EXPORTAÇÃO / SALVAMENTO RÁPIDO
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.60f, 0.90f, 1.0f));
                if (ImGui::Button(ICON_FA_FLOPPY_DISK " SALVAR AMOSTRAS SELECIONADAS NO PROJETO", ImVec2(-1, 38))) {
                    SaveSelectedSlices(engine);
                }
                ImGui::PopStyleColor();
            }

            ImGui::End();
            ImGui::PopStyleColor(2);
        }

    private:
        static void NormalizeSelectedSlices(StemSeparationEngine& engine) {
            for (auto& slice : engine.pending_slices) {
                if (!slice.selected || slice.data.empty()) continue;
                float max_peak = 0.0f;
                for (float s : slice.data) {
                    float a = std::abs(s);
                    if (a > max_peak) max_peak = a;
                }
                if (max_peak > 0.0001f) {
                    float gain = 0.99f / max_peak;
                    for (float& s : slice.data) s *= gain;
                    // Recalcular preview
                    for (float& pv : slice.waveform_preview) pv = std::clamp(pv * gain, 0.0f, 1.0f);
                }
            }
        }

        static void ReverseSelectedSlices(StemSeparationEngine& engine) {
            for (auto& slice : engine.pending_slices) {
                if (!slice.selected || slice.data.empty()) continue;
                // Inverter canais estéreo mantendo pares (L, R)
                size_t num_frames = slice.data.size() / 2;
                for (size_t f = 0; f < num_frames / 2; f++) {
                    size_t opp = num_frames - 1 - f;
                    std::swap(slice.data[f * 2], slice.data[opp * 2]);
                    std::swap(slice.data[f * 2 + 1], slice.data[opp * 2 + 1]);
                }
                std::reverse(slice.waveform_preview.begin(), slice.waveform_preview.end());
            }
        }

        static void SnapZeroCrossing(StemSeparationEngine& engine) {
            for (auto& slice : engine.pending_slices) {
                if (!slice.selected || slice.data.size() < 4) continue;
                // Encontrar o ponto mais próximo de 0.0V no início
                size_t start_offset = 0;
                for (size_t i = 0; i < std::min<size_t>(512, slice.data.size() - 2); i += 2) {
                    if (std::abs(slice.data[i]) < 0.005f) {
                        start_offset = i;
                        break;
                    }
                }
                if (start_offset > 0) {
                    slice.data.erase(slice.data.begin(), slice.data.begin() + start_offset);
                }
            }
        }

        static void RemoveDCOffset(StemSeparationEngine& engine) {
            for (auto& slice : engine.pending_slices) {
                if (!slice.selected || slice.data.empty()) continue;
                double sum_l = 0.0, sum_r = 0.0;
                size_t frames = slice.data.size() / 2;
                for (size_t f = 0; f < frames; f++) {
                    sum_l += slice.data[f * 2];
                    sum_r += slice.data[f * 2 + 1];
                }
                float dc_l = (float)(sum_l / frames);
                float dc_r = (float)(sum_r / frames);
                for (size_t f = 0; f < frames; f++) {
                    slice.data[f * 2] -= dc_l;
                    slice.data[f * 2 + 1] -= dc_r;
                }
            }
        }

        static void SaveSelectedSlices(StemSeparationEngine& engine) {
            std::string base_path;
            const char* userProfile = std::getenv("USERPROFILE");
            if (userProfile) {
                base_path = std::string(userProfile) + "\\Documents\\Abduction_Studio_Samples";
            } else {
                base_path = "C:\\Abduction_Studio_Samples";
            }
            std::filesystem::create_directories(base_path);

            drwav_data_format format;
            format.container = drwav_container_riff;
            format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
            format.channels = 2;
            format.sampleRate = engine.getSampleRate();
            format.bitsPerSample = 32;

            for (auto& slice : engine.pending_slices) {
                if (slice.selected) {
                    std::string track_folder = base_path + "/" + track_names[slice.source_track];
                    std::filesystem::create_directories(track_folder);
                    std::string filepath = track_folder + "/" + slice.suggested_name + ".wav";
                    drwav wav;
                    if (drwav_init_file_write(&wav, filepath.c_str(), &format, NULL)) {
                        drwav_write_pcm_frames(&wav, slice.data.size() / 2, slice.data.data());
                        drwav_uninit(&wav);
                    }
                }
            }
            engine.pending_slices.clear();
        }
    };

    inline void RenderSampleEditor(bool* open, StemSeparationEngine& engine) {
        PrecisionSampleEditorUI::Render(open, engine);
    }
}
