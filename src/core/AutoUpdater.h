#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <iostream>
#include <fstream>
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
#include "ui/UIScaleManager.h"

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
        void Initialize(const std::string& defaultWorkerUrl = "", const std::string& fallbackVersion = "2.0.0", const std::string& secretKey = "") {
            m_workerUrl = defaultWorkerUrl;
            m_currentVersion = fallbackVersion;
            m_secretKey = secretKey;

            // Tenta carregar override do updater.json (incluindo a versão atual gravada)
            LoadConfig();

            // Fallback padrao caso nao haja configuracao
            if (m_workerUrl.empty() || m_workerUrl.find("YOUR_SUBDOMAIN") != std::string::npos) {
                m_workerUrl = "https://api.github.com/repos/victorhbarbosa92/abduction-studio";
            }
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
                        std::string ver = ExtractJsonString(content, "version");
                        if (!ver.empty()) m_currentVersion = ver;
                    }
                }
            } catch (...) {}
        }

        void SaveConfig(const std::filesystem::path& configPath = "updater.json") {
            try {
                std::ofstream file(configPath);
                if (file.is_open()) {
                    file << "{\n";
                    file << "  \"version\": \"" << m_currentVersion << "\",\n";
                    file << "  \"worker_url\": \"" << m_workerUrl << "\",\n";
                    file << "  \"secret_key\": \"" << m_secretKey << "\"\n";
                    file << "}\n";
                }
            } catch (...) {}
        }

        const std::string& GetCurrentVersion() const { return m_currentVersion; }
        void SetCurrentVersion(const std::string& ver) { m_currentVersion = ver; SaveConfig(); }

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

            // Grava a nova versão antes de aplicar o restart
            if (!m_manifest.latest_version.empty()) {
                m_currentVersion = m_manifest.latest_version;
                SaveConfig();
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

        // Renderiza o Modal/Diálogo de atualização com tema ImGui e escala adaptativa perfeita
        void RenderUI() {
            // Atalho de teclado: Ctrl+U para verificar atualizacoes manualmente
            if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_U, false)) {
                CheckForUpdatesAsync(true);
            }

            if (!m_showModal) return;

            ImGui::OpenPopup("Atualização do Abduction Studio##UpdaterModal");

            float scale = KuroUI::UIScaleManager::getScale();
            auto S = [scale](float v) { return v * scale; };

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 center = viewport->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            float maxW = viewport->Size.x * 0.90f;
            float maxH = viewport->Size.y * 0.90f;
            float minW = S(480.0f);
            ImGui::SetNextWindowSizeConstraints(ImVec2(minW, 0.0f), ImVec2(maxW, maxH));

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(20.0f), S(18.0f)));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(S(12.0f), S(10.0f)));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, S(6.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, S(8.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.08f, 0.08f, 0.11f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.75f, 0.45f, 0.45f));

            if (ImGui::BeginPopupModal("Atualização do Abduction Studio##UpdaterModal", &m_showModal, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
                UpdateStatus current = m_status.load();

                if (current == UpdateStatus::CHECKING) {
                    ImGui::TextUnformatted("Verificando se há atualizações disponíveis no GitHub...");
                    ImGui::Spacing();
                }
                else if (current == UpdateStatus::AVAILABLE) {
                    ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.45f, 1.0f), "Nova versao encontrada: v%s", m_manifest.latest_version.c_str());
                    ImGui::TextDisabled("Versao atualmente instalada: v%s", m_currentVersion.c_str());
                    ImGui::Separator();

                    if (!m_manifest.changelog.empty()) {
                        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.92f, 1.0f), "Novidades e alteracoes:");
                        
                        // Bloco destacado sem nenhuma barra de rolagem (altura automatica)
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.17f, 0.90f));
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(14.0f), S(12.0f)));

                        float wrapWidth = S(450.0f);
                        ImVec2 textSize = ImGui::CalcTextSize(m_manifest.changelog.c_str(), nullptr, false, wrapWidth);
                        float boxH = textSize.y + S(28.0f);

                        if (ImGui::BeginChild("ChangelogBox", ImVec2(wrapWidth, boxH), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
                            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapWidth - S(14.0f));
                            ImGui::TextUnformatted(m_manifest.changelog.c_str());
                            ImGui::PopTextWrapPos();
                            ImGui::EndChild();
                        }
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor();
                    }

                    if (m_manifest.size_bytes > 0) {
                        double mb = (double)m_manifest.size_bytes / (1024.0 * 1024.0);
                        ImGui::TextDisabled("Tamanho do download: %.1f MB", mb);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();

                    // Botoes dimensionados automaticamente com folga generosa para nunca cortar texto
                    float btnH = S(38.0f);
                    const char* txt1 = "Baixar e Atualizar Agora";
                    const char* txt2 = "Lembrar Mais Tarde";
                    float padX = S(22.0f);
                    float w1 = ImGui::CalcTextSize(txt1).x + padX * 2;
                    float w2 = ImGui::CalcTextSize(txt2).x + padX * 2;

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.62f, 0.35f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.78f, 0.45f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.50f, 0.28f, 1.0f));
                    if (ImGui::Button(txt1, ImVec2(w1, btnH))) {
                        StartDownloadAsync();
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine(0, S(12.0f));

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.22f, 0.28f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.32f, 0.40f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.18f, 0.24f, 1.0f));
                    if (ImGui::Button(txt2, ImVec2(w2, btnH))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopStyleColor(3);
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

                    ImGui::ProgressBar(prog, ImVec2(S(460.0f), S(26.0f)), overlay);
                    ImGui::Spacing();
                    ImGui::TextDisabled("Aguarde a conclusao do download para reiniciar.");
                }
                else if (current == UpdateStatus::READY_TO_INSTALL) {
                    ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.45f, 1.0f), "Download concluido com sucesso!");
                    ImGui::Spacing();
                    ImGui::TextWrapped("O Abduction Studio precisa reiniciar para aplicar os novos arquivos.");
                    ImGui::Separator();

                    float btnH = S(38.0f);
                    const char* txtRestart = "Reiniciar e Aplicar Agora";
                    const char* txtLater = "Instalar ao Fechar";
                    float padX = S(22.0f);
                    float wRestart = ImGui::CalcTextSize(txtRestart).x + padX * 2;
                    float wLater = ImGui::CalcTextSize(txtLater).x + padX * 2;

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.62f, 0.35f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.78f, 0.45f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.50f, 0.28f, 1.0f));
                    if (ImGui::Button(txtRestart, ImVec2(wRestart, btnH))) {
                        ApplyUpdateAndRestart();
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine(0, S(12.0f));

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.22f, 0.28f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.32f, 0.40f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.18f, 0.24f, 1.0f));
                    if (ImGui::Button(txtLater, ImVec2(wLater, btnH))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopStyleColor(3);
                }
                else if (current == UpdateStatus::UP_TO_DATE) {
                    ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "Seu Abduction Studio ja esta atualizado!");
                    ImGui::TextDisabled("Versao instalada: v%s", m_currentVersion.c_str());
                    ImGui::Spacing();
                    if (ImGui::Button("OK", ImVec2(S(120.0f), S(34.0f)))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }
                else if (current == UpdateStatus::FAILED) {
                    ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Verificacao de atualizacao:");
                    ImGui::TextWrapped("%s", m_errorMessage.c_str());
                    ImGui::Spacing();
                    if (ImGui::Button("Fechar", ImVec2(S(120.0f), S(34.0f)))) {
                        m_showModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(4);
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

        static int CompareVersions(const std::string& v1, const std::string& v2) {
            auto parse = [](const std::string& s) {
                std::vector<int> parts;
                std::stringstream ss(s);
                std::string item;
                while (std::getline(ss, item, '.')) {
                    try { parts.push_back(std::stoi(item)); } catch (...) { parts.push_back(0); }
                }
                return parts;
            };
            std::string s1 = v1; if (!s1.empty() && (s1[0] == 'v' || s1[0] == 'V')) s1 = s1.substr(1);
            std::string s2 = v2; if (!s2.empty() && (s2[0] == 'v' || s2[0] == 'V')) s2 = s2.substr(1);
            auto p1 = parse(s1);
            auto p2 = parse(s2);
            size_t len = std::max(p1.size(), p2.size());
            p1.resize(len, 0);
            p2.resize(len, 0);
            for (size_t i = 0; i < len; ++i) {
                if (p1[i] > p2[i]) return 1;
                if (p1[i] < p2[i]) return -1;
            }
            return 0;
        }

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

            bool isDirectGitHub = (m_workerUrl.find("api.github.com") != std::string::npos);

            std::string queryUrl = m_workerUrl;
            if (isDirectGitHub) {
                if (queryUrl.back() == '/') queryUrl.pop_back();
                queryUrl += "/releases/latest";
            } else {
                if (queryUrl.back() != '/') queryUrl += '/';
                queryUrl += "api/check-update?version=" + m_currentVersion;
                if (!m_secretKey.empty()) {
                    queryUrl += "&key=" + m_secretKey;
                }
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

            std::wstring headers = L"User-Agent: AbductionStudio-Updater/2.0\r\n";
            if (!m_secretKey.empty()) {
                std::wstring wKey(m_secretKey.begin(), m_secretKey.end());
                headers += L"X-Abduction-Key: " + wKey + L"\r\n";
            }

            std::string responseBody;
            if (WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)headers.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
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
                m_errorMessage = "Nenhuma atualização encontrada ou servidor indisponível.";
                m_status.store(UpdateStatus::UP_TO_DATE);
                if (m_userInitiated) m_showModal = true;
                return;
            }

            if (isDirectGitHub) {
                // Parse direto da API do GitHub Releases
                std::string tag = ExtractJsonString(responseBody, "tag_name");
                std::string cleanTag = tag;
                if (!cleanTag.empty() && (cleanTag[0] == 'v' || cleanTag[0] == 'V')) {
                    cleanTag = cleanTag.substr(1);
                }

                if (cleanTag.empty()) {
                    m_status.store(UpdateStatus::UP_TO_DATE);
                    if (m_userInitiated) m_showModal = true;
                    return;
                }

                bool isNewer = (CompareVersions(cleanTag, m_currentVersion) > 0);
                m_manifest.update_available = isNewer;
                m_manifest.latest_version = cleanTag;
                m_manifest.title = ExtractJsonString(responseBody, "name");
                m_manifest.changelog = ExtractJsonString(responseBody, "body");
                m_manifest.download_url = ExtractJsonString(responseBody, "browser_download_url");

                std::string sizeStr = ExtractJsonString(responseBody, "size");
                if (!sizeStr.empty()) {
                    try { m_manifest.size_bytes = std::stoull(sizeStr); } catch (...) { m_manifest.size_bytes = 0; }
                }

                if (isNewer && !m_manifest.download_url.empty()) {
                    m_status.store(UpdateStatus::AVAILABLE);
                    m_showModal = true;
                } else {
                    m_status.store(UpdateStatus::UP_TO_DATE);
                    if (m_userInitiated) m_showModal = true;
                }
            } else {
                // Parse do Cloudflare Worker Gateway
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
                    m_showModal = true;
                } else {
                    m_status.store(UpdateStatus::UP_TO_DATE);
                    if (m_userInitiated) m_showModal = true;
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

            std::wstring headers = L"User-Agent: AbductionStudio-Updater/2.0\r\n";
            if (!m_secretKey.empty()) {
                std::wstring wKey(m_secretKey.begin(), m_secretKey.end());
                headers += L"X-Abduction-Key: " + wKey + L"\r\n";
            }

            if (!WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)headers.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
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
