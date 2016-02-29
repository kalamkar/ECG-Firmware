// Copyright 2015 Dovetail Care Inc. All rights reserved.

#include "mbed.h"
#include "mbed_i2c.h"

#define LOG(...)    { pc.printf(__VA_ARGS__); }

// #include "tiny_ble.h"
#include "port_config.h"

#include "Bluetooth.h"
#include "DFUService.h"
#include "DeviceInformationService.h"
#include "NotifyReadService1.h"

#include "seeed_mpu6050.h"

#define MFR_NAME    "Dovetail Monitor"
#define MODEL_NUM   "Model1"
#define SERIAL_NUM  "SN1"
#define HW_REV      "hw-rev1"
#define FW_REV      "fw-rev1"
#define SW_REV      "soft-rev1"

#define BATTERY_READ_INTERVAL_SECS      30
#define CONNECTED_BLINK_INTERVAL_SECS   2
#define IDLE_TIMEOUT_SECS               60

enum DeviceMode {
    SLEEPING,
    ADVERTISING,
    SHORT_SESSION,
    LONG_SESSION
};

static DeviceMode deviceMode = SLEEPING;

const static char     DEVICE_NAME[]     = "DovetailV2";
static const uint16_t SERVICES[]        = { SHORT_UUID_SERVICE,
                                            GattService::UUID_DEVICE_INFORMATION_SERVICE,
                                            DFUServiceShortUUID,
                                            GattService::UUID_BATTERY_SERVICE};

DigitalOut      blue(LED_BLUE);
DigitalOut      green(LED_GREEN);
DigitalOut      red(LED_RED);

InterruptIn     motionProbe(MPU6050_INT);
Serial          pc(UART_TX, UART_RX);

DigitalIn       lo1(LO_MINUS);
DigitalIn       lo2(LO_PLUS);
AnalogIn        ecg(ECG_SIGNAL);
DigitalOut      ecgPower(SDN_BAR);

Ticker          sensorTicker;
Ticker          connectionTicker;
Ticker          idleTicker;

BLEDevice       ble;

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
    if (ble.getGapState().connected) {
        ble.disconnect(Gap::LOCAL_HOST_TERMINATED_CONNECTION);
    } else if (deviceMode == ADVERTISING) {
        ble.stopAdvertising();
    }
    idleTicker.detach();
    red = 1; green = 1; blue = 1;
}

void maybeGoToSleep() {
    // TODO(abhi): Add conditions for data collection to SHORT_SESSION and LONG_SESSION
    if (deviceMode == ADVERTISING || deviceMode == SHORT_SESSION || deviceMode == LONG_SESSION) {
        goToSleep();
    }
}

void wakeUp() {
    LOG("Woken up and starting BLE advertising.\n");
    idleTicker.attach(&maybeGoToSleep, IDLE_TIMEOUT_SECS);
    ble.startAdvertising();
    deviceMode = ADVERTISING;
    red = 0; green = 0; blue = 0;
}

void onTap(unsigned char direction, unsigned char count) {
    LOG("Tap motion detected dir %d and strength %d\n", direction, count);
    wakeUp();
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params) {
// void connectionCallback(Gap::Handle_t handle, Gap::addr_type_t peerAddrType, const Gap::address_t peerAddr, const Gap::ConnectionParams_t *params) {
    ble.stopAdvertising();
    sensorTicker.attach(&triggerEcg, 0.01); // Trigger Sensor every 10 milliseconds
    connectionTicker.attach(&toggleLED, CONNECTED_BLINK_INTERVAL_SECS);
    ecgPower = 1;
    red = 1; green = 0; blue = 1;
    LOG("Connected to device.\n");
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params) {
// void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    ble.startAdvertising();
    red = 0; green = 0; blue = 0;
    sensorTicker.detach();
    connectionTicker.detach();
    ecgPower = 0;
    LOG("Disconnected from device.\n");
}

uint8_t toUint8(short num) {
    uint16_t value = num >= 0x8000 ? 0xFF00 : num < (0-0x8000) ? 0 : num + 0x8000;
    return value >> 8;
}

void readUpdateAccel(NotifyReadService1 &monitorService) {
    unsigned long sensor_timestamp;
    short gyro[3], accel[3], sensors;
    long quat[4];
    unsigned char more = 1;

    while (more) {
        dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
        if ((sensors & INV_XYZ_ACCEL) && (ble.getGapState().connected)) {
//            LOG("ACC: %d, %d, %d\n", accel[0], accel[1], accel[2]);
//            monitorService.addValue(toUint8(accel[2]));
        }
    }

}

void readUpdateECG(NotifyReadService1 &monitorService) {
    uint8_t value = 0;
    if ((lo1 == 0) && (lo2 == 0)) {
        value = ecg.read_u16() * 255 / 1023; // Convert 10-bit ADC values to 8-bit
    }
    monitorService.addValue(value);
}

int main(void) {
    red = 0; green = 1; blue = 1;    

    pc.baud(115200);
    LOG("\n--- DovetailV2 Monitor ---\n");

    LOG("Initializing BTLE...\n");
    initBluetooth(ble);
    LOG("BTLE Initialized.\n");

    NotifyReadService1 monitorService(ble);
    DeviceInformationService deviceInfo(ble, MFR_NAME, MODEL_NUM, SERIAL_NUM, HW_REV, FW_REV, SW_REV);
    DFUService dfu(ble);

    setupAdvertising(ble, (uint8_t *) SERVICES, DEVICE_NAME);
    
    initMotionProcessor();
    motionProbe.fall(&triggerAccel);

    red = 1; green = 0; blue = 1;  // Show green light for a second to show boot success.
    wait(1);
    red = 1; green = 1; blue = 1;

    // infinite loop
    while (true) {
        if (readEcg && ble.getGapState().connected) {
            readEcg = false;
            readUpdateECG(monitorService);
        }

        if (readAccel) {
            readAccel = false;
            readUpdateAccel(monitorService);
        }
        ble.waitForEvent(); // low power wait for event
    }
}
