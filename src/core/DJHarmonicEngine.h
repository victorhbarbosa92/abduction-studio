#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <cctype>
#include <unordered_map>

#ifndef IM_COL32
#define IM_COL32(R,G,B,A) (((unsigned int)(A)<<24) | ((unsigned int)(B)<<16) | ((unsigned int)(G)<<8) | ((unsigned int)(R)))
#endif

#ifdef RELATIVE
#undef RELATIVE
#endif

namespace KuroAudio {

    enum class HarmonicCompatibility {
        PERFECT = 0,        // Mesma chave exata (ex: 8A <-> 8A)
        HARMONIC_RELATIVE,  // Relativo Maior/Menor (ex: 8A <-> 8B)
        ENERGY_PLUS_1,      // +1 no Camelot (ex: 8A -> 9A) - Leve aumento de energia
        ENERGY_MINUS_1,     // -1 no Camelot (ex: 8A -> 7A) - Leve relaxamento
        ENERGY_BOOST_2,     // +2 no Camelot (ex: 8A -> 10A) - Energy Boost dramático
        ENERGY_DROP_2,      // -2 no Camelot (ex: 8A -> 6A) - Energy Drop
        DIAGONAL_PLUS_1,    // +1 e troca de modo (ex: 8A -> 9B)
        DIAGONAL_MINUS_1,   // -1 e troca de modo (ex: 8A -> 7B)
        CLASH               // Chaves distantes / Incompatíveis harmonicamente
    };

    struct CamelotKey {
        int number = 8;         // 1 a 12
        char letter = 'A';      // 'A' = Menor (Minor), 'B' = Maior (Major)
        std::string raw_str = "8A / Am";
        std::string musical_key = "Am";
        bool valid = true;

        std::string getCamelotCode() const {
            return std::to_string(number) + letter;
        }

        std::string getFullFormatted() const {
            return getCamelotCode() + " (" + musical_key + ")";
        }
    };

    class DJHarmonicEngine {
    public:
        // Mapeamento Oficial do Camelot Wheel (Mixed In Key / Rekordbox)
        // 1A = Abm, 2A = Ebm, 3A = Bbm, 4A = Fm, 5A = Cm, 6A = Gm, 7A = Dm, 8A = Am, 9A = Em, 10A = Bm, 11A = F#m, 12A = C#m
        // 1B = B,   2B = F#,  3B = Db,  4B = Ab, 5B = Eb, 6B = Bb, 7B = F,  8B = C,  9B = G,  10B = D,  11B = A,   12B = E
        static CamelotKey parseCamelotKey(const std::string& input) { return parseKey(input); }
        static CamelotKey parseKey(const std::string& input) {
            CamelotKey ck;
            if (input.empty()) {
                ck.valid = false;
                return ck;
            }

            std::string s = input;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::toupper(c); });

            // 1. Tenta identificar notação Camelot direta: "8A", "08A", "9B", "11A", "8A / AM", etc.
            for (int num = 12; num >= 1; num--) {
                std::string s_num = std::to_string(num);
                size_t p = s.find(s_num);
                if (p != std::string::npos) {
                    // Confere se o caractere seguinte é A ou B
                    size_t letter_pos = p + s_num.length();
                    if (letter_pos < s.length()) {
                        char c = s[letter_pos];
                        if (c == 'A' || c == 'B') {
                            ck.number = num;
                            ck.letter = c;
                            ck.musical_key = getMusicalKeyFromCamelot(ck.number, ck.letter);
                            ck.raw_str = input;
                            ck.valid = true;
                            return ck;
                        }
                    }
                }
            }

            // 2. Mapeamento por tonalidade musical tradicional (ex: "AM", "A MINOR", "C MAJOR", "F#M")
            static const std::unordered_map<std::string, std::pair<int, char>> key_map = {
                // Menores (Minor - 'A')
                { "ABM", {1, 'A'} }, { "G#M", {1, 'A'} }, { "G# MINOR", {1, 'A'} }, { "AB MINOR", {1, 'A'} },
                { "EBM", {2, 'A'} }, { "D#M", {2, 'A'} }, { "D# MINOR", {2, 'A'} }, { "EB MINOR", {2, 'A'} },
                { "BBM", {3, 'A'} }, { "A#M", {3, 'A'} }, { "A# MINOR", {3, 'A'} }, { "BB MINOR", {3, 'A'} },
                { "FM",  {4, 'A'} }, { "F MINOR", {4, 'A'} },
                { "CM",  {5, 'A'} }, { "C MINOR", {5, 'A'} },
                { "GM",  {6, 'A'} }, { "G MINOR", {6, 'A'} },
                { "DM",  {7, 'A'} }, { "D MINOR", {7, 'A'} },
                { "AM",  {8, 'A'} }, { "A MINOR", {8, 'A'} },
                { "EM",  {9, 'A'} }, { "E MINOR", {9, 'A'} },
                { "BM",  {10, 'A'} }, { "B MINOR", {10, 'A'} },
                { "F#M", {11, 'A'} }, { "GBM", {11, 'A'} }, { "F# MINOR", {11, 'A'} }, { "GB MINOR", {11, 'A'} },
                { "C#M", {12, 'A'} }, { "DBM", {12, 'A'} }, { "C# MINOR", {12, 'A'} }, { "DB MINOR", {12, 'A'} },

                // Maiores (Major - 'B')
                { "B",   {1, 'B'} }, { "B MAJOR", {1, 'B'} },
                { "F#",  {2, 'B'} }, { "GB", {2, 'B'} }, { "F# MAJOR", {2, 'B'} }, { "GB MAJOR", {2, 'B'} },
                { "DB",  {3, 'B'} }, { "C#", {3, 'B'} }, { "DB MAJOR", {3, 'B'} }, { "C# MAJOR", {3, 'B'} },
                { "AB",  {4, 'B'} }, { "G#", {4, 'B'} }, { "AB MAJOR", {4, 'B'} }, { "G# MAJOR", {4, 'B'} },
                { "EB",  {5, 'B'} }, { "D#", {5, 'B'} }, { "EB MAJOR", {5, 'B'} }, { "D# MAJOR", {5, 'B'} },
                { "BB",  {6, 'B'} }, { "A#", {6, 'B'} }, { "BB MAJOR", {6, 'B'} }, { "A# MAJOR", {6, 'B'} },
                { "F",   {7, 'B'} }, { "F MAJOR", {7, 'B'} },
                { "C",   {8, 'B'} }, { "C MAJOR", {8, 'B'} },
                { "G",   {9, 'B'} }, { "G MAJOR", {9, 'B'} },
                { "D",   {10, 'B'} }, { "D MAJOR", {10, 'B'} },
                { "A",   {11, 'B'} }, { "A MAJOR", {11, 'B'} },
                { "E",   {12, 'B'} }, { "E MAJOR", {12, 'B'} }
            };

            // Remove caracteres especiais e busca no mapa
            std::string clean = "";
            for (char c : s) {
                if (std::isalnum((unsigned char)c) || c == '#' || c == ' ') clean += c;
            }
            while (!clean.empty() && clean.front() == ' ') clean.erase(clean.begin());
            while (!clean.empty() && clean.back() == ' ') clean.pop_back();

            auto it = key_map.find(clean);
            if (it != key_map.end()) {
                ck.number = it->second.first;
                ck.letter = it->second.second;
                ck.musical_key = getMusicalKeyFromCamelot(ck.number, ck.letter);
                ck.raw_str = input;
                ck.valid = true;
                return ck;
            }

            // Fallback default
            ck.number = 8;
            ck.letter = 'A';
            ck.musical_key = "Am";
            ck.raw_str = input;
            ck.valid = false;
            return ck;
        }

        static std::string getMusicalKeyFromCamelot(int number, char letter) {
            if (letter == 'A') { // Minor
                switch (number) {
                    case 1: return "Abm";
                    case 2: return "Ebm";
                    case 3: return "Bbm";
                    case 4: return "Fm";
                    case 5: return "Cm";
                    case 6: return "Gm";
                    case 7: return "Dm";
                    case 8: return "Am";
                    case 9: return "Em";
                    case 10: return "Bm";
                    case 11: return "F#m";
                    case 12: return "C#m";
                }
            } else { // Major
                switch (number) {
                    case 1: return "B";
                    case 2: return "F#";
                    case 3: return "Db";
                    case 4: return "Ab";
                    case 5: return "Eb";
                    case 6: return "Bb";
                    case 7: return "F";
                    case 8: return "C";
                    case 9: return "G";
                    case 10: return "D";
                    case 11: return "A";
                    case 12: return "E";
                }
            }
            return "Am";
        }

        static std::string getRekordboxMusicalNotation(int number, char letter) {
            if (letter == 'A') { // Minor
                switch (number) {
                    case 1: return "Abm";
                    case 2: return "Ebm";
                    case 3: return "Bbm";
                    case 4: return "Fm";
                    case 5: return "Cm";
                    case 6: return "Gm";
                    case 7: return "Dm";
                    case 8: return "Am";
                    case 9: return "Em";
                    case 10: return "Bm";
                    case 11: return "F#m";
                    case 12: return "C#m";
                }
            } else { // Major
                switch (number) {
                    case 1: return "B";
                    case 2: return "F#";
                    case 3: return "Db";
                    case 4: return "Ab";
                    case 5: return "Eb";
                    case 6: return "Bb";
                    case 7: return "F";
                    case 8: return "C";
                    case 9: return "G";
                    case 10: return "D";
                    case 11: return "A";
                    case 12: return "E";
                }
            }
            return "Am";
        }

        static std::string formatKeyForDisplay(const std::string& input, bool use_camelot) {
            CamelotKey ck = parseKey(input);
            if (!ck.valid) return "-";
            if (use_camelot) {
                return ck.getCamelotCode();
            } else {
                return getRekordboxMusicalNotation(ck.number, ck.letter);
            }
        }

        // Comparação Harmônica com o Camelot Wheel
        static HarmonicCompatibility compareKeys(const CamelotKey& base, const CamelotKey& target) {
            if (!base.valid || !target.valid) return HarmonicCompatibility::CLASH;

            int diff = (target.number - base.number);
            if (diff > 6) diff -= 12;
            if (diff < -6) diff += 12;

            if (base.letter == target.letter) {
                if (diff == 0) return HarmonicCompatibility::PERFECT;
                if (diff == 1) return HarmonicCompatibility::ENERGY_PLUS_1;
                if (diff == -1) return HarmonicCompatibility::ENERGY_MINUS_1;
                if (diff == 2) return HarmonicCompatibility::ENERGY_BOOST_2;
                if (diff == -2) return HarmonicCompatibility::ENERGY_DROP_2;
            } else {
                if (diff == 0) return HarmonicCompatibility::HARMONIC_RELATIVE;
                if (diff == 1) return HarmonicCompatibility::DIAGONAL_PLUS_1;
                if (diff == -1) return HarmonicCompatibility::DIAGONAL_MINUS_1;
            }

            return HarmonicCompatibility::CLASH;
        }

        static HarmonicCompatibility compareKeys(const std::string& base_str, const std::string& target_str) {
            return compareKeys(parseKey(base_str), parseKey(target_str));
        }

        // Transpõe chave Camelot por semitons (-12 a +12)
        static CamelotKey transposeKey(const CamelotKey& base, int semitones) {
            if (!base.valid) return base;
            CamelotKey res = base;
            int s = semitones % 12;
            if (s < 0) s += 12;
            // No círculo das quintas do Camelot, cada semitom para cima move o número em +7 mod 12
            int new_num = ((base.number - 1 + s * 7) % 12) + 1;
            res.number = new_num;
            res.musical_key = getMusicalKeyFromCamelot(res.number, res.letter);
            res.raw_str = res.getCamelotCode() + " / " + res.musical_key;
            return res;
        }

        // Calcula a transposição ótima em semitons (-6 a +6) para sincronizar harmonicamente (Key Sync)
        static int getOptimalKeySyncShift(const std::string& base_master_str, const std::string& target_slave_str) {
            CamelotKey master = parseKey(base_master_str);
            CamelotKey slave = parseKey(target_slave_str);
            if (!master.valid || !slave.valid) return 0;

            int best_shift = 0;
            int best_score = -1;

            // Busca o menor deslocamento em semitons (-6 a +6) que maximize a pontuação harmônica
            for (int s = 0; s <= 6; s++) {
                int candidates[2] = { s, -s };
                for (int i = 0; i < (s == 0 ? 1 : 2); i++) {
                    int shift = candidates[i];
                    CamelotKey shifted_slave = transposeKey(slave, shift);
                    HarmonicCompatibility comp = compareKeys(master, shifted_slave);
                    int score = 0;
                    if (comp == HarmonicCompatibility::PERFECT) score = 100 - std::abs(shift);
                    else if (comp == HarmonicCompatibility::HARMONIC_RELATIVE) score = 95 - std::abs(shift);
                    else if (comp == HarmonicCompatibility::ENERGY_PLUS_1) score = 85 - std::abs(shift);
                    else if (comp == HarmonicCompatibility::ENERGY_MINUS_1) score = 80 - std::abs(shift);
                    else score = 20 - std::abs(shift);

                    if (score > best_score) {
                        best_score = score;
                        best_shift = shift;
                    }
                }
            }
            return best_shift;
        }

        // Retorna score de harmonia de 0 a 100
        static int getCompatibilityScore(const std::string& base_str, const std::string& target_str) {
            HarmonicCompatibility comp = compareKeys(base_str, target_str);
            switch (comp) {
                case HarmonicCompatibility::PERFECT: return 100;
                case HarmonicCompatibility::HARMONIC_RELATIVE: return 95;
                case HarmonicCompatibility::ENERGY_PLUS_1: return 90;
                case HarmonicCompatibility::ENERGY_MINUS_1: return 85;
                case HarmonicCompatibility::ENERGY_BOOST_2: return 75;
                case HarmonicCompatibility::DIAGONAL_PLUS_1: return 70;
                case HarmonicCompatibility::DIAGONAL_MINUS_1: return 65;
                case HarmonicCompatibility::ENERGY_DROP_2: return 60;
                case HarmonicCompatibility::CLASH: default: return 20;
            }
        }

        static bool isHarmonicallyCompatible(const std::string& base_str, const std::string& target_str) {
            HarmonicCompatibility comp = compareKeys(base_str, target_str);
            return (comp != HarmonicCompatibility::CLASH && comp != HarmonicCompatibility::ENERGY_DROP_2);
        }

        // Texto do Badge de Compatibilidade para a UI
        static std::string getCompatibilityBadgeText(const std::string& base_str, const std::string& target_str) {
            HarmonicCompatibility comp = compareKeys(base_str, target_str);
            switch (comp) {
                case HarmonicCompatibility::PERFECT:           return "PERFEITO (100%)";
                case HarmonicCompatibility::HARMONIC_RELATIVE:  return "RELATIVO (95%)";
                case HarmonicCompatibility::ENERGY_PLUS_1:     return "+1 ENERGIA";
                case HarmonicCompatibility::ENERGY_MINUS_1:    return "-1 FLUIDO";
                case HarmonicCompatibility::ENERGY_BOOST_2:    return "+2 BOOST ⚡";
                case HarmonicCompatibility::DIAGONAL_PLUS_1:   return "DIAGONAL +1";
                case HarmonicCompatibility::DIAGONAL_MINUS_1:  return "DIAGONAL -1";
                case HarmonicCompatibility::ENERGY_DROP_2:     return "-2 DROP";
                case HarmonicCompatibility::CLASH: default:    return "DIVERGENTE";
            }
        }

        // Cor de Destaque RGBA para o Badge de Compatibilidade
        static unsigned int getCompatibilityBadgeColor(const std::string& base_str, const std::string& target_str) {
            HarmonicCompatibility comp = compareKeys(base_str, target_str);
            switch (comp) {
                case HarmonicCompatibility::PERFECT:           return IM_COL32(0, 255, 128, 255);  // Verde Esmeralda
                case HarmonicCompatibility::HARMONIC_RELATIVE:  return IM_COL32(0, 225, 255, 255);  // Ciano Neon
                case HarmonicCompatibility::ENERGY_PLUS_1:  return IM_COL32(255, 205, 0, 255);  // Ouro / Ambar
                case HarmonicCompatibility::ENERGY_MINUS_1: return IM_COL32(0, 180, 255, 255);  // Azul Céu
                case HarmonicCompatibility::ENERGY_BOOST_2: return IM_COL32(255, 120, 0, 255);  // Laranja Elétrico
                case HarmonicCompatibility::DIAGONAL_PLUS_1:return IM_COL32(200, 130, 255, 255);// Violeta
                case HarmonicCompatibility::DIAGONAL_MINUS_1:return IM_COL32(140, 160, 255, 255);
                case HarmonicCompatibility::ENERGY_DROP_2:  return IM_COL32(255, 100, 100, 255);
                case HarmonicCompatibility::CLASH: default: return IM_COL32(110, 125, 140, 180);// Cinza escuro
            }
        }

        // Cor Oficial do Camelot Wheel por Número (1 a 12)
        static unsigned int getCamelotWheelColor(int number) {
            switch (number) {
                case 1:  return IM_COL32(255, 55, 120, 255); // Rosa Neon
                case 2:  return IM_COL32(255, 60, 50, 255);  // Vermelho Coral
                case 3:  return IM_COL32(255, 110, 20, 255); // Laranja Intenso
                case 4:  return IM_COL32(255, 165, 0, 255);  // Ambar
                case 5:  return IM_COL32(255, 215, 0, 255);  // Ouro / Amarelo
                case 6:  return IM_COL32(170, 255, 0, 255);  // Lima
                case 7:  return IM_COL32(0, 255, 100, 255);  // Verde Menta
                case 8:  return IM_COL32(0, 255, 220, 255);  // Ciano / Turquesa
                case 9:  return IM_COL32(0, 180, 255, 255);  // Azul Claro
                case 10: return IM_COL32(40, 110, 255, 255); // Azul Royal
                case 11: return IM_COL32(140, 60, 255, 255); // Roxo / Indigo
                case 12: return IM_COL32(220, 40, 255, 255); // Magenta
                default: return IM_COL32(0, 255, 200, 255);
            }
        }
    };

} // namespace KuroAudio
