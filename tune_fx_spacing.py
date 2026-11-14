with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    text = f.read()

# 1. Update h in renderRekordboxFXBar to 62.0f
text = text.replace(
    'float h = KuroUI::UIScaleManager::Scale(58.0f);',
    'float h = KuroUI::UIScaleManager::Scale(62.0f);',
    1
)

# 2. Add Dummy after SetCursorScreenPos at end of renderRekordboxFXBar
old_end = '''            chBtn("1##fx2_1", &rekordbox_fx2_ch[0], "Roteamento FX2: Ativar no Deck 1");
                    chBtn("2##fx2_2", &rekordbox_fx2_ch[1], "Roteamento FX2: Ativar no Deck 2");
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));
        }'''

new_end = '''            chBtn("1##fx2_1", &rekordbox_fx2_ch[0], "Roteamento FX2: Ativar no Deck 1");
                    chBtn("2##fx2_2", &rekordbox_fx2_ch[1], "Roteamento FX2: Ativar no Deck 2");
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));
            ImGui::Dummy(ImVec2(w, 2.0f));
        }'''

if old_end in text:
    text = text.replace(old_end, new_end, 1)
    print("Updated renderRekordboxFXBar cursor spacing.")
else:
    print("Could not find exact old_end block, searching fallback...")
    idx_fx = text.find('void renderRekordboxFXBar(')
    idx_jog = text.find('void renderRekordboxJogwheel(')
    target = 'ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));\n        }'
    rep = 'ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));\n            ImGui::Dummy(ImVec2(w, 2.0f));\n        }'
    sub = text[idx_fx:idx_jog]
    if target in sub:
        sub_new = sub.replace(target, rep, 1)
        text = text[:idx_fx] + sub_new + text[idx_jog:]
        print("Updated via fallback.")

with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
    f.write(text)

print("FX bar spacing tuned.")
