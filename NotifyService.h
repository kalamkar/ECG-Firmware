// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __NOTIFY_SERVICE_H__
#define __NOTIFY_SERVICE_H__

static const uint16_t MAX_DATA_LEN      = 20;

class NotifyService {
public:

    NotifyService(BLEDevice &_ble, const uint8_t serviceUuid[], const uint8_t charUuid[]) :
        ble(_ble),
        length(0),
        sensorData(charUuid, data, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&sensorData};
        GattService         monitorService(serviceUuid, charTable, 1);
        ble.addService(monitorService);
    }

    void addValue(uint8_t value) {
        buffer[length] = value;
        length++;
        
        // Notify the variation value once buffer is full
        if (length == MAX_DATA_LEN) {
            memcpy(data, buffer, MAX_DATA_LEN);
            ble.updateCharacteristicValue(sensorData.getValueHandle(), data, MAX_DATA_LEN);
            length = 0;
        }
    }

private:
    BLEDevice           &ble;

    uint16_t            length;
    
    uint8_t             buffer[MAX_DATA_LEN];
    uint8_t             data[MAX_DATA_LEN];

    GattCharacteristic  sensorData;
};

#endif /* #ifndef __NOTIFY_SERVICE_H__*/