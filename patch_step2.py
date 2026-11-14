import os, sys

path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h"
bak_path = path + ".bak"

with open(path, "r", encoding="utf-8") as f:
    code = f.read()

# 1. Top bar: Add DAW return button and UI Designer button
target_topbar_left = """            // ── 1. ALA ESQUERDA: CAPSULAS DE WORKSPACE ──────────────────────────
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 6.0f, pos.y + 3.0f));

            // [LIBRARY]"""

replacement_topbar_left = """            // ── 1. ALA ESQUERDA: RETORNO À DAW & CAPSULAS DE WORKSPACE ──────────────────
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 6.0f, pos.y + 3.0f));

            // [⬅ VOLTAR À DAW] Botão Proeminente de Retorno à DAW (Timeline / Arranjador)
            capsuleBtn("##TopReturnToDaw", ICON_FA_ARROW_LEFT " VOLTAR À DAW", false, 
                       IM_COL32(0, 220, 255, 255), IM_COL32(230, 250, 255, 255),
                       "VOLTAR PARA A DAW: Alternar para a Timeline e Arranjador da DAW (Atalho: F11)", [&]() {
                if (p_open) *p_open = false;
                KuroUI::show_dj_modal = false;
            });

            // [🎨 DESIGNER DE UI] Botão para abrir o Editor de UI/UX e Customização de Layout
            capsuleBtn("##TopUIDesignerBtn", ICON_FA_PALETTE " EDITOR DE UI", show_ui_designer_modal,
                       IM_COL32(185, 80, 255, 255), IM_COL32(245, 225, 255, 255),
                       "EDITOR DE UI/UX: Personalizar tamanhos, formas de onda, jogs, mixer, pads, cores e layout", [&]() {
                show_ui_designer_modal = !show_ui_designer_modal;
            });

            // [LIBRARY]"""

if target_topbar_left not in code:
    print("ERROR: target_topbar_left not found")
    sys.exit(1)

code = code.replace(target_topbar_left, replacement_topbar_left, 1)

# 2. Main layout dimensions in render()
target_dimensions = """                    float bottom_extra_h = 0.0f;
                    if (library_view_mode == LIB_VIEW_SPLIT) {
                        float desired_lib_h = std::clamp(total_avail_h * 0.36f, 
                                                         KuroUI::UIScaleManager::Scale(160.0f), 
                                                         KuroUI::UIScaleManager::Scale(380.0f));
                        library_height = desired_lib_h / KuroUI::UIScaleManager::getScale();
                        bottom_extra_h += desired_lib_h;
                    }
                    if (show_export_modal) bottom_extra_h += KuroUI::UIScaleManager::Scale(136.0f);
                    if (show_sampler_rack) bottom_extra_h += KuroUI::UIScaleManager::Scale(150.0f);

                    float full_height_h = total_avail_h - bottom_extra_h - bottom_bar_h - 4.0f;
                    if (full_height_h < KuroUI::UIScaleManager::Scale(280.0f)) full_height_h = KuroUI::UIScaleManager::Scale(280.0f);

                    // Altura das Waveforms dentro da coluna principal proporcional à tela
                    ImGuiViewport* vp_curr = ImGui::GetMainViewport();
                    if (!wf_height_user_adjusted) {
                        waveform_height = std::clamp(full_height_h * 0.28f, 
                                                     KuroUI::UIScaleManager::Scale(90.0f), 
                                                     KuroUI::UIScaleManager::Scale(180.0f));
                    }
                    float wave_h_px = waveform_height;
                    float decks_body_h = full_height_h - wave_h_px - 4.0f;
                    if (decks_body_h < KuroUI::UIScaleManager::Scale(190.0f)) {
                        decks_body_h = KuroUI::UIScaleManager::Scale(190.0f);
                        wave_h_px = full_height_h - decks_body_h - 4.0f;
                    }

                    auto renderDecksAndMixer = [&](float area_w, float area_h) {
                        float gap_px = KuroUI::UIScaleManager::Scale(4.0f);
                        float mixer_w = std::clamp(area_w * 0.15f,
                                                     KuroUI::UIScaleManager::Scale(180.0f),
                                                     KuroUI::UIScaleManager::Scale(260.0f));"""

replacement_dimensions = """                    float bottom_extra_h = 0.0f;
                    if (library_view_mode == LIB_VIEW_SPLIT) {
                        float desired_lib_h = std::clamp(total_avail_h * library_split_ratio, 
                                                         KuroUI::UIScaleManager::Scale(110.0f), 
                                                         KuroUI::UIScaleManager::Scale(650.0f));
                        library_height = desired_lib_h / KuroUI::UIScaleManager::getScale();
                        bottom_extra_h += desired_lib_h;
                    }
                    if (show_export_modal) bottom_extra_h += KuroUI::UIScaleManager::Scale(136.0f);
                    if (show_sampler_rack) bottom_extra_h += KuroUI::UIScaleManager::Scale(150.0f);

                    float full_height_h = total_avail_h - bottom_extra_h - bottom_bar_h - 4.0f;
                    if (full_height_h < KuroUI::UIScaleManager::Scale(240.0f)) full_height_h = KuroUI::UIScaleManager::Scale(240.0f);

                    // Altura das Waveforms dentro da coluna principal proporcional à tela
                    if (!wf_height_user_adjusted) {
                        waveform_height = std::clamp(full_height_h * wave_height_ratio, 
                                                     KuroUI::UIScaleManager::Scale(60.0f), 
                                                     KuroUI::UIScaleManager::Scale(380.0f));
                    }
                    float wave_h_px = waveform_height;
                    float decks_body_h = full_height_h - wave_h_px - 4.0f;
                    if (decks_body_h < KuroUI::UIScaleManager::Scale(170.0f)) {
                        decks_body_h = KuroUI::UIScaleManager::Scale(170.0f);
                        wave_h_px = full_height_h - decks_body_h - 4.0f;
                    }

                    auto renderDecksAndMixer = [&](float area_w, float area_h) {
                        float gap_px = KuroUI::UIScaleManager::Scale(4.0f);
                        float mixer_w = std::clamp(area_w * mixer_width_ratio,
                                                     KuroUI::UIScaleManager::Scale(140.0f),
                                                     KuroUI::UIScaleManager::Scale(480.0f));"""

if target_dimensions not in code:
    print("ERROR: target_dimensions not found")
    sys.exit(1)

code = code.replace(target_dimensions, replacement_dimensions, 1)

# 3. Deck Panel Jog & Pad proportions
target_deck_upper = """            // Split vertical do corpo responsivo (Pioneer DDJ / Rekordbox 2-Tier):
            float upper_ratio = (body_h >= 420.0f) ? 0.62f : ((body_h >= 300.0f) ? 0.58f : 0.54f);
            float upper_h = std::clamp(body_h * upper_ratio, 120.0f, 380.0f);
            float lower_y = body_y + upper_h + 4.0f;
            float lower_h = body_h - upper_h - 6.0f;

            // Distribuição horizontal responsiva para cobrir 100% da largura em qualquer resolução:
            float left_w = std::clamp(usable_w * 0.055f, 48.0f, 58.0f);
            float fader_w = std::clamp(usable_w * 0.085f, 62.0f, 90.0f);
            float mem_w = std::clamp(usable_w * 0.13f, 80.0f, 130.0f);
            float side_w = std::clamp(usable_w * 0.12f, 80.0f, 120.0f);
            float jog_bay_w = usable_w - left_w - fader_w - mem_w - side_w - gap_x * 4.0f;

            float j_sz = std::clamp(std::min(jog_bay_w - 12.0f, upper_h - 8.0f), 80.0f, 320.0f);"""

replacement_deck_upper = """            // Split vertical do corpo responsivo (Pioneer DDJ / Rekordbox 2-Tier):
            float upper_ratio = deck_upper_ratio;
            float upper_h = std::clamp(body_h * upper_ratio, 100.0f, 450.0f);
            float lower_y = body_y + upper_h + 4.0f;
            float lower_h = body_h - upper_h - 6.0f;

            // Distribuição horizontal responsiva para cobrir 100% da largura em qualquer resolução:
            float left_w = std::clamp(usable_w * 0.055f, 48.0f, 58.0f);
            float fader_w = std::clamp(usable_w * 0.085f, 62.0f, 90.0f);
            float mem_w = std::clamp(usable_w * 0.13f, 80.0f, 130.0f);
            float side_w = std::clamp(usable_w * 0.12f, 80.0f, 120.0f);
            float jog_bay_w = usable_w - left_w - fader_w - mem_w - side_w - gap_x * 4.0f;

            float j_sz = std::clamp(std::min(jog_bay_w - 12.0f, upper_h - 8.0f) * jog_size_multiplier, 45.0f, 450.0f);"""

if target_deck_upper not in code:
    print("ERROR: target_deck_upper not found")
    sys.exit(1)

code = code.replace(target_deck_upper, replacement_deck_upper, 1)

# 4. Pads height scale in renderRekordboxPads
target_pad_h = "            float pad_h = std::clamp((avail_pad_h - pad_gap_y) * 0.48f, 22.0f, 130.0f);"
replacement_pad_h = "            float pad_h = std::clamp((avail_pad_h - pad_gap_y) * 0.48f * pad_height_scale, 18.0f, 150.0f);"

if target_pad_h not in code:
    print("ERROR: target_pad_h not found")
    sys.exit(1)

code = code.replace(target_pad_h, replacement_pad_h, 1)

# 5. Knobs size in drawRekordboxKnob
target_knob_draw = """            auto drawRekordboxKnob = [&](const char* id, const char* label, float* p_val, float v_min, float v_max, float def_val, float cx, float cy, float radius, float gr_lin = 1.0f, bool is_comp = false) {
                ImVec2 center = ImVec2(cx, cy);"""

replacement_knob_draw = """            auto drawRekordboxKnob = [&](const char* id, const char* label, float* p_val, float v_min, float v_max, float def_val, float cx, float cy, float radius, float gr_lin = 1.0f, bool is_comp = false) {
                radius *= knob_size_multiplier;
                ImVec2 center = ImVec2(cx, cy);"""

if target_knob_draw not in code:
    print("ERROR: target_knob_draw not found")
    sys.exit(1)

code = code.replace(target_knob_draw, replacement_knob_draw, 1)

# 6. Click activation on Multi-FX and Single-FX knobs
target_multi_fx_knob = """                    if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(kid, &slot.dry_wet, 0.0f, 1.0f, 6.5f,
                                                                        ImVec4(0.0f, 0.75f, 1.0f, 1.0f), "%.0f%%", 0.0f, false, false, false, false, false)) {
                        slot.enabled = (slot.dry_wet > 0.005f);
                        if (slot.enabled) fx_unit.enabled = true;
                    }"""

replacement_multi_fx_knob = """                    if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(kid, &slot.dry_wet, 0.0f, 1.0f, 6.5f * knob_size_multiplier,
                                                                        ImVec4(0.0f, 0.75f, 1.0f, 1.0f), "%.0f%%", 0.0f, false, false, false, false, false)) {
                        slot.enabled = (slot.dry_wet > 0.005f);
                        if (slot.enabled) fx_unit.enabled = true;
                    }
                    if (fx_click_toggle_enabled && ImGui::IsItemClicked(0)) {
                        slot.enabled = !slot.enabled;
                        if (slot.enabled) {
                            if (slot.dry_wet < 0.20f) slot.dry_wet = 0.50f;
                            fx_unit.enabled = true;
                        }
                    }"""

if target_multi_fx_knob not in code:
    print("ERROR: target_multi_fx_knob not found")
    sys.exit(1)

code = code.replace(target_multi_fx_knob, replacement_multi_fx_knob, 1)

target_single_fx_knob = """                if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(kdid, &fx_unit.dry_wet, 0.0f, 1.0f, 7.0f,
                                                                    ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%.0f%%", 0.5f, false, false, false, false, false)) {
                    fx_unit.enabled = (fx_unit.dry_wet > 0.005f);
                }"""

replacement_single_fx_knob = """                if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(kdid, &fx_unit.dry_wet, 0.0f, 1.0f, 7.0f * knob_size_multiplier,
                                                                    ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%.0f%%", 0.5f, false, false, false, false, false)) {
                    fx_unit.enabled = (fx_unit.dry_wet > 0.005f);
                }
                if (fx_click_toggle_enabled && ImGui::IsItemClicked(0)) {
                    fx_unit.enabled = !fx_unit.enabled;
                    if (fx_unit.enabled && fx_unit.dry_wet < 0.20f) fx_unit.dry_wet = 0.50f;
                }"""

if target_single_fx_knob not in code:
    print("ERROR: target_single_fx_knob not found")
    sys.exit(1)

code = code.replace(target_single_fx_knob, replacement_single_fx_knob, 1)

# 7. Waveform stroke thickness and custom colors
target_wave_stroke = "                        int line_radius = 4;"
replacement_wave_stroke = "                        int line_radius = std::clamp(wave_line_thickness, 1, 8);"

if target_wave_stroke not in code:
    print("ERROR: target_wave_stroke not found")
    sys.exit(1)

code = code.replace(target_wave_stroke, replacement_wave_stroke, 1)

target_wave_color = """                    } else if (color_mode == 4) { // SUNSET
                        float sum = l + m + h + 0.0001f;
                        ir = (uint8_t)std::clamp((int)((145 * l + 255 * m + 255 * h) / sum), 0, 255);
                        ig = (uint8_t)std::clamp((int)((30 * l + 85 * m + 215 * h) / sum), 0, 255);
                        ib = (uint8_t)std::clamp((int)((160 * l + 0 * m + 0 * h) / sum), 0, 255);
                    } else { // RGB OFICIAL PIONEER REKORDBOX"""

replacement_wave_color = """                    } else if (color_mode == 4) { // SUNSET
                        float sum = l + m + h + 0.0001f;
                        ir = (uint8_t)std::clamp((int)((145 * l + 255 * m + 255 * h) / sum), 0, 255);
                        ig = (uint8_t)std::clamp((int)((30 * l + 85 * m + 215 * h) / sum), 0, 255);
                        ib = (uint8_t)std::clamp((int)((160 * l + 0 * m + 0 * h) / sum), 0, 255);
                    } else if (color_mode == 5) { // PERSONALIZADO DO DESIGNER UI/UX
                        float sum = l + m + h + 0.0001f;
                        ir = (uint8_t)std::clamp(((custom_wave_color_bass[0] * l + custom_wave_color_mid[0] * m + custom_wave_color_high[0] * h) / sum) * 255.0f, 0.0f, 255.0f);
                        ig = (uint8_t)std::clamp(((custom_wave_color_bass[1] * l + custom_wave_color_mid[1] * m + custom_wave_color_high[1] * h) / sum) * 255.0f, 0.0f, 255.0f);
                        ib = (uint8_t)std::clamp(((custom_wave_color_bass[2] * l + custom_wave_color_mid[2] * m + custom_wave_color_high[2] * h) / sum) * 255.0f, 0.0f, 255.0f);
                    } else { // RGB OFICIAL PIONEER REKORDBOX"""

if target_wave_color not in code:
    print("ERROR: target_wave_color not found")
    sys.exit(1)

code = code.replace(target_wave_color, replacement_wave_color, 1)

# Write to file
if os.path.exists(bak_path):
    os.remove(bak_path)
os.rename(path, bak_path)
with open(path, "w", encoding="utf-8") as f:
    f.write(code)
os.remove(bak_path)
print("SUCCESS part 2")

