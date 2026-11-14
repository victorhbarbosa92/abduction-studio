#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "UIScaleManager.h"
#include "../core/DJAudioDeviceManager.h"
#include "../core/DJAudioRouting.h"

namespace KuroUI {

    class DJAudioRoutingModal {
    public:
        static void render() {
            auto& devMgr = KuroAudio::DJAudioDeviceManager::getInstance();
            if (!devMgr.show_routing_modal) return;

            ImGui::SetNextWindowSize(ImVec2(UIScaleManager::Scale(680.0f), UIScaleManager::Scale(580.0f)), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.12f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.95f, 0.60f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));

            if (ImGui::Begin("CONFIGURAÇÃO DE SAÍDAS DE ÁUDIO // MESTRE & RETORNO##AudioModal", &devMgr.show_routing_modal, flags)) {

                // Cabeçalho com Ícone e Descrição Intuitiva
                ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), ICON_FA_SLIDERS " ROTEAMENTO INTELIGENTE DE HARDWARE");
                ImGui::SameLine();
                ImGui::TextDisabled("| Auto-detecção por Computador");

                ImGui::Separator();
                ImGui::Spacing();

                // Banner Informativo do Computador Atual
                size_t num_devs = devMgr.output_devices.size();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.12f, 0.18f, 0.70f));
                ImGui::BeginChild("##DeviceSummary", ImVec2(0, UIScaleManager::Scale(46.0f)), true);
                ImGui::TextColored(ImVec4(0.40f, 0.90f, 1.0f, 1.0f), ICON_FA_DESKTOP " Dispositivos Detectados Neste PC: %d", (int)num_devs);
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - UIScaleManager::Scale(180.0f));
                if (ImGui::Button(ICON_FA_ROTATE " Re-escanear Placas", ImVec2(UIScaleManager::Scale(170.0f), UIScaleManager::Scale(26.0f)))) {
                    devMgr.rescanDevices();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Recarregar a lista de placas de som físicas conectadas agora (USB, Fones, HDMI)");
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // =====================================================================
                // 1. ESCOLHA DO MODO DE OPERAÇÃO
                // =====================================================================
                ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.20f, 1.0f), ICON_FA_LAYER_GROUP " 1. COMO VOCÊ DESEJA OUVIR?");
                ImGui::Spacing();

                int current_mode = (int)devMgr.routing_mode;

                // Card 1: Duas Placas (Master em uma, Fone em outra)
                bool is_dual = (current_mode == (int)KuroAudio::DJRoutingMode::DUAL_DEVICE);
                if (ImGui::RadioButton("DUAS PLACAS DE SOM (Recomendado)", is_dual)) {
                    devMgr.routing_mode = KuroAudio::DJRoutingMode::DUAL_DEVICE;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("- Caixas no PC / Alto-falantes e Fone no USB / Bluetooth");

                // Card 2: Placa Multicanal 4CH (Controladora DJ)
                bool is_multi = (current_mode == (int)KuroAudio::DJRoutingMode::MULTI_CHANNEL_4CH);
                if (ImGui::RadioButton("PLACA MULTICANAL 4CH", is_multi)) {
                    devMgr.routing_mode = KuroAudio::DJRoutingMode::MULTI_CHANNEL_4CH;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("- Interface DJ com 4 saídas (Ch 1-2 Master, Ch 3-4 Fone)");

                // Card 3: Cabo Split Mono (Divisor P2)
                bool is_split = (current_mode == (int)KuroAudio::DJRoutingMode::SPLIT_MONO);
                if (ImGui::RadioButton("CABO SPLIT MONO (Divisor P2)", is_split)) {
                    devMgr.routing_mode = KuroAudio::DJRoutingMode::SPLIT_MONO;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("- 1 única saída P2: Esquerda = Master Mono, Direita = Fone Mono");

                // Card 4: Apenas Master
                bool is_stereo = (current_mode == (int)KuroAudio::DJRoutingMode::STEREO_MASTER_ONLY);
                if (ImGui::RadioButton("APENAS SAÍDA MESTRE", is_stereo)) {
                    devMgr.routing_mode = KuroAudio::DJRoutingMode::STEREO_MASTER_ONLY;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("- Sem fone de ouvido independente (Apenas caixas de som normais)");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // =====================================================================
                // 2. SELEÇÃO DE DISPOSITIVOS FÍSICOS
                // =====================================================================
                ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.20f, 1.0f), ICON_FA_PLUG " 2. ATRIBUIR AS SAÍDAS DESTE COMPUTADOR");
                if (devMgr.output_devices.size() >= 2) {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - UIScaleManager::Scale(310.0f));
                    if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Auto-Separar Caixas + Fone", ImVec2(UIScaleManager::Scale(300.0f), UIScaleManager::Scale(24.0f)))) {
                        devMgr.routing_mode = KuroAudio::DJRoutingMode::DUAL_DEVICE;
                        for (const auto& d : devMgr.output_devices) {
                            std::string lower = d.name;
                            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                            if (lower.find("usb") != std::string::npos) {
                                devMgr.master_device_id = d.id;
                                devMgr.master_device_name = d.name;
                                break;
                            }
                        }
                        for (const auto& d : devMgr.output_devices) {
                            if (d.id != devMgr.master_device_id) {
                                devMgr.cue_device_id = d.id;
                                devMgr.cue_device_name = d.name;
                                break;
                            }
                        }
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Configura automaticamente Caixas USB no Master e Fones P2 no Retorno.");
                }
                ImGui::Spacing();

                if (devMgr.output_devices.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), ICON_FA_TRIANGLE_EXCLAMATION " Nenhuma placa de som de saída detectada neste computador!");
                } else {
                    // SELETOR DA SAÍDA MESTRE (DANCEFLOOR / CAIXAS DE SOM)
                    ImGui::BeginGroup();
                    ImGui::TextColored(ImVec4(0.00f, 0.95f, 0.85f, 1.0f), ICON_FA_VOLUME_HIGH " SAÍDA MESTRE (Pista / Caixas de Som):");
                    
                    std::string current_master_label = devMgr.master_device_name.empty() ? "Selecione..." : devMgr.master_device_name;
                    ImGui::SetNextItemWidth(UIScaleManager::Scale(420.0f));
                    if (ImGui::BeginCombo("##MasterDeviceCombo", current_master_label.c_str())) {
                        for (const auto& dev : devMgr.output_devices) {
                            bool is_selected = (dev.id == devMgr.master_device_id);
                            std::string item_text = dev.name + (dev.is_default ? " [PADRÃO]" : "") + " (" + std::to_string(dev.output_channels) + "ch)";
                            if (ImGui::Selectable(item_text.c_str(), is_selected)) {
                                devMgr.master_device_id = dev.id;
                                devMgr.master_device_name = dev.name;
                            }
                            if (is_selected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_BELL " Testar Mestre", ImVec2(UIScaleManager::Scale(140.0f), 0))) {
                        devMgr.triggerMasterTestTone();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tocar tom suave de 440 Hz na Saída Mestre para conferir as caixas");

                    // VU Meter do Master
                    float vu_m_l = devMgr.master_vu_l.load();
                    float vu_m_r = devMgr.master_vu_r.load();
                    ImGui::ProgressBar(vu_m_l, ImVec2(UIScaleManager::Scale(205.0f), UIScaleManager::Scale(6.0f)), "");
                    ImGui::SameLine(0, 5);
                    ImGui::ProgressBar(vu_m_r, ImVec2(UIScaleManager::Scale(205.0f), UIScaleManager::Scale(6.0f)), "");
                    ImGui::EndGroup();

                    ImGui::Spacing();
                    ImGui::Spacing();

                    // SELETOR DA SAÍDA DE RETORNO / FONE (CUE)
                    if (devMgr.routing_mode == KuroAudio::DJRoutingMode::DUAL_DEVICE) {
                        ImGui::BeginGroup();
                        ImGui::TextColored(ImVec4(1.00f, 0.40f, 0.70f, 1.0f), ICON_FA_HEADPHONES " SAÍDA DE RETORNO (Fones de Ouvido / CUE):");

                        std::string current_cue_label = devMgr.cue_device_name.empty() ? "Selecione..." : devMgr.cue_device_name;
                        ImGui::SetNextItemWidth(UIScaleManager::Scale(420.0f));
                        if (ImGui::BeginCombo("##CueDeviceCombo", current_cue_label.c_str())) {
                            for (const auto& dev : devMgr.output_devices) {
                                bool is_selected = (dev.id == devMgr.cue_device_id);
                                std::string item_text = dev.name + (dev.is_default ? " [PADRÃO]" : "") + " (" + std::to_string(dev.output_channels) + "ch)";
                                if (ImGui::Selectable(item_text.c_str(), is_selected)) {
                                    devMgr.cue_device_id = dev.id;
                                    devMgr.cue_device_name = dev.name;
                                }
                                if (is_selected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button(ICON_FA_BELL " Testar Fone", ImVec2(UIScaleManager::Scale(140.0f), 0))) {
                            devMgr.triggerCueTestTone();
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tocar tom suave de 880 Hz no Fone de Ouvido para conferir a pré-escuta");

                        // VU Meter do Cue
                        float vu_c_l = devMgr.cue_vu_l.load();
                        float vu_c_r = devMgr.cue_vu_r.load();
                        ImGui::ProgressBar(vu_c_l, ImVec2(UIScaleManager::Scale(205.0f), UIScaleManager::Scale(6.0f)), "");
                        ImGui::SameLine(0, 5);
                        ImGui::ProgressBar(vu_c_r, ImVec2(UIScaleManager::Scale(205.0f), UIScaleManager::Scale(6.0f)), "");
                        ImGui::EndGroup();

                        if (devMgr.master_device_id == devMgr.cue_device_id) {
                            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), ICON_FA_CIRCLE_INFO " Dica: Escolha duas placas diferentes para separar as caixas e o fone.");
                        }
                    } else if (devMgr.routing_mode == KuroAudio::DJRoutingMode::MULTI_CHANNEL_4CH) {
                        ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.4f, 1.0f), ICON_FA_CHECK " Roteamento 4-Canais Ativo:");
                        ImGui::BulletText("Canais Físicos 1 & 2: Saída Mestre (Caixas da Pista)");
                        ImGui::BulletText("Canais Físicos 3 & 4: Saída Retorno (Fones da Controladora DJ)");
                    } else if (devMgr.routing_mode == KuroAudio::DJRoutingMode::SPLIT_MONO) {
                        ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.4f, 1.0f), ICON_FA_CHECK " Roteamento Split P2 Ativo:");
                        ImGui::BulletText("Pino Esquerdo (L): Saída Mestre Mono (Ligue nas Caixas)");
                        ImGui::BulletText("Pino Direito (R): Saída Retorno CUE Mono (Ligue no Fone de Ouvido)");
                    } else if (devMgr.routing_mode == KuroAudio::DJRoutingMode::STEREO_MASTER_ONLY) {
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.12f, 0.05f, 0.85f));
                        ImGui::BeginChild("##StereoNotice", ImVec2(0, UIScaleManager::Scale(68.0f)), true);
                        ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.20f, 1.0f), ICON_FA_TRIANGLE_EXCLAMATION " O Fone de Ouvido está DESATIVADO neste modo!");
                        ImGui::TextDisabled("Para escutar a pré-escuta (CUE) no fone e as caixas de som separadamente:");
                        if (ImGui::Button(ICON_FA_HEADPHONES " ATIVAR PRÉ-ESCUTA NO FONE (DUAS PLACAS DE SOM)", ImVec2(UIScaleManager::Scale(460.0f), UIScaleManager::Scale(26.0f)))) {
                            devMgr.routing_mode = KuroAudio::DJRoutingMode::DUAL_DEVICE;
                            for (const auto& dev : devMgr.output_devices) {
                                if (dev.id != devMgr.master_device_id) {
                                    devMgr.cue_device_id = dev.id;
                                    devMgr.cue_device_name = dev.name;
                                    break;
                                }
                            }
                        }
                        ImGui::EndChild();
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // =====================================================================
                // 3. BOTÕES DE AÇÃO: SALVAR E FECHAR
                // =====================================================================
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.65f, 0.85f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.80f, 1.0f, 1.0f));
                if (ImGui::Button(ICON_FA_FLOPPY_DISK " SALVAR E APLICAR SAÍDAS", ImVec2(UIScaleManager::Scale(240.0f), UIScaleManager::Scale(34.0f)))) {
                    devMgr.saveConfig();
                    devMgr.need_reinit.store(true);
                    devMgr.show_routing_modal = false;
                }
                ImGui::PopStyleColor(2);

                ImGui::SameLine();
                if (ImGui::Button("Fechar", ImVec2(UIScaleManager::Scale(100.0f), UIScaleManager::Scale(34.0f)))) {
                    devMgr.show_routing_modal = false;
                }

                ImGui::SameLine(ImGui::GetContentRegionAvail().x - UIScaleManager::Scale(180.0f));
                ImGui::TextDisabled("Taxa: %d Hz | Buff: %d", devMgr.sample_rate, devMgr.buffer_size);

            }
            ImGui::End();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
