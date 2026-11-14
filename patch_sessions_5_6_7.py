import sys

with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

idx_w1 = content.find('void renderStackedRekordboxWaveforms(KuroAudio::DJEngine& engine, float w, float h) {')
idx_w2 = content.find('void renderUSBExportDrawer(KuroAudio::DJEngine& engine) {')

if idx_w1 == -1 or idx_w2 == -1:
    print("Error: Could not locate function bounds!")
    sys.exit(1)

new_waveform_func = '''        void renderStackedRekordboxWaveforms(KuroAudio::DJEngine& engine, float w, float h) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            float lane_gap = 2.0f; // Divisor Rekordbox
            float min_lane = KuroUI::UIScaleManager::Scale(78.0f);
            float lane_h   = std::max(min_lane, (h - lane_gap) * 0.5f);
            float total_stack_h = lane_h * 2.0f + lane_gap;

            // Botoes de Controle / Zoom Verticais na lateral esquerda (^, v, +, RST, -, 1L/2L)
            float ctrl_w = 26.0f;
            float wave_x = pos.x + ctrl_w;
            float wave_w = w - ctrl_w;
            float center_x = wave_x + wave_w * 0.5f;

            dl->AddRectFilled(pos, ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(12, 15, 20, 255));
            dl->AddLine(ImVec2(pos.x + ctrl_w, pos.y), ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(24, 30, 42, 255), 1.0f);

            auto zoomSideBtn = [&](const char* id, const char* lbl, float y_off, auto on_click) {
                ImVec2 bp = ImVec2(pos.x + 2.0f, pos.y + y_off);
                dl->AddRectFilled(bp, ImVec2(bp.x + 22.0f, bp.y + 16.0f), IM_COL32(20, 25, 34, 255), 2.0f);
                dl->AddRect(bp, ImVec2(bp.x + 22.0f, bp.y + 16.0f), IM_COL32(38, 48, 65, 200), 2.0f, 0, 1.0f);
                ImVec2 sz = ImGui::CalcTextSize(lbl);
                dl->AddText(ImVec2(bp.x + (22.0f - sz.x) * 0.5f, bp.y + 1.0f), IM_COL32(190, 205, 225, 240), lbl);
                ImGui::SetCursorScreenPos(bp);
                if (ImGui::InvisibleButton(id, ImVec2(22.0f, 16.0f))) on_click();
            };

            zoomSideBtn("##WfSideUp", "^", 4.0f, [&]() {
                if (waveform_zoom_bars > 1) waveform_zoom_bars /= 2;
            });
            zoomSideBtn("##WfSideDn", "v", 24.0f, [&]() {
                if (waveform_zoom_bars < 64) waveform_zoom_bars *= 2;
            });
            zoomSideBtn("##WfSidePlus", "+", 46.0f, [&]() {
                wf_height_user_adjusted = true;
                waveform_height = std::clamp(waveform_height + 24.0f, 130.0f, 380.0f);
            });
            zoomSideBtn("##WfSideRst", "<", 66.0f, [&]() {
                waveform_zoom_bars = 8;
                wf_height_user_adjusted = false;
                waveform_height = 200.0f;
            });
            zoomSideBtn("##WfSideMinus", "-", 86.0f, [&]() {
                wf_height_user_adjusted = true;
                waveform_height = std::clamp(waveform_height - 24.0f, 130.0f, 380.0f);
            });
            // Alternador de Modo: 1L (Pista Unica Sobreposta) vs 2L (Pistas Empilhadas)
            zoomSideBtn("##WfSideMode", waveform_view_mode == 0 ? "1L" : "2L", 108.0f, [&]() {
                waveform_view_mode = (waveform_view_mode == 0 ? 1 : 0);
            });
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar visualizacao: 1L = Pista Unica Sobreposta Espectral | 2L = Modo Empilhado");

            int active_l_id = ddj_hardware.getActiveLeftDeck();
            int active_r_id = ddj_hardware.getActiveRightDeck();
            KuroAudio::DJDeck& deck1 = engine.decks[active_l_id];
            KuroAudio::DJDeck& deck2 = engine.decks[active_r_id];

            float dt = ImGui::GetIO().DeltaTime;
            if (dt <= 0.0001f || dt > 0.1f) dt = 1.0f / 60.0f;

            // Sincronizacao de Hertz para ambos os Decks
            DeckVisualHertzSync& vs1 = vis_sync[deck1.deck_id];
            if (!vs1.active) { vs1.smooth_frame = deck1.current_frame; vs1.active = true; }
            if (deck1.is_playing.load() && !deck1.jog_touch) {
                double eff_speed1 = (1.0 + deck1.pitch_percent * 0.01);
                vs1.smooth_frame += eff_speed1 * deck1.sample_rate * (double)dt;
                double diff1 = deck1.current_frame - vs1.smooth_frame;
                if (std::abs(diff1) > (deck1.sample_rate * 0.20)) vs1.smooth_frame = deck1.current_frame;
                else vs1.smooth_frame += diff1 * std::clamp((double)dt * 14.0, 0.02, 0.35);
            } else {
                vs1.smooth_frame = deck1.current_frame;
            }

            DeckVisualHertzSync& vs2 = vis_sync[deck2.deck_id];
            if (!vs2.active) { vs2.smooth_frame = deck2.current_frame; vs2.active = true; }
            if (deck2.is_playing.load() && !deck2.jog_touch) {
                double eff_speed2 = (1.0 + deck2.pitch_percent * 0.01);
                vs2.smooth_frame += eff_speed2 * deck2.sample_rate * (double)dt;
                double diff2 = deck2.current_frame - vs2.smooth_frame;
                if (std::abs(diff2) > (deck2.sample_rate * 0.20)) vs2.smooth_frame = deck2.current_frame;
                else vs2.smooth_frame += diff2 * std::clamp((double)dt * 14.0, 0.02, 0.35);
            } else {
                vs2.smooth_frame = deck2.current_frame;
            }

            // =========================================================================================
            // MODO 0: MOTOR DE WAVEFORM EM PISTA ÚNICA SOBREPOSTA (SESSÕES 5, 6, 7)
            // =========================================================================================
            if (waveform_view_mode == 0) {
                float uni_lane_h = total_stack_h;
                ImVec2 uni_pos = ImVec2(wave_x, pos.y);
                float mid_y = uni_pos.y + uni_lane_h * 0.5f;
                float max_half_h = (uni_lane_h * 0.5f) - 6.0f;

                // 1. Fundo Preto Puro Obsidian Club Standard
                dl->AddRectFilled(uni_pos, ImVec2(uni_pos.x + wave_w, uni_pos.y + uni_lane_h), IM_COL32(0, 0, 0, 255));
                dl->AddRect(uni_pos, ImVec2(uni_pos.x + wave_w, uni_pos.y + uni_lane_h), IM_COL32(20, 26, 38, 255), 0.0f, 0, 1.0f);

                // Linha de centro neutra (1px ultrafina)
                dl->AddLine(ImVec2(uni_pos.x, mid_y), ImVec2(uni_pos.x + wave_w, mid_y), IM_COL32(28, 38, 52, 160), 1.0f);

                // Referencia de tempo compartilhada (baseada no BPM principal)
                double ref_bpm = (deck1.is_playing.load() && deck1.bpm > 20.0) ? deck1.bpm :
                                 ((deck2.is_playing.load() && deck2.bpm > 20.0) ? deck2.bpm :
                                 ((deck1.bpm > 20.0) ? deck1.bpm : 126.0));
                double seconds_per_bar = (60.0 / ref_bpm) * 4.0;
                double window_sec = (double)waveform_zoom_bars * seconds_per_bar;
                if (window_sec < 0.5) window_sec = 0.5;

                double window_f1 = window_sec * deck1.sample_rate;
                double start_f1  = vs1.smooth_frame - window_f1 * 0.5;
                double window_f2 = window_sec * deck2.sample_rate;
                double start_f2  = vs2.smooth_frame - window_f2 * 0.5;

                // --- 2. GRADE MESTRE ÚNICA (SESSÃO 7) ---
                // Beatgrid Primária (Deck 1 / Master): atravessa a pista inteira de cima a baixo
                if (deck1.bpm > 20.0 && deck1.sample_rate > 1000.0) {
                    double natural_beat_len1 = (60.0 / deck1.bpm) * deck1.sample_rate;
                    if (natural_beat_len1 > 10.0) {
                        int b_start1 = (int)std::floor((start_f1 - deck1.first_beat_frame) / natural_beat_len1);
                        int b_end1   = (int)std::ceil((start_f1 + window_f1 - deck1.first_beat_frame) / natural_beat_len1);
                        for (int b = b_start1; b <= b_end1; b++) {
                            double beat_f = deck1.first_beat_frame + (double)b * natural_beat_len1;
                            float bx = uni_pos.x + (float)((beat_f - start_f1) / window_f1) * wave_w;
                            if (bx >= uni_pos.x && bx <= uni_pos.x + wave_w) {
                                if (b % 4 == 0) {
                                    // Downbeat 1.1: Marcador Vermelho Rekordbox com tag numérica superior
                                    dl->AddLine(ImVec2(bx, uni_pos.y), ImVec2(bx, uni_pos.y + uni_lane_h), IM_COL32(255, 30, 50, 220), 1.5f);
                                    char b_tag[16];
                                    snprintf(b_tag, sizeof(b_tag), "%d.1", (b / 4) + 1);
                                    dl->AddRectFilled(ImVec2(bx - 9.0f, uni_pos.y + 1.0f), ImVec2(bx + 9.0f, uni_pos.y + 12.0f), IM_COL32(180, 15, 30, 230), 2.0f);
                                    dl->AddText(ImVec2(bx - 7.5f, uni_pos.y + 0.5f), IM_COL32(255, 255, 255, 255), b_tag);
                                } else {
                                    // Batidas intermediárias 2, 3, 4: linha vertical sutil
                                    dl->AddLine(ImVec2(bx, uni_pos.y), ImVec2(bx, uni_pos.y + uni_lane_h), IM_COL32(140, 160, 190, 45), 1.0f);
                                    dl->AddLine(ImVec2(bx, uni_pos.y), ImVec2(bx, uni_pos.y + 4.0f), IM_COL32(180, 205, 235, 160), 1.0f);
                                    dl->AddLine(ImVec2(bx, uni_pos.y + uni_lane_h - 4.0f), ImVec2(bx, uni_pos.y + uni_lane_h), IM_COL32(180, 205, 235, 160), 1.0f);
                                }
                            }
                        }
                    }
                }

                // Beatgrid Secundária (Deck 2): Marcadores e ticks em Âmbar na borda inferior da pista
                if (deck2.bpm > 20.0 && deck2.sample_rate > 1000.0) {
                    double natural_beat_len2 = (60.0 / deck2.bpm) * deck2.sample_rate;
                    if (natural_beat_len2 > 10.0) {
                        int b_start2 = (int)std::floor((start_f2 - deck2.first_beat_frame) / natural_beat_len2);
                        int b_end2   = (int)std::ceil((start_f2 + window_f2 - deck2.first_beat_frame) / natural_beat_len2);
                        for (int b = b_start2; b <= b_end2; b++) {
                            double beat_f2 = deck2.first_beat_frame + (double)b * natural_beat_len2;
                            float bx2 = uni_pos.x + (float)((beat_f2 - start_f2) / window_f2) * wave_w;
                            if (bx2 >= uni_pos.x && bx2 <= uni_pos.x + wave_w) {
                                if (b % 4 == 0) {
                                    // Downbeat Deck 2: Tick Âmbar/Dourado com tag inferior
                                    dl->AddLine(ImVec2(bx2, uni_pos.y + uni_lane_h - 14.0f), ImVec2(bx2, uni_pos.y + uni_lane_h), IM_COL32(255, 160, 0, 240), 1.8f);
                                    char b2_tag[16];
                                    snprintf(b2_tag, sizeof(b2_tag), "%d.1", (b / 4) + 1);
                                    dl->AddRectFilled(ImVec2(bx2 - 9.0f, uni_pos.y + uni_lane_h - 13.0f), ImVec2(bx2 + 9.0f, uni_pos.y + uni_lane_h - 2.0f), IM_COL32(160, 90, 0, 230), 2.0f);
                                    dl->AddText(ImVec2(bx2 - 7.5f, uni_pos.y + uni_lane_h - 13.5f), IM_COL32(255, 235, 160, 255), b2_tag);
                                } else {
                                    // Ticks intermediários Deck 2
                                    dl->AddLine(ImVec2(bx2, uni_pos.y + uni_lane_h - 5.0f), ImVec2(bx2, uni_pos.y + uni_lane_h), IM_COL32(255, 170, 0, 150), 1.2f);
                                }
                            }
                        }
                    }
                }

                // --- 3. RENDERIZAÇÃO DA WAVEFORM SOBREPOSTA ESPECTRAL COM REATIVIDADE DE EQ (SESSÕES 5 & 6) ---
                deck1.generateDetailedWaveformIfNeeded();
                deck2.generateDetailedWaveformIfNeeded();

                // Fatores de ganho de EQ em tempo real diretamente dos Knobs da controladora
                float eq_l1 = std::clamp(deck1.eq_low,  0.0f, 2.5f);
                float eq_m1 = std::clamp(deck1.eq_mid,  0.0f, 2.5f);
                float eq_h1 = std::clamp(deck1.eq_high, 0.0f, 2.5f);

                float eq_l2 = std::clamp(deck2.eq_low,  0.0f, 2.5f);
                float eq_m2 = std::clamp(deck2.eq_mid,  0.0f, 2.5f);
                float eq_h2 = std::clamp(deck2.eq_high, 0.0f, 2.5f);

                float col_step = 1.0f;
                int num_cols = std::max(1, (int)(wave_w / col_step));
                double frames_per_col1 = window_f1 / (double)num_cols;
                double frames_per_col2 = window_f2 / (double)num_cols;
                size_t det_sz1 = deck1.detailed_waveform.size();
                size_t det_sz2 = deck2.detailed_waveform.size();
                size_t tot_f1  = deck1.buffer_l.size();
                size_t tot_f2  = deck2.buffer_l.size();

                for (int x = 0; x < num_cols; x++) {
                    float cur_x = uni_pos.x + (float)x * col_step;

                    // Amostragem Deck 1 (Ciano Elétrico)
                    float amp1 = 0.0f, core_amp1 = 0.0f;
                    ImU32 col_outer1 = 0, col_core1 = 0;
                    double f_cur1 = start_f1 + (double)x * frames_per_col1;
                    if (f_cur1 >= 0.0 && f_cur1 < (double)tot_f1 && det_sz1 > 0) {
                        double bin1 = f_cur1 / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP;
                        size_t b0_1 = (size_t)std::clamp(std::floor(bin1), 0.0, (double)(det_sz1 - 1));
                        size_t b1_1 = std::min(b0_1 + 1, det_sz1 - 1);
                        float frac1 = (float)(bin1 - (double)b0_1);
                        float st1 = frac1 * frac1 * (3.0f - 2.0f * frac1);

                        const auto& pt0_1 = deck1.detailed_waveform[b0_1];
                        const auto& pt1_1 = deck1.detailed_waveform[b1_1];

                        float l1 = (pt0_1.low  + (pt1_1.low  - pt0_1.low)  * st1) * eq_l1;
                        float m1 = (pt0_1.mid  + (pt1_1.mid  - pt0_1.mid)  * st1) * eq_m1;
                        float h1 = (pt0_1.high + (pt1_1.high - pt0_1.high) * st1) * eq_h1;

                        float eff_p1 = (l1 * 0.50f + m1 * 0.35f + h1 * 0.25f);
                        if (eff_p1 > 0.002f) {
                            float norm1 = std::clamp((eff_p1 - 0.002f) / (1.0f - 0.002f), 0.0f, 1.0f);
                            amp1 = std::clamp(std::pow(norm1, 1.02f) * max_half_h, 0.8f, max_half_h);
                            core_amp1 = amp1 * (0.42f * std::clamp(eq_l1, 0.15f, 1.6f));

                            int cr1 = (int)std::clamp(l1 * 30.0f + m1 * 50.0f + h1 * 170.0f, 0.0f, 180.0f);
                            int cg1 = (int)std::clamp(170.0f + m1 * 80.0f, 120.0f, 255.0f);
                            int cb1 = (int)std::clamp(230.0f + h1 * 25.0f, 190.0f, 255.0f);
                            if (eq_l1 < 0.15f) { cr1 = 15; cg1 = 205; cb1 = 255; }
                            col_outer1 = IM_COL32(cr1, cg1, cb1, 210);
                            col_core1  = IM_COL32(200, 255, 255, 255);
                        }
                    }

                    // Amostragem Deck 2 (Âmbar / Dourado)
                    float amp2 = 0.0f, core_amp2 = 0.0f;
                    ImU32 col_outer2 = 0, col_core2 = 0;
                    double f_cur2 = start_f2 + (double)x * frames_per_col2;
                    if (f_cur2 >= 0.0 && f_cur2 < (double)tot_f2 && det_sz2 > 0) {
                        double bin2 = f_cur2 / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP;
                        size_t b0_2 = (size_t)std::clamp(std::floor(bin2), 0.0, (double)(det_sz2 - 1));
                        size_t b1_2 = std::min(b0_2 + 1, det_sz2 - 1);
                        float frac2 = (float)(bin2 - (double)b0_2);
                        float st2 = frac2 * frac2 * (3.0f - 2.0f * frac2);

                        const auto& pt0_2 = deck2.detailed_waveform[b0_2];
                        const auto& pt1_2 = deck2.detailed_waveform[b1_2];

                        float l2 = (pt0_2.low  + (pt1_2.low  - pt0_2.low)  * st2) * eq_l2;
                        float m2 = (pt0_2.mid  + (pt1_2.mid  - pt0_2.mid)  * st2) * eq_m2;
                        float h2 = (pt0_2.high + (pt1_2.high - pt0_2.high) * st2) * eq_h2;

                        float eff_p2 = (l2 * 0.50f + m2 * 0.35f + h2 * 0.25f);
                        if (eff_p2 > 0.002f) {
                            float norm2 = std::clamp((eff_p2 - 0.002f) / (1.0f - 0.002f), 0.0f, 1.0f);
                            amp2 = std::clamp(std::pow(norm2, 1.02f) * max_half_h, 0.8f, max_half_h);
                            core_amp2 = amp2 * (0.42f * std::clamp(eq_l2, 0.15f, 1.6f));

                            int cr2 = (int)std::clamp(225.0f + l2 * 30.0f, 190.0f, 255.0f);
                            int cg2 = (int)std::clamp(75.0f + m2 * 110.0f, 40.0f, 220.0f);
                            int cb2 = (int)std::clamp(15.0f + h2 * 100.0f, 0.0f, 120.0f);
                            if (eq_l2 < 0.15f) { cr2 = 255; cg2 = 180; cb2 = 40; }
                            col_outer2 = IM_COL32(cr2, cg2, cb2, 210);
                            col_core2  = IM_COL32(255, 245, 185, 255);
                        }
                    }

                    // HIERARQUIA DE AMPLITUDE E SOBREPOSIÇÃO INTELIGENTE
                    if (amp1 < 0.8f && amp2 < 0.8f) {
                        dl->AddLine(ImVec2(cur_x, mid_y), ImVec2(cur_x + 1.0f, mid_y), IM_COL32(28, 38, 52, 140), 1.0f);
                    } else if (amp1 >= 0.8f && amp2 < 0.8f) {
                        // Somente Deck 1 com som (Ciano)
                        dl->AddLine(ImVec2(cur_x, mid_y - amp1), ImVec2(cur_x, mid_y + amp1), col_outer1, 1.0f);
                        if (core_amp1 >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp1), ImVec2(cur_x, mid_y + core_amp1), col_core1, 1.0f);
                    } else if (amp2 >= 0.8f && amp1 < 0.8f) {
                        // Somente Deck 2 com som (Âmbar)
                        dl->AddLine(ImVec2(cur_x, mid_y - amp2), ImVec2(cur_x, mid_y + amp2), col_outer2, 1.0f);
                        if (core_amp2 >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp2), ImVec2(cur_x, mid_y + core_amp2), col_core2, 1.0f);
                    } else {
                        // AMBOS OS DECKS COM SOM SIMULTÂNEO: A MAIOR ENVOLVE, A MENOR FICA SOBREPOSTA INTERIORMENTE
                        if (amp1 >= amp2) {
                            // Deck 1 (Ciano) é dominante: Envelope Externo
                            dl->AddLine(ImVec2(cur_x, mid_y - amp1), ImVec2(cur_x, mid_y + amp1), col_outer1, 1.0f);
                            // Deck 2 (Âmbar) sobreposto internamente com alta visibilidade
                            dl->AddLine(ImVec2(cur_x, mid_y - amp2), ImVec2(cur_x, mid_y + amp2), IM_COL32(255, 150, 20, 255), 1.0f);
                            // Núcleos brilhantes
                            if (core_amp1 >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp1), ImVec2(cur_x, mid_y + core_amp1), col_core1, 1.0f);
                            if (core_amp2 >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp2), ImVec2(cur_x, mid_y + core_amp2), col_core2, 1.0f);
                        } else {
                            // Deck 2 (Âmbar) é dominante: Envelope Externo
                            dl->AddLine(ImVec2(cur_x, mid_y - amp2), ImVec2(cur_x, mid_y + amp2), col_outer2, 1.0f);
                            // Deck 1 (Ciano) sobreposto internamente com alta visibilidade
                            dl->AddLine(ImVec2(cur_x, mid_y - amp1), ImVec2(cur_x, mid_y + amp1), IM_COL32(0, 230, 255, 255), 1.0f);
                            // Núcleos brilhantes
                            if (core_amp2 >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp2), ImVec2(cur_x, mid_y + core_amp2), col_core2, 1.0f);
                            if (core_amp1 >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp1), ImVec2(cur_x, mid_y + core_amp1), col_core1, 1.0f);
                        }
                    }
                }

                // CUE Points Deck 1 (Topo da pista)
                if (deck1.duration_sec > 0.0 && deck1.cue_frame >= 0.0) {
                    float cx_cue1 = uni_pos.x + (float)((deck1.cue_frame - start_f1) / window_f1) * wave_w;
                    if (cx_cue1 >= uni_pos.x && cx_cue1 <= uni_pos.x + wave_w) {
                        ImU32 c_cue = IM_COL32(0, 229, 255, 255);
                        dl->AddLine(ImVec2(cx_cue1, uni_pos.y), ImVec2(cx_cue1, mid_y), c_cue, 1.2f);
                        dl->AddRectFilled(ImVec2(cx_cue1 - 10.0f, uni_pos.y + 1.0f), ImVec2(cx_cue1 + 10.0f, uni_pos.y + 13.0f), c_cue, 2.0f);
                        dl->AddText(ImVec2(cx_cue1 - 8.0f, uni_pos.y + 0.5f), IM_COL32(0, 0, 0, 255), "CUE1");
                    }
                }

                // CUE Points Deck 2 (Base da pista)
                if (deck2.duration_sec > 0.0 && deck2.cue_frame >= 0.0) {
                    float cx_cue2 = uni_pos.x + (float)((deck2.cue_frame - start_f2) / window_f2) * wave_w;
                    if (cx_cue2 >= uni_pos.x && cx_cue2 <= uni_pos.x + wave_w) {
                        ImU32 c_cue2 = IM_COL32(255, 160, 0, 255);
                        dl->AddLine(ImVec2(cx_cue2, mid_y), ImVec2(cx_cue2, uni_pos.y + uni_lane_h), c_cue2, 1.2f);
                        dl->AddRectFilled(ImVec2(cx_cue2 - 10.0f, uni_pos.y + uni_lane_h - 13.0f), ImVec2(cx_cue2 + 10.0f, uni_pos.y + uni_lane_h - 1.0f), c_cue2, 2.0f);
                        dl->AddText(ImVec2(cx_cue2 - 8.0f, uni_pos.y + uni_lane_h - 13.5f), IM_COL32(0, 0, 0, 255), "CUE2");
                    }
                }

                // Hot Cues Deck 1 & 2
                const char* hot_cue_letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };
                for (int c = 0; c < 8; c++) {
                    if (deck1.hot_cues[c].active) {
                        double cf1 = deck1.hot_cues[c].time_sec * deck1.sample_rate;
                        float cx1 = uni_pos.x + (float)((cf1 - start_f1) / window_f1) * wave_w;
                        if (cx1 >= uni_pos.x && cx1 <= uni_pos.x + wave_w) {
                            ImU32 hc_col = deck1.hot_cues[c].color_rgba;
                            dl->AddLine(ImVec2(cx1, uni_pos.y), ImVec2(cx1, mid_y), hc_col, 1.0f);
                            dl->AddRectFilled(ImVec2(cx1 - 7.0f, uni_pos.y + 1.0f), ImVec2(cx1 + 7.0f, uni_pos.y + 13.0f), hc_col, 2.0f);
                            dl->AddText(ImVec2(cx1 - 4.0f, uni_pos.y + 0.5f), IM_COL32(0, 0, 0, 255), hot_cue_letters[c]);
                        }
                    }
                    if (deck2.hot_cues[c].active) {
                        double cf2 = deck2.hot_cues[c].time_sec * deck2.sample_rate;
                        float cx2 = uni_pos.x + (float)((cf2 - start_f2) / window_f2) * wave_w;
                        if (cx2 >= uni_pos.x && cx2 <= uni_pos.x + wave_w) {
                            ImU32 hc_col2 = deck2.hot_cues[c].color_rgba;
                            dl->AddLine(ImVec2(cx2, mid_y), ImVec2(cx2, uni_pos.y + uni_lane_h), hc_col2, 1.0f);
                            dl->AddRectFilled(ImVec2(cx2 - 7.0f, uni_pos.y + uni_lane_h - 13.0f), ImVec2(cx2 + 7.0f, uni_pos.y + uni_lane_h - 1.0f), hc_col2, 2.0f);
                            dl->AddText(ImVec2(cx2 - 4.0f, uni_pos.y + uni_lane_h - 13.5f), IM_COL32(0, 0, 0, 255), hot_cue_letters[c]);
                        }
                    }
                }

                // --- 4. AGULHA CENTRAL PLAYHEAD ÚNICA COM TELEMETRIA ---
                dl->AddLine(ImVec2(center_x, uni_pos.y), ImVec2(center_x, uni_pos.y + uni_lane_h), IM_COL32(255, 255, 255, 255), 1.0f);
                dl->AddLine(ImVec2(center_x, uni_pos.y), ImVec2(center_x, uni_pos.y + uni_lane_h), IM_COL32(0, 220, 255, 60), 3.0f);

                // Indicador Superior Ciano (Deck 1) apontando para baixo
                dl->AddTriangleFilled(ImVec2(center_x - 4.0f, uni_pos.y), ImVec2(center_x + 4.0f, uni_pos.y), ImVec2(center_x, uni_pos.y + 6.5f), IM_COL32(0, 245, 212, 255));
                // Indicador Inferior Âmbar (Deck 2) apontando para cima
                dl->AddTriangleFilled(ImVec2(center_x - 4.0f, uni_pos.y + uni_lane_h), ImVec2(center_x + 4.0f, uni_pos.y + uni_lane_h), ImVec2(center_x, uni_pos.y + uni_lane_h - 6.5f), IM_COL32(255, 170, 0, 255));

                // Contador de compassos / Telemetria bilateral da agulha
                if (deck1.bpm > 20.0 && deck1.sample_rate > 1000.0) {
                    double beat_len1 = (60.0 / deck1.bpm) * deck1.sample_rate;
                    double bars1 = ((deck1.current_frame - deck1.first_beat_frame) / beat_len1) / 4.0;
                    char buf1[32]; snprintf(buf1, sizeof(buf1), "D1 %.1fBars", bars1);
                    ImVec2 sz1 = ImGui::CalcTextSize(buf1);
                    dl->AddText(ImVec2(center_x - sz1.x - 6.0f, uni_pos.y + 4.0f), IM_COL32(0, 235, 255, 255), buf1);
                }
                if (deck2.bpm > 20.0 && deck2.sample_rate > 1000.0) {
                    double beat_len2 = (60.0 / deck2.bpm) * deck2.sample_rate;
                    double bars2 = ((deck2.current_frame - deck2.first_beat_frame) / beat_len2) / 4.0;
                    char buf2[32]; snprintf(buf2, sizeof(buf2), "D2 %.1fBars", bars2);
                    dl->AddText(ImVec2(center_x + 6.0f, uni_pos.y + 4.0f), IM_COL32(255, 175, 20, 255), buf2);
                }

                // --- 5. INTERATIVIDADE DE MOUSE NA PISTA ÚNICA ---
                ImGui::SetCursorScreenPos(uni_pos);
                ImGui::InvisibleButton("##UniSuperimposedWaveLane", ImVec2(wave_w, uni_lane_h));

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DJ_TRACK_ID")) {
                        const char* dropped_id = (const char*)payload->Data;
                        auto* ptr = KuroAudio::DJLibraryManager::getInstance().getTrackById(dropped_id);
                        if (ptr) {
                            ImVec2 mp = ImGui::GetMousePos();
                            int target_deck_dnd = (mp.y < mid_y) ? deck1.deck_id : deck2.deck_id;
                            if (ptr->is_downloaded) spotify_service.loadLibraryTrack(*ptr, target_deck_dnd, engine);
                            else spotify_service.downloadLibraryTrackAsync(*ptr, target_deck_dnd, engine);
                        }
                    }
                    dl->AddRect(uni_pos, ImVec2(uni_pos.x + wave_w, uni_pos.y + uni_lane_h), IM_COL32(0, 240, 255, 255), 2.0f, 0, 2.5f);
                    ImGui::EndDragDropTarget();
                }

                bool lane_active = ImGui::IsItemActive();
                bool lane_clicked = ImGui::IsItemClicked(0);
                bool lane_hovered = ImGui::IsItemHovered();
                ImVec2 mouse_pos = ImGui::GetMousePos();
                bool target_is_d1 = (mouse_pos.y < mid_y);
                KuroAudio::DJDeck& target_deck = target_is_d1 ? deck1 : deck2;
                DeckVisualHertzSync& target_vs  = target_is_d1 ? vs1 : vs2;
                double target_window_f         = target_is_d1 ? window_f1 : window_f2;

                if (lane_active) {
                    ImGuiIO& io = ImGui::GetIO();
                    if (io.KeyAlt) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                        int g_idx = (grid_edit_deck == 0) ? active_l_id : active_r_id;
                        auto& g_deck = engine.decks[g_idx];
                        if (io.MouseDelta.x != 0.0f) {
                            double delta_frames = ((double)io.MouseDelta.x / (double)wave_w) * target_window_f;
                            if (io.KeyShift) delta_frames *= 0.20;
                            g_deck.first_beat_frame += delta_frames;
                            KuroAudio::DJGridEngine::getInstance().setFirstBeatMarker(g_deck.deck_id, g_deck.first_beat_frame);
                        }
                        double rel_ms = (g_deck.first_beat_frame - g_deck.current_frame) / g_deck.sample_rate * 1000.0;
                        ImGui::SetTooltip("REPOSICIONAR BEATGRID (Deck %d): Deslocamento %.1f ms", g_idx + 1, rel_ms);
                    } else {
                        target_deck.jog_touch = true;
                        target_deck.scratch_active_ticks = 6;
                        if (io.MouseDelta.x != 0.0f) {
                            double delta_frames = -((double)io.MouseDelta.x / (double)w) * target_window_f;
                            if (io.KeyShift) delta_frames *= 4.0;
                            target_deck.current_frame += delta_frames;
                            target_deck.current_frame = std::clamp(target_deck.current_frame, 0.0, (double)target_deck.buffer_l.size());
                            target_vs.smooth_frame = target_deck.current_frame;
                            target_deck.target_scratch_velocity = (float)(-io.MouseDelta.x * 0.18f);
                            target_deck.jog_visual_angle -= (float)io.MouseDelta.x * 0.025f;
                        }
                    }
                } else if (lane_clicked) {
                    if (ImGui::GetIO().KeyAlt) {
                        float click_x = ImGui::GetMousePos().x;
                        double offset_frames = ((double)(click_x - center_x) / (double)wave_w) * target_window_f;
                        target_deck.first_beat_frame = std::max(0.0, target_deck.current_frame + offset_frames);
                        KuroAudio::DJGridEngine::getInstance().setFirstBeatMarker(target_deck.deck_id, target_deck.first_beat_frame);
                    } else {
                        float click_x = ImGui::GetMousePos().x;
                        double offset_frames = ((double)(click_x - center_x) / (double)w) * target_window_f;
                        target_deck.current_frame = std::clamp(target_deck.current_frame + offset_frames, 0.0, (double)target_deck.buffer_l.size());
                        target_vs.smooth_frame = target_deck.current_frame;
                    }
                } else if (lane_hovered && ImGui::IsMouseReleased(0)) {
                    target_deck.jog_touch = false;
                }

                if (lane_hovered && ImGui::GetIO().MouseWheel != 0.0f) {
                    float wheel = ImGui::GetIO().MouseWheel;
                    if (ImGui::GetIO().KeyCtrl) {
                        if (wheel > 0.0f) { if (waveform_zoom_bars > 1) waveform_zoom_bars /= 2; }
                        else { if (waveform_zoom_bars < 64) waveform_zoom_bars *= 2; }
                    } else {
                        double seek_step = (double)wheel * (target_deck.sample_rate * (ImGui::GetIO().KeyShift ? 4.0 : 0.4));
                        target_deck.current_frame = std::clamp(target_deck.current_frame + seek_step, 0.0, (double)target_deck.buffer_l.size());
                        target_vs.smooth_frame = target_deck.current_frame;
                    }
                }

                if (lane_hovered) {
                    ImGui::SetTooltip("PISTA UNICA SOBREPOSTA: Superior = Deck 1 (Ciano) | Inferior = Deck 2 (Ambar) | Alt+Arrastar: Beatgrid | Ctrl+Scroll: Zoom");
                }

                ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_stack_h));
                ImGui::Dummy(ImVec2(w, 2.0f));
                return;
            }

            // =========================================================================================
            // MODO 1: PISTAS DUPLAS EMPILHADAS TRADICIONAIS (MODO CLÁSSICO ALTERNATIVO)
            // =========================================================================================
            ImVec2 l0_pos = ImVec2(wave_x, pos.y);
            ImVec2 l1_pos = ImVec2(wave_x, pos.y + lane_h + lane_gap);
            KuroAudio::DJDeck* lane_decks[2] = { &deck1, &deck2 };
            int deck_ids[2] = { active_l_id, active_r_id };
            ImVec4 lane_themes[2] = {
                (active_l_id == 0 ? col_deck_1 : col_deck_3),
                (active_r_id == 1 ? col_deck_2 : col_deck_4)
            };

            for (int lane = 0; lane < 2; lane++) {
                KuroAudio::DJDeck& deck = *lane_decks[lane];
                int d_num = deck_ids[lane] + 1;
                ImVec4 theme_col = lane_themes[lane];
                ImVec2 l_pos = (lane == 0) ? l0_pos : l1_pos;

                dl->AddRectFilled(l_pos, ImVec2(l_pos.x + wave_w, l_pos.y + lane_h), IM_COL32(0, 0, 0, 255));
                if (lane == 0) dl->AddLine(ImVec2(l_pos.x, l_pos.y + lane_h), ImVec2(l_pos.x + w, l_pos.y + lane_h), IM_COL32(20, 26, 36, 255), 1.0f);
                else dl->AddLine(l_pos, ImVec2(l_pos.x + w, l_pos.y), IM_COL32(20, 26, 36, 255), 1.0f);

                float mid_y = l_pos.y + lane_h * 0.5f;
                dl->AddLine(ImVec2(l_pos.x, mid_y), ImVec2(l_pos.x + w, mid_y), IM_COL32(28, 38, 52, 160), 1.0f);

                DeckVisualHertzSync& vs = vis_sync[deck.deck_id];
                double render_frame = vs.smooth_frame;
                double eff_bpm = (deck.bpm > 20.0) ? deck.bpm : 126.0;
                double seconds_per_bar = (60.0 / eff_bpm) * 4.0;
                double window_frames_d = (double)waveform_zoom_bars * seconds_per_bar * deck.sample_rate;
                if (window_frames_d < deck.sample_rate * 0.5) window_frames_d = deck.sample_rate * 0.5;
                double start_f_d = render_frame - window_frames_d * 0.5;

                if (!deck.buffer_l.empty() && deck.sample_rate > 1000.0) {
                    size_t total_f = deck.buffer_l.size();
                    if (deck.bpm > 20.0) {
                        double natural_beat_len = (60.0 / deck.bpm) * deck.sample_rate;
                        if (natural_beat_len > 10.0) {
                            int b_start = (int)std::floor((start_f_d - deck.first_beat_frame) / natural_beat_len);
                            int b_end   = (int)std::ceil((start_f_d + window_frames_d - deck.first_beat_frame) / natural_beat_len);
                            for (int b = b_start; b <= b_end; b++) {
                                double beat_f = deck.first_beat_frame + (double)b * natural_beat_len;
                                float bx = l_pos.x + (float)((beat_f - start_f_d) / window_frames_d) * wave_w;
                                if (bx >= l_pos.x && bx <= l_pos.x + wave_w) {
                                    if (b % 4 == 0) {
                                        dl->AddLine(ImVec2(bx, l_pos.y), ImVec2(bx, l_pos.y + lane_h), IM_COL32(255, 30, 50, 220), 1.0f);
                                        char b_str[16]; snprintf(b_str, sizeof(b_str), "%d.1", (b / 4) + 1);
                                        dl->AddRectFilled(ImVec2(bx - 9.0f, l_pos.y + 1.0f), ImVec2(bx + 9.0f, l_pos.y + 12.0f), IM_COL32(180, 15, 30, 230), 2.0f);
                                        dl->AddText(ImVec2(bx - 7.5f, l_pos.y + 0.5f), IM_COL32(255, 255, 255, 255), b_str);
                                    } else {
                                        dl->AddLine(ImVec2(bx, l_pos.y), ImVec2(bx, l_pos.y + lane_h), IM_COL32(160, 175, 195, 45), 1.0f);
                                        dl->AddLine(ImVec2(bx, l_pos.y), ImVec2(bx, l_pos.y + 3.0f), IM_COL32(180, 195, 215, 140), 1.0f);
                                        dl->AddLine(ImVec2(bx, l_pos.y + lane_h - 3.0f), ImVec2(bx, l_pos.y + lane_h), IM_COL32(180, 195, 215, 140), 1.0f);
                                    }
                                }
                            }
                        }
                    }

                    deck.generateDetailedWaveformIfNeeded();
                    float col_step = 1.0f;
                    int num_cols = std::max(1, (int)(wave_w / col_step));
                    double frames_per_col = window_frames_d / (double)num_cols;
                    float max_half_h = (lane_h * 0.5f) - 3.0f;
                    size_t detailed_sz = deck.detailed_waveform.size();

                    for (int x = 0; x < num_cols; x++) {
                        double f_cur = start_f_d + (double)x * frames_per_col;
                        float cur_x  = l_pos.x + (float)x * col_step;
                        if (f_cur < 0.0 || f_cur >= (double)total_f || detailed_sz == 0) {
                            dl->AddLine(ImVec2(cur_x, mid_y), ImVec2(cur_x + 1.0f, mid_y), IM_COL32(30, 42, 58, 150), 1.0f);
                            continue;
                        }
                        double bin_pos = f_cur / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP;
                        size_t b0 = (size_t)std::clamp(std::floor(bin_pos), 0.0, (double)(detailed_sz - 1));
                        size_t b1 = std::min(b0 + 1, detailed_sz - 1);
                        float frac = (float)(bin_pos - (double)b0);
                        float s_t = frac * frac * (3.0f - 2.0f * frac);

                        const auto& pt0 = deck.detailed_waveform[b0];
                        const auto& pt1 = deck.detailed_waveform[b1];
                        float p_tot = pt0.peak + (pt1.peak - pt0.peak) * s_t;
                        float pt_l  = pt0.low  + (pt1.low  - pt0.low)  * s_t;
                        float pt_m  = pt0.mid  + (pt1.mid  - pt0.mid)  * s_t;
                        float pt_h  = pt0.high + (pt1.high - pt0.high) * s_t;

                        if (p_tot < 0.002f) {
                            dl->AddLine(ImVec2(cur_x, mid_y), ImVec2(cur_x + 1.0f, mid_y), IM_COL32(30, 42, 58, 150), 1.0f);
                            continue;
                        }

                        float norm_p = std::clamp((p_tot - 0.002f) / (1.0f - 0.002f), 0.0f, 1.0f);
                        float p_shaped = std::pow(norm_p, 1.02f);
                        float tot_amp = std::clamp(p_shaped * max_half_h, 0.8f, max_half_h);

                        ImU32 outer_col, core_col;
                        computeRekordboxRGB(pt_l, pt_m, pt_h, p_tot, 1.0f, outer_col, core_col);
                        dl->AddLine(ImVec2(cur_x, mid_y - tot_amp), ImVec2(cur_x, mid_y + tot_amp), outer_col, 1.0f);
                        float core_amp = tot_amp * 0.44f;
                        if (core_amp >= 1.2f) dl->AddLine(ImVec2(cur_x, mid_y - core_amp), ImVec2(cur_x, mid_y + core_amp), core_col, 1.0f);
                    }

                    if (deck.duration_sec > 0.0 && deck.cue_frame >= 0.0) {
                        float cx_cue = l_pos.x + (float)((deck.cue_frame - start_f_d) / window_frames_d) * wave_w;
                        if (cx_cue >= l_pos.x && cx_cue <= l_pos.x + w) {
                            ImU32 col_cue_amb = IM_COL32(255, 160, 0, 255);
                            dl->AddLine(ImVec2(cx_cue, l_pos.y), ImVec2(cx_cue, l_pos.y + lane_h), col_cue_amb, 1.0f);
                            dl->AddRectFilled(ImVec2(cx_cue - 10.0f, l_pos.y + 1.0f), ImVec2(cx_cue + 10.0f, l_pos.y + 12.0f), col_cue_amb, 2.0f);
                            dl->AddText(ImVec2(cx_cue - 8.0f, l_pos.y + 0.5f), IM_COL32(0, 0, 0, 255), "CUE");
                        }
                    }

                    for (int c = 0; c < 8; c++) {
                        if (deck.hot_cues[c].active) {
                            double cf = deck.hot_cues[c].time_sec * deck.sample_rate;
                            float cx = l_pos.x + (float)((cf - start_f_d) / window_frames_d) * wave_w;
                            if (cx >= l_pos.x && cx <= l_pos.x + w) {
                                ImU32 cue_c = deck.hot_cues[c].color_rgba;
                                dl->AddLine(ImVec2(cx, l_pos.y), ImVec2(cx, l_pos.y + lane_h), cue_c, 1.0f);
                                dl->AddRectFilled(ImVec2(cx - 7.0f, l_pos.y + 1.0f), ImVec2(cx + 7.0f, l_pos.y + 13.0f), cue_c, 2.0f);
                                dl->AddText(ImVec2(cx - 4.0f, l_pos.y + 0.5f), IM_COL32(0, 0, 0, 255), hot_cue_letters[c]);
                            }
                        }
                    }
                }

                char lane_btn_id[32];
                snprintf(lane_btn_id, sizeof(lane_btn_id), "##WaveLane_%d", deck.deck_id);
                ImGui::SetCursorScreenPos(l_pos);
                ImGui::InvisibleButton(lane_btn_id, ImVec2(wave_w, lane_h));

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DJ_TRACK_ID")) {
                        const char* dropped_id = (const char*)payload->Data;
                        auto* ptr = KuroAudio::DJLibraryManager::getInstance().getTrackById(dropped_id);
                        if (ptr) {
                            if (ptr->is_downloaded) spotify_service.loadLibraryTrack(*ptr, deck.deck_id, engine);
                            else spotify_service.downloadLibraryTrackAsync(*ptr, deck.deck_id, engine);
                        }
                    }
                    dl->AddRect(l_pos, ImVec2(l_pos.x + wave_w, l_pos.y + lane_h), ImGui::ColorConvertFloat4ToU32(theme_col), 2.0f, 0, 2.5f);
                    ImGui::EndDragDropTarget();
                }

                bool lane_active = ImGui::IsItemActive();
                bool lane_clicked = ImGui::IsItemClicked(0);
                bool lane_hovered = ImGui::IsItemHovered();

                if (lane_active) {
                    ImGuiIO& io = ImGui::GetIO();
                    if (io.KeyAlt) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                        if (io.MouseDelta.x != 0.0f) {
                            double delta_frames = ((double)io.MouseDelta.x / (double)wave_w) * window_frames_d;
                            if (io.KeyShift) delta_frames *= 0.20;
                            deck.first_beat_frame += delta_frames;
                            KuroAudio::DJGridEngine::getInstance().setFirstBeatMarker(deck.deck_id, deck.first_beat_frame);
                        }
                    } else {
                        deck.jog_touch = true;
                        deck.scratch_active_ticks = 6;
                        if (io.MouseDelta.x != 0.0f) {
                            double delta_frames = -((double)io.MouseDelta.x / (double)w) * window_frames_d;
                            if (io.KeyShift) delta_frames *= 4.0;
                            deck.current_frame = std::clamp(deck.current_frame + delta_frames, 0.0, (double)deck.buffer_l.size());
                            vs.smooth_frame = deck.current_frame;
                            deck.target_scratch_velocity = (float)(-io.MouseDelta.x * 0.18f);
                            deck.jog_visual_angle -= (float)io.MouseDelta.x * 0.025f;
                        }
                    }
                } else if (lane_clicked) {
                    if (ImGui::GetIO().KeyAlt) {
                        float click_x = ImGui::GetMousePos().x;
                        double offset_frames = ((double)(click_x - center_x) / (double)wave_w) * window_frames_d;
                        deck.first_beat_frame = std::max(0.0, deck.current_frame + offset_frames);
                        KuroAudio::DJGridEngine::getInstance().setFirstBeatMarker(deck.deck_id, deck.first_beat_frame);
                    } else {
                        float click_x = ImGui::GetMousePos().x;
                        double offset_frames = ((double)(click_x - center_x) / (double)w) * window_frames_d;
                        deck.current_frame = std::clamp(deck.current_frame + offset_frames, 0.0, (double)deck.buffer_l.size());
                        vs.smooth_frame = deck.current_frame;
                    }
                } else if (lane_hovered && ImGui::IsMouseReleased(0)) {
                    deck.jog_touch = false;
                }
            }

            dl->AddLine(ImVec2(center_x, pos.y), ImVec2(center_x, pos.y + total_stack_h), IM_COL32(255, 255, 255, 255), 1.0f);
            ImU32 col_play_tri = IM_COL32(255, 140, 0, 255);
            dl->AddTriangleFilled(ImVec2(center_x - 3.5f, pos.y), ImVec2(center_x + 3.5f, pos.y), ImVec2(center_x, pos.y + 5.5f), col_play_tri);
            dl->AddTriangleFilled(ImVec2(center_x - 3.5f, pos.y + total_stack_h), ImVec2(center_x + 3.5f, pos.y + total_stack_h), ImVec2(center_x, pos.y + total_stack_h - 5.5f), col_play_tri);

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_stack_h));
            ImGui::Dummy(ImVec2(w, 2.0f));
        }'''

content = content[:idx_w1] + new_waveform_func + '\n\n        ' + content[idx_w2:]

with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
    f.write(content)

print("Applied Sessions 5, 6, 7 Superimposed Waveform Engine successfully.")
