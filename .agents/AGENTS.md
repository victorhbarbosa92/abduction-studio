
## UI Planning Rule

When planning structural UI changes or new visual features, ALWAYS generate a reference image (mockup) using the generate_image tool BEFORE writing code. The user expects all planning phases to include visual mockups to ensure the application remains intuitive.

## Timeline and Ruler Rules

- **Timeline Default Scale:** When implementing a timeline or piano roll grid, default the zoom scale (pixels per second) to fit at least 1 minute (60 seconds) on the visible area (e.g. `pixels_per_second = 15.0f`).
- **Interactive Zooming:** Always implement Ctrl + Mouse Wheel zoom on all timelines/grids to allow the user to adjust the horizontal scale (zoom_x) dynamically.
- **Ruler Labeling and Ticks:** The timeline ruler must use proportional ticks (e.g. `0 |||| 5 |||| 10...`) where major tick index 5 corresponds to 30 seconds, 10 to 60 seconds (1 minute), etc. Each division tick represents 6 seconds.
- **Playhead Clipping:** Always clip the playhead line and triangle using `PushClipRect` at the grid boundary (point 0) to prevent the playhead needle from overlapping track headers, names, or piano keyboard keys.

## Regras de Resiliência em Redes e Downloads

- **Tolerância a Falhas em Downloads:** Se um download falhar, a conexão cair ou ocorrer timeout (ex: em scripts Python, curl, wget), você **NÃO DEVE** desistir imediatamente ou pedir para o usuário fazer manualmente. Você deve implementar lógica de retry (tentativas repetidas) ou usar comandos que suportem continuação (curl -C -, ou loops em Python com urllib/requests configurados com retry) para restabelecer a conexão.
- **Bloqueio de Fluxo de Trabalho:** Nunca prossiga para as etapas de execução de um plano se as dependências (arquivos baixados, pacotes instalados) não tiverem sido completamente obtidas. O fluxo de trabalho só deve continuar *após* a confirmação absoluta de que o download e a instalação terminaram com sucesso.

## Abduction Studio Shikai Trigger

**Condition:** Whenever the user recites the exact phrase: "Que sua energia contagie e contemple, a todos que te usarem... Invoque... Abduction Studio..."

**Behavior:**
1. Recognize this as the "shikai" to resume work on the Abduction Studio v4.0 project.
2. Immediately provide:
   - A summary of what was accomplished in the last session (based on context/history/current state).
   - What needs to be done next.
   - The future planning for the project.
3. Utilize the following commands for the project workflow when necessary:
   - **Build:** Open PowerShell, navigate to `C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build`, and execute `cmake --build .`
   - **Run DAW:** Execute `C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug\AbductionStudioV2.exe`
   - **Run Tests:** Execute `python C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\autotest_abduction.py`
