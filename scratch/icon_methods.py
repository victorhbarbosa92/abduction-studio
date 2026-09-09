import os

# Define the C++ code for icon drawing functions to add to KuroUI
icon_drawing_methods = r'''
        // ── VECTOR HUD ICONS FOR PLAYLIST TOOLBAR ────────────────────────────────
        static void DrawIconPencil(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p_tip(c.x - r * 0.65f, c.y + r * 0.65f);
            ImVec2 p_top(c.x + r * 0.65f, c.y - r * 0.65f);
            float w = 2.4f;
            draw->AddLine(ImVec2(p_tip.x - w, p_tip.y - w), ImVec2(p_top.x - w, p_top.y - w), col, 1.4f);
            draw->AddLine(ImVec2(p_tip.x + w, p_tip.y + w), ImVec2(p_top.x + w, p_top.y + w), col, 1.4f);
            draw->AddTriangleFilled(ImVec2(p_tip.x - w, p_tip.y - w), ImVec2(p_tip.x + w, p_tip.y + w), ImVec2(c.x - r * 0.95f, c.y + r * 0.95f), col);
            draw->AddLine(ImVec2(p_top.x - w, p_top.y - w), ImVec2(p_top.x + w, p_top.y + w), col, 1.8f);
        }

        static void DrawIconBrush(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x + r * 0.1f, c.y - r * 0.1f), ImVec2(c.x + r * 0.85f, c.y - r * 0.85f), col, 2.0f);
            draw->AddLine(ImVec2(c.x - r * 0.15f, c.y - r * 0.15f), ImVec2(c.x + r * 0.2f, c.y + r * 0.2f), col, 2.5f);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.15f, c.y - r * 0.15f), ImVec2(c.x + r * 0.2f, c.y + r * 0.2f), ImVec2(c.x - r * 0.80f, c.y + r * 0.80f), col);
        }

        static void DrawIconRazor(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 pts[4] = {
                ImVec2(c.x - r * 0.75f, c.y - r * 0.35f),
                ImVec2(c.x + r * 0.45f, c.y - r * 0.85f),
                ImVec2(c.x + r * 0.75f, c.y + r * 0.35f),
                ImVec2(c.x - r * 0.45f, c.y + r * 0.85f)
            };
            draw->AddPolyline(pts, 4, col, ImDrawFlags_Closed, 1.4f);
            draw->AddLine(ImVec2(c.x - r * 0.35f, c.y + r * 0.12f), ImVec2(c.x + r * 0.35f, c.y - r * 0.12f), col, 1.4f);
            draw->AddCircleFilled(c, 1.5f, col);
        }

        static void DrawIconSlip(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - 3.5f, c.y - r * 0.75f), ImVec2(c.x - 3.5f, c.y + r * 0.75f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + 3.5f, c.y - r * 0.75f), ImVec2(c.x + 3.5f, c.y + r * 0.75f), col, 1.5f);
            draw->AddTriangleFilled(ImVec2(c.x - 5.5f, c.y), ImVec2(c.x - 1.5f, c.y - 3.0f), ImVec2(c.x - 1.5f, c.y + 3.0f), col);
            draw->AddTriangleFilled(ImVec2(c.x + 5.5f, c.y), ImVec2(c.x + 1.5f, c.y - 3.0f), ImVec2(c.x + 1.5f, c.y + 3.0f), col);
        }

        static void DrawIconSelect(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p0(c.x - r * 0.65f, c.y - r * 0.80f);
            ImVec2 p1(c.x + r * 0.65f, c.y + r * 0.10f);
            ImVec2 p2(c.x - r * 0.05f, c.y + r * 0.18f);
            ImVec2 p3(c.x - r * 0.35f, c.y + r * 0.85f);
            draw->AddTriangleFilled(p0, p1, p2, col);
            draw->AddLine(p2, p3, col, 2.0f);
        }

        static void DrawIconDelete(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRect(ImVec2(c.x - r * 0.50f, c.y - r * 0.20f), ImVec2(c.x + r * 0.50f, c.y + r * 0.75f), col, 1.5f, 0, 1.2f);
            draw->AddLine(ImVec2(c.x - r * 0.70f, c.y - r * 0.35f), ImVec2(c.x + r * 0.70f, c.y - r * 0.35f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - r * 0.25f, c.y - r * 0.65f), ImVec2(c.x + r * 0.25f, c.y - r * 0.65f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - 2.0f, c.y), ImVec2(c.x - 2.0f, c.y + r * 0.55f), col, 1.0f);
            draw->AddLine(ImVec2(c.x + 2.0f, c.y), ImVec2(c.x + 2.0f, c.y + r * 0.55f), col, 1.0f);
        }

        static void DrawIconMute(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddRectFilled(ImVec2(c.x - r * 0.75f, c.y - r * 0.35f), ImVec2(c.x - r * 0.30f, c.y + r * 0.35f), col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.30f, c.y - r * 0.35f), ImVec2(c.x + r * 0.05f, c.y - r * 0.75f), ImVec2(c.x + r * 0.05f, c.y + r * 0.75f), col);
            draw->AddLine(ImVec2(c.x + r * 0.30f, c.y - r * 0.50f), ImVec2(c.x + r * 0.80f, c.y + r * 0.50f), col, 1.6f);
            draw->AddLine(ImVec2(c.x + r * 0.80f, c.y - r * 0.50f), ImVec2(c.x + r * 0.30f, c.y + r * 0.50f), col, 1.6f);
        }

        static void DrawIconSnap(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.75f, c.y), ImVec2(c.x + r * 0.75f, c.y), col, 1.5f);
            draw->AddLine(ImVec2(c.x, c.y - r * 0.75f), ImVec2(c.x, c.y + r * 0.75f), col, 1.5f);
            draw->AddCircleFilled(c, 1.5f, col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.85f, c.y), ImVec2(c.x - r * 0.50f, c.y - 2.5f), ImVec2(c.x - r * 0.50f, c.y + 2.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.85f, c.y), ImVec2(c.x + r * 0.50f, c.y - 2.5f), ImVec2(c.x + r * 0.50f, c.y + 2.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x, c.y - r * 0.85f), ImVec2(c.x - 2.5f, c.y - r * 0.50f), ImVec2(c.x + 2.5f, c.y - r * 0.50f), col);
            draw->AddTriangleFilled(ImVec2(c.x, c.y + r * 0.85f), ImVec2(c.x - 2.5f, c.y + r * 0.50f), ImVec2(c.x + 2.5f, c.y + r * 0.50f), col);
        }

        static void DrawIconPattern(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddCircleFilled(ImVec2(c.x - r * 0.40f, c.y + r * 0.45f), 2.2f, col);
            draw->AddCircleFilled(ImVec2(c.x + r * 0.40f, c.y + r * 0.25f), 2.2f, col);
            draw->AddLine(ImVec2(c.x - r * 0.22f, c.y + r * 0.45f), ImVec2(c.x - r * 0.22f, c.y - r * 0.55f), col, 1.4f);
            draw->AddLine(ImVec2(c.x + r * 0.58f, c.y + r * 0.25f), ImVec2(c.x + r * 0.58f, c.y - r * 0.75f), col, 1.4f);
            draw->AddLine(ImVec2(c.x - r * 0.22f, c.y - r * 0.55f), ImVec2(c.x + r * 0.58f, c.y - r * 0.75f), col, 2.2f);
        }

        static void DrawIconPsytrance(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddEllipse(c, ImVec2(r * 0.85f, r * 0.35f), col, 0.0f, 16, 1.5f);
            draw->AddCircle(ImVec2(c.x, c.y - r * 0.25f), r * 0.40f, col, 12, 1.4f);
            draw->AddCircleFilled(ImVec2(c.x - r * 0.45f, c.y), 1.2f, col);
            draw->AddCircleFilled(ImVec2(c.x, c.y + 1.0f), 1.4f, col);
            draw->AddCircleFilled(ImVec2(c.x + r * 0.45f, c.y), 1.2f, col);
        }

        static void DrawIconBezier(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            ImVec2 p0(c.x - r * 0.75f, c.y + r * 0.55f);
            ImVec2 cp1(c.x - r * 0.25f, c.y - r * 0.75f);
            ImVec2 cp2(c.x + r * 0.25f, c.y + r * 0.75f);
            ImVec2 p1(c.x + r * 0.75f, c.y - r * 0.55f);
            draw->AddBezierCubic(p0, cp1, cp2, p1, col, 1.8f);
            draw->AddCircleFilled(p0, 2.0f, col);
            draw->AddCircleFilled(p1, 2.0f, col);
            draw->AddCircle(cp1, 1.5f, col);
            draw->AddCircle(cp2, 1.5f, col);
        }

        static void DrawIconAiSparkle(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddQuadFilled(
                ImVec2(c.x, c.y - r * 0.85f),
                ImVec2(c.x + r * 0.35f, c.y),
                ImVec2(c.x, c.y + r * 0.85f),
                ImVec2(c.x - r * 0.35f, c.y),
                col
            );
            draw->AddCircleFilled(ImVec2(c.x + r * 0.65f, c.y - r * 0.55f), 1.4f, col);
            draw->AddCircleFilled(ImVec2(c.x - r * 0.65f, c.y + r * 0.55f), 1.2f, col);
        }

        static void DrawIconLoop(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddCircle(c, r * 0.65f, col, 16, 1.5f);
            draw->AddTriangleFilled(ImVec2(c.x + r * 0.65f, c.y - 1.0f), ImVec2(c.x + r * 0.95f, c.y - 4.5f), ImVec2(c.x + r * 0.35f, c.y - 4.5f), col);
            draw->AddTriangleFilled(ImVec2(c.x - r * 0.65f, c.y + 1.0f), ImVec2(c.x - r * 0.95f, c.y + 4.5f), ImVec2(c.x - r * 0.35f, c.y + 4.5f), col);
        }

        static void DrawIconPresets(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y - r * 0.5f), ImVec2(c.x + r * 0.65f, c.y - r * 0.5f), col, 1.8f);
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y), ImVec2(c.x + r * 0.35f, c.y), col, 1.8f);
            draw->AddLine(ImVec2(c.x - r * 0.65f, c.y + r * 0.5f), ImVec2(c.x + r * 0.65f, c.y + r * 0.5f), col, 1.8f);
        }

        static void DrawIconFit(ImDrawList* draw, ImVec2 c, float sz, ImU32 col) {
            float r = sz * 0.5f;
            float d = r * 0.70f;
            draw->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x - d + 3.5f, c.y - d), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x - d, c.y - d + 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y - d), ImVec2(c.x + d - 3.5f, c.y - d), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y - d), ImVec2(c.x + d, c.y - d + 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y + d), ImVec2(c.x - d + 3.5f, c.y + d), col, 1.5f);
            draw->AddLine(ImVec2(c.x - d, c.y + d), ImVec2(c.x - d, c.y + d - 3.5f), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y + d), ImVec2(c.x + d - 3.5f, c.y + d), col, 1.5f);
            draw->AddLine(ImVec2(c.x + d, c.y + d), ImVec2(c.x + d, c.y + d - 3.5f), col, 1.5f);
        }
'''

print("icon_drawing_methods defined, size:", len(icon_drawing_methods))
