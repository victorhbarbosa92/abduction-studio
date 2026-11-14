#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace KuroUI {

    enum class UIScaleMode {
        AUTO = 0,
        SCALE_70,
        SCALE_80,
        SCALE_90,
        SCALE_100,
        SCALE_125,
        SCALE_150,
        SCALE_175,
        SCALE_200
    };

    class UIScaleManager {
    private:
        static inline UIScaleMode current_mode = UIScaleMode::AUTO;
        static inline float current_scale = 1.0f;
        static inline float target_scale = 1.0f;
        static inline float content_scale_x = 1.0f;
        static inline float content_scale_y = 1.0f;
        static inline int window_w = 1280;
        static inline int window_h = 720;
        static inline bool scale_changed = false;

    public:
        static void init(GLFWwindow* window) {
            update(window);
            current_scale = target_scale;
        }

        static void update(GLFWwindow* window) {
            if (!window) return;

            glfwGetWindowSize(window, &window_w, &window_h);
            glfwGetWindowContentScale(window, &content_scale_x, &content_scale_y);

            float calculated_scale = 1.0f;
            if (current_mode == UIScaleMode::AUTO) {
                // Base: 1080p (altura 1080) -> scale 1.0f
                // Se a janela for menor (ex: 720p / 768p / 900p ou notebooks com escala de exibição ativa):
                // Adotamos escalas compactas (75%, 85%, 92%) para que todos os sequenciadores, mixers e plugins caibam sem transbordar.
                float h_ratio = (float)window_h / 1000.0f;
                float w_ratio = (float)window_w / 1800.0f;
                float auto_factor = (std::min)(h_ratio, w_ratio);

                if (window_h <= 740 || window_w <= 1300) {
                    calculated_scale = 0.75f; // 720p HD (1280x720) ou telas ultra-compactas
                } else if (window_h <= 820 || window_w <= 1440) {
                    calculated_scale = 0.85f; // 768p (1366x768) ou laptops
                } else if (window_h <= 950 || window_w <= 1680) {
                    calculated_scale = 0.92f; // 900p (1600x900)
                } else if (window_h <= 1120 && window_w <= 1980) {
                    calculated_scale = 1.00f; // 1080p Full HD Padrão (1920x1080)
                } else if (auto_factor <= 1.35f) {
                    calculated_scale = 1.25f; // 2K QHD (2560x1440)
                } else if (auto_factor <= 1.65f) {
                    calculated_scale = 1.50f; // 3K Display
                } else {
                    calculated_scale = 1.85f; // 4K UHD nativo (3840x2160)
                }
            } else {
                switch (current_mode) {
                    case UIScaleMode::SCALE_70:  calculated_scale = 0.70f; break;
                    case UIScaleMode::SCALE_80:  calculated_scale = 0.80f; break;
                    case UIScaleMode::SCALE_90:  calculated_scale = 0.90f; break;
                    case UIScaleMode::SCALE_100: calculated_scale = 1.00f; break;
                    case UIScaleMode::SCALE_125: calculated_scale = 1.25f; break;
                    case UIScaleMode::SCALE_150: calculated_scale = 1.50f; break;
                    case UIScaleMode::SCALE_175: calculated_scale = 1.75f; break;
                    case UIScaleMode::SCALE_200: calculated_scale = 2.00f; break;
                    default: calculated_scale = 1.00f; break;
                }
            }

            if (std::abs(calculated_scale - target_scale) > 0.01f) {
                target_scale = calculated_scale;
                current_scale = calculated_scale;
                scale_changed = true;
            } else {
                scale_changed = false;
            }
        }

        static float getScale() {
            return (current_scale > 0.1f) ? current_scale : 1.0f;
        }

        static float Scale(float value) {
            return value * getScale();
        }

        static ImVec2 Scale(float x, float y) {
            float s = getScale();
            return ImVec2(x * s, y * s);
        }

        static ImVec2 Scale(const ImVec2& v) {
            float s = getScale();
            return ImVec2(v.x * s, v.y * s);
        }

        static bool hasScaleChanged() {
            return scale_changed;
        }

        static void setScaleMode(UIScaleMode mode) {
            current_mode = mode;
            target_scale = 0.0f; // Força recálculo no próximo update
            scale_changed = true;
        }

        static UIScaleMode getScaleMode() {
            return current_mode;
        }

        static const char* getScaleModeName(UIScaleMode mode) {
            switch (mode) {
                case UIScaleMode::AUTO:      return "Auto (Detectar Monitor)";
                case UIScaleMode::SCALE_70:  return "70% (Ultra Compacto / 720p)";
                case UIScaleMode::SCALE_80:  return "80% (Compacto / 768p)";
                case UIScaleMode::SCALE_90:  return "90% (Medio / 900p)";
                case UIScaleMode::SCALE_100: return "100% (Padrao / 1080p)";
                case UIScaleMode::SCALE_125: return "125% (Grande / 1440p)";
                case UIScaleMode::SCALE_150: return "150% (Extra Grande / 2K)";
                case UIScaleMode::SCALE_175: return "175% (3K Display)";
                case UIScaleMode::SCALE_200: return "200% (4K UHD Nativo)";
                default:                     return "Custom";
            }
        }

        static void stepZoom(int direction) {
            int cur = (int)current_mode;
            if (direction > 0) {
                if (cur < (int)UIScaleMode::SCALE_200) {
                    setScaleMode((UIScaleMode)(cur == 0 ? (int)UIScaleMode::SCALE_100 : cur + 1));
                }
            } else if (direction < 0) {
                if (cur > (int)UIScaleMode::SCALE_70) {
                    setScaleMode((UIScaleMode)(cur - 1));
                }
            }
        }

        static void checkShortcuts() {
            ImGuiIO& io = ImGui::GetIO();
            if (io.KeyCtrl) {
                if (ImGui::IsKeyPressed(ImGuiKey_Equal, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false)) {
                    stepZoom(1); // Zoom In
                } else if (ImGui::IsKeyPressed(ImGuiKey_Minus, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false)) {
                    stepZoom(-1); // Zoom Out
                } else if (ImGui::IsKeyPressed(ImGuiKey_0, false) || ImGui::IsKeyPressed(ImGuiKey_Keypad0, false)) {
                    setScaleMode(UIScaleMode::AUTO); // Reset Auto
                }
            }
        }

        // Garante que todas as janelas do ImGui fiquem 100% dentro do viewport visível
        static void ClampAllFloatingWindows(ImGuiViewport* vp, float topbar_h) {
            if (!vp) return;
            ImGuiContext* g = ImGui::GetCurrentContext();
            if (!g) return;

            float min_x = vp->Pos.x;
            float max_x = vp->Pos.x + vp->Size.x;
            float min_y = vp->Pos.y + topbar_h;
            float max_y = vp->Pos.y + vp->Size.y;

            for (ImGuiWindow* win : g->Windows) {
                if (!win || win->WasActive == false) continue;
                // Ignorar janelas internas de renderização, dockspace ou barras fixas
                if (win->Flags & ImGuiWindowFlags_ChildWindow) continue;
                if (win->Flags & ImGuiWindowFlags_Tooltip) continue;
                if (win->Flags & ImGuiWindowFlags_Popup) continue;
                if (strcmp(win->Name, "##FLStyleTopbar") == 0 || 
                    strcmp(win->Name, "Kuro Main") == 0 ||
                    strcmp(win->Name, "DESKTOP") == 0 ||
                    strcmp(win->Name, "BROWSER") == 0 ||
                    strcmp(win->Name, "Browser") == 0) continue;

                // 1. Clampar tamanho se exceder a área de trabalho
                if (win->Size.x > vp->Size.x - 10.0f) {
                    win->Size.x = std::max(300.0f, vp->Size.x - 10.0f);
                    win->SizeFull.x = win->Size.x;
                }
                if (win->Size.y > (vp->Size.y - topbar_h - 10.0f)) {
                    win->Size.y = std::max(180.0f, vp->Size.y - topbar_h - 10.0f);
                    win->SizeFull.y = win->Size.y;
                }

                // 2. Reposicionar se estiver fora da tela
                if (win->Pos.x + win->Size.x > max_x) {
                    win->Pos.x = std::max(min_x, max_x - win->Size.x - 5.0f);
                }
                if (win->Pos.x < min_x) {
                    win->Pos.x = min_x + 5.0f;
                }
                if (win->Pos.y + win->Size.y > max_y) {
                    win->Pos.y = std::max(min_y, max_y - win->Size.y - 5.0f);
                }
                if (win->Pos.y < min_y) {
                    win->Pos.y = min_y + 2.0f;
                }
            }
        }

        static void renderMenuItems() {
            char header_buf[64];
            snprintf(header_buf, sizeof(header_buf), "Escala Atual: %.0f%% (Resolucao %dx%d)", getScale() * 100.0f, window_w, window_h);
            ImGui::TextDisabled("%s", header_buf);
            ImGui::Separator();

            UIScaleMode modes[] = {
                UIScaleMode::AUTO,
                UIScaleMode::SCALE_70,
                UIScaleMode::SCALE_80,
                UIScaleMode::SCALE_90,
                UIScaleMode::SCALE_100,
                UIScaleMode::SCALE_125,
                UIScaleMode::SCALE_150,
                UIScaleMode::SCALE_175,
                UIScaleMode::SCALE_200
            };

            for (auto m : modes) {
                bool selected = (current_mode == m);
                if (ImGui::MenuItem(getScaleModeName(m), nullptr, selected)) {
                    setScaleMode(m);
                }
            }

            ImGui::Separator();
            ImGui::TextDisabled("Atalhos: Ctrl + / Ctrl - / Ctrl 0");
        }

        static void renderMenuOptions() {
            if (ImGui::BeginMenu("GUI Scaling (Resolucao)")) {
                renderMenuItems();
                ImGui::EndMenu();
            }
        }
    };

}
