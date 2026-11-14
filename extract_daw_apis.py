import sqlite3
import json
import os

DB_PATH = "daw_apis.db"

def setup_database():
    print(f"1. Inicializando banco de dados SQLite em: {DB_PATH}")
    if os.path.exists(DB_PATH):
        os.remove(DB_PATH)
        
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    # Habilitar chaves estrangeiras
    cursor.execute("PRAGMA foreign_keys = ON;")
    
    # Tabela 1: DAWs
    cursor.execute("""
        CREATE TABLE daws (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            version TEXT NOT NULL,
            description TEXT
        );
    """)
    
    # Tabela 2: Classes/Módulos
    cursor.execute("""
        CREATE TABLE modules_classes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            daw_id INTEGER NOT NULL,
            type TEXT CHECK(type IN ('Class', 'Module')) NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            FOREIGN KEY (daw_id) REFERENCES daws(id) ON DELETE CASCADE,
            UNIQUE(daw_id, name)
        );
    """)
    
    # Tabela 3: Funções, Métodos e Propriedades
    cursor.execute("""
        CREATE TABLE functions_properties (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            container_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            item_type TEXT CHECK(item_type IN ('Method', 'Property', 'Function')) NOT NULL,
            signature TEXT,
            parameters TEXT, -- Armazenado como JSON string
            return_type TEXT,
            description TEXT,
            category TEXT,
            FOREIGN KEY (container_id) REFERENCES modules_classes(id) ON DELETE CASCADE,
            UNIQUE(container_id, name, item_type)
        );
    """)
    
    # Tabela 4: Parâmetros padrão de plugins conhecidos
    cursor.execute("""
        CREATE TABLE plugin_parameters (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            daw_id INTEGER NOT NULL,
            plugin_name TEXT NOT NULL,
            param_name TEXT NOT NULL,
            min_val REAL DEFAULT 0.0,
            max_val REAL DEFAULT 1.0,
            default_val REAL,
            unit TEXT,
            FOREIGN KEY (daw_id) REFERENCES daws(id) ON DELETE CASCADE,
            UNIQUE(daw_id, plugin_name, param_name)
        );
    """)
    
    conn.commit()
    return conn

def populate_data(conn):
    print("2. Populando tabelas com dados técnicos de Ableton Live e FL Studio...")
    cursor = conn.cursor()
    
    # Inserir DAWs
    cursor.execute("INSERT INTO daws (name, version, description) VALUES (?, ?, ?)", 
                   ("Ableton Live", "12.0", "Live Object Model (LOM) Python API para Scripts de Controle"))
    ableton_id = cursor.lastrowid
    
    cursor.execute("INSERT INTO daws (name, version, description) VALUES (?, ?, ?)", 
                   ("FL Studio", "21.2", "FL Studio MIDI Scripting Python API para Controladores de Hardware"))
    fl_id = cursor.lastrowid
    
    # --- ABLETON LIVE: Classes ---
    lom_classes = [
        ("Song", "Class", "Representa o set atual (projeto), controlando tempo, transporte, tracks e cenas."),
        ("Track", "Class", "Representa uma pista de áudio ou MIDI no Ableton Live."),
        ("Device", "Class", "Representa um dispositivo ou instrumento em uma pista (ex: Serum, AutoFilter)."),
        ("DeviceParameter", "Class", "Representa um parâmetro automatizável dentro de um Device."),
        ("Clip", "Class", "Representa um clipe MIDI ou de áudio em um slot de cena.")
    ]
    
    class_ids = {}
    for name, type_str, desc in lom_classes:
        cursor.execute("INSERT INTO modules_classes (daw_id, type, name, description) VALUES (?, ?, ?, ?)",
                       (ableton_id, type_str, name, desc))
        class_ids[name] = cursor.lastrowid
        
    # Ableton Song Members
    song_members = [
        ("tempo", "Property", "float", "O andamento (BPM) global do Live set. Faixa: 20.0 a 999.0.", "Tempo / Master"),
        ("is_playing", "Property", "bool", "Verdadeiro se o playhead global estiver em execução.", "Transporte"),
        ("start_playing", "Method", "None", "Inicia a reprodução global a partir do marcador de início.", "Transporte"),
        ("stop_playing", "Method", "None", "Para a reprodução global.", "Transporte"),
        ("create_track", "Method", "Track", "Cria uma nova pista de áudio ou MIDI no Live set no índice especificado.", "Edição"),
        ("delete_track", "Method", "None", "Remove a pista especificada do set.", "Edição")
    ]
    for name, item_type, ret, desc, cat in song_members:
        cursor.execute("""
            INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (class_ids["Song"], name, item_type, f"{name}()" if item_type == "Method" else None, ret, desc, cat))
        
    # Ableton Track Members
    track_members = [
        ("name", "Property", "str", "O nome legível da pista.", "Identificação"),
        ("mute", "Property", "bool", "Define ou retorna o estado de Mute (Silenciar) da pista.", "Mixer"),
        ("solo", "Property", "bool", "Define ou retorna o estado de Solo da pista.", "Mixer"),
        ("arm", "Property", "bool", "Define se a pista está armada para gravação (somente pistas MIDI/Audio).", "Gravação"),
        ("devices", "Property", "list<Device>", "Retorna a lista de dispositivos carregados na pista.", "Roteamento"),
        ("create_device", "Method", "Device", "Cria/carrega um novo dispositivo no índice especificado.", "Edição")
    ]
    for name, item_type, ret, desc, cat in track_members:
        cursor.execute("""
            INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (class_ids["Track"], name, item_type, f"{name}()" if item_type == "Method" else None, ret, desc, cat))

    # Ableton Device & DeviceParameter Members
    cursor.execute("""
        INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
        VALUES (?, 'name', 'Property', NULL, 'str', 'O nome do dispositivo.', 'Identificação')
    """, (class_ids["Device"],))
    cursor.execute("""
        INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
        VALUES (?, 'parameters', 'Property', NULL, 'list<DeviceParameter>', 'Lista de parâmetros controláveis.', 'Parâmetros')
    """, (class_ids["Device"],))
    cursor.execute("""
        INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
        VALUES (?, 'value', 'Property', NULL, 'float', 'O valor atual do parâmetro na escala interna.', 'Valores')
    """, (class_ids["DeviceParameter"],))
    cursor.execute("""
        INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
        VALUES (?, 'min', 'Property', NULL, 'float', 'O valor mínimo suportado pelo parâmetro.', 'Valores')
    """, (class_ids["DeviceParameter"],))
    cursor.execute("""
        INSERT INTO functions_properties (container_id, name, item_type, signature, return_type, description, category)
        VALUES (?, 'max', 'Property', NULL, 'float', 'O valor máximo suportado pelo parâmetro.', 'Valores')
    """, (class_ids["DeviceParameter"],))

    # --- FL STUDIO: Módulos ---
    fl_modules = [
        ("mixer", "Module", "Controla o Mixer do FL Studio, volumes, efeitos de tracks e barramentos de canais."),
        ("playlist", "Module", "Controla a Arranger/Playlist, mutes de faixas, nomes e posições do projeto."),
        ("channels", "Module", "Controla o Channel Rack do FL Studio, volumes de canais, nomes e seleção."),
        ("transport", "Module", "Controla os comandos globais de reprodução, metrônomo e loop."),
        ("plugins", "Module", "Obtém e define parâmetros de efeitos/instrumentos nos canais ou slots de mixer.")
    ]
    
    module_ids = {}
    for name, type_str, desc in fl_modules:
        cursor.execute("INSERT INTO modules_classes (daw_id, type, name, description) VALUES (?, ?, ?, ?)",
                       (fl_id, type_str, name, desc))
        module_ids[name] = cursor.lastrowid
        
    # FL Studio mixer module functions
    mixer_funcs = [
        ("setTrackVolume", "Function", "mixer.setTrackVolume(track_index, value)", '["track_index", "value"]', "None", "Define o fader de volume da track do mixer. Valor vai de 0.0 (silêncio) a 1.0 (0dB) ou 1.25 (+5.6dB).", "Volume"),
        ("getTrackVolume", "Function", "mixer.getTrackVolume(track_index)", '["track_index"]', "float", "Retorna o volume atual da track do mixer.", "Volume"),
        ("setTrackPan", "Function", "mixer.setTrackPan(track_index, value)", '["track_index", "value"]', "None", "Define o Pan (balanço estereofônico). Valor de -1.0 (total esquerda) a 1.0 (total direita).", "Panning"),
        ("muteTrack", "Function", "mixer.muteTrack(track_index, value)", '["track_index", "value"]', "None", "Silencia/Desmuta a track do mixer.", "Estado")
    ]
    for name, item_type, sig, params, ret, desc, cat in mixer_funcs:
        cursor.execute("""
            INSERT INTO functions_properties (container_id, name, item_type, signature, parameters, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, (module_ids["mixer"], name, item_type, sig, params, ret, desc, cat))

    # FL Studio playlist module functions
    playlist_funcs = [
        ("muteTrack", "Function", "playlist.muteTrack(track_index)", '["track_index"]', "None", "Muta/Desmuta a faixa correspondente na Playlist do arranjo.", "Estado"),
        ("getTrackName", "Function", "playlist.getTrackName(track_index)", '["track_index"]', "str", "Retorna o nome da faixa na Playlist no índice especificado.", "Identificação")
    ]
    for name, item_type, sig, params, ret, desc, cat in playlist_funcs:
        cursor.execute("""
            INSERT INTO functions_properties (container_id, name, item_type, signature, parameters, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, (module_ids["playlist"], name, item_type, sig, params, ret, desc, cat))

    # FL Studio channels module functions
    channels_funcs = [
        ("getChannelName", "Function", "channels.getChannelName(channel_index)", '["channel_index"]', "str", "Retorna o nome de identificação do canal no Channel Rack.", "Identificação"),
        ("showEditor", "Function", "channels.showEditor(channel_index, show)", '["channel_index", "show"]', "None", "Abre ou fecha a janela do editor de plugin/sampler do canal.", "Interface")
    ]
    for name, item_type, sig, params, ret, desc, cat in channels_funcs:
        cursor.execute("""
            INSERT INTO functions_properties (container_id, name, item_type, signature, parameters, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, (module_ids["channels"], name, item_type, sig, params, ret, desc, cat))

    # FL Studio plugins module functions
    plugins_funcs = [
        ("getParamValue", "Function", "plugins.getParamValue(value, index, param)", '["value", "index", "param"]', "float", "Retorna o valor cru de um parâmetro de plugin.", "Parâmetros"),
        ("setParamValue", "Function", "plugins.setParamValue(value, index, param)", '["value", "index", "param"]', "None", "Define o valor de um parâmetro de plugin em tempo real.", "Parâmetros")
    ]
    for name, item_type, sig, params, ret, desc, cat in plugins_funcs:
        cursor.execute("""
            INSERT INTO functions_properties (container_id, name, item_type, signature, parameters, return_type, description, category)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, (module_ids["plugins"], name, item_type, sig, params, ret, desc, cat))

    # --- Tabela 4: Parâmetros de Plugins Comuns ---
    plugins_params = [
        (ableton_id, "AutoFilter", "Cutoff", 20.0, 20000.0, 1000.0, "Hz"),
        (ableton_id, "AutoFilter", "Resonance", 0.0, 1.0, 0.3, "%"),
        (fl_id, "Fruity Delay 3", "Delay Time", 0.0, 1000.0, 250.0, "ms"),
        (fl_id, "Fruity Limiter", "Gain", -24.0, 24.0, 0.0, "dB"),
        (fl_id, "Fruity Parametric EQ 2", "Freq Band 4", 20.0, 20000.0, 1000.0, "Hz")
    ]
    for daw_id, plug, param, mn, mx, df, unit in plugins_params:
        cursor.execute("""
            INSERT INTO plugin_parameters (daw_id, plugin_name, param_name, min_val, max_val, default_val, unit)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (daw_id, plug, param, mn, mx, df, unit))

    conn.commit()
    print("Sucesso! Banco de dados populado.")

def run_tests(conn):
    print("3. Executando testes de integridade e consultas complexas no Banco de Dados...")
    cursor = conn.cursor()
    
    # Teste 1: Quantidade total de registros
    cursor.execute("SELECT COUNT(*) FROM daws;")
    print(f"   [Teste 1] Total de DAWs: {cursor.fetchone()[0]}")
    
    cursor.execute("SELECT COUNT(*) FROM modules_classes;")
    print(f"   [Teste 1] Total de Módulos/Classes: {cursor.fetchone()[0]}")
    
    cursor.execute("SELECT COUNT(*) FROM functions_properties;")
    print(f"   [Teste 1] Total de Funções/Métodos/Propriedades: {cursor.fetchone()[0]}")
    
    # Teste 2: Consultar todos os métodos relacionados a Volume/Mixer do FL Studio
    print("\n   [Teste 2] Métodos de Volume/Mixer do FL Studio:")
    cursor.execute("""
        SELECT mc.name, fp.name, fp.signature, fp.description 
        FROM functions_properties fp
        JOIN modules_classes mc ON fp.container_id = mc.id
        JOIN daws d ON mc.daw_id = d.id
        WHERE d.name = 'FL Studio' AND fp.category IN ('Volume', 'Panning', 'Mixer');
    """)
    for row in cursor.fetchall():
        print(f"      - {row[0]}.{row[1]} -> Assinatura: {row[2]} | Desc: {row[3]}")
        
    # Teste 3: Consultar propriedades da classe Song do Ableton Live
    print("\n   [Teste 3] Propriedades e Métodos do Song do Ableton Live:")
    cursor.execute("""
        SELECT fp.name, fp.item_type, fp.return_type, fp.description 
        FROM functions_properties fp
        JOIN modules_classes mc ON fp.container_id = mc.id
        JOIN daws d ON mc.daw_id = d.id
        WHERE d.name = 'Ableton Live' AND mc.name = 'Song';
    """)
    for row in cursor.fetchall():
        print(f"      - [{row[1]}] {row[0]} -> Retorno: {row[2]} | Desc: {row[3]}")
        
    # Teste 4: Verificar integridade de chave estrangeira (Cascade Delete)
    print("\n   [Teste 4] Validando Cascade Delete e Integridade de Chaves...")
    # Tenta deletar Ableton Live e verifica se classes e propriedades associadas somem
    cursor.execute("SELECT id FROM daws WHERE name = 'Ableton Live';")
    ab_id = cursor.fetchone()[0]
    cursor.execute("DELETE FROM daws WHERE id = ?;", (ab_id,))
    
    cursor.execute("SELECT COUNT(*) FROM modules_classes WHERE daw_id = ?;", (ab_id,))
    rem_mc = cursor.fetchone()[0]
    print(f"      Classes restantes do Ableton após delete: {rem_mc} (Esperado: 0)")
    
    # Desfazer transação do delete de teste
    conn.rollback()
    print("   [Teste 4] Transação revertida com sucesso.")

if __name__ == "__main__":
    connection = setup_database()
    try:
        populate_data(connection)
        run_tests(connection)
    finally:
        connection.close()
        print("\nRotina concluída com sucesso! daw_apis.db gerado e validado.")
