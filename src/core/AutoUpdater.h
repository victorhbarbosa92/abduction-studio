#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>
#pragma comment(lib, "winhttp.lib")
#endif

// Forward declaration do ImGui se disponivel
#include "imgui.h"

namespace KuroAudio {

    enum class UpdateStatus {
        IDLE,
        CHECKING,
        AVAILABLE,
        UP_TO_DATE,
        DOWNLOADING,
        READY_TO_INSTALL,
        FAILED
    };

    struct UpdateManifest {
        bool update_available = false;
        std::string latest_version;
        std::string current_version;
        std::string title;
        std::string changelog;
        std::string download_url;
        size_t size_bytes = 0;
    };

    class AutoUpdater {
    public:
        static AutoUpdater& GetInstance() {
            static AutoUpdater instance;
            return instance;
        }

        // Configuração inicial do updater
        void Initialize(const std::string& defaultWorkerUrl = "", const std::string& currentVersion = "2.0.0", const std::string& secretKey = "") {
            m_workerUrl = defaultWorkerUrl;
            m_currentVersion = currentVersion;
            m_secretKey = secretKey;

            // Tenta carregar override do updater.json se existir
            LoadConfig();
        }

        void LoadConfig(const std::filesystem::path& configPath = "updater.json") {
            try {
                if (std::filesystem::exists(configPath)) {
                    std::ifstream file(configPath);
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        std::string content = buffer.str();
                        std::string url = ExtractJsonString(content, "worker_url");
                        if (!url.empty()) m_workerUrl = url;
                        std::string key = ExtractJsonString(content, "secret_key");
                        if (!key.empty()) m_secretKey = key;
                    }
                }
            } catch (...) {}
        }

        const std::string& GetCurrentVersion() const { return m_currentVersion; }
        const UpdateManifest& GetManifest() const { return m_manifest; }
        UpdateStatus GetStatus() const { return m_status.load(); }
        float GetProgress() const { return m_progress.load(); }
        size_t GetDownloadedBytes() const { return m_downloadedBytes.load(); }
        size_t GetTotalBytes() const { return m_totalBytes.load(); }
        const std::string& GetErrorMessage() const { return m_errorMessage; }

        void SetWorkerUrl(const std::string& url) { m_workerUrl = url; }
        void SetSecretKey(const std::string& key) { m_secretKey = key; }

        // Dispara a verificação assíncrona em segundo plano
        void CheckForUpdatesAsync(bool userInitiated = false) {
            if (m_status.load() == UpdateStatus::CHECKING || m_status.load() == UpdateStatus::DOWNLOADING) {
                return;
            }

            m_userInitiated = userInitiated;
            m_status.store(UpdateStatus::CHECKING);
            m_errorMessage.clear();

            std::thread([this]() {
                this->PerformCheck();
            }).detach();
        }

        // Inicia o download assíncrono do pacote
        void StartDownloadAsync() {
            if (m_status.load() != UpdateStatus::AVAILABLE) return;

            m_status.store(UpdateStatus::DOWNLOADING);
            m_progress.store(0.0f);
            m_downloadedBytes.store(0);
            m_errorMessage.clear();

            std::thread([this]() {
                this->PerformDownload();
            }).detach();
        }

        // Executa o AbductionUpdater.exe e encerra a aplicação atual
        bool ApplyUpdateAndRestart() {
            if (m_status.load() != UpdateStatus::READY_TO_INSTALL || m_tempZipPath.empty()) {
                return false;
            }

            std::filesystem::path appDir = std::filesystem::current_path();
            std::filesystem::path updaterPath = appDir / "AbductionUpdater.exe";

            // Se não estiver na raiz, procura na pasta bin/
            if (!std::filesystem::exists(updaterPath)) {
                updaterPath = appDir / "bin" / "AbductionUpdater.exe";
            }

            if (!std::filesystem::exists(updaterPath)) {
                m_errorMessage = "Executável AbductionUpdater.exe não foi encontrado em: " + updaterPath.string();
                m_status.store(UpdateStatus::FAILED);
                return false;
            }

            DWORD currentPid = GetCurrentProcessId();

            std::wstring wUpdater = updaterPath.wstring();
            std::wstring wZip = m_tempZipPath.wstring();
            std::wstring wTarget = appDir.wstring();
            std::wstring wVer = std::wstring(m_manifest.latest_version.begin(), m_manifest.latest_version.end());

            // Parâmetros: --pid <PID> --zip "<ZIP>" --target "<TARGET>" --version "<VER>"
            std::wstring cmdArgs = L"\"" + wUpdater + L"\" --pid " + std::to_wstring(currentPid) +
                                  L" --zip \"" + wZip + L"\" --target \"" + wTarget + L"\"" +
                                  L" --version \"" + wVer + L"\"";

            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            std::vector<wchar_t> cmdBuf(cmdArgs.begin(), cmdArgs.end());
            cmdBuf.push_back(0);

            if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE, 0, NULL, wTarget.c_str(), &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                // Encerra imediatamente o Abduction Studio para liberação dos arquivos
                ExitProcess(0);
                return true;
            } else {
                m_errorMessage = "Falha ao iniciar processo do atualizador.";
                m_status.store(UpdateStatus::FAILED);
                return false;
            }
        }

        // Renderiza o Modal/Diálogo de atualização com tema ImGui
        void RenderUI() {
            if (!m_showModal) return;

            ImGui::OpenPopup("Atualização do Abduction Studio##UpdaterModal");

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Appearing);

            if (ImGui::BeginPopupModal("Atualização do Abduction Studio##UpdaterModal", &m_showModal, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
                UpdateStatus current = m_status.load();

                if (current == UpdateStatus::CHECKING) {
                    ImGui::TextUnformatted("Verificando se há atualizações...");
                    ImGui::Spacing();
                }
                else if (current == UpdateStatus::AVAILABLE) {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.4f, 1.0f), "Nova versao encontrada: v%s", m_manifest.latest_version.c_str());
                    ImGui::TextDisabled("Versao atual instalada: v%s", m_currentVersion.c_str());
                    ImGui::Separator();

                    if (!m_manifest.changelog.empty()) {
                        ImGui::TextUnformatted("Novidades e alteracoes:");
                        ImGui::BeginChild("ChangelogBox", ImVec2(450, 110), true);
                        ImGui::TextWrapped("%s", m_manifest.changelog.c_str());
                        ImGui::EndChild();
                    }

                    if (m_manifest.size_bytes > 0) {
                        double mb = (double)m_manifest.size_bytes / (1024.0 * 1024.0);
                        ImGui::TextDisabled("Tamanho do download: %.1f MB", mb);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();

                    if (ImGui::Button("Baixar e Atualizar Agora", ImVec2(200, 32))) {
                        StartDownloadAsync();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Lembrar Mais Tarde", ImVec2(150, 32))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }
                else if (current == UpdateStatus::DOWNLOADING) {
                    ImGui::TextUnformatted("Baixando atualizacao...");
                    float prog = m_progress.load();
                    size_t down = m_downloadedBytes.load();
                    size_t tot = m_totalBytes.load();

                    char overlay[64];
                    if (tot > 0) {
                        double downMb = (double)down / (1024.0 * 1024.0);
                        double totMb = (double)tot / (1024.0 * 1024.0);
                        snprintf(overlay, sizeof(overlay), "%.1f MB / %.1f MB (%.0f%%)", downMb, totMb, prog * 100.0f);
                    } else {
                        double downMb = (double)down / (1024.0 * 1024.0);
                        snprintf(overlay, sizeof(overlay), "%.1f MB baixados...", downMb);
                    }

                    ImGui::ProgressBar(prog, ImVec2(450, 24), overlay);
                    ImGui::Spacing();
                    ImGui::TextDisabled("Aguarde a conclusao do download para reiniciar.");
                }
                else if (current == UpdateStatus::READY_TO_INSTALL) {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.4f, 1.0f), "Download concluido com sucesso!");
                    ImGui::Spacing();
                    ImGui::TextWrapped("O Abduction Studio precisa reiniciar para aplicar os novos arquivos.");
                    ImGui::Separator();

                    if (ImGui::Button("Reiniciar e Aplicar Agora", ImVec2(220, 34))) {
                        ApplyUpdateAndRestart();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Instalar ao Fechar", ImVec2(160, 34))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }
                else if (current == UpdateStatus::UP_TO_DATE) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Seu Abduction Studio ja esta atualizado!");
                    ImGui::TextDisabled("Versao: v%s", m_currentVersion.c_str());
                    ImGui::Spacing();
                    if (ImGui::Button("OK", ImVec2(100, 28))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }
                else if (current == UpdateStatus::FAILED) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Erro durante a atualizacao:");
                    ImGui::TextWrapped("%s", m_errorMessage.c_str());
                    ImGui::Spacing();
                    if (ImGui::Button("Fechar", ImVec2(100, 28))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::EndPopup();
            }
        }

        void ShowModal(bool show = true) { m_showModal = show; }

    private:
        AutoUpdater() = default;
        ~AutoUpdater() = default;

        std::string m_workerUrl;
        std::string m_currentVersion = "2.0.0";
        std::string m_secretKey;

        std::atomic<UpdateStatus> m_status{UpdateStatus::IDLE};
        std::atomic<float> m_progress{0.0f};
        std::atomic<size_t> m_downloadedBytes{0};
        std::atomic<size_t> m_totalBytes{0};

        UpdateManifest m_manifest;
        std::filesystem::path m_tempZipPath;
        std::string m_errorMessage;
        bool m_showModal = false;
        bool m_userInitiated = false;

        // Extrai valor de chave JSON simples
        static std::string ExtractJsonString(const std::string& json, const std::string& key) {
            std::string search = "\"" + key + "\":";
            size_t pos = json.find(search);
            if (pos == std::string::npos) return "";

            pos += search.length();
            while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

            if (pos >= json.length()) return "";

            if (json[pos] == '"') {
                pos++;
                size_t endPos = json.find('"', pos);
                if (endPos == std::string::npos) return "";
                return json.substr(pos, endPos - pos);
            } else if (json[pos] == 't' || json[pos] == 'f') {
                size_t endPos = json.find_first_of(",}\n\r", pos);
                return json.substr(pos, endPos - pos);
            } else {
                size_t endPos = json.find_first_of(",}\n\r", pos);
                return json.substr(pos, endPos - pos);
            }
        }

        // Executa a requisição GET de verificação
        void PerformCheck() {
            if (m_workerUrl.empty()) {
                m_errorMessage = "URL do servidor de atualização não configurada.";
                m_status.store(UpdateStatus::FAILED);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            std::string queryUrl = m_workerUrl;
            if (queryUrl.back() != '/') queryUrl += '/';
            queryUrl += "api/check-update?version=" + m_currentVersion;

            if (!m_secretKey.empty()) {
                queryUrl += "&key=" + m_secretKey;
            }

            std::wstring wurl(queryUrl.begin(), queryUrl.end());
            URL_COMPONENTS urlComp = { 0 };
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwSchemeLength = (DWORD)-1;
            urlComp.dwHostNameLength = (DWORD)-1;
            urlComp.dwUrlPathLength = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
                m_errorMessage = "URL do servidor inválida.";
                m_status.store(UpdateStatus::FAILED);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
            std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength);
            bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

            HINTERNET hSession = WinHttpOpen(L"AbductionStudio-Updater/2.0",
                                             WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                             WINHTTP_NO_PROXY_NAME,
                                             WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) {
                m_errorMessage = "Falha ao inicializar WinHTTP.";
                m_status.store(UpdateStatus::FAILED);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), urlComp.nPort, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                m_errorMessage = "Não foi possível conectar ao servidor de atualizações.";
                m_status.store(UpdateStatus::FAILED);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                                   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                m_errorMessage = "Falha ao criar requisição HTTP.";
                m_status.store(UpdateStatus::FAILED);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));

            std::wstring headers;
            if (!m_secretKey.empty()) {
                std::wstring wKey(m_secretKey.begin(), m_secretKey.end());
                headers = L"X-Abduction-Key: " + wKey + L"\r\n";
            }

            std::string responseBody;
            if (WinHttpSendRequest(hRequest, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
                                  (DWORD)headers.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                WinHttpReceiveResponse(hRequest, NULL)) {

                DWORD dwSize = 0;
                DWORD dwDownloaded = 0;
                do {
                    dwSize = 0;
                    if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                        std::vector<char> buffer(dwSize + 1, 0);
                        if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                            responseBody.append(buffer.data(), dwDownloaded);
                        }
                    }
                } while (dwSize > 0);
            }

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            if (responseBody.empty()) {
                m_errorMessage = "Resposta vazia do servidor de atualizações.";
                m_status.store(UpdateStatus::FAILED);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            // Interpreta campos do JSON
            std::string updateAvailStr = ExtractJsonString(responseBody, "update_available");
            m_manifest.update_available = (updateAvailStr == "true");
            m_manifest.latest_version = ExtractJsonString(responseBody, "latest_version");
            m_manifest.title = ExtractJsonString(responseBody, "title");
            m_manifest.changelog = ExtractJsonString(responseBody, "changelog");
            m_manifest.download_url = ExtractJsonString(responseBody, "download_url");

            std::string sizeStr = ExtractJsonString(responseBody, "size_bytes");
            if (!sizeStr.empty()) {
                try { m_manifest.size_bytes = std::stoull(sizeStr); } catch (...) { m_manifest.size_bytes = 0; }
            }

            if (m_manifest.update_available) {
                m_status.store(UpdateStatus::AVAILABLE);
                m_showModal = true; // Abre o diálogo visual avisando o usuário
            } else {
                m_status.store(UpdateStatus::UP_TO_DATE);
                if (m_userInitiated) {
                    m_showModal = true;
                }
            }
        }

        // Executa o download com streaming e barra de progresso
        void PerformDownload() {
            std::string downloadUrl = m_manifest.download_url;
            if (downloadUrl.empty()) {
                m_errorMessage = "URL de download vazia.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            wchar_t tempPathBuf[MAX_PATH];
            GetTempPathW(MAX_PATH, tempPathBuf);
            m_tempZipPath = std::filesystem::path(tempPathBuf) / "AbductionStudio_Update.zip";

            std::wstring wurl(downloadUrl.begin(), downloadUrl.end());
            URL_COMPONENTS urlComp = { 0 };
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwSchemeLength = (DWORD)-1;
            urlComp.dwHostNameLength = (DWORD)-1;
            urlComp.dwUrlPathLength = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
                m_errorMessage = "URL de download inválida.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
            std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength);
            bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

            HINTERNET hSession = WinHttpOpen(L"AbductionStudio-Updater/2.0",
                                             WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                             WINHTTP_NO_PROXY_NAME,
                                             WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) {
                m_errorMessage = "Falha ao inicializar sessão de download.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), urlComp.nPort, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                m_errorMessage = "Falha ao conectar para download.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                                   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                m_errorMessage = "Falha ao abrir requisição de download.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));

            std::wstring headers;
            if (!m_secretKey.empty()) {
                std::wstring wKey(m_secretKey.begin(), m_secretKey.end());
                headers = L"X-Abduction-Key: " + wKey + L"\r\n";
            }

            if (!WinHttpSendRequest(hRequest, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
                                   (DWORD)headers.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
                !WinHttpReceiveResponse(hRequest, NULL)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                m_errorMessage = "Falha ao receber resposta do servidor de download.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            // Obtém tamanho total do cabeçalho Content-Length
            DWORD dwContentLength = 0;
            DWORD dwSizeOfLength = sizeof(dwContentLength);
            if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                                    WINHTTP_HEADER_NAME_BY_INDEX, &dwContentLength, &dwSizeOfLength, WINHTTP_NO_HEADER_INDEX)) {
                m_totalBytes.store(dwContentLength);
            } else {
                m_totalBytes.store(m_manifest.size_bytes);
            }

            HANDLE hFile = CreateFileW(m_tempZipPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                m_errorMessage = "Não foi possível criar o arquivo temporário de atualização.";
                m_status.store(UpdateStatus::FAILED);
                return;
            }

            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            DWORD bytesWritten = 0;
            std::vector<char> buffer(65536, 0); // Buffer de 64KB
            size_t totalReceived = 0;

            do {
                dwSize = 0;
                if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                    DWORD toRead = std::min(dwSize, (DWORD)buffer.size());
                    if (WinHttpReadData(hRequest, buffer.data(), toRead, &dwDownloaded) && dwDownloaded > 0) {
                        WriteFile(hFile, buffer.data(), dwDownloaded, &bytesWritten, NULL);
                        totalReceived += dwDownloaded;
                        m_downloadedBytes.store(totalReceived);

                        if (m_totalBytes.load() > 0) {
                            m_progress.store((float)totalReceived / (float)m_totalBytes.load());
                        }
                    }
                }
            } while (dwSize > 0);

            CloseHandle(hFile);
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            if (totalReceived > 0) {
                m_progress.store(1.0f);
                m_status.store(UpdateStatus::READY_TO_INSTALL);
            } else {
                m_errorMessage = "Download finalizado com tamanho zero.";
                m_status.store(UpdateStatus::FAILED);
            }
        }
    };

} // namespace KuroAudio
