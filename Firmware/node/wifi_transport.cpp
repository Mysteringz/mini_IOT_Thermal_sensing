#include "wifi_transport.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include "config.h"
#include "thermal_serializer.h"

namespace {
WiFiUDP g_udp;
uint32_t g_last_connect_attempt_ms = 0;
uint16_t g_packet_sequence = 0;

IPAddress remote_ip(
    WIFI_REMOTE_IP_A,
    WIFI_REMOTE_IP_B,
    WIFI_REMOTE_IP_C,
    WIFI_REMOTE_IP_D
);
} // namespace

void wifi_transport_init() {
    if (!WIFI_TELEMETRY_ENABLED) {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    g_last_connect_attempt_ms = millis();
}

void wifi_transport_update() {
    if (!WIFI_TELEMETRY_ENABLED) {
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    const uint32_t now = millis();
    if (now - g_last_connect_attempt_ms < WIFI_RECONNECT_INTERVAL_MS) {
        return;
    }

    g_last_connect_attempt_ms = now;
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool wifi_transport_send_packet(const ThermalProcessor* processor) {
    if (!WIFI_TELEMETRY_ENABLED || WiFi.status() != WL_CONNECTED) {
        return false;
    }

    FragmentedThermalPacket fragments;
    const uint8_t fragment_count = build_fragmented_packets(processor, g_packet_sequence++, &fragments);

    for (uint8_t fragment_index = 0; fragment_index < fragment_count; ++fragment_index) {
        if (!g_udp.beginPacket(remote_ip, WIFI_REMOTE_PORT)) {
            return false;
        }

        g_udp.write(fragments.data[fragment_index], fragments.sizes[fragment_index]);

        if (g_udp.endPacket() != 1) {
            return false;
        }
    }

    return true;
}
