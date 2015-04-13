// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __NOTIFY_SERVICE_H__
#define __NOTIFY_SERVICE_H__

#include "BLEDevice.h"

class NotifyService {
public:

    static const unsigned UUID_SERVICE      = 0x1901;
    static const unsigned UUID_CHAR_DATA    = 0x1902;
    static const uint16_t MAX_DATA_LEN      = 20;

    NotifyService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        sensorData(UUID_CHAR_DATA, data, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&sensorData};
        GattService         monitorService(NotifyService::UUID_SERVICE, charTable, 2);
        ble.addService(monitorService);
    }
    
    void addValue(uint16_t sample) {
        data[length++] = sample >> 2;
        
        if (length == MAX_DATA_LEN) {
            ble.updateCharacteristicValue(sensorData.getValueHandle(), data, MAX_DATA_LEN);            
            length = 0;
        }
    }

private:
    BLEDevice           &ble;

    uint16_t            length;
    uint8_t             data[MAX_DATA_LEN];

    GattCharacteristic  sensorData;
};

#endif /* #ifndef __NOTIFY_SERVICE_H__*/