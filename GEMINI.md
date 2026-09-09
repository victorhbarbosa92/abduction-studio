# Diretrizes do Projeto: Abduction Studio

## UI Planning Rule
When planning structural UI changes or new visual features, ALWAYS generate a reference image (mockup) using the generate_image tool BEFORE writing code. The user expects all planning phases to include visual mockups to ensure the application remains intuitive.

## Timeline and Ruler Rules
- **Timeline Default Scale:** When implementing a timeline or piano roll grid, default the zoom scale to fit at least 1 minute (60 seconds) on the visible area.
- **Interactive Zooming:** Always implement Ctrl + Mouse Wheel zoom on all timelines/grids.
- **Playhead Clipping:** Always clip the playhead line and triangle using `PushClipRect` at the grid boundary to prevent overlapping headers.

## Workflow Obrigatório de Testes em Tempo Real e Sessões Interativas
Sempre que for solicitado fazer testes, auditar bugs ou validar funcionalidades:
1. **Visibilidade Obrigatória (Nunca rodar no escuro/background)**:
   - Trazer sempre a janela da DAW para o primeiro plano maximizada.
   - Usar movimentação suave do mouse (*SmoothStep*) com pausas visíveis para o usuário acompanhar cada etapa na tela.
2. **Sessão Interativa Colaborativa (Usuário + IA)**:
   - Quando o usuário for testar manualmente com seu próprio mouse/teclado, a DAW é iniciada e mantida aberta com o chat pronto.
   - A cada clique ou interação do usuário, snapshots visuais da sessão são registrados.
   - O usuário envia seu áudio descrevendo o que sentiu e o agente analisa os prints cronológicos, correlaciona com o áudio e aplica as correções no código imediatamente.

## Padrão Obrigatório de Processamento de UI e Recorte de Assets (Transparência Alfa e Anti-Aliasing)
Sempre que uma nova interface, plugin, módulo ou mockup visual for criado ou selecionado:
1. **Pipeline Padrão Unificado (`tools/ui_asset_processor.py`)**:
   - É obrigatório utilizar a ferramenta oficial de recorte e processamento de assets.
   - Criar/manter a receita correspondente em `tools/recipes/<nome_plugin>_recipe.json`.
2. **Proibição Estrita de Bordas Quadradas sem Canal Alfa**:
   - É estritamente proibido renderizar elementos circulares (knobs, dials, botões de onda) como retângulos opacos (`Alpha = 255`).
   - Todo knob rotativo, dial e botão circular DEVE possuir canal alfa 100% transparente fora do seu raio (`Alpha = 0.0`), com suavização anti-aliased sub-pixel na borda externa.
3. **Geração e Empacotamento de Alta Fidelidade**:
   - Knobs e Dials devem utilizar a textura de metal escovado anisotrópico gerada proceduralmente ou recortada com máscara alfa circular perfeita.
   - Os assets devem ser compilados no pacote binário (`textures.bin` / formato `KUROTEX1`) para carregamento instantâneo via C++ no ImGui/OpenGL.
