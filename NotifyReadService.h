// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __NOTIFY_READ_SERVICE_H__
#define __NOTIFY_READ_SERVICE_H__

static const uint8_t  BASE_UUID[] = {
    0x40, 0x48, 0x46, 0xa0, 0x60, 0x8a, 0x11, 0xe5,
    0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint16_t SHORT_UUID_SERVICE      = 0x46A0;
static const uint16_t SHORT_UUID_CHAR_PEAK    = 0x46A1;
static const uint16_t SHORT_UUID_CHAR_DATA_X  = 0x46A2;
static const uint16_t SHORT_UUID_CHAR_DATA_Y  = 0x46A3;
static const uint16_t SHORT_UUID_CHAR_DATA_Z  = 0x46A4;

static const uint8_t  UUID_SERVICE[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_SERVICE >> 8), (uint8_t)(SHORT_UUID_SERVICE & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_PEAK[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_PEAK >> 8), (uint8_t)(SHORT_UUID_CHAR_PEAK & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_DATA_X[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_DATA_X >> 8), (uint8_t)(SHORT_UUID_CHAR_DATA_X & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_DATA_Y[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_DATA_Y >> 8), (uint8_t)(SHORT_UUID_CHAR_DATA_Y & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
static const uint8_t  UUID_CHAR_DATA_Z[] = {
    0x40, 0x48, (uint8_t)(SHORT_UUID_CHAR_DATA_Z >> 8), (uint8_t)(SHORT_UUID_CHAR_DATA_Z & 0xFF),
    0x60, 0x8a, 0x11, 0xe5, 0xab, 0x45, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

class NotifyReadService {
public:

    static const unsigned SENSOR_MAX        = 0xFF;
    static const uint16_t MAX_DATA_LEN      = 128;

    NotifyReadService(BLEDevice &_ble) :
        ble(_ble),
        length(0),
        peak(UUID_CHAR_PEAK, &peakPercent, sizeof(peakPercent), sizeof(peakPercent),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        sensorDataX(UUID_CHAR_DATA_X, dataX, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        sensorDataY(UUID_CHAR_DATA_Y, dataY, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        sensorDataZ(UUID_CHAR_DATA_Z, dataZ, 0, MAX_DATA_LEN,
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ) {
        GattCharacteristic *charTable[] = {&peak, &sensorDataX, &sensorDataY, &sensorDataZ};
        GattService         monitorService(UUID_SERVICE, charTable, 4);
        ble.addService(monitorService);
    }

    void addValue(uint8_t x, uint8_t y, uint8_t z) {
        bufferX[length] = x;
        bufferY[length] = y;
        bufferZ[length] = z;
        length++;

        // Notify the variation value once buffer is full
        if (length == MAX_DATA_LEN) {
            peakPercent = 1; // (sampleMax - sampleMin) * 100 / SENSOR_MAX;
            ble.updateCharacteristicValue(peak.getValueHandle(), &peakPercent, sizeof(peakPercent));

            memcpy(dataX, bufferX, MAX_DATA_LEN);
            memcpy(dataY, bufferY, MAX_DATA_LEN);
            memcpy(dataZ, bufferZ, MAX_DATA_LEN);
            
            ble.updateCharacteristicValue(sensorDataX.getValueHandle(), dataX, MAX_DATA_LEN);
            ble.updateCharacteristicValue(sensorDataY.getValueHandle(), dataY, MAX_DATA_LEN);
            ble.updateCharacteristicValue(sensorDataZ.getValueHandle(), dataZ, MAX_DATA_LEN);

            length = 0;
        }
    }

private:
    BLEDevice           &ble;

    uint16_t            length;
    
    uint8_t             bufferX[MAX_DATA_LEN];
    uint8_t             bufferY[MAX_DATA_LEN];
    uint8_t             bufferZ[MAX_DATA_LEN];

    uint8_t             dataX[MAX_DATA_LEN];
    uint8_t             dataY[MAX_DATA_LEN];
    uint8_t             dataZ[MAX_DATA_LEN];

    uint8_t             peakPercent;

    GattCharacteristic  peak;
    GattCharacteristic  sensorDataX;
    GattCharacteristic  sensorDataY;
    GattCharacteristic  sensorDataZ;
};

#endif /* #ifndef __NOTIFY_READ_SERVICE_H__*/