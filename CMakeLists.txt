cmake_minimum_required(VERSION 3.20)
project(AbductionStudioV2 VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/arch:AVX2 /fp:fast /FS /MP)
else()
    add_compile_options(-mavx2 -O3 -ffast-math)
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
    ${IMGUI_SOURCES}
)

if(MSVC)
    # console app for debugging
    # set_target_properties(AbductionStudioV2 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

target_include_directories(AbductionStudioV2 PRIVATE 
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${onnxruntime_SOURCE_DIR}/include
)

target_link_libraries(AbductionStudioV2 PRIVATE
    glfw
    rtaudio
    opengl32
    winmm
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
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/sign.ps1"
    COMMENT "Copiando IA (ONNX) e Assinando Digitalmente com Certificado Kuru..."
)
