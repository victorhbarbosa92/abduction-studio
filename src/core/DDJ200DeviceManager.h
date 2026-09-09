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
#include "../utils/Logger.h"

#pragma comment(lib, "winmm.lib")

namespace KuroAudio {

    class DDJ200DeviceManager {
    private:
        HMIDIIN hMidiIn = NULL;
        HMIDIOUT hMidiOut = NULL;
        std::atomic<bool> is_connected{ false };
        DJEngine* dj_engine = nullptr;

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

        bool autoConnect(DJEngine* engine = nullptr) {
            if (engine) dj_engine = engine;
            if (is_connected) return true;

            input_dev_id = -1;
            output_dev_id = -1;

            // Busca automática por dispositivos com nome "DDJ" ou "200" ou "Pioneer"
            UINT numIns = midiInGetNumDevs();
            for (UINT i = 0; i < numIns; i++) {
                MIDIINCAPSA caps;
                if (midiInGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
                    std::string name(caps.szPname);
                    if (name.find("DDJ") != std::string::npos || name.find("200") != std::string::npos || name.find("Pioneer") != std::string::npos) {
                        input_dev_id = i;
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
            KuroUtils::Log("[DDJ-200] Hardware Pioneer conectado com sucesso!");
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
            // Pacote SysEx oficial Pioneer DDJ-200 Wakeup / Initial Query
            unsigned char sysex[] = { 0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x0A, 0x00, 0x03, 0x01, 0xF7 };
            MIDIHDR hdr;
            ZeroMemory(&hdr, sizeof(hdr));
            hdr.lpData = (LPSTR)sysex;
            hdr.dwBufferLength = sizeof(sysex);
            hdr.dwBytesRecorded = sizeof(sysex);

            midiOutPrepareHeader(hMidiOut, &hdr, sizeof(hdr));
            midiOutLongMsg(hMidiOut, &hdr, sizeof(hdr));
            while ((hdr.dwFlags & MHDR_DONE) == 0) {
                Sleep(5);
            }
            midiOutUnprepareHeader(hMidiOut, &hdr, sizeof(hdr));
        }

        void turnOffAllLEDs() {
            if (!hMidiOut) return;
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
            if (!hMidiOut) return;
            if (deck_idx == active_left_deck) {
                sendShortMessage(0x90, 0x0B, is_playing ? 127 : 30);
                sendShortMessage(0x90, 0x0C, is_cue ? 127 : 0);
                sendShortMessage(0x90, 0x58, is_sync ? 127 : 0);
            } else if (deck_idx == active_right_deck) {
                sendShortMessage(0x91, 0x0B, is_playing ? 127 : 30);
                sendShortMessage(0x91, 0x0C, is_cue ? 127 : 0);
                sendShortMessage(0x91, 0x58, is_sync ? 127 : 0);
            }
        }

        void updatePadLEDs(int deck_idx, int mode, const DJHotCue* cues) {
            if (!hMidiOut) return;
            unsigned char ch = 0;
            if (deck_idx == active_left_deck) ch = 0x97;
            else if (deck_idx == active_right_deck) ch = 0x99;
            else return;

            for (int i = 0; i < 8; i++) {
                if (mode == 0) { // Hot Cue
                    sendShortMessage(ch, (unsigned char)i, cues[i].active ? 127 : 0);
                } else if (mode == 1) { // Sampler
                    sendShortMessage(ch, (unsigned char)i, 127);
                } else {
                    sendShortMessage(ch, (unsigned char)i, 64);
                }
            }
        }

        int getActiveLeftDeck() const { return active_left_deck; }
        int getActiveRightDeck() const { return active_right_deck; }
        void setActiveLeftDeck(int idx) { 
            active_left_deck = (idx == 2) ? 2 : 0; 
            if (dj_engine) {
                updateDeckLED(active_left_deck, dj_engine->decks[active_left_deck].is_playing, dj_engine->decks[active_left_deck].cue_active, false);
                updatePadLEDs(active_left_deck, dj_engine->decks[active_left_deck].pad_mode, dj_engine->decks[active_left_deck].hot_cues);
            }
        }
        void setActiveRightDeck(int idx) { 
            active_right_deck = (idx == 3) ? 3 : 1; 
            if (dj_engine) {
                updateDeckLED(active_right_deck, dj_engine->decks[active_right_deck].is_playing, dj_engine->decks[active_right_deck].cue_active, false);
                updatePadLEDs(active_right_deck, dj_engine->decks[active_right_deck].pad_mode, dj_engine->decks[active_right_deck].hot_cues);
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
                        deck_left.togglePlay();
                        sendShortMessage(0x90, 0x0B, deck_left.is_playing ? 127 : 30);
                    }
                    else if (data1 == 0x0C) { // Cue
                        deck_left.pressCue();
                        sendShortMessage(0x90, 0x0C, 127);
                    }
                    else if (data1 == 0x48) { // Cue goto and stop
                        deck_left.is_playing = false;
                        deck_left.current_frame = deck_left.cue_frame;
                        sendShortMessage(0x90, 0x0B, 0);
                    }
                    else if (data1 == 0x47) { // Cue set
                        deck_left.cue_frame = deck_left.current_frame;
                    }
                    else if (data1 == 0x58) { // Beat Sync / SHIFT: Alterna Deck 1 <-> 3
                        if (shift_deck_a) {
                            toggleLeftDeck();
                        } else {
                            dj_engine->syncBPM(active_right_deck, active_left_deck);
                            sendShortMessage(0x90, 0x58, 127);
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
                        deck_right.togglePlay();
                        sendShortMessage(0x91, 0x0B, deck_right.is_playing ? 127 : 30);
                    }
                    else if (data1 == 0x0C) {
                        deck_right.pressCue();
                        sendShortMessage(0x91, 0x0C, 127);
                    }
                    else if (data1 == 0x48) {
                        deck_right.is_playing = false;
                        deck_right.current_frame = deck_right.cue_frame;
                        sendShortMessage(0x91, 0x0B, 0);
                    }
                    else if (data1 == 0x47) {
                        deck_right.cue_frame = deck_right.current_frame;
                    }
                    else if (data1 == 0x58) { // Beat Sync / SHIFT: Alterna Deck 2 <-> 4
                        if (shift_deck_b) {
                            toggleRightDeck();
                        } else {
                            dj_engine->syncBPM(active_left_deck, active_right_deck);
                            sendShortMessage(0x91, 0x58, 127);
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
                        if (deck_left.pad_mode == 0) {
                            deck_left.jumpToHotCue(data1);
                            sendShortMessage(0x97, data1, 127);
                        } else if (deck_left.pad_mode == 1) {
                            dj_engine->triggerSampler(data1);
                        } else if (deck_left.pad_mode == 2) {
                            deck_left.toggleLoop(std::pow(2.0, (double)(data1 - 2)));
                        } else if (deck_left.pad_mode == 3) {
                            dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled;
                        }
                    }
                }
                // Shift + Pads Deck Esquerdo Ativo (Channel 8 / 0x98) -> Excluir Hot Cue
                else if (channel == 8) {
                    if (data1 < 8) {
                        deck_left.deleteHotCue(data1);
                        sendShortMessage(0x97, data1, 0);
                    }
                }
                // Pads Deck Direito Ativo (Channel 9 / 0x99)
                else if (channel == 9) {
                    if (data1 < 8) {
                        if (deck_right.pad_mode == 0) {
                            deck_right.jumpToHotCue(data1);
                            sendShortMessage(0x99, data1, 127);
                        } else if (deck_right.pad_mode == 1) {
                            dj_engine->triggerSampler(data1);
                        } else if (deck_right.pad_mode == 2) {
                            deck_right.toggleLoop(std::pow(2.0, (double)(data1 - 2)));
                        } else if (deck_right.pad_mode == 3) {
                            dj_engine->beat_fx.enabled = !dj_engine->beat_fx.enabled;
                        }
                    }
                }
                // Shift + Pads Deck Direito Ativo (Channel 10 / 0x9A) -> Excluir Hot Cue
                else if (channel == 10) {
                    if (data1 < 8) {
                        deck_right.deleteHotCue(data1);
                        sendShortMessage(0x99, data1, 0);
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
                }
            }
            // --- CONTROL CHANGE (0xB0 a 0xBF) ---
            else if (msg_type == 0xB0) {
                // Deck Esquerdo Ativo (Channel 0 / 0xB0)
                if (channel == 0) {
                    // Jog Outer Rim Rotation
                    if (data1 == 0x21) {
                        int delta = (int)data2 - 64;
                        if (shift_deck_a) {
                            // SHIFT + Rotação do Jog: Fast Search (Busca Rápida CDJ / Rekordbox)
                            deck_left.seekDelta(delta * 28);
                        } else if (deck_left.is_playing) {
                            deck_left.jog_pitch_bend += (float)delta * 0.0035f;
                            deck_left.jog_pitch_bend = std::clamp(deck_left.jog_pitch_bend, -0.40f, 0.40f);
                        } else {
                            deck_left.seekDelta(delta * 2);
                        }
                        deck_left.jog_visual_angle += (float)delta * 0.035f;
                    }
                    // Jog Platter Top Rotation (Scratch & Seek Bidirecional)
                    else if (data1 == 0x22) {
                        int delta = (int)data2 - 64;
                        if (shift_deck_a) {
                            deck_left.seekDelta(delta * 28);
                        } else {
                            if (deck_left.jog_touch) {
                                deck_left.target_scratch_velocity = (float)delta * 0.18f;
                                deck_left.scratch_active_ticks = 4;
                            }
                            deck_left.seekDelta(delta * 2);
                        }
                        deck_left.jog_visual_angle += (float)delta * 0.035f;
                    }
                    // Seek rápido direto
                    else if (data1 == 0x29) {
                        int delta = (int)data2 - 64;
                        deck_left.seekDelta(delta * 16);
                    }
                    // Tempo / Pitch Fader 14-bit
                    else if (data1 == 0x00) {
                        rate_msb_a = data2;
                        deck_left.setRate14Bit(rate_msb_a, rate_lsb_a, 8.0f);
                    }
                    else if (data1 == 0x20) {
                        rate_lsb_a = data2;
                        deck_left.setRate14Bit(rate_msb_a, rate_lsb_a, 8.0f);
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
                        if (shift_deck_b) {
                            // SHIFT + Rotação do Jog: Fast Search (Busca Rápida CDJ / Rekordbox)
                            deck_right.seekDelta(delta * 28);
                        } else if (deck_right.is_playing) {
                            deck_right.jog_pitch_bend += (float)delta * 0.0035f;
                            deck_right.jog_pitch_bend = std::clamp(deck_right.jog_pitch_bend, -0.40f, 0.40f);
                        } else {
                            deck_right.seekDelta(delta * 2);
                        }
                        deck_right.jog_visual_angle += (float)delta * 0.035f;
                    }
                    // Jog Platter Top Rotation (Scratch & Seek Bidirecional)
                    else if (data1 == 0x22) {
                        int delta = (int)data2 - 64;
                        if (shift_deck_b) {
                            deck_right.seekDelta(delta * 28);
                        } else {
                            if (deck_right.jog_touch) {
                                deck_right.target_scratch_velocity = (float)delta * 0.18f;
                                deck_right.scratch_active_ticks = 4;
                            }
                            deck_right.seekDelta(delta * 2);
                        }
                        deck_right.jog_visual_angle += (float)delta * 0.035f;
                    }
                    // Seek rápido direto
                    else if (data1 == 0x29) {
                        int delta = (int)data2 - 64;
                        deck_right.seekDelta(delta * 16);
                    }
                    else if (data1 == 0x00) {
                        rate_msb_b = data2;
                        deck_right.setRate14Bit(rate_msb_b, rate_lsb_b, 8.0f);
                    }
                    else if (data1 == 0x20) {
                        rate_lsb_b = data2;
                        deck_right.setRate14Bit(rate_msb_b, rate_lsb_b, 8.0f);
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
