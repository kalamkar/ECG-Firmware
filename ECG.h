#ifndef __ELECTRO_CARDIO_GRAM_H__
#define __ELECTRO_CARDIO_GRAM_H__

class ECG {
public:

    ECG() :
        lo1(LO_MINUS),
        lo2(LO_PLUS),
        ecg(ECG_SIGNAL),
        ecgPower(SDN_BAR) {
    }
    
    void start() {
        ecgPower = 1;
    }
    
    void stop() {
        ecgPower = 0;
    }

    uint8_t read() {
        uint8_t value = 0;
        if ((lo1 == 0) && (lo2 == 0)) {
            value = ecg.read_u16() * 255 / 1023; // Convert 10-bit ADC values to 8-bit
        }
        return value;
    }

private:

    DigitalIn       lo1;
    DigitalIn       lo2;
    AnalogIn        ecg;
    DigitalOut      ecgPower;
};

#endif /* #ifndef __ELECTRO_CARDIO_GRAM_H__ */