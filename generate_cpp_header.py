# -*- coding: utf-8 -*-
import sqlite3
import os

DB_PATH = "daw_apis.db"
HEADER_PATH = "src/core/DawApiData.h"

def escape_cpp_string(s):
    if s is None:
        return ""
    # Escapa barra invertida, aspas duplas, novas linhas e tabulações
    return s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n').replace('\t', '\\t').replace('\r', '')

def main():
    if not os.path.exists(DB_PATH):
        print(f"Erro: {DB_PATH} não encontrado!")
        return
        
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    # Query para extrair todas as informações de API de forma planificada
    query = """
        SELECT 
            d.name AS daw_name,
            c.name AS container_name,
            c.type AS container_type,
            f.name AS func_name,
            f.item_type,
            f.signature,
            f.return_type,
            f.description,
            f.category
        FROM functions_properties f
        JOIN modules_classes c ON f.container_id = c.id
        JOIN daws d ON c.daw_id = d.id
        ORDER BY d.name, c.name, f.name;
    """
    
    cursor.execute(query)
    rows = cursor.fetchall()
    
    print(f"Total de registros encontrados: {len(rows)}")
    
    header_content = """#pragma once
#include <string>
#include <vector>

namespace KuroAI {
    struct ApiItem {
        std::string daw;
        std::string container;      // Nome do Módulo ou Classe
        std::string container_type; // 'Module' ou 'Class'
        std::string name;
        std::string item_type;      // 'Function', 'Method' ou 'Property'
        std::string signature;
        std::string return_type;
        std::string description;
        std::string category;
    };

    inline const std::vector<ApiItem>& GetDawApiData() {
        static const std::vector<ApiItem> data = {
"""
    
    for row in rows:
        daw_name = escape_cpp_string(row[0])
        container_name = escape_cpp_string(row[1])
        container_type = escape_cpp_string(row[2])
        func_name = escape_cpp_string(row[3])
        item_type = escape_cpp_string(row[4])
        signature = escape_cpp_string(row[5])
        return_type = escape_cpp_string(row[6])
        description = escape_cpp_string(row[7])
        category = escape_cpp_string(row[8])
        
        item_line = f'            {{"{daw_name}", "{container_name}", "{container_type}", "{func_name}", "{item_type}", "{signature}", "{return_type}", "{description}", "{category}"}},\n'
        header_content += item_line
        
    header_content += """        };
        return data;
    }
}
"""
    
    os.makedirs(os.path.dirname(HEADER_PATH), exist_ok=True)
    with open(HEADER_PATH, "w", encoding="utf-8") as f:
        f.write(header_content)
        
    conn.close()
    print(f"Cabeçalho C++ gerado com sucesso em: {HEADER_PATH}")

if __name__ == "__main__":
    main()
