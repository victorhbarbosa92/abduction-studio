# 🎮 Workflow Obrigatório: Testes em Tempo Real e Sessões Interativas

---

## 🎯 Regra Fundamental de Testes (Sempre Ativa em Todas as Sessões)

Quando o usuário solicitar a execução de testes, validação de bugs ou demonstração de funcionalidades, o agente **DEVE SEGUIR ESTRITAMENTE** as seguintes diretrizes:

### 1. Testes Visíveis em Tempo Real na Tela (Nunca em Segundo Plano / "No Escuro")
- **Primeiro Plano Obrigatório**: Trazer sempre a janela do `Abduction Studio` para o primeiro plano (`SetForegroundWindow` / `ShowWindow(SW_MAXIMIZE)`).
- **Movimentação Suave do Mouse**: O cursor do mouse deve ser interpolado suavemente (com curva *SmoothStep*) em tempo real na tela do usuário, com pausas adequadas (0.5s a 1.5s) entre os cliques para que o usuário possa acompanhar visualmente cada ação.
- **Fluxo Completo do Zero**: O teste deve passar por todas as etapas reais (abrir menus, alternar entre modos/rádios/checkboxes, demonstrar imunidade a janelas sobrepostas e testar reprodução de áudio).

---

### 2. Sessão Interativa de Testes Colaborativa (Usuário + IA)
Quando o usuário quiser realizar os testes manualmente ou de forma conjunta:
1. **Ativação da DAW**: A DAW é inicializada e mantida em execução em primeiro plano na tela.
2. **Captura Contínua / Por Clique**: A engine registra os estados/prints a cada interação do usuário para auditoria visual.
3. **Escuta do Chat em Aberto**: O chat permanece aberto aguardando o relato por voz (áudio) ou texto do usuário.
4. **Diagnóstico & Correção**: Assim que o usuário envia sua mensagem/áudio descrevendo o comportamento observado, o agente analisa os registros da sessão, correlaciona com o áudio, encontra a causa raiz e implementa as correções no código.
