#include <windows.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG(msg, ...) do { printf(msg "\n", ##__VA_ARGS__); fflush(stdout); } while(0)

namespace Steinberg {
    typedef int32_t tresult;
    typedef const char* FIDString;
    typedef unsigned char TUID[16];

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
}

typedef bool (*InitDllFunc)();
typedef void* (*GetPluginFactoryFunc)();

int main() {
    HMODULE h = LoadLibraryA("plugins/MonkSynth.vst3/Contents/x86_64-win/MonkSynth.vst3");
    if (!h) return 1;

    auto init_dll = (InitDllFunc)GetProcAddress(h, "InitDll");
    if (init_dll) init_dll();

    auto get_factory = (GetPluginFactoryFunc)GetProcAddress(h, "GetPluginFactory");
    auto f = (Steinberg::IPluginFactory*)get_factory();

    Steinberg::PClassInfo ci;
    f->getClassInfo(0, &ci);

    unsigned char funknown_iid[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46
    };

    void* obj = nullptr;
    Steinberg::tresult r = f->createInstance((Steinberg::FIDString)ci.cid, (Steinberg::FIDString)funknown_iid, &obj);
    if (!obj) return 1;

    LOG("Obj FUnknown instanciado: %p", obj);
    auto unk = (Steinberg::FUnknown*)obj;

    // IComponent: 31 FF 31 E8 D5 F2 01 43 92 8E BB EE 25 69 78 02
    unsigned char icomp_com[16] = {
        0x31, 0xFF, 0x31, 0xE8, 0xD5, 0xF2, 0x01, 0x43,
        0x92, 0x8E, 0xBB, 0xEE, 0x25, 0x69, 0x78, 0x02
    };

    void* comp_ptr = nullptr;
    Steinberg::tresult r_comp = unk->queryInterface(icomp_com, &comp_ptr);
    LOG("queryInterface IComponent: %d, ptr = %p", r_comp, comp_ptr);

    // IAudioProcessor: 99 3F 04 42 DA B7 3C 45 A5 69 E7 9D 9A AE C3 3D
    unsigned char iaudio_com[16] = {
        0x99, 0x3F, 0x04, 0x42, 0xDA, 0xB7, 0x3C, 0x45,
        0xA5, 0x69, 0xE7, 0x9D, 0x9A, 0xAE, 0xC3, 0x3D
    };

    void* audio_ptr = nullptr;
    Steinberg::tresult r_audio = unk->queryInterface(iaudio_com, &audio_ptr);
    LOG("queryInterface IAudioProcessor: %d, ptr = %p", r_audio, audio_ptr);

    // IEditController: E3 BB D7 DC 42 77 8D 44 A8 74 AA CC 97 9C 75 9E
    unsigned char ictrl_com[16] = {
        0xE3, 0xBB, 0xD7, 0xDC, 0x42, 0x77, 0x8D, 0x44,
        0xA8, 0x74, 0xAA, 0xCC, 0x97, 0x9C, 0x75, 0x9E
    };
    void* ctrl_ptr = nullptr;
    Steinberg::tresult r_ctrl = unk->queryInterface(ictrl_com, &ctrl_ptr);
    LOG("queryInterface IEditController on Comp: %d, ptr = %p", r_ctrl, ctrl_ptr);

    // Agora testar Classe 1 (MonkSynth Controller)
    Steinberg::PClassInfo ci1;
    f->getClassInfo(1, &ci1);
    LOG("Class 1: name = %s, category = %s", ci1.name, ci1.category);

    void* obj1 = nullptr;
    f->createInstance((Steinberg::FIDString)ci1.cid, (Steinberg::FIDString)funknown_iid, &obj1);
    LOG("Class 1 obj: %p", obj1);
    if (obj1) {
        auto unk1 = (Steinberg::FUnknown*)obj1;
        void* ctrl_ptr1 = nullptr;
        Steinberg::tresult r_ctrl1 = unk1->queryInterface(ictrl_com, &ctrl_ptr1);
        LOG("queryInterface IEditController on Class 1: %d, ptr = %p", r_ctrl1, ctrl_ptr1);

        struct MinimalEditController {
            // FUnknown
            virtual Steinberg::tresult queryInterface(const Steinberg::TUID _iid, void** obj) = 0;
            virtual uint32_t addRef() = 0;
            virtual uint32_t release() = 0;
            // IPluginBase
            virtual Steinberg::tresult initialize(Steinberg::FUnknown* context) = 0;
            virtual Steinberg::tresult terminate() = 0;
            // IEditController
            virtual Steinberg::tresult setComponentState(void* state) = 0;
            virtual Steinberg::tresult setState(void* state) = 0;
            virtual Steinberg::tresult getState(void* state) = 0;
            virtual int32_t getParameterCount() = 0;
            virtual Steinberg::tresult getParameterInfo(int32_t paramIndex, void* info) = 0;
            virtual Steinberg::tresult getParamStringByValue(uint32_t id, double valueNormalized, void* string) = 0;
            virtual Steinberg::tresult getParamValueByString(uint32_t id, void* string, double& valueNormalized) = 0;
            virtual double normalizedParamToPlain(uint32_t id, double valueNormalized) = 0;
            virtual double plainParamToNormalized(uint32_t id, double plainValue) = 0;
            virtual double getParamNormalized(uint32_t id) = 0;
            virtual Steinberg::tresult setParamNormalized(uint32_t id, double value) = 0;
            virtual Steinberg::tresult setComponentHandler(void* handler) = 0;
            virtual void* createView(Steinberg::FIDString name) = 0;
        };

        struct ViewRect {
            int32_t left;
            int32_t top;
            int32_t right;
            int32_t bottom;
        };

        struct MinimalPlugView {
            virtual Steinberg::tresult queryInterface(const Steinberg::TUID _iid, void** obj) = 0;
            virtual uint32_t addRef() = 0;
            virtual uint32_t release() = 0;
            virtual Steinberg::tresult isPlatformTypeSupported(Steinberg::FIDString type) = 0;
            virtual Steinberg::tresult attached(void* parent, Steinberg::FIDString type) = 0;
            virtual Steinberg::tresult removed() = 0;
            virtual Steinberg::tresult onWheel(float distance) = 0;
            virtual Steinberg::tresult onKeyDown(char16_t key, int16_t keyCode, int16_t modifiers) = 0;
            virtual Steinberg::tresult onKeyUp(char16_t key, int16_t keyCode, int16_t modifiers) = 0;
            virtual Steinberg::tresult getSize(ViewRect* size) = 0;
        };

        auto ctrl = (MinimalEditController*)ctrl_ptr1;
        ctrl->initialize(nullptr);
        void* view = ctrl->createView("editor");
        LOG("createView('editor') = %p", view);
        if (view) {
            auto pv = (MinimalPlugView*)view;
            Steinberg::tresult r_supp = pv->isPlatformTypeSupported("HWND");
            LOG("isPlatformTypeSupported('HWND') = %d", r_supp);
            ViewRect vr = {0,0,0,0};
            Steinberg::tresult r_sz = pv->getSize(&vr);
            LOG("getSize = %d, rect = [%d, %d, %d, %d] (w=%d, h=%d)", r_sz, vr.left, vr.top, vr.right, vr.bottom, vr.right - vr.left, vr.bottom - vr.top);
            pv->release();
        }
        unk1->release();
    }

    unk->release();
    return 0;
}
