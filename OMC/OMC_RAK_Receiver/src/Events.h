#pragma once



struct SystemEvent {
    EventType type;
    // Optional parameters
    uint8_t r;
    uint8_t g;
    uint8_t b;
    double lat;
    double lon;
};
