/**
 * Abduction Studio - Secure Private Update Gateway
 * Cloudflare Worker
 * 
 * Funcionalidades:
 * 1. Consulta releases privadas do GitHub usando um token seguro no servidor.
 * 2. Faz streaming do download do patch/instalador sem expor tokens para o cliente.
 * 3. Opcionalmente valida uma chave secreta (X-Abduction-Key) para barrar acessos externos.
 */

export default {
  async fetch(request, env, ctx) {
    const url = new URL(request.url);

    // Configurações vindas das variáveis de ambiente (Secrets) da Cloudflare
    const GITHUB_TOKEN = env.GITHUB_TOKEN;
    const GITHUB_OWNER = env.GITHUB_OWNER;
    const GITHUB_REPO  = env.GITHUB_REPO;
    const UPDATE_SECRET = env.UPDATE_SECRET || ""; // Opcional: chave secreta do app

    // Headers CORS para permitir requisições se necessário
    const corsHeaders = {
      "Access-Control-Allow-Origin": "*",
      "Access-Control-Allow-Methods": "GET, HEAD, OPTIONS",
      "Access-Control-Allow-Headers": "Content-Type, X-Abduction-Key",
    };

    if (request.method === "OPTIONS") {
      return new Response(null, { headers: corsHeaders });
    }

    // Validação da chave secreta (se configurada no Worker)
    if (UPDATE_SECRET) {
      const clientKey = request.headers.get("X-Abduction-Key") || url.searchParams.get("key");
      if (clientKey !== UPDATE_SECRET) {
        return new Response(JSON.stringify({ error: "Acesso não autorizado." }), {
          status: 401,
          headers: { ...corsHeaders, "Content-Type": "application/json" }
        });
      }
    }

    // 1. Rota de checagem: /api/check-update?version=2.0.0
    if (url.pathname === "/api/check-update") {
      if (!GITHUB_TOKEN || !GITHUB_OWNER || !GITHUB_REPO) {
        return new Response(JSON.stringify({ error: "Worker não configurado com as credenciais do GitHub." }), {
          status: 500,
          headers: { ...corsHeaders, "Content-Type": "application/json" }
        });
      }

      const clientVersion = (url.searchParams.get("version") || "0.0.0").replace(/^v/i, "");

      try {
        const ghResponse = await fetch(`https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/releases/latest`, {
          headers: {
            "User-Agent": "Abduction-Updater-Gateway",
            "Authorization": `Bearer ${GITHUB_TOKEN}`,
            "Accept": "application/vnd.github.v3+json"
          }
        });

        if (!ghResponse.ok) {
          const errText = await ghResponse.text();
          return new Response(JSON.stringify({ error: "Falha ao consultar GitHub", details: errText }), {
            status: ghResponse.status,
            headers: { ...corsHeaders, "Content-Type": "application/json" }
          });
        }

        const release = await ghResponse.json();
        const latestVersion = (release.tag_name || "0.0.0").replace(/^v/i, "");
        const updateAvailable = compareVersions(latestVersion, clientVersion) > 0;

        // Procura o asset do tipo .zip ou instalador
        let targetAsset = release.assets.find(a => a.name.toLowerCase().endsWith(".zip"));
        if (!targetAsset && release.assets.length > 0) {
          targetAsset = release.assets[0];
        }

        const downloadPath = `/api/download/latest`;
        const fullDownloadUrl = `${url.origin}${downloadPath}${UPDATE_SECRET ? `?key=${UPDATE_SECRET}` : ""}`;

        const payload = {
          status: "success",
          update_available: updateAvailable,
          latest_version: latestVersion,
          current_version: clientVersion,
          title: release.name || `Abduction Studio v${latestVersion}`,
          changelog: release.body || "Correções e melhorias de estabilidade.",
          published_at: release.published_at,
          asset_name: targetAsset ? targetAsset.name : "",
          size_bytes: targetAsset ? targetAsset.size : 0,
          download_url: fullDownloadUrl
        };

        return new Response(JSON.stringify(payload, null, 2), {
          status: 200,
          headers: { ...corsHeaders, "Content-Type": "application/json" }
        });
      } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), {
          status: 500,
          headers: { ...corsHeaders, "Content-Type": "application/json" }
        });
      }
    }

    // 2. Rota de download: /api/download/latest
    if (url.pathname === "/api/download/latest") {
      if (!GITHUB_TOKEN || !GITHUB_OWNER || !GITHUB_REPO) {
        return new Response("Configurações ausentes no Worker", { status: 500 });
      }

      try {
        // Busca a última release para pegar o ID do asset
        const ghReleaseRes = await fetch(`https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/releases/latest`, {
          headers: {
            "User-Agent": "Abduction-Updater-Gateway",
            "Authorization": `Bearer ${GITHUB_TOKEN}`,
            "Accept": "application/vnd.github.v3+json"
          }
        });

        if (!ghReleaseRes.ok) {
          return new Response("Não foi possível localizar a release", { status: 404 });
        }

        const release = await ghReleaseRes.json();
        let targetAsset = release.assets.find(a => a.name.toLowerCase().endsWith(".zip"));
        if (!targetAsset && release.assets.length > 0) {
          targetAsset = release.assets[0];
        }

        if (!targetAsset) {
          return new Response("Nenhum arquivo anexado a esta release", { status: 404 });
        }

        // Requisita o asset privado usando 'application/octet-stream'
        // O GitHub responde com 302 redirecionando para a CDN segura da AWS
        const assetUrl = `https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/releases/assets/${targetAsset.id}`;
        const assetRes = await fetch(assetUrl, {
          redirect: "follow",
          headers: {
            "User-Agent": "Abduction-Updater-Gateway",
            "Authorization": `Bearer ${GITHUB_TOKEN}`,
            "Accept": "application/octet-stream"
          }
        });

        if (!assetRes.ok) {
          return new Response("Falha ao baixar arquivo do GitHub", { status: assetRes.status });
        }

        const responseHeaders = new Headers(corsHeaders);
        responseHeaders.set("Content-Disposition", `attachment; filename="${targetAsset.name}"`);
        responseHeaders.set("Content-Type", assetRes.headers.get("Content-Type") || "application/octet-stream");
        if (targetAsset.size) {
          responseHeaders.set("Content-Length", targetAsset.size.toString());
        }

        // Stream direto para o cliente
        return new Response(assetRes.body, {
          status: 200,
          headers: responseHeaders
        });
      } catch (err) {
        return new Response("Erro no servidor: " + err.message, { status: 500 });
      }
    }

    // Rota raiz (Health check)
    return new Response(JSON.stringify({
      service: "Abduction Studio Private Update Gateway",
      status: "online",
      endpoints: [
        "/api/check-update?version=X.Y.Z",
        "/api/download/latest"
      ]
    }, null, 2), {
      status: 200,
      headers: { ...corsHeaders, "Content-Type": "application/json" }
    });
  }
};

/**
 * Compara duas versões semânticas (ex: "2.1.0" vs "2.0.0")
 * Retorna: 1 se v1 > v2, -1 se v1 < v2, 0 se iguais
 */
function compareVersions(v1, v2) {
  const parts1 = v1.split(".").map(n => parseInt(n, 10) || 0);
  const parts2 = v2.split(".").map(n => parseInt(n, 10) || 0);
  const maxLen = Math.max(parts1.length, parts2.length);

  for (let i = 0; i < maxLen; i++) {
    const p1 = parts1[i] || 0;
    const p2 = parts2[i] || 0;
    if (p1 > p2) return 1;
    if (p1 < p2) return -1;
  }
  return 0;
}
