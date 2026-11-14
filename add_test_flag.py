import subprocess

target = 'src\\main.cpp'
temp = 'src\\main.cpp.tmp'

with open(target, 'r', encoding='utf-8') as f:
    content = f.read()

flag_hook = '''        } else if (arg == "--test-sources-modal") {'''

flag_code = '''        } else if (arg == "--test-master-source-matrix") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.search_modal_view = 1;
            KuroUI::g_dj_studio_ui.selected_source_track_id = "1891678034";
            KuroUI::g_dj_studio_ui.selected_source_track_title = "Paraiso De Goa";
            KuroUI::g_dj_studio_ui.selected_source_track_artist = "JOX (BR)";
            KuroUI::g_dj_studio_ui.selected_source_track_album = "Psychedelic Experience Vol. 2";
            KuroUI::g_dj_studio_ui.selected_source_track_bpm = 138.0f;
            KuroUI::g_dj_studio_ui.selected_source_track_key = "8A / Am";
            KuroUI::g_dj_studio_ui.selected_source_track_subgenre = "Full-On Night";
            KuroUI::g_dj_studio_ui.selected_source_track_dur = 434.0;
            KuroUI::g_dj_studio_ui.spotify_service.searchSourcesAsync("1891678034", "Paraiso De Goa", "JOX (BR)");
            KuroUI::g_dj_studio_ui.spotify_service.parseSourcesJsonFile("scratch/tracks/sources_1891678034.json");
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->deckA.track_artist = "Aura Vortex";
                g_dj_engine->deckA.bpm = 138.0;
                g_dj_engine->deckA.key_signature = "9B / G";

                std::string pB = "scratch/tracks/track_demo_djw.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_astrix.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                g_dj_engine->deckB.track_artist = "Astrix";
                g_dj_engine->deckB.bpm = 138.0;
                g_dj_engine->deckB.key_signature = "8A / Am";
            }
        } else if (arg == "--test-sources-modal") {'''

if flag_hook in content and '--test-master-source-matrix' not in content:
    content = content.replace(flag_hook, flag_code, 1)
    with open(temp, 'w', encoding='utf-8') as f:
        f.write(content)
    res = subprocess.run(['cmd', '/c', 'move', '/y', temp, target], capture_output=True, text=True)
    print('Added --test-master-source-matrix flag:', res.returncode, res.stdout.strip())
else:
    print('Already present or hook not found')
