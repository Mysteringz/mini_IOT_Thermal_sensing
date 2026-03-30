#ifndef THERMAL_SERIALIZER_H
#define THERMAL_SERIALIZER_H

#include <Arduino.h>
#include "thermal_image_processor.h"

#define THERMAL_PACKET_HEADER_SIZE 6
#define THERMAL_FRAGMENT_COUNT 4
#define THERMAL_FRAGMENT_HEADER_SIZE 10
#define THERMAL_BINARY_PAYLOAD_MAX_SIZE (IMAGE_SIZE * 4 + 1 + (4 * MAX_PEOPLE))
#define THERMAL_BINARY_PACKET_MAX_SIZE (THERMAL_PACKET_HEADER_SIZE + THERMAL_BINARY_PAYLOAD_MAX_SIZE)
#define THERMAL_FRAGMENT_PAYLOAD_MAX_SIZE ((THERMAL_BINARY_PACKET_MAX_SIZE + THERMAL_FRAGMENT_COUNT - 1) / THERMAL_FRAGMENT_COUNT)
#define THERMAL_FRAGMENT_PACKET_MAX_SIZE (THERMAL_FRAGMENT_HEADER_SIZE + THERMAL_FRAGMENT_PAYLOAD_MAX_SIZE)

typedef struct {
	uint8_t data[THERMAL_FRAGMENT_COUNT][THERMAL_FRAGMENT_PACKET_MAX_SIZE];
	uint16_t sizes[THERMAL_FRAGMENT_COUNT];
} FragmentedThermalPacket;

// Legacy function - kept for backwards compatibility with serial
void send_binary_packet(Stream& stream, const ThermalProcessor* processor);

// Build a fixed four-fragment representation of the binary packet for UDP.
// Returns the number of fragments written to the output structure.
uint8_t build_fragmented_packets(const ThermalProcessor* processor,
								 uint16_t sequence,
								 FragmentedThermalPacket* fragments);

// Writes the same four fragments to a stream back-to-back.
// Returns the number of fragments sent.
uint8_t send_fragmented_packets(Stream& stream, const ThermalProcessor* processor);

#endif // THERMAL_SERIALIZER_H
