#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include "../utils/JSONHelper.h"

namespace KuroUI {

    enum class DJPresetLayoutType {
        CLUB_STANDARD_2DECK = 0,
        TURNTABLIST_BATTLE_12INCH = 1,
        TECHNO_STUDIO_4DECK = 2,
        ULTRA_COMPACT_LAPTOP = 3,
        ULTRAWIDE_4K_HERO = 4
    };

    enum DJSelectableWidgetId {
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
        WIDGET_HEADPHONES = 10,
        WIDGET_TEMPO_PANEL = 11
    };

    class DJLayoutMetrics {
    public:
        // =========================================================================
        // 1. ESCALA GERAL E MODOS DE VISUALIZAÇÃO
        // =========================================================================
        float master_scale = 1.15f;              // Multiplicador global expandido (0.50f a 3.00f)
        int   deck_view_mode = 0;               // 0: Dual Layer (2+2 Club) | 1: 4 Decks Horizontal | 2: 4 Decks Vertical
        bool  edit_mode_active = false;         // Modo Edição Interativo WYSIWYG (Ctrl + E)
        bool  snap_editor_to_bottom = true;     // Garante que o painel do editor sempre abra encaixado no rodapé (anti-obstrução)
        int   selected_edit_widget = 0;         // 0: Nenhum, 1: Vinil/Jog, 2: Mixer, 3: Knobs, 4: Pads, 5: Faders, 6: Waveforms
        int   requested_tab_idx = -1;           // Aba forçada para seleção programática / teste (-1 = nenhuma)

        // =========================================================================
        // 2. MIXER CENTRAL E FAIXA DE CANAL
        // =========================================================================
        float mixer_width = 320.0f;              // Largura do console central expandida (140px a 800px)
        float mixer_channel_gap = 6.0f;          // Espaço entre canais
        float knob_radius_eq = 22.0f;            // Raio dos knobs de EQ (High, Mid, Low) ampliado para 4K/65"
        float knob_radius_cfx = 26.0f;           // Raio do knob de Color FX central ampliado
        float knob_radius_gain = 16.0f;          // Raio do knob de Trim / Ganho ampliado
        float knob_radius_comp = 20.0f;          // Raio do compressor de canal DJM
        float fader_height = 200.0f;             // Altura da calha do fader de canal ampliada para 4K/65"
        float fader_width = 28.0f;               // Largura da pegada do fader
        float crossfader_width = 250.0f;         // Largura da calha do crossfader ampliada
        float crossfader_height = 36.0f;         // Altura do crossfader ampliada
        float vu_meter_width = 5.0f;             // Largura da barra de LEDs do VU Meter
        int   mixer_topology_mode = 0;           // 0: Pioneer 3-Band | 1: Xone 4-Band | 2: Model 1 Contour
        float knob_spacing = 8.0f;               // Espaçamento vertical entre knobs de canal (2px a 30px)
        int   crossfader_orientation = 0;        // 0: Horizontal ↔ | 1: Vertical ↕
        int   channel_fader_orientation = 0;     // 0: Vertical ↕ | 1: Horizontal ↔
        float crossfader_offset_x = 0.0f;        // Deslocamento X do crossfader (arraste livre)
        float crossfader_offset_y = 0.0f;        // Deslocamento Y do crossfader (arraste livre)
        float mixer_knobs_offset_y = 0.0f;       // Deslocamento Y do bloco de knobs
        float mixer_faders_offset_y = 0.0f;      // Deslocamento Y do bloco de faders
        float headphone_offset_x = 0.0f;         // Deslocamento X da secao de fone
        float headphone_offset_y = 0.0f;         // Deslocamento Y da secao de fone
        float headphone_bar_width = 240.0f;      // Largura da barra de fone
        float headphone_bar_height = 24.0f;      // Altura da barra de fone
        float comp_knob_scale = 1.35f;           // Escala do knob dourado de compressor
        float transport_offset_x = 0.0f;         // Deslocamento X do bloco CUE/PLAY
        float transport_offset_y = 0.0f;         // Deslocamento Y do bloco CUE/PLAY
        float cue_play_gap = 8.0f;               // Espacamento minimo entre CUE e PLAY
        float mixer_cue_btn_h = 20.0f;           // Altura do botao CUE do canal no mixer
        float mixer_cue_btn_w = 42.0f;           // Largura do botao CUE do canal no mixer
        float cue_fader_gap = 12.0f;             // Distancia rigida anti-colisao entre botao CUE e fader de volume
        int   pad_grid_mode = 0;                 // 0: 8 Pads (4x2), 1: 16 Pads (8x2 Wide para TV 65 pol), 2: 16 Pads (4x4)
        float ui_font_scale = 1.0f;              // Escala adicional de fontes para legibilidade na TV (0.7x a 1.6x)
        float waveform_toolbar_width = 38.0f;    // Largura ergonomica da barra lateral de zoom/modos da waveform
        bool  enable_collision_physics = true;   // Fisica de colisao AABB ativada

        // =========================================================================
        // 3. PRATOS DE VINIL & JOGWHEELS
        // =========================================================================
        float jogwheel_diameter = 240.0f;        // Diâmetro do prato ampliado (80px a 500px)
        float jogwheel_center_ratio = 0.42f;     // Tamanho relativo do LCD/arte central
        bool  jogwheel_motorized_mode = true;    // Inércia visual de rotação
        float jogwheel_sensitivity = 1.0f;       // Sensibilidade de scratch/pitch bend

        // =========================================================================
        // 4. PERFORMANCE PADS & TRANSPORTE
        // =========================================================================
        float pad_width = 56.0f;                 // Largura individual dos 8 pads ampliada (28px a 130px)
        float pad_height = 42.0f;                // Altura dos pads ampliada (20px a 110px)
        float pad_gap = 5.0f;                    // Espaçamento entre pads
        float pad_subknob_radius = 11.0f;        // Raio dos sub-knobs dos pads (Stems/Bleed)
        float pad_font_scale = 1.20f;            // Escala dos textos dos pads
        float transport_button_radius = 24.0f;   // Raio dos botões redondos de PLAY e CUE
        float transport_column_width = 82.0f;    // Largura dedicada da coluna de Play / Cue
        float deck_tier_split_ratio = 0.54f;     // Split vertical Jogwheel vs Performance Pads (0.30 a 0.75)
        float fx_bar_height = 64.0f;             // Altura configurável do rack de efeitos FX (48px a 90px)

        // =========================================================================
        // 5. PITCH FADER, TEMPO & BPM
        // =========================================================================
        float tempo_fader_height = 165.0f;       // Altura da trilha do Pitch Fader ampliada
        float tempo_fader_width = 30.0f;         // Largura do slider ampliada
        float bpm_font_scale = 1.30f;            // Escala da tipografia de BPM/Pitch
        float side_tempo_panel_height = 0.0f;    // Altura do painel de BPM (0.0f = auto-fit dinamico sem espaco vazio)
        bool  show_side_tempo_key = true;        // Exibir Tom Musical (Key) e Key Sync no mostrador
        bool  show_side_tempo_master = true;     // Exibir botao Master Tempo (Key Lock)
        bool  show_side_tempo_phase_meter = true;// Exibir medidor de fase de compasso 1-2-3-4
        float track_title_font_scale = 1.20f;    // Escala do nome da faixa
        float time_counter_font_scale = 1.15f;   // Escala do relógio de tempo decorrido/remanescente

        // =========================================================================
        // 6. WAVEFORMS & COLORIMETRIA
        // =========================================================================
        float waveform_height = 165.0f;          // Altura elegante e compacta Rekordbox (não alonga para cima)
        float overview_height = 24.0f;           // Altura da régua de visualização da pista
        bool  ribbon_mode_active = false;        // Modo Fitas Desacopladas (Guitar Hero) vs Sólido
        int   waveform_engine_mode = 1;          // 1 = 3-Band Fused (Padrão CDJ-3000 / Rekordbox 7 Flagship), 0 = 3-Band Multi-Lane
        int   waveform_color_mode = 1;           // 1 = 3-Band CDJ-3000 Oficial (Graves Azul Royal, Médios Âmbar, Agudos Branco Diamante)
        float ribbon_lane_high_ratio = 0.28f;    // Proporção da fita de agudos
        float ribbon_lane_mid_ratio = 0.36f;     // Proporção da fita de médios
        float ribbon_lane_low_ratio = 0.36f;     // Proporção da fita de graves
        float waveform_horizontal_stretch = 1.65f; // Alongamento horizontal dos transientes/envelope (padrão amplo anterior: 1.65x)
        int   waveform_visible_beats = 16;       // Quantidade de batidas visíveis no visor (4 a 64 batidas, Default: 16)
        float waveform_scroll_speed = 1.0f;      // Multiplicador da velocidade de rolagem da waveform (0.50x a 3.00x, Default: 1.00x)
        bool  needs_waveform_recompute = false;  // Sinalizador de invalidação e reconstrução de cache da waveform

        // --- CONTROLES AVANÇADOS REKORDBOX 7 (ESTILO LINHAS, ESPAÇAMENTO & TRANSIENTES) ---
        int   waveform_line_style = 0;           // 0 = Linhas Nítidas Rekordbox 7, 1 = Barras com Respiro (Discrete Stems), 2 = Malha Sólida Contínua (Legacy)
        float waveform_line_spacing = 2.0f;      // Espaçamento horizontal entre linhas (Passo) [1.0px .. 4.0px] (Default: 2.0px)
        float waveform_line_width = 1.2f;        // Espessura de cada linha vertical [0.8px .. 3.0px] (Default: 1.2px)
        float waveform_line_gap = 0.8f;          // Separação / Respiro entre linhas [0.0px .. 2.0px] (Default: 0.8px)
        float waveform_transient_punch = 1.50f;  // Punch / Ataque dos transientes [0.50x .. 3.00x] (Eleva kicks e snares como agulhas)
        float waveform_dynamic_contrast = 1.35f; // Contraste Dinâmico / Gamma [0.80 .. 2.50] (Elimina sensação de waveform chapada)
        float waveform_envelope_decay = 0.20f;   // Decaimento / Retenção do Envelope [0.00x .. 2.00x] (0.0x = linhas puras sem preenchimento artificial)
        float waveform_core_brightness = 0.75f;  // Brilho do Miolo Incandescente [0.00 .. 1.00]
        bool  waveform_bipolar_mode = true;      // Modo Bipolar Simétrico (como Rekordbox 7 na Imagem 2) vs Unipolar

        // =========================================================================
        // 7. ANIMAÇÕES, METRÔNOMO VISUAL E PERFORMANCE
        // =========================================================================
        bool  enable_visual_beat_metronome = true; // Pulso a cada beat e clarão no Downbeat 1
        bool  enable_visual_animations = true;     // Animações de rotação, glow e LEDs
        bool  enable_anti_aliased_4k = true;       // Rasterização ultra-sharp com anti-aliasing orgânico

        // Singleton Instance
        static DJLayoutMetrics& getInstance() {
            static DJLayoutMetrics instance;
            return instance;
        }

        // Método utilitário de escala dinâmica: multiplica o valor base pela escala global
        inline float S(float base_val) const {
            return base_val * master_scale;
        }

        // Restaura todos os parâmetros de layout para os padrões de fábrica
        void resetToDefaults() {
            master_scale = 1.15f;
            deck_view_mode = 0;
            mixer_width = 320.0f;
            mixer_channel_gap = 6.0f;
            knob_radius_eq = 22.0f;
            knob_radius_cfx = 26.0f;
            knob_radius_gain = 16.0f;
            knob_radius_comp = 20.0f;
            fader_height = 200.0f;
            fader_width = 28.0f;
            crossfader_width = 250.0f;
            crossfader_height = 36.0f;
            vu_meter_width = 5.0f;
            mixer_topology_mode = 0;
            jogwheel_diameter = 240.0f;
            jogwheel_center_ratio = 0.42f;
            jogwheel_motorized_mode = true;
            jogwheel_sensitivity = 1.0f;
            pad_width = 56.0f;
            pad_height = 42.0f;
            pad_gap = 5.0f;
            pad_subknob_radius = 11.0f;
            pad_font_scale = 1.20f;
            transport_button_radius = 24.0f;
            transport_column_width = 82.0f;
            deck_tier_split_ratio = 0.54f;
            fx_bar_height = 64.0f;
            tempo_fader_height = 165.0f;
            tempo_fader_width = 30.0f;
            bpm_font_scale = 1.30f;
            track_title_font_scale = 1.20f;
            time_counter_font_scale = 1.15f;
            waveform_height = 165.0f;
            overview_height = 24.0f;
            ribbon_mode_active = false;
            waveform_engine_mode = 0;
            waveform_color_mode = 0;
            waveform_horizontal_stretch = 1.65f;
            waveform_visible_beats = 16;
            waveform_scroll_speed = 1.0f;
            waveform_line_style = 0;
            waveform_line_spacing = 2.0f;
            waveform_line_width = 1.2f;
            waveform_line_gap = 0.8f;
            waveform_transient_punch = 1.50f;
            waveform_dynamic_contrast = 1.35f;
            waveform_envelope_decay = 0.20f;
            waveform_core_brightness = 0.75f;
            waveform_bipolar_mode = true;
            enable_visual_beat_metronome = true;
            enable_visual_animations = true;
            enable_anti_aliased_4k = true;
            knob_spacing = 8.0f;
            crossfader_orientation = 0;
            channel_fader_orientation = 0;
            crossfader_offset_x = 0.0f;
            crossfader_offset_y = 0.0f;
            mixer_knobs_offset_y = 0.0f;
            mixer_faders_offset_y = 0.0f;
            headphone_offset_x = 0.0f;
            headphone_offset_y = 0.0f;
            headphone_bar_width = 240.0f;
            headphone_bar_height = 24.0f;
            comp_knob_scale = 1.35f;
            transport_offset_x = 0.0f;
            transport_offset_y = 0.0f;
            cue_play_gap = 8.0f;
            mixer_cue_btn_h = 20.0f;
            mixer_cue_btn_w = 42.0f;
            cue_fader_gap = 12.0f;
            side_tempo_panel_height = 0.0f;
            show_side_tempo_key = true;
            show_side_tempo_master = true;
            show_side_tempo_phase_meter = true;
            pad_grid_mode = 0;
            ui_font_scale = 1.0f;
            waveform_toolbar_width = 38.0f;
            enable_collision_physics = true;
            selected_edit_widget = WIDGET_NONE;
        }

        // =========================================================================
        // PRESETS PRÉ-CONFIGURADOS DE MERCADO
        // =========================================================================
        void applyPreset(DJPresetLayoutType preset) {
            switch (preset) {
                case DJPresetLayoutType::CLUB_STANDARD_2DECK:
                    master_scale = 1.15f;
                    deck_view_mode = 0;
                    mixer_width = 280.0f;
                    knob_radius_eq = 18.0f;
                    knob_radius_cfx = 25.0f;
                    fader_height = 150.0f;
                    jogwheel_diameter = 240.0f;
                    pad_width = 56.0f;
                    pad_height = 42.0f;
                    transport_button_radius = 24.0f;
                    transport_column_width = 82.0f;
                    deck_tier_split_ratio = 0.54f;
                    fx_bar_height = 64.0f;
                    tempo_fader_height = 165.0f;
                    waveform_height = 165.0f;
                    ribbon_mode_active = false;
                    waveform_engine_mode = 0;
                    mixer_topology_mode = 0;
                    waveform_horizontal_stretch = 1.65f;
                    break;

                case DJPresetLayoutType::TURNTABLIST_BATTLE_12INCH:
                    master_scale = 1.05f;
                    deck_view_mode = 0;
                    mixer_width = 210.0f; // Mixer estreito de batalha tipo Rane Seventy-Two
                    knob_radius_eq = 14.0f;
                    knob_radius_cfx = 20.0f;
                    fader_height = 120.0f;
                    crossfader_width = 260.0f; // Crossfader amplo para scratches
                    crossfader_height = 38.0f;
                    jogwheel_diameter = 340.0f; // Prato gigante de vinil 12" motorizado
                    pad_width = 54.0f; // Pads grandes para Cue Juggling
                    pad_height = 42.0f;
                    transport_button_radius = 24.0f;
                    transport_column_width = 82.0f;
                    deck_tier_split_ratio = 0.54f;
                    fx_bar_height = 64.0f;
                    tempo_fader_height = 160.0f;
                    waveform_height = 150.0f;
                    ribbon_mode_active = false;
                    waveform_engine_mode = 0;
                    mixer_topology_mode = 0;
                    waveform_horizontal_stretch = 1.70f;
                    break;

                case DJPresetLayoutType::TECHNO_STUDIO_4DECK:
                    master_scale = 0.92f;
                    deck_view_mode = 1; // 4 Decks Horizontais
                    mixer_width = 380.0f; // Mixer amplo de 4 canais
                    knob_radius_eq = 13.0f;
                    knob_radius_cfx = 18.0f;
                    fader_height = 140.0f;
                    jogwheel_diameter = 160.0f;
                    pad_width = 40.0f;
                    pad_height = 30.0f;
                    transport_button_radius = 20.0f;
                    transport_column_width = 76.0f;
                    deck_tier_split_ratio = 0.54f;
                    fx_bar_height = 60.0f;
                    tempo_fader_height = 130.0f;
                    waveform_height = 175.0f;
                    ribbon_mode_active = false;
                    waveform_engine_mode = 0;
                    mixer_topology_mode = 1; // Topologia Xone 4-Band
                    waveform_horizontal_stretch = 1.60f;
                    break;

                case DJPresetLayoutType::ULTRA_COMPACT_LAPTOP:
                    master_scale = 0.78f;
                    deck_view_mode = 0;
                    mixer_width = 175.0f;
                    knob_radius_eq = 11.0f;
                    knob_radius_cfx = 15.0f;
                    fader_height = 95.0f;
                    jogwheel_diameter = 130.0f;
                    pad_width = 34.0f;
                    pad_height = 24.0f;
                    transport_button_radius = 16.0f;
                    transport_column_width = 68.0f;
                    deck_tier_split_ratio = 0.52f;
                    fx_bar_height = 56.0f;
                    tempo_fader_height = 100.0f;
                    waveform_height = 130.0f;
                    ribbon_mode_active = false;
                    waveform_engine_mode = 0;
                    mixer_topology_mode = 0;
                    waveform_horizontal_stretch = 1.50f;
                    break;

                case DJPresetLayoutType::ULTRAWIDE_4K_HERO:
                    master_scale = 1.35f;
                    deck_view_mode = 0;
                    mixer_width = 320.0f;
                    knob_radius_eq = 20.0f;
                    knob_radius_cfx = 28.0f;
                    fader_height = 180.0f;
                    jogwheel_diameter = 290.0f;
                    pad_width = 64.0f;
                    pad_height = 48.0f;
                    transport_button_radius = 28.0f;
                    transport_column_width = 92.0f;
                    deck_tier_split_ratio = 0.54f;
                    fx_bar_height = 70.0f;
                    tempo_fader_height = 200.0f;
                    waveform_height = 180.0f;
                    ribbon_mode_active = false;
                    waveform_engine_mode = 0;
                    enable_anti_aliased_4k = true;
                    mixer_topology_mode = 1;
                    waveform_horizontal_stretch = 1.80f;
                    break;
            }
            waveform_visible_beats = 16;
            waveform_scroll_speed = 1.0f;
            needs_waveform_recompute = true;
        }

        // =========================================================================
        // SERIALIZAÇÃO & PERSISTÊNCIA EM ARQUIVOS JSON
        // =========================================================================
        bool saveToFile(const std::string& path = "assets/layouts/active_workspace_layout.json") const {
            try {
                std::filesystem::path p(path);
                if (p.has_parent_path()) {
                    std::filesystem::create_directories(p.parent_path());
                }
                std::ofstream f(path);
                if (!f.is_open()) return false;
                f.imbue(std::locale::classic());

                f << "{\n";
                f << "  \"master_scale\": " << master_scale << ",\n";
                f << "  \"deck_view_mode\": " << deck_view_mode << ",\n";
                f << "  \"mixer_width\": " << mixer_width << ",\n";
                f << "  \"mixer_channel_gap\": " << mixer_channel_gap << ",\n";
                f << "  \"knob_radius_eq\": " << knob_radius_eq << ",\n";
                f << "  \"knob_radius_cfx\": " << knob_radius_cfx << ",\n";
                f << "  \"knob_radius_gain\": " << knob_radius_gain << ",\n";
                f << "  \"knob_radius_comp\": " << knob_radius_comp << ",\n";
                f << "  \"fader_height\": " << fader_height << ",\n";
                f << "  \"fader_width\": " << fader_width << ",\n";
                f << "  \"crossfader_width\": " << crossfader_width << ",\n";
                f << "  \"crossfader_height\": " << crossfader_height << ",\n";
                f << "  \"mixer_topology_mode\": " << mixer_topology_mode << ",\n";
                f << "  \"jogwheel_diameter\": " << jogwheel_diameter << ",\n";
                f << "  \"jogwheel_center_ratio\": " << jogwheel_center_ratio << ",\n";
                f << "  \"jogwheel_motorized_mode\": " << (jogwheel_motorized_mode ? "true" : "false") << ",\n";
                f << "  \"pad_width\": " << pad_width << ",\n";
                f << "  \"pad_height\": " << pad_height << ",\n";
                f << "  \"pad_gap\": " << pad_gap << ",\n";
                f << "  \"pad_subknob_radius\": " << pad_subknob_radius << ",\n";
                f << "  \"transport_button_radius\": " << transport_button_radius << ",\n";
                f << "  \"transport_column_width\": " << transport_column_width << ",\n";
                f << "  \"deck_tier_split_ratio\": " << deck_tier_split_ratio << ",\n";
                f << "  \"fx_bar_height\": " << fx_bar_height << ",\n";
                f << "  \"tempo_fader_height\": " << tempo_fader_height << ",\n";
                f << "  \"tempo_fader_width\": " << tempo_fader_width << ",\n";
                f << "  \"bpm_font_scale\": " << bpm_font_scale << ",\n";
                f << "  \"waveform_height\": " << waveform_height << ",\n";
                f << "  \"overview_height\": " << overview_height << ",\n";
                f << "  \"waveform_horizontal_stretch\": " << waveform_horizontal_stretch << ",\n";
                f << "  \"waveform_visible_beats\": " << waveform_visible_beats << ",\n";
                f << "  \"waveform_scroll_speed\": " << waveform_scroll_speed << ",\n";
                f << "  \"ribbon_mode_active\": " << (ribbon_mode_active ? "true" : "false") << ",\n";
                f << "  \"waveform_engine_mode\": " << waveform_engine_mode << ",\n";
                f << "  \"waveform_color_mode\": " << waveform_color_mode << ",\n";
                f << "  \"waveform_line_style\": " << waveform_line_style << ",\n";
                f << "  \"waveform_line_spacing\": " << waveform_line_spacing << ",\n";
                f << "  \"waveform_line_width\": " << waveform_line_width << ",\n";
                f << "  \"waveform_line_gap\": " << waveform_line_gap << ",\n";
                f << "  \"waveform_transient_punch\": " << waveform_transient_punch << ",\n";
                f << "  \"waveform_dynamic_contrast\": " << waveform_dynamic_contrast << ",\n";
                f << "  \"waveform_envelope_decay\": " << waveform_envelope_decay << ",\n";
                f << "  \"waveform_core_brightness\": " << waveform_core_brightness << ",\n";
                f << "  \"waveform_bipolar_mode\": " << (waveform_bipolar_mode ? "true" : "false") << ",\n";
                f << "  \"enable_visual_beat_metronome\": " << (enable_visual_beat_metronome ? "true" : "false") << ",\n";
                f << "  \"enable_visual_animations\": " << (enable_visual_animations ? "true" : "false") << ",\n";
                f << "  \"enable_anti_aliased_4k\": " << (enable_anti_aliased_4k ? "true" : "false") << ",\n";
                f << "  \"knob_spacing\": " << knob_spacing << ",\n";
                f << "  \"crossfader_orientation\": " << crossfader_orientation << ",\n";
                f << "  \"channel_fader_orientation\": " << channel_fader_orientation << ",\n";
                f << "  \"crossfader_offset_x\": " << crossfader_offset_x << ",\n";
                f << "  \"crossfader_offset_y\": " << crossfader_offset_y << ",\n";
                f << "  \"mixer_knobs_offset_y\": " << mixer_knobs_offset_y << ",\n";
                f << "  \"mixer_faders_offset_y\": " << mixer_faders_offset_y << ",\n";
                f << "  \"headphone_offset_x\": " << headphone_offset_x << ",\n";
                f << "  \"headphone_offset_y\": " << headphone_offset_y << ",\n";
                f << "  \"headphone_bar_width\": " << headphone_bar_width << ",\n";
                f << "  \"headphone_bar_height\": " << headphone_bar_height << ",\n";
                f << "  \"comp_knob_scale\": " << comp_knob_scale << ",\n";
                f << "  \"transport_offset_x\": " << transport_offset_x << ",\n";
                f << "  \"transport_offset_y\": " << transport_offset_y << ",\n";
                f << "  \"cue_play_gap\": " << cue_play_gap << ",\n";
                f << "  \"mixer_cue_btn_h\": " << mixer_cue_btn_h << ",\n";
                f << "  \"mixer_cue_btn_w\": " << mixer_cue_btn_w << ",\n";
                f << "  \"cue_fader_gap\": " << cue_fader_gap << ",\n";
                f << "  \"side_tempo_panel_height\": " << side_tempo_panel_height << ",\n";
                f << "  \"show_side_tempo_key\": " << (show_side_tempo_key ? "true" : "false") << ",\n";
                f << "  \"show_side_tempo_master\": " << (show_side_tempo_master ? "true" : "false") << ",\n";
                f << "  \"show_side_tempo_phase_meter\": " << (show_side_tempo_phase_meter ? "true" : "false") << ",\n";
                f << "  \"pad_grid_mode\": " << pad_grid_mode << ",\n";
                f << "  \"ui_font_scale\": " << ui_font_scale << ",\n";
                f << "  \"waveform_toolbar_width\": " << waveform_toolbar_width << ",\n";
                f << "  \"enable_collision_physics\": " << (enable_collision_physics ? "true" : "false") << "\n";
                f << "}\n";
                f.close();
                return true;
            } catch (...) {
                return false;
            }
        }

        bool loadFromFile(const std::string& path = "assets/layouts/active_workspace_layout.json") {
            if (!std::filesystem::exists(path)) return false;
            try {
                std::ifstream f(path);
                if (!f.is_open()) return false;
                std::stringstream buffer;
                buffer << f.rdbuf();
                std::string json = buffer.str();

                master_scale = JSON::parseFloat(json, "master_scale", master_scale);
                deck_view_mode = JSON::parseInt(json, "deck_view_mode", deck_view_mode);
                mixer_width = JSON::parseFloat(json, "mixer_width", mixer_width);
                mixer_channel_gap = JSON::parseFloat(json, "mixer_channel_gap", mixer_channel_gap);
                knob_radius_eq = JSON::parseFloat(json, "knob_radius_eq", knob_radius_eq);
                knob_radius_cfx = JSON::parseFloat(json, "knob_radius_cfx", knob_radius_cfx);
                knob_radius_gain = JSON::parseFloat(json, "knob_radius_gain", knob_radius_gain);
                knob_radius_comp = JSON::parseFloat(json, "knob_radius_comp", knob_radius_comp);
                fader_height = JSON::parseFloat(json, "fader_height", fader_height);
                fader_width = JSON::parseFloat(json, "fader_width", fader_width);
                crossfader_width = JSON::parseFloat(json, "crossfader_width", crossfader_width);
                crossfader_height = JSON::parseFloat(json, "crossfader_height", crossfader_height);
                mixer_topology_mode = JSON::parseInt(json, "mixer_topology_mode", mixer_topology_mode);
                jogwheel_diameter = JSON::parseFloat(json, "jogwheel_diameter", jogwheel_diameter);
                jogwheel_center_ratio = JSON::parseFloat(json, "jogwheel_center_ratio", jogwheel_center_ratio);
                jogwheel_motorized_mode = JSON::parseBool(json, "jogwheel_motorized_mode", jogwheel_motorized_mode);
                pad_width = JSON::parseFloat(json, "pad_width", pad_width);
                pad_height = JSON::parseFloat(json, "pad_height", pad_height);
                pad_gap = JSON::parseFloat(json, "pad_gap", pad_gap);
                pad_subknob_radius = JSON::parseFloat(json, "pad_subknob_radius", pad_subknob_radius);
                transport_button_radius = JSON::parseFloat(json, "transport_button_radius", transport_button_radius);
                transport_column_width = JSON::parseFloat(json, "transport_column_width", transport_column_width);
                deck_tier_split_ratio = JSON::parseFloat(json, "deck_tier_split_ratio", deck_tier_split_ratio);
                fx_bar_height = JSON::parseFloat(json, "fx_bar_height", fx_bar_height);
                tempo_fader_height = JSON::parseFloat(json, "tempo_fader_height", tempo_fader_height);
                tempo_fader_width = JSON::parseFloat(json, "tempo_fader_width", tempo_fader_width);
                bpm_font_scale = JSON::parseFloat(json, "bpm_font_scale", bpm_font_scale);
                waveform_height = JSON::parseFloat(json, "waveform_height", waveform_height);
                overview_height = JSON::parseFloat(json, "overview_height", overview_height);
                waveform_horizontal_stretch = JSON::parseFloat(json, "waveform_horizontal_stretch", waveform_horizontal_stretch);
                waveform_visible_beats = JSON::parseInt(json, "waveform_visible_beats", waveform_visible_beats);
                waveform_scroll_speed = JSON::parseFloat(json, "waveform_scroll_speed", waveform_scroll_speed);
                ribbon_mode_active = JSON::parseBool(json, "ribbon_mode_active", ribbon_mode_active);
                waveform_engine_mode = JSON::parseInt(json, "waveform_engine_mode", waveform_engine_mode);
                waveform_color_mode = std::clamp(JSON::parseInt(json, "waveform_color_mode", waveform_color_mode), 0, 9);
                waveform_line_style = JSON::parseInt(json, "waveform_line_style", waveform_line_style);
                waveform_line_spacing = JSON::parseFloat(json, "waveform_line_spacing", waveform_line_spacing);
                waveform_line_width = JSON::parseFloat(json, "waveform_line_width", waveform_line_width);
                waveform_line_gap = JSON::parseFloat(json, "waveform_line_gap", waveform_line_gap);
                waveform_transient_punch = JSON::parseFloat(json, "waveform_transient_punch", waveform_transient_punch);
                waveform_dynamic_contrast = JSON::parseFloat(json, "waveform_dynamic_contrast", waveform_dynamic_contrast);
                waveform_envelope_decay = JSON::parseFloat(json, "waveform_envelope_decay", waveform_envelope_decay);
                waveform_core_brightness = JSON::parseFloat(json, "waveform_core_brightness", waveform_core_brightness);
                waveform_bipolar_mode = JSON::parseBool(json, "waveform_bipolar_mode", waveform_bipolar_mode);
                enable_visual_beat_metronome = JSON::parseBool(json, "enable_visual_beat_metronome", enable_visual_beat_metronome);
                enable_visual_animations = JSON::parseBool(json, "enable_visual_animations", enable_visual_animations);
                enable_anti_aliased_4k = JSON::parseBool(json, "enable_anti_aliased_4k", enable_anti_aliased_4k);
                knob_spacing = JSON::parseFloat(json, "knob_spacing", knob_spacing);
                crossfader_orientation = JSON::parseInt(json, "crossfader_orientation", crossfader_orientation);
                channel_fader_orientation = JSON::parseInt(json, "channel_fader_orientation", channel_fader_orientation);
                crossfader_offset_x = JSON::parseFloat(json, "crossfader_offset_x", crossfader_offset_x);
                crossfader_offset_y = JSON::parseFloat(json, "crossfader_offset_y", crossfader_offset_y);
                mixer_knobs_offset_y = JSON::parseFloat(json, "mixer_knobs_offset_y", mixer_knobs_offset_y);
                mixer_faders_offset_y = JSON::parseFloat(json, "mixer_faders_offset_y", mixer_faders_offset_y);
                headphone_offset_x = JSON::parseFloat(json, "headphone_offset_x", headphone_offset_x);
                headphone_offset_y = JSON::parseFloat(json, "headphone_offset_y", headphone_offset_y);
                headphone_bar_width = JSON::parseFloat(json, "headphone_bar_width", headphone_bar_width);
                headphone_bar_height = JSON::parseFloat(json, "headphone_bar_height", headphone_bar_height);
                comp_knob_scale = JSON::parseFloat(json, "comp_knob_scale", comp_knob_scale);
                transport_offset_x = JSON::parseFloat(json, "transport_offset_x", transport_offset_x);
                transport_offset_y = JSON::parseFloat(json, "transport_offset_y", transport_offset_y);
                cue_play_gap = JSON::parseFloat(json, "cue_play_gap", cue_play_gap);
                mixer_cue_btn_h = JSON::parseFloat(json, "mixer_cue_btn_h", mixer_cue_btn_h);
                mixer_cue_btn_w = JSON::parseFloat(json, "mixer_cue_btn_w", mixer_cue_btn_w);
                cue_fader_gap = JSON::parseFloat(json, "cue_fader_gap", cue_fader_gap);
                side_tempo_panel_height = JSON::parseFloat(json, "side_tempo_panel_height", side_tempo_panel_height);
                show_side_tempo_key = JSON::parseBool(json, "show_side_tempo_key", show_side_tempo_key);
                show_side_tempo_master = JSON::parseBool(json, "show_side_tempo_master", show_side_tempo_master);
                show_side_tempo_phase_meter = JSON::parseBool(json, "show_side_tempo_phase_meter", show_side_tempo_phase_meter);
                pad_grid_mode = JSON::parseInt(json, "pad_grid_mode", pad_grid_mode);
                ui_font_scale = JSON::parseFloat(json, "ui_font_scale", ui_font_scale);
                waveform_toolbar_width = JSON::parseFloat(json, "waveform_toolbar_width", waveform_toolbar_width);
                enable_collision_physics = JSON::parseBool(json, "enable_collision_physics", enable_collision_physics);
                f.close();
                return true;
            } catch (...) {
                return false;
            }
        }
    };
}
