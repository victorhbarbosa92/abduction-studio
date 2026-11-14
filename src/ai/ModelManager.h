#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <filesystem>
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

class DownloadProgressCallback : public IBindStatusCallback {
public:
    std::atomic<float>* progress_ptr;
    std::atomic<const char*>* status_ptr;

    DownloadProgressCallback(std::atomic<float>* p, std::atomic<const char*>* s) : progress_ptr(p), status_ptr(s) {}

    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (IsEqualIID(IID_IBindStatusCallback, riid) || IsEqualIID(IID_IUnknown, riid)) {
            *ppvObject = reinterpret_cast<void*>(this);
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }

    // IBindStatusCallback methods
    HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD, IBinding*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetPriority(LONG*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE OnLowResource(DWORD) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG, LPCWSTR) override {
        if (ulProgressMax > 0) {
            *progress_ptr = static_cast<float>(ulProgress) / static_cast<float>(ulProgressMax);
            if (*progress_ptr < 0.99f) {
                *status_ptr = "Baixando Modelo de IA...";
            } else {
                *status_ptr = "Finalizando Download...";
            }
        }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT, LPCWSTR) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD*, BINDINFO*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID, IUnknown*) override { return S_OK; }
};

class ModelManager {
public:
    static std::atomic<float> download_progress;
    static std::atomic<bool> is_downloading;
    static std::atomic<const char*> status_text;

    static void EnsureModelExists(const std::string& modelName, const std::string& url) {
        std::filesystem::create_directory("models");
        std::string path = "models/" + modelName;

        if (std::filesystem::exists(path)) {
            status_text = "Modelo pronto localmente.";
            return;
        }

        std::thread([path, url]() {
            is_downloading = true;
            status_text = "Conectando ao servidor...";
            download_progress = 0.0f;
            
            // Converter string para std::wstring para a API do Windows
            std::wstring wurl(url.begin(), url.end());
            std::wstring wpath(path.begin(), path.end());

            DownloadProgressCallback callback(&download_progress, &status_text);
            
            HRESULT hr = URLDownloadToFileW(NULL, wurl.c_str(), wpath.c_str(), 0, &callback);
            
            if (SUCCEEDED(hr)) {
                status_text = "Download concluído! IA Pronta.";
            } else {
                status_text = "Erro ao baixar modelo. Tentando Fallback.";
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
            is_downloading = false;
        }).detach();
    }
};

// Inicialização dos membros estáticos
std::atomic<float> ModelManager::download_progress{0.0f};
std::atomic<bool> ModelManager::is_downloading{false};
std::atomic<const char*> ModelManager::status_text{"Iniciando..."};
