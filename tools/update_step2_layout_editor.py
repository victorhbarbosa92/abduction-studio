import sys
import os

path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\DJLayoutEditorUI.h'

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Add DrawFittedText right above TouchSlider
old_class_start = """    class DJLayoutEditorUI {
    public:
        // Helper universal para Sliders táteis (Touch & Mouse Friendly) com botões de incremento/decremento"""

new_class_start = """    class DJLayoutEditorUI {
    public:
        // =========================================================================
        // MOTOR UNIVERSAL ANTI-COLISÃO DE TEXTO (TEXT-IN-BOX AUTO-FIT CLAMPING)
        // Garante que nenhum texto ultrapasse o retângulo do botão ou componente
        // =========================================================================
        static void DrawFittedText(ImDrawList* dl, ImFont* font, float target_size,
                                   ImVec2 box_min, ImVec2 box_max, ImU32 color,
                                   const char* text, float pad_x = 3.0f, float pad_y = 2.0f) {
            if (!dl || !text || text[0] == '\\0') return;
            if (!font) font = ImGui::GetFont();

            float box_w = box_max.x - box_min.x;
            float box_h = box_max.y - box_min.y;
            float avail_w = std::max(4.0f, box_w - pad_x * 2.0f);
            float avail_h = std::max(4.0f, box_h - pad_y * 2.0f);

            ImVec2 t_sz = font->CalcTextSizeA(target_size, FLT_MAX, 0.0f, text);
            float scale_factor = 1.0f;
            if (t_sz.x > avail_w && t_sz.x > 0.001f) scale_factor = std::min(scale_factor, avail_w / t_sz.x);
            if (t_sz.y > avail_h && t_sz.y > 0.001f) scale_factor = std::min(scale_factor, avail_h / t_sz.y);

            float eff_size = std::clamp(target_size * scale_factor, 7.0f, target_size);
            ImVec2 final_sz = font->CalcTextSizeA(eff_size, FLT_MAX, 0.0f, text);

            float tx = box_min.x + (box_w - final_sz.x) * 0.5f;
            float ty = box_min.y + (box_h - final_sz.y) * 0.5f;

            dl->AddText(font, eff_size, ImVec2(tx, ty), color, text);
        }

        // Helper universal para Sliders táteis (Touch & Mouse Friendly) com botões de incremento/decremento"""

assert old_class_start in c, "old_class_start not found"
c = c.replace(old_class_start, new_class_start)

# 2. Update DrawCanvasGizmo HUD pill with D-Pad Arrows and Collision Indicator
old_hud = """                // Floating On-Canvas Quick Action HUD Pill
                float hud_w = 310.0f;
                float hud_h = 32.0f;
                float hud_x = std::clamp(min.x + (max.x - min.x - hud_w) * 0.5f, 10.0f, ImGui::GetIO().DisplaySize.x - hud_w - 10.0f);
                float hud_y = (min.y > 45.0f) ? (min.y - hud_h - 6.0f) : (max.y + 6.0f);

                ImVec2 hud_min = ImVec2(hud_x, hud_y);
                ImVec2 hud_max = ImVec2(hud_x + hud_w, hud_y + hud_h);

                dl->AddRectFilled(hud_min, hud_max, IM_COL32(14, 18, 26, 245), 5.0f);
                dl->AddRect(hud_min, hud_max, IM_COL32(255, 195, 20, 220), 5.0f, 0, 1.5f);

                ImGui::SetCursorScreenPos(ImVec2(hud_min.x + 8.0f, hud_min.y + 4.0f));
                ImGui::PushID(name);

                // Badge do Nome
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.20f, 1.0f), "%s", name);
                ImGui::SameLine(0, 8.0f);

                // Alternador de Orientação: [ ↔ ] vs [ ↕ ]
                if (p_orientation) {
                    bool is_vert = (*p_orientation == 1);
                    ImGui::PushStyleColor(ImGuiCol_Button, is_vert ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.18f, 0.24f, 0.32f, 1.0f));
                    if (ImGui::Button(is_vert ? ICON_FA_ARROWS_UP_DOWN " ↕" : ICON_FA_ARROWS_LEFT_RIGHT " ↔", ImVec2(44.0f, 24.0f))) {
                        *p_orientation = 1 - *p_orientation;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar Orientação (%s -> Clique para %s)",
                                                                 is_vert ? "VERTICAL" : "HORIZONTAL",
                                                                 is_vert ? "HORIZONTAL" : "VERTICAL");
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0, 6.0f);
                }

                // Ajuste de Dimensão Rápido: [-] e [+]
                if (p_dim1) {
                    if (ImGui::Button("-##gizmo_sub", ImVec2(26.0f, 24.0f))) {
                        *p_dim1 = std::clamp(*p_dim1 - dim1_step, dim1_min, dim1_max);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Diminuir tamanho (-%.0f)", dim1_step);
                    ImGui::SameLine(0, 4.0f);
                    if (ImGui::Button("+##gizmo_add", ImVec2(26.0f, 24.0f))) {
                        *p_dim1 = std::clamp(*p_dim1 + dim1_step, dim1_min, dim1_max);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aumentar tamanho (+%.0f)", dim1_step);
                    ImGui::SameLine(0, 6.0f);
                }

                // Alça de Arraste Livre: [ ✥ Mover ]
                if (p_offset_x || p_offset_y) {
                    ImGui::Button(ICON_FA_UP_DOWN_LEFT_RIGHT "##drag_gizmo", ImVec2(28.0f, 24.0f));
                    if (ImGui::IsItemActive()) {
                        if (p_offset_x) *p_offset_x += ImGui::GetIO().MouseDelta.x;
                        if (p_offset_y) *p_offset_y += ImGui::GetIO().MouseDelta.y;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Segure e arraste para reposicionar livremente!");
                    ImGui::SameLine(0, 6.0f);
                }

                // Botão Fechar/Desmarcar: [ ✕ ]
                if (ImGui::Button(ICON_FA_XMARK "##close_gizmo", ImVec2(24.0f, 24.0f))) {
                    metrics.selected_edit_widget = WIDGET_NONE;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Desmarcar objeto");

                ImGui::PopID();"""

new_hud = """                // Floating On-Canvas Quick Action HUD Pill com Setas Direcionais D-Pad
                float hud_w = 440.0f;
                float hud_h = 34.0f;
                float hud_x = std::clamp(min.x + (max.x - min.x - hud_w) * 0.5f, 10.0f, ImGui::GetIO().DisplaySize.x - hud_w - 10.0f);
                float hud_y = (min.y > 48.0f) ? (min.y - hud_h - 6.0f) : (max.y + 6.0f);

                ImVec2 hud_min = ImVec2(hud_x, hud_y);
                ImVec2 hud_max = ImVec2(hud_x + hud_w, hud_y + hud_h);

                dl->AddRectFilled(hud_min, hud_max, IM_COL32(14, 18, 26, 245), 5.0f);
                dl->AddRect(hud_min, hud_max, IM_COL32(255, 195, 20, 220), 5.0f, 0, 1.5f);

                ImGui::SetCursorScreenPos(ImVec2(hud_min.x + 8.0f, hud_min.y + 4.0f));
                ImGui::PushID(name);

                // Badge do Nome
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.20f, 1.0f), "%s", name);
                ImGui::SameLine(0, 8.0f);

                // D-Pad Setas Direcionais (⬅ ⬆ ⬇ ➡)
                if (p_offset_x || p_offset_y) {
                    float arr_w = 26.0f;
                    float arr_h = 24.0f;
                    if (p_offset_x) {
                        if (ImGui::Button(ICON_FA_ARROW_LEFT "##dpad_l", ImVec2(arr_w, arr_h))) {
                            *p_offset_x -= 5.0f;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mover para a Esquerda (-5px)");
                        ImGui::SameLine(0, 2.0f);
                    }
                    if (p_offset_y) {
                        if (ImGui::Button(ICON_FA_ARROW_UP "##dpad_u", ImVec2(arr_w, arr_h))) {
                            *p_offset_y -= 5.0f;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mover para Cima (-5px)");
                        ImGui::SameLine(0, 2.0f);

                        if (ImGui::Button(ICON_FA_ARROW_DOWN "##dpad_d", ImVec2(arr_w, arr_h))) {
                            *p_offset_y += 5.0f;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mover para Baixo (+5px)");
                        ImGui::SameLine(0, 2.0f);
                    }
                    if (p_offset_x) {
                        if (ImGui::Button(ICON_FA_ARROW_RIGHT "##dpad_r", ImVec2(arr_w, arr_h))) {
                            *p_offset_x += 5.0f;
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mover para a Direita (+5px)");
                        ImGui::SameLine(0, 4.0f);
                    }
                }

                // Alternador de Orientação: [ ↔ ] vs [ ↕ ]
                if (p_orientation) {
                    bool is_vert = (*p_orientation == 1);
                    ImGui::PushStyleColor(ImGuiCol_Button, is_vert ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.18f, 0.24f, 0.32f, 1.0f));
                    if (ImGui::Button(is_vert ? ICON_FA_ARROWS_UP_DOWN " ↕" : ICON_FA_ARROWS_LEFT_RIGHT " ↔", ImVec2(40.0f, 24.0f))) {
                        *p_orientation = 1 - *p_orientation;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar Orientação (%s -> Clique para %s)",
                                                                 is_vert ? "VERTICAL" : "HORIZONTAL",
                                                                 is_vert ? "HORIZONTAL" : "VERTICAL");
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0, 4.0f);
                }

                // Ajuste de Dimensão Rápido: [-] e [+]
                if (p_dim1) {
                    if (ImGui::Button("-##gizmo_sub", ImVec2(24.0f, 24.0f))) {
                        *p_dim1 = std::clamp(*p_dim1 - dim1_step, dim1_min, dim1_max);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Diminuir tamanho (-%.0f)", dim1_step);
                    ImGui::SameLine(0, 2.0f);
                    if (ImGui::Button("+##gizmo_add", ImVec2(24.0f, 24.0f))) {
                        *p_dim1 = std::clamp(*p_dim1 + dim1_step, dim1_min, dim1_max);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aumentar tamanho (+%.0f)", dim1_step);
                    ImGui::SameLine(0, 4.0f);
                }

                // Alça de Arraste Livre: [ ✥ Mover ]
                if (p_offset_x || p_offset_y) {
                    ImGui::Button(ICON_FA_UP_DOWN_LEFT_RIGHT "##drag_gizmo", ImVec2(26.0f, 24.0f));
                    if (ImGui::IsItemActive()) {
                        if (p_offset_x) *p_offset_x += ImGui::GetIO().MouseDelta.x;
                        if (p_offset_y) *p_offset_y += ImGui::GetIO().MouseDelta.y;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Segure e arraste para reposicionar livremente!");
                    ImGui::SameLine(0, 4.0f);
                }

                // Botão Fechar/Desmarcar: [ ✕ ]
                if (ImGui::Button(ICON_FA_XMARK "##close_gizmo", ImVec2(22.0f, 24.0f))) {
                    metrics.selected_edit_widget = WIDGET_NONE;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Desmarcar objeto");

                ImGui::PopID();"""

assert old_hud in c, "old_hud not found"
c = c.replace(old_hud, new_hud)

# 3. Add WIDGET_HEADPHONES and new widgets button list in Tab 0
old_btns = """                            if (ImGui::Button(ICON_FA_WAVE_SQUARE " Waveforms Empilhadas", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_WAVEFORMS; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_BOLT " Rack de Efeitos FX", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_FX_BAR; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_TABLE_COLUMNS " Console do Mixer", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_MIXER_CONSOLE; }"""

new_btns = """                            if (ImGui::Button(ICON_FA_WAVE_SQUARE " Waveforms Empilhadas", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_WAVEFORMS; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_BOLT " Rack de Efeitos FX", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_FX_BAR; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_TABLE_COLUMNS " Console do Mixer", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_MIXER_CONSOLE; }

                            if (ImGui::Button(ICON_FA_HEADPHONES " Canais de Fone (Cue/Mix)", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_HEADPHONES; }"""

assert old_btns in c, "old_btns not found"
c = c.replace(old_btns, new_btns)

# 4. Add WIDGET_HEADPHONES and WIDGET_TRANSPORT cases in inspector
old_knobs_case = """                                case WIDGET_KNOBS_EQ: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_SLIDERS " KNOBS DE EQUALIZAÇÃO (TRIM, COMP, HIGH, MID, LOW)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Raio dos Knobs de EQ (Tamanho)", "knob_r_sel", &metrics.knob_radius_eq, 8.0f, 30.0f, 1.0f, "R: %.0f px", 18.0f, "Diâmetro dos botões de equalização.");
                                    TouchSlider("Espaçamento Vertical entre Knobs (Gap)", "knob_sp_sel", &metrics.knob_spacing, 2.0f, 35.0f, 1.0f, "%.0f px", 8.0f, "Distância de respiro entre os botões para nunca sobrepor.");
                                    TouchSlider("Deslocamento Vertical Y (Arrastar)", "knob_off_y", &metrics.mixer_knobs_offset_y, -50.0f, 50.0f, 2.0f, "%.0f px", 0.0f, "Posição vertical do bloco de equalização.");
                                    break;
                                }"""

new_knobs_case = """                                case WIDGET_HEADPHONES: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_HEADPHONES " CANAIS DE FONE & MONITORAMENTO (HEADPHONE CUE BAR)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Largura da Barra de Fone", "hp_w_sel", &metrics.headphone_bar_width, 140.0f, 500.0f, 10.0f, "%.0f px", 240.0f, "Largura horizontal da barra de fones.");
                                    TouchSlider("Altura da Barra de Fone", "hp_h_sel", &metrics.headphone_bar_height, 18.0f, 44.0f, 2.0f, "%.0f px", 24.0f, "Altura vertical dos controles de fone.");
                                    TouchSlider("Deslocamento Horizontal X (Arrastar)", "hp_off_x", &metrics.headphone_offset_x, -150.0f, 150.0f, 2.0f, "%.0f px", 0.0f, "Posição livre horizontal dos fones.");
                                    TouchSlider("Deslocamento Vertical Y (Arrastar)", "hp_off_y", &metrics.headphone_offset_y, -100.0f, 100.0f, 2.0f, "%.0f px", 0.0f, "Posição livre vertical para nunca sobrepor os knobs.");
                                    break;
                                }
                                case WIDGET_KNOBS_EQ: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_SLIDERS " KNOBS DE EQUALIZAÇÃO & COMPRESSOR DOURADO");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Escala do Compressor Dourado (COMP)", "comp_s_sel", &metrics.comp_knob_scale, 1.00f, 2.20f, 0.05f, "%.2fx", 1.35f, "Tamanho destacado do knob dourado e preto do compressor.");
                                    TouchSlider("Raio dos Knobs de EQ (Tamanho)", "knob_r_sel", &metrics.knob_radius_eq, 8.0f, 32.0f, 1.0f, "R: %.0f px", 18.0f, "Diâmetro dos botões de equalização.");
                                    TouchSlider("Espaçamento Vertical entre Knobs (Gap)", "knob_sp_sel", &metrics.knob_spacing, 2.0f, 35.0f, 1.0f, "%.0f px", 8.0f, "Distância de respiro entre os botões para nunca sobrepor.");
                                    TouchSlider("Deslocamento Vertical Y (Arrastar)", "knob_off_y", &metrics.mixer_knobs_offset_y, -80.0f, 80.0f, 2.0f, "%.0f px", 0.0f, "Posição vertical do bloco de equalização.");
                                    break;
                                }"""

assert old_knobs_case in c, "old_knobs_case not found"
c = c.replace(old_knobs_case, new_knobs_case)

# 5. Update WIDGET_PADS case to support pad_grid_mode (16 pads)
old_pads_case = """                                case WIDGET_PADS: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_GRIP " PERFORMANCE PADS");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Largura dos Performance Pads", "pad_w_sel", &metrics.pad_width, 28.0f, 140.0f, 2.0f, "%.0f px", 56.0f, "Largura individual de cada pad.");
                                    TouchSlider("Altura dos Performance Pads", "pad_h_sel", &metrics.pad_height, 20.0f, 120.0f, 2.0f, "%.0f px", 42.0f, "Altura individual de cada pad.");
                                    TouchSlider("Espaçamento entre Pads (Gap)", "pad_g_sel", &metrics.pad_gap, 1.0f, 16.0f, 1.0f, "%.0f px", 5.0f, "Distância de respiro entre os pads.");
                                    TouchSlider("Escala da Fonte dos Pads", "pad_f_sel", &metrics.pad_font_scale, 0.80f, 2.00f, 0.05f, "%.2fx", 1.20f, "Legibilidade das legendas e números.");
                                    break;
                                }"""

new_pads_case = """                                case WIDGET_PADS: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_GRIP " PERFORMANCE PADS (MODO 8 PADS & 16 PADS PARA TV 65\\\")");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    ImGui::Text("Grade de Pads:");
                                    bool is_p8 = (metrics.pad_grid_mode == 0);
                                    bool is_p16w = (metrics.pad_grid_mode == 1);
                                    bool is_p16m = (metrics.pad_grid_mode == 2);
                                    if (ImGui::RadioButton("8 Pads (4x2 Padrão)", is_p8)) metrics.pad_grid_mode = 0;
                                    ImGui::SameLine(0, 12.0f);
                                    if (ImGui::RadioButton("16 Pads Wide (8x2 para TV 65\\\")", is_p16w)) metrics.pad_grid_mode = 1;
                                    ImGui::SameLine(0, 12.0f);
                                    if (ImGui::RadioButton("16 Pads Matrix (4x4)", is_p16m)) metrics.pad_grid_mode = 2;
                                    ImGui::Dummy(ImVec2(0.0f, 4.0f));

                                    TouchSlider("Largura dos Performance Pads", "pad_w_sel", &metrics.pad_width, 28.0f, 160.0f, 2.0f, "%.0f px", 56.0f, "Largura individual de cada pad.");
                                    TouchSlider("Altura dos Performance Pads", "pad_h_sel", &metrics.pad_height, 20.0f, 130.0f, 2.0f, "%.0f px", 42.0f, "Altura individual de cada pad.");
                                    TouchSlider("Espaçamento entre Pads (Gap)", "pad_g_sel", &metrics.pad_gap, 1.0f, 16.0f, 1.0f, "%.0f px", 5.0f, "Distância de respiro entre os pads.");
                                    TouchSlider("Escala da Fonte dos Pads", "pad_f_sel", &metrics.pad_font_scale, 0.80f, 2.00f, 0.05f, "%.2fx", 1.20f, "Legibilidade das legendas e números.");
                                    break;
                                }"""

assert old_pads_case in c, "old_pads_case not found"
c = c.replace(old_pads_case, new_pads_case)

# 6. Update WIDGET_TRANSPORT case with cue_play_gap
old_transport_case = """                                case WIDGET_TRANSPORT: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_PLAY " TRANSPORTE (PLAY / PAUSE & CUE)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Raio dos Botões Redondos Play/Cue", "tr_r_sel", &metrics.transport_button_radius, 12.0f, 42.0f, 1.0f, "R: %.0f px", 24.0f, "Tamanho dos botões circulares.");
                                    TouchSlider("Largura da Coluna de Transporte", "tr_w_sel", &metrics.transport_column_width, 60.0f, 130.0f, 4.0f, "%.0f px", 82.0f, "Largura dedicada da coluna lateral.");
                                    break;
                                }"""

new_transport_case = """                                case WIDGET_TRANSPORT: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_PLAY " TRANSPORTE (CUE & PLAY ACOPLADOS COM ESPAÇAMENTO MÍNIMO)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Raio dos Botões Redondos CUE / PLAY", "tr_r_sel", &metrics.transport_button_radius, 12.0f, 48.0f, 1.0f, "R: %.0f px", 24.0f, "Tamanho dos botões circulares.");
                                    TouchSlider("Espaçamento Mínimo entre CUE e PLAY", "tr_gap_sel", &metrics.cue_play_gap, 2.0f, 24.0f, 1.0f, "%.0f px", 8.0f, "Distância estrita entre o botão CUE e o botão PLAY.");
                                    TouchSlider("Largura da Coluna de Transporte", "tr_w_sel", &metrics.transport_column_width, 60.0f, 150.0f, 4.0f, "%.0f px", 82.0f, "Largura dedicada da coluna lateral.");
                                    TouchSlider("Deslocamento X do Transporte", "tr_off_x", &metrics.transport_offset_x, -100.0f, 100.0f, 2.0f, "%.0f px", 0.0f, "Posição horizontal livre do bloco Play/Cue.");
                                    TouchSlider("Deslocamento Y do Transporte", "tr_off_y", &metrics.transport_offset_y, -100.0f, 100.0f, 2.0f, "%.0f px", 0.0f, "Posição vertical livre do bloco Play/Cue.");
                                    break;
                                }"""

assert old_transport_case in c, "old_transport_case not found"
c = c.replace(old_transport_case, new_transport_case)

# 7. Update Tab 1 (Geral & Escala) with ui_font_scale and enable_collision_physics
old_tab1 = """                        TouchSlider("Tamanho da Fonte de BPM / Pitch", "bpm_font_scale", &metrics.bpm_font_scale,
                                    0.80f, 2.50f, 0.05f, "%.2fx", 1.30f,
                                    "Escala da tipografia de BPM nos decks para facilitar a leitura à distância.");"""

new_tab1 = """                        TouchSlider("Tamanho da Fonte de BPM / Pitch", "bpm_font_scale", &metrics.bpm_font_scale,
                                    0.80f, 2.50f, 0.05f, "%.2fx", 1.30f,
                                    "Escala da tipografia de BPM nos decks para facilitar a leitura à distância.");

                        TouchSlider("Escala Adicional de Fontes da UI (TV 65\\\")", "ui_font_scale", &metrics.ui_font_scale,
                                    0.70f, 1.60f, 0.05f, "%.2fx", 1.00f,
                                    "Aumenta a legibilidade da tipografia de botões em telas grandes sem permitir vazamento.");

                        ImGui::Checkbox("Ativar Física de Colisão de Objetos (AABB Collision Guard)", &metrics.enable_collision_physics);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Impede que faders, knobs, fones ou botões sobreponham uns aos outros ou sumam da bancada.");"""

assert old_tab1 in c, "old_tab1 not found"
c = c.replace(old_tab1, new_tab1)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("DJLayoutEditorUI.h successfully updated!")

