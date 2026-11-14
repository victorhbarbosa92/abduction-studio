#pragma once
#include <string>

struct AudioEvent {
    int channel_id;
    std::string plugin_node_id; // Identifica o nó DAG
    int param_index;
    float target_value;
    unsigned int frames_to_lerp; // Quantos frames levará para ir do valor atual até o target_value
    unsigned int sample_offset; // O offset do sample atual no bloco de áudio onde este evento ocorreu
    
    // Suporte a Eventos MIDI (Fase 14)
    bool is_midi = false;
    int midi_pitch = 0;
    bool midi_is_on = false;
    float midi_velocity = 0.0f;
};
