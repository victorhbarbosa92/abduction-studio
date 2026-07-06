#pragma once
#include <string>
#include <atomic>
#include <iostream>
#include <cstdlib>

// Arquitetura Mock: Proteção contra falhas catastróficas de Plugins externos
// Em uma DAW real, isso esconderia IPC (Shared Memory) para outro processo .exe.
class PluginSandboxInstance {
private:
    std::string plugin_name;
    std::atomic<bool> is_crashed{false};
    std::atomic<bool> is_bypassed{false}; // Usado no Smart Background Freeze
    
public:
    explicit PluginSandboxInstance(const std::string& name) : plugin_name(name) {}

    // Simulação do processamento de áudio externo (IPC call)
    void processAudio(float** buffers, int channels, int samples) {
        if (is_crashed.load() || is_bypassed.load()) {
            return; // Silêncio ou bypass transparente se congelado/corrompido
        }
        
        // Simular um crash randômico
        if (std::rand() % 10000 == 1) { // 0.01% chance de crash catastrófico
            reportCrash();
        }
    }
    
    void reportCrash() {
        is_crashed = true;
        std::cerr << "\n[SANDBOX ALERTA] VST3 " << plugin_name << " apresentou falha catastrófica. Isolando via IPC e desativando.\n";
    }
    
    bool isCrashed() const { return is_crashed.load(); }
    
    // Chamado pelo sistema de Smart Background Freeze
    void setBypass(bool state) { is_bypassed = state; }
    std::string getName() const { return plugin_name; }
};
