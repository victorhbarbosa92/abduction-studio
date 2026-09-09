import re

path = 'src/ui/StudioUI.h'
with open(path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

# 1. Add #include "SciFiIconSystem.h" near top if not already at top
if '#include "SciFiIconSystem.h"' not in content[:5000]:
    content = '#include "SciFiIconSystem.h"\n' + content

# 2. Replace the saucer drawing inside RenderAndroidDesktopHub
old_saucer_code = '''            // 2. EMBLEMA CENTRAL DA NAVE ALIENIGENA EM RELEVO METALICO 3D (WALLPAPER)
            float s_rx = 135.0f;
            float s_ry = 48.0f;
            ImVec2 s_c = ImVec2(cx, cy - 20.0f);

            // Halo de brilho suave ciano atras da nave
            dl->AddCircleFilled(s_c, s_rx * 0.90f, IM_COL32(0, 229, 255, 14), 48);

            // Anel externo em baixo-relevo (sombra e chanfro de luz)
            dl->AddEllipseFilled(ImVec2(s_c.x, s_c.y + 3.0f), s_rx + 2.0f, s_ry + 2.0f, IM_COL32(10, 13, 18, 255), 48);
            dl->AddEllipseFilled(s_c, s_rx, s_ry, IM_COL32(26, 32, 42, 255), 48);
            dl->AddEllipse(s_c, s_rx, s_ry, IM_COL32(55, 68, 88, 255), 48, 1.5f);

            // Disco intermediario da nave
            dl->AddEllipseFilled(s_c, s_rx * 0.72f, s_ry * 0.65f, IM_COL32(34, 42, 54, 255), 40);
            dl->AddEllipse(s_c, s_rx * 0.72f, s_ry * 0.65f, IM_COL32(70, 88, 115, 255), 40, 1.0f);

            // Cupula superior da nave alienigena
            ImVec2 dome_c = ImVec2(s_c.x, s_c.y - s_ry * 0.28f);
            float dome_r = s_rx * 0.35f;
            dl->AddEllipseFilled(dome_c, dome_r, dome_r * 0.55f, IM_COL32(42, 54, 70, 255), 32);
            dl->AddEllipse(dome_c, dome_r, dome_r * 0.55f, IM_COL32(85, 110, 145, 255), 32, 1.2f);'''

new_saucer_code = '''            // 2. EMBLEMA CENTRAL DA NAVE ALIENIGENA EM RELEVO METALICO 3D (WALLPAPER)
            float s_rx = 135.0f;
            float s_ry = 48.0f;
            ImVec2 s_c = ImVec2(cx, cy - 20.0f);

            auto DrawSaucerEllipse = [&](ImVec2 center, float rx, float ry, ImU32 fill_col, ImU32 stroke_col, float stroke_w) {
                const int n = 36;
                ImVec2 pts[36];
                for (int i = 0; i < n; i++) {
                    float a = ((float)i / (float)n) * 6.2831853f;
                    pts[i] = ImVec2(center.x + cosf(a) * rx, center.y + sinf(a) * ry);
                }
                dl->AddConvexPolyFilled(pts, n, fill_col);
                if (stroke_col != 0) {
                    dl->AddPolyline(pts, n, stroke_col, ImDrawFlags_Closed, stroke_w);
                }
            };

            // Halo de brilho suave ciano atras da nave
            dl->AddCircleFilled(s_c, s_rx * 0.90f, IM_COL32(0, 229, 255, 14), 48);

            // Anel externo em baixo-relevo (sombra e chanfro de luz)
            DrawSaucerEllipse(ImVec2(s_c.x, s_c.y + 3.0f), s_rx + 2.0f, s_ry + 2.0f, IM_COL32(10, 13, 18, 255), 0, 0.0f);
            DrawSaucerEllipse(s_c, s_rx, s_ry, IM_COL32(26, 32, 42, 255), IM_COL32(55, 68, 88, 255), 1.5f);

            // Disco intermediario da nave
            DrawSaucerEllipse(s_c, s_rx * 0.72f, s_ry * 0.65f, IM_COL32(34, 42, 54, 255), IM_COL32(70, 88, 115, 255), 1.0f);

            // Cupula superior da nave alienigena
            ImVec2 dome_c = ImVec2(s_c.x, s_c.y - s_ry * 0.28f);
            float dome_r = s_rx * 0.35f;
            DrawSaucerEllipse(dome_c, dome_r, dome_r * 0.55f, IM_COL32(42, 54, 70, 255), IM_COL32(85, 110, 145, 255), 1.2f);'''

if old_saucer_code in content:
    content = content.replace(old_saucer_code, new_saucer_code)
    print("Replaced saucer code successfully!")
else:
    print("Could not find old_saucer_code, will search with normalized lines")

with open(path, 'w', encoding='utf-8') as f:
    f.write(content)
print("Updated StudioUI.h!")
