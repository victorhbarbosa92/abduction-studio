#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace KuroAudio {

    inline std::string safePathString(const std::filesystem::path& p) {
#ifdef _WIN32
        try {
            std::wstring ws = p.wstring();
            if (ws.empty()) return "";
            int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
            if (len > 0) {
                std::string res(len, 0);
                WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &res[0], len, NULL, NULL);
                std::replace(res.begin(), res.end(), '\\', '/');
                return res;
            }
        } catch (...) {}
#endif
        try {
            return p.generic_string();
        } catch (...) {
            try { return p.string(); } catch (...) { return ""; }
        }
    }

    inline std::string safeStem(const std::filesystem::path& p) {
#ifdef _WIN32
        try {
            std::wstring ws = p.stem().wstring();
            if (ws.empty()) return "";
            int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
            if (len > 0) {
                std::string res(len, 0);
                WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &res[0], len, NULL, NULL);
                return res;
            }
        } catch (...) {}
#endif
        try {
            return p.stem().string();
        } catch (...) {
            return "";
        }
    }

    inline std::string safeFilename(const std::filesystem::path& p) {
#ifdef _WIN32
        try {
            std::wstring ws = p.filename().wstring();
            if (ws.empty()) return "";
            int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
            if (len > 0) {
                std::string res(len, 0);
                WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &res[0], len, NULL, NULL);
                return res;
            }
        } catch (...) {}
#endif
        try {
            return p.filename().string();
        } catch (...) {
            return "";
        }
    }

    inline std::string safeExtension(const std::filesystem::path& p) {
#ifdef _WIN32
        try {
            std::wstring ws = p.extension().wstring();
            if (ws.empty()) return "";
            int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
            if (len > 0) {
                std::string res(len, 0);
                WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &res[0], len, NULL, NULL);
                return res;
            }
        } catch (...) {}
#endif
        try {
            return p.extension().string();
        } catch (...) {
            return "";
        }
    }

    inline std::string escapeJson(const std::string& input) {
        std::string res;
        for (char c : input) {
            if (c == '\\') res += "\\";
            else if (c == '"') res += "\"";
            else if (c == '\n') res += "\n";
            else if (c == '\r') res += "\r";
            else res += c;
        }
        return res;
    }

    inline std::string sanitizeFilename(const std::string& name) {
        std::string clean;
        clean.reserve(name.size());
        for (size_t i = 0; i < name.size(); ++i) {
            unsigned char c = (unsigned char)name[i];
            // Check for UTF-8 non-breaking space (0xC2 0xA0) or mojibake (0xC3 0x82 0xC2 0xA0)
            if (c == 0xC2 && i + 1 < name.size() && (unsigned char)name[i + 1] == 0xA0) {
                clean.push_back(' ');
                i++;
                continue;
            }
            if (c == 0xC3 && i + 1 < name.size() && (unsigned char)name[i + 1] == 0x82) {
                // Â character from Latin-1 mojibake
                i++;
                continue;
            }
            if (c == 0xA0) {
                clean.push_back(' ');
                continue;
            }
            if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                clean.push_back('_');
            } else if (c < 32) {
                clean.push_back(' ');
            } else {
                clean.push_back((char)c);
            }
        }
        // Collapse multiple spaces into single space
        std::string compact;
        compact.reserve(clean.size());
        bool last_space = false;
        for (char ch : clean) {
            if (ch == ' ') {
                if (!last_space) compact.push_back(' ');
                last_space = true;
            } else {
                compact.push_back(ch);
                last_space = false;
            }
        }
        clean = compact;
        while (!clean.empty() && (clean.back() == ' ' || clean.back() == '.')) {
            clean.pop_back();
        }
        while (!clean.empty() && (clean.front() == ' ' || clean.front() == '.')) {
            clean.erase(clean.begin());
        }
        return clean.empty() ? "Track" : clean;
    }

    inline std::wstring toWideUTF8(const std::string& str) {
        if (str.empty()) return L"";
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
        if (len > 0) {
            std::wstring res(len, 0);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &res[0], len);
            return res;
        }
#endif
        try {
            return std::filesystem::u8path(str).wstring();
        } catch (...) {
            return std::wstring(str.begin(), str.end());
        }
    }

    inline bool safeFileExists(const std::string& path) {
        if (path.empty()) return false;
        std::error_code ec;
        try {
            if (std::filesystem::exists(std::filesystem::u8path(path), ec)) return true;
        } catch (...) {}
        try {
            if (std::filesystem::exists(std::filesystem::path(path), ec)) return true;
        } catch (...) {}
        return false;
    }

    inline uintmax_t safeFileSize(const std::string& path) {
        if (path.empty()) return 0;
        std::error_code ec;
        try {
            auto u8 = std::filesystem::u8path(path);
            if (std::filesystem::exists(u8, ec)) return std::filesystem::file_size(u8, ec);
        } catch (...) {}
        try {
            auto p = std::filesystem::path(path);
            if (std::filesystem::exists(p, ec)) return std::filesystem::file_size(p, ec);
        } catch (...) {}
        return 0;
    }

    inline std::filesystem::path getAppDirectory() {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH] = { 0 };
        if (GetModuleFileNameW(NULL, buffer, MAX_PATH)) {
            return std::filesystem::path(buffer).parent_path();
        }
#endif
        std::error_code ec;
        return std::filesystem::current_path(ec);
    }

    inline std::filesystem::path getTracksDirectory() {
        std::error_code ec;
        auto exe_dir = getAppDirectory();

        // 1. exe_dir / "scratch" / "tracks" (quando o exe está na raiz)
        auto p1 = exe_dir / "scratch" / "tracks";
        if (std::filesystem::exists(p1, ec)) return p1;

        // 2. exe_dir.parent_path() / "scratch" / "tracks" (quando o exe está em build/)
        auto p2 = exe_dir.parent_path() / "scratch" / "tracks";
        if (std::filesystem::exists(p2, ec)) return p2;

        // 3. exe_dir.parent_path().parent_path() / "scratch" / "tracks" (quando o exe está em build/Release ou build/Debug)
        auto p3 = exe_dir.parent_path().parent_path() / "scratch" / "tracks";
        if (std::filesystem::exists(p3, ec)) return p3;

        // 4. current_path / "scratch" / "tracks"
        auto p4 = std::filesystem::current_path(ec) / "scratch" / "tracks";
        if (std::filesystem::exists(p4, ec)) return p4;

        // Fallback: se estiver em build, cria no parent
        if (std::filesystem::exists(exe_dir.parent_path() / "CMakeLists.txt", ec)) {
            std::filesystem::create_directories(p2, ec);
            return p2;
        }
        std::filesystem::create_directories(p1, ec);
        return p1;
    }

    inline std::filesystem::path getLibraryDbPath() {
        auto tracks_dir = getTracksDirectory();
        return tracks_dir / "dj_library.json";
    }

    inline std::string resolveExistingAudioPath(const std::string& input_path) {
        if (input_path.empty()) return "";
        std::error_code ec;

        // 1. Verificação direta se o arquivo já existe no caminho absoluto ou relativo fornecido
        try {
            auto p1 = std::filesystem::u8path(input_path);
            if (std::filesystem::exists(p1, ec) && std::filesystem::is_regular_file(p1, ec)) {
                return safePathString(p1);
            }
        } catch (...) {}

        auto tracks_dir = getTracksDirectory();

        // Limpeza de caracteres non-breaking space (0xA0 / 0xC2 0xA0) e mojibake
        std::string cleaned_in = input_path;
        for (size_t i = 0; i < cleaned_in.size(); ++i) {
            if ((unsigned char)cleaned_in[i] == 0xC2 && i + 1 < cleaned_in.size() && (unsigned char)cleaned_in[i + 1] == 0xA0) {
                cleaned_in.replace(i, 2, " ");
            } else if ((unsigned char)cleaned_in[i] == 0xC3 && i + 1 < cleaned_in.size() && (unsigned char)cleaned_in[i + 1] == 0x82) {
                cleaned_in.erase(i, 2);
                if (i > 0) i--;
            } else if ((unsigned char)cleaned_in[i] == 0xA0) {
                cleaned_in[i] = ' ';
            }
        }
        std::replace(cleaned_in.begin(), cleaned_in.end(), '\\', '/');

        // 2. Se o caminho começa com "scratch/tracks/" ou similar, resolve relativo a getTracksDirectory()
        const std::string pfx = "scratch/tracks/";
        if (cleaned_in.rfind(pfx, 0) == 0) {
            std::string sub = cleaned_in.substr(pfx.size());
            auto cand = tracks_dir / std::filesystem::u8path(sub);
            if (std::filesystem::exists(cand, ec) && std::filesystem::is_regular_file(cand, ec)) {
                return safePathString(cand);
            }
        }

        // 3. Verifica diretamente relativo a getTracksDirectory()
        try {
            auto cand2 = tracks_dir / std::filesystem::u8path(cleaned_in);
            if (std::filesystem::exists(cand2, ec) && std::filesystem::is_regular_file(cand2, ec)) {
                return safePathString(cand2);
            }
        } catch (...) {}

        // 4. Busca por nome de arquivo / stem dentro de scratch/tracks e subpastas
        try {
            std::filesystem::path fp = std::filesystem::u8path(cleaned_in);
            std::string fname = safeFilename(fp);
            std::string stem = safeStem(fp);
            if (!fname.empty()) {
                // Direto na raiz de tracks
                auto cand_f = tracks_dir / std::filesystem::u8path(fname);
                if (std::filesystem::exists(cand_f, ec) && std::filesystem::is_regular_file(cand_f, ec)) {
                    return safePathString(cand_f);
                }

                // Subpastas imediatas
                for (const auto& entry : std::filesystem::directory_iterator(tracks_dir, ec)) {
                    try {
                        if (entry.is_directory(ec)) {
                            auto cand_sub = entry.path() / std::filesystem::u8path(fname);
                            if (std::filesystem::exists(cand_sub, ec) && std::filesystem::is_regular_file(cand_sub, ec)) {
                                return safePathString(cand_sub);
                            }
                            // Correspondência por stem ou se a subpasta contiver o arquivo de áudio principal
                            if (!stem.empty() && safeFilename(entry.path()).find(stem) != std::string::npos) {
                                for (const auto& sf : std::filesystem::directory_iterator(entry.path(), ec)) {
                                    if (sf.is_regular_file(ec)) {
                                        std::string ext = safeExtension(sf.path());
                                        if (ext == ".wav" || ext == ".mp3" || ext == ".flac") {
                                            return safePathString(sf.path());
                                        }
                                    }
                                }
                            }
                        }
                    } catch (...) {}
                }
            }
        } catch (...) {}

        return input_path;
    }

    inline std::filesystem::path getToolsDirectory() {
        std::error_code ec;
        auto exe_dir = getAppDirectory();

        auto t1 = exe_dir / "tools";
        if (std::filesystem::exists(t1, ec)) return t1;

        auto t2 = exe_dir.parent_path() / "tools";
        if (std::filesystem::exists(t2, ec)) return t2;

        auto t3 = exe_dir.parent_path().parent_path() / "tools";
        if (std::filesystem::exists(t3, ec)) return t3;

        auto t4 = std::filesystem::current_path(ec) / "tools";
        if (std::filesystem::exists(t4, ec)) return t4;

        return t1;
    }

    inline std::filesystem::path getFfmpegPath() {
        std::error_code ec;
        auto tools_dir = getToolsDirectory();
        auto f1 = tools_dir / "ffmpeg.exe";
        if (std::filesystem::exists(f1, ec)) return f1;
        return "ffmpeg.exe";
    }

    inline std::filesystem::path getPythonExecutablePath() {
        std::error_code ec;
#ifdef _WIN32
        wchar_t* localappdata = nullptr;
        size_t len = 0;
        if (_wdupenv_s(&localappdata, &len, L"LOCALAPPDATA") == 0 && localappdata != nullptr) {
            std::filesystem::path base_lad(localappdata);
            free(localappdata);

            std::vector<std::filesystem::path> candidates = {
                base_lad / L"Programs" / L"Python" / L"Python312" / L"python.exe",
                base_lad / L"Programs" / L"Python" / L"Python311" / L"python.exe",
                base_lad / L"Programs" / L"Python" / L"Python310" / L"python.exe",
                base_lad / L"Programs" / L"Python" / L"Python313" / L"python.exe",
                base_lad / L"Programs" / L"Python" / L"Python314" / L"python.exe",
                base_lad / L"Python" / L"pythoncore-3.14-64" / L"python.exe"
            };
            for (const auto& p : candidates) {
                if (std::filesystem::exists(p, ec)) {
                    return p;
                }
            }
        }
#endif
        return std::filesystem::path(L"python.exe");
    }

    inline std::filesystem::path getWorkerScriptPath() {
        std::error_code ec;
        auto tools_dir = getToolsDirectory();
        auto w1 = tools_dir / "spotify_dj_worker.py";
        if (std::filesystem::exists(w1, ec)) return w1;
        return "tools/spotify_dj_worker.py";
    }

    inline bool RunHiddenProcess(const std::wstring& cmd, int timeout_ms = 90000, const std::wstring& work_dir = L"") {
#ifdef _WIN32
        if (cmd.empty()) return false;

        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        std::vector<wchar_t> cmd_buf(cmd.begin(), cmd.end());
        cmd_buf.push_back(L'\0');

        LPCWSTR lpDir = work_dir.empty() ? NULL : work_dir.c_str();

        BOOL success = CreateProcessW(
            NULL,
            cmd_buf.data(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            lpDir,
            &si,
            &pi
        );

        if (!success) return false;

        DWORD wait_res = WaitForSingleObject(pi.hProcess, timeout_ms);
        DWORD exit_code = 1;
        if (wait_res == WAIT_TIMEOUT) {
            TerminateProcess(pi.hProcess, 1);
        } else {
            GetExitCodeProcess(pi.hProcess, &exit_code);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return (wait_res == WAIT_OBJECT_0 && exit_code == 0);
#else
        return false;
#endif
    }

    inline bool RunCancellableHiddenProcess(const std::wstring& cmd, std::atomic<bool>& cancel_flag, int timeout_ms = 180000) {
#ifdef _WIN32
        if (cmd.empty()) return false;

        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        std::vector<wchar_t> cmd_buf(cmd.begin(), cmd.end());
        cmd_buf.push_back(L'\0');

        HANDLE hJob = CreateJobObjectW(NULL, NULL);
        if (hJob) {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
        }

        BOOL success = CreateProcessW(
            NULL,
            cmd_buf.data(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi
        );

        if (!success) {
            if (hJob) CloseHandle(hJob);
            return false;
        }

        if (hJob) {
            AssignProcessToJobObject(hJob, pi.hProcess);
        }

        int elapsed_ms = 0;
        const int poll_interval_ms = 80;
        DWORD wait_res = WAIT_TIMEOUT;
        bool was_cancelled = false;

        while (elapsed_ms < timeout_ms) {
            if (cancel_flag.load()) {
                was_cancelled = true;
                break;
            }
            wait_res = WaitForSingleObject(pi.hProcess, poll_interval_ms);
            if (wait_res != WAIT_TIMEOUT) {
                break;
            }
            elapsed_ms += poll_interval_ms;
        }

        DWORD exit_code = 1;
        if (was_cancelled || wait_res == WAIT_TIMEOUT) {
            if (hJob) TerminateJobObject(hJob, 1);
            TerminateProcess(pi.hProcess, 1);
        } else {
            GetExitCodeProcess(pi.hProcess, &exit_code);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (hJob) CloseHandle(hJob);

        return (!was_cancelled && wait_res == WAIT_OBJECT_0 && exit_code == 0);
#else
        return false;
#endif
    }

} // namespace KuroAudio
