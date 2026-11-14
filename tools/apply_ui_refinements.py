import re
import sys

HEADER_PATH = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h"

with open(HEADER_PATH, "r", encoding="utf-8") as f:
    content = f.read()

orig_content = content

# -----------------------------------------------------------------------------
# 1. FIX PITCH FADER WIDGET (MT / RANGE BUTTONS CUT OFF)
# -----------------------------------------------------------------------------
fader_old = """    // 2. Header: Label "TEMPO" + Valor "%" em destaque + Botões de Range e MT
    float hdr_h = KuroUI::UIScaleManager::Scale(44.0f);
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + hdr_h),
                      IM_COL32(16, 20, 28, 255), 4.0f,
                      ImDrawFlags_RoundCornersTop);
    dl->AddLine(ImVec2(pos.x, pos.y + hdr_h),
                ImVec2(pos.x + size.x, pos.y + hdr_h),
                IM_COL32(34, 44, 58, 220), 1.0f);

    // Linha 1 do Header: TEMPO e Botão MT (Key Lock)
    float mt_w = (size.x < KuroUI::UIScaleManager::Scale(75.0f)) ? KuroUI::UIScaleManager::Scale(26.0f) : KuroUI::UIScaleManager::Scale(32.0f);
    float mt_h = KuroUI::UIScaleManager::Scale(18.0f);
    ImVec2 mt_pos = ImVec2(pos.x + size.x - mt_w - KuroUI::UIScaleManager::Scale(4.0f), pos.y + KuroUI::UIScaleManager::Scale(3.0f));
    char mt_id[32];
    snprintf(mt_id, sizeof(mt_id), "MT##PF_%d", deck.deck_id);
    ImGui::SetCursorScreenPos(mt_pos);
    if (deck.key_sync_active) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 1.0f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 0.9f));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.9f, 1));
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 0.0f));
    if (ImGui::Button(mt_id, ImVec2(mt_w, mt_h))) {
      deck.key_sync_active = !deck.key_sync_active;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    // Texto TEMPO com clipping para nunca encavalar no botão MT
    dl->PushClipRect(ImVec2(pos.x + KuroUI::UIScaleManager::Scale(4.0f), pos.y + KuroUI::UIScaleManager::Scale(2.0f)),
                     ImVec2(mt_pos.x - KuroUI::UIScaleManager::Scale(2.0f), pos.y + KuroUI::UIScaleManager::Scale(22.0f)), true);
    dl->AddText(f_bld, KuroUI::UIScaleManager::Scale(10.5f), ImVec2(pos.x + KuroUI::UIScaleManager::Scale(5.0f), pos.y + KuroUI::UIScaleManager::Scale(4.0f)),
                IM_COL32(170, 185, 205, 230), "TEMPO");
    dl->PopClipRect();

    // Linha 2 do Header: Valor Numérico do Pitch e Botão de Range
    char val_str[32];
    if (std::abs(deck.pitch_percent) < 0.005f) {
      snprintf(val_str, sizeof(val_str), "±0.00%%");
    } else {
      snprintf(val_str, sizeof(val_str), "%+.2f%%", deck.pitch_percent);
    }
    for (char *p = val_str; *p; ++p) {
      if (*p == ',')
        *p = '.';
    }

    // Se estiver em 0.00%, brilha em VERDE NEON (LED 0% Pioneer Rekordbox)
    bool is_center_zero = (std::abs(deck.pitch_percent) < 0.02f);
    ImU32 col_val = is_center_zero ? IM_COL32(0, 240, 120, 255)
                                   : IM_COL32(235, 245, 255, 255);

    const char *rg_str =
        (deck.pitch_range >= 90.0f)
            ? "WIDE"
            : (deck.pitch_range >= 15.0f
                   ? "16%"
                   : (deck.pitch_range >= 9.0f ? "10%" : "6%"));
    float rg_w = (size.x < KuroUI::UIScaleManager::Scale(75.0f)) ? KuroUI::UIScaleManager::Scale(30.0f) : KuroUI::UIScaleManager::Scale(36.0f);
    float rg_h = KuroUI::UIScaleManager::Scale(18.0f);
    ImVec2 rg_pos = ImVec2(pos.x + size.x - rg_w - KuroUI::UIScaleManager::Scale(4.0f), pos.y + mt_h + KuroUI::UIScaleManager::Scale(5.0f));
    char rg_id[32];
    snprintf(rg_id, sizeof(rg_id), "%s##PFRG_%d", rg_str, deck.deck_id);
    ImGui::SetCursorScreenPos(rg_pos);
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (deck.pitch_range >= 90.0f)
                              ? ImVec4(0.6f, 0.1f, 0.15f, 0.9f)
                              : ImVec4(0.14f, 0.18f, 0.24f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_Text, (deck.pitch_range >= 90.0f)
                                             ? ImVec4(1.0f, 0.3f, 0.35f, 1.0f)
                                             : ImVec4(0.85f, 0.9f, 1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 0.0f));
    if (ImGui::Button(rg_id, ImVec2(rg_w, rg_h))) {
      if (deck.pitch_range < 8.0f)
        deck.pitch_range = 10.0f;
      else if (deck.pitch_range < 12.0f)
        deck.pitch_range = 16.0f;
      else if (deck.pitch_range < 20.0f)
        deck.pitch_range = 100.0f;
      else
        deck.pitch_range = 6.0f;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    // Texto de valor do pitch com clipping para não encavalar no botão de Range
    dl->PushClipRect(ImVec2(pos.x + KuroUI::UIScaleManager::Scale(4.0f), pos.y + KuroUI::UIScaleManager::Scale(20.0f)),
                     ImVec2(rg_pos.x - KuroUI::UIScaleManager::Scale(2.0f), pos.y + hdr_h - KuroUI::UIScaleManager::Scale(2.0f)), true);
    float v_font_sz = (size.x < KuroUI::UIScaleManager::Scale(75.0f)) ? KuroUI::UIScaleManager::Scale(11.0f) : KuroUI::UIScaleManager::Scale(12.5f);
    dl->AddText(f_bld, v_font_sz, ImVec2(pos.x + KuroUI::UIScaleManager::Scale(5.0f), pos.y + KuroUI::UIScaleManager::Scale(22.0f)), col_val,
                val_str);
    dl->PopClipRect();"""

fader_new = """    // 2. Header: Label "TEMPO" + Valor "%" em destaque + Botões de Range e MT
    float hdr_h = KuroUI::UIScaleManager::Scale(48.0f);
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + hdr_h),
                      IM_COL32(16, 20, 28, 255), 4.0f,
                      ImDrawFlags_RoundCornersTop);
    dl->AddLine(ImVec2(pos.x, pos.y + hdr_h),
                ImVec2(pos.x + size.x, pos.y + hdr_h),
                IM_COL32(34, 44, 58, 220), 1.0f);

    ImFont *f_ctrl = g_font_control_small ? g_font_control_small : f_bld;

    // Linha 1 do Header: TEMPO e Botão MT (Key Lock)
    float mt_w = std::clamp(size.x * 0.44f, KuroUI::UIScaleManager::Scale(34.0f), KuroUI::UIScaleManager::Scale(44.0f));
    float mt_h = KuroUI::UIScaleManager::Scale(19.0f);
    ImVec2 mt_pos = ImVec2(pos.x + size.x - mt_w - KuroUI::UIScaleManager::Scale(4.0f), pos.y + KuroUI::UIScaleManager::Scale(3.0f));
    char mt_id[32];
    snprintf(mt_id, sizeof(mt_id), "##PF_MT_%d", deck.deck_id);
    ImGui::SetCursorScreenPos(mt_pos);
    ImGui::InvisibleButton(mt_id, ImVec2(mt_w, mt_h));
    if (ImGui::IsItemClicked(0)) {
      deck.key_sync_active = !deck.key_sync_active;
    }
    bool mt_hov = ImGui::IsItemHovered();
    ImU32 mt_bg = deck.key_sync_active ? IM_COL32(0, 140, 255, 255) : (mt_hov ? IM_COL32(36, 48, 66, 240) : IM_COL32(22, 28, 38, 220));
    ImU32 mt_bd = deck.key_sync_active ? IM_COL32(140, 205, 255, 255) : IM_COL32(45, 58, 76, 200);
    dl->AddRectFilled(mt_pos, ImVec2(mt_pos.x + mt_w, mt_pos.y + mt_h), mt_bg, KuroUI::UIScaleManager::Scale(3.0f));
    dl->AddRect(mt_pos, ImVec2(mt_pos.x + mt_w, mt_pos.y + mt_h), mt_bd, KuroUI::UIScaleManager::Scale(3.0f), 0, 1.0f);
    ImVec2 sz_mt_txt = f_ctrl->CalcTextSizeA(KuroUI::UIScaleManager::Scale(11.0f), FLT_MAX, -1.0f, "MT");
    dl->AddText(f_ctrl, KuroUI::UIScaleManager::Scale(11.0f),
                ImVec2(mt_pos.x + (mt_w - sz_mt_txt.x) * 0.5f, mt_pos.y + (mt_h - sz_mt_txt.y) * 0.5f),
                deck.key_sync_active ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 200, 225, 230), "MT");

    // Texto TEMPO com clipping para nunca encavalar no botão MT
    dl->PushClipRect(ImVec2(pos.x + KuroUI::UIScaleManager::Scale(4.0f), pos.y + KuroUI::UIScaleManager::Scale(2.0f)),
                     ImVec2(mt_pos.x - KuroUI::UIScaleManager::Scale(2.0f), pos.y + KuroUI::UIScaleManager::Scale(22.0f)), true);
    dl->AddText(f_bld, KuroUI::UIScaleManager::Scale(11.0f), ImVec2(pos.x + KuroUI::UIScaleManager::Scale(5.0f), pos.y + KuroUI::UIScaleManager::Scale(4.0f)),
                IM_COL32(170, 185, 205, 230), "TEMPO");
    dl->PopClipRect();

    // Linha 2 do Header: Valor Numérico do Pitch e Botão de Range
    char val_str[32];
    if (std::abs(deck.pitch_percent) < 0.005f) {
      snprintf(val_str, sizeof(val_str), "±0.00%%");
    } else {
      snprintf(val_str, sizeof(val_str), "%+.2f%%", deck.pitch_percent);
    }
    for (char *p = val_str; *p; ++p) {
      if (*p == ',')
        *p = '.';
    }

    // Se estiver em 0.00%, brilha em VERDE NEON (LED 0% Pioneer Rekordbox)
    bool is_center_zero = (std::abs(deck.pitch_percent) < 0.02f);
    ImU32 col_val = is_center_zero ? IM_COL32(0, 240, 120, 255)
                                   : IM_COL32(235, 245, 255, 255);

    const char *rg_str =
        (deck.pitch_range >= 90.0f)
            ? "WIDE"
            : (deck.pitch_range >= 15.0f
                   ? "16%"
                   : (deck.pitch_range >= 9.0f ? "10%" : "6%"));
    float rg_w = std::clamp(size.x * 0.44f, KuroUI::UIScaleManager::Scale(36.0f), KuroUI::UIScaleManager::Scale(46.0f));
    float rg_h = KuroUI::UIScaleManager::Scale(19.0f);
    ImVec2 rg_pos = ImVec2(pos.x + size.x - rg_w - KuroUI::UIScaleManager::Scale(4.0f), pos.y + mt_h + KuroUI::UIScaleManager::Scale(5.0f));
    char rg_id[32];
    snprintf(rg_id, sizeof(rg_id), "##PFRG_%d", deck.deck_id);
    ImGui::SetCursorScreenPos(rg_pos);
    ImGui::InvisibleButton(rg_id, ImVec2(rg_w, rg_h));
    if (ImGui::IsItemClicked(0)) {
      if (deck.pitch_range < 8.0f)
        deck.pitch_range = 10.0f;
      else if (deck.pitch_range < 12.0f)
        deck.pitch_range = 16.0f;
      else if (deck.pitch_range < 20.0f)
        deck.pitch_range = 100.0f;
      else
        deck.pitch_range = 6.0f;
    }
    bool rg_hov = ImGui::IsItemHovered();
    ImU32 rg_bg = (deck.pitch_range >= 90.0f)
                      ? IM_COL32(150, 25, 38, 230)
                      : (rg_hov ? IM_COL32(38, 48, 64, 240) : IM_COL32(24, 30, 42, 220));
    ImU32 rg_bd = (deck.pitch_range >= 90.0f) ? IM_COL32(255, 75, 90, 255) : IM_COL32(50, 64, 82, 200);
    dl->AddRectFilled(rg_pos, ImVec2(rg_pos.x + rg_w, rg_pos.y + rg_h), rg_bg, KuroUI::UIScaleManager::Scale(3.0f));
    dl->AddRect(rg_pos, ImVec2(rg_pos.x + rg_w, rg_pos.y + rg_h), rg_bd, KuroUI::UIScaleManager::Scale(3.0f), 0, 1.0f);
    ImVec2 sz_rg_txt = f_ctrl->CalcTextSizeA(KuroUI::UIScaleManager::Scale(11.0f), FLT_MAX, -1.0f, rg_str);
    dl->AddText(f_ctrl, KuroUI::UIScaleManager::Scale(11.0f),
                ImVec2(rg_pos.x + (rg_w - sz_rg_txt.x) * 0.5f, rg_pos.y + (rg_h - sz_rg_txt.y) * 0.5f),
                (deck.pitch_range >= 90.0f) ? IM_COL32(255, 180, 190, 255) : IM_COL32(200, 220, 245, 230), rg_str);

    // Texto de valor do pitch com clipping para não encavalar no botão de Range
    dl->PushClipRect(ImVec2(pos.x + KuroUI::UIScaleManager::Scale(4.0f), pos.y + KuroUI::UIScaleManager::Scale(20.0f)),
                     ImVec2(rg_pos.x - KuroUI::UIScaleManager::Scale(2.0f), pos.y + hdr_h - KuroUI::UIScaleManager::Scale(2.0f)), true);
    float v_font_sz = (size.x < KuroUI::UIScaleManager::Scale(80.0f)) ? KuroUI::UIScaleManager::Scale(11.5f) : KuroUI::UIScaleManager::Scale(13.0f);
    dl->AddText(f_bld, v_font_sz, ImVec2(pos.x + KuroUI::UIScaleManager::Scale(5.0f), pos.y + KuroUI::UIScaleManager::Scale(24.0f)), col_val,
                val_str);
    dl->PopClipRect();"""

if fader_old in content:
    content = content.replace(fader_old, fader_new, 1)
    print("[1] Pitch Fader Header updated successfully.")
else:
    print("[1] ERROR: Pitch Fader Header not found!")
    sys.exit(1)

# Ensure fader width minimum in line 15843
clamp_old = "float fader_w = std::clamp(usable_w * 0.095f, KuroUI::UIScaleManager::Scale(78.0f), KuroUI::UIScaleManager::Scale(125.0f));"
clamp_new = "float fader_w = std::clamp(usable_w * 0.098f, KuroUI::UIScaleManager::Scale(88.0f), KuroUI::UIScaleManager::Scale(130.0f));"
if clamp_old in content:
    content = content.replace(clamp_old, clamp_new, 1)
    print("[1.1] Fader width clamp updated.")
else:
    print("[1.1] WARNING: Fader width clamp not found, continuing.")

# -----------------------------------------------------------------------------
# 2. FIX REKORDBOX TOP BAR: QUANTIZE CAPSULE & RIGHT ALA ANTI-COLLISION
# -----------------------------------------------------------------------------
topbar_quant_old = """    // Capsula [QUANTIZE] com anel verde-esmeralda
    {
      bool quant_active =
          engine.decks[0].quantize_active || engine.decks[1].quantize_active;
      capsuleBtn("##TopQuantEngine", "QUANTIZE", quant_active,
                 IM_COL32(0, 225, 125, 255), IM_COL32(210, 255, 230, 255),
                 "QUANTIZE: Alinhamento magnético automático de Cues, Loops e "
                 "saltos na Beatgrid",
                 [&]() {
                   engine.decks[0].quantize_active =
                       !engine.decks[0].quantize_active;
                   engine.decks[1].quantize_active =
                       engine.decks[0].quantize_active;
                 });
    }"""

topbar_quant_new = """    // Capsula [QUANTIZE] com anel verde-esmeralda (Padrão 1:1 Hardware Pill)
    {
      bool quant_active =
          engine.decks[0].quantize_active || engine.decks[1].quantize_active;
      ImVec2 sz_q = ImGui::CalcTextSize("QUANTIZE");
      float q_w = std::max(KuroUI::UIScaleManager::Scale(68.0f), sz_q.x + KuroUI::UIScaleManager::Scale(26.0f));
      float q_h = topbar_btn_h;
      ImVec2 q_pos = ImGui::GetCursorScreenPos();
      ImVec2 q_max = ImVec2(q_pos.x + q_w, q_pos.y + q_h);
      dl->AddRectFilled(q_pos, q_max,
                        quant_active ? IM_COL32(10, 28, 22, 255)
                                     : IM_COL32(14, 18, 24, 255),
                        q_h * 0.5f);
      dl->AddRect(q_pos, q_max,
                  quant_active ? IM_COL32(0, 230, 130, 240)
                               : IM_COL32(45, 54, 68, 180),
                  q_h * 0.5f, 0, quant_active ? 1.5f : 1.0f);
      dl->AddCircleFilled(ImVec2(q_pos.x + KuroUI::UIScaleManager::Scale(9.0f), q_pos.y + q_h * 0.5f), KuroUI::UIScaleManager::Scale(3.0f),
                          quant_active ? IM_COL32(0, 255, 140, 255)
                                       : IM_COL32(65, 80, 100, 200));
      dl->AddText(ImVec2(q_pos.x + KuroUI::UIScaleManager::Scale(16.0f) + (q_w - KuroUI::UIScaleManager::Scale(18.0f) - sz_q.x) * 0.5f,
                         q_pos.y + (q_h - sz_q.y) * 0.5f),
                  quant_active ? IM_COL32(210, 255, 230, 255) : IM_COL32(160, 175, 195, 220),
                  "QUANTIZE");

      ImGui::InvisibleButton("##TopQuantEngine", ImVec2(q_w, q_h));
      if (ImGui::IsItemClicked(0)) {
        engine.decks[0].quantize_active = !engine.decks[0].quantize_active;
        engine.decks[1].quantize_active = engine.decks[0].quantize_active;
      }
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("QUANTIZE: Alinhamento magnético automático de Cues, Loops e saltos na Beatgrid");
      ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(6.0f));
    }"""

if topbar_quant_old in content:
    content = content.replace(topbar_quant_old, topbar_quant_new, 1)
    print("[2] Top Bar QUANTIZE capsule updated.")
else:
    print("[2] ERROR: Top Bar QUANTIZE capsule not found!")
    sys.exit(1)

# Top Bar Right Ala calculation & placement
topbar_right_old = """    // ── 3. ALA DIREITA: SISTEMA, BUSCA E UTILITARIOS ────────────────────
    ImGui::SetCursorScreenPos(ImVec2(r_x, topbar_item_y));

    // [LOAD LOCK]
    auto &safety = KuroAudio::DJLiveSafety::getInstance();
    bool lck_on = safety.isLoadLockEnabled();
    capsuleBtn("##TopLoadLock",
               lck_on ? ICON_FA_LOCK " LOCK" : ICON_FA_LOCK_OPEN " UNLOCK",
               lck_on, IM_COL32(0, 200, 120, 255), IM_COL32(220, 255, 235, 255),
               lck_on ? "LOAD LOCK ATIVADO: Bloqueia carregar faixa em deck "
                        "tocando ao vivo"
                      : "LOAD LOCK DESATIVADO: Clique para ativar protecao",
               [&]() { safety.toggleLoadLock(); });

    // Status MIDI da controladora fisica
    bool hw_ok = ddj_hardware.isConnected();
    std::string midi_lbl =
        hw_ok ? ddj_hardware.getControllerModelName() : "MIDI";
    capsuleBtn("##TopMidiHw", midi_lbl.c_str(), hw_ok,
               IM_COL32(0, 230, 130, 255), IM_COL32(210, 255, 230, 255),
               "Central MIDI Learn: Clique para abrir o mapeamento de "
               "controles da mesa",
               [&]() {
                 show_midi_learn_modal = !show_midi_learn_modal;
                 ddj_hardware.autoConnect(&engine);
               });

    // Relogio Digital HH:MM
    {
      time_t rawtime;
      time(&rawtime);
      struct tm *timeinfo = localtime(&rawtime);
      char time_buf[16] = "18:20";
      if (timeinfo)
        strftime(time_buf, sizeof(time_buf), "%H:%M", timeinfo);

      ImVec2 clk_pos = ImGui::GetCursorScreenPos();
      ImVec2 sz_clk = ImGui::CalcTextSize(time_buf);
      float clk_w = sz_clk.x + KuroUI::UIScaleManager::Scale(8.0f);
      dl->AddText(ImVec2(clk_pos.x + KuroUI::UIScaleManager::Scale(2.0f), clk_pos.y + (topbar_btn_h - sz_clk.y) * 0.5f),
                  IM_COL32(180, 195, 215, 240), time_buf);
      ImGui::Dummy(ImVec2(clk_w, topbar_btn_h));
      ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(5.0f));
    }

    // Botão Cyberpunk / Rekordbox: ABDUCTION STUDIO // MASTER SOURCE MATRIX
    {
      ImVec2 s_pos = ImGui::GetCursorScreenPos();
      bool is_dl = spotify_service.isDownloading();
      float pulse = (std::sin((float)ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
      int dl_pct =
          std::clamp((int)spotify_service.getDownloadProgressPct(), 0, 100);

      std::string btn_label;
      if (is_dl) {
        int q_tot = spotify_service.getQueueTotalCount();
        if (q_tot > 1) {
          btn_label = std::string(ICON_FA_BOLT) + " FILA [" +
                      std::to_string(spotify_service.getQueueCurrentIndex()) +
                      "/" + std::to_string(q_tot) + "] " +
                      std::to_string(dl_pct) + "%";
        } else if (dl_pct > 0) {
          btn_label = std::string(ICON_FA_BOLT) + " ABDUZINDO " +
                      std::to_string(dl_pct) + "%";
        } else {
          btn_label = std::string(ICON_FA_BOLT) + " ABDUZINDO...";
        }
      } else {
        btn_label = std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUCTION HUB";
      }
      ImVec2 t_sz = ImGui::CalcTextSize(btn_label.c_str());
      float s_w = std::max(KuroUI::UIScaleManager::Scale(145.0f), t_sz.x + KuroUI::UIScaleManager::Scale(26.0f));"""

topbar_right_new = """    // ── 3. ALA DIREITA: SISTEMA, BUSCA E UTILITARIOS (ANTI-COLISÃO DINÂMICA) ─────
    auto &safety = KuroAudio::DJLiveSafety::getInstance();
    bool lck_on = safety.isLoadLockEnabled();
    const char *lck_txt = lck_on ? ICON_FA_LOCK " LOCK" : ICON_FA_LOCK_OPEN " UNLOCK";
    ImVec2 sz_lck = ImGui::CalcTextSize(lck_txt);
    float lck_w = std::max(KuroUI::UIScaleManager::Scale(34.0f), sz_lck.x + KuroUI::UIScaleManager::Scale(16.0f));

    bool hw_ok = ddj_hardware.isConnected();
    std::string midi_lbl = hw_ok ? ddj_hardware.getControllerModelName() : "MIDI";
    ImVec2 sz_midi = ImGui::CalcTextSize(midi_lbl.c_str());
    float midi_w = std::max(KuroUI::UIScaleManager::Scale(34.0f), sz_midi.x + KuroUI::UIScaleManager::Scale(16.0f));

    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    char time_buf[16] = "18:20";
    if (timeinfo)
      strftime(time_buf, sizeof(time_buf), "%H:%M", timeinfo);
    ImVec2 sz_clk = ImGui::CalcTextSize(time_buf);
    float clk_w = sz_clk.x + KuroUI::UIScaleManager::Scale(8.0f);

    bool is_dl = spotify_service.isDownloading();
    float pulse = (std::sin((float)ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
    int dl_pct = std::clamp((int)spotify_service.getDownloadProgressPct(), 0, 100);

    std::string btn_label;
    if (is_dl) {
      int q_tot = spotify_service.getQueueTotalCount();
      if (q_tot > 1) {
        btn_label = std::string(ICON_FA_BOLT) + " FILA [" +
                    std::to_string(spotify_service.getQueueCurrentIndex()) +
                    "/" + std::to_string(q_tot) + "] " +
                    std::to_string(dl_pct) + "%";
      } else if (dl_pct > 0) {
        btn_label = std::string(ICON_FA_BOLT) + " ABDUZINDO " +
                    std::to_string(dl_pct) + "%";
      } else {
        btn_label = std::string(ICON_FA_BOLT) + " ABDUZINDO...";
      }
    } else {
      btn_label = std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUCTION HUB";
    }
    ImVec2 t_sz = ImGui::CalcTextSize(btn_label.c_str());
    float s_w = std::max(KuroUI::UIScaleManager::Scale(145.0f), t_sz.x + KuroUI::UIScaleManager::Scale(26.0f));
    float x_w = p_open ? KuroUI::UIScaleManager::Scale(20.0f) : 0.0f;
    float gap_r = KuroUI::UIScaleManager::Scale(6.0f);

    float total_right_w = lck_w + gap_r + midi_w + gap_r + clk_w + gap_r + s_w + (p_open ? (gap_r + x_w) : 0.0f);
    float r_x_dyn = pos.x + w - total_right_w - KuroUI::UIScaleManager::Scale(8.0f);
    ImGui::SetCursorScreenPos(ImVec2(r_x_dyn, topbar_item_y));

    // [LOAD LOCK]
    capsuleBtn("##TopLoadLock",
               lck_txt,
               lck_on, IM_COL32(0, 200, 120, 255), IM_COL32(220, 255, 235, 255),
               lck_on ? "LOAD LOCK ATIVADO: Bloqueia carregar faixa em deck "
                        "tocando ao vivo"
                      : "LOAD LOCK DESATIVADO: Clique para ativar protecao",
               [&]() { safety.toggleLoadLock(); });

    // Status MIDI da controladora fisica
    capsuleBtn("##TopMidiHw", midi_lbl.c_str(), hw_ok,
               IM_COL32(0, 230, 130, 255), IM_COL32(210, 255, 230, 255),
               "Central MIDI Learn: Clique para abrir o mapeamento de "
               "controles da mesa",
               [&]() {
                 show_midi_learn_modal = !show_midi_learn_modal;
                 ddj_hardware.autoConnect(&engine);
               });

    // Relogio Digital HH:MM
    {
      ImVec2 clk_pos = ImGui::GetCursorScreenPos();
      dl->AddText(ImVec2(clk_pos.x + KuroUI::UIScaleManager::Scale(2.0f), clk_pos.y + (topbar_btn_h - sz_clk.y) * 0.5f),
                  IM_COL32(180, 195, 215, 240), time_buf);
      ImGui::Dummy(ImVec2(clk_w, topbar_btn_h));
      ImGui::SameLine(0, gap_r);
    }

    // Botão Cyberpunk / Rekordbox: ABDUCTION STUDIO // MASTER SOURCE MATRIX
    {
      ImVec2 s_pos = ImGui::GetCursorScreenPos();"""

if topbar_right_old in content:
    content = content.replace(topbar_right_old, topbar_right_new, 1)
    print("[2.1] Top Bar Right Ala updated.")
else:
    print("[2.1] ERROR: Top Bar Right Ala not found!")
    sys.exit(1)

# -----------------------------------------------------------------------------
# 3. SEARCH MODAL HEADER: DECK BADGES & WINDOW CONTROLS
# -----------------------------------------------------------------------------
modal_hdr_old = """        // Badges Tecnológicos no Centro-Direita
        ImGui::SameLine(0, 12.0f);
        ImGui::TextColored(ImVec4(0.00f, 1.00f, 0.60f, 1.0f), ICON_FA_BOLT " DSP 24-BIT / 48kHz");
        ImGui::SameLine(0, 10.0f);
        ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.0f), ICON_FA_SHIELD_HALVED " REKORDBOX 4-DECK MASTER");
        ImGui::SameLine(0, 10.0f);
        ImGui::TextColored(ImVec4(0.11f, 0.80f, 0.40f, 1.0f), ICON_FA_COMPACT_DISC " SPOTIFY SYNC ACTIVE");

        // 4 Deck status badges — canto direito
        float dbadge_x = h_start.x + avail_w - 335.0f;
        float dbadge_y = h_start.y + 9.0f;
        struct DeckBadge { const char* label; bool playing; ImVec4 col; };
        DeckBadge dbadges[4] = {
            { engine.decks[0].is_playing.load() ? "D1: ON AIR" : "D1: LIVRE", engine.decks[0].is_playing.load(), col_deck_1 },
            { engine.decks[1].is_playing.load() ? "D2: ON AIR" : "D2: CUE",   engine.decks[1].is_playing.load(), col_deck_2 },
            { engine.decks[2].is_playing.load() ? "D3: ON AIR" : "D3: LIVRE", engine.decks[2].is_playing.load(), col_deck_3 },
            { engine.decks[3].is_playing.load() ? "D4: ON AIR" : "D4: LIVRE", engine.decks[3].is_playing.load(), col_deck_4 },
        };
        for (int di = 0; di < 4; di++) {
            ImGui::SetCursorScreenPos(ImVec2(dbadge_x + di * 62.0f, dbadge_y));
            ImVec4 bc = dbadges[di].col;
            ImGui::PushStyleColor(ImGuiCol_Text, dbadges[di].playing ?
                ImVec4(bc.x, bc.y, bc.z, 1.0f) : ImVec4(bc.x * 0.55f, bc.y * 0.55f, bc.z * 0.55f, 0.80f));
            ImGui::TextUnformatted(dbadges[di].label);
            ImGui::PopStyleColor();
        }

        // Botões de Janela (Minimizar, Maximizar, Fechar)
        ImGui::SetCursorScreenPos(ImVec2(h_start.x + avail_w - 78.0f, h_start.y + 8.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.82f, 0.95f, 0.90f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        if (ImGui::Button("_##MinHub", ImVec2(22, 22))) { show_search_modal = false; }
        ImGui::SameLine(0, 3.0f);
        if (ImGui::Button("[ ]##MaxHub", ImVec2(22, 22))) { /* maximized */ }
        ImGui::SameLine(0, 3.0f);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.12f, 0.20f, 1.0f));
        if (ImGui::Button(ICON_FA_XMARK "##CloseHub", ImVec2(22, 22))) {
            show_search_modal = false;
            search_modal_view = 0;
            preview_player.stop();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();"""

modal_hdr_new = """        // Badges Tecnológicos no Centro-Direita (Exibidos somente em telas amplas para evitar colisão)
        if (avail_w > KuroUI::UIScaleManager::Scale(1280.0f)) {
            ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(12.0f));
            ImGui::TextColored(ImVec4(0.00f, 1.00f, 0.60f, 1.0f), ICON_FA_BOLT " DSP 24-BIT / 48kHz");
            ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(10.0f));
            ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.0f), ICON_FA_SHIELD_HALVED " 4-DECK MASTER");
            ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(10.0f));
            ImGui::TextColored(ImVec4(0.11f, 0.80f, 0.40f, 1.0f), ICON_FA_COMPACT_DISC " SPOTIFY SYNC");
        }

        // Botões de Janela ancorados na extrema direita
        float win_btn_w = KuroUI::UIScaleManager::Scale(24.0f);
        float win_btn_h = KuroUI::UIScaleManager::Scale(22.0f);
        float win_ctrl_w = win_btn_w * 3.0f + KuroUI::UIScaleManager::Scale(8.0f);
        float win_x = h_start.x + avail_w - win_ctrl_w - KuroUI::UIScaleManager::Scale(6.0f);
        float win_y = h_start.y + KuroUI::UIScaleManager::Scale(7.0f);

        ImGui::SetCursorScreenPos(ImVec2(win_x, win_y));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.82f, 0.95f, 0.90f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, KuroUI::UIScaleManager::Scale(3.0f));
        if (ImGui::Button("_##MinHub", ImVec2(win_btn_w, win_btn_h))) { show_search_modal = false; }
        ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(4.0f));
        if (ImGui::Button("[ ]##MaxHub", ImVec2(win_btn_w, win_btn_h))) { /* maximized */ }
        ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(4.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.12f, 0.20f, 1.0f));
        if (ImGui::Button(ICON_FA_XMARK "##CloseHub", ImVec2(win_btn_w, win_btn_h))) {
            show_search_modal = false;
            search_modal_view = 0;
            preview_player.stop();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        // 4 Deck status badges — Cápsulas de Hardware Independentes sem sobreposição
        float dbadge_slot_w = KuroUI::UIScaleManager::Scale(72.0f);
        float dbadge_slot_h = KuroUI::UIScaleManager::Scale(20.0f);
        float dbadges_tot_w = dbadge_slot_w * 4.0f + KuroUI::UIScaleManager::Scale(15.0f);
        float dbadge_x = win_x - dbadges_tot_w - KuroUI::UIScaleManager::Scale(12.0f);
        float dbadge_y = h_start.y + KuroUI::UIScaleManager::Scale(8.0f);

        struct DeckBadge { const char* label; bool playing; ImVec4 col; };
        DeckBadge dbadges[4] = {
            { engine.decks[0].is_playing.load() ? "D1: ON AIR" : "D1: LIVRE", engine.decks[0].is_playing.load(), col_deck_1 },
            { engine.decks[1].is_playing.load() ? "D2: ON AIR" : "D2: CUE",   engine.decks[1].is_playing.load(), col_deck_2 },
            { engine.decks[2].is_playing.load() ? "D3: ON AIR" : "D3: LIVRE", engine.decks[2].is_playing.load(), col_deck_3 },
            { engine.decks[3].is_playing.load() ? "D4: ON AIR" : "D4: LIVRE", engine.decks[3].is_playing.load(), col_deck_4 },
        };
        ImDrawList* h_dl = ImGui::GetWindowDrawList();
        for (int di = 0; di < 4; di++) {
            ImVec2 b_min = ImVec2(dbadge_x + di * (dbadge_slot_w + KuroUI::UIScaleManager::Scale(5.0f)), dbadge_y);
            ImVec2 b_max = ImVec2(b_min.x + dbadge_slot_w, b_min.y + dbadge_slot_h);
            ImVec4 bc = dbadges[di].col;
            ImU32 bd_bg = dbadges[di].playing ? IM_COL32((int)(bc.x * 40.0f), (int)(bc.y * 40.0f), (int)(bc.z * 40.0f), 240) : IM_COL32(12, 16, 24, 200);
            ImU32 bd_border = dbadges[di].playing ? ImGui::GetColorU32(bc) : IM_COL32((int)(bc.x * 120.0f), (int)(bc.y * 120.0f), (int)(bc.z * 120.0f), 180);
            h_dl->AddRectFilled(b_min, b_max, bd_bg, KuroUI::UIScaleManager::Scale(3.0f));
            h_dl->AddRect(b_min, b_max, bd_border, KuroUI::UIScaleManager::Scale(3.0f), 0, 1.0f);
            
            ImVec2 t_sz_b = ImGui::CalcTextSize(dbadges[di].label);
            h_dl->AddText(ImVec2(b_min.x + (dbadge_slot_w - t_sz_b.x) * 0.5f, b_min.y + (dbadge_slot_h - t_sz_b.y) * 0.5f),
                          dbadges[di].playing ? ImGui::GetColorU32(bc) : IM_COL32((int)(bc.x * 180.0f), (int)(bc.y * 180.0f), (int)(bc.z * 180.0f), 220),
                          dbadges[di].label);
        }"""

if modal_hdr_old in content:
    content = content.replace(modal_hdr_old, modal_hdr_new, 1)
    print("[3] Search Modal Header updated.")
else:
    print("[3] ERROR: Search Modal Header not found!")
    sys.exit(1)

# -----------------------------------------------------------------------------
# 4. OMNIBAR BUTTONS & PLATFORM CHIPS & BAIXAR DISCOGRAFIA
# -----------------------------------------------------------------------------
# Dig Deeper button in Omnibar
omni_dig_old = 'if (ImGui::Button(ICON_FA_COMPACT_DISC " 🔮 DIG DEEPER", ImVec2(omni_dig_w, 36.0f))) {'
omni_dig_new = 'if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " DIG DEEPER", ImVec2(omni_dig_w, KuroUI::UIScaleManager::Scale(36.0f)))) {'
if omni_dig_old in content:
    content = content.replace(omni_dig_old, omni_dig_new, 1)
    print("[4.1] Omnibar Dig Deeper icon updated.")
else:
    print("[4.1] WARNING: Omnibar Dig Deeper icon not found.")

# Platform filters with registered mark
filters_old = """            PlatFilter filters[] = {
                { 0, "● TODAS AS FONTES (109)", col_deck_1 },
                { 1, "● SPOTIFY SYNC (84)",    ImVec4(0.11f, 0.80f, 0.40f, 1.0f) },
                { 2, "● SOUNDCLOUD HQ (38)",   ImVec4(1.00f, 0.45f, 0.05f, 1.0f) },
                { 5, "● BEATPORT PRO (42)",    ImVec4(0.00f, 0.70f, 1.00f, 1.0f) },
                { 4, "● YOUTUBE TOPIC (56)",   ImVec4(1.00f, 0.25f, 0.28f, 1.0f) },
                { 3, "● TIDAL HI-RES (29)",    ImVec4(0.68f, 0.35f, 1.00f, 1.0f) },
            };"""

filters_new = """            PlatFilter filters[] = {
                { 0, "● TODAS AS FONTES",      col_deck_1 },
                { 1, "● SPOTIFY® SYNC",        ImVec4(0.11f, 0.80f, 0.40f, 1.0f) },
                { 2, "● SOUNDCLOUD® HQ",       ImVec4(1.00f, 0.45f, 0.05f, 1.0f) },
                { 5, "● BEATPORT® PRO",        ImVec4(0.00f, 0.70f, 1.00f, 1.0f) },
                { 4, "● YOUTUBE® TOPIC",       ImVec4(1.00f, 0.25f, 0.28f, 1.0f) },
                { 3, "● TIDAL® HI-RES",        ImVec4(0.68f, 0.35f, 1.00f, 1.0f) },
            };"""

if filters_old in content:
    content = content.replace(filters_old, filters_new, 1)
    print("[4.2] Platform Filters updated with ®.")
else:
    print("[4.2] WARNING: Platform Filters not found.")

# Baixar Discografia Button (height and width scaled to prevent slicing)
disc_btn_old = """            // Botão Roxo no Canto Direito: ⚡ BAIXAR DISCOGRAFIA (109 FAIXAS)
            ImGui::SameLine(avail_w - 265.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.43f, 0.16f, 0.71f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.22f, 0.88f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            auto results_all = spotify_service.getSearchResults();
            char disc_btn_lbl[64];
            snprintf(disc_btn_lbl, sizeof(disc_btn_lbl), ICON_FA_BOLT " BAIXAR DISCOGRAFIA (%d FAIXAS)", (int)results_all.size() > 0 ? (int)results_all.size() : 109);
            if (ImGui::Button(disc_btn_lbl, ImVec2(265.0f, 26.0f))) {"""

disc_btn_new = """            // Botão Roxo no Canto Direito: ⚡ BAIXAR DISCOGRAFIA (Totalmente escalado e sem corte)
            float disc_btn_w = KuroUI::UIScaleManager::Scale(290.0f);
            float disc_btn_h = KuroUI::UIScaleManager::Scale(32.0f);
            ImGui::SameLine(avail_w - disc_btn_w);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.43f, 0.16f, 0.71f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.22f, 0.88f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, KuroUI::UIScaleManager::Scale(5.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(KuroUI::UIScaleManager::Scale(8.0f), KuroUI::UIScaleManager::Scale(4.0f)));
            auto results_all = spotify_service.getSearchResults();
            char disc_btn_lbl[64];
            snprintf(disc_btn_lbl, sizeof(disc_btn_lbl), ICON_FA_BOLT " BAIXAR DISCOGRAFIA (%d FAIXAS)", (int)results_all.size() > 0 ? (int)results_all.size() : 109);
            if (ImGui::Button(disc_btn_lbl, ImVec2(disc_btn_w, disc_btn_h))) {
                if (!results_all.empty()) {
                    std::string artist_disc = results_all[0].artist;
                    spotify_service.importPlaylistAsCrate(artist_disc, results_all);
                    spotify_service.enqueueCrateDownload(results_all);
                    cloud_hub_active_tab = 2; // vai para Fila & Downloads
                    toast_message = std::string(ICON_FA_DOWNLOAD) + " Baixando discografia completa de " + artist_disc;
                    toast_timer = 4.0f;
                }
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);"""

# Notice that the block ends with } followed by ImGui::PopStyleVar(); ImGui::PopStyleColor(3);
# We match up to the original closing
disc_full_old = """            // Botão Roxo no Canto Direito: ⚡ BAIXAR DISCOGRAFIA (109 FAIXAS)
            ImGui::SameLine(avail_w - 265.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.43f, 0.16f, 0.71f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.22f, 0.88f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            auto results_all = spotify_service.getSearchResults();
            char disc_btn_lbl[64];
            snprintf(disc_btn_lbl, sizeof(disc_btn_lbl), ICON_FA_BOLT " BAIXAR DISCOGRAFIA (%d FAIXAS)", (int)results_all.size() > 0 ? (int)results_all.size() : 109);
            if (ImGui::Button(disc_btn_lbl, ImVec2(265.0f, 26.0f))) {
                if (!results_all.empty()) {
                    std::string artist_disc = results_all[0].artist;
                    spotify_service.importPlaylistAsCrate(artist_disc, results_all);
                    spotify_service.enqueueCrateDownload(results_all);
                    cloud_hub_active_tab = 2; // vai para Fila & Downloads
                    toast_message = std::string(ICON_FA_DOWNLOAD) + " Baixando discografia completa de " + artist_disc;
                    toast_timer = 4.0f;
                }
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);"""

if disc_full_old in content:
    content = content.replace(disc_full_old, disc_btn_new, 1)
    print("[4.3] Baixar Discografia Button updated.")
else:
    print("[4.3] ERROR: Baixar Discografia Button not found!")
    sys.exit(1)

# -----------------------------------------------------------------------------
# 5. TABLE RESULTS: FONTES BADGES & DIG DEEPER BUTTON REPLACEMENT
# -----------------------------------------------------------------------------
# Platform badges in table: SP, SC, BP, YT -> SP®, SC®, BP®, YT® with micro-pill
table_badges_old = """                            // Col 6: Fontes Disponíveis (Badges SP, SC, BP, YT - Fiel ao Mockup)
                            ImGui::TableSetColumnIndex(6);
                            auto mini_badge = [&](const char* lbl, ImVec4 col) {
                                ImGui::PushStyleColor(ImGuiCol_Text, col);
                                ImGui::TextUnformatted(lbl);
                                ImGui::PopStyleColor();
                                ImGui::SameLine(0, 4.0f);
                            };
                            mini_badge("SP", ImVec4(0.11f, 0.80f, 0.40f, 1.0f));
                            mini_badge("SC", ImVec4(1.00f, 0.45f, 0.05f, 1.0f));
                            mini_badge("BP", ImVec4(0.00f, 0.70f, 1.00f, 1.0f));
                            mini_badge("YT", ImVec4(1.00f, 0.25f, 0.28f, 1.0f));"""

table_badges_new = """                            // Col 6: Fontes Disponíveis (Badges SP®, SC®, BP®, YT® - Micro-Pill com ®)
                            ImGui::TableSetColumnIndex(6);
                            auto mini_badge = [&](const char* lbl, ImVec4 col) {
                                ImVec2 bpos = ImGui::GetCursorScreenPos();
                                ImVec2 bsz = ImGui::CalcTextSize(lbl);
                                float pill_w = bsz.x + KuroUI::UIScaleManager::Scale(8.0f);
                                float pill_h = KuroUI::UIScaleManager::Scale(16.0f);
                                ImDrawList* tbl_dl = ImGui::GetWindowDrawList();
                                tbl_dl->AddRectFilled(bpos, ImVec2(bpos.x + pill_w, bpos.y + pill_h),
                                                      IM_COL32((int)(col.x * 35.0f), (int)(col.y * 35.0f), (int)(col.z * 35.0f), 220),
                                                      KuroUI::UIScaleManager::Scale(3.0f));
                                tbl_dl->AddRect(bpos, ImVec2(bpos.x + pill_w, bpos.y + pill_h),
                                                ImGui::GetColorU32(col), KuroUI::UIScaleManager::Scale(3.0f), 0, 1.0f);
                                tbl_dl->AddText(ImVec2(bpos.x + (pill_w - bsz.x) * 0.5f, bpos.y + (pill_h - bsz.y) * 0.5f),
                                                ImGui::GetColorU32(col), lbl);
                                ImGui::Dummy(ImVec2(pill_w, pill_h));
                                ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(4.0f));
                            };
                            mini_badge("SP®", ImVec4(0.11f, 0.80f, 0.40f, 1.0f));
                            mini_badge("SC®", ImVec4(1.00f, 0.45f, 0.05f, 1.0f));
                            mini_badge("BP®", ImVec4(0.00f, 0.70f, 1.00f, 1.0f));
                            mini_badge("YT®", ImVec4(1.00f, 0.25f, 0.28f, 1.0f));"""

if table_badges_old in content:
    content = content.replace(table_badges_old, table_badges_new, 1)
    print("[5.1] Table Platform Badges updated.")
else:
    print("[5.1] ERROR: Table Platform Badges not found!")
    sys.exit(1)

# Table Row Dig Deeper button (replace broken glyph "🔮" with ICON_FA_WAND_MAGIC_SPARKLES)
table_dig_old = """                            // Botão 🔮 Dig Deeper na linha da tabela
                            ImGui::SameLine(0, 4.0f);
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.10f, 0.48f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.18f, 0.72f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.30f, 1.00f, 0.85f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            if (ImGui::Button("🔮", ImVec2(KuroUI::UIScaleManager::Scale(28.0f), KuroUI::UIScaleManager::Scale(26.0f)))) {
                                selected_source_track_id = sr.id;
                                selected_source_track_title = sr.title;
                                selected_source_track_artist = sr.artist;
                                selected_source_track_album = sr.album;
                                selected_source_track_bpm = sr.bpm;
                                selected_source_track_key = sr.key;
                                selected_source_track_subgenre = sr.subgenre;
                                spotify_service.digDeeperAsync(sr.title, sr.artist, sr.key, sr.bpm);
                                spotify_service.show_dig_deeper_view = true;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🔮 Dig Deeper: minerar almas gêmeas acústicas para '%s'", sr.title.c_str());"""

table_dig_new = """                            // Botão Dig Deeper na linha da tabela (Ícone Magic Sparkles de Alta Fidelidade)
                            ImGui::SameLine(0, KuroUI::UIScaleManager::Scale(4.0f));
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.10f, 0.48f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.18f, 0.72f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.30f, 1.00f, 0.85f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, KuroUI::UIScaleManager::Scale(4.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            char row_dig_id[32]; snprintf(row_dig_id, sizeof(row_dig_id), "%s##DigRow_%d", ICON_FA_WAND_MAGIC_SPARKLES, r_idx);
                            if (ImGui::Button(row_dig_id, ImVec2(KuroUI::UIScaleManager::Scale(28.0f), KuroUI::UIScaleManager::Scale(26.0f)))) {
                                selected_source_track_id = sr.id;
                                selected_source_track_title = sr.title;
                                selected_source_track_artist = sr.artist;
                                selected_source_track_album = sr.album;
                                selected_source_track_bpm = sr.bpm;
                                selected_source_track_key = sr.key;
                                selected_source_track_subgenre = sr.subgenre;
                                spotify_service.digDeeperAsync(sr.title, sr.artist, sr.key, sr.bpm);
                                spotify_service.show_dig_deeper_view = true;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip(ICON_FA_WAND_MAGIC_SPARKLES " Dig Deeper: minerar almas gêmeas acústicas para '%s'", sr.title.c_str());"""

if table_dig_old in content:
    content = content.replace(table_dig_old, table_dig_new, 1)
    print("[5.2] Table Dig Deeper button updated.")
else:
    print("[5.2] ERROR: Table Dig Deeper button not found!")
    sys.exit(1)

# Also ensure texture is uploaded when clicking row
row_sel_old = """                                selected_source_track_dur = sr.duration_sec;
                                selected_source_track_art_url = sr.artwork_url;
                                spotify_service.searchSourcesAsync(sr.id, sr.title, sr.artist);"""

row_sel_new = """                                selected_source_track_dur = sr.duration_sec;
                                selected_source_track_art_url = sr.artwork_url;
                                if (!sr.local_rgba_path.empty()) {
                                    selected_source_track_tex = KuroAudio::SpotifyDJService::UploadRGBATexture(sr.local_rgba_path);
                                }
                                spotify_service.searchSourcesAsync(sr.id, sr.title, sr.artist);"""

# Replace both occurrences in table selection
content = content.replace(row_sel_old, row_sel_new)
print("[5.3] Row texture upload added.")

# -----------------------------------------------------------------------------
# 6. RIGHT PANEL HERO BOX: ARTWORK THUMBNAIL (REAL COVER / VINYL ART)
# -----------------------------------------------------------------------------
hero_old = """                // ── HERO BOX DA FAIXA SELECIONADA (Fiel ao Mockup) ─────────────
                float hero_h = 132.0f;
                ImVec2 hero_p = ImGui::GetCursorScreenPos();
                rdl->AddRectFilled(hero_p, ImVec2(hero_p.x + rw, hero_p.y + hero_h), IM_COL32(8, 14, 22, 255), 5.0f);
                rdl->AddRect(hero_p, ImVec2(hero_p.x + rw, hero_p.y + hero_h), IM_COL32(0, 220, 200, 90), 5.0f, 0, 1.2f);

                // Arte Psicodélica em Círculos Concêntricos Sangoma Records (Fiel ao Mockup)
                float art_sz = 96.0f;
                ImVec2 art_p = ImVec2(hero_p.x + 12.0f, hero_p.y + (hero_h - art_sz) * 0.5f);
                rdl->AddRectFilled(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(6, 10, 18, 255), 4.0f);
                rdl->AddRect(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(0, 245, 212, 140), 4.0f);
                float cax = art_p.x + art_sz * 0.5f, cay = art_p.y + art_sz * 0.5f - 6.0f;
                rdl->AddCircleFilled(ImVec2(cax, cay), 3.0f, IM_COL32(0, 245, 212, 255));
                rdl->AddCircle(ImVec2(cax, cay), 9.0f,  IM_COL32(0, 245, 212, 220), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 17.0f, IM_COL32(196, 64, 255, 200), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 25.0f, IM_COL32(0, 187, 255, 180), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 33.0f, IM_COL32(255, 0, 127, 180), 24, 1.2f);
                rdl->AddCircle(ImVec2(cax, cay), 41.0f, IM_COL32(255, 170, 0, 160), 24, 1.2f);
                rdl->AddText(ImVec2(art_p.x + 22.0f, art_p.y + art_sz - 15.0f), IM_COL32(0, 245, 212, 220), "SANGOMA");

                // Textos do Hero Box com layout DPI espaçoso
                float info_x = art_p.x + art_sz + 18.0f;"""

hero_new = """                // ── HERO BOX DA FAIXA SELECIONADA (Thumbnail Ampliada com Destaque Máximo) ──────
                float hero_h = KuroUI::UIScaleManager::Scale(152.0f);
                ImVec2 hero_p = ImGui::GetCursorScreenPos();
                rdl->AddRectFilled(hero_p, ImVec2(hero_p.x + rw, hero_p.y + hero_h), IM_COL32(8, 14, 22, 255), KuroUI::UIScaleManager::Scale(6.0f));
                rdl->AddRect(hero_p, ImVec2(hero_p.x + rw, hero_p.y + hero_h), IM_COL32(0, 220, 200, 120), KuroUI::UIScaleManager::Scale(6.0f), 0, 1.5f);

                // Thumbnail / Capa Oficial da Faixa em Alta Fidelidade
                float art_sz = KuroUI::UIScaleManager::Scale(126.0f);
                ImVec2 art_p = ImVec2(hero_p.x + KuroUI::UIScaleManager::Scale(12.0f), hero_p.y + (hero_h - art_sz) * 0.5f);
                if (selected_source_track_tex > 0) {
                    // Renderiza a Capa Real Carregada
                    rdl->AddImageRounded((ImTextureID)(intptr_t)selected_source_track_tex,
                                         art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz),
                                         ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, KuroUI::UIScaleManager::Scale(6.0f));
                    rdl->AddRect(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz),
                                 IM_COL32(0, 245, 212, 220), KuroUI::UIScaleManager::Scale(6.0f), 0, 1.8f);
                } else {
                    // Vinyl Artwork com Ranhuras Acústicas e Selo Central (Zero sobreposição de texto)
                    rdl->AddRectFilled(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(12, 16, 24, 255), KuroUI::UIScaleManager::Scale(6.0f));
                    rdl->AddRect(art_p, ImVec2(art_p.x + art_sz, art_p.y + art_sz), IM_COL32(0, 245, 212, 180), KuroUI::UIScaleManager::Scale(6.0f), 0, 1.5f);
                    float cax = art_p.x + art_sz * 0.5f, cay = art_p.y + art_sz * 0.5f;
                    rdl->AddCircleFilled(ImVec2(cax, cay), art_sz * 0.44f, IM_COL32(6, 8, 12, 255), 32);
                    rdl->AddCircle(ImVec2(cax, cay), art_sz * 0.44f, IM_COL32(32, 45, 60, 200), 32, 1.0f);
                    rdl->AddCircle(ImVec2(cax, cay), art_sz * 0.36f, IM_COL32(24, 34, 48, 180), 32, 1.0f);
                    rdl->AddCircle(ImVec2(cax, cay), art_sz * 0.28f, IM_COL32(38, 52, 70, 180), 32, 1.0f);
                    rdl->AddCircleFilled(ImVec2(cax, cay), art_sz * 0.18f, IM_COL32(0, 200, 175, 255), 24);
                    rdl->AddCircleFilled(ImVec2(cax, cay), KuroUI::UIScaleManager::Scale(3.5f), IM_COL32(10, 14, 20, 255), 16);
                    ImVec2 cd_sz = ImGui::CalcTextSize(ICON_FA_COMPACT_DISC);
                    rdl->AddText(ImVec2(cax - cd_sz.x * 0.5f, cay - cd_sz.y * 0.5f), IM_COL32(10, 15, 22, 255), ICON_FA_COMPACT_DISC);
                }

                // Textos do Hero Box com layout DPI espaçoso
                float info_x = art_p.x + art_sz + KuroUI::UIScaleManager::Scale(16.0f);"""

if hero_old in content:
    content = content.replace(hero_old, hero_new, 1)
    print("[6.1] Right Panel Hero Box Artwork updated.")
else:
    print("[6.1] ERROR: Right Panel Hero Box Artwork not found!")
    sys.exit(1)

# Fix Dig Deeper button inside Hero Box
hero_dig_old = 'if (ImGui::SmallButton(ICON_FA_COMPACT_DISC " 🔮 DIG DEEPER")) {'
hero_dig_new = 'if (ImGui::SmallButton(ICON_FA_WAND_MAGIC_SPARKLES " DIG DEEPER")) {'
if hero_dig_old in content:
    content = content.replace(hero_dig_old, hero_dig_new, 1)
    print("[6.2] Hero Box Dig Deeper icon updated.")
else:
    print("[6.2] WARNING: Hero Box Dig Deeper icon not found.")

# -----------------------------------------------------------------------------
# 7. SOURCE CARDS: CURSOR POSITION RESTORATION & BRAND MOTIFS & REGISTERED MARK
# -----------------------------------------------------------------------------
card_func_old = """                auto render_source_card = [&](const char* icon, const char* name, const char* sub1, const char* sub2,
                                               int score, ImVec4 border_col, bool is_solid_btn, ImVec4 btn_col, const char* btn_lbl,
                                               const std::string& url) {
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = KuroUI::UIScaleManager::Scale(98.0f);
                    ImVec2 cp = ImGui::GetCursorScreenPos();
                    ImGui::Dummy(ImVec2(cw, ch));

                    rdl->AddRectFilled(cp, ImVec2(cp.x + cw, cp.y + ch), IM_COL32(10, 16, 26, 250), KuroUI::UIScaleManager::Scale(5.0f));
                    rdl->AddRect(cp, ImVec2(cp.x + cw, cp.y + ch), ImGui::GetColorU32(border_col), KuroUI::UIScaleManager::Scale(5.0f), 0, 1.2f);

                    // Círculo colorido com ícone
                    float ic_r = KuroUI::UIScaleManager::Scale(18.0f);
                    float icx = cp.x + KuroUI::UIScaleManager::Scale(28.0f), icy = cp.y + ch * 0.5f;
                    rdl->AddCircleFilled(ImVec2(icx, icy), ic_r, ImGui::GetColorU32(border_col), 20);
                    ImVec2 isz = ImGui::CalcTextSize(icon);
                    rdl->AddText(ImVec2(icx - isz.x * 0.5f, icy - isz.y * 0.5f), IM_COL32(0, 0, 0, 255), icon);

                    // Botão de Download no Extremo Direito (Alinhado verticalmente no centro)
                    float btn_w = KuroUI::UIScaleManager::Scale(240.0f);
                    float btn_h = KuroUI::UIScaleManager::Scale(38.0f);
                    float btn_x = cp.x + cw - btn_w - KuroUI::UIScaleManager::Scale(14.0f);
                    float btn_y = cp.y + (ch - btn_h) * 0.5f;

                    // Bloco de Estrelas & Pureza Espectral (À esquerda do botão)
                    float stars_w = KuroUI::UIScaleManager::Scale(150.0f);
                    float stars_x = btn_x - stars_w - KuroUI::UIScaleManager::Scale(16.0f);
                    float stars_y = cp.y + (ch - KuroUI::UIScaleManager::Scale(38.0f)) * 0.5f;
                    char score_str[32]; snprintf(score_str, sizeof(score_str), "%s %s %s %s %s %d%%",
                        ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, score);
                    rdl->AddText(ImVec2(stars_x, stars_y), IM_COL32(255, 215, 50, 255), score_str);
                    rdl->AddText(ImVec2(stars_x, stars_y + KuroUI::UIScaleManager::Scale(20.0f)), IM_COL32(130, 165, 205, 190), "Pureza Espectral");

                    // Textos do Provedor (Título, Sub1, Sub2)
                    float tx = cp.x + KuroUI::UIScaleManager::Scale(58.0f);
                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + KuroUI::UIScaleManager::Scale(12.0f)));
                    ImGui::PushStyleColor(ImGuiCol_Text, border_col);
                    ImGui::TextUnformatted(name);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + KuroUI::UIScaleManager::Scale(38.0f)));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 0.90f));
                    ImGui::TextUnformatted(sub1);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + KuroUI::UIScaleManager::Scale(64.0f)));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.62f, 0.75f, 0.85f));
                    ImGui::TextUnformatted(sub2);
                    ImGui::PopStyleColor();

                    // Render do Botão
                    ImGui::SetCursorScreenPos(ImVec2(btn_x, btn_y));
                    if (is_solid_btn) {
                        ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn_col.x + 0.15f, btn_col.y + 0.15f, btn_col.z + 0.10f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.08f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.08f, 0.12f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn_col.x * 0.35f, btn_col.y * 0.35f, btn_col.z * 0.35f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Border, btn_col);
                        ImGui::PushStyleColor(ImGuiCol_Text, btn_col);
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, KuroUI::UIScaleManager::Scale(4.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
                    if (ImGui::Button(btn_lbl, ImVec2(btn_w, btn_h))) {
                        if (!selected_source_track_id.empty()) {
                            spotify_service.downloadSourceDirectAsync(selected_source_track_id,
                                selected_source_track_title, selected_source_track_artist, url, 0, selected_source_track_subgenre);
                            toast_message = std::string(ICON_FA_DOWNLOAD) + " Abduzindo " + selected_source_track_title + " [" + name + "]";
                            toast_timer = 4.0f;
                        }
                    }
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(is_solid_btn ? 3 : 4);
                };"""

card_func_new = """                auto render_source_card = [&](const char* icon, const char* name, const char* sub1, const char* sub2,
                                               int score, ImVec4 border_col, bool is_solid_btn, ImVec4 btn_col, const char* btn_lbl,
                                               const std::string& url) {
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = KuroUI::UIScaleManager::Scale(104.0f);
                    ImVec2 cp = ImGui::GetCursorScreenPos();
                    ImGui::Dummy(ImVec2(cw, ch));

                    rdl->AddRectFilled(cp, ImVec2(cp.x + cw, cp.y + ch), IM_COL32(10, 16, 26, 250), KuroUI::UIScaleManager::Scale(6.0f));
                    rdl->AddRect(cp, ImVec2(cp.x + cw, cp.y + ch), ImGui::GetColorU32(border_col), KuroUI::UIScaleManager::Scale(6.0f), 0, 1.4f);

                    // Círculo colorido com ícone oficial da plataforma
                    float ic_r = KuroUI::UIScaleManager::Scale(20.0f);
                    float icx = cp.x + KuroUI::UIScaleManager::Scale(30.0f), icy = cp.y + ch * 0.5f;
                    rdl->AddCircleFilled(ImVec2(icx, icy), ic_r, ImGui::GetColorU32(border_col), 24);
                    ImVec2 isz = ImGui::CalcTextSize(icon);
                    rdl->AddText(ImVec2(icx - isz.x * 0.5f, icy - isz.y * 0.5f), IM_COL32(0, 0, 0, 255), icon);

                    // Botão de Download no Extremo Direito (Alinhado verticalmente no centro)
                    float btn_w = KuroUI::UIScaleManager::Scale(240.0f);
                    float btn_h = KuroUI::UIScaleManager::Scale(38.0f);
                    float btn_x = cp.x + cw - btn_w - KuroUI::UIScaleManager::Scale(14.0f);
                    float btn_y = cp.y + (ch - btn_h) * 0.5f;

                    // Bloco de Estrelas & Pureza Espectral (À esquerda do botão)
                    float stars_w = KuroUI::UIScaleManager::Scale(150.0f);
                    float stars_x = btn_x - stars_w - KuroUI::UIScaleManager::Scale(16.0f);
                    float stars_y = cp.y + (ch - KuroUI::UIScaleManager::Scale(38.0f)) * 0.5f;
                    char score_str[32]; snprintf(score_str, sizeof(score_str), "%s %s %s %s %s %d%%",
                        ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, ICON_FA_STAR, score);
                    rdl->AddText(ImVec2(stars_x, stars_y), IM_COL32(255, 215, 50, 255), score_str);
                    rdl->AddText(ImVec2(stars_x, stars_y + KuroUI::UIScaleManager::Scale(20.0f)), IM_COL32(130, 165, 205, 190), "Pureza Espectral");

                    // Textos do Provedor (Título, Sub1, Sub2) com margens seguras anti-fatiamento
                    float tx = cp.x + KuroUI::UIScaleManager::Scale(62.0f);
                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + KuroUI::UIScaleManager::Scale(12.0f)));
                    ImGui::PushStyleColor(ImGuiCol_Text, border_col);
                    ImGui::TextUnformatted(name);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + KuroUI::UIScaleManager::Scale(38.0f)));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 0.90f));
                    ImGui::TextUnformatted(sub1);
                    ImGui::PopStyleColor();

                    ImGui::SetCursorScreenPos(ImVec2(tx, cp.y + KuroUI::UIScaleManager::Scale(64.0f)));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.62f, 0.75f, 0.85f));
                    ImGui::TextUnformatted(sub2);
                    ImGui::PopStyleColor();

                    // Render do Botão
                    ImGui::SetCursorScreenPos(ImVec2(btn_x, btn_y));
                    if (is_solid_btn) {
                        ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn_col.x + 0.15f, btn_col.y + 0.15f, btn_col.z + 0.10f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.08f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.08f, 0.12f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn_col.x * 0.35f, btn_col.y * 0.35f, btn_col.z * 0.35f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Border, btn_col);
                        ImGui::PushStyleColor(ImGuiCol_Text, btn_col);
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, KuroUI::UIScaleManager::Scale(5.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
                    if (ImGui::Button(btn_lbl, ImVec2(btn_w, btn_h))) {
                        if (!selected_source_track_id.empty()) {
                            spotify_service.downloadSourceDirectAsync(selected_source_track_id,
                                selected_source_track_title, selected_source_track_artist, url, 0, selected_source_track_subgenre);
                            toast_message = std::string(ICON_FA_DOWNLOAD) + " Abduzindo " + selected_source_track_title + " [" + name + "]";
                            toast_timer = 4.0f;
                        }
                    }
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(is_solid_btn ? 3 : 4);

                    // RESTAURAÇÃO CRÍTICA DO CURSOR IMGUI: Garante que o próximo card comece abaixo deste com espaçamento limpo
                    ImGui::SetCursorScreenPos(ImVec2(cp.x, cp.y + ch + KuroUI::UIScaleManager::Scale(8.0f)));
                    ImGui::Dummy(ImVec2(cw, KuroUI::UIScaleManager::Scale(2.0f)));
                };"""

if card_func_old in content:
    content = content.replace(card_func_old, card_func_new, 1)
    print("[7.1] Source Card function updated with clean cursor restoration.")
else:
    print("[7.1] ERROR: Source Card function not found!")
    sys.exit(1)

# Now update the 4 card invocations with registered mark (®)
cards_calls_old = """                // 1. SPOTIFY MASTER SYNC (PROVEDOR OFICIAL) - Solid Green Button (Fiel ao Mockup)
                render_source_card(ICON_FA_COMPACT_DISC, "SPOTIFY MASTER SYNC (PROVEDOR OFICIAL)",
                    "Áudio Master Sincronizado • 320 kbps Vorbis / DSP FLAC • 24-bit / 48kHz",
                    "Metadados oficiais verificados • Capa 600x600 HD • ISRC Sangoma Records",
                    99, ImVec4(0.11f, 0.80f, 0.40f, 1.0f), true, ImVec4(0.11f, 0.80f, 0.40f, 1.0f),
                    ICON_FA_BOLT " ABDUZIR SPOTIFY", sp_url);

                ImGui::Spacing();

                // 2. SOUNDCLOUD HQ LOSSLESS STREAM - Outline Orange (Fiel ao Mockup)
                render_source_card(ICON_FA_CLOUD, "SOUNDCLOUD HQ LOSSLESS STREAM",
                    "Upload Oficial pelo Artista • Web Audio HQ 48.0 kHz",
                    "Versão estendida de palco com transição limpa para mixagem",
                    95, ImVec4(1.00f, 0.45f, 0.05f, 1.0f), false, ImVec4(1.00f, 0.45f, 0.05f, 1.0f),
                    "ABDUZIR SOUNDCLOUD", sc_url);

                ImGui::Spacing();

                // 3. BEATPORT PRO STUDIO MASTER (EXTENDED MIX) - Outline Blue (Fiel ao Mockup)
                render_source_card(ICON_FA_HEADPHONES, "BEATPORT PRO STUDIO MASTER (EXTENDED MIX)",
                    "WAV Master 24-bit / 48kHz • Beatport Catálogo • Dynamic Range 12",
                    "Intro / Outro com compassos alinhados para corte de DJ",
                    100, ImVec4(0.00f, 0.70f, 1.00f, 1.0f), false, ImVec4(0.00f, 0.70f, 1.00f, 1.0f),
                    "ABDUZIR BEATPORT", bp_url);

                ImGui::Spacing();

                // 4. YOUTUBE MUSIC TOPIC // SANGOMA AUDIO - Outline Red (Fiel ao Mockup)
                render_source_card(ICON_FA_PLAY, "YOUTUBE MUSIC TOPIC // SANGOMA AUDIO",
                    "Canal Oficial de Distribuição • Opus HQ 160kbps / 48kHz",
                    "Áudio master direto da gravadora sem cortes ou comerciais",
                    92, ImVec4(1.00f, 0.25f, 0.28f, 1.0f), false, ImVec4(1.00f, 0.25f, 0.28f, 1.0f),
                    "ABDUZIR YOUTUBE", yt_url);"""

cards_calls_new = """                // 1. SPOTIFY® MASTER SYNC (PROVEDOR OFICIAL) - Solid Green Button
                render_source_card(ICON_FA_COMPACT_DISC, "SPOTIFY® MASTER SYNC (PROVEDOR OFICIAL)",
                    "Áudio Master Sincronizado • 320 kbps Vorbis / DSP FLAC • 24-bit / 48kHz",
                    "Metadados oficiais verificados • Capa 600x600 HD • ISRC Sangoma Records",
                    99, ImVec4(0.11f, 0.80f, 0.40f, 1.0f), true, ImVec4(0.11f, 0.80f, 0.40f, 1.0f),
                    ICON_FA_BOLT " ABDUZIR SPOTIFY®", sp_url);

                // 2. SOUNDCLOUD® HQ LOSSLESS STREAM - Outline Orange
                render_source_card(ICON_FA_CLOUD, "SOUNDCLOUD® HQ LOSSLESS STREAM",
                    "Upload Oficial pelo Artista • Web Audio HQ 48.0 kHz",
                    "Versão estendida de palco com transição limpa para mixagem",
                    95, ImVec4(1.00f, 0.45f, 0.05f, 1.0f), false, ImVec4(1.00f, 0.45f, 0.05f, 1.0f),
                    "ABDUZIR SOUNDCLOUD®", sc_url);

                // 3. BEATPORT® PRO STUDIO MASTER (EXTENDED MIX) - Outline Blue
                render_source_card(ICON_FA_HEADPHONES, "BEATPORT® PRO STUDIO MASTER (EXTENDED MIX)",
                    "WAV Master 24-bit / 48kHz • Beatport Catálogo • Dynamic Range 12",
                    "Intro / Outro com compassos alinhados para corte de DJ",
                    100, ImVec4(0.00f, 0.70f, 1.00f, 1.0f), false, ImVec4(0.00f, 0.70f, 1.00f, 1.0f),
                    "ABDUZIR BEATPORT®", bp_url);

                // 4. YOUTUBE MUSIC® TOPIC // SANGOMA AUDIO - Outline Red
                render_source_card(ICON_FA_PLAY, "YOUTUBE MUSIC® TOPIC // SANGOMA AUDIO",
                    "Canal Oficial de Distribuição • Opus HQ 160kbps / 48kHz",
                    "Áudio master direto da gravadora sem cortes ou comerciais",
                    92, ImVec4(1.00f, 0.25f, 0.28f, 1.0f), false, ImVec4(1.00f, 0.25f, 0.28f, 1.0f),
                    "ABDUZIR YOUTUBE®", yt_url);"""

if cards_calls_old in content:
    content = content.replace(cards_calls_old, cards_calls_new, 1)
    print("[7.2] Source Card invocations updated with ® branding.")
else:
    print("[7.2] ERROR: Source Card invocations not found!")
    sys.exit(1)

with open(HEADER_PATH, "w", encoding="utf-8") as f:
    f.write(content)

print("SUCCESS: All UI refinements written to KuroDJStudioUI.h!")
