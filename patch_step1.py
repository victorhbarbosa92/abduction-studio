import os, sys

path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h"
bak_path = path + ".bak"

with open(path, "r", encoding="utf-8") as f:
    code = f.read()

# 1. Add Customizer Fields right after show_export_modal
target_decl = "        bool show_export_modal = false;"
customizer_fields = """        bool show_export_modal = false;

        // =========================================================================
        // SISTEMA DE AUTONOMIA TOTAL DE LAYOUT E DESIGNER UI/UX
        // =========================================================================
        bool show_ui_designer_modal = false;
        int ui_layout_preset = 0; // 0 = Rekordbox Default, 1 = Customizado, 2 = Compacto, 3 = Expandido 4K

        // 1. Formas de Onda (Waveforms)
        float wave_height_ratio = 0.28f;
        int wave_line_thickness = 4; // 1 a 8 texels
        float custom_wave_color_bass[3] = { 0.0f, 0.45f, 1.0f };   // Graves (Bass)
        float custom_wave_color_mid[3]  = { 1.0f, 0.55f, 0.0f };   // Médios (Mid)
        float custom_wave_color_high[3] = { 0.95f, 0.95f, 0.95f }; // Agudos (High)

        // 2. Jogs & Decks
        float jog_size_multiplier = 1.0f; // 0.5x a 1.8x
        float deck_upper_ratio = 0.58f;    // 0.40x a 0.75x
        bool show_jog_needle = true;
        bool show_vinyl_texture = true;

        // 3. Knobs & Efeitos
        float knob_size_multiplier = 1.0f; // 0.6x a 1.7x
        bool fx_click_toggle_enabled = true; // Ativação dos efeitos com clique no knob!

        // 4. Mixer Central
        float mixer_width_ratio = 0.20f;   // 0.12x a 0.35x
        bool show_central_frequency_wave = true;
        bool show_headphone_cue_bar = true;

        // 5. Performance Pads
        float pad_height_scale = 1.0f;     // 0.6x a 2.0x
        int pad_layout_mode = 0;           // 0 = 8 Pads (2x4), 1 = 4 Pads (1x4)
        bool show_pad_labels = true;

        // 6. Biblioteca & Pista no Rodapé
        float library_split_ratio = 0.36f; // 0.15x a 0.65x

        void applyLayoutPreset(int preset_id) {
            ui_layout_preset = preset_id;
            if (preset_id == 0) {
                // PADRÃO REKORDBOX OFICIAL
                waveform_height = 180.0f;
                wave_height_ratio = 0.28f;
                waveform_zoom_bars = 4;
                waveform_color_mode = 0;
                waveform_render_style = 2;
                wave_line_thickness = 4;
                jog_size_multiplier = 1.0f;
                deck_upper_ratio = 0.58f;
                knob_size_multiplier = 1.0f;
                mixer_width_ratio = 0.20f;
                pad_height_scale = 1.0f;
                pad_layout_mode = 0;
                library_split_ratio = 0.36f;
                show_central_frequency_wave = true;
                show_headphone_cue_bar = true;
                fx_click_toggle_enabled = true;
                font_accessibility_scale = 1.15f;
            } else if (preset_id == 1) {
                // CARREGAR MEU LAYOUT SALVO
                loadCustomLayout();
            } else if (preset_id == 2) {
                // COMPACTO (NOTEBOOK / 720p / 768p)
                waveform_height = 120.0f;
                wave_height_ratio = 0.22f;
                waveform_zoom_bars = 4;
                waveform_color_mode = 0;
                wave_line_thickness = 3;
                jog_size_multiplier = 0.82f;
                deck_upper_ratio = 0.54f;
                knob_size_multiplier = 0.85f;
                mixer_width_ratio = 0.16f;
                pad_height_scale = 0.80f;
                library_split_ratio = 0.30f;
                show_central_frequency_wave = false;
                show_headphone_cue_bar = true;
                fx_click_toggle_enabled = true;
                font_accessibility_scale = 1.0f;
            } else if (preset_id == 3) {
                // EXPANDIDO (ULTRA-WIDE / 2K / 4K)
                waveform_height = 240.0f;
                wave_height_ratio = 0.32f;
                waveform_zoom_bars = 4;
                waveform_color_mode = 0;
                wave_line_thickness = 5;
                jog_size_multiplier = 1.35f;
                deck_upper_ratio = 0.62f;
                knob_size_multiplier = 1.25f;
                mixer_width_ratio = 0.24f;
                pad_height_scale = 1.30f;
                library_split_ratio = 0.40f;
                show_central_frequency_wave = true;
                show_headphone_cue_bar = true;
                fx_click_toggle_enabled = true;
                font_accessibility_scale = 1.30f;
            }
        }

        void saveCustomLayout(const std::string& path = "scratch/dj_ui_custom_layout.json") {
            try {
                std::filesystem::create_directories("scratch");
                std::ofstream f(path);
                if (f.is_open()) {
                    f << "{\\n";
                    f << "  \\\"ui_layout_preset\\\": " << ui_layout_preset << ",\\n";
                    f << "  \\\"waveform_height\\\": " << waveform_height << ",\\n";
                    f << "  \\\"wave_height_ratio\\\": " << wave_height_ratio << ",\\n";
                    f << "  \\\"waveform_zoom_bars\\\": " << waveform_zoom_bars << ",\\n";
                    f << "  \\\"waveform_color_mode\\\": " << waveform_color_mode << ",\\n";
                    f << "  \\\"waveform_render_style\\\": " << waveform_render_style << ",\\n";
                    f << "  \\\"wave_line_thickness\\\": " << wave_line_thickness << ",\\n";
                    f << "  \\\"custom_wave_color_bass\\\": [" << custom_wave_color_bass[0] << ", " << custom_wave_color_bass[1] << ", " << custom_wave_color_bass[2] << "],\\n";
                    f << "  \\\"custom_wave_color_mid\\\": [" << custom_wave_color_mid[0] << ", " << custom_wave_color_mid[1] << ", " << custom_wave_color_mid[2] << "],\\n";
                    f << "  \\\"custom_wave_color_high\\\": [" << custom_wave_color_high[0] << ", " << custom_wave_color_high[1] << ", " << custom_wave_color_high[2] << "],\\n";
                    f << "  \\\"jog_size_multiplier\\\": " << jog_size_multiplier << ",\\n";
                    f << "  \\\"deck_upper_ratio\\\": " << deck_upper_ratio << ",\\n";
                    f << "  \\\"knob_size_multiplier\\\": " << knob_size_multiplier << ",\\n";
                    f << "  \\\"mixer_width_ratio\\\": " << mixer_width_ratio << ",\\n";
                    f << "  \\\"pad_height_scale\\\": " << pad_height_scale << ",\\n";
                    f << "  \\\"pad_layout_mode\\\": " << pad_layout_mode << ",\\n";
                    f << "  \\\"library_split_ratio\\\": " << library_split_ratio << ",\\n";
                    f << "  \\\"show_central_frequency_wave\\\": " << (show_central_frequency_wave ? "true" : "false") << ",\\n";
                    f << "  \\\"show_headphone_cue_bar\\\": " << (show_headphone_cue_bar ? "true" : "false") << ",\\n";
                    f << "  \\\"fx_click_toggle_enabled\\\": " << (fx_click_toggle_enabled ? "true" : "false") << ",\\n";
                    f << "  \\\"font_accessibility_scale\\\": " << font_accessibility_scale << "\\n";
                    f << "}\\n";
                }
            } catch (...) {}
        }

        void loadCustomLayout(const std::string& path = "scratch/dj_ui_custom_layout.json") {
            try {
                if (!std::filesystem::exists(path)) return;
                std::ifstream f(path);
                if (!f.is_open()) return;
                std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                auto getVal = [&](const std::string& key) -> float {
                    size_t kp = s.find("\\\"" + key + "\\\":");
                    if (kp == std::string::npos) return -9999.0f;
                    size_t vs = s.find_first_not_of(" \\t", kp + key.length() + 3);
                    if (vs == std::string::npos) return -9999.0f;
                    size_t ve = s.find_first_of(",}\\r\\n", vs);
                    if (ve == std::string::npos) return -9999.0f;
                    return (float)atof(s.substr(vs, ve - vs).c_str());
                };
                auto getBool = [&](const std::string& key, bool def) -> bool {
                    size_t kp = s.find("\\\"" + key + "\\\":");
                    if (kp == std::string::npos) return def;
                    size_t vs = s.find_first_not_of(" \\t", kp + key.length() + 3);
                    if (vs == std::string::npos) return def;
                    return s.substr(vs, 4) == "true";
                };

                float v;
                v = getVal("ui_layout_preset"); if (v >= 0.0f) ui_layout_preset = (int)v;
                v = getVal("waveform_height"); if (v >= 50.0f) waveform_height = v;
                v = getVal("wave_height_ratio"); if (v > 0.05f) wave_height_ratio = v;
                v = getVal("waveform_zoom_bars"); if (v >= 1.0f) waveform_zoom_bars = (int)v;
                v = getVal("waveform_color_mode"); if (v >= 0.0f) waveform_color_mode = (int)v;
                v = getVal("waveform_render_style"); if (v >= 0.0f) waveform_render_style = (int)v;
                v = getVal("wave_line_thickness"); if (v >= 1.0f) wave_line_thickness = (int)v;
                v = getVal("jog_size_multiplier"); if (v >= 0.3f) jog_size_multiplier = v;
                v = getVal("deck_upper_ratio"); if (v >= 0.2f) deck_upper_ratio = v;
                v = getVal("knob_size_multiplier"); if (v >= 0.3f) knob_size_multiplier = v;
                v = getVal("mixer_width_ratio"); if (v >= 0.05f) mixer_width_ratio = v;
                v = getVal("pad_height_scale"); if (v >= 0.3f) pad_height_scale = v;
                v = getVal("pad_layout_mode"); if (v >= 0.0f) pad_layout_mode = (int)v;
                v = getVal("library_split_ratio"); if (v >= 0.10f) library_split_ratio = v;
                show_central_frequency_wave = getBool("show_central_frequency_wave", true);
                show_headphone_cue_bar = getBool("show_headphone_cue_bar", true);
                fx_click_toggle_enabled = getBool("fx_click_toggle_enabled", true);
                v = getVal("font_accessibility_scale"); if (v >= 0.5f) font_accessibility_scale = v;
            } catch (...) {}
        }
"""

if target_decl not in code:
    print("ERROR: target_decl not found")
    sys.exit(1)

code = code.replace(target_decl, customizer_fields, 1)

# 2. In init(), load custom layout
target_init = "            // Etapa 3: Carrega a base local de artistas seguidos no Radar\n            loadFollowedArtists();"
init_add = """            // Etapa 3: Carrega a base local de artistas seguidos no Radar
            loadFollowedArtists();

            // Carrega layout e customizações de UI/UX do usuário
            loadCustomLayout();"""

if target_init not in code:
    print("ERROR: target_init not found")
    sys.exit(1)

code = code.replace(target_init, init_add, 1)

# Write to file
if os.path.exists(bak_path):
    os.remove(bak_path)
os.rename(path, bak_path)
with open(path, "w", encoding="utf-8") as f:
    f.write(code)
os.remove(bak_path)
print("SUCCESS part 1")

