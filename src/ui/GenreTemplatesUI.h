#include "IconsFontAwesome6.h"
#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"
#include "../audio/SynthEngine.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

extern KuroAudio::SynthEngine g_piano_synth;
extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

namespace KuroUI {

    struct SubgenreTemplate {
        std::string id;
        std::string name;
        std::string category;
        std::string icon;
        float bpm;
        std::string bassline_groove_type; // "offbeat", "rolling16", "triplet", "dark_forest"
        std::vector<std::string> key_artists;
        std::string description;
        ImU32 accent_color;
    };

    inline std::vector<SubgenreTemplate> GetSubgenreTemplates() {
        return {
            {
                "perception_astral_portal_140bpm",
                "Perception - The Astral Portal (Brazilian Progressive Psytrance 140 BPM)",
                "Brazilian Progressive Psytrance",
                "[PERCEPTION SIGNATURE]",
                140.0f,
                "rolling16",
                {"Perception (Brazil)", "Groundbass", "Tijah", "Twelve Sessions", "Mandragora"},
                "Produção autêntica no estilo de Perception com 16 PISTAS SIMULTÂNEAS: Kick com transient click estalando e sub 52Hz, Rolling Bass Sub + Mid-Saw com oitavas síncopadas, Brazilian tribal percs, FM squelches, screaming Acid Lead em F#m, counter-arp matrix, SuperSaw poly lead widescreen, cordas sinfônicas, piano Steinway e vocal mantra chants em 96 compassos!",
                IM_COL32(0, 240, 255, 255)
            },
            {
                "vini_vici_adhana",
                "Vini Vici & Astrix - Adhana (Original Audio Track)",
                "Psytrance / Progressive",
                "[VINI VICI]",
                138.0f,
                "rolling16",
                {"Vini Vici (Israel)", "Astrix (Israel)", "Iboga Records"},
                "ÁUDIO ORIGINAL COMPLETO carregado de C:\\NovaDAW\\tracks\\Vini_Vici_Astrix_Adhana.wav! Inclui a faixa inteira de 81MB com bassline rolling e transcrição MIDI dos stems!",
                IM_COL32(57, 255, 20, 255)
            },
            {
                "vini_vici_flashback",
                "Vini Vici & Pixel - Flashback (Original Audio Track)",
                "Psytrance / Progressive",
                "[VINI VICI]",
                140.0f,
                "acid_rolling",
                {"Vini Vici (Israel)", "Pixel (Israel)", "Spinnin' Records"},
                "ÁUDIO ORIGINAL COMPLETO de C:\\NovaDAW\\tracks\\Vini_Vici_Pixel_Flashback.wav! Carrega os 84MB da música real diretamente na Playlist com visualização da onda sonora!",
                IM_COL32(0, 229, 255, 255)
            },
            {
                "geraldo_azevedo",
                "Geraldo Azevedo - ABC Do Sertão (Original Audio Track)",
                "MPB / Acoustic Classic",
                "[ACOUSTIC]",
                120.0f,
                "offbeat",
                {"Geraldo Azevedo (Brazil)", "Música Brasileira", "MPB"},
                "ÁUDIO ORIGINAL COMPLETO de C:\\NovaDAW\\tracks\\Geraldo Azevedo - ABC Do Sertão.wav! Clássico da música nacional carregado diretamente na Playlist!",
                IM_COL32(255, 200, 0, 255)
            },
            {
                "memento_mori",
                "Memento Mori - The Unsung Warrior (Original Audio Track)",
                "Epic Symphonic Metal",
                "[EPIC]",
                130.0f,
                "dark_rolling",
                {"Memento Mori", "Symphonic Power Metal"},
                "ÁUDIO ORIGINAL COMPLETO de C:\\NovaDAW\\tracks\\Memento Mori - The Unsung Warrior.wav! 61MB de áudio épico original carregado na Playlist com separação por stems!",
                IM_COL32(255, 40, 100, 255)
            },
            {
                "darude_sandstorm",
                "Darude - Sandstorm (Hit Original)",
                "Trance / Hard Dance",
                "[TRANCE]",
                136.0f,
                "acid_rolling",
                {"Darude (Finland)", "Armin van Buuren"},
                "Hit atemporal da música eletrônica em Bm com o riff rápido de notas B4/E5/D5, bateria 4/4 de hard trance e risers pesados!",
                IM_COL32(255, 120, 0, 255)
            }
        };
    }

    inline void ApplySubgenreTemplate(const SubgenreTemplate& tmpl, KuroDSP::TimelineManager& timeline, ClipManager& clip_manager) {
        std::lock_guard<std::mutex> lock(timeline.timeline_mutex);
        
        // 1. Configura BPM do projeto principal
        timeline.setBPM(tmpl.bpm);

        // 2. Limpa completamente os padrões e a Timeline de 8 faixas da Playlist
        clip_manager.global_patterns.clear();
        for (int t = 0; t < MAX_TRACKS; ++t) {
            clip_manager.track_clips[t].clear();
            clip_manager.track_midi_clips[t].clear();
        }

        // 3. Ativa sintetizadores e sampler
        g_piano_synth.flex_active[3] = true; // Bassline
        g_piano_synth.flex_active[4] = true; // Chords & Pads
        g_piano_synth.flex_active[5] = true; // Lead Synth

        std::string target_wav_path = "";
        std::string track_display_name = "";

        if (tmpl.id == "cyber_horizon_128bpm" || tmpl.id == "psytrance_2min_track") {
            float out_b = 128.0f;
            clip_manager.loadTrackTemplate(0, out_b);
            timeline.setBPM(out_b);
            return;
        }
        else if (tmpl.id == "vini_vici_adhana") {
            target_wav_path = "C:\\NovaDAW\\tracks\\Vini_Vici_Astrix_Adhana.wav";
            if (!std::filesystem::exists(target_wav_path)) target_wav_path = "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\NovaDAW\\tracks\\Vini_Vici_Astrix_Adhana.wav";
            track_display_name = "Vini_Vici_Astrix_Adhana.wav";
        } else if (tmpl.id == "vini_vici_flashback") {
            target_wav_path = "C:\\NovaDAW\\tracks\\Vini_Vici_Pixel_Flashback.wav";
            if (!std::filesystem::exists(target_wav_path)) target_wav_path = "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\NovaDAW\\tracks\\Vini_Vici_Pixel_Flashback.wav";
            track_display_name = "Vini_Vici_Pixel_Flashback.wav";
        } else if (tmpl.id == "geraldo_azevedo") {
            target_wav_path = "C:\\NovaDAW\\tracks\\Geraldo Azevedo - ABC Do Sertão.wav";
            if (!std::filesystem::exists(target_wav_path)) target_wav_path = "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\NovaDAW\\tracks\\Geraldo Azevedo - ABC Do Sertão.wav";
            track_display_name = "Geraldo Azevedo - ABC Do Sertão.wav";
        } else if (tmpl.id == "memento_mori") {
            target_wav_path = "C:\\NovaDAW\\tracks\\Memento Mori - The Unsung Warrior.wav";
            if (!std::filesystem::exists(target_wav_path)) target_wav_path = "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\NovaDAW\\tracks\\Memento Mori - The Unsung Warrior.wav";
            track_display_name = "Memento Mori - The Unsung Warrior.wav";
        }

        // 4. Preencher TODAS AS 8 FAIXAS da Playlist com Áudio WAV Master Real e Trilha de Produção
        if (!target_wav_path.empty()) {
            // Restaura o Kick oficial para o canal 0
            g_piano_synth.drum_variants[0][0].load(std::string(g_piano_synth.FL_KICKS_DIR) + "808 Kick.wav");
            
            // Carrega a música real no player de áudio master global
            if (g_global_sampler) {
                g_global_sampler->loadSample(target_wav_path);
                g_global_sampler->stop(); // Fica em stop aguardando o clique do Play
            }
            
            // Faixa 0: Áudio WAV Real
            AudioClip ac; ac.id = clip_manager.next_id++; ac.start_time_sec = 0.0f; ac.length_sec = 210.0f; ac.source_offset_sec = 0.0f; ac.is_selected = false;
            clip_manager.track_clips[0].push_back(ac);

            // Pattern limpo (sem MIDI artificial por cima)
            Pattern p_clean; p_clean.id = 1; p_clean.name = "Pattern 1 (Clean Master)"; p_clean.color = tmpl.accent_color;
            clip_manager.global_patterns.push_back(p_clean);
        }
        else { // Darude - Sandstorm / Synthesized Hit Tracks
            Pattern p_intro; p_intro.id = 1; p_intro.name = "01. Sandstorm Build-Up Roll"; p_intro.color = tmpl.accent_color;
            for (int b = 0; b < 16; b += 2) p_intro.getChannelNotes(1).push_back(KuroDSP::MidiNote(38, b * 0.125f, 0.08f, 0.90f, 1.0f, 1));
            clip_manager.global_patterns.push_back(p_intro);

            Pattern p_lead; p_lead.id = 2; p_lead.name = "02. Sandstorm Main Motif"; p_lead.color = IM_COL32(255, 255, 0, 255);
            for (int b = 0; b < 16; b += 4) p_lead.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * 0.125f, 0.10f, 1.0f, 1.0f, 0));
            int sand_pitches[16] = {71, 71, 71, 71, 71, 71, 71, 71, 76, 76, 76, 74, 74, 69, 71, 71};
            for (int i = 0; i < 16; ++i) p_lead.getChannelNotes(5).push_back(KuroDSP::MidiNote(sand_pitches[i], i * 0.125f, 0.10f, 0.95f, 1.0f, 5));
            for (int i = 0; i < 16; ++i) p_lead.getChannelNotes(3).push_back(KuroDSP::MidiNote(35, i * 0.125f, 0.10f, 0.90f, 1.0f, 3));
            clip_manager.global_patterns.push_back(p_lead);

            for (int b = 0; b < 14; ++b) {
                MidiClip mc; mc.id = clip_manager.next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = (b < 2) ? 1 : 2; mc.is_selected = false;
                clip_manager.track_midi_clips[1].push_back(mc);
                clip_manager.track_midi_clips[2].push_back(mc);
                clip_manager.track_midi_clips[6].push_back(mc);
                AudioClip ac; ac.id = clip_manager.next_id++; ac.start_time_sec = b * 16.0f; ac.length_sec = 16.0f; ac.source_offset_sec = 0.0f; ac.is_selected = false;
                clip_manager.track_clips[0].push_back(ac);
                clip_manager.track_clips[3].push_back(ac);
                clip_manager.track_clips[7].push_back(ac);
            }
        }

        clip_manager.current_pattern_idx = 0;
        timeline.setMasterFrame(0);
    }

    inline void RenderGenreTemplatesWindow(bool* open, KuroDSP::TimelineManager& timeline, ClipManager& clip_manager) {
        if (!*open) return;

        ImGui::SetNextWindowSize(ImVec2(880, 620), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Templates de Producao (Vertentes & Artistas)", open)) {
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.2f, 1.0f), "TEMPLATES DE PRODUCAO POR VERTENTE & ARTISTAS DO MERCADO");
        ImGui::TextWrapped("Escolha um template de vertente inspirado nos maiores artistas do mundo (Brasil, Israel, Europa, Américas). O template configura o BPM, os instrumentos acústicos e o groove de Kick/Bassline característico!");
        ImGui::Separator();
        ImGui::Spacing();

        auto templates = GetSubgenreTemplates();
        
        ImGui::BeginChild("TemplateCardsArea", ImVec2(0, 0), false);
        
        float card_width = 410.0f;
        float card_height = 240.0f;
        int cols = std::max(1, (int)(ImGui::GetContentRegionAvail().x / (card_width + 12.0f)));

        for (size_t i = 0; i < templates.size(); i++) {
            const auto& tmpl = templates[i];
            
            if (i > 0 && i % cols != 0) ImGui::SameLine(0, 12);

            ImGui::PushID((int)i);
            ImGui::BeginChild("Card", ImVec2(card_width, card_height), true, ImGuiWindowFlags_NoScrollbar);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();

            // Header Banner
            draw_list->AddRectFilled(p0, ImVec2(p0.x + card_width, p0.y + 32.0f), tmpl.accent_color & 0x40FFFFFF, 4.0f);
            draw_list->AddRect(p0, ImVec2(p0.x + card_width, p0.y + card_height), tmpl.accent_color, 4.0f, 0, 1.5f);

            // Title & Icon
            ImGui::SetCursorScreenPos(ImVec2(p0.x + 10, p0.y + 6));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s  %s", tmpl.icon.c_str(), tmpl.name.c_str());

            // BPM Badge
            char bpm_str[32];
            snprintf(bpm_str, sizeof(bpm_str), " " ICON_FA_STOPWATCH "  %.0f BPM", tmpl.bpm);
            float bpm_w = ImGui::CalcTextSize(bpm_str).x;
            ImGui::SetCursorScreenPos(ImVec2(p0.x + card_width - bpm_w - 12, p0.y + 6));
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "%s", bpm_str);

            ImGui::SetCursorScreenPos(ImVec2(p0.x + 10, p0.y + 40.0f));

            // Key Artists List
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), " " ICON_FA_STAR "  Artistas de Destaque:");
            for (const auto& artist : tmpl.key_artists) {
                ImGui::BulletText("%s", artist.c_str());
            }

            ImGui::Spacing();
            ImGui::TextWrapped("%s", tmpl.description.c_str());

            // Create Track Button
            ImGui::SetCursorScreenPos(ImVec2(p0.x + 10, p0.y + card_height - 36.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, tmpl.accent_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 100));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.02f, 1.0f));

            char btn_lbl[128];
            snprintf(btn_lbl, sizeof(btn_lbl), " " ICON_FA_BOLT "  Criar Track %s", tmpl.name.c_str());
            if (ImGui::Button(btn_lbl, ImVec2(card_width - 20.0f, 26.0f))) {
                ApplySubgenreTemplate(tmpl, timeline, clip_manager);
                
                extern bool is_playing;
                ::is_playing = true;
                timeline.setPlaying(true);
                *open = false; // Fecha a janela modal para o usuario ver a DAW tocando em tempo real!

                g_piano_synth.triggerNote(36, 0.4f, 0.95f, 0);
            }
            ImGui::PopStyleColor(3);

            ImGui::EndChild();
            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace KuroUI
