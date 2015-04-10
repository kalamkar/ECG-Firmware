// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __MONITOR_SERVICE_H__
#define __MONITOR_SERVICE_H__

#include "BLEDevice.h"

class MonitorService {
public:

    static const unsigned UUID_SERVICE      = 0x1901;
    static const unsigned UUID_CHAR         = 0x1902;    
    static const uint16_t MAX_DATA_LEN      = 512;

    MonitorService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        monitor(UUID_CHAR, data, length, MAX_DATA_LEN,
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&monitor};
        GattService         monitorService(MonitorService::UUID_SERVICE, charTable, 1);
        ble.addService(monitorService);
    }
    
    void addValue(uint16_t sample) {
        data[length++] = sample >> 2;
        if (length == MAX_DATA_LEN) {
            ble.updateCharacteristicValue(monitor.getValueHandle(), data, length);   
            length = 0;
        }
    }

private:
    BLEDevice           &ble;

    uint8_t             data[MAX_DATA_LEN];
    uint16_t            length;

    GattCharacteristic  monitor;
};

#endif /* #ifndef __MONITOR_SERVICE_H__*/