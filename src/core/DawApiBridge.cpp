#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "DawApiBridge.h"
#include "TimelineManager.h"
#include "../ui/Commands.h"

#pragma comment(lib, "ws2_32.lib")

// Declarações externas das variáveis de estado do Abduction Studio V2
extern bool is_playing;
extern KuroDSP::TimelineManager timeline;
extern float track_volumes[8];
extern float track_pans[8];
extern bool track_mutes[8];
extern bool track_solos[8];
extern float g_master_volume;

namespace DawApiBridge {

    // --- FL STUDIO MIXER MODULE INTEGRATION ---
    namespace mixer {
        void setTrackVolume(int track_index, float db_value) {
            if (track_index >= 0 && track_index < 8) {
                track_volumes[track_index] = std::clamp(db_value, -60.0f, 6.0f);
            }
        }
        
        float getTrackVolume(int track_index) {
            if (track_index >= 0 && track_index < 8) {
                return track_volumes[track_index];
            }
            return -60.0f;
        }
        
        void setTrackPan(int track_index, float pan_value) {
            if (track_index >= 0 && track_index < 8) {
                track_pans[track_index] = std::clamp(pan_value, -1.0f, 1.0f);
            }
        }
        
        float getTrackPan(int track_index) {
            if (track_index >= 0 && track_index < 8) {
                return track_pans[track_index];
            }
            return 0.0f;
        }
        
        void muteTrack(int track_index, bool value) {
            if (track_index >= 0 && track_index < 8) {
                track_mutes[track_index] = value;
            }
        }
        
        bool isTrackMuted(int track_index) {
            if (track_index >= 0 && track_index < 8) {
                return track_mutes[track_index];
            }
            return false;
        }
        
        void soloTrack(int track_index, bool value) {
            if (track_index >= 0 && track_index < 8) {
                track_solos[track_index] = value;
            }
        }
        
        bool isTrackSolo(int track_index) {
            if (track_index >= 0 && track_index < 8) {
                return track_solos[track_index];
            }
            return false;
        }
    }
    
    // --- FL STUDIO TRANSPORT MODULE INTEGRATION ---
    namespace transport {
        void start() {
            KuroUI::TransportController::Play(timeline);
        }
        
        void stop() {
            KuroUI::TransportController::Stop(timeline);
        }
        
        bool isPlaying() {
            return is_playing;
        }
    }

    // --- ABLETON LIVE SONG CLASS INTEGRATION ---
    class Song {
    public:
        static inline float getTempo() {
            return timeline.getBPM();
        }
        
        static inline void setTempo(float bpm) {
            timeline.setBPM(std::clamp(bpm, 20.0f, 999.0f));
        }
        
        static inline bool isPlaying() {
            return is_playing;
        }
        
        static inline void startPlaying() {
            KuroUI::TransportController::Play(timeline);
        }
        
        static inline void stopPlaying() {
            KuroUI::TransportController::Stop(timeline);
        }
    };
    
    // --- ABLETON LIVE TRACK CLASS INTEGRATION ---
    class Track {
    public:
        static inline void setMute(int track_index, bool mute) {
            if (track_index >= 0 && track_index < 8) {
                track_mutes[track_index] = mute;
            }
        }
        
        static inline bool getMute(int track_index) {
            if (track_index >= 0 && track_index < 8) {
                return track_mutes[track_index];
            }
            return false;
        }
        
        static inline void setSolo(int track_index, bool solo) {
            if (track_index >= 0 && track_index < 8) {
                track_solos[track_index] = solo;
            }
        }
        
        static inline bool getSolo(int track_index) {
            if (track_index >= 0 && track_index < 8) {
                return track_solos[track_index];
            }
            return false;
        }
    };

    // --- SOCKET LISTENER THREAD (REAL-TIME IPC) ---
    void runUdpListener() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "[DawApiBridge] Erro ao inicializar Winsock2." << std::endl;
            return;
        }
        
        SOCKET recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (recvSocket == INVALID_SOCKET) {
            std::cerr << "[DawApiBridge] Erro ao criar socket UDP." << std::endl;
            WSACleanup();
            return;
        }
        
        sockaddr_in recvAddr;
        recvAddr.sin_family = AF_INET;
        recvAddr.sin_port = htons(9000); // Escuta na porta UDP 9000
        recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(recvSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
            std::cerr << "[DawApiBridge] Erro no bind do socket na porta 9000." << std::endl;
            closesocket(recvSocket);
            WSACleanup();
            return;
        }
        
        std::cout << "[DawApiBridge] Ouvinte UDP de API (Ableton/FL Studio) ativado na porta 9000!" << std::endl;
        
        char recvBuf[1024];
        int recvAddrSize = sizeof(recvAddr);
        
        while (true) {
            int bytes = recvfrom(recvSocket, recvBuf, sizeof(recvBuf) - 1, 0, (SOCKADDR*)&recvAddr, &recvAddrSize);
            if (bytes > 0) {
                recvBuf[bytes] = '\0';
                std::string cmd(recvBuf);
                
                std::stringstream ss(cmd);
                std::string action;
                ss >> action;
                
                if (action == "mixer.setTrackVolume") {
                    int track;
                    float vol;
                    if (ss >> track >> vol) {
                        mixer::setTrackVolume(track, vol);
                    }
                } else if (action == "mixer.setTrackPan") {
                    int track;
                    float pan;
                    if (ss >> track >> pan) {
                        mixer::setTrackPan(track, pan);
                    }
                } else if (action == "mixer.muteTrack") {
                    int track;
                    int mute;
                    if (ss >> track >> mute) {
                        mixer::muteTrack(track, mute != 0);
                    }
                } else if (action == "mixer.soloTrack") {
                    int track;
                    int solo;
                    if (ss >> track >> solo) {
                        mixer::soloTrack(track, solo != 0);
                    }
                } else if (action == "transport.start") {
                    transport::start();
                } else if (action == "transport.stop") {
                    transport::stop();
                } else if (action == "Song.tempo") {
                    float bpm;
                    if (ss >> bpm) {
                        Song::setTempo(bpm);
                    }
                } else if (action == "Song.start_playing") {
                    Song::startPlaying();
                } else if (action == "Song.stop_playing") {
                    Song::stopPlaying();
                } else if (action == "Track.setMute") {
                    int track;
                    int mute;
                    if (ss >> track >> mute) {
                        Track::setMute(track, mute != 0);
                    }
                } else if (action == "Track.setSolo") {
                    int track;
                    int solo;
                    if (ss >> track >> solo) {
                        Track::setSolo(track, solo != 0);
                    }
                }
            } else {
                break;
            }
        }
        closesocket(recvSocket);
        WSACleanup();
    }

    void startBridge() {
        std::thread listenerThread(runUdpListener);
        listenerThread.detach();
    }

    void sendLocalUdpCommand(const std::string& cmd) {
        SOCKET sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sendSocket == INVALID_SOCKET) return;
        
        sockaddr_in recvAddr;
        recvAddr.sin_family = AF_INET;
        recvAddr.sin_port = htons(9000);
        recvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        sendto(sendSocket, cmd.c_str(), (int)cmd.size(), 0, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
        closesocket(sendSocket);
    }
}

