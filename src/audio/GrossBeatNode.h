#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <mutex>

namespace KuroDSP {

class GrossBeatNode {
public:
    // O Gross Beat trabalha em ciclos baseados no compasso (geralmente 2 compassos ou 1 compasso)
    // Para Psycore/HiTech, 1 bar a 175 BPM = ~60/175 * 4 = 1.37 segundos = ~60480 samples em 44.1kHz
    GrossBeatNode() {
        buffer_L.resize(MAX_BUFFER_SIZE, 0.0f);
        buffer_R.resize(MAX_BUFFER_SIZE, 0.0f);
        write_pos = 0;
        
        // Inicializa com reta linear (playback normal)
        time_points = { {0.0f, 1.0f}, {1.0f, 0.0f} }; 
        vol_points = { {0.0f, 1.0f}, {1.0f, 1.0f} };
        
        bpm = 175.0f;
        sample_rate = 44100.0f;
        updateBufferLength();
    }

    struct Point {
        float x; // 0.0 a 1.0 (Tempo ao longo do ciclo)
        float y; // 0.0 a 1.0 (Para tempo: 1.0 = buffer mais recente (delay 0), 0.0 = delay maximo)
    };

    void setBPM(float new_bpm) {
        if (new_bpm > 0 && bpm != new_bpm) {
            bpm = new_bpm;
            updateBufferLength();
        }
    }

    void setSampleRate(float sr) {
        if (sr > 0 && sample_rate != sr) {
            sample_rate = sr;
            updateBufferLength();
        }
    }

    void setTimePoints(const std::vector<Point>& points) {
        std::lock_guard<std::mutex> lock(points_mutex);
        time_points = points;
    }
    
    void setVolPoints(const std::vector<Point>& points) {
        std::lock_guard<std::mutex> lock(points_mutex);
        vol_points = points;
    }

    void processBlock(float* out_L, float* out_R, int num_samples) {
        std::lock_guard<std::mutex> lock(points_mutex);
        
        if (!enabled) return; // Se desativado, não altera out_L e out_R
        
        for (int i = 0; i < num_samples; ++i) {
            // 1. Grava o áudio limpo atual no buffer
            buffer_L[write_pos] = out_L[i];
            buffer_R[write_pos] = out_R[i];

            // Posição no tempo do loop atual (0.0 a 1.0)
            float phase = (float)write_pos / (float)active_buffer_length;

            // Interpola a curva de tempo e volume
            float time_val = interpolateEnvelope(time_points, phase);
            float vol_val = interpolateEnvelope(vol_points, phase);

            // Calcula o atraso em samples
            // time_val = 1.0 significa delay 0 (Tocar o que acabou de gravar)
            // time_val = 0.0 significa delay max (active_buffer_length - 1)
            float delay_samples = (1.0f - time_val) * (active_buffer_length - 1);
            
            // Calcula a posição de leitura no buffer (com wrap-around)
            float read_pos_exact = (float)write_pos - delay_samples;
            if (read_pos_exact < 0.0f) read_pos_exact += active_buffer_length;

            // Interpolação linear da leitura
            int read_idx1 = (int)read_pos_exact;
            int read_idx2 = (read_idx1 + 1) % active_buffer_length;
            float frac = read_pos_exact - read_idx1;

            float sample_L = buffer_L[read_idx1] * (1.0f - frac) + buffer_L[read_idx2] * frac;
            float sample_R = buffer_R[read_idx1] * (1.0f - frac) + buffer_R[read_idx2] * frac;

            // Aplica curva de volume e o botão de mix
            out_L[i] = sample_L * vol_val * mix + out_L[i] * (1.0f - mix);
            out_R[i] = sample_R * vol_val * mix + out_R[i] * (1.0f - mix);

            // Avança o write pointer
            write_pos = (write_pos + 1) % active_buffer_length;
        }
    }

    bool enabled = false;
    float mix = 1.0f; // Dry/Wet

private:
    float interpolateEnvelope(const std::vector<Point>& points, float x) {
        if (points.empty()) return 1.0f;
        if (points.size() == 1) return points[0].y;
        if (x <= points.front().x) return points.front().y;
        if (x >= points.back().x) return points.back().y;

        for (size_t i = 0; i < points.size() - 1; ++i) {
            if (x >= points[i].x && x <= points[i + 1].x) {
                float range = points[i+1].x - points[i].x;
                if (range <= 0.0f) return points[i].y;
                float t = (x - points[i].x) / range;
                // Linear interpolation
                return points[i].y * (1.0f - t) + points[i+1].y * t;
            }
        }
        return 1.0f;
    }

    void updateBufferLength() {
        // Supondo 1 compasso de 4 batidas (4/4)
        float beats_per_bar = 4.0f;
        float bars = 1.0f; // Tamanho padrão do ciclo do Gross Beat (1 bar)
        float seconds_per_beat = 60.0f / bpm;
        float total_seconds = seconds_per_beat * beats_per_bar * bars;
        active_buffer_length = (int)(total_seconds * sample_rate);
        
        if (active_buffer_length > MAX_BUFFER_SIZE) {
            active_buffer_length = MAX_BUFFER_SIZE;
        }
        if (write_pos >= active_buffer_length) write_pos = 0;
    }

    const int MAX_BUFFER_SIZE = 44100 * 10; // Suporta até 10 segundos
    std::vector<float> buffer_L;
    std::vector<float> buffer_R;
    int write_pos;
    int active_buffer_length;

    float bpm;
    float sample_rate;

    std::vector<Point> time_points;
    std::vector<Point> vol_points;
    std::mutex points_mutex;
};

} // namespace
