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
