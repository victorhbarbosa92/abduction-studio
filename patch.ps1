$c = Get-Content src\main.cpp -Raw
$c = $c -replace 'RtAudio adc\(RtAudio::WINDOWS_WASAPI\);', "try {`n        RtAudio adc(RtAudio::WINDOWS_WASAPI);"
$c = $c -replace 'return 0;\r?\n\}', "    } catch(const std::exception& e) { std::ofstream err(`"debug_kuro.txt`", std::ios::app); err << `"[ERROR] Exception: `" << e.what() << `"\n`"; err.flush(); } catch(...) { std::ofstream err(`"debug_kuro.txt`", std::ios::app); err << `"[ERROR] Desconhecido`\n`"; err.flush(); }`n    return 0;`n}"
Set-Content src\main.cpp $c
