// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "mbed_i2c.h"

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

#include "Bluetooth.h"
#include "DFUService.h"
#include "UARTService.h"
#include "DeviceInformationService.h"
#include "NotifyReadService.h"

#define LOG(...)    { pc.printf(__VA_ARGS__); }

#include "mpu6050.h"

#define LED_GREEN   p21
#define LED_RED     p22
#define LED_BLUE    p23
#define BUTTON_PIN  p17
#define BATTERY_PIN p1

#define UART_TX     p9
#define UART_RX     p11
#define UART_CTS    p8
#define UART_RTS    p10

#define MFR_NAME "Dovetail Monitor"
#define MODEL_NUM "Model1"
#define SERIAL_NUM "SN1"
#define HW_REV "hw-rev1"
#define FW_REV "fw-rev1"
#define SW_REV "soft-rev1"

const static char     DEVICE_NAME[]        = "Pregnansi";
static const uint16_t uuid16_list[]        = {NotifyReadService::UUID_SERVICE,
                                              GattService::UUID_DEVICE_INFORMATION_SERVICE}; //,
//                                              UARTServiceShortUUID,
//                                              DFUServiceShortUUID};

DigitalOut blue(LED_BLUE);
DigitalOut green(LED_GREEN);
DigitalOut red(LED_RED);

InterruptIn button(BUTTON_PIN);
AnalogIn    battery(BATTERY_PIN);
Serial      pc(UART_TX, UART_RX);

BLE ble;

AnalogIn sensor(P0_4);

volatile bool triggerSensorPolling = false;

void triggerSensor() {
    triggerSensorPolling = true;
}

void onButtonPress(void) {
    LOG("Button pressed\n");
}

void onTap(unsigned char direction, unsigned char count) {
    LOG("Tap motion detected\n");
}

void onOrientationChange(unsigned char orientation) {
    LOG("Oriention changed\n");
}

void updatesEnabledCallback(Gap::Handle_t handle) {
    red = 0; green = 0; blue = 1;
}

void updatesDisabledCallback(Gap::Handle_t handle) {
    red = 0; green = 1; blue = 1;
}
    
void connectionCallback(const Gap::ConnectionCallbackParams_t *) {
    ble.stopAdvertising();
    red = 0; green = 1; blue = 1;
}

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    red = 0; green = 1; blue = 0;
}

int main(void) {
    pc.baud(115200);
    LOG("---- Dovetail Monitor ----\n");

    button.fall(onButtonPress);

    initMotionProcessor();
    initBluetooth(ble);

    NotifyReadService monitorService(ble);
    DeviceInformationService deviceInfo(ble, MFR_NAME, MODEL_NUM, SERIAL_NUM, HW_REV, FW_REV, SW_REV);
//    DFUService dfu(ble);
//    UARTService uartService(ble);

    startAdvertising(ble, (uint8_t *) uuid16_list, DEVICE_NAME);
    
    red = 0; green = 1; blue = 0;

    // infinite loop
    while (true) {
        if (triggerSensorPolling && ble.getGapState().connected) {
            triggerSensorPolling = false;

            unsigned long sensor_timestamp;
            short gyro[3], accel[3], sensors;
            long quat[4];
            unsigned char more = 1;

            while (more) {
                /* This function gets new data from the FIFO when the DMP is in
                 * use. The FIFO can contain any combination of gyro, accel,
                 * quaternion, and gesture data. The sensors parameter tells the
                 * caller which data fields were actually populated with new data.
                 * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
                 * the FIFO isn't being filled with accel data.
                 * The driver parses the gesture data to determine if a gesture
                 * event has occurred; on an event, the application will be notified
                 * via a callback (assuming that a callback function was properly
                 * registered). The more parameter is non-zero if there are
                 * leftover packets in the FIFO.
                 */
                dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
                /* Gyro and accel data are written to the FIFO by the DMP in chip
                 * frame and hardware units. This behavior is convenient because it
                 * keeps the gyro and accel outputs of dmp_read_fifo and
                 * mpu_read_fifo consistent.
                 */
                if (sensors & INV_XYZ_GYRO) {
                    // LOG("GYRO: %d, %d, %d\n", gyro[0], gyro[1], gyro[2]);
                }
                if (sensors & INV_XYZ_ACCEL) {
                    // LOG("ACC: %d, %d, %d\n", accel[0], accel[1], accel[2]);
                    monitorService.addValue(accel[2]);
                }

                /* Unlike gyro and accel, quaternions are written to the FIFO in
                 * the body frame, q30. The orientation is set by the scalar passed
                 * to dmp_set_orientation during initialization.
                 */
                if (sensors & INV_WXYZ_QUAT) {
                    // LOG("QUAT: %ld, %ld, %ld, %ld\n", quat[0], quat[1], quat[2], quat[3]);
                }
            }

            // monitorService.addValue(sensor.read_u16());
        }
        ble.waitForEvent(); // low power wait for event
    }
}
