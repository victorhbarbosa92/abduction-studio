#include "QMBeatGridEngine.h"
#include "../utils/Logger.h"
#include "../thirdparty/dr_wav.h"

// QM-DSP includes
#include "../thirdparty/qm-dsp/dsp/onsets/DetectionFunction.h"
#include "../thirdparty/qm-dsp/dsp/tempotracking/TempoTrackV2.h"
#include "../thirdparty/qm-dsp/dsp/tempotracking/DownBeat.h"
#include "../thirdparty/qm-dsp/dsp/rateconversion/Decimator.h"
#include "../thirdparty/qm-dsp/maths/MathUtilities.h"

#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>

namespace KuroAudio {

    QMBeatGridResult QMBeatGridEngine::analyze(const std::vector<float>& buffer_l,
                                              const std::vector<float>& buffer_r,
                                              double sample_rate,
                                              double tempo_prior_bpm) {
        QMBeatGridResult res;
        if (buffer_l.empty() || sample_rate < 1000.0) return res;

        size_t total_samples = buffer_l.size();
        bool has_stereo = (!buffer_r.empty() && buffer_r.size() == buffer_l.size());

        // 1. Converter para mono em float (para DownBeat) e double (para DetectionFunction)
        std::vector<float> mono_f(total_samples);
        std::vector<double> mono_d(total_samples);

        for (size_t i = 0; i < total_samples; ++i) {
            float s = has_stereo ? (0.5f * (buffer_l[i] + buffer_r[i])) : buffer_l[i];
            mono_f[i] = s;
            mono_d[i] = (double)s;
        }

        // 2. Extração de Onsets usando Complex Spectral Difference (DF_COMPLEXSD)
        DFConfig dfConfig;
        dfConfig.stepSize = 512;
        dfConfig.frameLength = 1024;
        dfConfig.DFType = DF_COMPLEXSD;
        dfConfig.adaptiveWhitening = false;
        dfConfig.whiteningRelaxCoeff = -1;
        dfConfig.whiteningFloor = -1;
        dfConfig.dbRise = 3.0;

        DetectionFunction dfProcessor(dfConfig);
        std::vector<double> df_values;
        df_values.reserve(total_samples / dfConfig.stepSize);

        int stepSize = dfConfig.stepSize;
        int frameLength = dfConfig.frameLength;
        std::vector<double> frame(frameLength, 0.0);

        for (size_t pos = 0; pos + frameLength <= mono_d.size(); pos += stepSize) {
            for (int k = 0; k < frameLength; ++k) {
                frame[k] = mono_d[pos + k];
            }
            double val = dfProcessor.processTimeDomain(frame.data());
            df_values.push_back(val);
        }

        if (df_values.size() < 16) return res;

        // 3. Rastreamento de Tempo com Comb Filter Bank e Viterbi (QM TempoTrackV2)
        TempoTrackV2 tt((float)sample_rate, stepSize);
        std::vector<double> beat_period;
        std::vector<double> tempi;

        double prior = (tempo_prior_bpm > 30.0 && tempo_prior_bpm < 300.0) ? tempo_prior_bpm : 135.0;
        tt.calculateBeatPeriod(df_values, beat_period, tempi, prior, false);

        std::vector<double> raw_beats;
        tt.calculateBeats(df_values, beat_period, raw_beats);

        if (raw_beats.size() < 4) return res;

        // 4. Detecção de Downbeat 1.1 via Divergência Jensen-Shannon (QM DownBeat)
        size_t decFactor = 16;
        DownBeat db((float)sample_rate, decFactor, stepSize);
        db.setBeatsPerBar(4);

        for (size_t pos = 0; pos + stepSize <= mono_f.size(); pos += stepSize) {
            db.pushAudioBlock(mono_f.data() + pos);
        }

        size_t dec_audio_len = 0;
        const float* dec_audio = db.getBufferedAudio(dec_audio_len);
        std::vector<int> downbeats;
        db.findDownBeats(dec_audio, dec_audio_len, raw_beats, downbeats);

        // 5. Const Regions (Algoritmo de Ironing do Mixxx para extrair o trecho de tempo estavel)
        std::vector<double> coarse_beats(raw_beats.size());
        for (size_t i = 0; i < raw_beats.size(); ++i) {
            coarse_beats[i] = raw_beats[i] * (double)stepSize;
        }

        struct ConstRegion {
            double first_beat;
            double beat_length;
            int count;
        };
        std::vector<ConstRegion> const_regions;

        const double max_phase_err = 0.025 * sample_rate;     // 25ms de tolerancia auditiva Mixxx
        const double max_phase_err_sum = 0.100 * sample_rate; // 100ms de desvio acumulado Mixxx
        int left_idx = 0;
        int right_idx = (int)coarse_beats.size() - 1;

        while (left_idx < (int)coarse_beats.size() - 1) {
            double mean_beat_len = (coarse_beats[right_idx] - coarse_beats[left_idx]) / (double)(right_idx - left_idx);
            int outliers = 0;
            double ironed = coarse_beats[left_idx];
            double err_sum = 0.0;
            int i = left_idx + 1;
            for (; i <= right_idx; ++i) {
                ironed += mean_beat_len;
                double p_err = ironed - coarse_beats[i];
                err_sum += p_err;
                if (std::abs(p_err) > max_phase_err) {
                    outliers++;
                    if (outliers > 1 || i == left_idx + 1) break;
                }
                if (std::abs(err_sum) > max_phase_err_sum) break;
            }

            if (i > right_idx) {
                const_regions.push_back({coarse_beats[left_idx], mean_beat_len, right_idx - left_idx});
                left_idx = right_idx;
                right_idx = (int)coarse_beats.size() - 1;
            } else {
                right_idx--;
                if (right_idx <= left_idx + 1) {
                    left_idx++;
                    right_idx = (int)coarse_beats.size() - 1;
                }
            }
        }

        double longest_region_len = 0.0;
        double longest_beat_len = (60.0 / 135.0) * sample_rate;
        for (const auto& cr : const_regions) {
            double reg_span = cr.beat_length * (double)cr.count;
            if (reg_span > longest_region_len && cr.beat_length > 100.0) {
                longest_region_len = reg_span;
                longest_beat_len = cr.beat_length;
            }
        }

        double raw_bpm = (60.0 * sample_rate) / longest_beat_len;
        if (raw_bpm < 30.0 || raw_bpm > 300.0) raw_bpm = 135.0;

        // 6. Validacao por Ressonancia Harmonica (Harmonic Comb Filter sobre Detection Function)
        // Avalia candidatos (Inteiros, Meio-BPM, Contínuo) contra a forma de onda de todo o áudio
        auto eval_comb = [&](double test_bpm) -> double {
            if (test_bpm < 30.0 || test_bpm > 300.0) return 0.0;
            double t_step = (60.0 * sample_rate) / (test_bpm * (double)stepSize);
            if (t_step < 2.0 || t_step >= (double)df_values.size()) return 0.0;
            
            int num_phases = 32;
            double max_comb = 0.0;
            for (int p = 0; p < num_phases; ++p) {
                double phi = ((double)p / (double)num_phases) * t_step;
                double comb = 0.0;
                int count = 0;
                for (double idx = phi; idx < (double)df_values.size(); idx += t_step) {
                    size_t i0 = (size_t)idx;
                    double frac = idx - (double)i0;
                    double v = (1.0 - frac) * df_values[i0];
                    if (i0 + 1 < df_values.size()) v += frac * df_values[i0 + 1];
                    comb += v;
                    count++;
                }
                if (count > 0) comb /= (double)count;
                if (comb > max_comb) max_comb = comb;
            }
            return max_comb;
        };

        std::vector<double> candidates;
        double round_int = std::round(raw_bpm);
        candidates.push_back(round_int);
        candidates.push_back(round_int - 1.0);
        candidates.push_back(round_int + 1.0);
        candidates.push_back(round_int - 0.5);
        candidates.push_back(round_int + 0.5);
        candidates.push_back(raw_bpm);

        double best_bpm = round_int;
        double max_score = -1.0;
        double int_score = -1.0;

        for (double c_bpm : candidates) {
            double sc = eval_comb(c_bpm);
            if (std::abs(c_bpm - round_int) < 0.01) int_score = sc;
            if (sc > max_score) {
                max_score = sc;
                best_bpm = c_bpm;
            }
        }

        // Se o BPM inteiro tiver pelo menos 88% da energia do pico do comb, o BPM inteiro prevalece
        // (Isso reflete 100% como música eletrônica comercial é produzida em DAWs)
        double final_bpm = best_bpm;
        if (int_score > 0.0 && int_score >= 0.88 * max_score) {
            final_bpm = round_int;
        }

        double beat_len_frames = (60.0 / final_bpm) * sample_rate;
        double bar_len_frames = beat_len_frames * 4.0;

        // 7. Alinhamento de Fase de Sub-Amostra (Maximizacao de Onsets de Kicks na Forma de Onda)
        // Amostra a derivada retificada do áudio nos primeiros 120s da faixa
        size_t env_len = std::min(mono_f.size(), (size_t)(120.0 * sample_rate));
        std::vector<float> onset_flux(env_len, 0.0f);
        for (size_t i = 1; i < env_len; ++i) {
            float diff = std::abs(mono_f[i]) - std::abs(mono_f[i - 1]);
            if (diff > 0.0f) onset_flux[i] = diff;
        }

        double best_anchor = 0.0;
        double max_anchor_flux = -1.0;
        double step_test = 16.0; // Passos de ~0.36ms
        for (double p = 0.0; p < beat_len_frames; p += step_test) {
            double flux_sum = 0.0;
            for (double f = p; f < (double)env_len; f += beat_len_frames) {
                size_t fi = (size_t)f;
                if (fi < env_len) flux_sum += onset_flux[fi];
            }
            if (flux_sum > max_anchor_flux) {
                max_anchor_flux = flux_sum;
                best_anchor = p;
            }
        }

        // Refinamento fino a nível de amostra unitária (+-16 frames = +-0.36ms)
        double fine_anchor = best_anchor;
        for (double p = best_anchor - 16.0; p <= best_anchor + 16.0; p += 1.0) {
            double flux_sum = 0.0;
            for (double f = p; f < (double)env_len; f += beat_len_frames) {
                size_t fi = (size_t)f;
                if (fi < env_len) flux_sum += onset_flux[fi];
            }
            if (flux_sum > max_anchor_flux) {
                max_anchor_flux = flux_sum;
                fine_anchor = p;
            }
        }

        // 8. Fase de compasso 4/4 e projeção do primeiro Downbeat 1.1 usando QM DownBeat
        int best_bar_phase = 0;
        if (!downbeats.empty()) {
            int phase_votes[4] = {0, 0, 0, 0};
            for (int db_idx : downbeats) {
                if (db_idx >= 0 && db_idx < (int)raw_beats.size()) {
                    double raw_f = raw_beats[db_idx] * (double)stepSize;
                    double rel = (raw_f - fine_anchor) / beat_len_frames;
                    int b_num = (int)std::round(rel);
                    int phase = ((b_num % 4) + 4) % 4;
                    phase_votes[phase]++;
                }
            }
            int max_votes = -1;
            for (int p = 0; p < 4; ++p) {
                if (phase_votes[p] > max_votes) {
                    max_votes = phase_votes[p];
                    best_bar_phase = p;
                }
            }
        }

        double first_downbeat_frame = fine_anchor + (double)best_bar_phase * beat_len_frames;
        while (first_downbeat_frame < 0.0) first_downbeat_frame += bar_len_frames;
        while (first_downbeat_frame >= bar_len_frames) first_downbeat_frame -= bar_len_frames;

        // 9. Construção completa da lista RekordboxBeat (Downbeat 1.1 vermelho + beats 2, 3, 4 brancos)
        res.is_valid = true;
        res.bpm = final_bpm;
        res.raw_bpm = raw_bpm;
        res.first_beat_frame = first_downbeat_frame;
        res.first_beat_sec = first_downbeat_frame / sample_rate;
        res.first_beat_offset_ms = res.first_beat_sec * 1000.0;

        int total_beats_est = (int)((total_samples - first_downbeat_frame) / beat_len_frames) + 1;
        if (total_beats_est > 0) {
            res.beats.reserve(total_beats_est);
            int beat_counter = 0;
            for (double f = first_downbeat_frame; f < (double)total_samples; f += beat_len_frames, beat_counter++) {
                RekordboxBeat b;
                b.bar_beat = (beat_counter % 4) + 1; // 1 = Red Downbeat, 2, 3, 4 = White Beats
                b.tempo_bpm = final_bpm;
                b.frame = (int64_t)std::round(f);
                b.time_sec = (double)b.frame / sample_rate;
                b.time_ms = b.time_sec * 1000.0;
                res.beats.push_back(b);
            }
        }
        res.total_beats = (int)res.beats.size();

        KuroUtils::Log("[QM BeatGrid Engine] Análise Concluída: " + std::to_string(final_bpm) + 
                       " BPM (Raw: " + std::to_string(raw_bpm) + 
                       "), Downbeat 1.1 em " + std::to_string(res.first_beat_sec) + 
                       "s (" + std::to_string(res.first_beat_offset_ms) + " ms) - Total Beats: " + 
                       std::to_string(res.total_beats));

        return res;
    }

    QMBeatGridResult QMBeatGridEngine::analyzeFile(const std::string& wav_path,
                                                  double tempo_prior_bpm) {
        QMBeatGridResult res;
        unsigned int channels = 0;
        unsigned int sample_rate = 0;
        drwav_uint64 total_pcm_frame_count = 0;
        float* p_sample_data = drwav_open_file_and_read_pcm_frames_f32(wav_path.c_str(), &channels, &sample_rate, &total_pcm_frame_count, NULL);

        if (!p_sample_data || total_pcm_frame_count == 0) {
            return res;
        }

        std::vector<float> buf_l(total_pcm_frame_count);
        std::vector<float> buf_r;
        if (channels > 1) buf_r.resize(total_pcm_frame_count);

        for (drwav_uint64 i = 0; i < total_pcm_frame_count; ++i) {
            buf_l[i] = p_sample_data[i * channels];
            if (channels > 1) {
                buf_r[i] = p_sample_data[i * channels + 1];
            }
        }
        drwav_free(p_sample_data, NULL);

        return analyze(buf_l, buf_r, (double)sample_rate, tempo_prior_bpm);
    }

} // namespace KuroAudio
