#pragma once
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace KuroAI {

    class KuroDatabase {
    public:
        // Busca instantânea (Sem IA) no banco de dados local
        static bool fetchMetadata(const std::string& filepath, float& out_bpm, std::string& out_key) {
            std::string lower_path = filepath;
            std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), 
                           [](unsigned char c){ return std::tolower(c); });
            
            // Simulação de um banco de dados integrado (ID3/MusicBrainz Cache)
            // Na vida real, usaríamos libcurl + nlohmann/json para bater na API do Spotify/MusicBrainz
            if (lower_path.find("billie jean") != std::string::npos || lower_path.find("michael jackson") != std::string::npos) {
                out_bpm = 117.0f;
                out_key = "F# Menor";
                std::cout << "[KuroDatabase] HIT! Metadados encontrados instantaneamente via Cache nativo." << std::endl;
                return true;
            }
            if (lower_path.find("smells like teen spirit") != std::string::npos || lower_path.find("nirvana") != std::string::npos) {
                out_bpm = 116.0f;
                out_key = "F Menor";
                std::cout << "[KuroDatabase] HIT! Metadados encontrados instantaneamente via Cache nativo." << std::endl;
                return true;
            }
            if (lower_path.find("get lucky") != std::string::npos || lower_path.find("daft punk") != std::string::npos) {
                out_bpm = 116.0f;
                out_key = "F# Menor";
                std::cout << "[KuroDatabase] HIT! Metadados encontrados instantaneamente via Cache nativo." << std::endl;
                return true;
            }
            
            // Tentativa de ler a tag do arquivo bruto (Fallback determinístico)
            // Se o arquivo tiver o BPM no nome, ex: "loop_128bpm.wav"
            size_t bpm_pos = lower_path.find("bpm");
            if (bpm_pos != std::string::npos && bpm_pos >= 3) {
                std::string num_str = lower_path.substr(bpm_pos - 3, 3);
                try {
                    out_bpm = std::stof(num_str);
                    out_key = "C Maior"; // Fallback padrão
                    std::cout << "[KuroDatabase] HIT! BPM extraído da Tag/Nome do Arquivo." << std::endl;
                    return true;
                } catch (...) {
                    // ignorar erro
                }
            }

            // Não encontrado no DB, precisa cair para o KuroMIR (Mas sem usar IA pesada, apenas matemática)
            std::cout << "[KuroDatabase] Arquivo não encontrado no Cache nativo. Usando KuroMIR Matemático." << std::endl;
            return false;
        }
    };

}
