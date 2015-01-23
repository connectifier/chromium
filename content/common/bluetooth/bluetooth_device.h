// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_BLUETOOTH_BLUETOOTH_DEVICE_H_
#define CONTENT_COMMON_BLUETOOTH_BLUETOOTH_DEVICE_H_

#include "content/common/content_export.h"
#include "base/basictypes.h"
#include "base/strings/string16.h"

#include <string>

namespace content {

// Data sent over IPC representing a Bluetooth device, corresponding to
// blink::WebBluetoothDevice.
struct CONTENT_EXPORT BluetoothDevice {
    BluetoothDevice();
    std::string instance_id;
    base::string16 name;
    uint32 device_class;
    uint16 vendor_id;
    uint16 product_id;
    uint16 product_version;
    bool paired;
    bool connected;
};

}  // namespace content

#endif  // CONTENT_COMMON_BLUETOOTH_BLUETOOTH_DEVICE_H_
