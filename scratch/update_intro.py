import re

def update_song_arranger():
    path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\core\PsySongArranger.h"
    with open(path, "r", encoding="utf-8") as f:
        code = f.read()

    # Enhance INTRO section in PsySongArranger.h
    old_intro = """            // ── 1. INTRO ──
            if (cfg.has_intro && cfg.intro_bars > 0) {
                float sec_len = cfg.intro_bars * bar;
                tl.addSectionMarker(cur_t, "INTRO", 0xFFFFD700);

                int blocks = cfg.intro_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                // Áudio clips estéreo visíveis imediatamente no topo da timeline (Compassos 1 a 16)
                addAudioClip(0, "PRYZMA_PsyKick_Stereo_Stem.wav", cur_t, block_dur, 0);
                addAudioClip(4, "PRYZMA_Percussion_Stereo_Loop.wav", cur_t + block_dur, block_dur, 0);

                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(6, 6, bt, block_dur);   // Tribal Percussion
                    addClip(11, 11, bt, block_dur); // Dark Sub Drone
                    addClip(12, 12, bt, block_dur); // Mystic Strings
                    addClip(14, 14, bt, block_dur); // Vocal Mantra
                    addAudioClip(14, "PRYZMA_Vocal_Chant_142.wav", bt, block_dur, 0xFFFF5078);
                    addAudioClip(15, "PRYZMA_FX_Atmo_Sweep.wav", bt, block_dur, 0xFFFF3232);
                    if (b > 0) {
                        addClip(5, 5, bt, block_dur); // Shakers
                    }
                }
                cur_t += sec_len;
            }"""

    new_intro = """            // ── 1. INTRO (Full Psytrance Intro with Stereo Stems & Neon MIDI Patterns) ──
            if (cfg.has_intro && cfg.intro_bars > 0) {
                float sec_len = cfg.intro_bars * bar;
                tl.addSectionMarker(cur_t, "INTRO", 0xFFFFD700);

                int blocks = cfg.intro_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                // Audio Stem Kick Stereo contínuo por toda a Intro (Compassos 1 a 16)
                addAudioClip(0, "PRYZMA_PsyKick_Stereo_Stem.wav", cur_t, sec_len, 0);

                // Compasso 1 a 8: Atmosfera, Arpeggios e Percussão Tribal
                addClip(6, 6, cur_t, block_dur);   // 06. Tribal Percussion
                addClip(7, 7, cur_t, block_dur);   // 07. FM Squelch & Modular Zaps
                addClip(9, 9, cur_t, block_dur);   // 09. Counter-Arp Pluck Matrix
                addClip(11, 11, cur_t, sec_len);   // 11. Dark Sub Drone
                addClip(12, 12, cur_t, sec_len);   // 12. Mystic Strings & Pads
                addAudioClip(14, "PRYZMA_Vocal_Chant_142.wav", cur_t, sec_len, 0xFFFF5078);
                addAudioClip(15, "PRYZMA_FX_Atmo_Sweep.wav", cur_t, sec_len, 0xFFFF3232);

                // Compasso 9 a 16 (Intro Parte 2): O Rolling Bassline entra com força total!
                float t_p2 = cur_t + block_dur;
                addClip(1, 2, t_p2, block_dur);    // 02. Sub Bass (KB-B-B)
                addClip(2, 2, t_p2, block_dur);    // 03. Mid Saw Bass (KB-B-B)
                addClip(3, 4, t_p2, block_dur);    // 04. Snare & Smash Clap
                addAudioClip(4, "PRYZMA_Percussion_Stereo_Loop.wav", t_p2, block_dur, 0);
                addClip(5, 5, t_p2, block_dur);    // 05. Closed Hats & Shakers
                addClip(6, 6, t_p2, block_dur);    // 06. Tribal Percussion
                addClip(7, 7, t_p2, block_dur);    // 07. FM Squelch
                addClip(8, 8, t_p2, block_dur);    // 08. Main Acid Lead (303 Hook)
                addClip(9, 9, t_p2, block_dur);    // 09. Counter-Arp Pluck Matrix

                cur_t += sec_len;
            }"""

    if old_intro in code:
        code = code.replace(old_intro, new_intro)
        print("PsySongArranger.h updated with complete Intro clips!")
    else:
        print("Warning: old_intro pattern not found in PsySongArranger.h, checking alternative...")
        # fallback replacement using regex
        pattern = r"// ── 1\. INTRO ──.*?cur_t \+= sec_len;\s*\}"
        code = re.sub(pattern, new_intro, code, flags=re.DOTALL)
        print("PsySongArranger.h updated with regex.")

    with open(path, "w", encoding="utf-8") as f:
        f.write(code)

if __name__ == "__main__":
    update_song_arranger()
