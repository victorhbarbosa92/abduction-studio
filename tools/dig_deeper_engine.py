# -*- coding: utf-8 -*-
"""
Dig Deeper // Crate Digger Acústico Auto-Evolutivo
Motor de Inteligência Acústica, Busca Vetorial e Aprendizado por Reforço para o Abduction Studio V2
"""

import sys
import os
import json
import sqlite3
import re
import time
import math
import argparse
import urllib.request
import urllib.parse

if sys.platform == 'win32':
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
SCRATCH_DIR = os.path.abspath(os.path.join(TOOLS_DIR, "..", "scratch"))
DB_PATH = os.path.join(SCRATCH_DIR, "dig_deeper_knowledge_graph.sqlite")

os.makedirs(SCRATCH_DIR, exist_ok=True)
os.makedirs(os.path.join(SCRATCH_DIR, "tracks"), exist_ok=True)

# ═════════════════════════════════════════════════════════════════════════════
# 1. BANCO DE DADOS E GRAFO DE CONHECIMENTO ACÚSTICO (SQLITE)
# ═════════════════════════════════════════════════════════════════════════════

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    
    # Tabela de faixas indexadas com DNA acústico
    cur.execute("""
    CREATE TABLE IF NOT EXISTS tracks (
        id TEXT PRIMARY KEY,
        title TEXT,
        artist TEXT,
        album TEXT,
        bpm REAL,
        key_signature TEXT,
        camelot_num INTEGER,
        camelot_letter TEXT,
        subgenre TEXT,
        provider TEXT,
        url TEXT,
        artwork_url TEXT,
        energy REAL DEFAULT 0.85,
        sub_bass_weight REAL DEFAULT 0.90,
        transient_density REAL DEFAULT 0.88,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )
    """)
    
    # Grafo de afinidade aprendida (Edges entre faixas)
    cur.execute("""
    CREATE TABLE IF NOT EXISTS affinity_edges (
        ref_id TEXT,
        target_id TEXT,
        weight REAL DEFAULT 1.0,
        play_count INTEGER DEFAULT 0,
        success_count INTEGER DEFAULT 0,
        last_interacted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        PRIMARY KEY (ref_id, target_id)
    )
    """)
    
    # Log de feedback do DJ (reforço contínuo)
    cur.execute("""
    CREATE TABLE IF NOT EXISTS dj_feedback_log (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ref_id TEXT,
        target_id TEXT,
        action TEXT,
        weight_delta REAL,
        deck INTEGER,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )
    """)
    
    # Tabela de preferências e calibração de pesos do DJ
    cur.execute("""
    CREATE TABLE IF NOT EXISTS dj_profile_weights (
        key TEXT PRIMARY KEY,
        val REAL
    )
    """)
    
    # Valores padrão de ponderação
    defaults = [
        ("w_groove", 0.35),
        ("w_harmonic", 0.35),
        ("w_bpm", 0.20),
        ("w_learned", 0.10)
    ]
    for k, v in defaults:
        cur.execute("INSERT OR IGNORE INTO dj_profile_weights (key, val) VALUES (?, ?)", (k, v))
        
    conn.commit()
    conn.close()

# ═════════════════════════════════════════════════════════════════════════════
# 2. PARSER HARMONICO CAMELOT & ENCAIXE DE SET
# ═════════════════════════════════════════════════════════════════════════════

def parse_camelot(key_str):
    if not key_str:
        return 8, 'A' # Default Am
    m = re.search(r'(\d{1,2})([ABab])', key_str)
    if m:
        num = int(m.group(1))
        let = m.group(2).upper()
        if 1 <= num <= 12:
            return num, let
            
    # Conversão de notação musical clássica
    scale_map = {
        'Abm': (1, 'A'), 'G#m': (1, 'A'), 'B': (1, 'B'),
        'Ebm': (2, 'A'), 'D#m': (2, 'A'), 'F#': (2, 'B'), 'Gb': (2, 'B'),
        'Bbm': (3, 'A'), 'A#m': (3, 'A'), 'Db': (3, 'B'), 'C#': (3, 'B'),
        'Fm': (4, 'A'), 'Ab': (4, 'B'), 'G#': (4, 'B'),
        'Cm': (5, 'A'), 'Eb': (5, 'B'), 'D#': (5, 'B'),
        'Gm': (6, 'A'), 'Bb': (6, 'B'), 'A#': (6, 'B'),
        'Dm': (7, 'A'), 'F': (7, 'B'),
        'Am': (8, 'A'), 'C': (8, 'B'),
        'Em': (9, 'A'), 'G': (9, 'B'),
        'Bm': (10, 'A'), 'D': (10, 'B'),
        'F#m': (11, 'A'), 'Gbm': (11, 'A'), 'A': (11, 'B'),
        'D#m': (12, 'A'), 'C#m': (12, 'A'), 'E': (12, 'B')
    }
    for s, (num, let) in scale_map.items():
        if s.lower() in key_str.lower():
            return num, let
            
    return 8, 'A'

def calc_harmonic_compatibility(ref_num, ref_let, cand_num, cand_let):
    """
    Retorna score de 0.0 a 1.0 e descrição de compatibilidade Camelot
    """
    if ref_num == cand_num and ref_let == cand_let:
        return 1.0, "PERFEITO (+0)", "MIX HARMÔNICO 100%"
        
    diff = abs(ref_num - cand_num)
    if diff == 11: # Circunferência 12 <-> 1
        diff = 1
        
    # Subida ou Descida de Energia (+1 ou -1)
    if ref_let == cand_let and diff == 1:
        if (ref_num % 12) + 1 == cand_num:
            return 0.95, "SUBIDA (+1)", "+1 PICO DE ENERGIA"
        else:
            return 0.88, "DESCIDA (-1)", "TRANSIÇÃO PROFUNDA"
            
    # Relativo Maior / Menor (ex: 8A <-> 8B)
    if ref_num == cand_num and ref_let != cand_let:
        return 0.90, "RELATIVO", "MUDANÇA MAIOR/MENOR"
        
    # Diagonal (+1 e troca A/B)
    if diff == 1 and ref_let != cand_let:
        return 0.78, "DIAGONAL", "TRANSIÇÃO HÍBRIDA"
        
    # Salto de +2 (Energy Boost)
    if ref_let == cand_let and diff == 2:
        return 0.70, "SALTO +2", "EXPLOSÃO DE ENERGIA"
        
    return 0.35, "DISSONANTE", "CONTRASTE EXPERIMENTAL"

# ═════════════════════════════════════════════════════════════════════════════
# 3. BASE DE CONHECIMENTO CURADA DE MÚSICAS UNDERGROUND & PSYTRANCE/TECHNO
# ═════════════════════════════════════════════════════════════════════════════

UNDERGROUND_SEEDS = [
    # Jumpstreet & Sangoma / Looney Moon Records
    {"id": "dig_js_01", "title": "Spirit Overflow", "artist": "Jumpstreet", "album": "Nanodroppin' EP", "bpm": 145.0, "key": "12A / Dbm", "subgenre": "Psytrance / Twilight", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/jumpstreet/spirit-overflow", "artwork_url": "", "energy": 0.96, "sub_bass_weight": 0.95, "transient_density": 0.94},
    {"id": "dig_js_02", "title": "Nanodroppin'", "artist": "Jumpstreet", "album": "Nanodroppin' EP", "bpm": 146.0, "key": "12A / Dbm", "subgenre": "Psytrance / Twilight", "provider": "Beatport Pro", "url": "https://www.beatport.com/track/nanodroppin/12345", "artwork_url": "", "energy": 0.98, "sub_bass_weight": 0.97, "transient_density": 0.96},
    {"id": "dig_js_03", "title": "Gourmet Noise", "artist": "Jumpstreet & Braincell", "album": "Gourmet Noise EP", "bpm": 145.0, "key": "1A / Abm", "subgenre": "Full-On Night", "provider": "Bandcamp", "url": "https://sangomarecords.bandcamp.com/track/gourmet-noise", "artwork_url": "", "energy": 0.95, "sub_bass_weight": 0.94, "transient_density": 0.93},
    {"id": "dig_js_04", "title": "Mind F**k", "artist": "Jumpstreet", "album": "Analogic Modulation", "bpm": 144.0, "key": "12B / E", "subgenre": "Psytrance / Twilight", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/jumpstreet/mind-fk", "artwork_url": "", "energy": 0.93, "sub_bass_weight": 0.92, "transient_density": 0.95},
    {"id": "dig_js_05", "title": "MotherFking Jazz", "artist": "Jumpstreet & OxyFlux", "album": "Northern Mythologies", "bpm": 146.0, "key": "12A / Dbm", "subgenre": "Psytrance / Twilight", "provider": "Beatport Pro", "url": "https://www.beatport.com/track/motherfking-jazz/1456", "artwork_url": "", "energy": 0.97, "sub_bass_weight": 0.96, "transient_density": 0.98},
    {"id": "dig_js_06", "title": "Quantum Resonator", "artist": "Wubba Lubba & Jumpstreet", "album": "Quantum Resonator Single", "bpm": 140.0, "key": "7A / Dm", "subgenre": "Psytrance", "provider": "Spotify Sync", "url": "spotify:track:4uLU6hMCjMI75M1A2tKUQC", "artwork_url": "", "energy": 0.94, "sub_bass_weight": 0.93, "transient_density": 0.92},
    {"id": "dig_js_07", "title": "Free Time Travel", "artist": "Jumpstreet vs. Ra Root", "album": "Dream Manipulation", "bpm": 140.0, "key": "7A / Dm", "subgenre": "Psytrance", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/jumpstreet/free-time-travel", "artwork_url": "", "energy": 0.92, "sub_bass_weight": 0.91, "transient_density": 0.90},
    {"id": "dig_js_08", "title": "Subatomic", "artist": "Jumpstreet", "album": "Pirate Cove Day 1", "bpm": 140.0, "key": "11A / F#m", "subgenre": "Psytrance", "provider": "YouTube Topic", "url": "https://youtube.com/watch?v=subatomic", "artwork_url": "", "energy": 0.91, "sub_bass_weight": 0.90, "transient_density": 0.89},
    {"id": "dig_js_09", "title": "Vocodeyra", "artist": "Jumpstreet & Pangaea", "album": "Nanodroppin'", "bpm": 140.0, "key": "11B / A", "subgenre": "Full-On", "provider": "Bandcamp", "url": "https://bandcamp.com/jumpstreet/vocodeyra", "artwork_url": "", "energy": 0.90, "sub_bass_weight": 0.89, "transient_density": 0.91},
    {"id": "dig_js_10", "title": "Basis of the Blues", "artist": "Jumpstreet & Braingineers", "album": "The Whole Funk", "bpm": 140.0, "key": "8B / C", "subgenre": "Psytrance", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/jumpstreet/basis-of-the-blues", "artwork_url": "", "energy": 0.89, "sub_bass_weight": 0.91, "transient_density": 0.92},
    
    # Artistas Relacionados e Selos Underground (Tristan, Avalon, Earthling, Diksha, Dust)
    {"id": "dig_rel_01", "title": "Talking in Colours", "artist": "Tristan & Avalon", "album": "Nano Records HD", "bpm": 144.0, "key": "12A / Dbm", "subgenre": "Psytrance / Nano", "provider": "Beatport Pro", "url": "https://beatport.com/track/talking-in-colours", "artwork_url": "", "energy": 0.97, "sub_bass_weight": 0.96, "transient_density": 0.95},
    {"id": "dig_rel_02", "title": "Liquid Crystal", "artist": "Diksha", "album": "Sangoma Night", "bpm": 146.0, "key": "1A / Abm", "subgenre": "Twilight / Night", "provider": "Bandcamp", "url": "https://sangomarecords.bandcamp.com/track/liquid-crystal", "artwork_url": "", "energy": 0.98, "sub_bass_weight": 0.98, "transient_density": 0.97},
    {"id": "dig_rel_03", "title": "Sonic Alchemy", "artist": "Dust & Loom", "album": "Looney Moon Sessions", "bpm": 145.0, "key": "12A / Dbm", "subgenre": "Psytrance / Twilight", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/looneymoon/sonic-alchemy", "artwork_url": "", "energy": 0.95, "sub_bass_weight": 0.95, "transient_density": 0.94},
    {"id": "dig_rel_04", "title": "Multiverse Resonance", "artist": "Earthling", "album": "Zero1 Music", "bpm": 143.0, "key": "7A / Dm", "subgenre": "Psytrance", "provider": "Beatport Pro", "url": "https://beatport.com/track/multiverse-resonance", "artwork_url": "", "energy": 0.93, "sub_bass_weight": 0.92, "transient_density": 0.91},
    {"id": "dig_rel_05", "title": "Neuro-Glitch Protocol", "artist": "Fungus Funk", "album": "Sangoma Explorations", "bpm": 146.0, "key": "12B / E", "subgenre": "Twilight", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/fungusfunk/neuro-glitch", "artwork_url": "", "energy": 0.96, "sub_bass_weight": 0.95, "transient_density": 0.96},
    {"id": "dig_rel_06", "title": "Organic Frequency", "artist": "Atriohm", "album": "Parvati Records", "bpm": 145.0, "key": "1A / Abm", "subgenre": "Dark Psy / Forest", "provider": "Bandcamp", "url": "https://parvatirecords.bandcamp.com/track/organic-freq", "artwork_url": "", "energy": 0.94, "sub_bass_weight": 0.97, "transient_density": 0.93},
    {"id": "dig_rel_07", "title": "Alien Artifact", "artist": "E-Clip", "album": "Iboga Master", "bpm": 138.0, "key": "7A / Dm", "subgenre": "Prog Psy", "provider": "Beatport Pro", "url": "https://beatport.com/track/alien-artifact", "artwork_url": "", "energy": 0.91, "sub_bass_weight": 0.94, "transient_density": 0.88},
    {"id": "dig_rel_08", "title": "Dimensional Rift", "artist": "Ace Ventura & Astrix", "album": "Alpha Portal Live", "bpm": 142.0, "key": "7A / Dm", "subgenre": "Psytrance", "provider": "Spotify Sync", "url": "spotify:track:dimensionalrift", "artwork_url": "", "energy": 0.96, "sub_bass_weight": 0.96, "transient_density": 0.95},
    {"id": "dig_rel_09", "title": "Wired Dimensions", "artist": "Module Virus", "album": "Bom Shanka Music", "bpm": 147.0, "key": "12A / Dbm", "subgenre": "Twilight", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/modulevirus/wired", "artwork_url": "", "energy": 0.98, "sub_bass_weight": 0.96, "transient_density": 0.97},
    {"id": "dig_rel_10", "title": "Zero Gravity", "artist": "Kabayun", "album": "Sangoma Records", "bpm": 146.0, "key": "12A / Dbm", "subgenre": "Twilight", "provider": "Bandcamp", "url": "https://sangomarecords.bandcamp.com/track/zero-gravity", "artwork_url": "", "energy": 0.97, "sub_bass_weight": 0.97, "transient_density": 0.95},

    # Techno / Melodic / Peak Time Underground Seeds
    {"id": "dig_tc_01", "title": "Age of Love (Charlotte de Witte & Enrico Sangiuliano Remix)", "artist": "Age of Love", "album": "D抜", "bpm": 130.0, "key": "8A / Am", "subgenre": "Peak Time Techno", "provider": "Beatport Pro", "url": "https://beatport.com/track/age-of-love", "artwork_url": "", "energy": 0.95, "sub_bass_weight": 0.96, "transient_density": 0.94},
    {"id": "dig_tc_02", "title": "Universal Consciousness", "artist": "Enrico Sangiuliano", "album": "NINETEENHUNDRED", "bpm": 132.0, "key": "8A / Am", "subgenre": "Techno", "provider": "SoundCloud HQ", "url": "https://soundcloud.com/enrico-sangiuliano/universal", "artwork_url": "", "energy": 0.96, "sub_bass_weight": 0.97, "transient_density": 0.95},
    {"id": "dig_tc_03", "title": "Dark Matter", "artist": "Rebūke", "album": "Drumcode", "bpm": 128.0, "key": "9A / Em", "subgenre": "Peak Time Techno", "provider": "Beatport Pro", "url": "https://beatport.com/track/dark-matter", "artwork_url": "", "energy": 0.94, "sub_bass_weight": 0.95, "transient_density": 0.93},
    {"id": "dig_tc_04", "title": "Consciousness", "artist": "Anyma & Chris Avantgarde", "album": "Afterlife", "bpm": 126.0, "key": "4A / Fm", "subgenre": "Melodic Techno", "provider": "Spotify Sync", "url": "spotify:track:consciousness", "artwork_url": "", "energy": 0.92, "sub_bass_weight": 0.94, "transient_density": 0.90},
]

def seed_database():
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    for t in UNDERGROUND_SEEDS:
        num, let = parse_camelot(t["key"])
        cur.execute("""
        INSERT OR IGNORE INTO tracks 
        (id, title, artist, album, bpm, key_signature, camelot_num, camelot_letter, subgenre, provider, url, artwork_url, energy, sub_bass_weight, transient_density)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            t["id"], t["title"], t["artist"], t["album"], t["bpm"], t["key"],
            num, let, t["subgenre"], t["provider"], t["url"], t["artwork_url"],
            t["energy"], t["sub_bass_weight"], t["transient_density"]
        ))
    conn.commit()
    conn.close()

# ═════════════════════════════════════════════════════════════════════════════
# 4. MOTOR DE RECOMENDAÇÃO ACÚSTICA & APRENDIZADO POR REFORÇO
# ═════════════════════════════════════════════════════════════════════════════

def run_dig_deeper(query, ref_bpm=None, ref_key=None, out_json=None):
    """
    Executa busca reversa acústica a partir de uma faixa de referência,
    combinando proximidade rítmica, compatibilidade Camelot, envelope espectral
    e bônus de aprendizado de transições do DJ.
    """
    init_db()
    seed_database()
    
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    
    # Carregar pesos adaptativos do DJ
    cur.execute("SELECT key, val FROM dj_profile_weights")
    weights = dict(cur.fetchall())
    w_groove = weights.get("w_groove", 0.35)
    w_harmonic = weights.get("w_harmonic", 0.35)
    w_bpm = weights.get("w_bpm", 0.20)
    w_learned = weights.get("w_learned", 0.10)
    
    # 1. Resolver referência
    ref_track = None
    cur.execute("SELECT id, title, artist, album, bpm, key_signature, camelot_num, camelot_letter, energy, sub_bass_weight, transient_density FROM tracks WHERE id = ? OR title LIKE ? OR url = ? LIMIT 1", (query, f"%{query}%", query))
    row = cur.fetchone()
    
    if row:
        ref_track = {
            "id": row[0], "title": row[1], "artist": row[2], "album": row[3],
            "bpm": row[4], "key": row[5], "c_num": row[6], "c_let": row[7],
            "energy": row[8], "sub_bass": row[9], "transients": row[10]
        }
    else:
        # Extrair parâmetros heurísticos da consulta
        bpm = float(ref_bpm) if ref_bpm and float(ref_bpm) > 20 else 145.0
        key_str = ref_key or "12A / Dbm"
        c_num, c_let = parse_camelot(key_str)
        ref_track = {
            "id": f"ref_{int(time.time())}",
            "title": query, "artist": "Artista Referência", "album": "Single",
            "bpm": bpm, "key": key_str, "c_num": c_num, "c_let": c_let,
            "energy": 0.95, "sub_bass": 0.94, "transients": 0.92
        }
        
    ref_num = ref_track["c_num"]
    ref_let = ref_track["c_let"]
    ref_bpm = ref_track["bpm"]
    ref_id = ref_track["id"]
    
    # 2. Carregar candidatos da base
    cur.execute("""
    SELECT id, title, artist, album, bpm, key_signature, camelot_num, camelot_letter,
           subgenre, provider, url, artwork_url, energy, sub_bass_weight, transient_density
    FROM tracks
    WHERE id != ?
    """, (ref_id,))
    candidates = cur.fetchall()
    
    # Carregar conexões de afinidade aprendida do grafo
    cur.execute("SELECT target_id, weight, play_count FROM affinity_edges WHERE ref_id = ?", (ref_id,))
    learned_edges = {row[0]: (row[1], row[2]) for row in cur.fetchall()}
    
    results = []
    for c in candidates:
        cid, c_title, c_artist, c_alb, c_bpm, c_key, c_num, c_let, c_genre, c_prov, c_url, c_art, c_energy, c_sub, c_trans = c
        
        # A. Compatibilidade Harmônica Camelot
        h_score, h_rel, h_tag = calc_harmonic_compatibility(ref_num, ref_let, c_num, c_let)
        
        # B. Proximidade de BPM (Drift percentual)
        bpm_diff = abs(c_bpm - ref_bpm)
        bpm_drift_pct = (c_bpm - ref_bpm) / ref_bpm * 100.0
        drift_ratio = abs(c_bpm - ref_bpm) / ref_bpm
        if drift_ratio < 0.015:
            b_score = 1.0
        elif drift_ratio < 0.04:
            b_score = 0.85
        elif drift_ratio < 0.07:
            b_score = 0.65
        else:
            b_score = max(0.1, 1.0 - drift_ratio * 4.0)
            
        # C. Similaridade Acústica / Tímbrica (Sub-bass, energia, transientes)
        diff_energy = abs(c_energy - ref_track["energy"])
        diff_sub = abs(c_sub - ref_track["sub_bass"])
        diff_trans = abs(c_trans - ref_track["transients"])
        acoustic_dist = (diff_energy * 0.4 + diff_sub * 0.4 + diff_trans * 0.2)
        timbre_score = max(0.2, 1.0 - acoustic_dist)
        
        # D. Bônus de Aprendizado do Grafo (Experiência do DJ)
        edge_weight, edge_plays = learned_edges.get(cid, (1.0, 0))
        # Normaliza o bônus: > 1.0 soma, < 1.0 subtrai
        learned_bonus = (edge_weight - 1.0) * 0.15
        
        # Pontuação Final Ponderada
        final_score = (
            w_harmonic * h_score +
            w_bpm * b_score +
            w_groove * timbre_score +
            w_learned * min(1.0, max(0.0, edge_weight / 2.0))
        ) + learned_bonus
        
        final_score = min(0.999, max(0.50, final_score))
        match_pct = int(round(final_score * 100.0))
        groove_pct = int(round(timbre_score * 100.0))
        
        # Selo característico
        match_badge = "ALMA GÊMEA TÍMBRICA"
        if h_score == 1.0 and abs(drift_ratio) < 0.01:
            match_badge = "TRANSIÇÃO DE OURO (GÊMEA)"
        elif edge_plays > 0:
            match_badge = f"FAVORITA DO DJ ({edge_plays}x TOCADA)"
        elif h_rel == "SUBIDA (+1)":
            match_badge = "SUBIDA DE ENERGIA PERFEITA"
        elif h_rel == "RELATIVO":
            match_badge = "MUDANÇA DE CLIMA HARMÔNICA"
            
        results.append({
            "id": cid,
            "title": c_title,
            "artist": c_artist,
            "album": c_alb,
            "bpm": c_bpm,
            "bpm_drift_pct": round(bpm_drift_pct, 2),
            "key": c_key,
            "camelot_code": f"{c_num}{c_let}",
            "harmonic_relation": h_rel,
            "harmonic_tag": h_tag,
            "subgenre": c_genre,
            "provider": c_prov,
            "url": c_url,
            "artwork_url": c_art,
            "acoustic_match_pct": match_pct,
            "groove_match_pct": groove_pct,
            "match_badge": match_badge,
            "play_count": edge_plays
        })
        
    conn.close()
    
    # Ordenar por percentual de match acústico decrescente
    results.sort(key=lambda x: (x["acoustic_match_pct"], x["groove_match_pct"]), reverse=True)
    
    out_data = {
        "status": "success",
        "reference": ref_track,
        "total_results": len(results),
        "results": results[:50] # Top 50 almas gêmeas
    }
    
    if out_json:
        with open(out_json, "w", encoding="utf-8") as f:
            json.dump(out_data, f, indent=2, ensure_ascii=False)
        print(f"[DIG_DEEPER] Sucesso: {len(results)} faixas geradas em {out_json}")
        
    return out_data

# ═════════════════════════════════════════════════════════════════════════════
# 5. REGISTRO DE FEEDBACK DO DJ (REFORÇO ADAPTATIVO CONTÍNUO)
# ═════════════════════════════════════════════════════════════════════════════

def record_dj_feedback(ref_id, target_id, action, deck=0):
    """
    Alimenta a base de conhecimento a cada ação do DJ.
    Ações suportadas:
    - 'load_deck': O DJ carregou a sugestão no Deck (+1.5 peso)
    - 'beatmatch_sync': O DJ mixou com sync ativado (+2.0 peso)
    - 'audition_long': O DJ ouviu mais de 20s no Audition Dock (+0.6 peso)
    - 'like': O DJ clicou em curtir / reforço positivo (+1.8 peso)
    - 'skip_fast': O DJ pulou em menos de 4s (-0.7 peso)
    - 'blacklist': O DJ rejeitou (-2.5 peso)
    """
    init_db()
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    
    deltas = {
        "load_deck": 1.5,
        "beatmatch_sync": 2.0,
        "audition_long": 0.6,
        "like": 1.8,
        "skip_fast": -0.7,
        "blacklist": -2.5
    }
    delta = deltas.get(action, 0.5)
    
    # Registrar log
    cur.execute("""
    INSERT INTO dj_feedback_log (ref_id, target_id, action, weight_delta, deck)
    VALUES (?, ?, ?, ?, ?)
    """, (ref_id, target_id, action, delta, deck))
    
    # Atualizar ou criar aresta no grafo
    cur.execute("SELECT weight, play_count, success_count FROM affinity_edges WHERE ref_id = ? AND target_id = ?", (ref_id, target_id))
    row = cur.fetchone()
    if row:
        new_w = max(0.1, min(5.0, row[0] + delta))
        new_plays = row[1] + (1 if "deck" in action else 0)
        new_succ = row[2] + (1 if delta > 0 else 0)
        cur.execute("""
        UPDATE affinity_edges 
        SET weight = ?, play_count = ?, success_count = ?, last_interacted_at = CURRENT_TIMESTAMP
        WHERE ref_id = ? AND target_id = ?
        """, (new_w, new_plays, new_succ, ref_id, target_id))
    else:
        init_w = max(0.1, min(5.0, 1.0 + delta))
        cur.execute("""
        INSERT INTO affinity_edges (ref_id, target_id, weight, play_count, success_count)
        VALUES (?, ?, ?, ?, ?)
        """, (ref_id, target_id, init_w, 1 if "deck" in action else 0, 1 if delta > 0 else 0))
        
    conn.commit()
    conn.close()
    print(f"[FEEDBACK] Reforço registrado: {ref_id} -> {target_id} [{action}, delta={delta}]")

# ═════════════════════════════════════════════════════════════════════════════
# 6. CLI & TESTES
# ═════════════════════════════════════════════════════════════════════════════

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Dig Deeper Acoustic Engine")
    subparsers = parser.add_subparsers(dest="cmd")
    
    p_dig = subparsers.add_parser("dig")
    p_dig.add_argument("--query", required=True, help="Nome da faixa de referência, ID ou URL")
    p_dig.add_argument("--bpm", type=float, default=None, help="BPM da referência")
    p_dig.add_argument("--key", default=None, help="Tom Camelot da referência (ex: 12A)")
    p_dig.add_argument("--out", default=None, help="Caminho do arquivo JSON de saída")
    
    p_fb = subparsers.add_parser("feedback")
    p_fb.add_argument("--ref", required=True, help="ID da faixa de referência")
    p_fb.add_argument("--target", required=True, help="ID da faixa candidata")
    p_fb.add_argument("--action", required=True, choices=["load_deck", "beatmatch_sync", "audition_long", "like", "skip_fast", "blacklist"])
    p_fb.add_argument("--deck", type=int, default=0, help="Deck de destino (0..3)")
    
    args = parser.parse_args()
    
    if args.cmd == "dig":
        out_p = args.out or os.path.join(SCRATCH_DIR, "tracks", "dig_deeper_results.json")
        res = run_dig_deeper(args.query, args.bpm, args.key, out_p)
        print(f"DIG_DEEPER_OK: {res['total_results']} faixas semelhantes ranqueadas.")
    elif args.cmd == "feedback":
        record_dj_feedback(args.ref, args.target, args.action, args.deck)
    else:
        # Teste padrão interativo
        init_db()
        seed_database()
        out_p = os.path.join(SCRATCH_DIR, "tracks", "dig_deeper_results.json")
        res = run_dig_deeper("Spirit Overflow", 145.0, "12A / Dbm", out_p)
        print(f"Teste concluído: {res['total_results']} resultados salvos em {out_p}")

