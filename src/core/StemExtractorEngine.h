#pragma once
#include <string>
#include <vector>
#include "../ai/StemSeparationEngine.h"
#include "../utils/Logger.h"

namespace KuroAudio {

    class StemExtractorEngine {
    private:
        StemSeparationEngine internal_ai;
        std::string current_file = "Nenhum arquivo carregado";

    public:
        StemExtractorEngine() = default;
        ~StemExtractorEngine() = default;

        void dropFileToIsolate(const std::string& filepath) {
            current_file = filepath;
            KuroUtils::Log("[Stem Extractor] Arquivo recebido para isolamento limpo: " + filepath);
            internal_ai.reset();
            internal_ai.startProcessing(filepath); // Começa a separar em background independentemente da DAW
        }

        void exportSingleStem(int track_index) {
            if (!internal_ai.hasFinished()) return;
            internal_ai.exportStems(".", 4); // old behavior
        }
        
        void exportAllStems() {
            if (!internal_ai.hasFinished()) return;
            KuroUtils::Log("[Stem Extractor] Exportando TODAS as faixas isoladas...");
            internal_ai.exportStems("Stems_Exportadas", 20); // Salva na subpasta
        }
        
        bool isTrackActive(int index) const {
            return internal_ai.isTrackActive(index);
        }

        bool isProcessing() const {
            return internal_ai.isRunning();
        }

        bool isFinished() const {
            return internal_ai.hasFinished();
        }

        float getProgress() const {
            return internal_ai.getProgress();
        }

        std::string getStatus() const {
            return internal_ai.getStatus();
        }

        std::string getCurrentFile() const {
            return current_file;
        }
    };

} // namespace KuroAudio
