// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "BLEDevice.h"
#include "NotifyReadService.h"
#include "DeviceInformationService.h"

BLEDevice  ble;
DigitalOut led1(LED1);
AnalogIn sensor(P0_4);

I2C i2c(P0_10, P0_8)
const static int MMA845X = 0x1D;

SPI spi(P0_9, P0_11, P0_8); // mosi, miso, sclk
DigitalOut cs(P0_10);

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

void connectionCallback(Gap::Handle_t handle, Gap::addr_type_t peerAddrType,
                        const Gap::address_t peerAddr, const Gap::ConnectionParams_t *) {
    ble.stopAdvertising();
}

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    updatesDisabledCallback(handle);
}

int spiRead() {
    cs = 0; 
    int value = spi.write(0x00);
    cs = 1;    
    return value;
}

int i2cRead() {
    char cmd[2];
    cmd[0] = 0x01;
    cmd[1] = 0x00;
    i2c.write(MMA845X, cmd, 2);
 
    wait(0.5);
 
    cmd[0] = 0x00;
    i2c.write(addr, cmd, 1);
    i2c.read(addr, cmd, 2);
}

int main(void) {
    led1 = 0;
    
    // Setup the spi for 12 bit data, Mode 0 with a 100KHz clock rate
    spi.format(12, 0);
    spi.frequency(100000);
    cs = 1;
    
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onConnection(connectionCallback);
    ble.onUpdatesEnabled(updatesEnabledCallback);
    ble.onUpdatesDisabled(updatesDisabledCallback);

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
            // monitorService.addValue(spiRead());
            monitorService.addValue(sensor.read_u16());
        }
        ble.waitForEvent(); // low power wait for event
    }
}
