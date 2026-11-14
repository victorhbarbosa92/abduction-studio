cmake_minimum_required(VERSION 3.20)
project(AbductionStudioV2 VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/O2 /Oi /Ot /fp:fast /FS /MP /Zi)
else()
    add_compile_options(-O2 -ffast-math -Wno-deprecated-declarations)
endif()

include(FetchContent)

# 1. Fetch GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 2. Fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW 1
)
FetchContent_MakeAvailable(imgui)

# 3. Fetch RtAudio (Manual build to avoid 'uninstall' target collision with GLFW)
FetchContent_Declare(
    rtaudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG 6.0.1
)
FetchContent_GetProperties(rtaudio)
if(NOT rtaudio_POPULATED)
    FetchContent_Populate(rtaudio)
endif()

add_library(rtaudio STATIC ${rtaudio_SOURCE_DIR}/RtAudio.cpp)
target_include_directories(rtaudio PUBLIC ${rtaudio_SOURCE_DIR} ${rtaudio_SOURCE_DIR}/include)
target_compile_definitions(rtaudio PUBLIC __WINDOWS_WASAPI__)
target_link_libraries(rtaudio PUBLIC ole32 ksuser mfplat mfuuid wmcodecdspuuid avrt)

# 4. Fetch ONNX Runtime (Pre-compiled for Windows x64)
FetchContent_Declare(
    onnxruntime
    URL "https://github.com/microsoft/onnxruntime/releases/download/v1.17.1/onnxruntime-win-x64-1.17.1.zip"
)
FetchContent_MakeAvailable(onnxruntime)

# Download dr_wav.h to src/ai if it doesn't exist
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/ai/dr_wav.h")
    file(DOWNLOAD "https://raw.githubusercontent.com/mackron/dr_libs/master/dr_wav.h" "${CMAKE_SOURCE_DIR}/src/ai/dr_wav.h" SHOW_PROGRESS)
endif()

# Prepare ImGui source files
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(AbductionStudioV2
    src/main.cpp
    src/core/miniz.c
    src/core/DawApiBridge.cpp
    src/core/QMBeatGridEngine.cpp
    src/thirdparty/qm-dsp/dsp/onsets/DetectionFunction.cpp
    src/thirdparty/qm-dsp/dsp/phasevocoder/PhaseVocoder.cpp
    src/thirdparty/qm-dsp/dsp/transforms/FFT.cpp
    src/thirdparty/qm-dsp/dsp/tempotracking/TempoTrackV2.cpp
    src/thirdparty/qm-dsp/dsp/tempotracking/DownBeat.cpp
    src/thirdparty/qm-dsp/dsp/rateconversion/Decimator.cpp
    src/thirdparty/qm-dsp/maths/MathUtilities.cpp
    src/thirdparty/qm-dsp/maths/KLDivergence.cpp
    src/thirdparty/qm-dsp/ext/kissfft/kiss_fft.c
    src/thirdparty/qm-dsp/ext/kissfft/tools/kiss_fftr.c
    ${IMGUI_SOURCES}
)

if(MSVC)
    set_target_properties(AbductionStudioV2 PROPERTIES LINK_FLAGS "/DEBUG /MAP:AbductionStudioV2.map")
elseif(MINGW)
    target_link_options(AbductionStudioV2 PRIVATE -static)
endif()

target_compile_definitions(AbductionStudioV2 PRIVATE
    kiss_fft_scalar=double
    _USE_MATH_DEFINES
    M_PI=3.14159265358979323846
)

target_include_directories(AbductionStudioV2 PRIVATE 
    src
    src/thirdparty/qm-dsp
    src/thirdparty/qm-dsp/ext/kissfft
    src/thirdparty/qm-dsp/ext/kissfft/tools
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${onnxruntime_SOURCE_DIR}/include
)

target_link_libraries(AbductionStudioV2 PRIVATE
    glfw
    rtaudio
    opengl32
    winmm
    mfplay
    ws2_32
    winhttp
    ${onnxruntime_SOURCE_DIR}/lib/onnxruntime.lib
)

# Sistema Anti-Lock (Mata instâncias antigas antes de compilar)
add_custom_command(TARGET AbductionStudioV2 PRE_BUILD
    COMMAND powershell -Command "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue; exit 0"
    COMMENT "Anti-Lock: Encerrando AbductionStudioV2 em background..."
)

# Pós-processamento para copiar DLLs e assinar binário
add_custom_command(TARGET AbductionStudioV2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll" "$<TARGET_FILE_DIR:AbductionStudioV2>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/cloud_stem_extractor.py" "$<TARGET_FILE_DIR:AbductionStudioV2>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/edm_stem_extractor.py" "$<TARGET_FILE_DIR:AbductionStudioV2>/"
    COMMAND powershell -Command "if (Test-Path '${CMAKE_SOURCE_DIR}/libc++.dll') { Copy-Item '${CMAKE_SOURCE_DIR}/libc++.dll' '$<TARGET_FILE_DIR:AbductionStudioV2>/' -Force; Copy-Item '${CMAKE_SOURCE_DIR}/libunwind.dll' '$<TARGET_FILE_DIR:AbductionStudioV2>/' -Force; Copy-Item '${CMAKE_SOURCE_DIR}/libwinpthread-1.dll' '$<TARGET_FILE_DIR:AbductionStudioV2>/' -Force }"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:AbductionStudioV2>" "${CMAKE_SOURCE_DIR}/AbductionStudioV2.exe"
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/sign.ps1"
    COMMENT "Copiando IA (ONNX), DLLs, atualizando executavel na raiz e Assinando Digitalmente..."
)
