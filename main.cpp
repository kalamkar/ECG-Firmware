// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "mbed_i2c.h"

#define LOG(...)    { pc.printf(__VA_ARGS__); }

#include "config.h"

#include "Bluetooth.h"
#include "ECG.h"
#include "MotionProcessor.h"

enum DeviceMode {
    SLEEPING,
    ADVERTISING,
    SHORT_SESSION,
    LONG_SESSION
};

static DeviceMode deviceMode = SLEEPING;

DigitalOut      blue(LED_BLUE);
DigitalOut      green(LED_GREEN);
DigitalOut      red(LED_RED);

InterruptIn     motionProbe(MPU6050_INT);
Serial          pc(UART_TX, UART_RX);

Ticker          sensorTicker;
Ticker          advertisingTicker;
Ticker          idleTicker;

BluetoothSmart  bluetooth;
ECG             ecg;
MotionProcessor mpu;

volatile bool   readEcg = false;
volatile bool   readAccel = false;

void triggerEcg(void) {
    readEcg = true;
}

void triggerAccel(void) {
    readAccel = true;
}

void toggleLED(void) {
    blue = !blue;
}

void stopAdvertising() {
    bluetooth.stop();
    advertisingTicker.detach();
    idleTicker.detach();
}

void goToSleep() {
    LOG("Device going to sleep mode.\n");
    if (bluetooth.isConnected()) {
        bluetooth.disconnect();
    } else if (deviceMode == ADVERTISING) {
        stopAdvertising();
    }
    deviceMode = SLEEPING;
    red = 1; green = 1; blue = 1;
}

void onIdleTimeout() {
    if (deviceMode == ADVERTISING) {
        goToSleep();
    }
}

void startAdvertising() {
    bluetooth.start();
    deviceMode = ADVERTISING;
    advertisingTicker.attach(&toggleLED, CONNECTED_BLINK_INTERVAL_SECS);
    idleTicker.attach(&onIdleTimeout, IDLE_TIMEOUT_SECS);
    red = 1; green = 1; blue = 0;
}

void wakeUp() {
    if (deviceMode != SLEEPING) {
        return;
    }
    LOG("Device woken up.\n");
    startAdvertising();
}

void onTap(unsigned char direction, unsigned char count) {
    LOG("Tap motion detected dir %d and strength %d\n", direction, count);
    wakeUp();
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params) {
// void connectionCallback(Gap::Handle_t handle, Gap::addr_type_t peerAddrType, const Gap::address_t peerAddr, const Gap::ConnectionParams_t *params) {
    stopAdvertising();
    deviceMode = SHORT_SESSION;
    sensorTicker.attach_us(&triggerEcg, SENSOR_TICKER_MICROS);
    ecg.start();
    red = 1; green = 0; blue = 0;
    LOG("Connected to device.\n");
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params) {
// void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    sensorTicker.detach();
    ecg.stop();
    startAdvertising();
    LOG("Disconnected from device.\n");
}

int main(void) {
    red = 0; green = 1; blue = 1;    

    pc.baud(115200);
    LOG("\n--- DovetailV2 Monitor ---\n");
    
    motionProbe.fall(&triggerAccel);

    if (mpu.hasInitialized() && bluetooth.hasInitialized()) {
        LOG("MPU6050 and bluetooth initialized.\n");
        red = 1; green = 0; blue = 1;  // Show green light for a second to show boot success.
        wait(1);
        red = 1; green = 1; blue = 1;
    } else {
        LOG("Failed to initialize mpu6050 or bluetooth.\n");
    }
    

    // infinite loop
    while (true) {
        if (readEcg && bluetooth.isConnected()) {
            readEcg = false;
            bluetooth.service().addValue(ecg.read());
        }

        if (readAccel) {
            readAccel = false;
            mpu.processData();
        }
        bluetooth.sleep();
    }
}
