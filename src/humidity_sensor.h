/*
 * Humidity Sensor - Dynamic Endpoint for Matter
 * Device Type: Humidity Sensor (0x0015)
 * Cluster: Relative Humidity Measurement (0x0405)
 */

#pragma once

#include <lib/core/CHIPError.h>

CHIP_ERROR InitHumiditySensorEndpoint();
void SetMeasuredHumidity(uint16_t value);
