#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <iostream>
#include <fstream>
#define NOMINMAX
#include <windows.h>
#include "../thirdparty/clap-main/include/clap/clap.h"

namespace KuroDSP {

    struct PluginInfo {
        std::string id;
        std::string name;
        std::string vendor;
        std::string version;
        std::string format_type; // "CLAP", "VST3", "DLL"
        std::string category;    // "Synth", "Effect", "Dynamics", "Utility"
        std::string filepath;
        bool is_valid = true;
    };

    class PluginScannerManager {
    private:
        std::vector<std::string> search_directories;
        std::vector<PluginInfo> scanned_plugins;
        mutable std::mutex plugins_mutex;

        std::atomic<bool> is_scanning{false};
        std::atomic<float> scan_progress{0.0f};
        std::string current_status_msg;

    public:
        PluginScannerManager() {
            initDefaultDirectories();
            loadPreRegisteredPlugins();
        }

        ~PluginScannerManager() = default;

        void initDefaultDirectories() {
            search_directories.clear();
            
            std::vector<std::string> defaults = {
                "C:\\Program Files\\Common Files\\CLAP",
                "C:\\Program Files\\Common Files\\VST3",
                "C:\\Program Files\\VstPlugins",
                "C:\\Program Files\\Steinberg\\VstPlugins",
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\plugins",
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\VST_Tests"
            };

            for (const auto& path : defaults) {
                search_directories.push_back(path);
            }
        }

        void loadPreRegisteredPlugins() {
            std::lock_guard<std::mutex> lock(plugins_mutex);
            scanned_plugins = {
                { "surge_xt_clap", "Surge XT Effects", "Surge Synth Team", "1.3.0", "CLAP", "Effect / Multi-FX", "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\VST_Tests\\Surge XT Effects.clap", true },
                { "monksynth_vst3", "MonkSynth VST3", "Alien Audio Labs", "2.1.0", "VST3", "Synthesizer / Voice", "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\plugins\\MonkSynth.vst3\\Contents\\x86_64-win\\MonkSynth.vst3", true },
                { "delay_lama_vst", "Delay Lama 3D Vocal", "AudioNerdz", "1.5.0", "VST3", "Vocal Synth / Formant", "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\plugins\\DelayLama.dll", true },
                { "kuro_compressor", "Gothic Compressor", "Abduction Core", "1.0.0", "BUILT-IN", "Dynamics / Compressor", "Internal DSP", true },
                { "spatial_reverb", "Spatial Delay & Cavern Reverb", "Abduction Core", "1.0.0", "BUILT-IN", "Reverb / Delay", "Internal DSP", true },
                { "abyss_pitch", "Abyss Pitch Shifter Granular", "Abduction Core", "1.0.0", "BUILT-IN", "Pitch / FX", "Internal DSP", true }
            };
            current_status_msg = "Biblioteca inicial carregada (" + std::to_string(scanned_plugins.size()) + " plugins disponíveis).";
        }

        const std::vector<std::string>& getDirectories() const {
            return search_directories;
        }

        void addDirectory(const std::string& path) {
            if (!path.empty()) {
                for (const auto& dir : search_directories) {
                    if (dir == path) return;
                }
                search_directories.push_back(path);
            }
        }

        void removeDirectory(size_t index) {
            if (index < search_directories.size()) {
                search_directories.erase(search_directories.begin() + index);
            }
        }

        bool isScanning() const { return is_scanning.load(); }
        float getProgress() const { return scan_progress.load(); }
        std::string getStatusMsg() const { return current_status_msg; }

        std::vector<PluginInfo> getPlugins() const {
            std::lock_guard<std::mutex> lock(plugins_mutex);
            return scanned_plugins;
        }

        void scanAsync() {
            if (is_scanning.load()) return;

            is_scanning = true;
            scan_progress = 0.0f;
            current_status_msg = "Iniciando varredura de plugins...";

            std::thread([this]() {
                std::vector<PluginInfo> found;
                std::vector<std::string> files_to_check;

                for (const auto& dir_str : search_directories) {
                    if (!std::filesystem::exists(dir_str)) continue;

                    try {
                        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_str, std::filesystem::directory_options::skip_permission_denied)) {
                            if (entry.is_regular_file()) {
                                std::string ext = entry.path().extension().string();
                                for (auto& c : ext) c = (char)tolower(c);

                                if (ext == ".clap" || ext == ".vst3" || ext == ".dll") {
                                    files_to_check.push_back(entry.path().string());
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "[Scanner] Exceção ao ler diretório: " << e.what() << "\n";
                    }
                }

                size_t total = files_to_check.size();
                if (total == 0) {
                    current_status_msg = "Nenhum arquivo de plugin (.clap, .vst3, .dll) encontrado nas pastas especificadas.";
                    is_scanning = false;
                    scan_progress = 1.0f;
                    return;
                }

                for (size_t i = 0; i < total; ++i) {
                    const std::string& file_path = files_to_check[i];
                    current_status_msg = "Verificando: " + std::filesystem::path(file_path).filename().string();
                    scan_progress = (float)(i + 1) / (float)total;

                    PluginInfo info;
                    info.filepath = file_path;
                    info.name = std::filesystem::path(file_path).stem().string();
                    info.vendor = "Desconhecido";
                    info.version = "1.0";

                    std::string ext = std::filesystem::path(file_path).extension().string();
                    for (auto& c : ext) c = (char)tolower(c);

                    if (ext == ".clap") {
                        info.format_type = "CLAP";
                        info.category = "Instrument / FX";
                        inspectClapFile(file_path, info);
                    } else if (ext == ".vst3") {
                        info.format_type = "VST3";
                        info.category = "VST3 Plugin";
                    } else {
                        info.format_type = "DLL (VST2/CLAP)";
                        info.category = "Effect";
                    }

                    found.push_back(info);
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }

                {
                    std::lock_guard<std::mutex> lock(plugins_mutex);
                    scanned_plugins = found;
                }

                current_status_msg = "Varredura concluída! " + std::to_string(found.size()) + " plugins registrados.";
                is_scanning = false;
                scan_progress = 1.0f;
            }).detach();
        }

    private:
        void inspectClapFile(const std::string& path, PluginInfo& info) {
            HMODULE handle = LoadLibraryA(path.c_str());
            if (!handle) return;

            auto entry = (const clap_plugin_entry_t*)GetProcAddress(handle, "clap_entry");
            if (entry && entry->init(path.c_str())) {
                auto factory = (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
                if (factory && factory->get_plugin_count(factory) > 0) {
                    auto desc = factory->get_plugin_descriptor(factory, 0);
                    if (desc) {
                        if (desc->name) info.name = desc->name;
                        if (desc->vendor) info.vendor = desc->vendor;
                        if (desc->version) info.version = desc->version;
                        if (desc->id) info.id = desc->id;
                    }
                }
                entry->deinit();
            }
            FreeLibrary(handle);
        }
    };
}
