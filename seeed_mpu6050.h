#ifndef __MPU_6050_H__
#define __MPU_6050_H__

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"


#define DEFAULT_MPU_HZ  (100)

#define TAP_COUNT 2
#define TAP_THRESHOLD 50

extern Serial pc;

static signed char board_orientation[9] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
};

extern void onTap(unsigned char direction, unsigned char count);

void check_i2c_bus(void) {
    DigitalInOut scl(MPU6050_SCL);
    DigitalInOut sda(MPU6050_SDA);

    scl.input();
    sda.input();
    int scl_level = scl;
    int sda_level = sda;
    if (scl_level == 0 || sda_level == 0) {
        LOG("scl: %d, sda: %d, i2c bus is not released\r\n", scl_level, sda_level);

        scl.output();
        for (int i = 0; i < 8; i++) {
            scl = 0;
            wait_us(10);
            scl = 1;
            wait_us(10);
        }
    }

    scl.input();

    scl_level = scl;
    sda_level = sda;
    if (scl_level == 0 || sda_level == 0) {
        LOG("scl: %d, sda: %d, i2c bus is still not released\r\n", scl_level, sda_level);
    }
}

/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
static inline unsigned short inv_row_2_scale(const signed char *row) {
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx){
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;

    return scalar;
}


void initMotionProcessor() {
    LOG("Initializing Motion Processor...\n");

    check_i2c_bus();
    mbed_i2c_clear(MPU6050_SDA, MPU6050_SCL);
    mbed_i2c_init(MPU6050_SDA, MPU6050_SCL);

    int notInited = mpu_init(0);
    if (notInited) {
        LOG("failed to initialize mpu6050 %d\r\n", notInited);
        return;
    }

    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(DEFAULT_MPU_HZ);

    /* Read back configuration in case it was set improperly. */
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);

    dmp_load_motion_driver_firmware();
    dmp_set_orientation(
        inv_orientation_matrix_to_scalar(board_orientation));
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

#endif /* #ifndef __MPU_6050_H__ */