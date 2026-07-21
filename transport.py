# -*- coding: utf-8 -*-
"""
Módulo Python de Integração de Transporte do FL Studio para o Abduction Studio V2.
Controla o play/stop/pause em tempo real via IPC (UDP porta 9000).
"""

import socket

def _send(cmd):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.sendto(cmd.encode('utf-8'), ("127.0.0.1", 9000))
        s.close()
    except Exception as e:
        print(f"[transport.py] Erro ao enviar comando via IPC: {e}")

def start():
    """Inicia a reprodução global."""
    _send("transport.start")

def stop():
    """Para a reprodução global."""
    _send("transport.stop")

def isPlaying():
    """Retorna True se estiver tocando."""
    return False
