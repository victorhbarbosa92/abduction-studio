#include <windows.h>
#include <iostream>
#include <string.h>
#include "clap/clap.h"

// Basic GUI
bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    std::cout << "[DummyCLAP] gui_create chamada!\n";
    return true;
}
bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    std::cout << "[DummyCLAP] is_api_supported chamado para " << api << "!\n";
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
}
bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    std::cout << "[DummyCLAP] set_parent recebendo HWND: " << window->win32 << "\n";
    return true;
}
bool gui_show(const clap_plugin_t *plugin) {
    std::cout << "[DummyCLAP] SHOW GUI!!!\n";
    MessageBoxA(NULL, "Sua DAW hosteou esta Janela Win32 com SUCESSO!", "Dummy CLAP VST", MB_OK | MB_ICONINFORMATION);
    return true;
}
void gui_destroy(const clap_plugin_t *plugin) {}

const clap_plugin_gui_t clap_gui = {
    gui_is_api_supported,
    NULL, // get_preferred_api
    gui_create,
    gui_destroy,
    NULL, // set_scale
    NULL, // get_size
    NULL, // can_resize
    NULL, // get_resize_hints
    NULL, // adjust_size
    NULL, // set_size
    gui_set_parent,
    NULL, // set_transient
    NULL, // suggest_title
    gui_show,
    NULL, // hide
};

// Plugin instance
bool plugin_init(const clap_plugin_t *plugin) {
    std::cout << "[DummyCLAP] plugin_init chamado.\n";
    return true;
}
void plugin_destroy(const clap_plugin_t *plugin) {}
const void* get_extension(const clap_plugin_t *plugin, const char *id) {
    if (strcmp(id, CLAP_EXT_GUI) == 0) return &clap_gui;
    return NULL;
}

clap_plugin_t plugin_instance = {
    NULL, // desc
    NULL, // plugin_data
    plugin_init,
    plugin_destroy,
    NULL, // activate
    NULL, // deactivate
    NULL, // start_processing
    NULL, // stop_processing
    NULL, // reset
    NULL, // process
    get_extension,
    NULL, // on_main_thread
};

// Factory
uint32_t get_plugin_count(const clap_plugin_factory_t *factory) { return 1; }
static const char* features[] = { CLAP_PLUGIN_FEATURE_SYNTHESIZER, NULL };
clap_plugin_descriptor_t desc = {
    CLAP_VERSION,
    "com.antigravity.dummy", "Dummy Synth", "Antigravity", "https://antigravity",
    "", "", "1.0", "A dummy synth", features
};
const clap_plugin_descriptor_t* get_plugin_descriptor(const clap_plugin_factory_t *factory, uint32_t index) {
    return &desc;
}
const clap_plugin_t* create_plugin(const clap_plugin_factory_t *factory, const clap_host_t *host, const char *plugin_id) {
    plugin_instance.desc = &desc;
    return &plugin_instance;
}

clap_plugin_factory_t clap_factory = {
    get_plugin_count,
    get_plugin_descriptor,
    create_plugin
};

// Entry
bool entry_init(const char *plugin_path) { return true; }
void entry_deinit() {}
const void* get_factory(const char *factory_id) {
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) return &clap_factory;
    return NULL;
}

extern "C" __declspec(dllexport) const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    entry_init,
    entry_deinit,
    get_factory
};
