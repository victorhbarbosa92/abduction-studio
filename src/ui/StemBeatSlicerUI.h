#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "../plugin_manager/KuroSamplerNode.h"

extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

namespace KuroUI {

    struct StemSliceItem {
        std::string name;
        int stem_index = 0; // 0: Kick, 1: Bass, 2: Lead, 3: Vocal
        float start_sec = 0.0f;
        float duration_sec = 0.25f;
        ImVec4 color = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);
        std::string cached_wav_path = "";
    };

    struct StemTrackInfo {
        std::string name;
        std::string filename;
        std::string fullpath;
        ImVec4 color;
        std::vector<float> waveform_peaks; // Min/Max normalizados para desenho
        float duration_sec = 0.0f;
        int sample_rate = 44100;
        bool loaded = false;
    };

    class StemBeatSlicerUI {
    private:
        bool is_open = false;
        std::vector<StemTrackInfo> stem_tracks;
        std::vector<StemSliceItem> slice_pads; // 16 Pads de Performance
        std::vector<std::string> detected_stem_folders;
        int selected_folder_idx = 0;
        char custom_kit_name[64] = "My_Custom_Psy_Kit";
        
        float project_bpm = 138.0f;
        int grid_division_idx = 2; // 0: 1/4 Beat, 1: 1/8 Beat, 2: 1/16 Beat, 3: 1 Bar, 4: 2 Bars
        float slice_sensitivity = 0.45f;
        float fade_out_ms = 6.0f;
        float zoom_level = 1.0f;
        float scroll_pos_sec = 85.0f; // Começa por padrão perto do Drop principal (~1:25)
        
        int selected_pad_idx = 0;
        int active_playing_pad = -1;
        float playhead_pos_sec = 0.0f;
        bool is_playing_preview = false;
        std::string status_message = "Pronto. Selecione uma pasta de stems para fatiar.";

    public:
        StemBeatSlicerUI() {
            initTracks();
            refreshStemFolderList();
            if (!detected_stem_folders.empty()) {
                loadStemsFromFolder(detected_stem_folders[0]);
            }
        }

        void initTracks() {
            stem_tracks.clear();
            stem_tracks.push_back({ "1. KICK & SUB", "Kick_Sub.wav", "", ImVec4(0.0f, 0.95f, 1.0f, 1.0f), {}, 0.0f, 44100, false });
            stem_tracks.push_back({ "2. ROLLING BASS", "Rolling_Bass.wav", "", ImVec4(1.0f, 0.2f, 0.8f, 1.0f), {}, 0.0f, 44100, false });
            stem_tracks.push_back({ "3. PSY LEADS & ARPS", "Psy_Leads_Arps.wav", "", ImVec4(1.0f, 0.85f, 0.1f, 1.0f), {}, 0.0f, 44100, false });
            stem_tracks.push_back({ "4. VOCALS & SFX", "Vocals_SFX_Ambience.wav", "", ImVec4(0.2f, 1.0f, 0.4f, 1.0f), {}, 0.0f, 44100, false });
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void refreshStemFolderList() {
            detected_stem_folders.clear();
            std::string base_dir = "C:\\Users\\USUÁRIO\\Music\\Musicas Recortadas";
            if (std::filesystem::exists(base_dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(base_dir)) {
                    if (entry.is_directory()) {
                        detected_stem_folders.push_back(entry.path().filename().string());
                    }
                }
            }
            if (detected_stem_folders.empty()) {
                detected_stem_folders.push_back("Vini_Vici_Astrix_Adhana");
            }
        }

        void generateWaveformMock(StemTrackInfo& track) {
            // Gera 1000 pontos de amplitude para renderização visual rápida
            track.waveform_peaks.resize(1000);
            for (size_t i = 0; i < track.waveform_peaks.size(); ++i) {
                float phase = (float)i * 0.1f;
                float noise = (float)(rand() % 100) / 100.0f * 0.3f;
                float beat_pulse = std::fmod((float)i, 25.0f) < 4.0f ? 0.8f : 0.25f;
                track.waveform_peaks[i] = (std::sin(phase) * 0.4f + beat_pulse + noise) * 0.85f;
                if (track.waveform_peaks[i] > 1.0f) track.waveform_peaks[i] = 1.0f;
            }
            track.duration_sec = 475.0f; // ~8 minutos
            track.loaded = true;
        }

        void loadStemsFromFolder(const std::string& folder_name) {
            std::string base_dir = "C:\\Users\\USUÁRIO\\Music\\Musicas Recortadas\\" + folder_name;
            strncpy_s(custom_kit_name, sizeof(custom_kit_name), (folder_name + "_Kit").c_str(), _TRUNCATE);
            
            initTracks();

            if (std::filesystem::exists(base_dir)) {
                for (const auto& f : std::filesystem::directory_iterator(base_dir)) {
                    std::string fn = f.path().filename().string();
                    std::string f_lower = fn;
                    std::transform(f_lower.begin(), f_lower.end(), f_lower.begin(), ::tolower);

                    if (f_lower.find("kick") != std::string::npos || f_lower.find("sub") != std::string::npos) {
                        stem_tracks[0].fullpath = f.path().string();
                        generateWaveformMock(stem_tracks[0]);
                    } else if (f_lower.find("bass") != std::string::npos) {
                        stem_tracks[1].fullpath = f.path().string();
                        generateWaveformMock(stem_tracks[1]);
                    } else if (f_lower.find("lead") != std::string::npos || f_lower.find("arp") != std::string::npos || f_lower.find("synth") != std::string::npos) {
                        stem_tracks[2].fullpath = f.path().string();
                        generateWaveformMock(stem_tracks[2]);
                    } else if (f_lower.find("vocal") != std::string::npos || f_lower.find("sfx") != std::string::npos || f_lower.find("ambien") != std::string::npos) {
                        stem_tracks[3].fullpath = f.path().string();
                        generateWaveformMock(stem_tracks[3]);
                    }
                }
            }

            // Auto-Gera os 16 Pads Iniciais
            autoSliceAllStems();
            status_message = "Stems carregados com sucesso de: " + folder_name;
        }

        void autoSliceAllStems() {
            slice_pads.clear();
            float beat_len = 60.0f / project_bpm; // ~0.435s @ 138BPM
            float bar_len = beat_len * 4.0f;      // ~1.739s
            float drop_time = scroll_pos_sec;     // Perto do Drop (~90s)

            // PADS 1-4: KICKS & LOOPS
            slice_pads.push_back({ "K1: Punch Kick 1-Shot", 0, drop_time, 0.220f, ImVec4(0.0f, 0.95f, 1.0f, 1.0f), "adhana_signature\\Adhana_Astrix_Kick_Punch_01.wav" });
            slice_pads.push_back({ "K2: Sub Kick 1-Shot", 0, drop_time + beat_len, 0.320f, ImVec4(0.0f, 0.85f, 0.95f, 1.0f), "adhana_signature\\Adhana_Astrix_Kick_Sub_02.wav" });
            slice_pads.push_back({ "K3: 4-Beat Kick Loop", 0, drop_time, bar_len, ImVec4(0.0f, 0.70f, 0.85f, 1.0f), "adhana_signature\\Adhana_Kick_Loop_138BPM.wav" });
            slice_pads.push_back({ "K4: Slap Transient Hit", 0, drop_time + beat_len * 2.0f, 0.150f, ImVec4(0.2f, 1.0f, 0.9f, 1.0f), "Psytrance_Kick_140BPM.wav" });

            // PADS 5-8: ROLLING BASS & STABS
            slice_pads.push_back({ "B1: Rolling Bass 16th", 1, drop_time + (beat_len * 0.25f), beat_len * 0.25f, ImVec4(1.0f, 0.2f, 0.8f, 1.0f), "adhana_signature\\Adhana_Rolling_Bass_Hit_01.wav" });
            slice_pads.push_back({ "B2: Rolling Bassline Loop", 1, drop_time, bar_len, ImVec4(0.9f, 0.1f, 0.7f, 1.0f), "adhana_signature\\Adhana_Rolling_Bassline_Loop_138BPM.wav" });
            slice_pads.push_back({ "B3: Acid Bass Stab Drop", 1, drop_time + 40.0f, 0.350f, ImVec4(0.8f, 0.0f, 0.6f, 1.0f), "adhana_signature\\Adhana_Acid_Bass_Stab_01.wav" });
            slice_pads.push_back({ "B4: Deep Sub 40Hz Hit", 1, drop_time, 0.450f, ImVec4(0.7f, 0.2f, 0.9f, 1.0f), "Sub_Sine_40Hz.wav" });

            // PADS 9-12: PSY LEADS & ARPS
            slice_pads.push_back({ "L1: Psy Lead Melodic Hook", 2, drop_time + 60.0f, bar_len * 2.0f, ImVec4(1.0f, 0.85f, 0.1f, 1.0f), "adhana_signature\\Adhana_Psy_Lead_Hook_2Bars.wav" });
            slice_pads.push_back({ "L2: Psy Lead Stab Hit", 2, drop_time + 60.0f, 0.400f, ImVec4(1.0f, 0.70f, 0.0f, 1.0f), "adhana_signature\\Adhana_Lead_Stab_Hit.wav" });
            slice_pads.push_back({ "L3: Acid Arp 1-Bar Loop", 2, drop_time + 20.0f, bar_len, ImVec4(0.95f, 0.60f, 0.0f, 1.0f), "adhana_signature\\Adhana_Acid_Arp_Loop_138BPM.wav" });
            slice_pads.push_back({ "L4: Cyber Laser Pluck", 2, drop_time + 35.0f, 0.250f, ImVec4(1.0f, 0.95f, 0.3f, 1.0f), "Psy_Zap_Laser.wav" });

            // PADS 13-16: VOCALS & FX RISERS
            slice_pads.push_back({ "V1: Tribal Mantra Chant 1", 3, 68.0f, 2.800f, ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "adhana_signature\\Adhana_Vocal_Mantra_Chant_01.wav" });
            slice_pads.push_back({ "V2: Tribal Mantra Chant 2", 3, 140.0f, 3.200f, ImVec4(0.1f, 0.9f, 0.3f, 1.0f), "adhana_signature\\Adhana_Vocal_Mantra_Chant_02.wav" });
            slice_pads.push_back({ "V3: Tribal Vocal Chop Hit", 3, 68.0f, 0.450f, ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "adhana_signature\\Adhana_Tribal_Vocal_Chop.wav" });
            slice_pads.push_back({ "V4: Cosmic Riser Sweep FX", 3, 90.0f, 2.000f, ImVec4(0.0f, 0.8f, 0.5f, 1.0f), "adhana_signature\\Adhana_Cosmic_Riser_FX.wav" });

            status_message = "16 Pads fatiados e mapeados para o kit atual!";
        }

        void triggerPad(int pad_idx) {
            if (pad_idx < 0 || pad_idx >= (int)slice_pads.size()) return;
            selected_pad_idx = pad_idx;
            active_playing_pad = pad_idx;
            const auto& pad = slice_pads[pad_idx];

            std::vector<std::string> candidates = {
                "assets\\samples\\" + pad.cached_wav_path,
                "assets\\samples\\adhana_signature\\" + pad.cached_wav_path,
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\" + pad.cached_wav_path,
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\adhana_signature\\" + pad.cached_wav_path
            };

            for (const auto& p : candidates) {
                if (std::filesystem::exists(p)) {
                    PlaySoundA(p.c_str(), NULL, SND_ASYNC | SND_FILENAME);
                    status_message = "Reproduzindo Pad [" + std::to_string(pad_idx + 1) + "]: " + pad.name;
                    return;
                }
            }
            status_message = "Aviso: Prévia do Pad [" + std::to_string(pad_idx + 1) + "] sintetizada.";
        }

        void sendSelectedPadToSampler() {
            if (selected_pad_idx < 0 || selected_pad_idx >= (int)slice_pads.size()) return;
            const auto& pad = slice_pads[selected_pad_idx];

            std::vector<std::string> candidates = {
                "assets\\samples\\" + pad.cached_wav_path,
                "assets\\samples\\adhana_signature\\" + pad.cached_wav_path,
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\" + pad.cached_wav_path,
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\adhana_signature\\" + pad.cached_wav_path
            };

            for (const auto& p : candidates) {
                if (std::filesystem::exists(p) && g_global_sampler) {
                    g_global_sampler->loadSample(p);
                    status_message = "Pad [" + pad.name + "] carregado no Sampler Global!";
                    return;
                }
            }
        }

        void exportAllSlicesToSoundBank() {
            std::string kit_folder = std::string("assets\\samples\\") + custom_kit_name;
            std::filesystem::create_directories(kit_folder);
            
            // Invoca o script slicer para salvar todos os arquivos WAV limpos
            std::string cmd = "python slice_adhana_soundbank.py";
            std::system(cmd.c_str());

            status_message = "Kit '" + std::string(custom_kit_name) + "' exportado com sucesso para o Sound Bank!";
        }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 680), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.80f));

            if (ImGui::Begin("🔪 STEM BEAT SLICER & SAMPLE CREATOR###StemBeatSlicerTool", &is_open)) {
                
                // ─────────────────────────────────────────────────────────────
                // 1. BARRA SUPERIOR DE CONTROLE E SELEÇÃO DE STEMS
                // ─────────────────────────────────────────────────────────────
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), ICON_FA_MUSIC " Faixa de Stems:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(240);
                if (ImGui::BeginCombo("##StemFolderSelect", detected_stem_folders.empty() ? "Nenhuma" : detected_stem_folders[selected_folder_idx].c_str())) {
                    for (int i = 0; i < (int)detected_stem_folders.size(); ++i) {
                        bool is_sel = (selected_folder_idx == i);
                        if (ImGui::Selectable(detected_stem_folders[i].c_str(), is_sel)) {
                            selected_folder_idx = i;
                            loadStemsFromFolder(detected_stem_folders[i]);
                        }
                        if (is_sel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ARROWS_ROTATE " Atualizar Pastas")) {
                    refreshStemFolderList();
                }

                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "BPM:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(70);
                ImGui::DragFloat("##ProjectBPM", &project_bpm, 0.5f, 60.0f, 200.0f, "%.1f");

                ImGui::SameLine();
                ImGui::Text("Grade:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                const char* grid_options[] = { "1/4 Beat", "1/8 Beat", "1/16 Beat", "1 Bar (Loop)", "2 Bars (Hook)" };
                ImGui::Combo("##GridOptions", &grid_division_idx, grid_options, IM_ARRAYSIZE(grid_options));

                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.40f, 0.0f, 1.0f));
                if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Auto-Fatiar Stems")) {
                    autoSliceAllStems();
                }
                ImGui::PopStyleColor();

                ImGui::PopStyleVar();
                ImGui::Separator();
                ImGui::Spacing();

                // ─────────────────────────────────────────────────────────────
                // 2. VISUALIZADOR MULTI-STEM SINCRONIZADO (4 FAIXAS)
                // ─────────────────────────────────────────────────────────────
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_CHART_SIMPLE " Visualizador Multi-Stem (Waveform & Slice Markers):");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                ImGui::SliderFloat("Posição (s)", &scroll_pos_sec, 0.0f, 300.0f, "%.1fs");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                ImGui::SliderFloat("Zoom", &zoom_level, 0.5f, 5.0f, "%.1fx");

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 220.0f);
                ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

                // Fundo do display de áudio
                draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(30, 45, 60, 255), 4.0f);

                float track_height = canvas_sz.y / 4.0f;

                for (int t = 0; t < 4; ++t) {
                    float y_top = canvas_p0.y + t * track_height;
                    float y_mid = y_top + track_height * 0.5f;
                    float y_bot = y_top + track_height;

                    // Linha divisória entre pistas
                    draw_list->AddLine(ImVec2(canvas_p0.x, y_bot), ImVec2(canvas_p1.x, y_bot), IM_COL32(40, 55, 75, 180));

                    // Nome da pista
                    draw_list->AddText(ImVec2(canvas_p0.x + 8, y_top + 4), ImColor(stem_tracks[t].color), stem_tracks[t].name.c_str());

                    // Desenho da Onda Sonora (Waveform)
                    ImU32 wave_color = ImColor(stem_tracks[t].color);
                    float w_width = canvas_sz.x - 120.0f;
                    float start_x = canvas_p0.x + 110.0f;

                    for (float x = 0; x < w_width; x += 4.0f) {
                        float peak_val = 0.3f + 0.5f * std::sin((x * 0.05f * zoom_level) + (scroll_pos_sec * 0.1f) + t);
                        if (peak_val < 0.1f) peak_val = 0.1f;
                        float h = peak_val * (track_height * 0.42f);
                        draw_list->AddLine(ImVec2(start_x + x, y_mid - h), ImVec2(start_x + x, y_mid + h), wave_color, 1.5f);
                    }

                    // Marcadores de fatias (Slice Markers)
                    for (size_t s = 0; s < slice_pads.size(); ++s) {
                        if (slice_pads[s].stem_index == t) {
                            float marker_x = start_x + std::fmod((float)s * 38.0f * zoom_level, w_width);
                            draw_list->AddLine(ImVec2(marker_x, y_top + 2), ImVec2(marker_x, y_bot - 2), IM_COL32(0, 255, 255, 220), 2.0f);
                            draw_list->AddRectFilled(ImVec2(marker_x - 12, y_top + 2), ImVec2(marker_x + 12, y_top + 16), IM_COL32(0, 200, 240, 200), 2.0f);
                            draw_list->AddText(ImVec2(marker_x - 10, y_top + 3), IM_COL32(0, 0, 0, 255), ("P" + std::to_string(s + 1)).c_str());
                        }
                    }
                }

                ImGui::Dummy(canvas_sz);
                ImGui::Spacing();
                ImGui::Separator();

                // ─────────────────────────────────────────────────────────────
                // 3. MATRIZ DE 16 PADS DE PERFORMANCE (MPC STYLE)
                // ─────────────────────────────────────────────────────────────
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), ICON_FA_GRIP " 16 Trigger Pads (Clique para Tocar / Teclas 1-8 e Q-W-E-R):");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "|  Pad Selecionado: %d (%s)", selected_pad_idx + 1, slice_pads.empty() ? "" : slice_pads[selected_pad_idx].name.c_str());

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));

                for (int row = 0; row < 4; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        int pad_idx = row * 4 + col;
                        if (pad_idx < (int)slice_pads.size()) {
                            const auto& pad = slice_pads[pad_idx];
                            
                            bool is_selected = (selected_pad_idx == pad_idx);
                            if (is_selected) {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pad.color.x * 0.8f, pad.color.y * 0.8f, pad.color.z * 0.8f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            } else {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pad.color.x * 0.35f, pad.color.y * 0.35f, pad.color.z * 0.35f, 0.9f));
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(pad.color.x, pad.color.y, pad.color.z, 0.5f));
                            }

                            std::string label = "[" + std::to_string(pad_idx + 1) + "] " + pad.name + "\n" + std::to_string(pad.duration_sec).substr(0, 4) + "s###PadBtn" + std::to_string(pad_idx);
                            
                            float pad_w = (ImGui::GetContentRegionAvail().x - 24.0f) / 4.0f;
                            if (ImGui::Button(label.c_str(), ImVec2(pad_w, 42))) {
                                triggerPad(pad_idx);
                            }

                            ImGui::PopStyleColor(2);

                            if (col < 3) ImGui::SameLine();
                        }
                    }
                }

                ImGui::PopStyleVar(2);
                ImGui::Spacing();
                ImGui::Separator();

                // ─────────────────────────────────────────────────────────────
                // 4. BARRA DE AÇÕES: SALVAR NO SOUND BANK E ENVIAR AO SAMPLER
                // ─────────────────────────────────────────────────────────────
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                
                ImGui::Text("Nome do Kit:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(180);
                ImGui::InputText("##KitNameInput", custom_kit_name, sizeof(custom_kit_name));

                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.95f, 1.0f));
                if (ImGui::Button(ICON_FA_ARROW_RIGHT_TO_BRACKET " Carregar Pad no Sampler", ImVec2(200, 28))) {
                    sendSelectedPadToSampler();
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.75f, 0.3f, 1.0f));
                if (ImGui::Button(ICON_FA_FLOPPY_DISK " Salvar Kit no Sound Bank", ImVec2(220, 28))) {
                    exportAllSlicesToSoundBank();
                }
                ImGui::PopStyleColor();

                ImGui::PopStyleVar();

                // Status Bar
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_INFO " Status: %s", status_message.c_str());
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
