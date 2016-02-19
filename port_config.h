#ifndef __PORT_CONFIG_H__
#define __PORT_CONFIG_H__

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

#define MPU6050_ADDRESS  0x69  // 0x69 = AD0 High per datasheet

#define UART_RX     p18
#define UART_TX     p20

#endif // __PORT_CONFIG_H__
