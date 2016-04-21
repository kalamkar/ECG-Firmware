#ifndef __MOTION_PROCESSOR_H__
#define __MOTION_PROCESSOR_H__

#include "LIS3DH.h"

#define LIS3DH_ADDR_G  (0x18 << 1)    // SA0(=SDO pin) = Ground
#define LIS3DH_ADDR_V  (0x19 << 1)    // SA0(=SDO pin) = Vdd

extern Serial   pc;

extern void onTap();

class MotionProcessor {

public:

    MotionProcessor() :
        initialized(false),
        i2c(ACCEL_SDA, ACCEL_SCL),
        wakeUp(ACCEL_INT) {
        init();
    }

    void processData() {
    }
    
    bool hasInitialized() {
        return initialized;
    }
    
    void enableDoubleTap() {
        LOG("Enabling double tap...\n");
        wakeUp.fall(&onTap);
        write_reg(LIS3DH_CLICK_CFG, 0x20); // Double Tap on Z axis
        write_reg(LIS3DH_CLICK_THS, 10);
        write_reg(LIS3DH_TIME_LIMIT, 10);
        write_reg(LIS3DH_TIME_LATENCY, 20);
        write_reg(LIS3DH_TIME_WINDOW, 255);
        LOG("CLICK CFG set to 0x%x\n", read_reg(LIS3DH_CLICK_CFG));
    }


private:

    void init() {
        LOG("Setting up accelerometer...\n");
        i2c.frequency(100);
        LOG("Who Am I 0x%x == 0x%x\n", read_reg(LIS3DH_WHO_AM_I), I_AM_LIS3DH);

        write_reg(LIS3DH_CTRL_REG1, 0x4C);
        write_reg(LIS3DH_CTRL_REG2, 0xC4);
        write_reg(LIS3DH_CTRL_REG3, 0x80);
        write_reg(LIS3DH_CTRL_REG4, 0x0);

        LOG("Control Reg 1 set to 0x%x\n", read_reg(LIS3DH_CTRL_REG1));
        LOG("Control Reg 2 set to 0x%x\n", read_reg(LIS3DH_CTRL_REG2));
        LOG("Control Reg 3 set to 0x%x\n", read_reg(LIS3DH_CTRL_REG3));
        LOG("Control Reg 4 set to 0x%x\n", read_reg(LIS3DH_CTRL_REG4));
        initialized = true;
    }
    
    uint8_t read_reg(uint8_t addr) {
        uint8_t data;
        i2c.write(LIS3DH_ADDR_G, (char*) &addr, 1, true);
        i2c.read(LIS3DH_ADDR_G, (char*) &data, 1, false);
        return data;
    }

    void write_reg(uint8_t addr, uint8_t data) {
        char dt[2];
        dt[0] = addr;
        dt[1] = data;
        i2c.write(LIS3DH_ADDR_G, dt, 2, false);
    }

    
    static uint8_t toUint8(short num) {
        uint16_t value = num >= 0x8000 ? 0xFF00 : num < (0-0x8000) ? 0 : num + 0x8000;
        return value >> 8;
    }
    
private:
    bool            initialized;
    I2C             i2c;
    InterruptIn     wakeUp;
};

#endif /* #ifndef __MOTION_PROCESSOR_H__ */