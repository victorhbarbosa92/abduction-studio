#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>

namespace KuroAudio {

    enum class DJMidiBindingType {
        NOTE_TOGGLE = 0,    // Play/Pause, Sync, Mute
        NOTE_MOMENTARY = 1, // Cue down/up, Pad Roll
        CC_ABSOLUTE = 2,    // Knobs de EQ, Faders de Volume (0..127)
        CC_RELATIVE = 3,    // Jog wheel rotary (Centro 64)
        PITCH_BEND = 4      // Pitch slider de alta resolução
    };

    struct DJMidiBinding {
        std::string action_id;       // Ex: "deck0_play", "deck0_volume", "crossfader"
        unsigned char status = 0;    // Ex: 0x90 (Note On Ch 1), 0xB0 (CC Ch 1)
        unsigned char data1 = 0;     // Note number ou CC number
        DJMidiBindingType type = DJMidiBindingType::CC_ABSOLUTE;
        float min_val = 0.0f;
        float max_val = 1.0f;
    };

    class DJMidiLearnEngine {
    private:
        std::vector<DJMidiBinding> bindings;
        bool is_learning = false;
        std::string learning_action_id;
        DJMidiBindingType learning_type = DJMidiBindingType::CC_ABSOLUTE;
        std::mutex midi_mutex;

        std::function<void(const std::string& action_id, float value)> action_callback = nullptr;

    public:
        DJMidiLearnEngine() = default;

        static DJMidiLearnEngine& getInstance() {
            static DJMidiLearnEngine instance;
            return instance;
        }

        void setActionCallback(std::function<void(const std::string& action_id, float value)> cb) {
            action_callback = cb;
        }

        void startLearn(const std::string& action_id, DJMidiBindingType type = DJMidiBindingType::CC_ABSOLUTE) {
            std::lock_guard<std::mutex> lock(midi_mutex);
            is_learning = true;
            learning_action_id = action_id;
            learning_type = type;
        }

        void cancelLearn() {
            std::lock_guard<std::mutex> lock(midi_mutex);
            is_learning = false;
            learning_action_id.clear();
        }

        bool isLearning() const {
            return is_learning;
        }

        std::string getLearningAction() const {
            return learning_action_id;
        }

        // Processa qualquer mensagem bruta recebida pela porta MIDI
        bool handleMidiMessage(unsigned char status, unsigned char data1, unsigned char data2) {
            std::lock_guard<std::mutex> lock(midi_mutex);

            // 1. Se estiver em modo de aprendizado (MIDI Learn), grava o binding
            if (is_learning && !learning_action_id.empty()) {
                // Remove qualquer binding anterior com o mesmo action_id
                bindings.erase(std::remove_if(bindings.begin(), bindings.end(),
                    [this](const DJMidiBinding& b) { return b.action_id == learning_action_id; }),
                    bindings.end());

                DJMidiBinding nb;
                nb.action_id = learning_action_id;
                nb.status = status;
                nb.data1 = data1;
                nb.type = learning_type;
                bindings.push_back(nb);

                is_learning = false;
                learning_action_id.clear();
                return true;
            }

            // 2. Executa a ação correspondente ao evento recebido
            bool handled = false;
            for (const auto& b : bindings) {
                if (b.status == status && b.data1 == data1) {
                    float val = 0.0f;

                    if (b.type == DJMidiBindingType::CC_ABSOLUTE) {
                        val = (float)data2 / 127.0f;
                    }
                    else if (b.type == DJMidiBindingType::CC_RELATIVE) {
                        // Centro 64: >64 avança, <64 retrocede
                        val = (float)((int)data2 - 64);
                    }
                    else if (b.type == DJMidiBindingType::NOTE_MOMENTARY) {
                        val = (data2 > 0) ? 1.0f : 0.0f;
                    }
                    else if (b.type == DJMidiBindingType::NOTE_TOGGLE) {
                        if (data2 > 0) val = 1.0f; // Dispara pulso de toggle
                        else continue;
                    }

                    if (action_callback) {
                        action_callback(b.action_id, val);
                    }
                    handled = true;
                }
            }
            return handled;
        }

        // Serialização JSON simples para perfis de controladoras
        bool saveToFile(const std::string& filepath) {
            std::lock_guard<std::mutex> lock(midi_mutex);
            std::ofstream f(filepath);
            if (!f.is_open()) return false;

            f << "{\n  \"mappings\": [\n";
            for (size_t i = 0; i < bindings.size(); i++) {
                const auto& b = bindings[i];
                f << "    {\n"
                  << "      \"action_id\": \"" << b.action_id << "\",\n"
                  << "      \"status\": " << (int)b.status << ",\n"
                  << "      \"data1\": " << (int)b.data1 << ",\n"
                  << "      \"type\": " << (int)b.type << "\n"
                  << "    }" << (i + 1 < bindings.size() ? "," : "") << "\n";
            }
            f << "  ]\n}\n";
            return true;
        }

        bool loadFromFile(const std::string& filepath) {
            std::lock_guard<std::mutex> lock(midi_mutex);
            std::ifstream f(filepath);
            if (!f.is_open()) return false;
            bindings.clear();

            std::string line;
            std::string cur_act;
            int cur_status = -1, cur_data1 = -1, cur_type = -1;
            while (std::getline(f, line)) {
                auto getVal = [](const std::string& l, const std::string& key) -> std::string {
                    auto pos = l.find("\"" + key + "\":");
                    if (pos == std::string::npos) return "";
                    auto start = l.find_first_not_of(" \t\":", pos + key.length() + 2);
                    if (start == std::string::npos) return "";
                    auto end = l.find_first_of(",\"\n\r}", start);
                    return l.substr(start, end - start);
                };

                if (line.find("\"action_id\":") != std::string::npos) {
                    cur_act = getVal(line, "action_id");
                }
                if (line.find("\"status\":") != std::string::npos) {
                    try { cur_status = std::stoi(getVal(line, "status")); } catch (...) {}
                }
                if (line.find("\"data1\":") != std::string::npos) {
                    try { cur_data1 = std::stoi(getVal(line, "data1")); } catch (...) {}
                }
                if (line.find("\"type\":") != std::string::npos) {
                    try { cur_type = std::stoi(getVal(line, "type")); } catch (...) {}
                }

                if (line.find("}") != std::string::npos && !cur_act.empty() && cur_status >= 0 && cur_data1 >= 0) {
                    DJMidiBinding b;
                    b.action_id = cur_act;
                    b.status = (unsigned char)cur_status;
                    b.data1 = (unsigned char)cur_data1;
                    b.type = (cur_type >= 0) ? (DJMidiBindingType)cur_type : DJMidiBindingType::CC_ABSOLUTE;
                    bindings.push_back(b);
                    cur_act.clear();
                    cur_status = -1; cur_data1 = -1; cur_type = -1;
                }
            }
            return true;
        }

        const std::vector<DJMidiBinding>& getBindings() const {
            return bindings;
        }

        void clearBindings() {
            std::lock_guard<std::mutex> lock(midi_mutex);
            bindings.clear();
        }

        void addBinding(const std::string& action_id, unsigned char status, unsigned char data1, DJMidiBindingType type) {
            std::lock_guard<std::mutex> lock(midi_mutex);
            bindings.push_back({ action_id, status, data1, type, 0.0f, 1.0f });
        }
    };

} // namespace KuroAudio
