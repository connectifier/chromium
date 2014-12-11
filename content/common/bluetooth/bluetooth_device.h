// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_BLUETOOTH_BLUETOOTH_DEVICE_H_
#define CONTENT_COMMON_BLUETOOTH_BLUETOOTH_DEVICE_H_

namespace content {

// Data sent over IPC representing a Bluetooth device, corresponding to
// blink::WebBluetoothDevice.
struct CONTENT_EXPORT BluetoothDevice {
    std::string instance_id;
    base::string16 name;
};

}  // namespace content

#endif  // CONTENT_COMMON_BLUETOOTH_BLUETOOTH_DEVICE_H_
