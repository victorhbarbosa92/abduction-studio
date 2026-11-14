#include "RtAudio.h"
#include "src/core/DJEngine.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== AUDIO SYSTEM DIAGNOSTIC ===" << std::endl;
    
    RtAudio adc(RtAudio::WINDOWS_WASAPI);
    unsigned int deviceCount = adc.getDeviceCount();
    std::cout << "Device Count: " << deviceCount << std::endl;
    
    unsigned int defaultOut = adc.getDefaultOutputDevice();
    std::cout << "Default Output Device ID: " << defaultOut << std::endl;
    
    for (unsigned int i = 0; i < deviceCount; i++) {
        RtAudio::DeviceInfo info = adc.getDeviceInfo(i);
        std::cout << "Device " << i << ": " << info.name 
                  << " | outChans: " << info.outputChannels 
                  << " | inChans: " << info.inputChannels 
                  << " | isDefault: " << info.isDefaultOutput 
                  << " | prefSR: " << info.preferredSampleRate << std::endl;
    }
    
    std::string wav_path = "scratch/tracks/track_1725169850.wav";
    std::cout << "Testing loading: " << wav_path << std::endl;
    KuroAudio::DJEngine engine;
    bool ok = engine.deckA.loadTrack(wav_path);
    std::cout << "Loaded: " << ok << ", frames: " << engine.deckA.buffer_l.size() 
              << ", duration: " << engine.deckA.duration_sec 
              << ", sample_rate: " << engine.deckA.sample_rate << std::endl;
    
    if (ok && !engine.deckA.buffer_l.empty()) {
        float max_val = 0.0f;
        for (float s : engine.deckA.buffer_l) {
            if (std::abs(s) > max_val) max_val = std::abs(s);
        }
        std::cout << "Deck A buffer_l max absolute sample: " << max_val << std::endl;
    }

    // Test opening stream on default device
    if (deviceCount > 0) {
        RtAudio::StreamParameters parameters;
        parameters.deviceId = defaultOut;
        parameters.nChannels = 2;
        parameters.firstChannel = 0;
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 256;
        
        std::cout << "Attempting to open stream with 44100 Hz..." << std::endl;
        try {
            adc.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, 
                [](void* out, void* in, unsigned int n, double t, RtAudioStreamStatus s, void* u) -> int {
                    return 0;
                }, NULL);
            std::cout << "openStream(44100) SUCCEEDED! Buffer frames: " << bufferFrames << std::endl;
            adc.closeStream();
        } catch (const std::exception& e) {
            std::cout << "openStream(44100) FAILED with std::exception: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "openStream(44100) FAILED with unknown exception!" << std::endl;
        }

        // Test with preferredSampleRate
        RtAudio::DeviceInfo defInfo = adc.getDeviceInfo(defaultOut);
        if (defInfo.preferredSampleRate != 44100 && defInfo.preferredSampleRate > 0) {
            sampleRate = defInfo.preferredSampleRate;
            std::cout << "Attempting to open stream with preferred sample rate: " << sampleRate << " Hz..." << std::endl;
            try {
                adc.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, 
                    [](void* out, void* in, unsigned int n, double t, RtAudioStreamStatus s, void* u) -> int {
                        return 0;
                    }, NULL);
                std::cout << "openStream(" << sampleRate << ") SUCCEEDED! Buffer frames: " << bufferFrames << std::endl;
                adc.closeStream();
            } catch (const std::exception& e) {
                std::cout << "openStream(" << sampleRate << ") FAILED: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "openStream(" << sampleRate << ") FAILED with unknown exception!" << std::endl;
            }
        }
    }
    
    return 0;
}
