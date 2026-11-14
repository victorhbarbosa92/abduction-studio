import re

path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\core\DJEngine.h'
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Expand hot_cues[8] to hot_cues[16]
old_cues_decl = "        // 8 Hot Cues\n        DJHotCue hot_cues[8];"
new_cues_decl = "        // 16 Hot Cues (A..P Pioneer Flagship)\n        DJHotCue hot_cues[16];"
assert old_cues_decl in c, "old_cues_decl not found"
c = c.replace(old_cues_decl, new_cues_decl)

# 2. Expand constructor loop for hot cues
old_ctor_cues = """            for (int i = 0; i < 8; i++) {
                hot_cues[i].id = i;
                hot_cues[i].active = false;
                hot_cues[i].time_sec = 0.0;
                uint32_t colors[] = { 
                    0xFF00E5FF, 0xFFFF0055, 0xFF00FF66, 0xFFFFCC00, 
                    0xFFBF00FF, 0xFFFF6600, 0xFF00FFFF, 0xFFFFFFFF 
                };
                hot_cues[i].color_rgba = colors[i % 8];
            }"""

new_ctor_cues = """            uint32_t colors[16] = { 
                0xFF00E5FF, 0xFFFF0055, 0xFF00FF66, 0xFFFFCC00, 
                0xFFBF00FF, 0xFFFF6600, 0xFF00FFFF, 0xFFFFFFFF,
                0xFFFF3399, 0xFF33CCFF, 0xFFFF9933, 0xFF66FF33,
                0xFFCC33FF, 0xFFFFFF33, 0xFF33FFFF, 0xFFFF6666
            };
            for (int i = 0; i < 16; i++) {
                hot_cues[i].id = i;
                hot_cues[i].active = false;
                hot_cues[i].time_sec = 0.0;
                hot_cues[i].color_rgba = colors[i % 16];
            }"""
assert old_ctor_cues in c, "old_ctor_cues not found"
c = c.replace(old_ctor_cues, new_ctor_cues)

# 3. setHotCue, jumpToHotCue, deleteHotCue bounds
c = c.replace("void setHotCue(int index) {\n            if (index < 0 || index >= 8) return;",
              "void setHotCue(int index) {\n            if (index < 0 || index >= 16) return;")
c = c.replace("void jumpToHotCue(int index) {\n            if (index < 0 || index >= 8) return;",
              "void jumpToHotCue(int index) {\n            if (index < 0 || index >= 16) return;")
c = c.replace("void deleteHotCue(int index) {\n            if (index < 0 || index >= 8) return;",
              "void deleteHotCue(int index) {\n            if (index < 0 || index >= 16) return;")

# 4. Copy loop in clone
c = c.replace("for (int i = 0; i < 8; i++) hot_cues[i] = src.hot_cues[i];",
              "for (int i = 0; i < 16; i++) hot_cues[i] = src.hot_cues[i];")

# 5. DJSamplerSlot samplers[8] -> samplers[16]
old_smp_decl = "        DJSamplerSlot samplers[8];"
new_smp_decl = "        DJSamplerSlot samplers[16];"
assert old_smp_decl in c, "old_smp_decl not found"
c = c.replace(old_smp_decl, new_smp_decl)

# 6. initBuiltinSamplers
old_smp_init = """        void initBuiltinSamplers() {
            std::string names[8] = { 
                "CLAP SHOT", "PSY LASER", "SUB DROP", "SIREN RISER", 
                "SNARE ROLL", "CRASH BOOM", "AIR HORN", "VOCAL DROP" 
            };
            
            for (int i = 0; i < 8; i++) {
                samplers[i].name = names[i];
                samplers[i].is_playing = false;
                samplers[i].playback_pos = 0.0;
                int len = 44100 * (i == 2 || i == 3 ? 2 : 1);
                samplers[i].buffer_l.resize(len);
                samplers[i].buffer_r.resize(len);
                
                for (int s = 0; s < len; s++) {
                    float t = (float)s / 44100.0f;
                    float env = std::exp(-t * (i == 0 ? 12.0f : (i == 1 ? 6.0f : 3.0f)));
                    float wave = 0.0f;
                    if (i == 0) {
                        wave = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * env;
                    } else if (i == 1) {
                        float f = 2500.0f * std::exp(-t * 15.0f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    } else if (i == 2) {
                        float f = 120.0f * std::exp(-t * 2.5f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * std::exp(-t * 1.5f);
                    } else {
                        float f = 440.0f + i * 110.0f;
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    }
                    samplers[i].buffer_l[s] = wave * 0.8f;
                    samplers[i].buffer_r[s] = wave * 0.8f;
                }
            }
        }

        void triggerSampler(int slot) {
            if (slot >= 0 && slot < 8) {"""

new_smp_init = """        void initBuiltinSamplers() {
            std::string names[16] = { 
                "CLAP SHOT", "PSY LASER", "SUB DROP", "SIREN RISER", 
                "SNARE ROLL", "CRASH BOOM", "AIR HORN", "VOCAL DROP",
                "808 SUB", "SHAKER 16", "GLITCH HIT", "REV CYMBAL",
                "ACID STAB", "CHORD HIT", "SCRATCH FX", "WHITE NOISE"
            };
            
            for (int i = 0; i < 16; i++) {
                samplers[i].name = names[i];
                samplers[i].is_playing = false;
                samplers[i].playback_pos = 0.0;
                int len = 44100 * (i == 2 || i == 3 || i == 8 || i == 11 ? 2 : 1);
                samplers[i].buffer_l.resize(len);
                samplers[i].buffer_r.resize(len);
                
                for (int s = 0; s < len; s++) {
                    float t = (float)s / 44100.0f;
                    float env = std::exp(-t * (i == 0 || i == 9 ? 12.0f : (i == 1 || i == 10 ? 6.0f : 3.0f)));
                    float wave = 0.0f;
                    if (i == 0 || i == 9 || i == 15) {
                        wave = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * env;
                    } else if (i == 1 || i == 10) {
                        float f = 2500.0f * std::exp(-t * 15.0f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    } else if (i == 2 || i == 8) {
                        float f = (i == 8 ? 65.0f : 120.0f) * std::exp(-t * 2.0f);
                        wave = std::sin(2.0f * (float)M_PI * f * t) * std::exp(-t * 1.2f);
                    } else {
                        float f = 220.0f + (i % 8) * 110.0f;
                        wave = std::sin(2.0f * (float)M_PI * f * t) * env;
                    }
                    samplers[i].buffer_l[s] = wave * 0.8f;
                    samplers[i].buffer_r[s] = wave * 0.8f;
                }
            }
        }

        void triggerSampler(int slot) {
            if (slot >= 0 && slot < 16) {"""

assert old_smp_init in c, "old_smp_init not found"
c = c.replace(old_smp_init, new_smp_init)

# 7. Mixer loop: for (int s = 0; s < 8; s++) -> 16
c = c.replace("for (int s = 0; s < 8; s++) {\n                    if (samplers[s].is_playing) {",
              "for (int s = 0; s < 16; s++) {\n                    if (samplers[s].is_playing) {")

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("DJEngine.h successfully updated for 16 Hot Cues and 16 Sampler slots!")
