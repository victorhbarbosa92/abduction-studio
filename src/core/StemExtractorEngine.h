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
            if (!internal_ai.hasFinished()) {
                KuroUtils::Log("[Stem Extractor] Aguarde a separação terminar antes de exportar.");
                return;
            }
            // Chama uma função interna para salvar
            // Simulando o processo que criaria "Kick_Isolated.wav"
            KuroUtils::Log("[Stem Extractor] Exportando Stem Isolada da faixa " + std::to_string(track_index) + " para a pasta raiz.");
            internal_ai.exportStems(".", 4); // Por enquanto exporta tudo para a raiz
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
