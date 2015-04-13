// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "BLEDevice.h"
#include "NotifyReadService.h"
#include "DeviceInformationService.h"

BLEDevice  ble;
DigitalOut led1(LED1);
AnalogIn sensor(P0_4);

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

void connectionCallback(Gap::Handle_t handle, Gap::addr_type_t peerAddrType,
                        const Gap::address_t peerAddr, const Gap::ConnectionParams_t *) {
    ble.stopAdvertising();
    sensorTicker.attach(&triggerSensor, 0.01); // Trigger Sensor every 10 milliseconds
    ledTicker.attach(&toggleLED, 1); // Trigger Sensor every second
}

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    sensorTicker.detach();
    ledTicker.detach();
    led1 = 0;
}

int main(void) {
    led1 = 0;
    
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onConnection(connectionCallback);

    // Monitor and device information services provided by the BLE device
    NotifyReadService monitorService(ble);
    DeviceInformationService deviceInfo(ble, "Dovetail Monitor", "Model1", "SN1", "hw-rev1",
                                        "fw-rev1", "soft-rev1");

    // Setup advertising.
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                     | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                     (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                     (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(1000));
    ble.startAdvertising();

    // infinite loop
    while (1) {
        if (triggerSensorPolling && ble.getGapState().connected) {
            triggerSensorPolling = false;
            monitorService.addValue(sensor.read_u16());
        }
        ble.waitForEvent(); // low power wait for event
    }
}
