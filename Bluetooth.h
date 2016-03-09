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


extern void onInit();
extern void onConnect();
extern void onDisconnect();
extern void onAdvertisingStarted();
extern void onAdvertisingStopped();

extern Serial pc;

class BluetoothSmart {
public:
    BluetoothSmart() :
        monitorService(NULL),
        deviceInfo(NULL),
        dfu(NULL) {
    }
    
    ~BluetoothSmart() {
        cleanUp();
    }

    void start() {
        ble.init(this, &BluetoothSmart::onInit);
    }

    void stop() {
        if (isConnected()) {
            ble.disconnect(Gap::LOCAL_HOST_TERMINATED_CONNECTION);
        } else if (isAdvertising()) {
            ble.stopAdvertising();
        }
        // Commented out shutdown as it doesn't restore services properly.
        // ble.shutdown();
        // LOG("Shutdown bluetooth.\n");
        cleanUp();
    }

    bool hasInitialized() {
        return ble.hasInitialized();
    }
        
    bool isConnected() {
        return ble.getGapState().connected;
    }
    
    bool isAdvertising() {
        return ble.getGapState().advertising;
    }

    void sleep() {
        ble.waitForEvent();
    }
    
    DovetailService& service() {
        return (*monitorService);
    }
private:
        
    void onInit(BLE::InitializationCompleteCallbackContext *context) {
        if (context->error != BLE_ERROR_NONE) {
            LOG("Bluetooth initialization failed with error %d.\n", context->error);
            return;
        }
        LOG("Bluetooth initialized.\n");

        ble.gap().onConnection(this, &BluetoothSmart::onConnection);
        ble.gap().onDisconnection(this, &BluetoothSmart::onDisconnection);
                
        // Advertising setup
        ble.clearAdvertisingPayload();
        ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                        | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
        ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                        (uint8_t *) services, sizeof(services));
        ble.accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
        ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                        (uint8_t *) deviceName, strlen(deviceName));
        ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
        ble.setAdvertisingInterval(ADVERTISING_INTERVAL_MILLIS);

        onInit();

        monitorService = new DovetailService(ble);
        deviceInfo = new DeviceInformationService(ble, MFR_NAME, MODEL_NUM, SERIAL_NUM, HW_REV, FW_REV, SW_REV);
        dfu = new DFUService(ble);

        ble.startAdvertising();
        onAdvertisingStarted();
    }
    
    void onConnection(const Gap::ConnectionCallbackParams_t *params) {
        LOG("Connected to device.\n");
        
        ble.stopAdvertising();
        onAdvertisingStopped();
        
        onConnect();
    }
    
    void onDisconnection(const Gap::DisconnectionCallbackParams_t *params) {
        LOG("Disconnected from device.\n");
        
        ble.startAdvertising();
        onAdvertisingStarted();

        onDisconnect();
    }
    
    void cleanUp() {
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