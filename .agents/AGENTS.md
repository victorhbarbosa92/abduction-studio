
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
