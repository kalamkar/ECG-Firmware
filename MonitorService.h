// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __MONITOR_SERVICE_H__
#define __MONITOR_SERVICE_H__

#include "BLEDevice.h"

class MonitorService {
public:

    static const unsigned SENSOR_MAX        = 1024;
    
    static const unsigned UUID_SERVICE      = 0x1901;
    static const unsigned UUID_CHAR_DATA    = 0x1902;
    static const unsigned UUID_CHAR_PEAK    = 0x1903;
    static const uint16_t MAX_DATA_LEN      = 512;

    MonitorService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        sampleMax(0),
        sensorData(UUID_CHAR_DATA, data, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        peak(UUID_CHAR_PEAK, &peakPercent, sizeof(peakPercent), sizeof(peakPercent),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&sensorData, &peak};
        GattService         monitorService(MonitorService::UUID_SERVICE, charTable, 2);
        ble.addService(monitorService);
    }
    
    void addValue(uint16_t sample) {
        buffer[length++] = sample >> 2;
        sampleMax = sampleMax < sample ? sample : sampleMax;
        
        // Notify the variation value once buffer is full
        if (length == MAX_DATA_LEN) {
            peakPercent = sampleMax * 100 / SENSOR_MAX;
            ble.updateCharacteristicValue(peak.getValueHandle(), &peakPercent, sizeof(peakPercent));
            
            memcpy(data, buffer, MAX_DATA_LEN);
            ble.updateCharacteristicValue(sensorData.getValueHandle(), data, MAX_DATA_LEN);
            
            length = 0;
            sampleMax = 0;
        }
    }

private:
    BLEDevice           &ble;

    uint8_t             buffer[MAX_DATA_LEN];
    uint16_t            length;
    uint8_t             data[MAX_DATA_LEN];
    
    uint8_t             peakPercent;
    uint16_t            sampleMax;

    GattCharacteristic  sensorData;
    GattCharacteristic  peak;
};

#endif /* #ifndef __MONITOR_SERVICE_H__*/