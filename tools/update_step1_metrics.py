import sys
import os

path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\DJLayoutMetrics.h'

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Update DJSelectableWidgetId
old_enum = """    enum DJSelectableWidgetId {
        WIDGET_NONE = 0,
        WIDGET_MIXER_CONSOLE = 1,
        WIDGET_KNOBS_EQ = 2,
        WIDGET_CHANNEL_FADERS = 3,
        WIDGET_CROSSFADER = 4,
        WIDGET_JOGWHEEL = 5,
        WIDGET_PADS = 6,
        WIDGET_TRANSPORT = 7,
        WIDGET_WAVEFORMS = 8,
        WIDGET_FX_BAR = 9
    };"""

new_enum = """    enum DJSelectableWidgetId {
        WIDGET_NONE = 0,
        WIDGET_MIXER_CONSOLE = 1,
        WIDGET_KNOBS_EQ = 2,
        WIDGET_CHANNEL_FADERS = 3,
        WIDGET_CROSSFADER = 4,
        WIDGET_JOGWHEEL = 5,
        WIDGET_PADS = 6,
        WIDGET_TRANSPORT = 7,
        WIDGET_WAVEFORMS = 8,
        WIDGET_FX_BAR = 9,
        WIDGET_HEADPHONES = 10
    };"""

assert old_enum in c, "old_enum not found"
c = c.replace(old_enum, new_enum)

# 2. Add new metrics variables
old_vars = """        float mixer_knobs_offset_y = 0.0f;       // Deslocamento Y do bloco de knobs
        float mixer_faders_offset_y = 0.0f;      // Deslocamento Y do bloco de faders"""

new_vars = """        float mixer_knobs_offset_y = 0.0f;       // Deslocamento Y do bloco de knobs
        float mixer_faders_offset_y = 0.0f;      // Deslocamento Y do bloco de faders
        float headphone_offset_x = 0.0f;         // Deslocamento X da secao de fone
        float headphone_offset_y = 0.0f;         // Deslocamento Y da secao de fone
        float headphone_bar_width = 240.0f;      // Largura da barra de fone
        float headphone_bar_height = 24.0f;      // Altura da barra de fone
        float comp_knob_scale = 1.35f;           // Escala do knob dourado de compressor
        float transport_offset_x = 0.0f;         // Deslocamento X do bloco CUE/PLAY
        float transport_offset_y = 0.0f;         // Deslocamento Y do bloco CUE/PLAY
        float cue_play_gap = 8.0f;               // Espacamento minimo entre CUE e PLAY
        int   pad_grid_mode = 0;                 // 0: 8 Pads (4x2), 1: 16 Pads (8x2 Wide para TV 65 pol), 2: 16 Pads (4x4)
        float ui_font_scale = 1.0f;              // Escala adicional de fontes para legibilidade na TV (0.7x a 1.6x)
        float waveform_toolbar_width = 38.0f;    // Largura ergonomica da barra lateral de zoom/modos da waveform
        bool  enable_collision_physics = true;   // Fisica de colisao AABB ativada"""

assert old_vars in c, "old_vars not found"
c = c.replace(old_vars, new_vars)

# 3. Update resetToDefaults
old_reset = """            mixer_knobs_offset_y = 0.0f;
            mixer_faders_offset_y = 0.0f;
            selected_edit_widget = WIDGET_NONE;"""

new_reset = """            mixer_knobs_offset_y = 0.0f;
            mixer_faders_offset_y = 0.0f;
            headphone_offset_x = 0.0f;
            headphone_offset_y = 0.0f;
            headphone_bar_width = 240.0f;
            headphone_bar_height = 24.0f;
            comp_knob_scale = 1.35f;
            transport_offset_x = 0.0f;
            transport_offset_y = 0.0f;
            cue_play_gap = 8.0f;
            pad_grid_mode = 0;
            ui_font_scale = 1.0f;
            waveform_toolbar_width = 38.0f;
            enable_collision_physics = true;
            selected_edit_widget = WIDGET_NONE;"""

assert old_reset in c, "old_reset not found"
c = c.replace(old_reset, new_reset)

# 4. Update saveToFile
old_save = """                f << "  \\"mixer_knobs_offset_y\\": " << mixer_knobs_offset_y << ",\\n";
                f << "  \\"mixer_faders_offset_y\\": " << mixer_faders_offset_y << "\\n";
                f << "}\\n";"""

new_save = """                f << "  \\"mixer_knobs_offset_y\\": " << mixer_knobs_offset_y << ",\\n";
                f << "  \\"mixer_faders_offset_y\\": " << mixer_faders_offset_y << ",\\n";
                f << "  \\"headphone_offset_x\\": " << headphone_offset_x << ",\\n";
                f << "  \\"headphone_offset_y\\": " << headphone_offset_y << ",\\n";
                f << "  \\"headphone_bar_width\\": " << headphone_bar_width << ",\\n";
                f << "  \\"headphone_bar_height\\": " << headphone_bar_height << ",\\n";
                f << "  \\"comp_knob_scale\\\": " << comp_knob_scale << ",\\n";
                f << "  \\"transport_offset_x\\": " << transport_offset_x << ",\\n";
                f << "  \\"transport_offset_y\\": " << transport_offset_y << ",\\n";
                f << "  \\"cue_play_gap\\": " << cue_play_gap << ",\\n";
                f << "  \\"pad_grid_mode\\": " << pad_grid_mode << ",\\n";
                f << "  \\"ui_font_scale\\": " << ui_font_scale << ",\\n";
                f << "  \\"waveform_toolbar_width\\": " << waveform_toolbar_width << ",\\n";
                f << "  \\"enable_collision_physics\\": " << (enable_collision_physics ? "true" : "false") << "\\n";
                f << "}\\n";"""

assert old_save in c, "old_save not found"
c = c.replace(old_save, new_save)

# 5. Update loadFromFile
old_load = """                mixer_knobs_offset_y = JSON::parseFloat(json, "mixer_knobs_offset_y", mixer_knobs_offset_y);
                mixer_faders_offset_y = JSON::parseFloat(json, "mixer_faders_offset_y", mixer_faders_offset_y);
                f.close();"""

new_load = """                mixer_knobs_offset_y = JSON::parseFloat(json, "mixer_knobs_offset_y", mixer_knobs_offset_y);
                mixer_faders_offset_y = JSON::parseFloat(json, "mixer_faders_offset_y", mixer_faders_offset_y);
                headphone_offset_x = JSON::parseFloat(json, "headphone_offset_x", headphone_offset_x);
                headphone_offset_y = JSON::parseFloat(json, "headphone_offset_y", headphone_offset_y);
                headphone_bar_width = JSON::parseFloat(json, "headphone_bar_width", headphone_bar_width);
                headphone_bar_height = JSON::parseFloat(json, "headphone_bar_height", headphone_bar_height);
                comp_knob_scale = JSON::parseFloat(json, "comp_knob_scale", comp_knob_scale);
                transport_offset_x = JSON::parseFloat(json, "transport_offset_x", transport_offset_x);
                transport_offset_y = JSON::parseFloat(json, "transport_offset_y", transport_offset_y);
                cue_play_gap = JSON::parseFloat(json, "cue_play_gap", cue_play_gap);
                pad_grid_mode = JSON::parseInt(json, "pad_grid_mode", pad_grid_mode);
                ui_font_scale = JSON::parseFloat(json, "ui_font_scale", ui_font_scale);
                waveform_toolbar_width = JSON::parseFloat(json, "waveform_toolbar_width", waveform_toolbar_width);
                enable_collision_physics = JSON::parseBool(json, "enable_collision_physics", enable_collision_physics);
                f.close();"""

assert old_load in c, "old_load not found"
c = c.replace(old_load, new_load)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("DJLayoutMetrics.h successfully updated!")

