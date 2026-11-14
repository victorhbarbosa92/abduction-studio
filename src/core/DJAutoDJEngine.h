#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "DJEngine.h"
#include "DJLibraryManager.h"
#include "DJHarmonicEngine.h"

namespace KuroAudio {

    enum class AutoDJState {
        IDLE = 0,
        MONITORING_PHRASE = 1,
        QUEUEING_NEXT_TRACK = 2,
        IN_TRANSITION = 3,
        FINISHED = 4
    };

    class DJAutoDJEngine {
    private:
        bool enabled = false;
        int transition_length_bars = 32; // 8, 16 ou 32 compassos
        bool bass_swap_enabled = true;

        AutoDJState current_state = AutoDJState::IDLE;
        int active_deck_idx = 0;
        int incoming_deck_idx = 1;

        std::string status_message = "AutoDJ Desativado";
        std::string queued_track_title = "";
        std::string queued_track_artist = "";
        std::string queued_track_key = "";
        double queued_track_bpm = 128.0;
        std::string queued_track_path = "";

        // Metadados de transição
        double transition_start_frame = 0.0;
        double total_transition_frames = 1.0;
        float transition_progress = 0.0f;
        int current_transition_bar = 0;
        bool has_executed_bass_swap = false;
        float bass_swap_flash_alpha = 0.0f;

    public:
        DJAutoDJEngine() = default;

        static DJAutoDJEngine& getInstance() {
            static DJAutoDJEngine instance;
            return instance;
        }

        bool isEnabled() const { return enabled; }
        void setEnabled(bool en) {
            enabled = en;
            if (!enabled) {
                current_state = AutoDJState::IDLE;
                status_message = "AutoDJ Desativado";
                transition_progress = 0.0f;
            } else {
                current_state = AutoDJState::MONITORING_PHRASE;
                status_message = "AutoDJ Ativado: Monitorando Frase...";
            }
        }
        void toggleEnabled() { setEnabled(!enabled); }

        int getTransitionLengthBars() const { return transition_length_bars; }
        void setTransitionLengthBars(int bars) {
            transition_length_bars = (bars == 8 || bars == 16 || bars == 32) ? bars : 32;
        }

        bool isBassSwapEnabled() const { return bass_swap_enabled; }
        void setBassSwapEnabled(bool bs) { bass_swap_enabled = bs; }

        AutoDJState getState() const { return current_state; }
        const std::string& getStatusMessage() const { return status_message; }
        float getTransitionProgress() const { return transition_progress; }
        int getCurrentTransitionBar() const { return current_transition_bar; }
        float getBassSwapFlashAlpha() const { return bass_swap_flash_alpha; }
        int getActiveDeckIdx() const { return active_deck_idx; }
        int getIncomingDeckIdx() const { return incoming_deck_idx; }
        const std::string& getQueuedTrackTitle() const { return queued_track_title; }
        const std::string& getQueuedTrackKey() const { return queued_track_key; }

        // Seleção de melhor candidato harmônico da biblioteca
        bool findBestHarmonicTrack(const std::string& current_key, double current_bpm, DJLibraryTrack& out_track) {
            auto& lib = DJLibraryManager::getInstance();
            auto tracks = lib.getTracksForFolder("all");
            if (tracks.empty()) return false;

            CamelotKey cur_ck = DJHarmonicEngine::parseKey(current_key);

            int best_score = -9999;
            DJLibraryTrack best_match;
            bool found = false;

            for (const auto& t : tracks) {
                if (t.local_wav_path.empty() || !std::filesystem::exists(t.local_wav_path)) continue;
                if (!t.title.empty() && t.title == queued_track_title) continue;

                CamelotKey t_ck = DJHarmonicEngine::parseKey(t.key);
                int score = 0;

                // 1. Compatibilidade Harmônica Camelot
                if (cur_ck.valid && t_ck.valid) {
                    auto comp = DJHarmonicEngine::compareKeys(cur_ck, t_ck);
                    switch (comp) {
                        case HarmonicCompatibility::PERFECT: score += 100; break;
                        case HarmonicCompatibility::ENERGY_PLUS_1: score += 85; break;
                        case HarmonicCompatibility::HARMONIC_RELATIVE: score += 75; break;
                        case HarmonicCompatibility::ENERGY_BOOST_2: score += 65; break;
                        case HarmonicCompatibility::ENERGY_MINUS_1: score += 55; break;
                        case HarmonicCompatibility::DIAGONAL_PLUS_1: score += 50; break;
                        case HarmonicCompatibility::DIAGONAL_MINUS_1: score += 45; break;
                        default: score += 10; break;
                    }
                }

                // 2. Proximidade de BPM
                if (current_bpm > 20.0 && t.bpm > 20.0) {
                    double bpm_diff_pct = std::abs((t.bpm - current_bpm) / current_bpm) * 100.0;
                    if (bpm_diff_pct <= 2.0) score += 50;
                    else if (bpm_diff_pct <= 5.0) score += 30;
                    else if (bpm_diff_pct <= 8.0) score += 10;
                    else score -= 30;
                }

                if (score > best_score) {
                    best_score = score;
                    best_match = t;
                    found = true;
                }
            }

            if (found) {
                out_track = best_match;
                return true;
            }
            return false;
        }

        // Loop de atualização do AutoDJ chamado a cada frame de UI / tick de controle
        void update(DJEngine& engine, double dt) {
            if (bass_swap_flash_alpha > 0.0f) {
                bass_swap_flash_alpha = std::max(0.0f, bass_swap_flash_alpha - (float)dt * 1.6f);
            }

            if (!enabled) return;

            auto& master_deck = engine.decks[active_deck_idx];
            auto& next_deck = engine.decks[incoming_deck_idx];

            // Se o deck ativo não estiver tocando, mas o outro estiver, atualiza o índice
            if (!master_deck.is_playing.load() && next_deck.is_playing.load()) {
                active_deck_idx = incoming_deck_idx;
                incoming_deck_idx = 1 - active_deck_idx;
                return;
            }

            // Se nenhum deck estiver tocando, tenta iniciar o Deck 1
            if (!master_deck.is_playing.load() && !next_deck.is_playing.load()) {
                if (master_deck.duration_sec > 0.0) {
                    master_deck.is_playing.store(true);
                    current_state = AutoDJState::MONITORING_PHRASE;
                    status_message = "AutoDJ: Deck " + std::to_string(active_deck_idx + 1) + " Iniciado!";
                } else {
                    status_message = "AutoDJ: Carregue uma faixa para iniciar";
                }
                return;
            }

            double cur_pos_sec = (master_deck.sample_rate > 0.0) ? (master_deck.current_frame / master_deck.sample_rate) : 0.0;
            double rem_sec = std::max(0.0, master_deck.duration_sec - cur_pos_sec);
            double beat_len_sec = (master_deck.bpm > 20.0) ? (60.0 / master_deck.bpm) : 0.5;
            double bar_len_sec = beat_len_sec * 4.0;
            int rem_bars = (int)(rem_sec / bar_len_sec);

            // ── FASE 1: MONITORAMENTO & SELEÇÃO DE FAIXA HARMÔNICA ────────────
            if (current_state == AutoDJState::MONITORING_PHRASE) {
                char msg[128];
                snprintf(msg, sizeof(msg), "AutoDJ: Frase do Deck %d (Faltam %d comp. / %ds)", active_deck_idx + 1, rem_bars, (int)rem_sec);
                status_message = msg;

                // Faltando 16 compassos além da transição: busca e carrega a próxima faixa
                if (rem_bars <= (transition_length_bars + 16) && queued_track_title.empty()) {
                    current_state = AutoDJState::QUEUEING_NEXT_TRACK;
                    DJLibraryTrack candidate;
                    if (findBestHarmonicTrack(master_deck.getEffectiveKey(), master_deck.bpm, candidate)) {
                        queued_track_title = candidate.title;
                        queued_track_artist = candidate.artist;
                        queued_track_key = candidate.key;
                        queued_track_bpm = candidate.bpm;
                        queued_track_path = candidate.local_wav_path;

                        bool load_ok = next_deck.loadTrack(queued_track_path);
                        if (load_ok && next_deck.duration_sec > 0.0 && !next_deck.buffer_l.empty()) {
                            next_deck.track_title = queued_track_title;
                            next_deck.track_artist = queued_track_artist;
                            next_deck.bpm = queued_track_bpm;
                            next_deck.key_signature = queued_track_key;
                            next_deck.current_frame = 0.0;

                            // Sincroniza BPM automaticamente com quantização de fase
                            engine.syncBPM(active_deck_idx, incoming_deck_idx);
                            status_message = "Carregado no Deck " + std::to_string(incoming_deck_idx + 1) + ": " + queued_track_title + " (" + queued_track_key + ")";
                        } else {
                            // Se falhou ao decodificar com segurança, reseta para não avançar para deck vazio
                            current_state = AutoDJState::MONITORING_PHRASE;
                            queued_track_title = "";
                            queued_track_path = "";
                        }
                    } else {
                        current_state = AutoDJState::MONITORING_PHRASE;
                    }
                }
            }

            // ── FASE 2: GATILHO DA TRANSIÇÃO (ENTRADA DO COMPASSO 1) ───────────
            if (rem_bars <= transition_length_bars && current_state != AutoDJState::IN_TRANSITION) {
                // Blindagem anti-crash: só dispara se o próximo deck tiver áudio válido carregado
                if (next_deck.duration_sec <= 0.0 || next_deck.buffer_l.empty()) {
                    status_message = "AutoDJ: Aguardando carregamento seguro da faixa...";
                    return;
                }
                current_state = AutoDJState::IN_TRANSITION;
                transition_start_frame = master_deck.current_frame;
                total_transition_frames = (double)transition_length_bars * bar_len_sec * master_deck.sample_rate;
                if (total_transition_frames <= 1000.0) total_transition_frames = 44100.0 * 20.0;

                next_deck.current_frame = 0.0;
                next_deck.volume = 0.0f;
                next_deck.eq_low = 0.0f;   // BASS KILL inicial no deck que está entrando!
                next_deck.eq_mid = 0.0f;
                next_deck.eq_high = 0.0f;
                next_deck.is_playing.store(true);

                has_executed_bass_swap = false;
                transition_progress = 0.0f;
                current_transition_bar = 1;
                status_message = "Iniciando Transição de " + std::to_string(transition_length_bars) + " Compassos...";
            }

            // ── FASE 3: COREOGRAFIA DE MIXAGEM & BASS SWAP CIRÚRGICO ───────────
            if (current_state == AutoDJState::IN_TRANSITION) {
                double elapsed_frames = master_deck.current_frame - transition_start_frame;
                float progress = std::clamp((float)(elapsed_frames / total_transition_frames), 0.0f, 1.0f);
                transition_progress = progress;
                current_transition_bar = std::clamp((int)(progress * (float)transition_length_bars) + 1, 1, transition_length_bars);

                if (progress < 0.5f) {
                    // Metade 1: Entrada suave dos médios/agudos do Deck B com grave cortado
                    float p1 = progress * 2.0f; // 0.0 -> 1.0
                    next_deck.volume = p1;
                    next_deck.eq_mid = p1 * 1.0f;
                    next_deck.eq_high = p1 * 1.0f;
                    next_deck.eq_low = 0.0f; // Bass Kill

                    master_deck.volume = 1.0f;
                    master_deck.eq_low = 1.0f;

                    // Crossfader centrado suavemente
                    float target_xf = (active_deck_idx == 0) ? (0.0f + p1 * 0.5f) : (1.0f - p1 * 0.5f);
                    engine.setCrossfader(target_xf);

                    char msg[128];
                    snprintf(msg, sizeof(msg), "AutoDJ: Mixando Compasso %d/%d (Preparando Drop...)", current_transition_bar, transition_length_bars);
                    status_message = msg;

                } else {
                    // Ponto de virada exato: O BASS SWAP!
                    if (!has_executed_bass_swap) {
                        has_executed_bass_swap = true;
                        if (bass_swap_enabled) {
                            master_deck.eq_low = 0.0f; // Corta grave do Master
                            next_deck.eq_low = 1.0f;   // Explode o grave do novo Deck
                            bass_swap_flash_alpha = 1.0f;
                            KuroUtils::Log("[AutoDJ] 💥 BASS SWAP EXECUTADO no Compasso " + std::to_string(current_transition_bar) + "!");
                        }
                    }

                    // Metade 2: Deck B assume 100%, Deck A sofre fade-out
                    float p2 = (progress - 0.5f) * 2.0f; // 0.0 -> 1.0
                    master_deck.volume = std::max(0.0f, 1.0f - p2);
                    master_deck.eq_high = std::max(0.0f, 1.0f - p2);
                    master_deck.eq_mid = std::max(0.0f, 1.0f - p2 * 0.8f);

                    next_deck.volume = 1.0f;
                    next_deck.eq_mid = 1.0f;
                    next_deck.eq_high = 1.0f;
                    next_deck.eq_low = 1.0f;

                    float target_xf = (active_deck_idx == 0) ? (0.5f + p2 * 0.5f) : (0.5f - p2 * 0.5f);
                    engine.setCrossfader(target_xf);

                    char msg[128];
                    snprintf(msg, sizeof(msg), "AutoDJ: %s Compasso %d/%d", 
                             (bass_swap_enabled ? "BASS SWAP ATIVO!" : "Mixando"), current_transition_bar, transition_length_bars);
                    status_message = msg;
                }

                // ── CONCLUSÃO DA TRANSIÇÃO ─────────────────────────────────────
                if (progress >= 1.0f) {
                    master_deck.is_playing.store(false);
                    master_deck.volume = 1.0f;
                    master_deck.eq_low = 1.0f;
                    master_deck.eq_mid = 1.0f;
                    master_deck.eq_high = 1.0f;

                    // Inverte os papéis dos decks
                    active_deck_idx = incoming_deck_idx;
                    incoming_deck_idx = 1 - active_deck_idx;
                    engine.setMasterDeck(active_deck_idx);

                    engine.setCrossfader(active_deck_idx == 0 ? 0.0f : 1.0f);
                    current_state = AutoDJState::MONITORING_PHRASE;
                    transition_progress = 0.0f;
                    queued_track_title = "";
                    has_executed_bass_swap = false;

                    status_message = "AutoDJ: Deck " + std::to_string(active_deck_idx + 1) + " assumiu o Master!";
                    KuroUtils::Log("[AutoDJ] Transição concluída com sucesso. Deck " + std::to_string(active_deck_idx + 1) + " é o novo Master.");
                }
            }
        }
    };

} // namespace KuroAudio
