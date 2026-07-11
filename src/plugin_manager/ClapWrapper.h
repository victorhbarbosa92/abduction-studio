#pragma once
#include "DAG.h"
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include "../thirdparty/clap-main/include/clap/clap.h"

extern HWND main_hwnd;

namespace KuroDSP {

    class ClapWrapper : public PluginNode {
    private:
        HMODULE dll_handle = nullptr;
        bool is_loaded = false;
        
        // Ponteiros C-ABI do CLAP
        const clap_plugin_entry_t* clap_entry = nullptr;
        const clap_plugin_factory_t* clap_factory = nullptr;
        const clap_plugin_t* clap_plugin = nullptr;
        
        // Extensões
        const clap_plugin_gui_t* clap_gui = nullptr;

        // Fila de Eventos MIDI pendentes para o próximo bloco DSP
        std::vector<clap_event_note_t> pending_events;

        // Estrutura host estática (C-ABI)
        static clap_host_t host;
        
        // Callbacks obrigatórios do Host
        static const void* get_extension(const clap_host_t* host, const char* extension_id) {
            return nullptr; // Por enquanto, sem extensões de GUI ou Áudio
        }
        
        static void request_restart(const clap_host_t* host) {}
        static void request_process(const clap_host_t* host) {}
        static void request_callback(const clap_host_t* host) {}

        // Callbacks para a Event List
        static uint32_t event_list_size(const struct clap_input_events *list) { 
            ClapWrapper* cw = static_cast<ClapWrapper*>(list->ctx);
            return cw->pending_events.size();
        }
        static const clap_event_header_t* event_list_get(const struct clap_input_events *list, uint32_t index) {
            ClapWrapper* cw = static_cast<ClapWrapper*>(list->ctx);
            if (index >= cw->pending_events.size()) return nullptr;
            return (const clap_event_header_t*)&cw->pending_events[index].header;
        }
        static bool dummy_event_list_try_push(const struct clap_output_events *list, const clap_event_header_t *event) { return true; }

    public:
        ClapWrapper(const std::string& id, const std::string& name) 
            : PluginNode(id, name) {
            
            // Inicializando a estrutura do Host
            host.clap_version = CLAP_VERSION;
            host.host_data = this;
            host.name = "Abduction Studio V2";
            host.vendor = "Antigravity";
            host.url = "https://abduction.studio";
            host.version = "1.0.0";
            host.get_extension = get_extension;
            host.request_restart = request_restart;
            host.request_process = request_process;
            host.request_callback = request_callback;
        }

        ~ClapWrapper() {
            unload();
        }

        bool load(const std::string& dll_path) {
            dll_handle = LoadLibraryA(dll_path.c_str());
            if (!dll_handle) {
                std::cerr << "[CLAP] Erro ao carregar DLL: " << dll_path << "\n";
                return false;
            }
            
            // 1. Obter Entry Point
            clap_entry = (const clap_plugin_entry_t*)GetProcAddress(dll_handle, "clap_entry");
            if (!clap_entry) {
                std::cerr << "[CLAP] Erro: dll não exporta clap_entry!\n";
                return false;
            }
            
            // 2. Inicializar Entry
            if (!clap_entry->init(dll_path.c_str())) {
                std::cerr << "[CLAP] Erro: clap_entry->init() falhou.\n";
                return false;
            }
            
            // 3. Obter Factory
            clap_factory = (const clap_plugin_factory_t*)clap_entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
            if (!clap_factory) {
                std::cerr << "[CLAP] Erro: factory não encontrada.\n";
                return false;
            }
            
            uint32_t count = clap_factory->get_plugin_count(clap_factory);
            if (count == 0) {
                std::cerr << "[CLAP] DLL não contém plugins.\n";
                return false;
            }
            
            // 4. Instanciar o primeiro plugin
            const clap_plugin_descriptor_t* desc = clap_factory->get_plugin_descriptor(clap_factory, 0);
            clap_plugin = clap_factory->create_plugin(clap_factory, &host, desc->id);
            
            if (!clap_plugin) {
                std::cerr << "[CLAP] Erro ao instanciar plugin: " << desc->id << "\n";
                return false;
            }
            
            // 5. Inicializar Instância
            if (!clap_plugin->init(clap_plugin)) {
                std::cerr << "[CLAP] Erro ao inicializar instância do plugin.\n";
                return false;
            }
            
            std::cout << "[CLAP] Inicializado com sucesso: " << desc->name << "\n";
            is_loaded = true;
            
            // ==========================================
            // INICIALIZAÇÃO DA INTERFACE GRÁFICA (GUI)
            // ==========================================
            clap_gui = (const clap_plugin_gui_t*)clap_plugin->get_extension(clap_plugin, CLAP_EXT_GUI);
            if (clap_gui) {
                if (clap_gui->is_api_supported(clap_plugin, CLAP_WINDOW_API_WIN32, false)) {
                    
                    if (clap_gui->create(clap_plugin, CLAP_WINDOW_API_WIN32, false)) {
                        
                        clap_window_t win;
                        win.api = CLAP_WINDOW_API_WIN32;
                        win.win32 = (void*)main_hwnd;
                        
                        clap_gui->set_parent(clap_plugin, &win);
                        clap_gui->show(clap_plugin);
                        std::cout << "[CLAP] GUI Win32 ancorada com sucesso!\n";
                    }
                }
            } else {
                std::cout << "[CLAP] Plugin não possui interface gráfica (CLAP_EXT_GUI).\n";
            }
            
            // ==========================================
            // ATIVAÇÃO DO MOTOR DE ÁUDIO
            // ==========================================
            if (clap_plugin->activate(clap_plugin, 44100, 1024, 1024)) {
                if (clap_plugin->start_processing(clap_plugin)) {
                    std::cout << "[CLAP] Processamento de Áudio Iniciado!\n";
                } else {
                    std::cerr << "[CLAP] Erro: start_processing falhou.\n";
                }
            } else {
                std::cerr << "[CLAP] Erro: activate falhou.\n";
            }

            return true;
        }

        void unload() {
            if (clap_gui && clap_plugin) {
                clap_gui->destroy(clap_plugin);
            }
            if (clap_plugin) {
                clap_plugin->stop_processing(clap_plugin);
                clap_plugin->deactivate(clap_plugin);
                clap_plugin->destroy(clap_plugin);
                clap_plugin = nullptr;
            }
            if (clap_entry) {
                clap_entry->deinit();
                clap_entry = nullptr;
            }
            if (dll_handle) {
                FreeLibrary(dll_handle);
                dll_handle = nullptr;
            }
            is_loaded = false;
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (!is_loaded || is_bypassed) return;
            
            // 1. Configurar Buffers de Entrada e Saída
            float* in_channels[2] = { left, right };
            float* out_channels[2] = { left, right };
            
            clap_audio_buffer_t audio_buffer;
            audio_buffer.data32 = in_channels;
            audio_buffer.data64 = nullptr;
            audio_buffer.channel_count = 2;
            audio_buffer.latency = 0;
            audio_buffer.constant_mask = 0;

            clap_audio_buffer_t out_audio_buffer;
            out_audio_buffer.data32 = out_channels;
            out_audio_buffer.data64 = nullptr;
            out_audio_buffer.channel_count = 2;
            out_audio_buffer.latency = 0;
            out_audio_buffer.constant_mask = 0;
            
            // 2. Configurar Listas de Eventos (Injetando a Fila MIDI)
            clap_input_events_t in_events = { this, event_list_size, event_list_get };
            clap_output_events_t out_events = { this, dummy_event_list_try_push };

            // 3. Estrutura Principal de Processamento
            clap_process_t process_data;
            process_data.steady_time = -1;
            process_data.frames_count = frames;
            process_data.transport = nullptr;
            process_data.audio_inputs = &audio_buffer;
            process_data.audio_outputs = &out_audio_buffer;
            process_data.audio_inputs_count = 1;
            process_data.audio_outputs_count = 1;
            process_data.in_events = &in_events;
            process_data.out_events = &out_events;

            // 4. Invocação C-ABI (Execução do Plugin)
            clap_plugin->process(clap_plugin, &process_data);
            
            // Limpa a fila de eventos após processá-los
            pending_events.clear();
        }

        void pushMidiNote(int key, bool is_note_on, float velocity = 1.0f) {
            if (!is_loaded) return;
            clap_event_note_t note_ev;
            note_ev.header.size = sizeof(clap_event_note_t);
            note_ev.header.time = 0; // Início do bloco
            note_ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            note_ev.header.type = is_note_on ? CLAP_EVENT_NOTE_ON : CLAP_EVENT_NOTE_OFF;
            note_ev.header.flags = 0;

            note_ev.note_id = -1; // Wildcard
            note_ev.port_index = -1; // Wildcard
            note_ev.channel = 0;
            note_ev.key = key;
            note_ev.velocity = velocity;

            pending_events.push_back(note_ev);
        }

        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
            if (!is_loaded) return;
        }
    };

    // Inicialização da variável estática
    clap_host_t ClapWrapper::host = {};
}
