#include "thermal_serializer.h"
#include <string.h>

namespace {
const uint8_t kBinaryHeader[4] = {0xFE, 0x01, 0xFE, 0x01};
const uint8_t kFragmentHeader[4] = {0xFE, 0x02, 0xFE, 0x02};

uint16_t serialize_binary_packet(const ThermalProcessor* processor, uint8_t* buffer) {
    uint16_t offset = 0;
    const uint8_t num_detected = processor->num_detected;
    const uint16_t payload_size = (uint16_t)(IMAGE_SIZE * 4 + 1 + (4 * num_detected));

    memcpy(buffer + offset, kBinaryHeader, sizeof(kBinaryHeader));
    offset += sizeof(kBinaryHeader);

    memcpy(buffer + offset, &payload_size, sizeof(payload_size));
    offset += sizeof(payload_size);

    memcpy(buffer + offset, processor->thermal_image, IMAGE_SIZE);
    offset += IMAGE_SIZE;

    memcpy(buffer + offset, processor->background, IMAGE_SIZE);
    offset += IMAGE_SIZE;

    memcpy(buffer + offset, processor->distance_map, IMAGE_SIZE);
    offset += IMAGE_SIZE;

    memcpy(buffer + offset, processor->labeled_image, IMAGE_SIZE);
    offset += IMAGE_SIZE;

    buffer[offset++] = num_detected;
    for (uint8_t i = 0; i < num_detected; i++) {
        buffer[offset++] = processor->detected_people[i].y;
        buffer[offset++] = processor->detected_people[i].x;
        memcpy(buffer + offset, &processor->detected_people[i].area, sizeof(uint16_t));
        offset += sizeof(uint16_t);
    }

    return offset;
}
} // namespace

void send_binary_packet(Stream& stream, const ThermalProcessor* processor) {
    uint8_t binary_packet[THERMAL_BINARY_PACKET_MAX_SIZE];
    const uint16_t packet_size = serialize_binary_packet(processor, binary_packet);
    stream.write(binary_packet, packet_size);
}

uint8_t build_fragmented_packets(const ThermalProcessor* processor,
                                 uint16_t sequence,
                                 FragmentedThermalPacket* fragments) {
    uint8_t binary_packet[THERMAL_BINARY_PACKET_MAX_SIZE];
    const uint16_t binary_packet_size = serialize_binary_packet(processor, binary_packet);
    const uint16_t bytes_per_fragment = (binary_packet_size + THERMAL_FRAGMENT_COUNT - 1) / THERMAL_FRAGMENT_COUNT;

    for (uint8_t fragment_index = 0; fragment_index < THERMAL_FRAGMENT_COUNT; ++fragment_index) {
        const uint16_t payload_offset = fragment_index * bytes_per_fragment;
        const uint16_t remaining = (payload_offset < binary_packet_size)
            ? (binary_packet_size - payload_offset)
            : 0;
        const uint16_t payload_size = remaining > bytes_per_fragment ? bytes_per_fragment : remaining;
        uint16_t offset = 0;

        memcpy(fragments->data[fragment_index] + offset, kFragmentHeader, sizeof(kFragmentHeader));
        offset += sizeof(kFragmentHeader);

        memcpy(fragments->data[fragment_index] + offset, &sequence, sizeof(sequence));
        offset += sizeof(sequence);

        fragments->data[fragment_index][offset++] = fragment_index;
        fragments->data[fragment_index][offset++] = THERMAL_FRAGMENT_COUNT;

        memcpy(fragments->data[fragment_index] + offset, &payload_size, sizeof(payload_size));
        offset += sizeof(payload_size);

        if (payload_size > 0) {
            memcpy(fragments->data[fragment_index] + offset,
                   binary_packet + payload_offset,
                   payload_size);
            offset += payload_size;
        }

        fragments->sizes[fragment_index] = offset;
    }

    return THERMAL_FRAGMENT_COUNT;
}

uint8_t send_fragmented_packets(Stream& stream, const ThermalProcessor* processor) {
    static uint16_t sequence = 0;
    FragmentedThermalPacket fragments;
    const uint8_t fragment_count = build_fragmented_packets(processor, sequence++, &fragments);

    for (uint8_t i = 0; i < fragment_count; ++i) {
        stream.write(fragments.data[i], fragments.sizes[i]);
    }

    return fragment_count;
}
