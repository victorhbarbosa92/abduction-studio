#pragma once
#include <string>
#include <memory>
#include "../plugin_manager/DAG.h"
#include "../utils/Logger.h"

// Classe Placeholder/Motor Backend para o "Modo DJ"
namespace KuroAudio {

    class DJEngine {
    private:
        float crossfader = 0.5f; // 0.0 = Deck A, 1.0 = Deck B
        bool deckA_playing = false;
        bool deckB_playing = false;
        
        std::string deckA_track = "Nenhuma faixa carregada";
        std::string deckB_track = "Nenhuma faixa carregada";

    public:
        DJEngine() = default;
        ~DJEngine() = default;

        void loadDeckA(const std::string& filepath) {
            deckA_track = filepath;
            KuroUtils::Log("[DJ Engine] Deck A carregado: " + filepath);
        }

        void loadDeckB(const std::string& filepath) {
            deckB_track = filepath;
            KuroUtils::Log("[DJ Engine] Deck B carregado: " + filepath);
        }

        void setCrossfader(float value) {
            if (value < 0.0f) value = 0.0f;
            if (value > 1.0f) value = 1.0f;
            crossfader = value;
        }
        
        float getCrossfader() const {
            return crossfader;
        }

        void playDeckA() {
            deckA_playing = true;
            KuroUtils::Log("[DJ Engine] Deck A Play");
        }

        void pauseDeckA() {
            deckA_playing = false;
            KuroUtils::Log("[DJ Engine] Deck A Pause");
        }

        void playDeckB() {
            deckB_playing = true;
            KuroUtils::Log("[DJ Engine] Deck B Play");
        }

        void pauseDeckB() {
            deckB_playing = false;
            KuroUtils::Log("[DJ Engine] Deck B Pause");
        }

        void syncBPM() {
            KuroUtils::Log("[DJ Engine] Sync Ativado (Aguardando Timestretching DSP)");
        }

        std::string getDeckATrack() const { return deckA_track; }
        std::string getDeckBTrack() const { return deckB_track; }
        bool isDeckAPlaying() const { return deckA_playing; }
        bool isDeckBPlaying() const { return deckB_playing; }
    };

} // namespace KuroAudio
