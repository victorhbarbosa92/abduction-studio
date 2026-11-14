#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

namespace KuroAudio {

    class NativeHttpClient {
    public:
        // Helper para codificar strings em formato de URL (percent-encoding)
        static std::string urlEncode(const std::string& value) {
            std::ostringstream escaped;
            escaped.fill('0');
            escaped << std::hex;
            for (char c : value) {
                if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
                    escaped << c;
                } else if (c == ' ') {
                    escaped << '+';
                } else {
                    escaped << '%' << std::setw(2) << std::uppercase << ((int)(unsigned char)c);
                }
            }
            return escaped.str();
        }

        // Requisição HTTP/HTTPS GET de alta performance com WinHTTP nativo
        static std::string get(const std::string& url_str, int timeout_ms = 8000) {
#ifdef _WIN32
            std::wstring wurl(url_str.begin(), url_str.end());
            URL_COMPONENTS urlComp;
            ZeroMemory(&urlComp, sizeof(urlComp));
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwSchemeLength    = (DWORD)-1;
            urlComp.dwHostNameLength  = (DWORD)-1;
            urlComp.dwUrlPathLength   = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
                return "";
            }

            std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
            std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength);
            bool is_https = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
            INTERNET_PORT port = urlComp.nPort;

            HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AbductionStudio/2.0",
                                             WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                             WINHTTP_NO_PROXY_NAME,
                                             WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) return "";

            WinHttpSetTimeouts(hSession, timeout_ms / 2, timeout_ms / 2, timeout_ms, timeout_ms);

            HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                return "";
            }

            DWORD flags = is_https ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                                   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Seguir redirecionamentos HTTP 301/302 automaticamente
            DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));

            std::string result;
            if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                WinHttpReceiveResponse(hRequest, NULL)) {
                
                DWORD dwSize = 0;
                DWORD dwDownloaded = 0;
                do {
                    dwSize = 0;
                    if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                        std::vector<char> buffer(dwSize + 1, 0);
                        if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                            result.append(buffer.data(), dwDownloaded);
                        }
                    }
                } while (dwSize > 0);
            }

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
#else
            return "";
#endif
        }

        // Download de arquivo em streaming direto para o disco (sem estourar RAM)
        static bool downloadFile(const std::string& url_str, const std::filesystem::path& dest_path, int timeout_ms = 15000) {
#ifdef _WIN32
            std::wstring wurl(url_str.begin(), url_str.end());
            URL_COMPONENTS urlComp;
            ZeroMemory(&urlComp, sizeof(urlComp));
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwSchemeLength    = (DWORD)-1;
            urlComp.dwHostNameLength  = (DWORD)-1;
            urlComp.dwUrlPathLength   = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
                return false;
            }

            std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
            std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength);
            bool is_https = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
            INTERNET_PORT port = urlComp.nPort;

            HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AbductionStudio/2.0",
                                             WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                             WINHTTP_NO_PROXY_NAME,
                                             WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) return false;

            WinHttpSetTimeouts(hSession, timeout_ms / 2, timeout_ms / 2, timeout_ms, timeout_ms);

            HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                return false;
            }

            DWORD flags = is_https ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                                   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return false;
            }

            DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));

            bool success = false;
            if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                WinHttpReceiveResponse(hRequest, NULL)) {

                DWORD statusCode = 0;
                DWORD statusSize = sizeof(statusCode);
                WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                    WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);

                if (statusCode >= 200 && statusCode < 400) {
                    std::ofstream outfile(dest_path, std::ios::binary);
                    if (outfile.is_open()) {
                        DWORD dwSize = 0;
                        DWORD dwDownloaded = 0;
                        std::vector<char> buffer(65536, 0);
                        size_t total = 0;
                        do {
                            dwSize = 0;
                            if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                                DWORD toRead = std::min(dwSize, (DWORD)buffer.size());
                                if (WinHttpReadData(hRequest, buffer.data(), toRead, &dwDownloaded)) {
                                    outfile.write(buffer.data(), dwDownloaded);
                                    total += dwDownloaded;
                                }
                            }
                        } while (dwSize > 0);
                        outfile.close();
                        success = (total > 0);
                    }
                }
            }

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return success;
#else
            return false;
#endif
        }
    };

} // namespace KuroAudio
