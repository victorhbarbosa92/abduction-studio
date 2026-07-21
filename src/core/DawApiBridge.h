#pragma once
#include <string>

namespace DawApiBridge {
    void startBridge();
    void sendLocalUdpCommand(const std::string& cmd);
}

