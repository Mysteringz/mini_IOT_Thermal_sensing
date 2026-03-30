#ifndef WIFI_TRANSPORT_H
#define WIFI_TRANSPORT_H

#include <Arduino.h>
#include "thermal_image_processor.h"

void wifi_transport_init();
void wifi_transport_update();
bool wifi_transport_send_packet(const ThermalProcessor* processor);

#endif // WIFI_TRANSPORT_H
