#pragma once
#include <GLFW/glfw3.h>
#include "imgui.h"
#include <vector>
#include <cmath>

namespace KuroUI {
    struct KeySpriteTextures {
        GLuint white_key_tex = 0;
        GLuint black_key_tex = 0;
        GLuint white_pressed_tex = 0;
        GLuint black_pressed_tex = 0;
        bool is_loaded = false;

        void Init() {
            if (is_loaded) return;

            int w = 128, h = 64;

            // 1. White Key Texture Sprite (Ivory White with 3D Bevel & Gloss Profile)
            std::vector<unsigned char> white_pixels(w * h * 4);
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = (y * w + x) * 4;
                    float ny = (float)y / h;

                    // Ivory White gradient cap (#f4f6f0)
                    unsigned char r = (unsigned char)(240 + ny * 10);
                    unsigned char g = (unsigned char)(245 + ny * 8);
                    unsigned char b = (unsigned char)(238 + ny * 10);
                    unsigned char a = 255;

                    // Glossy Top Bevel Highlight
                    if (y < 3) { r = 255; g = 255; b = 255; }
                    // Dark Bottom Edge Separator
                    if (y >= h - 2) { r = 65; g = 80; b = 75; }
                    // Left/Right Bevel Border
                    if (x < 2 || x >= w - 2) { r = 75; g = 90; b = 85; }

                    white_pixels[idx + 0] = r;
                    white_pixels[idx + 1] = g;
                    white_pixels[idx + 2] = b;
                    white_pixels[idx + 3] = a;
                }
            }

            glGenTextures(1, &white_key_tex);
            glBindTexture(GL_TEXTURE_2D, white_key_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixels.data());

            // 2. Black Key Texture Sprite (Matte Obsidian Black with 3D Bevel & Drop Shadow)
            std::vector<unsigned char> black_pixels(w * h * 4);
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = (y * w + x) * 4;
                    float ny = (float)y / h;

                    // Obsidian Black gradient (#14181c)
                    unsigned char r = (unsigned char)(18 + ny * 12);
                    unsigned char g = (unsigned char)(22 + ny * 14);
                    unsigned char b = (unsigned char)(28 + ny * 16);
                    unsigned char a = 255;

                    // Top Bevel Edge Highlight
                    if (y < 2) { r = 75; g = 95; b = 115; }
                    // Side Highlight Edge
                    if (x < 2 || x >= w - 2) { r = 45; g = 60; b = 75; }

                    black_pixels[idx + 0] = r;
                    black_pixels[idx + 1] = g;
                    black_pixels[idx + 2] = b;
                    black_pixels[idx + 3] = a;
                }
            }

            glGenTextures(1, &black_key_tex);
            glBindTexture(GL_TEXTURE_2D, black_key_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, black_pixels.data());

            // 3. White Key Pressed Texture Sprite (Neon Lime Green #39ff14)
            std::vector<unsigned char> white_pressed_pixels(w * h * 4);
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = (y * w + x) * 4;
                    white_pressed_pixels[idx + 0] = 57;
                    white_pressed_pixels[idx + 1] = 255;
                    white_pressed_pixels[idx + 2] = 20;
                    white_pressed_pixels[idx + 3] = 255;
                }
            }
            glGenTextures(1, &white_pressed_tex);
            glBindTexture(GL_TEXTURE_2D, white_pressed_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pressed_pixels.data());

            // 4. Black Key Pressed Texture Sprite (Neon Cyan #00e5ff)
            std::vector<unsigned char> black_pressed_pixels(w * h * 4);
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = (y * w + x) * 4;
                    black_pressed_pixels[idx + 0] = 0;
                    black_pressed_pixels[idx + 1] = 229;
                    black_pressed_pixels[idx + 2] = 255;
                    black_pressed_pixels[idx + 3] = 255;
                }
            }
            glGenTextures(1, &black_pressed_tex);
            glBindTexture(GL_TEXTURE_2D, black_pressed_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, black_pressed_pixels.data());

            // 5. Neon Note Cyan Texture Sprite (#00E5FF Glass Pill)
            std::vector<unsigned char> note_cyan_pixels(w * h * 4);
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = (y * w + x) * 4;
                    float ny = (float)y / h;
                    unsigned char r = (unsigned char)(0 + ny * 30);
                    unsigned char g = (unsigned char)(210 + ny * 45);
                    unsigned char b = 255;
                    unsigned char a = 230;
                    if (y < 4) { r = 255; g = 255; b = 255; a = 255; } // Top Glass Highlight
                    if (x < 2 || x >= w - 2 || y >= h - 2) { r = 255; g = 255; b = 255; a = 240; }
                    note_cyan_pixels[idx + 0] = r;
                    note_cyan_pixels[idx + 1] = g;
                    note_cyan_pixels[idx + 2] = b;
                    note_cyan_pixels[idx + 3] = a;
                }
            }
            glGenTextures(1, &neon_note_cyan_tex);
            glBindTexture(GL_TEXTURE_2D, neon_note_cyan_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, note_cyan_pixels.data());

            // 6. Neon Note Purple Texture Sprite (#B000FF Glass Pill)
            std::vector<unsigned char> note_purple_pixels(w * h * 4);
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = (y * w + x) * 4;
                    float ny = (float)y / h;
                    unsigned char r = (unsigned char)(160 + ny * 50);
                    unsigned char g = (unsigned char)(0 + ny * 20);
                    unsigned char b = 255;
                    unsigned char a = 230;
                    if (y < 4) { r = 255; g = 255; b = 255; a = 255; }
                    if (x < 2 || x >= w - 2 || y >= h - 2) { r = 255; g = 255; b = 255; a = 240; }
                    note_purple_pixels[idx + 0] = r;
                    note_purple_pixels[idx + 1] = g;
                    note_purple_pixels[idx + 2] = b;
                    note_purple_pixels[idx + 3] = a;
                }
            }
            glGenTextures(1, &neon_note_purple_tex);
            glBindTexture(GL_TEXTURE_2D, neon_note_purple_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, note_purple_pixels.data());

            is_loaded = true;
        }

        GLuint neon_note_cyan_tex = 0;
        GLuint neon_note_purple_tex = 0;
        GLuint knob_aluminum_tex = 0;
        GLuint fader_cap_tex = 0;
        GLuint header_glass_tex = 0;
    };

    inline KeySpriteTextures g_piano_sprites;
}
