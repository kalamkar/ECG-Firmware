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

void goToSleep() {
    LOG("Device going to sleep mode.\n");
    deviceMode = SLEEPING;
    red = 1; green = 1; blue = 1;
    bluetooth.stop();
    sensorTicker.detach();
    advertisingTicker.detach();
    idleTicker.detach();
    ecg.stop();
    sd_power_system_off();  // Should be the last statement. On wakeup resets the system.
}

void onIdleTimeout() {
    if (deviceMode == ADVERTISING) {
        goToSleep();
    }
}

void onAdvertisingStopped() {
    LOG("Stopped bluetooth advertising.\n");
    advertisingTicker.detach();
    idleTicker.detach();
}

void onAdvertisingStarted() {
    LOG("Advertising started.\n");
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
    bluetooth.start();
}

void onTap(unsigned char direction, unsigned char count) {
    LOG("Tap motion detected dir %d and strength %d\n", direction, count);
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
    set_time(1256729737);
    red = 0; green = 1; blue = 1;    

    pc.baud(115200);
    LOG("\n--- DovetailV2 Monitor ---\n");
    
    motionProbe.fall(&triggerAccel);

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
            if (bluetooth.isConnected()) {
                uint8_t value = ecg.read();
                bluetooth.service().addValue(value);
                if (ecg.getIdleSeconds() > IDLE_TIMEOUT_SECS) {
                    goToSleep();
                }
            }
        }

        if (readAccel) {
            readAccel = false;
            mpu.processData();
        }
        
        bluetooth.sleep();
    }
}
