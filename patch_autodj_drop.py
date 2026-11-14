with open('src/main.cpp', 'r', encoding='utf-8') as f:
    c = f.read()

target = '''                auto& autodj = KuroAudio::DJAutoDJEngine::getInstance();
                autodj.setEnabled(true);
                autodj.setTransitionLengthBars(32);
                autodj.setBassSwapEnabled(true);
            }'''

repl = '''                auto& autodj = KuroAudio::DJAutoDJEngine::getInstance();
                autodj.setEnabled(true);
                autodj.setTransitionLengthBars(32);
                autodj.setBassSwapEnabled(true);
                g_dj_engine->deckB.current_frame = 36.0 * g_dj_engine->deckB.sample_rate;
                g_dj_engine->deckB.is_playing.store(true);
                g_dj_engine->deckB.volume = 1.0f;
                g_dj_engine->deckB.eq_low = 1.0f;
                g_dj_engine->deckA.eq_low = 0.0f;
                g_dj_engine->setCrossfader(0.5f);
            }'''

if target in c:
    c = c.replace(target, repl, 1)
    with open('src/main.cpp', 'w', encoding='utf-8') as f:
        f.write(c)
    print("SUCCESS")
else:
    print("TARGET NOT FOUND")
