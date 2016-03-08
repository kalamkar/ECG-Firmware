#ifndef __DOVETAIL_CONFIG_H__
#define __DOVETAIL_CONFIG_H__


// LED and other timer intervals
#define CONNECTED_BLINK_INTERVAL_SECS   2
#define IDLE_TIMEOUT_SECS               60
#define SENSOR_TICKER_MICROS            1000.0f  // Trigger Sensor every 1 milliseconds


// Bluetooth config
#define DEVICE_NAME "DovetailV2"
#define MFR_NAME    "Dovetail Monitor"
#define MODEL_NUM   "Model1"
#define SERIAL_NUM  "SN1"
#define HW_REV      "hw-rev1"
#define FW_REV      "fw-rev1"
#define SW_REV      "soft-rev1"

#define ADVERTISING_INTERVAL_MILLIS 300


// Accelerometer / Motion processor config
#define DEFAULT_MPU_HZ  (100)
#define TAP_COUNT 2
#define TAP_THRESHOLD 100

// Hardware pins on Dovetail V2 board
#define LO_MINUS    p1
#define ECG_SIGNAL  p2
#define LO_PLUS     p3
#define SDN_BAR     p5

#define LED_GREEN   p12
#define LED_RED     p16
#define LED_BLUE    p15

#define MPU6050_SDA p9
#define MPU6050_SCL p8
#define MPU6050_INT p11

#define UART_RX     p18
#define UART_TX     p20


#endif // __DOVETAIL_CONFIG_H__
