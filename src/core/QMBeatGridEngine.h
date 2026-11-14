#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "DJRekordboxAnlz.h"

namespace KuroAudio {

    struct QMBeatGridResult {
        bool is_valid = false;
        double bpm = 120.0;
        double raw_bpm = 120.0;
        double first_beat_frame = 0.0;
        double first_beat_sec = 0.0;
        double first_beat_offset_ms = 0.0;
        int total_beats = 0;
        std::vector<RekordboxBeat> beats;
    };

    class QMBeatGridEngine {
    public:
        /**
         * Executa a análise completa de Beatgrid usando a engine Queen Mary (Mixxx / QM-DSP)
         * - Complex Spectral Difference (DF_COMPLEXSD)
         * - TempoTrackV2 com Comb Filter Bank e Viterbi Decoding
         * - DownBeat com divergência espectral Jensen-Shannon
         * - Regressão Linear por Mínimos Quadrados (Least-Squares Global Fit)
         * - Cravação inteligente em BPM nominal de música eletrônica
         */
        static QMBeatGridResult analyze(const std::vector<float>& buffer_l,
                                       const std::vector<float>& buffer_r,
                                       double sample_rate,
                                       double tempo_prior_bpm = 135.0);

        /**
         * Analisa diretamente um arquivo WAV no disco
         */
        static QMBeatGridResult analyzeFile(const std::string& wav_path,
                                           double tempo_prior_bpm = 135.0);
    };

} // namespace KuroAudio
