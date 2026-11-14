#pragma once
#include "imgui.h"
#include "DJLayoutMetrics.h"
#include <cmath>
#include <algorithm>

namespace KuroUI {

class DJBeatPulseEngine {
private:
  DJBeatPulseEngine() = default;

public:
  static DJBeatPulseEngine& getInstance() {
    static DJBeatPulseEngine instance;
    return instance;
  }

  // =========================================================================
  // PALETA CROMÁTICA OFICIAL DO METRÔNOMO DE COMPASSO 4/4
  // =========================================================================
  // Beat 1: Downbeat de Ouro Puro / Solar Flare (Marcação Principal)
  // Beat 2: Ciano Elétrico (Arpejo / Resposta)
  // Beat 3: Âmbar Aquecido (Meio-tempo)
  // Beat 4: Verde Lima Cintilante (Preparação para o próximo compasso)
  static ImU32 getBeatColor(int beat_in_bar, float alpha_mult = 1.0f) {
    float a = std::clamp(alpha_mult, 0.0f, 1.0f);
    switch (beat_in_bar % 4) {
    case 1: // Beat 1 (Downbeat Principal)
      return IM_COL32(255, 215, 0, (int)(255 * a));
    case 2: // Beat 2
      return IM_COL32(0, 230, 255, (int)(255 * a));
    case 3: // Beat 3
      return IM_COL32(255, 145, 25, (int)(255 * a));
    case 0: // Beat 4 (mod 4 == 0)
      return IM_COL32(65, 255, 115, (int)(255 * a));
    default:
      return IM_COL32(255, 255, 255, (int)(255 * a));
    }
  }

  static ImVec4 getBeatColorVec4(int beat_in_bar, float alpha = 1.0f) {
    switch (beat_in_bar % 4) {
    case 1:
      return ImVec4(1.0f, 0.84f, 0.0f, alpha);
    case 2:
      return ImVec4(0.0f, 0.90f, 1.0f, alpha);
    case 3:
      return ImVec4(1.0f, 0.57f, 0.10f, alpha);
    case 0:
      return ImVec4(0.25f, 1.0f, 0.45f, alpha);
    default:
      return ImVec4(1.0f, 1.0f, 1.0f, alpha);
    }
  }

  // =========================================================================
  // RENDERIZAÇÃO DO METRÔNOMO VISUAL NO PLAYHEAD ESTILO TESSAIGA (INUYASHA)
  // - A própria barra respira organicamente com expansão e contração de lâmina viva
  // - Cada beat (2, 3, 4) pulsa, respira e emite ondas de choque/cicatrizes de vento (Kaze no Kizu)
  // - O Beat 1 (Downbeat) SE DESTACA com clarão solar dourado, shockwave duplo e núcleo de diamante!
  // =========================================================================
  void renderPlayheadMetronome(ImDrawList *dl, ImVec2 top_p, float lane_h,
                               int beat_in_bar, double beat_phase,
                               bool is_playing) {
    if (!dl) return;
    auto &metrics = DJLayoutMetrics::getInstance();
    if (!metrics.enable_visual_beat_metronome)
      return;

    float t = (float)ImGui::GetTime();

    // 1. Respiração Orgânica Viva da Lâmina ("como se a própria barra respirasse")
    // Ciclo contínuo de inalação e exalação biológica
    float idle_sin = std::sin(t * 3.6f);
    float idle_norm = (idle_sin + 1.0f) * 0.5f; // [0.0 .. 1.0]

    // Pulso exponencial de impacto na batida (rápido ataque e decaimento musical)
    float pulse = 0.0f;
    if (is_playing && metrics.enable_visual_animations) {
      double ph = std::clamp(beat_phase, 0.0, 1.0);
      pulse = (float)std::exp(-ph * 4.4);
    }

    ImVec2 bot_p = ImVec2(top_p.x, top_p.y + lane_h);
    float mid_y = top_p.y + lane_h * 0.5f;
    bool is_downbeat = ((beat_in_bar % 4) == 1);

    // 2. Modulação Orgânica da Largura da Lâmina (A lâmina infla e desinfla com a respiração)
    float blade_breath_w = 2.0f + 2.8f * idle_norm + (is_downbeat ? 14.0f * pulse : 5.0f * pulse);
    float miasma_outer_w = blade_breath_w * (is_downbeat ? 2.6f : 1.9f);

    int miasma_alpha = (int)(25 + 35 * idle_norm + (is_downbeat ? 160 * pulse : 75 * pulse));
    miasma_alpha = std::clamp(miasma_alpha, 15, 230);

    ImU32 beat_col_core = is_downbeat ? IM_COL32(255, 215, 0, 255) : getBeatColor(beat_in_bar, 1.0f);
    ImU32 miasma_outer_col = is_downbeat ? IM_COL32(255, 185, 20, miasma_alpha / 3)
                                         : getBeatColor(beat_in_bar, (float)(miasma_alpha / 3) / 255.0f);
    ImU32 miasma_mid_col   = is_downbeat ? IM_COL32(255, 220, 50, miasma_alpha)
                                         : getBeatColor(beat_in_bar, (float)miasma_alpha / 255.0f);

    // Camada 1: Miasma Demoníaco Eflúvio Externo Difuso (Aura de respiração ampla)
    dl->AddRectFilled(ImVec2(top_p.x - miasma_outer_w, top_p.y),
                      ImVec2(top_p.x + miasma_outer_w, bot_p.y),
                      miasma_outer_col, miasma_outer_w * 0.4f);

    // Camada 2: Bainha de Energia Viva Contida (Respiração intermediária)
    dl->AddRectFilled(ImVec2(top_p.x - blade_breath_w, top_p.y),
                      ImVec2(top_p.x + blade_breath_w, bot_p.y),
                      miasma_mid_col, blade_breath_w * 0.45f);

    // 3. EFEITO BEAT 1 (DOWNBEAT DESTACADO: DESPERTAR DA TESSAIGA / BAKURYUHA)
    if (is_downbeat && is_playing && pulse > 0.01f) {
      // Onda de Choque Dupla Circular Expansiva Tessaiga (Shockwave Blast)
      float blast_radius1 = (lane_h * 0.22f) * (0.5f + pulse * 1.5f);
      float blast_radius2 = (lane_h * 0.12f) * (0.4f + pulse * 1.2f);
      ImU32 blast_ring_col1 = IM_COL32(255, 225, 60, (int)(210 * pulse));
      ImU32 blast_ring_col2 = IM_COL32(255, 255, 180, (int)(250 * pulse));
      ImU32 blast_core_col  = IM_COL32(255, 255, 250, (int)(245 * pulse));

      // Anéis de choque de energia sagrada/demoníaca
      dl->AddCircle(ImVec2(top_p.x, mid_y), blast_radius1, blast_ring_col1, 32, 2.2f);
      dl->AddCircle(ImVec2(top_p.x, mid_y), blast_radius2, blast_ring_col2, 24, 1.8f);
      dl->AddCircleFilled(ImVec2(top_p.x, mid_y), blast_radius2 * 0.45f, blast_core_col, 20);

      // Cicatrizes de Vento (Kaze no Kizu): Feixes horizontais cortando lateralmente a onda
      float scar_w = (lane_h * 0.60f) * pulse;
      dl->AddLine(ImVec2(top_p.x - scar_w, mid_y), ImVec2(top_p.x + scar_w, mid_y),
                  IM_COL32(255, 245, 160, (int)(220 * pulse)), 2.5f);

      // Coluna vertical de plasma luminoso puro (Espada Incandescente)
      float beam_w = 4.0f + 3.0f * pulse;
      dl->AddLine(top_p, bot_p, IM_COL32(255, 255, 255, (int)(255 * pulse)), beam_w);

      // Orbes solares de impacto nos vértices do Playhead (topo e base)
      float orb_r = 6.0f + 6.0f * pulse;
      dl->AddCircleFilled(top_p, orb_r, IM_COL32(255, 240, 80, (int)(230 * pulse)), 16);
      dl->AddCircleFilled(bot_p, orb_r, IM_COL32(255, 240, 80, (int)(230 * pulse)), 16);
      dl->AddCircleFilled(top_p, orb_r * 0.5f, IM_COL32(255, 255, 255, (int)(255 * pulse)), 12);
      dl->AddCircleFilled(bot_p, orb_r * 0.5f, IM_COL32(255, 255, 255, (int)(255 * pulse)), 12);
    } else if (is_playing && pulse > 0.01f) {
      // 4. BEATS 2, 3 E 4: Pulso orgânico nítido com a cor do compasso e onda lateral
      float scar_w = (lane_h * 0.28f) * pulse;
      ImU32 p_col = getBeatColor(beat_in_bar, 0.70f * pulse);
      ImU32 p_core = getBeatColor(beat_in_bar, 0.95f * pulse);

      // Feixe lateral Kaze no Kizu sutil da cor do tempo
      dl->AddLine(ImVec2(top_p.x - scar_w, mid_y), ImVec2(top_p.x + scar_w, mid_y), p_col, 1.8f);

      // Orbe pulsante central
      dl->AddCircleFilled(ImVec2(top_p.x, mid_y), 4.5f + 7.0f * pulse, p_core, 16);
      dl->AddCircle(ImVec2(top_p.x, mid_y), 7.0f + 10.0f * pulse, p_col, 20, 1.5f);
    }

    // 5. Agulha Central Playhead (Lâmina de Diamante com Espinha Incandescente)
    float core_w = is_downbeat ? (2.4f + 2.0f * pulse) : (1.8f + 0.8f * pulse);
    ImU32 core_col = is_downbeat ? IM_COL32(255, 255, 245, 255) : IM_COL32(255, 255, 255, 240);
    dl->AddLine(top_p, bot_p, core_col, core_w);
  }

  // =========================================================================
  // INDICADOR METRÔNOMO NUMÉRICO VISUAL (1, 2, 3, 4) NO CABEÇALHO DO DECK
  // =========================================================================
  void renderBeatDigitCapsule(ImDrawList *dl, ImVec2 pos, int beat_in_bar,
                              double beat_phase, bool is_playing) {
    if (!dl) return;
    auto &metrics = DJLayoutMetrics::getInstance();
    if (!metrics.enable_visual_beat_metronome)
      return;

    float pulse = 0.0f;
    if (is_playing && metrics.enable_visual_animations) {
      pulse = (float)std::exp(-std::clamp(beat_phase, 0.0, 1.0) * 4.5);
    }

    // 4 Cápsulas circulares alinhadas horizontalmente representando os beats 1, 2, 3, 4
    float r = 5.0f;
    float gap = 4.0f;

    for (int b = 1; b <= 4; b++) {
      ImVec2 cp = ImVec2(pos.x + (b - 1) * (r * 2.0f + gap) + r, pos.y + r);
      bool is_active_beat = ((beat_in_bar % 4) == (b % 4));

      if (is_active_beat && is_playing) {
        float active_r = r * (1.0f + pulse * 0.35f);
        ImU32 fill_col = getBeatColor(b, 0.90f);
        dl->AddCircleFilled(cp, active_r, fill_col, 16);
        if (pulse > 0.05f && metrics.enable_visual_animations) {
          dl->AddCircle(cp, active_r + 2.5f * pulse,
                        getBeatColor(b, 0.50f * pulse), 16, 1.5f);
        }
      } else {
        dl->AddCircleFilled(cp, r, IM_COL32(25, 32, 45, 180), 12);
        dl->AddCircle(cp, r, IM_COL32(45, 58, 80, 200), 12, 1.0f);
      }
    }
  }
};

} // namespace KuroUI
