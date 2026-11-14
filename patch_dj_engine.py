import re

file_path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\core\DJEngine.h'
with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Update DJBeatFXUnit class variables for Single-FX
target_decl = '''        // Modo Multi-FX vs Single-FX
        int fx_mode = 0; // 0 = MULTI-FX [...], 1 = SINGLE-FX [•]
        FXSlot multi_slots[4];
        bool auto_bpm = true;
        double bpm = 120.0;'''

replacement_decl = '''        // Modo Multi-FX vs Single-FX
        int fx_mode = 0; // 0 = MULTI-FX [...], 1 = SINGLE-FX [•]
        FXSlot multi_slots[4];
        bool auto_bpm = true;
        double bpm = 120.0;

        // Parâmetros estendidos para modo Single-FX
        float single_tail_decay = 0.72f;
        float single_reverb_mix = 0.35f;
        float single_smooth_gain = 0.0f;
        float single_tail_energy = 0.0f;'''

if target_decl in content:
    content = content.replace(target_decl, replacement_decl)

# 2. Update processSingleEffect signature and opening
old_proc_sig = '''        void processSingleEffect(int fx_type, float eff_dry_wet, float beat_frac, float* left, float* right, unsigned int frames, double bpm, double sample_rate, int slot_idx) {
            int s = std::clamp(slot_idx, 0, 3);
            double beat_sec = 60.0 / std::clamp(bpm, 40.0, 220.0);
            int delay_frames = (int)(beat_frac * beat_sec * sample_rate);
            if (delay_frames < 64) delay_frames = 64;
            if (delay_frames >= BUFFER_SIZE - 2000) delay_frames = BUFFER_SIZE - 2000;

            float wet_mix = std::clamp(eff_dry_wet, 0.0f, 1.0f);
            float dry_mix = 1.0f - wet_mix * 0.45f;
            if (fx_type == FX_TRANS || fx_type == FX_VINYL_BRAKE || fx_type == FX_ROLL || fx_type == FX_SLIP_ROLL || fx_type == FX_REV_ROLL) {
                dry_mix = 1.0f - wet_mix;
            }

            for (unsigned int i = 0; i < frames; i++) {
                float in_l = left[i];
                float in_r = right[i];
                float fx_l = 0.0f;
                float fx_r = 0.0f;'''

new_proc_sig = '''        void processSingleEffect(int fx_type, float eff_dry_wet, float beat_frac, float* left, float* right, unsigned int frames, double bpm, double sample_rate, int slot_idx,
                                 bool is_slot_enabled = true, float tail_decay = 0.72f, float reverb_mix = 0.35f, float* p_smooth_gain = nullptr, float* p_tail_energy = nullptr) {
            int s = std::clamp(slot_idx, 0, 3);
            double beat_sec = 60.0 / std::clamp(bpm, 40.0, 220.0);
            int delay_frames = (int)(beat_frac * beat_sec * sample_rate);
            if (delay_frames < 64) delay_frames = 64;
            if (delay_frames >= BUFFER_SIZE - 2000) delay_frames = BUFFER_SIZE - 2000;

            float wet_mix = std::clamp(eff_dry_wet, 0.0f, 1.0f);
            float target_smooth = is_slot_enabled ? 1.0f : 0.0f;
            float smooth_gain = p_smooth_gain ? *p_smooth_gain : target_smooth;
            float fb = std::clamp(tail_decay, 0.15f, 0.95f);
            float block_energy = 0.0f;

            for (unsigned int i = 0; i < frames; i++) {
                // Envelope suave (~8ms) para transicao sem estalos (fade-in / fade-out)
                smooth_gain += (target_smooth - smooth_gain) * 0.005f;

                float in_l = left[i];
                float in_r = right[i];

                // Audio novo alimentado no efeito (faded out quando desligado)
                float fx_in_l = in_l * smooth_gain;
                float fx_in_r = in_r * smooth_gain;

                float fx_l = 0.0f;
                float fx_r = 0.0f;'''

if old_proc_sig in content:
    content = content.replace(old_proc_sig, new_proc_sig)

# In the delay cases, replace in_l with fx_in_l and in_r with fx_in_r for buffer writing
content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.60f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.60f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + del_l * 0.68f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + del_r * 0.68f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + std::clamp(fx_l * 0.64f, -1.5f, 1.5f);', 'delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(fx_l * fb, -1.5f, 1.5f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + std::clamp(fx_r * 0.64f, -1.5f, 1.5f);', 'delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(fx_r * fb, -1.5f, 1.5f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.74f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.74f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.45f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.75f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.45f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.75f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + (fx_l + fx_r) * 0.25f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + (fx_l + fx_r) * (fb * 0.40f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + (fx_l + fx_r) * 0.25f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + (fx_l + fx_r) * (fb * 0.40f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + std::clamp(del_l * 0.62f, -1.2f, 1.2f);', 'delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(del_l * fb, -1.2f, 1.2f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + std::clamp(del_r * 0.62f, -1.2f, 1.2f);', 'delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(del_r * fb, -1.2f, 1.2f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + del_l * 0.65f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + del_r * 0.65f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.65f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.65f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.72f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.95f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.72f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.95f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.5f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.70f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.5f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.70f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.55f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * (fb * 0.75f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.55f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * (fb * 0.75f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + std::clamp(fx_l * 0.68f, -1.5f, 1.5f);', 'delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(fx_l * fb, -1.5f, 1.5f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + std::clamp(fx_r * 0.68f, -1.5f, 1.5f);', 'delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(fx_r * fb, -1.5f, 1.5f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + std::clamp(fx_l * 0.70f, -1.5f, 1.5f);', 'delay_buf_l[s][write_pos[s]] = fx_in_l + std::clamp(fx_l * fb, -1.5f, 1.5f);')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + std::clamp(fx_r * 0.70f, -1.5f, 1.5f);', 'delay_buf_r[s][write_pos[s]] = fx_in_r + std::clamp(fx_r * fb, -1.5f, 1.5f);')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + del_l * 0.66f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + del_r * 0.66f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + del_l * 0.62f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + del_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + del_r * 0.62f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + del_r * fb;')

content = content.replace('delay_buf_l[s][write_pos[s]] = in_l + fx_l * 0.70f;', 'delay_buf_l[s][write_pos[s]] = fx_in_l + fx_l * fb;')
content = content.replace('delay_buf_r[s][write_pos[s]] = in_r + fx_r * 0.70f;', 'delay_buf_r[s][write_pos[s]] = fx_in_r + fx_r * fb;')

# 3. Update the tail / mixing and processBlock
old_tail_mix = '''                if (eff_dry_wet <= 0.001f) {
                    roll_locked[s] = false;
                    vinyl_brake_speed[s] = 1.0f;
                    vinyl_brake_pos[s] = (double)write_pos[s];
                }

                left[i] = in_l * dry_mix + fx_l * wet_mix;
                right[i] = in_r * dry_mix + fx_r * wet_mix;

                write_pos[s] = (write_pos[s] + 1) % BUFFER_SIZE;
            }
        }

        void processBlock(float* left, float* right, unsigned int frames, double track_bpm, double sample_rate) {
            double eff_bpm = (auto_bpm || bpm <= 30.0) ? track_bpm : bpm;
            if (eff_bpm <= 10.0) eff_bpm = 120.0;
            if (fx_mode == 0) {
                // Multi-FX: Processa qualquer slot ativo ou com dry_wet > 0.001f
                for (int s = 0; s < 4; s++) {
                    float eff_dry_wet = multi_slots[s].dry_wet;
                    if (multi_slots[s].enabled || eff_dry_wet > 0.001f) {
                        multi_slots[s].enabled = (eff_dry_wet > 0.001f);
                        if (eff_dry_wet > 0.001f) {
                            enabled = true;
                            processSingleEffect(multi_slots[s].type, eff_dry_wet, multi_slots[s].beat_fraction, left, right, frames, eff_bpm, sample_rate, s);
                        }
                    }
                }
            } else {
                // Single-FX: Processa o efeito principal se dry_wet > 0.001f ou enabled
                float eff_dry_wet = dry_wet;
                if (enabled || eff_dry_wet > 0.001f) {
                    enabled = (eff_dry_wet > 0.001f);
                    if (eff_dry_wet > 0.001f) {
                        processSingleEffect(type, eff_dry_wet, beat_fraction, left, right, frames, eff_bpm, sample_rate, 0);
                    }
                }
            }
        }'''

new_tail_mix = '''                // Injecao de Reverb Difuso / Espacial nos Efeitos ("reverbs nos efeitos")
                if (reverb_mix > 0.02f) {
                    int rt1 = (write_pos[s] - delay_frames / 5 + BUFFER_SIZE) % BUFFER_SIZE;
                    int rt2 = (write_pos[s] - delay_frames / 3 + BUFFER_SIZE) % BUFFER_SIZE;
                    int rt3 = (write_pos[s] - (delay_frames * 2) / 3 + BUFFER_SIZE) % BUFFER_SIZE;
                    float rev_d_l = (delay_buf_l[s][rt1] * 0.35f + delay_buf_r[s][rt2] * 0.35f + delay_buf_l[s][rt3] * 0.30f);
                    float rev_d_r = (delay_buf_r[s][rt1] * 0.35f + delay_buf_l[s][rt2] * 0.35f + delay_buf_l[s][rt3] * 0.30f);
                    fx_l = fx_l * (1.0f - reverb_mix * 0.35f) + rev_d_l * (reverb_mix * 0.65f);
                    fx_r = fx_r * (1.0f - reverb_mix * 0.35f) + rev_d_r * (reverb_mix * 0.65f);
                }

                // Quando o slot esta desligado mas a cauda continua, o som seco (dry) permanece 100% limpo
                float dry_mix = is_slot_enabled ? (1.0f - wet_mix * 0.45f) : 1.0f;
                if (is_slot_enabled && (fx_type == FX_TRANS || fx_type == FX_VINYL_BRAKE || fx_type == FX_ROLL || fx_type == FX_SLIP_ROLL || fx_type == FX_REV_ROLL)) {
                    dry_mix = 1.0f - wet_mix;
                }

                if (eff_dry_wet <= 0.001f && !is_slot_enabled) {
                    roll_locked[s] = false;
                    vinyl_brake_speed[s] = 1.0f;
                    vinyl_brake_pos[s] = (double)write_pos[s];
                }

                left[i] = in_l * dry_mix + fx_l * wet_mix;
                right[i] = in_r * dry_mix + fx_r * wet_mix;

                block_energy += std::abs(fx_l) + std::abs(fx_r);
                write_pos[s] = (write_pos[s] + 1) % BUFFER_SIZE;
            }

            if (p_smooth_gain) *p_smooth_gain = smooth_gain;
            if (p_tail_energy) *p_tail_energy = (*p_tail_energy) * 0.92f + (block_energy / (float)frames) * 0.08f;
        }

        void processBlock(float* left, float* right, unsigned int frames, double track_bpm, double sample_rate) {
            double eff_bpm = (auto_bpm || bpm <= 30.0) ? track_bpm : bpm;
            if (eff_bpm <= 10.0) eff_bpm = 120.0;
            if (fx_mode == 0) {
                // Multi-FX: Processa qualquer slot onde o botao ON esta ligado OU houver cauda residual tocando
                for (int s = 0; s < 4; s++) {
                    auto& slot = multi_slots[s];
                    bool has_tail = (slot.tail_energy > 0.0001f || slot.smooth_gain > 0.001f);
                    if (slot.enabled || has_tail) {
                        processSingleEffect(slot.type, slot.dry_wet, slot.beat_fraction, left, right, frames, eff_bpm, sample_rate, s,
                                            slot.enabled, slot.tail_decay, slot.reverb_mix, &slot.smooth_gain, &slot.tail_energy);
                    }
                }
            } else {
                // Single-FX: Processa se enabled ou cauda ativa
                bool has_tail = (single_tail_energy > 0.0001f || single_smooth_gain > 0.001f);
                if (enabled || has_tail) {
                    processSingleEffect(type, dry_wet, beat_fraction, left, right, frames, eff_bpm, sample_rate, 0,
                                        enabled, single_tail_decay, single_reverb_mix, &single_smooth_gain, &single_tail_energy);
                }
            }
        }'''

if old_tail_mix in content:
    content = content.replace(old_tail_mix, new_tail_mix)

# 4. Post-fader routing in DJEngine.processAudioBlock
old_routing = '''            // Channel assignment para Pioneer Beat FX (FX 1 e FX 2)
            auto applyBeatFXToChannel = [&](DJBeatFXUnit& fx, int ch_id, float* l, float* r, double ch_bpm) {
                if (fx.channel_assign == ch_id) {
                    fx.processBlock(l, r, frames, ch_bpm, host_sample_rate);
                }
            };

            applyBeatFXToChannel(beat_fx, 0, blk0_l.data(), blk0_r.data(), decks[0].bpm);
            applyBeatFXToChannel(beat_fx, 1, blk1_l.data(), blk1_r.data(), decks[1].bpm);
            applyBeatFXToChannel(beat_fx, 3, blk2_l.data(), blk2_r.data(), decks[2].bpm);
            applyBeatFXToChannel(beat_fx, 4, blk3_l.data(), blk3_r.data(), decks[3].bpm);

            applyBeatFXToChannel(beat_fx2, 0, blk0_l.data(), blk0_r.data(), decks[0].bpm);
            applyBeatFXToChannel(beat_fx2, 1, blk1_l.data(), blk1_r.data(), decks[1].bpm);
            applyBeatFXToChannel(beat_fx2, 3, blk2_l.data(), blk2_r.data(), decks[2].bpm);
            applyBeatFXToChannel(beat_fx2, 4, blk3_l.data(), blk3_r.data(), decks[3].bpm);

            // Crossfader e Soma de 4 Decks (Lado A = Deck 1 e 3; Lado B = Deck 2 e 4)
            float gain_a = 1.0f;
            float gain_b = 1.0f;
            if (crossfader_curve == 1) { // 1 = SHARP / SCRATCH
                gain_a = (crossfader >= 0.92f) ? (1.0f - crossfader) / 0.08f : 1.0f;
                gain_b = (crossfader <= 0.08f) ? crossfader / 0.08f : 1.0f;
            } else if (crossfader_curve == 2) { // 2 = LINEAR
                gain_a = 1.0f - crossfader;
                gain_b = crossfader;
            } else { // 0 = SMOOTH EQUAL POWER
                float cf_rad = crossfader * (float)(M_PI * 0.5);
                gain_a = std::cos(cf_rad);
                gain_b = std::sin(cf_rad);
            }

            std::vector<float> master_l(frames, 0.0f);
            std::vector<float> master_r(frames, 0.0f);

            float vol0 = decks[0].volume;
            float vol1 = decks[1].volume;
            float vol2 = decks[2].volume;
            float vol3 = decks[3].volume;

            for (unsigned int i = 0; i < frames; i++) {
                float side_a_l = blk0_l[i] * vol0 + blk2_l[i] * vol2;
                float side_a_r = blk0_r[i] * vol0 + blk2_r[i] * vol2;
                float side_b_l = blk1_l[i] * vol1 + blk3_l[i] * vol3;
                float side_b_r = blk1_r[i] * vol1 + blk3_r[i] * vol3;'''

new_routing = '''            // Pre-escala de Volume do Canal: Transforma Beat FX em 100% POST-FADER!
            // Quando o fader de volume e puxado para zero, o audio da musica cessa,
            // mas a cauda de reverb/eco dos efeitos continua soando naturalmente sem corte abrupto!
            float vol0 = decks[0].volume;
            float vol1 = decks[1].volume;
            float vol2 = decks[2].volume;
            float vol3 = decks[3].volume;

            for (unsigned int f = 0; f < frames; f++) {
                blk0_l[f] *= vol0; blk0_r[f] *= vol0;
                blk1_l[f] *= vol1; blk1_r[f] *= vol1;
                blk2_l[f] *= vol2; blk2_r[f] *= vol2;
                blk3_l[f] *= vol3; blk3_r[f] *= vol3;
            }

            // Channel assignment para Pioneer Beat FX (FX 1 e FX 2) - POST-FADER SPILLOVER
            auto applyBeatFXToChannel = [&](DJBeatFXUnit& fx, int ch_id, float* l, float* r, double ch_bpm) {
                if (fx.channel_assign == ch_id) {
                    fx.processBlock(l, r, frames, ch_bpm, host_sample_rate);
                }
            };

            applyBeatFXToChannel(beat_fx, 0, blk0_l.data(), blk0_r.data(), decks[0].bpm);
            applyBeatFXToChannel(beat_fx, 1, blk1_l.data(), blk1_r.data(), decks[1].bpm);
            applyBeatFXToChannel(beat_fx, 3, blk2_l.data(), blk2_r.data(), decks[2].bpm);
            applyBeatFXToChannel(beat_fx, 4, blk3_l.data(), blk3_r.data(), decks[3].bpm);

            applyBeatFXToChannel(beat_fx2, 0, blk0_l.data(), blk0_r.data(), decks[0].bpm);
            applyBeatFXToChannel(beat_fx2, 1, blk1_l.data(), blk1_r.data(), decks[1].bpm);
            applyBeatFXToChannel(beat_fx2, 3, blk2_l.data(), blk2_r.data(), decks[2].bpm);
            applyBeatFXToChannel(beat_fx2, 4, blk3_l.data(), blk3_r.data(), decks[3].bpm);

            // Crossfader e Soma de 4 Decks (Lado A = Deck 1 e 3; Lado B = Deck 2 e 4)
            float gain_a = 1.0f;
            float gain_b = 1.0f;
            if (crossfader_curve == 1) { // 1 = SHARP / SCRATCH
                gain_a = (crossfader >= 0.92f) ? (1.0f - crossfader) / 0.08f : 1.0f;
                gain_b = (crossfader <= 0.08f) ? crossfader / 0.08f : 1.0f;
            } else if (crossfader_curve == 2) { // 2 = LINEAR
                gain_a = 1.0f - crossfader;
                gain_b = crossfader;
            } else { // 0 = SMOOTH EQUAL POWER
                float cf_rad = crossfader * (float)(M_PI * 0.5);
                gain_a = std::cos(cf_rad);
                gain_b = std::sin(cf_rad);
            }

            std::vector<float> master_l(frames, 0.0f);
            std::vector<float> master_r(frames, 0.0f);

            for (unsigned int i = 0; i < frames; i++) {
                float side_a_l = blk0_l[i] + blk2_l[i];
                float side_a_r = blk0_r[i] + blk2_r[i];
                float side_b_l = blk1_l[i] + blk3_l[i];
                float side_b_r = blk1_r[i] + blk3_r[i];'''

if old_routing in content:
    content = content.replace(old_routing, new_routing)

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content)

print('Successfully patched src/core/DJEngine.h')

