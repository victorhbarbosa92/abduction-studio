#include <iostream>
#include "src/core/QMBeatGridEngine.h"

int main() {
    std::string track = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/scratch/tracks/Atohmic, Synyster Live & Aspx - Shake It/Atohmic, Synyster Live & Aspx - Shake It.wav";
    std::cout << "Testing QMBeatGridEngine on: " << track << std::endl;
    auto res = KuroAudio::QMBeatGridEngine::analyzeFile(track, 135.0);
    std::cout << "Result: is_valid=" << res.is_valid << " bpm=" << res.bpm << " raw_bpm=" << res.raw_bpm << " first_beat_sec=" << res.first_beat_sec << " total_beats=" << res.total_beats << std::endl;
    return 0;
}
