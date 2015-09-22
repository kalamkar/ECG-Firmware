// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "BLE.h"

extern void updatesEnabledCallback(Gap::Handle_t handle);
extern void updatesDisabledCallback(Gap::Handle_t handle);    
extern void connectionCallback(const Gap::ConnectionCallbackParams_t *);
extern void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason);

void initBluetooth(BLE &ble) {
    ble.init();
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);
    ble.gattServer().onUpdatesEnabled(updatesEnabledCallback);
    ble.gattServer().onUpdatesDisabled(updatesDisabledCallback);
}

void startAdvertising(BLE &ble, uint8_t *uuid16_list, const char *deviceName) {
    // Setup advertising.
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                    | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                    uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                    (uint8_t *) deviceName, strlen(deviceName));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(300); // milli seconds
    ble.gap().startAdvertising();
}

#endif /* #ifndef __BLUETOOTH_H__ */