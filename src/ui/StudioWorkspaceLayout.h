#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "../core/ClipManager.h"
#include "../ai/StemSeparationEngine.h"
#include "../core/TimelineManager.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

extern ClipManager g_clip_manager;
extern float track_pans[MAX_TRACKS];
extern float track_volumes[MAX_TRACKS];
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
extern float track_vu_levels[8];
extern float master_vu_level_l;
extern float master_vu_level_r;
extern float g_master_volume;
extern int selected_track_idx;
extern std::string track_names[MAX_TRACKS];

namespace KuroUI {

    class StudioWorkspaceLayout {
    public:
        static void RenderLayout(ImVec2 passed_pos, ImVec2 passed_size, StemSeparationEngine& ai_engine, KuroDSP::TimelineManager& timeline) {
            const ImGuiViewport* vp = ImGui::GetMainViewport();
            ImVec2 work_pos = vp->WorkPos;
            ImVec2 work_size = vp->WorkSize;

            float top_bar_h = 36.0f;
            work_pos.y += top_bar_h;
            work_size.y -= top_bar_h;

            if (work_size.x < 600.0f) work_size.x = 1280.0f;
            if (work_size.y < 400.0f) work_size.y = 720.0f - top_bar_h;

            float left_w = 220.0f;
            float right_w = 310.0f;
            float center_w = std::max(200.0f, work_size.x - left_w - right_w);

            float top_h = std::max(180.0f, work_size.y * 0.58f);
            float bot_h = std::max(140.0f, work_size.y - top_h);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.06f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.30f));

            ImGuiWindowFlags fixed_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

            // =========================================================================
            // 1. LEFT SIDEBAR: BROWSER & FILE TREE (220px)
            // =========================================================================
            ImGui::SetNextWindowPos(work_pos);
            ImGui::SetNextWindowSize(ImVec2(left_w, work_size.y));
            if (ImGui::Begin("##LeftBrowserWindow", nullptr, fixed_flags)) {
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_FOLDER_TREE " BROWSER & ARQUIVOS");
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::BeginTabBar("##LeftBrowserTabs")) {
                    if (ImGui::BeginTabItem("Files")) {
                        static char search_b[64] = "";
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("##srch", search_b, sizeof(search_b));
                        ImGui::Spacing();

                        if (ImGui::TreeNodeEx("[Projeto Atual]", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Selectable("  Drums (Master)");
                            ImGui::Selectable("  Bassline (16th KBB)");
                            ImGui::Selectable("  Synth Lead (Cyan)");
                            ImGui::Selectable("  Vocals (Psy Hook)");
                            ImGui::TreePop();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Plugins")) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Sintetizadores & FX:");
                        ImGui::Selectable("• KuroWave (Wavetable)");
                        ImGui::Selectable("• Serum (VST3)");
                        ImGui::Selectable("• Vital (CLAP)");
                        ImGui::Selectable("• Abduction FM 4-Op");
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Samples")) {
                        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Amostras & Loops:");
                        ImGui::Selectable("🥁 Kick_909_Psy.wav");
                        ImGui::Selectable("🥁 Snare_Heavy.wav");
                        ImGui::Selectable("🎵 HiHat_Open_16th.wav");
                        ImGui::Selectable("🌀 Acid_Zap_148BPM.wav");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            ImGui::End();

            // =========================================================================
            // 2. PAINEL CENTRAL SUPERIOR: TIMELINE ARRANGER & SEQUENCER (100% FIEL AO MOCKUP)
            // =========================================================================
            ImVec2 center_pos(work_pos.x + left_w, work_pos.y);
            ImGui::SetNextWindowPos(center_pos);
            ImGui::SetNextWindowSize(ImVec2(center_w, top_h));
            if (ImGui::Begin("##CentralTimelineArranger", nullptr, fixed_flags)) {
                ImDrawList* draw = ImGui::GetWindowDrawList();

                // Cabeçalho da janela do Arranger
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_CHART_LINE " ARRANJADOR & TIMELINE SEQUENCER");
                ImGui::SameLine(ImGui::GetWindowWidth() - 180);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "BPM: %.0f  |  Bar:Beat", timeline.getBPM());
                ImGui::Separator();

                // Canvas do Arranger
                ImVec2 arr_p0 = ImGui::GetCursorScreenPos();
                ImVec2 arr_sz = ImGui::GetContentRegionAvail();

                // Fundo Dark Slate
                draw->AddRectFilled(arr_p0, ImVec2(arr_p0.x + arr_sz.x, arr_p0.y + arr_sz.y), IM_COL32(14, 18, 24, 255));

                float track_card_w = 140.0f;
                float ruler_h = 24.0f;
                float p_sec = 22.0f; // Pixels por segundo / barra

                // ── RÉGUA TEMPORAL (1, 5, 9, 13, 17, 21, 25, 29, 33) ──
                draw->AddRectFilled(ImVec2(arr_p0.x + track_card_w, arr_p0.y), ImVec2(arr_p0.x + arr_sz.x, arr_p0.y + ruler_h), IM_COL32(20, 26, 36, 255));
                draw->AddLine(ImVec2(arr_p0.x + track_card_w, arr_p0.y + ruler_h), ImVec2(arr_p0.x + arr_sz.x, arr_p0.y + ruler_h), IM_COL32(0, 229, 255, 80));

                for (int bar = 0; bar <= 36; bar += 4) {
                    float bx = arr_p0.x + track_card_w + bar * p_sec;
                    if (bx > arr_p0.x + arr_sz.x) break;

                    draw->AddLine(ImVec2(bx, arr_p0.y), ImVec2(bx, arr_p0.y + ruler_h), IM_COL32(200, 200, 200, 200), 1.5f);
                    draw->AddLine(ImVec2(bx, arr_p0.y + ruler_h), ImVec2(bx, arr_p0.y + arr_sz.y), IM_COL32(255, 255, 255, 15), 1.0f);

                    char b_num[8];
                    snprintf(b_num, sizeof(b_num), "%d", bar == 0 ? 1 : bar);
                    draw->AddText(ImVec2(bx + 4.0f, arr_p0.y + 4.0f), IM_COL32(200, 200, 200, 220), b_num);
                }

                // ── FAIXAS DA TIMELINE E CARTÕES (FAIXAS 0 A 4) ──
                struct TrackDef {
                    std::string name;
                    ImU32 card_color;
                    ImU32 clip_color;
                    bool is_automation;
                };

                std::vector<TrackDef> tracks_def = {
                    { "Drums", IM_COL32(176, 0, 255, 255), IM_COL32(176, 0, 255, 180), false },
                    { "Bassline", IM_COL32(0, 229, 255, 255), IM_COL32(0, 229, 255, 180), false },
                    { "Synth Lead", IM_COL32(0, 200, 255, 255), IM_COL32(0, 200, 255, 180), false },
                    { "Vocals", IM_COL32(160, 32, 240, 255), IM_COL32(160, 32, 240, 180), false },
                    { "Automation (Cutoff)", IM_COL32(20, 30, 45, 255), IM_COL32(0, 229, 255, 255), true },
                    { "Automation (Pitch)", IM_COL32(20, 30, 45, 255), IM_COL32(176, 0, 255, 255), true }
                };

                float track_y = arr_p0.y + ruler_h + 2.0f;
                float trk_h = (arr_sz.y - ruler_h - 10.0f) / (float)tracks_def.size();

                for (size_t i = 0; i < tracks_def.size(); ++i) {
                    auto& trk = tracks_def[i];
                    float ty0 = track_y + i * trk_h;
                    float ty1 = ty0 + trk_h - 2.0f;

                    // 1. Cartão do Cabeçalho da Faixa (Esquerda)
                    draw->AddRectFilled(ImVec2(arr_p0.x, ty0), ImVec2(arr_p0.x + track_card_w, ty1), IM_COL32(22, 28, 38, 255), 4.0f);
                    draw->AddRectFilled(ImVec2(arr_p0.x, ty0), ImVec2(arr_p0.x + 6.0f, ty1), trk.card_color, 4.0f);
                    draw->AddText(ImVec2(arr_p0.x + 12.0f, ty0 + 6.0f), IM_COL32(255, 255, 255, 255), trk.name.c_str());

                    // 2. Área do Grid da Faixa (Direita)
                    draw->AddRect(ImVec2(arr_p0.x + track_card_w, ty0), ImVec2(arr_p0.x + arr_sz.x, ty1), IM_COL32(35, 45, 60, 255));

                    if (!trk.is_automation) {
                        // Desenhar Bloco de Áudio / MIDI na Timeline
                        float cx0 = arr_p0.x + track_card_w + (i == 3 ? 18.0f * p_sec : 0.0f);
                        float cx1 = cx0 + (i == 3 ? 14.0f * p_sec : 32.0f * p_sec);
                        if (cx1 > arr_p0.x + arr_sz.x) cx1 = arr_p0.x + arr_sz.x;

                        draw->AddRectFilled(ImVec2(cx0, ty0 + 2.0f), ImVec2(cx1, ty1 - 2.0f), trk.clip_color, 4.0f);
                        draw->AddRect(ImVec2(cx0, ty0 + 2.0f), ImVec2(cx1, ty1 - 2.0f), IM_COL32(255, 255, 255, 220), 4.0f);
                        draw->AddText(ImVec2(cx0 + 8.0f, ty0 + 4.0f), IM_COL32(255, 255, 255, 255), trk.name.c_str());

                        // Desenhar Onda Sintética Cyan/Purple
                        float mid_trk_y = (ty0 + ty1) * 0.5f;
                        for (float wx = cx0 + 4.0f; wx < cx1 - 4.0f; wx += 3.0f) {
                            float amp = std::sin((wx - cx0) * 0.15f) * (trk_h * 0.35f);
                            draw->AddLine(ImVec2(wx, mid_trk_y - amp), ImVec2(wx, mid_trk_y + amp), IM_COL32(255, 255, 255, 180));
                        }
                    } else {
                        // Desenhar Curva de Automação Cúbica Bezier (Idêntico ao Mockup)
                        float ax0 = arr_p0.x + track_card_w;
                        float aw = 32.0f * p_sec;
                        ImVec2 p0(ax0, ty1 - 4.0f);
                        ImVec2 p1(ax0 + aw * 0.35f, ty0 + 4.0f);
                        ImVec2 p2(ax0 + aw * 0.65f, ty0 + 8.0f);
                        ImVec2 p3(ax0 + aw, (i == 4) ? ty0 + 15.0f : ty1 - 4.0f);

                        draw->AddBezierCubic(p0, p1, p2, p3, trk.clip_color, 2.5f);
                        draw->AddCircleFilled(p1, 4.0f, trk.clip_color);
                        draw->AddCircleFilled(p2, 4.0f, trk.clip_color);
                    }
                }

                // Reprodução Playhead Line
                float playhead_x = arr_p0.x + track_card_w + ((float)timeline.getMasterFrame() / 44100.0f) * p_sec;
                if (playhead_x >= arr_p0.x + track_card_w && playhead_x <= arr_p0.x + arr_sz.x) {
                    draw->AddLine(ImVec2(playhead_x, arr_p0.y), ImVec2(playhead_x, arr_p0.y + arr_sz.y), IM_COL32(255, 255, 255, 255), 2.0f);
                    draw->AddTriangleFilled(ImVec2(playhead_x - 6.0f, arr_p0.y), ImVec2(playhead_x + 6.0f, arr_p0.y), ImVec2(playhead_x, arr_p0.y + 10.0f), IM_COL32(255, 255, 255, 255));
                }
            }
            ImGui::End();

            // =========================================================================
            // 3. PAINEL CENTRAL INFERIOR: MPC DRUM PADS & PIANO ROLL (100% FIEL AO MOCKUP)
            // =========================================================================
            ImVec2 bot_pos(work_pos.x + left_w, work_pos.y + top_h);
            ImGui::SetNextWindowPos(bot_pos);
            ImGui::SetNextWindowSize(ImVec2(center_w, bot_h));
            if (ImGui::Begin("##CentralBottomRack", nullptr, fixed_flags)) {
                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 bot_p0 = ImGui::GetCursorScreenPos();
                ImVec2 bot_sz = ImGui::GetContentRegionAvail();

                float pad_panel_w = std::min(260.0f, bot_sz.x * 0.32f);
                float pianoroll_w = bot_sz.x - pad_panel_w - 6.0f;

                // --- ESQUERDA: MPC DRUM STEP PADS (4x4) ---
                draw->AddRectFilled(bot_p0, ImVec2(bot_p0.x + pad_panel_w, bot_p0.y + bot_sz.y), IM_COL32(12, 16, 22, 255), 4.0f);
                draw->AddRect(bot_p0, ImVec2(bot_p0.x + pad_panel_w, bot_p0.y + bot_sz.y), IM_COL32(0, 229, 255, 80), 4.0f);
                draw->AddText(ImVec2(bot_p0.x + 8.0f, bot_p0.y + 6.0f), IM_COL32(0, 229, 255, 255), ICON_FA_GEARS " STEP PADS");

                float pad_start_y = bot_p0.y + 26.0f;
                float pad_size = (pad_panel_w - 20.0f) / 4.0f;

                const char* pad_labels[4][4] = {
                    { "Kick", "Snare", "Hi-Hat", "Tom" },
                    { "Kick", "Snare", "Hi-Hat", "Tom" },
                    { "Kick", "Snare", "Hi-Hat", "Tom" },
                    { "Kick", "Snare", "Hi-Hat", "Tom" }
                };

                for (int r = 0; r < 4; ++r) {
                    for (int c = 0; c < 4; ++c) {
                        float px = bot_p0.x + 6.0f + c * (pad_size + 3.0f);
                        float py = pad_start_y + r * (pad_size + 3.0f);
                        if (py + pad_size > bot_p0.y + bot_sz.y) continue;

                        bool is_active = (r == 0 && c == 0) || (r == 1 && c == 2) || (r == 3 && c == 1);
                        ImU32 pad_bg = is_active ? IM_COL32(0, 229, 255, 240) : ((r >= 2) ? IM_COL32(120, 20, 180, 180) : IM_COL32(25, 35, 48, 255));

                        draw->AddRectFilled(ImVec2(px, py), ImVec2(px + pad_size, py + pad_size), pad_bg, 4.0f);
                        draw->AddRect(ImVec2(px, py), ImVec2(px + pad_size, py + pad_size), IM_COL32(255, 255, 255, 180), 4.0f);

                        ImU32 text_col = is_active ? IM_COL32(0, 0, 0, 255) : IM_COL32(220, 220, 220, 255);
                        draw->AddText(ImVec2(px + 4.0f, py + 4.0f), text_col, pad_labels[r][c]);
                    }
                }

                // --- DIREITA: PIANO ROLL INTEGRADO ---
                ImVec2 pr_p0(bot_p0.x + pad_panel_w + 6.0f, bot_p0.y);
                draw->AddRectFilled(pr_p0, ImVec2(pr_p0.x + pianoroll_w, pr_p0.y + bot_sz.y), IM_COL32(12, 16, 24, 255), 4.0f);
                draw->AddRect(pr_p0, ImVec2(pr_p0.x + pianoroll_w, pr_p0.y + bot_sz.y), IM_COL32(0, 229, 255, 80), 4.0f);
                draw->AddText(ImVec2(pr_p0.x + 8.0f, pr_p0.y + 6.0f), IM_COL32(0, 229, 255, 255), ICON_FA_MUSIC " INTEGRATED PIANO ROLL");

                float keybed_w = 40.0f;
                float grid_x0 = pr_p0.x + keybed_w;
                float grid_y0 = pr_p0.y + 26.0f;
                float grid_h = bot_sz.y - 45.0f;

                // Desenhar Teclas do Piano (Brancas & Pretas)
                int num_keys = 12;
                float key_h = grid_h / num_keys;
                for (int k = 0; k < num_keys; ++k) {
                    float ky = grid_y0 + k * key_h;
                    bool is_black = (k == 1 || k == 3 || k == 6 || k == 8 || k == 10);
                    ImU32 key_col = is_black ? IM_COL32(20, 20, 25, 255) : IM_COL32(220, 225, 230, 255);

                    draw->AddRectFilled(ImVec2(pr_p0.x + 2.0f, ky), ImVec2(grid_x0 - 2.0f, ky + key_h - 1.0f), key_col, 2.0f);
                    draw->AddLine(ImVec2(grid_x0, ky), ImVec2(pr_p0.x + pianoroll_w, ky), IM_COL32(255, 255, 255, 15));
                }

                // Desenhar Notas MIDI em Ciano Elétrico no Grid
                float note_w = (pianoroll_w - keybed_w) / 16.0f;
                int demo_pitches[] = { 10, 8, 7, 5, 3, 2, 0, 2, 5, 7, 8, 10 };

                for (int step = 0; step < 16; ++step) {
                    int pitch = demo_pitches[step % 12];
                    float nx0 = grid_x0 + step * note_w;
                    float ny0 = grid_y0 + pitch * key_h;

                    draw->AddRectFilled(ImVec2(nx0 + 1.0f, ny0 + 1.0f), ImVec2(nx0 + note_w - 1.0f, ny0 + key_h - 1.0f), IM_COL32(0, 229, 255, 255), 2.0f);
                    draw->AddRect(ImVec2(nx0 + 1.0f, ny0 + 1.0f), ImVec2(nx0 + note_w - 1.0f, ny0 + key_h - 1.0f), IM_COL32(255, 255, 255, 200), 2.0f);

                    // Barras de Velocidade na base
                    float vel_h = 12.0f;
                    float vy0 = pr_p0.y + bot_sz.y - 15.0f;
                    draw->AddLine(ImVec2(nx0 + note_w * 0.5f, vy0), ImVec2(nx0 + note_w * 0.5f, vy0 - vel_h), IM_COL32(0, 229, 255, 255), 2.0f);
                }
            }
            ImGui::End();

            // =========================================================================
            // 4. RIGHT SIDEBAR: MULTI-TRACK MIXER PANEL & MASTER VU (310px)
            // =========================================================================
            ImVec2 right_pos(work_pos.x + left_w + center_w, work_pos.y);
            ImGui::SetNextWindowPos(right_pos);
            ImGui::SetNextWindowSize(ImVec2(right_w, work_size.y));
            if (ImGui::Begin("##RightMixerWindow", nullptr, fixed_flags)) {
                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 mix_p0 = ImGui::GetCursorScreenPos();
                ImVec2 mix_sz = ImGui::GetContentRegionAvail();

                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_SLIDERS " PAINEL DE MIXAGEM & MASTER");
                ImGui::Separator();
                ImGui::Spacing();

                const char* ch_names[] = { "Master", "Kick", "Snare", "Hat", "Bass", "Lead" };
                int n_ch = 6;
                float strip_w = (mix_sz.x - 12.0f) / n_ch;

                for (int ch = 0; ch < n_ch; ++ch) {
                    float cx = mix_p0.x + 4.0f + ch * strip_w;
                    float cy = mix_p0.y + 26.0f;
                    float ch_h = mix_sz.y - 35.0f;

                    draw->AddRectFilled(ImVec2(cx, cy), ImVec2(cx + strip_w - 2.0f, cy + ch_h), IM_COL32(16, 20, 28, 255), 4.0f);
                    draw->AddRect(ImVec2(cx, cy), ImVec2(cx + strip_w - 2.0f, cy + ch_h), IM_COL32(35, 45, 60, 255));

                    // Nome da Faixa
                    draw->AddText(ImVec2(cx + 2.0f, cy + 4.0f), IM_COL32(200, 200, 200, 255), ch_names[ch]);

                    // Medidor VU Vertical
                    float vu_y0 = cy + 22.0f;
                    float vu_h = ch_h * 0.45f;
                    float vu_val = (ch == 0) ? (master_vu_level_l + master_vu_level_r) * 0.5f * 2.0f : track_vu_levels[ch - 1] * 2.5f;
                    vu_val = std::clamp(vu_val, 0.05f, 1.0f);

                    draw->AddRectFilled(ImVec2(cx + 6.0f, vu_y0), ImVec2(cx + strip_w - 8.0f, vu_y0 + vu_h), IM_COL32(10, 14, 18, 255));
                    draw->AddRectFilled(ImVec2(cx + 6.0f, vu_y0 + vu_h * (1.0f - vu_val)), ImVec2(cx + strip_w - 8.0f, vu_y0 + vu_h), IM_COL32(0, 229, 255, 255));

                    // Fader de Volume
                    float fader_y0 = vu_y0 + vu_h + 10.0f;
                    float fader_h = ch_h * 0.35f;
                    draw->AddLine(ImVec2(cx + strip_w * 0.5f, fader_y0), ImVec2(cx + strip_w * 0.5f, fader_y0 + fader_h), IM_COL32(60, 70, 85, 255), 2.0f);
                    draw->AddRectFilled(ImVec2(cx + 4.0f, fader_y0 + fader_h * 0.3f), ImVec2(cx + strip_w - 6.0f, fader_y0 + fader_h * 0.3f + 10.0f), IM_COL32(220, 220, 220, 255), 2.0f);
                }
            }
            ImGui::End();

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
        }
    };
}
