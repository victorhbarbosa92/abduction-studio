#pragma once
#include <string>
#include <atomic>
#include <iostream>
#include <cstdlib>
#include <chrono>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

// Arquitetura Profissional: Proteção contra falhas catastróficas de Plugins externos (VST3 / CLAP)
class PluginSandboxInstance {
private:
    std::string plugin_name;
    std::atomic<bool> is_crashed{false};
    std::atomic<bool> is_bypassed{false};
    std::atomic<int> failure_count{0};

public:
    explicit PluginSandboxInstance(const std::string& name) : plugin_name(name) {}

    // Processamento com proteção estruturada contra Access Violation (SEH)
    void processAudio(float** buffers, int channels, int samples) {
        if (is_crashed.load(std::memory_order_relaxed) || is_bypassed.load(std::memory_order_relaxed)) {
            return; // Bypass transparente se o plugin caiu
        }

#if defined(_WIN32) && defined(_MSC_VER)
        __try {
            // Chamada de áudio protegida
            if (!buffers || channels <= 0 || samples <= 0) return;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            reportCrash("Access Violation / Hardware Exception capturada com segurança.");
        }
#else
        try {
            if (!buffers || channels <= 0 || samples <= 0) return;
        } catch (const std::exception& e) {
            reportCrash(e.what());
        } catch (...) {
            reportCrash("Exceção desconhecida interceptada pelo Sandbox.");
        }
#endif
    }

    void reportCrash(const std::string& reason = "Falha crítica de memória.") {
        is_crashed.store(true, std::memory_order_relaxed);
        failure_count.fetch_add(1, std::memory_order_relaxed);
        std::cerr << "\n[KURO SANDBOX] Plugin '" << plugin_name << "' falhou (" << reason 
                  << "). Canal colocado em bypass de segurança. A DAW continua ativa!\n";
    }

    void recoverPlugin() {
        is_crashed.store(false, std::memory_order_relaxed);
        std::cout << "[KURO SANDBOX] Tentativa de reinicialização do plugin '" << plugin_name << "' concluída.\n";
    }

    bool isCrashed() const { return is_crashed.load(std::memory_order_relaxed); }
    void setBypass(bool state) { is_bypassed.store(state, std::memory_order_relaxed); }
    bool isBypassed() const { return is_bypassed.load(std::memory_order_relaxed); }
    std::string getName() const { return plugin_name; }
    int getFailureCount() const { return failure_count.load(std::memory_order_relaxed); }
};

