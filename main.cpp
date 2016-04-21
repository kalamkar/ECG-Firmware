// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "nRF5xGap.h"

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

void goToSleep() {
    LOG("Device going to sleep mode.\n");
    deviceMode = SLEEPING;
    red = 1; green = 1; blue = 1;
    bluetooth.stop();
    sensorTicker.detach();
    advertisingTicker.detach();
    idleTicker.detach();
    ecg.stop();
    mpu.enableDoubleTap();
    sd_power_system_off();  // Should be the last statement. On wakeup resets the system.
    red = 0; green = 1; blue = 1; // Light up red if it cannot go to system off mode.
}

void onIdleTimeout() {
    if (deviceMode == ADVERTISING || ecg.getIdleSeconds() > IDLE_TIMEOUT_SECS) {
        goToSleep();
    }
}

void onAdvertisingStopped() {
    LOG("Stopped bluetooth advertising.\n");
    advertisingTicker.detach();
}

void onAdvertisingStarted() {
    LOG("Advertising started.\n");
    deviceMode = ADVERTISING;
    advertisingTicker.attach(&toggleLED, CONNECTED_BLINK_INTERVAL_SECS);
    red = 1; green = 1; blue = 0;
}

void wakeUp() {
    if (deviceMode != SLEEPING) {
        return;
    }
    LOG("Device woken up.\n");
    bluetooth.start();
    idleTicker.attach(&onIdleTimeout, IDLE_TIMEOUT_SECS);
}

void onTap() {
    LOG("Tap gesture detected\n");
    wakeUp();
}

void onBluetoothInit() {
    
}

void onConnect() {
    deviceMode = SHORT_SESSION;
    sensorTicker.attach_us(&triggerEcg, SENSOR_TICKER_MICROS);
    ecg.start();
    red = 1; green = 0; blue = 0;
}

void onDisconnect() {
    sensorTicker.detach();
    ecg.stop();
}

int main(void) {
    red = 0; green = 1; blue = 1;
    
    pc.baud(115200);
    LOG("\n--- DovetailV3 ---\n");

    if (mpu.hasInitialized()) {
        LOG("Motion processor initialized.\n");
    } else {
        LOG("Failed to initialize motion processor.\n");
        return -1;
    }
    
    wakeUp();

    // infinite loop
    while (true) {
        if (readEcg) {
            readEcg = false;
            bluetooth.addValue(ecg.read());
        }

        if (readAccel) {
            readAccel = false;
            mpu.processData();
        }
        
        bluetooth.sleep();
    }
}
