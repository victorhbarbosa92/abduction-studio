# 🛸 Abduction Studio

**Abduction Studio** é uma DAW (Digital Audio Workstation) com identidade visual alienígena e narrativa Sci-Fi, criada por **Kuru** — inspiração de Soma Cruz de Castlevania, do mundo Sci-Fi e do RPG, agora na vibe musical.

---

## Identidade Visual
- **Fundo:** Abissal (`#06040A`) — O vazio do espaço profundo
- **Acento Principal:** Crimson Neon (`#FF0055`) — O sangue digital da alma
- **Acento Secundário:** Deep Magenta (`#B300FF`) — O poder do domínio
- **Contraste Sci-Fi:** Cyan Alienígena (`#00FFF0`) — A tecnologia superior

## Features
- 🎛️ **Modo DJ** — Dois decks (Dimensão A [ALFA] / Dimensão B [ÔMEGA]) com crossfader, pitch e EQ
- 🎵 **Motor de Áudio** — Baseado em Tracktion Engine + JUCE
- 🎨 **Interface Nave** — CDJ Alienígena com visual gótico/cyberpunk
- 🔒 **Anti-Trava automática** — `Compilar_E_Liberar.bat` assina, desbloqueia e abre o exe

## Como Compilar

### Pré-requisitos
- Visual Studio 2022 com C++ Desktop
- CMake 3.22+
- Windows 10/11

### Build
```bat
# Clone com submodules
git clone --recurse-submodules https://github.com/SEU_USUARIO/abduction-studio.git
cd abduction-studio

# Configure e compile
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Ou use o script automático
.\Compilar_E_Liberar.bat
```

## Arquitetura
```
src/
├── App/              - Ponto de entrada (JUCEApplication)
├── Engine/           - Motor de áudio (DJEngineManager, PluginManager)
├── UI/
│   ├── DJMode/       - Interface dos decks CDJ
│   ├── LookAndFeel/  - Tema visual Kuru (NovaLookAndFeel)
│   ├── Mixer/        - Seção de mixagem
│   ├── Timeline/     - Linha do tempo de gravação
│   └── Transport/    - Barra de transporte
└── Utils/            - Constants, Helpers
```

---

*"O cockpit de domínio de almas. A nave está pronta."* — Kuru
