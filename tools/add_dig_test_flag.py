path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

target = """                if (std::filesystem::exists(pB)) {
                    g_dj_engine->deckB.loadTrack(pB);
                    g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                    g_dj_engine->deckB.track_artist = "Astrix";
                    g_dj_engine->deckB.bpm = 138.0;
                    g_dj_engine->deckB.key_signature = "8A / Am";
                }
            }
        } else if (arg == "--test-preview-audio") {"""

replacement = """                if (std::filesystem::exists(pB)) {
                    g_dj_engine->deckB.loadTrack(pB);
                    g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                    g_dj_engine->deckB.track_artist = "Astrix";
                    g_dj_engine->deckB.bpm = 138.0;
                    g_dj_engine->deckB.key_signature = "8A / Am";
                }
            }
        } else if (arg == "--test-dig-deeper") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_home_dashboard_ui.show_on_startup = false;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.focus_search_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = false;
            if (std::filesystem::exists("scratch/tracks/search_results.tsv")) {
                KuroUI::g_dj_studio_ui.spotify_service.parseSearchResultsTsv("scratch/tracks/search_results.tsv");
            }
            if (std::filesystem::exists("scratch/tracks/dig_deeper_results.json")) {
                KuroUI::g_dj_studio_ui.spotify_service.parseDigDeeperJsonFile("scratch/tracks/dig_deeper_results.json");
            }
            KuroUI::g_dj_studio_ui.spotify_service.show_dig_deeper_view = true;
        } else if (arg == "--test-preview-audio") {"""

if target in content:
    content = content.replace(target, replacement, 1)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print("Added --test-dig-deeper flag to main.cpp!")
else:
    print("Target not found in main.cpp!")

