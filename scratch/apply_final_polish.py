import os

print("Applying PopStyleColor fix and AudioClips to top tracks...")

# 1. Update PsySongArranger.h
with open("src/core/PsySongArranger.h", "r", encoding="utf-8") as f:
    arr = f.read()

old_intro_part = """                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(6, 6, bt, block_dur);   // Tribal Percussion"""

new_intro_part = """                // Áudio clips estéreo visíveis imediatamente no topo da timeline (Compassos 1 a 16)
                addAudioClip(0, "PRYZMA_PsyKick_Stereo_Stem.wav", cur_t, block_dur, 0);
                addAudioClip(4, "PRYZMA_Percussion_Stereo_Loop.wav", cur_t + block_dur, block_dur, 0);

                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(6, 6, bt, block_dur);   // Tribal Percussion"""

if old_intro_part in arr:
    arr = arr.replace(old_intro_part, new_intro_part)
    with open("src/core/PsySongArranger.h", "w", encoding="utf-8") as f:
        f.write(arr)
    print("PsySongArranger.h updated with top audio clips!")
else:
    print("old_intro_part not found or already updated.")

# 2. Update KuroPlaylistUI.h for PopStyleColor
with open("src/ui/KuroPlaylistUI.h", "r", encoding="utf-8") as f:
    kuro = f.read()

old_pat_end = """            DrawIconPattern(draw, ImVec2(pat_p0.x + 11.0f, pat_p0.y + 13.0f), 13.0f, IM_COL32(180, 130, 255, 240));

            ImGui::SameLine(0, 8);"""

new_pat_end = """            DrawIconPattern(draw, ImVec2(pat_p0.x + 11.0f, pat_p0.y + 13.0f), 13.0f, IM_COL32(180, 130, 255, 240));

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);

            ImGui::SameLine(0, 8);"""

assert old_pat_end in kuro, "old_pat_end not found"
kuro = kuro.replace(old_pat_end, new_pat_end)

old_zoom_end = """            DrawIconFit(draw, ImVec2(fit_p0.x + 11.0f, fit_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ajustar Zoom para Caber Todo o Arranjo (Fit)");

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
        }"""

new_zoom_end = """            DrawIconFit(draw, ImVec2(fit_p0.x + 11.0f, fit_p0.y + 13.0f), 13.0f, IM_COL32(0, 229, 255, 230));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ajustar Zoom para Caber Todo o Arranjo (Fit)");

            ImGui::PopStyleColor(2);
        }"""

assert old_zoom_end in kuro, "old_zoom_end not found"
kuro = kuro.replace(old_zoom_end, new_zoom_end)

with open("src/ui/KuroPlaylistUI.h", "w", encoding="utf-8") as f:
    f.write(kuro)

print("KuroPlaylistUI.h updated with exact Push/Pop style balance!")
