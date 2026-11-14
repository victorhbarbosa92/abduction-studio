#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "../core/DJEngine.h"
#include "DJLayoutMetrics.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif
#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#endif
#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

#ifndef APIENTRY
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

namespace KuroUI {

    typedef char GLchar;
    typedef ptrdiff_t GLsizeiptr;
    typedef ptrdiff_t GLintptr;

    typedef GLuint (APIENTRY *PFN_glCreateShader)(GLenum type);
    typedef void   (APIENTRY *PFN_glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
    typedef void   (APIENTRY *PFN_glCompileShader)(GLuint shader);
    typedef void   (APIENTRY *PFN_glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
    typedef void   (APIENTRY *PFN_glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    typedef GLuint (APIENTRY *PFN_glCreateProgram)(void);
    typedef void   (APIENTRY *PFN_glAttachShader)(GLuint program, GLuint shader);
    typedef void   (APIENTRY *PFN_glLinkProgram)(GLuint program);
    typedef void   (APIENTRY *PFN_glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
    typedef void   (APIENTRY *PFN_glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    typedef void   (APIENTRY *PFN_glUseProgram)(GLuint program);
    typedef void   (APIENTRY *PFN_glDeleteShader)(GLuint shader);
    typedef void   (APIENTRY *PFN_glDeleteProgram)(GLuint program);
    typedef GLint  (APIENTRY *PFN_glGetUniformLocation)(GLuint program, const GLchar *name);
    typedef void   (APIENTRY *PFN_glUniform1i)(GLint location, GLint v0);
    typedef void   (APIENTRY *PFN_glUniform1f)(GLint location, GLfloat v0);
    typedef void   (APIENTRY *PFN_glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
    typedef void   (APIENTRY *PFN_glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    typedef void   (APIENTRY *PFN_glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

    typedef void   (APIENTRY *PFN_glGenBuffers)(GLsizei n, GLuint *buffers);
    typedef void   (APIENTRY *PFN_glBindBuffer)(GLenum target, GLuint buffer);
    typedef void   (APIENTRY *PFN_glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    typedef void   (APIENTRY *PFN_glDeleteBuffers)(GLsizei n, const GLuint *buffers);

    typedef void   (APIENTRY *PFN_glGenVertexArrays)(GLsizei n, GLuint *arrays);
    typedef void   (APIENTRY *PFN_glBindVertexArray)(GLuint array);
    typedef void   (APIENTRY *PFN_glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
    typedef void   (APIENTRY *PFN_glEnableVertexAttribArray)(GLuint index);
    typedef void   (APIENTRY *PFN_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

    typedef void   (APIENTRY *PFN_glActiveTexture)(GLenum texture);
    typedef void   (APIENTRY *PFN_glGenFramebuffers)(GLsizei n, GLuint *framebuffers);
    typedef void   (APIENTRY *PFN_glBindFramebuffer)(GLenum target, GLuint framebuffer);
    typedef void   (APIENTRY *PFN_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    typedef GLenum (APIENTRY *PFN_glCheckFramebufferStatus)(GLenum target);
    typedef void   (APIENTRY *PFN_glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);

    struct WaveGLProcs {
        PFN_glCreateShader          glCreateShader = nullptr;
        PFN_glShaderSource          glShaderSource = nullptr;
        PFN_glCompileShader         glCompileShader = nullptr;
        PFN_glGetShaderiv           glGetShaderiv = nullptr;
        PFN_glGetShaderInfoLog      glGetShaderInfoLog = nullptr;
        PFN_glCreateProgram         glCreateProgram = nullptr;
        PFN_glAttachShader          glAttachShader = nullptr;
        PFN_glLinkProgram           glLinkProgram = nullptr;
        PFN_glGetProgramiv          glGetProgramiv = nullptr;
        PFN_glGetProgramInfoLog     glGetProgramInfoLog = nullptr;
        PFN_glUseProgram            glUseProgram = nullptr;
        PFN_glDeleteShader          glDeleteShader = nullptr;
        PFN_glDeleteProgram         glDeleteProgram = nullptr;
        PFN_glGetUniformLocation    glGetUniformLocation = nullptr;
        PFN_glUniform1i             glUniform1i = nullptr;
        PFN_glUniform1f             glUniform1f = nullptr;
        PFN_glUniform2f             glUniform2f = nullptr;
        PFN_glUniform3f             glUniform3f = nullptr;
        PFN_glUniform4f             glUniform4f = nullptr;

        PFN_glGenBuffers            glGenBuffers = nullptr;
        PFN_glBindBuffer            glBindBuffer = nullptr;
        PFN_glBufferData            glBufferData = nullptr;
        PFN_glDeleteBuffers         glDeleteBuffers = nullptr;

        PFN_glGenVertexArrays       glGenVertexArrays = nullptr;
        PFN_glBindVertexArray       glBindVertexArray = nullptr;
        PFN_glDeleteVertexArrays    glDeleteVertexArrays = nullptr;
        PFN_glEnableVertexAttribArray glEnableVertexAttribArray = nullptr;
        PFN_glVertexAttribPointer   glVertexAttribPointer = nullptr;

        PFN_glActiveTexture         glActiveTexture = nullptr;
        PFN_glGenFramebuffers       glGenFramebuffers = nullptr;
        PFN_glBindFramebuffer       glBindFramebuffer = nullptr;
        PFN_glFramebufferTexture2D  glFramebufferTexture2D = nullptr;
        PFN_glCheckFramebufferStatus glCheckFramebufferStatus = nullptr;
        PFN_glDeleteFramebuffers    glDeleteFramebuffers = nullptr;

        bool init() {
            #define LOAD_WAVE_PROC(fn) fn = (PFN_##fn)glfwGetProcAddress(#fn); if (!fn) return false;
            LOAD_WAVE_PROC(glCreateShader);
            LOAD_WAVE_PROC(glShaderSource);
            LOAD_WAVE_PROC(glCompileShader);
            LOAD_WAVE_PROC(glGetShaderiv);
            LOAD_WAVE_PROC(glGetShaderInfoLog);
            LOAD_WAVE_PROC(glCreateProgram);
            LOAD_WAVE_PROC(glAttachShader);
            LOAD_WAVE_PROC(glLinkProgram);
            LOAD_WAVE_PROC(glGetProgramiv);
            LOAD_WAVE_PROC(glGetProgramInfoLog);
            LOAD_WAVE_PROC(glUseProgram);
            LOAD_WAVE_PROC(glDeleteShader);
            LOAD_WAVE_PROC(glDeleteProgram);
            LOAD_WAVE_PROC(glGetUniformLocation);
            LOAD_WAVE_PROC(glUniform1i);
            LOAD_WAVE_PROC(glUniform1f);
            LOAD_WAVE_PROC(glUniform2f);
            LOAD_WAVE_PROC(glUniform3f);
            LOAD_WAVE_PROC(glUniform4f);
            LOAD_WAVE_PROC(glGenBuffers);
            LOAD_WAVE_PROC(glBindBuffer);
            LOAD_WAVE_PROC(glBufferData);
            LOAD_WAVE_PROC(glDeleteBuffers);
            LOAD_WAVE_PROC(glGenVertexArrays);
            LOAD_WAVE_PROC(glBindVertexArray);
            LOAD_WAVE_PROC(glDeleteVertexArrays);
            LOAD_WAVE_PROC(glEnableVertexAttribArray);
            LOAD_WAVE_PROC(glVertexAttribPointer);
            LOAD_WAVE_PROC(glActiveTexture);
            LOAD_WAVE_PROC(glGenFramebuffers);
            LOAD_WAVE_PROC(glBindFramebuffer);
            LOAD_WAVE_PROC(glFramebufferTexture2D);
            LOAD_WAVE_PROC(glCheckFramebufferStatus);
            LOAD_WAVE_PROC(glDeleteFramebuffers);
            #undef LOAD_WAVE_PROC
            return true;
        }
    };

    class KuroGPUWaveformEngine {
    private:
        bool m_initialized = false;
        bool m_gl_supported = false;
        WaveGLProcs m_gl;

        GLuint m_program = 0;
        GLuint m_vao = 0;
        GLuint m_vbo = 0;

        // Uniforms
        GLint m_u_wave_tex = -1;
        GLint m_u_tex_width = -1;
        GLint m_u_tex_height = -1;
        GLint m_u_start_bin = -1;
        GLint m_u_end_bin = -1;
        GLint m_u_total_bins = -1;
        GLint m_u_color_mode = -1;
        GLint m_u_engine_mode = -1;
        GLint m_u_vertical_side = -1;
        GLint m_u_dynamic_contrast = -1;
        GLint m_u_transient_punch = -1;
        GLint m_u_core_brightness = -1;
        GLint m_u_line_style = -1;
        GLint m_u_line_spacing = -1;
        GLint m_u_is_drop = -1;
        GLint m_u_resolution = -1;

        struct TrackTex {
            GLuint texture_id = 0;
            int width = 0;
            int height = 0;
            size_t num_bins = 0;
            std::string path = "";
            float stretch = 0.0f;
            float decay = 0.0f;
        };
        TrackTex m_track_textures[4];

        struct FBOState {
            GLuint fbo = 0;
            GLuint texture_id = 0;
            int width = 0;
            int height = 0;
        };
        FBOState m_deck_fbos[4];
        FBOState m_overlaid_fbo;

        KuroGPUWaveformEngine() = default;

    public:
        ~KuroGPUWaveformEngine() {
            cleanup();
        }

        static KuroGPUWaveformEngine& getInstance() {
            static KuroGPUWaveformEngine instance;
            return instance;
        }

        bool isSupported() const { return m_gl_supported; }

        void cleanup() {
            if (!m_initialized) return;

            for (auto& tt : m_track_textures) {
                if (tt.texture_id != 0) {
                    glDeleteTextures(1, &tt.texture_id);
                    tt.texture_id = 0;
                }
            }

            for (auto& fb : m_deck_fbos) {
                if (fb.texture_id != 0) glDeleteTextures(1, &fb.texture_id);
                if (fb.fbo != 0 && m_gl.glDeleteFramebuffers) m_gl.glDeleteFramebuffers(1, &fb.fbo);
                fb.texture_id = 0;
                fb.fbo = 0;
            }

            if (m_overlaid_fbo.texture_id != 0) glDeleteTextures(1, &m_overlaid_fbo.texture_id);
            if (m_overlaid_fbo.fbo != 0 && m_gl.glDeleteFramebuffers) m_gl.glDeleteFramebuffers(1, &m_overlaid_fbo.fbo);
            m_overlaid_fbo.texture_id = 0;
            m_overlaid_fbo.fbo = 0;

            if (m_vbo != 0 && m_gl.glDeleteBuffers) m_gl.glDeleteBuffers(1, &m_vbo);
            if (m_vao != 0 && m_gl.glDeleteVertexArrays) m_gl.glDeleteVertexArrays(1, &m_vao);
            if (m_program != 0 && m_gl.glDeleteProgram) m_gl.glDeleteProgram(m_program);

            m_vbo = 0;
            m_vao = 0;
            m_program = 0;
            m_initialized = false;
            m_gl_supported = false;
        }

        bool init() {
            if (m_initialized) return m_gl_supported;
            m_initialized = true;

            if (!m_gl.init()) {
                std::cerr << "[KuroGPUWaveformEngine] ERRO: Falha ao carregar ponteiros OpenGL via GLFW.\n";
                m_gl_supported = false;
                return false;
            }

            // 1. Compilar Vertex Shader
            static const char* kVertexShaderSource = R"(#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
)";

            // 2. Compilar Fragment Shader (Motor CDJ-3000 / Rekordbox 7 Oficial)
            static const char* kFragmentShaderSource = R"(#version 330 core
in vec2 v_uv;
out vec4 FragColor;

uniform sampler2D u_wave_tex;
uniform float     u_tex_width;
uniform float     u_tex_height;
uniform float     u_start_bin;
uniform float     u_end_bin;
uniform float     u_total_bins;

uniform int       u_color_mode;      // 0=RGB, 1=3-Band CDJ-3000, 2=Blue CDJ-2000, 3=Neon, 4=Sunset
uniform int       u_engine_mode;     // 0=Multi-Lane (3 sub-lanes HI/MID/LOW), 1=Fused (Pioneer single lane)
uniform int       u_vertical_side;   // 0=Bipolar (both sides), 1=Upper half only, -1=Lower half only
uniform float     u_dynamic_contrast;// e.g. 1.25
uniform float     u_transient_punch; // e.g. 1.50
uniform float     u_core_brightness; // e.g. 0.85
uniform int       u_line_style;      // 0=Linhas Rekordbox 7, 1=Barras com respiro, 2=Malha Continua
uniform float     u_line_spacing;    // e.g. 2.0px
uniform int       u_is_drop;         // 1=Drop active (flares bass band in hot crimson)
uniform vec2      u_resolution;      // Viewport resolution (pixels)

// Curva decibel perceptual Rekordbox / Pioneer WeDJ (-48 dB de piso)
float to_pioneer_db(float v) {
    if (v <= 0.0001) return 0.0;
    float db = 20.0 * log(v) * 0.4342944819; // ln para log10
    if (db < -48.0) return 0.0;
    float norm = (db + 48.0) / 48.0;
    return pow(clamp(norm, 0.0, 1.0), u_dynamic_contrast);
}

// Amostragem subpixel contínua por interpolação Hermite anti-jitter
vec4 fetchWavePoint(float bin) {
    if (bin < 0.0 || bin >= u_total_bins || u_total_bins <= 0.0) {
        return vec4(0.0);
    }
    float b0 = floor(bin);
    float b1 = min(b0 + 1.0, u_total_bins - 1.0);
    float fr = fract(bin);
    fr = fr * fr * (3.0 - 2.0 * fr); // Hermite smoothing anti-jitter

    float row0 = floor(b0 / u_tex_width);
    float col0 = mod(b0, u_tex_width);
    vec2 uv0 = vec2((col0 + 0.5) / u_tex_width, (row0 + 0.5) / u_tex_height);

    float row1 = floor(b1 / u_tex_width);
    float col1 = mod(b1, u_tex_width);
    vec2 uv1 = vec2((col1 + 0.5) / u_tex_width, (row1 + 0.5) / u_tex_height);

    vec4 p0 = texture(u_wave_tex, uv0);
    vec4 p1 = texture(u_wave_tex, uv1);

    return mix(p0, p1, fr);
}

// Suavização anti-aliasing subpixel de bordas
float edge_aa(float val, float threshold, float px) {
    return 1.0 - smoothstep(threshold - px, threshold + px, val);
}

void main() {
    float bin = mix(u_start_bin, u_end_bin, v_uv.x);
    vec4 wave = fetchWavePoint(bin);

    // Amplitudes físicas com curva Pioneer dB e transient punch
    float p_low  = to_pioneer_db(wave.r);
    float p_mid  = to_pioneer_db(wave.g);
    float p_high = to_pioneer_db(wave.b);
    float p_peak = to_pioneer_db(wave.a);

    // Fundo Obsidian Club Standard
    vec3 bg_color = mix(vec3(0.035, 0.045, 0.065), vec3(0.050, 0.065, 0.090), v_uv.y);

    // Fora do áudio (silêncio / lead-in / lead-out):
    bool is_silent = (bin < 0.0 || bin >= u_total_bins || wave.a <= 0.005);

    // Modulação de estriação Rekordbox 7
    float line_mod = 1.0;
    if (u_line_style == 0) { // Linhas nítidas Rekordbox 7
        float sp = max(1.0, u_line_spacing);
        float xm = mod(gl_FragCoord.x, sp);
        float l_val = smoothstep(0.0, 0.35, xm) * (1.0 - smoothstep(sp * 0.65, sp, xm));
        line_mod = mix(0.85, 1.0, l_val);
    } else if (u_line_style == 1) { // Barras com respiro
        float sp = max(1.5, u_line_spacing);
        float xm = mod(gl_FragCoord.x, sp);
        line_mod = (xm < sp * 0.70) ? 1.0 : 0.40;
    }

    // =========================================================================
    // MODO 0: 3-BAND MULTI-LANE (3 BAIAS INDEPENDENTES HI / MID / LOW)
    // =========================================================================
    if (u_engine_mode == 0) {
        float h_center = 0.14;
        float m_center = 0.46;
        float l_center = 0.82;

        float h_half = 0.14 * 0.86;
        float m_half = 0.18 * 0.86;
        float l_half = 0.18 * 0.86;

        float py = v_uv.y;
        float px_v = 1.0 / u_resolution.y;

        // Divisórias sutis
        if (abs(py - 0.28) < px_v * 0.8 || abs(py - 0.64) < px_v * 0.8) {
            FragColor = vec4(vec3(0.12, 0.16, 0.22), 1.0);
            return;
        }

        // Linhas de centro neutras
        if (abs(py - h_center) < px_v * 0.6 || abs(py - m_center) < px_v * 0.6 || abs(py - l_center) < px_v * 0.6) {
            bg_color = mix(bg_color, vec3(0.18, 0.24, 0.32), 0.75);
        }

        if (is_silent) {
            // Traço lead-in suave de osciloscópio
            if (abs(py - h_center) < px_v * 1.0 || abs(py - m_center) < px_v * 1.0 || abs(py - l_center) < px_v * 1.0) {
                bg_color = mix(bg_color, vec3(0.24, 0.32, 0.45), 0.85);
            }
            if (mod(gl_FragCoord.x, 32.0) < 1.5 && (abs(py - h_center) < 0.025 || abs(py - m_center) < 0.025 || abs(py - l_center) < 0.025)) {
                bg_color += vec3(0.08, 0.12, 0.18);
            }
            FragColor = vec4(bg_color, 1.0);
            return;
        }

        vec3 wave_col = bg_color;
        float wave_alpha = 0.0;

        if (py < 0.28) {
            // BAIAS DE AGUDOS (HI)
            float dist = abs(py - h_center);
            float amp = clamp(p_high * u_transient_punch * 1.15, 0.0, 1.0) * h_half;
            if (wave.b > 0.005) amp = max(amp, px_v * 2.0);
            float m = edge_aa(dist, amp, px_v);

            vec3 c_outer = vec3(0.71, 0.94, 1.00);
            vec3 c_core  = vec3(1.0, 1.0, 1.0);
            if (u_color_mode == 0)      c_outer = vec3(0.00, 0.90, 1.00); // RGB
            else if (u_color_mode == 1) c_outer = vec3(0.71, 0.94, 1.00); // 3-Band
            else if (u_color_mode == 2) c_outer = vec3(0.78, 0.92, 1.00); // Blue
            else if (u_color_mode == 3) c_outer = vec3(0.00, 0.96, 1.00); // Neon
            else if (u_color_mode == 4) c_outer = vec3(1.00, 0.92, 0.24); // Sunset
            else if (u_color_mode == 5) c_outer = vec3(0.55, 1.00, 0.78); // Emerald
            else if (u_color_mode == 6) c_outer = vec3(0.20, 0.94, 1.00); // Vaporwave
            else if (u_color_mode == 7) c_outer = vec3(1.00, 0.90, 0.27); // Inferno
            else if (u_color_mode == 8) c_outer = vec3(0.84, 0.98, 1.00); // Glacier
            else                        c_outer = vec3(1.00, 1.00, 1.00); // Monochrome
            float core_t = 1.0 - smoothstep(0.0, amp * 0.55, dist);
            wave_col = mix(c_outer, c_core, core_t * u_core_brightness);
            wave_alpha = m;
        } else if (py < 0.64) {
            // BAIAS DE MÉDIOS (MID)
            float dist = abs(py - m_center);
            float amp = clamp(p_mid * u_transient_punch * 0.95, 0.0, 1.0) * m_half;
            if (wave.g > 0.005) amp = max(amp, px_v * 2.0);
            float m = edge_aa(dist, amp, px_v);

            vec3 c_outer = vec3(0.92, 0.53, 0.06);
            vec3 c_core  = vec3(1.0, 0.85, 0.35);
            if (u_color_mode == 0)      c_outer = vec3(1.00, 0.65, 0.08); // RGB
            else if (u_color_mode == 1) c_outer = vec3(0.92, 0.53, 0.06); // 3-Band
            else if (u_color_mode == 2) c_outer = vec3(0.08, 0.58, 1.00); // Blue
            else if (u_color_mode == 3) c_outer = vec3(0.00, 1.00, 0.47); // Neon
            else if (u_color_mode == 4) c_outer = vec3(1.00, 0.53, 0.04); // Sunset
            else if (u_color_mode == 5) c_outer = vec3(0.16, 0.92, 0.27); // Emerald
            else if (u_color_mode == 6) c_outer = vec3(1.00, 0.10, 0.71); // Vaporwave
            else if (u_color_mode == 7) c_outer = vec3(1.00, 0.39, 0.00); // Inferno
            else if (u_color_mode == 8) c_outer = vec3(0.00, 0.73, 0.92); // Glacier
            else                        c_outer = vec3(0.61, 0.65, 0.71); // Monochrome
            float core_t = 1.0 - smoothstep(0.0, amp * 0.55, dist);
            wave_col = mix(c_outer, c_core, core_t * u_core_brightness);
            wave_alpha = m;
        } else {
            // BAIAS DE GRAVES (LOW)
            float dist = abs(py - l_center);
            float amp = clamp(p_low * u_transient_punch * 0.95, 0.0, 1.0) * l_half;
            if (wave.r > 0.005) amp = max(amp, px_v * 2.0);
            float m = edge_aa(dist, amp, px_v);

            vec3 c_outer = vec3(0.00, 0.33, 0.96);
            vec3 c_core  = vec3(0.18, 0.68, 1.0);
            if (u_is_drop == 1) {
                c_outer = vec3(1.0, 0.10, 0.28);
                c_core  = vec3(1.0, 0.55, 0.75);
            } else if (u_color_mode == 0) {
                c_outer = vec3(1.00, 0.00, 0.55);
                c_core  = vec3(1.00, 0.45, 0.80);
            } else if (u_color_mode == 1) {
                c_outer = vec3(0.00, 0.33, 0.96);
                c_core  = vec3(0.18, 0.68, 1.00);
            } else if (u_color_mode == 2) {
                c_outer = vec3(0.00, 0.25, 0.80);
                c_core  = vec3(0.30, 0.60, 1.00);
            } else if (u_color_mode == 3) {
                c_outer = vec3(0.86, 0.00, 1.00);
                c_core  = vec3(1.00, 0.45, 1.00);
            } else if (u_color_mode == 4) {
                c_outer = vec3(0.92, 0.10, 0.16);
                c_core  = vec3(1.00, 0.45, 0.20);
            } else if (u_color_mode == 5) {
                c_outer = vec3(0.00, 0.45, 0.16);
                c_core  = vec3(0.15, 0.85, 0.35);
            } else if (u_color_mode == 6) {
                c_outer = vec3(0.48, 0.05, 0.78);
                c_core  = vec3(0.80, 0.25, 1.00);
            } else if (u_color_mode == 7) {
                c_outer = vec3(0.86, 0.04, 0.04);
                c_core  = vec3(1.00, 0.25, 0.05);
            } else if (u_color_mode == 8) {
                c_outer = vec3(0.04, 0.20, 0.63);
                c_core  = vec3(0.20, 0.65, 0.95);
            } else {
                c_outer = vec3(0.22, 0.24, 0.27);
                c_core  = vec3(0.55, 0.58, 0.65);
            }
            float core_t = 1.0 - smoothstep(0.0, amp * 0.55, dist);
            wave_col = mix(c_outer, c_core, core_t * u_core_brightness);
            wave_alpha = m;
        }

        wave_alpha *= line_mod;
        vec3 final_color = mix(bg_color, wave_col, wave_alpha);
        FragColor = vec4(final_color, 1.0);
        return;
    }

    // =========================================================================
    // MODO 1: FUSED / PIONEER CDJ-3000 & REKORDBOX 7 (EIXO ÚNICO CENTRAL)
    // =========================================================================
    float y_center = 0.5;
    float dist = 0.0;
    float max_headroom = 0.82;

    if (u_vertical_side == 0) {
        dist = abs(v_uv.y - y_center) * 2.0;
    } else if (u_vertical_side == 1) {
        if (v_uv.y > 0.5) {
            FragColor = vec4(bg_color, 1.0);
            return;
        }
        dist = (0.5 - v_uv.y) * 2.0;
    } else {
        if (v_uv.y < 0.5) {
            FragColor = vec4(bg_color, 1.0);
            return;
        }
        dist = (v_uv.y - 0.5) * 2.0;
    }

    float px_v = fwidth(dist);
    if (px_v <= 0.0001) px_v = 1.0 / (u_resolution.y * 0.5);

    // Linha de centro neutra
    float center_line = 1.0 - smoothstep(0.0, px_v * 1.5, dist);
    bg_color = mix(bg_color, vec3(0.16, 0.22, 0.30), center_line * 0.85);

    if (is_silent) {
        float lead_line = 1.0 - smoothstep(0.0, px_v * 2.0, dist);
        bg_color = mix(bg_color, vec3(0.22, 0.30, 0.42), lead_line * 0.80);
        if (mod(gl_FragCoord.x, 32.0) < 1.5 && dist < 0.12) {
            bg_color += vec3(0.09, 0.13, 0.19);
        }
        FragColor = vec4(bg_color, 1.0);
        return;
    }

    // Amplitudes de cada banda (Proporções Áureas Pioneer CDJ-3000 / Rekordbox 7)
    // Low: Base estrutural sólida
    // Mid: Corpo harmônico musical
    // High: Agulhas de transientes e miolo incandescente
    float amp_l = clamp(p_low  * u_transient_punch * 1.00, 0.0, 1.0) * max_headroom;
    float amp_m = clamp(p_mid  * u_transient_punch * 0.80, 0.0, 1.0) * max_headroom * 0.78;
    float amp_h = clamp(p_high * u_transient_punch * 0.65, 0.0, 1.0) * max_headroom * 0.52;

    // Transientes de pratos e caixas (Needle Spikes) furam como agulhas brancas
    if (p_high > 0.35) {
        amp_h = max(amp_h, p_high * max_headroom * 0.72);
    }

    if (wave.a > 0.005) {
        amp_l = max(amp_l, px_v * 2.5);
    }

    float mask_l = edge_aa(dist, amp_l, px_v);
    float mask_m = edge_aa(dist, amp_m, px_v);
    float mask_h = edge_aa(dist, amp_h, px_v);

    vec3 wave_col = vec3(0.0);
    float total_alpha = 0.0;

    if (u_color_mode == 1) {
        // REKORDBOX 3-BAND OFICIAL (CDJ-3000 / REKORDBOX 7)
        vec3 col_l_out = (u_is_drop == 1) ? vec3(1.0, 0.10, 0.28) : vec3(0.00, 0.33, 0.96);
        vec3 col_l_in  = (u_is_drop == 1) ? vec3(1.0, 0.55, 0.75) : vec3(0.18, 0.68, 1.00);

        vec3 col_m_out = vec3(0.92, 0.53, 0.06);
        vec3 col_m_in  = vec3(1.00, 0.76, 0.18);

        vec3 col_h_out = vec3(0.71, 0.94, 1.00);
        vec3 col_h_in  = vec3(1.00, 1.00, 1.00);

        float core_l = 1.0 - smoothstep(0.0, amp_l * 0.55, dist);
        float core_m = 1.0 - smoothstep(0.0, amp_m * 0.55, dist);
        float core_h = 1.0 - smoothstep(0.0, amp_h * 0.55, dist);

        vec3 cl = mix(col_l_out, col_l_in, core_l * u_core_brightness);
        vec3 cm = mix(col_m_out, col_m_in, core_m * u_core_brightness);
        vec3 ch = mix(col_h_out, col_h_in, core_h * u_core_brightness);

        wave_col = cl;
        wave_col = mix(wave_col, cm, mask_m * 0.96);
        wave_col = mix(wave_col, ch, mask_h * 0.98);

        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else if (u_color_mode == 0) {
        // REKORDBOX & TRAKTOR RGB ESPECTRAL OFICIAL
        float max_amp = max(amp_l, max(amp_m, amp_h));
        float mask_tot = edge_aa(dist, max_amp, px_v);

        float r = p_low * 255.0 + p_mid * 235.0 + p_high * 30.0;
        float g = p_mid * 160.0 + p_high * 225.0;
        float b = p_low * 140.0 + p_high * 255.0;

        if (p_low > 0.25 && p_mid < 0.35) g *= 0.65;
        if (p_high > 0.15 && p_low > 0.18) { r = max(r, 240.0); b = max(b, 210.0); }

        float max_c = max(max(r, g), max(b, 1.0));
        float boost = 255.0 / max_c;
        r = clamp(r * boost / 255.0, 0.0, 1.0);
        g = clamp(g * boost / 255.0, 0.0, 1.0);
        b = clamp(b * boost / 255.0, 0.0, 1.0);

        vec3 rgb_col = vec3(r, g, b);
        float core_glow = (1.0 - smoothstep(0.0, max_amp * 0.50, dist)) * u_core_brightness;
        wave_col = mix(rgb_col * 0.85, vec3(1.0), core_glow * 0.55);
        total_alpha = mask_tot;

    } else if (u_color_mode == 2) {
        // BLUE CDJ-2000 NEXUS
        float max_amp = max(amp_l, max(amp_m, amp_h));
        float mask_tot = edge_aa(dist, max_amp, px_v);
        vec3 c_out = vec3(0.0, 0.25, 0.80);
        vec3 c_mid = vec3(0.08, 0.58, 1.00);
        vec3 c_core = vec3(0.85, 0.95, 1.00);

        float core_t = 1.0 - smoothstep(0.0, max_amp * 0.50, dist);
        wave_col = mix(mix(c_out, c_mid, mask_m), c_core, core_t * u_core_brightness);
        total_alpha = mask_tot;

    } else if (u_color_mode == 3) {
        // NEON CYBERPUNK
        vec3 col_l = vec3(0.86, 0.00, 1.00);
        vec3 col_m = vec3(0.00, 1.00, 0.47);
        vec3 col_h = vec3(0.00, 0.96, 1.00);
        wave_col = mix(mix(col_l, col_m, mask_m), col_h, mask_h);
        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else if (u_color_mode == 4) {
        // SUNSET SOLAR
        vec3 col_l = vec3(0.92, 0.10, 0.16);
        vec3 col_m = vec3(1.00, 0.53, 0.04);
        vec3 col_h = vec3(1.00, 0.92, 0.24);
        wave_col = mix(mix(col_l, col_m, mask_m), col_h, mask_h);
        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else if (u_color_mode == 5) {
        // EMERALD MATRIX (Acid Techno)
        vec3 col_l = (u_is_drop == 1) ? vec3(1.0, 0.10, 0.28) : vec3(0.00, 0.45, 0.16);
        vec3 col_m = vec3(0.16, 0.92, 0.27);
        vec3 col_h = vec3(0.55, 1.00, 0.78);
        float core_t = (1.0 - smoothstep(0.0, max(amp_l, max(amp_m, amp_h)) * 0.50, dist)) * u_core_brightness;
        wave_col = mix(mix(col_l, col_m, mask_m), col_h, mask_h);
        wave_col = mix(wave_col, vec3(0.85, 1.00, 0.90), core_t * 0.60);
        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else if (u_color_mode == 6) {
        // VAPORWAVE / SYNTHWAVE 80s
        vec3 col_l = (u_is_drop == 1) ? vec3(1.0, 0.10, 0.28) : vec3(0.48, 0.05, 0.78);
        vec3 col_m = vec3(1.00, 0.10, 0.71);
        vec3 col_h = vec3(0.20, 0.94, 1.00);
        float core_t = (1.0 - smoothstep(0.0, max(amp_l, max(amp_m, amp_h)) * 0.50, dist)) * u_core_brightness;
        wave_col = mix(mix(col_l, col_m, mask_m), col_h, mask_h);
        wave_col = mix(wave_col, vec3(1.00, 0.85, 1.00), core_t * 0.60);
        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else if (u_color_mode == 7) {
        // INFERNO MAGMA
        vec3 col_l = vec3(0.86, 0.04, 0.04);
        vec3 col_m = vec3(1.00, 0.39, 0.00);
        vec3 col_h = vec3(1.00, 0.90, 0.27);
        float core_t = (1.0 - smoothstep(0.0, max(amp_l, max(amp_m, amp_h)) * 0.50, dist)) * u_core_brightness;
        wave_col = mix(mix(col_l, col_m, mask_m), col_h, mask_h);
        wave_col = mix(wave_col, vec3(1.00, 0.98, 0.80), core_t * 0.70);
        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else if (u_color_mode == 8) {
        // GLACIER FROST
        vec3 col_l = (u_is_drop == 1) ? vec3(1.0, 0.10, 0.28) : vec3(0.04, 0.20, 0.63);
        vec3 col_m = vec3(0.00, 0.73, 0.92);
        vec3 col_h = vec3(0.84, 0.98, 1.00);
        float core_t = (1.0 - smoothstep(0.0, max(amp_l, max(amp_m, amp_h)) * 0.50, dist)) * u_core_brightness;
        wave_col = mix(mix(col_l, col_m, mask_m), col_h, mask_h);
        wave_col = mix(wave_col, vec3(1.00, 1.00, 1.00), core_t * 0.70);
        total_alpha = max(mask_l, max(mask_m, mask_h));

    } else {
        // MONOCHROME PLATINUM
        float max_amp = max(amp_l, max(amp_m, amp_h));
        float mask_tot = edge_aa(dist, max_amp, px_v);
        vec3 c_out = vec3(0.22, 0.24, 0.27);
        vec3 c_mid = vec3(0.61, 0.65, 0.71);
        vec3 c_core = vec3(1.00, 1.00, 1.00);
        float core_t = 1.0 - smoothstep(0.0, max_amp * 0.45, dist);
        wave_col = mix(mix(c_out, c_mid, mask_m), c_core, core_t * u_core_brightness);
        total_alpha = mask_tot;
    }

    total_alpha *= line_mod;
    vec3 final_color = mix(bg_color, wave_col, total_alpha);

    FragColor = vec4(final_color, 1.0);
}
)";

            GLuint vs = m_gl.glCreateShader(GL_VERTEX_SHADER);
            m_gl.glShaderSource(vs, 1, &kVertexShaderSource, nullptr);
            m_gl.glCompileShader(vs);

            GLint ok = 0;
            m_gl.glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                char log[1024];
                m_gl.glGetShaderInfoLog(vs, sizeof(log), nullptr, log);
                std::cerr << "[KuroGPUWaveformEngine] ERRO no Vertex Shader: " << log << "\n";
                m_gl.glDeleteShader(vs);
                m_gl_supported = false;
                return false;
            }

            GLuint fs = m_gl.glCreateShader(GL_FRAGMENT_SHADER);
            m_gl.glShaderSource(fs, 1, &kFragmentShaderSource, nullptr);
            m_gl.glCompileShader(fs);

            m_gl.glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                char log[1024];
                m_gl.glGetShaderInfoLog(fs, sizeof(log), nullptr, log);
                std::cerr << "[KuroGPUWaveformEngine] ERRO no Fragment Shader: " << log << "\n";
                m_gl.glDeleteShader(vs);
                m_gl.glDeleteShader(fs);
                m_gl_supported = false;
                return false;
            }

            m_program = m_gl.glCreateProgram();
            m_gl.glAttachShader(m_program, vs);
            m_gl.glAttachShader(m_program, fs);
            m_gl.glLinkProgram(m_program);

            m_gl.glDeleteShader(vs);
            m_gl.glDeleteShader(fs);

            m_gl.glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
            if (!ok) {
                char log[1024];
                m_gl.glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
                std::cerr << "[KuroGPUWaveformEngine] ERRO ao linkar Shader Program: " << log << "\n";
                m_gl.glDeleteProgram(m_program);
                m_program = 0;
                m_gl_supported = false;
                return false;
            }

            // Uniform locations
            m_u_wave_tex          = m_gl.glGetUniformLocation(m_program, "u_wave_tex");
            m_u_tex_width         = m_gl.glGetUniformLocation(m_program, "u_tex_width");
            m_u_tex_height        = m_gl.glGetUniformLocation(m_program, "u_tex_height");
            m_u_start_bin         = m_gl.glGetUniformLocation(m_program, "u_start_bin");
            m_u_end_bin           = m_gl.glGetUniformLocation(m_program, "u_end_bin");
            m_u_total_bins        = m_gl.glGetUniformLocation(m_program, "u_total_bins");
            m_u_color_mode        = m_gl.glGetUniformLocation(m_program, "u_color_mode");
            m_u_engine_mode       = m_gl.glGetUniformLocation(m_program, "u_engine_mode");
            m_u_vertical_side     = m_gl.glGetUniformLocation(m_program, "u_vertical_side");
            m_u_dynamic_contrast  = m_gl.glGetUniformLocation(m_program, "u_dynamic_contrast");
            m_u_transient_punch   = m_gl.glGetUniformLocation(m_program, "u_transient_punch");
            m_u_core_brightness   = m_gl.glGetUniformLocation(m_program, "u_core_brightness");
            m_u_line_style        = m_gl.glGetUniformLocation(m_program, "u_line_style");
            m_u_line_spacing      = m_gl.glGetUniformLocation(m_program, "u_line_spacing");
            m_u_is_drop           = m_gl.glGetUniformLocation(m_program, "u_is_drop");
            m_u_resolution        = m_gl.glGetUniformLocation(m_program, "u_resolution");

            // 3. Configurar Fullscreen Quad VAO / VBO
            // Note: v_uv.y = 0.0 no topo (NDC +1), v_uv.y = 1.0 na base (NDC -1)
            float quad_vertices[] = {
                // pos.x, pos.y, uv.x, uv.y
                -1.0f, -1.0f, 0.0f, 1.0f,
                 1.0f, -1.0f, 1.0f, 1.0f,
                 1.0f,  1.0f, 1.0f, 0.0f,

                -1.0f, -1.0f, 0.0f, 1.0f,
                 1.0f,  1.0f, 1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f, 0.0f
            };

            m_gl.glGenVertexArrays(1, &m_vao);
            m_gl.glBindVertexArray(m_vao);

            m_gl.glGenBuffers(1, &m_vbo);
            m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            m_gl.glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

            m_gl.glEnableVertexAttribArray(0);
            m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            m_gl.glEnableVertexAttribArray(1);
            m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            m_gl.glBindVertexArray(0);
            m_gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

            m_gl_supported = true;
            std::cout << "[KuroGPUWaveformEngine] GPU Shader Engine inicializado com sucesso (OpenGL 3.3 Core).\n";
            return true;
        }

        void updateTrackTexture(int deck_idx, const KuroAudio::DJDeck& deck) {
            if (deck_idx < 0 || deck_idx >= 4) return;
            TrackTex& tt = m_track_textures[deck_idx];

            size_t num_bins = deck.detailed_waveform.size();
            if (num_bins == 0) {
                if (tt.texture_id != 0) {
                    glDeleteTextures(1, &tt.texture_id);
                    tt.texture_id = 0;
                    tt.num_bins = 0;
                    tt.path = "";
                }
                return;
            }

            if (tt.texture_id != 0 &&
                tt.path == deck.track_path &&
                tt.num_bins == num_bins &&
                std::abs(tt.stretch - deck.waveform_stretch) < 0.01f &&
                std::abs(tt.decay - deck.waveform_envelope_decay) < 0.01f) {
                return; // Em dia
            }

            int tex_w = 2048;
            int tex_h = (int)((num_bins + tex_w - 1) / tex_w);
            std::vector<uint8_t> data((size_t)tex_w * (size_t)tex_h * 4, 0);

            for (size_t i = 0; i < num_bins; i++) {
                const auto& pt = deck.detailed_waveform[i];
                size_t off = i * 4;
                data[off + 0] = (uint8_t)std::clamp(pt.low  * 255.0f, 0.0f, 255.0f);
                data[off + 1] = (uint8_t)std::clamp(pt.mid  * 255.0f, 0.0f, 255.0f);
                data[off + 2] = (uint8_t)std::clamp(pt.high * 255.0f, 0.0f, 255.0f);
                data[off + 3] = (uint8_t)std::clamp(pt.peak * 255.0f, 0.0f, 255.0f);
            }

            if (tt.texture_id == 0) {
                glGenTextures(1, &tt.texture_id);
            }

            glBindTexture(GL_TEXTURE_2D, tt.texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
            glBindTexture(GL_TEXTURE_2D, 0);

            tt.width = tex_w;
            tt.height = tex_h;
            tt.num_bins = num_bins;
            tt.path = deck.track_path;
            tt.stretch = deck.waveform_stretch;
            tt.decay = deck.waveform_envelope_decay;
        }

        bool ensureFBO(FBOState& fbo, int width, int height) {
            if (width <= 0 || height <= 0) return false;
            if (fbo.fbo != 0 && fbo.width == width && fbo.height == height && fbo.texture_id != 0) {
                return true;
            }

            if (fbo.texture_id != 0) {
                glDeleteTextures(1, &fbo.texture_id);
                fbo.texture_id = 0;
            }
            if (fbo.fbo != 0 && m_gl.glDeleteFramebuffers) {
                m_gl.glDeleteFramebuffers(1, &fbo.fbo);
                fbo.fbo = 0;
            }

            fbo.width = width;
            fbo.height = height;

            glGenTextures(1, &fbo.texture_id);
            glBindTexture(GL_TEXTURE_2D, fbo.texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);

            m_gl.glGenFramebuffers(1, &fbo.fbo);
            m_gl.glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
            m_gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.texture_id, 0);

            GLenum status = m_gl.glCheckFramebufferStatus(GL_FRAMEBUFFER);
            m_gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return (status == GL_FRAMEBUFFER_COMPLETE);
        }

        GLuint renderDeckWaveform(
            int deck_idx,
            float width,
            float height,
            double start_frame,
            double window_frames,
            const KuroAudio::DJDeck& deck,
            int engine_mode,
            int color_mode,
            int vertical_side,
            bool is_drop
        ) {
            if (!init()) return 0;
            if (deck_idx < 0 || deck_idx >= 4) return 0;
            if (width < 4.0f || height < 4.0f) return 0;

            int i_w = (int)width;
            int i_h = (int)height;
            FBOState& fbo = m_deck_fbos[deck_idx];
            if (!ensureFBO(fbo, i_w, i_h)) return 0;

            updateTrackTexture(deck_idx, deck);
            TrackTex& tt = m_track_textures[deck_idx];

            GLint last_fbo = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
            GLint last_vp[4];
            glGetIntegerv(GL_VIEWPORT, last_vp);
            GLboolean last_blend = glIsEnabled(GL_BLEND);
            GLboolean last_cull = glIsEnabled(GL_CULL_FACE);
            GLboolean last_depth = glIsEnabled(GL_DEPTH_TEST);
            GLboolean last_scissor = glIsEnabled(GL_SCISSOR_TEST);

            m_gl.glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
            glViewport(0, 0, i_w, i_h);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_SCISSOR_TEST);

            m_gl.glUseProgram(m_program);

            m_gl.glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tt.texture_id);
            m_gl.glUniform1i(m_u_wave_tex, 0);

            float start_bin = (float)(start_frame / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP);
            float end_bin   = (float)((start_frame + window_frames) / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP);
            float total_bins = (float)tt.num_bins;

            auto& layout_m = KuroUI::DJLayoutMetrics::getInstance();

            m_gl.glUniform1f(m_u_tex_width, (float)tt.width);
            m_gl.glUniform1f(m_u_tex_height, (float)tt.height);
            m_gl.glUniform1f(m_u_start_bin, start_bin);
            m_gl.glUniform1f(m_u_end_bin, end_bin);
            m_gl.glUniform1f(m_u_total_bins, total_bins);

            m_gl.glUniform1i(m_u_color_mode, color_mode);
            m_gl.glUniform1i(m_u_engine_mode, engine_mode);
            m_gl.glUniform1i(m_u_vertical_side, vertical_side);

            m_gl.glUniform1f(m_u_dynamic_contrast, std::clamp(layout_m.waveform_dynamic_contrast, 0.80f, 2.50f));
            m_gl.glUniform1f(m_u_transient_punch, std::clamp(layout_m.waveform_transient_punch, 0.50f, 3.00f));
            m_gl.glUniform1f(m_u_core_brightness, std::clamp(layout_m.waveform_core_brightness, 0.0f, 1.0f));

            m_gl.glUniform1i(m_u_line_style, layout_m.waveform_line_style);
            m_gl.glUniform1f(m_u_line_spacing, std::clamp(layout_m.waveform_line_spacing, 1.0f, 4.0f));
            m_gl.glUniform1i(m_u_is_drop, is_drop ? 1 : 0);
            m_gl.glUniform2f(m_u_resolution, (float)i_w, (float)i_h);

            m_gl.glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            m_gl.glBindVertexArray(0);

            m_gl.glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
            glViewport(last_vp[0], last_vp[1], last_vp[2], last_vp[3]);
            if (last_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
            if (last_depth) glEnable(GL_DEPTH_TEST);
            if (last_cull) glEnable(GL_CULL_FACE);
            if (last_scissor) glEnable(GL_SCISSOR_TEST);

            return fbo.texture_id;
        }

        GLuint renderOverlaidWaveform(
            float width,
            float height,
            double start_f1, double window_f1, const KuroAudio::DJDeck& deck1, bool is_drop1,
            double start_f2, double window_f2, const KuroAudio::DJDeck& deck2, bool is_drop2,
            int engine_mode,
            int color_mode
        ) {
            if (!init()) return 0;
            if (width < 4.0f || height < 4.0f) return 0;

            int i_w = (int)width;
            int i_h = (int)height;
            if (!ensureFBO(m_overlaid_fbo, i_w, i_h)) return 0;

            updateTrackTexture(deck1.deck_id, deck1);
            updateTrackTexture(deck2.deck_id, deck2);

            GLint last_fbo = 0; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
            GLint last_vp[4]; glGetIntegerv(GL_VIEWPORT, last_vp);
            GLboolean last_blend = glIsEnabled(GL_BLEND);
            GLboolean last_cull = glIsEnabled(GL_CULL_FACE);
            GLboolean last_depth = glIsEnabled(GL_DEPTH_TEST);
            GLboolean last_scissor = glIsEnabled(GL_SCISSOR_TEST);

            m_gl.glBindFramebuffer(GL_FRAMEBUFFER, m_overlaid_fbo.fbo);
            glViewport(0, 0, i_w, i_h);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_SCISSOR_TEST);

            auto& layout_m = KuroUI::DJLayoutMetrics::getInstance();

            // Pass 1: Deck 1 (Upper half, vertical_side = 1)
            {
                TrackTex& tt1 = m_track_textures[deck1.deck_id];
                m_gl.glUseProgram(m_program);
                m_gl.glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tt1.texture_id);
                m_gl.glUniform1i(m_u_wave_tex, 0);

                float s_bin = (float)(start_f1 / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP);
                float e_bin = (float)((start_f1 + window_f1) / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP);

                m_gl.glUniform1f(m_u_tex_width, (float)tt1.width);
                m_gl.glUniform1f(m_u_tex_height, (float)tt1.height);
                m_gl.glUniform1f(m_u_start_bin, s_bin);
                m_gl.glUniform1f(m_u_end_bin, e_bin);
                m_gl.glUniform1f(m_u_total_bins, (float)tt1.num_bins);

                m_gl.glUniform1i(m_u_color_mode, color_mode);
                m_gl.glUniform1i(m_u_engine_mode, 1); // Fused
                m_gl.glUniform1i(m_u_vertical_side, 1); // Upper half

                m_gl.glUniform1f(m_u_dynamic_contrast, std::clamp(layout_m.waveform_dynamic_contrast, 0.80f, 2.50f));
                m_gl.glUniform1f(m_u_transient_punch, std::clamp(layout_m.waveform_transient_punch, 0.50f, 3.00f));
                m_gl.glUniform1f(m_u_core_brightness, std::clamp(layout_m.waveform_core_brightness, 0.0f, 1.0f));

                m_gl.glUniform1i(m_u_line_style, layout_m.waveform_line_style);
                m_gl.glUniform1f(m_u_line_spacing, std::clamp(layout_m.waveform_line_spacing, 1.0f, 4.0f));
                m_gl.glUniform1i(m_u_is_drop, is_drop1 ? 1 : 0);
                m_gl.glUniform2f(m_u_resolution, (float)i_w, (float)i_h);

                m_gl.glBindVertexArray(m_vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            // Pass 2: Deck 2 (Lower half, vertical_side = -1)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                TrackTex& tt2 = m_track_textures[deck2.deck_id];
                m_gl.glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tt2.texture_id);
                m_gl.glUniform1i(m_u_wave_tex, 0);

                float s_bin = (float)(start_f2 / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP);
                float e_bin = (float)((start_f2 + window_f2) / (double)KuroAudio::DJDeck::DETAILED_WAVE_STEP);

                m_gl.glUniform1f(m_u_tex_width, (float)tt2.width);
                m_gl.glUniform1f(m_u_tex_height, (float)tt2.height);
                m_gl.glUniform1f(m_u_start_bin, s_bin);
                m_gl.glUniform1f(m_u_end_bin, e_bin);
                m_gl.glUniform1f(m_u_total_bins, (float)tt2.num_bins);

                m_gl.glUniform1i(m_u_color_mode, color_mode);
                m_gl.glUniform1i(m_u_engine_mode, 1);
                m_gl.glUniform1i(m_u_vertical_side, -1); // Lower half

                m_gl.glUniform1i(m_u_is_drop, is_drop2 ? 1 : 0);

                glDrawArrays(GL_TRIANGLES, 0, 6);
                m_gl.glBindVertexArray(0);
            }

            m_gl.glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
            glViewport(last_vp[0], last_vp[1], last_vp[2], last_vp[3]);
            if (last_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
            if (last_depth) glEnable(GL_DEPTH_TEST);
            if (last_cull) glEnable(GL_CULL_FACE);
            if (last_scissor) glEnable(GL_SCISSOR_TEST);

            return m_overlaid_fbo.texture_id;
        }
    };

} // namespace KuroUI
