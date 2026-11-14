#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include "RtAudio.h"
#include "../utils/Logger.h"

namespace KuroAudio {

    enum class DJRoutingMode {
        DUAL_DEVICE = 0,         // 2 Placas diferentes (Master em uma, Fone/Retorno em outra)
        MULTI_CHANNEL_4CH = 1,   // 1 Placa multicanal (Ch 1-2 Master, Ch 3-4 Retorno)
        SPLIT_MONO = 2,          // 1 Placa estéreo com Cabo Split DJ (Esq = Master, Dir = Fone)
        STEREO_MASTER_ONLY = 3   // 1 Placa estéreo normal (Apenas Master na saída principal)
    };

    struct AudioDeviceInfo {
        unsigned int id = 0;
        std::string name = "";
        int output_channels = 0;
        int preferred_sample_rate = 48000;
        bool is_default = false;
    };

    class DJAudioDeviceManager {
    public:
        static DJAudioDeviceManager& getInstance() {
            static DJAudioDeviceManager instance;
            return instance;
        }

        // Lista de dispositivos físicos disponíveis NESTE computador
        std::vector<AudioDeviceInfo> output_devices;
        unsigned int default_device_id = 0;

        // Configuração Ativa
        DJRoutingMode routing_mode = DJRoutingMode::DUAL_DEVICE;
        unsigned int master_device_id = 0;
        std::string master_device_name = "";
        unsigned int cue_device_id = 0;
        std::string cue_device_name = "";

        unsigned int sample_rate = 48000;
        unsigned int buffer_size = 256;
        int output_channels_master = 2;

        std::atomic<bool> need_reinit{ false };
        bool show_routing_modal = false;

        // Gerador de Tom de Teste (Bip suave de 1.2s para identificar saídas)
        std::atomic<bool> test_tone_master{ false };
        std::atomic<bool> test_tone_cue{ false };
        double master_tone_phase = 0.0;
        double cue_tone_phase = 0.0;
        int master_tone_samples_left = 0;
        int cue_tone_samples_left = 0;

        // Ring Buffer Circular Lock-Free para o Retorno no Modo Dual Device
        static const size_t RING_SIZE = 32768;
        float ring_buf_l[RING_SIZE];
        float ring_buf_r[RING_SIZE];
        std::atomic<size_t> ring_write_pos{ 0 };
        std::atomic<size_t> ring_read_pos{ 0 };

        // VU Meters em tempo real
        std::atomic<float> master_vu_l{ 0.0f };
        std::atomic<float> master_vu_r{ 0.0f };
        std::atomic<float> cue_vu_l{ 0.0f };
        std::atomic<float> cue_vu_r{ 0.0f };

    private:
        DJAudioDeviceManager() {
            std::memset(ring_buf_l, 0, sizeof(ring_buf_l));
            std::memset(ring_buf_r, 0, sizeof(ring_buf_r));
            loadConfig();
            rescanDevices();
        }

    public:
        // Escaneia os dispositivos de áudio físicos REAIS do computador em execução
        void rescanDevices() {
            output_devices.clear();
            try {
                RtAudio dac(RtAudio::WINDOWS_WASAPI);
                std::vector<unsigned int> dev_ids = dac.getDeviceIds();
                default_device_id = dac.getDefaultOutputDevice();

                for (unsigned int id : dev_ids) {
                    RtAudio::DeviceInfo info;
                    try { info = dac.getDeviceInfo(id); } catch (...) { continue; }
                    if (info.outputChannels >= 1) {
                        AudioDeviceInfo d;
                        d.id = id;
                        d.name = info.name;
                        d.output_channels = info.outputChannels;
                        d.preferred_sample_rate = info.preferredSampleRate > 0 ? info.preferredSampleRate : 48000;
                        d.is_default = (id == default_device_id);
                        output_devices.push_back(d);
                    }
                }
                KuroUtils::Log("[Audio Device Manager] Dispositivos de saida detectados neste PC: " + std::to_string(output_devices.size()));
            } catch (const std::exception& e) {
                KuroUtils::Log(std::string("[Audio Device Manager] Erro ao escanear dispositivos: ") + e.what());
            } catch (...) {
                KuroUtils::Log("[Audio Device Manager] Erro desconhecido ao escanear dispositivos.");
            }

            // Auto-seleciona dispositivos padrão caso nenhum esteja configurado
            if (!output_devices.empty()) {
                bool master_found = false;
                for (const auto& dev : output_devices) {
                    if (dev.id == master_device_id || (!master_device_name.empty() && dev.name == master_device_name)) {
                        master_device_id = dev.id;
                        master_device_name = dev.name;
                        master_found = true;
                        break;
                    }
                }
                if (!master_found) {
                    for (const auto& dev : output_devices) {
                        if (dev.is_default) {
                            master_device_id = dev.id;
                            master_device_name = dev.name;
                            master_found = true;
                            break;
                        }
                    }
                    if (!master_found) {
                        master_device_id = output_devices[0].id;
                        master_device_name = output_devices[0].name;
                    }
                }

                bool cue_found = false;
                for (const auto& dev : output_devices) {
                    if (dev.id != master_device_id && (dev.id == cue_device_id || (!cue_device_name.empty() && dev.name == cue_device_name))) {
                        cue_device_id = dev.id;
                        cue_device_name = dev.name;
                        cue_found = true;
                        break;
                    }
                }
                if (!cue_found) {
                    // Tenta achar um dispositivo que contenha "fone", "headphone", "usb" ou seja diferente do Master
                    for (const auto& dev : output_devices) {
                        if (dev.id != master_device_id) {
                            std::string lower = dev.name;
                            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                            if (lower.find("usb") != std::string::npos || lower.find("fone") != std::string::npos || 
                                lower.find("headphone") != std::string::npos || lower.find("headset") != std::string::npos) {
                                cue_device_id = dev.id;
                                cue_device_name = dev.name;
                                cue_found = true;
                                break;
                            }
                        }
                    }
                    if (!cue_found) {
                        for (const auto& dev : output_devices) {
                            if (dev.id != master_device_id) {
                                cue_device_id = dev.id;
                                cue_device_name = dev.name;
                                cue_found = true;
                                break;
                            }
                        }
                    }
                    if (!cue_found) {
                        cue_device_id = master_device_id;
                        cue_device_name = master_device_name;
                        if (routing_mode == DJRoutingMode::DUAL_DEVICE && output_devices.size() <= 1) {
                            routing_mode = DJRoutingMode::STEREO_MASTER_ONLY;
                        }
                    }
                }
            }
        }

        void pushCueSamples(float l, float r) {
            size_t w = ring_write_pos.load(std::memory_order_relaxed);
            ring_buf_l[w] = l;
            ring_buf_r[w] = r;
            ring_write_pos.store((w + 1) % RING_SIZE, std::memory_order_release);
        }

        void popCueSamples(float* out_l, float* out_r, unsigned int frames) {
            size_t r = ring_read_pos.load(std::memory_order_relaxed);
            size_t w = ring_write_pos.load(std::memory_order_acquire);

            size_t available = (w >= r) ? (w - r) : (RING_SIZE - r + w);

            // Drift compensation para sincronizar clocks WASAPI de placas diferentes:
            if (available > (RING_SIZE * 3 / 4)) {
                // Buffer enchendo demais: avança leitura
                r = (w + RING_SIZE - (RING_SIZE / 2)) % RING_SIZE;
            }

            for (unsigned int i = 0; i < frames; i++) {
                if (r != w) {
                    out_l[i] = ring_buf_l[r];
                    out_r[i] = ring_buf_r[r];
                    r = (r + 1) % RING_SIZE;
                } else {
                    out_l[i] = 0.0f;
                    out_r[i] = 0.0f;
                }
            }
            ring_read_pos.store(r, std::memory_order_release);
        }

        void triggerMasterTestTone() {
            master_tone_samples_left = (int)(sample_rate * 1.2); // 1.2 segundos
            master_tone_phase = 0.0;
            test_tone_master.store(true);
        }

        void triggerCueTestTone() {
            cue_tone_samples_left = (int)(sample_rate * 1.2); // 1.2 segundos
            cue_tone_phase = 0.0;
            test_tone_cue.store(true);
        }

        // Salva a configuração personalizada deste computador em JSON
        void saveConfig() {
            try {
                std::filesystem::create_directories("scratch");
                std::ofstream f("scratch/audio_device_config.json");
                if (f.is_open()) {
                    f << "{\n";
                    f << "  \"routing_mode\": " << (int)routing_mode << ",\n";
                    f << "  \"master_device_id\": " << master_device_id << ",\n";
                    f << "  \"master_device_name\": \"" << master_device_name << "\",\n";
                    f << "  \"cue_device_id\": " << cue_device_id << ",\n";
                    f << "  \"cue_device_name\": \"" << cue_device_name << "\",\n";
                    f << "  \"sample_rate\": " << sample_rate << ",\n";
                    f << "  \"buffer_size\": " << buffer_size << "\n";
                    f << "}\n";
                    f.close();
                    KuroUtils::Log("[Audio Device Manager] Configuracoes de audio salvas com sucesso.");
                }
            } catch (...) {}
        }

        // Carrega as preferências salvas
        void loadConfig() {
            try {
                if (!std::filesystem::exists("scratch/audio_device_config.json")) return;
                std::ifstream f("scratch/audio_device_config.json");
                if (!f.is_open()) return;

                std::string line;
                while (std::getline(f, line)) {
                    if (line.find("\"routing_mode\"") != std::string::npos) {
                        size_t colon = line.find(':');
                        if (colon != std::string::npos) {
                            routing_mode = (DJRoutingMode)std::stoi(line.substr(colon + 1));
                        }
                    } else if (line.find("\"master_device_name\"") != std::string::npos) {
                        size_t first = line.find('\"', line.find(':'));
                        size_t last = line.rfind('\"');
                        if (first != std::string::npos && last > first) {
                            master_device_name = line.substr(first + 1, last - first - 1);
                        }
                    } else if (line.find("\"cue_device_name\"") != std::string::npos) {
                        size_t first = line.find('\"', line.find(':'));
                        size_t last = line.rfind('\"');
                        if (first != std::string::npos && last > first) {
                            cue_device_name = line.substr(first + 1, last - first - 1);
                        }
                    } else if (line.find("\"sample_rate\"") != std::string::npos) {
                        size_t colon = line.find(':');
                        if (colon != std::string::npos) {
                            unsigned int parsed_sr = (unsigned int)std::stoi(line.substr(colon + 1));
                            sample_rate = (parsed_sr >= 8000) ? parsed_sr : 48000;
                        }
                    } else if (line.find("\"buffer_size\"") != std::string::npos) {
                        size_t colon = line.find(':');
                        if (colon != std::string::npos) {
                            unsigned int parsed_bf = (unsigned int)std::stoi(line.substr(colon + 1));
                            buffer_size = (parsed_bf >= 64) ? parsed_bf : 256;
                        }
                    }
                }
                f.close();
            } catch (...) {}
        }

        const char* getModeTitle(DJRoutingMode m) const {
            switch (m) {
                case DJRoutingMode::DUAL_DEVICE: return "Duas Placas (Caixas + Fone USB/Bluetooth)";
                case DJRoutingMode::MULTI_CHANNEL_4CH: return "Placa Multicanal 4CH (Controladora DJ)";
                case DJRoutingMode::SPLIT_MONO: return "Cabo Split Mono (Divisor P2)";
                case DJRoutingMode::STEREO_MASTER_ONLY: return "Apenas Saida Mestre (2 Canais)";
            }
            return "Personalizado";
        }
    };

} // namespace KuroAudio
