#include "utils.h"

std::string Utils::formatIp(uint32_t ip) {
    char buffer[16];
    sprintf(buffer, "%d.%d.%d.%d", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
    return buffer;
}
