// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __MONITOR_SERVICE_H__
#define __MONITOR_SERVICE_H__

#include "BLEDevice.h"

class MonitorService {
public:

    static const unsigned UUID_SERVICE      = 0x1901;
    static const unsigned UUID_CHAR         = 0x1902;
    static const unsigned UUID_ATTRIBUTE    = 0x1903;
    static const unsigned MAX_VALUE_BYTES   = 20;

    MonitorService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        monitor(UUID_CHAR, valueBytes, length, MonitorService::MAX_VALUE_BYTES,
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&monitor};
        GattService         monitorService(MonitorService::UUID_SERVICE, charTable, 1);
        ble.addService(monitorService);
    }
    
    void addValue(uint8_t value) {
        valueBytes[length++] = value;
        if (length == MAX_VALUE_BYTES) {
            updateBLE();
            length = 0;
        }
    }
    
private:
    
    void updateBLE() {
        ble.updateCharacteristicValue(monitor.getValueAttribute().getHandle(), valueBytes, length);
    }

private:
    BLEDevice           &ble;

    uint8_t valueBytes[MAX_VALUE_BYTES];
    uint8_t length;

    GattCharacteristic  monitor;
};

#endif /* #ifndef __MONITOR_SERVICE_H__*/