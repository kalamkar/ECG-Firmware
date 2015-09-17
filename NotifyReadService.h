// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __NOTIFY_READ_SERVICE_H__
#define __NOTIFY_READ_SERVICE_H__

#include "BLE.h"

class NotifyReadService {
public:

    static const unsigned SENSOR_MAX        = 0xFF;
    
    static const unsigned UUID_SERVICE      = 0x1901;
    static const unsigned UUID_CHAR_DATA    = 0x1902;
    static const unsigned UUID_CHAR_PEAK    = 0x1903;
    static const unsigned UUID_CHAR_STATUS  = 0x1904;
    static const uint16_t MAX_DATA_LEN      = 512;

    NotifyReadService(BLE &_ble) :
        ble(_ble),
        length(0),
        statusValue(0),
        sampleMax(0),
        sensorData(UUID_CHAR_DATA, data, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        peak(UUID_CHAR_PEAK, &peakPercent, sizeof(peakPercent), sizeof(peakPercent),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        status(UUID_CHAR_STATUS, &statusValue, sizeof(statusValue), sizeof(statusValue),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&sensorData, &peak, &status};
        GattService         monitorService(NotifyReadService::UUID_SERVICE, charTable, 3);
        ble.addService(monitorService);
    }
    
    void addValue(uint8_t sample) {
        buffer[length++] = sample;
        sampleMax = sample > sampleMax ? sample : sampleMax;
        sampleMin = sample < sampleMin ? sample : sampleMin;
        
        // Notify the variation value once buffer is full
        if (length == MAX_DATA_LEN) {
            peakPercent = (sampleMax - sampleMin) * 100 / SENSOR_MAX;
            ble.updateCharacteristicValue(peak.getValueHandle(), &peakPercent, sizeof(peakPercent));
            
            memcpy(data, buffer, MAX_DATA_LEN);
            ble.updateCharacteristicValue(sensorData.getValueHandle(), data, MAX_DATA_LEN);
            
            length = 0;
            sampleMax = 0;
            sampleMin = SENSOR_MAX;
        }
    }
    
    void updateStatus() {
        ble.updateCharacteristicValue(status.getValueHandle(), &statusValue, sizeof(statusValue));
    }
    
    uint8_t* getStatus() {
        return &statusValue;
    }

private:
    BLEDevice           &ble;

    uint8_t             buffer[MAX_DATA_LEN];
    uint16_t            length;
    uint8_t             data[MAX_DATA_LEN];
    uint8_t             statusValue;
    
    uint8_t             peakPercent;
    uint8_t             sampleMax;
    uint8_t             sampleMin;

    GattCharacteristic  sensorData;
    GattCharacteristic  peak;
    GattCharacteristic  status;
};

#endif /* #ifndef __NOTIFY_READ_SERVICE_H__*/