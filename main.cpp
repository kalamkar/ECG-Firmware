// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "BLE.h"
#include "NotifyReadService.h"
#include "DeviceInformationService.h"
#include "MMA8452.h"
#include "MMA8451Q.h"

#undef MMA8452_DEBUG

const static uint8_t MMA845X = 0x1D;
//const static uint8_t CTRL_REG1 = 0x2A;
//const static uint8_t XYZ_DATA_CFG = 0x0E;

BLE  ble;
DigitalOut led1(LED1);
AnalogIn sensor(P0_4);

I2C i2c(P0_10, P0_8);
// MMA8452 acc(P0_10, P0_8, 40000);
MMA8451Q acc(P0_10, P0_8, MMA845X);


Ticker sensorTicker;
Ticker ledTicker;

const static char     DEVICE_NAME[]        = "Dovetail1";
static const uint16_t uuid16_list[]        = {NotifyReadService::UUID_SERVICE,
                                              GattService::UUID_DEVICE_INFORMATION_SERVICE};
static volatile bool  triggerSensorPolling = false;

void toggleLED() {
    led1 = !led1;
}

void triggerSensor() {
    triggerSensorPolling = true;
}

void updatesEnabledCallback(Gap::Handle_t handle) {
    sensorTicker.attach(&triggerSensor, 0.01); // Trigger Sensor every 10 milliseconds
    ledTicker.attach(&toggleLED, 1); // Trigger Sensor every second
}

void updatesDisabledCallback(Gap::Handle_t handle) {
    sensorTicker.detach();
    ledTicker.detach();
    led1 = 0;
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *) {
    ble.stopAdvertising();
}

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    updatesDisabledCallback(handle);
}

int main(void) {
    led1 = 0;

    ble.init();
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);
    ble.gattServer().onUpdatesEnabled(updatesEnabledCallback);
    ble.gattServer().onUpdatesDisabled(updatesDisabledCallback);
    

    // Monitor and device information services provided by the BLE device
    NotifyReadService monitorService(ble);
    DeviceInformationService deviceInfo(ble, "Dovetail Monitor", "Model1", "SN1", "hw-rev1",
                                        "fw-rev1", "soft-rev1");

    // Setup advertising.
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                     | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                     (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                     (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); // milli seconds
    ble.gap().startAdvertising();

//    acc.setBitDepth(MMA8452::BIT_DEPTH_8);
//    acc.setDynamicRange(MMA8452::DYNAMIC_RANGE_2G);
//    acc.setDataRate(MMA8452::RATE_100);

    // infinite loop
    while (1) {
        if (triggerSensorPolling && ble.getGapState().connected) {
            triggerSensorPolling = false;
//            int z = 0;
//            acc.readZCount(&z);
//            monitorService.addValue(z);
            monitorService.addValue(acc.getAccX() * 0xFF);
            // monitorService.addValue(sensor.read_u16());
        }
        ble.waitForEvent(); // low power wait for event
    }
}
