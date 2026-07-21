# -*- coding: utf-8 -*-
"""
Módulo Python de Integração do Ableton Live (LOM) para o Abduction Studio V2.
Este arquivo simula o comportamento da biblioteca 'Live' do Ableton,
controlando a DAW Abduction Studio em tempo real via IPC (UDP porta 9000).
"""

import socket

def _send(cmd):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.sendto(cmd.encode('utf-8'), ("127.0.0.1", 9000))
        s.close()
    except Exception as e:
        print(f"[Live.py] Erro ao enviar comando via IPC: {e}")

class Track:
    def __init__(self, index):
        self.index = index

    @property
    def mute(self):
        return False

    @mute.setter
    def mute(self, value):
        _send(f"Track.setMute {self.index} {1 if value else 0}")

    @property
    def solo(self):
        return False

    @solo.setter
    def solo(self, value):
        _send(f"Track.setSolo {self.index} {1 if value else 0}")

class Song:
    def __init__(self):
        pass

    @property
    def tempo(self):
        return 120.0

    @tempo.setter
    def tempo(self, value):
        _send(f"Song.tempo {value}")

    @property
    def is_playing(self):
        return False

    def start_playing(self):
        _send("Song.start_playing")

    def stop_playing(self):
        _send("Song.stop_playing")

    def get_track(self, index):
        return Track(index)

class LiveApp:
    def __init__(self):
        pass

    def song(self):
        return Song()

# Singleton do Ableton LOM
def get_app():
    return LiveApp()
