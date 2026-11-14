#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "imgui.h"

namespace KuroUI {

    struct DJSkinDefinition {
        std::string skin_id = "default_obsidian";
        std::string skin_name = "Abduction Dark Obsidian";
        std::string author = "Abduction Team";
        std::string version = "2.0";

        // Cores da Interface
        ImVec4 col_background       = ImVec4(0.04f, 0.05f, 0.07f, 1.0f);
        ImVec4 col_deck_a           = ImVec4(0.00f, 0.88f, 1.00f, 1.0f); // Ciano
        ImVec4 col_deck_b           = ImVec4(1.00f, 0.65f, 0.00f, 1.0f); // Âmbar
        ImVec4 col_deck_c           = ImVec4(0.00f, 0.95f, 0.60f, 1.0f); // Verde
        ImVec4 col_deck_d           = ImVec4(0.95f, 0.20f, 0.85f, 1.0f); // Magenta
        ImVec4 col_mixer_surface    = ImVec4(0.06f, 0.08f, 0.11f, 1.0f);
        ImVec4 col_jogwheel_ring    = ImVec4(0.12f, 0.16f, 0.22f, 1.0f);
        ImVec4 col_accent_gold      = ImVec4(1.00f, 0.80f, 0.20f, 1.0f);

        // Shaders e Texturas
        bool enable_vinyl_groove_texture = true;
        bool enable_glow_bloom = true;
        bool enable_brushed_metal = false;
    };

    class DJSkinPackageImporter {
    private:
        DJSkinDefinition m_active_skin;
        std::vector<DJSkinDefinition> m_installed_skins;

        DJSkinPackageImporter() {
            // 1. Dark Obsidian (Padrão Oficial)
            m_installed_skins.push_back(m_active_skin);

            // 2. Cyberpunk Neon
            DJSkinDefinition cyber;
            cyber.skin_id = "cyberpunk_neon";
            cyber.skin_name = "Cyberpunk 2077 Neon";
            cyber.author = "NightCity Labs";
            cyber.col_background = ImVec4(0.03f, 0.03f, 0.06f, 1.0f);
            cyber.col_deck_a = ImVec4(0.00f, 0.98f, 0.88f, 1.0f); // Ciano Elétrico
            cyber.col_deck_b = ImVec4(1.00f, 0.00f, 0.45f, 1.0f); // Rosa Choque
            cyber.col_mixer_surface = ImVec4(0.07f, 0.05f, 0.12f, 1.0f);
            cyber.col_accent_gold = ImVec4(0.95f, 0.95f, 0.00f, 1.0f);
            m_installed_skins.push_back(cyber);

            // 3. Vintage Brushed Silver (Xone / Pioneer DJM-500 Style)
            DJSkinDefinition vintage;
            vintage.skin_id = "vintage_silver";
            vintage.skin_name = "Allen & Heath Vintage Silver";
            vintage.author = "Analog Heritage";
            vintage.col_background = ImVec4(0.12f, 0.13f, 0.15f, 1.0f);
            vintage.col_mixer_surface = ImVec4(0.24f, 0.25f, 0.28f, 1.0f);
            vintage.col_deck_a = ImVec4(0.85f, 0.20f, 0.20f, 1.0f); // Vermelho Analógico
            vintage.col_deck_b = ImVec4(0.20f, 0.55f, 0.85f, 1.0f); // Azul
            vintage.enable_brushed_metal = true;
            m_installed_skins.push_back(vintage);

            // 4. White Diamond
            DJSkinDefinition white;
            white.skin_id = "white_diamond";
            white.skin_name = "White Diamond Edition";
            white.author = "Pioneer Limited";
            white.col_background = ImVec4(0.88f, 0.90f, 0.92f, 1.0f);
            white.col_mixer_surface = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);
            white.col_deck_a = ImVec4(0.00f, 0.60f, 0.85f, 1.0f);
            white.col_deck_b = ImVec4(0.90f, 0.45f, 0.00f, 1.0f);
            m_installed_skins.push_back(white);
        }

    public:
        static DJSkinPackageImporter& getInstance() {
            static DJSkinPackageImporter instance;
            return instance;
        }

        const DJSkinDefinition& getActiveSkin() const { return m_active_skin; }
        const std::vector<DJSkinDefinition>& getInstalledSkins() const { return m_installed_skins; }

        void applySkin(const std::string& skin_id) {
            for (const auto& s : m_installed_skins) {
                if (s.skin_id == skin_id) {
                    m_active_skin = s;
                    std::cout << "[Skin Engine] Skin ativada: " << s.skin_name << std::endl;
                    return;
                }
            }
        }

        // Importador Drag-and-Drop de arquivo .abskin
        bool importSkinFile(const std::string& filepath) {
            std::filesystem::path p(filepath);
            if (p.extension() != ".abskin" && p.extension() != ".json") {
                return false;
            }

            std::ifstream f(filepath);
            if (!f.is_open()) return false;

            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            DJSkinDefinition custom;
            custom.skin_id = p.stem().string();
            custom.skin_name = p.stem().string();
            custom.author = "Custom Import";

            // Parsing simples de JSON para cores
            // Se encontrar a tag "name", usa como nome
            size_t name_pos = content.find("\"name\":");
            if (name_pos != std::string::npos) {
                size_t q1 = content.find("\"", name_pos + 7);
                size_t q2 = content.find("\"", q1 + 1);
                if (q1 != std::string::npos && q2 != std::string::npos) {
                    custom.skin_name = content.substr(q1 + 1, q2 - q1 - 1);
                }
            }

            m_installed_skins.push_back(custom);
            m_active_skin = custom;
            std::cout << "[Skin Engine] Skin customizada importada com sucesso: " << custom.skin_name << std::endl;
            return true;
        }
    };

} // namespace KuroUI

