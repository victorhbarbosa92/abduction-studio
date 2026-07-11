#pragma once
#include "LFOEngine.h"
#include <string>
#include <algorithm>

namespace KuroDSP {

    struct LFOModulator {
        LFOEngine engine;
        bool active = false;
        int shape = 0; // 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square
        float rate = 1.0f; // Hz
        float depth = 0.5f; // 0.0 to 1.0
        bool sync = false;
        int sync_rate_idx = 2; // default 1/4 note
        
        int target_track = -1; // -1 = None, 0-7 = Track
        int target_param = 0;  // 0 = Dark Drive, 1 = Alien Freq, 2 = Ritual Depth, 3 = Ritual Rate
        
        // Base/default values of target parameter
        float base_value = 2.5f;
    };
}

extern KuroDSP::LFOModulator g_lfo_modulators[4];
