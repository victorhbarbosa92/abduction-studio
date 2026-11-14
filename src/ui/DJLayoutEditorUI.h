#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "DJLayoutMetrics.h"
#include "UIScaleManager.h"
#include <string>
#include <algorithm>
#include <cstdio>

namespace KuroUI {

    class DJLayoutEditorUI {
    public:
        // =========================================================================
        // MOTOR UNIVERSAL ANTI-COLISÃO DE TEXTO (TEXT-IN-BOX AUTO-FIT CLAMPING)
        // Garante que nenhum texto ultrapasse o retângulo do botão ou componente
        // =========================================================================
        static void DrawFittedText(ImDrawList* dl, ImFont* font, float target_size,
                                   ImVec2 box_min, ImVec2 box_max, ImU32 color,
                                   const char* text, float pad_x = 3.0f, float pad_y = 2.0f) {
            if (!dl || !text || text[0] == '\0') return;
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

        // Helper universal para Sliders táteis (Touch & Mouse Friendly) com botões de incremento/decremento
        static bool TouchSlider(const char* label, const char* id, float* p_val, float v_min, float v_max, 
                                float step, const char* fmt, float default_val, const char* tooltip) {
            if (!p_val) return false;
            *p_val = std::clamp(*p_val, v_min, v_max);
            ImGui::PushID(id);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            
            // 1. Linha Superior: Nome do Controle + Badge de Valor Estilizado
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.92f, 0.94f, 0.98f, 1.0f), "%s", label);
            
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), fmt, *p_val);
            ImVec2 val_sz = ImGui::CalcTextSize(val_buf);
            
            float avail_w = ImGui::GetContentRegionAvail().x;
            float badge_w = val_sz.x + 20.0f;
            ImGui::SameLine(avail_w - badge_w + ImGui::GetCursorPosX() - 6.0f);
            
            ImVec2 badge_min = ImGui::GetCursorScreenPos();
            ImVec2 badge_max = ImVec2(badge_min.x + badge_w, badge_min.y + 22.0f);
            dl->AddRectFilled(badge_min, badge_max, IM_COL32(18, 24, 34, 255), 3.0f);
            dl->AddRect(badge_min, badge_max, IM_COL32(0, 200, 255, 180), 3.0f);
            dl->AddText(ImVec2(badge_min.x + 10.0f, badge_min.y + 3.0f), IM_COL32(0, 230, 255, 255), val_buf);
            ImGui::Dummy(ImVec2(badge_w, 22.0f));
            
            // 2. Linha de Ação Tátil: [ - ] ============= Slider ============= [ + ] [ ⟲ ]
            float btn_w = 42.0f;
            float btn_h = 32.0f;
            float gap = 6.0f;
            float reset_w = 34.0f;
            float slider_w = std::max(120.0f, avail_w - btn_w * 2.0f - reset_w - gap * 3.0f);
            
            bool changed = false;
            
            // Botão [ - ] (Subtrair passo)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.26f, 0.36f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.60f, 0.90f, 1.0f));
            if (ImGui::Button("-##sub", ImVec2(btn_w, btn_h))) {
                *p_val = std::clamp(*p_val - step, v_min, v_max);
                changed = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reduzir (-%s)", fmt);
            
            ImGui::SameLine(0, gap);
            
            // Slider Largo Amplo (Fácil de arrastar com mouse ou toque)
            ImGui::PushItemWidth(slider_w);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 7.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.13f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 0.78f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.20f, 0.95f, 1.0f, 1.0f));
            if (ImGui::SliderFloat("##sld", p_val, v_min, v_max, "")) {
                changed = true;
            }
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    *p_val = default_val;
                    changed = true;
                }
                if (tooltip && tooltip[0]) {
                    ImGui::SetTooltip("%s\n(Duplo clique para restaurar padrão: %s)", tooltip, val_buf);
                }
            }
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();
            ImGui::PopItemWidth();
            
            ImGui::SameLine(0, gap);
            
            // Botão [ + ] (Somar passo)
            if (ImGui::Button("+##add", ImVec2(btn_w, btn_h))) {
                *p_val = std::clamp(*p_val + step, v_min, v_max);
                changed = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aumentar (+%s)", fmt);
            
            ImGui::SameLine(0, gap);
            
            // Botão Reset [ ⟲ ]
            if (ImGui::Button(ICON_FA_ROTATE_LEFT "##rst", ImVec2(reset_w, btn_h))) {
                *p_val = default_val;
                changed = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Restaurar valor padrão");
            
            ImGui::PopStyleColor(3);
            ImGui::PopID();
            
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            return changed;
        }

        // Helper universal para Sliders inteiros táteis (passos discretos como 4, 8, 16 batidas)
        static bool IntTouchSlider(const char* label, const char* id, int* p_val, int v_min, int v_max, 
                                   int step, const char* fmt, int default_val, const char* tooltip) {
            if (!p_val) return false;
            *p_val = std::clamp(*p_val, v_min, v_max);
            ImGui::PushID(id);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            
            // 1. Linha Superior: Nome do Controle + Badge de Valor Estilizado
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.92f, 0.94f, 0.98f, 1.0f), "%s", label);
            
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), fmt, *p_val);
            ImVec2 val_sz = ImGui::CalcTextSize(val_buf);
            
            float avail_w = ImGui::GetContentRegionAvail().x;
            float badge_w = val_sz.x + 20.0f;
            ImGui::SameLine(avail_w - badge_w + ImGui::GetCursorPosX() - 6.0f);
            
            ImVec2 badge_min = ImGui::GetCursorScreenPos();
            ImVec2 badge_max = ImVec2(badge_min.x + badge_w, badge_min.y + 22.0f);
            dl->AddRectFilled(badge_min, badge_max, IM_COL32(18, 24, 34, 255), 3.0f);
            dl->AddRect(badge_min, badge_max, IM_COL32(0, 200, 255, 180), 3.0f);
            dl->AddText(ImVec2(badge_min.x + 10.0f, badge_min.y + 3.0f), IM_COL32(0, 230, 255, 255), val_buf);
            ImGui::Dummy(ImVec2(badge_w, 22.0f));
            
            // 2. Linha de Ação Tátil: [ - ] ============= Slider ============= [ + ] [ ↺ ]
            float btn_w = 42.0f;
            float btn_h = 32.0f;
            float gap = 6.0f;
            float reset_w = 34.0f;
            float slider_w = std::max(120.0f, avail_w - btn_w * 2.0f - reset_w - gap * 3.0f);
            
            bool changed = false;
            
            // Botão [ - ] (Subtrair passo)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.26f, 0.36f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.60f, 0.90f, 1.0f));
            if (ImGui::Button("-##sub", ImVec2(btn_w, btn_h))) {
                *p_val = std::clamp(*p_val - step, v_min, v_max);
                changed = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reduzir");
            
            ImGui::SameLine(0, gap);
            
            // Slider Largo Amplo
            ImGui::PushItemWidth(slider_w);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 7.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.13f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 0.78f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.20f, 0.95f, 1.0f, 1.0f));
            if (ImGui::SliderInt("##sld", p_val, v_min, v_max, "")) {
                changed = true;
            }
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    *p_val = default_val;
                    changed = true;
                }
                if (tooltip && tooltip[0]) {
                    ImGui::SetTooltip("%s\n(Duplo clique para restaurar padrão: %s)", tooltip, val_buf);
                }
            }
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();
            ImGui::PopItemWidth();
            
            ImGui::SameLine(0, gap);
            
            // Botão [ + ] (Somar passo)
            if (ImGui::Button("+##add", ImVec2(btn_w, btn_h))) {
                *p_val = std::clamp(*p_val + step, v_min, v_max);
                changed = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aumentar");
            
            ImGui::SameLine(0, gap);
            
            // Botão Reset [ ↺ ]
            if (ImGui::Button(ICON_FA_ROTATE_LEFT "##rst", ImVec2(reset_w, btn_h))) {
                *p_val = default_val;
                changed = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Restaurar valor padrão");
            
            ImGui::PopStyleColor(3);
            ImGui::PopID();
            
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            return changed;
        }

        // =========================================================================
        // ENGINE VISUAL INTERATIVA (GIZMO NO CANVAS: SELEÇÃO, TAMANHO, ORIENTAÇÃO & ARRASTO)
        // =========================================================================
        static bool DrawCanvasGizmo(DJSelectableWidgetId id, const char* name, ImVec2 min, ImVec2 max,
                                    int* p_orientation = nullptr, float* p_dim1 = nullptr, float dim1_step = 2.0f,
                                    float dim1_min = 10.0f, float dim1_max = 500.0f,
                                    float* p_offset_x = nullptr, float* p_offset_y = nullptr) {
            auto& metrics = DJLayoutMetrics::getInstance();
            if (!metrics.edit_mode_active) return false;

            ImDrawList* dl = ImGui::GetForegroundDrawList();
            bool is_selected = (metrics.selected_edit_widget == (int)id);
            bool is_hovered = ImGui::IsMouseHoveringRect(min, max);

            if (is_hovered && !is_selected) {
                // Efeito Hover: Borda Ciano Pulsante Neon com Tag de Identificação
                dl->AddRect(min, max, IM_COL32(0, 225, 255, 190), 3.0f, 0, 1.8f);
                ImVec2 t_sz = ImGui::CalcTextSize(name);
                dl->AddRectFilled(ImVec2(min.x + 3.0f, min.y + 3.0f), ImVec2(min.x + t_sz.x + 16.0f, min.y + 20.0f), IM_COL32(10, 16, 26, 235), 3.0f);
                dl->AddRect(ImVec2(min.x + 3.0f, min.y + 3.0f), ImVec2(min.x + t_sz.x + 16.0f, min.y + 20.0f), IM_COL32(0, 200, 255, 180), 3.0f);
                dl->AddText(ImVec2(min.x + 8.0f, min.y + 4.0f), IM_COL32(0, 235, 255, 255), name);

                if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
                    metrics.selected_edit_widget = (int)id;
                    return true;
                }
            }

            if (is_selected) {
                // Efeito Selecionado: Moldura Dourada Pioneer com Alças nos 4 Cantos
                dl->AddRect(min, max, IM_COL32(255, 195, 20, 255), 4.0f, 0, 2.5f);
                float hs = 8.0f;
                dl->AddRectFilled(ImVec2(min.x - 2.0f, min.y - 2.0f), ImVec2(min.x + hs, min.y + hs), IM_COL32(255, 195, 20, 255));
                dl->AddRectFilled(ImVec2(max.x - hs, min.y - 2.0f), ImVec2(max.x + 2.0f, min.y + hs), IM_COL32(255, 195, 20, 255));
                dl->AddRectFilled(ImVec2(min.x - 2.0f, max.y - hs), ImVec2(min.x + hs, max.y + 2.0f), IM_COL32(255, 195, 20, 255));
                dl->AddRectFilled(ImVec2(max.x - hs, max.y - hs), ImVec2(max.x + 2.0f, max.y + 2.0f), IM_COL32(255, 195, 20, 255));

                // Tag discreta no canto com o nome do objeto (Zero colisão com as ondas)
                ImVec2 t_sz = ImGui::CalcTextSize(name);
                float tag_w = t_sz.x + 18.0f;
                float tag_h = 18.0f;
                float tag_x = min.x + 6.0f;
                float tag_y = min.y + 4.0f;
                dl->AddRectFilled(ImVec2(tag_x, tag_y), ImVec2(tag_x + tag_w, tag_y + tag_h), IM_COL32(10, 16, 26, 240), 3.0f);
                dl->AddRect(ImVec2(tag_x, tag_y), ImVec2(tag_x + tag_w, tag_y + tag_h), IM_COL32(255, 195, 20, 220), 3.0f);
                dl->AddText(ImVec2(tag_x + 6.0f, tag_y + 1.5f), IM_COL32(255, 205, 30, 255), name);

                // Quick Action HUD Pill: Janela Flutuante Top-Level Isolada (Zero Colisão/Mutação com Janelas-Filhas)
                if (id != WIDGET_WAVEFORMS && (p_offset_x || p_offset_y || p_orientation)) {
                    float hud_w = 300.0f;
                    float hud_h = 32.0f;
                    float hud_x = std::clamp(min.x + (max.x - min.x - hud_w) * 0.5f, 10.0f, ImGui::GetIO().DisplaySize.x - hud_w - 10.0f);
                    float hud_y = (min.y > 54.0f) ? (min.y - hud_h - 4.0f) : (max.y + 4.0f);

                    char hud_win_id[64];
                    snprintf(hud_win_id, sizeof(hud_win_id), "##GizmoHUD_%s", name);
                    ImGui::SetNextWindowPos(ImVec2(hud_x, hud_y));
                    ImGui::SetNextWindowSize(ImVec2(hud_w, hud_h));
                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.10f, 0.96f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.76f, 0.08f, 0.86f));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));

                    ImGuiWindowFlags hud_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

                    if (ImGui::Begin(hud_win_id, nullptr, hud_flags)) {
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
                            if (ImGui::Button(is_vert ? ICON_FA_ARROWS_UP_DOWN " ↕" : ICON_FA_ARROWS_LEFT_RIGHT " ↔", ImVec2(38.0f, 24.0f))) {
                                *p_orientation = 1 - *p_orientation;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar Orientação (%s -> Clique para %s)",
                                                                         is_vert ? "VERTICAL" : "HORIZONTAL",
                                                                         is_vert ? "HORIZONTAL" : "VERTICAL");
                            ImGui::PopStyleColor();
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
                    }
                    ImGui::End();
                    ImGui::PopStyleVar(3);
                    ImGui::PopStyleColor(2);
                }
            }

            return is_selected;
        }

        static void RenderEditToolbar() {
            auto& metrics = DJLayoutMetrics::getInstance();
            if (!metrics.edit_mode_active) return;

            ImGuiViewport* vp = ImGui::GetMainViewport();

            // Dimensões compactas e adaptativas que respeitam a tela e liberam o topo para as waveforms
            float win_w = std::clamp(vp->Size.x * 0.72f, 560.0f, 920.0f);
            float win_h = std::clamp(vp->Size.y * 0.44f, 250.0f, 380.0f);
            float dock_bottom_x = vp->Pos.x + (vp->Size.x - win_w) * 0.5f;
            float dock_bottom_y = vp->Pos.y + vp->Size.y - win_h - 28.0f;

            if (metrics.snap_editor_to_bottom) {
                ImGui::SetNextWindowPos(ImVec2(dock_bottom_x, dock_bottom_y), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(win_w, win_h), ImGuiCond_Always);
                metrics.snap_editor_to_bottom = false;
            } else {
                ImGui::SetNextWindowSize(ImVec2(win_w, win_h), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(ImVec2(dock_bottom_x, dock_bottom_y), ImGuiCond_FirstUseEver);
            }

            // Altura segura abaixo das waveforms para anti-obstrução automática
            float wave_safe_bottom = vp->Pos.y + 26.0f + std::max(92.0f, metrics.waveform_height) + 8.0f;

            ImGuiWindowFlags flags = ImGuiWindowFlags_None; // Permite colapsar / minimizar com duplo clique

            // Tema visual escuro Club com borda dourada Pioneer e transparência configurável
            static float editor_alpha = 0.94f;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.11f, editor_alpha));
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.12f, 0.16f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.15f, 0.20f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.70f, 0.10f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));

            bool window_open = metrics.edit_mode_active;
            if (ImGui::Begin(ICON_FA_SLIDERS "  ESTÚDIO DE ESCALONAMENTO E LAYOUT (WYSIWYG)", &window_open, flags)) {
                metrics.edit_mode_active = window_open;
                if (!window_open) {
                    metrics.saveToFile();
                    metrics.snap_editor_to_bottom = true;
                }

                // Proteção anti-obstrução automática: Se a janela estiver sobre as waveforms no topo quando estiver editando ondas
                ImVec2 cur_win_pos = ImGui::GetWindowPos();
                if (cur_win_pos.y < wave_safe_bottom && (metrics.selected_edit_widget == WIDGET_WAVEFORMS || metrics.requested_tab_idx == 5)) {
                    ImGui::SetWindowPos(ImVec2(cur_win_pos.x, std::max(dock_bottom_y, wave_safe_bottom)));
                }

                // Cabeçalho de Orientação com Ações Rápidas de Posicionamento
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_PEN_RULER " PERSONALIZAÇÃO VISUAL & ERGONOMIA LIVRE");
                
                float avail_head = ImGui::GetContentRegionAvail().x;
                float btn_w1 = 175.0f;
                float btn_w2 = 150.0f;
                if (avail_head > (btn_w1 + btn_w2 + 20.0f)) {
                    ImGui::SameLine(avail_head - (btn_w1 + btn_w2 + 10.0f));
                    if (ImGui::SmallButton(ICON_FA_ARROW_DOWN " Encaixar no Rodapé")) {
                        ImGui::SetWindowPos(ImVec2(dock_bottom_x, dock_bottom_y));
                        ImGui::SetWindowSize(ImVec2(win_w, win_h));
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Encaixa a janela no rodapé para liberar visão total das Waveforms e Decks");

                    ImGui::SameLine(0, 6.0f);
                    if (ImGui::SmallButton(editor_alpha < 0.8f ? ICON_FA_EYE " Transparente (40%)" : ICON_FA_EYE_SLASH " Sólido (94%)")) {
                        editor_alpha = (editor_alpha < 0.8f) ? 0.94f : 0.40f;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alterna a transparência para enxergar através dos controles");
                }

                ImGui::TextColored(ImVec4(0.70f, 0.80f, 0.90f, 0.90f), "%s", 
                    "Ajuste cada módulo com [+] e [-] ou sliders. As Waveforms permanecem 100% visíveis no topo.");
                ImGui::Dummy(ImVec2(0.0f, 4.0f));

                if (ImGui::BeginTabBar("##LayoutStudioTabs", ImGuiTabBarFlags_None)) {
                    
                    // =========================================================================
                    // ABA 0: OBJETO SELECIONADO (INSPECTOR VISUAL INSTANTÂNEO & ORIENTAÇÃO)
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_CROSSHAIRS " Objeto Selecionado", nullptr, (metrics.requested_tab_idx == 0 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 0) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));

                        if (metrics.selected_edit_widget == WIDGET_NONE) {
                            ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_HAND_POINTER " CLIQUE EM QUALQUER ELEMENTO NA MESA:");
                            ImGui::TextColored(ImVec4(0.70f, 0.80f, 0.90f, 0.90f), 
                                "Clique diretamente nos Knobs de EQ, Faders de Volume, Crossfader, Jogwheels ou Pads\n"
                                "para inspecionar e editar aqui instantaneamente.");
                            ImGui::Dummy(ImVec2(0.0f, 8.0f));
                            ImGui::Separator();
                            ImGui::Dummy(ImVec2(0.0f, 6.0f));

                            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), "OU SELECIONE UM MÓDULO PARA CUSTOMIZAR:");
                            float bw = 210.0f;
                            float bh = 36.0f;
                            if (ImGui::Button(ICON_FA_SLIDERS " Knobs de Equalização (EQ)", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_KNOBS_EQ; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN " Faders de Volume", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_CHANNEL_FADERS; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT " Crossfader Central", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_CROSSFADER; }

                            if (ImGui::Button(ICON_FA_COMPACT_DISC " Pratos / Jogwheel", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_JOGWHEEL; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_GRIP " Performance Pads", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_PADS; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_PLAY " Botões Play / Cue", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_TRANSPORT; }

                            if (ImGui::Button(ICON_FA_WAVE_SQUARE " Waveforms Empilhadas", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_WAVEFORMS; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_BOLT " Rack de Efeitos FX", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_FX_BAR; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_TABLE_COLUMNS " Console do Mixer", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_MIXER_CONSOLE; }

                            if (ImGui::Button(ICON_FA_HEADPHONES " Canais de Fone (Cue/Mix)", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_HEADPHONES; }
                            ImGui::SameLine(0, 10.0f);
                            if (ImGui::Button(ICON_FA_GAUGE " Visor de BPM / Pitch", ImVec2(bw, bh))) { metrics.selected_edit_widget = WIDGET_TEMPO_PANEL; }
                        } else {
                            // Painel de edição do objeto ativo
                            if (ImGui::Button(ICON_FA_XMARK " Desmarcar Objeto Ativo", ImVec2(180.0f, 28.0f))) {
                                metrics.selected_edit_widget = WIDGET_NONE;
                            }
                            ImGui::Dummy(ImVec2(0.0f, 6.0f));

                            switch (metrics.selected_edit_widget) {
                                case WIDGET_CROSSFADER: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_ARROWS_LEFT_RIGHT " CROSSFADER CENTRAL (Batalha / Transição)");
                                    ImGui::Dummy(ImVec2(0.0f, 4.0f));

                                    // Alternador de Orientação
                                    ImGui::Text("Orientação do Crossfader:");
                                    bool is_h = (metrics.crossfader_orientation == 0);
                                    ImGui::PushStyleColor(ImGuiCol_Button, is_h ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                                    if (ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT " Modo Horizontal ↔ (Padrão)", ImVec2(240.0f, 36.0f))) {
                                        metrics.crossfader_orientation = 0;
                                    }
                                    ImGui::PopStyleColor();
                                    ImGui::SameLine(0, 12.0f);
                                    ImGui::PushStyleColor(ImGuiCol_Button, !is_h ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                                    if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN " Modo Vertical ↕ (Batalha/Scratch)", ImVec2(260.0f, 36.0f))) {
                                        metrics.crossfader_orientation = 1;
                                    }
                                    ImGui::PopStyleColor();

                                    ImGui::Dummy(ImVec2(0.0f, 8.0f));
                                    TouchSlider("Comprimento / Largura da Trilha", "xf_w_sel", &metrics.crossfader_width, 100.0f, 400.0f, 10.0f, "%.0f px", 250.0f, "Extensão do trilho do crossfader.");
                                    TouchSlider("Altura / Pegada Tátil", "xf_h_sel", &metrics.crossfader_height, 22.0f, 56.0f, 2.0f, "%.0f px", 36.0f, "Área de toque e altura do cap.");
                                    TouchSlider("Deslocamento Horizontal (Arrastar X)", "xf_off_x", &metrics.crossfader_offset_x, -120.0f, 120.0f, 2.0f, "%.0f px", 0.0f, "Posição livre horizontal do crossfader.");
                                    TouchSlider("Deslocamento Vertical (Arrastar Y)", "xf_off_y", &metrics.crossfader_offset_y, -60.0f, 60.0f, 2.0f, "%.0f px", 0.0f, "Posição livre vertical do crossfader.");
                                    break;
                                }
                                case WIDGET_HEADPHONES: {
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
                                    TouchSlider("Escala do Compressor Dourado (COMP)", "comp_s_sel", &metrics.comp_knob_scale, 1.00f, 2.50f, 0.05f, "%.2fx", 1.35f, "Tamanho destacado do knob dourado e preto do compressor.");
                                    TouchSlider("Raio dos Knobs de EQ (Tamanho)", "knob_r_sel", &metrics.knob_radius_eq, 8.0f, 45.0f, 1.0f, "R: %.0f px", 18.0f, "Diâmetro dos botões de equalização.");
                                    TouchSlider("Espaçamento Vertical entre Knobs (Gap)", "knob_sp_sel", &metrics.knob_spacing, 2.0f, 80.0f, 1.0f, "%.0f px", 8.0f, "Distância de respiro entre os botões para nunca sobrepor.");
                                    TouchSlider("Deslocamento Vertical Y (Arrastar)", "knob_off_y", &metrics.mixer_knobs_offset_y, -150.0f, 150.0f, 2.0f, "%.0f px", 0.0f, "Posição vertical do bloco de equalização.");
                                    break;
                                }
                                case WIDGET_CHANNEL_FADERS: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_ARROWS_UP_DOWN " FADERS DE CANAL (VOLUME DE ÁUDIO)");
                                    ImGui::Dummy(ImVec2(0.0f, 4.0f));

                                    // Alternador de Orientação dos Faders de Canal
                                    ImGui::Text("Orientação dos Faders de Canal:");
                                    bool is_f_v = (metrics.channel_fader_orientation == 0);
                                    ImGui::PushStyleColor(ImGuiCol_Button, is_f_v ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                                    if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN " Modo Vertical ↕ (Padrão Club)", ImVec2(240.0f, 36.0f))) {
                                        metrics.channel_fader_orientation = 0;
                                    }
                                    ImGui::PopStyleColor();
                                    ImGui::SameLine(0, 12.0f);
                                    ImGui::PushStyleColor(ImGuiCol_Button, !is_f_v ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                                    if (ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT " Modo Horizontal ↔ (Ergonomia Ampla)", ImVec2(260.0f, 36.0f))) {
                                        metrics.channel_fader_orientation = 1;
                                    }
                                    ImGui::PopStyleColor();

                                    ImGui::Dummy(ImVec2(0.0f, 8.0f));
                                    TouchSlider("Comprimento / Altura do Fader", "fader_h_sel", &metrics.fader_height, 40.0f, 500.0f, 5.0f, "%.0f px", 180.0f, "Comprimento da trilha do fader de canal.");
                                    TouchSlider("Largura da Pegada do Fader (Knob Cap)", "fader_w_sel", &metrics.fader_width, 16.0f, 56.0f, 2.0f, "%.0f px", 28.0f, "Espessura da pegada para toque e clique.");
                                    TouchSlider("Deslocamento Vertical Y (Arrastar)", "fader_off_y", &metrics.mixer_faders_offset_y, -150.0f, 150.0f, 2.0f, "%.0f px", 0.0f, "Posição vertical do bloco de faders.");
                                    TouchSlider("Altura do Botão CUE do Mixer", "cue_btn_h_sel", &metrics.mixer_cue_btn_h, 14.0f, 42.0f, 1.0f, "%.0f px", 20.0f, "Altura do botão de pré-escuta CUE.");
                                    TouchSlider("Largura do Botão CUE do Mixer", "cue_btn_w_sel", &metrics.mixer_cue_btn_w, 24.0f, 90.0f, 2.0f, "%.0f px", 42.0f, "Largura do botão de pré-escuta CUE.");
                                    TouchSlider("Respiro Anti-Colisão CUE vs Fader", "cue_fad_gap_sel", &metrics.cue_fader_gap, 6.0f, 40.0f, 1.0f, "%.0f px", 12.0f, "Distância de segurança rígida para o cap nunca colidir com o CUE.");
                                    break;
                                }
                                case WIDGET_TEMPO_PANEL: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_GAUGE " VISOR DE BPM, PITCH, TOM (KEY) & MEDIDOR DE FASE");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Escala da Fonte de BPM / Visor", "bpm_f_s", &metrics.bpm_font_scale, 0.80f, 2.80f, 0.05f, "%.2fx", 1.30f, "Tamanho dos dígitos de BPM no display.");
                                    TouchSlider("Altura do Painel de BPM (0 = Auto-Fit)", "bpm_p_h", &metrics.side_tempo_panel_height, 0.0f, 450.0f, 5.0f, "%.0f px", 0.0f, "Altura fixa (0 = se ajusta automaticamente sem sobras pretas).");
                                    ImGui::Checkbox("Exibir Tom Musical (Key) e Botão Key Sync", &metrics.show_side_tempo_key);
                                    ImGui::Checkbox("Exibir Botão Pioneer Master Tempo (MT)", &metrics.show_side_tempo_master);
                                    ImGui::Checkbox("Exibir Medidor de Fase de Compasso (1-2-3-4)", &metrics.show_side_tempo_phase_meter);
                                    break;
                                }
                                case WIDGET_JOGWHEEL: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_COMPACT_DISC " PRATOS DE VINIL & JOGWHEELS");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Diâmetro do Prato de Vinil / Jogwheel", "jog_d_sel", &metrics.jogwheel_diameter, 80.0f, 500.0f, 10.0f, "%.0f px", 240.0f, "Tamanho total do disco giratório.");
                                    TouchSlider("Proporção do Display Central On-Jog", "jog_c_sel", &metrics.jogwheel_center_ratio, 0.20f, 0.65f, 0.02f, "%.2f", 0.42f, "Tamanho do mostrador LCD central.");
                                    ImGui::Checkbox("Modo Motorizado / Inércia de Rotação", &metrics.jogwheel_motorized_mode);
                                    break;
                                }
                                case WIDGET_PADS: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_GRIP " PERFORMANCE PADS (MODO 8 PADS & 16 PADS PARA TV 65\")");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    ImGui::Text("Grade de Pads:");
                                    bool is_p8 = (metrics.pad_grid_mode == 0);
                                    bool is_p16w = (metrics.pad_grid_mode == 1);
                                    bool is_p16m = (metrics.pad_grid_mode == 2);
                                    if (ImGui::RadioButton("8 Pads (4x2 Padrão)", is_p8)) metrics.pad_grid_mode = 0;
                                    ImGui::SameLine(0, 12.0f);
                                    if (ImGui::RadioButton("16 Pads Wide (8x2 para TV 65\")", is_p16w)) metrics.pad_grid_mode = 1;
                                    ImGui::SameLine(0, 12.0f);
                                    if (ImGui::RadioButton("16 Pads Matrix (4x4)", is_p16m)) metrics.pad_grid_mode = 2;
                                    ImGui::Dummy(ImVec2(0.0f, 4.0f));

                                    TouchSlider("Largura dos Performance Pads", "pad_w_sel", &metrics.pad_width, 28.0f, 160.0f, 2.0f, "%.0f px", 56.0f, "Largura individual de cada pad.");
                                    TouchSlider("Altura dos Performance Pads", "pad_h_sel", &metrics.pad_height, 20.0f, 130.0f, 2.0f, "%.0f px", 42.0f, "Altura individual de cada pad.");
                                    TouchSlider("Espaçamento entre Pads (Gap)", "pad_g_sel", &metrics.pad_gap, 1.0f, 16.0f, 1.0f, "%.0f px", 5.0f, "Distância de respiro entre os pads.");
                                    TouchSlider("Escala da Fonte dos Pads", "pad_f_sel", &metrics.pad_font_scale, 0.80f, 2.00f, 0.05f, "%.2fx", 1.20f, "Legibilidade das legendas e números.");
                                    break;
                                }
                                case WIDGET_TRANSPORT: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_PLAY " TRANSPORTE (CUE & PLAY ACOPLADOS COM ESPAÇAMENTO MÍNIMO)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Raio dos Botões Redondos CUE / PLAY", "tr_r_sel", &metrics.transport_button_radius, 12.0f, 48.0f, 1.0f, "R: %.0f px", 24.0f, "Tamanho dos botões circulares.");
                                    TouchSlider("Espaçamento Mínimo entre CUE e PLAY", "tr_gap_sel", &metrics.cue_play_gap, 2.0f, 24.0f, 1.0f, "%.0f px", 8.0f, "Distância estrita entre o botão CUE e o botão PLAY.");
                                    TouchSlider("Largura da Coluna de Transporte", "tr_w_sel", &metrics.transport_column_width, 60.0f, 150.0f, 4.0f, "%.0f px", 82.0f, "Largura dedicada da coluna lateral.");
                                    TouchSlider("Deslocamento X do Transporte", "tr_off_x", &metrics.transport_offset_x, -100.0f, 100.0f, 2.0f, "%.0f px", 0.0f, "Posição horizontal livre do bloco Play/Cue.");
                                    TouchSlider("Deslocamento Y do Transporte", "tr_off_y", &metrics.transport_offset_y, -100.0f, 100.0f, 2.0f, "%.0f px", 0.0f, "Posição vertical livre do bloco Play/Cue.");
                                    break;
                                }
                                case WIDGET_WAVEFORMS: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_WAVE_SQUARE " FORMAS DE ONDA EMPILHADAS (WAVEFORMS)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Altura das Waveforms Empilhadas", "wav_h_sel", &metrics.waveform_height, 70.0f, 320.0f, 10.0f, "%.0f px", 165.0f, "Espaço vertical das ondas.");
                                    IntTouchSlider("Batidas no Visor (Zoom em Batidas)", "wav_b_sel", &metrics.waveform_visible_beats, 4, 64, 4, "%d batidas", 16, "Quantidade de batidas exibidas de ponta a ponta na janela (4 a 64).");
                                    TouchSlider("Velocidade da Rolagem da Waveform", "wav_spd_sel", &metrics.waveform_scroll_speed, 0.50f, 3.00f, 0.10f, "%.2fx", 1.00f, "Velocidade em que as formas de onda correm na tela.");
                                    TouchSlider("Alongamento Horizontal de Transientes", "wav_s_sel", &metrics.waveform_horizontal_stretch, 1.00f, 4.00f, 0.10f, "%.2fx", 1.65f, "Abre a forma de onda horizontalmente (Padrão anterior: 1.65x).");
                                    
                                    ImGui::Dummy(ImVec2(0.0f, 4.0f));
                                    ImGui::Text("Ajuste Rápido de Visor:");
                                    float qw = (ImGui::GetContentRegionAvail().x - 12.0f) / 3.0f;
                                    if (ImGui::Button("Padrão Anterior (16B / 1.65x)", ImVec2(qw, 28.0f))) {
                                        metrics.waveform_visible_beats = 16;
                                        metrics.waveform_horizontal_stretch = 1.65f;
                                        metrics.waveform_scroll_speed = 1.0f;
                                        metrics.waveform_engine_mode = 0;
                                        metrics.needs_waveform_recompute = true;
                                    }
                                    ImGui::SameLine(0, 6.0f);
                                    if (ImGui::Button("Modo Compacto (32B / 1.0x)", ImVec2(qw, 28.0f))) {
                                        metrics.waveform_visible_beats = 32;
                                        metrics.waveform_horizontal_stretch = 1.00f;
                                        metrics.waveform_scroll_speed = 1.0f;
                                        metrics.needs_waveform_recompute = true;
                                    }
                                    ImGui::SameLine(0, 6.0f);
                                    if (ImGui::Button("Super Zoom (8B / 2.0x)", ImVec2(qw, 28.0f))) {
                                        metrics.waveform_visible_beats = 8;
                                        metrics.waveform_horizontal_stretch = 2.00f;
                                        metrics.waveform_scroll_speed = 1.0f;
                                        metrics.needs_waveform_recompute = true;
                                    }
                                    ImGui::Dummy(ImVec2(0.0f, 4.0f));

                                    const char* color_opts_sel[] = {
                                        "RGB Espectral", "3-Band CDJ-3000", "Blue Nexus", "Neon Cyberpunk", "Sunset Solar",
                                        "Emerald Matrix", "Vaporwave 80s", "Inferno Magma", "Glacier Frost", "Monochrome Platinum"
                                    };
                                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                    if (ImGui::Combo("##WaveColorComboSel", &metrics.waveform_color_mode, color_opts_sel, IM_ARRAYSIZE(color_opts_sel))) {
                                        metrics.needs_waveform_recompute = true;
                                    }
                                    break;
                                }
                                case WIDGET_FX_BAR: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_BOLT " RACK DE EFEITOS FX (BEAT FX & CFX)");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Altura do Rack de Efeitos", "fx_h_sel", &metrics.fx_bar_height, 48.0f, 110.0f, 4.0f, "%.0f px", 64.0f, "Altura total da barra horizontal superior.");
                                    break;
                                }
                                case WIDGET_MIXER_CONSOLE:
                                default: {
                                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), ICON_FA_TABLE_COLUMNS " CONSOLE DO MIXER CENTRAL");
                                    ImGui::Dummy(ImVec2(0.0f, 6.0f));
                                    TouchSlider("Largura do Console do Mixer", "mix_w_sel", &metrics.mixer_width, 140.0f, 800.0f, 10.0f, "%.0f px", 280.0f, "Largura total entre os decks (escala livre para TV 65\").");
                                    TouchSlider("Espaçamento entre Canais (Gap)", "mix_g_sel", &metrics.mixer_channel_gap, 2.0f, 30.0f, 1.0f, "%.0f px", 6.0f, "Espaço entre canal esquerdo e direito.");
                                    TouchSlider("Largura do VU Meter LED", "mix_vu_w_sel", &metrics.vu_meter_width, 2.0f, 12.0f, 0.5f, "%.1f px", 5.0f, "Espessura da barra de níveis de áudio de cada canal.");
                                    break;
                                }
                            }
                        }

                        ImGui::EndTabItem();
                    }

                    // =========================================================================
                    // ABA 1: GERAL & ESCALA GLOBAL
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_EXPAND " Geral & Escala", nullptr, (metrics.requested_tab_idx == 1 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 1) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));
                        
                        TouchSlider("Escala Global da Interface", "master_scale", &metrics.master_scale, 
                                    0.50f, 3.00f, 0.05f, "%.2fx", 1.15f, 
                                    "Ajusta o tamanho de todos os botões, pratos, faders e tipografia simultaneamente.");

                        TouchSlider("Tamanho da Fonte de BPM / Pitch", "bpm_font_scale", &metrics.bpm_font_scale,
                                    0.80f, 2.50f, 0.05f, "%.2fx", 1.30f,
                                    "Escala da tipografia de BPM nos decks para facilitar a leitura à distância.");

                        TouchSlider("Escala Adicional de Fontes da UI (TV 65\")", "ui_font_scale", &metrics.ui_font_scale,
                                    0.70f, 1.60f, 0.05f, "%.2fx", 1.00f,
                                    "Aumenta a legibilidade da tipografia de botões em telas grandes sem permitir vazamento.");

                        ImGui::Checkbox("Ativar Física de Colisão de Objetos (AABB Collision Guard)", &metrics.enable_collision_physics);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Impede que faders, knobs, fones ou botões sobreponham uns aos outros ou sumam da bancada.");

                        TouchSlider("Tamanho da Fonte de Faixa & Tempos", "track_title_font_scale", &metrics.track_title_font_scale,
                                    0.80f, 2.00f, 0.05f, "%.2fx", 1.20f,
                                    "Escala das informações de título, artista e relógio de tempo.");

                        TouchSlider("Tamanho da Fonte dos Performance Pads", "pad_font_scale", &metrics.pad_font_scale,
                                    0.80f, 2.00f, 0.05f, "%.2fx", 1.20f,
                                    "Escala das legendas e números dentro dos 8 pads de cada deck.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        ImGui::Checkbox("Rasterização Ultra-Sharp 4K (Anti-Aliased)", &metrics.enable_anti_aliased_4k);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Renderiza curvas e linhas vetoriais com anti-aliasing orgânico de alta definição.");

                        ImGui::SameLine(0, 20.0f);
                        ImGui::Checkbox("Animações Dinâmicas e Glows", &metrics.enable_visual_animations);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ativa transições de iluminação, pulsos luminosos de beat e rotação a 60fps.");

                        ImGui::EndTabItem();
                    }

                    // =========================================================================
                    // ABA 2: MIXER CENTRAL & FADERS
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_SLIDERS " Mixer Central", nullptr, (metrics.requested_tab_idx == 2 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 2) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));

                        TouchSlider("Largura do Console do Mixer", "mixer_width", &metrics.mixer_width,
                                    140.0f, 600.0f, 10.0f, "%.0f px", 280.0f,
                                    "Largura total da coluna central do mixer entre os decks A e B.");

                        TouchSlider("Espaçamento entre Canais (Gap)", "mixer_channel_gap", &metrics.mixer_channel_gap,
                                    2.0f, 20.0f, 1.0f, "%.0f px", 6.0f,
                                    "Distância lateral entre os canais 1 e 2 no console.");

                        TouchSlider("Raio dos Knobs de Equalização (EQ)", "knob_radius_eq", &metrics.knob_radius_eq,
                                    8.0f, 30.0f, 1.0f, "R: %.0f px", 18.0f,
                                    "Diâmetro dos botões giratórios de High, Mid e Low.");

                        TouchSlider("Espaçamento Vertical entre Knobs de EQ (Gap)", "knob_spacing", &metrics.knob_spacing,
                                    2.0f, 35.0f, 1.0f, "%.0f px", 8.0f,
                                    "Distância vertical de respiro entre os knobs de TRIM, HIGH, MID e LOW para nunca sobrepor.");

                        TouchSlider("Deslocamento Vertical Y dos Knobs", "mixer_knobs_offset_y", &metrics.mixer_knobs_offset_y,
                                    -50.0f, 50.0f, 2.0f, "%.0f px", 0.0f,
                                    "Ajuste fino da posição vertical do rack de knobs de equalização.");

                        TouchSlider("Raio do Knob de Trim / Ganho", "knob_radius_gain", &metrics.knob_radius_gain,
                                    8.0f, 26.0f, 1.0f, "R: %.0f px", 16.0f,
                                    "Tamanho do botão giratório de TRIM de cada canal.");

                        TouchSlider("Raio dos Knobs de Sound Color FX", "knob_radius_cfx", &metrics.knob_radius_cfx,
                                    10.0f, 32.0f, 1.0f, "R: %.0f px", 26.0f,
                                    "Tamanho dos botões giratórios centrais de Sound Color FX.");

                        TouchSlider("Escala do Knob de Compressor DJM", "comp_knob_scale", &metrics.comp_knob_scale,
                                    1.00f, 1.60f, 0.05f, "%.2fx", 1.35f,
                                    "Escala visual de destaque do knob dourado de compressor.");

                        TouchSlider("Largura do Botão CUE do Mixer", "mixer_cue_btn_w", &metrics.mixer_cue_btn_w,
                                    24.0f, 65.0f, 2.0f, "%.0f px", 42.0f,
                                    "Largura do botão de monitoração de fone CUE do canal.");

                        TouchSlider("Altura do Botão CUE do Mixer", "mixer_cue_btn_h", &metrics.mixer_cue_btn_h,
                                    14.0f, 34.0f, 1.0f, "%.0f px", 20.0f,
                                    "Altura do botão CUE do canal.");

                        TouchSlider("Distância Anti-Colisão CUE / Fader", "cue_fader_gap", &metrics.cue_fader_gap,
                                    4.0f, 30.0f, 1.0f, "%.0f px", 12.0f,
                                    "Respiro mínimo garantido entre a tecla CUE e a calha do fader de volume.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        // Orientação dos Faders de Canal
                        ImGui::Text("Orientação dos Faders de Canal (Volume):");
                        bool is_cf_vert = (metrics.channel_fader_orientation == 0);
                        ImGui::PushStyleColor(ImGuiCol_Button, is_cf_vert ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN " ↕ Vertical (Padrão Club)", ImVec2(230.0f, 34.0f))) {
                            metrics.channel_fader_orientation = 0;
                        }
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0, 10.0f);
                        ImGui::PushStyleColor(ImGuiCol_Button, !is_cf_vert ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT " ↔ Horizontal (Ergonomia Ampla)", ImVec2(250.0f, 34.0f))) {
                            metrics.channel_fader_orientation = 1;
                        }
                        ImGui::PopStyleColor();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        TouchSlider("Altura dos Faders de Canal (Volume)", "fader_height", &metrics.fader_height,
                                    40.0f, 220.0f, 5.0f, "%.0f px", 150.0f,
                                    "Comprimento da trilha vertical do fader de volume de cada deck.");

                        TouchSlider("Largura da Pegada do Fader (Knob Cap)", "fader_width", &metrics.fader_width,
                                    16.0f, 44.0f, 2.0f, "%.0f px", 28.0f,
                                    "Largura física do cap de arrasto do fader (facilita o toque e clique).");

                        TouchSlider("Deslocamento Vertical Y dos Faders", "mixer_faders_offset_y", &metrics.mixer_faders_offset_y,
                                    -50.0f, 50.0f, 2.0f, "%.0f px", 0.0f,
                                    "Ajuste fino da posição vertical do rack de faders de canal.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        // Orientação do Crossfader
                        ImGui::Text("Orientação do Crossfader:");
                        bool is_xf_h = (metrics.crossfader_orientation == 0);
                        ImGui::PushStyleColor(ImGuiCol_Button, is_xf_h ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT " ↔ Horizontal (Padrão Rekordbox)", ImVec2(240.0f, 34.0f))) {
                            metrics.crossfader_orientation = 0;
                        }
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0, 10.0f);
                        ImGui::PushStyleColor(ImGuiCol_Button, !is_xf_h ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN " ↕ Vertical (Batalha / Scratch)", ImVec2(250.0f, 34.0f))) {
                            metrics.crossfader_orientation = 1;
                        }
                        ImGui::PopStyleColor();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        TouchSlider("Largura do Crossfader", "crossfader_width", &metrics.crossfader_width,
                                    120.0f, 400.0f, 10.0f, "%.0f px", 250.0f,
                                    "Comprimento horizontal do rasgo do crossfader na base do mixer.");

                        TouchSlider("Altura / Área Tátil do Crossfader", "crossfader_height", &metrics.crossfader_height,
                                    24.0f, 56.0f, 2.0f, "%.0f px", 36.0f,
                                    "Altura da área de clique e toque do crossfader.");

                        TouchSlider("Deslocamento Horizontal X do Crossfader", "crossfader_offset_x", &metrics.crossfader_offset_x,
                                    -120.0f, 120.0f, 2.0f, "%.0f px", 0.0f,
                                    "Posição livre horizontal do crossfader.");

                        TouchSlider("Deslocamento Vertical Y do Crossfader", "crossfader_offset_y", &metrics.crossfader_offset_y,
                                    -60.0f, 60.0f, 2.0f, "%.0f px", 0.0f,
                                    "Posição livre vertical do crossfader.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        TouchSlider("Largura da Barra de Fones", "headphone_bar_width", &metrics.headphone_bar_width,
                                    100.0f, 400.0f, 10.0f, "%.0f px", 240.0f,
                                    "Largura do painel de controle de fones de ouvido.");

                        TouchSlider("Altura da Barra de Fones", "headphone_bar_height", &metrics.headphone_bar_height,
                                    16.0f, 50.0f, 2.0f, "%.0f px", 24.0f,
                                    "Altura do painel de controle de fones de ouvido.");

                        TouchSlider("Deslocamento X da Barra de Fones", "headphone_offset_x", &metrics.headphone_offset_x,
                                    -150.0f, 150.0f, 2.0f, "%.0f px", 0.0f,
                                    "Posição horizontal livre do bloco de fone.");

                        TouchSlider("Deslocamento Y da Barra de Fones", "headphone_offset_y", &metrics.headphone_offset_y,
                                    -100.0f, 100.0f, 2.0f, "%.0f px", 0.0f,
                                    "Posição vertical livre do bloco de fone.");

                        ImGui::EndTabItem();
                    }

                    // =========================================================================
                    // ABA 3: PRATOS & VINIL / JOG
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_COMPACT_DISC " Vinil & Jog", nullptr, (metrics.requested_tab_idx == 3 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 3) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));

                        TouchSlider("Diâmetro do Prato de Vinil / Jogwheel", "jogwheel_diameter", &metrics.jogwheel_diameter,
                                    80.0f, 500.0f, 10.0f, "%.0f px", 240.0f,
                                    "Tamanho do disco giratório (permite desde pratos compactos até vinil de 12 polegadas).");

                        TouchSlider("Proporção do Display Central On-Jog", "jogwheel_center_ratio", &metrics.jogwheel_center_ratio,
                                    0.20f, 0.65f, 0.02f, "%.2f", 0.42f,
                                    "Tamanho do mostrador LCD / arte de capa no centro do jogwheel.");

                        TouchSlider("Divisão Vertical Deck (Jog vs Pads)", "deck_tier_split_ratio", &metrics.deck_tier_split_ratio,
                                    0.30f, 0.75f, 0.02f, "%.2f", 0.54f,
                                    "Proporção de altura entre a baia superior (Jogwheel) e a bancada inferior (Pads).");

                        TouchSlider("Sensibilidade do Scratch / Pitch Bend", "jogwheel_sensitivity", &metrics.jogwheel_sensitivity,
                                    0.20f, 3.00f, 0.05f, "%.2fx", 1.00f,
                                    "Multiplicador de resposta ao toque e rotação do jogwheel.");

                        TouchSlider("Altura do Painel de Pitch / Tempo", "side_tempo_panel_height", &metrics.side_tempo_panel_height,
                                    0.0f, 260.0f, 5.0f, "%.0f px", 0.0f,
                                    "Altura da baia do pitch fader (0 px = Auto-Fit dinâmico sem folgas).");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        ImGui::Checkbox("Modo Motorizado / Inércia de Rotação", &metrics.jogwheel_motorized_mode);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Simula inércia física do vinil com rotação suave contínua a 33/45 RPM.");

                        ImGui::Checkbox("Exibir Tom Musical (Key) e Key Sync", &metrics.show_side_tempo_key);
                        ImGui::Checkbox("Exibir Botão Master Tempo (Key Lock)", &metrics.show_side_tempo_master);
                        ImGui::Checkbox("Exibir Medidor de Fase de Compasso (1-2-3-4)", &metrics.show_side_tempo_phase_meter);

                        ImGui::EndTabItem();
                    }

                    // =========================================================================
                    // ABA 4: PADS & TRANSPORTE (PLAY / CUE)
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_GRIP " Pads & Transporte", nullptr, (metrics.requested_tab_idx == 4 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 4) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));

                        // Configuração da Grade de Pads
                        ImGui::Text("Configuração da Grade de Pads:");
                        bool is_p8 = (metrics.pad_grid_mode == 0);
                        bool is_p16w = (metrics.pad_grid_mode == 1);
                        bool is_p16m = (metrics.pad_grid_mode == 2);
                        float g_btn_w = (ImGui::GetContentRegionAvail().x - 12.0f) / 3.0f;
                        
                        ImGui::PushStyleColor(ImGuiCol_Button, is_p8 ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button("8 Pads (4x2)", ImVec2(g_btn_w, 30.0f))) metrics.pad_grid_mode = 0;
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0, 6.0f);
                        ImGui::PushStyleColor(ImGuiCol_Button, is_p16w ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button("16 Wide (8x2 TV)", ImVec2(g_btn_w, 30.0f))) metrics.pad_grid_mode = 1;
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0, 6.0f);
                        ImGui::PushStyleColor(ImGuiCol_Button, is_p16m ? ImVec4(0.0f, 0.55f, 0.85f, 1.0f) : ImVec4(0.14f, 0.18f, 0.25f, 1.0f));
                        if (ImGui::Button("16 Matrix (4x4)", ImVec2(g_btn_w, 30.0f))) metrics.pad_grid_mode = 2;
                        ImGui::PopStyleColor();

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        TouchSlider("Altura dos Performance Pads", "pad_height", &metrics.pad_height,
                                    20.0f, 120.0f, 2.0f, "%.0f px", 42.0f,
                                    "Altura individual de cada um dos pads de performance.");

                        TouchSlider("Largura dos Performance Pads", "pad_width", &metrics.pad_width,
                                    28.0f, 140.0f, 2.0f, "%.0f px", 56.0f,
                                    "Largura individual de cada pad de performance.");

                        TouchSlider("Espaçamento entre Pads (Gap)", "pad_gap", &metrics.pad_gap,
                                    1.0f, 16.0f, 1.0f, "%.0f px", 5.0f,
                                    "Distância de respiro entre as teclas de pad.");

                        TouchSlider("Tamanho da Fonte dos Pads", "pad_font_scale", &metrics.pad_font_scale,
                                    0.80f, 2.00f, 0.05f, "%.2fx", 1.20f,
                                    "Escala das legendas e números nos pads.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        TouchSlider("Raio dos Botões Redondos Play/Cue", "transport_button_radius", &metrics.transport_button_radius,
                                    12.0f, 42.0f, 1.0f, "R: %.0f px", 24.0f,
                                    "Tamanho dos botões circulares iluminados de CUE e PLAY/PAUSE.");

                        TouchSlider("Largura da Coluna de Transporte", "transport_column_width", &metrics.transport_column_width,
                                    60.0f, 130.0f, 4.0f, "%.0f px", 82.0f,
                                    "Largura da coluna lateral reservada exclusivamente para o Play e Cue.");

                        TouchSlider("Espaçamento Vertical CUE / PLAY (Gap)", "cue_play_gap", &metrics.cue_play_gap,
                                    2.0f, 32.0f, 1.0f, "%.0f px", 8.0f,
                                    "Espaçamento vertical de segurança entre os botões redondos CUE e PLAY.");

                        TouchSlider("Deslocamento X Botões Play/Cue", "transport_offset_x", &metrics.transport_offset_x,
                                    -50.0f, 50.0f, 1.0f, "%.0f px", 0.0f,
                                    "Ajuste horizontal livre da posição dos botões de transporte.");

                        TouchSlider("Deslocamento Y Botões Play/Cue", "transport_offset_y", &metrics.transport_offset_y,
                                    -50.0f, 50.0f, 1.0f, "%.0f px", 0.0f,
                                    "Ajuste vertical livre da posição dos botões de transporte.");

                        ImGui::EndTabItem();
                    }

                    // =========================================================================
                    // ABA 5: WAVEFORMS & EFEITOS
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_WAVE_SQUARE " Waveforms & FX", nullptr, (metrics.requested_tab_idx == 5 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 5) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));

                        TouchSlider("Altura do Rack de Efeitos (Beat FX & CFX)", "fx_bar_height", &metrics.fx_bar_height,
                                    48.0f, 110.0f, 4.0f, "%.0f px", 64.0f,
                                    "Altura da barra horizontal de efeitos no topo da mesa.");

                        TouchSlider("Altura das Waveforms Empilhadas", "waveform_height", &metrics.waveform_height,
                                    70.0f, 320.0f, 10.0f, "%.0f px", 165.0f,
                                    "Espaço vertical reservado para as formas de onda dos Decks 1 e 2.");

                        IntTouchSlider("Batidas no Visor (Zoom em Batidas)", "waveform_visible_beats", &metrics.waveform_visible_beats,
                                       4, 64, 4, "%d batidas", 16,
                                       "Quantidade de batidas exibidas de ponta a ponta na janela (4 a 64).");

                        TouchSlider("Velocidade da Rolagem da Waveform", "waveform_scroll_speed", &metrics.waveform_scroll_speed,
                                    0.50f, 3.00f, 0.10f, "%.2fx", 1.00f,
                                    "Velocidade em que as formas de onda correm na tela.");

                        TouchSlider("Alongamento Horizontal de Transientes", "waveform_horizontal_stretch", &metrics.waveform_horizontal_stretch,
                                    1.00f, 4.00f, 0.05f, "%.2fx", 1.65f,
                                    "Abre os transientes da música para visualização ampla (Padrão anterior: 1.65x).");

                        TouchSlider("Altura da Régua de Overview da Pista", "overview_height", &metrics.overview_height,
                                    16.0f, 48.0f, 2.0f, "%.0f px", 24.0f,
                                    "Altura da régua de visualização geral da música.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Text("Ajuste Rápido de Visor:");
                        float q_w = (ImGui::GetContentRegionAvail().x - 12.0f) / 3.0f;
                        if (ImGui::Button("Padrão Anterior (16B / 1.65x)", ImVec2(q_w, 28.0f))) {
                            metrics.waveform_visible_beats = 16;
                            metrics.waveform_horizontal_stretch = 1.65f;
                            metrics.waveform_scroll_speed = 1.0f;
                            metrics.waveform_engine_mode = 0;
                            metrics.needs_waveform_recompute = true;
                        }
                        ImGui::SameLine(0, 6.0f);
                        if (ImGui::Button("Modo Compacto (32B / 1.0x)", ImVec2(q_w, 28.0f))) {
                            metrics.waveform_visible_beats = 32;
                            metrics.waveform_horizontal_stretch = 1.00f;
                            metrics.waveform_scroll_speed = 1.0f;
                            metrics.needs_waveform_recompute = true;
                        }
                        ImGui::SameLine(0, 6.0f);
                        if (ImGui::Button("Super Zoom (8B / 2.0x)", ImVec2(q_w, 28.0f))) {
                            metrics.waveform_visible_beats = 8;
                            metrics.waveform_horizontal_stretch = 2.00f;
                            metrics.waveform_scroll_speed = 1.0f;
                            metrics.needs_waveform_recompute = true;
                        }

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 4.0f));

                        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), ICON_FA_SLIDERS " MOTOR DE RENDERIZAÇÃO DA WAVEFORM:");
                        const char* eng_opts[] = {
                            "3-Band Multi-Lane (Padrão Rekordbox 3 Baias HI/MID/LOW)",
                            "3-Band Fused (Fusão CDJ-3000 / Rekordbox 7 Oficial Sobreposta)",
                            "Rekordbox 7 RGB Espectral (Transientes em Linhas)",
                            "Multi-Ribbon Guitar Hero (Fitas Vetoriais Neon)",
                            "Fusão Híbrida (Linhas Rekordbox + Contorno Neon)"
                        };
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::Combo("##WaveEngineCombo", &metrics.waveform_engine_mode, eng_opts, IM_ARRAYSIZE(eng_opts))) {
                            metrics.ribbon_mode_active = (metrics.waveform_engine_mode == 3);
                        }

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), ICON_FA_PALETTE " PALETA DE CORES DA WAVEFORM:");
                        const char* color_opts[] = {
                            "RGB Espectral (Rekordbox & Traktor: Rosa Graves, Âmbar Médios, Ciano Agudos)",
                            "3-Band CDJ-3000 (Graves Azul Royal, Médios Âmbar, Agudos Branco Diamante)",
                            "Blue Nexus (CDJ-2000 Clássico Azul Monocromático)",
                            "Neon Cyberpunk (Graves Magenta, Médios Verde Lime, Agudos Ciano)",
                            "Sunset Solar (Graves Roxo/Vermelho, Médios Laranja, Agudos Dourado)",
                            "Emerald Matrix (Acid Techno: Graves Verde Floresta, Médios Lima Neon, Agudos Menta)",
                            "Vaporwave 80s (Synthwave: Graves Violeta, Médios Rosa Choque, Agudos Ciano)",
                            "Inferno Magma (Hardstyle: Graves Carmesim, Médios Lava Laranja, Agudos Ouro)",
                            "Glacier Frost (Trance: Graves Azul Abissal, Médios Turquesa Glacial, Agudos Branco)",
                            "Monochrome Platinum (Minimal: Graves Grafite, Médios Titânio, Agudos Branco Puro)"
                        };
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::Combo("##WaveColorCombo", &metrics.waveform_color_mode, color_opts, IM_ARRAYSIZE(color_opts))) {
                            metrics.needs_waveform_recompute = true;
                        }

                        ImGui::Dummy(ImVec2(0.0f, 2.0f));
                        ImGui::TextColored(ImVec4(0.70f, 0.85f, 1.0f, 1.0f), "Predefinições Rápidas de Cores (1-Clique):");
                        float col_btn_w = (ImGui::GetContentRegionAvail().x - 16.0f) / 5.0f;
                        struct ColorPresetQuick { const char* label; int mode; ImVec4 col; };
                        ColorPresetQuick quick_cols[10] = {
                            { "RGB", 0, ImVec4(1.00f, 0.10f, 0.55f, 1.0f) },
                            { "3-BAND", 1, ImVec4(0.00f, 0.55f, 1.00f, 1.0f) },
                            { "BLUE", 2, ImVec4(0.10f, 0.45f, 0.95f, 1.0f) },
                            { "NEON", 3, ImVec4(0.85f, 0.00f, 0.95f, 1.0f) },
                            { "SUNSET", 4, ImVec4(0.95f, 0.40f, 0.05f, 1.0f) },
                            { "EMERALD", 5, ImVec4(0.10f, 0.85f, 0.30f, 1.0f) },
                            { "VAPOR", 6, ImVec4(0.75f, 0.15f, 0.95f, 1.0f) },
                            { "INFERNO", 7, ImVec4(0.95f, 0.20f, 0.05f, 1.0f) },
                            { "GLACIER", 8, ImVec4(0.00f, 0.75f, 0.95f, 1.0f) },
                            { "MONO", 9, ImVec4(0.60f, 0.65f, 0.75f, 1.0f) }
                        };
                        for (int i = 0; i < 10; i++) {
                            if (i > 0 && i % 5 != 0) ImGui::SameLine(0, 4.0f);
                            bool is_active = (metrics.waveform_color_mode == quick_cols[i].mode);
                            if (is_active) {
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }
                            ImGui::PushStyleColor(ImGuiCol_Button, is_active ? ImVec4(quick_cols[i].col.x * 0.45f, quick_cols[i].col.y * 0.45f, quick_cols[i].col.z * 0.45f, 0.95f) : ImVec4(0.08f, 0.12f, 0.18f, 0.85f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(quick_cols[i].col.x * 0.60f, quick_cols[i].col.y * 0.60f, quick_cols[i].col.z * 0.60f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, quick_cols[i].col);
                            if (ImGui::Button(quick_cols[i].label, ImVec2(col_btn_w, 26.0f))) {
                                metrics.waveform_color_mode = quick_cols[i].mode;
                                metrics.needs_waveform_recompute = true;
                            }
                            ImGui::PopStyleColor(3);
                            if (is_active) ImGui::PopStyleColor();
                        }

                        ImGui::Dummy(ImVec2(0.0f, 6.0f));
                        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.20f, 1.0f), ICON_FA_CHART_SIMPLE " CONTROLES DE LINHAS & DINÂMICA (ESTILO REKORDBOX 7):");

                        const char* style_opts[] = {
                            "Linhas Verticais Nítidas Rekordbox 7 (Linhas Finas com Respiro)",
                            "Barras com Respiro (Discrete Stems com Intervalo)",
                            "Malha Sólida Contínua (Continuous Mesh - Bloco Antigo)"
                        };
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("##WaveLineStyleCombo", &metrics.waveform_line_style, style_opts, IM_ARRAYSIZE(style_opts));

                        TouchSlider("Espaçamento / Passo das Linhas", "waveform_line_spacing", &metrics.waveform_line_spacing,
                                    1.0f, 4.0f, 0.1f, "%.1f px", 1.8f,
                                    "Distância horizontal entre cada linha (1.0px = ultra-denso, 1.8px = padrão Rekordbox 7, 3.0px = bem espaçado).");

                        TouchSlider("Espessura de Cada Linha", "waveform_line_width", &metrics.waveform_line_width,
                                    0.8f, 3.0f, 0.1f, "%.1f px", 1.2f,
                                    "Grossura das linhas verticais (0.8px = agulhas finas, 1.2px = nítido equilibrado, 2.0px = encorpado).");

                        TouchSlider("Separação / Gap entre Linhas", "waveform_line_gap", &metrics.waveform_line_gap,
                                    0.0f, 2.0f, 0.1f, "%.1f px", 0.6f,
                                    "Respiro escuro entre as linhas verticais para evitar aspecto de waveform chapada.");

                        TouchSlider("Punch & Ataque de Transientes", "waveform_transient_punch", &metrics.waveform_transient_punch,
                                    0.50f, 3.00f, 0.05f, "%.2fx", 1.60f,
                                    "Multiplica o impacto vertical dos kicks e snares fazendo as agulhas saltarem no ritmo.");

                        TouchSlider("Contraste Dinâmico / Gamma", "waveform_dynamic_contrast", &metrics.waveform_dynamic_contrast,
                                    0.80f, 2.50f, 0.05f, "%.2f", 1.35f,
                                    "Empurra sons fracos para o piso e projeta os picos da música, eliminando visual chapado/quadrado.");

                        TouchSlider("Retenção / Cauda de Envelope", "waveform_envelope_decay", &metrics.waveform_envelope_decay,
                                    0.00f, 2.00f, 0.05f, "%.2fx", 0.20f,
                                    "Tempo de retenção horizontal dos picos (0.00x = linhas puras de transiente, sem preenchimento artificial).");

                        TouchSlider("Brilho do Miolo Incandescente", "waveform_core_brightness", &metrics.waveform_core_brightness,
                                    0.00f, 1.00f, 0.05f, "%.2f", 0.75f,
                                    "Intensidade do núcleo de alta energia no centro das linhas verticais.");

                        ImGui::Checkbox("Formato Bipolar Simétrico (Espelhado no Eixo Zero - Rekordbox 7)", &metrics.waveform_bipolar_mode);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Exibe as linhas acima e abaixo da linha central como no visor original do Rekordbox 7.");

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::TextColored(ImVec4(0.70f, 0.85f, 1.0f, 1.0f), "Predefinições Rápidas de Estilo (1-Clique):");
                        float btn_half_w = (ImGui::GetContentRegionAvail().x - 6.0f) * 0.5f;

                        if (ImGui::Button("Linhas Rekordbox 7 (Oficial)", ImVec2(btn_half_w, 28.0f))) {
                            metrics.waveform_line_style = 0;
                            metrics.waveform_line_spacing = 1.8f;
                            metrics.waveform_line_width = 1.2f;
                            metrics.waveform_line_gap = 0.6f;
                            metrics.waveform_transient_punch = 1.60f;
                            metrics.waveform_dynamic_contrast = 1.35f;
                            metrics.waveform_envelope_decay = 0.20f;
                            metrics.waveform_horizontal_stretch = 1.00f;
                            metrics.waveform_bipolar_mode = true;
                            metrics.needs_waveform_recompute = true;
                        }
                        ImGui::SameLine(0, 6.0f);
                        if (ImGui::Button("Agulhas Finas (Ultra-Sharp)", ImVec2(btn_half_w, 28.0f))) {
                            metrics.waveform_line_style = 0;
                            metrics.waveform_line_spacing = 1.5f;
                            metrics.waveform_line_width = 0.9f;
                            metrics.waveform_line_gap = 0.6f;
                            metrics.waveform_transient_punch = 2.10f;
                            metrics.waveform_dynamic_contrast = 1.50f;
                            metrics.waveform_envelope_decay = 0.05f;
                            metrics.waveform_horizontal_stretch = 1.00f;
                            metrics.waveform_bipolar_mode = true;
                            metrics.needs_waveform_recompute = true;
                        }

                        if (ImGui::Button("Barras Encorpadas (Stems)", ImVec2(btn_half_w, 28.0f))) {
                            metrics.waveform_line_style = 1;
                            metrics.waveform_line_spacing = 2.4f;
                            metrics.waveform_line_width = 1.8f;
                            metrics.waveform_line_gap = 0.6f;
                            metrics.waveform_transient_punch = 1.40f;
                            metrics.waveform_dynamic_contrast = 1.25f;
                            metrics.waveform_envelope_decay = 0.35f;
                            metrics.waveform_horizontal_stretch = 1.15f;
                            metrics.waveform_bipolar_mode = true;
                            metrics.needs_waveform_recompute = true;
                        }
                        ImGui::SameLine(0, 6.0f);
                        if (ImGui::Button("Malha Contínua (Sólida)", ImVec2(btn_half_w, 28.0f))) {
                            metrics.waveform_line_style = 2;
                            metrics.waveform_line_spacing = 1.0f;
                            metrics.waveform_line_width = 1.0f;
                            metrics.waveform_line_gap = 0.0f;
                            metrics.waveform_transient_punch = 1.00f;
                            metrics.waveform_dynamic_contrast = 1.05f;
                            metrics.waveform_envelope_decay = 1.00f;
                            metrics.waveform_horizontal_stretch = 1.65f;
                            metrics.waveform_bipolar_mode = true;
                            metrics.needs_waveform_recompute = true;
                        }

                        ImGui::Dummy(ImVec2(0.0f, 4.0f));
                        ImGui::Checkbox("Metrônomo Visual (Downbeat Flash)", &metrics.enable_visual_beat_metronome);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pisca suavemente a cada batida e emite clarão dourado no primeiro compasso.");

                        ImGui::EndTabItem();
                    }

                    // =========================================================================
                    // ABA 6: PRESETS PRONTOS & BACKUP
                    // =========================================================================
                    if (ImGui::BeginTabItem(ICON_FA_CUBES " Presets & Backup", nullptr, (metrics.requested_tab_idx == 6 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))) {
                        if (metrics.requested_tab_idx == 6) metrics.requested_tab_idx = -1;
                        ImGui::Dummy(ImVec2(0.0f, 8.0f));
                        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.10f, 1.0f), "PRESETS DE MERCADO (1-CLIQUE):");

                        float pb_w = ImGui::GetContentRegionAvail().x;
                        float pb_h = 36.0f;

                        if (ImGui::Button(ICON_FA_MUSIC " Padrão Club 2-Decks (CDJ-3000 Style)", ImVec2(pb_w, pb_h))) {
                            metrics.applyPreset(DJPresetLayoutType::CLUB_STANDARD_2DECK);
                            metrics.needs_waveform_recompute = true;
                        }
                        if (ImGui::Button(ICON_FA_RECORD_VINYL " Batalha de Vinil 12\" (Pratos Gigantes)", ImVec2(pb_w, pb_h))) {
                            metrics.applyPreset(DJPresetLayoutType::TURNTABLIST_BATTLE_12INCH);
                            metrics.needs_waveform_recompute = true;
                        }
                        if (ImGui::Button(ICON_FA_HEADPHONES " Techno Studio 4-Decks (Xone 4-Band)", ImVec2(pb_w, pb_h))) {
                            metrics.applyPreset(DJPresetLayoutType::TECHNO_STUDIO_4DECK);
                            metrics.needs_waveform_recompute = true;
                        }
                        if (ImGui::Button(ICON_FA_LAPTOP " Compacto Laptop (Ultrabooks)", ImVec2(pb_w, pb_h))) {
                            metrics.applyPreset(DJPresetLayoutType::ULTRA_COMPACT_LAPTOP);
                            metrics.needs_waveform_recompute = true;
                        }
                        if (ImGui::Button(ICON_FA_DESKTOP " UltraWide 4K Hero Display", ImVec2(pb_w, pb_h))) {
                            metrics.applyPreset(DJPresetLayoutType::ULTRAWIDE_4K_HERO);
                            metrics.needs_waveform_recompute = true;
                        }

                        ImGui::Dummy(ImVec2(0.0f, 10.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));

                        float half_w = (pb_w - 10.0f) * 0.5f;
                        if (ImGui::Button(ICON_FA_FLOPPY_DISK " Salvar meu Layout em Arquivo", ImVec2(half_w, pb_h))) {
                            metrics.saveToFile("assets/layouts/user_custom_layout.json");
                        }
                        ImGui::SameLine(0, 10.0f);
                        if (ImGui::Button(ICON_FA_FOLDER_OPEN " Carregar meu Layout Salvo", ImVec2(half_w, pb_h))) {
                            metrics.loadFromFile("assets/layouts/user_custom_layout.json");
                            metrics.needs_waveform_recompute = true;
                        }

                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }

                // =========================================================================
                // RODAPÉ: AÇÕES PRINCIPAIS (TOUCH-FRIENDLY 42PX)
                // =========================================================================
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0.0f, 6.0f));

                float total_w = ImGui::GetContentRegionAvail().x;
                float act_btn_h = 42.0f;
                float done_w = total_w * 0.62f;
                float reset_w = total_w - done_w - 10.0f;

                // Botão [ CONCLUIR E FECHAR ]
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.40f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.75f, 0.55f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.90f, 0.65f, 1.0f));
                if (ImGui::Button(ICON_FA_CHECK " CONCLUIR E FECHAR (Ctrl + E)", ImVec2(done_w, act_btn_h))) {
                    metrics.edit_mode_active = false;
                    metrics.saveToFile();
                }
                ImGui::PopStyleColor(3);

                ImGui::SameLine(0, 10.0f);

                // Botão [ RESTAURAR PADRÕES ]
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.15f, 0.15f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
                if (ImGui::Button(ICON_FA_ROTATE_LEFT " Restaurar Padrões", ImVec2(reset_w, act_btn_h))) {
                    metrics.resetToDefaults();
                    metrics.saveToFile();
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::End();

            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(4);
        }

        // Renderiza um divisor arrastável (Splitter) interativo entre dois painéis
        static bool Splitter(const char* str_id, bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f) {
            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = g.CurrentWindow;
            ImGuiID id = window->GetID(str_id);
            ImRect bb;
            bb.Min = window->DC.CursorPos;
            bb.Max = split_vertically ? ImVec2(bb.Min.x + thickness, bb.Min.y + splitter_long_axis_size) : ImVec2(bb.Min.x + splitter_long_axis_size, bb.Min.y + thickness);
            return ImGui::SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
        }
    };

}
