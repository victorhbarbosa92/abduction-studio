#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>
#include <algorithm>
#include "NativePlugins.h"
#include "../audio/LFOEngine.h"

#include "KuroPedalboard.h"

namespace KuroDSP {

    // Base Node para qualquer plugin (Nativo ou CLAP)
    class PluginNode {
    protected:
        std::string node_id;
        std::string node_name;
        bool is_bypassed = false;
        
    public:
        PluginNode(const std::string& id, const std::string& name) 
            : node_id(id), node_name(name) {}
        
        virtual ~PluginNode() = default;
        
        virtual void process(float* left, float* right, unsigned int frames) = 0;
        virtual void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) = 0;
        virtual void pushMidiNote(int key, bool is_note_on, float velocity = 1.0f) {}
        
        void setBypass(bool bypass) { is_bypassed = bypass; }
        bool getBypass() const { return is_bypassed; }
        std::string getId() const { return node_id; }
        std::string getName() const { return node_name; }
    };

    // Nó de Roteamento que encapsula uma Pedaleira (KuroRack)
    class RackNode : public PluginNode {
    public:
        Pedalboard rack;
        Pedalboard* rack_ptr = nullptr;
        
        RackNode(const std::string& id, const std::string& name, Pedalboard* ptr = nullptr) 
            : PluginNode(id, name), rack_ptr(ptr) {}
        
        void process(float* left, float* right, unsigned int frames) override {
            if (rack_ptr) {
                rack_ptr->process(left, right, frames);
            } else {
                rack.process(left, right, frames);
            }
        }
        
        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
            // A interpolação de automação LERP será tratada futuramente nos efeitos filhos
        }
    };

    // Nó de junção (Bus) para roteamento passivo (ex: saídas de track)
    class BusNode : public PluginNode {
    public:
        float* volume_ptr = nullptr;
        
        BusNode(const std::string& id, const std::string& name, float* vol_ptr = nullptr) 
            : PluginNode(id, name), volume_ptr(vol_ptr) {}
        
        void process(float* left, float* right, unsigned int frames) override {
            // Pass-through
        }
        
        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
            if (param_index == 0 && volume_ptr) {
                *volume_ptr = target_value; // Updates UI and audio engine smooth target
            }
        }
    };

    class ReverbNode : public PluginNode {
    public:
        KuroReverb reverb;
        ReverbNode(const std::string& id, const std::string& name) : PluginNode(id, name) {}
        void process(float* left, float* right, unsigned int frames) override {
            reverb.process(left, right, frames);
        }
        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {}
    };

    class DelayNode : public PluginNode {
    public:
        KuroDelay delay;
        DelayNode(const std::string& id, const std::string& name) : PluginNode(id, name) {}
        void process(float* left, float* right, unsigned int frames) override {
            delay.process(left, right, frames);
        }
        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {}
    };

    class DummyNativeNode : public PluginNode {
    public:
        DummyNativeNode(const std::string& id, const std::string& name) : PluginNode(id, name) {}
        void process(float* left, float* right, unsigned int frames) override {}
        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {}
    };

    // Estrutura DAG para roteamento flexível
    class AudioGraph {
    private:
        struct Edge {
            std::string to_id;
            float weight = 1.0f;
            float* dynamic_weight_ptr = nullptr;
        };

        struct NodeEntry {
            std::shared_ptr<PluginNode> plugin;
            std::vector<std::string> inputs;
            std::vector<Edge> outputs;
            
            // Buffers temporários alinhados (lock-free safe) para somatório do DAG
            std::vector<float> buffer_l;
            std::vector<float> buffer_r;
        };
        
        std::unordered_map<std::string, NodeEntry> nodes;
        std::vector<std::string> execution_order; // Ordem topológica pré-calculada
        
        unsigned int max_block_size = 2048;

        // --- SISTEMA DE MODULAÇÃO (FASE 9) ---
        struct ModRouting {
            int lfo_index;
            std::string target_node_id;
            int target_param_index;
            float depth;
            float base_value;
        };
        std::vector<LFOEngine> lfos;
        std::vector<ModRouting> mod_routings;

    public:
        AudioGraph() {}
        
        void setMaxBlockSize(unsigned int size) {
            max_block_size = size;
            for (auto& pair : nodes) {
                pair.second.buffer_l.resize(size, 0.0f);
                pair.second.buffer_r.resize(size, 0.0f);
            }
        }
        
        void addNode(const std::string& id, std::shared_ptr<PluginNode> plugin) {
            NodeEntry entry;
            entry.plugin = plugin;
            entry.buffer_l.resize(max_block_size, 0.0f);
            entry.buffer_r.resize(max_block_size, 0.0f);
            nodes[id] = entry;
        }
        
        void connect(const std::string& from_id, const std::string& to_id, float* dynamic_weight_ptr = nullptr) {
            if (nodes.count(from_id) && nodes.count(to_id)) {
                nodes[from_id].outputs.push_back({to_id, 1.0f, dynamic_weight_ptr});
                nodes[to_id].inputs.push_back(from_id);
            }
            recalculateExecutionOrder();
        }
        
        void disconnectAllOutputs(const std::string& node_id) {
            if (nodes.count(node_id)) {
                // Remove this node from the inputs list of its target nodes
                for (const auto& edge : nodes[node_id].outputs) {
                    auto& target_inputs = nodes[edge.to_id].inputs;
                    target_inputs.erase(std::remove(target_inputs.begin(), target_inputs.end(), node_id), target_inputs.end());
                }
                nodes[node_id].outputs.clear();
                recalculateExecutionOrder();
            }
        }
        
        // Simples ordenação topológica (Kahn's Algorithm) pode ser chamada pela UI (nunca na audio thread)
        void recalculateExecutionOrder() {
            execution_order.clear();
            std::unordered_map<std::string, int> in_degree;
            
            for (const auto& pair : nodes) {
                in_degree[pair.first] = pair.second.inputs.size();
            }
            
            std::vector<std::string> queue;
            for (const auto& pair : in_degree) {
                if (pair.second == 0) queue.push_back(pair.first);
            }
            
            while (!queue.empty()) {
                std::string current = queue.back();
                queue.pop_back();
                execution_order.push_back(current);
                
                for (const auto& edge : nodes[current].outputs) {
                    in_degree[edge.to_id]--;
                    if (in_degree[edge.to_id] == 0) {
                        queue.push_back(edge.to_id);
                    }
                }
            }
        }
        
        // Chamado no Audio Callback (LOCK-FREE REALTIME SAFE)
        void process(float* in_out_l, float* in_out_r, unsigned int frames, float current_bpm = 140.0f) {
            if (frames > max_block_size) return; // Prevent buffer overflow
            
            // 0. Processa LFOs e ModMatrix
            if (!lfos.empty() && !mod_routings.empty()) {
                std::vector<float> lfo_values(lfos.size());
                for(size_t i=0; i<lfos.size(); i++) {
                    lfo_values[i] = lfos[i].process(current_bpm);
                }
                for(auto& mod : mod_routings) {
                    if (auto node = getNode(mod.target_node_id)) {
                        float modulated_val = mod.base_value + (lfo_values[mod.lfo_index] * mod.depth);
                        node->setParameter(mod.target_param_index, modulated_val);
                    }
                }
            }

            // 1. Limpa buffers (exceto nós de entrada)
            for (const auto& node_id : execution_order) {
                auto& entry = nodes[node_id];
                std::fill(entry.buffer_l.begin(), entry.buffer_l.begin() + frames, 0.0f);
                std::fill(entry.buffer_r.begin(), entry.buffer_r.begin() + frames, 0.0f);
            }
            
            // 2. Processa o DAG na ordem topológica
            for (const auto& node_id : execution_order) {
                auto& entry = nodes[node_id];
                
                if (!entry.plugin->getBypass()) {
                    entry.plugin->process(entry.buffer_l.data(), entry.buffer_r.data(), frames);
                }
                
                // 3. Acumula o output deste nó nas entradas dos nós de destino
                for (const auto& edge : entry.outputs) {
                    auto& dest = nodes[edge.to_id];
                    float gain = edge.dynamic_weight_ptr ? *(edge.dynamic_weight_ptr) : edge.weight;
                    
                    for (unsigned int i = 0; i < frames; ++i) {
                        dest.buffer_l[i] += entry.buffer_l[i] * gain;
                        dest.buffer_r[i] += entry.buffer_r[i] * gain;
                    }
                }
            }
            
            // 4. Manda o output do nó Master para as saídas, misturando com o que já existe (Synth)
            if (nodes.count("Master")) {
                auto& master_node = nodes["Master"];
                for (unsigned int i = 0; i < frames; ++i) {
                    in_out_l[i] += master_node.buffer_l[i];
                    in_out_r[i] += master_node.buffer_r[i];
                }
            }
        }
        
        std::shared_ptr<PluginNode> getNode(const std::string& id) {
            if (nodes.count(id)) return nodes[id].plugin;
            return nullptr;
        }

        const std::vector<float>* getNodeBufferL(const std::string& id) const {
            if (nodes.count(id)) return &nodes.at(id).buffer_l;
            return nullptr;
        }

        const std::vector<float>* getNodeBufferR(const std::string& id) const {
            if (nodes.count(id)) return &nodes.at(id).buffer_r;
            return nullptr;
        }

        std::vector<std::string> getNodeIds() const {
            std::vector<std::string> ids;
            for (const auto& pair : nodes) ids.push_back(pair.first);
            return ids;
        }

        // --- API DA MATRIZ DE MODULAÇÃO ---
        int addLFO() {
            lfos.push_back(LFOEngine());
            return lfos.size() - 1;
        }

        LFOEngine& getLFO(int index) {
            return lfos[index];
        }

        void addModulation(int lfo_idx, const std::string& target_node, int param_idx, float depth, float base_val) {
            mod_routings.push_back({lfo_idx, target_node, param_idx, depth, base_val});
        }
        
        void clearModulations() {
            mod_routings.clear();
        }
    };
}
