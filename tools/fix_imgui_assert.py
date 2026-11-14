path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/ui/KuroDJStudioUI.h"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Fix card start and end with proper ImGui Dummy submission
card_start_old = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 110.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();"""

card_start_new = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 110.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();
                            ImGui::Dummy(ImVec2(item_w, item_h));"""

content = content.replace(card_start_old, card_start_new)

card_end_old = """                            ImGui::PopID();
                            // Restaura o cursor para baixo do cartão com margem limpa
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x, item_p.y + item_h + 8.0f));
                        }"""

card_end_new = """                            ImGui::PopID();
                            // Restaura o cursor para baixo do cartão com margem limpa e submete item Dummy para validar boundaries
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x, item_p.y + item_h + 8.0f));
                            ImGui::Dummy(ImVec2(item_w, 2.0f));
                        }"""

content = content.replace(card_end_old, card_end_new)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)

print("Applied Dummy submission after SetCursorScreenPos!")

