import sys
import os

path_bak = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h.bak_rb7'
path_target = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(path_bak, 'r', encoding='utf-8') as f:
    bak_lines = f.readlines()

# Extract lines 9584 to 9934 (0-indexed 9583 to 9934)
aux_and_mem_lines = bak_lines[9583:9935]
aux_and_mem_code = "".join(aux_and_mem_lines)

# Upgrade drawTactileBtn in Memory Cue to use DrawFittedText
old_mem_txt = """                // Texto centralizado
                ImU32 txt_col = is_active ? active_txt : (is_hov ? IM_COL32(245, 250, 255, 255) : IM_COL32(175, 190, 210, 240));
                ImVec2 t_sz = ImGui::CalcTextSize(label);
                dl->AddText(ImVec2(b_pos.x + (b_size.x - t_sz.x) * 0.5f, b_pos.y + (b_size.y - t_sz.y) * 0.5f), txt_col, label);"""

new_mem_txt = """                // Texto centralizado com Auto-Fit e Proteção de Colisão (Nunca vaza)
                ImU32 txt_col = is_active ? active_txt : (is_hov ? IM_COL32(245, 250, 255, 255) : IM_COL32(175, 190, 210, 240));
                auto& metrics_mc = KuroUI::DJLayoutMetrics::getInstance();
                ImFont* f_tactile = g_font_control_small ? g_font_control_small : (g_font_rekordbox_bold ? g_font_rekordbox_bold : ImGui::GetFont());
                DJLayoutEditorUI::DrawFittedText(dl, f_tactile, 11.0f * metrics_mc.ui_font_scale, b_pos, b_max, txt_col, label, 2.0f, 1.0f);"""

if old_mem_txt in aux_and_mem_code:
    aux_and_mem_code = aux_and_mem_code.replace(old_mem_txt, new_mem_txt)

with open(path_target, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Add #include "DJStudioRackUI.h"
inc_target = '#include "DJAcademyGameUI.h"'
inc_replace = '#include "DJAcademyGameUI.h"\n#include "DJStudioRackUI.h"'
if inc_target in c and '#include "DJStudioRackUI.h"' not in c:
    c = c.replace(inc_target, inc_replace, 1)

# 2. Insert aux_and_mem_code before renderRekordboxPitchFaderWidget
pitch_str = "void renderRekordboxPitchFaderWidget(KuroAudio::DJDeck& deck, ImVec2 pos, ImVec2 size, ImVec4 theme_col) {"
assert pitch_str in c, "renderRekordboxPitchFaderWidget not found"
c = c.replace(pitch_str, aux_and_mem_code + "\n\n        " + pitch_str, 1)

# 3. Remove duplicate 'auto& metrics = KuroUI::DJLayoutMetrics::getInstance();'
dup_metrics = """            float ch_w = (w - 12.0f) * 0.5f;

            // Seção inferior garantida para o Crossfader e Atribuição (Respiro amplo, nunca é cortada fora)
            auto& metrics = KuroUI::DJLayoutMetrics::getInstance();
            float bot_sec_h;"""

fix_metrics = """            float ch_w = (w - 12.0f) * 0.5f;

            // Seção inferior garantida para o Crossfader e Atribuição (Respiro amplo, nunca é cortada fora)
            float bot_sec_h;"""
assert dup_metrics in c, "dup_metrics not found"
c = c.replace(dup_metrics, fix_metrics, 1)

# 4. Add Topbar Capsule button for 19" Rack and shortcut F4
old_topbar = """            // [LIVE SYNTH & FX RACK (F3)]
            std::string live_btn_lbl = std::string(ICON_FA_MICROCHIP) + " LIVE & FX";
            capsuleBtn("##TopCapsuleLiveFX", live_btn_lbl.c_str(), show_live_synth_fx_rack,
                       IM_COL32(235, 75, 185, 255), IM_COL32(255, 215, 245, 255),
                       "LIVE PERFORMANCE RACK (F3): Sintetizador Acid 303 & Transições de Emergência Echo Out na tela central!", [&]() {
                show_live_synth_fx_rack = !show_live_synth_fx_rack;
            });"""

new_topbar = """            // [LIVE SYNTH & FX RACK (F3)]
            std::string live_btn_lbl = std::string(ICON_FA_MICROCHIP) + " LIVE & FX";
            capsuleBtn("##TopCapsuleLiveFX", live_btn_lbl.c_str(), show_live_synth_fx_rack,
                       IM_COL32(235, 75, 185, 255), IM_COL32(255, 215, 245, 255),
                       "LIVE PERFORMANCE RACK (F3): Sintetizador Acid 303 & Transições de Emergência Echo Out na tela central!", [&]() {
                show_live_synth_fx_rack = !show_live_synth_fx_rack;
            });

            // [19-INCH MODULAR STUDIO PERFORMANCE RACK (F4)]
            std::string rack_btn_lbl = std::string(ICON_FA_SERVER) + " RACK 19\\\"";
            capsuleBtn("##TopCapsuleRack19", rack_btn_lbl.c_str(), KuroUI::DJStudioRackUI::getInstance().isOpen(),
                       IM_COL32(0, 215, 245, 255), IM_COL32(215, 250, 255, 255),
                       "19\\\" MODULAR STUDIO PERFORMANCE RACK (F4): Chassi Pro Audio com 6 Unidades de Rack 1U/2U (Acid, Telemetria, Live Coach, 16 Pads, Macros e CFX)!", [&]() {
                KuroUI::DJStudioRackUI::getInstance().toggle();
            });"""
assert old_topbar in c, "old_topbar not found"
c = c.replace(old_topbar, new_topbar, 1)

# 5. Add F4 key and render call for DJStudioRackUI
old_render_end = """                KuroUI::DJAcademyGameUI::getInstance().render(engine, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
            }
            ImGui::End();"""

new_render_end = """                // --- 12. RACK MODULAR DE ESTÚDIO 19\" (F4) ---
                if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F4, false)) {
                    KuroUI::DJStudioRackUI::getInstance().toggle();
                }

                KuroUI::DJAcademyGameUI::getInstance().render(engine, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
                KuroUI::DJStudioRackUI::getInstance().render(engine);
            }
            ImGui::End();"""
assert old_render_end in c, "old_render_end not found"
c = c.replace(old_render_end, new_render_end, 1)

with open(path_target, 'w', encoding='utf-8') as f:
    f.write(c)

print("KuroDJStudioUI.h successfully patched with Auxiliary Controls, Memory Cue, Rack 19, and metric deduplication!")

