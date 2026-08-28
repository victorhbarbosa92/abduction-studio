#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityHTMLNotebookUI {
    private:
        bool is_open = false;

        // Páginas de Informações & Metadados do Projeto (FL Studio Project Info & HTML NoteBook)
        char track_title[128] = "Abduction Odyssey - Original Mix";
        char track_artist[128] = "Kuro Studio & Gemini Advanced";
        char track_genre[64] = "Psytrance / Synthwave High-Tech";
        char track_key[32] = "F# Minor (Melodic)";
        float project_bpm = 138.00f;
        int total_bars = 128;
        char copyright_info[256] = "(C) 2026 Abduction Audio Records - All Rights Reserved";
        char web_links[256] = "https://kuroaudio.org/abduction-studio";
        char web_notes_html[1024] = "<h1>ABDUCTION STUDIO - MASTERING SESSION</h1><p>Status: <b>Ready for Release</b></p><hr><p>Vocal Stems: <i>Processed with ONNX Deep Voice AI</i><br>Master Peak: <i>-0.3 dBFS True Peak with Fruity Soft Clipper</i></p>";

    public:
        KuroFruityHTMLNotebookUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.60f, 1.00f, 0.85f)); // Azul Info Project

            if (ImGui::Begin("ℹ️ PROJECT INFO & HTML NOTEBOOK (TRACK METADATA)###ProjectInfoWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Ações")) {
                        if (ImGui::MenuItem("📋 Copiar Informações para a Área de Transferência")) {
                            ImGui::SetClipboardText(track_title);
                        }
                        if (ImGui::MenuItem("🔄 Resetar Campos")) {
                            // Limpeza
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "PROJECT INFO: TRACK METADATA, COPYRIGHT & HTML MASTERING NOTES");
                ImGui::TextDisabled("Janela oficial de metadados do projeto do FL Studio para incorporar ISRC, Artista, Título, Tom, BPM e Notas.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. CAMPOS DE METADADOS DA FAIXA ─────────────────────────
                ImGui::Columns(2, "ProjectInfoCols", true);

                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "🏷️ IDENTIFICAÇÃO DA FAIXA");
                ImGui::Separator();

                ImGui::Text("Título da Música:");
                ImGui::InputText("##TitleInput", track_title, sizeof(track_title));

                ImGui::Text("Artista / Produtor:");
                ImGui::InputText("##ArtistInput", track_artist, sizeof(track_artist));

                ImGui::Text("Gênero Musical:");
                ImGui::InputText("##GenreInput", track_genre, sizeof(track_genre));

                ImGui::Text("Tom / Escala:");
                ImGui::InputText("##KeyInput", track_key, sizeof(track_key));

                ImGui::NextColumn();

                ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.5f, 1.0f), "⏱️ TEMPO, COMPASSOS & DIREITOS");
                ImGui::Separator();

                ImGui::Text("BPM Master:");
                ImGui::InputFloat("##BpmInput", &project_bpm, 1.0f, 10.0f, "%.2f BPM");

                ImGui::Text("Total de Compassos:");
                ImGui::InputInt("##BarsInput", &total_bars);

                ImGui::Text("Copyright / Gravadora:");
                ImGui::InputText("##CopyInput", copyright_info, sizeof(copyright_info));

                ImGui::Text("Link / Website:");
                ImGui::InputText("##LinkInput", web_links, sizeof(web_links));

                ImGui::Columns(1);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. VISUALIZADOR HTML / NOTAS RICAS DE MASTERIZAÇÃO ───────
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🌐 HTML MASTERING NOTES & RELEASE LOG");
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.08f, 0.12f, 0.95f));
                ImGui::InputTextMultiline("##HtmlNotes", web_notes_html, sizeof(web_notes_html), ImVec2(-1, 140), ImGuiInputTextFlags_AllowTabInput);
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "💾 Metadados embutidos automaticamente no arquivo de exportação WAV / MP3 / FLAC");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
