#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "imgui.h"

namespace KuroAudio {

    enum class TrackPhraseType {
        INTRO = 0,
        VERSE_1,
        BUILDUP,
        DROP,
        BREAK_RESPRO,
        VERSE_2,
        OUTRO,
        UNKNOWN
    };

    struct TrackPhraseBlock {
        TrackPhraseType type = TrackPhraseType::UNKNOWN;
        double start_sec = 0.0;
        double end_sec = 0.0;
        int start_beat = 0;
        int end_beat = 0;
        float avg_energy = 0.0f;
        float low_energy = 0.0f;
        float mid_energy = 0.0f;
        float high_energy = 0.0f;
        std::string label;
    };

    class DJMIRPhrasingEngine {
    private:
        DJMIRPhrasingEngine() = default;

    public:
        static DJMIRPhrasingEngine& getInstance() {
            static DJMIRPhrasingEngine instance;
            return instance;
        }

        // =========================================================================
        // PALETA CROMÁTICA OFICIAL DO RECONHECIMENTO DE FASES (MIR)
        // REGRA ABSOLUTA DO USUÁRIO: Todos os Versos possuem uma cor padrão universal
        // no visor da pista (STANDARD_VERSE_COLOR = Verde Esmeralda Padrão).
        // =========================================================================
        static ImU32 getStandardVerseColor(float alpha = 1.0f) {
            return IM_COL32(0, 204, 136, (int)(255 * std::clamp(alpha, 0.0f, 1.0f)));
        }

        static ImU32 getPhraseColor(TrackPhraseType type, bool is_overview_strip = false, float alpha = 1.0f) {
            int a = (int)(255 * std::clamp(alpha, 0.0f, 1.0f));

            // Na régua de visão geral da pista, qualquer variação de Verso recebe a cor padrão universal
            if (is_overview_strip) {
                if (type == TrackPhraseType::VERSE_1 || type == TrackPhraseType::VERSE_2) {
                    return getStandardVerseColor(alpha);
                }
            }

            switch (type) {
                case TrackPhraseType::INTRO:
                    return IM_COL32(0, 140, 255, a); // Azul Safira Profundo
                case TrackPhraseType::VERSE_1:
                    return IM_COL32(0, 204, 136, a); // Verde Esmeralda Padrão
                case TrackPhraseType::BUILDUP:
                    return IM_COL32(255, 170, 0, a); // Âmbar Solar / Tensão
                case TrackPhraseType::DROP:
                    return IM_COL32(255, 35, 80, a); // Vermelho Carmim / Alta Energia
                case TrackPhraseType::BREAK_RESPRO:
                    return IM_COL32(165, 55, 245, a); // Roxo Místico / Fase de Respiro
                case TrackPhraseType::VERSE_2:
                    return is_overview_strip ? getStandardVerseColor(alpha) : IM_COL32(0, 225, 180, a); // Verso 2
                case TrackPhraseType::OUTRO:
                    return IM_COL32(45, 75, 120, a); // Azul Ardósia / Encerramento
                default:
                    return IM_COL32(80, 100, 130, a);
            }
        }

        static const char* getPhraseLabel(TrackPhraseType type) {
            switch (type) {
                case TrackPhraseType::INTRO:        return "INTRO";
                case TrackPhraseType::VERSE_1:      return "VERSO 1";
                case TrackPhraseType::BUILDUP:      return "BUILD";
                case TrackPhraseType::DROP:         return "DROP";
                case TrackPhraseType::BREAK_RESPRO: return "RESPIRO";
                case TrackPhraseType::VERSE_2:      return "VERSO 2";
                case TrackPhraseType::OUTRO:        return "OUTRO";
                default:                            return "PHRASE";
            }
        }

        // =========================================================================
        // MOTOR DE ANÁLISE ESTRUTURAL MIR (MUSIC INFORMATION RETRIEVAL)
        // Divide e classifica a música em blocos musicais de 16 ou 32 compassos
        // =========================================================================
        std::vector<TrackPhraseBlock> analyzeTrackPhrases(double duration_sec, double bpm,
                                                         const std::vector<float>& waveform_peaks) {
            std::vector<TrackPhraseBlock> phrases;
            if (duration_sec < 15.0 || bpm < 40.0) return phrases;

            double beat_duration = 60.0 / bpm;
            // Frase musical típica de pista: 32 beats (8 compassos de 4/4) ou 64 beats (16 compassos)
            int phrase_beats = (bpm > 115.0) ? 64 : 32;
            double phrase_sec = phrase_beats * beat_duration;

            int total_phrases = (int)std::ceil(duration_sec / phrase_sec);
            if (total_phrases < 2) total_phrases = 2;

            size_t num_peaks = waveform_peaks.size();

            // 1. Extração de perfis de energia por bloco de frase
            std::vector<float> phrase_energies(total_phrases, 0.0f);
            float max_e = 0.001f;

            for (int p = 0; p < total_phrases; p++) {
                double p_start = p * phrase_sec;
                double p_end = std::min((p + 1) * phrase_sec, duration_sec);
                float sum_e = 0.0f;
                int count = 0;

                if (num_peaks > 0) {
                    size_t idx_start = (size_t)((p_start / duration_sec) * num_peaks);
                    size_t idx_end = (size_t)((p_end / duration_sec) * num_peaks);
                    idx_end = std::min(idx_end, num_peaks);
                    for (size_t i = idx_start; i < idx_end; i++) {
                        sum_e += waveform_peaks[i];
                        count++;
                    }
                }
                float avg = (count > 0) ? (sum_e / count) : 0.4f;
                phrase_energies[p] = avg;
                if (avg > max_e) max_e = avg;
            }

            // Normaliza energias [0..1]
            for (int p = 0; p < total_phrases; p++) {
                phrase_energies[p] /= max_e;
            }

            // 2. Classificação Heurística Musical Estrutural (Intro, Verso, Build, Drop, Respiro, Verso 2, Outro)
            bool found_first_drop = false;
            bool passed_break = false;

            for (int p = 0; p < total_phrases; p++) {
                TrackPhraseBlock block;
                block.start_sec = p * phrase_sec;
                block.end_sec = std::min((p + 1) * phrase_sec, duration_sec);
                block.start_beat = p * phrase_beats;
                block.end_beat = (p + 1) * phrase_beats;
                block.avg_energy = phrase_energies[p];

                // Detecção por posição e dinâmica de energia
                if (p == 0) {
                    block.type = TrackPhraseType::INTRO;
                } else if (p == total_phrases - 1) {
                    block.type = TrackPhraseType::OUTRO;
                } else {
                    float cur_e = phrase_energies[p];
                    float prev_e = phrase_energies[p - 1];
                    float next_e = (p + 1 < total_phrases) ? phrase_energies[p + 1] : 0.0f;

                    if (!found_first_drop) {
                        // Antes do Drop 1
                        if (cur_e > 0.78f && cur_e >= prev_e) {
                            block.type = TrackPhraseType::DROP;
                            found_first_drop = true;
                        } else if (next_e > 0.78f && cur_e > prev_e) {
                            block.type = TrackPhraseType::BUILDUP;
                        } else {
                            block.type = TrackPhraseType::VERSE_1;
                        }
                    } else if (!passed_break) {
                        // Após o Drop 1: Procura o Break / Fase de Respiro
                        if (cur_e < 0.45f && prev_e > 0.65f) {
                            block.type = TrackPhraseType::BREAK_RESPRO;
                            passed_break = true;
                        } else if (cur_e > 0.75f) {
                            block.type = TrackPhraseType::DROP;
                        } else {
                            block.type = TrackPhraseType::VERSE_1;
                        }
                    } else {
                        // Após a Fase de Respiro: Verso 2, Build 2 e Drop 2
                        if (cur_e > 0.82f) {
                            block.type = TrackPhraseType::DROP;
                        } else if (next_e > 0.82f && cur_e > prev_e) {
                            block.type = TrackPhraseType::BUILDUP;
                        } else {
                            block.type = TrackPhraseType::VERSE_2;
                        }
                    }
                }

                block.label = getPhraseLabel(block.type);
                phrases.push_back(block);
            }

            return phrases;
        }

        // =========================================================================
        // RENDERIZAÇÃO DA RÉGUA DE VISÃO GERAL COM AS CORES DAS FASES DA PISTA
        // =========================================================================
        void renderOverviewStripPhrases(ImDrawList* dl, ImVec2 pos, float w, float h,
                                        const std::vector<TrackPhraseBlock>& phrases,
                                        double duration_sec, double current_sec) {
            if (duration_sec <= 0.0 || phrases.empty()) return;

            // Fundo escuro sutil
            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(10, 14, 20, 220), 2.0f);

            // Desenha cada bloco de frase
            for (const auto& ph : phrases) {
                float x1 = pos.x + (float)(ph.start_sec / duration_sec) * w;
                float x2 = pos.x + (float)(ph.end_sec / duration_sec) * w;
                float bw = std::max(1.0f, x2 - x1);

                // Cor padronizada para versos no visor da pista!
                ImU32 ph_col = getPhraseColor(ph.type, true, 0.82f);
                dl->AddRectFilled(ImVec2(x1, pos.y), ImVec2(x2, pos.y + h), ph_col);

                // Linha de divisão sutil
                dl->AddLine(ImVec2(x1, pos.y), ImVec2(x1, pos.y + h), IM_COL32(0, 0, 0, 140), 1.0f);

                // Label da fase se houver espaço na tela
                if (bw > 36.0f && h >= 14.0f) {
                    ImVec2 txt_sz = ImGui::CalcTextSize(ph.label.c_str());
                    if (txt_sz.x < bw - 4.0f) {
                        float tx = x1 + (bw - txt_sz.x) * 0.5f;
                        float ty = pos.y + (h - txt_sz.y) * 0.5f;
                        dl->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 220), ph.label.c_str());
                    }
                }
            }

            // Indicador de agulha do ponto atual (Playhead da Régua)
            if (current_sec >= 0.0 && current_sec <= duration_sec) {
                float px = pos.x + (float)(current_sec / duration_sec) * w;
                dl->AddLine(ImVec2(px, pos.y - 1.0f), ImVec2(px, pos.y + h + 1.0f), IM_COL32(255, 255, 255, 255), 2.0f);
                dl->AddTriangleFilled(ImVec2(px - 3.0f, pos.y), ImVec2(px + 3.0f, pos.y), ImVec2(px, pos.y + 4.0f), IM_COL32(255, 215, 0, 255));
            }
        }
    };

} // namespace KuroAudio

