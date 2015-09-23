// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "mbed_i2c.h"

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "tiny_ble.h"

#include "Bluetooth.h"
#include "BatteryService.h"
#include "DFUService.h"
#include "DeviceInformationService.h"
#include "NotifyReadService.h"

#define LOG(...)    { pc.printf(__VA_ARGS__); }

#include "mpu6050.h"

#define MFR_NAME    "Dovetail Monitor"
#define MODEL_NUM   "Model1"
#define SERIAL_NUM  "SN1"
#define HW_REV      "hw-rev1"
#define FW_REV      "fw-rev1"
#define SW_REV      "soft-rev1"

#define BATTERY_READ_INTERVAL_SECS      30
#define CONNECTED_BLINK_INTERVAL_SECS   2

const static char     DEVICE_NAME[]     = "Pregnansi";
static const uint16_t SERVICES[]        = { SHORT_UUID_SERVICE,
                                            GattService::UUID_DEVICE_INFORMATION_SERVICE,
                                            DFUServiceShortUUID,
                                            GattService::UUID_BATTERY_SERVICE};

DigitalOut blue(LED_BLUE);
DigitalOut green(LED_GREEN);
DigitalOut red(LED_RED);

InterruptIn button(BUTTON_PIN);
// InterruptIn motionProbe(p14);
AnalogIn    battery(BATTERY_PIN);
Serial      pc(UART_TX, UART_RX);

BLE ble;
Ticker sensorTicker;
Ticker batteryTicker;
Ticker connectionTicker;

volatile bool triggerSensorPolling = false;
volatile bool readBattery = false;

void triggerSensor(void) {
    triggerSensorPolling = true;
}

void triggerBattery(void) {
    readBattery = true;
}

void toggleLED(void) {
    blue = !blue;
}

void onButtonPress(void) {
    LOG("Button pressed\n");
}

void onTap(unsigned char direction, unsigned char count) {
    LOG("Tap motion detected dir %d and strength %d\n", direction, count);
}

void onOrientationChange(unsigned char orientation) {
    LOG("Oriention changed %d\n", orientation);
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *) {
    ble.stopAdvertising();
    batteryTicker.attach(&triggerBattery, BATTERY_READ_INTERVAL_SECS);
    connectionTicker.attach(&toggleLED, CONNECTED_BLINK_INTERVAL_SECS);
    red = 1; green = 0; blue = 1;
    LOG("Connected to device.\n");
}

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    batteryTicker.detach();
    connectionTicker.detach();
    red = 1; green = 0; blue = 0;
    LOG("Disconnected from device.\n");
}

int main(void) {
    red = 0; green = 1; blue = 1;

    pc.baud(115200);
    LOG("\n--- Pregnansi Monitor ---\n");

    button.fall(onButtonPress);

    LOG("Initializing BTLE...\n");
    initBluetooth(ble);
    LOG("BTLE Initialized.\n");

    NotifyReadService monitorService(ble);
    DeviceInformationService deviceInfo(ble, MFR_NAME, MODEL_NUM, SERIAL_NUM, HW_REV, FW_REV, SW_REV);
    DFUService dfu(ble);
    BatteryService batteryService(ble);

    startAdvertising(ble, (uint8_t *) SERVICES, DEVICE_NAME);

    LOG("Initializing Motion Processor...\n");
    initMotionProcessor();
    LOG("Motion Processor initialized.\n");

    sensorTicker.attach(&triggerSensor, 0.01); // Trigger Sensor every 10 milliseconds
    // motionProbe.fall(&triggerSensor);

    red = 1; green = 0; blue = 0;

    // infinite loop
    while (true) {
        if (readBattery && ble.getGapState().connected) {
            readBattery = false;
            uint8_t levelPercent = battery.read_u16() * (1.0f / (float)0x3FF) * 1.2 * 12.2 / 2.2;
            batteryService.updateBatteryLevel(levelPercent);

            unsigned long steps = 0;
            if(dmp_get_pedometer_step_count(&steps) == 0) {
                LOG("Step count is %lu\n", steps);
            }
        }

        if (triggerSensorPolling && ble.getGapState().connected) {
            triggerSensorPolling = false;

            unsigned long sensor_timestamp;
            short gyro[3], accel[3], sensors;
            long quat[4];
            unsigned char more = 1;

            while (more) {
                dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
                if (sensors & INV_XYZ_GYRO) {
                    // LOG("GYRO: %d, %d, %d\n", gyro[0], gyro[1], gyro[2]);
                }

                if (sensors & INV_XYZ_ACCEL) {
                    // LOG("ACC: %d, %d, %d\n", accel[0], accel[1], accel[2]);
                    uint16_t value = accel[2] >= 0x8000 ? 0x8000 : accel[2] <0 -0x8000 ? 0
                            : accel[2] + 0x8000;
                    monitorService.addValue(value >> 8);
                }

                if (sensors & INV_WXYZ_QUAT) {
                    // LOG("QUAT: %ld, %ld, %ld, %ld\n", quat[0], quat[1], quat[2], quat[3]);
                }
            }
        }
        ble.waitForEvent(); // low power wait for event
    }
}
