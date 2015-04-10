// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __MONITOR_SERVICE_H__
#define __MONITOR_SERVICE_H__

#include "BLEDevice.h"

class MonitorService {
public:

    static const unsigned UUID_SERVICE      = 0x1901;
    static const unsigned UUID_CHAR         = 0x1902;
    static const uint16_t MAX_VALUE_BYTES   = 2;

    MonitorService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        monitor(UUID_CHAR, valueBytes, length, MonitorService::MAX_VALUE_BYTES,
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&monitor};
        GattService         monitorService(MonitorService::UUID_SERVICE, charTable, 1);
        ble.addService(monitorService);
    }
    
    void addValue(uint16_t value) {
        valueBytes[length++] = value & 0x00FF;
        valueBytes[length++] = value >> 8;
        if (length == MAX_VALUE_BYTES) {
            ble.updateCharacteristicValue(monitor.getValueHandle(), valueBytes, length);   
            length = 0;
        }
    }

private:
    BLEDevice           &ble;

    uint8_t  valueBytes[MAX_VALUE_BYTES];
    uint16_t length;

    GattCharacteristic  monitor;
};

#endif /* #ifndef __MONITOR_SERVICE_H__*/