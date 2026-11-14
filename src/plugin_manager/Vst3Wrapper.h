#pragma once
#include "DAG.h"
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <mutex>
#include <filesystem>

namespace KuroDSP {

    // =========================================================================
    // DEFINIÇÕES E INTERFACES DA ARQUITETURA STEINBERG VST3 (ABI NATIVA)
    // =========================================================================
    namespace Steinberg {
        typedef int32_t tresult;
        typedef const char* FIDString;
        typedef unsigned char TUID[16];
        typedef int32_t TBool;
        typedef uint32_t ParamID;
        typedef double ParamValue;
        typedef double TQuarterNotes;
        typedef uint64_t SpeakerArrangement;

        static const tresult kResultOk = 0;
        static const tresult kResultTrue = 0;
        static const tresult kResultFalse = 1;
        static const tresult kInvalidArgument = -2147467261;
        static const tresult kNoInterface = -2147467262;

        class FUnknown {
        public:
            virtual tresult queryInterface(const TUID _iid, void** obj) = 0;
            virtual uint32_t addRef() = 0;
            virtual uint32_t release() = 0;
        };

        struct PClassInfo {
            TUID cid;
            int32_t cardinality;
            char category[32];
            char name[64];
        };

        class IPluginFactory : public FUnknown {
        public:
            virtual tresult getFactoryInfo(void* info) = 0;
            virtual int32_t countClasses() = 0;
            virtual tresult getClassInfo(int32_t index, PClassInfo* info) = 0;
            virtual tresult createInstance(FIDString cid, FIDString _iid, void** obj) = 0;
        };

        class IPluginBase : public FUnknown {
        public:
            virtual tresult initialize(FUnknown* context) = 0;
            virtual tresult terminate() = 0;
        };

        class IComponent : public IPluginBase {
        public:
            virtual tresult getControllerClassId(TUID classId) = 0;
            virtual tresult setIoMode(int32_t mode) = 0;
            virtual int32_t getBusCount(int32_t type, int32_t dir) = 0;
            virtual tresult getBusInfo(int32_t type, int32_t dir, int32_t index, void* bus) = 0;
            virtual tresult getRoutingInfo(void* inInfo, void* outInfo) = 0;
            virtual tresult activateBus(int32_t type, int32_t dir, int32_t index, TBool state) = 0;
            virtual tresult setActive(TBool state) = 0;
            virtual tresult setState(void* state) = 0;
            virtual tresult getState(void* state) = 0;
        };

        struct ProcessSetup {
            int32_t processMode;
            int32_t symbolicSampleSize;
            int32_t maxSamplesPerBlock;
            double sampleRate;
        };

        struct AudioBusBuffers {
            int32_t numChannels;
            uint64_t silenceFlags;
            union {
                float** channelBuffers32;
                double** channelBuffers64;
            };
        };

        namespace Vst {
            struct NoteOnEvent {
                int16_t channel;
                int16_t pitch;
                float tuning;
                float velocity;
                int32_t length;
                int32_t noteId;
            };

            struct NoteOffEvent {
                int16_t channel;
                int16_t pitch;
                float velocity;
                int32_t noteId;
                float tuning;
            };

            struct Event {
                int32_t busIndex;
                int32_t sampleOffset;
                TQuarterNotes ppqPosition;
                uint16_t flags;
                enum EventFlags { kIsLive = 1 << 0 };
                enum EventTypes {
                    kNoteOnEvent = 0,
                    kNoteOffEvent = 1
                };
                uint16_t type;
                union {
                    NoteOnEvent noteOn;
                    NoteOffEvent noteOff;
                    unsigned char rawData[32];
                };
            };

            class IEventList : public FUnknown {
            public:
                virtual int32_t getEventCount() = 0;
                virtual tresult getEvent(int32_t index, Event& e) = 0;
                virtual tresult addEvent(Event& e) = 0;
            };

            struct ProcessContext {
                uint32_t state;
                double sampleRate;
                int64_t projectTimeSamples;
                double systemTime;
                double continuousTimeSamples;
                TQuarterNotes projectTimeMusic;
                double barPositionMusic;
                double cycleStartMusic;
                double cycleEndMusic;
                double tempo;
                int32_t timeSigNumerator;
                int32_t timeSigDenominator;
                double chord;
                int32_t smpteOffsetSubframes;
                int32_t frameRate;
                int32_t samplesToNextClock;
            };

            struct ProcessData {
                int32_t processMode;
                int32_t symbolicSampleSize;
                int32_t numSamples;
                int32_t numInputs;
                int32_t numOutputs;
                AudioBusBuffers* inputs;
                AudioBusBuffers* outputs;
                void* inputParameterChanges;
                void* outputParameterChanges;
                IEventList* inputEvents;
                IEventList* outputEvents;
                ProcessContext* processContext;
            };

            class IAudioProcessor : public FUnknown {
            public:
                virtual tresult setBusArrangements(SpeakerArrangement* inputs, int32_t numIns, SpeakerArrangement* outputs, int32_t numOuts) = 0;
                virtual tresult getBusArrangement(int32_t dir, int32_t index, SpeakerArrangement& arr) = 0;
                virtual tresult canProcessSampleSize(int32_t symbolicSampleSize) = 0;
                virtual uint32_t getLatencySamples() = 0;
                virtual tresult setupProcessing(ProcessSetup& setup) = 0;
                virtual tresult setProcessing(TBool state) = 0;
                virtual tresult process(ProcessData& data) = 0;
                virtual uint32_t getTailSamples() = 0;
            };

            struct ViewRect {
                int32_t left;
                int32_t top;
                int32_t right;
                int32_t bottom;
                int32_t getWidth() const { return right - left; }
                int32_t getHeight() const { return bottom - top; }
            };

            class IPlugFrame;

            class IPlugView : public FUnknown {
            public:
                virtual tresult isPlatformTypeSupported(FIDString type) = 0;
                virtual tresult attached(void* parent, FIDString type) = 0;
                virtual tresult removed() = 0;
                virtual tresult onWheel(float distance) = 0;
                virtual tresult onKeyDown(char16_t key, int16_t keyCode, int16_t modifiers) = 0;
                virtual tresult onKeyUp(char16_t key, int16_t keyCode, int16_t modifiers) = 0;
                virtual tresult getSize(ViewRect* size) = 0;
                virtual tresult onSize(ViewRect* newSize) = 0;
                virtual tresult onFocus(TBool state) = 0;
                virtual tresult setFrame(IPlugFrame* frame) = 0;
                virtual tresult canResize() = 0;
                virtual tresult checkSizeConstraint(ViewRect* rect) = 0;
            };

            class IPlugFrame : public FUnknown {
            public:
                virtual tresult resizeView(IPlugView* view, ViewRect* newSize) = 0;
            };

            class IComponentHandler : public FUnknown {
            public:
                virtual tresult beginEdit(ParamID id) = 0;
                virtual tresult performEdit(ParamID id, ParamValue valueNormalized) = 0;
                virtual tresult endEdit(ParamID id) = 0;
                virtual tresult restartComponent(int32_t flags) = 0;
            };

            class IEditController : public IPluginBase {
            public:
                virtual tresult setComponentState(void* state) = 0;
                virtual tresult setState(void* state) = 0;
                virtual tresult getState(void* state) = 0;
                virtual int32_t getParameterCount() = 0;
                virtual tresult getParameterInfo(int32_t paramIndex, void* info) = 0;
                virtual tresult getParamStringByValue(ParamID id, ParamValue valueNormalized, void* string) = 0;
                virtual tresult getParamValueByString(ParamID id, void* string, ParamValue& valueNormalized) = 0;
                virtual ParamValue normalizedParamToPlain(ParamID id, ParamValue valueNormalized) = 0;
                virtual ParamValue plainParamToNormalized(ParamID id, ParamValue plainValue) = 0;
                virtual ParamValue getParamNormalized(ParamID id) = 0;
                virtual tresult setParamNormalized(ParamID id, ParamValue value) = 0;
                virtual tresult setComponentHandler(IComponentHandler* handler) = 0;
                virtual IPlugView* createView(FIDString name) = 0;
            };
        }
    }

    // =========================================================================
    // HOST IMPLEMENTATIONS DE IEventList, IPlugFrame & IComponentHandler
    // =========================================================================
    class Vst3EventList : public Steinberg::Vst::IEventList {
    private:
        std::vector<Steinberg::Vst::Event> events;
        uint32_t ref_count = 1;
        std::mutex list_mutex;

    public:
        Steinberg::tresult queryInterface(const Steinberg::TUID _iid, void** obj) override {
            *obj = this;
            addRef();
            return Steinberg::kResultOk;
        }
        uint32_t addRef() override { return ++ref_count; }
        uint32_t release() override {
            if (--ref_count == 0) return 0;
            return ref_count;
        }

        int32_t getEventCount() override {
            std::lock_guard<std::mutex> lock(list_mutex);
            return (int32_t)events.size();
        }
        Steinberg::tresult getEvent(int32_t index, Steinberg::Vst::Event& e) override {
            std::lock_guard<std::mutex> lock(list_mutex);
            if (index < 0 || index >= (int32_t)events.size()) return Steinberg::kInvalidArgument;
            e = events[index];
            return Steinberg::kResultOk;
        }
        Steinberg::tresult addEvent(Steinberg::Vst::Event& e) override {
            std::lock_guard<std::mutex> lock(list_mutex);
            events.push_back(e);
            return Steinberg::kResultOk;
        }
        void clear() {
            std::lock_guard<std::mutex> lock(list_mutex);
            events.clear();
        }
    };

    class Vst3PlugFrame : public Steinberg::Vst::IPlugFrame {
    private:
        HWND hwnd = nullptr;
        HWND child_hwnd = nullptr;
        uint32_t ref_count = 1;

    public:
        Vst3PlugFrame(HWND h) : hwnd(h) {}
        void setHWND(HWND h, HWND ch = nullptr) { hwnd = h; child_hwnd = ch; }

        Steinberg::tresult queryInterface(const Steinberg::TUID _iid, void** obj) override {
            *obj = this;
            addRef();
            return Steinberg::kResultOk;
        }
        uint32_t addRef() override { return ++ref_count; }
        uint32_t release() override {
            if (--ref_count == 0) return 0;
            return ref_count;
        }

        Steinberg::tresult resizeView(Steinberg::Vst::IPlugView* view, Steinberg::Vst::ViewRect* newSize) override {
            if (hwnd && newSize) {
                RECT r = { 0, 0, newSize->getWidth(), newSize->getHeight() };
                AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
                SetWindowPos(hwnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
                if (child_hwnd) {
                    SetWindowPos(child_hwnd, NULL, 0, 0, newSize->getWidth(), newSize->getHeight(), SWP_NOMOVE | SWP_NOZORDER);
                }
                if (view) view->onSize(newSize);
            }
            return Steinberg::kResultOk;
        }
    };

    class Vst3ComponentHandler : public Steinberg::Vst::IComponentHandler {
    private:
        uint32_t ref_count = 1;

    public:
        Steinberg::tresult queryInterface(const Steinberg::TUID _iid, void** obj) override {
            *obj = this;
            addRef();
            return Steinberg::kResultOk;
        }
        uint32_t addRef() override { return ++ref_count; }
        uint32_t release() override {
            if (--ref_count == 0) return 0;
            return ref_count;
        }

        Steinberg::tresult beginEdit(Steinberg::ParamID id) override { return Steinberg::kResultOk; }
        Steinberg::tresult performEdit(Steinberg::ParamID id, Steinberg::ParamValue valueNormalized) override { return Steinberg::kResultOk; }
        Steinberg::tresult endEdit(Steinberg::ParamID id) override { return Steinberg::kResultOk; }
        Steinberg::tresult restartComponent(int32_t flags) override { return Steinberg::kResultOk; }
    };

    // =========================================================================
    // CLASSE PRINCIPAL Vst3Wrapper
    // =========================================================================
    class Vst3Wrapper : public PluginNode {
    private:
        HMODULE dll_handle = nullptr;
        bool is_loaded = false;
        std::string plugin_path;

        // VST3 Interfaces
        Steinberg::IPluginFactory* factory = nullptr;
        Steinberg::FUnknown* comp_unknown = nullptr;
        Steinberg::IComponent* component = nullptr;
        Steinberg::Vst::IAudioProcessor* audio_processor = nullptr;
        Steinberg::FUnknown* ctrl_unknown = nullptr;
        Steinberg::Vst::IEditController* controller = nullptr;
        Steinberg::Vst::IPlugView* plug_view = nullptr;

        // Host Support
        Vst3EventList event_list;
        Vst3PlugFrame plug_frame{nullptr};
        Vst3ComponentHandler comp_handler;

        // GUI Window State
        HWND gui_hwnd = nullptr;
        HWND child_hwnd = nullptr;
        bool gui_open = false;
        int gui_width = 800;
        int gui_height = 600;

        // Audio Processing Context
        Steinberg::Vst::ProcessContext proc_context = {};
        float sample_rate = 44100.0f;
        int max_block_size = 1024;

        typedef bool (*InitDllFunc)();
        typedef bool (*ExitDllFunc)();
        typedef void* (*GetPluginFactoryFunc)();

        // Window Procedure para a janela flutuante nativa do plugin
        static LRESULT CALLBACK Vst3WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            Vst3Wrapper* wrapper = (Vst3Wrapper*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
            switch (uMsg) {
                case WM_CLOSE:
                    if (wrapper) wrapper->closeGui();
                    return 0;
                case WM_SIZE:
                    if (wrapper) {
                        RECT rc;
                        GetClientRect(hwnd, &rc);
                        if (wrapper->child_hwnd) {
                            SetWindowPos(wrapper->child_hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
                        }
                        if (wrapper->plug_view) {
                            Steinberg::Vst::ViewRect vr = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };
                            wrapper->plug_view->onSize(&vr);
                        }
                    }
                    break;
                case WM_DESTROY:
                    return 0;
            }
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }

    public:
        Vst3Wrapper(const std::string& id, const std::string& name)
            : PluginNode(id, name) {}

        ~Vst3Wrapper() override {
            unload();
        }

        bool load(const std::string& path) {
            unload();
            plugin_path = path;

            std::string actual_binary = path;
            std::filesystem::path p(path);
            if (std::filesystem::is_directory(p)) {
                // Suporte para bundle VST3 oficial: <Nome>.vst3/Contents/x86_64-win/<Nome>.vst3
                auto inner = p / "Contents" / "x86_64-win" / (p.stem().string() + ".vst3");
                if (std::filesystem::exists(inner)) {
                    actual_binary = inner.string();
                } else {
                    for (const auto& entry : std::filesystem::directory_iterator(p / "Contents" / "x86_64-win")) {
                        if (entry.path().extension() == ".vst3" || entry.path().extension() == ".dll") {
                            actual_binary = entry.path().string();
                            break;
                        }
                    }
                }
            }

            dll_handle = LoadLibraryA(actual_binary.c_str());
            if (!dll_handle) {
                std::cerr << "[VST3] Falha ao carregar DLL: " << actual_binary << " (Error: " << GetLastError() << ")\n";
                return false;
            }

            auto init_dll = (InitDllFunc)GetProcAddress(dll_handle, "InitDll");
            if (init_dll) init_dll();

            auto get_factory = (GetPluginFactoryFunc)GetProcAddress(dll_handle, "GetPluginFactory");
            if (!get_factory) {
                std::cerr << "[VST3] DLL não exporta GetPluginFactory!\n";
                unload();
                return false;
            }

            factory = (Steinberg::IPluginFactory*)get_factory();
            if (!factory) {
                std::cerr << "[VST3] GetPluginFactory retornou ponteiro nulo!\n";
                unload();
                return false;
            }

            int class_count = factory->countClasses();
            if (class_count == 0) {
                std::cerr << "[VST3] Nenhuma classe encontrada no pacote VST3!\n";
                unload();
                return false;
            }

            // IID Canônico de FUnknown (COM_COMPATIBLE)
            const unsigned char funknown_iid[16] = {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46
            };
            // IID Canônico de IComponent (COM_COMPATIBLE)
            const unsigned char icomp_iid[16] = {
                0x31, 0xFF, 0x31, 0xE8, 0xD5, 0xF2, 0x01, 0x43,
                0x92, 0x8E, 0xBB, 0xEE, 0x25, 0x69, 0x78, 0x02
            };
            // IID Canônico de IAudioProcessor (COM_COMPATIBLE)
            const unsigned char iaudio_iid[16] = {
                0x99, 0x3F, 0x04, 0x42, 0xDA, 0xB7, 0x3C, 0x45,
                0xA5, 0x69, 0xE7, 0x9D, 0x9A, 0xAE, 0xC3, 0x3D
            };
            // IID Canônico de IEditController (COM_COMPATIBLE)
            const unsigned char ictrl_iid[16] = {
                0xE3, 0xBB, 0xD7, 0xDC, 0x42, 0x77, 0x8D, 0x44,
                0xA8, 0x74, 0xAA, 0xCC, 0x97, 0x9C, 0x75, 0x9E
            };

            int comp_class_idx = -1;
            int ctrl_class_idx = -1;

            for (int i = 0; i < class_count; ++i) {
                Steinberg::PClassInfo ci;
                if (factory->getClassInfo(i, &ci) == Steinberg::kResultOk) {
                    std::string cat = ci.category;
                    if (cat == "Audio Module Class" || cat.find("Audio") != std::string::npos) {
                        if (comp_class_idx == -1) comp_class_idx = i;
                    } else if (cat == "Component Controller Class" || cat.find("Controller") != std::string::npos) {
                        if (ctrl_class_idx == -1) ctrl_class_idx = i;
                    }
                }
            }
            if (comp_class_idx == -1) comp_class_idx = 0; // Fallback para a primeira classe

            Steinberg::PClassInfo comp_ci;
            factory->getClassInfo(comp_class_idx, &comp_ci);

            // Instancia o Audio Component
            void* comp_obj = nullptr;
            if (factory->createInstance((Steinberg::FIDString)comp_ci.cid, (Steinberg::FIDString)funknown_iid, &comp_obj) != Steinberg::kResultOk || !comp_obj) {
                std::cerr << "[VST3] Falha ao instanciar Component: " << comp_ci.name << "\n";
                unload();
                return false;
            }
            comp_unknown = (Steinberg::FUnknown*)comp_obj;

            // Query IComponent
            void* icomp_ptr = nullptr;
            comp_unknown->queryInterface(icomp_iid, &icomp_ptr);
            component = (Steinberg::IComponent*)icomp_ptr;

            // Query IAudioProcessor
            void* iaudio_ptr = nullptr;
            comp_unknown->queryInterface(iaudio_iid, &iaudio_ptr);
            audio_processor = (Steinberg::Vst::IAudioProcessor*)iaudio_ptr;

            if (!component || !audio_processor) {
                std::cerr << "[VST3] Objeto não implementa IComponent ou IAudioProcessor!\n";
                unload();
                return false;
            }

            // Inicialização do Componente
            component->initialize(nullptr);

            // Setup de Áudio
            Steinberg::ProcessSetup setup;
            setup.processMode = 0; // Realtime
            setup.symbolicSampleSize = 0; // 32-bit float
            setup.maxSamplesPerBlock = max_block_size;
            setup.sampleRate = sample_rate;
            audio_processor->setupProcessing(setup);

            // Ativa Barramentos e Áudio
            component->setActive(true);
            audio_processor->setProcessing(true);

            // Procurar ou instanciar o Controller
            if (ctrl_class_idx != -1) {
                Steinberg::PClassInfo ctrl_ci;
                factory->getClassInfo(ctrl_class_idx, &ctrl_ci);
                void* ctrl_obj = nullptr;
                if (factory->createInstance((Steinberg::FIDString)ctrl_ci.cid, (Steinberg::FIDString)funknown_iid, &ctrl_obj) == Steinberg::kResultOk && ctrl_obj) {
                    ctrl_unknown = (Steinberg::FUnknown*)ctrl_obj;
                    void* ictrl_ptr = nullptr;
                    ctrl_unknown->queryInterface(ictrl_iid, &ictrl_ptr);
                    controller = (Steinberg::Vst::IEditController*)ictrl_ptr;
                    if (controller) {
                        controller->initialize(nullptr);
                        controller->setComponentHandler(&comp_handler);
                    }
                }
            } else {
                // Tenta query IEditController no próprio componente (alguns plugins unificam)
                void* ictrl_ptr = nullptr;
                if (comp_unknown->queryInterface(ictrl_iid, &ictrl_ptr) == Steinberg::kResultOk) {
                    controller = (Steinberg::Vst::IEditController*)ictrl_ptr;
                    if (controller) {
                        controller->setComponentHandler(&comp_handler);
                    }
                }
            }

            is_loaded = true;
            std::cout << "[VST3] Sucesso ao carregar e inicializar: " << comp_ci.name << " (" << path << ")\n";
            return true;
        }

        void unload() {
            closeGui();
            if (audio_processor) {
                audio_processor->setProcessing(false);
                audio_processor = nullptr;
            }
            if (component) {
                component->setActive(false);
                component->terminate();
                component = nullptr;
            }
            if (controller) {
                if (ctrl_unknown) controller->terminate();
                controller = nullptr;
            }
            if (comp_unknown) {
                comp_unknown->release();
                comp_unknown = nullptr;
            }
            if (ctrl_unknown) {
                ctrl_unknown->release();
                ctrl_unknown = nullptr;
            }
            if (factory) {
                factory = nullptr;
            }
            if (dll_handle) {
                auto exit_dll = (ExitDllFunc)GetProcAddress(dll_handle, "ExitDll");
                if (exit_dll) exit_dll();
                FreeLibrary(dll_handle);
                dll_handle = nullptr;
            }
            is_loaded = false;
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (!is_loaded || !audio_processor || is_bypassed) return;

            // 1. Buffers de Áudio Estéreo
            float* in_channels[2] = { left, right };
            float* out_channels[2] = { left, right };

            Steinberg::AudioBusBuffers in_bus;
            in_bus.numChannels = 2;
            in_bus.silenceFlags = 0;
            in_bus.channelBuffers32 = in_channels;

            Steinberg::AudioBusBuffers out_bus;
            out_bus.numChannels = 2;
            out_bus.silenceFlags = 0;
            out_bus.channelBuffers32 = out_channels;

            // 2. Montar ProcessData
            Steinberg::Vst::ProcessData pdata;
            pdata.processMode = 0; // kRealtime
            pdata.symbolicSampleSize = 0; // kSample32
            pdata.numSamples = (int32_t)frames;
            pdata.numInputs = 1;
            pdata.numOutputs = 1;
            pdata.inputs = &in_bus;
            pdata.outputs = &out_bus;
            pdata.inputParameterChanges = nullptr;
            pdata.outputParameterChanges = nullptr;
            pdata.inputEvents = &event_list;
            pdata.outputEvents = nullptr;
            pdata.processContext = &proc_context;

            // 3. Execução DSP do Plugin VST3
            audio_processor->process(pdata);

            // 4. Limpar eventos MIDI processados no bloco
            event_list.clear();
        }

        void pushMidiNote(int key, bool is_note_on, float velocity = 1.0f) override {
            if (!is_loaded) return;
            Steinberg::Vst::Event ev = {};
            ev.busIndex = 0;
            ev.sampleOffset = 0;
            ev.ppqPosition = 0.0;
            ev.flags = Steinberg::Vst::Event::kIsLive;

            if (is_note_on) {
                ev.type = Steinberg::Vst::Event::kNoteOnEvent;
                ev.noteOn.channel = 0;
                ev.noteOn.pitch = (int16_t)key;
                ev.noteOn.velocity = std::clamp(velocity, 0.0f, 1.0f);
                ev.noteOn.length = 0;
                ev.noteOn.noteId = -1;
                ev.noteOn.tuning = 0.0f;
            } else {
                ev.type = Steinberg::Vst::Event::kNoteOffEvent;
                ev.noteOff.channel = 0;
                ev.noteOff.pitch = (int16_t)key;
                ev.noteOff.velocity = std::clamp(velocity, 0.0f, 1.0f);
                ev.noteOff.noteId = -1;
                ev.noteOff.tuning = 0.0f;
            }
            event_list.addEvent(ev);
        }

        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
            if (!is_loaded || !controller) return;
            controller->setParamNormalized((Steinberg::ParamID)param_index, target_value);
        }

        // =====================================================================
        // CONTROLE DE INTERFACE GRÁFICA (GUI) NATIVA FLUTUANTE
        // =====================================================================
        bool openGui(HWND parent = nullptr) {
            if (!is_loaded || !controller) {
                std::cerr << "[VST3 GUI] Controller não disponível para abrir interface.\n";
                return false;
            }
            if (gui_open && gui_hwnd) {
                ShowWindow(gui_hwnd, SW_RESTORE);
                SetForegroundWindow(gui_hwnd);
                return true;
            }

            OleInitialize(NULL);

            plug_view = controller->createView("editor");
            if (!plug_view) {
                std::cerr << "[VST3 GUI] createView('editor') retornou nulo.\n";
                return false;
            }

            if (plug_view->isPlatformTypeSupported("HWND") != Steinberg::kResultOk) {
                std::cerr << "[VST3 GUI] Plataforma HWND não é suportada por este plugin.\n";
                plug_view->release();
                plug_view = nullptr;
                return false;
            }

            Steinberg::Vst::ViewRect rect = { 0, 0, 800, 600 };
            if (plug_view->getSize(&rect) == Steinberg::kResultOk) {
                gui_width = rect.getWidth();
                gui_height = rect.getHeight();
            }

            static bool class_registered = false;
            const char* class_name = "Abduction_VST3_Window";
            if (!class_registered) {
                WNDCLASSEXA wc = {};
                wc.cbSize = sizeof(WNDCLASSEXA);
                wc.lpfnWndProc = Vst3WindowProc;
                wc.hInstance = GetModuleHandle(NULL);
                wc.lpszClassName = class_name;
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
                RegisterClassExA(&wc);
                class_registered = true;
            }

            RECT win_rc = { 0, 0, gui_width, gui_height };
            DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
            AdjustWindowRect(&win_rc, style, FALSE);

            std::string title = "[VST3] " + node_name + " - Abduction Studio";
            gui_hwnd = CreateWindowExA(
                WS_EX_APPWINDOW,
                class_name,
                title.c_str(),
                style,
                CW_USEDEFAULT, CW_USEDEFAULT,
                win_rc.right - win_rc.left,
                win_rc.bottom - win_rc.top,
                parent,
                NULL,
                GetModuleHandle(NULL),
                NULL
            );

            if (!gui_hwnd) {
                std::cerr << "[VST3 GUI] CreateWindowExA falhou!\n";
                plug_view->release();
                plug_view = nullptr;
                return false;
            }

            SetWindowLongPtrA(gui_hwnd, GWLP_USERDATA, (LONG_PTR)this);

            child_hwnd = CreateWindowExA(
                0,
                "STATIC",
                "",
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                0, 0, gui_width, gui_height,
                gui_hwnd,
                NULL,
                GetModuleHandle(NULL),
                NULL
            );

            plug_frame.setHWND(gui_hwnd, child_hwnd);
            plug_view->setFrame(&plug_frame);

            if (plug_view->attached((void*)(child_hwnd ? child_hwnd : gui_hwnd), "HWND") != Steinberg::kResultOk) {
                std::cerr << "[VST3 GUI] plug_view->attached() falhou.\n";
                if (child_hwnd) {
                    DestroyWindow(child_hwnd);
                    child_hwnd = nullptr;
                }
                DestroyWindow(gui_hwnd);
                gui_hwnd = nullptr;
                plug_view->release();
                plug_view = nullptr;
                return false;
            }

            ShowWindow(gui_hwnd, SW_SHOW);
            UpdateWindow(gui_hwnd);
            SetForegroundWindow(gui_hwnd);
            gui_open = true;
            return true;
        }

        void closeGui() {
            if (plug_view) {
                plug_view->removed();
                plug_view->release();
                plug_view = nullptr;
            }
            if (child_hwnd) {
                DestroyWindow(child_hwnd);
                child_hwnd = nullptr;
            }
            if (gui_hwnd) {
                DestroyWindow(gui_hwnd);
                gui_hwnd = nullptr;
            }
            gui_open = false;
        }

        bool isGuiOpen() const { return gui_open && (gui_hwnd != nullptr); }
        bool isLoaded() const { return is_loaded; }
    };
}
