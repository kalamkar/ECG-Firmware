// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __DOVETAIL_SERVICE_H__
#define __DOVETAIL_SERVICE_H__

static const uint8_t  BASE_UUID[] = {
    0x40, 0x48, 0x46, 0xa0, 0x60, 0x8a, 0x11, 0xe5,
    0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint16_t SHORT_UUID_SERVICE      = 0x46A0;
static const uint16_t SHORT_UUID_CHAR_DATA    = 0x46A1;

static const uint8_t  UUID_SERVICE[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_SERVICE >> 8), (uint8_t)(SHORT_UUID_SERVICE & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_DATA[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_DATA >> 8), (uint8_t)(SHORT_UUID_CHAR_DATA & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

class DovetailService {
public:

    static const unsigned SENSOR_MAX        = 0xFF;
    static const uint16_t MAX_DATA_LEN      = 20;

    DovetailService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        sensorData(UUID_CHAR_DATA, data, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        GattCharacteristic *charTable[] = {&sensorData};
        GattService         monitorService(UUID_SERVICE, charTable, 1);
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

#endif /* #ifndef __DOVETAIL_SERVICE_H__*/