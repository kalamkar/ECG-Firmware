// Copyright 2015 Dovetail Care Inc. All rights reserved.

#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "BLEDevice.h"

#define ADVERTISING_INTERVAL_MILLIS 300

// extern void connectionCallback(const Gap::ConnectionCallbackParams_t *);
extern void connectionCallback(Gap::Handle_t handle, Gap::addr_type_t peerAddrType, const Gap::address_t peerAddr, const Gap::ConnectionParams_t *params);
// extern void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *);
extern void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason);

void initBluetooth(BLEDevice &ble) {
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onConnection(connectionCallback);
}

void startAdvertising(BLEDevice &ble, uint8_t *uuid16_list, const char *deviceName) {
    // Setup advertising.
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                    | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                    uuid16_list, sizeof(uuid16_list));
    ble.accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                    (uint8_t *) deviceName, strlen(deviceName));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(ADVERTISING_INTERVAL_MILLIS);
    ble.startAdvertising();
}

#endif /* #ifndef __BLUETOOTH_H__ */