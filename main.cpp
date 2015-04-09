// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "BLEDevice.h"
#include "MonitorService.h"
#include "DeviceInformationService.h"

/* Enable the following if you need to throttle the connection interval. This has
 * the effect of reducing energy consumption after a connection is made;
 * particularly for applications where the central may want a fast connection
 * interval.*/
#define UPDATE_PARAMS_FOR_LONGER_CONNECTION_INTERVAL 0

BLEDevice  ble;
DigitalOut led1(LED1);
AnalogIn sensor(A4);

// Ticker idleTicker;
Ticker sensorTicker;

const static char     DEVICE_NAME[]        = "Dovetail1";
static const uint16_t uuid16_list[]        = {MonitorService::UUID_SERVICE,
                                              GattService::UUID_DEVICE_INFORMATION_SERVICE};
static volatile bool  triggerSensorPolling = false;

void toggleLED(void) {
    led1 = !led1;
}

void triggerSensor(void) {
    /* Note that the ticker callback executes in interrupt context, so it is safer to do
     * heavy-weight sensor polling from the main thread. */
    triggerSensorPolling = true;
}

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    // idleTicker.attach(&toggleLED, 1); // blink LED every second
    sensorTicker.detach();
    led1 = 0;
}

void connectionCallback(Gap::Handle_t handle, Gap::addr_type_t peerAddrType,
                        const Gap::address_t peerAddr, const Gap::ConnectionParams_t *) {
    ble.stopAdvertising();
    sensorTicker.attach(&triggerSensor, 0.5); // Trigger Sensor every 500 milliseconds
    // idleTicker.detach();
}

int main(void) {
    led1 = 1;
    // idleTicker.attach(&toggleLED, 1); // blink LED every second

    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onConnection(connectionCallback);

    MonitorService monitorService(ble);

    DeviceInformationService deviceInfo(ble, "Dovetail Monitor", "Model1", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");

    // Setup advertising.
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(1000));
    ble.startAdvertising();

    // infinite loop
    while (1) {
        if (triggerSensorPolling && ble.getGapState().connected) {
            triggerSensorPolling = false;
            toggleLED();

            // Do blocking calls or whatever is necessary for sensor polling.
            // Read and update sensor value
            monitorService.addValue(sensor.read());
            // monitorService.addValue((uint8_t) rand());
        } else {
            ble.waitForEvent(); // low power wait for event
        }
    }
}
