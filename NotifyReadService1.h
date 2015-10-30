// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __NOTIFY_READ_SERVICE_1_H__
#define __NOTIFY_READ_SERVICE_1_H__

static const uint8_t  BASE_UUID[] = {
    0x40, 0x48, 0x46, 0xa0, 0x60, 0x8a, 0x11, 0xe5,
    0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint16_t SHORT_UUID_SERVICE      = 0x46A0;
static const uint16_t SHORT_UUID_CHAR_PEAK    = 0x46A1;
static const uint16_t SHORT_UUID_CHAR_DATA    = 0x46A4;

static const uint8_t  UUID_SERVICE[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_SERVICE >> 8), (uint8_t)(SHORT_UUID_SERVICE & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_PEAK[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_PEAK >> 8), (uint8_t)(SHORT_UUID_CHAR_PEAK & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_DATA[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_DATA >> 8), (uint8_t)(SHORT_UUID_CHAR_DATA & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

class NotifyReadService1 {
public:

    static const unsigned SENSOR_MAX        = 0xFF;
    static const uint16_t MAX_DATA_LEN      = 500;

    NotifyReadService1(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        peak(UUID_CHAR_PEAK, &peakPercent, sizeof(peakPercent), sizeof(peakPercent),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        sensorData(UUID_CHAR_DATA, data, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ) {
        GattCharacteristic *charTable[] = {&peak, &sensorData};
        GattService         monitorService(UUID_SERVICE, charTable, 2);
        ble.addService(monitorService);
    }

    void addValue(uint8_t value) {
        buffer[length] = value;
        length++;
        
        // Notify the variation value once buffer is full
        if (length == MAX_DATA_LEN) {
            peakPercent = 1; // (sampleMax - sampleMin) * 100 / SENSOR_MAX;
            ble.updateCharacteristicValue(peak.getValueHandle(), &peakPercent, sizeof(peakPercent));

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

    uint8_t             peakPercent;

    GattCharacteristic  peak;
    GattCharacteristic  sensorData;
};

#endif /* #ifndef __NOTIFY_READ_SERVICE_1_H__*/