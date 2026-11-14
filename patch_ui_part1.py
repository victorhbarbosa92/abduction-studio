import re

# 1. Patch DJLayoutMetrics.h
metrics_file = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\DJLayoutMetrics.h'
with open(metrics_file, 'r', encoding='utf-8') as f:
    m_content = f.read()

old_metrics_part = '''        float transport_button_radius = 28.0f;   // Raio dos botões redondos de PLAY e CUE ampliado'''
new_metrics_part = '''        float transport_button_radius = 24.0f;   // Raio dos botões redondos de PLAY e CUE
        float transport_column_width = 82.0f;    // Largura dedicada da coluna de Play / Cue
        float deck_tier_split_ratio = 0.54f;     // Split vertical Jogwheel vs Performance Pads (0.30 a 0.75)
        float fx_bar_height = 64.0f;             // Altura configurável do rack de efeitos FX'''

if old_metrics_part in m_content:
    m_content = m_content.replace(old_metrics_part, new_metrics_part)

old_save_part = '''                f << "  \"transport_button_radius\": " << transport_button_radius << ",\\n";'''
new_save_part = '''                f << "  \"transport_button_radius\": " << transport_button_radius << ",\\n";
                f << "  \"transport_column_width\": " << transport_column_width << ",\\n";
                f << "  \"deck_tier_split_ratio\": " << deck_tier_split_ratio << ",\\n";
                f << "  \"fx_bar_height\": " << fx_bar_height << ",\\n";'''

if old_save_part in m_content:
    m_content = m_content.replace(old_save_part, new_save_part)

old_load_part = '''                transport_button_radius = JSON::parseFloat(json, "transport_button_radius", transport_button_radius);'''
new_load_part = '''                transport_button_radius = JSON::parseFloat(json, "transport_button_radius", transport_button_radius);
                transport_column_width = JSON::parseFloat(json, "transport_column_width", transport_column_width);
                deck_tier_split_ratio = JSON::parseFloat(json, "deck_tier_split_ratio", deck_tier_split_ratio);
                fx_bar_height = JSON::parseFloat(json, "fx_bar_height", fx_bar_height);'''

if old_load_part in m_content:
    m_content = m_content.replace(old_load_part, new_load_part)

with open(metrics_file, 'w', encoding='utf-8') as f:
    f.write(m_content)

print('Successfully patched DJLayoutMetrics.h')

# 2. Patch DJLayoutEditorUI.h
editor_file = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\DJLayoutEditorUI.h'
with open(editor_file, 'r', encoding='utf-8') as f:
    e_content = f.read()

# Make the HUD bar 2 rows so nothing is squished or clipped
old_hud_geom = '''            // Painel Flutuante Superior do Modo Edição (HUD Superior Dourado / Neon)
            float bar_h = 44.0f;
            float bar_w = std::min(vp->Size.x - 40.0f, 1380.0f);'''

new_hud_geom = '''            // Painel Flutuante Superior do Modo Edição (HUD Superior 2-Linhas com Liberdade Total)
            float bar_h = 76.0f;
            float bar_w = std::min(vp->Size.x - 30.0f, 1480.0f);'''

if old_hud_geom in e_content:
    e_content = e_content.replace(old_hud_geom, new_hud_geom)

old_sliders = '''                // 1. Sliders Rápidos de Dimensionamento em Tempo Real
                ImGui::PushItemWidth(90.0f);
                if (ImGui::SliderFloat("Escala Global##ESCG", &metrics.master_scale, 0.65f, 2.20f, "%.2fx")) {
                    // Escala global dinâmica
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Escala Global da Interface (0.65x a 2.20x)");

                ImGui::SameLine(0, 10.0f);
                if (ImGui::SliderFloat("Mixer##MXW", &metrics.mixer_width, 160.0f, 480.0f, "%.0fpx")) {
                    // Largura do Mixer
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Largura do Console do Mixer (160px a 480px)");

                ImGui::SameLine(0, 10.0f);
                if (ImGui::SliderFloat("Vinil/Jog##JOGD", &metrics.jogwheel_diameter, 100.0f, 440.0f, "%.0fpx")) {
                    // Diâmetro do Vinil
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Diâmetro do Prato de Vinil / Jogwheel (100px a 440px)");

                ImGui::SameLine(0, 10.0f);
                if (ImGui::SliderFloat("Knobs EQ##KNEQ", &metrics.knob_radius_eq, 8.0f, 30.0f, "R:%.0f")) {
                    // Knobs de EQ
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Raio dos Knobs de Equalização (8px a 30px)");

                ImGui::SameLine(0, 10.0f);
                if (ImGui::SliderFloat("Pads##PADW", &metrics.pad_width, 28.0f, 90.0f, "%.0fpx")) {
                    metrics.pad_height = metrics.pad_width * 0.75f;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tamanho dos 8 Performance Pads");

                ImGui::SameLine(0, 10.0f);
                if (ImGui::SliderFloat("BPM Font##BPMF", &metrics.bpm_font_scale, 0.8f, 2.5f, "%.1fx")) {
                    // Escala de fonte de BPM
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tamanho da Fonte dos Dígitos de BPM (0.8x a 2.5x)");
                ImGui::PopItemWidth();'''

new_sliders = '''                // LINHA 1: Escalas e Dimensões Principais
                ImGui::PushItemWidth(86.0f);
                if (ImGui::SliderFloat("Escala Global##ESCG", &metrics.master_scale, 0.50f, 3.00f, "%.2fx")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Escala Global da Interface (0.50x a 3.00x)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Mixer##MXW", &metrics.mixer_width, 140.0f, 600.0f, "%.0fpx")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Largura do Console do Mixer (140px a 600px)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Jogwheel##JOGD", &metrics.jogwheel_diameter, 80.0f, 500.0f, "%.0fpx")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Diâmetro do Prato de Vinil / Jogwheel (80px a 500px)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Knobs EQ##KNEQ", &metrics.knob_radius_eq, 8.0f, 32.0f, "R:%.0f")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Raio dos Knobs de Equalização (8px a 32px)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Pads Largura##PADW", &metrics.pad_width, 28.0f, 130.0f, "%.0fpx")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Largura dos 8 Performance Pads (28px a 130px)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Pads Altura##PADH", &metrics.pad_height, 20.0f, 110.0f, "%.0fpx")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Altura dos Performance Pads (20px a 110px)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Play/Cue##TRBTN", &metrics.transport_button_radius, 12.0f, 42.0f, "R:%.0f")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Raio dos botões Play/Pause e CUE (12px a 42px)");

                ImGui::SameLine(0, 8.0f);
                if (ImGui::SliderFloat("Split Jog/Pads##SPLITR", &metrics.deck_tier_split_ratio, 0.30f, 0.75f, "%.2f")) {}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Divisão vertical entre Jogwheel (Superior) e Pads (Inferior)");

                ImGui::PopItemWidth();

                // LINHA 2: Configurações de Motor, Alongamento, Presets e Ações
                ImGui::Dummy(ImVec2(0.0f, 1.0f));
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_SLIDERS " MOTOR & EFEITOS:");
                ImGui::SameLine(0, 10.0f);'''

if old_sliders in e_content:
    e_content = e_content.replace(old_sliders, new_sliders)

with open(editor_file, 'w', encoding='utf-8') as f:
    f.write(e_content)

print('Successfully patched DJLayoutEditorUI.h')

