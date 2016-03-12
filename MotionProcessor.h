#ifndef __MOTION_PROCESSOR_H__
#define __MOTION_PROCESSOR_H__

#include "mbed_i2c.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

extern Serial pc;

extern void onTap(unsigned char direction, unsigned char count);

class MotionProcessor {

public:

    MotionProcessor() :
        initialized(false) {
        init();
    }

    void processData() {
        unsigned long sensor_timestamp;
        short gyro[3], accel[3], sensors;
        long quat[4];
        unsigned char more = 1;

        while (more) {
            dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
            if ((sensors & INV_XYZ_ACCEL)) {
//            LOG("ACC: %d, %d, %d\n", accel[0], accel[1], accel[2]);
//            monitorService.addValue(toUint8(accel[2]));
            }
        }
    }
    
    bool hasInitialized() {
        return initialized;
    }

private:

    void init() {
        LOG("Initializing Motion Processor...\n");

        mbed_i2c_clear(MPU6050_SDA, MPU6050_SCL);
        mbed_i2c_init(MPU6050_SDA, MPU6050_SCL);

        initialized = mpu_init(0) == 0;
        if (!initialized) {
            return;
        }

        /* Get/set hardware configuration. */
        /* Wake up all sensors. */
        mpu_set_sensors(INV_XYZ_ACCEL);
        /* Push accel data into the FIFO. */
        mpu_configure_fifo(INV_XYZ_ACCEL);
        mpu_set_sample_rate(DEFAULT_MPU_HZ);
        // mpu_lp_accel_mode(DEFAULT_MPU_HZ);

        /* Read back configuration in case it was set improperly. */
        unsigned char accel_fsr;
        mpu_get_accel_fsr(&accel_fsr);

        dmp_load_motion_driver_firmware();
        dmp_register_tap_cb(&onTap);

        uint16_t dmp_features = DMP_FEATURE_TAP | DMP_FEATURE_SEND_RAW_ACCEL;
//    uint16_t dmp_features = DMP_FEATURE_PEDOMETER | DMP_FEATURE_TAP | DMP_FEATURE_ANDROID_ORIENT
//            | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL;
        dmp_enable_feature(dmp_features);
        dmp_set_fifo_rate(DEFAULT_MPU_HZ);
        mpu_set_dmp_state(1);

        dmp_set_interrupt_mode(DMP_INT_GESTURE);
        dmp_set_tap_thresh(TAP_Z, TAP_THRESHOLD);
        dmp_set_tap_count(TAP_COUNT);

        LOG("Motion Processor initialized.\n");
    }
    
    static uint8_t toUint8(short num) {
        uint16_t value = num >= 0x8000 ? 0xFF00 : num < (0-0x8000) ? 0 : num + 0x8000;
        return value >> 8;
    }
    
private:
    bool initialized;
};

#endif /* #ifndef __MOTION_PROCESSOR_H__ */