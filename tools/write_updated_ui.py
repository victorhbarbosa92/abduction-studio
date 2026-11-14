import os

target_file = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroPsytranceRollingBassUI.h"

content = r"""#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "KuroRollingBassTextureManager.h"
#include "../plugin_manager/ThematicSynths.h"
#include "../core/ClipManager.h"
#include "../core/TimelineManager.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroUI {

    extern bool show_piano_roll;
    extern bool g_need_focus_piano_roll;

    class KuroPsytranceRollingBassUI {
    public:
        bool is_open = false;

        enum LayerTab {
            TAB_FULL_BASE = 0,
            TAB_KICK = 1,
            TAB_BASS = 2,
            TAB_SNARE = 3
        };
        int current_layer_tab = TAB_BASS;

        struct BassPreset {
            const char* name;
            int synth_model;   // 0=Moog 24dB, 1=Virus TI, 2=Nord 303, 3=FM Sub, 4=Analog Monster
            int root_note_idx; // 0=C1 ... 6=F#1
            int scale_idx;     // 0=Phrygian, 1=Natural Minor, 2=Dorian, 3=Harmonic Minor, 4=Locrian, 5=Major, 6=Gypsy
            int pattern_type;  // 0=KBBB, 1=KBB, 2=KB, 3=Octave Jump
            int osc_type;      // 0=Saw, 1=Square, 2=Sub-Sine, 3=Morph
            float phase_deg;
            float cutoff_hz;
            float resonance;
            float decay_sec;
            float env_amt;
            float drive;
            float sub_lvl;
            float click;
            float pitch_depth; // Semitons (0 a 36)
            float pitch_decay; // Segundos (0.001 a 0.025)
            float hpf_hz;      // Low-Cut Hz (15 a 80)
        };

        struct KickPreset {
            const char* name;
            float start_hz;
            float end_hz;
            float pitch_decay_ms;
            float amp_decay_ms;
            float click;
            float drive;
        };

        struct SnarePreset {
            const char* name;
            float tone_hz;
            float noise_decay_ms;
            float snap;
            float vol;
        };

        static const int NUM_STEPS = 16;
        static inline const char* const root_note_labels[12] = { "C1 (36)", "C#1 (37)", "D1 (38)", "D#1 (39)", "E1 (40)", "F1 (41)", "F#1 (42)", "G1 (43)", "G#1 (44)", "A1 (45)", "A#1 (46)", "B1 (47)" };
        static inline const char* const root_note_short[12] = { "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1" };
        static inline const char* const scale_names[7] = { "Phrygian", "Natural Minor", "Dorian", "Harmonic Minor", "Locrian", "Major", "Gypsy/Flamenco" };
        static inline const int scale_intervals[7][8] = {
            { 0, 1, 3, 5, 7, 8, 10, 12 }, // Phrygian
            { 0, 2, 3, 5, 7, 8, 10, 12 }, // Natural Minor
            { 0, 2, 3, 5, 7, 9, 10, 12 }, // Dorian
            { 0, 2, 3, 5, 7, 8, 11, 12 }, // Harmonic Minor
            { 0, 1, 3, 5, 6, 8, 10, 12 }, // Locrian
            { 0, 2, 4, 5, 7, 9, 11, 12 }, // Major
            { 0, 1, 4, 5, 7, 8, 11, 12 }  // Gypsy
        };

        static inline const char* const synth_model_names[5] = {
            "Kuro Moog 24dB Ladder",
            "Virus TI Hyper-Saw",
            "Nord Lead Punch 303",
            "FM Sub-Rolling Bass",
            "Analog Monster Sub"
        };
        static inline const char* const synth_model_short[5] = {
            "Moog 24dB",
            "Virus TI",
            "Nord Acid",
            "FM Bass",
            "Analog Sub"
        };

        static inline const BassPreset bass_presets[6] = {
            { "138 BPM F# Full-On Rolling KBBB", 0, 6, 0, 0, 0, 90.0f, 480.0f, 0.42f, 0.082f, 0.88f, 0.35f, 0.40f, 0.45f, 18.0f, 0.004f, 32.0f },
            { "140 BPM G Progressive KBB Galop", 1, 7, 1, 1, 0, 60.0f, 620.0f, 0.45f, 0.095f, 0.80f, 0.40f, 0.35f, 0.50f, 16.0f, 0.005f, 30.0f },
            { "142 BPM E Dark Psy Rolling KBBB", 2, 4, 0, 0, 1, 90.0f, 380.0f, 0.55f, 0.075f, 0.92f, 0.50f, 0.45f, 0.60f, 22.0f, 0.003f, 35.0f },
            { "138 BPM A Offbeat Classic KB",    3, 9, 1, 2, 2, 90.0f, 550.0f, 0.35f, 0.110f, 0.75f, 0.30f, 0.50f, 0.40f, 14.0f, 0.006f, 28.0f },
            { "145 BPM D# Hi-Tech Octave Jump",  1, 3, 3, 3, 0, 45.0f, 750.0f, 0.48f, 0.070f, 0.95f, 0.45f, 0.38f, 0.55f, 24.0f, 0.003f, 34.0f },
            { "136 BPM D Tribal Phrygian Roll",  4, 2, 0, 0, 0, 90.0f, 420.0f, 0.40f, 0.090f, 0.82f, 0.38f, 0.42f, 0.45f, 18.0f, 0.004f, 30.0f }
        };

        static inline const KickPreset kick_presets[5] = {
            { "138 BPM Psy Punch (Hard Hit)", 185.0f, 50.0f, 36.0f, 220.0f, 0.80f, 0.60f },
            { "140 BPM Full-On Click & Sub",  210.0f, 48.0f, 32.0f, 200.0f, 0.88f, 0.70f },
            { "142 BPM Darkpsy Heavy Punch",  240.0f, 45.0f, 28.0f, 180.0f, 0.95f, 0.80f },
            { "136 BPM Progressive Deep Body",160.0f, 52.0f, 44.0f, 260.0f, 0.70f, 0.45f },
            { "145 BPM Hi-Tech Laser Kick",   260.0f, 44.0f, 24.0f, 170.0f, 0.90f, 0.75f }
        };

        static inline const SnarePreset snare_presets[4] = {
            { "Psy 909 Tight Snare", 210.0f, 135.0f, 0.85f, 0.80f },
            { "Crunchy Noise Clap",  180.0f, 160.0f, 0.90f, 0.85f },
            { "Laser Zap Snare",     260.0f, 110.0f, 0.95f, 0.78f },
            { "Tribal Wood Snap",    160.0f, 90.0f,  0.75f, 0.75f }
        };

        // Pistas do Step Sequencer:
        // 1. Rolling Bass (KBBB padrão)
        bool step_active[NUM_STEPS] = {
            false, true, true, true,   // Beat 1 (Kick, B, B, B)
            false, true, true, true,   // Beat 2
            false, true, true, true,   // Beat 3
            false, true, true, true    // Beat 4
        };
        float step_velocity[NUM_STEPS] = {
            0.45f, 0.95f, 0.78f, 0.85f,
            0.45f, 0.92f, 0.75f, 0.82f,
            0.45f, 0.95f, 0.78f, 0.85f,
            0.45f, 0.92f, 0.82f, 0.88f
        };
        int step_pitch_offset[NUM_STEPS] = { 0 };

        // 2. Kick Drum (4-on-the-floor: passos 0, 4, 8, 12)
        bool kick_step_active[NUM_STEPS] = {
            true, false, false, false,
            true, false, false, false,
            true, false, false, false,
            true, false, false, false
        };
        float kick_step_velocity[NUM_STEPS] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.96f, 0.0f, 0.0f, 0.0f,
            0.98f, 0.0f, 0.0f, 0.0f,
            0.96f, 0.0f, 0.0f, 0.0f
        };

        // 3. Snare / Clap (Tempos 2 e 4: passos 4 e 12)
        bool snare_step_active[NUM_STEPS] = {
            false, false, false, false,
            true, false, false, false,
            false, false, false, false,
            true, false, false, false
        };
        float snare_step_velocity[NUM_STEPS] = {
            0.0f, 0.0f, 0.0f, 0.0f,
            0.92f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.95f, 0.0f, 0.0f, 0.0f
        };

        int current_synth_model = 0;
        int current_preset_idx = 0;
        int current_kick_preset_idx = 0;
        int current_snare_preset_idx = 0;
        int current_root_idx = 6;  // F#1 (MIDI 42)
        int current_scale_idx = 0; // Phrygian
        int current_pattern_type = 0; // 0=KBBB, 1=KBB, 2=KB, 3=Octave Jump
        int current_bars_to_generate = 4; // 1, 2, 4, 8 compassos

        std::string toast_message = "";
        float toast_timer = 0.0f;

        // Estado do Sequenciador de Audition Loop em Tempo Real
        bool is_auditioning = false;
        float audition_timer = 0.0f;
        int audition_current_step = -1;
        int audition_active_note_id = -1;

        KuroDSP::PsytranceRollingBassSynth* synth_dsp = nullptr;

    public:
        KuroPsytranceRollingBassUI() {
            applyPreset(0);
            applyKickPreset(0);
            applySnarePreset(0);
        }

        void stopAudition() {
            if (is_auditioning) {
                is_auditioning = false;
                if (synth_dsp) {
                    synth_dsp->pushMidiEvent({ -999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f });
                    synth_dsp->stopAllDrums();
                }
                audition_current_step = -1;
                audition_active_note_id = -1;
                audition_timer = 0.0f;
            }
        }

        void setSynthDsp(KuroDSP::PsytranceRollingBassSynth* dsp) {
            synth_dsp = dsp;
            if (synth_dsp) {
                applyPreset(current_preset_idx);
                applyKickPreset(current_kick_preset_idx);
                applySnarePreset(current_snare_preset_idx);
            }
        }

        void applyPatternType(int p_type) {
            current_pattern_type = p_type;
            for (int i = 0; i < NUM_STEPS; ++i) {
                int beat_step = i % 4;
                step_pitch_offset[i] = 0;

                if (p_type == 0) { // KBBB Full-on 16th
                    step_active[i] = (beat_step != 0);
                    step_velocity[i] = (beat_step == 1) ? 0.95f : (beat_step == 2) ? 0.78f : (beat_step == 3) ? 0.85f : 0.45f;
                } else if (p_type == 1) { // KBB Galop
                    step_active[i] = (beat_step == 2 || beat_step == 3);
                    step_velocity[i] = (beat_step == 2) ? 0.92f : (beat_step == 3) ? 0.80f : 0.40f;
                } else if (p_type == 2) { // KB Offbeat
                    step_active[i] = (beat_step == 2);
                    step_velocity[i] = (beat_step == 2) ? 0.95f : 0.40f;
                } else if (p_type == 3) { // Octave Jump
                    step_active[i] = (beat_step != 0);
                    step_velocity[i] = (beat_step == 1) ? 0.95f : (beat_step == 2) ? 0.78f : 0.85f;
                    if (beat_step == 3) step_pitch_offset[i] = 12;
                }
            }
        }

        void applyPreset(int idx) {
            if (idx < 0 || idx >= 6) return;
            current_preset_idx = idx;
            const auto& p = bass_presets[idx];
            current_synth_model = p.synth_model;
            current_pattern_type = p.pattern_type;
            applyPatternType(p.pattern_type);

            if (synth_dsp) {
                synth_dsp->setSynthModel(p.synth_model);
                synth_dsp->setOscType(p.osc_type);
                synth_dsp->setPhaseRetrigger(p.phase_deg);
                synth_dsp->setCutoff(p.cutoff_hz);
                synth_dsp->setResonance(p.resonance);
                synth_dsp->setEnvDecay(p.decay_sec);
                synth_dsp->setEnvAmount(p.env_amt);
                synth_dsp->setDrive(p.drive);
                synth_dsp->setSubLevel(p.sub_lvl);
                synth_dsp->setTransientClick(p.click);
                synth_dsp->setPitchDepth(p.pitch_depth);
                synth_dsp->setPitchDecay(p.pitch_decay);
                synth_dsp->setHpfCutoff(p.hpf_hz);
            }
        }

        void applyKickPreset(int idx) {
            if (idx < 0 || idx >= 5) return;
            current_kick_preset_idx = idx;
            const auto& p = kick_presets[idx];
            if (synth_dsp) {
                synth_dsp->setKickStartFreq(p.start_hz);
                synth_dsp->setKickEndFreq(p.end_hz);
                synth_dsp->setKickPitchDecay(p.pitch_decay_ms);
                synth_dsp->setKickAmpDecay(p.amp_decay_ms);
                synth_dsp->setKickClick(p.click);
                synth_dsp->setKickDrive(p.drive);
            }
        }

        void applySnarePreset(int idx) {
            if (idx < 0 || idx >= 4) return;
            current_snare_preset_idx = idx;
            const auto& p = snare_presets[idx];
            if (synth_dsp) {
                synth_dsp->setSnareTone(p.tone_hz);
                synth_dsp->setSnareNoiseDecay(p.noise_decay_ms);
                synth_dsp->setSnareSnap(p.snap);
                synth_dsp->setSnareVolume(p.vol);
            }
        }

        // ── GERADORES DE NOTAS MIDI PARA PISTAS SEPARADAS ──
        std::vector<KuroDSP::MidiNote> generateBassNotes(float bpm, int num_bars) {
            std::vector<KuroDSP::MidiNote> generated;
            float beat_sec = 60.0f / (bpm > 20.0f ? bpm : 138.0f);
            float step_sec = beat_sec * 0.25f;
            float bar_sec = beat_sec * 4.0f;
            int base_midi_pitch = 36 + current_root_idx;

            for (int b = 0; b < num_bars; ++b) {
                float bar_offset = b * bar_sec;
                for (int s = 0; s < NUM_STEPS; ++s) {
                    if (!step_active[s]) continue;

                    KuroDSP::MidiNote n;
                    n.pitch = base_midi_pitch + step_pitch_offset[s];
                    n.start_time = bar_offset + s * step_sec;
                    n.duration = step_sec * 0.85f;
                    n.velocity = std::clamp(step_velocity[s], 0.1f, 1.0f);
                    n.is_selected = false;
                    generated.push_back(n);
                }
            }
            return generated;
        }

        std::vector<KuroDSP::MidiNote> generateKickNotes(float bpm, int num_bars) {
            std::vector<KuroDSP::MidiNote> generated;
            float beat_sec = 60.0f / (bpm > 20.0f ? bpm : 138.0f);
            float step_sec = beat_sec * 0.25f;
            float bar_sec = beat_sec * 4.0f;

            for (int b = 0; b < num_bars; ++b) {
                float bar_offset = b * bar_sec;
                for (int s = 0; s < NUM_STEPS; ++s) {
                    if (!kick_step_active[s]) continue;

                    KuroDSP::MidiNote n;
                    n.pitch = 36; // C2 Kick
                    n.start_time = bar_offset + s * step_sec;
                    n.duration = step_sec * 0.95f;
                    n.velocity = std::clamp(kick_step_velocity[s], 0.1f, 1.0f);
                    n.is_selected = false;
                    generated.push_back(n);
                }
            }
            return generated;
        }

        std::vector<KuroDSP::MidiNote> generateSnareNotes(float bpm, int num_bars) {
            std::vector<KuroDSP::MidiNote> generated;
            float beat_sec = 60.0f / (bpm > 20.0f ? bpm : 138.0f);
            float step_sec = beat_sec * 0.25f;
            float bar_sec = beat_sec * 4.0f;

            for (int b = 0; b < num_bars; ++b) {
                float bar_offset = b * bar_sec;
                for (int s = 0; s < NUM_STEPS; ++s) {
                    if (!snare_step_active[s]) continue;

                    KuroDSP::MidiNote n;
                    n.pitch = 38; // D2 Snare
                    n.start_time = bar_offset + s * step_sec;
                    n.duration = step_sec * 0.90f;
                    n.velocity = std::clamp(snare_step_velocity[s], 0.1f, 1.0f);
                    n.is_selected = false;
                    generated.push_back(n);
                }
            }
            return generated;
        }

        std::vector<KuroDSP::MidiNote> generateMidiNotes(float bpm, int num_bars) {
            return generateBassNotes(bpm, num_bars);
        }

    private:
        // ── WIDGETS CUSTOMIZADOS ──
        void DrawKuroLogo(ImDrawList* dl, ImVec2 pos, float size, ImU32 col) {
            float s = size;
            dl->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x, pos.y + s), col, 3.5f);
            dl->AddLine(ImVec2(pos.x + s * 0.4f, pos.y + s * 0.5f), ImVec2(pos.x + s * 0.95f, pos.y + s * 0.05f), col, 3.5f);
            dl->AddLine(ImVec2(pos.x + s * 0.4f, pos.y + s * 0.5f), ImVec2(pos.x + s * 0.95f, pos.y + s * 0.95f), col, 3.5f);
            dl->AddRect(ImVec2(pos.x - 3, pos.y - 3), ImVec2(pos.x + s + 3, pos.y + s + 3), (col & 0x00FFFFFF) | 0x44000000, 3.0f, 0, 1.0f);
        }

        bool DrawLayerTab(const char* label, ImVec2 pos, ImVec2 size, bool is_selected, ImU32 active_color) {
            ImGui::SetCursorScreenPos(pos);
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImGuiID btn_id = window->GetID(label);
            ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, btn_id)) return false;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, btn_id, &hovered, &held);

            ImDrawList* dl = window->DrawList;
            ImU32 bg_col = is_selected ? ((active_color & 0x00FFFFFF) | 0x33000000) : (hovered ? IM_COL32(20, 28, 38, 255) : IM_COL32(10, 14, 20, 255));
            ImU32 border_col = is_selected ? active_color : (hovered ? IM_COL32(0, 180, 210, 180) : IM_COL32(28, 42, 56, 255));
            ImU32 text_col = is_selected ? IM_COL32(255, 255, 255, 255) : (hovered ? IM_COL32(200, 230, 250, 255) : IM_COL32(130, 160, 185, 255));

            dl->AddRectFilled(bb.Min, bb.Max, bg_col, 4.0f);
            dl->AddRect(bb.Min, bb.Max, border_col, 4.0f, 0, is_selected ? 1.8f : 1.0f);

            if (is_selected) {
                dl->AddLine(ImVec2(bb.Min.x + 3, bb.Max.y - 1), ImVec2(bb.Max.x - 3, bb.Max.y - 1), active_color, 2.5f);
            }

            ImVec2 text_sz = ImGui::CalcTextSize(label);
            ImVec2 text_pos(bb.Min.x + (size.x - text_sz.x) * 0.5f, bb.Min.y + (size.y - text_sz.y) * 0.5f);
            dl->AddText(text_pos, text_col, label);

            return pressed;
        }

        bool DrawWaveCircleBtn(const char* id, ImDrawList* dl, ImVec2 center, float radius, int wave_type, bool is_selected) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImGuiID btn_id = window->GetID(id);
            ImRect bb(ImVec2(center.x - radius, center.y - radius), ImVec2(center.x + radius, center.y + radius));
            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, btn_id)) return false;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, btn_id, &hovered, &held);

            ImU32 bg_col = is_selected ? IM_COL32(14, 34, 46, 255) : (hovered ? IM_COL32(20, 28, 38, 255) : IM_COL32(12, 16, 22, 255));
            ImU32 ring_col = is_selected ? IM_COL32(0, 240, 255, 255) : (hovered ? IM_COL32(0, 180, 210, 200) : IM_COL32(32, 50, 68, 255));
            ImU32 icon_col = is_selected ? IM_COL32(0, 255, 255, 255) : (hovered ? IM_COL32(180, 220, 240, 255) : IM_COL32(130, 160, 185, 255));

            dl->AddCircleFilled(center, radius, bg_col, 28);
            dl->AddCircle(center, radius, ring_col, 28, is_selected ? 2.5f : 1.2f);

            if (is_selected) {
                dl->AddCircle(center, radius + 2.5f, IM_COL32(0, 229, 255, 80), 28, 2.0f);
            }

            float hw = radius * 0.52f;
            float hh = radius * 0.42f;
            if (wave_type == 0) { // Saw
                dl->AddLine(ImVec2(center.x - hw, center.y + hh), ImVec2(center.x, center.y - hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x, center.y - hh), ImVec2(center.x, center.y + hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x, center.y + hh), ImVec2(center.x + hw, center.y - hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x + hw, center.y - hh), ImVec2(center.x + hw, center.y + hh), icon_col, 2.2f);
            } else if (wave_type == 1) { // Square
                dl->AddLine(ImVec2(center.x - hw, center.y + hh), ImVec2(center.x - hw, center.y - hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x - hw, center.y - hh), ImVec2(center.x, center.y - hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x, center.y - hh), ImVec2(center.x, center.y + hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x, center.y + hh), ImVec2(center.x + hw, center.y + hh), icon_col, 2.2f);
                dl->AddLine(ImVec2(center.x + hw, center.y + hh), ImVec2(center.x + hw, center.y - hh), icon_col, 2.2f);
            } else if (wave_type == 2) { // Sine
                const int npts = 16;
                ImVec2 s_pts[npts];
                for (int i = 0; i < npts; ++i) {
                    float t = (float)i / (float)(npts - 1);
                    float x = center.x - hw + t * (hw * 2.0f);
                    float y = center.y - std::sin(t * (float)M_PI * 2.0f) * hh;
                    s_pts[i] = ImVec2(x, y);
                }
                for (int i = 0; i < npts - 1; ++i) {
                    dl->AddLine(s_pts[i], s_pts[i + 1], icon_col, 2.2f);
                }
            }
            return pressed;
        }

        bool DrawPhotorealisticPhaseDial(const char* id, ImDrawList* dl, ImTextureID tex, ImVec2 center, float radius, float* val_deg) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImGuiID dial_id = window->GetID(id);
            float total_r = radius + 22.0f;
            ImRect bb(ImVec2(center.x - total_r, center.y - total_r), ImVec2(center.x + total_r, center.y + total_r + 14.0f));
            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, dial_id)) return false;

            bool hovered, held;
            bool changed = false;
            ImGui::ButtonBehavior(bb, dial_id, &hovered, &held);

            if (held) {
                float dx = ImGui::GetIO().MousePos.x - center.x;
                float dy = ImGui::GetIO().MousePos.y - center.y;
                float angle_rad = std::atan2(dy, dx) + 0.5f * (float)M_PI;
                if (angle_rad < 0.0f) angle_rad += 2.0f * (float)M_PI;
                *val_deg = angle_rad * 180.0f / (float)M_PI;
                if (*val_deg < 0.0f) *val_deg = 0.0f;
                if (*val_deg > 360.0f) *val_deg = 360.0f;
                changed = true;
            }

            static const int grad_degrees[] = { 0, 30, 90, 120, 180, 210, 270, 300, 360 };
            for (int g = 0; g < 9; ++g) {
                int deg = grad_degrees[g];
                float rad = (deg - 90) * (float)M_PI / 180.0f;
                float r_tick_inner = radius + 4.0f;
                float r_tick_outer = radius + 8.0f;
                float r_text = radius + 15.0f;

                ImVec2 pt_in(center.x + std::cos(rad) * r_tick_inner, center.y + std::sin(rad) * r_tick_inner);
                ImVec2 pt_out(center.x + std::cos(rad) * r_tick_outer, center.y + std::sin(rad) * r_tick_outer);
                dl->AddLine(pt_in, pt_out, IM_COL32(0, 180, 200, 160), 1.0f);

                char lbl[8];
                snprintf(lbl, sizeof(lbl), "%d", deg);
                ImVec2 text_sz = ImGui::CalcTextSize(lbl);
                ImVec2 pt_txt(center.x + std::cos(rad) * r_text - text_sz.x * 0.5f, center.y + std::sin(rad) * r_text - text_sz.y * 0.5f);
                dl->AddText(pt_txt, IM_COL32(110, 150, 175, 200), lbl);
            }

            dl->AddCircleFilled(center, radius, IM_COL32(16, 22, 28, 255), 32);
            dl->AddCircle(center, radius, IM_COL32(28, 44, 58, 255), 32, 1.5f);
            dl->AddCircleFilled(center, radius * 0.85f, IM_COL32(24, 32, 40, 255), 32);
            dl->AddCircle(center, radius * 0.85f, IM_COL32(40, 58, 75, 255), 32, 1.0f);

            float cur_rad_end = (*val_deg - 90.0f) * (float)M_PI / 180.0f;
            dl->PathArcTo(center, radius - 2.0f, -0.5f * (float)M_PI, cur_rad_end, 32);
            dl->PathStroke(IM_COL32(0, 229, 255, 255), 0, 3.0f);

            ImVec2 pt_pointer(center.x + std::cos(cur_rad_end) * (radius * 0.72f), center.y + std::sin(cur_rad_end) * (radius * 0.72f));
            dl->AddLine(center, pt_pointer, IM_COL32(0, 240, 255, 255), 2.5f);
            dl->AddCircleFilled(pt_pointer, 3.0f, IM_COL32(255, 255, 255, 255));

            return changed;
        }

        bool DrawPhotorealisticRotatingKnob(const char* id, ImDrawList* dl, ImTextureID tex, ImVec2 center, float radius, float* val, float min_v, float max_v, const char* label, const char* format_str, ImU32 accent_col = IM_COL32(0, 229, 255, 255)) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImGuiID knob_id = window->GetID(id);
            float h_extra = 26.0f;
            ImRect bb(ImVec2(center.x - radius, center.y - radius), ImVec2(center.x + radius, center.y + radius + h_extra));
            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, knob_id)) return false;

            bool hovered, held;
            bool changed = false;
            ImGui::ButtonBehavior(bb, knob_id, &hovered, &held);

            if (held) {
                float delta = ImGui::GetIO().MouseDelta.y;
                float step = (max_v - min_v) / 140.0f;
                *val -= delta * step;
                if (*val < min_v) *val = min_v;
                if (*val > max_v) *val = max_v;
                changed = true;
            }

            float norm = (*val - min_v) / (max_v - min_v);
            norm = std::clamp(norm, 0.0f, 1.0f);

            float angle_min = 0.75f * (float)M_PI;
            float angle_max = 2.25f * (float)M_PI;
            float angle_cur = angle_min + norm * (angle_max - angle_min);

            // Halo e arco de fundo
            dl->PathArcTo(center, radius + 2.0f, angle_min, angle_max, 24);
            dl->PathStroke(IM_COL32(18, 28, 38, 255), 0, 2.5f);

            // Arco ativo neon com a cor de destaque do instrumento
            dl->PathArcTo(center, radius + 2.0f, angle_min, angle_cur, 24);
            dl->PathStroke(accent_col, 0, 2.5f);

            dl->AddCircleFilled(center, radius, IM_COL32(18, 24, 30, 255), 24);
            dl->AddCircle(center, radius, IM_COL32(36, 50, 64, 255), 24, 1.2f);
            dl->AddCircleFilled(center, radius * 0.85f, IM_COL32(26, 36, 46, 255), 24);

            float ptr_r = radius * 0.70f;
            ImVec2 ptr_pt(center.x + std::cos(angle_cur) * ptr_r, center.y + std::sin(angle_cur) * ptr_r);
            dl->AddLine(center, ptr_pt, accent_col, 2.0f);
            dl->AddCircleFilled(ptr_pt, 2.2f, IM_COL32(255, 255, 255, 255));

            if (label) {
                ImVec2 lbl_sz = ImGui::CalcTextSize(label);
                ImVec2 lbl_pos(center.x - lbl_sz.x * 0.5f, center.y + radius + 4.0f);
                dl->AddText(lbl_pos, IM_COL32(160, 195, 220, 255), label);
            }

            char val_str[32];
            snprintf(val_str, sizeof(val_str), format_str, *val);
            ImVec2 val_sz = ImGui::CalcTextSize(val_str);
            ImVec2 val_pos(center.x - val_sz.x * 0.5f, center.y + radius + 15.0f);
            dl->AddText(val_pos, (accent_col & 0x00FFFFFF) | 0xCC000000, val_str);

            if (hovered) {
                ImGui::SetTooltip("%s: %s", label ? label : "", val_str);
            }

            return changed;
        }

        bool DrawPatternBtn(const char* label, ImVec2 pos, ImVec2 size, bool is_selected) {
            ImGui::SetCursorScreenPos(pos);
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImGuiID btn_id = window->GetID(label);
            ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, btn_id)) return false;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, btn_id, &hovered, &held);

            ImDrawList* dl = window->DrawList;
            ImU32 bg_col = is_selected ? IM_COL32(10, 32, 44, 255) : (hovered ? IM_COL32(18, 26, 35, 255) : IM_COL32(12, 16, 22, 255));
            ImU32 border_col = is_selected ? IM_COL32(0, 229, 255, 255) : (hovered ? IM_COL32(0, 180, 200, 180) : IM_COL32(24, 38, 50, 255));
            ImU32 text_col = is_selected ? IM_COL32(0, 240, 255, 255) : IM_COL32(160, 190, 215, 255);

            dl->AddRectFilled(bb.Min, bb.Max, bg_col, 5.0f);
            dl->AddRect(bb.Min, bb.Max, border_col, 5.0f, 0, is_selected ? 1.8f : 1.0f);

            if (is_selected) {
                dl->AddRect(ImVec2(bb.Min.x - 1, bb.Min.y - 1), ImVec2(bb.Max.x + 1, bb.Max.y + 1), IM_COL32(0, 229, 255, 50), 6.0f, 0, 1.5f);
            }

            ImVec2 text_sz = ImGui::CalcTextSize(label);
            ImVec2 text_pos(bb.Min.x + (size.x - text_sz.x) * 0.5f, bb.Min.y + (size.y - text_sz.y) * 0.5f);
            dl->AddText(text_pos, text_col, label);

            return pressed;
        }

        // ── MATRIZ MULTI-LANE 16 PASSOS (KICK + ROLLING BASS + SNARE/CLAP) ──
        void DrawMultiLane16StepMatrix(ImDrawList* dl, ImVec2 pos, ImVec2 size) {
            float step_w = size.x / 16.0f;
            float header_h = 16.0f;
            float row1_y = pos.y + header_h;
            float row1_h = 24.0f; // Kick lane
            float row2_y = row1_y + row1_h + 16.0f;
            float row2_h = size.y - (row1_h + 16.0f + 24.0f + 16.0f + header_h); // Bass velocity bars (~170px)
            float row3_y = row2_y + row2_h + 16.0f;
            float row3_h = 24.0f; // Snare lane

            ImGuiIO& io = ImGui::GetIO();
            ImVec2 mouse = io.MousePos;

            dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(pos.x + 4.0f, pos.y), IM_COL32(255, 150, 40, 220), "🥊 KICK [4x4]");
            dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(pos.x + 4.0f, row2_y - 13.0f), IM_COL32(0, 229, 255, 220), "⚡ ROLLING BASS [16th]");
            dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(pos.x + 4.0f, row3_y - 13.0f), IM_COL32(230, 70, 200, 220), "💥 SNARE / CLAP [2&4]");

            for (int s = 0; s < NUM_STEPS; ++s) {
                float sx = pos.x + s * step_w;
                float cx = sx + step_w * 0.5f;

                // Playhead Highlight cruzando todas as pistas durante o Audition Loop
                if (is_auditioning && audition_current_step == s) {
                    dl->AddRectFilled(ImVec2(sx + 1, pos.y), ImVec2(sx + step_w - 1, row3_y + row3_h + 4.0f), IM_COL32(0, 229, 255, 30), 2.0f);
                    dl->AddRect(ImVec2(sx + 1, pos.y), ImVec2(sx + step_w - 1, row3_y + row3_h + 4.0f), IM_COL32(0, 240, 255, 180), 2.0f, 0, 1.2f);
                }

                // ── 1. LANE KICK: Botões Circulares de LED Âmbar ──
                float kick_cy = row1_y + row1_h * 0.5f;
                bool k_act = kick_step_active[s];
                ImU32 k_bg = k_act ? IM_COL32(255, 140, 30, 255) : IM_COL32(22, 16, 12, 255);
                ImU32 k_border = k_act ? IM_COL32(255, 200, 100, 255) : IM_COL32(45, 30, 20, 255);
                dl->AddCircleFilled(ImVec2(cx, kick_cy), 6.5f, k_bg, 16);
                dl->AddCircle(ImVec2(cx, kick_cy), 6.5f, k_border, 16, k_act ? 1.8f : 1.0f);
                if (k_act) {
                    dl->AddCircle(ImVec2(cx, kick_cy), 9.5f, IM_COL32(255, 140, 30, 70), 16, 1.5f);
                }

                ImRect k_bb(ImVec2(cx - 8, kick_cy - 8), ImVec2(cx + 8, kick_cy + 8));
                if (ImGui::IsMouseClicked(0) && k_bb.Contains(mouse)) {
                    kick_step_active[s] = !kick_step_active[s];
                }

                // ── 2. LANE BASS: Barras de Velocidade com LEDs ──
                bool b_act = step_active[s];
                float top_led_y = row2_y + 4.0f;
                float bar_top_y = row2_y + 14.0f;
                float bar_h = row2_h - 26.0f;
                float bot_led_y = bar_top_y + bar_h + 6.0f;

                ImU32 led_top_col = b_act ? IM_COL32(57, 255, 140, 255) : IM_COL32(20, 50, 40, 255);
                dl->AddCircleFilled(ImVec2(cx, top_led_y), 2.5f, led_top_col);
                if (b_act) {
                    dl->AddCircle(ImVec2(cx, top_led_y), 4.0f, IM_COL32(57, 255, 140, 90), 12, 1.0f);
                }

                ImRect b_led_bb(ImVec2(sx, row2_y), ImVec2(sx + step_w, bar_top_y));
                if (ImGui::IsMouseClicked(0) && b_led_bb.Contains(mouse)) {
                    step_active[s] = !step_active[s];
                }

                float bar_w = step_w * 0.62f;
                float bx0 = cx - bar_w * 0.5f;
                float bx1 = cx + bar_w * 0.5f;
                dl->AddRectFilled(ImVec2(bx0, bar_top_y), ImVec2(bx1, bar_top_y + bar_h), IM_COL32(10, 16, 22, 255), 2.0f);

                ImRect bar_bb(ImVec2(sx, bar_top_y), ImVec2(sx + step_w, bar_top_y + bar_h));
                if (ImGui::IsMouseDown(0) && bar_bb.Contains(mouse)) {
                    float rel_y = (bar_top_y + bar_h - mouse.y) / bar_h;
                    step_velocity[s] = std::clamp(rel_y, 0.05f, 1.0f);
                }

                float cur_h = bar_h * step_velocity[s];
                float by_top = bar_top_y + bar_h - cur_h;
                float by_bot = bar_top_y + bar_h;
                ImU32 bar_top_col = b_act ? IM_COL32(100, 255, 210, 255) : IM_COL32(40, 80, 75, 200);
                ImU32 bar_bot_col = b_act ? IM_COL32(0, 180, 220, 255) : IM_COL32(15, 35, 45, 200);
                dl->AddRectFilledMultiColor(ImVec2(bx0, by_top), ImVec2(bx1, by_bot), bar_top_col, bar_top_col, bar_bot_col, bar_bot_col);

                // LED inferior de Shift de Oitava
                bool has_pitch = (step_pitch_offset[s] != 0);
                ImU32 bot_col = has_pitch ? IM_COL32(0, 229, 255, 255) : IM_COL32(20, 34, 46, 255);
                dl->AddCircleFilled(ImVec2(cx, bot_led_y), 2.5f, bot_col);
                ImRect bot_led_bb(ImVec2(sx, bot_led_y - 4), ImVec2(sx + step_w, bot_led_y + 6));
                if (ImGui::IsMouseClicked(0) && bot_led_bb.Contains(mouse)) {
                    step_pitch_offset[s] = (step_pitch_offset[s] == 0) ? 12 : 0;
                }

                // ── 3. LANE SNARE / CLAP: Botões Circulares de LED Magenta ──
                float snare_cy = row3_y + row3_h * 0.5f;
                bool s_act = snare_step_active[s];
                ImU32 s_bg = s_act ? IM_COL32(230, 60, 190, 255) : IM_COL32(20, 14, 22, 255);
                ImU32 s_border = s_act ? IM_COL32(255, 120, 230, 255) : IM_COL32(45, 22, 45, 255);
                dl->AddCircleFilled(ImVec2(cx, snare_cy), 6.5f, s_bg, 16);
                dl->AddCircle(ImVec2(cx, snare_cy), 6.5f, s_border, 16, s_act ? 1.8f : 1.0f);
                if (s_act) {
                    dl->AddCircle(ImVec2(cx, snare_cy), 9.5f, IM_COL32(230, 60, 190, 70), 16, 1.5f);
                }

                ImRect s_bb(ImVec2(cx - 8, snare_cy - 8), ImVec2(cx + 8, snare_cy + 8));
                if (ImGui::IsMouseClicked(0) && s_bb.Contains(mouse)) {
                    snare_step_active[s] = !snare_step_active[s];
                }
            }
        }

        // ── OSCILOSCÓPIO VETORIAL RETICULAR HÍBRIDO (BASS, KICK, SNARE, FULL) ──
        void DrawOscilloscope(ImDrawList* dl, ImVec2 pos, ImVec2 size) {
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(6, 10, 15, 255), 6.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(18, 32, 44, 255), 6.0f);

            // Grade reticular milimétrica
            float grid_dx = size.x / 8.0f;
            float grid_dy = size.y / 4.0f;
            for (int i = 1; i < 8; ++i) {
                dl->AddLine(ImVec2(pos.x + i * grid_dx, pos.y + 4), ImVec2(pos.x + i * grid_dx, pos.y + size.y - 4), IM_COL32(0, 180, 210, 20), 1.0f);
            }
            for (int j = 1; j < 4; ++j) {
                dl->AddLine(ImVec2(pos.x + 4, pos.y + j * grid_dy), ImVec2(pos.x + size.x - 4, pos.y + j * grid_dy), IM_COL32(0, 180, 210, 20), 1.0f);
            }
            dl->AddLine(ImVec2(pos.x + 4, pos.y + size.y * 0.5f), ImVec2(pos.x + size.x - 4, pos.y + size.y * 0.5f), IM_COL32(0, 229, 255, 50), 1.0f);

            static const int OSC_PREVIEW_POINTS = 160;
            float osc_buf[OSC_PREVIEW_POINTS] = { 0 };

            int p_mode = (current_layer_tab == TAB_KICK ? 1 : (current_layer_tab == TAB_SNARE ? 2 : (current_layer_tab == TAB_FULL_BASE ? 3 : 0)));

            if (synth_dsp) {
                synth_dsp->getOscilloscopeBuffer(osc_buf, OSC_PREVIEW_POINTS, p_mode);
            }

            float mid_y = pos.y + size.y * 0.5f;
            float amp_scale = (size.y * 0.44f);
            float dx = size.x / (float)(OSC_PREVIEW_POINTS - 1);

            // Cor do feixe neon de acordo com a aba ativa
            ImU32 beam_halo = (current_layer_tab == TAB_KICK) ? IM_COL32(255, 140, 30, 45) : (current_layer_tab == TAB_SNARE ? IM_COL32(230, 60, 190, 45) : IM_COL32(0, 229, 255, 45));
            ImU32 beam_main = (current_layer_tab == TAB_KICK) ? IM_COL32(255, 180, 60, 220) : (current_layer_tab == TAB_SNARE ? IM_COL32(255, 100, 220, 220) : IM_COL32(0, 245, 255, 220));

            for (int i = 0; i < OSC_PREVIEW_POINTS - 1; ++i) {
                float x1 = pos.x + i * dx;
                float y1 = mid_y - osc_buf[i] * amp_scale;
                float x2 = pos.x + (i + 1) * dx;
                float y2 = mid_y - osc_buf[i + 1] * amp_scale;

                dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), beam_halo, 4.5f);
                dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), beam_main, 2.2f);
                dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(240, 255, 255, 255), 1.0f);
            }

            // HUD informativo no canto superior do osciloscópio
            char hud_buf[80];
            if (current_layer_tab == TAB_KICK) {
                float sf = synth_dsp ? synth_dsp->getKickStartFreq() : 185.0f;
                float ef = synth_dsp ? synth_dsp->getKickEndFreq() : 50.0f;
                snprintf(hud_buf, sizeof(hud_buf), "KICK | %.0f->%.0fHz | Sub %s Phase-Locked", sf, ef, root_note_short[std::clamp(current_root_idx, 0, 11)]);
            } else if (current_layer_tab == TAB_SNARE) {
                float tf = synth_dsp ? synth_dsp->getSnareTone() : 210.0f;
                snprintf(hud_buf, sizeof(hud_buf), "SNARE | Tone %.0fHz | Snap %.2f", tf, synth_dsp ? synth_dsp->getSnareSnap() : 0.85f);
            } else if (current_layer_tab == TAB_FULL_BASE) {
                snprintf(hud_buf, sizeof(hud_buf), "FULL K&B BASE | %s %s | Phase Coherent", root_note_short[std::clamp(current_root_idx, 0, 11)], scale_names[std::clamp(current_scale_idx, 0, 6)]);
            } else {
                int cur_osc = synth_dsp ? synth_dsp->getOscType() : 0;
                const char* osc_lbl = (cur_osc == 0) ? "SAW" : (cur_osc == 1) ? "SQUARE" : (cur_osc == 2) ? "SUB-SINE" : "MORPH";
                snprintf(hud_buf, sizeof(hud_buf), "%s | %s | %s", osc_lbl, root_note_short[std::clamp(current_root_idx, 0, 11)], synth_model_short[std::clamp(current_synth_model, 0, 4)]);
            }
            dl->AddText(ImGui::GetFont(), 10.5f, ImVec2(pos.x + 12.0f, pos.y + 8.0f), IM_COL32(0, 210, 240, 220), hud_buf);

            // LED de Trigger do Osciloscópio (Verde pulsante em reprodução)
            ImU32 led_col = is_auditioning ? IM_COL32(57, 255, 140, 255) : IM_COL32(0, 229, 255, 220);
            dl->AddCircle(ImVec2(pos.x + size.x - 16, pos.y + 14), 4.5f, led_col, 12, 1.2f);
            dl->AddCircleFilled(ImVec2(pos.x + size.x - 16, pos.y + 14), 2.2f, led_col);
            if (is_auditioning) {
                dl->AddCircle(ImVec2(pos.x + size.x - 16, pos.y + 14), 7.5f, IM_COL32(57, 255, 140, 90), 12, 1.2f);
            }
        }

        // ── CURVAS DE ENVELOPE (FILTRO, KICK, SNARE) ──
        void DrawFilterEnvelopeCurve(ImDrawList* dl, ImVec2 pos, ImVec2 size, float decay_sec) {
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(6, 10, 14, 255), 4.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 38, 50, 255), 4.0f);

            const int num_pts = 32;
            ImVec2 pts[num_pts];
            float decay_rate = 6.0f / (decay_sec > 0.01f ? decay_sec : 0.085f);
            decay_rate = std::clamp(decay_rate, 8.0f, 40.0f);

            for (int i = 0; i < num_pts; ++i) {
                float t = (float)i / (float)(num_pts - 1);
                float px = pos.x + 6.0f + t * (size.x - 12.0f);
                float val = (t < 0.06f) ? (t / 0.06f) : std::exp(-(t - 0.06f) * (decay_rate * 0.15f));
                float py = pos.y + size.y - 6.0f - val * (size.y - 12.0f);
                pts[i] = ImVec2(px, py);
            }

            for (int i = 0; i < num_pts - 1; ++i) {
                ImVec2 p0 = pts[i];
                ImVec2 p1 = pts[i + 1];
                dl->AddQuadFilled(p0, p1, ImVec2(p1.x, pos.y + size.y - 6.0f), ImVec2(p0.x, pos.y + size.y - 6.0f), IM_COL32(0, 229, 255, 35));
                dl->AddLine(p0, p1, IM_COL32(0, 240, 255, 255), 2.0f);
            }
        }

        void DrawKickDecayCurve(ImDrawList* dl, ImVec2 pos, ImVec2 size, float amp_decay_ms) {
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(14, 10, 8, 255), 4.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(45, 28, 18, 255), 4.0f);

            const int num_pts = 32;
            ImVec2 pts[num_pts];
            float decay_rate = 250.0f / (amp_decay_ms > 20.0f ? amp_decay_ms : 220.0f);

            for (int i = 0; i < num_pts; ++i) {
                float t = (float)i / (float)(num_pts - 1);
                float px = pos.x + 6.0f + t * (size.x - 12.0f);
                float val = (t < 0.03f) ? (t / 0.03f) : std::exp(-(t - 0.03f) * decay_rate * 3.5f);
                float py = pos.y + size.y - 6.0f - val * (size.y - 12.0f);
                pts[i] = ImVec2(px, py);
            }

            for (int i = 0; i < num_pts - 1; ++i) {
                ImVec2 p0 = pts[i];
                ImVec2 p1 = pts[i + 1];
                dl->AddQuadFilled(p0, p1, ImVec2(p1.x, pos.y + size.y - 6.0f), ImVec2(p0.x, pos.y + size.y - 6.0f), IM_COL32(255, 140, 30, 35));
                dl->AddLine(p0, p1, IM_COL32(255, 160, 40, 255), 2.0f);
            }
        }

    public:
        bool need_focus = true;

        void render(ClipManager& clip_manager, float bpm, int target_channel_idx = 1) {
            if (!is_open) {
                stopAudition();
                return;
            }

            // Atalho de Teclado: Barra de Espaço para Play/Stop do Audition Loop
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
                is_auditioning = !is_auditioning;
                if (!is_auditioning) {
                    stopAudition();
                } else {
                    audition_timer = 0.0f;
                    audition_current_step = -1;
                }
            }

            // Motor de Sequenciamento em Tempo Real do Audition (Kick + Rolling Bass + Snare)
            if (is_auditioning && synth_dsp) {
                float cur_bpm = (bpm > 20.0f ? bpm : 138.0f);
                float beat_sec = 60.0f / cur_bpm;
                float step_sec = beat_sec * 0.25f; // Semicolcheia (16th note)
                float total_cycle_sec = step_sec * NUM_STEPS; // 1 compasso (16 steps)

                float dt = ImGui::GetIO().DeltaTime;
                audition_timer += dt;
                if (audition_timer >= total_cycle_sec) {
                    audition_timer = std::fmod(audition_timer, total_cycle_sec);
                }

                int step_idx = (int)(audition_timer / step_sec);
                if (step_idx < 0) step_idx = 0;
                if (step_idx >= NUM_STEPS) step_idx = NUM_STEPS - 1;

                float step_phase = (audition_timer - step_idx * step_sec) / step_sec;

                if (step_idx != audition_current_step) {
                    if (audition_active_note_id != -1) {
                        synth_dsp->pushMidiEvent({ audition_active_note_id, 0, false, 0.0f, 0.0f, 0.0f, 0.0f });
                        audition_active_note_id = -1;
                    }

                    audition_current_step = step_idx;

                    // 1. Disparar Kick Drum
                    if (kick_step_active[step_idx]) {
                        synth_dsp->triggerKick(kick_step_velocity[step_idx]);
                    }

                    // 2. Disparar Rolling Bass
                    if (step_active[step_idx]) {
                        int midi_key = 36 + current_root_idx + step_pitch_offset[step_idx];
                        audition_active_note_id = 9000 + step_idx;
                        float vel = std::clamp(step_velocity[step_idx], 0.1f, 1.0f);
                        synth_dsp->pushMidiEvent({ audition_active_note_id, midi_key, true, vel, 0.0f, 0.0f, 0.0f });
                    }

                    // 3. Disparar Snare / Clap
                    if (snare_step_active[step_idx]) {
                        synth_dsp->triggerSnare(snare_step_velocity[step_idx]);
                    }
                } else {
                    // Gate do Bass: desliga aos 85% do step para dar clareza de transiente
                    if (step_phase > 0.85f && audition_active_note_id != -1) {
                        synth_dsp->pushMidiEvent({ audition_active_note_id, 0, false, 0.0f, 0.0f, 0.0f, 0.0f });
                        audition_active_note_id = -1;
                    }
                }
            } else if (!is_auditioning && audition_active_note_id != -1) {
                stopAudition();
            }

            ImVec2 disp_sz = ImGui::GetIO().DisplaySize;
            ImGui::SetNextWindowSize(ImVec2(1040, 580), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2((disp_sz.x - 1040) * 0.5f, (disp_sz.y - 580) * 0.5f), ImGuiCond_Always);
            if (need_focus) {
                ImGui::SetNextWindowFocus();
                need_focus = false;
            }
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.08f, 0.99f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.18f, 0.24f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 12));

            g_rolling_bass_tex_pack.Init();

            if (ImGui::Begin("KURO PSYTRANCE RHYTHM & BASS WORKSTATION###RollingBassMainWindow", &is_open, flags)) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 win_pos = ImGui::GetWindowPos();
                ImVec2 win_size = ImGui::GetWindowSize();

                // Barras de brilho neon nas laterais da janela
                dl->AddRectFilled(ImVec2(win_pos.x + 3.0f, win_pos.y + 40.0f), ImVec2(win_pos.x + 6.0f, win_pos.y + win_size.y - 40.0f), IM_COL32(0, 229, 255, 220), 1.5f);
                dl->AddRectFilled(ImVec2(win_pos.x + win_size.x - 6.0f, win_pos.y + 40.0f), ImVec2(win_pos.x + win_size.x - 3.0f, win_pos.y + win_size.y - 40.0f), IM_COL32(0, 229, 255, 220), 1.5f);

                // ──────────────────────────────────────────────────────────────────────────
                // 1. TOP HEADER (LOGO + LAYER TABS + ROOT NOTE + SCALE + AUDITION LOOP)
                // ──────────────────────────────────────────────────────────────────────────
                float header_y = win_pos.y + 26.0f;

                // Logo KURO
                DrawKuroLogo(dl, ImVec2(win_pos.x + 20.0f, header_y + 2.0f), 24.0f, IM_COL32(0, 229, 255, 255));
                dl->AddText(ImGui::GetFont(), 20.0f, ImVec2(win_pos.x + 52.0f, header_y + 1.0f), IM_COL32(0, 240, 255, 255), "KURO");
                dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(win_pos.x + 52.0f, header_y + 22.0f), IM_COL32(110, 160, 195, 255), "PSY RHYTHM & BASS");

                // ── ABAS DE CAMADAS DO INSTRUMENTO (LAYER TABS) ──
                float tab_x = win_pos.x + 218.0f;
                float tab_h = 28.0f;
                float tab_y = header_y + 8.0f;

                if (DrawLayerTab("🎛️ FULL BASE", ImVec2(tab_x, tab_y), ImVec2(92.0f, tab_h), current_layer_tab == TAB_FULL_BASE, IM_COL32(0, 229, 255, 255))) {
                    current_layer_tab = TAB_FULL_BASE;
                }
                tab_x += 98.0f;

                if (DrawLayerTab("🥊 KICK", ImVec2(tab_x, tab_y), ImVec2(74.0f, tab_h), current_layer_tab == TAB_KICK, IM_COL32(255, 140, 30, 255))) {
                    current_layer_tab = TAB_KICK;
                }
                tab_x += 80.0f;

                if (DrawLayerTab("⚡ ROLLING BASS", ImVec2(tab_x, tab_y), ImVec2(110.0f, tab_h), current_layer_tab == TAB_BASS, IM_COL32(0, 240, 180, 255))) {
                    current_layer_tab = TAB_BASS;
                }
                tab_x += 116.0f;

                if (DrawLayerTab("💥 SNARE / CLAP", ImVec2(tab_x, tab_y), ImVec2(98.0f, tab_h), current_layer_tab == TAB_SNARE, IM_COL32(230, 60, 190, 255))) {
                    current_layer_tab = TAB_SNARE;
                }
                tab_x += 104.0f;

                // ── SELETORES DE TOM E ESCALA ──
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.24f, 0.34f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

                float root_x = win_pos.x + 625.0f;
                dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(root_x, header_y - 3.0f), IM_COL32(110, 160, 195, 255), "ROOT NOTE");
                ImGui::SetCursorScreenPos(ImVec2(root_x, header_y + 12.0f));
                ImGui::SetNextItemWidth(72.0f);
                if (ImGui::Combo("##RootNoteCombo", &current_root_idx, root_note_short, IM_ARRAYSIZE(root_note_short))) {
                    if (synth_dsp) {
                        synth_dsp->tuneKickToRootNote(current_root_idx);
                        if (is_auditioning && audition_active_note_id != -1 && audition_current_step >= 0) {
                            int midi_key = 36 + current_root_idx + step_pitch_offset[audition_current_step];
                            synth_dsp->pushMidiEvent({ audition_active_note_id, midi_key, true, std::clamp(step_velocity[audition_current_step], 0.1f, 1.0f), 0.0f, 0.0f, 0.0f });
                        }
                    }
                }

                float scale_x = root_x + 82.0f;
                dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(scale_x, header_y - 3.0f), IM_COL32(110, 160, 195, 255), "SCALE");
                ImGui::SetCursorScreenPos(ImVec2(scale_x, header_y + 12.0f));
                ImGui::SetNextItemWidth(105.0f);
                if (ImGui::Combo("##ScaleCombo", &current_scale_idx, scale_names, IM_ARRAYSIZE(scale_names))) {
                }

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);

                // ── BOTÃO AUDITION LOOP (Pill Neon Verde) ──
                float aud_btn_x = win_pos.x + 828.0f;
                float aud_btn_w = win_pos.x + win_size.x - 20.0f - aud_btn_x;
                float aud_btn_h = 32.0f;
                ImVec2 aud_p0(aud_btn_x, header_y + 8.0f);
                ImVec2 aud_p1(aud_p0.x + aud_btn_w, aud_p0.y + aud_btn_h);
                ImGui::SetCursorScreenPos(aud_p0);
                ImGuiID aud_id = ImGui::GetID("##AuditionLoopToggle");
                ImRect aud_bb(aud_p0, aud_p1);
                ImGui::ItemSize(aud_bb);
                if (ImGui::ItemAdd(aud_bb, aud_id)) {
                    bool hov, held;
                    if (ImGui::ButtonBehavior(aud_bb, aud_id, &hov, &held)) {
                        is_auditioning = !is_auditioning;
                        if (!is_auditioning) {
                            stopAudition();
                        } else {
                            audition_timer = 0.0f;
                            audition_current_step = -1;
                        }
                    }

                    ImU32 aud_bg = is_auditioning 
                        ? (hov ? IM_COL32(28, 85, 48, 255) : IM_COL32(18, 62, 34, 255))
                        : (hov ? IM_COL32(12, 36, 22, 255) : IM_COL32(8, 24, 15, 255));
                    ImU32 aud_border = is_auditioning ? IM_COL32(57, 255, 140, 255) : (hov ? IM_COL32(60, 240, 130, 255) : IM_COL32(40, 190, 100, 255));
                    ImU32 aud_txt_col = is_auditioning ? IM_COL32(255, 255, 255, 255) : (hov ? IM_COL32(100, 255, 170, 255) : IM_COL32(57, 255, 140, 255));

                    dl->AddRectFilled(aud_bb.Min, aud_bb.Max, aud_bg, 16.0f);
                    dl->AddRect(aud_bb.Min, aud_bb.Max, aud_border, 16.0f, 0, is_auditioning ? 2.0f : 1.2f);
                    if (is_auditioning || hov) {
                        dl->AddRect(ImVec2(aud_bb.Min.x - 1, aud_bb.Min.y - 1), ImVec2(aud_bb.Max.x + 1, aud_bb.Max.y + 1), IM_COL32(57, 255, 140, 50), 17.0f, 0, 2.0f);
                    }

                    const char* aud_label = is_auditioning ? "STOP AUDITION [SPACE]" : "AUDITION K&B [SPACE]";
                    ImVec2 aud_txt_sz = ImGui::CalcTextSize(aud_label);
                    dl->AddText(ImVec2(aud_bb.Min.x + (aud_btn_w - aud_txt_sz.x) * 0.5f, aud_bb.Min.y + (aud_btn_h - aud_txt_sz.y) * 0.5f), aud_txt_col, aud_label);
                }

                // ──────────────────────────────────────────────────────────────────────────
                // 2. CORPO PRINCIPAL (3 COLUNAS)
                // ──────────────────────────────────────────────────────────────────────────
                float main_top_y = header_y + 44.0f;
                float card_h = 378.0f;
                float col1_w = 260.0f;
                float col2_w = 345.0f;
                float col3_w = 365.0f;
                float card_spacing = 14.0f;

                ImVec2 c1_p0(win_pos.x + 20.0f, main_top_y);
                ImVec2 c1_p1(c1_p0.x + col1_w, c1_p0.y + card_h);
                dl->AddRectFilled(c1_p0, c1_p1, IM_COL32(10, 14, 20, 255), 8.0f);
                dl->AddRect(c1_p0, c1_p1, IM_COL32(22, 36, 48, 255), 8.0f, 0, 1.2f);

                // ── COLUNA 1 DINÂMICA CONFORME A ABA ATIVA ──
                if (current_layer_tab == TAB_KICK) {
                    ImVec2 t_sz = ImGui::CalcTextSize("KICK PUNCH & ATTACK");
                    dl->AddText(ImVec2(c1_p0.x + (col1_w - t_sz.x) * 0.5f, c1_p0.y + 12.0f), IM_COL32(255, 160, 40, 255), "KICK PUNCH & ATTACK");

                    // Seletor de Modelo / Preset de Kick
                    dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(c1_p0.x + 20.0f, c1_p0.y + 36.0f), IM_COL32(180, 200, 215, 255), "KICK MODEL");
                    ImGui::SetCursorScreenPos(ImVec2(c1_p0.x + 20.0f, c1_p0.y + 50.0f));
                    ImGui::SetNextItemWidth(col1_w - 40.0f);
                    const char* kick_p_names[] = {
                        "138 BPM Psy Punch (Hard Hit)",
                        "140 BPM Full-On Click & Sub",
                        "142 BPM Darkpsy Heavy Punch",
                        "136 BPM Progressive Deep Body",
                        "145 BPM Hi-Tech Laser Kick"
                    };
                    if (ImGui::Combo("##KickPresetCombo", &current_kick_preset_idx, kick_p_names, IM_ARRAYSIZE(kick_p_names))) {
                        applyKickPreset(current_kick_preset_idx);
                    }

                    // Botão para afinar o Kick com a tônica do baixo
                    ImGui::SetCursorScreenPos(ImVec2(c1_p0.x + 20.0f, c1_p0.y + 90.0f));
                    if (ImGui::Button("🎯 TUNE SUB TO ROOT NOTE", ImVec2(col1_w - 40.0f, 26.0f))) {
                        if (synth_dsp) {
                            synth_dsp->tuneKickToRootNote(current_root_idx);
                            toast_message = "✨ Bumbo afinado com perfeição na tônica do Baixo!";
                            toast_timer = 2.5f;
                        }
                    }

                    // Knobs de Ataque e Punch do Kick
                    float k_knob_y1 = c1_p0.y + 165.0f;
                    float cur_k_start = synth_dsp ? synth_dsp->getKickStartFreq() : 185.0f;
                    float cur_k_pdecay = synth_dsp ? synth_dsp->getKickPitchDecay() : 36.0f;

                    if (DrawPhotorealisticRotatingKnob("##KickStartFreq", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 65.0f, k_knob_y1), 18.0f, &cur_k_start, 100.0f, 380.0f, "Start Pitch", "%.0f Hz", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickStartFreq(cur_k_start);
                    }
                    if (DrawPhotorealisticRotatingKnob("##KickPitchDecay", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 195.0f, k_knob_y1), 18.0f, &cur_k_pdecay, 15.0f, 90.0f, "Pitch Decay", "%.0f ms", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickPitchDecay(cur_k_pdecay);
                    }

                    float k_knob_y2 = c1_p0.y + 265.0f;
                    float cur_k_click = synth_dsp ? synth_dsp->getKickClick() : 0.80f;
                    float cur_k_vol = synth_dsp ? synth_dsp->getKickVolume() : 0.95f;

                    if (DrawPhotorealisticRotatingKnob("##KickClick", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 65.0f, k_knob_y2), 18.0f, &cur_k_click, 0.0f, 1.0f, "Beater Click", "%.2f", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickClick(cur_k_click);
                    }
                    if (DrawPhotorealisticRotatingKnob("##KickVol", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 195.0f, k_knob_y2), 18.0f, &cur_k_vol, 0.0f, 1.5f, "Kick Level", "%.2f", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickVolume(cur_k_vol);
                    }

                    char sub_info[64];
                    float cur_sub_f = synth_dsp ? synth_dsp->getKickEndFreq() : 50.0f;
                    snprintf(sub_info, sizeof(sub_info), "Sub Body: %.1f Hz (%s)", cur_sub_f, root_note_short[std::clamp(current_root_idx, 0, 11)]);
                    ImVec2 si_sz = ImGui::CalcTextSize(sub_info);
                    dl->AddText(ImVec2(c1_p0.x + (col1_w - si_sz.x) * 0.5f, c1_p0.y + card_h - 26.0f), IM_COL32(255, 180, 70, 200), sub_info);

                } else if (current_layer_tab == TAB_SNARE) {
                    ImVec2 t_sz = ImGui::CalcTextSize("SNARE TONE & BODY");
                    dl->AddText(ImVec2(c1_p0.x + (col1_w - t_sz.x) * 0.5f, c1_p0.y + 12.0f), IM_COL32(230, 80, 210, 255), "SNARE TONE & BODY");

                    dl->AddText(ImGui::GetFont(), 10.0f, ImVec2(c1_p0.x + 20.0f, c1_p0.y + 36.0f), IM_COL32(180, 200, 215, 255), "SNARE MODEL");
                    ImGui::SetCursorScreenPos(ImVec2(c1_p0.x + 20.0f, c1_p0.y + 50.0f));
                    ImGui::SetNextItemWidth(col1_w - 40.0f);
                    const char* snare_p_names[] = { "Psy 909 Tight Snare", "Crunchy Noise Clap", "Laser Zap Snare", "Tribal Wood Snap" };
                    if (ImGui::Combo("##SnarePresetCombo", &current_snare_preset_idx, snare_p_names, IM_ARRAYSIZE(snare_p_names))) {
                        applySnarePreset(current_snare_preset_idx);
                    }

                    float s_knob_y1 = c1_p0.y + 140.0f;
                    float cur_s_tone = synth_dsp ? synth_dsp->getSnareTone() : 210.0f;
                    float cur_s_snap = synth_dsp ? synth_dsp->getSnareSnap() : 0.85f;

                    if (DrawPhotorealisticRotatingKnob("##SnareTone", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 65.0f, s_knob_y1), 18.0f, &cur_s_tone, 120.0f, 350.0f, "Tone Pitch", "%.0f Hz", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareTone(cur_s_tone);
                    }
                    if (DrawPhotorealisticRotatingKnob("##SnareSnap", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 195.0f, s_knob_y1), 18.0f, &cur_s_snap, 0.0f, 1.0f, "Snap Attack", "%.2f", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareSnap(cur_s_snap);
                    }

                    float s_knob_y2 = c1_p0.y + 245.0f;
                    float cur_s_decay = synth_dsp ? synth_dsp->getSnareNoiseDecay() : 135.0f;
                    float cur_s_vol = synth_dsp ? synth_dsp->getSnareVolume() : 0.80f;

                    if (DrawPhotorealisticRotatingKnob("##SnareNoiseDecay", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 65.0f, s_knob_y2), 18.0f, &cur_s_decay, 50.0f, 350.0f, "Noise Tail", "%.0f ms", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareNoiseDecay(cur_s_decay);
                    }
                    if (DrawPhotorealisticRotatingKnob("##SnareVol", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 195.0f, s_knob_y2), 18.0f, &cur_s_vol, 0.0f, 1.5f, "Snare Level", "%.2f", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareVolume(cur_s_vol);
                    }

                } else if (current_layer_tab == TAB_FULL_BASE) {
                    ImVec2 t_sz = ImGui::CalcTextSize("K&B QUICK BALANCER");
                    dl->AddText(ImVec2(c1_p0.x + (col1_w - t_sz.x) * 0.5f, c1_p0.y + 12.0f), IM_COL32(0, 229, 255, 255), "K&B QUICK BALANCER");

                    float fb_y = c1_p0.y + 60.0f;
                    float cur_k_v = synth_dsp ? synth_dsp->getKickVolume() : 0.95f;
                    if (DrawPhotorealisticRotatingKnob("##FB_KickVol", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 65.0f, fb_y), 17.0f, &cur_k_v, 0.0f, 1.5f, "Kick Level", "%.2f", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickVolume(cur_k_v);
                    }

                    float cur_b_cut = synth_dsp ? synth_dsp->getCutoff() : 550.0f;
                    if (DrawPhotorealisticRotatingKnob("##FB_BassCut", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 195.0f, fb_y), 17.0f, &cur_b_cut, 40.0f, 6000.0f, "Bass Cutoff", "%.0f Hz", IM_COL32(0, 229, 255, 255))) {
                        if (synth_dsp) synth_dsp->setCutoff(cur_b_cut);
                    }

                    float fb_y2 = c1_p0.y + 175.0f;
                    float cur_s_v = synth_dsp ? synth_dsp->getSnareVolume() : 0.80f;
                    if (DrawPhotorealisticRotatingKnob("##FB_SnareVol", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 65.0f, fb_y2), 17.0f, &cur_s_v, 0.0f, 1.5f, "Snare Level", "%.2f", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareVolume(cur_s_v);
                    }

                    float cur_drive = synth_dsp ? synth_dsp->getDrive() : 0.35f;
                    if (DrawPhotorealisticRotatingKnob("##FB_MasterDrive", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 195.0f, fb_y2), 17.0f, &cur_drive, 0.0f, 1.0f, "Master Drive", "%.2f", IM_COL32(0, 240, 180, 255))) {
                        if (synth_dsp) synth_dsp->setDrive(cur_drive);
                    }

                    // Badge de Coerência de Fase
                    ImVec2 badge_p(c1_p0.x + 16.0f, c1_p0.y + 285.0f);
                    dl->AddRectFilled(badge_p, ImVec2(badge_p.x + col1_w - 32.0f, badge_p.y + 44.0f), IM_COL32(10, 28, 20, 255), 4.0f);
                    dl->AddRect(badge_p, ImVec2(badge_p.x + col1_w - 32.0f, badge_p.y + 44.0f), IM_COL32(57, 255, 140, 180), 4.0f);
                    dl->AddCircleFilled(ImVec2(badge_p.x + 16.0f, badge_p.y + 22.0f), 5.0f, IM_COL32(57, 255, 140, 255));
                    dl->AddText(ImVec2(badge_p.x + 30.0f, badge_p.y + 14.0f), IM_COL32(200, 255, 220, 255), "PHASE COHERENT K&B");

                } else { // TAB_BASS
                    ImVec2 t1_sz = ImGui::CalcTextSize("OSCILLATOR & PUNCH");
                    dl->AddText(ImVec2(c1_p0.x + (col1_w - t1_sz.x) * 0.5f, c1_p0.y + 10.0f), IM_COL32(160, 195, 220, 255), "OSCILLATOR & PUNCH");

                    // Seletor de Modelo de Sintetizador
                    dl->AddText(ImGui::GetFont(), 9.5f, ImVec2(c1_p0.x + 16.0f, c1_p0.y + 30.0f), IM_COL32(0, 229, 255, 220), "SYNTH ENGINE");
                    ImGui::SetCursorScreenPos(ImVec2(c1_p0.x + 16.0f, c1_p0.y + 44.0f));
                    ImGui::SetNextItemWidth(col1_w - 32.0f);
                    if (ImGui::Combo("##SynthModelCombo", &current_synth_model, synth_model_names, IM_ARRAYSIZE(synth_model_names))) {
                        if (synth_dsp) synth_dsp->setSynthModel(current_synth_model);
                    }

                    int cur_osc = synth_dsp ? synth_dsp->getOscType() : 0;
                    float wave_btn_y = c1_p0.y + 98.0f;
                    float wave_btn_r = 18.0f;
                    float wave_spacing = col1_w / 4.0f;

                    if (DrawWaveCircleBtn("##WaveSaw", dl, ImVec2(c1_p0.x + wave_spacing * 1.0f, wave_btn_y), wave_btn_r, 0, cur_osc == 0)) {
                        if (synth_dsp) synth_dsp->setOscType(0);
                    }
                    dl->AddText(ImVec2(c1_p0.x + wave_spacing * 1.0f - 11.0f, wave_btn_y + wave_btn_r + 3.0f), IM_COL32(150, 180, 205, 255), "Saw");

                    if (DrawWaveCircleBtn("##WaveSquare", dl, ImVec2(c1_p0.x + wave_spacing * 2.0f, wave_btn_y), wave_btn_r, 1, cur_osc == 1)) {
                        if (synth_dsp) synth_dsp->setOscType(1);
                    }
                    dl->AddText(ImVec2(c1_p0.x + wave_spacing * 2.0f - 18.0f, wave_btn_y + wave_btn_r + 3.0f), IM_COL32(150, 180, 205, 255), "Square");

                    if (DrawWaveCircleBtn("##WaveSubSine", dl, ImVec2(c1_p0.x + wave_spacing * 3.0f, wave_btn_y), wave_btn_r, 2, cur_osc == 2)) {
                        if (synth_dsp) synth_dsp->setOscType(2);
                    }
                    dl->AddText(ImVec2(c1_p0.x + wave_spacing * 3.0f - 22.0f, wave_btn_y + wave_btn_r + 3.0f), IM_COL32(150, 180, 205, 255), "Sub-sine");

                    float phase_dial_y = c1_p0.y + 195.0f;
                    float cur_phase = synth_dsp ? synth_dsp->getPhaseRetrigger() : 90.0f;
                    if (DrawPhotorealisticPhaseDial("##PhaseDial", dl, g_rolling_bass_tex_pack.tex_dial_phase, ImVec2(c1_p0.x + col1_w * 0.5f, phase_dial_y), 30.0f, &cur_phase)) {
                        if (synth_dsp) synth_dsp->setPhaseRetrigger(cur_phase);
                    }
                    ImVec2 p_lbl_sz = ImGui::CalcTextSize("Phase Retrigger");
                    dl->AddText(ImVec2(c1_p0.x + (col1_w - p_lbl_sz.x) * 0.5f, phase_dial_y + 36.0f), IM_COL32(160, 195, 220, 255), "Phase Retrigger");

                    float punch_knob_y = c1_p0.y + 316.0f;
                    float cur_p_depth = synth_dsp ? synth_dsp->getPitchDepth() : 18.0f;
                    float cur_p_decay = synth_dsp ? synth_dsp->getPitchDecay() : 0.004f;
                    float cur_click = synth_dsp ? synth_dsp->getTransientClick() : 0.45f;

                    if (DrawPhotorealisticRotatingKnob("##LaserPunchKnob", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 48.0f, punch_knob_y), 18.0f, &cur_p_depth, 0.0f, 36.0f, "Laser Punch", "%.0f st")) {
                        if (synth_dsp) synth_dsp->setPitchDepth(cur_p_depth);
                    }
                    if (DrawPhotorealisticRotatingKnob("##PunchDecayKnob", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 130.0f, punch_knob_y), 18.0f, &cur_p_decay, 0.001f, 0.020f, "Decay", "%.3f s")) {
                        if (synth_dsp) synth_dsp->setPitchDecay(cur_p_decay);
                    }
                    if (DrawPhotorealisticRotatingKnob("##ClickKnob", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(c1_p0.x + 212.0f, punch_knob_y), 18.0f, &cur_click, 0.0f, 1.0f, "Click", "%.2f")) {
                        if (synth_dsp) synth_dsp->setTransientClick(cur_click);
                    }
                }

                // ── COLUNA 2: VECTOR OSCILLOSCOPE & MODELAGEM ACÚSTICA (Centro) ────────────
                ImVec2 c2_p0(c1_p1.x + card_spacing, main_top_y);
                float c2_sub1_h = 184.0f;
                float c2_sub2_h = 182.0f;

                // Sub-Card 1: VECTOR OSCILLOSCOPE
                ImVec2 sc1_p0 = c2_p0;
                ImVec2 sc1_p1(sc1_p0.x + col2_w, sc1_p0.y + c2_sub1_h);
                dl->AddRectFilled(sc1_p0, sc1_p1, IM_COL32(10, 14, 20, 255), 8.0f);
                dl->AddRect(sc1_p0, sc1_p1, IM_COL32(22, 36, 48, 255), 8.0f, 0, 1.2f);
                dl->AddText(ImVec2(sc1_p0.x + 14.0f, sc1_p0.y + 10.0f), IM_COL32(0, 229, 255, 255), "VECTOR OSCILLOSCOPE");

                DrawOscilloscope(dl, ImVec2(sc1_p0.x + 12.0f, sc1_p0.y + 30.0f), ImVec2(col2_w - 24.0f, c2_sub1_h - 42.0f));

                // Sub-Card 2: Filtro Moog (Bass) / Corpo & Saturação (Kick) / Noise & Shape (Snare)
                ImVec2 sc2_p0(sc1_p0.x, sc1_p1.y + 12.0f);
                ImVec2 sc2_p1(sc2_p0.x + col2_w, sc2_p0.y + c2_sub2_h);
                dl->AddRectFilled(sc2_p0, sc2_p1, IM_COL32(10, 14, 20, 255), 8.0f);
                dl->AddRect(sc2_p0, sc2_p1, IM_COL32(22, 36, 48, 255), 8.0f, 0, 1.2f);

                if (current_layer_tab == TAB_KICK) {
                    dl->AddText(ImVec2(sc2_p0.x + 14.0f, sc2_p0.y + 10.0f), IM_COL32(255, 160, 40, 255), "KICK BODY, DRIVE & PHASE ALIGN");
                    float k_f_y = sc2_p0.y + 70.0f;
                    float cur_k_end = synth_dsp ? synth_dsp->getKickEndFreq() : 50.0f;
                    float cur_k_amp_dec = synth_dsp ? synth_dsp->getKickAmpDecay() : 220.0f;
                    float cur_k_drv = synth_dsp ? synth_dsp->getKickDrive() : 0.60f;

                    if (DrawPhotorealisticRotatingKnob("##KickEndFreq", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 44.0f, k_f_y), 19.0f, &cur_k_end, 30.0f, 95.0f, "Sub Pitch", "%.0f Hz", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickEndFreq(cur_k_end);
                    }
                    if (DrawPhotorealisticRotatingKnob("##KickAmpDecay", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 104.0f, k_f_y), 19.0f, &cur_k_amp_dec, 80.0f, 450.0f, "Body Decay", "%.0f ms", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickAmpDecay(cur_k_amp_dec);
                    }
                    if (DrawPhotorealisticRotatingKnob("##KickDrive", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 164.0f, k_f_y), 19.0f, &cur_k_drv, 0.0f, 1.0f, "Analog Drive", "%.2f", IM_COL32(255, 140, 30, 255))) {
                        if (synth_dsp) synth_dsp->setKickDrive(cur_k_drv);
                    }
                    DrawKickDecayCurve(dl, ImVec2(sc2_p0.x + 208.0f, sc2_p0.y + 36.0f), ImVec2(col2_w - 220.0f, c2_sub2_h - 48.0f), cur_k_amp_dec);

                } else if (current_layer_tab == TAB_SNARE) {
                    dl->AddText(ImVec2(sc2_p0.x + 14.0f, sc2_p0.y + 10.0f), IM_COL32(230, 80, 210, 255), "NOISE CRUNCH & STEREO SNAP");
                    float s_f_y = sc2_p0.y + 70.0f;
                    float cur_sn_dec = synth_dsp ? synth_dsp->getSnareNoiseDecay() : 135.0f;
                    float cur_sn_snap = synth_dsp ? synth_dsp->getSnareSnap() : 0.85f;
                    float cur_sn_vol = synth_dsp ? synth_dsp->getSnareVolume() : 0.80f;

                    if (DrawPhotorealisticRotatingKnob("##S_NoiseDecay", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 50.0f, s_f_y), 19.0f, &cur_sn_dec, 50.0f, 350.0f, "Noise Decay", "%.0f ms", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareNoiseDecay(cur_sn_dec);
                    }
                    if (DrawPhotorealisticRotatingKnob("##S_SnapLevel", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 120.0f, s_f_y), 19.0f, &cur_sn_snap, 0.0f, 1.0f, "Snap Transient", "%.2f", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareSnap(cur_sn_snap);
                    }
                    if (DrawPhotorealisticRotatingKnob("##S_MasterVol", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 190.0f, s_f_y), 19.0f, &cur_sn_vol, 0.0f, 1.5f, "Snare Level", "%.2f", IM_COL32(230, 70, 200, 255))) {
                        if (synth_dsp) synth_dsp->setSnareVolume(cur_sn_vol);
                    }
                    DrawKickDecayCurve(dl, ImVec2(sc2_p0.x + 230.0f, sc2_p0.y + 36.0f), ImVec2(col2_w - 242.0f, c2_sub2_h - 48.0f), cur_sn_dec);

                } else { // TAB_BASS ou TAB_FULL_BASE
                    dl->AddText(ImVec2(sc2_p0.x + 14.0f, sc2_p0.y + 10.0f), IM_COL32(170, 205, 230, 255), "24dB Moog Ladder Low-Pass Filter");
                    float filter_knob_y = sc2_p0.y + 70.0f;
                    float cur_cutoff = synth_dsp ? synth_dsp->getCutoff() : 550.0f;
                    float cur_reso = synth_dsp ? synth_dsp->getResonance() : 0.42f;
                    float cur_hpf = synth_dsp ? synth_dsp->getHpfCutoff() : 32.0f;

                    if (DrawPhotorealisticRotatingKnob("##CutoffKnob", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 44.0f, filter_knob_y), 19.0f, &cur_cutoff, 40.0f, 6000.0f, "Cutoff", "%.0f Hz")) {
                        if (synth_dsp) synth_dsp->setCutoff(cur_cutoff);
                    }
                    if (DrawPhotorealisticRotatingKnob("##ResoKnob", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 104.0f, filter_knob_y), 19.0f, &cur_reso, 0.0f, 0.95f, "Resonance", "%.2f")) {
                        if (synth_dsp) synth_dsp->setResonance(cur_reso);
                    }
                    if (DrawPhotorealisticRotatingKnob("##HpfLowCutKnob", dl, g_rolling_bass_tex_pack.tex_knob_metallic, ImVec2(sc2_p0.x + 164.0f, filter_knob_y), 19.0f, &cur_hpf, 15.0f, 80.0f, "Sub Low-Cut", "%.0f Hz")) {
                        if (synth_dsp) synth_dsp->setHpfCutoff(cur_hpf);
                    }
                    float cur_decay = synth_dsp ? synth_dsp->getEnvDecay() : 0.085f;
                    DrawFilterEnvelopeCurve(dl, ImVec2(sc2_p0.x + 208.0f, sc2_p0.y + 36.0f), ImVec2(col2_w - 220.0f, c2_sub2_h - 48.0f), cur_decay);
                }

                // ── COLUNA 3: PATTERN GENERATOR & MULTI-LANE 16-STEP MATRIX (Direita) ─────
                ImVec2 c3_p0(c2_p1.x + card_spacing, main_top_y);
                ImVec2 c3_p1(c3_p0.x + col3_w, c3_p0.y + card_h);
                dl->AddRectFilled(c3_p0, c3_p1, IM_COL32(10, 14, 20, 255), 8.0f);
                dl->AddRect(c3_p0, c3_p1, IM_COL32(22, 36, 48, 255), 8.0f, 0, 1.2f);

                ImVec2 t3_sz = ImGui::CalcTextSize("PATTERN GENERATOR");
                dl->AddText(ImVec2(c3_p0.x + (col3_w - t3_sz.x) * 0.5f, c3_p0.y + 12.0f), IM_COL32(160, 195, 220, 255), "PATTERN GENERATOR");

                float pbtn_w = (col3_w - 36.0f) * 0.5f;
                float pbtn_h = 24.0f;
                float pbtn_y0 = c3_p0.y + 34.0f;
                float pbtn_y1 = pbtn_y0 + pbtn_h + 6.0f;

                if (DrawPatternBtn("KBBB 16th Roll", ImVec2(c3_p0.x + 14.0f, pbtn_y0), ImVec2(pbtn_w, pbtn_h), current_pattern_type == 0)) {
                    applyPatternType(0);
                }
                if (DrawPatternBtn("KBB Galop", ImVec2(c3_p0.x + 22.0f + pbtn_w, pbtn_y0), ImVec2(pbtn_w, pbtn_h), current_pattern_type == 1)) {
                    applyPatternType(1);
                }
                if (DrawPatternBtn("KB Offbeat", ImVec2(c3_p0.x + 14.0f, pbtn_y1), ImVec2(pbtn_w, pbtn_h), current_pattern_type == 2)) {
                    applyPatternType(2);
                }
                if (DrawPatternBtn("Octave Jump", ImVec2(c3_p0.x + 22.0f + pbtn_w, pbtn_y1), ImVec2(pbtn_w, pbtn_h), current_pattern_type == 3)) {
                    applyPatternType(3);
                }

                float matrix_y = pbtn_y1 + pbtn_h + 16.0f;
                float matrix_h = card_h - (matrix_y - c3_p0.y) - 14.0f;
                DrawMultiLane16StepMatrix(dl, ImVec2(c3_p0.x + 14.0f, matrix_y), ImVec2(col3_w - 28.0f, matrix_h));

                // ──────────────────────────────────────────────────────────────────────────
                // 3. BOTTOM ACTION BAR (PISTAS SEPARADAS & BASE COMPLETA)
                // ──────────────────────────────────────────────────────────────────────────
                float bot_btn_y = main_top_y + card_h + 12.0f;
                float bot_btn_h = 44.0f;

                // 1. GEN KICK (Track 0)
                float b_k_x = win_pos.x + 20.0f;
                float b_k_w = 168.0f;
                ImVec2 bk_pos(b_k_x, bot_btn_y);
                ImGui::SetCursorScreenPos(bk_pos);
                ImGuiID btn_k_id = ImGui::GetID("##BtnGenKick");
                ImRect bk_bb(bk_pos, ImVec2(bk_pos.x + b_k_w, bk_pos.y + bot_btn_h));
                ImGui::ItemSize(bk_bb);
                if (ImGui::ItemAdd(bk_bb, btn_k_id)) {
                    bool hov, held;
                    if (ImGui::ButtonBehavior(bk_bb, btn_k_id, &hov, &held)) {
                        auto k_notes = generateKickNotes(bpm, current_bars_to_generate);
                        if (!clip_manager.global_patterns.empty()) {
                            int pat_idx = clip_manager.current_pattern_idx;
                            clip_manager.global_patterns[pat_idx].getChannelNotes(0) = k_notes;
                            toast_message = "🥊 Pista de Kick (Track 0) gerada com sucesso!";
                            toast_timer = 3.5f;
                            show_piano_roll = true;
                            g_need_focus_piano_roll = true;
                        }
                    }
                    ImU32 bg_c = hov ? IM_COL32(36, 22, 10, 255) : IM_COL32(22, 14, 8, 255);
                    ImU32 b_c = hov ? IM_COL32(255, 170, 40, 255) : IM_COL32(230, 130, 20, 255);
                    dl->AddRectFilled(bk_bb.Min, bk_bb.Max, bg_c, 8.0f);
                    dl->AddRect(bk_bb.Min, bk_bb.Max, b_c, 8.0f, 0, 1.6f);
                    const char* txt = "🥊 GEN KICK (TRK 0)";
                    ImVec2 t_sz = ImGui::CalcTextSize(txt);
                    dl->AddText(ImVec2(bk_bb.Min.x + (b_k_w - t_sz.x) * 0.5f, bk_bb.Min.y + (bot_btn_h - t_sz.y) * 0.5f), b_c, txt);
                }

                // 2. GEN BASS (Track 1)
                float b_b_x = b_k_x + b_k_w + 10.0f;
                float b_b_w = 168.0f;
                ImVec2 bb_pos(b_b_x, bot_btn_y);
                ImGui::SetCursorScreenPos(bb_pos);
                ImGuiID btn_b_id = ImGui::GetID("##BtnGenBass");
                ImRect bb_rect(bb_pos, ImVec2(bb_pos.x + b_b_w, bb_pos.y + bot_btn_h));
                ImGui::ItemSize(bb_rect);
                if (ImGui::ItemAdd(bb_rect, btn_b_id)) {
                    bool hov, held;
                    if (ImGui::ButtonBehavior(bb_rect, btn_b_id, &hov, &held)) {
                        auto b_notes = generateBassNotes(bpm, current_bars_to_generate);
                        if (!clip_manager.global_patterns.empty()) {
                            int pat_idx = clip_manager.current_pattern_idx;
                            clip_manager.global_patterns[pat_idx].getChannelNotes(1) = b_notes;
                            toast_message = "⚡ Pista de Rolling Bass (Track 1) gerada com sucesso!";
                            toast_timer = 3.5f;
                            show_piano_roll = true;
                            g_need_focus_piano_roll = true;
                        }
                    }
                    ImU32 bg_c = hov ? IM_COL32(10, 36, 20, 255) : IM_COL32(6, 22, 14, 255);
                    ImU32 b_c = hov ? IM_COL32(57, 255, 140, 255) : IM_COL32(40, 220, 110, 255);
                    dl->AddRectFilled(bb_rect.Min, bb_rect.Max, bg_c, 8.0f);
                    dl->AddRect(bb_rect.Min, bb_rect.Max, b_c, 8.0f, 0, 1.6f);
                    const char* txt = "⚡ GEN BASS (TRK 1)";
                    ImVec2 t_sz = ImGui::CalcTextSize(txt);
                    dl->AddText(ImVec2(bb_rect.Min.x + (b_b_w - t_sz.x) * 0.5f, bb_rect.Min.y + (bot_btn_h - t_sz.y) * 0.5f), b_c, txt);
                }

                // 3. GEN SNARE (Track 2)
                float b_s_x = b_b_x + b_b_w + 10.0f;
                float b_s_w = 168.0f;
                ImVec2 bs_pos(b_s_x, bot_btn_y);
                ImGui::SetCursorScreenPos(bs_pos);
                ImGuiID btn_s_id = ImGui::GetID("##BtnGenSnare");
                ImRect bs_rect(bs_pos, ImVec2(bs_pos.x + b_s_w, bs_pos.y + bot_btn_h));
                ImGui::ItemSize(bs_rect);
                if (ImGui::ItemAdd(bs_rect, btn_s_id)) {
                    bool hov, held;
                    if (ImGui::ButtonBehavior(bs_rect, btn_s_id, &hov, &held)) {
                        auto s_notes = generateSnareNotes(bpm, current_bars_to_generate);
                        if (!clip_manager.global_patterns.empty()) {
                            int pat_idx = clip_manager.current_pattern_idx;
                            clip_manager.global_patterns[pat_idx].getChannelNotes(2) = s_notes;
                            toast_message = "💥 Pista de Snare / Clap (Track 2) gerada com sucesso!";
                            toast_timer = 3.5f;
                            show_piano_roll = true;
                            g_need_focus_piano_roll = true;
                        }
                    }
                    ImU32 bg_c = hov ? IM_COL32(32, 10, 30, 255) : IM_COL32(20, 6, 20, 255);
                    ImU32 b_c = hov ? IM_COL32(245, 80, 220, 255) : IM_COL32(210, 50, 185, 255);
                    dl->AddRectFilled(bs_rect.Min, bs_rect.Max, bg_c, 8.0f);
                    dl->AddRect(bs_rect.Min, bs_rect.Max, b_c, 8.0f, 0, 1.6f);
                    const char* txt = "💥 GEN SNARE (TRK 2)";
                    ImVec2 t_sz = ImGui::CalcTextSize(txt);
                    dl->AddText(ImVec2(bs_rect.Min.x + (b_s_w - t_sz.x) * 0.5f, bs_rect.Min.y + (bot_btn_h - t_sz.y) * 0.5f), b_c, txt);
                }

                // 4. GENERATE FULL BASE (ALL TRACKS 0, 1, 2) - HERO GLOW BUTTON
                float b_all_x = b_s_x + b_s_w + 10.0f;
                float b_all_w = 310.0f;
                ImVec2 ball_pos(b_all_x, bot_btn_y);
                ImGui::SetCursorScreenPos(ball_pos);
                ImGuiID btn_all_id = ImGui::GetID("##BtnGenFullBase");
                ImRect ball_rect(ball_pos, ImVec2(ball_pos.x + b_all_w, ball_pos.y + bot_btn_h));
                ImGui::ItemSize(ball_rect);
                if (ImGui::ItemAdd(ball_rect, btn_all_id)) {
                    bool hov, held;
                    if (ImGui::ButtonBehavior(ball_rect, btn_all_id, &hov, &held)) {
                        auto k_notes = generateKickNotes(bpm, current_bars_to_generate);
                        auto b_notes = generateBassNotes(bpm, current_bars_to_generate);
                        auto s_notes = generateSnareNotes(bpm, current_bars_to_generate);
                        if (!clip_manager.global_patterns.empty()) {
                            int pat_idx = clip_manager.current_pattern_idx;
                            clip_manager.global_patterns[pat_idx].getChannelNotes(0) = k_notes;
                            clip_manager.global_patterns[pat_idx].getChannelNotes(1) = b_notes;
                            clip_manager.global_patterns[pat_idx].getChannelNotes(2) = s_notes;
                            toast_message = "🚀 Base Completa (Kick + Bass + Snare) gerada nas Tracks 0, 1 e 2!";
                            toast_timer = 4.0f;
                            show_piano_roll = true;
                            g_need_focus_piano_roll = true;
                        }
                    }
                    ImU32 bg_c = hov ? IM_COL32(12, 42, 54, 255) : IM_COL32(8, 26, 36, 255);
                    ImU32 b_c = hov ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 220, 240, 255);
                    dl->AddRectFilled(ball_rect.Min, ball_rect.Max, bg_c, 8.0f);
                    dl->AddRect(ball_rect.Min, ball_rect.Max, b_c, 8.0f, 0, 2.0f);
                    dl->AddRect(ImVec2(ball_rect.Min.x - 1, ball_rect.Min.y - 1), ImVec2(ball_rect.Max.x + 1, ball_rect.Max.y + 1), IM_COL32(0, 229, 255, 60), 9.0f, 0, 2.0f);
                    const char* txt = "🚀 GEN FULL BASE (ALL TRACKS)";
                    ImVec2 t_sz = ImGui::CalcTextSize(txt);
                    dl->AddText(ImVec2(ball_rect.Min.x + (b_all_w - t_sz.x) * 0.5f, ball_rect.Min.y + (bot_btn_h - t_sz.y) * 0.5f), b_c, txt);
                }

                // 5. TO PLAYLIST
                float b_pl_x = b_all_x + b_all_w + 10.0f;
                float b_pl_w = win_pos.x + win_size.x - 20.0f - b_pl_x;
                ImVec2 bpl_pos(b_pl_x, bot_btn_y);
                ImGui::SetCursorScreenPos(bpl_pos);
                ImGuiID btn_pl_id = ImGui::GetID("##BtnGenPlaylist");
                ImRect bpl_rect(bpl_pos, ImVec2(bpl_pos.x + b_pl_w, bpl_pos.y + bot_btn_h));
                ImGui::ItemSize(bpl_rect);
                if (ImGui::ItemAdd(bpl_rect, btn_pl_id)) {
                    bool hov, held;
                    if (ImGui::ButtonBehavior(bpl_rect, btn_pl_id, &hov, &held)) {
                        auto k_notes = generateKickNotes(bpm, current_bars_to_generate);
                        auto b_notes = generateBassNotes(bpm, current_bars_to_generate);
                        auto s_notes = generateSnareNotes(bpm, current_bars_to_generate);

                        Pattern new_pat;
                        new_pat.id = clip_manager.next_id++;
                        new_pat.name = "Psy K&B " + std::string(root_note_short[current_root_idx]);
                        new_pat.color = 0xFF00FFCC;
                        new_pat.getChannelNotes(0) = k_notes;
                        new_pat.getChannelNotes(1) = b_notes;
                        new_pat.getChannelNotes(2) = s_notes;
                        clip_manager.global_patterns.push_back(new_pat);

                        float beat_sec = 60.0f / (bpm > 20.0f ? bpm : 138.0f);
                        float len_sec = beat_sec * 4.0f * current_bars_to_generate;

                        MidiClip mclip;
                        mclip.id = clip_manager.next_id++;
                        mclip.pattern_id = new_pat.id;
                        mclip.start_time_sec = 0.0f;
                        mclip.length_sec = len_sec;
                        mclip.name = new_pat.name;

                        clip_manager.track_midi_clips[0].push_back(mclip);
                        clip_manager.track_midi_clips[1].push_back(mclip);
                        clip_manager.track_midi_clips[2].push_back(mclip);

                        toast_message = "✨ Base Completa (Kick, Bass e Snare) adicionada à Playlist!";
                        toast_timer = 3.5f;
                    }
                    ImU32 bg_c = hov ? IM_COL32(10, 24, 34, 255) : IM_COL32(6, 16, 24, 255);
                    ImU32 b_c = hov ? IM_COL32(0, 210, 240, 255) : IM_COL32(0, 160, 200, 255);
                    dl->AddRectFilled(bpl_rect.Min, bpl_rect.Max, bg_c, 8.0f);
                    dl->AddRect(bpl_rect.Min, bpl_rect.Max, b_c, 8.0f, 0, 1.4f);
                    const char* txt = "📑 PLAYLIST";
                    ImVec2 t_sz = ImGui::CalcTextSize(txt);
                    dl->AddText(ImVec2(bpl_rect.Min.x + (b_pl_w - t_sz.x) * 0.5f, bpl_rect.Min.y + (bot_btn_h - t_sz.y) * 0.5f), b_c, txt);
                }

                // Toast Notification Overlay
                if (toast_timer > 0.0f) {
                    toast_timer -= ImGui::GetIO().DeltaTime;
                    ImVec2 toast_sz = ImGui::CalcTextSize(toast_message.c_str());
                    ImVec2 toast_p(win_pos.x + (win_size.x - toast_sz.x) * 0.5f, bot_btn_y - 20.0f);
                    dl->AddRectFilled(ImVec2(toast_p.x - 8, toast_p.y - 3), ImVec2(toast_p.x + toast_sz.x + 8, toast_p.y + toast_sz.y + 3), IM_COL32(10, 24, 18, 240), 4.0f);
                    dl->AddText(toast_p, IM_COL32(57, 255, 140, 255), toast_message.c_str());
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);
        }
    };
}
"""

with open(target_file, "w", encoding="utf-8") as f:
    f.write(content)

print(f"File {target_file} written successfully! Total bytes: {len(content)}")
