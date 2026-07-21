import urllib.request
import re
import sqlite3
import os
import json

DB_PATH = "daw_apis.db"
LOG_PATH = "daw_apis_scraped.log"

# URLs de documentações reais da comunidade
FL_STUDIO_URLS = {
    "mixer": "https://raw.githubusercontent.com/delorenj/skills/main/flstudio-scripting/references/api-mixer.md",
    "transport": "https://raw.githubusercontent.com/delorenj/skills/main/flstudio-scripting/references/api-transport.md",
    "channels": "https://raw.githubusercontent.com/delorenj/skills/main/flstudio-scripting/references/api-channels.md",
    "playlist": "https://raw.githubusercontent.com/delorenj/skills/main/flstudio-scripting/references/api-playlist.md"
}

ABLETON_URLS = {
    "Song": "https://raw.githubusercontent.com/mikecfisher/ableton-lom-skill/main/references/song.md",
    "Track": "https://raw.githubusercontent.com/mikecfisher/ableton-lom-skill/main/references/track.md",
    "Device": "https://raw.githubusercontent.com/mikecfisher/ableton-lom-skill/main/references/device.md"
}

def fetch_url(url):
    print(f"Buscando URL: {url}")
    try:
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'}
        )
        with urllib.request.urlopen(req, timeout=10) as response:
            return response.read().decode('utf-8')
    except Exception as e:
        print(f"Aviso: Erro ao buscar URL ({e}). Usando fallback offline.")
        return None

def parse_fl_studio_file(mod_name, content):
    # Divide o arquivo por "### " para obter blocos de funções
    blocks = content.split("### ")
    extracted = []
    
    for block in blocks[1:]:
        # 1. Nome da função (primeira linha, entre crases)
        first_line = block.split("\n")[0].strip()
        name_match = re.search(r"`([a-zA-Z0-9_]+)`", first_line)
        if not name_match:
            continue
        func_name = name_match.group(1)
        
        # 2. Assinatura e tipo de retorno (bloco python)
        sig_match = re.search(r"```python\n(.*?)\n```", block, re.DOTALL)
        signature = ""
        params = []
        return_type = "None"
        
        if sig_match:
            signature_raw = sig_match.group(1).strip()
            # Normaliza quebras de linha na assinatura
            signature = " ".join(signature_raw.split())
            
            # Tenta pegar retorno (ex: -> float)
            ret_match = re.search(r"->\s*([a-zA-Z0-9_<>\s\[\]]+)", signature)
            if ret_match:
                return_type = ret_match.group(1).strip()
                
            # Extrai os parâmetros entre parenteses
            param_match = re.search(r"\((.*?)\)", signature)
            if param_match:
                p_raw = param_match.group(1).strip()
                params = [p.split(":")[0].strip().split("=")[0].strip() for p in p_raw.split(",") if p.strip()]
                
        # 3. Descrição (tudo após o bloco python)
        desc_lines = []
        if sig_match:
            parts = block.split("```")
            if len(parts) >= 3:
                desc_text = parts[2].strip()
                desc_lines = [line.strip() for line in desc_text.split("\n") if line.strip() and not line.strip().startswith("-")]
        
        description = " ".join(desc_lines) if desc_lines else "Sem descrição detalhada."
        
        extracted.append({
            "module": mod_name,
            "name": func_name,
            "signature": signature if signature else f"{mod_name}.{func_name}()",
            "parameters": params,
            "return_type": return_type,
            "description": description,
            "category": f"FL MIDI {mod_name.upper()} / Web Scraped"
        })
        
    return extracted

def parse_ableton_file(cls_name, content):
    extracted = []
    # Encontra todas as tabelas Markdown no arquivo
    # Formato: | Property | Type | Access | Observable | Description |
    # Ou: | Method | Return Type | Description |
    lines = content.split("\n")
    in_table = False
    headers = []
    
    for line in lines:
        line = line.strip()
        if line.startswith("|") and line.endswith("|"):
            cells = [cell.strip() for cell in line.split("|")[1:-1]]
            if not cells:
                continue
                
            # Se for a linha divisória da tabela
            if all(cell.startswith("-") or cell == "" for cell in cells):
                continue
                
            # Verifica cabeçalhos
            if not in_table or "property" in [c.lower() for c in cells] or "method" in [c.lower() for c in cells]:
                headers = [c.lower() for c in cells]
                in_table = True
                continue
                
            # Mapeia valores para as colunas correspondentes
            item = {}
            for h, val in zip(headers, cells):
                item[h] = val
                
            # Valida e extrai
            item_name = item.get("property") or item.get("method") or item.get("name")
            if not item_name:
                continue
                
            # Limpa crases
            item_name = item_name.replace("`", "").replace("()", "").strip()
            
            # Evita nomes reservados ou inválidos
            if item_name.lower() in ["property", "method", "name", "value", "type"]:
                continue
                
            item_type = "Method" if "method" in headers else "Property"
            return_type = item.get("type") or item.get("return type") or item.get("return_type") or "Unknown"
            return_type = return_type.replace("`", "").strip()
            
            desc = item.get("description") or "Sem descrição."
            desc = desc.strip()
            
            extracted.append({
                "class": cls_name,
                "name": item_name,
                "item_type": item_type,
                "return_type": return_type,
                "description": desc,
                "category": f"Ableton LOM {cls_name} / Web Scraped"
            })
        else:
            in_table = False
            
    return extracted

def merge_into_database(fl_data, ableton_data):
    print("Atualizando banco de dados SQL com os dados raspados da Web...")
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("PRAGMA foreign_keys = ON;")
    
    # Obter os IDs das DAWs no DB
    cursor.execute("SELECT id FROM daws WHERE name = 'Ableton Live';")
    ableton_row = cursor.fetchone()
    ableton_id = ableton_row[0] if ableton_row else None
    
    cursor.execute("SELECT id FROM daws WHERE name = 'FL Studio';")
    fl_row = cursor.fetchone()
    fl_id = fl_row[0] if fl_row else None
    
    if not ableton_id or not fl_id:
        print("Erro: DAWs não encontradas no banco. Execute extract_daw_apis.py primeiro.")
        conn.close()
        return

    # Inserir dados do FL Studio
    for item in fl_data:
        cursor.execute("""
            INSERT OR IGNORE INTO modules_classes (daw_id, type, name, description)
            VALUES (?, 'Module', ?, 'Módulo extraído dinamicamente via Web Scraping')
        """, (fl_id, item['module']))
        
        cursor.execute("SELECT id FROM modules_classes WHERE daw_id = ? AND name = ?;", (fl_id, item['module']))
        mod_id = cursor.fetchone()[0]
        
        cursor.execute("""
            INSERT OR REPLACE INTO functions_properties 
            (container_id, name, item_type, signature, parameters, return_type, description, category)
            VALUES (?, ?, 'Function', ?, ?, ?, ?, ?)
        """, (mod_id, item['name'], item['signature'], json.dumps(item['parameters']), 
              item['return_type'], item['description'], item['category']))
        
    # Inserir dados do Ableton Live
    for item in ableton_data:
        cursor.execute("""
            INSERT OR IGNORE INTO modules_classes (daw_id, type, name, description)
            VALUES (?, 'Class', ?, 'Classe extraída dinamicamente via Web Scraping')
        """, (ableton_id, item['class']))
        
        cursor.execute("SELECT id FROM modules_classes WHERE daw_id = ? AND name = ?;", (ableton_id, item['class']))
        cls_id = cursor.fetchone()[0]
        
        cursor.execute("""
            INSERT OR REPLACE INTO functions_properties 
            (container_id, name, item_type, signature, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (cls_id, item['name'], item['item_type'], 
              f"{item['name']}()" if item['item_type'] == 'Method' else None, 
              item['return_type'], item['description'], item['category']))
        
    conn.commit()
    
    # Exibir estatísticas atualizadas
    cursor.execute("SELECT COUNT(*) FROM functions_properties;")
    total_funcs = cursor.fetchone()[0]
    print(f"Sucesso! Total de funções/métodos no banco de dados agora: {total_funcs}")
    
    conn.close()

def save_to_log(fl_data, ableton_data):
    print(f"Salvando dados estruturados no log: {LOG_PATH}")
    with open(LOG_PATH, "w", encoding="utf-8") as f:
        f.write("=== LOG DE EXTRAÇÃO E WEB SCRAPING - ABLETON & FL STUDIO ===\n")
        f.write("Destino da Integração: Abduction Studio V2\n\n")
        
        f.write("--- FL STUDIO (MIDI SCRIPTING API - SCRAPED) ---\n")
        for idx, item in enumerate(fl_data):
            f.write(f"[{idx+1}] Módulo: {item['module']} | Função: {item['name']}\n")
            f.write(f"    Assinatura: {item['signature']}\n")
            f.write(f"    Parâmetros: {item['parameters']}\n")
            f.write(f"    Retorno: {item['return_type']}\n")
            f.write(f"    Descrição: {item['description']}\n\n")
            
        f.write("\n--- ABLETON LIVE (LOM - SCRAPED) ---\n")
        for idx, item in enumerate(ableton_data):
            f.write(f"[{idx+1}] Classe: {item['class']} | Tipo: {item['item_type']} | Nome: {item['name']}\n")
            f.write(f"    Retorno: {item['return_type']}\n")
            f.write(f"    Descrição: {item['description']}\n\n")

if __name__ == "__main__":
    fl_data_all = []
    ableton_data_all = []
    
    # 1. Scrape do FL Studio
    for mod_name, url in FL_STUDIO_URLS.items():
        content = fetch_url(url)
        if content:
            parsed = parse_fl_studio_file(mod_name, content)
            fl_data_all.extend(parsed)
            print(f"Módulo '{mod_name}': {len(parsed)} funções extraídas.")
            
    # 2. Scrape do Ableton Live
    for cls_name, url in ABLETON_URLS.items():
        content = fetch_url(url)
        if content:
            parsed = parse_ableton_file(cls_name, content)
            ableton_data_all.extend(parsed)
            print(f"Classe '{cls_name}': {len(parsed)} propriedades/métodos extraídos.")
            
    # 3. Salva logs e banco
    save_to_log(fl_data_all, ableton_data_all)
    if fl_data_all or ableton_data_all:
        merge_into_database(fl_data_all, ableton_data_all)
    else:
        print("Nenhum dado raspado dinamicamente das documentações.")
        
    print("Processamento do Web Scraping finalizado.")
