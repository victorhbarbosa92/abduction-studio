#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <iostream>
#include <atomic>
#include <cmath>
#include <functional>
#include "DJEngine.h"
#include "DJMidiLearnEngine.h"
#include "../utils/Logger.h"

#pragma comment(lib, "winmm.lib")

namespace KuroAudio {

    class DDJ200DeviceManager {
    private:
        HMIDIIN hMidiIn = NULL;
        HMIDIOUT hMidiOut = NULL;
        std::atomic<bool> is_connected{ false };
        DJEngine* dj_engine = nullptr;

        std::string detected_device_name = "";
        std::string controller_model_name = "DDJ-400";

        int input_dev_id = -1;
        int output_dev_id = -1;

        // Estado 4-Decks (0=Deck 1, 1=Deck 2, 2=Deck 3, 3=Deck 4)
        int active_left_deck = 0;  // 0 ou 2
        int active_right_deck = 1; // 1 ou 3

        // Estado dos botões de Shift da controladora
        bool shift_deck_a = false;
        bool shift_deck_b = false;

        // Estado 14-bit dos faders
        int rate_msb_a = 64, rate_lsb_a = 0;
        int rate_msb_b = 64, rate_lsb_b = 0;
        int vol_msb_a = 127, vol_lsb_a = 0;
        int vol_msb_b = 127, vol_lsb_b = 0;
        int cf_msb = 64, cf_lsb = 0;

        // Estado 14-bit dos EQs e Filtros (Centro neutro em 64 MSB / 0 LSB = 8192)
        int eq_hi_msb_a = 64, eq_hi_lsb_a = 0;
        int eq_mid_msb_a = 64, eq_mid_lsb_a = 0;
        int eq_low_msb_a = 64, eq_low_lsb_a = 0;

        int eq_hi_msb_b = 64, eq_hi_lsb_b = 0;
        int eq_mid_msb_b = 64, eq_mid_lsb_b = 0;
        int eq_low_msb_b = 64, eq_low_lsb_b = 0;

        int filter_msb_a = 64, filter_lsb_a = 0;
        int filter_msb_b = 64, filter_lsb_b = 0;

        static float calc14BitEQ(int msb, int lsb) {
            int val = (msb << 7) | (lsb & 0x7F); // 0 a 16383, centro 8192
            if (val <= 8192) {
                return (float)val / 8192.0f; // 0.0f a 1.0f (centro neutro 12 horas)
            } else {
                return 1.0f + ((float)(val - 8192) / 8191.0f) * 1.0f; // 1.0f a 2.0f (+6dB boost)
            }
        }

        static float calc14BitFilter(int msb, int lsb) {
            int val = (msb << 7) | (lsb & 0x7F); // 0 a 16383, centro 8192
            return (float)val / 16383.0f; // 0.0f a 1.0f, centro = 0.50f (12 horas)
        }

        // Callback para navegação de biblioteca quando o DJ gira o jog segurando Shift
        std::function<void(int delta)> on_browse_callback = nullptr;

        // Callback nativo de entrada MIDI do WinMM
        static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
            if (wMsg == MIM_DATA) {
                DDJ200DeviceManager* self = (DDJ200DeviceManager*)dwInstance;
                if (self) {
                    DWORD msg = (DWORD)dwParam1;
                    unsigned char status = (unsigned char)(msg & 0xFF);
                    unsigned char data1  = (unsigned char)((msg >> 8) & 0xFF);
                    unsigned char data2  = (unsigned char)((msg >> 16) & 0xFF);
                    self->handleMidiEvent(status, data1, data2);
                }
            }
        }

    public:
        DDJ200DeviceManager() = default;

        ~DDJ200DeviceManager() {
            disconnect();
        }

        void setEngine(DJEngine* engine) {
            dj_engine = engine;
        }

        void setBrowseCallback(std::function<void(int delta)> cb) {
            on_browse_callback = cb;
        }

        bool isConnected() const {
            return is_connected.load();
        }

        std::string getControllerModelName() const {
            return controller_model_name.empty() ? "DDJ-400" : controller_model_name;
        }

        std::string getDetectedDeviceName() const {
            return detected_device_name;
        }

        bool autoConnect(DJEngine* engine = nullptr) {
            if (engine) dj_engine = engine;
            if (is_connected) return true;

            input_dev_id = -1;
            output_dev_id = -1;

            // Busca automática por dispositivos Pioneer DDJ-400, DDJ-FLX4, DDJ-200 ou similares
            UINT numIns = midiInGetNumDevs();
            for (UINT i = 0; i < numIns; i++) {
                MIDIINCAPSA caps;
                if (midiInGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
                    std::string name(caps.szPname);
                    if (name.find("DDJ") != std::string::npos || name.find("FLX") != std::string::npos ||
                        name.find("400") != std::string::npos || name.find("200") != std::string::npos ||
                        name.find("Pioneer") != std::string::npos) {
                        input_dev_id = i;
                        detected_device_name = name;
                        if (name.find("FLX4") != std::string::npos || name.find("FLX-4") != std::string::npos || name.find("FLX") != std::string::npos) {
                            controller_model_name = "DDJ-FLX4";
                        } else if (name.find("400") != std::string::npos) {
                            controller_model_name = "DDJ-400";
                        } else if (name.find("200") != std::string::npos) {
                            controller_model_name = "DDJ-200";
                        } else {
                            controller_model_name = name;
                        }
                        break;
                    }
                }
            }

            UINT numOuts = midiOutGetNumDevs();
            for (UINT i = 0; i < numOuts; i++) {
                MIDIOUTCAPSA caps;
                if (midiOutGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
                    std::string name(caps.szPname);
                    if (name.find("DDJ") != std::string::npos || name.find("200") != std::string::npos || name.find("Pioneer") != std::string::npos) {
                        output_dev_id = i;
                        break;
                    }
                }
            }

            if (input_dev_id < 0 && numIns > 0) {
                // Se não for Pioneer DDJ, conecta a primeira controladora MIDI USB detectada (Numark, Hercules, Traktor, AKAI, etc)
                input_dev_id = 0;
                MIDIINCAPSA caps;
                if (midiInGetDevCapsA(0, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
                    detected_device_name = caps.szPname;
                    controller_model_name = caps.szPname;
                } else {
                    detected_device_name = "Generic MIDI Controller";
                    controller_model_name = "Generic MIDI";
                }
                if (output_dev_id < 0 && numOuts > 0) {
                    for (UINT o = 0; o < numOuts; o++) {
                        MIDIOUTCAPSA ocaps;
                        if (midiOutGetDevCapsA(o, &ocaps, sizeof(ocaps)) == MMSYSERR_NOERROR) {
                            std::string oname(ocaps.szPname);
                            if (oname.find("Microsoft") == std::string::npos && oname.find("Wavetable") == std::string::npos) {
                                output_dev_id = o;
                                break;
                            }
                        }
                    }
                }
            }

            if (input_dev_id < 0) {
                return false;
            }

            MMRESULT resIn = midiInOpen(&hMidiIn, input_dev_id, (DWORD_PTR)MidiInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
            if (resIn != MMSYSERR_NOERROR) {
                return false;
            }
            midiInStart(hMidiIn);

            if (output_dev_id >= 0) {
                MMRESULT resOut = midiOutOpen(&hMidiOut, output_dev_id, 0, 0, CALLBACK_NULL);
                if (resOut == MMSYSERR_NOERROR) {
                    sendHandshakeSysEx();
                    initLEDs();
                }
            }

            is_connected = true;
            KuroUtils::Log("[" + controller_model_name + "] Hardware Pioneer conectado com sucesso!");
            return true;
        }

        void disconnect() {
            if (!is_connected) return;
            is_connected = false;

            if (hMidiIn) {
                midiInStop(hMidiIn);
                midiInReset(hMidiIn);
                midiInClose(hMidiIn);
                hMidiIn = NULL;
            }

            if (hMidiOut) {
                turnOffAllLEDs();
                midiOutReset(hMidiOut);
                midiOutClose(hMidiOut);
                hMidiOut = NULL;
            }

            KuroUtils::Log("[DDJ-200] Desconectada com segurança.");
        }

        void sendShortMessage(unsigned char status, unsigned char data1, unsigned char data2) {
            if (!hMidiOut) return;
            DWORD msg = status | (data1 << 8) | (data2 << 16);
            midiOutShortMsg(hMidiOut, msg);
        }

        void sendHandshakeSysEx() {
            if (!hMidiOut) return;
            // Somente envia handshake SysEx para controladoras Pioneer DDJ reais
            if (controller_model_name.find("DDJ") == std::string::npos &&
                controller_model_name.find("FLX") == std::string::npos &&
                controller_model_name.find("Pioneer") == std::string::npos) {
                return;
            }

            // Pacote SysEx oficial Pioneer DDJ-200 Wakeup / Initial Query
            unsigned char sysex[] = { 0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x0A, 0x00, 0x03, 0x01, 0xF7 };
            MIDIHDR hdr;
            ZeroMemory(&hdr, sizeof(hdr));
            hdr.lpData = (LPSTR)sysex;
            hdr.dwBufferLength = sizeof(sysex);
            hdr.dwBytesRecorded = sizeof(sysex);

            if (midiOutPrepareHeader(hMidiOut, &hdr, sizeof(hdr)) == MMSYSERR_NOERROR) {
                midiOutLongMsg(hMidiOut, &hdr, sizeof(hdr));
                int timeout_ms = 0;
                while ((hdr.dwFlags & MHDR_DONE) == 0 && timeout_ms < 60) {
                    Sleep(5);
                    timeout_ms += 5;
                }
                midiOutUnprepareHeader(hMidiOut, &hdr, sizeof(hdr));
            }
        }

        // Cache de estado para evitar envio de mensagens redundantes via MIDI (elimina saturação da porta e engasgos de FPS)
        int last_deck_playing[4] = { -1, -1, -1, -1 };
        int last_deck_cue[4]     = { -1, -1, -1, -1 };
        int last_deck_sync[4]    = { -1, -1, -1, -1 };
        int last_pad_vals[4][8]  = {
            { -1, -1, -1, -1, -1, -1, -1, -1 },
            { -1, -1, -1, -1, -1, -1, -1, -1 },
            { -1, -1, -1, -1, -1, -1, -1, -1 },
            { -1, -1, -1, -1, -1, -1, -1, -1 }
        };

        void resetMidiCaches() {
            for (int d = 0; d < 4; d++) {
                last_deck_playing[d] = -1;
                last_deck_cue[d]     = -1;
                last_deck_sync[d]    = -1;
                for (int p = 0; p < 8; p++) last_pad_vals[d][p] = -1;
            }
        }

        void turnOffAllLEDs() {
            if (!hMidiOut) return;
            resetMidiCaches();
            for (unsigned char ch = 0; ch <= 1; ch++) {
                sendShortMessage(0x90 + ch, 0x0B, 0); // Play
                sendShortMessage(0x90 + ch, 0x0C, 0); // Cue
                sendShortMessage(0x90 + ch, 0x58, 0); // Sync
                sendShortMessage(0x90 + ch, 0x54, 0); // Headphone
                for (unsigned char p = 0; p < 8; p++) {
                    sendShortMessage(0x97 + 2 * ch, p, 0); // Pads Hot Cues
                }
            }
            sendShortMessage(0x96, 0x63, 0); // Master cue
            sendShortMessage(0x96, 0x3F, 0); // Transition FX
        }

        void initLEDs() {
            if (!hMidiOut) return;
            turnOffAllLEDs();
            sendShortMessage(0x90, 0x0B, 127); // Play Deck A
            sendShortMessage(0x90, 0x0C, 127); // Cue Deck A
            sendShortMessage(0x91, 0x0B, 127); // Play Deck B
            sendShortMessage(0x91, 0x0C, 127); // Cue Deck B
            sendShortMessage(0x96, 0x3F, 127); // Transition FX On
        }

        void updateDeckLED(int deck_idx, bool is_playing, bool is_cue, bool is_sync) {
            if (!hMidiOut || deck_idx < 0 || deck_idx >= 4) return;
            unsigned char ch = (deck_idx == active_left_deck) ? 0x90 : ((deck_idx == active_right_deck) ? 0x91 : 0);
            if (ch == 0) return;

            int play_val = is_playing ? 127 : 30;
            if (last_deck_playing[deck_idx] != play_val) {
                last_deck_playing[deck_idx] = play_val;
                sendShortMessage(ch, 0x0B, (unsigned char)play_val);
            }

            int cue_val = is_cue ? 127 : 0;
            if (last_deck_cue[deck_idx] != cue_val) {
                last_deck_cue[deck_idx] = cue_val;
                sendShortMessage(ch, 0x0C, (unsigned char)cue_val);
            }

            int sync_val = is_sync ? 127 : 0;
            if (last_deck_sync[deck_idx] != sync_val) {
                last_deck_sync[deck_idx] = sync_val;
                sendShortMessage(ch, 0x58, (unsigned char)sync_val);
            }
        }

        // Sincronização Bi-direcional de Jogwheel LED, Ângulo de Rotação e Platter Touch
        int last_sent_jog_angle_a = -1;
        int last_sent_jog_angle_b = -1;
        bool last_sent_touch_a = false;
        bool last_sent_touch_b = false;
        bool last_sent_beat_flash_a = false;
        bool last_sent_beat_flash_b = false;
        uint32_t last_jog_sync_time_ms = 0;

        void updateJogwheelFeedback(const DJDeck& deck_left, const DJDeck& deck_right) {
            if (!hMidiOut) return;
            
            // Rate limiting suave (~60Hz / 16ms) para preservar a banda da porta MIDI
            uint32_t now = (uint32_t)GetTickCount();
            if (now - last_jog_sync_time_ms < 16) return;
            last_jog_sync_time_ms = now;

            // 1. Deck Esquerdo Ativo (Canal 0x90 / 0xB0)
            {
                // Ângulo virtual normalizado de 0 a 127
                float ang_norm = std::fmod(std::fmod(deck_left.jog_visual_angle, (float)(2.0 * M_PI)) + (float)(2.0 * M_PI), (float)(2.0 * M_PI));
                int val_7bit = std::clamp((int)(ang_norm / (2.0f * (float)M_PI) * 127.0f), 0, 127);
                if (val_7bit != last_sent_jog_angle_a) {
                    last_sent_jog_angle_a = val_7bit;
                    sendShortMessage(0xB0, 0x22, (unsigned char)val_7bit); // CC 0x22 Jog Position
                    sendShortMessage(0xB0, 0x21, (unsigned char)val_7bit); // CC 0x21 Outer Rim Position
                }

                // Touch Sensor Platter Feedback (Note 0x36)
                if (deck_left.jog_touch != last_sent_touch_a) {
                    last_sent_touch_a = deck_left.jog_touch;
                    sendShortMessage(0x90, 0x36, deck_left.jog_touch ? 127 : 0);
                }

                // Beat Pulse / Phase LED (Note 0x24)
                if (deck_left.is_playing && deck_left.bpm > 20.0 && deck_left.sample_rate > 1000.0) {
                    double beat_len = (60.0 / deck_left.bpm) * deck_left.sample_rate;
                    double rel = deck_left.current_frame - deck_left.first_beat_frame;
                    double phase = std::fmod(rel, beat_len);
                    if (phase < 0.0) phase += beat_len;
                    bool flash = (phase < beat_len * 0.12); // Pulso nos primeiros 12% do beat
                    if (flash != last_sent_beat_flash_a) {
                        last_sent_beat_flash_a = flash;
                        sendShortMessage(0x90, 0x24, flash ? 127 : 0);
                    }
                } else if (last_sent_beat_flash_a) {
                    last_sent_beat_flash_a = false;
                    sendShortMessage(0x90, 0x24, 0);
                }
            }

            // VU Meters LEDs (CC 0x02) - Padrão Pioneer DDJ-400 / DDJ-FLX4
            int vu_l = std::clamp((int)(deck_left.vu_meter * 127.0f), 0, 127);
            int vu_r = std::clamp((int)(deck_right.vu_meter * 127.0f), 0, 127);
            sendShortMessage(0xB0, 0x02, (unsigned char)vu_l);
            sendShortMessage(0xB1, 0x02, (unsigned char)vu_r);

            // CUE / Headphone PFL LEDs (Note 0x54) & Master Cue (0x63)
            if (dj_engine) {
                sendShortMessage(0x90, 0x54, dj_engine->getCueDeckA() ? 127 : 0);
                sendShortMessage(0x91, 0x54, dj_engine->getCueDeckB() ? 127 : 0);
                sendShortMessage(0x96, 0x63, dj_engine->getCueMaster() ? 127 : 0);
                sendShortMessage(0x90, 0x63, dj_engine->getCueMaster() ? 127 : 0);
            }

            // 2. Deck Direito Ativo (Canal 0x91 / 0xB1)
            {
                float ang_norm = std::fmod(std::fmod(deck_right.jog_visual_angle, (float)(2.0 * M_PI)) + (float)(2.0 * M_PI), (float)(2.0 * M_PI));
                int val_7bit = std::clamp((int)(ang_norm / (2.0f * (float)M_PI) * 127.0f), 0, 127);
                if (val_7bit != last_sent_jog_angle_b) {
                    last_sent_jog_angle_b = val_7bit;
                    sendShortMessage(0xB1, 0x22, (unsigned char)val_7bit);
                    sendShortMessage(0xB1, 0x21, (unsigned char)val_7bit);
                }

                if (deck_right.jog_touch != last_sent_touch_b) {
                    last_sent_touch_b = deck_right.jog_touch;
                    sendShortMessage(0x91, 0x36, deck_right.jog_touch ? 127 : 0);
                }

                if (deck_right.is_playing && deck_right.bpm > 20.0 && deck_right.sample_rate > 1000.0) {
                    double beat_len = (60.0 / deck_right.bpm) * deck_right.sample_rate;
                    double rel = deck_right.current_frame - deck_right.first_beat_frame;
                    double phase = std::fmod(rel, beat_len);
                    if (phase < 0.0) phase += beat_len;
                    bool flash = (phase < beat_len * 0.12);
                    if (flash != last_sent_beat_flash_b) {
                        last_sent_beat_flash_b = flash;
                        sendShortMessage(0x91, 0x24, flash ? 127 : 0);
                    }
                } else if (last_sent_beat_flash_b) {
                    last_sent_beat_flash_b = false;
                    sendShortMessage(0x91, 0x24, 0);
                }
            }
        }

        std::function<void(int deck_idx, int mode)> on_pad_mode_changed = nullptr;

        void setControllerPadMode(int deck_idx, int mode) {
            if (deck_idx < 0 || deck_idx >= 4 || !dj_engine) return;
            auto& d = dj_engine->decks[deck_idx];
            d.pad_mode = std::clamp(mode, 0, 5);
            for (int p = 0; p < 8; p++) last_pad_vals[deck_idx][p] = -1;
            updatePadLEDs(deck_idx, d.pad_mode, d.hot_cues, d.loop_active, d.loop_beats, 
                          d.stem_mute_drums, d.stem_mute_vocals, d.stem_mute_inst, d.stem_mode);
        }

        void updatePadLEDs(int deck_idx, int mode, const DJHotCue* cues, bool loop_active = false, double loop_beats = 4.0, bool stem_mute_d = false, bool stem_mute_v = false, bool stem_mute_i = false, bool stem_eq = false) {
            if (!hMidiOut || deck_idx < 0 || deck_idx >= 4) return;
            unsigned char ch = 0;
            if (deck_idx == active_left_deck) ch = 0x97;
            else if (deck_idx == active_right_deck) ch = 0x99;
            else return;

            const double loop_sizes[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0 };
            for (int i = 0; i < 8; i++) {
                int target_val = 0;
                if (mode == 0) { // 0: HOT CUE
                    target_val = cues[i].active ? 127 : 0;
                } else if (mode == 1) { // 1: BEAT LOOP (1/4, 1/2, 1, 2, 4, 8, 16, 32)
                    if (loop_active && std::abs(loop_beats - loop_sizes[i]) < 0.001) {
                        target_val = 127; // Pad do loop ativo brilha intensamente!
                    } else {
                        target_val = loop_active ? 25 : 55; // Demais pads de loop
                    }
                } else if (mode == 2) { // 2: PAD FX 1
                    target_val = (i < 2) ? 80 : ((i < 4) ? 60 : 70);
                } else if (mode == 3) { // 3: SAMPLER
                    target_val = (dj_engine && i < 8 && dj_engine->samplers[i].is_playing) ? 127 : 50;
                } else if (mode == 4) { // 4: LIVE STEMS
                    if (i == 0) target_val = !stem_mute_d ? 127 : 15;
                    else if (i == 1) target_val = !stem_mute_v ? 127 : 15;
                    else if (i == 2) target_val = !stem_mute_i ? 127 : 15;
                    else if (i == 3) target_val = stem_eq ? 127 : 30;
                    else target_val = 60;
                } else if (mode == 5) { // 5: BEAT JUMP
                    target_val = (i % 2 == 1) ? 110 : 45; // Jump fwd bright, jump bwd dim
                }

                if (last_pad_vals[deck_idx][i] != target_val) {
                    last_pad_vals[deck_idx][i] = target_val;
                    sendShortMessage(ch, (unsigned char)i, (unsigned char)target_val);
                }
            }
        }

        int getActiveLeftDeck() const { return active_left_deck; }
        int getActiveRightDeck() const { return active_right_deck; }
        void setActiveLeftDeck(int idx) { 
            active_left_deck = (idx == 2) ? 2 : 0; 
            for (int p = 0; p < 8; p++) last_pad_vals[active_left_deck][p] = -1;
            last_deck_playing[active_left_deck] = -1;
            last_deck_cue[active_left_deck] = -1;
            last_deck_sync[active_left_deck] = -1;
            if (dj_engine) {
                auto& d = dj_engine->decks[active_left_deck];
                updateDeckLED(active_left_deck, d.is_playing, d.cue_active, false);
                updatePadLEDs(active_left_deck, d.pad_mode, d.hot_cues, d.loop_active, d.loop_beats, d.stem_mute_drums, d.stem_mute_vocals, d.stem_mute_inst, d.stem_mode);
            }
        }
        void setActiveRightDeck(int idx) { 
            active_right_deck = (idx == 3) ? 3 : 1; 
            for (int p = 0; p < 8; p++) last_pad_vals[active_right_deck][p] = -1;
            last_deck_playing[active_right_deck] = -1;
            last_deck_cue[active_right_deck] = -1;
            last_deck_sync[active_right_deck] = -1;
            if (dj_engine) {
                auto& d = dj_engine->decks[active_right_deck];
                updateDeckLED(active_right_deck, d.is_playing, d.cue_active, false);
                updatePadLEDs(active_right_deck, d.pad_mode, d.hot_cues, d.loop_active, d.loop_beats, d.stem_mute_drums, d.stem_mute_vocals, d.stem_mute_inst, d.stem_mode);
            }
        }
        void toggleLeftDeck() { 
            setActiveLeftDeck(active_left_deck == 0 ? 2 : 0); 
        }
        void toggleRightDeck() { 
            setActiveRightDeck(active_right_deck == 1 ? 3 : 1); 
        }

        // Mapeamento Oficial da Pioneer DDJ-200 com Suporte Nativo a 4 Decks
        void handleMidiEvent(unsigned char status, unsigned char data1, unsigned char data2) {
            // Delega para o motor de MIDI Learn (se estiver aprendendo ou se houver mapeamento customizado)
            if (DJMidiLearnEngine::getInstance().handleMidiMessage(status, data1, data2)) {
                return;
            }

            if (!dj_engine) return;

            DJDeck& deck_left = dj_engine->decks[active_left_deck];
            DJDeck& deck_right = dj_engine->decks[active_right_deck];

            unsigned char msg_type = status & 0xF0;
            unsigned char channel  = status & 0x0F;

            // --- NOTE ON (0x90 a 0x9F) ---
            if (msg_type == 0x90 && data2 > 0) {
                // Deck Esquerdo Ativo (Deck 1 ou Deck 3)
                if (channel == 0) {
                    if (data1 == 0x0B) { // Play / Pause
                        if (deck_left.buffer_l.empty() && active_left_deck != 0 && !dj_engine->decks[0].buffer_l.empty()) {
                            active_left_deck = 0;
                        }
                        auto& target_deck = dj_engine->decks[active_left_deck];
                        target_deck.togglePlay(dj_engine ? &dj_engine->decks[dj_engine->getMasterDeckId()] : &deck_right);
                        sendShortMessage(0x90, 0x0B, target_deck.is_playing ? 127 : 30);
                        KuroUtils::Log(std::string("[DDJ-200] Play/Pause Deck ") + std::to_string(active_left_deck + 1) + (target_deck.is_playing ? ": TOCANDO" : ": PAUSADO"));
                    }
                    else if (data1 == 0x47) { // Shift + Play (Retorna ao Cue/Inicio e Toca)
                        if (deck_left.buffer_l.empty() && active_left_deck != 0 && !dj_engine->decks[0].buffer_l.empty()) {
                            active_left_deck = 0;
                        }
                        auto& target_deck = dj_engine->decks[active_left_deck];
                        target_deck.current_frame = target_deck.cue_frame;
                        if (!target_deck.is_playing) {
                            target_deck.togglePlay(dj_engine ? &dj_engine->decks[dj_engine->getMasterDeckId()] : &deck_right);
                        }
                        sendShortMessage(0x90, 0x0B, target_deck.is_playing ? 127 : 30);
                        KuroUtils::Log(std::string("[DDJ-200] Shift+Play Deck ") + std::to_string(active_left_deck + 1) + ": TOCANDO");
                    }
                    else if (data1 == 0x0C) { // Cue
                        deck_left.pressCue();
                        sendShortMessage(0x90, 0x0C, 127);
                    }
                    else if (data1 == 0x10) { // Loop In / 4 Beat Auto (DDJ-400 / FLX4)
                        deck_left.setLoopIn();
                    }
                    else if (data1 == 0x11) { // Loop Out / Reloop Exit (DDJ-400 / FLX4)
                        if (deck_left.loop_active) deck_left.loop_active = false;
                        else deck_left.setLoopOut();
                    }
                    else if (data1 == 0x50) { // Loop Halve / Prev (DDJ-400 / FLX4)
                        deck_left.loopHalve();
                    }
                    else if (data1 == 0x51) { // Loop Double / Next (DDJ-400 / FLX4)
                        deck_left.loopDouble();
                    }
                    else if (data1 == 0x48) { // Cue goto and stop
                        deck_left.is_playing = false;
                        deck_left.current_frame = deck_left.cue_frame;
                        sendShortMessage(0x90, 0x0B, 0);
                    }
                    else if (data1 == 0x58) { // Beat Sync / SHIFT: Alterna Deck 1 <-> 3
                        if (shift_deck_a) {
                            toggleLeftDeck();
                        } else {
                            dj_engine->toggleSync(active_left_deck, active_right_deck);
                            sendShortMessage(0x90, 0x58, dj_engine->decks[active_left_deck].sync_active ? 127 : 0);
                            sendShortMessage(0x91, 0x58, dj_engine->decks[active_right_deck].sync_active ? 127 : 0);
                        }
                    }
                    else if (data1 == 0x54) { // Headphone Listen (PFL)
                        dj_engine->setCueDeckA(!dj_engine->getCueDeckA());
                        sendShortMessage(0x90, 0x54, dj_engine->getCueDeckA() ? 127 : 0);
                    }
                    else if (data1 == 0x3F) { // Shift Esquerdo pressionado
                        shift_deck_a = true;
                    }
                    else if (data1 == 0x36) { // Platter Touch Sensor (Scratch)
                        deck_left.jog_touch = true;
                        deck_left.scratch_velocity = 0.0f;
                        deck_left.target_scratch_velocity = 0.0f;
                        deck_left.scratch_active_ticks = 0;
                    }
                }
                // Deck Direito Ativo (Deck 2 ou Deck 4)
                else if (channel == 1) {
                    if (data1 == 0x0B) {
                        if (deck_right.buffer_l.empty() && active_right_deck != 1 && !dj_engine->decks[1].buffer_l.empty()) {
                            active_right_deck = 1;
                        }
                        auto& target_deck = dj_engine->decks[active_right_deck];
                        target_deck.togglePlay(dj_engine ? &dj_engine->decks[dj_engine->getMasterDeckId()] : &deck_left);
                        sendShortMessage(0x91, 0x0B, target_deck.is_playing ? 127 : 30);
                        KuroUtils::Log(std::string("[DDJ-200] Play/Pause Deck ") + std::to_string(active_right_deck + 1) + (target_deck.is_playing ? ": TOCANDO" : ": PAUSADO"));
                    }
                    else if (data1 == 0x47) { // Shift + Play
                        if (deck_right.buffer_l.empty() && active_right_deck != 1 && !dj_engine->decks[1].buffer_l.empty()) {
                            active_right_deck = 1;
                        }
                        auto& target_deck = dj_engine->decks[active_right_deck];
                        target_deck.current_frame = target_deck.cue_frame;
                        if (!target_deck.is_playing) {
                            target_deck.togglePlay(dj_engine ? &dj_engine->decks[dj_engine->getMasterDeckId()] : &deck_left);
                        }
                        sendShortMessage(0x91, 0x0B, target_deck.is_playing ? 127 : 30);
                        KuroUtils::Log(std::string("[DDJ-200] Shift+Play Deck ") + std::to_string(active_right_deck + 1) + ": TOCANDO");
                    }
                    else if (data1 == 0x0C) {
                        deck_right.pressCue();
                        sendShortMessage(0x91, 0x0C, 127);
                    }
                    else if (data1 == 0x10) { // Loop In / 4 Beat Auto (DDJ-400 / FLX4)
                        deck_right.setLoopIn();
                    }
                    else if (data1 == 0x11) { // Loop Out / Reloop Exit (DDJ-400 / FLX4)
                        if (deck_right.loop_active) deck_right.loop_active = false;
                        else deck_right.setLoopOut();
                    }
                    else if (data1 == 0x50) { // Loop Halve / Prev (DDJ-400 / FLX4)
                        deck_right.loopHalve();
                    }
                    else if (data1 == 0x51) { // Loop Double / Next (DDJ-400 / FLX4)
                        deck_right.loopDouble();
                    }
                    else if (data1 == 0x48) {
                        deck_right.is_playing = false;
                        deck_right.current_frame = deck_right.cue_frame;
                        sendShortMessage(0x91, 0x0B, 0);
                    }
                    else if (data1 == 0x58) { // Beat Sync / SHIFT: Alterna Deck 2 <-> 4
                        if (shift_deck_b) {
                            toggleRightDeck();
                        } else {
                            dj_engine->toggleSync(active_right_deck, active_left_deck);
                            sendShortMessage(0x90, 0x58, dj_engine->decks[active_left_deck].sync_active ? 127 : 0);
                            sendShortMessage(0x91, 0x58, dj_engine->decks[active_right_deck].sync_active ? 127 : 0);
                        }
                    }
                    else if (data1 == 0x54) {
                        dj_engine->setCueDeckB(!dj_engine->getCueDeckB());
                        sendShortMessage(0x91, 0x54, dj_engine->getCueDeckB() ? 127 : 0);
                    }
                    else if (data1 == 0x3F) {
                        shift_deck_b = true;
                    }
                    else if (data1 == 0x36) {
                        deck_right.jog_touch = true;
                        deck_right.scratch_velocity = 0.0f;
                        deck_right.target_scratch_velocity = 0.0f;
                        deck_right.scratch_active_ticks = 0;
                    }
                }
                // Master / Central (Channel 6 / 0x96)
                else if (channel == 6) {
                    if (data1 == 0x63) { // Headphone Master Cue
                        dj_engine->setCueMaster(!dj_engine->getCueMaster());
                        sendShortMessage(0x96, 0x63, dj_engine->getCueMaster() ? 127 : 0);
                    }
                    else if (data1 == 0x3F) { // Transition FX Toggle
                        float cf = dj_engine->getCrossfader();
                        dj_engine->setCrossfader(cf < 0.5f ? 1.0f : 0.0f);
                    }
                }
                // Pads Deck Esquerdo Ativo (Channel 7 / 0x97)
                else if (channel == 7) {
                    if (data1 < 8) {
                        if (shift_deck_a && data1 < 6) {
                            // SHIFT + Pad 1..6: Alterna Modo de Performance
                            deck_left.pad_mode = data1; // 0=Hot Cue, 1=Beat Loop, 2=Pad FX, 3=Sampler, 4=Stems, 5=Beat Jump
                            if (on_pad_mode_changed) on_pad_mode_changed(active_left_deck, data1);
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 0) {
                            // MODO 0: HOT CUES
                            if (deck_left.hot_cues[data1].active) deck_left.jumpToHotCue(data1);
                            else deck_left.setHotCue(data1);
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 1) {
                            // MODO 1: BEAT LOOP (1/4, 1/2, 1, 2, 4, 8, 16, 32 beats)
                            const double loop_sizes[8] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0 };
                            deck_left.toggleLoop(loop_sizes[data1]);
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 2) {
                            // MODO 2: PAD FX 1
                            if (data1 == 0) deck_left.toggleLoop(0.5); // Roll 1/2
                            else if (data1 == 1) deck_left.toggleLoop(1.0); // Roll 1
                            else if (data1 == 2) { dj_engine->beat_fx.type = 0; dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled; }
                            else if (data1 == 3) { dj_engine->beat_fx.type = 1; dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled; }
                            else if (data1 == 4) deck_left.sound_color_fx_mode = (deck_left.sound_color_fx_mode == 0 ? -1 : 0);
                            else if (data1 == 5) deck_left.sound_color_fx_mode = (deck_left.sound_color_fx_mode == 2 ? -1 : 2);
                            else if (data1 == 6) { dj_engine->beat_fx.type = 2; dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled; }
                            else if (data1 == 7) dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled;
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 3) {
                            // MODO 3: SAMPLER
                            dj_engine->triggerSampler(data1);
                        } else if (deck_left.pad_mode == 4) {
                            // MODO 4: LIVE STEMS
                            if (data1 == 0) deck_left.toggleStemVocal();
                            else if (data1 == 1) deck_left.toggleStemMelody();
                            else if (data1 == 2) deck_left.toggleStemDrums();
                            else if (data1 == 3) deck_left.soloStemVocal();
                            else if (data1 == 4) deck_left.triggerVocalEchoOut();
                            else if (data1 == 5) deck_left.soloStemMelody();
                            else if (data1 == 6) deck_left.toggleStemDrums();
                            else if (data1 == 7) deck_left.resetStems();
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 5) {
                            // MODO 5: BEAT JUMP (< 1, 1 >, < 2, 2 >, < 4, 4 >, < 8, 8 >)
                            const double jump_beats[8] = { -1.0, 1.0, -2.0, 2.0, -4.0, 4.0, -8.0, 8.0 };
                            deck_left.beatJump(jump_beats[data1]);
                        }
                    }
                }
                // Shift + Pads Deck Esquerdo Ativo (Channel 8 / 0x98)
                else if (channel == 8) {
                    if (data1 < 8) {
                        if (deck_left.pad_mode == 0) {
                            deck_left.deleteHotCue(data1);
                            sendShortMessage(0x97, data1, 0);
                        } else if (deck_left.pad_mode == 1) {
                            if (data1 == 0) deck_left.loopHalve();
                            else if (data1 == 1) deck_left.loopDouble();
                            else if (data1 == 2) deck_left.setLoopIn();
                            else if (data1 == 3) deck_left.setLoopOut();
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 4) {
                            deck_left.resetStems();
                            updatePadLEDs(active_left_deck, deck_left.pad_mode, deck_left.hot_cues, deck_left.loop_active, deck_left.loop_beats, deck_left.stem_mute_drums, deck_left.stem_mute_vocals, deck_left.stem_mute_inst, deck_left.stem_mode);
                        } else if (deck_left.pad_mode == 5) {
                            const double jump_long[8] = { -16.0, 16.0, -32.0, 32.0, -64.0, 64.0, -0.5, 0.5 };
                            deck_left.beatJump(jump_long[data1]);
                        }
                    }
                }
                // Pads Deck Direito Ativo (Channel 9 / 0x99)
                else if (channel == 9) {
                    if (data1 < 8) {
                        if (shift_deck_b && data1 < 6) {
                            deck_right.pad_mode = data1;
                            if (on_pad_mode_changed) on_pad_mode_changed(active_right_deck, data1);
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 0) {
                            if (deck_right.hot_cues[data1].active) deck_right.jumpToHotCue(data1);
                            else deck_right.setHotCue(data1);
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 1) {
                            // MODO 1: BEAT LOOP (1/4, 1/2, 1, 2, 4, 8, 16, 32 beats)
                            const double loop_sizes[8] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0 };
                            deck_right.toggleLoop(loop_sizes[data1]);
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 2) {
                            // MODO 2: PAD FX 1
                            if (data1 == 0) deck_right.toggleLoop(0.5);
                            else if (data1 == 1) deck_right.toggleLoop(1.0);
                            else if (data1 == 2) { dj_engine->beat_fx.type = 0; dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled; }
                            else if (data1 == 3) { dj_engine->beat_fx.type = 1; dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled; }
                            else if (data1 == 4) deck_right.sound_color_fx_mode = (deck_right.sound_color_fx_mode == 0 ? -1 : 0);
                            else if (data1 == 5) deck_right.sound_color_fx_mode = (deck_right.sound_color_fx_mode == 2 ? -1 : 2);
                            else if (data1 == 6) { dj_engine->beat_fx.type = 2; dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled; }
                            else if (data1 == 7) dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled;
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 3) {
                            // MODO 3: SAMPLER
                            dj_engine->triggerSampler(data1);
                        } else if (deck_right.pad_mode == 4) {
                            // MODO 4: LIVE STEMS
                            if (data1 == 0) deck_right.toggleStemVocal();
                            else if (data1 == 1) deck_right.toggleStemMelody();
                            else if (data1 == 2) deck_right.toggleStemDrums();
                            else if (data1 == 3) deck_right.soloStemVocal();
                            else if (data1 == 4) deck_right.triggerVocalEchoOut();
                            else if (data1 == 5) deck_right.soloStemMelody();
                            else if (data1 == 6) deck_right.toggleStemDrums();
                            else if (data1 == 7) deck_right.resetStems();
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 5) {
                            // MODO 5: BEAT JUMP (< 1, 1 >, < 2, 2 >, < 4, 4 >, < 8, 8 >)
                            const double jump_beats[8] = { -1.0, 1.0, -2.0, 2.0, -4.0, 4.0, -8.0, 8.0 };
                            deck_right.beatJump(jump_beats[data1]);
                        }
                    }
                }
                // Shift + Pads Deck Direito Ativo (Channel 10 / 0x9A)
                else if (channel == 10) {
                    if (data1 < 8) {
                        if (deck_right.pad_mode == 0) {
                            deck_right.deleteHotCue(data1);
                            sendShortMessage(0x99, data1, 0);
                        } else if (deck_right.pad_mode == 1) {
                            if (data1 == 0) deck_right.loopHalve();
                            else if (data1 == 1) deck_right.loopDouble();
                            else if (data1 == 2) deck_right.setLoopIn();
                            else if (data1 == 3) deck_right.setLoopOut();
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 4) {
                            deck_right.resetStems();
                            updatePadLEDs(active_right_deck, deck_right.pad_mode, deck_right.hot_cues, deck_right.loop_active, deck_right.loop_beats, deck_right.stem_mute_drums, deck_right.stem_mute_vocals, deck_right.stem_mute_inst, deck_right.stem_mode);
                        } else if (deck_right.pad_mode == 5) {
                            const double jump_long[8] = { -16.0, 16.0, -32.0, 32.0, -64.0, 64.0, -0.5, 0.5 };
                            deck_right.beatJump(jump_long[data1]);
                        }
                    }
                }
            }
            // --- NOTE OFF (0x80 a 0x8F ou velocity 0) ---
            else if (msg_type == 0x80 || (msg_type == 0x90 && data2 == 0)) {
                if (channel == 0) {
                    if (data1 == 0x0C) { // Cue Release Deck Esquerdo
                        deck_left.releaseCue();
                        sendShortMessage(0x90, 0x0C, deck_left.is_playing ? 127 : 0);
                    } else if (data1 == 0x36) { // Touch Release Deck Esquerdo
                        deck_left.jog_touch = false;
                        deck_left.scratch_velocity = 0.0f;
                        deck_left.target_scratch_velocity = 0.0f;
                    } else if (data1 == 0x3F) {
                        shift_deck_a = false;
                    }
                } else if (channel == 1) {
                    if (data1 == 0x0C) { // Cue Release Deck Direito
                        deck_right.releaseCue();
                        sendShortMessage(0x91, 0x0C, deck_right.is_playing ? 127 : 0);
                    } else if (data1 == 0x36) { // Touch Release Deck Direito
                        deck_right.jog_touch = false;
                        deck_right.scratch_velocity = 0.0f;
                        deck_right.target_scratch_velocity = 0.0f;
                    } else if (data1 == 0x3F) {
                        shift_deck_b = false;
                    }
                } else if (channel == 7) {
                    if (deck_left.pad_mode == 1) {
                        deck_left.releasePadRoll();
                    }
                } else if (channel == 9) {
                    if (deck_right.pad_mode == 1) {
                        deck_right.releasePadRoll();
                    }
                }
            }
            // --- CONTROL CHANGE (0xB0 a 0xBF) ---
            else if (msg_type == 0xB0) {
                // Deck Esquerdo Ativo (Channel 0 / 0xB0)
                if (channel == 0) {
                    // Jog Outer Rim Rotation
                    if (data1 == 0x21) {
                        int delta = (int)data2 - 64;
                        float sens = (deck_left.jog_feeling_mode == 0) ? 1.40f : ((deck_left.jog_feeling_mode == 2) ? 0.75f : 1.0f);
                        if (shift_deck_a) {
                            // SHIFT + Rotação do Jog: Fast Search (Busca Rápida CDJ / Rekordbox)
                            deck_left.seekDelta((int)(delta * 12 * sens));
                        } else if (deck_left.is_playing) {
                            deck_left.jog_pitch_bend += (float)delta * 0.0018f * sens;
                            deck_left.jog_pitch_bend = std::clamp(deck_left.jog_pitch_bend, -0.35f, 0.35f);
                        } else {
                            deck_left.seekDelta((int)(delta * sens));
                        }
                        deck_left.jog_visual_angle += (float)delta * 0.025f * sens;
                        if (deck_left.jog_feeling_mode == 0 && std::abs(delta) > 1) {
                            deck_left.jog_momentum_velocity = (float)delta * 0.040f;
                        }
                    }
                    // Jog Platter Top Rotation (Scratch & Seek Bidirecional)
                    else if (data1 == 0x22) {
                        int delta = (int)data2 - 64;
                        float sens = (deck_left.jog_feeling_mode == 0) ? 1.35f : ((deck_left.jog_feeling_mode == 2) ? 0.80f : 1.0f);
                        if (shift_deck_a) {
                            deck_left.seekDelta((int)(delta * 12 * sens));
                        } else {
                            if (deck_left.jog_touch) {
                                deck_left.target_scratch_velocity = (float)delta * 0.09f * sens;
                                deck_left.scratch_active_ticks = 4;
                            } else {
                                deck_left.seekDelta((int)(delta * sens));
                            }
                        }
                        deck_left.jog_visual_angle += (float)delta * 0.025f * sens;
                        if (deck_left.jog_feeling_mode == 0 && std::abs(delta) > 1) {
                            deck_left.jog_momentum_velocity = (float)delta * 0.045f;
                        }
                    }
                    // Seek rápido direto
                    else if (data1 == 0x29) {
                        int delta = (int)data2 - 64;
                        deck_left.seekDelta(delta * 8);
                    }
                    // Tempo / Pitch Fader 14-bit
                    else if (data1 == 0x00) {
                        rate_msb_a = data2;
                        deck_left.setRate14Bit(rate_msb_a, rate_lsb_a, deck_left.pitch_range);
                        sendShortMessage(0x90, 0x58, 0); // Takeover manual: desliga LED SYNC
                    }
                    else if (data1 == 0x20) {
                        rate_lsb_a = data2;
                        deck_left.setRate14Bit(rate_msb_a, rate_lsb_a, deck_left.pitch_range);
                        sendShortMessage(0x90, 0x58, 0); // Takeover manual: desliga LED SYNC
                    }
                    // Volume Fader 14-bit
                    else if (data1 == 0x13) {
                        vol_msb_a = data2;
                        deck_left.setVolume14Bit(vol_msb_a, vol_lsb_a);
                    }
                    else if (data1 == 0x33) {
                        vol_lsb_a = data2;
                        deck_left.setVolume14Bit(vol_msb_a, vol_lsb_a);
                    }
                    // Equalizadores 3-Bandas 14-bit
                    else if (data1 == 0x07) { // High EQ MSB
                        eq_hi_msb_a = data2;
                        deck_left.eq_high = calc14BitEQ(eq_hi_msb_a, eq_hi_lsb_a);
                    }
                    else if (data1 == 0x27) { // High EQ LSB
                        eq_hi_lsb_a = data2;
                        deck_left.eq_high = calc14BitEQ(eq_hi_msb_a, eq_hi_lsb_a);
                    }
                    else if (data1 == 0x0B) { // Mid EQ MSB
                        eq_mid_msb_a = data2;
                        deck_left.eq_mid = calc14BitEQ(eq_mid_msb_a, eq_mid_lsb_a);
                    }
                    else if (data1 == 0x2B) { // Mid EQ LSB
                        eq_mid_lsb_a = data2;
                        deck_left.eq_mid = calc14BitEQ(eq_mid_msb_a, eq_mid_lsb_a);
                    }
                    else if (data1 == 0x0F) { // Low EQ MSB
                        eq_low_msb_a = data2;
                        deck_left.eq_low = calc14BitEQ(eq_low_msb_a, eq_low_lsb_a);
                    }
                    else if (data1 == 0x2F) { // Low EQ LSB
                        eq_low_lsb_a = data2;
                        deck_left.eq_low = calc14BitEQ(eq_low_msb_a, eq_low_lsb_a);
                    }
                }
                // Deck Direito Ativo (Channel 1 / 0xB1)
                else if (channel == 1) {
                    // Jog Outer Rim Rotation Deck Direito
                    if (data1 == 0x21) {
                        int delta = (int)data2 - 64;
                        float sens = (deck_right.jog_feeling_mode == 0) ? 1.40f : ((deck_right.jog_feeling_mode == 2) ? 0.75f : 1.0f);
                        if (shift_deck_b) {
                            // SHIFT + Rotação do Jog: Fast Search (Busca Rápida CDJ / Rekordbox)
                            deck_right.seekDelta((int)(delta * 12 * sens));
                        } else if (deck_right.is_playing) {
                            deck_right.jog_pitch_bend += (float)delta * 0.0018f * sens;
                            deck_right.jog_pitch_bend = std::clamp(deck_right.jog_pitch_bend, -0.35f, 0.35f);
                        } else {
                            deck_right.seekDelta((int)(delta * sens));
                        }
                        deck_right.jog_visual_angle += (float)delta * 0.025f * sens;
                        if (deck_right.jog_feeling_mode == 0 && std::abs(delta) > 1) {
                            deck_right.jog_momentum_velocity = (float)delta * 0.040f;
                        }
                    }
                    // Jog Platter Top Rotation (Scratch & Seek Bidirecional)
                    else if (data1 == 0x22) {
                        int delta = (int)data2 - 64;
                        float sens = (deck_right.jog_feeling_mode == 0) ? 1.35f : ((deck_right.jog_feeling_mode == 2) ? 0.80f : 1.0f);
                        if (shift_deck_b) {
                            deck_right.seekDelta((int)(delta * 12 * sens));
                        } else {
                            if (deck_right.jog_touch) {
                                deck_right.target_scratch_velocity = (float)delta * 0.09f * sens;
                                deck_right.scratch_active_ticks = 4;
                            } else {
                                deck_right.seekDelta((int)(delta * sens));
                            }
                        }
                        deck_right.jog_visual_angle += (float)delta * 0.025f * sens;
                        if (deck_right.jog_feeling_mode == 0 && std::abs(delta) > 1) {
                            deck_right.jog_momentum_velocity = (float)delta * 0.045f;
                        }
                    }
                    // Seek rápido direto
                    else if (data1 == 0x29) {
                        int delta = (int)data2 - 64;
                        deck_right.seekDelta(delta * 8);
                    }
                    else if (data1 == 0x00) {
                        rate_msb_b = data2;
                        deck_right.setRate14Bit(rate_msb_b, rate_lsb_b, deck_right.pitch_range);
                        sendShortMessage(0x91, 0x58, 0); // Takeover manual: desliga LED SYNC
                    }
                    else if (data1 == 0x20) {
                        rate_lsb_b = data2;
                        deck_right.setRate14Bit(rate_msb_b, rate_lsb_b, deck_right.pitch_range);
                        sendShortMessage(0x91, 0x58, 0); // Takeover manual: desliga LED SYNC
                    }
                    else if (data1 == 0x13) {
                        vol_msb_b = data2;
                        deck_right.setVolume14Bit(vol_msb_b, vol_lsb_b);
                    }
                    else if (data1 == 0x33) {
                        vol_lsb_b = data2;
                        deck_right.setVolume14Bit(vol_msb_b, vol_lsb_b);
                    }
                    else if (data1 == 0x07) { // High EQ MSB
                        eq_hi_msb_b = data2;
                        deck_right.eq_high = calc14BitEQ(eq_hi_msb_b, eq_hi_lsb_b);
                    }
                    else if (data1 == 0x27) { // High EQ LSB
                        eq_hi_lsb_b = data2;
                        deck_right.eq_high = calc14BitEQ(eq_hi_msb_b, eq_hi_lsb_b);
                    }
                    else if (data1 == 0x0B) { // Mid EQ MSB
                        eq_mid_msb_b = data2;
                        deck_right.eq_mid = calc14BitEQ(eq_mid_msb_b, eq_mid_lsb_b);
                    }
                    else if (data1 == 0x2B) { // Mid EQ LSB
                        eq_mid_lsb_b = data2;
                        deck_right.eq_mid = calc14BitEQ(eq_mid_msb_b, eq_mid_lsb_b);
                    }
                    else if (data1 == 0x0F) { // Low EQ MSB
                        eq_low_msb_b = data2;
                        deck_right.eq_low = calc14BitEQ(eq_low_msb_b, eq_low_lsb_b);
                    }
                    else if (data1 == 0x2F) { // Low EQ LSB
                        eq_low_lsb_b = data2;
                        deck_right.eq_low = calc14BitEQ(eq_low_msb_b, eq_low_lsb_b);
                    }
                }
                // Mixer & Master (Channel 6 / 0xB6)
                else if (channel == 6) {
                    // Super Knob Deck Esquerdo Ativo (CFX / Filter 14-bit)
                    if (data1 == 0x17) {
                        filter_msb_a = data2;
                        deck_left.filter_bipolar = calc14BitFilter(filter_msb_a, filter_lsb_a);
                    }
                    else if (data1 == 0x37) {
                        filter_lsb_a = data2;
                        deck_left.filter_bipolar = calc14BitFilter(filter_msb_a, filter_lsb_a);
                    }
                    // Super Knob Deck Direito Ativo (CFX / Filter 14-bit)
                    else if (data1 == 0x18) {
                        filter_msb_b = data2;
                        deck_right.filter_bipolar = calc14BitFilter(filter_msb_b, filter_lsb_b);
                    }
                    else if (data1 == 0x38) {
                        filter_lsb_b = data2;
                        deck_right.filter_bipolar = calc14BitFilter(filter_msb_b, filter_lsb_b);
                    }
                    // Crossfader 14-bit
                    else if (data1 == 0x1F) {
                        cf_msb = data2;
                        dj_engine->setCrossfader((float)((cf_msb << 7) | cf_lsb) / 16383.0f);
                    }
                    else if (data1 == 0x3F) {
                        cf_lsb = data2;
                        dj_engine->setCrossfader((float)((cf_msb << 7) | cf_lsb) / 16383.0f);
                    }
                }
            }
        }
    };

} // namespace KuroAudio
