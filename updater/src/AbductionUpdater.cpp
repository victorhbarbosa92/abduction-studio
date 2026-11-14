#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <iostream>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

// Janela simples de progresso nativa do Windows
HWND g_hWndProgress = NULL;
HWND g_hProgressBar = NULL;
HWND g_hStatusText  = NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CreateProgressUI(HINSTANCE hInstance, const std::wstring& versionText) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"AbductionUpdaterClass";
    RegisterClassExW(&wcex);

    int winWidth = 420;
    int winHeight = 150;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - winWidth) / 2;
    int posY = (screenH - winHeight) / 2;

    g_hWndProgress = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"AbductionUpdaterClass",
        L"Atualizador do Abduction Studio",
        WS_POPUP | WS_CAPTION | WS_BORDER | WS_VISIBLE,
        posX, posY, winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );

    if (g_hWndProgress) {
        std::wstring title = L"Instalando atualização do Abduction Studio...";
        if (!versionText.empty()) {
            title += L" (" + versionText + L")";
        }

        g_hStatusText = CreateWindowExW(
            0, L"STATIC", title.c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            25, 20, 370, 35,
            g_hWndProgress, NULL, hInstance, NULL
        );

        // Fonte moderna Segoe UI
        HFONT hFont = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        SendMessageW(g_hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hProgressBar = CreateWindowExW(
            0, PROGRESS_CLASSW, NULL,
            WS_CHILD | WS_VISIBLE | PBS_MARQUEE,
            25, 60, 360, 24,
            g_hWndProgress, NULL, hInstance, NULL
        );

        // Ativa animação contínua (Marquee)
        SendMessageW(g_hProgressBar, PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)30);
        UpdateWindow(g_hWndProgress);
    }
}

void PumpMessages() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Executa comando silencioso e aguarda término
bool RunSilentCommand(const std::wstring& cmd) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back(0);

    if (!CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }

    // Processa eventos da janela de progresso enquanto aguarda
    while (WaitForSingleObject(pi.hProcess, 50) == WAIT_TIMEOUT) {
        PumpMessages();
    }

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (exitCode == 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return 1;

    DWORD targetPid = 0;
    std::wstring zipPath;
    std::wstring targetDir;
    std::wstring exeName = L"AbductionStudioV2.exe";
    std::wstring versionStr;
    bool silent = false;

    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];
        if (arg == L"--pid" && i + 1 < argc) {
            targetPid = (DWORD)_wtoi(argv[++i]);
        } else if (arg == L"--zip" && i + 1 < argc) {
            zipPath = argv[++i];
        } else if (arg == L"--target" && i + 1 < argc) {
            targetDir = argv[++i];
        } else if (arg == L"--exe" && i + 1 < argc) {
            exeName = argv[++i];
        } else if (arg == L"--version" && i + 1 < argc) {
            versionStr = argv[++i];
        } else if (arg == L"--silent") {
            silent = true;
        }
    }
    LocalFree(argv);

    if (zipPath.empty() || targetDir.empty()) {
        MessageBoxW(NULL, L"Parâmetros insuficientes para o atualizador.", L"Abduction Studio Updater", MB_ICONERROR);
        return 1;
    }

    // 1. Aguarda o processo anterior do Abduction Studio encerrar completamente
    if (targetPid > 0) {
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, targetPid);
        if (hProcess != NULL) {
            WaitForSingleObject(hProcess, 15000);
            CloseHandle(hProcess);
        }
    }

    // Aguarda liberação dos arquivos no Windows
    Sleep(600);

    // 2. Exibe barra de progresso visual se não estiver em modo silencioso
    if (!silent) {
        CreateProgressUI(hInstance, versionStr);
        PumpMessages();
    }

    // 3. Extrai o pacote de atualização sobre a pasta de destino
    // Método Primário: tar.exe nativo do Windows 10/11
    std::wstring tarCmd = L"tar.exe -xf \"" + zipPath + L"\" -C \"" + targetDir + L"\"";
    bool extractSuccess = RunSilentCommand(tarCmd);

    // Método de Contingência: PowerShell Expand-Archive se o tar falhar
    if (!extractSuccess) {
        std::wstring psCmd = L"powershell.exe -NoProfile -NonInteractive -Command \"Expand-Archive -Path '" + 
                             zipPath + L"' -DestinationPath '" + targetDir + L"' -Force\"";
        extractSuccess = RunSilentCommand(psCmd);
    }

    // Fecha a janela de progresso
    if (g_hWndProgress) {
        DestroyWindow(g_hWndProgress);
    }

    if (!extractSuccess) {
        MessageBoxW(NULL, L"Não foi possível extrair os arquivos da atualização. Verifique as permissões de gravação.",
                    L"Erro na Atualização", MB_ICONERROR);
        return 2;
    }

    // 4. Remove o arquivo zip temporário
    DeleteFileW(zipPath.c_str());

    // 5. Reinicia o Abduction Studio atualizado
    std::wstring fullExePath = targetDir;
    if (!fullExePath.empty() && fullExePath.back() != L'\\') fullExePath += L'\\';
    fullExePath += exeName;

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessW(fullExePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, targetDir.c_str(), &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::wstring errMsg = L"A atualização foi instalada, mas ocorreu um erro ao reabrir o aplicativo:\n" + fullExePath;
        MessageBoxW(NULL, errMsg.c_str(), L"Abduction Studio", MB_ICONINFORMATION);
    }

    return 0;
}
