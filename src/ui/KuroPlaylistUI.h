#pragma once
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"
#include "../audio/SynthEngine.h"
#include "../ai/StemSeparationEngine.h"
#include "../core/PsySongArranger.h"
#include "SciFiIconSystem.h"
#include "ChannelInstrumentManager.h"
#include "FileDialog.h"

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

extern bool is_playing;
extern ClipManager g_clip_manager;
extern KuroDSP::TimelineManager timeline;
extern KuroAudio::SynthEngine g_piano_synth;
extern std::unique_ptr<StemSeparationEngine> g_ai_engine;
extern std::string track_names[MAX_TRACKS];
extern float track_linear_volumes[MAX_TRACKS];
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
namespace KuroUI {
    extern bool show_piano_roll;
    extern bool show_psy_arranger_modal;
    extern float g_playlist_scroll_x;
    extern float g_playlist_scroll_y;

    class CloudStemExtractorUI;
    extern CloudStemExtractorUI g_cloud_stem_ui;

    struct PlaylistTexturePack {
        GLuint tex_toolbar_tools = 0;
        GLuint tex_tool_pencil = 0;
        GLuint tex_tool_brush = 0;
        GLuint tex_tool_razor = 0;
        GLuint tex_tool_slip = 0;
        GLuint tex_tool_select = 0;
        GLuint tex_snap_pill = 0;
        GLuint tex_track_card_drums = 0;
        GLuint tex_track_card_psy_bass = 0;
        GLuint tex_track_card_acid_lead = 0;
        GLuint tex_track_card_automation = 0;
        GLuint tex_clip_audio_stereo = 0;
        GLuint tex_clip_audio_synth = 0;
        GLuint tex_clip_midi_purple = 0;
        GLuint tex_clip_midi_drop = 0;
        GLuint tex_clip_auto_bezier = 0;
        GLuint tex_playhead_pointer = 0;
        GLuint tex_btn_mute_solo = 0;
        GLuint tex_fader_slider_glow = 0;
        bool is_loaded = false;

        static GLuint UploadGLTexture(int w, int h, const unsigned char* data) {
            if (!data || w <= 0 || h <= 0) return 0;
            GLuint tex_id = 0;
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            return tex_id;
        }

        bool LoadFromBin(const std::string& filepath) {
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open()) return false;

            char magic[8];
            file.read(magic, 8);
            if (std::string(magic, 8) != "KUROTEX1") return false;

            uint32_t count = 0;
            file.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));

            struct TocEntry {
                char name[32];
                uint32_t w, h, size;
            };
            std::vector<TocEntry> toc(count);
            for (uint32_t i = 0; i < count; ++i) {
                file.read(reinterpret_cast<char*>(&toc[i]), sizeof(TocEntry));
            }

            for (uint32_t i = 0; i < count; ++i) {
                std::vector<unsigned char> buf(toc[i].size);
                file.read(reinterpret_cast<char*>(buf.data()), toc[i].size);
                std::string name(toc[i].name);
                GLuint tex = UploadGLTexture(toc[i].w, toc[i].h, buf.data());

                if (name == "toolbar_tools_capsule") tex_toolbar_tools = tex;
                else if (name == "tool_pencil") tex_tool_pencil = tex;
                else if (name == "tool_brush") tex_tool_brush = tex;
                else if (name == "tool_razor") tex_tool_razor = tex;
                else if (name == "tool_slip") tex_tool_slip = tex;
                else if (name == "tool_select") tex_tool_select = tex;
                else if (name == "snap_pill") tex_snap_pill = tex;
                else if (name == "track_card_drums") tex_track_card_drums = tex;
                else if (name == "track_card_psy_bass") tex_track_card_psy_bass = tex;
                else if (name == "track_card_acid_lead") tex_track_card_acid_lead = tex;
                else if (name == "track_card_automation") tex_track_card_automation = tex;
                else if (name == "clip_audio_stereo_waveform") tex_clip_audio_stereo = tex;
                else if (name == "clip_audio_synth_dual") tex_clip_audio_synth = tex;
                else if (name == "clip_midi_pattern_purple") tex_clip_midi_purple = tex;
                else if (name == "clip_midi_pattern_drop") tex_clip_midi_drop = tex;
                else if (name == "clip_automation_bezier") tex_clip_auto_bezier = tex;
                else if (name == "playhead_laser_pointer") tex_playhead_pointer = tex;
                else if (name == "btn_mute_solo_pair") tex_btn_mute_solo = tex;
                else if (name == "fader_slider_glow") tex_fader_slider_glow = tex;
            }

            is_loaded = true;
            return true;
        }

        void Init() {
            if (is_loaded) return;
            std::vector<std::string> paths = {
                "assets/ui/playlist/textures.bin",
                "assets/ui/playlist_textures.bin",
                "../assets/ui/playlist/textures.bin",
                "../../assets/ui/playlist/textures.bin"
            };
            for (const auto& p : paths) {
                if (std::filesystem::exists(p) && LoadFromBin(p)) {
                    break;
                }
            }
        }
    };

    class KuroPlaylistUI {
    public:
        enum PlaylistTool {
            TOOL_DRAW = 0,    // Pencil (P)
            TOOL_PAINT,       // Brush (B)
            TOOL_DELETE,      // Eraser (D / E)
            TOOL_MUTE,        // Mute (T)
            TOOL_SLICE,       // Razor / Knife (C)
            TOOL_SELECT,      // Select / Area (S)
            TOOL_SLIP,        // Slip / Nudge
            TOOL_ZOOM         // Zoom (Z)
        };

        PlaylistTexturePack textures;

        PlaylistTool current_tool = TOOL_DRAW;
        int current_snap = 0;          // 0 = 1/4 Beat (16th)
        float playlist_zoom_x = 42.0f; // Pixels per Bar
        float playlist_scroll_x = 0.0f;
        float playlist_scroll_y = 0.0f;
        bool snap_to_zero_crossing = false;
        bool follow_playhead = true;
        bool scroll_initialized = false;
        float playlist_track_h = 58.0f;
        float track_card_w = 260.0f;
        bool show_ghost_waveform = false;

        bool is_dragging_clip = false;
        bool is_resizing_right = false;
        bool is_resizing_left = false;
        bool is_slicing = false;
        bool is_scrubbing_ruler = false;
        bool is_dragging_loop = false;
        bool is_dragging_fader = false;
        int active_fader_track = -1;

        int active_drag_track = -1;
        int active_drag_clip_idx = -1;
        int active_drag_type = 0; // 0=Audio, 1=MIDI, 2=Auto
        float drag_orig_start = 0.0f;
        float drag_orig_len = 0.0f;
        float drag_orig_offset = 0.0f;
        ImVec2 drag_mouse_start_pos = ImVec2(0, 0);

        bool show_timeline_inpainting_modal = false;
        int rename_track_target = -1;
        char rename_buffer[128] = { 0 };

        KuroPlaylistUI() {}

        void render(bool* p_open = nullptr) {
            textures.Init();

            if (!scroll_initialized) {
                if (g_playlist_scroll_x > 0.0f) playlist_scroll_x = g_playlist_scroll_x;
                if (g_playlist_scroll_y > 0.0f) playlist_scroll_y = g_playlist_scroll_y;
                scroll_initialized = true;
            }

            ImGui::SetNextWindowSize(ImVec2(1200, 680), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(80, 60), ImGuiCond_FirstUseEver);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.045f, 0.060f, 0.085f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.08f, 0.16f, 0.24f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

            char title_buf[128];
            snprintf(title_buf, sizeof(title_buf), "PLAYLIST / SONG ARRANGER - Abduction Studio V2 [BPM: %.1f]###KuroPlaylistWindow", ::timeline.getBPM());

            if (ImGui::Begin(title_buf, p_open, ImGuiWindowFlags_NoCollapse)) {
                ImDrawList* draw = ImGui::GetWindowDrawList();

                float bpm = ::timeline.getBPM();
                if (bpm <= 10.0f) bpm = 140.0f;
                float beat_len_sec = 60.0f / bpm;
                float bar_len_sec = beat_len_sec * 4.0f;

                auto get_snap_step_sec = [&](int snap_mode) -> float {
                    switch (snap_mode) {
                        case 0: return beat_len_sec * 0.25f; // 1/16th (1/4 beat)
                        case 1: return beat_len_sec * 0.50f; // 1/8th (1/2 beat)
                        case 2: return beat_len_sec;         // 1 Beat (1/4 bar)
                        case 3: return beat_len_sec * 2.0f;  // 1/2 Bar
                        case 4: return bar_len_sec;          // 1 Bar
                        default: return 0.001f;              // Free
                    }
                };
                float snap_sec = get_snap_step_sec(current_snap);

                auto snap_time = [snap_sec](float t) -> float {
                    if (snap_sec <= 0.005f) return (std::max)(0.0f, t);
                    return (std::max)(0.0f, std::round(t / snap_sec) * snap_sec);
                };

                handleKeyboardShortcuts(bar_len_sec);

                // 1. TOOLBAR SUPERIOR MODULAR COM TODAS AS FERRAMENTAS
                renderModernToolbar(bpm, bar_len_sec);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 2. LAYOUT PRINCIPAL: RÉGUA TEMPORAL + TRACK HEADERS + GRID
                ImVec2 grid_p0 = ImGui::GetCursorScreenPos();
                ImVec2 avail = ImGui::GetContentRegionAvail();
                float view_w = avail.x;
                float view_h = avail.y;

                float ruler_h = 32.0f;
                float trk_h = playlist_track_h;
                float time_to_px = playlist_zoom_x / bar_len_sec;

                float actual_song_end_sec = ::timeline.getSongEndSec();
                float total_song_sec = (std::max)(actual_song_end_sec + bar_len_sec * 4.0f, bar_len_sec * 32.0f);
                float grid_total_w = total_song_sec * time_to_px;

                ImGuiIO& io = ImGui::GetIO();
                bool is_playlist_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
                if (is_playlist_hovered && io.MouseWheel != 0.0f) {
                    if (io.KeyCtrl) {
                        float zoom_delta = io.MouseWheel * 4.0f;
                        playlist_zoom_x = (std::clamp)(playlist_zoom_x + zoom_delta, 14.0f, 160.0f);
                    } else if (io.KeyShift) {
                        playlist_scroll_x = (std::clamp)(playlist_scroll_x - io.MouseWheel * 60.0f, 0.0f, (std::max)(0.0f, grid_total_w - (view_w - track_card_w - 20.0f)));
                    } else {
                        playlist_scroll_y = (std::clamp)(playlist_scroll_y - io.MouseWheel * 40.0f, 0.0f, (std::max)(0.0f, (MAX_TRACKS * trk_h) - (view_h - ruler_h - 20.0f)));
                    }
                }

                draw->AddRectFilled(grid_p0, ImVec2(grid_p0.x + view_w, grid_p0.y + view_h), IM_COL32(8, 12, 18, 255), 6.0f);
                draw->AddRect(grid_p0, ImVec2(grid_p0.x + view_w, grid_p0.y + view_h), IM_COL32(20, 32, 48, 200), 6.0f);

                renderRuler(draw, grid_p0, view_w, ruler_h, time_to_px, bar_len_sec, total_song_sec, is_playlist_hovered);

                renderTracksAndClips(draw, grid_p0, view_w, view_h, ruler_h, trk_h, time_to_px, bar_len_sec, snap_sec, snap_time, is_playlist_hovered);

                renderLaserPlayhead(draw, grid_p0, view_w, view_h, ruler_h, time_to_px);

                renderScrollbars(grid_p0, view_w, view_h, ruler_h, grid_total_w, trk_h);

                renderSubModals();
            }
            ImGui::End();

            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);
        }

    private:
        void handleKeyboardShortcuts(float bar_len_sec) {
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                if (ImGui::IsKeyPressed(ImGuiKey_P)) current_tool = TOOL_DRAW;
                if (ImGui::IsKeyPressed(ImGuiKey_B)) current_tool = TOOL_PAINT;
                if (ImGui::IsKeyPressed(ImGuiKey_D) || ImGui::IsKeyPressed(ImGuiKey_E)) current_tool = TOOL_DELETE;
                if (ImGui::IsKeyPressed(ImGuiKey_T)) current_tool = TOOL_MUTE;
                if (ImGui::IsKeyPressed(ImGuiKey_C)) current_tool = TOOL_SLICE;
                if (ImGui::IsKeyPressed(ImGuiKey_S) && !ImGui::GetIO().KeyCtrl) current_tool = TOOL_SELECT;
                if (ImGui::IsKeyPressed(ImGuiKey_Z) && !ImGui::GetIO().KeyCtrl) current_tool = TOOL_ZOOM;

                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B)) {
                    if (active_drag_track >= 0 && active_drag_track < MAX_TRACKS) {
                        if (active_drag_type == 0 && active_drag_clip_idx >= 0) {
                            g_clip_manager.duplicateAudioClip(active_drag_track, active_drag_clip_idx, bar_len_sec);
                        } else if (active_drag_type == 1 && active_drag_clip_idx >= 0) {
                            g_clip_manager.duplicateMidiClip(active_drag_track, active_drag_clip_idx, bar_len_sec);
                        }
                    }
                }
            }
        }


        // ── VECTOR HUD ICONS FOR PLAYLIST TOOLBAR ────────────────────────────────
        static void DrawIconPencil(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p_tip(c.x - r * 0.65f, c.y + r * 0.65f);
            ImVec2 p_top(c.x + r * 0.65f, c.y - r * 0.65f);
            float w = 2.4f;
            draw->AddLine(ImVec2(p_tip.x - w, p_tip.y - w), ImVec2(p_top.x - w, p_top.y - w), col, 1.4f);
            draw->AddLine(ImVec2(p_tip.x + w, p_tip.y + w), ImVec2(p_top.x + w, p_top.y + w), col, 1.4f);
            draw->AddTriangleFilled(ImVec2(p_tip.x - w, p_tip.y - w), ImVec2(p_tip.x + w, p_tip.y + w), ImVec2(c.x - r * 0.95f, c.y + r * 0.95f), col);
            draw->AddLine(ImVec2(p_top.x - w, p_top.y - w), ImVec2(p_top.x + w, p_top.y + w), col, 1.8f);
        }

        static void DrawIconBrush(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x + r * 0.1f, c.y - r * 0.1f), ImVec2(c.x + r * 0.85f, c.y - r * 0.85f), col, 2.0f);
            draw->AddLine(ImVec2(c.x - r * 0.15f, c.y - r * 0.15f), ImVec2(c.x + r * 0.2f, c.y + r * 0.2f), col, 2.5f);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.15f, c.y - r * 0.15f), ImVec2(c.x + r * 0.2f, c.y + r * 0.2f), ImVec2(c.x - r * 0.80f, c.y + r * 0.80f), col);
        }

        static void DrawIconRazor(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 pts[4] = {
                ImVec2(c.x - r * 0.75f, c.y - r * 0.35f),
                ImVec2(c.x + r * 0.45f, c.y - r * 0.85f),
                ImVec2(c.x + r * 0.75f, c.y + r * 0.35f),
                ImVec2(c.x - r * 0.45f, c.y + r * 0.85f)
            };
            draw->AddPolyline(pts, 4, col, ImDrawFlags_Closed, 1.4f);
            draw->AddLine(ImVec2(c.x - r * 0.35f, c.y + r * 0.12f), ImVec2(c.x + r * 0.35f, c.y - r * 0.12f), col, 1.4f);
            draw->AddCircleFilled(c, 1.5f, col);
        }

        static void DrawIconSlip(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - 3.5f, c.y - r * 0.75f), ImVec2(c.x - 3.5f, c.y + r * 0.75f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + 3.5f, c.y - r * 0.75f), ImVec2(c.x + 3.5f, c.y + r * 0.75f), col, 1.5f);
            draw->AddTriangleFilled(ImVec2(c.x - 5.5f, c.y), ImVec2(c.x - 1.5f, c.y - 3.0f), ImVec2(c.x - 1.5f, c.y + 3.0f), col);
            draw->AddTriangleFilled(ImVec2(c.x + 5.5f, c.y), ImVec2(c.x + 1.5f, c.y - 3.0f), ImVec2(c.x + 1.5f, c.y + 3.0f), col);
        }

        static void DrawIconSelect(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p0(c.x - r * 0.65f, c.y - r * 0.80f);
            ImVec2 p1(c.x + r * 0.65f, c.y + r * 0.10f);
            ImVec2 p2(c.x - r * 0.05f, c.y + r * 0.18f);
            ImVec2 p3(c.x - r * 0.35f, c.y + r * 0.85f);
            draw->AddTriangleFilled(p0, p1, p2, col);
            draw->AddLine(p2, p3, col, 2.0f);
        }

        static void DrawIconDelete(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRect(ImVec2(c.x - r * 0.50f, c.y - r * 0.20f), ImVec2(c.x + r * 0.50f, c.y + r * 0.75f), col, 1.5f, 0, 1.2f);
            draw->AddLine(ImVec2(c.x - r * 0.70f, c.y - r * 0.35f), ImVec2(c.x + r * 0.70f, c.y - r * 0.35f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - r * 0.25f, c.y - r * 0.65f), ImVec2(c.x + r * 0.25f, c.y - r * 0.65f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - 2.0f, c.y), ImVec2(c.x - 2.0f, c.y + r * 0.55f), col, 1.0f);
            draw->AddLine(ImVec2(c.x + 2.0f, c.y), ImVec2(c.x + 2.0f, c.y + r * 0.55f), col, 1.0f);
        }

        static void DrawIconMute(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRectFilled(ImVec2(c.x - r * 0.75f, c.y - r * 0.35f), ImVec2(c.x - r * 0.30f, c.y + r * 0.35f), col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.30f, c.y - r * 0.35f), ImVec2(c.x + r * 0.05f, c.y - r * 0.75f), ImVec2(c.x + r * 0.05f, c.y + r * 0.75f), col);
            draw->AddLine(ImVec2(c.x + r * 0.30f, c.y - r * 0.50f), ImVec2(c.x + r * 0.80f, c.y + r * 0.50f), col, 1.6f);
            draw->AddLine(ImVec2(c.x + r * 0.80f, c.y - r * 0.50f), ImVec2(c.x + r * 0.30f, c.y + r * 0.50f), col, 1.6f);
        }

        static void DrawIconSnap(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.75f, c.y), ImVec2(c.x + r * 0.75f, c.y), col, 1.5f);
            draw->AddLine(ImVec2(c.x, c.y - r * 0.75f), ImVec2(c.x, c.y + r * 0.75f), col, 1.5f);
            draw->AddCircleFilled(c, 1.5f, col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.85f, c.y), ImVec2(c.x - r * 0.50f, c.y - 2.5f), ImVec2(c.x - r * 0.50f, c.y + 2.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.85f, c.y), ImVec2(c.x + r * 0.50f, c.y - 2.5f), ImVec2(c.x + r * 0.50f, c.y + 2.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x, c.y - r * 0.85f), ImVec2(c.x - 2.5f, c.y - r * 0.50f), ImVec2(c.x + 2.5f, c.y - r * 0.50f), col);
            draw->AddTriangleFilled(ImVec2(c.x, c.y + r * 0.85f), ImVec2(c.x - 2.5f, c.y + r * 0.50f), ImVec2(c.x + 2.5f, c.y + r * 0.50f), col);
        }

        static void DrawIconPattern(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddCircleFilled(ImVec2(c.x - r * 0.40f, c.y + r * 0.45f), 2.2f, col);
            draw->AddCircleFilled(ImVec2(c.x + r * 0.40f, c.y + r * 0.25f), 2.2f, col);
            draw->AddLine(ImVec2(c.x - r * 0.22f, c.y + r * 0.45f), ImVec2(c.x - r * 0.22f, c.y - r * 0.55f), col, 1.4f);
            draw->AddLine(ImVec2(c.x + r * 0.58f, c.y + r * 0.25f), ImVec2(c.x + r * 0.58f, c.y - r * 0.75f), col, 1.4f);
            draw->AddLine(ImVec2(c.x - r * 0.22f, c.y - r * 0.55f), ImVec2(c.x + r * 0.58f, c.y - r * 0.75f), col, 2.2f);
        }

        static void DrawIconPsytrance(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddEllipse(c, ImVec2(r * 0.85f, r * 0.35f), col, 0.0f, 16, 1.5f);
            draw->AddCircle(ImVec2(c.x, c.y - r * 0.25f), r * 0.40f, col, 12, 1.4f);
            draw->AddCircleFilled(ImVec2(c.x - r * 0.45f, c.y), 1.2f, col);
            draw->AddCircleFilled(ImVec2(c.x, c.y + 1.0f), 1.4f, col);
            draw->AddCircleFilled(ImVec2(c.x + r * 0.45f, c.y), 1.2f, col);
        }

        static void DrawIconBezier(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p0(c.x - r * 0.75f, c.y + r * 0.55f);
            ImVec2 cp1(c.x - r * 0.25f, c.y - r * 0.75f);
            ImVec2 cp2(c.x + r * 0.25f, c.y + r * 0.75f);
            ImVec2 p1(c.x + r * 0.75f, c.y - r * 0.55f);
            draw->AddBezierCubic(p0, cp1, cp2, p1, col, 1.8f);
            draw->AddCircleFilled(p0, 2.0f, col);
            draw->AddCircleFilled(p1, 2.0f, col);
            draw->AddCircle(cp1, 1.5f, col);
            draw->AddCircle(cp2, 1.5f, col);
        }

        static void DrawIconAiSparkle(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddQuadFilled(
                ImVec2(c.x, c.y - r * 0.85f),
                ImVec2(c.x + r * 0.35f, c.y),
                ImVec2(c.x, c.y + r * 0.85f),
                ImVec2(c.x - r * 0.35f, c.y),
                col
            );
            draw->AddCircleFilled(ImVec2(c.x + r * 0.65f, c.y - r * 0.55f), 1.4f, col);
            draw->AddCircleFilled(ImVec2(c.x - r * 0.65f, c.y + r * 0.55f), 1.2f, col);
        }

        static void DrawIconLoop(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddCircle(c, r * 0.65f, col, 16, 1.5f);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.65f, c.y - 1.0f), ImVec2(c.x + r * 0.95f, c.y - 4.5f), ImVec2(c.x + r * 0.35f, c.y - 4.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.65f, c.y + 1.0f), ImVec2(c.x - r * 0.95f, c.y + 4.5f), ImVec2(c.x - r * 0.35f, c.y + 4.5f), col);
        }

        static void DrawIconPresets(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y - r * 0.5f), ImVec2(c.x + r * 0.65f, c.y - r * 0.5f), col, 1.8f);
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y), ImVec2(c.x + r * 0.35f, c.y), col, 1.8f);
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y + r * 0.5f), ImVec2(c.x + r * 0.65f, c.y + r * 0.5f), col, 1.8f);
        }

                static void DrawIconZeroCross(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.55f, c.y - r * 0.70f), ImVec2(c.x - r * 0.55f, c.y + r * 0.70f), col, 2.0f);
            draw->AddLine(ImVec2(c.x - r * 0.55f, c.y), ImVec2(c.x + r * 0.35f, c.y), col, 1.8f);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.75f, c.y), ImVec2(c.x + r * 0.20f, c.y - r * 0.40f), ImVec2(c.x + r * 0.20f, c.y + r * 0.40f), col);
        }

        static void DrawIconFollow(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRectFilled(ImVec2(c.x - r * 0.65f, c.y - r * 0.70f), ImVec2(c.x - r * 0.18f, c.y + r * 0.70f), col, 1.0f);
            draw->AddRectFilled(ImVec2(c.x + r * 0.18f, c.y - r * 0.70f), ImVec2(c.x + r * 0.65f, c.y + r * 0.70f), col, 1.0f);
        }

        static void DrawIconFit(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            float d = r * 0.70f;
            draw->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x - d + 3.5f, c.y - d), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x - d, c.y - d + 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y - d), ImVec2(c.x + d - 3.5f, c.y - d), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y - d), ImVec2(c.x + d, c.y - d + 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y + d), ImVec2(c.x - d + 3.5f, c.y + d), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y + d), ImVec2(c.x - d, c.y + d - 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y + d), ImVec2(c.x + d - 3.5f, c.y + d), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y + d), ImVec2(c.x + d, c.y + d - 3.5f), col, 1.5f);
        }

        void renderModernToolbar(float bpm, float bar_len_sec) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImGuiIO& io = ImGui::GetIO();

            // ── 1. TOOLBAR CAPSULE CONTAINER (Pencil, Brush, Razor, Slip, Select, Del, Mute) ──
            struct ToolDef {
                PlaylistTool tool;
                const char* label;
                const char* tooltip;
            };
            ToolDef tools[] = {
                { TOOL_DRAW,   "Pencil", "Draw / Pencil Tool (P): Inserir ou mover clipes na grade" },
                { TOOL_PAINT,  "Brush",  "Paint / Brush Tool (B): Pintar multiplos clipes consecutivos" },
                { TOOL_SLICE,  "Razor",  "Slice / Razor Tool (C): Cortar clipes na posicao do cursor" },
                { TOOL_SLIP,   "Slip",   "Slip Tool: Deslocar conteudo interno de audio/MIDI sem mover o clipe" },
                { TOOL_SELECT, "Select", "Select Tool (S): Selecao retangular de clipes em area" },
                { TOOL_DELETE, "Del",    "Delete / Eraser Tool (D): Excluir clipes com clique" },
                { TOOL_MUTE,   "Mute",   "Mute Tool (T): Mutar / desmutar clipes individualmente" }
            };

            float tool_w = 40.0f;
            float capsule_h = 36.0f;
            float capsule_w = tool_w * 7.0f + 6.0f;
            ImVec2 cap_p0 = ImGui::GetCursorScreenPos();
            ImVec2 cap_p1(cap_p0.x + capsule_w, cap_p0.y + capsule_h);

            // Background & Border of the Tool Capsule
            draw->AddRectFilled(cap_p0, cap_p1, IM_COL32(12, 18, 26, 250), 7.0f);
            draw->AddRect(cap_p0, cap_p1, IM_COL32(30, 44, 60, 255), 7.0f, 0, 1.0f);

            for (int i = 0; i < 7; ++i) {
                ImVec2 b_p0(cap_p0.x + 3.0f + i * tool_w, cap_p0.y + 2.0f);
                ImVec2 b_p1(b_p0.x + tool_w - 1.0f, cap_p0.y + capsule_h - 2.0f);

                bool is_act = (current_tool == tools[i].tool);
                bool is_hov = (io.MousePos.x >= b_p0.x && io.MousePos.x <= b_p1.x && io.MousePos.y >= b_p0.y && io.MousePos.y <= b_p1.y);

                if (is_hov && io.MouseClicked[0]) {
                    current_tool = tools[i].tool;
                    is_act = true;
                }

                if (is_act) {
                    draw->AddRectFilled(b_p0, b_p1, IM_COL32(0, 180, 220, 50), 5.0f);
                    draw->AddRect(b_p0, b_p1, IM_COL32(0, 229, 255, 230), 5.0f, 0, 1.5f);
                    draw->AddLine(ImVec2(b_p0.x + 4.0f, b_p1.y - 1.0f), ImVec2(b_p1.x - 4.0f, b_p1.y - 1.0f), IM_COL32(0, 255, 255, 255), 2.0f);
                } else if (is_hov) {
                    draw->AddRectFilled(b_p0, b_p1, IM_COL32(28, 42, 58, 200), 5.0f);
                }

                ImU32 icon_col = is_act ? IM_COL32(0, 240, 255, 255) : (is_hov ? IM_COL32(230, 245, 255, 255) : IM_COL32(135, 155, 180, 255));
                ImU32 text_col = is_act ? IM_COL32(0, 240, 255, 255) : (is_hov ? IM_COL32(220, 240, 255, 255) : IM_COL32(110, 135, 160, 255));

                ImVec2 icon_center(b_p0.x + (tool_w - 1.0f) * 0.5f, b_p0.y + 11.0f);

                switch (tools[i].tool) {
                    case TOOL_DRAW:   DrawIconPencil(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_PAINT:  DrawIconBrush(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_SLICE:  DrawIconRazor(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_SLIP:   DrawIconSlip(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_SELECT: DrawIconSelect(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_DELETE: DrawIconDelete(draw, icon_center, 15.0f, icon_col); break;
                    case TOOL_MUTE:   DrawIconMute(draw, icon_center, 15.0f, icon_col); break;
                    default: break;
                }

                ImVec2 t_sz = ImGui::CalcTextSize(tools[i].label);
                draw->AddText(ImVec2(icon_center.x - t_sz.x * 0.5f, b_p0.y + 20.0f), text_col, tools[i].label);

                if (is_hov) {
                    ImGui::SetTooltip("%s", tools[i].tooltip);
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(cap_p1.x + 8.0f, cap_p0.y + 4.0f));

            // ── 2. SNAP PILL WITH HUD ICON (Matching Mockup) ──
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.13f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.22f, 0.32f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImVec2 snap_p0 = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemWidth(132);
            const char* snap_names[] = { "       1/4 Beat", "       1/2 Beat", "       1 Beat", "       1/2 Bar", "       1 Bar", "       Free" };
            if (ImGui::Combo("##PlaylistSnap", &current_snap, snap_names, IM_ARRAYSIZE(snap_names))) {
            }
            DrawIconSnap(draw, ImVec2(snap_p0.x + 12.0f, snap_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);

            ImGui::SameLine(0, 5);

            // ── 3. THREE MOCKUP TOOL BUTTONS: ZeroCross [|->], Fit [[]], Follow [||] ──
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.20f, 0.30f, 1.00f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            // 3A. Zero-Crossing Snap [ |-> ]
            ImVec2 zc_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##ZeroCrossBtn", ImVec2(26, 26))) {
                snap_to_zero_crossing = !snap_to_zero_crossing;
            }
            DrawIconZeroCross(draw, ImVec2(zc_p0.x + 13.0f, zc_p0.y + 13.0f), 13.0f, snap_to_zero_crossing ? IM_COL32(0, 240, 255, 255) : IM_COL32(130, 155, 185, 220));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zero-Crossing Snap: Alinhar cortes no cruzamento por zero");

            ImGui::SameLine(0, 4);

            // 3B. Fit Timeline to Screen [ [ ] ]
            ImVec2 fit_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##FitBtn", ImVec2(26, 26))) {
                float song_end = ::timeline.getSongEndSec();
                if (song_end > 1.0f) {
                    float avail_w = ImGui::GetContentRegionAvail().x - track_card_w - 40.0f;
                    if (avail_w > 100.0f) {
                        playlist_zoom_x = (avail_w / song_end) * bar_len_sec;
                        playlist_zoom_x = (std::clamp)(playlist_zoom_x, 12.0f, 240.0f);
                        playlist_scroll_x = 0.0f;
                    }
                }
            }
            DrawIconFit(draw, ImVec2(fit_p0.x + 13.0f, fit_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Fit to Screen: Enquadrar toda a musica na tela");

            ImGui::SameLine(0, 4);

            // 3C. Follow Playhead [ || ]
            ImVec2 fol_p0 = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##FollowBtn", ImVec2(26, 26))) {
                follow_playhead = !follow_playhead;
            }
            DrawIconFollow(draw, ImVec2(fol_p0.x + 13.0f, fol_p0.y + 13.0f), 13.0f, follow_playhead ? IM_COL32(0, 240, 255, 255) : IM_COL32(130, 155, 185, 220));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Follow Playhead: Seguir cursor durante reproducao");

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(1);

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 4. PATTERN PICKER WITH NOTE ICON ──
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.13f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.22f, 0.32f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImVec2 pat_p0 = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemWidth(142);
            std::vector<std::string> pat_names;
            for (const auto& p : g_clip_manager.global_patterns) {
                pat_names.push_back("       " + p.name);
            }
            if (pat_names.empty()) pat_names.push_back("       Pattern 1");
            int pat_idx = (std::clamp)(g_clip_manager.current_pattern_idx, 0, (int)pat_names.size() - 1);
            if (ImGui::BeginCombo("##PatPicker", pat_names[pat_idx].c_str())) {
                for (size_t p = 0; p < pat_names.size(); p++) {
                    bool is_sel = (p == (size_t)pat_idx);
                    if (ImGui::Selectable(pat_names[p].c_str(), is_sel)) {
                        g_clip_manager.current_pattern_idx = (int)p;
                    }
                    if (is_sel) ImGui::SetItemDefaultFocus();
                }
                if (ImGui::Selectable("  + Novo Padrao (Pattern)...")) {
                    Pattern np;
                    np.id = g_clip_manager.next_id++;
                    np.name = "Pattern " + std::to_string(g_clip_manager.global_patterns.size() + 1);
                    np.color = 0xFF00E5FF;
                    g_clip_manager.global_patterns.push_back(np);
                    g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                }
                ImGui::EndCombo();
            }
            DrawIconPattern(draw, ImVec2(pat_p0.x + 12.0f, pat_p0.y + 13.0f), 13.0f, IM_COL32(180, 130, 255, 240));

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 5. ARRANJADOR PSYTRANCE WITH SAUCER ICON ──
            ImVec2 psy_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.75f, 0.98f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.88f, 1.00f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.04f, 0.08f, 1.0f));
            if (ImGui::Button("       ARRANJADOR PSYTRANCE", ImVec2(0, 26))) {
                show_psy_arranger_modal = true;
            }
            ImGui::PopStyleColor(3);
            DrawIconPsytrance(draw, ImVec2(psy_p0.x + 13.0f, psy_p0.y + 13.0f), 14.0f, IM_COL32(0, 20, 35, 255));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Gerador de Estrutura Completa Psytrance (Intro, Build-ups, Drops, Breaks e Automacoes)");

            ImGui::SameLine(0, 5);

            // ── 6. + AUTO [v] WITH BEZIER ICON ──
            ImVec2 auto_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.16f, 0.24f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.22f, 0.32f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.88f, 1.0f, 1.0f));
            if (ImGui::Button("      + AUTO [v]", ImVec2(0, 26))) {
                ImGui::OpenPopup("AddAutoClipPopup");
            }
            ImGui::PopStyleColor(3);
            DrawIconBezier(draw, ImVec2(auto_p0.x + 12.0f, auto_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 240));

            if (ImGui::BeginPopup("AddAutoClipPopup")) {
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Criar Clipe de Automacao:");
                ImGui::Separator();
                if (ImGui::MenuItem("Master Volume Auto (Bezier)")) {
                    g_clip_manager.addAutomationClip(0, 0.0f, bar_len_sec * 8.0f, "Master Volume", "Master", 0);
                }
                if (ImGui::MenuItem("Filter Cutoff Sweep Auto")) {
                    g_clip_manager.addAutomationClip(1, 0.0f, bar_len_sec * 8.0f, "Cutoff Sweep", "Filter", 1);
                }
                if (ImGui::MenuItem("Reverb Wet Mix Auto")) {
                    g_clip_manager.addAutomationClip(2, 0.0f, bar_len_sec * 8.0f, "Reverb Wet", "Reverb", 2);
                }
                if (ImGui::MenuItem("Pitch Riser Auto (+12 st)")) {
                    g_clip_manager.addAutomationClip(3, 0.0f, bar_len_sec * 4.0f, "Pitch Riser", "Pitch", 3);
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 5);

            // ── 7. IA TOOLS [v] WITH SPARKLE ICON ──
            ImVec2 ai_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.48f, 0.16f, 0.85f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.22f, 0.95f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("      IA TOOLS [v]", ImVec2(0, 26))) {
                ImGui::OpenPopup("PlaylistAiToolsPopup");
            }
            ImGui::PopStyleColor(3);
            DrawIconAiSparkle(draw, ImVec2(ai_p0.x + 12.0f, ai_p0.y + 13.0f), 13.0f, IM_COL32(255, 230, 255, 255));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Suite de Inteligencia Artificial: Desconstrucao de Stems, Inpainting Generativo e Alinhamento Kick/Bass");

            if (ImGui::BeginPopup("PlaylistAiToolsPopup")) {
                ImGui::TextColored(ImVec4(0.85f, 0.45f, 1.0f, 1.0f), "SUITE DE INTELIGENCIA ARTIFICIAL");
                ImGui::Separator();
                if (ImGui::MenuItem("Desconstruir Musica (IA) - Separar Stems para Arranjo")) {
                    std::string chosen = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3;*.flac)\0*.wav;*.mp3;*.flac\0");
                    if (!chosen.empty()) {
                        g_cloud_stem_ui.DeconstructAnySong(chosen, g_clip_manager, ::timeline);
                    }
                }
                if (ImGui::MenuItem("AI Inpaint Arranjo (Variacoes de Build-up / Drops)")) {
                    show_timeline_inpainting_modal = true;
                }
                ImGui::Separator();
                std::string ghost_lbl = show_ghost_waveform ? "Desativar Ghost Align (Kick/Bass)" : "Ativar Ghost Align (Kick/Bass)";
                if (ImGui::MenuItem(ghost_lbl.c_str())) {
                    show_ghost_waveform = !show_ghost_waveform;
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 8. LOOP TOGGLE WITH REPEAT ICON ──
            ImVec2 loop_p0 = ImGui::GetCursorScreenPos();
            bool is_loop = ::timeline.loop_enabled;
            if (is_loop) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.75f, 0.90f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.05f, 0.10f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 1.0f));
            }
            if (ImGui::Button(is_loop ? "      LOOP: ON" : "      LOOP: OFF", ImVec2(0, 26))) {
                ::timeline.loop_enabled = !::timeline.loop_enabled;
            }
            ImGui::PopStyleColor(2);
            DrawIconLoop(draw, ImVec2(loop_p0.x + 12.0f, loop_p0.y + 13.0f), 13.0f, is_loop ? IM_COL32(0, 20, 35, 255) : IM_COL32(0, 229, 255, 230));

            ImGui::SameLine(0, 5);

            // ── 9. ARRANJOS PRESETS WITH STACK ICON ──
            ImVec2 arr_p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 1.0f));
            if (ImGui::Button("      Arranjos [v]", ImVec2(0, 26))) {
                ImGui::OpenPopup("ArrangementPresetsPopup");
            }
            ImGui::PopStyleColor(2);
            DrawIconPresets(draw, ImVec2(arr_p0.x + 12.0f, arr_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 220));

            if (ImGui::BeginPopup("ArrangementPresetsPopup")) {
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "Presets de Estrutura de Musica:");
                ImGui::Separator();
                if (ImGui::MenuItem("Full Psytrance 138 (Padrao Comercial - 7min)")) {
                    KuroArranger::PsyArrangerConfig cfg;
                    cfg.bpm = 138.0f;
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                if (ImGui::MenuItem("Full-On Power Psy (142 BPM - Astrix/Adhana Style)")) {
                    KuroArranger::PsyArrangerConfig cfg;
                    cfg.bpm = 142.0f;
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                if (ImGui::MenuItem("Dark / Twilight Psy (148 BPM - Fast Peak)")) {
                    KuroArranger::PsyArrangerConfig cfg;
                    cfg.bpm = 148.0f;
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                }
                if (ImGui::MenuItem("Limpar Todos os Clipes da Playlist")) {
                    g_clip_manager.pushUndo();
                    g_clip_manager.reset();
                    ::timeline.clearSectionMarkers();
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 5);

            // ── 10. ZOOM CONTROLS [-] [+] ──
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.90f, 1.00f, 1.0f));

            if (ImGui::Button("-##ZoomOut", ImVec2(26, 26))) {
                playlist_zoom_x = (std::max)(12.0f, playlist_zoom_x * 0.80f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom Out (-)");

            ImGui::SameLine(0, 3);
            if (ImGui::Button("+##ZoomIn", ImVec2(26, 26))) {
                playlist_zoom_x = (std::min)(240.0f, playlist_zoom_x * 1.25f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom In (+)");

            ImGui::PopStyleColor(2);
        }

        void renderRuler(ImDrawList* draw, ImVec2 grid_p0, float view_w, float ruler_h, float time_to_px, float bar_len_sec, float total_song_sec, bool is_hovered) {
            float grid_start_x = grid_p0.x + track_card_w;
            float grid_view_w = view_w - track_card_w;

            draw->AddRectFilled(ImVec2(grid_p0.x, grid_p0.y), ImVec2(grid_p0.x + view_w, grid_p0.y + ruler_h), IM_COL32(11, 16, 24, 255), 4.0f);
            draw->AddLine(ImVec2(grid_p0.x, grid_p0.y + ruler_h), ImVec2(grid_p0.x + view_w, grid_p0.y + ruler_h), IM_COL32(24, 38, 56, 255), 1.0f);

            draw->AddText(ImVec2(grid_p0.x + 14.0f, grid_p0.y + 8.0f), IM_COL32(0, 229, 255, 230), "TRACKS");
            draw->AddText(ImVec2(grid_p0.x + track_card_w - 55.0f, grid_p0.y + 8.0f), IM_COL32(130, 155, 185, 200), "VOL / M / S");

            draw->PushClipRect(ImVec2(grid_start_x, grid_p0.y), ImVec2(grid_start_x + grid_view_w, grid_p0.y + ruler_h), true);

            int total_bars = (int)std::ceil(total_song_sec / bar_len_sec) + 16;
            for (int bar = 0; bar < total_bars; ++bar) {
                float bar_time = bar * bar_len_sec;
                float bx = grid_start_x + (bar_time * time_to_px) - playlist_scroll_x;
                if (bx < grid_start_x - 50.0f || bx > grid_start_x + grid_view_w + 50.0f) continue;

                draw->AddLine(ImVec2(bx, grid_p0.y + ruler_h - 12.0f), ImVec2(bx, grid_p0.y + ruler_h), IM_COL32(0, 200, 255, 180), 1.0f);

                for (int beat = 1; beat < 4; ++beat) {
                    float beat_x = bx + (beat * (bar_len_sec * 0.25f) * time_to_px);
                    draw->AddLine(ImVec2(beat_x, grid_p0.y + ruler_h - 6.0f), ImVec2(beat_x, grid_p0.y + ruler_h), IM_COL32(25, 45, 65, 180), 1.0f);
                }

                int step = (playlist_zoom_x < 25.0f) ? 4 : (playlist_zoom_x < 45.0f) ? 2 : 1;
                if (bar % step == 0) {
                    char bar_str[16];
                    snprintf(bar_str, sizeof(bar_str), "Bar %d", bar + 1);
                    draw->AddText(ImVec2(bx + 4.0f, grid_p0.y + 6.0f), IM_COL32(160, 195, 230, 255), bar_str);
                }
            }

            for (const auto& marker : ::timeline.section_markers) {
                float mx = grid_start_x + (marker.time_sec * time_to_px) - playlist_scroll_x;
                if (mx < grid_start_x - 80.0f || mx > grid_start_x + grid_view_w + 80.0f) continue;

                ImVec2 tag_p0(mx, grid_p0.y + 4.0f);
                ImVec2 tag_p1(mx + 68.0f, grid_p0.y + ruler_h - 4.0f);

                ImU32 tag_col = IM_COL32(0, 212, 255, 230);
                if (marker.name.find("DROP") != std::string::npos) tag_col = IM_COL32(255, 200, 0, 230);
                else if (marker.name.find("BUILD") != std::string::npos) tag_col = IM_COL32(180, 0, 255, 230);
                else if (marker.name.find("BREAK") != std::string::npos) tag_col = IM_COL32(0, 255, 160, 230);

                draw->AddRectFilled(tag_p0, tag_p1, tag_col, 3.0f);
                draw->AddText(ImVec2(tag_p0.x + 4.0f, tag_p0.y + 2.0f), IM_COL32(0, 0, 0, 255), marker.name.c_str());
            }

            draw->PopClipRect();

            ImGuiIO& io = ImGui::GetIO();
            if (is_hovered && ImGui::IsMouseHoveringRect(ImVec2(grid_start_x, grid_p0.y), ImVec2(grid_start_x + grid_view_w, grid_p0.y + ruler_h))) {
                if (ImGui::IsMouseClicked(0)) {
                    is_scrubbing_ruler = true;
                }
                if (ImGui::IsMouseClicked(1)) {
                    is_dragging_loop = true;
                    float click_t = (io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px;
                    ::timeline.loop_start_sec = (std::max)(0.0f, click_t);
                    ::timeline.loop_end_sec = ::timeline.loop_start_sec + bar_len_sec * 4.0f;
                    ::timeline.loop_enabled = true;
                }
            }
            if (is_scrubbing_ruler) {
                if (io.MouseDown[0]) {
                    float seek_t = (io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px;
                    ::timeline.setMasterFrame((uint64_t)((std::max)(0.0f, seek_t) * 44100.0f));
                } else {
                    is_scrubbing_ruler = false;
                }
            }
            if (is_dragging_loop) {
                if (io.MouseDown[1]) {
                    float drag_t = (io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px;
                    ::timeline.loop_end_sec = (std::max)(::timeline.loop_start_sec + 0.1f, drag_t);
                } else {
                    is_dragging_loop = false;
                }
            }
        }

        template<typename FSnap>
        void renderTracksAndClips(ImDrawList* draw, ImVec2 grid_p0, float view_w, float view_h, float ruler_h, float trk_h, float time_to_px, float bar_len_sec, float snap_sec, FSnap snap_time, bool is_hovered) {
            float grid_start_x = grid_p0.x + track_card_w;
            float grid_view_w = view_w - track_card_w;
            float grid_view_h = view_h - ruler_h;

            static const ImU32 trk_palette[] = {
                IM_COL32(0, 245, 212, 255),   // 01. Kick (Cyan Neon)
                IM_COL32(157, 78, 221, 255),  // 02. Sub Bass (Purple)
                IM_COL32(255, 0, 110, 255),   // 03. Mid Saw Bass (Pink)
                IM_COL32(0, 187, 249, 255),   // 04. Snare / Clap (Sky Blue)
                IM_COL32(254, 228, 64, 255),  // 05. Hi-Hats Open (Gold)
                IM_COL32(181, 228, 140, 255), // 06. Closed Hats (Lime)
                IM_COL32(82, 182, 154, 255),  // 07. Percussion (Emerald)
                IM_COL32(72, 202, 228, 255),  // 08. Squelch / Zaps (Ice Blue)
                IM_COL32(0, 150, 199, 255),   // 09. Acid 303 (Deep Blue)
                IM_COL32(114, 9, 183, 255),   // 10. Arp / Pluck (Violet)
                IM_COL32(247, 37, 133, 255),  // 11. SuperSaw Lead (Hot Magenta)
                IM_COL32(243, 114, 44, 255),  // 12. Dark Drone (Orange)
                IM_COL32(249, 199, 79, 255),  // 13. Strings / Pads (Yellow)
                IM_COL32(144, 190, 109, 255), // 14. Piano (Sage)
                IM_COL32(67, 170, 139, 255),  // 15. Vocals Mantra (Teal)
                IM_COL32(249, 65, 68, 255),   // 16. FX Snare Roll (Coral)
                IM_COL32(0, 255, 200, 255),   // 17. Auto Cutoff (Mint)
                IM_COL32(0, 212, 255, 255),   // 18. Auto Reverb (Cyan)
                IM_COL32(255, 0, 255, 255),   // 19. Auto Pitch Riser (Magenta)
                IM_COL32(120, 140, 160, 255)  // 20. Extra
            };

            int visible_tracks = MAX_TRACKS;
            int total_lanes_to_draw = (std::max)(visible_tracks, (int)std::ceil((grid_view_h + playlist_scroll_y) / trk_h) + 2);
            float track_start_y = grid_p0.y + ruler_h + 2.0f - playlist_scroll_y;

            ImGuiIO& io = ImGui::GetIO();

            draw->PushClipRect(ImVec2(grid_start_x, grid_p0.y + ruler_h), ImVec2(grid_start_x + grid_view_w, grid_p0.y + view_h), true);

            int total_bars = (int)std::ceil((grid_view_w + playlist_scroll_x) / (bar_len_sec * time_to_px)) + 16;
            for (int bar = 0; bar < total_bars; ++bar) {
                float bar_time = bar * bar_len_sec;
                float bx = grid_start_x + (bar_time * time_to_px) - playlist_scroll_x;
                if (bx < grid_start_x - 10.0f || bx > grid_start_x + grid_view_w + 10.0f) continue;

                draw->AddLine(ImVec2(bx, grid_p0.y + ruler_h), ImVec2(bx, grid_p0.y + view_h), IM_COL32(22, 34, 50, 220), 1.0f);

                for (int beat = 1; beat < 4; ++beat) {
                    float beat_x = bx + (beat * (bar_len_sec * 0.25f) * time_to_px);
                    draw->AddLine(ImVec2(beat_x, grid_p0.y + ruler_h), ImVec2(beat_x, grid_p0.y + view_h), IM_COL32(14, 22, 32, 140), 1.0f);
                }
            }

            draw->PopClipRect();

            draw->PushClipRect(ImVec2(grid_p0.x, grid_p0.y + ruler_h), ImVec2(grid_p0.x + view_w, grid_p0.y + view_h), true);

            for (int t = 0; t < total_lanes_to_draw; ++t) {
                float ty0 = track_start_y + t * trk_h;
                float ty1 = ty0 + trk_h - 4.0f;

                if (ty1 < grid_p0.y + ruler_h || ty0 > grid_p0.y + view_h) continue;

                if (t % 2 == 1) {
                    draw->AddRectFilled(ImVec2(grid_start_x, ty0), ImVec2(grid_start_x + grid_view_w, ty1), IM_COL32(11, 16, 24, 160));
                }
                draw->AddLine(ImVec2(grid_start_x, ty1), ImVec2(grid_start_x + grid_view_w, ty1), IM_COL32(18, 28, 42, 255), 1.0f);

                if (t >= visible_tracks) {
                    draw->AddRectFilled(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1), IM_COL32(10, 15, 22, 220), 6.0f);
                    draw->AddRect(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1), IM_COL32(18, 26, 36, 180), 6.0f);
                    std::string extra_lbl = "Track " + std::to_string(t + 1);
                    draw->AddText(ImVec2(grid_p0.x + 14.0f, ty0 + 10.0f), IM_COL32(100, 125, 150, 180), extra_lbl.c_str());
                    continue;
                }

                // Card do Cabeçalho da Faixa
                bool is_sel = (t == active_drag_track);
                ImU32 card_bg = is_sel ? IM_COL32(22, 34, 48, 255) : IM_COL32(14, 20, 28, 255);
                draw->AddRectFilled(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1), card_bg, 6.0f);
                draw->AddRect(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1), is_sel ? IM_COL32(0, 229, 255, 230) : IM_COL32(24, 38, 54, 255), 6.0f);

                draw->AddRectFilled(ImVec2(grid_p0.x, ty0 + 2.0f), ImVec2(grid_p0.x + 4.0f, ty1 - 2.0f), trk_palette[t], 3.0f);

                char num_str[8];
                snprintf(num_str, sizeof(num_str), "%02d", t + 1);
                draw->AddRectFilled(ImVec2(grid_p0.x + 8.0f, ty0 + 8.0f), ImVec2(grid_p0.x + 30.0f, ty0 + 26.0f), IM_COL32(20, 28, 40, 255), 4.0f);
                draw->AddText(ImVec2(grid_p0.x + 11.0f, ty0 + 9.0f), IM_COL32(130, 160, 195, 255), num_str);

                std::string display_name = (t < MAX_TRACKS && !::track_names[t].empty()) ? ::track_names[t] : ("Track " + std::to_string(t + 1));
                bool has_sample = (t < MAX_TRACKS && g_piano_synth.getDrumSample(t).loaded);
                if (has_sample && display_name.find("[SMP]") == std::string::npos) {
                    display_name = "[SMP] " + display_name;
                }

                std::string clipped_name = display_name;
                if (clipped_name.length() > 20) clipped_name = clipped_name.substr(0, 18) + "..";

                draw->AddText(ImVec2(grid_p0.x + 36.0f, ty0 + 8.0f), has_sample ? IM_COL32(0, 240, 255, 255) : IM_COL32(230, 242, 255, 255), clipped_name.c_str());

                if (has_sample) {
                    int r_note = g_piano_synth.sampler_settings[t].root_note;
                    char root_str[16];
                    snprintf(root_str, sizeof(root_str), "R:%d", r_note);
                    draw->AddText(ImVec2(grid_p0.x + track_card_w - 76.0f, ty0 + 8.0f), IM_COL32(0, 200, 255, 220), root_str);
                }

                // Fader de Volume Horizontal Embutido
                float fader_x0 = grid_p0.x + 36.0f;
                float fader_x1 = grid_p0.x + track_card_w - 70.0f;
                float fader_y = ty0 + 34.0f;
                float cur_vol = (t < MAX_TRACKS) ? ::track_linear_volumes[t] : 0.8f;
                cur_vol = (std::clamp)(cur_vol, 0.0f, 1.0f);

                draw->AddRectFilled(ImVec2(fader_x0, fader_y - 2.0f), ImVec2(fader_x1, fader_y + 2.0f), IM_COL32(18, 26, 36, 255), 2.0f);
                float thumb_x = fader_x0 + (fader_x1 - fader_x0) * cur_vol;
                draw->AddRectFilled(ImVec2(fader_x0, fader_y - 2.0f), ImVec2(thumb_x, fader_y + 2.0f), IM_COL32(0, 229, 255, 240), 2.0f);
                draw->AddRectFilled(ImVec2(thumb_x - 4.0f, fader_y - 7.0f), ImVec2(thumb_x + 4.0f, fader_y + 7.0f), IM_COL32(0, 245, 255, 255), 3.0f);
                draw->AddRect(ImVec2(thumb_x - 4.0f, fader_y - 7.0f), ImVec2(thumb_x + 4.0f, fader_y + 7.0f), IM_COL32(255, 255, 255, 200), 3.0f);

                ImVec2 fader_hit_p0(fader_x0 - 6.0f, fader_y - 10.0f);
                ImVec2 fader_hit_p1(fader_x1 + 6.0f, fader_y + 10.0f);
                bool hover_fader = is_hovered && ImGui::IsMouseHoveringRect(fader_hit_p0, fader_hit_p1);
                if (hover_fader && ImGui::IsMouseClicked(0)) {
                    is_dragging_fader = true;
                    active_fader_track = t;
                }
                if (is_dragging_fader && active_fader_track == t) {
                    if (io.MouseDown[0]) {
                        float new_v = (io.MousePos.x - fader_x0) / (fader_x1 - fader_x0);
                        if (t < MAX_TRACKS) ::track_linear_volumes[t] = (std::clamp)(new_v, 0.0f, 1.0f);
                    } else {
                        is_dragging_fader = false;
                        active_fader_track = -1;
                    }
                }

                // Botões M e S
                float btn_m_x = grid_p0.x + track_card_w - 58.0f;
                float btn_s_x = grid_p0.x + track_card_w - 30.0f;
                float btn_y = ty0 + 26.0f;

                bool is_muted = (t < MAX_TRACKS) ? ::track_mutes[t] : false;
                bool is_solo = (t < MAX_TRACKS) ? ::track_solos[t] : false;

                ImVec2 btn_m_p0(btn_m_x, btn_y);
                ImVec2 btn_m_p1(btn_m_x + 22.0f, btn_y + 20.0f);
                bool hover_m = is_hovered && ImGui::IsMouseHoveringRect(btn_m_p0, btn_m_p1);
                draw->AddRectFilled(btn_m_p0, btn_m_p1, is_muted ? IM_COL32(230, 40, 40, 255) : hover_m ? IM_COL32(32, 46, 62, 255) : IM_COL32(18, 25, 35, 255), 4.0f);
                draw->AddRect(btn_m_p0, btn_m_p1, is_muted ? IM_COL32(255, 100, 100, 255) : IM_COL32(34, 48, 66, 255), 4.0f);
                draw->AddText(ImVec2(btn_m_x + 6.0f, btn_y + 3.0f), is_muted ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 205, 230, 255), "M");
                if (hover_m && ImGui::IsMouseClicked(0)) {
                    if (t < MAX_TRACKS) ::track_mutes[t] = !::track_mutes[t];
                }

                ImVec2 btn_s_p0(btn_s_x, btn_y);
                ImVec2 btn_s_p1(btn_s_x + 22.0f, btn_y + 20.0f);
                bool hover_s = is_hovered && ImGui::IsMouseHoveringRect(btn_s_p0, btn_s_p1);
                draw->AddRectFilled(btn_s_p0, btn_s_p1, is_solo ? IM_COL32(250, 195, 0, 255) : hover_s ? IM_COL32(32, 46, 62, 255) : IM_COL32(18, 25, 35, 255), 4.0f);
                draw->AddRect(btn_s_p0, btn_s_p1, is_solo ? IM_COL32(255, 230, 100, 255) : IM_COL32(34, 48, 66, 255), 4.0f);
                draw->AddText(ImVec2(btn_s_x + 6.0f, btn_y + 3.0f), is_solo ? IM_COL32(0, 0, 0, 255) : IM_COL32(180, 205, 230, 255), "S");
                if (hover_s && ImGui::IsMouseClicked(0)) {
                    if (t < MAX_TRACKS) ::track_solos[t] = !::track_solos[t];
                }

                if (ImGui::BeginDragDropTargetCustom(ImRect(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1)), ImGui::GetID(("TrackHeaderDND_" + std::to_string(t)).c_str()))) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SAMPLE_PATH")) {
                        const char* dropped_path = (const char*)payload->Data;
                        if (t < MAX_TRACKS) {
                            g_piano_synth.loadTrackSample(t, dropped_path);
                            ::track_names[t] = "[SMP] " + std::filesystem::path(dropped_path).filename().string();
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImVec2 head_hit_p0(grid_p0.x, ty0);
                ImVec2 head_hit_p1(grid_p0.x + track_card_w - 65.0f, ty0 + 26.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(head_hit_p0, head_hit_p1)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        rename_track_target = t;
                        strncpy(rename_buffer, (t < MAX_TRACKS) ? ::track_names[t].c_str() : "", sizeof(rename_buffer) - 1);
                        ImGui::OpenPopup("RenameTrackModal");
                    }
                    if (ImGui::IsMouseClicked(1)) {
                        active_drag_track = t;
                        ImGui::OpenPopup(("TrackContextMenu_" + std::to_string(t)).c_str());
                    }
                }

                if (ImGui::BeginPopup(("TrackContextMenu_" + std::to_string(t)).c_str())) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "CONFIGURACOES DA FAIXA %d", t + 1);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Renomear Faixa...")) {
                        rename_track_target = t;
                        strncpy(rename_buffer, (t < MAX_TRACKS) ? ::track_names[t].c_str() : "", sizeof(rename_buffer) - 1);
                        ImGui::OpenPopup("RenameTrackModal");
                    }
                    if (ImGui::MenuItem("Carregar Sample WAV (Sampler Cromatico)...")) {
                        std::string chosen = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3;*.flac)\0*.wav;*.mp3;*.flac\0");
                        if (!chosen.empty() && t < MAX_TRACKS) {
                            g_piano_synth.loadTrackSample(t, chosen.c_str());
                            ::track_names[t] = "[SMP] " + std::filesystem::path(chosen).filename().string();
                        }
                    }
                    if (t < MAX_TRACKS && g_piano_synth.getDrumSample(t).loaded) {
                        if (ImGui::BeginMenu("Nota Raiz do Sampler (Root)")) {
                            int cur_root = g_piano_synth.sampler_settings[t].root_note;
                            if (ImGui::MenuItem("F#1 (30 - Psytrance Sub)", nullptr, cur_root == 30)) g_piano_synth.sampler_settings[t].root_note = 30;
                            if (ImGui::MenuItem("G1 (31 - Prog Trance)", nullptr, cur_root == 31)) g_piano_synth.sampler_settings[t].root_note = 31;
                            if (ImGui::MenuItem("A1 (33 - Dark Psy)", nullptr, cur_root == 33)) g_piano_synth.sampler_settings[t].root_note = 33;
                            if (ImGui::MenuItem("C2 (36 - Bass Standard)", nullptr, cur_root == 36)) g_piano_synth.sampler_settings[t].root_note = 36;
                            if (ImGui::MenuItem("C3 (48 - Synth Standard)", nullptr, cur_root == 48)) g_piano_synth.sampler_settings[t].root_note = 48;
                            if (ImGui::MenuItem("C5 (72 - High Lead)", nullptr, cur_root == 72)) g_piano_synth.sampler_settings[t].root_note = 72;
                            ImGui::EndMenu();
                        }
                        if (ImGui::MenuItem("Remover Sample da Faixa")) {
                            g_piano_synth.getDrumSample(t).loaded = false;
                            g_piano_synth.getDrumSample(t).sample_data.clear();
                        }
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Abrir no Piano Roll")) {
                        g_clip_manager.current_pattern_idx = (t < (int)g_clip_manager.global_patterns.size()) ? t : 0;
                        show_piano_roll = true;
                    }
                    ImGui::EndPopup();
                }

                renderClipsForTrack(draw, t, ty0, ty1, grid_start_x, grid_view_w, time_to_px, bar_len_sec, trk_palette[t], snap_sec, snap_time, is_hovered);
            }

            draw->PopClipRect();

            if (ImGui::BeginPopupModal("RenameTrackModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Novo Nome para a Faixa %d:", rename_track_target + 1);
                ImGui::InputText("##NewTrackNameInput", rename_buffer, sizeof(rename_buffer));
                if (ImGui::Button("Salvar", ImVec2(90, 24))) {
                    if (rename_track_target >= 0 && rename_track_target < MAX_TRACKS && strlen(rename_buffer) > 0) {
                        ::track_names[rename_track_target] = rename_buffer;
                    }
                    rename_track_target = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancelar", ImVec2(90, 24))) {
                    rename_track_target = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (is_hovered && ImGui::IsMouseClicked(0) && !is_dragging_clip && !is_resizing_right && !is_resizing_left && !is_slicing && !is_scrubbing_ruler && !is_dragging_loop && !is_dragging_fader) {
                if (io.MousePos.x > grid_start_x && io.MousePos.x < grid_start_x + grid_view_w && io.MousePos.y > grid_p0.y + ruler_h && io.MousePos.y < grid_p0.y + view_h) {
                    int clicked_track = (int)((io.MousePos.y - (grid_p0.y + ruler_h + 2.0f - playlist_scroll_y)) / trk_h);
                    if (clicked_track >= 0 && clicked_track < MAX_TRACKS) {
                        float click_time = snap_time((io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px);
                        if (current_tool == TOOL_DRAW || current_tool == TOOL_PAINT) {
                            MidiClip mc;
                            mc.id = g_clip_manager.next_id++;
                            mc.start_time_sec = click_time;
                            mc.length_sec = bar_len_sec * 4.0f;
                            mc.pattern_id = g_clip_manager.current_pattern_idx;
                            mc.name = "Pattern " + std::to_string(mc.pattern_id + 1);
                            g_clip_manager.track_midi_clips[clicked_track].push_back(mc);
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                show_piano_roll = true;
                            }
                        }
                    }
                }
            }
        }

        template<typename FSnap>
        void renderClipsForTrack(ImDrawList* draw, int t, float ty0, float ty1, float grid_start_x, float grid_view_w, float time_to_px, float bar_len_sec, ImU32 trk_color, float snap_sec, FSnap snap_time, bool is_hovered) {
            ImGuiIO& io = ImGui::GetIO();
            float trk_h = ty1 - ty0;

            // ── 1. CLIPS DE AUDIO COM FORMA DE ONDA ESTEREO REAL (WAVEFORMS) ──
            auto& a_clips = g_clip_manager.track_clips[t];
            for (int i = 0; i < (int)a_clips.size(); ++i) {
                auto& ac = a_clips[i];
                float cx0 = grid_start_x + (ac.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (ac.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                // Container com cantos arredondados e borda neon suave
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(14, 20, 32, 245), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Header do clipe de áudio
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(10, 15, 24, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(220, 245, 255, 255), ac.name.c_str());

                // Configuração da Forma de Onda Estéreo (Dual Lane: L canal superior, R canal inferior)
                float wave_top = ty0 + 17.0f;
                float wave_bot = ty1 - 3.0f;
                float wave_h = wave_bot - wave_top;
                float mid_y = wave_top + wave_h * 0.5f;
                float cl_l = wave_top + wave_h * 0.25f; // Linha central Canal L
                float cl_r = wave_top + wave_h * 0.75f; // Linha central Canal R

                // Linha divisória de canais e baselines sutis
                draw->AddLine(ImVec2(draw_x0, mid_y), ImVec2(draw_x1, mid_y), IM_COL32(35, 55, 80, 160), 1.0f);
                draw->AddLine(ImVec2(draw_x0, cl_l), ImVec2(draw_x1, cl_l), IM_COL32(0, 200, 255, 45), 1.0f);
                draw->AddLine(ImVec2(draw_x0, cl_r), ImVec2(draw_x1, cl_r), IM_COL32(180, 110, 255, 45), 1.0f);

                const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->getStemBuffer(t) : nullptr;
                auto& ds = g_piano_synth.getDrumSample(t);

                float step_px = 2.0f;
                float sr = 44100.0f;
                float beat_sec = bar_len_sec * 0.25f;

                for (float px = draw_x0; px < draw_x1; px += step_px) {
                    float clip_t = ((px - cx0) / time_to_px) + ac.source_offset_sec;
                    float amp_l = 0.0f;
                    float amp_r = 0.0f;

                    if (ds.loaded && !ds.sample_data.empty() && ds.total_frames > 0) {
                        float sample_dur = (float)ds.total_frames / (float)ds.sample_rate;
                        float loop_clip_t = (sample_dur < beat_sec) ? fmodf(clip_t, beat_sec) : fmodf(clip_t, sample_dur);
                        size_t frame_idx = (size_t)(loop_clip_t * (float)ds.sample_rate);
                        if (frame_idx < (size_t)ds.total_frames) {
                            if (ds.channels >= 2) {
                                amp_l = std::abs(ds.sample_data[frame_idx * ds.channels]);
                                amp_r = std::abs(ds.sample_data[frame_idx * ds.channels + 1]);
                            } else {
                                amp_l = amp_r = std::abs(ds.sample_data[frame_idx]);
                            }
                        }
                    } else if (p_stem_buf && !p_stem_buf->empty()) {
                        size_t s_idx = (size_t)(clip_t * sr);
                        if (s_idx < p_stem_buf->size()) {
                            amp_l = amp_r = std::abs((*p_stem_buf)[s_idx]);
                        }
                    } else {
                        // Modelagem de onda estéreo densa, contínua e orgânica (estilo WAV master real de DAW)
                        float beat_dur = bar_len_sec * 0.25f;
                        float sixteenth = beat_dur * 0.25f;
                        float t_in_beat = fmodf(clip_t, beat_dur);
                        float t_in_16 = fmodf(clip_t, sixteenth);

                        float transient = std::exp(-t_in_16 * 28.0f);
                        float beat_punch = std::exp(-t_in_beat * 10.0f);
                        float base_body = 0.42f + 0.26f * std::sin(clip_t * 3.14159f / bar_len_sec);
                        float osc = std::sin(clip_t * 640.0f) * 0.22f + std::sin(clip_t * 1380.0f) * 0.16f;

                        float raw_amp = base_body + beat_punch * 0.38f + transient * 0.22f + std::abs(osc);
                        amp_l = std::clamp(raw_amp, 0.08f, 0.94f);
                        amp_r = std::clamp(raw_amp * 0.92f + std::sin(clip_t * 320.0f) * 0.04f, 0.08f, 0.94f);
                    }

                    amp_l = std::clamp(amp_l, 0.06f, 0.95f);
                    amp_r = std::clamp(amp_r, 0.06f, 0.95f);

                    float max_h = wave_h * 0.22f;
                    float h_l = amp_l * max_h;
                    float h_r = amp_r * max_h;

                    // Cores estéreo: Gradiente Roxo Neon -> Ciano Elétrico (idêntico à referência do usuário)
                    float norm_pos = std::clamp((px - cx0) / (cx1 - cx0), 0.0f, 1.0f);
                    int r_val = (int)(150.0f * (1.0f - norm_pos * 0.85f));
                    int g_val = (int)(110.0f + norm_pos * 135.0f);
                    int b_val = 255;
                    ImU32 col_l = (t == 0) ? IM_COL32(r_val, g_val, b_val, 245) : IM_COL32(0, 235, 255, 245);
                    ImU32 col_r = (t == 0) ? IM_COL32(r_val, (int)(g_val * 0.92f), b_val, 245) : IM_COL32(0, 225, 250, 245);

                    // Desenha simetricamente em torno do baseline de cada canal
                    draw->AddLine(ImVec2(px, cl_l - h_l), ImVec2(px, cl_l + h_l), col_l, 1.8f);
                    draw->AddLine(ImVec2(px, cl_r - h_r), ImVec2(px, cl_r + h_r), col_r, 1.8f);
                }

                ImVec2 hit_p0(draw_x0, ty0 + 2.0f);
                ImVec2 hit_p1(draw_x1, ty1 - 2.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(hit_p0, hit_p1)) {
                    if (current_tool == TOOL_DELETE && ImGui::IsMouseClicked(0)) {
                        a_clips.erase(a_clips.begin() + i);
                        --i;
                        continue;
                    }
                    if (current_tool == TOOL_SLICE && ImGui::IsMouseClicked(0)) {
                        float cut_time = snap_time((io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px);
                        if (cut_time > ac.start_time_sec + 0.05f && cut_time < ac.start_time_sec + ac.length_sec - 0.05f) {
                            AudioClip ac2 = ac;
                            float first_len = cut_time - ac.start_time_sec;
                            ac2.start_time_sec = cut_time;
                            ac2.length_sec = ac.length_sec - first_len;
                            ac2.source_offset_sec = ac.source_offset_sec + first_len;
                            ac.length_sec = first_len;
                            a_clips.insert(a_clips.begin() + i + 1, ac2);
                            break;
                        }
                    }
                }
            }

            // ── 2. CLIPS DE PADRAO MIDI COM NOTAS VISIVEIS EM NEON CIANO ──
            auto& m_clips = g_clip_manager.track_midi_clips[t];
            for (int i = 0; i < (int)m_clips.size(); ++i) {
                auto& mc = m_clips[i];
                float cx0 = grid_start_x + (mc.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (mc.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                // Container estilo Modern DAW: Fundo roxo profundo com contorno neon
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(26, 18, 44, 245), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Header do clipe MIDI
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(16, 12, 28, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(245, 230, 255, 255), mc.name.c_str());

                const Pattern* p = nullptr;
                for (const auto& pat : g_clip_manager.global_patterns) {
                    if (pat.id == mc.pattern_id) { p = &pat; break; }
                }
                if (!p && mc.pattern_id >= 0 && mc.pattern_id < (int)g_clip_manager.global_patterns.size()) {
                    p = &g_clip_manager.global_patterns[mc.pattern_id];
                }

                if (p) {
                    std::vector<int> target_channels;
                    if (t >= 0 && t < MAX_TRACKS && !p->getChannelNotes(t).empty()) {
                        target_channels.push_back(t);
                    } else {
                        for (int ch = 0; ch < MAX_TRACKS; ch++) {
                            if (!p->getChannelNotes(ch).empty()) target_channels.push_back(ch);
                        }
                    }

                    float pat_len = 0.0f;
                    int min_pitch = 127;
                    int max_pitch = 0;
                    for (int ch : target_channels) {
                        for (const auto& n : p->getChannelNotes(ch)) {
                            if (n.start_time + n.duration > pat_len) pat_len = n.start_time + n.duration;
                            if (n.pitch < min_pitch) min_pitch = n.pitch;
                            if (n.pitch > max_pitch) max_pitch = n.pitch;
                        }
                    }
                    if (pat_len <= 0.001f) pat_len = bar_len_sec * 4.0f;
                    float bars = std::ceil(pat_len / bar_len_sec);
                    if (bars < 1.0f) bars = 1.0f;
                    pat_len = bars * bar_len_sec;

                    // Garante uma extensao de afinacao confortavel (pelo menos 1 oitava)
                    int pitch_span = max_pitch - min_pitch;
                    if (pitch_span < 12) {
                        int mid_p = (min_pitch + max_pitch) / 2;
                        min_pitch = mid_p - 6;
                        max_pitch = mid_p + 6;
                    }

                    float grid_inner_y0 = ty0 + 17.0f;
                    float grid_inner_h = (trk_h - 22.0f);

                    for (float loop_start = 0.0f; loop_start < mc.length_sec; loop_start += pat_len) {
                        // Linha divisoria vertical suave de cada compasso dentro do clipe
                        for (float bar_div = loop_start; bar_div < loop_start + pat_len && bar_div < mc.length_sec; bar_div += bar_len_sec) {
                            if (bar_div > 0.0f) {
                                float div_x = cx0 + (bar_div * time_to_px);
                                if (div_x >= draw_x0 && div_x <= draw_x1) {
                                    draw->AddLine(ImVec2(div_x, ty0 + 17.0f), ImVec2(div_x, ty1 - 2.0f), IM_COL32(255, 255, 255, 22), 1.0f);
                                }
                            }
                        }

                        for (int ch : target_channels) {
                            for (const auto& note : p->getChannelNotes(ch)) {
                                float note_start_rel = loop_start + note.start_time;
                                if (note_start_rel >= mc.length_sec) continue;
                                float note_dur_rel = note.duration;
                                if (note_start_rel + note_dur_rel > mc.length_sec) {
                                    note_dur_rel = mc.length_sec - note_start_rel;
                                }

                                float note_x0 = cx0 + (note_start_rel * time_to_px);
                                float note_x1 = note_x0 + (note_dur_rel * time_to_px);
                                if (note_x1 - note_x0 < 4.0f) note_x1 = note_x0 + 4.0f;

                                if (note_x1 < draw_x0 || note_x0 > draw_x1) continue;

                                float n_draw_x0 = (std::max)(note_x0, draw_x0);
                                float n_draw_x1 = (std::min)(note_x1, draw_x1);

                                float pitch_norm = (float)(note.pitch - min_pitch) / (float)(max_pitch - min_pitch);
                                pitch_norm = std::clamp(pitch_norm, 0.06f, 0.94f);
                                float note_y = (ty1 - 6.0f) - pitch_norm * (grid_inner_h - 8.0f);

                                // NOTAS MIDI BRILHANTES EM CIANO NEON COM BRILHO AURA E NUCLEO BRANCO
                                // 1. Brilho Aura Ciano
                                draw->AddRectFilled(ImVec2(n_draw_x0 - 0.5f, note_y - 3.0f), ImVec2(n_draw_x1 + 0.5f, note_y + 3.0f), IM_COL32(0, 220, 255, 65), 2.5f);
                                // 2. Corpo Neon Ciano
                                draw->AddRectFilled(ImVec2(n_draw_x0, note_y - 2.0f), ImVec2(n_draw_x1, note_y + 2.0f), IM_COL32(0, 240, 255, 245), 2.0f);
                                // 3. Nucleo Branco Puro
                                draw->AddLine(ImVec2(n_draw_x0 + 1.0f, note_y), ImVec2(n_draw_x1 - 1.0f, note_y), IM_COL32(240, 255, 255, 255), 1.2f);
                            }
                        }
                    }
                }

                ImVec2 hit_p0(draw_x0, ty0 + 2.0f);
                ImVec2 hit_p1(draw_x1, ty1 - 2.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(hit_p0, hit_p1)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        g_clip_manager.current_pattern_idx = mc.pattern_id;
                        show_piano_roll = true;
                    }
                    if (current_tool == TOOL_DELETE && ImGui::IsMouseClicked(0)) {
                        m_clips.erase(m_clips.begin() + i);
                        --i;
                        continue;
                    }
                }
            }

            // 3. CLIPS DE AUTOMACAO COM CURVAS BEZIER E GRADIENTE
            auto& auto_clips = g_clip_manager.track_auto_clips[t];
            for (int i = 0; i < (int)auto_clips.size(); ++i) {
                auto& ac = auto_clips[i];
                float cx0 = grid_start_x + (ac.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (ac.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(10, 24, 32, 235), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), ac.color, 5.0f);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(8, 18, 26, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(180, 240, 255, 255), ac.name.c_str());

                float auto_inner_y0 = ty0 + 18.0f;
                float auto_inner_h = (trk_h - 22.0f);

                if (ac.points.size() >= 2) {
                    for (size_t pt = 0; pt < ac.points.size() - 1; ++pt) {
                        auto& p0 = ac.points[pt];
                        auto& p1 = ac.points[pt + 1];

                        float px0 = cx0 + (p0.time_rel_sec * time_to_px);
                        float px1 = cx0 + (p1.time_rel_sec * time_to_px);
                        float py0 = (ty1 - 4.0f) - p0.value * auto_inner_h;
                        float py1 = (ty1 - 4.0f) - p1.value * auto_inner_h;

                        float cp_x = (px0 + px1) * 0.5f;
                        float cp_y = (py0 + py1) * 0.5f + p0.tension * 30.0f;

                        int segs = 16;
                        for (int s = 0; s < segs; ++s) {
                            float t_a = (float)s / (float)segs;
                            float t_b = (float)(s + 1) / (float)segs;

                            float xa = (1.0f - t_a) * (1.0f - t_a) * px0 + 2.0f * (1.0f - t_a) * t_a * cp_x + t_a * t_a * px1;
                            float ya = (1.0f - t_a) * (1.0f - t_a) * py0 + 2.0f * (1.0f - t_a) * t_a * cp_y + t_a * t_a * py1;
                            float xb = (1.0f - t_b) * (1.0f - t_b) * px0 + 2.0f * (1.0f - t_b) * t_b * cp_x + t_b * t_b * px1;
                            float yb = (1.0f - t_b) * (1.0f - t_b) * py0 + 2.0f * (1.0f - t_b) * t_b * cp_y + t_b * t_b * py1;

                            if (xb >= draw_x0 && xa <= draw_x1) {
                                draw->AddLine(ImVec2(xa, ya), ImVec2(xb, yb), ac.color, 2.0f);
                                draw->AddLine(ImVec2(xa, ya), ImVec2(xa, ty1 - 3.0f), IM_COL32(0, 229, 255, 20), 1.0f);
                            }
                        }

                        if (px0 >= draw_x0 && px0 <= draw_x1) {
                            draw->AddCircleFilled(ImVec2(px0, py0), 3.5f, IM_COL32(255, 255, 255, 255));
                            draw->AddCircle(ImVec2(px0, py0), 4.5f, ac.color, 12, 1.5f);
                        }
                    }
                    auto& plast = ac.points.back();
                    float plast_x = cx0 + (plast.time_rel_sec * time_to_px);
                    float plast_y = (ty1 - 4.0f) - plast.value * auto_inner_h;
                    if (plast_x >= draw_x0 && plast_x <= draw_x1) {
                        draw->AddCircleFilled(ImVec2(plast_x, plast_y), 3.5f, IM_COL32(255, 255, 255, 255));
                        draw->AddCircle(ImVec2(plast_x, plast_y), 4.5f, ac.color, 12, 1.5f);
                    }
                }
            }
        }
        void renderLaserPlayhead(ImDrawList* draw, ImVec2 grid_p0, float view_w, float view_h, float ruler_h, float time_to_px) {
            float grid_start_x = grid_p0.x + track_card_w;
            float grid_view_w = view_w - track_card_w;

            float cur_time = (float)::timeline.getMasterFrame() / 44100.0f;
            float playhead_x = grid_start_x + (cur_time * time_to_px) - playlist_scroll_x;

            if (playhead_x >= grid_start_x && playhead_x <= grid_start_x + grid_view_w) {
                draw->PushClipRect(ImVec2(grid_start_x, grid_p0.y), ImVec2(grid_start_x + grid_view_w, grid_p0.y + view_h), true);

                draw->AddLine(ImVec2(playhead_x, grid_p0.y + ruler_h), ImVec2(playhead_x, grid_p0.y + view_h), IM_COL32(0, 229, 255, 60), 4.0f);
                draw->AddLine(ImVec2(playhead_x, grid_p0.y + ruler_h), ImVec2(playhead_x, grid_p0.y + view_h), IM_COL32(0, 245, 255, 240), 1.5f);

                ImVec2 tri_top(playhead_x, grid_p0.y + ruler_h);
                ImVec2 tri_left(playhead_x - 7.0f, grid_p0.y + 6.0f);
                ImVec2 tri_right(playhead_x + 7.0f, grid_p0.y + 6.0f);

                draw->AddTriangleFilled(tri_left, tri_right, tri_top, IM_COL32(0, 240, 255, 255));
                draw->AddTriangle(tri_left, tri_right, tri_top, IM_COL32(255, 255, 255, 255), 1.0f);

                draw->PopClipRect();
            }
        }

        void renderScrollbars(ImVec2 grid_p0, float view_w, float view_h, float ruler_h, float grid_total_w, float trk_h) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImGuiIO& io = ImGui::GetIO();

            float grid_start_x = grid_p0.x + track_card_w;
            float grid_view_w = view_w - track_card_w;
            float grid_view_h = view_h - ruler_h;

            float h_bar_y = grid_p0.y + view_h - 10.0f;
            float h_bar_w = grid_view_w - 14.0f;
            draw->AddRectFilled(ImVec2(grid_start_x, h_bar_y), ImVec2(grid_start_x + h_bar_w, h_bar_y + 8.0f), IM_COL32(14, 20, 28, 255), 4.0f);

            float h_ratio = (std::clamp)(grid_view_w / (grid_total_w + 1.0f), 0.08f, 1.0f);
            float h_thumb_w = (std::max)(35.0f, h_bar_w * h_ratio);
            float max_scroll_x = (std::max)(1.0f, grid_total_w - grid_view_w);
            float h_thumb_x = grid_start_x + (playlist_scroll_x / max_scroll_x) * (h_bar_w - h_thumb_w);

            ImVec2 h_thumb_p0(h_thumb_x, h_bar_y);
            ImVec2 h_thumb_p1(h_thumb_x + h_thumb_w, h_bar_y + 8.0f);
            bool hover_h_thumb = ImGui::IsMouseHoveringRect(h_thumb_p0, h_thumb_p1);
            draw->AddRectFilled(h_thumb_p0, h_thumb_p1, hover_h_thumb ? IM_COL32(0, 200, 235, 255) : IM_COL32(0, 160, 200, 200), 4.0f);

            if (ImGui::IsMouseHoveringRect(ImVec2(grid_start_x, h_bar_y), ImVec2(grid_start_x + h_bar_w, h_bar_y + 8.0f)) && io.MouseDown[0]) {
                float click_ratio = (io.MousePos.x - grid_start_x) / h_bar_w;
                playlist_scroll_x = (std::clamp)(click_ratio * max_scroll_x, 0.0f, max_scroll_x);
            }

            float v_bar_x = grid_p0.x + view_w - 10.0f;
            float v_bar_h = grid_view_h - 14.0f;
            float total_content_h = MAX_TRACKS * trk_h;
            draw->AddRectFilled(ImVec2(v_bar_x, grid_p0.y + ruler_h), ImVec2(v_bar_x + 8.0f, grid_p0.y + ruler_h + v_bar_h), IM_COL32(14, 20, 28, 255), 4.0f);

            float v_ratio = (std::clamp)(grid_view_h / (total_content_h + 1.0f), 0.10f, 1.0f);
            float v_thumb_h = (std::max)(35.0f, v_bar_h * v_ratio);
            float max_scroll_y = (std::max)(1.0f, total_content_h - grid_view_h);
            float v_thumb_y = grid_p0.y + ruler_h + (playlist_scroll_y / max_scroll_y) * (v_bar_h - v_thumb_h);

            ImVec2 v_thumb_p0(v_bar_x, v_thumb_y);
            ImVec2 v_thumb_p1(v_bar_x + 8.0f, v_thumb_y + v_thumb_h);
            bool hover_v_thumb = ImGui::IsMouseHoveringRect(v_thumb_p0, v_thumb_p1);
            draw->AddRectFilled(v_thumb_p0, v_thumb_p1, hover_v_thumb ? IM_COL32(0, 200, 235, 255) : IM_COL32(0, 160, 200, 200), 4.0f);

            if (ImGui::IsMouseHoveringRect(ImVec2(v_bar_x, grid_p0.y + ruler_h), ImVec2(v_bar_x + 8.0f, grid_p0.y + ruler_h + v_bar_h)) && io.MouseDown[0]) {
                float click_ratio = (io.MousePos.y - (grid_p0.y + ruler_h)) / v_bar_h;
                playlist_scroll_y = (std::clamp)(click_ratio * max_scroll_y, 0.0f, max_scroll_y);
            }
        }

        void renderSubModals() {
            if (show_psy_arranger_modal) {
                ImGui::OpenPopup("PsySongArrangerModal");
            }
            if (ImGui::BeginPopupModal("PsySongArrangerModal", &show_psy_arranger_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
                static KuroArranger::PsyArrangerConfig cfg;
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), "PSYTRANCE SONG STRUCTURE GENERATOR & ARRANGER");
                ImGui::TextDisabled("Gera automaticamente a estrutura completa de uma faixa na Playlist com clipes multi-track e automacoes");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Subgenero / Estilo:");
                ImGui::SameLine();
                const char* subgenres[] = { "Progressive Psy (138 BPM)", "Full-On Psy (142 BPM)", "Twilight / Dark Psy (148 BPM)" };
                if (ImGui::Combo("##ArrangerSubgenre", &cfg.subgenre, subgenres, IM_ARRAYSIZE(subgenres))) {
                    if (cfg.subgenre == 0) { cfg.bpm = 138.0f; cfg.root_note = 31; }
                    else if (cfg.subgenre == 1) { cfg.bpm = 142.0f; cfg.root_note = 30; }
                    else if (cfg.subgenre == 2) { cfg.bpm = 148.0f; cfg.root_note = 33; }
                }

                ImGui::Text("Tom da Faixa (Root Key):");
                ImGui::SameLine();
                const char* keys[] = { "F# Menor (F#1 - 46Hz)", "G Menor (G1 - 49Hz)", "A Menor (A1 - 55Hz)", "D Menor (D1 - 37Hz)", "E Menor (E1 - 41Hz)" };
                int key_idx = 0;
                if (cfg.root_note == 30) key_idx = 0;
                else if (cfg.root_note == 31) key_idx = 1;
                else if (cfg.root_note == 33) key_idx = 2;
                else if (cfg.root_note == 26) key_idx = 3;
                else if (cfg.root_note == 28) key_idx = 4;
                if (ImGui::Combo("##ArrangerKey", &key_idx, keys, IM_ARRAYSIZE(keys))) {
                    if (key_idx == 0) cfg.root_note = 30;
                    else if (key_idx == 1) cfg.root_note = 31;
                    else if (key_idx == 2) cfg.root_note = 33;
                    else if (key_idx == 3) cfg.root_note = 26;
                    else if (key_idx == 4) cfg.root_note = 28;
                }

                ImGui::SliderFloat("BPM / Andamento", &cfg.bpm, 130.0f, 152.0f, "%.1f BPM");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Secoes do Arranjo (Compassos):");
                ImGui::Spacing();

                ImGui::Checkbox("Intro", &cfg.has_intro); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##IntroBars", &cfg.intro_bars, 8, 16);
                ImGui::Checkbox("Build-up 1", &cfg.has_buildup1); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##Buildup1Bars", &cfg.buildup1_bars, 4, 8);
                ImGui::Checkbox("Drop 1 (Full-On)", &cfg.has_drop1); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##Drop1Bars", &cfg.drop1_bars, 16, 32);
                ImGui::Checkbox("Breakdown / Chill", &cfg.has_break); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##BreakBars", &cfg.break_bars, 8, 16);
                ImGui::Checkbox("Build-up 2 (Climax)", &cfg.has_buildup2); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##Buildup2Bars", &cfg.buildup2_bars, 4, 8);
                ImGui::Checkbox("Drop 2 (Peak Energy)", &cfg.has_drop2); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##Drop2Bars", &cfg.drop2_bars, 16, 32);
                ImGui::Checkbox("Outro", &cfg.has_outro); ImGui::SameLine(200); ImGui::SetNextItemWidth(100); ImGui::InputInt("##OutroBars", &cfg.outro_bars, 8, 16);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Pistas de Automacao & Amostras:");
                ImGui::Checkbox("Gerar Automacao de Filter Cutoff Sweep", &cfg.enable_filter_sweep);
                ImGui::Checkbox("Gerar Automacao de Reverb Wet Washout", &cfg.enable_reverb_washout);
                ImGui::Checkbox("Gerar Automacao de Pitch Riser (+12 semitons)", &cfg.enable_pitch_riser);
                ImGui::Checkbox("Carregar Amostras do Pack Sonicspore PRYZMA", &cfg.use_pryzma_samples);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 1.0f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button("GERAR ARRANJO COMPLETO NA PLAYLIST", ImVec2(320, 36))) {
                    KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
                    playlist_zoom_x = 28.0f;
                    playlist_scroll_x = 0.0f;
                    show_psy_arranger_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
                if (ImGui::Button("Cancelar", ImVec2(90, 36))) {
                    show_psy_arranger_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (show_timeline_inpainting_modal) {
                ImGui::OpenPopup("TimelineInpaintingModal");
            }
            if (ImGui::BeginPopupModal("TimelineInpaintingModal", &show_timeline_inpainting_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "TIMELINE ARRANGEMENT INPAINTING & GENERATIVE SUITE");
                ImGui::TextDisabled("Gera variacoes generativas de arranjo e transicoes no trecho selecionado");
                ImGui::Separator();
                ImGui::Spacing();

                static int tl_mode = 0;
                static float tl_energy = 80.0f;
                const char* tl_modes[] = { "Build-up Rush (Snare Roll & Pitch Climb)", "Drop Energy Mutation (Explosive Variation)", "Filter Transition Sweep (Smooth Washout)" };
                ImGui::Combo("Modo de Arranjo", &tl_mode, tl_modes, IM_ARRAYSIZE(tl_modes));

                ImGui::SliderFloat("Intensidade da Transicao", &tl_energy, 1.0f, 100.0f, "%.0f%%");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("GERAR INPAINTING NO ARRANJO", ImVec2(230, 32))) {
                    show_timeline_inpainting_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancelar", ImVec2(90, 32))) {
                    show_timeline_inpainting_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    };

    inline KuroPlaylistUI g_playlist_ui;
}