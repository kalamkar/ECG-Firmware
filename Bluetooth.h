// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __BLUETOOTH_SMART_H__
#define __BLUETOOTH_SMART_H__

#include "BLE.h"

#include "DFUService.h"
#include "DeviceInformationService.h"
#include "NotifyService.h"

const static char     deviceName[]      = DEVICE_NAME;
const static uint16_t services[]        = { SHORT_UUID_SERVICE,
                                            GattService::UUID_DEVICE_INFORMATION_SERVICE,
                                            DFUServiceShortUUID,
                                            GattService::UUID_BATTERY_SERVICE};


extern void connectionCallback(const Gap::ConnectionCallbackParams_t *);
extern void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *);

extern Serial pc;

class BluetoothSmart {
public:
    BluetoothSmart() {
        init();
    }
        
    ~BluetoothSmart() {
        clear();
    }

    void start() {
        ble.startAdvertising();
        LOG("Advertising started.\n");
    }
    
    void stop() {
        // ble.shutdown();
        ble.stopAdvertising();
        LOG("Stopped bluetooth advertising.\n");
    }
    
    bool hasInitialized() {
        return ble.hasInitialized();
    }
        
    bool isConnected() {
        return ble.getGapState().connected;
    }
    
    void disconnect() {
        ble.disconnect(Gap::LOCAL_HOST_TERMINATED_CONNECTION);
    }
    
    void sleep() {
        ble.waitForEvent();
    }
    
    DovetailService& service() {
        return (*monitorService);
    }
private:

    void init() {
        ble.init();
        ble.onDisconnection(disconnectionCallback);
        ble.onConnection(connectionCallback);
        
        // Advertising setup
        ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                        | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
        ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                        (uint8_t *) services, sizeof(services));
        ble.accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
        ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                        (uint8_t *) deviceName, strlen(deviceName));
        ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
        ble.setAdvertisingInterval(ADVERTISING_INTERVAL_MILLIS);
        
        monitorService = new DovetailService(ble);
        deviceInfo = new DeviceInformationService(ble, MFR_NAME, MODEL_NUM, SERIAL_NUM, HW_REV, FW_REV, SW_REV);
        dfu = new DFUService(ble);
    }
    
    void clear() {
        if (monitorService != NULL) {
            delete monitorService;
            monitorService = NULL;
        }
        if (deviceInfo != NULL) {
            delete deviceInfo;
            deviceInfo = NULL;
        }
        if (dfu != NULL) {
            delete dfu;
            dfu = NULL;
        }
    }
    
private:
    BLE                         ble;
    
    DovetailService             *monitorService;
    DeviceInformationService    *deviceInfo;
    DFUService                  *dfu;
    
};

#endif /* #ifndef __BLUETOOTH_SMART_H__ */