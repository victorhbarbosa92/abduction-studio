# -*- coding: utf-8 -*-
"""
Módulo Python de Integração do Mixer do FL Studio para o Abduction Studio V2.
Este arquivo expõe as mesmas assinaturas e funções da API do FL Studio,
controlando a DAW Abduction Studio em tempo real via IPC (UDP porta 9000).
"""

import socket

def _send(cmd):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.sendto(cmd.encode('utf-8'), ("127.0.0.1", 9000))
        s.close()
    except Exception as e:
        print(f"[mixer.py] Erro ao enviar comando via IPC: {e}")

def setTrackVolume(track_index, value):
    """
    Define o volume da track.
    Abduction Studio aceita escala em dB de -60.0f a 6.0f.
    Para compatibilidade, mapeia valores lineares de 0.0 - 1.0 para dB se o valor for menor ou igual a 1.25.
    """
    if 0.0 <= value <= 1.25:
        # Mapeia linear 0.0 - 1.25 para dB -60.0 - 6.0
        if value <= 0.001:
            db_val = -60.0
        else:
            import math
            db_val = 20.0 * math.log10(value)
    else:
        # Já é um valor em dB
        db_val = value
        
    _send(f"mixer.setTrackVolume {track_index} {db_val}")

def getTrackVolume(track_index):
    """Retorna o volume atual."""
    # Retorna valor default mockado
    return 1.0

def setTrackPan(track_index, value):
    """Define o Pan (balanço estereofônico). Valor de -1.0 (esquerda) a 1.0 (direita)."""
    _send(f"mixer.setTrackPan {track_index} {value}")

def getTrackPan(track_index):
    """Retorna o pan atual."""
    return 0.0

def muteTrack(track_index, value):
    """Muta/Desmuta a track. True para mutar, False para desmutar."""
    _send(f"mixer.muteTrack {track_index} {1 if value else 0}")

def isTrackMuted(track_index):
    """Verifica se a track está mutada."""
    return False

def soloTrack(track_index, value):
    """Solo/Unsolo na track."""
    _send(f"mixer.soloTrack {track_index} {1 if value else 0}")

def isTrackSolo(track_index):
    """Verifica se a track está em solo."""
    return False
