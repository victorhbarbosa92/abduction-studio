# Configuração do Cloudflare Worker (Ponte Segura de Atualizações)

Este Worker atua como intermediário seguro entre as máquinas com o **Abduction Studio** e o seu **repositório privado no GitHub**. 
Ele garante que **o código e as releases nunca fiquem públicos**, e seu token do GitHub **nunca é exposto** dentro do executável.

---

### Passo 1: Criar o Worker na Cloudflare (100% Gratuito)

1. Acesse [dash.cloudflare.com](https://dash.cloudflare.com) e faça login (crie uma conta gratuita se não tiver).
2. No menu lateral, clique em **Compute (Workers) > Workers & Pages**.
3. Clique no botão azul **Create application** (ou **Criar aplicativo**) e escolha **Create Worker**.
4. Defina um nome, por exemplo: `abduction-updater-gateway`.
5. Clique em **Deploy**.

---

### Passo 2: Colar o Código

1. Na tela do worker criado, clique em **Edit code** (ou **Editar código**).
2. Apague todo o código existente no editor.
3. Copie todo o conteúdo do arquivo `worker.js` e cole no editor da Cloudflare.
4. Clique em **Save and deploy** no canto superior direito.

---

### Passo 3: Configurar as Chaves Secretas (Variables & Secrets)

No painel do seu Worker:
1. Vá na aba **Settings** (Configurações) > **Variables and Secrets**.
2. Em **Environment Variables**, adicione as seguintes variáveis:

| Nome da Variável | Tipo | Valor de Exemplo | Descrição |
| :--- | :--- | :--- | :--- |
| `GITHUB_OWNER` | Text | `seu-usuario-github` | Seu nome de usuário ou organização no GitHub |
| `GITHUB_REPO` | Text | `abduction_studio_v2` | Nome do repositório privado onde ficam as releases |
| `GITHUB_TOKEN` | **Secret** (Criptografado) | `ghp_xxxxxxxxxxxx` | Seu GitHub Personal Access Token (com escopo `repo` ou `Contents: Read`) |
| `UPDATE_SECRET` | Text ou Secret (Opcional) | `abduction_sec_2026_xyz` | Chave de autorização entre o app e o Worker (opcional) |

> [!TIP]
> **Como gerar o GitHub Token:**
> 1. No GitHub, vá em **Settings** > **Developer settings** > **Personal access tokens** > **Tokens (classic)** (ou Fine-grained).
> 2. Clique em **Generate new token**.
> 3. Marque a permissão `repo` (para acessar repositórios privados).
> 4. Copie o token gerado e cole como Secret no Cloudflare Worker.

---

### Passo 4: Obter a URL do seu Gateway

Na tela principal do seu Worker, você verá a URL gerada pela Cloudflare:
`https://abduction-updater-gateway.<seu-subdominio>.workers.dev`

Guarde essa URL! Ela será usada no `AutoUpdater.h` do Abduction Studio.
