#pragma once
#include <string>
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <shlobj.h>

namespace KuroUI {

    class FileDialog {
    public:
        static std::string OpenFile(const char* filter = "All Files (*.*)\0*.*\0", HWND owner = NULL) {
            OPENFILENAMEA ofn;
            CHAR szFile[260] = {0};

            ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
            ofn.lStructSize = sizeof(OPENFILENAMEA);
            ofn.hwndOwner = owner;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn) == TRUE) {
                return std::string(ofn.lpstrFile);
            }
            return std::string("");
        }
        static std::string SaveFile(const char* filter = "All Files (*.*)\0*.*\0", HWND owner = NULL) {
            OPENFILENAMEA ofn;
            CHAR szFile[260] = {0};

            ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
            ofn.lStructSize = sizeof(OPENFILENAMEA);
            ofn.hwndOwner = owner;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

            if (GetSaveFileNameA(&ofn) == TRUE) {
                return std::string(ofn.lpstrFile);
            }
            return std::string("");
        }

        // Seleção de pasta usando IFileDialog COM (moderno Windows Vista+)
        static std::string SelectFolder(const char* title = "Selecionar Pasta", HWND owner = NULL) {
            std::string result;
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr)) {
                IFileDialog* pfd = nullptr;
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
                if (SUCCEEDED(hr)) {
                    DWORD dwOptions;
                    pfd->GetOptions(&dwOptions);
                    pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
                    
                    // Converter título para wide string
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
                    std::wstring wtitle(wlen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, title, -1, &wtitle[0], wlen);
                    pfd->SetTitle(wtitle.c_str());
                    
                    hr = pfd->Show(owner);
                    if (SUCCEEDED(hr)) {
                        IShellItem* psi = nullptr;
                        hr = pfd->GetResult(&psi);
                        if (SUCCEEDED(hr)) {
                            PWSTR pszPath = nullptr;
                            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                            if (SUCCEEDED(hr)) {
                                int len = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, NULL, 0, NULL, NULL);
                                result.resize(len - 1);
                                WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, &result[0], len, NULL, NULL);
                                CoTaskMemFree(pszPath);
                            }
                            psi->Release();
                        }
                    }
                    pfd->Release();
                }
                CoUninitialize();
            }
            return result;
        }
    };

}
